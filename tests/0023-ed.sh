#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF >$tmp
Line
EOF

$EXEC timeout 5 ../ed -s /dev/null <<EOF | diff -u $tmp -
0a
   Line
.
s# *##
,p
Q
EOF
