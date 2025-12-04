/* src/utils.c */
#include "../include/common.h"
#include "../include/utils.h"

// Error handling wrapper (perror + exit)
void error_handling(char *message) {
    perror(message);
    exit(1);
}

// SIGINT (Ctrl+C) Handler
// Ensures FIFO cleanup to prevent zombie files
void sigint_handler(int signo) {
    pid_t pid = getpid();
    char private_fifo[64];

    // Construct private FIFO name
    sprintf(private_fifo, CLIENT_FIFO_TEMPLATE, pid);

    printf("\n[System] Caught signal %d (SIGINT). Cleaning up FIFO...\n", signo);

    // Remove FIFO file
    if (unlink(private_fifo) == 0) {
        printf("[System] FIFO '%s' removed successfully.\n", private_fifo);
    }

    exit(0);
}

// Register signal handler using sigaction()
void setup_signal_handling() {
    struct sigaction sa;
    
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    }
}