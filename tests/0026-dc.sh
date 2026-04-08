#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

# Expected output for printnum tests
cat <<EOF >$tmp
test 1:
100
test 2:
0
test 3:
-42
test 4:
.5
test 5:
.05
test 6:
.001
test 7:
1.5
test 8:
-.5
test 9:
-1.25
test 10:
.4
test 11:
.0
.1
test 12:
.0
test 13:
1.0
test 14:
.2
test 15:
.1
test 16:
.01
test 17:
.001
test 18:
.8
EOF

$EXEC ../dc <<EOF | diff -u $tmp -
[test 1:]pc 100p
[test 2:]pc 0p
[test 3:]pc _42p
[test 4:]pc .5p
[test 5:]pc .05p
[test 6:]pc .001p
[test 7:]pc 1.5p
[test 8:]pc _.5p
[test 9:]pc _1.25p
[test 10:]pc 16o.3p
[test 11:]pc 2o.1p10op
[test 12:]pc 2o.3p
[test 13:]pc 2o1.1p
[test 14:]pc 3o.7p
[test 15:]pc 2o.5p
[test 16:]pc 2o.25p
[test 17:]pc 2o.125p
[test 18:]pc 16o.5p
EOF
