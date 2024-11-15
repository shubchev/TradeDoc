cmake_minimum_required(VERSION 3.20)

project("TradeDoc" VERSION 1.0.0.0)

set(CMAKE_BUILD_PARALLEL_LEVEL 8)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Compile options
#add_compile_definitions(_UNICODE)
#add_compile_definitions(UNICODE)
add_compile_definitions(HOST_COMPILE)

set(CONFIGURATION_TYPE $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>)
set(DEBUG_POSTFIX $<$<CONFIG:Debug>:d>)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE HDR_FILES src/*.h src/*.hpp)
file(GLOB_RECURSE SRC_FILES src/*.cc src/*.c src/*.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HDR_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})


add_executable(TradeDoc WIN32
    ${HDR_FILES}
    ${SRC_FILES}
    src/TradeDoc.rc
)

add_dependencies(TradeDoc
    cpp-utils)

target_compile_definitions(TradeDoc PUBLIC
    HOST_COMPILE
)

set(ZCONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/build_out/external/cpp-utils/external/zlib")

target_include_directories(TradeDoc PUBLIC
    ${PROJECT_SOURCE_DIR}/external/cpp-utils/external/glm
    ${PROJECT_SOURCE_DIR}/external/cpp-utils/external/GLFW/include
    ${PROJECT_SOURCE_DIR}/external/cpp-utils/lib/include
	${ZCONF_INCLUDE_DIRS}
)

target_link_directories(TradeDoc PUBLIC
    ${CMAKE_BINARY_DIR}/external/cpp-utils/lib/${CONFIGURATION_TYPE}
    ${CMAKE_BINARY_DIR}/external/cpp-utils/external/glfw/src/${CONFIGURATION_TYPE}
    ${CMAKE_BINARY_DIR}/external/cpp-utils/external/base64/${CONFIGURATION_TYPE}
    ${CMAKE_BINARY_DIR}/external/cpp-utils/external/zlib/${CONFIGURATION_TYPE}
    ${PROJECT_SOURCE_DIR}/external/cpp-utils/external/jpeg-bins/lib/win10/x64/${CONFIGURATION_TYPE}
    ${OpenCL_INCLUDE_DIR}
)

target_link_libraries(TradeDoc
    cpp-utils.lib
    glfw3.lib
	zlibstatic${DEBUG_POSTFIX}.lib
    turbojpeg-static.lib
    Wininet.lib
	base64.lib
	OpenGL32.lib
	${OpenCL_LIBRARY}
)


add_subdirectory(external/cpp-utils EXCLUDE_FROM_ALL)