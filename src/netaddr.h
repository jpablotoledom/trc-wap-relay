/*
 * Claiming a browser's hardwired gateway addresses on this PC.
 *
 * Windows XP can't rewrite a packet's destination (no DNAT), so instead the
 * PC *owns* the browser's gateway addresses: the PPP software (MochaPPP,
 * Softick PPP, ...) forwards the device's datagrams through Winsock, and
 * Windows delivers anything sent to a local address to whoever listens there
 * - this program. Addresses are added at runtime with the IP Helper API (no
 * reboot, nothing persistent) and removed again on exit.
 */
#ifndef TRC_NETADDR_H
#define TRC_NETADDR_H

#include <winsock2.h>

/* Adds `ip` to the network connection Windows uses to reach `route_to` (in
 * practice, the default connection). Succeeds without doing anything if the
 * address already exists. Logs the reason and returns -1 on failure. */
int netaddr_claim(const char *ip, struct in_addr route_to);

/* Removes every address netaddr_claim() added. Safe to call more than once. */
void netaddr_release_all(void);

/* How many addresses netaddr_claim() added and still holds. */
int netaddr_claimed_count(void);

#endif
