# Neomar / Rover 1.5 (Palm OS)

*[Leer en español](../../es/navegadores/neomar.md)*

| | |
|---|---|
| Profile | `neomar` (default) |
| Browser | Neomar, internally *Rover 1.5*, bundled on some official Palm OS software CDs |
| Tested on | Palm m105, Palm OS 3.5.3 |
| Calls | `165.160.13.20` and `165.160.15.20`, UDP `49300` |
| Protocol | WTP over UDP (Rover's own framing) carrying WSP GET; replies are WBXML |
| Status | ✅ Working end to end: Palm m105 → serial → Windows XP SP3 + MochaPPP → internet → theretrocenter.com |

## What you need

- A Palm OS device with Neomar installed.
- A Windows PC (XP or later) the Palm connects to - serial or USB cradle.
- PPP server software on the PC: **MochaPPP** (tested) or **Softick PPP**.
- Internet access on the PC.
- `trc-wap-relay.exe`.

## 1. Get the PPP link working

Set up MochaPPP or Softick PPP as you normally would so the Palm gets a
TCP/IP connection through the PC (Prefs → Network on the Palm, using the
connection your PPP software documents). Neomar connects by IP address, so
the Palm's DNS setting doesn't matter for this.

## 2. Run the relay

On the PC, as Administrator:

```
trc-wap-relay.exe
```

It resolves the gateway, adds `165.160.13.20` and `165.160.15.20` to the
PC's internet connection and starts listening:

```
[14:31:10] Gateway: theretrocenter.com (167.114.153.137)
[14:31:10] Added 165.160.13.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG Network Connection (DHCP)
[14:31:10] [neomar] Listening on 165.160.13.20:49300 -> gateway 167.114.153.137:49300
[14:31:10] Added 165.160.15.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG Network Connection (DHCP)
[14:31:11] [neomar] Listening on 165.160.15.20:49300 -> gateway 167.114.153.137:49300
[14:31:11] Ready. Open the browser on the device and load a page. Ctrl+C to quit.
```

Allow it if Windows Firewall asks.

## 3. Browse

Open Neomar on the Palm and press **Start**. In the relay window:

```
[14:31:42] [neomar] New session: device 165.160.13.20:1026 -> 165.160.13.20:49300 -> gateway 167.114.153.137:49300
[14:31:42] [neomar] device 165.160.13.20:1026 -> gateway: 92 bytes
[14:31:43] [neomar] gateway -> device 165.160.13.20:1026: 812 bytes
```

(The device shows up with a local address because the PPP software sends
on its behalf from the PC.)

## Confirming your Rover build's addresses

The two addresses were observed on one Rover 1.5 build; others may differ.
If the relay never logs `New session` when you press Start:

- **Softick PPP:** enable *Tools → Log*, press Start on the Palm and look for
  a line like `UDP_IN: 10.0.0.1:xxxx -> <ip>:<port>`.
- **MochaPPP:** capture the PC's network card with Wireshark (filter `udp`)
  while pressing Start, and read the destination address and port.

Then run the relay with what you found:

```
trc-wap-relay.exe -b neomar -l <ip>[,<ip>...] [-P <port>]
```

and consider opening an issue so the profile can be updated.

## Coming from the route/ARP method

If you followed neomar-wap-proxy's original setup (static route + static
ARP entry to a gateway on your LAN), remove those entries first:

```bat
route delete 165.160.13.20
route delete 165.160.15.20
arp -d 165.160.13.20
arp -d 165.160.15.20
```

## Gateway side

Rover only accepts a WTP Result that mirrors its Invoke's first four bytes
(TID, the trailing byte, even the RID flag) with the PDU type switched to
Result, and it doesn't reassemble segmented replies reliably, so pages must
fit one datagram. The Retro Center's gateway handles both; to run your own,
see [neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy)
and its `docs/PROTOCOL.md`, then point the relay at it with `-t`.
