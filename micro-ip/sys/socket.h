#pragma once 

#include <sys/types.h>

#define	SOL_SOCKET (0xffff)
#define	SO_REUSEADDR (0x0004)
#define	AF_INET	2
#define	AF_INET6 24

typedef unsigned short sa_family_t;
typedef unsigned socklen_t;

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[32];
};

struct sockaddr_storage {
    sa_family_t ss_family;
    char sa_data[32];
};

enum __socket_type {
  SOCK_DGRAM = 2
};

#ifdef __cplusplus
extern "C" {
#endif

int socket(int domain, int type, int protocol);
int	setsockopt(int, int, int, const void *, socklen_t);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
    struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);

int close(int fd);

#ifdef __cplusplus
}
#endif

