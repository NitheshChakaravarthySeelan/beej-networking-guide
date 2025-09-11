#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct addrinfo {
	int ai_flags; // AI_PASSIVE, AI_CANONNAME, etc.
	int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
	int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
	int ai_protocol; // use 0 for "any"
	size_t ai_addrlen; // size of ai_addr in bytes
	struct sockaddr *ai_addr; // struct sockaddr_in or _in6
	char *ai_canonname; // full canonical hostname
	struct addrinfo *ai_next; // linked list, next node 
};

struct sockaddr {
	unsigned short sa_family; // address family, AF_xxx will be IPv4, etc.
	char sa_data[14]; // 14 bytes of protocol address
};

// To deal with struct sockaddr, programmers created a parallel structure: struct sockaddr_in (“in” for
// “Internet”) to be used with IPv4.

// (IPv4 only--see struct in6_addr for IPv6)
// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};

// (IPv4 only--see struct sockaddr_in6 for IPv6)
struct sockaddr_in {
    short int sin_family; // Address family, AF_INET
    unsigned short int sin_port; // Port number
    struct in_addr sin_addr; // Internet address
    unsigned char sin_zero[8]; // Same size as struct sockaddr
};

struct in6_addr {
    unsigned char s6_addr[16]; // IPv6 address
};

// (IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)
struct sockaddr_in6 {
    u_int16_t sin6_family; // address family, AF_INET6
    u_int16_t sin6_port; // port, Network Byte Order
    u_int32_t sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr; // IPv6 address
    u_int32_t sin6_scope_id; // Scope ID
};

// IP Address inet_pton() -> Converts an IP address in numbers and dots notattion into either a struct_in_addr or a struct_in6_addr
