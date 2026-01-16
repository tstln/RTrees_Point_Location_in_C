cmake_minimum_required(VERSION 3.10)
project(RTreeApp C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/RTree_from_superliminal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files for the RTree library
file(GLOB RTREE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/RTree_from_superliminal/*.c")

# Remove Test.c (has main)
list(REMOVE_ITEM RTREE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/RTree_from_superliminal/Test.c")

# Remove Split_l.c (Linear split) to avoid conflict with Split_q.c (Quadratic split)
# We choose Quadratic by default.
list(REMOVE_ITEM RTREE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/RTree_from_superliminal/Split_l.c")

# Source files for our application
file(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Create executable
add_executable(RTreeRUN ${APP_SOURCES} ${RTREE_SOURCES})

# Link math library
target_link_libraries(RTreeRUN m)
