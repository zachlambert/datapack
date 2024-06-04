function(create_demo group name)
    add_executable(demo_${group}_${name} ${group}/${name}.cpp)
    target_link_libraries(demo_${group}_${name} datapack datapack_examples)
endfunction()

create_demo(binary array)
create_demo(binary benchmark)
create_demo(binary binary)

create_demo(encode base64)

create_demo(json dump)
create_demo(json load)

create_demo(object compare)
create_demo(object object)
create_demo(object reader)
create_demo(object writer)

create_demo(schema schema)

create_demo(util debug)
create_demo(util random)
