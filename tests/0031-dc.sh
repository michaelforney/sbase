#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test % command: modulo (remainder)
# Note: scale (k) persists between operations, so we reset it explicitly
cat <<EOF >$tmp
test 1:
1
test 2:
1
test 3:
2
test 4:
0
test 5:
2
test 6:
0
test 7:
0
test 8:
3
test 9:
-1
test 10:
1
test 11:
-1
test 12:
-1
test 13:
1
test 14:
-1
test 15:
0
test 16:
0
test 17:
0
test 18:
1
test 19:
1
test 20:
1
test 21:
99
test 22:
0
test 23:
0
test 24:
0
test 25:
1
test 26:
789
test 27:
4
test 28:
0
test 29:
1
test 30:
0
test 31:
1
test 32:
0
test 33:
0
test 34:
0
test 35:
0
test 36:
7
test 37:
1
test 38:
15
test 39:
0
test 40:
1
test 41:
0
test 42:
0
test 43:
0
test 44:
1.5
test 45:
0
test 46:
0
test 47:
0
test 48:
0
test 49:
0
test 50:
0
test 51:
0
test 52:
0
test 53:
0
test 54:
.001
test 55:
.01
test 56:
.01
test 57:
0
test 58:
.02
test 59:
-.01
test 60:
.01
test 61:
-.01
test 62:
.0001
test 63:
.0002
test 64:
.0005
test 65:
0
test 66:
.005
test 67:
.0050
test 68:
0
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 0k 7 3%p
[test 2:]pc 0k 10 3%p
[test 3:]pc 0k 100 7%p
[test 4:]pc 0k 15 5%p
[test 5:]pc 0k 17 5%p
[test 6:]pc 0k 0 5%p
[test 7:]pc 0k 5 5%p
[test 8:]pc 0k 3 7%p
[test 9:]pc 0k _7 3%p
[test 10:]pc 0k 7 _3%p
[test 11:]pc 0k _7 _3%p
[test 12:]pc 0k _10 3%p
[test 13:]pc 0k 10 _3%p
[test 14:]pc 0k _10 _3%p
[test 15:]pc 0k 1 1%p
[test 16:]pc 0k 2 2%p
[test 17:]pc 0k 3 3%p
[test 18:]pc 0k 1 2%p
[test 19:]pc 0k 1 10%p
[test 20:]pc 0k 1 100%p
[test 21:]pc 0k 99 100%p
[test 22:]pc 0k 5 1%p
[test 23:]pc 0k 100 1%p
[test 24:]pc 0k _5 1%p
[test 25:]pc 0k 1000000 999999%p
[test 26:]pc 0k 123456789 1000%p
[test 27:]pc 0k 99999999999 7%p
[test 28:]pc 0k 12345678901234567890 9%p
[test 29:]pc 0k 99999999999999999999 2%p
[test 30:]pc 0k 99999999999999999999 3%p
[test 31:]pc 0k 99999999999999999999 99999999999999999998%p
[test 32:]pc 0k 99999999999999999999 99999999999999999999%p
[test 33:]pc 0k 8 2%p
[test 34:]pc 0k 8 4%p
[test 35:]pc 0k 16 8%p
[test 36:]pc 0k 15 8%p
[test 37:]pc 0k 17 8%p
[test 38:]pc 0k 255 16%p
[test 39:]pc 0k 256 16%p
[test 40:]pc 0k 257 16%p
[test 41:]pc 0k 7.5 2.5%p
[test 42:]pc 0k 1.5 .5%p
[test 43:]pc 0k 3.75 1.25%p
[test 44:]pc 0k 7.5 3%p
[test 45:]pc 0k 4.5 1.5%p
[test 46:]pc 0k 2.5 .5%p
[test 47:]pc 0k _7.5 2.5%p
[test 48:]pc 0k 7.5 _2.5%p
[test 49:]pc 0k _7.5 _2.5%p
[test 50:]pc 0k _1.5 .5%p
[test 51:]pc 0k 1.5 _.5%p
[test 52:]pc 0k _1.5 _.5%p
[test 53:]pc 0k .001 .001%p
[test 54:]pc 0k .01 .003%p
[test 55:]pc 2k 7 3%p
[test 56:]pc 2k 10 3%p
[test 57:]pc 2k 17 5%p
[test 58:]pc 2k 22 7%p
[test 59:]pc 2k _7 3%p
[test 60:]pc 2k 7 _3%p
[test 61:]pc 2k _7 _3%p
[test 62:]pc 4k 1 3%p
[test 63:]pc 4k 2 3%p
[test 64:]pc 4k 10 7%p
[test 65:]pc 2k 1.5 .5%p
[test 66:]pc 2k 3.5 1.5%p
[test 67:]pc 2k 7.25 2.25%p
[test 68:]pc 2k 10.5 3.5%p
EOF
