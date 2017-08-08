## Process this file with automake to produce Makefile.in

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

install-data-am:
	if test ! -d $(DESTDIR)$(enable_libpath)/bjlib; then mkdir -p $(DESTDIR)$(enable_libpath)/bjlib ; fi ; \
	$(INSTALL_DATA) canon_mfp2.conf $(DESTDIR)$(enable_libpath)/bjlib
