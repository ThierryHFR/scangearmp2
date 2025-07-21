				---Sane backend 'canon_pixma' for CANON's scanners---


# DESCRIPTION

	Because of changes in CANON's network communication protocols, some of CANON's scanner 
	became unsuported by sane.

	This backend is for making the CANON's scanners, using the new communication protocol,
	to be supported by sane and any of the frontends implementing sane. It reuse the code 
	of the scangearmp2 program to access to the scanners functionalities and implementing 
	the sane functions.

	It was done as a patch to have a minimum impact on the orginal scangearmp2 source code,
	but currently we choose to abandon this idea because it was difficult to make the 
	releases and had not that much interest for the developement.
	
	For ubuntu-based distributions: https://launchpad.net/~thierry-f/+archive/ubuntu/fork-michael-gruz
	For debian-based distribution : https://github.com/Ordissimo/scangearmp2/releases/tag/4.12 
	
	The scangearmp2 version of my repository, contains 3 elements:
	1 - binary libraries where the source code is not provided (under Canon license), it lets us use and 
	redistribute it, without the right to decompile it.
       2 - the scangearmp2 GUI in GPL provided by Canon, I have extended these features and migrated it
       to GTK4.
	3 - The code developed by the Compagny Ordissimo which is under GPL license, which is based on
	Canon's proprietary libraries. This code provides a backend to "sane".
	
	- libcanon-proprio-bin (https://github.com/Ordissimo/libcanon-proprio-bin) contains the proprietary
	libraries for Canon, I repackage them but do not make any modification on them. This allows you
	to use them without having to install the whole GTK2 toolkit, this supposes that you install
	libmfp2-canon.
	- libmfp2-canon (https://github.com/Ordissimo/libmfp2-canon) is the part provided by Ordissimo, which
	is under GPL license and is a backend of the sane project
	
	Finally, and to clear up any ambiguity, I am the developer of the company Ordissimo with no ties 
	whatsoever to Canon.

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

#Add 2020
G7000 series
G7080 series
GM4000 series
GM4080 series


Add Dec 2020
TS3400 series
E3400 series
TR7000 series
G2020 series
G3060 series
G2060 series
G3020 series
TS7430 series
XK90 series
TS8430 series
TR7600 series
TR8600 series
TR8630 series
TS6400 series
TS7400 series


Add Dec 2021
TS5350i series
G600 series
TS3500 series
TR4600 series
E4500 series
TR4700 series
XK500 series
TS8530 series
XK100 series
TS7530 series
TS7450i series
GX6000 series
GX7000 series
TS5400 series


Add May 2022
TS2400 series
TS2600 series

Add Dec 2022
TS8630 series
XK110 series
GX3000 series
GX4000 series

Add Jui 2023
G3070 series
G2070 series
G4070 series
G3030 series
G2030 series
TC-20M
TC-5200M

Add Oct 2023
TS8700 series
XK120 series
GX1000 series
GX2000 series
TS6630 series
TS7700 series
TS7600i series
TS6730 series
TR7800 series
TS7700i series
TS7700A series
GX6500 series

Add May 2024
TS3600 series
TS3700 series
E3600 series

Add Oct 2024
G4090 series
G4080 series
G3090 series
G3080 series
TS8800 series
XK130 series



```
# ADVANTAGES

Better image quality than 'pixma' backend (with an output image size of 2480x3507 pixels) Usable in Wi-fi.

# STATE

Tested with sane 1.0.25, 1.0.27, 1.0.29 and 1.3.1<br>
In Platen, the backend recognizes CARD, L_L, L_P, 4X6_L, 4X6_P, HAGAKI_L, HAGAKI_P, 2L_L, 2L_P, A5, B5, A4 and LETTER formats.<br>
In ADF, the backend recognizes A4 and LETTER formats.<br>
The scan works in color or in gray map.<br>
The resolution option allow to have a 75, 150, 300 or 600 quality for the output.<br>
The source option allow to chose between Platen, ADF and ADF Duplex, i don't have a means of detection, so it depends on the hardware.<br>

# KNOWN PROBLEMS

When using xsane :
bug with the display of the selected color option (Fixed)

# BINARIES

## Binary for the majority of Distribution

https://download.opensuse.org/repositories/home:/thierry1970/
```
CentOS 8                  x86_64, aarch64
Arch                      x86_64
Debian 9.0                x86_64, i386, aarch64
Debian 10                 x86_64, i386, aarch64
Debian 11                 x86_64, i386, aarch64
Fedora 31                 x86_64, aarch64
Fedora 32                 x86_64, aarch64
Fedora 33                 x86_64, aarch64
Fedora 34                 x86_64, i386, aarch64
Raspbian 9.0              aarch64
Raspbian 10               aarch64
Raspbian 11               aarch64
xUbuntu 18.04             x86_64, i386
xUbuntu 20.04             x86_64, i386, aarch64
xUbuntu 22.04             x86_64, i386, aarch64
```


# INSTALLATION

## For Arch Linux systems :
```
makepkg && sudo pacman -U scangearmp2-sane-*.tar.zst
```

## For debian systems :
###### Get sources :
```
mkdir build
cd build
git clone https://github.com/ThierryFR/scangearmp2.git
git checkout debian
```
###### Get development environment :
```
apt update
apt install debhelper libglib2.0-dev libgtk-3-dev libusb-1.0-0-dev libtool-bin libjpeg-dev intltool libsane-dev
cp -a scangearmp2 scangearmp2-4.80
rm -rf scangearmp2-4.80/.git scangearmp2-4.80/debian/
tar cJvf scangearmp2_4.80.orig.tar.xz scangearmp2-80
```
###### Build Sources :
```
cd scangearmp2
dpkg-buildpackage -us -uc ## 
```

###### Install :
```
apt install ../scangearmp2_4.80-1_amd64.deb
```
## For redhat systems : <font color="red">This is no longer current</font>

###### Get development environment :
```
yum install gtk3-devel
yum install libusb-devel
yum install libjpeg-devel
yum install gettext-devel
yum install cmake
yum install rpm-build
```
###### Get sources :
```
wget https://github.com/ThierryFR/scangearmp2/releases/download/4.80/scangearmp2.spec
wget https://github.com/ThierryFR/scangearmp2/releases/download/4.80/scangearmp2_4.80.tar.xz
tar xvf scangearmp2_4.80-1.tar.xz
tar czvf scangearmp2_4.80.orig.tar.gz scangearmp2
mv scangearmp2_4.80a.orig.tar.gz ~/rpmbuild/SOURCES/
```

Or, from git repository
```
git archive --format=tar --prefix=scangearmp2-4.80/ HEAD |gzip >scangearmp2_4.80.orig.tar.gz
mv scangearmp2_4.80.orig.tar.gz ~/rpmbuild/SOURCES/
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
rpm -i ~/rpmbuild/RPMS/x86_64/scangearmp2-4.80-1.x86_64.rpm
```

###### Firewall :
In case of problem with `firewalld` 
 you can copy the service definition 'scangearmp2/etc/canon-scan.xml'
 into '/etc/firewalld/services/' and activate the service (for the adequate zone(s))

```
sudo cp /etc/canon-scan.xml /etc/firewalld/services/
firewall-cmd --permanent --zone=home --add-service=canon-scan
```

Nota: 
- A way to list detected scanners is `scanimage -L`
- To see firewall's rejected packages `firewall-cmd --set-log-denied=all` and then call `journalctl -f`
- See https://firewalld.org/documentation/service/options.html

# LICENSE

## Licence of scangearmp2 in : scangearmp2/README.md
The following files are licensed under the terms of the GNU General Public License. (See the file scangearmp2/COPYING.) :
- scangearmp2/src/sane
- debian/patches/series</br>
- debian/source/format</br>
	
