cmake_minimum_required(VERSION 3.29)
project(rasterizer)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# Fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Include GLM and your own include folder
include_directories(${glm_SOURCE_DIR} include)

add_executable(rasterizer main.cpp)
