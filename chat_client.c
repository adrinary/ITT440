#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int sock;

void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("\r%s> ", buffer);
        fflush(stdout);
    }
    printf("\nDisconnected from server.\n");
    exit(0);
    return NULL;
}

int main(void) {
    struct sockaddr_in server_addr;
    char input[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.189.129");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to Chat Server! Type messages and press Enter (/quit to exit).\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_handler, NULL);
    pthread_detach(recv_thread);

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        send(sock, input, strlen(input), 0);
        if (strncmp(input, "/quit", 5) == 0) {
            break;
        }
    }

    close(sock);
    return 0;
}
