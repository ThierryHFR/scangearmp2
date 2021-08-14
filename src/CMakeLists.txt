add_executable(scangearmp2 	main.c
                            callbacks.c
                            canon_mfp_io.c
                            canon_mfp_tools.c
                            cnmsfunc.c
                            errordlg.c
                            file_control.c
                            jpeg2pdf.c
                            keep_setting.c
                            mainui.c
                            progressbar.c
                            scanmain.c
                            selectdevice.c
              )

set_target_properties(scangearmp2 PROPERTIES
                      C_STANDARD 11
                      LINK_FLAGS "-Wl,--export-dynamic")

if(DEBUG)
  target_compile_definitions(scangearmp2 PRIVATE "_SGMP_DEBUG" "_SGMP_DEBUG_VERBOSE_")
  target_compile_options(scangearmp2 PRIVATE "-O0" "-ggdb3")
else(DEBUG)
  target_compile_options(scangearmp2 PRIVATE "-O2")
endif(DEBUG)

target_include_directories(scangearmp2 PRIVATE ${GTK3_INCLUDE_DIRS} ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(scangearmp2 pthread ${GTK3_LIBRARIES} ${LIBUSB_LIBRARIES})
target_link_libraries(scangearmp2 cncpmslld2 cncpnet2 cncpnet20 cncpnet30)
target_compile_definitions(scangearmp2 PRIVATE "PACKAGE_DATA_DIR=\"../share/scangearmp2\"")
target_compile_definitions(scangearmp2 PRIVATE "CNMSLIBPATH=../${CMAKE_INSTALL_LIBDIR}/bjlib")
target_compile_definitions(scangearmp2 PRIVATE "ENABLE_NLS")
install(TARGETS scangearmp2 DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(sane)
