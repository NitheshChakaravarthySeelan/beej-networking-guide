#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stddef.h>
#include<arpa/inet.h>
#include<string.h>
#include<netinet/in.h>

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res,*p;
    int status;

    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0,sizeof(hints));

    hints.ai_family = AF_UNSPEC; // It could be both
    hints.ai_socktype = SOCK_STREAM; // TCP

	/*
	- Checks if "www.example.net" is already cached in your local DNS resolver.
	If not, it asks a DNS server.
	- The DNS server replies with one or more IP addresses.
		Maybe an IPv4: 192.0.2.88
		Maybe an IPv6: 2001:db8::171
	- The OS takes each IP and wraps it into a struct addrinfo node:
		ai_family = AF_INET (for IPv4) or AF_INET6 (for IPv6).
		ai_socktype = SOCK_STREAM (because we asked for TCP).
		ai_addr points to a struct sockaddr_in (IPv4) or struct sockaddr_in6 (IPv6) that stores the actual IP.
		ai_next points to the next result (linked list).
	So now res points to the head of a linked list of addresses.
	*/
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    printf("IP addresses for %s:\n\n", argv[1]);

	for (p=res;p!=NULL;p=p->ai_next) {
		void *addr;
		char *ipver;

		struct sockaddr_in* ipv4;
		struct sockaddr_in6* ipv6;
		char ipstr[INET6_ADDRSTRLEN]; 

		if (p->ai_family == AF_INET) {
			ipv4 = (struct sockaddr_in*)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		}
		else {
			ipv6 = (struct sockaddr_in6*)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
 		printf(" %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(res);
	return 0;
}
