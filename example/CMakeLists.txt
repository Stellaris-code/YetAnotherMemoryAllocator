file(GLOB_RECURSE source_files "*.c")

include_directories("../src")

add_executable(example ${source_files})
target_link_libraries(example ${PROJECT_NAME})
