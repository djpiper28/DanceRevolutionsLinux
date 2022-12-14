cmake_minimum_required(VERSION 3.9)

project(
  dance-revolutions
  VERSION 0.1
  LANGUAGES C)

set(REPO_URL "https://github.com/DanceDanceRevolutionsLinux")

message(STATUS "Building ${CMAKE_PROJECT_NAME}")
message(STATUS "Git repo: ${REPO_URL}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build OS: ${CMAKE_SYSTEM_NAME}")

# Get the branch name
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Commit info: ${GIT_COMMIT_HASH}@${GIT_BRANCH}")

# Version info
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions(
  "-DVERSION=\"${CMAKE_PROJECT_VERSION} (${GIT_COMMIT_HASH}@${GIT_BRANCH})\"")
add_definitions("-DOS=\"${CMAKE_SYSTEM_NAME}\"")
add_definitions("-DREPO_URL=\"${REPO_URL}\"")
add_definitions("-DBUFFER_LENGTH=4096")
add_definitions("-DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")
add_definitions("-DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")

# __FILENAME__
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
add_definitions("-D__FILENAME__=(__FILE__ + SOURCE_PATH_SIZE)")
add_definitions("-DDEBUG")

set(COMPILER_FLAGS "-Wno-unused-parameter -Wall -Wextra -Wpedantic -Werror -g")
set(CMAKE_C_FLAGS "-Og ${COMPILER_FLAGS}")

# Setup doxygen
find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)

# Compile the library (static build and, unit tests)
set(LIB_SRC ./dance.h ./dance.c ./testing.h/logger.c ./testing.h/logger.c)

set(LIBS usb-1.0 pthread)

add_library(dance-revolutions ${LIB_SRC})
target_link_libraries(dance-revolutions ${LIBS})
doxygen_add_docs(dance-revolutions-docs ${LIB_SRC}
                 "Generate documentation pages")

add_executable(dance-revolutions-test ${LIB_SRC} ./main.c)
target_link_libraries(dance-revolutions-test ${LIBS})
