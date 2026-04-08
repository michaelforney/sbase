#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF > $tmp
FOOBAR123LOREM@IPSUM
EOF

($EXEC ../tr -cd '[:upper:][:digit:]@' <<EOF; echo) | diff -u $tmp -
Forem OOsBm __aor Ait amet, consectetuR ad1piscing elit. i2teger e\$#icit3r
neLue eOet sRm acEumsan eM@Ismod. aePean ac niSi eU erat gravida vulputateM
EOF
