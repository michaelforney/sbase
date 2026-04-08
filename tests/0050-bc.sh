#!/bin/sh

tmp=$$.tmp

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<'EOF' > $tmp
par=1
inc=4
EOF

$EXEC ../bc -sp ../dc <<'EOF' | diff -u - $tmp
define alpha(par, inc) {
	auto cnt

	par = par + 1
	cnt = par + inc
	return (cnt)
}

par = 1
inc = alpha(par, 2)
print "par=",par
print "inc=",inc
EOF
