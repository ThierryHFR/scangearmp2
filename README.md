				---Sane backend for CANON's scanners---
</br>
</br>
DESCRIPTION
</br>
</br>
	Because of changes in CANON's network communication protocols,some of CANON's scanner 
	became unsuported by sane.
</br>
	This backend is for making the CANON's scanners, using the new communication protocol,
	to be supported by sane and any of the frontends implementing sane. It reuse the code 
	of the scangearmp2 program to access to the scanners functionalities and implementing 
	the sane functions.
</br>
</br>
	The concerned scanner are :
</br>
</br>
	#E460 series</br>
	#E470 series</br>
	#E480 series</br>
</br>
	#G3000 series</br>
	#G4000 series</br>
</br>
	#MB2000 series</br>
	#MB2100 series</br>
	#MB2300 series</br>
	#MB2700 series</br>
</br>
	#MB5000 series</br>
	#MB5100 series</br>
	#MB5300 series</br>
	#MB5400 series</br>
</br>
	#MG2900 series</br>
</br>
	#MG3000 series</br>
	#MG3600 series</br>
</br>
	#MG5600 series</br>
	#MG5700 series</br>
</br>
	#MG6600 series</br>
	#MG6800 series</br>
	#MG6900 series</br>
</br>
	#MG7500 series</br>
	#MG7700 series</br>
</br>
	#MX490 series</br>
</br>
	#TS5000 series</br>
	#TS6000 series</br>
	#TS8000 series</br>
	#TS9000 series</br>
</br>
STATE
</br>
	Currently the backend allow image in A4 format.
	The scan works in color or in gray map(must be change in the code).
	The options are not well handled, so they might not works.
</br>
</br>
KNOWN BUGS
</br>
	*Cancelling a scan cause : 
</br>
		-If the scanner is in usb the operation stay blocked and seems to
		do nothing.
</br>
		-If the scanner is in network the operation stay blocked and will
		loop on writing an message.
	
</br>
	*Do a scan after a abnormal end will, if the device is connected in usb,
		fail in the function sane_start with an IO error.
</br>
</br>
REQUIREMENTS
</br>
	requirements for scangearmp2 in : scangearmp2/README.md
</br>
</br>
LICENSE
</br>
   	licence of scangearmp2 in : scangearmp2/COPYING
</br>
</br>
EXCEPTION
</br>
    * As a special exception, these programs are permissible to link with the
    libraries released as the binary modules, including the librarie "libcncp*".
</br>
    * If you write modifications of your own for these programs, it is your
     choice whether to permit this exception to apply to your modifications.
     If you do not wish that, delete this exception.
	
