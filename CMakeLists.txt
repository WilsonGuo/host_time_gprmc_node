cmake_minimum_required(VERSION 3.0.2)
project(host_time_gprmc_node)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)
catkin_package(
 CATKIN_DEPENDS roscpp rospy std_msgs
)
## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
 include
  ${catkin_INCLUDE_DIRS}
)



link_directories(
  ${catkin_LIB_DIRS}
)


add_executable(host_time_gprmc src/host_time_gprmc.cpp)
target_link_libraries(host_time_gprmc ${catkin_LIBRARIES})

