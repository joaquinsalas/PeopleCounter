#!/bin/bash

echo " " | sudo -S route del -net 0.0.0.0 netmask 0.0.0.0 gw 192.168.0.1 dev wlan0
echo " " | sudo -S route add -net 0.0.0.0 netmask 0.0.0.0 gw 128.1.1.250 dev eth1
