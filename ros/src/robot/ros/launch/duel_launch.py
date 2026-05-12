"""
Duel launch - 2 hraci na jednej klavesnici.
P1 (sipky)  -> namespace /p1
P2 (WASD)   -> namespace /p2

Teleop nie je sucastou launchu. Spustite ich rucne v dalsich 2 terminaloch:
  python3 ~/Desktop/OPK/ros/src/robot/ros/scripts/teleop_arrows.py
  python3 ~/Desktop/OPK/ros/src/robot/ros/scripts/teleop_wasd.py
"""
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('robot')

    default_config = os.path.join(pkg_share, 'config', 'environment.yaml')
    rviz_config    = os.path.join(pkg_share, 'rviz',   'duel.rviz')

    config_arg = DeclareLaunchArgument(
        'config_file', default_value=default_config,
        description='Cesta k YAML konfiguracii prostredia.')

    config_file = LaunchConfiguration('config_file')

    map_node = Node(
        package='robot', executable='map_node', name='map_node',
        parameters=[{
            'config_file': config_file,
            'world_frame': 'map',
        }],
        output='screen',
    )

    p1_robot = Node(
        package='robot', executable='robot_node',
        name='robot_node', namespace='p1',
        parameters=[{
            'config_file': config_file,
            'world_frame': 'map',
            'robot_frame': 'p1_base',
            'start_x': -7.0,
            'start_y':  6.0,
            'start_theta': 0.0,
            'game_state_topic': '/game/state',
        }],
        output='screen',
    )

    p1_lidar = Node(
        package='robot', executable='lidar_node',
        name='lidar_node', namespace='p1',
        parameters=[{
            'config_file': config_file,
            'laser_frame': 'p1_laser',
            'robot_frame': 'p1_base',
        }],
        output='screen',
    )

    p2_robot = Node(
        package='robot', executable='robot_node',
        name='robot_node', namespace='p2',
        parameters=[{
            'config_file': config_file,
            'world_frame': 'map',
            'robot_frame': 'p2_base',
            'start_x': -5.0,
            'start_y':  6.0,
            'start_theta': 3.14159,
            'game_state_topic': '/game/state',
        }],
        output='screen',
    )

    p2_lidar = Node(
        package='robot', executable='lidar_node',
        name='lidar_node', namespace='p2',
        parameters=[{
            'config_file': config_file,
            'laser_frame': 'p2_laser',
            'robot_frame': 'p2_base',
        }],
        output='screen',
    )

    game_node = Node(
        package='robot', executable='game_node', name='game_node',
        parameters=[{
            'config_file': config_file,
            'world_frame': 'map',
            'p1_pose_topic': '/p1/robot/pose',
            'p2_pose_topic': '/p2/robot/pose',
        }],
        output='screen',
    )

    rviz = Node(
        package='rviz2', executable='rviz2', name='rviz2',
        arguments=['-d', rviz_config],
        output='screen',
    )

    return LaunchDescription([
        config_arg,
        map_node,
        p1_robot, p1_lidar,
        p2_robot, p2_lidar,
        game_node,
        rviz,
    ])
