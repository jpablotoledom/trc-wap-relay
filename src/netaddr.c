#include "netaddr.h"
#include "log.h"
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CLAIMED 32

/* NTE contexts returned by AddIPAddress(), needed to delete them again. */
static ULONG claimed[MAX_CLAIMED];
static int claimed_count = 0;

/* Netmasks tried in order. Windows XP rejects /32 with
 * ERROR_INVALID_PARAMETER (the address would be its own subnet's network and
 * broadcast address) and accepts /29, which keeps the side effect to an
 * 8-address slice of a range nobody uses anymore. Newer Windows takes /32. */
static const char *MASKS[] = { "255.255.255.255", "255.255.255.248", "255.255.255.0", NULL };

static void describe_interface(DWORD if_index, char *out, size_t size) {
    _snprintf(out, size, "interface %lu", (unsigned long)if_index);
    out[size - 1] = '\0';

    ULONG len = 0;
    if (GetAdaptersInfo(NULL, &len) != ERROR_BUFFER_OVERFLOW || len == 0) return;
    IP_ADAPTER_INFO *info = malloc(len);
    if (info && GetAdaptersInfo(info, &len) == NO_ERROR) {
        for (IP_ADAPTER_INFO *a = info; a; a = a->Next) {
            if (a->Index != if_index) continue;
            _snprintf(out, size, "%s%s", a->Description, a->DhcpEnabled ? " (DHCP)" : "");
            out[size - 1] = '\0';
            break;
        }
    }
    free(info);
}

int netaddr_claim(const char *ip, struct in_addr route_to) {
    if (claimed_count >= MAX_CLAIMED) {
        log_line("ERROR: too many addresses to add (max %d)", MAX_CLAIMED);
        return -1;
    }

    DWORD if_index = 0;
    DWORD rc = GetBestInterface(route_to.s_addr, &if_index);
    if (rc != NO_ERROR) {
        log_line("ERROR: no network connection reaches the gateway (error %lu)", (unsigned long)rc);
        return -1;
    }

    char name[160];
    describe_interface(if_index, name, sizeof(name));

    ULONG context = 0, instance = 0;
    const char **mask = MASKS;
    for (; *mask; mask++) {
        rc = AddIPAddress(inet_addr(ip), inet_addr(*mask), if_index, &context, &instance);
        if (rc != ERROR_INVALID_PARAMETER) break;
    }
    if (rc == ERROR_OBJECT_ALREADY_EXISTS || rc == ERROR_DUP_DOMAINNAME) return 0;
    if (rc != NO_ERROR) {
        if (rc == ERROR_ACCESS_DENIED)
            log_line("ERROR: cannot add %s - run trc-wap-relay as Administrator, or use -n", ip);
        else
            log_line("ERROR: cannot add %s to %s [index %lu] (error %lu)", ip, name,
                     (unsigned long)if_index, (unsigned long)rc);
        return -1;
    }

    claimed[claimed_count++] = context;
    log_line("Added %s/%s to %s", ip, *mask, name);
    return 0;
}

void netaddr_release_all(void) {
    while (claimed_count > 0) DeleteIPAddress(claimed[--claimed_count]);
}

int netaddr_claimed_count(void) {
    return claimed_count;
}
