#include "relay.h"
#include "log.h"
#include "netaddr.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_LISTENERS  16
/* listeners + sessions must stay under Winsock's FD_SETSIZE (64) */
#define MAX_SESSIONS   44
#define SESSION_IDLE_S 120
#define BUF_SIZE       65536

typedef struct {
    SOCKET fd;
    struct sockaddr_in local;      /* the browser gateway address we answer on */
    struct sockaddr_in upstream;   /* remote gateway host + this profile's port */
    const BrowserProfile *profile;
} Listener;

typedef struct {
    int in_use;
    int listener;
    struct sockaddr_in peer;       /* device side, as seen through the PPP software */
    SOCKET upstream;
    time_t last_seen;
} Session;

static RelayConfig config;
static Listener listeners[MAX_LISTENERS];
static int listener_count = 0;
static Session sessions[MAX_SESSIONS];

void relay_init(const RelayConfig *cfg) {
    config = *cfg;
}

int relay_listener_count(void) {
    return listener_count;
}

static int try_bind(SOCKET fd, const struct sockaddr_in *addr) {
    return bind(fd, (const struct sockaddr *)addr, sizeof(*addr)) == 0;
}

int relay_listen(const BrowserProfile *profile, const char *ip,
                 unsigned short port, unsigned short gateway_port) {
    if (listener_count >= MAX_LISTENERS) {
        log_line("ERROR: too many listening addresses (max %d)", MAX_LISTENERS);
        return -1;
    }

    Listener *l = &listeners[listener_count];
    memset(l, 0, sizeof(*l));
    l->profile = profile;
    l->local.sin_family = AF_INET;
    l->local.sin_port = htons(port);
    l->local.sin_addr.s_addr = inet_addr(ip);
    if (l->local.sin_addr.s_addr == INADDR_NONE) {
        log_line("ERROR: '%s' is not an IPv4 address", ip);
        return -1;
    }
    l->upstream = config.gateway;
    l->upstream.sin_port = htons(gateway_port);

    l->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (l->fd == INVALID_SOCKET) {
        log_line("ERROR: cannot create a socket (Winsock error %d)", WSAGetLastError());
        return -1;
    }

    int bound = try_bind(l->fd, &l->local);
    if (!bound && WSAGetLastError() == WSAEADDRNOTAVAIL && config.claim_addresses) {
        if (netaddr_claim(ip, config.gateway.sin_addr) != 0) {   /* reason already logged */
            closesocket(l->fd);
            return -1;
        }
        /* a freshly added address can take a moment to become usable */
        for (int tries = 0; !bound && tries < 30; tries++) {
            Sleep(100);
            bound = try_bind(l->fd, &l->local);
        }
    }
    if (!bound) {
        int err = WSAGetLastError();
        if (err == WSAEADDRNOTAVAIL)
            log_line("ERROR: %s is not an address of this PC%s", ip,
                     config.claim_addresses ? "" : " - configure it by hand, or drop -n");
        else if (err == WSAEADDRINUSE)
            log_line("ERROR: %s:%u is already in use by another program", ip, port);
        else
            log_line("ERROR: cannot listen on %s:%u (Winsock error %d)", ip, port, err);
        closesocket(l->fd);
        return -1;
    }

    char a[32], b[32];
    log_line("[%s] Listening on %s -> gateway %s", profile->id,
             addr_str(&l->local, a, sizeof(a)), addr_str(&l->upstream, b, sizeof(b)));
    listener_count++;
    return 0;
}

static Session *find_or_open_session(int li, const struct sockaddr_in *peer) {
    time_t now = time(NULL);
    Session *free_slot = NULL, *oldest = NULL;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session *s = &sessions[i];
        if (!s->in_use) { if (!free_slot) free_slot = s; continue; }
        if (s->listener == li && s->peer.sin_addr.s_addr == peer->sin_addr.s_addr &&
            s->peer.sin_port == peer->sin_port) {
            s->last_seen = now;
            return s;
        }
        if (!oldest || s->last_seen < oldest->last_seen) oldest = s;
    }

    Session *s = free_slot;
    if (!s) {   /* table full: recycle the least recently used session */
        s = oldest;
        closesocket(s->upstream);
        s->in_use = 0;
    }

    const Listener *l = &listeners[li];
    SOCKET up = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (up == INVALID_SOCKET) return NULL;
    if (connect(up, (const struct sockaddr *)&l->upstream, sizeof(l->upstream)) != 0) {
        log_line("ERROR: cannot reach the gateway (Winsock error %d)", WSAGetLastError());
        closesocket(up);
        return NULL;
    }
    s->in_use = 1;
    s->listener = li;
    s->peer = *peer;
    s->upstream = up;
    s->last_seen = now;

    char a[32], b[32], c[32];
    log_line("[%s] New session: device %s -> %s -> gateway %s", l->profile->id,
             addr_str(peer, a, sizeof(a)), addr_str(&l->local, b, sizeof(b)),
             addr_str(&l->upstream, c, sizeof(c)));
    return s;
}

static void expire_sessions(void) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].in_use && now - sessions[i].last_seen > SESSION_IDLE_S) {
            closesocket(sessions[i].upstream);
            sessions[i].in_use = 0;
        }
    }
}

/* device -> gateway */
static void forward_from_device(int li, char *buf) {
    struct sockaddr_in peer;
    int plen = sizeof(peer);
    int n = recvfrom(listeners[li].fd, buf, BUF_SIZE, 0, (struct sockaddr *)&peer, &plen);
    if (n <= 0) return;   /* e.g. WSAECONNRESET left over from an ICMP error */

    Session *s = find_or_open_session(li, &peer);
    if (!s) return;

    char a[32];
    if (send(s->upstream, buf, n, 0) == SOCKET_ERROR)
        log_line("ERROR: send to the gateway failed (Winsock error %d)", WSAGetLastError());
    else
        log_traffic("[%s] device %s -> gateway: %d bytes", listeners[li].profile->id,
                    addr_str(&peer, a, sizeof(a)), n);
}

/* gateway -> device, sent from the address the device called */
static void forward_to_device(Session *s, char *buf) {
    const Listener *l = &listeners[s->listener];
    int n = recv(s->upstream, buf, BUF_SIZE, 0);
    if (n == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAECONNRESET)
            log_line("ERROR: [%s] the gateway refused the datagram - is UDP %u open on the server?",
                     l->profile->id, (unsigned)ntohs(l->upstream.sin_port));
        return;
    }
    s->last_seen = time(NULL);
    sendto(l->fd, buf, n, 0, (const struct sockaddr *)&s->peer, sizeof(s->peer));

    char a[32];
    log_traffic("[%s] gateway -> device %s: %d bytes", l->profile->id,
                addr_str(&s->peer, a, sizeof(a)), n);
}

void relay_run(void) {
    static char buf[BUF_SIZE];
    for (;;) {
        fd_set rd;
        FD_ZERO(&rd);
        for (int i = 0; i < listener_count; i++) FD_SET(listeners[i].fd, &rd);
        for (int i = 0; i < MAX_SESSIONS; i++)
            if (sessions[i].in_use) FD_SET(sessions[i].upstream, &rd);

        struct timeval tv = { 5, 0 };
        int ready = select(0, &rd, NULL, NULL, &tv);
        if (ready == SOCKET_ERROR) {
            log_line("ERROR: select failed (Winsock error %d)", WSAGetLastError());
            Sleep(1000);
            continue;
        }
        expire_sessions();
        if (ready == 0) continue;

        for (int li = 0; li < listener_count; li++)
            if (FD_ISSET(listeners[li].fd, &rd)) forward_from_device(li, buf);

        for (int i = 0; i < MAX_SESSIONS; i++)
            if (sessions[i].in_use && FD_ISSET(sessions[i].upstream, &rd))
                forward_to_device(&sessions[i], buf);
    }
}
