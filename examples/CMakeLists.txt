add_library(datapack_examples
    src/entity.cpp
)
target_include_directories(datapack_examples PUBLIC include)
target_link_libraries(datapack_examples datapack)
