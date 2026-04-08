#!/bin/sh

$EXEC ../ed -s /dev/null <<EOF | grep 'file modified' > /dev/null
a
1
2
.
1Q
q
h
EOF
