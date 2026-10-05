/*
 * The UDP relay engine, independent of any particular browser.
 *
 * A listener is one (browser gateway address, port) this PC answers on.
 * Every distinct device-side peer (ip:port, per listener) gets a session: its
 * own upstream socket connected to the remote gateway, so replies find their
 * way back to the right device and concurrent transactions never mix.
 * Replies are sent back from the listener's socket - i.e. from the address
 * the device called - because WAP clients drop datagrams from anywhere else.
 */
#ifndef TRC_RELAY_H
#define TRC_RELAY_H

#include "browsers.h"
#include <winsock2.h>

typedef struct {
    struct sockaddr_in gateway;  /* remote gateway host; the port comes from each profile */
    int claim_addresses;         /* add missing addresses with netaddr_claim() */
} RelayConfig;

void relay_init(const RelayConfig *config);

/* Starts answering on ip:port for `profile`, forwarding to the gateway on
 * `gateway_port`. Returns 0, or -1 (reason already logged). */
int relay_listen(const BrowserProfile *profile, const char *ip,
                 unsigned short port, unsigned short gateway_port);

int relay_listener_count(void);

/* Runs the relay loop; never returns. */
void relay_run(void);

#endif
