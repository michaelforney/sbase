#!/bin/sh

set -e

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

(
	$EXEC ../printf '123\n'
	$EXEC ../printf '%d\n'
	$EXEC ../printf '%b' 'foo\nbar\n'

	# Two flags used simulatenously, + and 0
	$EXEC ../printf '%+04d %+4d ' 1 2 3 -400; ../printf "\n"
	# Missing format specifier; should have sane error message
	$EXEC ../printf '%000' FOO || echo "Expected failure"
) > $tmp 2>&1

diff -u - $tmp <<'EOF'
123
0
foo
bar
+001   +2 +003 -400 
../printf: Missing format specifier.
Expected failure
EOF
