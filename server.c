/**
 * A server file designed to connect with a client. For the purposes of 
 * this project, the user must create two concurrent file servers in the 
 * in the event that the connection is severed with the primary server.
 * 
 * @authors Sophie Holland and Mike Bruno
 * @date 27 April 2026
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#ifdef __linux__
#include <asm-generic/socket.h>
#endif

#define DEFAULT_PORT 4433
#define BUFFER_SIZE 256
#define EXPORT_DIR "./exported_dir"

int create_socket(unsigned int port) {
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        fprintf(stderr, "Server socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Bind error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(s, 5) < 0) {
        fprintf(stderr, "Listen error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %u\n", port);
    return s;
}

int main(int argc, char *argv[]) {
    unsigned int port = (argc == 2) ? atoi(argv[1]) : DEFAULT_PORT;

    int listenfd = create_socket(port);

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        char buffer[BUFFER_SIZE];

        int clientfd = accept(listenfd, (struct sockaddr*)&client, &len);
        if (clientfd < 0) continue;

        printf("Client connected\n");

        while (1) {
            bzero(buffer, BUFFER_SIZE);

            int n = read(clientfd, buffer, BUFFER_SIZE - 1);
            if (n <= 0) break;

            buffer[n] = '\0';

            if (strcmp(buffer, "exit") == 0) break;

            char filename[BUFFER_SIZE];
            if (sscanf(buffer, "GET %s", filename) != 1) {
                char *msg = "ERROR invalid request";
                write(clientfd, msg, strlen(msg) + 1);
                continue;
            }

            if (strstr(filename, "..")) {
                char *msg = "ERROR forbidden path";
                write(clientfd, msg, strlen(msg) + 1);
                continue;
            }

            char path[512];
            snprintf(path, sizeof(path), "%s/%s", EXPORT_DIR, filename);

            FILE *fp = fopen(path, "r");
            if (!fp) {
                char *msg = "ERROR file not found";
                write(clientfd, msg, strlen(msg) + 1);
                continue;
            }

            char filedata[1024] = {0};
            fread(filedata, 1, sizeof(filedata) - 1, fp);
            fclose(fp);

            char response[1200];
            snprintf(response, sizeof(response), "OK %s", filedata);

            write(clientfd, response, strlen(response) + 1);
        }

        close(clientfd);
        printf("Client disconnected\n");
    }

    return 0;
}
