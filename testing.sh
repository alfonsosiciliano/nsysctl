#!/bin/sh

# Regression % sysctl -N, name
for t in -aN -aNT -aNW
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl $t -I > nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    meld sysctl.txt nsysctl.txt
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -ad, name and description
for t in -ad -adT -adW
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}N  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -at, name and type
for t in -at
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}N  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

#Regression  % sysctl -a, name and value
for t in -aT -aTo -aWo -aWx -aW -ao
do
    echo "$t"
    sysctl $t >> sysctl.txt
    ./nsysctl ${t}NV  >> nsysctl.txt
    diff sysctl.txt nsysctl.txt | diffstat -m
    rm sysctl.txt nsysctl.txt
done

echo "sysctl -a"
sysctl -a >> sysctl.txt
./nsysctl -aNV >> nsysctl.txt
diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
#meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt

