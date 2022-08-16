foreach(bin_lib_file libcncpmslld2.so.3.0.0 libcncpnet2.so.1.2.5 libcncpnet20.so.1.0.0 libcncpnet30.so.1.0.0)

  #get SONAME from canon shared libs to construct symlinks
  string(REGEX REPLACE "^lib(.*)\.so.*" "\\1" bin_lib_name ${bin_lib_file})
  execute_process(COMMAND objdump -p ${bin_lib_file}
                  COMMAND grep SONAME
                  COMMAND sed "s/^[[:space:]]*SONAME[[:space:]]*//"
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  OUTPUT_VARIABLE bin_lib_soname
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                 )

  # move canon shared libs from source dir to build dir and create symlinks
  add_custom_target("fake_build_${bin_lib_name}"
                    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/${bin_lib_file}" "./"
                    COMMAND ln -sf ${bin_lib_file} ${bin_lib_soname}
                   )
  add_library(${bin_lib_name} SHARED IMPORTED GLOBAL)
  add_dependencies(${bin_lib_name} "fake_build_${bin_lib_name}")
  set_target_properties(${bin_lib_name} PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${bin_lib_file}")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${bin_lib_file}"
                "${CMAKE_CURRENT_BINARY_DIR}/${bin_lib_soname}"
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE
         )
endforeach(bin_lib_file)

install(DIRECTORY bjlib DESTINATION ${CMAKE_INSTALL_LIBDIR} USE_SOURCE_PERMISSIONS)
