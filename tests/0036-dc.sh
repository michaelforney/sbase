#!/bin/sh

set -e

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test i, o, k, I, O, K commands
cat <<'EOF' >$tmp
test 1:
10
test 2:
10
test 3:
0
test 4:
16
test 5:
16
10
test 6:
5
test 7:
A
test 8:
FF
test 9:
10
test 10:
1010
test 11:
10
test 12:
.33333
test 13:
 12 15
test 14:
 01 04 19 19
test 15:
 01.10
test 16:
.05 00
EOF

$EXEC ../dc <<'EOF' | diff -u $tmp -
[test 1:]pc Ip
[test 2:]pc Op
[test 3:]pc Kp
[test 4:]pc 16i Ip
[test 5:]pc Ao Ip Op
[test 6:]pc Ai 5k Kp
[test 7:]pc 16o 10p
[test 8:]pc 255p
[test 9:]pc 10o 16i Ap
[test 10:]pc Ai 2o 10p
[test 11:]pc Ao 2i 1010p
[test 12:]pc Ai 5k 1 3/p
[test 13:]pc 20o 255p
[test 14:]pc 9999p
[test 15:]pc 1.5p
[test 16:]pc .25p
EOF
