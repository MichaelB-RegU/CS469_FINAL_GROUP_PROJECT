/**
 * A client file designed to connect with a server. If the connection is severed
 * with the original server, it will connect to a secondary backup file.
 * A filename is inputted on the client side from a local directory, then 
 * is marshalled and sent over to the server.
 * If there is a successful file transfer, the information stored within the
 * file is sent back to the server and printed out to the user.
 * 
 * @authors Sophie Holland and Mike Bruno
 * @date 27 April 2026
 */

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PRIMARY_PORT 4433
#define BACKUP_PORT 4434
#define BUFFER_SIZE 256
#define MAX_HOST 256

int try_connect(char *host, unsigned int port) {
    int sockfd;
    struct hostent *server;
    struct sockaddr_in addr;

    server = gethostbyname(host);
    if (!server) return -1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int connect_any(char *host) {
    int sock;

    while (1) {
        sock = try_connect(host, PRIMARY_PORT);
        if (sock >= 0) {
            printf("Connected to PRIMARY\n");
            return sock;
        }

        printf("Primary failed, trying BACKUP...\n");

        sock = try_connect(host, BACKUP_PORT);
        if (sock >= 0) {
            printf("Connected to BACKUP\n");
            return sock;
        }

        printf("Both servers down. Retrying...\n");
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: client <hostname>\n");
        exit(1);
    }

    char *host = argv[1];
    char buffer[BUFFER_SIZE];

    int sockfd = connect_any(host);

    while (1) {
        printf("Enter filename (or exit): ");

        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            write(sockfd, "exit", 5);
            break;
        }

        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "GET %s", buffer);

        // SEND (with reconnect)
        if (write(sockfd, request, strlen(request) + 1) < 0) {
            printf("Connection lost. Reconnecting...\n");
            close(sockfd);
            sockfd = connect_any(host);
            continue;
        }

        // RECEIVE (with reconnect)
        bzero(buffer, BUFFER_SIZE);
        int n = read(sockfd, buffer, BUFFER_SIZE);

        if (n <= 0) {
            printf("Server disconnected. Reconnecting...\n");
            close(sockfd);
            sockfd = connect_any(host);
            continue;
        }

        if (strncmp(buffer, "OK", 2) == 0) {
            printf("%s\n", buffer + 3);
        } else {
            printf("%s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}