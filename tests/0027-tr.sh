#!/bin/sh

tmp=tmp.$$

trap 'rm -f $tmp' EXIT
trap 'exit $?' HUP INT TERM

cat <<EOF > $tmp
STATIC INT RADIX = 16;
STATIC INT PFLAG;
STATIC INT AFLAG;
STATIC INT VFLAG;
STATIC INT GFLAG;
STATIC INT UFLAG;
STATIC INT ARFLAG;
EOF

$EXEC ../tr '[:lower:]' '[:upper:]' <<EOF | diff -u $tmp -
static int radix = 16;
static int Pflag;
static int Aflag;
static int vflag;
static int gflag;
static int uflag;
static int arflag;
EOF
