add_executable(test-resolution-capabilities
  test_resolution_capabilities.c
  ../src/resolution_capabilities.c
)

target_include_directories(test-resolution-capabilities PRIVATE ../src)
add_test(NAME resolution-capabilities COMMAND test-resolution-capabilities)

add_executable(test-image-processing
  test_image_processing.c
  ../src/image_processing.c
)

target_include_directories(test-image-processing PRIVATE ../src ${GTK4_INCLUDE_DIRS})
target_link_libraries(test-image-processing ${GTK4_LIBRARIES} JPEG::JPEG m)
add_test(NAME image-processing COMMAND test-image-processing)

add_executable(test-scan-geometry
  test_scan_geometry.c
  ../src/scan_geometry.c
)

target_include_directories(test-scan-geometry PRIVATE ../src)
add_test(NAME scan-geometry COMMAND test-scan-geometry)
