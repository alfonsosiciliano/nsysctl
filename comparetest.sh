#!/bin/sh

echo "-aT"
sysctl -aT >> sysctl.txt
./nsysctl -aT  >> nsysctl.txt
diff sysctl.txt nsysctl.txt | diffstat -m
rm sysctl.txt nsysctl.txt


echo "-aW"
sysctl -aW >> sysctl.txt
./nsysctl -aW  >> nsysctl.txt
#diff sysctl.txt nsysctl.txt | diffstat -m
#rm sysctl.txt nsysctl.txt

echo "-a"
#sysctl -a >> sysctl.txt
#./nsysctl -a  >> nsysctl.txt



#diff -y -W 120 sysctl.txt nsysctl.txt 
#rm sysctl.txt nsysctl.txt
meld sysctl.txt nsysctl.txt
rm sysctl.txt nsysctl.txt
