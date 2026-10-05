# TRC WAP Relay

*[Leer en español](LEAME.md)*

Bring vintage WAP browsers back online from the Windows PC they connect
through - no extra hardware, no router tricks.

Old WAP browsers don't look their gateway up: the operator's IP address is
baked into the browser, and those gateways went dark twenty years ago.
**trc-wap-relay** is a single small `.exe` (≈50 KB, no dependencies, runs on
**Windows XP** and later) that makes the PC answer on those dead addresses
and relays the traffic to a modern WAP gateway on the internet -
[The Retro Center](https://theretrocenter.com)'s by default.

```
Palm m105 ── serial ── Windows XP ──────────── internet ──────────── theretrocenter.com
 Neomar                 MochaPPP                                      WAP gateway
                        trc-wap-relay  (answers on 165.160.13.20)     UDP 49300
```

## Supported browsers

| Profile | Browser | Device | Status |
|---|---|---|---|
| `neomar` | Neomar / Rover 1.5 | Palm OS (Palm m105, Palm OS 3.5.3) | ✅ Tested end to end on real hardware |

The project is built to grow: each browser is a small *profile* (the
addresses and port it calls), and the relay engine is shared. See
[Adding a browser](docs/en/ADDING_A_BROWSER.md).

## Quick start (Neomar on a Palm)

1. Connect the Palm to the Windows PC and get a working PPP link (MochaPPP,
   Softick PPP, …) - see [the Neomar guide](docs/en/browsers/neomar.md).
2. Copy `trc-wap-relay.exe` to the PC and run it **as Administrator**:
   ```
   trc-wap-relay.exe
   ```
   You should see:
   ```
   [14:31:10] Gateway: theretrocenter.com (167.114.153.137)
   [14:31:10] Added 165.160.13.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG ... (DHCP)
   [14:31:10] [neomar] Listening on 165.160.13.20:49300 -> gateway 167.114.153.137:49300
   ...
   [14:31:11] Ready. Open the browser on the device and load a page. Ctrl+C to quit.
   ```
3. Open Neomar on the Palm and press **Start**.
4. Quit with **Ctrl+C**: the addresses the relay added are removed again.

## Usage

```
trc-wap-relay.exe [-b browser[,browser...]] [-t host] [-p port] [-l ip,ip,...] [-P port] [-n] [-q]
trc-wap-relay.exe -L | -h | -V

  -b  browser profile(s) to serve       (default neomar; -L lists them)
  -t  remote WAP gateway host            (default theretrocenter.com)
  -p  gateway port, overriding the profile's (single profile only)
  -l  addresses to answer on, overriding the profile's (single profile only)
  -P  port to answer on, overriding the profile's (single profile only)
  -n  don't add missing addresses to this PC; they must already exist
  -q  quiet: don't log every datagram
  -L  list the supported browsers
  -V  print the version
```

Examples:

```
trc-wap-relay.exe -t my-gateway.example.com      use your own gateway
trc-wap-relay.exe -q                             only log sessions and errors
trc-wap-relay.exe -n                             addresses configured by hand (no admin rights)
```

The remote gateway must speak the browser's protocol and be reachable on
the profile's UDP port. The Retro Center runs one; for your own, see
[neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy)
(Python) or The Retro Center's Boat Rudder server.

## How it works

Windows XP can't rewrite a packet's destination, so instead the PC
**owns** the browser's dead gateway addresses for as long as the relay
runs. The PPP software hands the device's traffic to Windows, Windows
delivers anything addressed to a local address to whoever listens there -
the relay - and the relay forwards it to the real gateway, sending the
replies back **from the address the device called** (WAP clients ignore
anything else).

Details, diagrams and design decisions: [Architecture](docs/en/ARCHITECTURE.md).

## Building

From Linux, with the MinGW-w64 cross compiler:

```bash
sudo apt install gcc-mingw-w64-i686 make
./build.sh                 # -> dist/trc-wap-relay.exe
```

On Windows with MinGW: `make CC=gcc`.

The build pins the PE header to version 5.01 so Windows XP accepts it, links
statically, and the code only uses APIs present on XP (Winsock 2, IP Helper).

## Testing

```bash
sudo apt install wine python3
make test
```

Runs the `.exe` under Wine against a fake gateway and a fake device on
loopback addresses: command line, replies coming from the called address,
mirrored headers, >1200-byte datagrams and one session per device peer.
Two paths need real Windows and are verified there: adding addresses
(`AddIPAddress` isn't implemented in Wine) and reporting a gateway that
refuses the datagram (Wine doesn't surface ICMP errors).

## Project layout

```
trc-wap-relay/
├── src/
│   ├── main.c        command line, startup and shutdown
│   ├── browsers.c/h  browser profiles (addresses and ports each one calls)
│   ├── relay.c/h     UDP relay engine: listeners, sessions, forwarding
│   ├── netaddr.c/h   adding/removing addresses on Windows (IP Helper API)
│   └── log.c/h       timestamped console log
├── tests/            Wine-based tests with a fake gateway and device
├── docs/
│   ├── en/           architecture, adding a browser, troubleshooting, browser guides
│   ├── es/           the same, in Spanish
│   └── diagrams/     PlantUML sources
├── Makefile, build.sh
└── README.md / LEAME.md, CHANGELOG.md, LICENSE
```

## Documentation

- [Architecture](docs/en/ARCHITECTURE.md)
- [Neomar / Rover 1.5 guide](docs/en/browsers/neomar.md)
- [Troubleshooting](docs/en/TROUBLESHOOTING.md)
- [Adding a browser](docs/en/ADDING_A_BROWSER.md)

## Credits

The Neomar/Rover protocol was reverse-engineered in
[neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy), where
this relay started as `tools/xp-relay`.

## Contact

**Jonathan Pablo Toledo Moya**
theretrocenter.com@gmail.com

## License

MIT - see [LICENSE](LICENSE).
