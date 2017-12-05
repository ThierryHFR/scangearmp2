				---Sane backend 'canon_pixma' for CANON's scanners---
</br>
</br>
DESCRIPTION</br>
</br>
	Because of changes in CANON's network communication protocols,some of CANON's scanner 
	became unsuported by sane.</br>
</br>
	This backend is for making the CANON's scanners, using the new communication protocol,
	to be supported by sane and any of the frontends implementing sane. It reuse the code 
	of the scangearmp2 program to access to the scanners functionalities and implementing 
	the sane functions.</br>
</br>
	It was done as a patch to have a minimun impact on the orginal scangearmp2 source code,
	but currently we choose to abandon this idea because it was difficult to make the 
	releases and had not that much interrest for the developement.</br>
</br>
	The scanner supported by this backend are :</br>
</br>

</br>
#TS9100 series</br>
#TS8100 series</br>
#TS6100 series</br>
#TR8500 series</br>
#TR7500 series</br>
#TS5100 series</br>
#TS3100 series</br>
#E3100 series</br>
#TS9180 series</br>
#TS8180 series</br>
#TS6180 series</br>
#TR8580 series</br>
#TS8130 series</br>
#TS6130 series</br>
#TR8530 series</br>
#TR7530 series</br>
#XK50 series</br>
#XK70 series</br>
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
</br>
ADVANTAGES
</br>
Better image quality than 'pixma' backend (with an output image size of 2480x3507 pixels) </br>
Usable in Wi-fi.</br>
</br>
</br>

STATE</br>
</br>
Tested with sane 1.0.25 and 1.0.27 (may not work for lower versions)</br>
Currently the backend allow image in A4 format.</br>
The scan works in color or in gray map.</br>
The options are not well handled, so they might not work.</br>
The color option allow to chose between color or graymap modes.</br>
The resolution option allow to have a hight or low quality for the output.</br>
</br>
KNOWN PROBLEMS</br>
</br>
When using xsane :</br>
bug with the display of the selected color option (Fixed)</br> 
</br>
REQUIREMENTS</br>
</br>
Requirements for scangearmp2 in : scangearmp2/README.md</br>
</br>
INSTALLATION</br>
</br>
<u>For debian systems :</u> </br>
</br>
git clone https://github.com/Ordissimo/scangearmp2.git</br>
cp -a scangearmp2 scangearmp2-3.50</br>
rm -rf scangearmp2-3.50/.git scangearmp2-3.50/debian/</br>
tar cJvf scangearmp2_3.50.orig.tar.xz scangearmp2-3.50</br>
cd scangearmp2</br>
debuild -tc</br>
dpkg -i ../scangearmp2_3.50-1_amd64.deb</br>
</br>
<u>For redhat systems :</u> </br>
</br>
#Get developement environnement :</br>
yum install gtk2-devel</br>
yum install libusb-devel </br>
yum install libjpeg-devel</br>
yum install gettext-devel</br>
yum install libtool</br>
yum install automake</br>
yum install autoconf</br>
yum install rpm-build</br>
</br>
#Get sources :</br>
wget https://github.com/Ordissimo/scangearmp2/releases/download/3.40.2/scangearmp2.spec</br>
wget https://github.com/Ordissimo/scangearmp2/releases/download/3.50-2/scangearmp2_3.50-2ubuntu.artful.tar.xz</br>
tar xvf scangearmp2_3.50-2ubuntu.artful.tar.xz</br>
tar czvf scangearmp2_3.50.orig.tar.gz scangearmp2</br>
mv scangearmp2_3.50.orig.tar.gz ~/rpmbuild/SOURCES/</br>
</br>
#Build Sources :</br>
rpmbuild -bp scangearmp2.spec</br>
rpmbuild -bc --short-circuit scangearmp2.spec</br>
rpmbuild -bi --short-circuit scangearmp2.spec</br>
rpmbuild -ba scangearmp2.spec</br>
</br>
#Install :
rpm -i ~/rpmbuild/RPMS/x86_64/scangearmp2-3.50-2.x86_64.rpm</br>
</br>
</br>
<u>For all distributions you need to activate the backend :</u></br>
echo "canon_pixma"  >> /etc/sane.d/dll.conf # for activate the backend in sane</br>

</br>
</br>
LICENSE</br>
</br>
<u>Licence of scangearmp2 in :</u> scangearmp2/README.md</br>
The following files are licensed under the terms of the GNU General Public License. (See the file scangearmp2/COPYING.) :</br>
-  scangearmp2/src/sane</br>
-  debian/patches/series</br>
-  debian/source/format</br>
</br>
</br>
	
