add_executable(run_tests tests.cc)
target_link_libraries(run_tests PRIVATE project_warnings)
target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

add_test(NAME unit_tests COMMAND run_tests)
