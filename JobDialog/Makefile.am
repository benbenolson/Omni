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

# @BUG Yet another bug in automake:
# if DEBUG
# CXXFLAGS += -g -DDEBUG=1
# endif
# CXXFLAGS += @JOB_DIALOG_CFLAGS@
#
# yields:
# JobDialog/Makefile.am: CXXFLAGS defined both conditionally and unconditionally

# @BUG this does not work
# libomnijobdialog_la_CXXFLAGS = @CDEFINES@ @JOB_DIALOG_CFLAGS@

AM_CXXFLAGS = @JOB_DIALOG_CFLAGS@

pkglib_LTLIBRARIES = libomnijobdialog.la

noinst_PROGRAMS = Tester

libomnijobdialog_la_SOURCES = ComboBoxHandler.cpp \
                              Driver.cpp \
                              DriverInfo.cpp \
                              DriverProperty.cpp \
                              JobPropertyDialogController.cpp \
                              JobPropertyDialogView.cpp \
                              TextBoxHandler.cpp \
                              WidgetManager.cpp \
                              OmniGSInterface.cpp
libomnijobdialog_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

Tester_SOURCES = Tester.cpp
Tester_LDADD   = libomnijobdialog.la $(top_builddir)/libomni.la $(top_builddir)/XMLLib/libOmniXMLLibrary.la
Tester_LDFLAGS = @JOB_DIALOG_LFLAGS@ @DEBUG_LIBS@
