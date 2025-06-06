include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
include(GoogleTest)
set(INSTALL_GTEST OFF)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

function(create_test name)
    add_executable(test_${name} ${name}.cpp)
    target_link_libraries(test_${name} datapack datapack_examples GTest::gtest_main)
    gtest_discover_tests(test_${name})
endfunction()

create_test(binary)
create_test(debug)
create_test(encode)
create_test(json)
create_test(object)
create_test(random)
create_test(schema)
