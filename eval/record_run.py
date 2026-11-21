# Record run to CSV files in eval/runs/<run_id>/

# Start after build but before a mission:

# For single run recording:
#   cd ~/Documents/ros2-warehouse-inspector && source ros2_ws/install/setup.bash
#   python3 eval/record_run.py --run-id lap1
#   (in another terminal) ros2 action send_goal --feedback /inspect_shelves inspector_interfaces/action/InspectShelves "{shelves: [], laps: 1}"
# 
# For multi-run recording
#   cd ~/Documents/ros2-warehouse-inspector && source ros2_ws/install/setup.bash
#   bash eval/run_laps.sh 10


import argparse
import csv
import datetime as dt
import math
import pathlib
import signal

import rclpy
from common import RUNS_DIR
from geometry_msgs.msg import PoseWithCovarianceStamped
from inspector_interfaces.msg import FocusScore, InspectionResult, MarkerDetectionArray
from nav_msgs.msg import Odometry
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.parameter import Parameter
from rclpy.signals import SignalHandlerOptions


def yaw(q):
    return math.atan2(2.0 * (q.w * q.z + q.x * q.y), 1.0 - 2.0 * (q.y * q.y + q.z * q.z))


def stamp(header):
    return header.stamp.sec + header.stamp.nanosec * 1e-9


class Recorder(Node):

    def __init__(self, out_dir: pathlib.Path):
        super().__init__('eval_recorder')
        self.set_parameters([Parameter('use_sim_time', Parameter.Type.BOOL, True)])
        out_dir.mkdir(parents=True, exist_ok=True)
        self._files = []
        self._writers = {}

        def writer(name, fields):
            f = open(out_dir / f'{name}.csv', 'w', newline='')
            self._files.append(f)
            w = csv.writer(f)
            w.writerow(fields)
            self._writers[name] = (w, f)

        writer('inspection_results',
               ['stamp', 'shelf', 'expected_id', 'observed_id', 'status', 'focus', 'dwell_s'])
        writer('focus', ['stamp', 'tenengrad', 'in_focus'])
        writer('detections', ['stamp', 'marker_id', 'distance_m', 'processing_ms'])
        writer('amcl_pose', ['stamp', 'x', 'y', 'yaw', 'var_x', 'var_y', 'var_yaw'])
        for name in ('ekf_odom', 'wheel_odom', 'ground_truth'):
            writer(name, ['stamp', 'x', 'y', 'yaw'])

        self.create_subscription(InspectionResult, '/inspection_results', self._on_result, 50)
        self.create_subscription(FocusScore, '/focus_score', self._on_focus, 50)
        self.create_subscription(MarkerDetectionArray, '/detections', self._on_det, 50)
        self.create_subscription(PoseWithCovarianceStamped, '/amcl_pose', self._on_amcl, 50)
        self.create_subscription(Odometry, '/odometry/filtered',
                                 lambda m: self._on_odom('ekf_odom', m), 50)
        self.create_subscription(Odometry, '/odom', lambda m: self._on_odom('wheel_odom', m), 50)
        self.get_logger().info(f'Recording to {out_dir}')

    def _row(self, name, row):
        w, f = self._writers[name]
        w.writerow(row)
        f.flush() 

    def _on_result(self, m):
        self._row('inspection_results', [stamp(m.header), m.shelf, m.expected_marker_id,
                                          m.observed_marker_id, m.status, m.focus_score,
                                          m.dwell_time_s])

    def _on_focus(self, m):
        self._row('focus', [stamp(m.header), m.tenengrad, int(m.in_focus)])

    def _on_det(self, m):
        for d in m.detections:
            self._row('detections', [stamp(m.header), d.marker_id, d.distance_m,
                                     m.processing_time_ms])

    def _on_amcl(self, m):
        p, c = m.pose.pose, m.pose.covariance
        self._row('amcl_pose', [stamp(m.header), p.position.x, p.position.y,
                                yaw(p.orientation), c[0], c[7], c[35]])

    def _on_odom(self, name, m):
        p = m.pose.pose
        self._row(name, [stamp(m.header), p.position.x, p.position.y, yaw(p.orientation)])

    def close(self):
        for f in self._files:
            f.close()


def main():
    signal.signal(signal.SIGINT, signal.default_int_handler)  

    parser = argparse.ArgumentParser(description='Record run to CSV files in eval/runs/<run_id>')
    parser.add_argument('--run-id', default=dt.datetime.now().strftime('%Y%m%d-%H%M%S'))
    args = parser.parse_args()

    rclpy.init(signal_handler_options=SignalHandlerOptions.NO)
    node = Recorder(RUNS_DIR / args.run_id)
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    finally:
        node.close()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
