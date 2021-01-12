#!/bin/bash

sshpass -p " " ssh orbbec@192.168.10.1 pkill slave_test& 
echo slave1 ok
sshpass -p " " ssh orbbec@192.168.10.2 pkill slave_test& 
echo slave2 ok
sshpass -p " " ssh orbbec@192.168.10.3 pkill slave_test& 
echo slave3 ok
pkill master_test&
echo master ok
