add_executable(test-resolution-capabilities
  test_resolution_capabilities.c
  ../src/resolution_capabilities.c
)

target_include_directories(test-resolution-capabilities PRIVATE ../src)
add_test(NAME resolution-capabilities COMMAND test-resolution-capabilities)
