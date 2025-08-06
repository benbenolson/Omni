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

noinst_PROGRAMS    = TestDeviceFontMgr StressDeviceFontMgr Print
noinst_LTLIBRARIES = libdevicefont.la

AM_CPPFLAGS = -I$(top_srcdir)/UPDF

libdevicefont_la_SOURCES = ByteArray.cpp \
                           CommandSequence.cpp \
                           DeviceFont.cpp \
                           DeviceFontException.cpp \
                           DeviceFontMgr.cpp \
                           Event.cpp \
                           FontParser.cpp \
                           Panose.cpp \
                           Parameter.cpp \
                           UPDFFontParser.cpp \
                           XmlFile.cpp
libdevicefont_la_LIBADD  = $(top_builddir)/UPDF/libUPDFExpression.la $(top_builddir)/XMLLib/libOmniXMLLibrary.la
libdevicefont_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

TestDeviceFontMgr_SOURCES = TestDeviceFontMgr.cpp
TestDeviceFontMgr_LDADD   = libdevicefont.la
TestDeviceFontMgr_LDFLAGS = @XML_LIBS@

StressDeviceFontMgr_SOURCES = StressDeviceFontMgr.cpp
StressDeviceFontMgr_LDADD   = libdevicefont.la
StressDeviceFontMgr_LDFLAGS = @XML_LIBS@

Print_SOURCES = Print.cpp
Print_LDADD   = libdevicefont.la
Print_LDFLAGS = @XML_LIBS@
