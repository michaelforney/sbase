#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Expected output for z, Z, and X operators
cat <<EOF >$tmp
test 1:
0
test 2:
1
test 3:
2
test 4:
3
test 5:
5
test 6:
1
test 7:
3
test 8:
2
test 9:
3
test 10:
4
test 11:
1
test 12:
1
test 13:
1
test 14:
1
test 15:
1
test 16:
1
test 17:
0
test 18:
1
test 19:
2
test 20:
3
test 21:
5
EOF

# Test z (stack depth), Z (digit count/string length), X (scale)
$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc zp c
[test 2:]pc 1 zp c
[test 3:]pc 1 2 zp c
[test 4:]pc 1 2 3 zp c
[test 5:]pc 12345Zp c
[test 6:]pc 0Zp c
[test 7:]pc 123Zp c
[test 8:]pc 1.5Zp c
[test 9:]pc 1.23Zp c
[test 10:]pc 1.001Zp c
[test 11:]pc 0.5Zp c
[test 12:]pc 0.05Zp c
[test 13:]pc 0.005Zp c
[test 14:]pc .5Zp c
[test 15:]pc .05Zp c
[test 16:]pc .005Zp c
[test 17:]pc 0Xp c
[test 18:]pc 1.2Xp c
[test 19:]pc 1.23Xp c
[test 20:]pc 1.234Xp c
[test 21:]pc [hello]Zp c
EOF
