#!/bin/sh

$EXEC ../ed -s /dev/null <<EOF | (read a && test $a = 1)
a
1
2
3
4
5
6
7
8
9
.
1s/^//
ka
9s/^//
'a
w
EOF
