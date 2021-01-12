#!/bin/bash

sshpass -p " " ssh orbbec@192.168.10.1 pkill slave
echo slave1 ok
sshpass -p " " ssh orbbec@192.168.10.2 pkill slave 
echo slave2 ok
sshpass -p " " ssh orbbec@192.168.10.3 pkill slave 
echo slave3 ok
#sshpass -p " " ssh orbbec@192.168.10.4 pkill slave
#echo slave4 ok
pkill master
echo master ok
/home/orbbec/contador/clear_status
