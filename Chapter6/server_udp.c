#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unitstd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "12345"
#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char buf[MAXBUFLEN];
	int numbytes;
	struct sockaddr_storage their_addr;		// Defines the client's address not our address
	char s[INET6_ADDRSTRLEN];
	socklen_t addr_len;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 			// No requirements what is the local IP, just bind to all available interfaces
	
	if ((rv = getaddrinfo(NULL,PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p=servinfo; p!= NULL; p=p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sockfd == -1) {
			continue;
		}

		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);			// If we run multiple server on the same port this error could occur
			continue;
		}

		break;
	}

	if (p==NULL) {
		fprintf(stderr, "listener: failed to bind\n");
		exit(1);
	}

	freeaddrinfo(servinfo);
	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof(their_addr);

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1,0,(struct sockaddr*)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s)));
	
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);
	return 0;
}
