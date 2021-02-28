set(MO_NAME "scangearmp2.mo")
foreach(lang de fr ja zh ru)
  add_custom_command(OUTPUT "${lang}/LC_MESSAGES/${MO_NAME}"
                      COMMAND mkdir -p "${lang}/LC_MESSAGES"
                      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po" -o "${lang}/LC_MESSAGES/${MO_NAME}"
                     )
  list(APPEND MO_FILES "${lang}/LC_MESSAGES/${MO_NAME}")
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lang}" DESTINATION "${CMAKE_INSTALL_LOCALEDIR}")
 endforeach(lang)
 add_custom_target(generate-mo ALL SOURCES ${MO_FILES})

#no need to call this target at every building - run it manually when needed.
add_custom_target(update_translations
                  COMMAND intltool-update -r -g scangearmp2
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 )
