#include "../include/common.h"
#include "../include/file_handler.h"

// File transfer function using Memory Mapping (mmap)
void send_file_with_mmap(int server_fd, pid_t my_pid, char *filepath) {
    int fd;
    struct stat file_info;
    char *mapped_data;
    Message msg;

    // 1. Open source file
    if ((fd = open(filepath, O_RDONLY)) == -1) {
        perror("File open error");
        return;
    }
    fstat(fd, &file_info);

    // 2. Map file to memory
    mapped_data = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_data == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        return;
    }

    printf("[System] Sending file: %s (%lld bytes)\n", filepath, (long long)file_info.st_size);

    // 3. Notify start of transfer
    msg.pid = my_pid;
    msg.type = MSG_FILE_START;
    strncpy(msg.data, filepath, MAX_MSG_LEN);
    write(server_fd, &msg, sizeof(Message));

    // 4. Data transfer loop (Chunking)
    long sent_bytes = 0;
    long total_bytes = file_info.st_size;

    msg.type = MSG_FILE_DATA;

    while (sent_bytes < total_bytes) {
        // Calculate chunk size
        int chunk_size = MAX_MSG_LEN;
        if (total_bytes - sent_bytes < MAX_MSG_LEN) {
            chunk_size = total_bytes - sent_bytes;
        }

        // Copy from memory map
        memcpy(msg.data, mapped_data + sent_bytes, chunk_size);
        msg.data_size = chunk_size;

        write(server_fd, &msg, sizeof(Message));
        
        sent_bytes += chunk_size;
        usleep(1000); // Flow control to prevent pipe overflow
    }

    printf("[System] File transfer complete.\n");

    // 5. Cleanup (Unmap & Close)
    munmap(mapped_data, file_info.st_size);
    close(fd);
}