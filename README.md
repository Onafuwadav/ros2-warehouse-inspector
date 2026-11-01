<!-- PROJECT SHIELDS -->
[![MIT License][license-shield]][license-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <!-- Add a logo later: <img src="docs/images/logo.png" alt="Logo" width="80" height="80"> -->

  <h3 align="center">ROS 2 Warehouse Inspector</h3>

  <p align="center">
    A simulated mobile warehouse inspection robot built with ROS 2, Nav2 and OpenCV that patrols shelves and checks markers, reporting findings to a live web dashboard, with a grounded AI assistant for querying inspection data to speed up fault diagnosis.
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#author">Author</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

<!-- Add a screenshot of the warehouse world here: [![Screenshot][product-screenshot]](#) -->

> **Everything here is simulated** (Gazebo). No real robot hardware is used, and every result in this repo comes from a script you can re-run.

| Part | What | Status |
|---|---|---|
| A | Autonomous inspection robot (SLAM, Nav2, ArUco, focus check) | In progress |
| B | Telemetry backend (FastAPI + PostgreSQL) and React dashboard | Planned |
| C | Docker, CI and a live demo link | Planned |
| D | Grounded AI assistant for inspection data queries | Planned |



### Built With

* [![ROS 2][ROS2-badge]][ROS2-url]
* [![Gazebo][Gazebo-badge]][Gazebo-url]
* [![C++][Cpp-badge]][Cpp-url]
* [![Python][Python-badge]][Python-url]

<!-- Add as each part lands: FastAPI, PostgreSQL, React, TypeScript, Docker, GitHub Actions -->



<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

### Installation



<!-- USAGE EXAMPLES -->
## Usage



<!-- ROADMAP -->
## Roadmap

**Part A: inspection robot**
- [ ] **M1** Warehouse world: a Gazebo warehouse with ArUco-tagged shelves, the TurtleBot3 spawned in it and driven by keyboard, with camera, lidar and TF shown in RViz
- [ ] **M2** SLAM map: drive the robot around to build a map of the warehouse, save it and reload it
- [ ] **M3** Autonomous patrol: a C++ mission node sends the robot to every shelf with Nav2, with a script measuring success rate and time to goal
- [ ] **M4** Perception: detect shelf markers with OpenCV, score image sharpness with Tenengrad, and slow down and re-check when the camera is blurred
- [ ] **M5** Sensor fusion: an EKF fuses noisy wheel odometry and IMU, and its error is measured against ground truth

**Part B: backend and dashboard**
- [ ] **M6** Backend: FastAPI and PostgreSQL store runs, poses, detections and events, with login and tests
- [ ] **M7** Live dashboard: the robot streams telemetry over WebSockets, and the dashboard shows it on a live map where you click to send goals

**Part C: deployment**
- [ ] **M8** CI/CD and live demo: every push is built and tested, and the dashboard and API are deployed to a public link

**Part D: AI assistant**
- [ ] **M9** Assistant backend: the model answers questions only by calling tools that query the robot's data, and an evaluation script scores it on 20 test questions
- [ ] **M10** Ask page: type a question, watch the agent move to the shelves it is checking, and get an answer that links to the runs and photos behind it

**Wrap-up**
- [ ] **M11** Architecture write-up, demo video and evaluation results



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.



<!-- AUTHOR -->
## Author

David Onafuwa



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)



<!-- MARKDOWN LINKS & IMAGES -->
[license-shield]: https://img.shields.io/github/license/Onafuwadav/ros2-warehouse-inspector.svg?style=for-the-badge
[license-url]: https://github.com/Onafuwadav/ros2-warehouse-inspector/blob/main/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/YOUR_LINKEDIN
[product-screenshot]: docs/images/screenshot.png
[ROS2-badge]: https://img.shields.io/badge/ROS_2-Jazzy-22314E?style=for-the-badge&logo=ros&logoColor=white
[ROS2-url]: https://docs.ros.org/en/jazzy/
[Gazebo-badge]: https://img.shields.io/badge/Gazebo-Harmonic-F58113?style=for-the-badge
[Gazebo-url]: https://gazebosim.org/
[Cpp-badge]: https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white
[Cpp-url]: https://isocpp.org/
[Python-badge]: https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white
[Python-url]: https://www.python.org/

