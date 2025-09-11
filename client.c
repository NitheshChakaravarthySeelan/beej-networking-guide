// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         // for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT "3490"

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res, *p;
    int sockfd;
    char buf[100];
    int numbytes;

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    if (getaddrinfo(argv[1], PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    if ((numbytes = recv(sockfd, buf, sizeof buf - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);

    close(sockfd);
    return 0;
}
