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

pkglib_LTLIBRARIES =
noinst_LTLIBRARIES =
CLEANFILES =

noinst_PROGRAMS = Tester TestGhostscript TestDeviceObjects

Tester_SOURCES = Tester.cpp
Tester_LDADD   = @GLIB_LIBS@ @DEBUG_LIBS@ $(top_builddir)/libomni.la

TestGhostscript_SOURCES = TestGhostscript.cpp
TestGhostscript_LDADD   = @GLIB_LIBS@ @DEBUG_LIBS@ $(top_builddir)/libomni.la

TestDeviceObjects_SOURCES = TestDeviceObjects.cpp
TestDeviceObjects_LDADD   = @GLIB_LIBS@ @DEBUG_LIBS@ $(top_builddir)/libomni.la

BVT:
	(cd ..; ./OmniDevices --buildOnly)
	(cd ..; ./OmniDeviceOptions --buildOnly --All)
	(if ! ./TestDeviceObjects --buildOnly --All; then echo "Error:Build break!"; fi) 2>&1 | tee DeviceObjects.out
	(if ! ./TestGhostscript --buildOnly; then echo "Error:Build break!"; fi) 2>&1 | tee Ghostscript.out

include $(srcdir)/libraries1.mak
include $(srcdir)/libraries2.mak
include $(srcdir)/libraries3.mak
