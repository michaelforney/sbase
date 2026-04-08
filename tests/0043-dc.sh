#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test -i flag for extended register names
# <n> syntax: n is parsed as decimal and used as register name byte
# "str" syntax: str is used as multi-character register name

$EXEC ../dc -i <<'EOF' >$tmp 2>&1
[test 1:]pc 42 s<65> l<65> p c
[test 2:]pc 100 s"foo" l"foo" p c
[test 3:]pc 99 s<65> lA p c
[test 4:]pc 1 S<66> 2 S<66> L<66> p L<66> p c
[test 5:]pc 10 S"bar" 20 S"bar" L"bar" p L"bar" p c
[test 6:]pc 5 s<67> lC p c
[test 7:]pc 1 s"x" 2 s"xy" 3 s"xyz" l"x" p l"xy" p l"xyz" p c
[test 8:]pc 77 s<0> l<0> p c
[test 9:]pc 88 s"D" lD p c
[test 10:]pc [42p] s<69> l<69> x c
[test 11:]pc [99p] s"macro" l"macro" x c
[test 12:]pc 1 s<70> 2 s<70> 3 s<70> l<70> p c
[test 13:]pc 10 s"reg" 20 s"reg" 30 s"reg" l"reg" p c
EOF

diff -u - $tmp <<'EOF'
test 1:
42
test 2:
100
test 3:
99
test 4:
2
1
test 5:
20
10
test 6:
5
test 7:
1
2
3
test 8:
77
test 9:
88
test 10:
42
test 11:
99
test 12:
3
test 13:
30
EOF
