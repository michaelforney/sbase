#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF >$tmp
y
1
x
y
EOF

$EXEC ../ed -s /dev/null <<EOF  | diff -u $tmp -
a
1
2
3
.
1r !printf 'x\ny\n'
p
1,3p
w
EOF
