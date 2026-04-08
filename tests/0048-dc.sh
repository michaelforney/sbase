#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<'EOF' > $tmp
1
EOF

$EXEC ../dc -i <<'EOF' | diff -u - $tmp
[Splp 1+dsps. 0 Lps. 1Q]s<1>

1dsps.
lpl<1>xs.
lpps.
EOF
