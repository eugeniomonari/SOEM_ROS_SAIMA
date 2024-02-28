set(SOURCES ecat_comm_test.cpp)
add_executable(ecat_comm_test ${SOURCES})
target_link_libraries(ecat_comm_test soem)
install(TARGETS ecat_comm_test DESTINATION bin)
