#!/bin/sh

export TZ=UTC

trap 'rm -f test.res' EXIT HUP INT TERM
trap 'exit $?' HUP INT TERM

for i in *-*.sh
do
	printf "Test: %s\n\n" $i >> test.log
	(./$i >> test.log 2>&1 && printf '[PASS]\t' || printf '[FAIL]\t'
	echo "$i") | tee -a test.log
done |
tee test.res

! grep FAIL test.res >/dev/null
