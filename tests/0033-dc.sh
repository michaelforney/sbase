#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Expected output for exponentiation tests
# Values derived from system bc
cat <<EOF >$tmp
test 1:
1
test 2:
2
test 3:
8
test 4:
1024
test 5:
243
test 6:
1000000
test 7:
4
test 8:
-8
test 9:
16
test 10:
-32
test 11:
-27
test 12:
81
test 13:
-1000
test 14:
-100000
test 15:
1000000
test 16:
1
test 17:
1
test 18:
1
test 19:
1
test 20:
.5000000000
test 21:
.2500000000
test 22:
.1250000000
test 23:
.0625000000
test 24:
.0010000000
test 25:
-.1250000000
test 26:
.0625000000
test 27:
2.2500
test 28:
3.375000000
test 29:
.2500
test 30:
.125000000
test 31:
2.2500
test 32:
-3.375000000
test 33:
1.56250000
test 34:
.06250000
test 35:
.0156250000
test 36:
.06250000
test 37:
-.0156250000
test 38:
.0156250000
test 39:
-.0019531250
test 40:
4.0000000000
test 41:
8.0000000000
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 2 0^p
[test 2:]pc 2 1^p
[test 3:]pc 2 3^p
[test 4:]pc 2 10^p
[test 5:]pc 3 5^p
[test 6:]pc 10 6^p
[test 7:]pc _2 2^p
[test 8:]pc _2 3^p
[test 9:]pc _2 4^p
[test 10:]pc _2 5^p
[test 11:]pc _3 3^p
[test 12:]pc _3 4^p
[test 13:]pc _10 3^p
[test 14:]pc _10 5^p
[test 15:]pc _10 6^p
[test 16:]pc 0 0^p
[test 17:]pc 5 0^p
[test 18:]pc _5 0^p
[test 19:]pc 100 0^p
[test 20:]pc 10k 2 _1^p
[test 21:]pc 10k 2 _2^p
[test 22:]pc 10k 2 _3^p
[test 23:]pc 10k 4 _2^p
[test 24:]pc 10k 10 _3^p
[test 25:]pc 10k _2 _3^p
[test 26:]pc 10k _2 _4^p
[test 27:]pc 1.50 2^p
[test 28:]pc 1.500 3^p
[test 29:]pc .50 2^p
[test 30:]pc .500 3^p
[test 31:]pc _1.50 2^p
[test 32:]pc _1.500 3^p
[test 33:]pc 1.2500 2^p
[test 34:]pc .2500 2^p
[test 35:]pc .250000 3^p
[test 36:]pc _.2500 2^p
[test 37:]pc _.250000 3^p
[test 38:]pc .125000 2^p
[test 39:]pc _.125000000 3^p
[test 40:]pc 10k .50 _2^p
[test 41:]pc 10k .500 _3^p
EOF
