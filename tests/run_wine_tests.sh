#!/bin/bash
# Runs dist/trc-wap-relay.exe under Wine against a fake gateway and a fake
# device on loopback addresses (Linux routes all of 127.0.0.0/8 to lo):
#   127.0.0.1  fake remote gateway
#   127.0.0.2  plays the browser's hardwired gateway address (-l ... -n)
#   127.0.0.3  the device, as the PPP software would send from
# Ports are moved off the profile's 49300 (-P/-p) so a local WAP gateway that
# already listens there - e.g. a Boat Rudder dev server - doesn't interfere.
# Adding addresses (netaddr_claim) can't be exercised here - Wine doesn't
# implement AddIPAddress - so that path needs a real Windows machine.
set -u
cd "$(dirname "$0")/.."
EXE=dist/trc-wap-relay.exe
LOCAL_PORT=49391     # what the fake device calls (the relay's -P)
GATEWAY_PORT=49392   # where the fake gateway answers (the relay's -p)
export WINEDEBUG=-all
LOG=$(mktemp)
failures=0
pass() { echo "PASS $1"; }
fail() { echo "FAIL $1"; failures=$((failures + 1)); }
expect_output() {   # name, pattern, command...
    local name=$1 pattern=$2; shift 2
    if "$@" 2>&1 | tr -d '\r' | grep -qE "$pattern"; then pass "$name"; else fail "$name"; fi
}

[ -f "$EXE" ] || { echo "Build first: make"; exit 1; }
command -v wine >/dev/null || { echo "wine not found"; exit 1; }

echo "== command line"
expect_output "-V prints the version"       '^trc-wap-relay [0-9]'        wine $EXE -V
expect_output "-L lists neomar"             'neomar +Neomar'              wine $EXE -L
expect_output "unknown browser is rejected" "unknown browser 'nope'"      wine $EXE -b nope
expect_output "-n with a missing address"   'is not an address of this PC' timeout 10 wine $EXE -n -l 192.0.2.1 -P $LOCAL_PORT -t 127.0.0.1

echo "== gateway not listening"
timeout 15 wine $EXE -n -t 127.0.0.1 -l 127.0.0.2 -P $LOCAL_PORT -p $GATEWAY_PORT > "$LOG" 2>&1 &
sleep 4
python3 - "$LOCAL_PORT" <<'PY'
import socket, sys
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("127.0.0.3", 0))
s.sendto(bytes([8, 0, 1, 0x12]) + b"hello", ("127.0.0.2", int(sys.argv[1])))
PY
sleep 2
# Wine doesn't surface the ICMP "port unreachable" on a connected UDP socket,
# so this one can only be confirmed on real Windows (it is, on XP SP3).
if tr -d '\r' < "$LOG" | grep -q "refused the datagram"; then pass "refused datagram is reported"
elif tr -d '\r' < "$LOG" | grep -q "device 127.0.0.3:[0-9]* -> gateway"; then echo "SKIP refused datagram is reported (Wine doesn't report ICMP errors; forwarded fine)"
else fail "datagram forwarded while the gateway is down"; fi
wait

echo "== relaying"
python3 tests/fake_gateway.py 127.0.0.1 $GATEWAY_PORT & GW=$!
timeout 20 wine $EXE -n -t 127.0.0.1 -l 127.0.0.2 -P $LOCAL_PORT -p $GATEWAY_PORT > "$LOG" 2>&1 &
sleep 4
python3 tests/fake_device.py 127.0.0.3 127.0.0.2 $LOCAL_PORT || failures=$((failures + 1))
kill $GW 2>/dev/null
wait 2>/dev/null
sessions=$(tr -d '\r' < "$LOG" | grep -c "New session")
[ "$sessions" -eq 2 ] && pass "one session per device peer" || fail "one session per device peer (got $sessions)"

rm -f "$LOG"
echo
[ $failures -eq 0 ] && echo "All tests passed." || echo "$failures test(s) failed."
exit $failures
