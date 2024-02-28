
set(SOURCES custom_test.c)
add_executable(custom_test ${SOURCES})
target_link_libraries(custom_test soem)
install(TARGETS custom_test DESTINATION bin)
