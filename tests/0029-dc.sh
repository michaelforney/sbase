#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test * command: multiplication
cat <<EOF >$tmp
test 1:
6
test 2:
0
test 3:
6
test 4:
10000
test 5:
10000
test 6:
-30
test 7:
-30
test 8:
-30
test 9:
-30
test 10:
-25
test 11:
-25
test 12:
0
test 13:
0
test 14:
0
test 15:
0
test 16:
5
test 17:
5
test 18:
-5
test 19:
-5
test 20:
.5
test 21:
1.50
test 22:
.50
test 23:
0
test 24:
3.7
test 25:
.2
test 26:
1.00
test 27:
1.000
test 28:
0
test 29:
-3.7
test 30:
-3.7
test 31:
3.7
test 32:
-.2
test 33:
-.2
test 34:
.2
test 35:
0
test 36:
0
test 37:
12345678901234567890
test 38:
-12345678901234567890
test 39:
81
test 40:
9801
test 41:
998001
test 42:
99980001
test 43:
9999999800000001
test 44:
0
test 45:
.10000
test 46:
1.0
test 47:
4.5
test 48:
4.5
test 49:
1.0
test 50:
1.00
test 51:
1.000
test 52:
4
test 53:
16
test 54:
64
test 55:
256
test 56:
65536
test 57:
1
test 58:
-1
test 59:
-1
test 60:
1
test 61:
0
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 2 3*p
[test 2:]pc 0 0*p
[test 3:]pc _2 _3*p
[test 4:]pc 100 100*p
[test 5:]pc _100 _100*p
[test 6:]pc 10 _3*p
[test 7:]pc 3 _10*p
[test 8:]pc _3 10*p
[test 9:]pc _10 3*p
[test 10:]pc 5 _5*p
[test 11:]pc _5 5*p
[test 12:]pc 0 5*p
[test 13:]pc 0 _5*p
[test 14:]pc 5 0*p
[test 15:]pc _5 0*p
[test 16:]pc 1 5*p
[test 17:]pc 5 1*p
[test 18:]pc _1 5*p
[test 19:]pc 5 _1*p
[test 20:]pc 1.0 .5*p
[test 21:]pc 1.5 1.00*p
[test 22:]pc 1.00 .50*p
[test 23:]pc .1 .5*p
[test 24:]pc 1.5 2.5*p
[test 25:]pc .5 .5*p
[test 26:]pc .25 4*p
[test 27:]pc .125 8*p
[test 28:]pc .001 .001*p
[test 29:]pc _1.5 2.5*p
[test 30:]pc 1.5 _2.5*p
[test 31:]pc _1.5 _2.5*p
[test 32:]pc _.5 .5*p
[test 33:]pc .5 _.5*p
[test 34:]pc _.5 _.5*p
[test 35:]pc 99999999999999999999 0*p
[test 36:]pc _99999999999999999999 0*p
[test 37:]pc 12345678901234567890 1*p
[test 38:]pc 12345678901234567890 _1*p
[test 39:]pc 9 9*p
[test 40:]pc 99 99*p
[test 41:]pc 999 999*p
[test 42:]pc 9999 9999*p
[test 43:]pc 99999999 99999999*p
[test 44:]pc .0001 .0001*p
[test 45:]pc .00001 10000*p
[test 46:]pc .1 10*p
[test 47:]pc 1.5 3*p
[test 48:]pc 3 1.5*p
[test 49:]pc 10 .1*p
[test 50:]pc 100 .01*p
[test 51:]pc 1000 .001*p
[test 52:]pc 2 2*p
[test 53:]pc 4 4*p
[test 54:]pc 8 8*p
[test 55:]pc 16 16*p
[test 56:]pc 256 256*p
[test 57:]pc 1 1*p
[test 58:]pc _1 1*p
[test 59:]pc 1 _1*p
[test 60:]pc _1 _1*p
[test 61:]pc 0 0*p
EOF
