#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define PORT "12345"
#define MAXDATASIZE 100						// Maximum buffer size for data transfer 

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}

	int sockfd, numbytes;					// Initializing the socket and the total bytes
	char buf[MAXDATASIZE];					// Initializing the buffer
	struct addrinfo hints, *clientinfo, *p;
	char s[INET6_ADDRSTRLEN];				// Storing the len of the ip address
	int rv; 

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &clientinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	/*
	 * We will be looping through all the result and connect to the first available 
	 * As this is client we don't need any specific port to bind to so while connecting the OS automatically connects to a port.
	 *
	*/
	for (p = clientinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr*) p->ai_addr),s, sizeof(s)); // Don't need to cast the p->ai_addr in here just for clarity as it's already type (struct sockaddr *)

		printf("client: attempting connection to %s\n", s);

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*) p->ai_addr), s, sizeof(s));
	printf("client: connected to %s\n", s);

	freeaddrinfo(clientinfo);

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
	}

	buf[numbytes] = '\0';
	printf("client: received '%s'\n", buf);
	close(sockfd);
	return 0;
}



	
