#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF >$tmp
1
2
3
EOF

$EXEC ../ed -s /dev/null <<EOF | diff -u $tmp -
a
1
2
3
.
g/^$/d
,p
Q
EOF
