#pragma once

#include <sys/socket.h>

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr_in {
    short            sin_family;
    unsigned short   sin_port;
    struct in_addr   sin_addr;
    char             sin_zero[8];
};

struct in6_addr { 
    u_int8_t s6_addr[16]; 
};

struct sockaddr_in6 {
    sa_family_t     sin6_family;
    in_port_t       sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};
