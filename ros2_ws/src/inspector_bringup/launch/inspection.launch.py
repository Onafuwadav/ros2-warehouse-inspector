# Starts inspection nodes

# Usage (with navigation.launch.py already running):
#   ros2 launch inspector_bringup inspection.launch.py

# Then start a mission:
#   ros2 action send_goal --feedback /inspect_shelves \
#     inspector_interfaces/action/InspectShelves "{shelves: [], laps: 1}"

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    bringup_dir = get_package_share_directory('inspector_bringup')

    sim_time = {'use_sim_time': True}

    mission = Node(
        package='inspector_mission', executable='mission_node',
        name='mission_node', output='screen',
        parameters=[sim_time, {
            'waypoints_file': os.path.join(bringup_dir, 'config', 'inspection_waypoints.yaml'),
        }],
    )

    return LaunchDescription(
        [mission]
    )
