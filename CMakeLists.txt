cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

project(frame-streamer VERSION 0.1)

find_package(OpenCV REQUIRED)

set(INCLUDE_DIRECTORIES
	${OpenCV_INCLUDE_DIRS}
	include
)

add_library(${PROJECT_NAME} SHARED
    src/udpinterface.cpp
    src/framesender.cpp
    src/framereceiver.cpp
    src/utils.cpp
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
)

target_link_libraries(server PUBLIC
    ${OpenCV_LIBS}
)

add_executable(client
    src/client.cpp
)

target_include_directories(client PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(client PRIVATE
    ${PROJECT_NAME}
    camera-capture
)

target_link_libraries(client PUBLIC
    ${OpenCV_LIBS}
)

target_compile_options(client PUBLIC -fconcepts)

add_compile_options(-Wall -Wextra -pedantic  -o)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_custom_target(format
#     COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -iname \"*.cpp\" -o -iname \"*.hpp\" |xargs clang-tidy -format-style=file -p ${CMAKE_BINARY_DIR} -fix"
#     COMMAND bash -c "find ${CMAKE_SOURCE_DIR} ${CMAKE_INCLUDE_DIR} -iname \"*.cpp\" -o -iname \"*.hpp\"| xargs clang-format --style=file -i"
#     VERBATIM
# )

