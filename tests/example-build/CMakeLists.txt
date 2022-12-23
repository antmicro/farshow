cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(farshow-example-build)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(farshow REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(
    ${farshow_INCLUDE_DIRS}
    ../../third-party/
    ../../third-party/include
    ../../third-party/imgui
)

add_executable(${PROJECT_NAME}-server
    app-example.cpp
)
target_include_directories(
    ${PROJECT_NAME}-server PUBLIC ${INCLUDE_DIRECTORIES}
)
target_link_libraries(${PROJECT_NAME}-server PUBLIC
    farshow-connection
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARY}
    GLEW
    glfw
)

add_executable(${PROJECT_NAME}-client
    farshow-client.cpp
)
target_include_directories(
    ${PROJECT_NAME}-client PUBLIC ${INCLUDE_DIRECTORIES}
)
target_link_libraries(${PROJECT_NAME}-client PUBLIC
    farshow-client
    DearImGui
)

add_compile_options(-Wall -Wextra -pedantic  -o)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
