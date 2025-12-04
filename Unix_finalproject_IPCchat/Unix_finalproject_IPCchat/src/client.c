#include "../include/common.h"
#include "../include/file_handler.h" // 파일 전송 함수 포함
#include "../include/utils.h"

// [수정] 시그널 핸들러에서 사용하기 위한 전역 변수 선언
int g_public_fd;
char g_private_fifo[64];
pid_t g_pid;

// [추가] 클라이언트 전용 종료 핸들러 (Ctrl+C 대응)
void client_sigint_handler(int signo) {
    Message msg;
    msg.type = MSG_DISCONNECT;
    msg.pid = g_pid;

    // 1. 서버에 종료 메시지 전송
    write(g_public_fd, &msg, sizeof(Message));

    // 2. 내 파이프 삭제
    unlink(g_private_fifo);

    printf("\n[Client] Disconnected safely via Signal.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int public_fd, private_fd;
    // char private_fifo_name[64]; // 지역 변수 대신 전역 변수 g_private_fifo 사용
    Message msg;
    pid_t pid = getpid();

    // 전역 변수 초기화
    g_pid = pid;

    // 1. 전용 FIFO 생성 (Ch 10. mkfifo)
    sprintf(g_private_fifo, CLIENT_FIFO_TEMPLATE, pid);
    mkfifo(g_private_fifo, 0666);

    // 2. 서버에게 접속 알림 보내기
    public_fd = open(PUBLIC_FIFO, O_WRONLY);
    if (public_fd == -1) {
        perror("Public FIFO open error");
        unlink(g_private_fifo);
        exit(1);
    }
    g_public_fd = public_fd; // 전역 변수에 저장

    msg.pid = pid;
    msg.type = MSG_CONNECT;
    write(public_fd, &msg, sizeof(Message));

    // [중요] Ctrl+C 눌렀을 때 작동할 핸들러 등록
    signal(SIGINT, client_sigint_handler);

    printf("[Client] Connected to server. Start chatting!\n");

    // 3. 프로세스 분기 (Ch 7. fork)
    pid_t child_pid = fork();

    if (child_pid == 0) { 
        /* --- [자식 프로세스] : 메시지 수신 담당 (Reader) --- */
        private_fd = open(g_private_fifo, O_RDONLY);
        
        // 파일 수신용 변수
        int recv_file_fd = -1;

        while (1) {
            if (read(private_fd, &msg, sizeof(Message)) > 0) {
                if (msg.type == MSG_TEXT) {
                    printf("User %d: %s\n", msg.pid, msg.data);
                }
                else if (msg.type == MSG_FILE_START) {
                    // [추가] 파일 전송 시작 알림 수신
                    char save_name[300];
                    char *pure_filename;

                    // 경로가 포함된 경우(/Users/...) 파일명만 추출
                    pure_filename = strrchr(msg.data, '/');
                    if (pure_filename) {
                        pure_filename++; // '/' 다음 글자부터
                    } else {
                        pure_filename = msg.data;
                    }

                    // 저장할 이름 생성 (예: received_helloworld.c)
                    snprintf(save_name, sizeof(save_name), "received_%s", pure_filename);
                    
                    printf("\n[Client] Receiving file... Saving as '%s'\n", save_name);
                    
                    // 파일 생성 (쓰기 전용, 생성, 내용 초기화)
                    recv_file_fd = open(save_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (recv_file_fd == -1) {
                        perror("File open fail");
                    }
                }
                else if (msg.type == MSG_FILE_DATA) {
                    // [추가] 실제 파일 데이터 수신 및 쓰기
                    if (recv_file_fd != -1) {
                        write(recv_file_fd, msg.data, msg.data_size);
                        
                        // 진행 상황 표시 (점 찍기)
                        printf("."); 
                        fflush(stdout);
                    }
                }
            }
        }
    } else { 
        /* --- [부모 프로세스] : 키보드 입력 담당 (Writer) --- */
        char buffer[MAX_MSG_LEN];
        
        while (1) {
            fgets(buffer, MAX_MSG_LEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거

            // 명령어 처리 (/exit, /send)
            if (strcmp(buffer, "/exit") == 0) {
                break; // 루프 탈출 후 종료 처리
            } 
            else if (strncmp(buffer, "/send ", 6) == 0) {
                // [수정] 파일 전송 로직 활성화 (주석 해제)
                send_file_with_mmap(public_fd, pid, buffer + 6);
            } 
            else {
                // 일반 메시지 전송
                msg.type = MSG_TEXT;
                strcpy(msg.data, buffer);
                write(public_fd, &msg, sizeof(Message));
            }
        }
        
        // 정상 종료 처리 (/exit 입력 시)
        msg.type = MSG_DISCONNECT;
        write(public_fd, &msg, sizeof(Message));
        kill(child_pid, SIGKILL); // 자식 프로세스 종료
        unlink(g_private_fifo);   // 파이프 파일 삭제
        exit(0);
    }
}