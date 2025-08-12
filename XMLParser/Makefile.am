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

noinst_PROGRAMS = parser
noinst_SCRIPTS  = executeParser

CLEANFILES      = Epson_*

parser_SOURCES = OmniDomParser.cpp \
                 Main.cpp
parser_LDADD = $(top_builddir)/libomni.la $(top_builddir)/XMLLib/libOmniXMLLibrary.la @XML_LIBS@
parser_LDFLAGS = @GLIB_LIBS@

# BUG - this executes when make install is called
executeParser:		parser
	./runParser ${MAKECMDGOALS}

# The -a option will return 1 if files are modified and 0 otherwise
test:			parser
	if ./parser -a < "Device List"; then true; fi
