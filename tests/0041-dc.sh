#!/bin/sh

tmp1=$$.tmp1
tmp2=$$.tmp2

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

cat <<'EOF' > $tmp1
test 1:
test 2:
test 3:
test 4:
test 5:
99
test 6:
1
4
test 7:
in-macro
after-macro
test 8:
inner
after-all
test 9:
before
after
test 10:
not-equal
continued
test 11:
equal
continued
test 12:
3
2
done
test 12a:
3
done
test 13:
0
1
2
done
test 13a:
0
done
test 14:
deep
outer
final
test 15:
42
test 16:
done
test 17:
first
last
test 18:
before
test 19:
before-q
test 20:
equal
../dc: stack empty
../dc: Q command argument exceeded string execution depth
../dc: Q command requires a number >= 0
../dc: Q command argument exceeded string execution depth
EOF

(exec 2>$tmp2
$EXEC ../dc <<'EOF'
[test 1:]pc Q
[test 2:]pc 1Q
[test 3:]pc  _1Q
[test 4:]pc [100Q]x
[test 5:]pc 99 [1Q]x p
[test 6:]pc [[1p q 2p]x 3p]x 4p
[test 7:]pc [[in-macro]p 1Q [not-printed]p]x [after-macro]p
[test 8:]pc [[[inner]p 2Q [not1]p]x [not2]p]x [after-all]p
[test 9:]pc [before]p 0Q [after]p
[test 10:]pc [[equal-quit]p q]sa 5 3 =a [not-equal]p [continued]p
[test 11:]pc [[equal-quit]p q]sa 5 5 !=a [equal]p [continued]p
[test 12:]pc 3[[p 1- d 2 !>b 1Q]x]sb lbx [done]p
[test 12a:]pc 3[[p 1- d 2 >b 1Q]x]sb lbx [done]p
[test 13:]pc 0[[p 1+ d 2 !<b 1Q]x]sb lbx [done]p
[test 13a:]pc 0[[p 1+ d 2 <b 1Q]x]sb lbx [done]p
[test 14:]pc [[[[deep]p 2Q [x]p]x [y]p]x [outer]p]x [final]p
[test 15:]pc [[42 q]x [x]p]x p
[test 16:]pc [[1Q [not]p]x [done]p]x
[test 17:]pc [[[first]p q q q]x [x]p]x [last]p
[test 18:]pc [before]p q [after]p
EOF

$EXEC ../dc <<'EOF'
[test 19:]pc [[before-q]p q [after-q]p]x [never]p
EOF

$EXEC ../dc <<'EOF'
[test 20:]pc [[equal]p q]sa 5 5 =a [not-printed]p
EOF
cat $tmp2
) | diff -u - $tmp1
