#!/bin/sh

## Testing ##


# Regression
for t in -aNe -ade -aNTe -aWNe
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl $t  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done


# TO FIX
echo "aT"
sysctl -aT >> sysctl.txt
./nsysctl -aT >> nsysctl.txt
#diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt
