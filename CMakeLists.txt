cmake_minimum_required(VERSION 3.5)

project(scangearmp2-sane)

# The target architecture.
# For cross-compilation this should still work as you're supposed to set this var.
# X64 mean 64-bit generically, whether AMD64 or AARCH64.
set(TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "Target architecture")
# message(FATAL_ERROR ">>>[${CMAKE_SYSTEM_PROCESSOR}]")
if (TARGET_ARCH MATCHES "^mips64el")
  set(ARCH 1)
elseif (TARGET_ARCH MATCHES "^aarch64")
  set(ARCH 2)
elseif (TARGET_ARCH MATCHES "^x86_64")
  set(ARCH 3)
elseif (TARGET_ARCH MATCHES "^i686")
  set(ARCH 4)
else ()
  message(FATAL_ERROR "Arch device not supported")
endif ()

set(CMAKE_INSTALL_PREFIX "/usr")
set(CFLAGS_WARN     "-Wall -Wextra -Werror -Wno-deprecated-declarations -Wno-format-truncation -Wno-stringop-truncation")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${CFLAGS_WARN}")

include(GNUInstallDirs) # for make install

find_program(INTLTOOL_UPDATE_EXECUTABLE intltool-update)
if(NOT INTLTOOL_UPDATE_EXECUTABLE)
    message(FATAL_ERROR "Couldn't find intltool-update")
endif()

find_package(Gettext REQUIRED)
include(FindPkgConfig)
pkg_search_module(LIBUSB REQUIRED libusb-1.0)
pkg_search_module(GTK4 REQUIRED gtk4)
pkg_search_module(SANE REQUIRED sane-backends)
find_package(JPEG REQUIRED)

option(DEBUG "turn on debug info" OFF)

add_subdirectory(po)
add_subdirectory(src)

if (ARCH EQUAL 1)
add_subdirectory(usr/libmips64)
elseif (ARCH EQUAL 2)
add_subdirectory(usr/libaarch64)
elseif (ARCH EQUAL 3)
add_subdirectory(usr/lib64)
elseif (ARCH EQUAL 4)
add_subdirectory(usr/lib32)
endif()
add_subdirectory(usr/share/scangearmp2)

install(FILES "etc/udev/rules.d/80-canon_mfp2.rules" DESTINATION "/etc/udev/rules.d/")
install(FILES "etc/sane.d/dll.d/mfp2" DESTINATION "/etc/sane.d/dll.d/")
install(FILES "etc/sane.d/canon_pixma.conf" DESTINATION "/etc/sane.d/")
install(FILES "etc/canon-scan.xml" DESTINATION "/etc/")
install(FILES "usr/share/applications/scangearmp2.desktop" DESTINATION "/usr/share/applications/")
