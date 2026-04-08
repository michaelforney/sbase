#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

touch $tmp
$EXEC ../ed -s $tmp <<EOF | (read a && test $a = 1)
a
1
.
w
,c
2
.
W
e
1p
EOF
