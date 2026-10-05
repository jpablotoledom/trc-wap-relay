# Troubleshooting

*[Leer en español](../es/SOLUCION_DE_PROBLEMAS.md)*

The relay's log usually says where the chain breaks. Read it top to bottom.

## At startup

| Log line | Meaning | What to do |
|---|---|---|
| `cannot resolve theretrocenter.com` | The PC has no working internet/DNS. | Check the PC's connection; or use `-t <ip-address>`. |
| `cannot add 165.160.13.20 - run trc-wap-relay as Administrator` | Adding addresses was denied. | Right-click → *Run as…* an administrator account, or configure the addresses by hand and use `-n` (below). |
| `cannot add ... (error N)` | Windows refused the address on that connection. The line names the adapter and whether it uses DHCP. | Try `-n` with the addresses on the Microsoft Loopback Adapter (below), and report the error number. |
| `is not an address of this PC - configure it by hand, or drop -n` | With `-n`, the address isn't on any adapter. | Configure it (below), or run without `-n`. |
| `... is already in use by another program` | Something else listens on that address and port - often a second copy of the relay. | Close the other program. |
| `Nothing to listen on - exiting.` | Every address failed; the reasons are in the lines above. | |

## When you load a page

| What you see | Meaning | What to do |
|---|---|---|
| No `New session` at all | The device's traffic doesn't reach the relay. | Check the PPP link works; confirm the browser really calls the profile's addresses and port (see its guide - e.g. [Neomar](browsers/neomar.md#confirming-your-rover-builds-addresses)). |
| `device -> gateway`, never `gateway -> device` | The gateway doesn't answer. | Firewall on the gateway server, the gateway service not running, or the wrong port (`-p`). |
| `the gateway refused the datagram - is UDP N open on the server?` | An ICMP "port unreachable" came back: the server is reachable but nothing listens on that UDP port. | Start/enable the gateway on the server, or fix the port. |
| `gateway -> device` lines but the device shows an error | The gateway answered something the browser doesn't accept. | A gateway-side problem: check its logs and the browser's protocol notes. |

## Configuring the addresses by hand (`-n`)

Useful without administrator rights, or if Windows refuses to add the
addresses automatically.

1. Add the **Microsoft Loopback Adapter**: Control Panel → Add Hardware →
   "Yes, I have already connected the hardware" → "Add a new hardware
   device" → "Install the hardware that I manually select from a list" →
   Network adapters → Microsoft → **Microsoft Loopback Adapter**.
2. Find its connection name with `ipconfig /all`, then:
   ```bat
   netsh interface ip set address name="Local Area Connection 2" static 165.160.13.20 255.255.255.0
   netsh interface ip add address name="Local Area Connection 2" 165.160.15.20 255.255.255.0
   ```
3. Run `trc-wap-relay.exe -n`.

These addresses persist until you remove them from the adapter.

## Leftover addresses

If the relay was killed instead of closed with Ctrl+C, the addresses it
added stay until the next reboot or until that connection resets. They are
harmless, and the next run reuses them. To remove them right away, disable
and re-enable the connection.
