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
#include <openssl/sha.h>

#define PRIMARY_PORT 4433
#define BACKUP_PORT 4434
#define BUFFER_SIZE 256
#define RESPONSE_SIZE 1200
#define MAX_HOST 256

int try_connect(char *host, unsigned int port);
int connect_any(char *host);
void hash_data(const unsigned char *data, size_t len, char *hash_output);

int try_connect(char *host, unsigned int port) {
    int sockfd;
    struct hostent *server;
    struct sockaddr_in addr;

    server = gethostbyname(host);
    if (!server) return -1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    memset(&addr, 0, sizeof(addr));
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

void hash_data(const unsigned char *data, size_t len, char *hash_output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(data, len, hash);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }

    hash_output[SHA256_DIGEST_LENGTH * 2] = '\0';
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
        snprintf(request, sizeof(request), "GET|%s|END", buffer);

        if (write(sockfd, request, strlen(request) + 1) < 0) {
            printf("Connection lost. Reconnecting...\n");
            close(sockfd);
            sockfd = connect_any(host);
            continue;
        }

        char response[RESPONSE_SIZE];
        bzero(response, RESPONSE_SIZE);

        int n = read(sockfd, response, RESPONSE_SIZE - 1);

        if (n <= 0) {
            printf("Server disconnected. Reconnecting...\n");
            close(sockfd);
            sockfd = connect_any(host);
            continue;
        }

        response[n] = '\0';

        if (strncmp(response, "OK|", 3) == 0) {

            char received_hash[(SHA256_DIGEST_LENGTH * 2) + 1];
            char filedata[1024];

            bzero(received_hash, sizeof(received_hash));
            bzero(filedata, sizeof(filedata));

            if (sscanf(response, "OK|%64[^|]|%1023[^\n]", received_hash, filedata) == 2) {
                char calculated_hash[(SHA256_DIGEST_LENGTH * 2) + 1];

                hash_data((unsigned char *)filedata, strlen(filedata), calculated_hash);

                if (strcmp(received_hash, calculated_hash) == 0) {
                    printf("%s\n", filedata);
                    printf("SHA256 verification passed.\n");
                } else {
                    printf("ERROR file integrity check failed.\n");
                }
            } else {
                printf("ERROR invalid response from server.\n");
            }
        } else {
            printf("%s\n", response);
        }
    }

    close(sockfd);
    return 0;
}
