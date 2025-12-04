/* include/file_handler.h */
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <sys/types.h>

// Zero-copy file transfer using mmap()
// server_fd: Public FIFO descriptor
// my_pid: Client PID
// filepath: Target file path
void send_file_with_mmap(int server_fd, pid_t my_pid, char *filepath);

#endif