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

noinst_PROGRAMS    = ParameterConverterTester # UPDFDeviceTester
pkglib_LTLIBRARIES = libUPDFOmniDevice.la # is not ready yet...
noinst_LTLIBRARIES = libUPDFExpression.la # libUPDFOmniDevice.la

ParameterConverterTester_SOURCES = ParameterConverterTester.cpp
ParameterConverterTester_LDADD   = libUPDFExpression.la

# UPDFDeviceTester_SOURCES = UPDFDeviceTester.cpp
# UPDFDeviceTester_LDADD = $(top_builddir)/libomni.la
# UPDFDeviceTester_LDFLAGS = @GLIB_LIBS@

libUPDFOmniDevice_la_SOURCES = UPDFDeviceBlitter.cpp \
                               UPDFDeviceCopies.cpp \
                               UPDFDeviceForm.cpp \
                               UPDFDeviceInstance.cpp \
                               UPDFDeviceMedia.cpp \
                               UPDFDeviceNUp.cpp \
                               UPDFDeviceOrientation.cpp \
                               UPDFDeviceOutputBin.cpp \
                               UPDFDevicePrintMode.cpp \
                               UPDFDeviceResolution.cpp \
			       UPDFDeviceSheetCollate.cpp \
                               UPDFDeviceSide.cpp \
                               UPDFDeviceTray.cpp \
                               UPDFDeviceTrimming.cpp \
                               UPDFDevice.cpp
libUPDFOmniDevice_la_LIBADD  = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
libUPDFOmniDevice_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@ @XML_LIBS@

libUPDFExpression_la_SOURCES = Expression.cpp \
                               ExpressionException.cpp \
                               ParameterConverter.cpp

test:			testParameters testDevice

testParameters:		ParameterConverterTester
	./ParameterConverterTester

testDevice:		libUPDFOmniDevice.la
	( \
        cd ..; \
	source ./setit UPDF~; \
	cd UPDF; \
	OmniDeviceTester -sproperties='debugoutput=all UPDFMasterFile="UPDFDeviceConfiguration-HPLaserJet9000.xml"' --driver UPDFOmniDevice	\
	)

testDevice2:		libUPDFOmniDevice.la
	( \
        cd ..; \
	source ./setit UPDF~; \
	cd UPDF; \
	OmniDeviceOptions --Verbose -sproperties='UPDFMasterFile="UPDFDeviceConfiguration-HPLaserJet9000.xml"' --driver UPDFOmniDevice	\
	)
