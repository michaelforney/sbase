#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

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
6
test 7:
7
test 8:
9
test 9:
10
test 10:
1
test 11:
1
test 12:
1.41
test 13:
1.4142
test 14:
1.414213
test 15:
1.7
test 16:
1.732
test 17:
1.73205
test 18:
.50
test 19:
.2500
test 20:
.10
test 21:
.0100
test 22:
.001000
test 23:
.7
test 24:
.353
test 25:
.3
test 26:
.316
test 27:
.31622
test 28:
.0316
test 29:
1.20
test 30:
1.5000
test 31:
1.22
test 32:
1.2247
test 33:
1.110
test 34:
1.11085
test 35:
.9486
test 36:
.999499
test 37:
1.58
test 38:
3.5128
test 39:
2.0
test 40:
2.00
test 41:
2.000
test 42:
2.0000000000
test 43:
100.0000
test 44:
11.111075
test 45:
100000000
test 46:
9999
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 0k 0vp
[test 2:]pc 0k 1vp
[test 3:]pc 0k 4vp
[test 4:]pc 0k 9vp
[test 5:]pc 0k 25vp
[test 6:]pc 0k 36vp
[test 7:]pc 0k 49vp
[test 8:]pc 0k 81vp
[test 9:]pc 0k 100vp
[test 10:]pc 0k 2vp
[test 11:]pc 0k 3vp
[test 12:]pc 2k 2vp
[test 13:]pc 4k 2vp
[test 14:]pc 6k 2vp
[test 15:]pc 1k 3vp
[test 16:]pc 3k 3vp
[test 17:]pc 5k 3vp
[test 18:]pc 2k .25vp
[test 19:]pc 4k .0625vp
[test 20:]pc 2k .01vp
[test 21:]pc 4k .0001vp
[test 22:]pc 6k .000001vp
[test 23:]pc 1k .5vp
[test 24:]pc 3k .125vp
[test 25:]pc 1k .1vp
[test 26:]pc 3k .1vp
[test 27:]pc 5k .1vp
[test 28:]pc 4k .001vp
[test 29:]pc 2k 1.44vp
[test 30:]pc 4k 2.25vp
[test 31:]pc 2k 1.5vp
[test 32:]pc 4k 1.5vp
[test 33:]pc 3k 1.234vp
[test 34:]pc 5k 1.234vp
[test 35:]pc 4k .9vp
[test 36:]pc 6k .999vp
[test 37:]pc 2k 2.5vp
[test 38:]pc 4k 12.34vp
[test 39:]pc 1k 4vp
[test 40:]pc 2k 4vp
[test 41:]pc 3k 4vp
[test 42:]pc 10k 4vp
[test 43:]pc 4k 10000vp
[test 44:]pc 6k 123.456vp
[test 45:]pc 0k 10000000000000000vp
[test 46:]pc 0k 99980001vp
EOF
