## Process this file with automake to produce Makefile.in
# SUBDIRS = sane

INCLUDES = \
	-I$(top_srcdir)/include \
	@PACKAGE_CFLAGS@ \
	@GMODULE_CFLAGS@ \
	@LIBUSB_1_0_CFLAGS@

bin_PROGRAMS = scangearmp2

scangearmp2_SOURCES = \
	main.c \
	support.h \
	cnmsstrings.h \
	canon_mfp_io.c \
	canon_mfp_tools.c canon_mfp_tools.h \
	mainui.c mainui.h \
	selectdevice.c selectdevice.h \
	scanmain.c scanmain.h \
	errordlg.c errordlg.h \
	progressbar.c progressbar.h \
	file_control.c file_control.h \
	keep_setting.c keep_setting.h \
	jpeg2pdf.c jpeg2pdf.h \
	cnmsfunc.c cnmsfunc.h \
	callbacks.c callbacks.h

EXTRA_DIST = \
	canon_mfp2.conf

scangearmp2_LDADD = @PACKAGE_LIBS@ @GMODULE_LIBS@ @LIBUSB_1_0_LIBS@ -lcncpmslld2 -lcncpnet2 -lcncpnet20 -lcncpnet30 -lpthread $(INTLLIBS)

scangearmp2_CFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)/$(PACKAGE)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

# UNAME = `shell uname -m`

libsane_canon_pixmadir = /usr/lib/sane
libsane_canon_pixma_LTLIBRARIES = libsane-canon_pixma.la
libsane_canon_pixma_la_SOURCES = sane/saneopts.h sane/sane.h sane/sanei_debug.h \
	sane/sanei.h sane/sanei_init_debug.c sane/sanei_backend.h \
	sane/canon_pixma.c sane/canon_pixma.h sane/errordlg.c sane/errordlg.h \
	keep_setting.c canon_mfp_io.c cnmsstrings.h errors.h \
	keep_setting.h canon_mfp_tools.c file_control.c canon_mfp_tools.h \
	cnmsfunc.c cnmsfunc.h file_control.h support.h
libsane_canon_pixma_la_LDFLAGS = -version-info 1:0:0
# The following is a interlibrary dependency that must be compiled first.
# if [ "$(UNAME)" = "x86_64" ]
# then
libsane_canon_pixma_la_LIBADD = -L$(top_srcdir)/../com/libs_bin64 -lcncpmslld2 -lcncpnet2 -lcncpnet20 -lcncpnet30 -lpthread -ldl $(LIBUSB_1_0_LIBS)
# fi
# if [ "$(UNAME)" <> "x86_64" ]
# then
# 	libsane_canon_pixma_la_LIBADD = -L$(top_srcdir)/../com/libs_bin32 -lcncpmslld2 -lcncpnet2 -lcncpnet20 -lcncpnet30 -lpthread -ldl $(LIBUSB_1_0_LIBS)
# fi
#libsane_canon_pixma_la_CFLAGS = -DWITH_NONAMESPACES -DSOAP_DEBUG
libsane_canon_pixma_la_CFLAGS = $(LIBUSB_1_0_CFLAGS) -I$(top_srcdir)/include




install-data-am:
	if test ! -d $(DESTDIR)$(enable_libpath)/bjlib; then mkdir -p $(DESTDIR)$(enable_libpath)/bjlib ; fi ; \
	$(INSTALL_DATA) canon_mfp2.conf $(DESTDIR)$(enable_libpath)/bjlib
