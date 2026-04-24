/**
 * @file server.c
 * @author Sophie Holland & Michael Bruno
 * @date 16 APR 2026
 * @brief Basic file server skeleton for CS469 final project.
 *
 * Listens on a specified port, accepts a client connection,
 * receives a requested filename, and sends the file contents
 * back in chunks.
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 5
#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256

// sets up socket, binds it to a port, and starts listening
static int create_server_socket(int port);

// handles a single client connection
static void handle_client(int client_fd);

// opens the requested file and sends it over the socket
static int send_file(int client_fd, const char *filename);

int main(int argc, char *argv[])
{
    int server_fd;
    int client_fd;
    int port;
    struct sockaddr_in client_addr;
    socklen_t client_len;

    // make sure user gave a port number
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    if (port <= 0)
    {
        fprintf(stderr, "Invalid port number.\n");
        return EXIT_FAILURE;
    }

    // create and set up server socket
    server_fd = create_server_socket(port);
    if (server_fd < 0)
    {
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d...\n", port);

    // main loop: keep accepting clients
    while (true)
    {
        client_len = sizeof(client_addr);

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("Client connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // handle request from this client
        handle_client(client_fd);

        // done with this client
        close(client_fd);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}

static int create_server_socket(int port)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr;

    // create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return -1;
    }

    // allows us to restart server without waiting for port to free up
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    // set up address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // bind socket to port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }

    // start listening for connections
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

static void handle_client(int client_fd)
{
    char filename[FILENAME_SIZE];
    ssize_t bytes_read;

    memset(filename, 0, sizeof(filename));

    // read filename from client
    bytes_read = read(client_fd, filename, sizeof(filename) - 1);
    if (bytes_read < 0)
    {
        perror("read");
        return;
    }

    if (bytes_read == 0)
    {
        fprintf(stderr, "Client disconnected before sending filename.\n");
        return;
    }

    // make sure string is null terminated
    filename[bytes_read] = '\0';

    // remove newline if it was included
    filename[strcspn(filename, "\r\n")] = '\0';

    printf("Requested file: %s\n", filename);

    // send file back to client
    if (send_file(client_fd, filename) < 0)
    {
        fprintf(stderr, "Failed to send file: %s\n", filename);
    }
}

static int send_file(int client_fd, const char *filename)
{
    FILE *fp;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    // try to open file
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        const char *msg = "ERROR: File not found or cannot be opened.\n";
        perror("fopen");
        write(client_fd, msg, strlen(msg));
        return -1;
    }

    // read file in chunks and send it
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        size_t total_sent = 0;

        // make sure entire chunk is sent (write might not send all at once)
        while (total_sent < bytes_read)
        {
            ssize_t bytes_sent = write(client_fd,
                                       buffer + total_sent,
                                       bytes_read - total_sent);

            if (bytes_sent < 0)
            {
                perror("write");
                fclose(fp);
                return -1;
            }

            total_sent += (size_t)bytes_sent;
        }
    }

    if (ferror(fp))
    {
        perror("fread");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}
