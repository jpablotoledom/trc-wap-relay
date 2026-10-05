#!/usr/bin/env python3
"""Stand-in for the PPP-connected device: sends Rover-style WTP Invokes to
the browser's gateway address and checks each reply comes back from that
same address, intact. Two independent peers run back to back.

usage: fake_device.py <source-ip> <gateway-ip> <port>
Exit status 0 = all checks passed.
"""
import socket
import sys

src, gw, port = sys.argv[1], sys.argv[2], int(sys.argv[3])
failures = 0

def check(name, ok, detail=""):
    global failures
    print(("PASS " if ok else "FAIL ") + name + (" - " + detail if detail else ""))
    failures += not ok

for peer in range(2):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((src, 0))
    sock.settimeout(5)
    for tid in range(3):
        invoke = bytes([0x08, 0x00, tid, 0x12]) + b"GET-%d-%d" % (peer, tid)
        sock.sendto(invoke, (gw, port))
        try:
            data, addr = sock.recvfrom(65536)
        except socket.timeout:
            check("peer %d request %d answered" % (peer, tid), False, "timeout")
            continue
        check("peer %d request %d from %s:%d" % (peer, tid, gw, port), addr == (gw, port), "got %s:%d" % addr)
        check("peer %d request %d mirrored header" % (peer, tid), data[:4] == bytes([0x10, 0x00, tid, 0x12]), data[:4].hex())
        check("peer %d request %d payload" % (peer, tid), data[4:].startswith(b"REPLY:GET-%d-%d:" % (peer, tid)) and len(data) > 1200,
              "%d bytes" % len(data))
    sock.close()

sys.exit(1 if failures else 0)
