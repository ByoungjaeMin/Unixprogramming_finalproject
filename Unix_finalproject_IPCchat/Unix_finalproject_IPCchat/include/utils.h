/* include/utils.h */
#ifndef UTILS_H
#define UTILS_H

// 에러 처리 함수
void error_handling(char *message);

// 시그널 핸들러 설정 함수 (Ctrl+C 처리)
void setup_signal_handling();

// 시그널 핸들러 본체 (필요시 직접 호출용)
void sigint_handler(int signo);

#endif