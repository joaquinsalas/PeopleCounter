#!/bin/bash

rm /home/orbbec/contador/test/images/im*.png
rm /home/orbbec/contador/test/eventos_test.txt
rm /home/orbbec/contador/test/tracklets_test.txt

sshpass -p " " ssh orbbec@192.168.10.1 /home/orbbec/contador/test/slave_test 200&
echo slave1 ok
sshpass -p " " ssh orbbec@192.168.10.2 /home/orbbec/contador/test/slave_test 200&
echo slave2 ok
sshpass -p " " ssh orbbec@192.168.10.3 /home/orbbec/contador/test/slave_test 200&
echo slave3 ok
/home/orbbec/contador/test/master_test 200&
echo master ok
