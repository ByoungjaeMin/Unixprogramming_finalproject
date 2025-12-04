#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> // Memory Mapping (mmap)
#include <signal.h>   // Signal Handling

// Constants
#define PUBLIC_FIFO "/tmp/public_fifo"
#define MAX_MSG_LEN 256
#define CLIENT_FIFO_TEMPLATE "/tmp/client_%d_fifo"

// Message Types
typedef enum {
    MSG_CONNECT,
    MSG_TEXT,
    MSG_FILE_START,
    MSG_FILE_DATA,
    MSG_DISCONNECT,
    MSG_SERVER_SHUTDOWN // Server shutdown notification
} MsgType;

// Protocol Structure
typedef struct {
    pid_t pid;              // Sender PID
    MsgType type;           // Message type
    char data[MAX_MSG_LEN]; // Payload (Text or File chunk)
    int data_size;          // Actual data size
} Message;

#endif