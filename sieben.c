#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#define CONNECTION_LIMIT 5

int main(int argc, char *argv[])
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

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

    // actual code goes here 

    return EXIT_SUCCESS;
}
