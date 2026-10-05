/*
 * trc-wap-relay - lets vintage WAP browsers reach a modern WAP gateway (The
 * Retro Center's, by default) from the Windows PC they connect through, with
 * nothing else on the network.
 *
 * Those browsers are hardwired to their operator's long-dead gateway
 * address. This program claims that address on the PC (see netaddr.h) and
 * relays the traffic to the remote gateway (see relay.h). Which addresses
 * and ports each browser uses lives in browsers.c.
 *
 * Runs on Windows XP and later. See README.md for usage.
 */
#include "browsers.h"
#include "log.h"
#include "netaddr.h"
#include "relay.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION        "0.1.0"
#define DEFAULT_TARGET "theretrocenter.com"
#define DEFAULT_BROWSERS "neomar"

static void usage(const char *prog) {
    printf("usage: %s [-b browser[,browser...]] [-t host] [-p port] [-l ip,ip,...] [-P port] [-n] [-q]\n"
           "       %s -L | -h | -V\n\n"
           "  -b  browser profile(s) to serve       (default %s; -L lists them)\n"
           "  -t  remote WAP gateway host            (default %s)\n"
           "  -p  gateway port, overriding the profile's (single profile only)\n"
           "  -l  addresses to answer on, overriding the profile's (single profile only)\n"
           "  -P  port to answer on, overriding the profile's (single profile only)\n"
           "  -n  don't add missing addresses to this PC; they must already exist\n"
           "  -q  quiet: don't log every datagram\n"
           "  -L  list the supported browsers\n"
           "  -V  print the version\n",
           prog, prog, DEFAULT_BROWSERS, DEFAULT_TARGET);
}

static void list_browsers(void) {
    printf("Supported browsers:\n\n");
    for (int i = 0; BROWSERS[i]; i++) {
        const BrowserProfile *b = BROWSERS[i];
        printf("  %-10s %s\n", b->id, b->name);
        printf("  %-10s device:   %s\n", "", b->device);
        printf("  %-10s calls:    ", "");
        for (int a = 0; b->addresses[a]; a++) printf("%s%s", a ? ", " : "", b->addresses[a]);
        printf(" (UDP %u)\n", b->port);
        printf("  %-10s gateway:  UDP %u - %s\n\n", "", b->gateway_port, b->protocol);
    }
}

static int resolve(const char *host, struct sockaddr_in *out) {
    memset(out, 0, sizeof(*out));
    out->sin_family = AF_INET;
    out->sin_addr.s_addr = inet_addr(host);
    if (out->sin_addr.s_addr != INADDR_NONE) return 0;

    struct hostent *he = gethostbyname(host);
    if (!he || he->h_addrtype != AF_INET || !he->h_addr_list[0]) return -1;
    memcpy(&out->sin_addr, he->h_addr_list[0], sizeof(out->sin_addr));
    return 0;
}

static BOOL WINAPI on_console_close(DWORD event) {
    (void)event;
    if (netaddr_claimed_count() > 0) {
        netaddr_release_all();
        printf("\nRemoved the gateway addresses from this PC.\n");
        fflush(stdout);
    }
    ExitProcess(0);
    return TRUE;
}

int main(int argc, char **argv) {
    const char *target_host = DEFAULT_TARGET;
    char browser_list[128] = DEFAULT_BROWSERS;
    char listen_override[256] = "";
    int port_override = 0;
    int local_port_override = 0;
    int claim = 1;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        int has_value = i + 1 < argc;
        if (!strcmp(arg, "-b") && has_value) {
            strncpy(browser_list, argv[++i], sizeof(browser_list) - 1);
        } else if (!strcmp(arg, "-t") && has_value) {
            target_host = argv[++i];
        } else if (!strcmp(arg, "-p") && has_value) {
            port_override = atoi(argv[++i]);
            if (port_override <= 0 || port_override > 65535) {
                printf("ERROR: invalid port '%s'\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(arg, "-P") && has_value) {
            local_port_override = atoi(argv[++i]);
            if (local_port_override <= 0 || local_port_override > 65535) {
                printf("ERROR: invalid port '%s'\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(arg, "-l") && has_value) {
            strncpy(listen_override, argv[++i], sizeof(listen_override) - 1);
        } else if (!strcmp(arg, "-n")) {
            claim = 0;
        } else if (!strcmp(arg, "-q")) {
            log_quiet = 1;
        } else if (!strcmp(arg, "-L")) {
            list_browsers();
            return 0;
        } else if (!strcmp(arg, "-V")) {
            printf("trc-wap-relay %s\n", VERSION);
            return 0;
        } else {
            usage(argv[0]);
            return !strcmp(arg, "-h") ? 0 : 2;
        }
    }

    /* Resolve the requested profiles up front, so a typo fails fast. */
    const BrowserProfile *selected[16];
    int selected_count = 0;
    for (char *id = strtok(browser_list, ","); id; id = strtok(NULL, ",")) {
        const BrowserProfile *b = browser_find(id);
        if (!b) {
            printf("ERROR: unknown browser '%s' - run with -L to list them\n", id);
            return 2;
        }
        if (selected_count < (int)(sizeof(selected) / sizeof(selected[0]))) selected[selected_count++] = b;
    }
    if ((port_override || local_port_override || listen_override[0]) && selected_count != 1) {
        printf("ERROR: -p, -P and -l apply to a single browser profile\n");
        return 2;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("ERROR: WSAStartup failed\n");
        return 1;
    }

    printf("trc-wap-relay %s - WAP gateway relay for vintage browsers\n\n", VERSION);

    RelayConfig config = { .claim_addresses = claim };
    if (resolve(target_host, &config.gateway) != 0) {
        log_line("ERROR: cannot resolve %s - check this PC's internet connection/DNS", target_host);
        return 1;
    }
    log_line("Gateway: %s (%s)", target_host, inet_ntoa(config.gateway.sin_addr));
    relay_init(&config);

    SetConsoleCtrlHandler(on_console_close, TRUE);
    atexit(netaddr_release_all);

    for (int s = 0; s < selected_count; s++) {
        const BrowserProfile *b = selected[s];
        unsigned short gateway_port = port_override ? (unsigned short)port_override : b->gateway_port;
        unsigned short local_port = local_port_override ? (unsigned short)local_port_override : b->port;
        if (listen_override[0]) {
            for (char *ip = strtok(listen_override, ","); ip; ip = strtok(NULL, ","))
                relay_listen(b, ip, local_port, gateway_port);
        } else {
            for (int a = 0; b->addresses[a]; a++)
                relay_listen(b, b->addresses[a], local_port, gateway_port);
        }
    }

    if (relay_listener_count() == 0) {
        log_line("Nothing to listen on - exiting.");
        return 1;
    }
    log_line("Ready. Open the browser on the device and load a page. Ctrl+C to quit.");
    relay_run();
    return 0;
}
