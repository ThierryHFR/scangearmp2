%define VERSION 3.90
%define RELEASE 3

%define _arc  %(getconf LONG_BIT)
%define _is64 %(if [ `getconf LONG_BIT` = "64" ] ; then  printf "64";  fi)

%define _prefix /usr/local
%define _bindir %{_prefix}/bin
%define _libdir /usr/lib%{_is64}

%define CNCP_LIBS_COM libcncpmslld2 libcncpnet2 libcncpnet20 libcncpnet30

Summary: ScanGear MP Ver.%{VERSION} for Linux
Name: scangearmp2
Version: %{VERSION}
Release: %{RELEASE}
License: See the LICENSE*.txt file.
Vendor: CANON INC.
Group: Applications/Graphics
Source0: scangearmp2_%{version}.orig.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
Requires: gtk2, libusb, libjpeg-devel
BuildRequires: gtk2-devel, libusb-devel, libjpeg-devel, gettext-devel, libtool, automake, autoconf


%description
ScanGear MP for Linux.
This ScanGear MP provides scanning functions for Canon Multifunction Inkjet Printer.


%prep
%setup -q -n scangearmp2-%{version}


%build
#make


%install
# make install directory
mkdir -p ${RPM_BUILD_ROOT}%{_bindir}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/bjlib
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/sane
mkdir -p ${RPM_BUILD_ROOT}/etc/udev/rules.d/
mkdir -p ${RPM_BUILD_ROOT}%{_datadir}/doc/scangearmp2-%{version}

# copy common libraries
install -c -s -m 755 com/libs_bin%{_arc}/*.so.* ${RPM_BUILD_ROOT}%{_libdir}
install -c -m 666 com/ini/canon_mfp2_net.ini ${RPM_BUILD_ROOT}%{_libdir}/bjlib

# copy rules file
pushd scangearmp2
	install -c -m 644 etc/*.rules ${RPM_BUILD_ROOT}/etc/udev/rules.d/
popd

# make install
pushd scangearmp2
	./autogen.sh --prefix=%{_prefix} --enable-libpath=%{_libdir} LDFLAGS="-L`pwd`/../com/libs_bin%{_arc}"
	make clean
	make
	make install DESTDIR=${RPM_BUILD_ROOT} 
	# remove .la .a
	rm -f ${RPM_BUILD_ROOT}%{_libdir}/*.la ${RPM_BUILD_ROOT}%{_libdir}/*.a
popd
	install -c -m 666 scangearmp2/src/libsane-canon_pixma.la ${RPM_BUILD_ROOT}%{_libdir}/sane
	install -c -m 666 scangearmp2/src/.libs/libsane-canon_pixma.a ${RPM_BUILD_ROOT}%{_libdir}/sane
	install -c -m 666 scangearmp2/src/.libs/libsane-canon_pixma.so.1.0.0 ${RPM_BUILD_ROOT}%{_libdir}/sane
	ln -sf %{_libdir}/sane/libsane-canon_pixma.so.1.0.0 ${RPM_BUILD_ROOT}%{_libdir}/sane/libsane-canon_pixma.so
	ln -sf %{_libdir}/sane/libsane-canon_pixma.so.1.0.0 ${RPM_BUILD_ROOT}%{_libdir}/sane/libsane-canon_pixma.so.1

%clean
rm -rf $RPM_BUILD_ROOT


%post
if [ -x /sbin/ldconfig ]; then
	/sbin/ldconfig
fi
#reload udev rules
if [ -x /sbin/udevadm ]; then
	/sbin/udevadm control --reload-rules 2> /dev/null
	/sbin/udevadm trigger --action=add --subsystem-match=usb 2> /dev/null
fi

%postun
# remove symbolic link (common libs)
for LIBS in %{CNCP_LIBS_COM}
do
	if [ -h %{_libdir}/${LIBS}.so ]; then
		rm -f %{_libdir}/${LIBS}.so
	fi	
done

# remove sgmp2_setting files
rm -f /var/tmp/canon_sgmp2_setting*.*

# remove directory
if [ "$1" = 0 ] ; then
	rmdir -p --ignore-fail-on-non-empty %{_prefix}/share/locale/*/LC_MESSAGES
	rmdir -p --ignore-fail-on-non-empty %{_prefix}/share/scangearmp2
	rmdir -p --ignore-fail-on-non-empty %{_libdir}/bjlib
	rmdir -p --ignore-fail-on-non-empty %{_bindir}
fi

if [ -x /sbin/ldconfig ]; then
	/sbin/ldconfig
fi


%files
%defattr(-,root,root)
%{_libdir}/libcncpmslld2.so*
%{_libdir}/libcncpnet2.so*
%{_libdir}/libcncpnet20.so*
%{_libdir}/libcncpnet30.so*

%{_bindir}/scangearmp2
%{_libdir}/bjlib/canon_mfp2.conf
%{_libdir}/sane/*
%{_prefix}/share/locale/*/LC_MESSAGES/scangearmp2.mo
%{_prefix}/share/scangearmp2/*

/etc/udev/rules.d/*.rules

%doc doc/LICENSE-scangearmp-%{VERSION}EN.txt
%doc doc/LICENSE-scangearmp-%{VERSION}JP.txt
%doc doc/LICENSE-scangearmp-%{VERSION}FR.txt
%doc doc/LICENSE-scangearmp-%{VERSION}SC.txt

%attr(666,root,root) %{_libdir}/bjlib/canon_mfp2_net.ini




%ChangeLog

