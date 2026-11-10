<br />
<div align="center">
  <!-- Add logo: <img src="docs/images/logo.png" alt="Logo" width="" height=""> -->

  <h1 align="center">ROS 2 Warehouse Inspector</h1>

  <p align="center">
    Simulated ROS 2 warehouse inspection robot with AI assistant
  </p>
</div>


## About The Project
<!-- [Screenshot of warehouse world](link) -->
A simulated mobile warehouse inspection robot built with ROS 2 that patrols shelves, checks markers and reports findings to a live web dashboard, with a grounded AI assistant for querying inspection data to speed up fault diagnosis

## Design notes
- Parameter speed limits match real TurtleBot3 Waffle specifications (0.26 m/s, 1.82 rad/s)
- Robot footprint (0.22 m radius) matches the real TurtleBot3 Waffle

### Built With
[![ROS 2][ROS2-badge]][ROS2-url] [![Gazebo][Gazebo-badge]][Gazebo-url] [![C++][Cpp-badge]][Cpp-url] [![Python][Python-badge]][Python-url]

<!-- Add: FastAPI, PostgreSQL, React, TypeScript, Docker, GitHub Actions -->


## Getting Started

### Dependencies

### Installation

### Usage



## License
Distributed under the MIT License. See [LICENSE](https://github.com/Onafuwadav/ros2-warehouse-inspector/blob/main/LICENSE) for more information.


## Author
David Onafuwa


## Acknowledgments
- Used [Steve Macenski's](https://github.com/SteveMacenski/slam_toolbox/blob/jazzy/config/mapper_params_online_async.yaml) slam toolbox config as starting point 

- Used [nav2's](https://github.com/ros-navigation/navigation2/blob/jazzy/nav2_bringup/params/nav2_params.yaml) bringup params as starting point



<!-- MARKDOWN LINKS & IMAGES -->
[ROS2-badge]: https://img.shields.io/badge/ROS_2-Jazzy-22314E?style=for-the-badge&logo=ros&logoColor=white
[ROS2-url]: https://docs.ros.org/en/jazzy/

[Gazebo-badge]: https://img.shields.io/badge/Gazebo-Harmonic-F58113?style=for-the-badge
[Gazebo-url]: https://gazebosim.org/

[Cpp-badge]: https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white
[Cpp-url]: https://isocpp.org/

[Python-badge]: https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white
[Python-url]: https://www.python.org/