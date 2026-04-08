#!/bin/sh

$EXEC ../ed -s /dev/null <<EOF | (read a && read b && test $a-$b = 1-2)
0a
1
2
3
4
5
6
.
1z1
z1
w
EOF
