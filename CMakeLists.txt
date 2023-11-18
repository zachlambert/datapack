cmake_minimum_required(VERSION 3.14)
project(datapack)

# Setup

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

set(BUILD_ADDITIONAL_TARGETS OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BUILD_ADDITIONAL_TARGETS ON)
endif()

# Library

add_library(datapack SHARED
  src/json.cpp
)
target_include_directories(datapack PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


# Additional targets

if(BUILD_ADDITIONAL_TARGETS)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
    include(GoogleTest)
    set(INSTALL_GTEST OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_json test/json.cpp)
    target_link_libraries(test_json datapack GTest::gtest_main)
    gtest_discover_tests(test_json)
endif()


# Create install target
# https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/datapackConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS datapack
    EXPORT datapackTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/datapackConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/datapackConfig.cmake"
    INSTALL_DESTINATION lib/cmake/datapack
)

install(EXPORT datapackTargets DESTINATION lib/cmake/datapack)
install(FILES "${PROJECT_BINARY_DIR}/datapackConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/datapackConfig.cmake"
        DESTINATION lib/cmake/datapack)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
