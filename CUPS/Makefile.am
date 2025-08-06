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

bin_PROGRAMS = CUPSToOmni GenerateOmniPPDs GenerateOmniPPD

CUPSEXEROOT  = @DEFAULTCUPSEXEROOT@
CUPSDATAROOT = @DEFAULTCUPSDATAROOT@

CUPSToOmni_SOURCES = CUPSToOmni.cpp
CUPSToOmni_LDFLAGS = @GLIB_LIBS@ @CUPS_LIBS@ @DEBUG_LIBS@
CUPSToOmni_LDADD   = $(top_builddir)/libomni.la

GenerateOmniPPDs_SOURCES = GenerateOmniPPDs.cpp
GenerateOmniPPDs_LDFLAGS = @GLIB_LIBS@ @CUPS_LIBS@ @DEBUG_LIBS@
GenerateOmniPPDs_LDADD   = $(top_builddir)/libomni.la

GenerateOmniPPD_SOURCES = GenerateOmniPPD.cpp
GenerateOmniPPD_LDFLAGS = @GLIB_LIBS@ @CUPS_LIBS@ @DEBUG_LIBS@
GenerateOmniPPD_LDADD   = $(top_builddir)/libomni.la

generateBuildPPDs:		${bin_PROGRAMS}
	(	rm -rf PPDs; \
		cd ..; \
		eval . ./setit "`echo ${DEVICES} | sed 's/ test//'`"; \
		cd CUPS; \
		export OMNI_XML_ROOT_PATH=${pkgdatadir}/; \
		./GenerateOmniPPDs --buildOnly; \
		if test $$? != 0; then echo "Error: Build break!"; fi; \
		touch gen-stamp; \
	)

generateInstallPPDs:		GenerateOmniPPD GenerateOmniPPDs
	rm -rf PPDs
	./GenerateOmniPPDs;

localInstall:
	/etc/rc.d/init.d/cups restart

install-exec-hook:		${bin_PROGRAMS}
	mkdir -p $(DESTDIR)${CUPSEXEROOT}
	cp $(DESTDIR)${bindir}/CUPSToOmni* $(DESTDIR)${CUPSEXEROOT}

#
# @NOTE - call "make generateBuildPPDs" before calling "make install"!
#
install-data-hook:
	mkdir -p $(DESTDIR)${CUPSDATAROOT}
	(	if ! test -d PPDs; then echo "Error: Install break!"; echo "make generateBuildPPDs first!"; exit; fi; \
		cd PPDs; \
		for i in `find . -type d -mindepth 1 | xargs echo`; do \
			cd $$i; \
			if test ! -d $(DESTDIR)${CUPSDATAROOT}/$$i; then mkdir -p $(DESTDIR)${CUPSDATAROOT}/$$i; fi; \
			echo cp * $(DESTDIR)${CUPSDATAROOT}/$$i; \
			cp * $(DESTDIR)${CUPSDATAROOT}/$$i; \
			cd ..; \
		done; \
	)
