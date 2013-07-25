#!/bin/bash

make cleanall
make
rm test_result.txt
touch test_result.txt
for i in 14510025 7780738 4036624 2056977 1038436 704556 30385
do
	./index $i | grep "elapsed" >> test_result.txt
	echo "done $i"
done

exit 0
