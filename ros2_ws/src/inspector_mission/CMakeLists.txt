cmake_minimum_required(VERSION 3.8)
project(inspector_mission)

# Same warning flags `ros2 pkg create` generates. Treat new warnings as bugs.
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(inspector_interfaces REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(tf2_ros REQUIRED)

find_package(std_msgs REQUIRED)

# Split that keeps the logic testable without ROS running:
#   mission_logic  plain C++ library: the state machine, waypoint parsing.
#                  gtest links against this directly.
#   mission_node   thin rclcpp wrapper: subscriptions, action client/server, timers.
add_library(mission_logic src/mission_logic.cpp)
target_include_directories(mission_logic PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(mission_logic yaml-cpp::yaml-cpp)

add_executable(mission_node src/mission_node.cpp)
target_link_libraries(mission_node
  mission_logic
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  tf2_ros::tf2_ros
  ${nav2_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${inspector_interfaces_TARGETS})

# Executables go in lib/<package> so `ros2 run inspector_mission mission_node` finds them.
install(TARGETS mission_logic mission_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  # cpplint also demands a copyright line; run it ourselves without that check.
  list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_cpplint)
  ament_lint_auto_find_test_dependencies()
  find_package(ament_cmake_cpplint REQUIRED)
  ament_cpplint(FILTERS "-legal/copyright")

  # Unit tests for the state machine. `colcon test` runs them.
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_mission_logic test/test_mission_logic.cpp)
  target_link_libraries(test_mission_logic mission_logic)
endif()

ament_package()
