#!/bin/sh

set -e

tmp1=tmp1.$$
tmp2=tmp2.$$

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF >$tmp1
foo
bar

baz
EOF

# Unspecified whether quit with a dirty buffer is considered an error, allow both
$EXEC ../ed $tmp1 <<EOF >$tmp2 || test $? -eq 1
v#^\$#p
p
g/^\$/d
,p
q
a
fizz
buzz
.
i
foobar
.
w
v!z\$!d
,p
q
1,2j
1,2j
,p
q
EOF

diff -u - $tmp2 <<EOF
13
foo
bar
baz
baz
foo
bar
baz
?
29
baz
fizz
buzz
?
bazfizzbuzz
?
EOF

diff -u - $tmp1 <<EOF
foo
bar
baz
fizz
foobar
buzz
EOF
