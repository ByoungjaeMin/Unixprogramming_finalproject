#include "../include/common.h"
#include "../include/file_handler.h" // 파일 전송 함수 포함
#include "../include/utils.h"

int main(int argc, char *argv[]) {
    int public_fd, private_fd;
    char private_fifo_name[64];
    Message msg;
    pid_t pid = getpid();

    // 1. 전용 FIFO 생성 (Ch 10. mkfifo)
    sprintf(private_fifo_name, CLIENT_FIFO_TEMPLATE, pid);
    mkfifo(private_fifo_name, 0666);

    // 2. 서버에게 접속 알림 보내기
    public_fd = open(PUBLIC_FIFO, O_WRONLY);
    msg.pid = pid;
    msg.type = MSG_CONNECT;
    write(public_fd, &msg, sizeof(Message));

    printf("[Client] Connected to server. Start chatting!\n");

    // 3. 프로세스 분기 (Ch 7. fork)
    pid_t child_pid = fork();

    if (child_pid == 0) { 
        /* --- [자식 프로세스] : 메시지 수신 담당 (Reader) --- */
        private_fd = open(private_fifo_name, O_RDONLY);
        
        while (1) {
            if (read(private_fd, &msg, sizeof(Message)) > 0) {
                if (msg.type == MSG_TEXT) {
                    printf("User %d: %s\n", msg.pid, msg.data);
                }
                // TODO: 파일 전송 메시지 수신 시 처리 로직 추가
            }
        }
        // 자식 프로세스 종료 시 처리
    } else { 
        /* --- [부모 프로세스] : 키보드 입력 담당 (Writer) --- */
        char buffer[MAX_MSG_LEN];
        
        // 시그널 핸들러 등록 (Ctrl+C 눌렀을 때 종료 처리) - Ch 8. signal/sigaction
        // signal(SIGINT, sigint_handler); // utils.c에 구현 필요

        while (1) {
            fgets(buffer, MAX_MSG_LEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거

            // 명령어 처리 (/exit, /send)
            if (strcmp(buffer, "/exit") == 0) {
                break; // 루프 탈출 후 종료 처리
            } 
            else if (strncmp(buffer, "/send ", 6) == 0) {
                // 파일 전송 로직 (Ch 9. mmap 활용)
                // send_file_with_mmap(public_fd, pid, buffer + 6);
            } 
            else {
                // 일반 메시지 전송
                msg.type = MSG_TEXT;
                strcpy(msg.data, buffer);
                write(public_fd, &msg, sizeof(Message));
            }
        }
        
        // 종료 처리
        msg.type = MSG_DISCONNECT;
        write(public_fd, &msg, sizeof(Message));
        kill(child_pid, SIGKILL); // 자식 프로세스 종료 (Ch 8. kill)
        unlink(private_fifo_name); // 파이프 파일 삭제
        exit(0);
    }
}