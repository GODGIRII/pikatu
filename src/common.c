#include "common.h"

int send_all(int fd, const char *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = send(fd, buf + total, len - total, 0);
        if (n < 0) return -1;
        total += n;
    }
    return total;
}
