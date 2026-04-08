#!/bin/sh

$EXEC ../ed -s /dev/null <<EOF | (read a && test $a = a)
a
1
2
3
.
1w !sed s/1/a/
w
EOF
