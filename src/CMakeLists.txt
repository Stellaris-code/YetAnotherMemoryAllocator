file(GLOB_RECURSE source_files "*.c")
list(REMOVE_ITEM source_files "main.c")
file(GLOB_RECURSE header_files "*.h")

add_library(${PROJECT_NAME} ${header_files} ${source_files})

if (USE_CLANG_TIDY)
    set(DO_CLANG_TIDY "clang-tidy" "-checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus*,cppcoreguidelines-*,llvm-include-order")
    set_target_properties(
      ${PROJECT_NAME} PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif()
