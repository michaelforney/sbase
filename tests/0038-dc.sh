#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Expected output for f, c, d, and r operators
cat <<EOF >$tmp
test 1:
test 2:
3
2
1
test 3:
0
test 4:
5
5
test 5:
3
3
2
1
test 6:
2
1
test 7:
2
3
1
test 8:
10
test 9:
1
test 10:
15
test 11:
test 12:
1
1
1
1
test 13:
-5
-5
test 14:
1.5
1.5
test 15:
2
3
1
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc f
[test 2:]pc 1 2 3 f c
[test 3:]pc 1 2 3 c zp c
[test 4:]pc 5 d f c
[test 5:]pc 1 2 3 d f c
[test 6:]pc 2 1 r f c
[test 7:]pc 1 2 3 r f c
[test 8:]pc 5 d +p c
[test 9:]pc 1 2 r -p c
[test 10:]pc 5 d d + +p c
[test 11:]pc 1 2 3 c f
[test 12:]pc 1 d d d f c
[test 13:]pc _5 d f c
[test 14:]pc 1.5 d f c
[test 15:]pc 1 2 3 r f c
EOF
