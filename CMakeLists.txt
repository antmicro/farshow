cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(frame-streamer VERSION 0.1)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads)

# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(INCLUDE_DIRECTORIES
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

add_library(${PROJECT_NAME} SHARED
    src/udpinterface.cpp
    src/framesender.cpp
    src/framereceiver.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRECTORIES})

add_executable(server
    src/server.cpp
)

target_include_directories(server PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(server PRIVATE
    ${PROJECT_NAME}
    camera-capture
)

target_link_libraries(server PUBLIC
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARY}
    GLEW
    glfw
)

add_executable(client
    src/client.cpp
)

target_include_directories(client PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(client PRIVATE
    ${PROJECT_NAME}
    DearImGui
)

target_link_libraries(client PUBLIC
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARY}
    GLEW
    glfw
    ${CMAKE_THREAD_LIBS_INIT}
)

add_compile_options(-Wall -Wextra -pedantic  -o -g)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
