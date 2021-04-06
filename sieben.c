#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#define CONNECTION_LIMIT 5

typedef struct connection_t {
    int client_fd, proxy_fd;
} connection_t;

void *handle_client(void *connection_ptr)
{
    connection_t const connection = *(connection_t*) connection_ptr;
    uint8_t buf[1024];

    while (true) {
        ssize_t length = read(connection.client_fd, buf, sizeof buf);
        
        if (length <= 0) break;
        if (write(connection.proxy_fd, buf, length) <= 0) break;
    }

    close(connection.proxy_fd);
    close(connection.client_fd);

    return NULL;
}

void *handle_proxy(void *connection_ptr)
{
    connection_t const connection = *(connection_t*) connection_ptr;
    uint8_t buf[1024];

    while (true) {
        ssize_t length = read(connection.proxy_fd, buf, sizeof buf);
        
        if (length <= 0) break;
        if (write(connection.client_fd, buf, length) <= 0) break;
    }

    close(connection.proxy_fd);
    close(connection.client_fd);

    return NULL;
}

void *handle_connection(void *client_fd_ptr)
{
    int const client_fd = *(int*) client_fd_ptr;
    int const proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (proxy_fd == -1) {
        close(client_fd);
        close(proxy_fd);
        return NULL;
    }

    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(25565)
    };


    // testing ip
    if (inet_pton(AF_INET, "192.168.1.40", &server_address.sin_addr) <= 0) {
        close(client_fd);
        close(proxy_fd);
        return NULL;
    }

    if (connect(proxy_fd, (struct sockaddr*) &server_address, sizeof server_address) != 0) {
        close(client_fd);
        close(proxy_fd);
        return NULL;
    }

    connection_t connection = {
        .client_fd = client_fd,
        .proxy_fd = proxy_fd
    };

    pthread_t client_thread, proxy_thread;

    pthread_create(&client_thread, NULL, handle_client, &connection);
    pthread_create(&proxy_thread, NULL, handle_proxy, &connection);

    pthread_join(client_thread, NULL);
    pthread_join(proxy_thread, NULL);

    close(client_fd);
    close(proxy_fd);

    return NULL;
}

int main(int argc, char *argv[])
{
    int const server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket failed\n");
        return EXIT_FAILURE;
    }

    int opt = 1;
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt) == -1) {
        perror("setsockopt failed\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = {
            .s_addr = INADDR_ANY
        },
        .sin_port = htons(25565)
    };

    if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof server_addr) == -1) {
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, CONNECTION_LIMIT) == -1) {
        perror("listen failed\n");
        return EXIT_FAILURE;
    }

    while (true) {

        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            close(client_fd);
            continue;
        }

        pthread_t thread;

        pthread_create(&thread, NULL, handle_connection, &client_fd);
    }

    return EXIT_SUCCESS;
}
