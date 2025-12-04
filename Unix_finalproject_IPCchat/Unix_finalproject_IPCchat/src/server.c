#include "../include/common.h"
#include "../include/utils.h"
#include <time.h> // [Add] Required for time calculations

#define MAX_CLIENTS 10

// Global variables
pid_t client_pids[MAX_CLIENTS];
int client_count = 0;
time_t start_time; // [Add] Stores server start timestamp

// Add client
void add_client(pid_t pid) {
    if (client_count < MAX_CLIENTS) {
        client_pids[client_count++] = pid;
        printf("[Server] User %d added. Total: %d\n", pid, client_count);
    } else {
        printf("[Server] Full! Cannot add User %d\n", pid);
    }
}

// Remove client
void remove_client(pid_t pid) {
    for (int i = 0; i < client_count; i++) {
        if (client_pids[i] == pid) {
            for (int j = i; j < client_count - 1; j++) {
                client_pids[j] = client_pids[j+1];
            }
            client_count--;
            printf("[Server] User %d removed. Total: %d\n", pid, client_count);
            return;
        }
    }
}

// Broadcast message
void broadcast_message(Message msg) {
    char client_fifo[64];
    int fd;

    for (int i = 0; i < client_count; i++) {
        if (client_pids[i] == msg.pid) continue;

        sprintf(client_fifo, CLIENT_FIFO_TEMPLATE, client_pids[i]);
        
        if ((fd = open(client_fifo, O_WRONLY | O_NONBLOCK)) != -1) {
            write(fd, &msg, sizeof(Message));
            close(fd);
        }
    }
}

// Signal Handler
void server_sigint_handler(int signo) {
    Message msg;
    msg.type = MSG_SERVER_SHUTDOWN;
    msg.pid = 0;
    strcpy(msg.data, "Server is shutting down by administrator.");

    printf("\n[Server] Sending shutdown signal to all clients...\n");
    broadcast_message(msg);
    
    usleep(100000); 

    printf("[Server] Shutting down... Removing FIFO.\n");
    unlink(PUBLIC_FIFO);
    exit(0);
}

// [Modified] System Information Handler (Shows REAL Server Status)
void handle_sys_info(int origin_pid) {
    FILE *fp;
    char buffer[MAX_MSG_LEN];
    Message msg;
    
    msg.type = MSG_TEXT;
    msg.pid = 0; 
    
    // 1. OS Info (Keep existing logic)
    fp = popen("uname -sr", "r"); 
    if (fp) {
        fgets(buffer, sizeof(buffer), fp);
        buffer[strcspn(buffer, "\n")] = 0;
        
        char final_msg[MAX_MSG_LEN];
        sprintf(final_msg, "Host OS: %s", buffer); // Changed label to Host OS
        
        printf("[Server Log] Reporting: %s\n", final_msg);
        
        strcpy(msg.data, final_msg);
        broadcast_message(msg);
        pclose(fp);
    }

    // 2. [NEW] Chat Server Status (Uptime & User Count)
    time_t now = time(NULL);
    double elapsed = difftime(now, start_time);
    
    int hours = (int)elapsed / 3600;
    int minutes = ((int)elapsed % 3600) / 60;
    int seconds = (int)elapsed % 60;

    char status_msg[MAX_MSG_LEN];
    // Format: "Server Status: Up [00:05:30], Users [2/10]"
    sprintf(status_msg, "Server Status: Up [%02d:%02d:%02d], Users [%d/%d]", 
            hours, minutes, seconds, client_count, MAX_CLIENTS);

    printf("[Server Log] Reporting: %s\n", status_msg);

    strcpy(msg.data, status_msg);
    broadcast_message(msg);
}

// Remote Exec Handler
void handle_remote_exec(char *command, int origin_pid) {
    FILE *fp;
    char buffer[MAX_MSG_LEN - 100]; 
    Message msg;

    char cmd_with_stderr[MAX_MSG_LEN];
    sprintf(cmd_with_stderr, "%s 2>&1", command);

    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) return;

    msg.type = MSG_TEXT;
    msg.pid = 0; 

    sprintf(msg.data, ">> Executing '%s' result:", command);
    broadcast_message(msg);

    printf("[Server Log] Command output start:\n");
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) < 1) continue;

        printf("    | %s\n", buffer); 

        msg.type = MSG_TEXT;
        msg.pid = 0;
        strcpy(msg.data, buffer);
        broadcast_message(msg);
        
        line_count++;
        if (line_count > 10) {
            strcpy(msg.data, "...(output truncated)...");
            broadcast_message(msg);
            break;
        }
        usleep(10000); 
    }
    printf("[Server Log] Command output end.\n");
    pclose(fp);
}

int main() {
    int public_fd;
    Message msg;
    char server_input[MAX_MSG_LEN]; 
    fd_set read_fds, temp_fds;
    int max_fd;

    // [New] Record start time
    start_time = time(NULL);

    signal(SIGINT, server_sigint_handler);

    if (mkfifo(PUBLIC_FIFO, 0666) == -1) { }

    printf("[Server] Server started (PID: %d).\n", getpid());
    printf("[Server] You can type commands or messages here to broadcast.\n");

    if ((public_fd = open(PUBLIC_FIFO, O_RDWR)) == -1) {
        perror("open error");
        exit(1);
    }

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(public_fd, &read_fds);
    max_fd = (public_fd > STDIN_FILENO) ? public_fd : STDIN_FILENO;

    while (1) {
        temp_fds = read_fds; 
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) {
            perror("select error");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &temp_fds)) {
            if (fgets(server_input, MAX_MSG_LEN, stdin) != NULL) {
                server_input[strcspn(server_input, "\n")] = 0;
                if (strlen(server_input) > 0) {
                    msg.type = MSG_TEXT;
                    msg.pid = 0; 
                    sprintf(msg.data, "%s", server_input);
                    printf("[Log] Server Announcement: %s\n", msg.data);
                    broadcast_message(msg);
                }
            }
        }

        if (FD_ISSET(public_fd, &temp_fds)) {
            if (read(public_fd, &msg, sizeof(Message)) > 0) {
                switch (msg.type) {
                    case MSG_CONNECT:
                        add_client(msg.pid);
                        break;

                    case MSG_TEXT:
                        printf("[Log] User %d: %s\n", msg.pid, msg.data);
                        if (strcmp(msg.data, "/info") == 0) {
                            handle_sys_info(msg.pid);
                        } 
                        else if (strncmp(msg.data, "/exec ", 6) == 0) {
                            handle_remote_exec(msg.data + 6, msg.pid);
                        } 
                        else {
                            broadcast_message(msg);
                        }
                        break;
                    
                    case MSG_FILE_START:
                    case MSG_FILE_DATA:
                        if(msg.type == MSG_FILE_START) 
                            printf("[Log] File transfer: %s\n", msg.data);
                        broadcast_message(msg);
                        break;

                    case MSG_DISCONNECT:
                        remove_client(msg.pid);
                        break;
                    case MSG_SERVER_SHUTDOWN:
                        break;
                }
            }
        }
    }
    
    close(public_fd);
    unlink(PUBLIC_FIFO);
    return 0;
}