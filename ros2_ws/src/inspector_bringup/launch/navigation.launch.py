"""
Navigate warehouse with Nav2 with AMCL localisation

Run Command:
  ros2 launch inspector_bringup navigation.launch.py
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    bringup_dir = get_package_share_directory('inspector_bringup')
    nav2_dir = get_package_share_directory('nav2_bringup')

    sim = LaunchConfiguration('sim')
    rviz = LaunchConfiguration('rviz')

    args = [
        DeclareLaunchArgument('sim', default_value='true'),
        DeclareLaunchArgument('headless', default_value='false'),
        DeclareLaunchArgument('rviz', default_value='true'),
        DeclareLaunchArgument('slam', default_value='False',
                              description='True = map live with slam_toolbox instead of AMCL'),
        DeclareLaunchArgument(
            'map', default_value=os.path.join(bringup_dir, 'maps', 'warehouse.yaml'),
            description='Map yaml saved in M2'),
        DeclareLaunchArgument(
            'params_file',
            default_value=os.path.join(bringup_dir, 'config', 'nav2_params.yaml')),
    ]

    sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(bringup_dir, 'launch', 'warehouse.launch.py')),
        launch_arguments={'headless': LaunchConfiguration('headless'),
                          'rviz': 'false'}.items(),
        condition=IfCondition(sim),
    )

    nav2 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(nav2_dir, 'launch', 'bringup_launch.py')),
        launch_arguments={
            'slam': LaunchConfiguration('slam'),
            'map': LaunchConfiguration('map'),
            'params_file': LaunchConfiguration('params_file'),
            'use_sim_time': 'true',
            # autostart brings lifecycle nodes to active
            'autostart': 'true',
            'use_composition': 'False',
        }.items(),
    )

    # Nav2's own RViz layout (2D Pose Estimate, Nav2 Goal, costmaps, path),
    # started directly rather than through nav2_bringup's rviz_launch.py.
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', os.path.join(nav2_dir, 'rviz', 'nav2_default_view.rviz')],
        parameters=[{'use_sim_time': True}],
        condition=IfCondition(rviz),
        output='screen',
    )

    return LaunchDescription(args + [GroupAction([sim_launch]), nav2, rviz_node])
