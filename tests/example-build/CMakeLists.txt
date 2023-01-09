cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(farshow-example-build)

find_package(OpenCV REQUIRED)
find_package(farshow REQUIRED)

include_directories(
    ../../third-party/include/
    ${OpenCV_INCLUDE_DIRS}
    ${farshow_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}-app
    app-example.cpp
)
target_link_libraries(${PROJECT_NAME}-app PUBLIC
    farshow-connection
    ${OpenCV_LIBS}
)

add_compile_options(-Wall -Wextra -pedantic  -o)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
