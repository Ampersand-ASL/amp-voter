/**
 * Copyright (C) 2026, Bruce MacKinnon KC1FSZ
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <arpa/inet.h>
#include <netinet/in.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// This is where we track the sockets
#define MAX_SOCKETS (4)

struct impl_socket {
    int active;
    int fd;
    int type;
    struct udp_pcb* u;
    // Be careful not to leak these!
    struct pbuf* rxBufs[8];
    struct sockaddr_in rxAddrs[8];
    unsigned rxLen;
};

static struct impl_socket Sockets[MAX_SOCKETS] = { };

// We start at 3 to avoid confusion
static int FdCounter = 3;

// This callback is dispatched by cyw43_arch_poll() when UDP data has been received
static void UdpRx(void *arg, struct udp_pcb *pcb, struct pbuf *p, 
    const ip_addr_t *addr, u16_t port) {

    struct impl_socket* socket = (struct impl_socket*)arg;

    // If we've got room then take possession of the buffer, otherwise
    // free it immediately.
    if (socket->active && socket->rxLen < 8) {

        // Push the pbuf onto the stack
        socket->rxBufs[socket->rxLen] = p;

        // NOTE: Here the address appears to be in little endian format
        uint32_t rxAddr = ip_addr_get_ip4_u32(addr);
        // Change into native format (first octet in high-order position)
        uint8_t a = rxAddr & 0xff;
        uint8_t b = (rxAddr >> 8) & 0xff;
        uint8_t c = (rxAddr >> 16) & 0xff;
        uint8_t d = (rxAddr >> 24) & 0xff;
        rxAddr = (a << 24) | (b << 16) | (c << 8) | d;

        socket->rxAddrs[socket->rxLen].sin_family = AF_INET;
        socket->rxAddrs[socket->rxLen].sin_addr.s_addr = rxAddr;
        socket->rxAddrs[socket->rxLen].sin_port = port;
        
        socket->rxLen++;
    } else {
        // #### TODO OVERFLOW COUNTER
        pbuf_free(p);
    }
}

const char *inet_ntop(int af, const void* src, char* dst, socklen_t size) {
    assert(0);
    return 0;
}

int inet_pton(int af, const char* src, void* dst) {
    if (af == AF_INET) {

        uint32_t result = 0;
        char acc[8];
        uint32_t accLen = 0;
        const char *p = src;
        uint32_t octets = 4;

        while (1) {
            if (*p == '.' || *p == 0) {
                acc[accLen] = 0;
                // Shift up
                result <<= 8;
                // Accumulate LSB
                result |= (uint8_t)atoi(acc);
                accLen = 0;
                // Count octets
                octets++;
                // Done yet?
                if (octets == 4 || *p == 0) {
                    break;
                }
            }
            else {
                acc[accLen++] = *p;
            }
            p++;
        }
        memcpy(dst, &result, 4);
        return 1;
    }
    else {
        return 0;
    }
}

int socket(int domain, int type, int protocol) {
    if (domain == AF_INET) {
        if (type == SOCK_DGRAM) {
            // Find a free socket
            int ix = -1;
            for (unsigned i = 0; i < MAX_SOCKETS && ix == -1; i++)
                if (!Sockets[i].active)
                    ix = i;
            // This means there are no more sockets available
            if (ix == -1)
                return -1;
            Sockets[ix].active = 1;
            Sockets[ix].fd = FdCounter++;
            Sockets[ix].type = type;
            Sockets[ix].u = udp_new_ip_type(IPADDR_TYPE_ANY);
            Sockets[ix].rxLen = 0;
            // Install a receiver on this socket
            udp_recv(Sockets[ix].u, UdpRx, Sockets + ix);
            return Sockets[ix].fd;
        }
        else assert(0);
    } 
    else {
        assert(0);
    }
    return -1;
}

int close(int fd) {
    // Find one matching the FD
    int ix = -1;
    for (unsigned i = 0; i < MAX_SOCKETS && ix == -1; i++)
        if (Sockets[i].active && Sockets[i].fd == fd)
            ix = i;
    if (ix == -1)
        return -1;
    Sockets[ix].active = 0;
    // Clean up any remaining buffers
    for (unsigned i = 0; i < Sockets[ix].rxLen; i++)
        pbuf_free(Sockets[ix].rxBufs[i]);
    if (Sockets[ix].type == SOCK_DGRAM) {
        udp_remove(Sockets[ix].u);
    }
    return 0;
}

static int _findSocket(int fd) {
    // Find one matching the FD
    for (unsigned i = 0; i < MAX_SOCKETS; i++)
        if (Sockets[i].active && Sockets[i].fd == fd)
            return i;
    return -1;
}

int	setsockopt(int, int, int, const void *, socklen_t) {
    return 0;
}

ssize_t recvfrom(int fd, void *b, size_t bLen, int flags,
    struct sockaddr *src_addr, socklen_t *addrlen) {
    // Find one matching the FD
    int ix = _findSocket(fd);
    if (ix == -1)
        return -1;
    if (Sockets[ix].rxLen > 0) {
        // Always working from the oldest
        int len = Sockets[ix].rxBufs[0]->len;
        if (bLen < len)
            len = bLen;
        memcpy(b, Sockets[ix].rxBufs[0]->payload, len);
        // #### TODO: LENGTH CHECK
        memcpy(src_addr, &Sockets[ix].rxAddrs[0], sizeof(struct sockaddr_in));
        // Free what we just returned
        pbuf_free(Sockets[ix].rxBufs[0]);
        // Pop the queue
        for (unsigned i = 0; i < Sockets[ix].rxLen - 1; i++)
             Sockets[ix].rxBufs[i] = Sockets[ix].rxBufs[i + 1];
        Sockets[ix].rxLen--;
        return len;
    }
    else {
        return 0;
    }
}

ssize_t sendto(int fd, const void *b, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen) {
    // Find one matching the FD
    int ix = _findSocket(fd);
    if (ix == -1)
        return -1;

    // This is big-endian (first octet is high)
    uint32_t addrHost = ((struct sockaddr_in*)dest_addr)->sin_addr.s_addr;
    uint16_t portHost = ((struct sockaddr_in*)dest_addr)->sin_port; 

    // Convert the big-endian 32-bit address
    ip4_addr_t targetIp;
    IP4_ADDR(&targetIp, (addrHost >> 24) & 0xff, (addrHost >> 16) & 0xff, 
        (addrHost >> 8) & 0xff, (addrHost >> 0) & 0xff);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    memcpy((uint8_t*)p->payload, b, len);
    err_t err = udp_sendto(Sockets[ix].u, p, &targetIp, portHost);
    pbuf_free(p);
    if (err == ERR_OK)
        return len;
    else 
        return -1;
}

