#!/bin/sh

set -e

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

printf 'something important' > $tmp
$EXEC ../ed -s $tmp <<EOF 2>/dev/null | diff -w $tmp -
1p
EOF
