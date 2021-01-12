#!/bin/bash

/home/orbbec/contador/calibration_master 2 3&
sshpass -p " " ssh orbbec@192.168.10.2 /home/orbbec/contador/calibration_slave&
sshpass -p " " ssh orbbec@192.168.10.3 /home/orbbec/contador/calibration_slave&
