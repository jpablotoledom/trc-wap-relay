/*
 * Console logging: one timestamped line per call, flushed immediately so the
 * output shows up live in a cmd.exe window or when redirected to a file.
 */
#ifndef TRC_LOG_H
#define TRC_LOG_H

#include <winsock2.h>
#include <stddef.h>

/* When set, log_traffic() lines (one per datagram) are suppressed. */
extern int log_quiet;

void log_line(const char *fmt, ...);
void log_traffic(const char *fmt, ...);

/* "a.b.c.d:port" into `out`. Safe to call several times in one log_line()
 * argument list (unlike inet_ntoa(), which reuses one static buffer). */
const char *addr_str(const struct sockaddr_in *addr, char *out, size_t size);

#endif
