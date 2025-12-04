Unix Final Project: IPC Chat System
1. Project Overview
This project is a multi-client chat application developed for the Unix System Programming course. It demonstrates advanced system programming concepts including Inter-Process Communication (IPC) using Named Pipes (FIFO), I/O Multiplexing, Signal Handling, and Memory Mapped File I/O.

2. Key Features & Technologies
IPC (Inter-Process Communication):

Uses Named Pipes (FIFO) for bi-directional communication between the server and multiple clients.

Public FIFO for client requests and Private FIFOs for server responses.

I/O Multiplexing:

The server utilizes select() to handle multiple client connections and administrator input simultaneously without blocking.

Advanced File Transfer:

Implements Zero-copy file transfer using mmap() (Memory Mapping) for efficient reading and transmission of large files.

Remote Shell Execution:

Clients can execute shell commands on the server using /exec.

Implemented using popen() and pipe redirection to capture standard output.

System Monitoring:

Real-time server uptime and active user count monitoring.

Graceful Shutdown:

Handles SIGINT (Ctrl+C) to safely close pipes and remove FIFO files, preventing zombie files.

3. System Architecture
Server: Central process that manages client logic, broadcasts messages, and handles system commands.

Client: Forked process architecture.

Parent Process: Handles user input (Writer).

Child Process: Handles message reception (Reader) to ensure asynchronous communication.

4. Build & Run
Prerequisites

GCC Compiler

Make utility

Compilation

Bash
make
Execution

1. Start the Server:

Bash
./bin/server
2. Start the Client(s): (Open a new terminal window)

Bash
./bin/client
5. Usage Commands
Command	Description
Normal Chat	Just type any text and press Enter to broadcast.
/info	Displays server OS info, uptime, and current user count.
/exec [cmd]	Executes a shell command on the server (e.g., /exec ls -l).
/send [file]	Sends a file to all users (e.g., /send data.txt).
/exit	Disconnects from the server and quits the program.
6. File Structure
src/: Source code (server.c, client.c, utils.c, file_handler.c)

include/: Header files (common.h, utils.h, file_handler.h)

bin/: Executables (generated after build)

Makefile: Build scrip
