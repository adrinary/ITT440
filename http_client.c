#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset, strlen, strcmp, strcat */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 5) {
        puts("Parameters: <host> <port> <method> <path> [<data> [<headers>]]");
        exit(0);
    }

    /* Where are we going to send it? */
    int portno = atoi(argv[2]) > 0 ? atoi(argv[2]) : 80;
    char *host = strlen(argv[1]) > 0 ? argv[1] : "localhost";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *message, response[65536];

    /* 1. Calculate exact buffer size needed for the HTTP request */
    message_size = 0;
    if (!strcmp(argv[3], "GET")) {
        message_size += strlen("%s %s%s%s HTTP/1.1\r\n");
        message_size += strlen(argv[3]);           /* method */
        message_size += strlen(argv[4]);           /* path */
        if (argc > 5) {
            message_size += 1;                     /* '?' separator */
            message_size += strlen(argv[5]);       /* query string */
        }
        for (i = 6; i < argc; i++)
            message_size += strlen(argv[i]) + strlen("\r\n"); /* headers */
        message_size += strlen("Host: %s\r\n");
        message_size += strlen(argv[1]);
        message_size += strlen("Connection: close\r\n");
        message_size += strlen("\r\n");
    } else {
        message_size += strlen("%s %s HTTP/1.1\r\n");
        message_size += strlen(argv[3]);           /* method */
        message_size += strlen(argv[4]);           /* path */
        for (i = 6; i < argc; i++)
            message_size += strlen(argv[i]) + strlen("\r\n"); /* headers */
        message_size += strlen("Host: %s\r\n");
        message_size += strlen(argv[1]);
        message_size += strlen("Connection: close\r\n");
        if (argc > 5)
            message_size += strlen("Content-Length: %lu\r\n") + 20;
        message_size += strlen("\r\n");
        if (argc > 5)
            message_size += strlen(argv[5]);       /* body */
    }

    /* 2. Allocate space and build the HTTP request string */
    message = malloc(message_size + 1);
    if (!message) error("ERROR allocating memory");

    if (!strcmp(argv[3], "GET")) {
        if (argc > 5)
            sprintf(message, "%s %s%s%s HTTP/1.1\r\n",
                    strlen(argv[3]) > 0 ? argv[3] : "GET",
                    strlen(argv[4]) > 0 ? argv[4] : "/",
                    strlen(argv[5]) > 0 ? "?" : "",
                    strlen(argv[5]) > 0 ? argv[5] : "");
        else
            sprintf(message, "%s %s HTTP/1.1\r\n",
                    strlen(argv[3]) > 0 ? argv[3] : "GET",
                    strlen(argv[4]) > 0 ? argv[4] : "/");

        for (i = 6; i < argc; i++) {
            strcat(message, argv[i]);
            strcat(message, "\r\n");
        }
        sprintf(message + strlen(message), "Host: %s\r\n", argv[1]);
        sprintf(message + strlen(message), "Connection: close\r\n");
        strcat(message, "\r\n");
    } else {
        sprintf(message, "%s %s HTTP/1.1\r\n",
                strlen(argv[3]) > 0 ? argv[3] : "POST",
                strlen(argv[4]) > 0 ? argv[4] : "/");

        for (i = 6; i < argc; i++) {
            strcat(message, argv[i]);
            strcat(message, "\r\n");
        }
        sprintf(message + strlen(message), "Host: %s\r\n", argv[1]);
        sprintf(message + strlen(message), "Connection: close\r\n");
        if (argc > 5)
            sprintf(message + strlen(message), "Content-Length: %lu\r\n", strlen(argv[5]));
        strcat(message, "\r\n");
        if (argc > 5)
            strcat(message, argv[5]);
    }

    /* Print constructed request */
    printf("Request:\n%s\n", message);

    /* 3. Create TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* 4. Resolve domain name to IP address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR no such host");

    /* 5. Fill server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* 6. Connect to the web server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* 7. Send the complete HTTP request (loop handles partial writes) */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);

    /* 8. Receive the complete HTTP response (loop handles TCP chunking) */
    memset(response, 0, sizeof(response));
    total = sizeof(response) - 1;
    received = 0;
    do {
        bytes = read(sockfd, response + received, total - received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received += bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* 9. Close socket and print HTTP response */
    close(sockfd);
    printf("Response:\n%s\n", response);

    free(message);
    return 0;
}
