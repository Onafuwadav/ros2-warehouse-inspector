#!/usr/bin/env bash

# Record N laps as eval/runs/lap01..lapNN, sim, Nav2 and mission node required to be already running
set -u
N=${1:-10}
PREFIX=${2:-lap}
for i in $(seq -w 1 "$N"); do
  echo "=== ${PREFIX}${i} ==="
  python3 eval/record_run.py --run-id "${PREFIX}${i}" &
  REC=$!
  sleep 3                                 
  ros2 action send_goal /inspect_shelves inspector_interfaces/action/InspectShelves "{shelves: [], laps: 1}" || echo "${PREFIX}${i}: goal failed"           
  kill -INT "$REC"
  for _ in 1 2 3 4 5; do kill -0 "$REC" 2>/dev/null || break; sleep 1; done
  kill -KILL "$REC" 2>/dev/null || true
  wait "$REC" 2>/dev/null || true

done