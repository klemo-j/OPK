"""
Spustí robot_node, lidar_node, map_node, game_node a RViz.
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
    rviz_config    = os.path.join(pkg_share, 'rviz',   'robot.rviz')

    config_arg = DeclareLaunchArgument(
        'config_file', default_value=default_config,
        description='Cesta k YAML konfigurácii prostredia.')

    use_rviz_arg = DeclareLaunchArgument(
        'use_rviz', default_value='true',
        description='Spustiť RViz?')

    config_file = LaunchConfiguration('config_file')

    robot_node = Node(
        package='robot', executable='robot_node', name='robot_node',
        parameters=[{'config_file': config_file}],
        output='screen',
    )

    lidar_node = Node(
        package='robot', executable='lidar_node', name='lidar_node',
        parameters=[{'config_file': config_file}],
        output='screen',
    )

    map_node = Node(
        package='robot', executable='map_node', name='map_node',
        parameters=[{'config_file': config_file}],
        output='screen',
    )

    game_node = Node(
        package='robot', executable='game_node', name='game_node',
        parameters=[{'config_file': config_file}],
        output='screen',
    )

    rviz = Node(
        package='rviz2', executable='rviz2', name='rviz2',
        arguments=['-d', rviz_config],
        output='screen',
    )

    return LaunchDescription([
        config_arg,
        use_rviz_arg,
        robot_node,
        lidar_node,
        map_node,
        game_node,
        rviz,
    ])