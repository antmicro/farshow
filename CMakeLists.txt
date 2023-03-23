cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
set(DRAPIDJSON_BUILD_TESTS OFF)

project(farshow VERSION 0.1)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads)
find_package(pybind11)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/include/>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui>
    $<INSTALL_INTERFACE:include>
)

add_library(${PROJECT_NAME}-imgui SHARED
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
    ${PROJECT_NAME}-imgui
    ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(${PROJECT_NAME}
    src/farshow-client.cpp
)

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}-client
)

add_compile_options(-Wall -Wextra -pedantic  -o)

configure_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY
)

install(
    TARGETS
    ${PROJECT_NAME}-connection
    ${PROJECT_NAME}-server-example
    ${PROJECT_NAME}-client
    ${PROJECT_NAME}
    ${PROJECT_NAME}-imgui
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
export(
    TARGETS ${PROJECT_NAME}-connection ${PROJECT_NAME}-client ${PROJECT_NAME}-imgui
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
)
install(
    EXPORT ${PROJECT_NAME}-targets DESTINATION lib/cmake/${PROJECT_NAME}
)
install(
    DIRECTORY include/farshow/ DESTINATION include/farshow/
)
install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

add_custom_target(format
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" |xargs clang-tidy -format-style=file -p ${CMAKE_BINARY_DIR} -fix"
    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" | xargs clang-format --style=file -i"
    VERBATIM
)

IF( pybind11_FOUND )
    add_subdirectory(third-party/cvnp)
    pybind11_add_module(python-farshow 
        src/python-bindings/main.cpp
        src/python-bindings/streamexception.cpp
        src/python-bindings/udpinterface.cpp
        src/udpinterface.cpp
        src/python-bindings/framesender.cpp
        src/framesender.cpp
        src/python-bindings/framereceiver.cpp
        src/framereceiver.cpp
    )
    set_target_properties(python-farshow PROPERTIES OUTPUT_NAME farshow)
    target_link_libraries(python-farshow PRIVATE 
        ${OpenCV_LIBS} 
        cvnp
    )
    file(COPY src/python-bindings/example.py
         DESTINATION .)
    install(TARGETS python-farshow
            LIBRARY DESTINATION lib
    )
ENDIF()
