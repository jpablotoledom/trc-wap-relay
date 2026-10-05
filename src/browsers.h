/*
 * Browser profiles: what each supported WAP browser is hardwired to call.
 *
 * Vintage WAP browsers don't ask DNS for their gateway - the operator's IP
 * address and port are baked into the browser or the device. A profile
 * records those, plus the port the same traffic must reach on the remote
 * gateway. Adding support for another browser means adding one entry to the
 * table in browsers.c (see docs/en/ADDING_A_BROWSER.md).
 */
#ifndef TRC_BROWSERS_H
#define TRC_BROWSERS_H

#define BROWSER_MAX_ADDRESSES 4

typedef struct {
    const char *id;              /* short name for -b, e.g. "neomar" */
    const char *name;            /* human-readable name */
    const char *device;          /* platform it runs on */
    const char *addresses[BROWSER_MAX_ADDRESSES + 1];  /* gateway IPs it calls, NULL-terminated */
    unsigned short port;         /* UDP port it calls on those addresses */
    unsigned short gateway_port; /* UDP port of the same service on the remote gateway */
    const char *protocol;        /* what travels over the wire, for documentation/logs */
} BrowserProfile;

/* NULL-terminated list of every known profile. */
extern const BrowserProfile *const BROWSERS[];

/* Profile by id (case-insensitive), or NULL. */
const BrowserProfile *browser_find(const char *id);

#endif
