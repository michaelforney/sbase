#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test - command: subtraction
cat <<EOF >$tmp
test 1:
-1
test 2:
0
test 3:
1
test 4:
0
test 5:
0
test 6:
13
test 7:
13
test 8:
-13
test 9:
-13
test 10:
10
test 11:
-10
test 12:
-5
test 13:
5
test 14:
5
test 15:
-5
test 16:
.5
test 17:
.50
test 18:
.50
test 19:
-.40
test 20:
.774
test 21:
-.766
test 22:
3.0
test 23:
0
test 24:
.001
test 25:
.002
test 26:
99999999999999999999
test 27:
-99999999999999999999
test 28:
12345678901234567890
test 29:
0
test 30:
8
test 31:
898
test 32:
99999999999999999998
test 33:
-99999999999999999998
test 34:
199999999999999999997
test 35:
199999999999999999997
test 36:
199999999999999999998
test 37:
-.999999
test 38:
0
test 39:
.000001
test 40:
.003
test 41:
.98
test 42:
-.41976
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 2 3-p
[test 2:]pc 0 0-p
[test 3:]pc _2 _3-p
[test 4:]pc 100 100-p
[test 5:]pc _100 _100-p
[test 6:]pc 10 _3-p
[test 7:]pc 3 _10-p
[test 8:]pc _3 10-p
[test 9:]pc _10 3-p
[test 10:]pc 5 _5-p
[test 11:]pc _5 5-p
[test 12:]pc 0 5-p
[test 13:]pc 0 _5-p
[test 14:]pc 5 0-p
[test 15:]pc _5 0-p
[test 16:]pc 1.0 .5-p
[test 17:]pc 1.5 1.00-p
[test 18:]pc 1.00 .50-p
[test 19:]pc .1 .50-p
[test 20:]pc 1.004 .23-p
[test 21:]pc _.5 .266-p
[test 22:]pc 1.5 _1.5-p
[test 23:]pc _.001 _.001-p
[test 24:]pc _.001 _.002-p
[test 25:]pc .001 _.001-p
[test 26:]pc 99999999999999999999 0-p
[test 27:]pc _99999999999999999999 0-p
[test 28:]pc 12345678901234567890 0-p
[test 29:]pc 0 0-p
[test 30:]pc 9 1-p
[test 31:]pc 999 101-p
[test 32:]pc 99999999999999999999 1-p
[test 33:]pc _99999999999999999999 _1-p
[test 34:]pc 99999999999999999999 _99999999999999999998-p
[test 35:]pc 99999999999999999998 _99999999999999999999-p
[test 36:]pc 99999999999999999999 _99999999999999999999-p
[test 37:]pc .000001 1-p
[test 38:]pc .0000000001 .0000000001-p
[test 39:]pc _.000001 _.000002-p
[test 40:]pc .001 _.002-p
[test 41:]pc .99 .01-p
[test 42:]pc .12345 .54321-p
EOF
