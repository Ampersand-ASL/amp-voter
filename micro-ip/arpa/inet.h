#pragma once

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *inet_ntop(int af, const void* src, char* dst, socklen_t size);
uint16_t ntohs(uint16_t netshort);
int inet_pton(int af, const char* src, void* dst);
uint16_t htons(uint16_t hostshort);

#ifdef __cplusplus
}
#endif
