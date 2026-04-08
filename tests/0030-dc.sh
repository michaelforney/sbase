#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Test / command: division
# Note: scale (k) persists between operations, so we reset it explicitly
cat <<EOF >$tmp
test 1:
3
test 2:
3
test 3:
10
test 4:
33
test 5:
-3
test 6:
-3
test 7:
3
test 8:
0
test 9:
1
test 10:
5
test 11:
0
test 12:
3.50
test 13:
.33
test 14:
.3333
test 15:
2.50
test 16:
3.14
test 17:
3.142857
test 18:
-3.50
test 19:
-3.50
test 20:
3.50
test 21:
-.33
test 22:
-.33
test 23:
.33
test 24:
3
test 25:
2
test 26:
3
test 27:
20
test 28:
0
test 29:
.05
test 30:
1000
test 31:
1000001
test 32:
1
test 33:
12345678901234567890
test 34:
.3333333333
test 35:
.6666666666
test 36:
.1428571428
test 37:
.1111111111
test 38:
.0909090909
test 39:
.5
test 40:
.500
test 41:
.50000
test 42:
.2
test 43:
.250
test 44:
.1
test 45:
.125
test 46:
-3
test 47:
-3
test 48:
3
test 49:
-2.50
test 50:
-2.50
test 51:
2.50
test 52:
1.00000000
test 53:
.00100000
test 54:
1000.00000000
test 55:
.01000000
test 56:
100.00000000
test 57:
1
test 58:
1
test 59:
-1
test 60:
-1
test 61:
0
test 62:
0
test 63:
0
test 64:
100
test 65:
-100
test 66:
-100
test 67:
100
test 68:
10
test 69:
100
test 70:
100
test 71:
1000
test 72:
0
test 73:
0
test 74:
0
test 75:
0
test 76:
0
test 77:
0
test 78:
.50
test 79:
.10
test 80:
.01
test 81:
.50
test 82:
.99
test 83:
9
test 84:
8
test 85:
7
test 86:
6
test 87:
5
test 88:
4
test 89:
3
test 90:
2
test 91:
1
test 92:
99999999999999999999
test 93:
0
test 94:
.00000000000000000001
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 0k 6 2/p
[test 2:]pc 0k 7 2/p
[test 3:]pc 0k 100 10/p
[test 4:]pc 0k 100 3/p
[test 5:]pc 0k _6 2/p
[test 6:]pc 0k 6 _2/p
[test 7:]pc 0k _6 _2/p
[test 8:]pc 0k 0 5/p
[test 9:]pc 0k 1 1/p
[test 10:]pc 0k 5 1/p
[test 11:]pc 0k 1 5/p
[test 12:]pc 2k 7 2/p
[test 13:]pc 2k 1 3/p
[test 14:]pc 4k 1 3/p
[test 15:]pc 2k 10 4/p
[test 16:]pc 2k 22 7/p
[test 17:]pc 6k 22 7/p
[test 18:]pc 2k _7 2/p
[test 19:]pc 2k 7 _2/p
[test 20:]pc 2k _7 _2/p
[test 21:]pc 2k _1 3/p
[test 22:]pc 2k 1 _3/p
[test 23:]pc 2k _1 _3/p
[test 24:]pc 0k 1.5 .5/p
[test 25:]pc 0k 3.0 1.5/p
[test 26:]pc 0k .75 .25/p
[test 27:]pc 0k 10 .5/p
[test 28:]pc 0k .5 10/p
[test 29:]pc 2k .5 10/p
[test 30:]pc 0k 1000000 1000/p
[test 31:]pc 0k 999999999999 999999/p
[test 32:]pc 0k 12345678901234567890 12345678901234567890/p
[test 33:]pc 0k 12345678901234567890 1/p
[test 34:]pc 10k 1 3/p
[test 35:]pc 10k 2 3/p
[test 36:]pc 10k 1 7/p
[test 37:]pc 10k 1 9/p
[test 38:]pc 10k 1 11/p
[test 39:]pc 1k 1 2/p
[test 40:]pc 3k 1 2/p
[test 41:]pc 5k 1 2/p
[test 42:]pc 1k 1 4/p
[test 43:]pc 3k 1 4/p
[test 44:]pc 1k 1 8/p
[test 45:]pc 3k 1 8/p
[test 46:]pc 0k _1.5 .5/p
[test 47:]pc 0k 1.5 _.5/p
[test 48:]pc 0k _1.5 _.5/p
[test 49:]pc 2k _10 4/p
[test 50:]pc 2k 10 _4/p
[test 51:]pc 2k _10 _4/p
[test 52:]pc 8k .001 .001/p
[test 53:]pc 8k .001 1/p
[test 54:]pc 8k 1 .001/p
[test 55:]pc 8k .0001 .01/p
[test 56:]pc 8k .01 .0001/p
[test 57:]pc 0k 1 1/p
[test 58:]pc 0k _1 _1/p
[test 59:]pc 0k _1 1/p
[test 60:]pc 0k 1 _1/p
[test 61:]pc 0k 0 1/p
[test 62:]pc 0k 0 _1/p
[test 63:]pc 0k 0 100/p
[test 64:]pc 0k 100 1/p
[test 65:]pc 0k _100 1/p
[test 66:]pc 0k 100 _1/p
[test 67:]pc 0k _100 _1/p
[test 68:]pc 0k 100 10/p
[test 69:]pc 0k 1000 10/p
[test 70:]pc 0k 10000 100/p
[test 71:]pc 0k 1000000 1000/p
[test 72:]pc 0k 10 100/p
[test 73:]pc 0k 10 1000/p
[test 74:]pc 0k 1 2/p
[test 75:]pc 0k 1 10/p
[test 76:]pc 0k 1 100/p
[test 77:]pc 0k 5 10/p
[test 78:]pc 2k 1 2/p
[test 79:]pc 2k 1 10/p
[test 80:]pc 2k 1 100/p
[test 81:]pc 2k 5 10/p
[test 82:]pc 2k 99 100/p
[test 83:]pc 0k 81 9/p
[test 84:]pc 0k 64 8/p
[test 85:]pc 0k 49 7/p
[test 86:]pc 0k 36 6/p
[test 87:]pc 0k 25 5/p
[test 88:]pc 0k 16 4/p
[test 89:]pc 0k 9 3/p
[test 90:]pc 0k 4 2/p
[test 91:]pc 0k 99999999999999999999 99999999999999999999/p
[test 92:]pc 0k 99999999999999999999 1/p
[test 93:]pc 0k 1 99999999999999999999/p
[test 94:]pc 20k 1 99999999999999999999/p
EOF
