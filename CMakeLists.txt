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

add_compile_options(-fconcepts-diagnostics-depth=2)

# Library

add_library(datapack SHARED
    src/util/debug.cpp
    src/util/random.cpp

    src/encode/base64.cpp
    src/encode/float_string.cpp

    src/object.cpp
    src/util/object_writer.cpp
    src/util/object_reader.cpp

    src/format/binary_writer.cpp
    src/format/binary_reader.cpp
    src/format/json.cpp
    # src/schema.cpp
)
target_include_directories(datapack PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


# Additional targets

if (BUILD_ADDITIONAL_TARGETS)
    add_subdirectory(examples)
    # add_subdirectory(demo)

    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
    include(GoogleTest)
    set(INSTALL_GTEST OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_util test/util/debug.cpp test/util/random.cpp)
    target_link_libraries(test_util datapack datapack_examples GTest::gtest_main)
    gtest_discover_tests(test_util)

    add_executable(test_encode test/encode/base64.cpp test/encode/float_string.cpp)
    target_link_libraries(test_encode datapack GTest::gtest_main)
    gtest_discover_tests(test_encode)

    add_executable(test_object
        test/object/object.cpp
        test/object/writer.cpp
        test/object/reader.cpp
    )
    target_link_libraries(test_object datapack datapack_examples GTest::gtest_main)
    gtest_discover_tests(test_object)

    add_executable(test_format
        test/format/binary.cpp
        test/format/binary_array.cpp
        test/format/json.cpp
    )
    target_link_libraries(test_format datapack datapack_examples GTest::gtest_main)
    gtest_discover_tests(test_format)
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
