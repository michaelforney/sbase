#!/bin/sh
tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

set -e

$EXEC ../ed -s $tmp <<EOF >/dev/null
a
1
2
.
w
5d
X
EOF

printf '1\n2\n1\n2\n' | diff -u $tmp -
