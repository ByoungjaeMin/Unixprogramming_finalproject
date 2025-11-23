#include "../include/common.h"
#include "../include/utils.h"

#define MAX_CLIENTS 10

// 접속자 관리를 위한 간단한 배열
pid_t client_pids[MAX_CLIENTS];
int client_count = 0;

// 클라이언트 추가
void add_client(pid_t pid) {
    if (client_count < MAX_CLIENTS) {
        client_pids[client_count++] = pid;
        printf("[Server] User %d added. Total: %d\n", pid, client_count);
    } else {
        printf("[Server] Full! Cannot add User %d\n", pid);
    }
}

// 클라이언트 제거
void remove_client(pid_t pid) {
    for (int i = 0; i < client_count; i++) {
        if (client_pids[i] == pid) {
            // 뒤에 있는 요소들을 앞으로 당김
            for (int j = i; j < client_count - 1; j++) {
                client_pids[j] = client_pids[j+1];
            }
            client_count--;
            printf("[Server] User %d removed. Total: %d\n", pid, client_count);
            return;
        }
    }
}

// 메시지 브로드캐스트 (모든 클라이언트에게 전송)
void broadcast_message(Message msg) {
    char client_fifo[64];
    int fd;

    for (int i = 0; i < client_count; i++) {
        // 보낸 본인에게는 다시 보내지 않음 (선택 사항)
        if (client_pids[i] == msg.pid) continue;

        sprintf(client_fifo, CLIENT_FIFO_TEMPLATE, client_pids[i]);
        
        // Non-blocking으로 열어서 클라이언트가 죽었을 때 멈추지 않게 함
        if ((fd = open(client_fifo, O_WRONLY | O_NONBLOCK)) != -1) {
            write(fd, &msg, sizeof(Message));
            close(fd);
        }
    }
}

// 서버 종료 시그널 핸들러
void server_sigint_handler(int signo) {
    printf("\n[Server] Shutting down... Removing FIFO.\n");
    unlink(PUBLIC_FIFO);
    exit(0);
}

int main() {
    int public_fd;
    Message msg;

    // 서버 전용 시그널 핸들러 등록
    signal(SIGINT, server_sigint_handler);

    // 1. 공용 FIFO 생성
    if (mkfifo(PUBLIC_FIFO, 0666) == -1) {
        // 이미 존재할 수 있으므로 에러 무시하고 진행 시도, 또는 unlink 후 재생성
        // perror("mkfifo error"); 
    }

    printf("[Server] Server started (PID: %d). Waiting for clients...\n", getpid());

    // 2. 공용 FIFO 열기
    if ((public_fd = open(PUBLIC_FIFO, O_RDONLY)) == -1) {
        perror("open error");
        exit(1);
    }

    // 3. 무한 루프로 메시지 대기
    while (1) {
        if (read(public_fd, &msg, sizeof(Message)) > 0) {
            switch (msg.type) {
                case MSG_CONNECT:
                    add_client(msg.pid);
                    // 입장 알림 브로드캐스트 (옵션)
                    break;

                case MSG_TEXT:
                    printf("[Log] User %d: %s\n", msg.pid, msg.data);
                    broadcast_message(msg);
                    break;
                
                case MSG_FILE_START:
                    printf("[Log] File transfer started by User %d: %s\n", msg.pid, msg.data);
                    broadcast_message(msg);
                    break;

                case MSG_FILE_DATA:
                    // 파일 데이터도 그대로 중계
                    broadcast_message(msg);
                    break;

                case MSG_DISCONNECT:
                    remove_client(msg.pid);
                    break;
            }
        }
    }
    
    close(public_fd);
    unlink(PUBLIC_FIFO);
    return 0;
}