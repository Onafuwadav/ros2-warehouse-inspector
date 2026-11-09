"""
Start Gazebo and spawn a TurtleBot in it
- Gazebo loads warehouse file
- robot_state_publisher publishes fixd tansforms between parts
- Add TurtleBot model and starts ros2-Gazebo bridge

Run command:
  ros2 launch inspector_bringup warehouse.launch.py

To drive from second terminal:
  ros2 run teleop_twist_keyboard teleop_twist_keyboard
"""


import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    AppendEnvironmentVariable,
    DeclareLaunchArgument,
    IncludeLaunchDescription,
)
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node


def generate_launch_description():
    bringup_dir = get_package_share_directory("inspector_bringup")
    tb3_sim_dir = get_package_share_directory("nav2_minimal_tb3_sim")
    ros_gz_sim_dir = get_package_share_directory("ros_gz_sim")

    world = LaunchConfiguration("world")
    headless = LaunchConfiguration("headless")
    rviz = LaunchConfiguration("rviz")

    # Launch arguments 
    args = [
        DeclareLaunchArgument(
            "world",
            default_value=os.path.join(bringup_dir, "worlds", "warehouse.sdf"),
            description="Full path to worlds sdf file",
        ),
        DeclareLaunchArgument(
            "headless",
            default_value="false",
            description="true = simulation run on server only (no 3D window)",
        ),
        DeclareLaunchArgument("rviz", default_value="false", description="Open RViz"),

        # Spawn pose
        DeclareLaunchArgument("x_pose", default_value="-6.0"),
        DeclareLaunchArgument("y_pose", default_value="0.0"),
        DeclareLaunchArgument("yaw", default_value="0.0"),
    ]

    # Gazebo finds marker model by searching GZ_SIM_RESOURCE_PATH
    set_model_path = AppendEnvironmentVariable(
        "GZ_SIM_RESOURCE_PATH", os.path.join(bringup_dir, "models")
    )

    # Gazebo looks for nav2_minimal_tb3_sim folder on the same path
    set_tb3_mesh_path = AppendEnvironmentVariable(
        "GZ_SIM_RESOURCE_PATH", os.path.dirname(tb3_sim_dir)
    )

    #   -r                    start the simulation running (not paused)
    #   -s                    server only (no GUI)
    #   --headless-rendering  render camera images without display
    gz_args = PythonExpression(
        ["'-r -s --headless-rendering ' if '", headless, "' == 'true' else '-r '"]
    )
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim_dir, "launch", "gz_sim.launch.py")
        ),
        launch_arguments={
            "gz_args": [gz_args, world],
            "on_exit_shutdown": "true",
        }.items(),
    )

    # URDF describes robot Gazebo spawns but in format ROS tools understand.
    with open(os.path.join(tb3_sim_dir, "urdf", "turtlebot3_waffle.urdf")) as f:
        robot_description = f.read()
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        # use_sim_time: follow Gazebo clock so timestamps line up even when the sim runs slower
        parameters=[{"use_sim_time": True, "robot_description": robot_description}],
    )

    spawn_tb3 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(tb3_sim_dir, "launch", "spawn_tb3.launch.py")
        ),
        launch_arguments={
            "x_pose": LaunchConfiguration("x_pose"),
            "y_pose": LaunchConfiguration("y_pose"),
            "z_pose": "0.01",
            "yaw": LaunchConfiguration("yaw"),
        }.items(),
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(bringup_dir, "rviz", "warehouse.rviz")],
        parameters=[{"use_sim_time": True}],
        condition=IfCondition(rviz),
        output="screen",
    )

    return LaunchDescription(
        args
        + [
            set_model_path,
            set_tb3_mesh_path,
            gazebo,
            robot_state_publisher,
            spawn_tb3,
            rviz_node,
        ]
    )
