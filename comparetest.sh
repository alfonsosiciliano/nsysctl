#!/bin/sh

## Testing ##


# Regression
for t in -aNe -ade -aNTe -aWNe -aT
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl $t  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done


# TO FIX
echo "a"
sysctl -a >> sysctl.txt
./nsysctl -a >> nsysctl.txt
#diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt
