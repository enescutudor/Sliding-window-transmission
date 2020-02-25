#!/bin/bash

SPEED=10
DELAY=10
LOSS=0
REORDER=0
CORRUPT=95
FILENAME=fileX

sudo rm recv_$FILENAME &>/dev/null

killall -9 link &>/dev/null
killall -9 recv &>/dev/null
killall -9 send &>/dev/null

./link_emulator/link speed=$SPEED delay=$DELAY loss=$LOSS corrupt=$CORRUPT reorder=$REORDER &>/dev/null &
sleep 1
./recv &
sleep 1

time ./send $FILENAME $SPEED $DELAY

echo "[SCRIPT] Finished transfer, check $FILENAME and recv_$FILENAME using diff"
