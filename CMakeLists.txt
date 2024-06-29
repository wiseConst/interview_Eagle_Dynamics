cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(PROJECT_NAME BallCollision)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

# SFML
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.x
)
FetchContent_GetProperties(SFML)
if(NOT SFML_POPULATED)
  FetchContent_Populate(SFML)
  add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME})

# Automatically group all sources into folders for MVS.
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "${CORE_DIR}/*.cpp" "${CORE_DIR}/*.h" "${CORE_DIR}/*.hpp")
set(ALL_FILES ${SRC_FILES})
foreach(FILE ${SRC_FILES})
    file(RELATIVE_PATH REL_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    get_filename_component(DIR "${REL_FILE}" DIRECTORY)
    string(REPLACE "/" "\\" GROUP "${DIR}")

    source_group("${GROUP}" FILES ${FILE})
endforeach()

add_executable(${PROJECT_NAME} ${ALL_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics)
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Source/>)

# For Windows users
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND_EXPAND_LISTS
)