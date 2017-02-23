# Project definition
cmake_minimum_required(VERSION 3.1)
project(KontrollerService VERSION 0.0.0 LANGUAGES CXX)

# Directories
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/binaries")

# Source files
set(SOURCES)
list(APPEND SOURCES
   "${SRC_DIR}/KontrollerService.h"
   "${SRC_DIR}/KontrollerService.cpp"
   "${SRC_DIR}/main.cpp"
)

# Target definitions
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
   PRIVATE "${SRC_DIR}"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED ON
)

# Libraries
set(KONTROLLER_SOCK_DIR "${LIB_DIR}/KontrollerSock")
add_subdirectory("${KONTROLLER_SOCK_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:KontrollerServer,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${PROJECT_NAME} "KontrollerServer")

# Install
install(TARGETS ${PROJECT_NAME}
   DESTINATION "${BIN_DIR}"
   PERMISSIONS OWNER_EXECUTE
   CONFIGURATIONS Release
)
