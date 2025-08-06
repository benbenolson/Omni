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

noinst_PROGRAMS = OmniXMLTool

AM_CXXFLAGS = @XML_TOOL_CFLAGS@ -I /usr/local/include/libxml++-1.0

OmniXMLTool_SOURCES = App.cpp \
                      CommandProperty.cpp \
                      CommandPropertyView.cpp \
                      CompositeNode.cpp \
                      ConnectionProperty.cpp \
                      ConnectionPropertyView.cpp \
                      Controller.cpp \
                      DataProperty.cpp \
                      DataPropertyView.cpp \
                      DefaultJobProperties.cpp \
                      DefaultJobPropertiesView.cpp \
                      DeviceToolSAX.cpp \
                      FormProperty.cpp \
                      FormPropertyView.cpp \
                      GammaTableProperty.cpp \
                      GammaTablePropertyView.cpp \
                      GeneralProperties.cpp \
                      GeneralPropertiesView.cpp \
                      GenericPropertyView.cpp \
                      GenericView.cpp \
                      HeadNode.cpp \
                      HeadNodeView.cpp \
                      MediaProperty.cpp \
                      MediaPropertyView.cpp \
                      Node.cpp \
                      OmniDCTUtils.cpp \
                      OrientationProperty.cpp \
                      OrientationPropertyView.cpp \
                      ParserCommands.cpp \
                      ParserConnections.cpp \
                      ParserDatas.cpp \
                      ParserDevice.cpp \
                      ParserForms.cpp \
                      ParserGammas.cpp \
                      ParserMedias.cpp \
                      ParserOrientations.cpp \
                      ParserPrintModes.cpp \
                      ParserResolutions.cpp \
                      ParserTrays.cpp \
                      PrintModeProperty.cpp \
                      PrintModePropertyView.cpp \
                      ResolutionProperty.cpp \
                      ResolutionPropertyView.cpp \
                      RootNode.cpp \
                      Skeleton.cpp \
                      TrayProperty.cpp \
                      TrayPropertyView.cpp
OmniXMLTool_LDADD   = $(top_builddir)/libomni.la
OmniXMLTool_LDFLAGS = @JOB_DIALOG_LFLAGS@ @DEBUG_LIBS@
