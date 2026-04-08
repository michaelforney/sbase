#!/bin/sh

tmp1=$$.tmp1
tmp2=$$.tmp2

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

# Test : and ; array commands
($EXEC ../dc <<'EOF' 2>$tmp2
[test 1:]pc 42 0:a 0;a p c
[test 2:]pc 10 0:b 20 1:b 30 2:b 0;b p 1;b p 2;b p c
[test 3:]pc 100 5:c 5;c p c
[test 4:]pc _42 0:d 0;d p c
[test 5:]pc 1.5 0:e 0;e p c
[test 6:]pc 99999999999999999999 0:f 0;f p c
[test 7:]pc [hello] 0:g 0;g p c
[test 8:]pc 1 0:h 2 0:h 0;h p c
[test 9:]pc 5 10:i 10;i p c
[test 10:]pc 1 0:j 2 1:j 3 2:j 0;j 1;j + 2;j +p c
[test 11:]pc 100 0:k 0;k 0;k *p c
[test 12:]pc 7 3:l 3;l 3;l 3;l + +p c
[test 13:]pc 1 0:0 2 1:0 0;0 1;0 +p c
[test 14:]pc 50 0:m 0;m 2/p c
[test 15:]pc 10 0:n 0;n 5 * 2:n 2;n p c
[test 16:]pc 42 _1:o
[test 17:]pc _1;p
[test 18:]pc 100 0:q 1 Sq 0;q p Lq p 0;q p c
[test 19:]pc 10 0:r 1 Sr 20 0:r 2 Sr 30 0:r 0;r p Lr p 0;r p Lr p 0;r p c
[test 20:]pc 5 0:s 1 Ss 2 Ss Ls p 0;s p Ls p 0;s p c
[test 21:]pc 42 0:t 99 st 0;t p lt p c
[test 22:]pc 1 0:u 2 1:u 99 Su 50 0:u 0;u p Lu p 0;u p 1;u p c
[test 23:]pc 10 0:v 20 1:v 1 Sv 2 Sv Lv p Lv p 0;v p 1;v p c
[test 24:]pc 100 5:w 1 Sw 200 5:w 2 Sw 300 5:w 5;w p Lw p 5;w p Lw p 5;w p c
EOF
cat $tmp2) > $tmp1

diff -u - $tmp1 <<'EOF'
test 1:
42
test 2:
10
20
30
test 3:
100
test 4:
-42
test 5:
1.5
test 6:
99999999999999999999
test 7:
hello
test 8:
2
test 9:
5
test 10:
6
test 11:
10000
test 12:
21
test 13:
3
test 14:
25
test 15:
50
test 16:
test 17:
test 18:
0
1
100
test 19:
30
2
20
1
10
test 20:
2
0
1
5
test 21:
42
99
test 22:
50
99
1
2
test 23:
2
1
10
20
test 24:
300
2
200
1
100
../dc: array index must fit in a positive integer
../dc: array index must fit in a positive integer
EOF
