#!/usr/bin/env python3
"""Stand-in for the remote WAP gateway: answers every datagram the way a
Rover gateway does (the Invoke's first 4 bytes mirrored, PDU type switched to
Result), followed by a payload large enough to exercise >1200-byte datagrams.

usage: fake_gateway.py <ip> <port>
"""
import socket
import sys

ip, port = sys.argv[1], int(sys.argv[2])
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ip, port))
while True:
    data, addr = sock.recvfrom(65536)
    header = bytes([(2 << 3) | (data[0] & 0x07)]) + data[1:4] if len(data) >= 4 else b""
    sock.sendto(header + b"REPLY:" + data[4:] + b":" + b"x" * 1200, addr)
