/* #include<stdio.h>
 * Standard I/O library
 *
 * #include<stdlib.h>
 * Standard library for general utilities
 * Eg: Memory allocation, Program control like exit(), Conversion like atoi(), atof(), Random Numbers
 *
 * #include<unistd.h>
 * POSIX standard funcitons for Unix/Linux systems.
 * Like: fork(), pipe(), read(), write(), close(), sleep()
 * For handling file descriptors and process
 *
 * #include<errno.h>
 * Provides access to the errno variable
 * Which holds error code for system calls and library function
 * Useful with functions like socket(), connect(), bind() etc to determine the cause of failure.
 *
 * #include<string.h>
 * String and Memory manipulation
 *
 * #include<sys/types.h>
 * Defines data types used in system calls.
 * pid_t, size_t, ssize_t, off_t etc.
 * Often included when working with sockets, processors, or files
 *
 * #include<sys/socket.h>
 * Socket programming API which is Essential for TCP/UDP networking.
 *
 * #include<netinet/in.h>
 * Internet address family constants and structures
 * Which provides:
 * 	struct sockaddr_in -> IPv4 socket address.
 * 	htons(), ntohs() -> network byte order conversion.
 *
 * #include<netdb.h>
 * Network database operations like function for translating hostnames and services name to addresses
 * getaddrinfo(), gethostbyname(), gethostbyaddr()
 *
 * #include<arpa/inet.h>
 * Ip address convertion utilities
 * 	inet_pton() -> convert IP string to network binary
 * 	inet_ntop() -> convert network binary to IP string
 *
 * #include<sys/wait.h>
 * Process control like wait()
 * Macros to check exit status like WIFEXITED() etc.
 * Example: In a server, wait for forked child processes handling clients.
 *
 * #include<signal.h>
 * For Signal handling like signal(), sigaction(), raise(), kill() 
 * Like to clean up zombie child process
*/


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
#define BACKLOG 10

/*
 * When a child exits, it becomes a zombie until the parent collects its status
 * Without this, your server could accumulate zombies, eventually exhausting system resources
*/
void sigchld_handler(int s) {
	(void)s; 						// To prevent Compiler warning as we are not using this
	
	int saved_errno = errno;				// waitpid() might overwrite, so we save and rewrite
	
	/*
	 * Waits for a child process to change state.
	 * -1 to wait for any child process
	 *  NULL to say that we don't care about exit status.
	 *  WNOHANG to return immediately, if no child has exited. It makes sure that this handler doesnot block the parent.
	*/
	while(waitpid(-1,NULL,WNOHANG) >0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main() {
	/*
	 * sockaddr_storage: is a generic, large struct designed to hold any kind of socket address.
	 * 			Its only job is to reserve enough memory so we can safely store any address. Like when we need to cast it
	 * socklen_t: Usually the size of the sockaddr_storage where we store the size of the socket address
	 * sigaction: Contains multiple fields and function to handle signals from the OS
	*/
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p=servinfo; p != NULL; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sockfd == -1) {
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);									// Could not associate it with the requested IP/port.
			continue;
		}
		
		// If bind succeeds. 
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	
	freeaddrinfo(servinfo);

	if (listen(sockfd,BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);				// Makes sure that no signal is blocked by the OS.	
	sa.sa_flags = SA_RESTART;				
								
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while (1) {
		/*
		 * sin_size: The kernel will copy the client's socket address into this memory buffer during accept()
		 * During the accept call 
		 * 	Checks the listening socket sockfd for any pending connections
		 * 	If there is not connection, the process blocks(sleeps) until a client connects.
		 * 	When a client connects. 
		 * 		The kernel creates a new socket with its own file descriptor for communication with this client.
		 * 		Copies the client's address information into their_addr.
		 * 		Updates the sin_size to the actual size of the client address structure.
		 * After that we check if the connection is successfully impelemented.
		 * We will be converting the IP to Readable form
		 * 	get_in_addr(): Checks the sa_family field and returns a pointer to the actual IP address part in memory.
		 * 	inet_ntop(): Convertion takes place here.
		 * 		ss_family: tells inet_ntop whether it's IPv4 or IPv6
		 * 		Pointer to the binary IP
		 * 		Output buffer s
		 * 		Size of the buffer
		*/

		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);			// Opened by the waiter

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof(s));
		printf("server: got connection from %s\n", s);

		if (!fork()) {
			close(sockfd);				// Doesn't need to handle new incoming orders so we close it.
			if (send(new_fd, "Hello, World!", 13,0) == -1) {
				perror("send");
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);					// Parent doesn't need the child process to be open it's handled by the child.
	}
	return 0;
}