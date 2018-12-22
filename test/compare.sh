#!/bin/sh

sysctl -ta>> sysctl.txt
./nsysctl -ta  >> nsysctl.txt

diff sysctl.txt nsysctl.txt | diffstat -m
#diff -y -W 120 sysctl.txt nsysctl.txt 
#rm sysctl.txt nsysctl.txt
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt
