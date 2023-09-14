#!/bin/bash
echo "running mavic pro"
python3 client.py -f flightLogs/FIT_mavic_pro_car.txt -t FIT_MAVIC_PRO_CAR_720 -s 121212 &
echo "running mavic dual"
python3 client.py -f flightLogs/FIT_mavic2_dual.txt -t FIT_MAVIC_DUAL -s 343434 &
echo "running mavic pro speed"
python3 client.py -f flightLogs/FIT_mavic_pro_speed.txt -t FIT_MAVIC_PRO_SPEED_720 -s 222323 &

