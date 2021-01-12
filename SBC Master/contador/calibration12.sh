#!/bin/bash

/home/orbbec/contador/calibration_master 1 2&
sshpass -p " " ssh orbbec@192.168.10.1 /home/orbbec/contador/calibration_slave&
sshpass -p " " ssh orbbec@192.168.10.2 /home/orbbec/contador/calibration_slave&
