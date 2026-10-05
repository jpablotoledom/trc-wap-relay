# Changelog

## 0.1.0 - 2026-10-04

First release, extracted from
[neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy)'s
`tools/xp-relay` and restructured to support more browsers.

- Browser profiles (`-b`, `-L`); first profile: **Neomar / Rover 1.5** (Palm OS).
  `-l`, `-P` and `-p` override a profile's addresses, local port and gateway port.
- Claims the browser's hardwired gateway addresses on the connection that
  reaches the gateway, with the smallest netmask Windows accepts (/32, or
  /29 on XP), and removes them on Ctrl+C or window close. `-n` uses
  addresses configured by hand instead.
- UDP relay with one upstream session per device peer; replies leave from
  the address the device called.
- Runs on Windows XP and later (PE 5.01, static, XP-only APIs).
- Wine-based tests (`make test`).

Tested end to end: Palm m105 + Neomar → serial → Windows XP SP3 + MochaPPP
(addresses added as /29 on a DHCP Wi-Fi connection) → internet →
theretrocenter.com.
