#!/bin/bash


rm test_result.txt
touch test_result.txt
for i in 10000 20000 30000 40000 50000 60000 70000 80000 90000 100000 1000000
do
	./index $i | grep "elapsed" >> test_result.txt
	echo "done $i"
done

exit 0
