#pragma once

#define POLLIN 0x001

struct pollfd {
    int   fd;
    short events;
    short revents;
};
