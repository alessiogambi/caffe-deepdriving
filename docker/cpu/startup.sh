#!/bin/bash
# Inspired by: http://blog.fx.lv/2017/08/running-gui-apps-in-docker-containers-using-vnc/

# Create a virtual display, start VNC
echo "XVFB"
Xvfb :1 -screen 0 800x600x16 &

echo "Setting up VNC" # Note that the password is HARDCODED !
mkdir ~/.vnc
x11vnc -storepasswd 1234 ~/.vnc/passwd

echo "Starting VNC"
/usr/bin/x11vnc -display :1.0 -usepw -forever -rfbport 5901 &

echo "Exporting Display"
export DISPLAY=:1.0

# Export DISPLAY variable from VNC server. Values are HARDCODED
export IN_PIPE="/pipes/in"
export OUT_PIPE="/pipes/out"

echo "Running TORCS Controller"
./torcs_run.sh 1
