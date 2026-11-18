cmake_minimum_required(VERSION 3.8)
project(inspector_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MarkerDetection.msg"
  "msg/MarkerDetectionArray.msg"
  "msg/FocusScore.msg"
  "msg/InspectionResult.msg"
  "msg/RunEvent.msg"
  "srv/GetInspectionReport.srv"
  "srv/SetFocusThreshold.srv"
  "action/InspectShelves.action"
  DEPENDENCIES std_msgs geometry_msgs
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_dependencies(rosidl_default_runtime)
ament_package()
