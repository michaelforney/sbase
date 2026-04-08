#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<'EOF' > $tmp
0
0
EOF

$EXEC ../bc -p ../dc <<'EOF' | diff -u - $tmp
define a() {
0
}

a()
EOF
