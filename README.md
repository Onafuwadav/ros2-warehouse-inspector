<div align="center">
  <img src="docs/images/warehouse_overview.png" alt="" width="420" height="auto" />
  <h1>ROS 2 Warehouse Inspector</h1>
  <p>
    Simulated warehouse inspection robot automating inventory audits, built with ROS 2 Jazzy, Nav2 and Gazebo.
  </p>
<p>
  <a href="LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT" />
  </a>
  <!-- ROS build and test
  <a href="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/ros.yml">
    <img src="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/ros.yml/badge.svg" alt="ROS" />
  </a>
  -->
  <!-- Backend tests
  <a href="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/backend.yml">
    <img src="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/backend.yml/badge.svg" alt="Backend" />
  </a>
  -->
  <!-- Dashboard build 
  <a href="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/dashboard.yml">
    <img src="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/dashboard.yml/badge.svg" alt="Dashboard" />
  </a>
  -->
  <!-- Docker 
  <a href="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/docker.yml">
    <img src="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/docker.yml/badge.svg" alt="Docker" />
  </a>
  -->
  <!-- Sim regression CI
  <a href="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/regression.yml">
    <img src="https://github.com/Onafuwadav/ros2-warehouse-inspector/actions/workflows/regression.yml/badge.svg" alt="Sim regression" />
  </a>
  -->
</p>

<!-- Live link
<h4>
    <a href="https://onafuwadav.github.io/ros2-warehouse-inspector/">Live dashboard demo</a>
  <span> · </span>
    <a href="docs/images/demo.mp4">Demo video</a>
</h4>
-->
</div>

## About The Project
Shelf audits done by hand are slow making them rare and causing inventory records to become inaccurate drift between them. This robot audits shelves on patrols and checks their markers. Planned next: reporting to a live web dashboard, and a grounded AI assistant for querying inspection data to speed up fault diagnosis


<details>
  <summary><b>System Architecture</b></summary>
  
Planned architecture. See the Roadmap for what is built so far

```mermaid
flowchart LR
  subgraph Robot["Simulated Robot"]
    sim[Warehouse sim] --> nav[SLAM, EKF, AMCL, Nav2]
    sim --> per[ArUco + focus]
    nav --> mis[Mission node]
    per --> mis
    mis --> br[Bridge node]
  end
  br -->|WebSocket + REST API| api[FastAPI]
  api --> db[(PostgreSQL)]
  api <-->|WebSocket| ui[Dashboard]
  ui -->|Questions| ast[LLM assistant]
  ast -->|Read only tools| api
  ui -->|Click map| api
  api -->|Nav goal| br
```

</details>

<details>
  <summary>Design Notes</summary>
  
- Parameter speed limits match real TurtleBot3 Waffle specifications (0.26 m/s, 1.82 rad/s)
- Robot footprint (0.22 m radius) matches the real TurtleBot3 Waffle
- 
</details>

<details>
  <summary>Screenshot</summary>
  
<div align="center">
  <img src="docs/images/patrol.png" alt="Robot patrolling the warehouse in Gazebo and RViz" width="700" />
</div>

</details>


### Tech Stack
[![ROS 2][ros-badge]][ros-url] [![Gazebo][gazebo-badge]][gazebo-url] [![Nav2][nav2-badge]][nav2-url] [![C++][cpp-badge]][cpp-url] [![Python][py-badge]][py-url] [![CMake][cmake-badge]][cmake-url] [![GoogleTest][gtest-badge]][gtest-url]
<!-- M4: [![OpenCV][opencv-badge]][opencv-url] -->
<!-- M6: [![FastAPI][fastapi-badge]][fastapi-url] [![PostgreSQL][postgres-badge]][postgres-url] [![SQLAlchemy][sqlalchemy-badge]][sqlalchemy-url] [![pytest][pytest-badge]][pytest-url] -->
<!-- M7: [![TypeScript][ts-badge]][ts-url] [![React][react-badge]][react-url] -->
<!-- M8: [![Docker][docker-badge]][docker-url] [![GitHub Actions][actions-badge]][actions-url] -->


## Getting Started
 
### Prerequisites
- Ubuntu 24.04 with [ROS 2 Jazzy](https://docs.ros.org/en/jazzy/Installation.html) (desktop) and Gazebo Harmonic
- Nav2, slam_toolbox and the TurtleBot3 simulation packages:

  ```bash
  sudo apt install ros-jazzy-navigation2 ros-jazzy-nav2-bringup ros-jazzy-slam-toolbox \
    ros-jazzy-nav2-minimal-tb3-sim
  ```
<!-- M6 onwards:
- Docker Engine with the Compose plugin
- Node.js 20+ and Python 3.12 to run backend or dashboard outside of Docker
-->

<!-- Installation
### Installation
```bash
git clone https://github.com/Onafuwadav/ros2-warehouse-inspector.git
cd ros2-warehouse-inspector/ros2_ws
rosdep install --from-paths src --ignore-src -y
colcon build
source install/setup.bash
```
-->


<!-- Run Locally
### Run Locally
| Command | What it does |
|---|---|
| `ros2 launch inspector_bringup navigation.launch.py` | World, AMCL, Nav2 and RViz on the saved map |
| `ros2 launch inspector_bringup inspection.launch.py` | Mission node (later also camera, ArUco, focus and bridge) |
| `docker compose up --build` | Backend, database and dashboard (from M6) |
| `colcon test && colcon test-result --verbose` | Unit tests and ROS linters |
-->


<!-- Usage
## Usage
### Run a patrol
```bash
ros2 action send_goal --feedback /inspect_shelves \
  inspector_interfaces/action/InspectShelves "{shelves: [shelf_B1, shelf_A1], laps: 1}"
```
An empty `shelves: []` visits every shelf. The feedback shows the state, the current shelf and the distance left. The result lists one entry per marker with its status, dwell time and the robot's pose.
-->

<!-- 
## Results
-->

## Roadmap
 
- [x] Warehouse, robot spawn and teleoperation
- [x] SLAM map, Localisation (AMCL) and Nav2 tuning
- [x] Autonomous patrol with a unit tested mission state machine
- [ ] Navigation success, recorded time to goal over 10 laps and ROS CI
- [ ] Camera, OpenCV ArUco detection and image sharpness check
- [ ] EKF localisation and localisation error checked against ground truth
- [ ] FastAPI/PostgreSQL telemetry backend and live React dashboard
- [ ] Docker, CI/CD and simulation regression tests
- [ ] Grounded LLM assistant for run analysis

## Contact
 
David Onafuwa: [LinkedIn](https://www.linkedin.com/in/david-onafuwa) · [Donafuwa@outlook.com](mailto:Donafuwa@outlook.com)

## Acknowledgments
- [TurtleBot3](https://github.com/ROBOTIS-GIT/turtlebot3) robot model
- [Nav2 minimal TurtleBot3 simulation](https://github.com/ros-navigation/nav2_minimal_turtlebot_simulation)
- [Nav2](https://github.com/ros-navigation/navigation2/blob/jazzy/nav2_bringup/params/nav2_params.yaml) default bringup parameters
- [Steve Macenski's](https://github.com/SteveMacenski/slam_toolbox/blob/jazzy/config/mapper_params_online_async.yaml) slam_toolbox config
- Badges from [shields.io](https://shields.io/)
<!-- 
M4
- ArUco detection and pose estimation: [OpenCV aruco module](https://docs.opencv.org/4.x/d5/dae/tutorial_aruco_detection.html).
- Tenengrad focus metric: E. Krotkov, *Focusing*, IJCV 1987; ported from my [autofocus](https://github.com/Onafuwadav/autofocus) project.

M5 
- EKF: [robot_localization](https://github.com/cra-ros-pkg/robot_localization) by Tom Moore and contributors.

M6-M7
- [FastAPI](https://fastapi.tiangolo.com/), [SQLAlchemy](https://www.sqlalchemy.org/) and [Alembic](https://alembic.sqlalchemy.org/) for the backend.
- [React](https://react.dev/) and [Vite](https://vite.dev/) for the dashboard.

M8
- Hosting: [Render](https://render.com/) (API), [Neon](https://neon.tech/) (Postgres), [GitHub Pages](https://pages.github.com/) (dashboard).

Part D
- Local model serving: [Ollama](https://ollama.com/).

Part F
- Recording and replay: [rosbag2](https://github.com/ros2/rosbag2)
-->

<!-- Tech Stack Badges -->
[ros-badge]: https://img.shields.io/badge/ROS_2-Jazzy-22314E?style=for-the-badge&logo=ros&logoColor=white
[ros-url]: https://docs.ros.org/en/jazzy/
[gazebo-badge]: https://img.shields.io/badge/Gazebo-Harmonic-F58113?style=for-the-badge
[gazebo-url]: https://gazebosim.org/
[nav2-badge]: https://img.shields.io/badge/Nav2-2E3440?style=for-the-badge
[nav2-url]: https://docs.nav2.org/
[cpp-badge]: https://img.shields.io/badge/C%2B%2B17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white
[cpp-url]: https://isocpp.org/
[py-badge]: https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white
[py-url]: https://www.python.org/
[cmake-badge]: https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white
[cmake-url]: https://cmake.org/
[gtest-badge]: https://img.shields.io/badge/GoogleTest-4285F4?style=for-the-badge&logo=google&logoColor=white
[gtest-url]: https://github.com/google/googletest
[opencv-badge]: https://img.shields.io/badge/OpenCV-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white
[opencv-url]: https://opencv.org/
[fastapi-badge]: https://img.shields.io/badge/FastAPI-009688?style=for-the-badge&logo=fastapi&logoColor=white
[fastapi-url]: https://fastapi.tiangolo.com/
[postgres-badge]: https://img.shields.io/badge/PostgreSQL-4169E1?style=for-the-badge&logo=postgresql&logoColor=white
[postgres-url]: https://www.postgresql.org/
[sqlalchemy-badge]: https://img.shields.io/badge/SQLAlchemy-D71F00?style=for-the-badge&logo=sqlalchemy&logoColor=white
[sqlalchemy-url]: https://www.sqlalchemy.org/
[pytest-badge]: https://img.shields.io/badge/pytest-0A9EDC?style=for-the-badge&logo=pytest&logoColor=white
[pytest-url]: https://docs.pytest.org/
[ts-badge]: https://img.shields.io/badge/TypeScript-3178C6?style=for-the-badge&logo=typescript&logoColor=white
[ts-url]: https://www.typescriptlang.org/
[react-badge]: https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB
[react-url]: https://react.dev/
[docker-badge]: https://img.shields.io/badge/Docker-2496ED?style=for-the-badge&logo=docker&logoColor=white
[docker-url]: https://www.docker.com/
[actions-badge]: https://img.shields.io/badge/GitHub_Actions-2088FF?style=for-the-badge&logo=githubactions&logoColor=white
[actions-url]: https://github.com/features/actions
