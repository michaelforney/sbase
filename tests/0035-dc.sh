#!/bin/sh

tmp1=$$.tmp1
tmp2=$$.tmp2

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

# Test negative number sqrt - should produce error message and push 0
# Test negative numbers: integers, fractions, odd and even fraction digits
($EXEC ../dc <<EOF 2>$tmp2
[test 1:]pc _1vp
[test 2:]pc _4vp
[test 3:]pc _.5vp
[test 4:]pc _.25vp
EOF
cat $tmp2) > $tmp1

diff -u - $tmp1 <<'EOF'
test 1:
0
test 2:
0
test 3:
0
test 4:
0
../dc: square root of negative number
../dc: square root of negative number
../dc: square root of negative number
../dc: square root of negative number
EOF
