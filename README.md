				---Sane backend 'canon_pixma' for CANON's scanners---


# DESCRIPTION

	Because of changes in CANON's network communication protocols,some of CANON's scanner 
	became unsuported by sane.

	This backend is for making the CANON's scanners, using the new communication protocol,
	to be supported by sane and any of the frontends implementing sane. It reuse the code 
	of the scangearmp2 program to access to the scanners functionalities and implementing 
	the sane functions.

	It was done as a patch to have a minimun impact on the orginal scangearmp2 source code,
	but currently we choose to abandon this idea because it was difficult to make the 
	releases and had not that much interrest for the developement.
	
	For ubuntu-based distributions: https://launchpad.net/~thierry-f/+archive/ubuntu/fork-michael-gruz

```
 
 ## Add 2018
 XK80 series
 
 TS9580 series
 TS9500 series
  
 TS8280 series
 TS8230 series
 TS8200 series
 
 TS6280 series
 TS6230 series
 TS6200 series
 
 TR9530 series
 TR4500 series
 
 G3010 series
 G4010 series
 
 E4200 series
 
 LiDE 400
 LiDE 300

# Add before 2018
TS9100 series
TS8100 series
TS6100 series
TR8500 series
TR7500 series
TS5100 series
TS3100 series
E3100 series
TS9180 series
TS8180 series
TS6180 series
TR8580 series
TS8130 series
TS6130 series
TR8530 series
TR7530 series
XK50 series
XK70 series

MG7500 series
MG6600 series
MG5600 series
MG2900 series
MB2000 series
MB2300 series
MB5000 series
MB5300 series
E460 series

MX490 series
E480 series

MG7700 series
MG6900 series
MG6800 series
MG5700 series
MG3600 series

G3000 series

TS9000 series
TS8000 series
TS6000 series
TS5000 series
MG3000 series
E470 series
G4000 series

MB2100 series
MB2700 series
MB5100 series
MB5400 series

# Add 2019
G6000 series
G6080 series
TS5300 series
TS5380 series
TS6300 series
TS6380 series
TS7330 series
TS8300 series
TS8380 series
TS8330 series
XK60 series
TS6330 series
TS3300 series
E3300 series

```
# ADVANTAGES

Better image quality than 'pixma' backend (with an output image size of 2480x3507 pixels) Usable in Wi-fi.

# STATE

Tested with sane 1.0.25 and 1.0.27 (may not work for lower versions)
Currently the backend allow image in A4 format.
The scan works in color or in gray map.
The options are not well handled, so they might not work.
The color option allow to chose between color or graymap modes.
The resolution option allow to have a hight or low quality for the output.

# KNOWN PROBLEMS

When using xsane :
bug with the display of the selected color option (Fixed)

# REQUIREMENTS

Requirements for scangearmp2 in : scangearmp2/README.md

# INSTALLATION

## For debian systems :
###### Get sources :
```
git clone https://github.com/Ordissimo/scangearmp2.git
```
###### Get developement environnement :
```
apt-get update
apt-get install debhelper libglib2.0-dev libgtk2.0-dev libusb-1.0-0-dev libtool-bin libjpeg-dev
# or, if failure, use:
apt-get install debhelper libglib2.0-dev libgtk2.0-dev libusb-1.0-0-dev libtool libjpeg-dev
cp -a scangearmp2 scangearmp2-3.90
rm -rf scangearmp2-3.90/.git scangearmp2-3.90/debian/
tar cJvf scangearmp2_3.90.orig.tar.xz scangearmp2-3.90
```
###### Build Sources :
```
cd scangearmp2
debuild -us -uc ## 
```

###### Install :
```
dpkg -i ../scangearmp2_3.90-1_amd64.deb
```
## For redhat systems :

###### Get developement environnement :
```
yum install gtk2-devel
yum install libusb-devel
yum install libjpeg-devel
yum install gettext-devel
yum install libtool
yum install automake
yum install autoconf
yum install rpm-build
```
###### Get sources :
```
wget https://github.com/Ordissimo/scangearmp2/releases/download/3.90-2/scangearmp2.spec
wget https://github.com/Ordissimo/scangearmp2/releases/download/3.90-2/scangearmp2_3.90-2ubuntu.artful.tar.xz
tar xvf scangearmp2_3.90-2ubuntu.artful.tar.xz
tar czvf scangearmp2_3.90.orig.tar.gz scangearmp2
mv scangearmp2_3.90.orig.tar.gz ~/rpmbuild/SOURCES/
```
###### Build Sources :
```
rpmbuild -bp scangearmp2.spec
rpmbuild -bc --short-circuit scangearmp2.spec
rpmbuild -bi --short-circuit scangearmp2.spec
rpmbuild -ba scangearmp2.spec
```
###### Install :
```
rpm -i ~/rpmbuild/RPMS/x86_64/scangearmp2-3.90-2.x86_64.rpm
```
# Activate the backend
## For all distributions you need to activate the backend :
```
echo "canon_pixma"  >> /etc/sane.d/dll.conf # for activate the backend in sane
```

# LICENSE

## Licence of scangearmp2 in : scangearmp2/README.md
The following files are licensed under the terms of the GNU General Public License. (See the file scangearmp2/COPYING.) :
- scangearmp2/src/sane
- debian/patches/series</br>
- debian/source/format</br>
	
