file(GLOB CC3K_SOURCES CONFIGURE_DEPENDS "*.cc")

add_executable(cc3k ${CC3K_SOURCES})
target_link_libraries(cc3k PRIVATE project_warnings)
target_include_directories(cc3k PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Make the data files (files/) discoverable next to the binary for ad-hoc runs.
add_custom_command(TARGET cc3k POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/files"
        "$<TARGET_FILE_DIR:cc3k>/files"
    COMMENT "Copying game data files next to cc3k"
)
