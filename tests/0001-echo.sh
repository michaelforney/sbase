#!/bin/sh

set -e

tmp=tmp1.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

$EXEC ../echo -n --hello-- --world--! > $tmp

tr -d '\n' <<'EOF' | diff -u - $tmp
--hello-- --world--!
EOF
