#!/bin/sh

set -e

tmp1=tmp1.$$

trap 'rm -f $tmp1' EXIT
trap 'exit $?' HUP INT TERM

$EXEC ../ed <<EOF >$tmp1
i
foo
bar
.
,t
1t
2t
2,3t
3,7p
EOF

diff -u - $tmp1 <<EOF
foo
bar
foo
bar
bar
EOF
