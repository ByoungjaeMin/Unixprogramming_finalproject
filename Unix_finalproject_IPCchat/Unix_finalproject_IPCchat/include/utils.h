/* include/utils.h */
#ifndef UTILS_H
#define UTILS_H

// Print error message and exit
void error_handling(char *message);

// Setup signal handler (SIGINT)
void setup_signal_handling();

// Signal handler implementation
void sigint_handler(int signo);

#endif