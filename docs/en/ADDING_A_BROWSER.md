# Adding a browser

*[Leer en español](../es/AGREGAR_UN_NAVEGADOR.md)*

trc-wap-relay starts with Neomar, but the relay engine knows nothing about
any particular browser. Supporting another one is mostly a matter of
finding out what it calls and writing that down as a profile.

## Does the browser need this relay?

Only browsers whose gateway **can't be changed** need it. If the browser
lets you type a gateway address (many WAP 1.x browsers do, under *WAP
gateway* / *proxy* settings), point it straight at a gateway that speaks
its protocol and no relay is involved. (The Retro Center's Boat Rudder
server can answer standard connectionless WSP on UDP 9200, but that port is
currently disabled on theretrocenter.com - it's experimental and untested
on real phones.)

## 1. Find out what it calls

With the device connected through the PC and the PPP link up:

- **Softick PPP:** *Tools → Log*, then load a page on the device and read
  the destination of the `UDP_IN` lines.
- **Anything else:** capture the PC's network card with Wireshark (filter
  `udp or tcp`) while loading a page.

Write down every destination address, the port, and whether it's UDP or
TCP. Try a couple of times - some browsers alternate between a primary and
a secondary gateway.

If the browser uses **TCP**, see [TCP browsers](#tcp-browsers) below.

## 2. Make sure a gateway speaks its protocol

The relay forwards bytes; the remote gateway has to understand them.
Standard WAP 1.x browsers send connectionless WSP (UDP 9200) or WTP+WSP
(UDP 9201); others, like Neomar, have quirks of their own. Check that the
gateway you'll point at (The Retro Center's Boat Rudder server, or your
own) handles that protocol, and on which port.

Before writing any code you can already test with the existing profile and
the overrides:

```
trc-wap-relay.exe -b neomar -l <ip>,<ip> -P <port-it-calls> -p <gateway-port>
```

(`-l` and `-P` replace the addresses and port to answer on, `-p` the
gateway's port.)

## 3. Add the profile

In [`src/browsers.c`](../../src/browsers.c), add an entry and list it in
`BROWSERS[]`:

```c
static const BrowserProfile EXAMPLE = {
    .id           = "example",                    /* used with -b, lowercase */
    .name         = "Example Browser 2.0",
    .device       = "Psion Series 5mx (EPOC R5)",
    .addresses    = { "192.0.2.10", "192.0.2.11", NULL },
    .port         = 9201,                         /* what the browser calls */
    .gateway_port = 9201,                         /* where the gateway answers */
    .protocol     = "WTP + WSP over UDP",
};

const BrowserProfile *const BROWSERS[] = {
    &NEOMAR,
    &EXAMPLE,
    NULL
};
```

A comment above the entry saying where the addresses came from (which
device, which build, how they were captured) helps the next person whose
build differs.

## 4. Document and test it

- Add a guide in `docs/en/browsers/<id>.md` and `docs/es/navegadores/<id>.md`
  (use [neomar.md](browsers/neomar.md) as a template), and a row in the
  *Supported browsers* table of both READMEs.
- `make test` must still pass. If the browser has its own framing, a
  variant of `tests/fake_device.py` / `tests/fake_gateway.py` that speaks
  it is welcome.
- Note in `CHANGELOG.md` what was tested on which hardware.

Several profiles can run at once: `trc-wap-relay.exe -b neomar,example`.

## TCP browsers

Browsers that reach their gateway over TCP (for example WAP 2.0 browsers
using an HTTP proxy on a fixed address) need a TCP relay, which doesn't
exist yet. The intended shape:

- a `transport` field in `BrowserProfile` (`TRANSPORT_UDP` /
  `TRANSPORT_TCP`);
- a `relay_tcp.c` next to `relay.c`, accepting on the claimed address and
  piping each connection to the gateway - `netaddr.c` works unchanged;
- `main.c` dispatching listeners by transport, and both relays driven from
  the same `select()` loop (mind the 64-socket `FD_SETSIZE`).
