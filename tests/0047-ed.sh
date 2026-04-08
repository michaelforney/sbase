#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF > $tmp
LLLx
yLyLyLyxy
zzzzxy
EOF

$EXEC ../ed -s /dev/null <<EOF | diff -u $tmp -
i
LLL
.
s! *!x!4
p
s# *#y#g
p
s/\(^\|L\)y/z/g
p
EOF
