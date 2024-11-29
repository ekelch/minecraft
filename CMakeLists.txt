cmake_minimum_required(VERSION 3.13.1)

Message("")
Message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
Message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
Message( STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND} )
Message( STATUS "CMAKE_VERSION: " ${CMAKE_VERSION} )
Message( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )
Message( STATUS "CMAKE_PROJECT_NAME: " ${CMAKE_PROJECT_NAME} )
Message( STATUS "CMAKE_FINDCMAKE: " ${CMAKE_SOURCE_DIR}/CMAKE )

project(minecraft)

# ------- Set Vars ------- #

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMAKE")
set(SOURCE_FILES    src/main.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

Message("")
Message( STATUS "SOURCE entry point : " ${SOURCE_FILES} )
Message("")

# ------- End Set Vars --- #

# ------- Executable ----- #


# ------- End Executable - #

# ------- Finds ---------- #

find_package(SDL3 REQUIRED COMPONENTS SDL3)

FIND_PACKAGE(Vulkan)
Message( STATUS "FINDING Vulkan" )
Message( STATUS "Vulkan_FOUND: " ${Vulkan_FOUND} )
IF (${Vulkan_FOUND})
    MESSAGE( STATUS "Vulkan_INCLUDE_DIRS: " ${Vulkan_INCLUDE_DIRS})
    MESSAGE( STATUS "Vulkan_LIBRARIES: " ${Vulkan_LIBRARIES})
ENDIF()
Message("")

SET(GLM_BINARY_DIR "/Users/evankelch/VulkanSDK/1.3.290.0/macOS/include/glm")

FIND_PACKAGE(Vulkan)
Message( STATUS "FINDING Vulkan" )
Message( STATUS "Vulkan_FOUND: " ${Vulkan_FOUND} )
IF (${Vulkan_FOUND})
    MESSAGE( STATUS "Vulkan_INCLUDE_DIRS: " ${Vulkan_INCLUDE_DIRS})
    MESSAGE( STATUS "Vulkan_LIBRARIES: " ${Vulkan_LIBRARIES})
ENDIF()
Message("")

ADD_EXECUTABLE(minecraft ${SOURCE_FILES} ${Vulkan_INCLUDE_DIRS})

file(COPY resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# ------- End Finds ------ #

# ------- Inc & Link ---- #

INCLUDE_DIRECTORIES(${SDL3_STATIC_LIBRARIES} ${Vulkan_INCLUDE_DIRS} ${GLM_BINARY_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} SDL3::SDL3 ${Vulkan_LIBRARIES})

# ------- End ----------- #
