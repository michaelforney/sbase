#!/bin/sh

set -e

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

echo 0 > $tmp

$EXEC ../dc -i <<'EOF' | diff -u - $tmp
[ 0 Lxs. 2Q]s<128>
[ .7853981633974483096156608458198757210492923498437764 1/ Lxs. 3Q]s<130>
[K 52><130> ]s<129>
[Sxlx 0=<128> lx 1=<129> 0 Lxs. 1Q]s<1>

 1l<1>xps.
EOF
