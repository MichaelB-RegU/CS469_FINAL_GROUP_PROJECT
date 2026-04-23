/**
 * @file client.c
 * @author Sophie Holland & Michael Bruno
 * @date 23 APR 2026
 * @brief Basic file client skeleton for CS469 final project.
 *
 * Connects to server, sends filename, receives file contents,
 * and prints them to stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

static int connect_to_server(const char *ip, int port);

int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *filename = argv[3];

    if (port <= 0)
    {
        fprintf(stderr, "Invalid port number.\n");
        return EXIT_FAILURE;
    }

    // connect to server
    sockfd = connect_to_server(server_ip, port);
    if (sockfd < 0)
    {
        return EXIT_FAILURE;
    }

    // send filename (server expects raw string)
    ssize_t sent = write(sockfd, filename, strlen(filename));
    if (sent < 0)
    {
        perror("write (filename)");
        close(sockfd);
        return EXIT_FAILURE;
    }

    write(sockfd, "\n", 1);

    printf("Requested file: %s\n\n", filename);

    // read response (file contents or error message)
    while ((bytes_received = read(sockfd, buffer, sizeof(buffer))) > 0)
    {
        fwrite(buffer, 1, bytes_received, stdout);
    }

    if (bytes_received < 0)
    {
        perror("read");
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

static int connect_to_server(const char *ip, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
