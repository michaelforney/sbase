#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF > $tmp
static int radix = 16;
static int Pflag;
static int Aflag;
static int vflag;
static int gflag;
static int uflag;
static int arflag;

EOF

$EXEC ../ed -s /dev/null <<EOF | diff -u $tmp -
a
int radix = 16;
int Pflag;
int Aflag;
int vflag;
int gflag;
int uflag;
int arflag;

.
?radix?;/^$/-s/^/static /
,p
Q
EOF
