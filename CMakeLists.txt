cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
set(DRAPIDJSON_BUILD_TESTS OFF)

project(farshow VERSION 0.1)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    include
    third-party/include/
    third-party/
    third-party/imgui
    ${OPENGL_INCLUDE_DIRS}
)

add_library(DearImGui SHARED
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_widgets.cpp
    third-party/imgui/imgui_tables.cpp
    third-party/imgui/imgui_demo.cpp
    third-party/imgui/backends/imgui_impl_glfw.cpp
    third-party/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(${PROJECT_NAME}-connection SHARED
    src/udpinterface.cpp
    src/framesender.cpp
    src/framereceiver.cpp
)

set_target_properties(${PROJECT_NAME}-connection PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

add_executable(${PROJECT_NAME}-server-example
    src/app-example.cpp
)

target_link_libraries(${PROJECT_NAME}-server-example PRIVATE
    ${PROJECT_NAME}-connection
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARY}
    GLEW
    glfw
)

add_library(${PROJECT_NAME}-client
    src/client.cpp
)

target_link_libraries(${PROJECT_NAME}-client PUBLIC
    ${PROJECT_NAME}-connection
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARY}
    GLEW
    glfw
    DearImGui
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_options(${PROJECT_NAME}-client PUBLIC -fconcepts)

add_executable(${PROJECT_NAME}
    src/farshow-client.cpp
)

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}-client
)

add_compile_options(-Wall -Wextra -pedantic  -o)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
