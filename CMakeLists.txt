cmake_minimum_required(VERSION 3.14)
project(datapack)

# Setup

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_ADDITIONAL_TARGETS OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BUILD_ADDITIONAL_TARGETS ON)
endif()

# Library

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    add_library(datapack SHARED
        # src/binary/reader.cpp
        # src/binary/writer.cpp
        # src/encode/base64.cpp
        # src/encode/floating_string.cpp
        src/object/object.cpp
        # src/object/operations.cpp
        # src/object/reader.cpp
        # src/object/writer.cpp
        # src/schema/token.cpp
        # src/schema/tokenizer.cpp
        # src/schema/schema.cpp
        # src/debug.cpp
        # src/json.cpp
        # src/random.cpp
    )
    target_include_directories(datapack PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

else()
    add_library(datapack STATIC
        src/binary/reader.cpp
        src/binary/writer.cpp
    )
    target_include_directories(datapack PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Additional targets

if (BUILD_ADDITIONAL_TARGETS AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    add_subdirectory(examples)
    add_subdirectory(demo)
    # add_subdirectory(test)
endif()


# Create install target

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
