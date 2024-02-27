#!/bin/bash

bin/iss_sort -print-settings -i /dev/null -d /tmp/ > test/dump_all.txt

cat test/dump_all.txt | \
    awk '/Settings begin/{flag=1;next}/Settings end/{flag=0}flag' > \
	test/dump_settings.txt

diff test/dump_settings.good test/dump_settings.txt
