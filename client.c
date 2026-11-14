#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
    int sock;
    struct sockaddr_in server_addr;
    char *message = "Hello from Client!";
    char buffer[BUFFER_SIZE] = {0};

    // 1. Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configure target server address (localhost 127.0.0.1)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 3. Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server on port %d.\n", PORT);

    // 4. Send message and read server reply
    send(sock, message, strlen(message), 0);
    printf("Message sent: %s\n", message);

    read(sock, buffer, BUFFER_SIZE);
    printf("Server reply: %s\n", buffer);

    // 5. Close socket
    close(sock);
    return 0;
}
