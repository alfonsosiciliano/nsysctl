#!/bin/sh

sysctl -o  kern >> sysctl.txt
./nsysctl -o  kern >> nsysctl.txt

diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
#rm sysctl.txt nsysctl.txt
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt
