file(GLOB CC3K_SOURCES CONFIGURE_DEPENDS "*.cc")
list(REMOVE_ITEM CC3K_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")

# Shared static library used by both the game binary and the test runner.
add_library(cc3k_core STATIC ${CC3K_SOURCES})
target_link_libraries(cc3k_core PUBLIC project_warnings)
target_include_directories(cc3k_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cc3k main.cc)
target_link_libraries(cc3k PRIVATE cc3k_core)

# Make the data files (files/ and data/) discoverable next to the binary
# for ad-hoc runs out of the build directory.
add_custom_command(TARGET cc3k POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/files"
        "$<TARGET_FILE_DIR:cc3k>/files"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:cc3k>/data"
    COMMENT "Copying game data files next to cc3k"
)
