#!/bin/sh

tmp1=tmp1.$$
tmp2=tmp2.$$

trap 'rm -f $tmp1 $tmp2' EXIT
trap 'exit $?' HUP INT TERM

../touch -t 200711121015 $tmp1

echo 'Modify: 2007-11-12 10:15:00.000000000' > $tmp2

stat $tmp1 | awk '/^Modify:/ {$4 = ""; print}' | diff -wu - $tmp2
