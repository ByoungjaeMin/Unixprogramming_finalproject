#include "../include/common.h"
#include "../include/file_handler.h" 
#include "../include/utils.h"
#include <errno.h>

// Global variables for signal handler
int g_public_fd;
char g_private_fifo[64];
pid_t g_pid;

// Client SIGINT Handler: Notify server and cleanup
void client_sigint_handler(int signo) {
    Message msg;
    msg.type = MSG_DISCONNECT;
    msg.pid = g_pid;

    write(g_public_fd, &msg, sizeof(Message));
    unlink(g_private_fifo);

    printf("\n[Client] Disconnected safely via Signal.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int public_fd, private_fd;
    Message msg;
    pid_t pid = getpid();

    // 1. Initialization
    g_pid = pid;
    
    // Ignore SIGPIPE to prevent crash if server dies unexpectedly
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, client_sigint_handler);

    // 2. Create Private FIFO
    sprintf(g_private_fifo, CLIENT_FIFO_TEMPLATE, pid);
    if (mkfifo(g_private_fifo, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo error");
        exit(1);
    }

    // 3. Connect to Server (Public FIFO)
    public_fd = open(PUBLIC_FIFO, O_WRONLY);
    if (public_fd == -1) {
        printf("\n[Error] Cannot connect to server. Is the server running?\n");
        unlink(g_private_fifo);
        exit(1);
    }
    g_public_fd = public_fd;

    // 4. Send Connect Message
    msg.pid = pid;
    msg.type = MSG_CONNECT;
    write(public_fd, &msg, sizeof(Message));

    // UI: Welcome
    printf("==========================================\n");
    printf(" Connected to server!\n");
    printf(" Your username is: User %d\n", pid);
    printf("==========================================\n\n");

    // 5. Fork Process (Separate Reader / Writer)
    pid_t child_pid = fork();

    if (child_pid == 0) { 
        /* --- [Child] Reader Process --- */
        private_fd = open(g_private_fifo, O_RDONLY);
        int recv_file_fd = -1;

        while (1) {
            if (read(private_fd, &msg, sizeof(Message)) > 0) {
                
                if (msg.type == MSG_TEXT) {
                    // UI: Distinguish Server vs User
                    if (msg.pid == 0) {
                        printf("\n[Server] %s\n", msg.data);
                    } else {
                        printf("\n[User %d] %s\n", msg.pid, msg.data);
                    }
                    
                    // [Modified] Refresh Prompt only after text message
                    printf("User %d: ", g_pid);
                    fflush(stdout);
                }
                else if (msg.type == MSG_SERVER_SHUTDOWN) {
                    // [Passive Detection] Handle Server Shutdown
                    printf("\n\n==========================================\n");
                    printf(" [Critical] %s\n", msg.data);
                    printf("==========================================\n");
                    
                    kill(getppid(), SIGTERM); // Terminate Parent
                    exit(0);
                }
                else if (msg.type == MSG_FILE_START) {
                    char save_name[300];
                    char *pure_filename = strrchr(msg.data, '/');
                    pure_filename = pure_filename ? pure_filename + 1 : msg.data;
                    
                    snprintf(save_name, sizeof(save_name), "received_%s", pure_filename);
                    
                    // [Modified] Print start message
                    printf("\n[System] Receiving file... Saving as '%s'\n", save_name);
                    
                    // [Modified] Print Prompt immediately here (Clean UI)
                    printf("User %d: ", g_pid);
                    fflush(stdout);

                    recv_file_fd = open(save_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                else if (msg.type == MSG_FILE_DATA) {
                    // [Modified] Silent Mode: Just write to file, no printf, no dots
                    if (recv_file_fd != -1) {
                        write(recv_file_fd, msg.data, msg.data_size);
                        // Removed printf("."); 
                    }
                }
                
                // [Modified] Removed unconditional prompt printing here
            }
        }
    } else { 
        /* --- [Parent] Writer Process --- */
        char buffer[MAX_MSG_LEN];
        
        while (1) {
            printf("User %d: ", pid);
            fflush(stdout);

            if (fgets(buffer, MAX_MSG_LEN, stdin) == NULL) break;
            buffer[strcspn(buffer, "\n")] = 0; 

            if (strcmp(buffer, "/exit") == 0) {
                break; 
            } 
            else if (strncmp(buffer, "/send ", 6) == 0) {
                // File Transfer
                send_file_with_mmap(public_fd, pid, buffer + 6);
            } 
            else {
                // Send Text
                msg.type = MSG_TEXT;
                strcpy(msg.data, buffer);
                
                // [Active Detection] Check for broken pipe
                if (write(public_fd, &msg, sizeof(Message)) == -1) {
                    fprintf(stderr, "\n[Error] Server is down or disconnected.\n");
                    fprintf(stderr, "[System] Exiting program...\n");
                    
                    kill(child_pid, SIGKILL);
                    unlink(g_private_fifo);
                    exit(1);
                }
            }
        }
        
        // Normal Exit
        msg.type = MSG_DISCONNECT;
        write(public_fd, &msg, sizeof(Message));
        
        kill(child_pid, SIGKILL);
        unlink(g_private_fifo);
        printf("[Client] Disconnected.\n");
        exit(0);
    }
}