cmake_minimum_required(VERSION 3.15)

project(main VERSION 1.0)

FILE(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../ ${PROJECT_SOURCE_DIR}/../lib)
target_link_libraries(${PROJECT_NAME} PUBLIC d_lib)