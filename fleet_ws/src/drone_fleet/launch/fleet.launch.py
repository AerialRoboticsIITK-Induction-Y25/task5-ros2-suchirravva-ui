from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_alpha',
            parameters=[
                {'drone_name': 'Alpha'},
                {'initial_battery': 100.0},
                {'mission_name': 'Alpha_Patrol'}
            ]
        ),
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_beta',
            parameters=[
                {'drone_name': 'Beta'},
                {'initial_battery': 60.0},
                {'mission_name': 'Beta_Mapping'}
            ]
        ),
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_gamma',
            parameters=[
                {'drone_name': 'Gamma'},
                {'initial_battery': 35.0}, 
                {'mission_name': 'Gamma_Scouting'}
            ]
        ),
        Node(
            package='drone_fleet',
            executable='fleet_manager',
            name='fleet_manager_node',
            output='screen'
        ),
        Node(
            package='drone_fleet',
            executable='health_monitor',
            name='health_monitor_node',
            output='screen'
        )
    ])