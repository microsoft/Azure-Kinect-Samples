
add_executable(simple_sample main.c)


# Dependencies of this library
target_link_libraries(simple_sample PRIVATE 
    k4a
    k4abt
    )


