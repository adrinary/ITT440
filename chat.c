#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define DEFAULT_PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_ip> [listen_port] [target_port]\n", argv[0]);
        return 1;
    }

    char *target_ip = argv[1];
    int listen_port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;
    int target_port = (argc >= 4) ? atoi(argv[3]) : DEFAULT_PORT;

    int listen_fd, peer_fd = -1;
    struct sockaddr_in server_addr, target_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. Create and bind listening socket (starts as Server)
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Waiting for chat request on port %d...\n", listen_port);
    printf("Press ENTER to connect to %s:%d...\n", target_ip, target_port);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(listen_fd, &readfds);
        int max_fd = (listen_fd > STDIN_FILENO) ? listen_fd : STDIN_FILENO;

        if (peer_fd != -1) {
            FD_SET(peer_fd, &readfds);
            if (peer_fd > max_fd) max_fd = peer_fd;
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        // Case A: Incoming connection on listening socket
        if (FD_ISSET(listen_fd, &readfds)) {
            int incoming_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (incoming_fd >= 0) {
                if (peer_fd == -1) {
                    peer_fd = incoming_fd;
                    printf("Hello\n");
                } else {
                    // Already chatting: reject 3rd machine
                    char *busy_msg = "Sorry I'm Busy\n";
                    send(incoming_fd, busy_msg, strlen(busy_msg), 0);
                    close(incoming_fd);
                }
            }
        }

        // Case B: Keyboard input
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                break;
            }
            if (peer_fd == -1) {
                // Pressing ENTER initiates connection to target IP
                if (buffer[0] == '\n') {
                    peer_fd = socket(AF_INET, SOCK_STREAM, 0);
                    memset(&target_addr, 0, sizeof(target_addr));
                    target_addr.sin_family = AF_INET;
                    target_addr.sin_port = htons(target_port);
                    inet_pton(AF_INET, target_ip, &target_addr.sin_addr);

                    if (connect(peer_fd, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
                        perror("connect");
                        close(peer_fd);
                        peer_fd = -1;
                    } else {
                        printf("Hello\n");
                    }
                }
            } else {
                // Send chat message to active peer
                send(peer_fd, buffer, strlen(buffer), 0);
            }
        }

        // Case C: Message received from connected peer
        if (peer_fd != -1 && FD_ISSET(peer_fd, &readfds)) {
            int bytes = recv(peer_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                printf("Peer disconnected.\n");
                close(peer_fd);
                peer_fd = -1;
                break;
            }
            buffer[bytes] = '\0';
            printf("Peer: %s", buffer);
            if (strncmp(buffer, "Sorry I'm Busy", 14) == 0) {
                close(peer_fd);
                peer_fd = -1;
                break;
            }
        }
    }

    if (peer_fd != -1) close(peer_fd);
    close(listen_fd);
    return 0;
}
