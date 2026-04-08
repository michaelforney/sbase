#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<'EOF' > $tmp
-.2840790438404122960275
EOF

$EXEC ../dc <<'EOF' | diff -u - $tmp
22k
_.1755705045849462583368
.618033988749894848204/p
EOF
