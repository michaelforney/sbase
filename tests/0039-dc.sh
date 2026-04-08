#!/bin/sh

tmp1=$$.tmp1
tmp2=$$.tmp2

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

# Test s, l, S, L register commands
($EXEC ../dc <<'EOF' 2>$tmp2
[test 1:]pc 5 sa la p c
[test 2:]pc lz p c
[test 3:]pc 1 sb 2 lb p c
[test 4:]pc 1 sc 2 sc lc p c
[test 5:]pc 1 sd ld ld +p c
[test 6:]pc 5 Se le p c
[test 7:]pc 1 Sf 2 Sf 3 Sf lf p c
[test 8:]pc 1 Sg 2 Sg Lg p c
[test 9:]pc 1 Sh 2 Sh Lh Lh +p c
[test 10:]pc 1 Si Li p c
[test 11:]pc 1 sj 2 Sj 3 Sj Lj Lj lj p c
[test 12:]pc _42 sk lk p c
[test 13:]pc 1.5 sl ll p c
[test 14:]pc 99999999999999999999 sm lm p c
[test 15:]pc [hello] sn ln p c
[test 16:]pc 1 so 2 sp lo lp +p c
[test 17:]pc 1 Sq 2 Sr Lq Lr +p c
[test 18:]pc 1 St 2 St 3 St Lt p Lt p Lt p c
[test 19:]pc 1 2 3 Su Su Su Lu Lu Lu + +p c
[test 20:]pc 1 sv lv lv lv + +p c
[test 21:]pc 1 Sw 2 Sw 3 Sw 4 Sw 5 Sw Lw p Lw p Lw p Lw p Lw p c
[test 22:]pc 1 Sx 2 Sy 3 Sx 4 Sy Lx Ly * Lx Ly * +p c
[test 23:]pc 42 s0 100 S0 L0 p L0 p c
[test 24:]pc LA
[test 25:]pc 1 SB LB LB
[test 26:]pc sC
[test 27:]pc SD
EOF
cat $tmp2) > $tmp1

diff -u - $tmp1 <<'EOF'
test 1:
5
test 2:
0
test 3:
1
test 4:
2
test 5:
2
test 6:
5
test 7:
3
test 8:
2
test 9:
3
test 10:
1
test 11:
1
test 12:
-42
test 13:
1.5
test 14:
99999999999999999999
test 15:
hello
test 16:
3
test 17:
3
test 18:
3
2
1
test 19:
6
test 20:
3
test 21:
5
4
3
2
1
test 22:
14
test 23:
100
42
test 24:
test 25:
test 26:
test 27:
../dc: stack register 'A' (101) is empty
../dc: stack register 'B' (102) is empty
../dc: stack empty
../dc: stack empty
EOF
