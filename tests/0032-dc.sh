#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test ~ command: divmod (pushes quotient then remainder, remainder on top)
# Output format: each ~ produces two lines: remainder (top), then quotient
# Note: ~ now uses divscale like / and %, so results should match
cat <<EOF >$tmp
test 1:
0
3
test 2:
1
3
test 3:
1
2
test 4:
1
3
test 5:
2
14
test 6:
0
10
test 7:
1
33
test 8:
0
3
test 9:
2
3
test 10:
0
0
test 11:
0
1
test 12:
3
0
test 13:
0
1
test 14:
1
0
test 15:
1
0
test 16:
1
0
test 17:
0
5
test 18:
0
100
test 19:
99
0
test 20:
0
-3
test 21:
0
-3
test 22:
0
3
test 23:
-1
-2
test 24:
1
-2
test 25:
-1
2
test 26:
-1
-3
test 27:
1
-3
test 28:
-1
3
test 29:
0
-10
test 30:
0
-10
test 31:
0
10
test 32:
0
-5
test 33:
0
-5
test 34:
0
5
test 35:
0
0
test 36:
1
1
test 37:
789
123456
test 38:
4
14285714285
test 39:
0
1371742100137174210
test 40:
1
49999999999999999999
test 41:
0
33333333333333333333
test 42:
1
1
test 43:
0
1
test 44:
0
3
test 45:
0
3
test 46:
1.5
3
test 47:
0
3
test 48:
1.5
2
test 49:
0
1
test 50:
0
5
test 51:
0
-3
test 52:
0
-3
test 53:
0
3
test 54:
0
-3
test 55:
0
-3
test 56:
0
3
test 57:
0
1
test 58:
.001
3
test 59:
.01
3
test 60:
0
20
test 61:
.5
0
test 62:
0
1000
test 63:
0
4
test 64:
0
4
test 65:
0
16
test 66:
15
15
test 67:
1
16
test 68:
0
32
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 6 2~f c
[test 2:]pc 7 2~f c
[test 3:]pc 7 3~f c
[test 4:]pc 10 3~f c
[test 5:]pc 100 7~f c
[test 6:]pc 100 10~f c
[test 7:]pc 100 3~f c
[test 8:]pc 15 5~f c
[test 9:]pc 17 5~f c
[test 10:]pc 0 5~f c
[test 11:]pc 5 5~f c
[test 12:]pc 3 7~f c
[test 13:]pc 1 1~f c
[test 14:]pc 1 2~f c
[test 15:]pc 1 5~f c
[test 16:]pc 1 10~f c
[test 17:]pc 5 1~f c
[test 18:]pc 100 1~f c
[test 19:]pc 99 100~f c
[test 20:]pc _6 2~f c
[test 21:]pc 6 _2~f c
[test 22:]pc _6 _2~f c
[test 23:]pc _7 3~f c
[test 24:]pc 7 _3~f c
[test 25:]pc _7 _3~f c
[test 26:]pc _10 3~f c
[test 27:]pc 10 _3~f c
[test 28:]pc _10 _3~f c
[test 29:]pc _100 10~f c
[test 30:]pc 100 _10~f c
[test 31:]pc _100 _10~f c
[test 32:]pc _5 1~f c
[test 33:]pc 5 _1~f c
[test 34:]pc _5 _1~f c
[test 35:]pc 0 _5~f c
[test 36:]pc 1000000 999999~f c
[test 37:]pc 123456789 1000~f c
[test 38:]pc 99999999999 7~f c
[test 39:]pc 12345678901234567890 9~f c
[test 40:]pc 99999999999999999999 2~f c
[test 41:]pc 99999999999999999999 3~f c
[test 42:]pc 99999999999999999999 99999999999999999998~f c
[test 43:]pc 99999999999999999999 99999999999999999999~f c
[test 44:]pc 7.5 2.5~f c
[test 45:]pc 1.5 .5~f c
[test 46:]pc 10.5 3~f c
[test 47:]pc 4.5 1.5~f c
[test 48:]pc 7.5 3~f c
[test 49:]pc .5 .5~f c
[test 50:]pc 2.5 .5~f c
[test 51:]pc _7.5 2.5~f c
[test 52:]pc 7.5 _2.5~f c
[test 53:]pc _7.5 _2.5~f c
[test 54:]pc _1.5 .5~f c
[test 55:]pc 1.5 _.5~f c
[test 56:]pc _1.5 _.5~f c
[test 57:]pc .001 .001~f c
[test 58:]pc .01 .003~f c
[test 59:]pc .1 .03~f c
[test 60:]pc 10 .5~f c
[test 61:]pc .5 10~f c
[test 62:]pc 100 .1~f c
[test 63:]pc 8 2~f c
[test 64:]pc 16 4~f c
[test 65:]pc 256 16~f c
[test 66:]pc 255 16~f c
[test 67:]pc 257 16~f c
[test 68:]pc 1024 32~f c
EOF
