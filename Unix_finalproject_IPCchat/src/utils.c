/* src/utils.c */
#include "../include/common.h"
#include "../include/utils.h"

// 에러 메시지를 출력하고 프로그램을 종료하는 함수
// Ch 1. 에러 처리 (perror, exit)
void error_handling(char *message) {
    perror(message);
    exit(1);
}

// SIGINT (Ctrl+C) 시그널을 처리하는 핸들러
// Ch 8. 시그널, Ch 3. 파일 삭제 (unlink)
void sigint_handler(int signo) {
    pid_t pid = getpid();
    char private_fifo[64];

    // 현재 프로세스의 PID를 이용해 파이프 이름 재구성
    sprintf(private_fifo, CLIENT_FIFO_TEMPLATE, pid);

    // 화면에 종료 메시지 출력
    printf("\n[System] Caught signal %d (SIGINT). Cleaning up FIFO...\n", signo);

    // 파이프 파일 삭제 (중요: 이걸 안 하면 좀비 파일이 남음)
    if (unlink(private_fifo) == 0) {
        printf("[System] FIFO '%s' removed successfully.\n", private_fifo);
    } else {
        // 서버 프로세스이거나 이미 삭제된 경우 등은 무시
    }

    // 프로세스 종료
    // Ch 7. 프로세스 종료 (exit)
    exit(0);
}

// 시그널 핸들러 등록 함수
void setup_signal_handling() {
    struct sigaction sa;
    
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // SIGINT 시그널에 대해 핸들러 등록
    // Ch 8. sigaction
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    }
}