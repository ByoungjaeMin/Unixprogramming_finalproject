IPC-Based Local Chat and File Transfer System

final project for a Unix System Programming course. It implements a local network chat and file transfer system using FIFO (Named Pipes) and Memory Mapping (mmap), without relying on socket libraries. It demonstrates inter-process communication (IPC) techniques native to Unix systems.

Key Features

Multi-Client Chat

Supports simultaneous conversation among multiple users via a central server that broadcasts messages.

Concurrency via Process Splitting

The client uses fork() to separate message input (parent process) and message reception (child process).

Allows receiving messages from others in real-time while typing.

High-Speed File Transfer

Transfers files by mapping them into memory using mmap().

Reduces disk I/O overhead and improves transfer efficiency compared to standard read/write operations.

Handles large files reliably by splitting them into chunks.

Safe Termination (Signal Handling)

Intercepts SIGINT (Ctrl+C) to prevent immediate termination.

Performs cleanup operations, such as unlinking FIFO files, before exiting to ensure no garbage files remain.

Development Environment and Technologies

Language: C

OS: Linux (Ubuntu recommended)

Build System: Make

Core Technologies:

IPC: mkfifo, open, read, write (Communication)

Process Management: fork, waitpid, exit (Multitasking)

Memory Management: mmap, munmap (File Handling)

Signal Handling: sigaction, signal (Exception Handling)

Installation and Build

Open a terminal in the source code directory and run the following command:

$ make

Upon successful compilation, the server and client executables will be created in the bin directory.

Usage

Run Server (Must be started first)
The server creates a public FIFO and waits for client connections.

$ ./bin/server

Run Client (In a new terminal)
Open a new terminal window to run a client. You can open multiple terminals to test multi-user chatting.

$ ./bin/client

Commands

The following commands are available in the client chat window:

Send Message: Type text and press Enter.

Send File: /send [filepath]
Example: /send data.txt

Exit Program: Type /exit or press Ctrl+C.
