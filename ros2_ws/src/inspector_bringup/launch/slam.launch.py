"""
Build map of warehouse while driving
- Reads scan and odom -> base TF and publishes map and map -> odom TF

Run command:
  ros2 launch inspector_bringup slam.launch.py

Save map:
  ros2 run nav2_map_server map_saver_cli -f src/inspector_bringup/maps/warehouse
"""


import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    bringup_dir = get_package_share_directory('inspector_bringup')
    slam_dir = get_package_share_directory('slam_toolbox')

    sim = LaunchConfiguration('sim')
    headless = LaunchConfiguration('headless')
    slam_params = LaunchConfiguration('slam_params_file')

    args = [
        DeclareLaunchArgument('sim', default_value='true',
                              description='Also start Gazebo and the robot'),
        DeclareLaunchArgument('headless', default_value='false'),
        DeclareLaunchArgument('rviz', default_value='true'),
        DeclareLaunchArgument(
            'slam_params_file',
            default_value=os.path.join(bringup_dir, 'config', 'slam_toolbox_params.yaml')),
    ]

    sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(bringup_dir, 'launch', 'warehouse.launch.py')),
        launch_arguments={'headless': headless,
                          'rviz': LaunchConfiguration('rviz')}.items(),
        condition=IfCondition(sim),
    )


    # include slam_toolbox launch file to handle lifecycle
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_dir, 'launch', 'online_async_launch.py')),
        launch_arguments={'use_sim_time': 'true',
                          'slam_params_file': slam_params}.items(),
    )

    return LaunchDescription(args + [sim_launch, slam_launch])

