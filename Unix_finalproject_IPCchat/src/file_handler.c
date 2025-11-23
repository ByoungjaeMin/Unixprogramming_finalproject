#include "../include/common.h"
#include "../include/file_handler.h"

// mmap을 이용한 파일 전송 함수 (Ch 9. 메모리 매핑)
void send_file_with_mmap(int server_fd, pid_t my_pid, char *filepath) {
    int fd;
    struct stat file_info;
    char *mapped_data;
    Message msg;

    // 1. 파일 열기
    if ((fd = open(filepath, O_RDONLY)) == -1) {
        perror("File open error");
        return;
    }
    fstat(fd, &file_info);

    // 2. 메모리 매핑
    mapped_data = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_data == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        return;
    }

    printf("[System] Sending file: %s (%lld bytes)\n", filepath, (long long)file_info.st_size);

    // 3. 전송 시작 알림
    msg.pid = my_pid;
    msg.type = MSG_FILE_START;
    strncpy(msg.data, filepath, MAX_MSG_LEN);
    write(server_fd, &msg, sizeof(Message));

    // 4. 실제 데이터 전송 (Chunking Loop)
    // 한 번에 MAX_MSG_LEN 만큼 잘라서 보냄
    long sent_bytes = 0;
    long total_bytes = file_info.st_size;

    msg.type = MSG_FILE_DATA;

    while (sent_bytes < total_bytes) {
        // 남은 바이트 계산
        int chunk_size = MAX_MSG_LEN;
        if (total_bytes - sent_bytes < MAX_MSG_LEN) {
            chunk_size = total_bytes - sent_bytes;
        }

        // 메모리에서 데이터 복사
        memcpy(msg.data, mapped_data + sent_bytes, chunk_size);
        msg.data_size = chunk_size; // Message 구조체에 data_size 필드 활용

        // 전송
        write(server_fd, &msg, sizeof(Message));
        
        sent_bytes += chunk_size;
        usleep(1000); // 전송 속도 조절 (너무 빠르면 파이프 버퍼 오버플로우 가능성)
    }

    printf("[System] 전송 완료.\n");

    // 5. 매핑 해제 및 닫기
    munmap(mapped_data, file_info.st_size);
    close(fd);
}