#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int log_quiet = 0;

static void log_vline(const char *fmt, va_list ap) {
    char stamp[16];
    time_t now = time(NULL);
    strftime(stamp, sizeof(stamp), "%H:%M:%S", localtime(&now));
    printf("[%s] ", stamp);
    vprintf(fmt, ap);
    printf("\n");
    fflush(stdout);
}

void log_line(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_vline(fmt, ap);
    va_end(ap);
}

void log_traffic(const char *fmt, ...) {
    if (log_quiet) return;
    va_list ap;
    va_start(ap, fmt);
    log_vline(fmt, ap);
    va_end(ap);
}

const char *addr_str(const struct sockaddr_in *addr, char *out, size_t size) {
    unsigned long ip = ntohl(addr->sin_addr.s_addr);
    _snprintf(out, size, "%lu.%lu.%lu.%lu:%u",
              (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
              (unsigned)ntohs(addr->sin_port));
    out[size - 1] = '\0';
    return out;
}
