/* include/file_handler.h */
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <sys/types.h>

// mmap을 이용한 파일 전송 함수 프로토타입 선언
// server_fd: 서버 공용 파이프의 파일 기술자
// my_pid: 클라이언트 자신의 PID
// filepath: 전송할 파일의 경로
void send_file_with_mmap(int server_fd, pid_t my_pid, char *filepath);

#endif