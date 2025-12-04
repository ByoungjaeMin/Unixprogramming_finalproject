#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> // Ch 9. 메모리 매핑
#include <signal.h>   // Ch 8. 시그널

// 상수 정의
#define PUBLIC_FIFO "/tmp/public_fifo"
#define MAX_MSG_LEN 256
#define CLIENT_FIFO_TEMPLATE "/tmp/client_%d_fifo"

// 메시지 타입 (일반 대화 vs 파일 전송)
typedef enum {
    MSG_CONNECT,
    MSG_TEXT,
    MSG_FILE_START,
    MSG_FILE_DATA,
    MSG_DISCONNECT
} MsgType;

// 메시지 구조체 (통신 프로토콜)
typedef struct {
    pid_t pid;              // 보낸 프로세스 ID (식별자)
    MsgType type;           // 메시지 종류
    char data[MAX_MSG_LEN]; // 실제 데이터 (텍스트 또는 파일 청크)
    int data_size;          // 데이터 크기
} Message;

#endif