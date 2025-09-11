// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT "3490"   // the port we’re listening on
#define BACKLOG 10    // how many pending connections queue will hold

int main() {
    struct addrinfo hints, *res, *p;
    int sockfd, new_fd;
    int yes = 1;
    char msg[] = "Hello from server!\n";

    // prepare hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // use my IP

    // get local address info
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    // loop through results and bind
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    // start listening
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    // accept a client
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
    if (new_fd == -1) {
        perror("accept");
        exit(1);
    }

    // send message to client
    send(new_fd, msg, sizeof msg, 0);

    close(new_fd);
    close(sockfd);
    return 0;
}
