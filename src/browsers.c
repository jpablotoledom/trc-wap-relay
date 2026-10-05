#include "browsers.h"
#include <string.h>

/*
 * Neomar (internally "Rover 1.5"), the WAP micro-browser bundled on some
 * official Palm OS software CDs. Rover sends WTP Invoke PDUs over UDP to its
 * operator gateway and only accepts a Result that mirrors the Invoke's first
 * four bytes. Addresses observed on a Palm m105 (Palm OS 3.5.3); other Rover
 * builds may differ - see docs/en/browsers/neomar.md to confirm yours.
 * Protocol reverse-engineered in https://github.com/jpablotoledom/neomar-wap-proxy
 */
static const BrowserProfile NEOMAR = {
    .id           = "neomar",
    .name         = "Neomar / Rover 1.5",
    .device       = "Palm OS (tested: Palm m105, Palm OS 3.5.3)",
    .addresses    = { "165.160.13.20", "165.160.15.20", NULL },
    .port         = 49300,
    .gateway_port = 49300,
    .protocol     = "WTP over UDP (Rover framing) + WSP GET",
};

const BrowserProfile *const BROWSERS[] = {
    &NEOMAR,
    NULL
};

const BrowserProfile *browser_find(const char *id) {
    for (int i = 0; BROWSERS[i]; i++)
        if (_stricmp(BROWSERS[i]->id, id) == 0) return BROWSERS[i];
    return NULL;
}
