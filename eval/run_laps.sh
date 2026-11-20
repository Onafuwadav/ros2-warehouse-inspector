#!/usr/bin/env bash

# Record N laps as eval/runs/lap01..lapNN, sim, Nav2 and mission node required to be already running
set -u
N=${1:-10}
for i in $(seq -w 1 "$N"); do
  echo "=== lap$i ==="
  python3 eval/record_run.py --run-id "lap$i" &
  REC=$!
  sleep 3                                 
  ros2 action send_goal /inspect_shelves inspector_interfaces/action/InspectShelves "{shelves: [], laps: 1}" || echo "lap$i: goal failed"           
  kill -INT "$REC"                         
  wait "$REC"
done