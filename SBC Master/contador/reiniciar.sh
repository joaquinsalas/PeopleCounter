#!/bin/bash

sshpass -p " " ssh orbbec@192.168.10.1 /home/orbbec/contador/slave& 
echo slave1 ok
sshpass -p " " ssh orbbec@192.168.10.2 /home/orbbec/contador/slave& 
echo slave2 ok
sshpass -p " " ssh orbbec@192.168.10.3 /home/orbbec/contador/slave& 
echo slave3 ok
#sshpass -p " " ssh orbbec@192.168.10.4 /home/orbbec/contador/slave& 
#echo slave4 ok
/home/orbbec/contador/master&
echo master ok
