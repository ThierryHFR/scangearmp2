				---Sane backend for CANON's scanners---
</br>
</br>
DESCRIPTION
</br>
</br>
	Because of changes in CANON's network communication protocols,some of CANON's scanner 
	became unsuported by sane.</br>
</br>
	This backend is for making the CANON's scanners, using the new communication protocol,
	to be supported by sane and any of the frontends implementing sane. It reuse the code 
	of the scangearmp2 program to access to the scanners functionalities and implementing 
	the sane functions.</br>
</br>
	It has been done as a patch to have a minimun impact on the orginal scangearmp2 source
	code.</br>
</br>
	The scanner supported by this backend are :
</br>
</br>
#MG7500 series</br>
#MG6600 series</br>
#MG5600 series</br>
#MG2900 series</br>
#MB2000 series</br>
#MB2300 series</br>
#MB5000 series</br>
#MB5300 series</br>
#E460 series</br>
</br>
#MX490 series</br>
#E480 series</br>
</br>
#MG7700 series</br>
#MG6900 series</br>
#MG6800 series</br>
#MG5700 series</br>
#MG3600 series</br>
</br>
#G3000 series</br>
</br>
#TS9000 series</br>
#TS8000 series</br>
#TS6000 series</br>
#TS5000 series</br>
#MG3000 series</br>
#E470 series</br>
#G4000 series</br>
</br>
#MB2100 series</br>
#MB2700 series</br>
#MB5100 series</br>
#MB5400 series</br>
</br>

STATE
</br>
</br>
	Tested with sane 1.0.27 (may not work for lower versions)</br>
	Currently the backend allow image in A4 format.</br>
	The scan works in color or in gray map(must be change in the code).</br>
	The options are not well handled, so they might not work.</br>
</br>
KNOWN BUGS
</br>
</br>
	*Cancelling a scan cause : 
</br>
		-If the scanner is in usb the operation stay blocked and seems to
		do nothing.
</br>
		-If the scanner is in network the operation stays blocked and will
		loop on writing an message.
	
</br>
	*Do a scan after a abnormal end will, if the device is connected in usb,
		fail in the function sane_start with an IO error.
</br>
</br>
REQUIREMENTS
</br>
</br>
	requirements for scangearmp2 in : scangearmp2/README.md
</br>
</br>
INSTALLATION
</br>
</br>
	for debian systems : 
</br>
		git clone https://github.com/Ordissimo/scangearmp2.git
</br>
		cp -a scangearmp2 scangearmp2-3.40
</br>
		rm -rf scangearmp2-3.40/.git scangearmp2-3.40/debian/
</br>
		tar cJvf scangearmp2_3.40.orig.tar.xz scangearmp2-3.40
</br>
		cd scangearmp2
</br>
		debuild -tc
</br>
		dpkg -i ../scangearmp2_3.40-1_amd64.deb
</br>
		echo "canon_pixma"  >> /etc/sane.d/dll.conf # for activate the backend in sane
</br>
</br>
LICENSE
</br>
   	licence of scangearmp2 in : scangearmp2/README.md
</br>
	The following files are licensed under the terms of the GNU General Public License. (See the file scangearmp2/COPYING.) :
</br>
	-  debian/patches/libsane-canon_pixma.patch
</br>
	-  debian/patches/series
</br>
	-  debian/source/format
</br>
</br>
</br>
	
