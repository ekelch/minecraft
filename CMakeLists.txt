cmake_minimum_required(VERSION 3.13.1)
project(minecraft)

# ------- Set Vars ------- #

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMAKE")
set(SOURCE_FILES    src/main.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_DEBUG_POSTFIX d)
add_library(tutorial_compiler_flags INTERFACE)

# ------- End Set Vars --- #

# ------- Finds ---------- #

find_package(SDL3 REQUIRED COMPONENTS SDL3)
SET(GLM_BINARY_DIR "/Users/evankelch/VulkanSDK/1.3.290.0/macOS/include/glm")
FIND_PACKAGE(Vulkan)

ADD_EXECUTABLE(minecraft ${SOURCE_FILES} ${Vulkan_INCLUDE_DIRS})
SET_TARGET_PROPERTIES(minecraft PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

file(COPY resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# ------- End Finds ------ #

# ------- Inc & Link ---- #

INCLUDE_DIRECTORIES(${SDL3_STATIC_LIBRARIES} ${Vulkan_INCLUDE_DIRS} ${GLM_BINARY_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} SDL3::SDL3 ${Vulkan_LIBRARIES})

# ------- End ----------- #
