#!/bin/bash

bin/iss_sort -print-settings -i /dev/null -d /tmp/ > tests/dump_all.txt

cat tests/dump_all.txt | \
    awk '/Settings begin/{flag=1;next}/Settings end/{flag=0}flag' > \
	tests/dump_settings.txt

diff -u tests/dump_settings.good tests/dump_settings.txt
