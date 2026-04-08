#!/bin/sh

tmp1=$$.tmp1
tmp2=$$.tmp2

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

# Test x, >, !>, <, !<, =, != commands
# Note: dc pops values and compares: first_popped OP second_popped
# So "3 5 >a" pops 5 then 3, checks 5 > 3 (true)
# And "5 3 >a" pops 3 then 5, checks 3 > 5 (false)
($EXEC ../dc <<'EOF' 2>$tmp2
[test 1:]pc [42p]x c
[test 2:]pc 5 x p c
[test 3:]pc []x c
[test 4:]pc [[10p]x]x c
[test 5:]pc [[YES]p]sa 3 5 >a c
[test 6:]pc [[NO]p]sa 5 3 >a c
[test 7:]pc [[NO]p]sa 5 5 >a c
[test 8:]pc [[YES]p]sa 5 3 <a c
[test 9:]pc [[NO]p]sa 3 5 <a c
[test 10:]pc [[NO]p]sa 5 5 <a c
[test 11:]pc [[YES]p]sa 5 5 =a c
[test 12:]pc [[NO]p]sa 5 3 =a c
[test 13:]pc [[NO]p]sa 3 5 !>a c
[test 14:]pc [[YES]p]sa 5 3 !>a c
[test 15:]pc [[YES]p]sa 5 5 !>a c
[test 16:]pc [[NO]p]sa 5 3 !<a c
[test 17:]pc [[YES]p]sa 3 5 !<a c
[test 18:]pc [[YES]p]sa 5 5 !<a c
[test 19:]pc [[YES]p]sa 5 3 !=a c
[test 20:]pc [[NO]p]sa 5 5 !=a c
[test 21:]pc [[NO]p]sa _3 _5 >a c
[test 22:]pc [[YES]p]sa _5 _3 >a c
[test 23:]pc [[NO]p]sa 3 _5 >a c
[test 24:]pc [[YES]p]sa _3 5 >a c
[test 25:]pc [[YES]p]sa 0 0 =a c
[test 26:]pc [[YES]p]sa _0 0 =a c
[test 27:]pc [[YES]p]sa 1.4 1.5 >a c
[test 28:]pc [[YES]p]sa 1.5 1.5 =a c
[test 29:]pc [[YES]p]sa 1.5 1.4 <a c
[test 30:]pc [[YES]p]sa 99999999999999999998 99999999999999999999 >a c
[test 31:]pc [d p 1 - d 0 <a]sa 5 la x c
[test 32:]pc [[YES]p]sa [2 2 =a]sb 2 2 =b c
[test 33:]pc 99 sa la x p c
[test 34:]pc [3p]sa [2p]sb 2 3 >a 3 2 <b c
[test 35:]pc [[NO]p]sa 1 2 <a z p c
[test 36:]pc [[[[[77p]]]]]x x x x x c
[test 37:]pc [[YES]p]sa 2k 1.50 1.5 =a c
[test 38:]pc [1p]x [2p]x [3p]x c
[test 39:]pc x
[test 40:]pc [[NO]p]sa 5 >a
[test 41:]pc [[NO]p]sa >a
EOF
cat $tmp2) > $tmp1

diff -u - $tmp1 <<'EOF'
test 1:
42
test 2:
5
test 3:
test 4:
10
test 5:
YES
test 6:
test 7:
test 8:
YES
test 9:
test 10:
test 11:
YES
test 12:
test 13:
test 14:
YES
test 15:
YES
test 16:
test 17:
YES
test 18:
YES
test 19:
YES
test 20:
test 21:
test 22:
YES
test 23:
test 24:
YES
test 25:
YES
test 26:
YES
test 27:
YES
test 28:
YES
test 29:
YES
test 30:
YES
test 31:
5
4
3
2
1
test 32:
YES
test 33:
99
test 34:
3
2
test 35:
0
test 36:
77
test 37:
YES
test 38:
1
2
3
test 39:
test 40:
test 41:
../dc: stack empty
../dc: stack empty
../dc: stack empty
EOF
