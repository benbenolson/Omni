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

noinst_PROGRAMS = ConvertFromDTDtoSchema \
                  FixXMLFiles \
                  FixXMLFiles2 \
                  FixXMLFiles3 \
                  FixXMLFiles4 \
                  FixXMLFiles5 \
                  FixXMLFiles6 \
                  FixXMLFiles7 \
                  ModifyXMLFiles

ConvertFromDTDtoSchema_SOURCES = ConvertFromDTDtoSchema.cpp

FixXMLFiles_SOURCES = FixXMLFiles.cpp
FixXMLFiles_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

FixXMLFiles2_SOURCES = FixXMLFiles2.cpp
FixXMLFiles2_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles2_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

FixXMLFiles3_SOURCES = FixXMLFiles3.cpp
FixXMLFiles3_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles3_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

FixXMLFiles4_SOURCES = FixXMLFiles4.cpp
FixXMLFiles4_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles4_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

FixXMLFiles5_SOURCES = FixXMLFiles5.cpp
FixXMLFiles5_LDADD   = $(top_builddir)/libomni.la
FixXMLFiles5_LDFLAGS = @XML_LIBS@ @GLIB_LIBS@ @DEBUG_LIBS@

FixXMLFiles6_SOURCES = FixXMLFiles6.cpp
FixXMLFiles6_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles6_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

FixXMLFiles7_SOURCES = FixXMLFiles7.cpp
FixXMLFiles7_LDADD   = $(top_builddir)/XMLLib/libOmniXMLLibrary.la
FixXMLFiles7_LDFLAGS = @XML_LIBS@ @DEBUG_LIBS@

ModifyXMLFiles_SOURCES = ModifyXMLFiles.cpp

test2:	FixXMLFiles2
	cp "../Epson/Epson Stylus Color 760.xml" .
	cp "../Canon/Canon BJ F800.xml" .
	./FixXMLFiles2

test3:	FixXMLFiles3 copyCanonFiles
	./FixXMLFiles3

test4:	FixXMLFiles4 copyCanonFiles
	./FixXMLFiles4

test5:	FixXMLFiles5 copyEpsonFiles
	./FixXMLFiles5

test6:	FixXMLFiles6 copyEpsonFiles2
	./FixXMLFiles6

test7:	FixXMLFiles7 copyEpsonFiles2
	./FixXMLFiles7

copyCanonFiles:
	cp "../Canon/Canon BJ F800 Data.xml"         .
	cp "../Canon/Canon BJ F800 Gammas.xml"       .
	cp "../Canon/Canon BJ F800.xml"              .
	cp "../Canon/Canon BJ-30 Trays.xml"          .
	cp "../Canon/Canon BJC-50v Forms.xml"        .
	cp "../Canon/Canon BJC-70 PrintModes.xml"    .
	cp "../Canon/Canon BJC-7000 Resolutions.xml" .
	cp "../Canon/Canon BJC-700J Commands.xml"    .
	cp "../Canon/Canon BJC-700J Connections.xml" .
	cp "../Canon/Canon BJC-700J Medias.xml"      .
	cp "../Canon/Canon Orientations.xml"         .

copyEpsonFiles:
	cp "../XMLParser/Epson Copies.xml"                       .
	cp "../XMLParser/Epson DLQ-3000+ PrintModes.xml"         .
	cp "../XMLParser/Epson Generic ESC2P Commands.xml"       .
	cp "../XMLParser/Epson NUps.xml"                         .
	cp "../XMLParser/Epson Orientations.xml"                 .
	cp "../XMLParser/Epson Output Bins.xml"                  .
	cp "../XMLParser/Epson Scalings.xml"                     .
	cp "../XMLParser/Epson Sheet Collates.xml"               .
	cp "../XMLParser/Epson Sides.xml"                        .
	cp "../XMLParser/Epson Stitchings.xml"                   .
	cp "../XMLParser/Epson Strings.xml"                      .
	cp "../XMLParser/Epson Stylus 200 Medias.xml"            .
	cp "../XMLParser/Epson Stylus 200 Trays.xml"             .
	cp "../XMLParser/Epson Stylus Color 660 Connections.xml" .
	cp "../XMLParser/Epson Stylus Color 660 Forms.xml"       .
	cp "../XMLParser/Epson Stylus Color 660 Resolutions.xml" .
	cp "../XMLParser/Epson Stylus Color 760 Data.xml"        .
	cp "../XMLParser/Epson Stylus Color 760 Gammas.xml"      .
	cp "../XMLParser/Epson Stylus Color 760.xml"             .
	cp "../XMLParser/Epson Trimmings.xml"                    .

copyEpsonFiles2:
	cp "../XMLParser/Epson Copies.xml"                              .
	cp "../Epson/Epson DLQ-3000+ PrintModes.xml"                    .
	cp "../Epson/Epson Generic ESC2P Commands.xml"                  .
	cp "../XMLParser/Epson NUps.xml"                                .
	cp "../Epson/Epson Orientations.xml"                            .
	cp "../XMLParser/Epson Output Bins.xml"                         .
	cp "../XMLParser/Epson Scalings.xml"                            .
	cp "../XMLParser/Epson Sheet Collates.xml"                      .
	cp "../XMLParser/Epson Sides.xml"                               .
	cp "../XMLParser/Epson Stitchings.xml"                          .
	cp "../XMLParser/Epson Strings.xml"                             .
	cp "../Epson/Epson Stylus 200 Medias.xml"                       .
	cp "../Epson/Epson Stylus 200 Trays.xml"                        .
	cp "../Epson/Epson Stylus Color 660 Connections.xml"            .
	cp "../Epson/Epson Stylus Color 660 Forms.xml"                  .
	cp "../Epson/Epson Stylus Color High Res ESCP2 Resolutions.xml" .
	cp "../Epson/Epson Stylus Color 760 Data.xml"                   .
	cp "../Epson/Epson Stylus Color 760 High Res Gammas.xml"        .
	cp "../Epson/Epson Stylus Color 760.xml"                        .
	cp "../XMLParser/Epson Trimmings.xml"                           .

diffCanonFiles:
	diff "../Canon/Canon BJ F800 Data.xml"         .
	diff "../Canon/Canon BJ F800 Gammas.xml"       .
	diff "../Canon/Canon BJ F800.xml"              .
	diff "../Canon/Canon BJ-30 Trays.xml"          .
	diff "../Canon/Canon BJC-50v Forms.xml"        .
	diff "../Canon/Canon BJC-70 PrintModes.xml"    .
	diff "../Canon/Canon BJC-7000 Resolutions.xml" .
	diff "../Canon/Canon BJC-700J Commands.xml"    .
	diff "../Canon/Canon BJC-700J Connections.xml" .
	diff "../Canon/Canon BJC-700J Medias.xml"      .
	diff "../Canon/Canon Orientations.xml"         .

diffEpsonFiles:
	diff "../XMLParser/Epson Copies.xml"                       .
	diff "../XMLParser/Epson DLQ-3000+ PrintModes.xml"         .
	diff "../XMLParser/Epson Generic ESC2P Commands.xml"       .
	diff "../XMLParser/Epson NUps.xml"                         .
	diff "../XMLParser/Epson Orientations.xml"                 .
	diff "../XMLParser/Epson Output Bins.xml"                  .
	diff "../XMLParser/Epson Scalings.xml"                     .
	diff "../XMLParser/Epson Sheet Collates.xml"               .
	diff "../XMLParser/Epson Sides.xml"                        .
	diff "../XMLParser/Epson Stitchings.xml"                   .
	diff "../XMLParser/Epson Strings.xml"                      .
	diff "../XMLParser/Epson Stylus 200 Medias.xml"            .
	diff "../XMLParser/Epson Stylus 200 Trays.xml"             .
	diff "../XMLParser/Epson Stylus Color 660 Connections.xml" .
	diff "../XMLParser/Epson Stylus Color 660 Forms.xml"       .
	diff "../XMLParser/Epson Stylus Color 660 Resolutions.xml" .
	diff "../XMLParser/Epson Stylus Color 760 Data.xml"        .
	diff "../XMLParser/Epson Stylus Color 760 Gammas.xml"      .
	diff "../XMLParser/Epson Stylus Color 760.xml"             .
	diff "../XMLParser/Epson Trimmings.xml"                    .

diffEpsonFiles2:
	diff "../XMLParser/Epson Copies.xml"                              .
	diff "../Epson/Epson DLQ-3000+ PrintModes.xml"                    .
	diff "../Epson/Epson Generic ESC2P Commands.xml"                  .
	diff "../XMLParser/Epson NUps.xml"                                .
	diff "../Epson/Epson Orientations.xml"                            .
	diff "../XMLParser/Epson Output Bins.xml"                         .
	diff "../XMLParser/Epson Scalings.xml"                            .
	diff "../XMLParser/Epson Sheet Collates.xml"                      .
	diff "../XMLParser/Epson Sides.xml"                               .
	diff "../XMLParser/Epson Stitchings.xml"                          .
	diff "../XMLParser/Epson Strings.xml"                             .
	diff "../Epson/Epson Stylus 200 Medias.xml"                       .
	diff "../Epson/Epson Stylus 200 Trays.xml"                        .
	diff "../Epson/Epson Stylus Color 660 Connections.xml"            .
	diff "../Epson/Epson Stylus Color 660 Forms.xml"                  .
	diff "../Epson/Epson Stylus Color High Res ESCP2 Resolutions.xml" .
	diff "../Epson/Epson Stylus Color 760 Data.xml"                   .
	diff "../Epson/Epson Stylus Color 760 High Res Gammas.xml"        .
	diff "../Epson/Epson Stylus Color 760.xml"                        .
	diff "../XMLParser/Epson Trimmings.xml"                           .
