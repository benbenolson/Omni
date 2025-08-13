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
AUTOMAKE_OPTIONS = foreign

DEVICES =	@COMPILED_DEVICES@
SUBDIRS =	hppcl3 \
                @BUILD_XML_LIBRARY@ \
		. \
		@BUILD_COMPILED_DEVICE@ \
		@BUILD_XML_DEVICE@ \
		Foomatic \
		@BUILD_CUPS@ \
		@BUILD_JOB_DIALOG@ \
		@BUILD_UPDF_DEVICE@ \
                @BUILD_TEST_DEVICE@ \
		tools \
		@BUILD_VENDORSUPPLIED@ \
		@BUILD_DEVICE_FONT_SUPPORT@ \
		${DEVICES}

export DEVICES

#@BUG we need a post install hook to be run after all SUBDIRS are processed
#install-exec-hook:
#	-strip $(libdir)/*

pkglib_LTLIBRARIES =	libomni.la

noinst_LTLIBRARIES =	libDitherLibraryExample.la \
			libPDCBlitterClient.la

bin_PROGRAMS = OmniDeviceValid \
               OmniDevices \
               OmniDeviceOptions \
               OmniPDCServer \
               OmniDeviceTester

noinst_PROGRAMS = DeviceTester2 \
                  DeviceTester3 \
                  DeviceTester4 \
                  DeviceTester5

libomni_la_SOURCES = BinaryData.cpp \
                     Capability.cpp \
                     DebugOutput.cpp \
                     Device.cpp \
                     DeviceBlitter.cpp \
                     DeviceCommand.cpp \
                     DeviceConnection.cpp \
                     DeviceCopies.cpp \
                     DeviceData.cpp \
                     DeviceDither.cpp \
                     DeviceForm.cpp \
                     DeviceGamma.cpp \
                     DeviceInfo.cpp \
                     DeviceInstance.cpp \
                     DeviceMedia.cpp \
                     DeviceNUp.cpp \
                     DeviceOrientation.cpp \
                     DeviceOutputBin.cpp \
                     DevicePrintMode.cpp \
                     DeviceResolution.cpp \
                     DeviceScaling.cpp \
                     DeviceSheetCollate.cpp \
                     DeviceSide.cpp \
                     DeviceStitching.cpp \
                     DeviceString.cpp \
                     DeviceTray.cpp \
                     DeviceTrimming.cpp \
                     Enumeration.cpp \
                     GhostscriptInterface.cpp \
                     GplCompression.cpp \
                     GplDitherInstance.cpp \
                     HardCopyCap.cpp \
                     JobProperties.cpp \
                     MonoDither.cpp \
                     Omni.cpp \
                     OmniDevice.cpp \
                     OmniProxy.cpp \
                     PDL.cpp \
                     PrintDevice.cpp \
                     PrinterCommand.cpp \
                     RasterCapabilities.cpp \
                     StringResource.cpp \
                     StringResourceEn.cpp \
                     OmniPDCProxy.cpp \
                     PluggableInstance.cpp \
                     PluggableBlitter.cpp
#                     PDCInterface.cpp

# autotools bug!
# adding the static library does not actually add the functions within the library!
# it also complains: *** Warning: Linking the shared library libomni.la against the
#                    *** static library ./hppcl3/libbitmap.a is not portable!
#libomni_la_LIBADD = $(top_builddir)/hppcl3/libbitmap.a
libomni_la_LIBADD  = $(top_builddir)/hppcl3/CMYKbitmap.lo $(top_builddir)/hppcl3/bitmap.lo $(top_builddir)/XMLLib/libOmniXMLLibrary.la @XML_LIBS@ @GLIB_LIBS@ @GLIB_LIBS@
libomni_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

libPDCBlitterClient_la_SOURCES = PDCBlitterClient.cpp

libDitherLibraryExample_la_SOURCES = DitherLibraryExample.cpp
libDitherLibraryExample_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@ @DEBUG_LIBS@

OmniDeviceValid_SOURCES = OmniDeviceValid.cpp
OmniDeviceValid_LDFLAGS = @GLIB_LIBS@
OmniDeviceValid_LDADD   = $(top_builddir)/libomni.la

OmniDevices_SOURCES = OmniDevices.cpp
OmniDevices_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
OmniDevices_LDADD   = $(top_builddir)/libomni.la

OmniDeviceOptions_SOURCES = OmniDeviceOptions.cpp
OmniDeviceOptions_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
OmniDeviceOptions_LDADD   = $(top_builddir)/libomni.la

OmniPDCServer_SOURCES = OmniServer.cpp
OmniPDCServer_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
OmniPDCServer_LDADD   = $(top_builddir)/libomni.la

OmniDeviceTester_SOURCES = OmniDeviceTester.cpp
OmniDeviceTester_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
OmniDeviceTester_LDADD   = $(top_builddir)/libomni.la

DeviceTester2_SOURCES = DeviceTester2.cpp
DeviceTester2_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
DeviceTester2_LDADD   = $(top_builddir)/libomni.la

DeviceTester3_SOURCES = DeviceTester3.cpp
DeviceTester3_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
DeviceTester3_LDADD   = $(top_builddir)/libomni.la

DeviceTester4_SOURCES = DeviceTester4.cpp
DeviceTester4_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
DeviceTester4_LDADD   = $(top_builddir)/libomni.la

DeviceTester5_SOURCES = DeviceTester5.cpp
DeviceTester5_LDFLAGS = @GLIB_LIBS@ @DEBUG_LIBS@
DeviceTester5_LDADD   = $(top_builddir)/libomni.la

BVT:
	(source setit; cd test; make BVT)

install-data-hook:
#	./OmniDevices --generateXMLFile > ${pkgdatadir}/devices.xml

# add $(DESTDIR)${pkglibdir} to /etc/ld.so.conf
localInstall:
	if ! grep $(DESTDIR)${pkglibdir} /etc/ld.so.conf >/dev/null 2>&1; then \
		/bin/cp /etc/ld.so.conf /etc/ld.so.conf.preomni; \
		echo $(DESTDIR)${pkglibdir} >> /etc/ld.so.conf; \
		ldconfig; \
	fi

# remove $(DESTDIR)${pkglibdir} to /etc/ld.so.conf
localUninstall:
	if grep $(DESTDIR)${pkglibdir} /etc/ld.so.conf >/dev/null 2>&1; then \
		/bin/cp /etc/ld.so.conf /etc/ld.so.conf.postomni; \
		sed -e '\,'$(DESTDIR)${pkglibdir}',d' /etc/ld.so.conf > /tmp/$$; \
		/bin/cp /tmp/$$ /etc/ld.so.conf; \
		/bin/rm /tmp/$$; \
		ldconfig; \
	fi
