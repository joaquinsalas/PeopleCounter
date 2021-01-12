#!/bin/bash

sshpass -p " " ssh orbbec@192.168.10.1 $echo $SPACE | sudo -S reboot 
echo slave1 ok
sshpass -p " " ssh orbbec@192.168.10.2 $echo $SPACE | sudo -S reboot 
echo slave2 ok
sshpass -p " " ssh orbbec@192.168.10.3 $echo $SPACE | sudo -S reboot 
echo slave3 ok

$echo $SPACE | sudo -S reboot
echo master ok

