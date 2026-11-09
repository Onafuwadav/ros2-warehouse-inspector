cmake_minimum_required(VERSION 3.8)
project(inspector_bringup)

find_package(ament_cmake REQUIRED)

# installs files `ros2 launch`
foreach(_dir launch worlds models config maps rviz urdf)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
    install(DIRECTORY ${_dir} DESTINATION share/${PROJECT_NAME})
  endif()
endforeach()

# source install/setup.bash adds models folder to path so Gazebo can find warehouse models
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")
  ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
