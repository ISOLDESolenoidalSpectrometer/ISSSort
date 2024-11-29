#!/bin/bash

# Check default settings in code.

bin/iss_sort -print-settings -i /dev/null -d /tmp/ > tests/dump_all.txt

cat tests/dump_all.txt | \
    awk '/Settings begin/{flag=1;next}/Settings end/{flag=0}flag' > \
	tests/dump_settings.txt

diff -u tests/dump_settings.good tests/dump_settings.txt

# Check with override values from settings.dat.

cat settings.dat | sed -e "s/^#//" > tests/with_settings_dat_settings.dat

bin/iss_sort -s tests/with_settings_dat_settings.dat -print-settings \
	     -i /dev/null -d /tmp/ > tests/with_settings_dat_dump_all.txt

cat tests/with_settings_dat_dump_all.txt | \
    awk '/Settings begin/{flag=1;next}/Settings end/{flag=0}flag' > \
	tests/with_settings_dat_dump_settings.txt

diff -u tests/dump_settings.good tests/with_settings_dat_dump_settings.txt
