#
# IBM Omni driver
# Copyright (c) International Business Machines Corp., 2000
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
# the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

noinst_PROGRAMS    = CopyOmniDeviceBIs InstallOmniDeviceXMLs
pkglib_LTLIBRARIES = libXMLOmniDevice.la
CLEANFILES         =

InstallOmniDeviceXMLs_SOURCES = InstallOmniDeviceXMLs.cpp
InstallOmniDeviceXMLs_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
InstallOmniDeviceXMLs_LDFLAGS = @XML_LIBS@

CopyOmniDeviceBIs_SOURCES = CopyOmniDeviceBIs.cpp
CopyOmniDeviceBIs_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la @XML_LIBS@
CopyOmniDeviceBIs_LDFLAGS =

libXMLOmniDevice_la_SOURCES = XMLDeviceCopies.cpp \
                              XMLDeviceForm.cpp \
                              XMLDeviceMedia.cpp \
                              XMLDeviceNUp.cpp \
                              XMLDeviceOrientation.cpp \
                              XMLDeviceOutputBin.cpp \
                              XMLDevicePrintMode.cpp \
                              XMLDeviceResolution.cpp \
                              XMLDeviceScaling.cpp \
                              XMLDeviceSheetCollate.cpp \
                              XMLDeviceSide.cpp \
                              XMLDeviceStitching.cpp \
                              XMLDeviceTray.cpp \
                              XMLDeviceTrimming.cpp \
                              XMLDeviceBlitter.cpp \
                              XMLDeviceInstance.cpp \
                              XMLDevice.cpp
libXMLOmniDevice_la_LIBADD  = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
libXMLOmniDevice_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@ @XML_LIBS@

install-exec-hook:
	mkdir -p "${DESTDIR}${pkgdatadir}"
	( \
	if test -z "$${DEVICELIST}"; then source ../devicesXML.list; fi; \
	for i in $${DEVICELIST} ; \
	do \
		DIR="`echo $${i} | sed -e 's/~//g' -e 's/_/ /g'`"; \
		if test "$${DIR}" = "test"; then \
			continue; \
		fi; \
		./InstallOmniDeviceXMLs -r "${DESTDIR}${pkgdatadir}" "$${DIR}"; \
	done; \
	)
	cp ../OmniDevice.xsd "${DESTDIR}${pkgdatadir}/.."

test:			testDevice

testDevice:		libXMLOmniDevice.la ${lib_LTLIBRARIES}
	( \
	cd ..; \
	. ./setit XMLDevice; \
	cd XMLDevice; \
	../OmniDeviceOptions jobprop='XMLMasterFile="Epson Stylus Color 760.xml"' driver libXMLOmniDevice.so; \
	)

testDeviceDebug:	libXMLOmniDevice.la ${lib_LTLIBRARIES}
	( \
	cd ..; \
	. ./setit XMLDevice; \
	cd XMLDevice; \
	../OmniDeviceOptions jobprop debugoutput=XMLDeviceInstance jobprop 'XMLMasterFile="Epson Stylus Color 760.xml"' driver libXMLOmniDevice.so; \
	)

testDeviceValgrind:	libXMLOmniDevice.la ${lib_LTLIBRARIES}
	( \
	cd ..; \
	. ./setit XMLDevice; \
	cd XMLDevice; \
	valgrind --leak-check=yes --leak-resolution=high ../OmniDeviceOptions jobprop 'XMLMasterFile="Epson Stylus Color 760.xml"' driver libXMLOmniDevice.so; \
	)

printDevice:		libXMLOmniDevice.la ${lib_LTLIBRARIES}
	( \
	cd ..; \
	. ./setit XMLDevice; \
	cd XMLDevice; \
	../DeviceTester debugoutput XML XMLMasterFile "Epson Stylus Color 760.xml" driver libXMLOmniDevice.so bmp ../beach24.bmp cout output24.prn cerr output24.err; \
	ls -l output24.*; \
	)

testDeviceDialog:	libXMLOmniDevice.la ${lib_LTLIBRARIES}
	( \
	cd ..; \
	. ./setit XMLDevice; \
	cd XMLDevice; \
	../JobDialog/Tester debugoutput XMLDeviceInstance XMLMasterFile "Epson Stylus Color 760.xml" driver libXMLOmniDevice.so; \
	)

include $(srcdir)/libraries1.mak
