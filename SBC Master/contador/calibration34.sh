#!/bin/bash

/home/orbbec/contador/calibration_master 3 4&
sshpass -p " " ssh orbbec@192.168.10.3 /home/orbbec/contador/calibration_slave&
sshpass -p " " ssh orbbec@192.168.10.4 /home/orbbec/contador/calibration_slave&
