#!/bin/sh

# Regression % sysctl -N = only name
for t in -aN -aNT -aNW
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl $t > nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -ad = name: description
for t in -ad -adT -adW
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}N  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -at = name: type
for t in -at
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}N  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -a = name: value
for t in -aT -aTo -aWo -aWx -aW
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}NV  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

# TO FIX
echo "sysctl -ao"
sysctl -ao >> sysctl.txt
./nsysctl -aoNV >> nsysctl.txt
#diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt

