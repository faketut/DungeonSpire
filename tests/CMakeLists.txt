add_executable(run_tests tests.cc)
target_link_libraries(run_tests PRIVATE project_warnings cc3k_core)
target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Tests reference ./src/files/* via relative path; ensure ctest runs from repo root.
add_test(NAME unit_tests COMMAND run_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
