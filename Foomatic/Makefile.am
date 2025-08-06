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

bin_PROGRAMS = OmniFoomaticGenerator

OmniFoomaticGenerator_SOURCES = OmniFoomaticGenerator.cpp omni2foo.cpp
OmniFoomaticGenerator_LDADD = $(top_builddir)/libomni.la
OmniFoomaticGenerator_LDFLAGS = @XML_LIBS@ @GLIB_LIBS@ @DEBUG_LIBS@

#OmniFoomaticGenerator_old_2_SOURCES = OmniFoomaticGenerator.old.2.cpp omni2foo.cpp
#OmniFoomaticGenerator_old_2_LDADD = $(top_builddir)/libomni.la
#OmniFoomaticGenerator_old_2_LDFLAGS = @XML_LIBS@ @GLIB_LIBS@ @DEBUG_LIBS@
#
#OmniFoomaticGenerator_old_1_SOURCES = OmniFoomaticGenerator.old.1.cpp omni2foo.cpp
#OmniFoomaticGenerator_old_1_LDADD = $(top_builddir)/libomni.la
#OmniFoomaticGenerator_old_1_LDFLAGS = @XML_LIBS@ @GLIB_LIBS@ @DEBUG_LIBS@

FOOMATICROOT = @DEFAULTFOOMATICROOT@

install-data-hook:
	mkdir -p $(DESTDIR)${pkgdatadir}
	cp share/foo2omni "${DESTDIR}${pkgdatadir}"

generateFoomaticData:		OmniFoomaticGenerator
	rm -rf foomatic-db/
	(	cd ..;	\
		eval . ./setit "`echo ${DEVICES} | sed 's/ test//'`"; \
		cd Foomatic; \
		export OMNI_XML_ROOT_PATH=${pkgdatadir}/; \
		if ! ./OmniFoomaticGenerator -l; then echo "Error: Build break!"; fi \
	)

generateFoomaticData2:		OmniFoomaticGenerator.old.2
	rm -rf foomatic-db/
	(	cd ..;	\
		eval . ./setit "`echo ${DEVICES} | sed 's/ test//'`"; \
		cd Foomatic; \
		export OMNI_XML_ROOT_PATH=${pkgdatadir}/; \
		if ! ./OmniFoomaticGenerator.old.2 -l; then echo "Error: Build break!"; fi \
	)

generateFoomaticData3:		OmniFoomaticGenerator.old.1
	rm -rf foomatic-db/
	(	cd ..;	\
		eval . ./setit "`echo ${DEVICES} | sed 's/ test//'`"; \
		cd Foomatic; \
		export OMNI_XML_ROOT_PATH=${pkgdatadir}/; \
		if ! ./OmniFoomaticGenerator.old.1 -l; then echo "Error: Build break!"; fi \
	)

localInstall:			generateFoomaticData
	mkdir -p $(DESTDIR)${FOOMATICROOT}/{printer,driver,opt}
	cp -a foomatic-db/db/source/printer/* $(DESTDIR)${FOOMATICROOT}/printer/
	cp -a foomatic-db/db/source/driver/* $(DESTDIR)${FOOMATICROOT}/driver/
	cp -a foomatic-db/db/source/opt/* $(DESTDIR)${FOOMATICROOT}/opt/

localInstall2:			generateFoomaticData2
	mkdir -p $(DESTDIR)${FOOMATICROOT}/{printer,driver,opt}
	cp -a foomatic-db/db/source/printer/* $(DESTDIR)${FOOMATICROOT}/printer/
	cp -a foomatic-db/db/source/driver/* $(DESTDIR)${FOOMATICROOT}/driver/
	cp -a foomatic-db/db/source/opt/* $(DESTDIR)${FOOMATICROOT}/opt/

localInstall3:			generateFoomaticData3
	mkdir -p $(DESTDIR)${FOOMATICROOT}/{printer,driver,opt}
	cp -a foomatic-db/db/source/printer/* $(DESTDIR)${FOOMATICROOT}/printer/
	cp -a foomatic-db/db/source/driver/* $(DESTDIR)${FOOMATICROOT}/driver/
	cp -a foomatic-db/db/source/opt/* $(DESTDIR)${FOOMATICROOT}/opt/

CLEANFILES = -r foomatic-db
