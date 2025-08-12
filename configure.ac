dnl
dnl IBM Omni driver
dnl Copyright (c) International Business Machines Corp., 2002
dnl
dnl This library is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation; either version 2.1 of the License, or
dnl (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl

dnl
dnl TODO:
dnl    - configure needs to be run if XMLParser/.../* changes
dnl    - if parser says no files are changed but build was interrupted then no build
dnl    - make everything in configure.in make sense
dnl    - make check
dnl    - handle xerces XML libraries
dnl    - handle xml4c XML libraries
dnl    - test device is built differently for some reason:
dnl      libtool: link: warning: `-version-info' is ignored for convenience libraries
dnl      because it is noinst_LTLIBRARIES?
dnl    - DitherLibraryExample
dnl      libtool: link: warning: `-version-info' is ignored for convenience libraries
dnl    - configure.in:306: AC_PROG_CPP was called before AC_PROG_CC
dnl    - run strip on installation
dnl    - libtool: install: warning: remember to run `libtool --finish /opt/Omni/lib
dnl    - RedHat package building: $ac_cv_prog_gcc is not set on the first configure
dnl      path!
dnl

dnl Arguments:                    defaults
dnl      --enable-debug           false
dnl      --enable-ef-debug        false
dnl      --enable-cups            true
dnl      --enable-jobdialog       true
dnl      --enable-xmltool	  true
dnl      --enable-xmltool2	  false
dnl      --enable-deviceupdf	  false
dnl      --enable-devicecompile   false
dnl      --enable-devicexml       true
dnl      --enable-testdevice      true
dnl      --enable-gomni-compat    false
dnl      --enable-device-fonts    false

dnl
dnl Process this file with autoconf to produce a configure script.
dnl
PACKAGE="Omni"
dnl automake bug. should be (${PACKAGE}, ${OMNI_MAJOR_VERSION}.${OMNI_MINOR_VERSION}.${OMNI_MICRO_VERSION})
dnl yet another automake bug.  ${pkgdatadir} and ${pkglibdir} use the tar name instead of the package name.
AC_INIT(Omni, 0.9.2, linuxprt@us.ibm.com, Omni)
AC_CONFIG_SRCDIR(BinaryData.cpp)
AC_PREREQ(2.52)
AC_SUBST(PACKAGE)

dnl
dnl Change the root installation prefix
dnl
dnl AC_PREFIX_DEFAULT(/opt/Omni)
AC_PREFIX_DEFAULT(/usr)

dnl
dnl Load in all of the version numbers
dnl
. ./version.mak

AC_SUBST(OMNI_MAJOR_VERSION)
AC_SUBST(OMNI_MINOR_VERSION)
AC_SUBST(OMNI_MICRO_VERSION)
AC_SUBST(OMNI_CURRENT_INTERFACE)
AC_SUBST(OMNI_INTERFACE_AGE)
AC_SUBST(OMNI_BINARY_AGE)
AC_SUBST(OMNI_VERSION)

dnl
dnl libtool versioning
dnl
LT_RELEASE=${OMNI_MAJOR_VERSION}.$OMNI_MINOR_VERSION}
LT_CURRENT=${OMNI_CURRENT_INTERFACE}
LT_REVISION=${OMNI_INTERFACE_AGE}
LT_AGE=${OMNI_BINARY_AGE}
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl
dnl Initialize automake stuff.
dnl
AM_INIT_AUTOMAKE(${PACKAGE}, ${OMNI_VERSION})

dnl
dnl Enable debugging
dnl
AC_ARG_ENABLE(debug,
[  --enable-debug                Turn on debugging],
[
case "${enableval}" in
yes|true)	debug=true							;;
no|false)	debug=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-debug])	;;
esac],
[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)
AC_MSG_RESULT([Compiling for debugging: ${debug}])

dnl
dnl Enable electric fence debugging
dnl
AC_ARG_ENABLE(ef_debug,
[  --enable-ef-debug             Turn on electric fence debugging],
[
case "${enableval}" in
yes|true)	ef_debug=true							;;
no|false)	ef_debug=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-ef-debug])	;;
esac],
[ef_debug=false])
DEBUG_LIBS=
if test x$ef_debug = xtrue; then
	DEBUG_LIBS=-lefence
fi
AC_SUBST(DEBUG_LIBS)

dnl
dnl Test for glib package
dnl
AC_PATH_PROG(GLIB_CONFIG, glib-config)
if test -z "$GLIB_CONFIG"; then
	AC_MSG_ERROR([Please install glib development package. (Perhaps 1.2 version that contains glib-config?)])
fi
GLIB_INCLUDES=`$GLIB_CONFIG --cflags gmodule`
GLIB_LIBS=`$GLIB_CONFIG --libs gmodule`
AC_SUBST(GLIB_INCLUDES)
AC_SUBST(GLIB_LIBS)

dnl
dnl AC_CHECK_LIB (library, function, [action-if-found], [action-if-not-found], [other-libraries])
dnl the default action will add to LIBS variable which everything links to.
dnl other-libraries needs -llib1 -llib2 syntax

dnl
dnl Yet another configure bug.  This one will bypass the following error:
dnl configure: error: conditional "AMDEP" was never defined.
dnl Usually this means the macro was only invoked conditionally.
dnl
dnl It is probably because AC_CHECK_LIB() was used in an if statement for
dnl testing for xerces.
dnl
AC_CHECK_LIB(gmodule,g_module_close)

dnl Checks for vsyslog function.
AC_CHECK_FUNCS(vsyslog)
LIBS="${LIBS} -lc"

dnl
dnl Set up initial support for building an XML library
dnl
BUILD_XML_LIBRARY=

dnl
dnl Test for xml package
dnl
xml_version=
AC_PATH_PROG(XML2_CONFIG, xml2-config)
if test ! -z "$XML2_CONFIG"; then
	xml_version=libxml2
	XML_INCLUDES=`$XML2_CONFIG --cflags`
	XML_LIBS=`$XML2_CONFIG --libs`
fi
if test -z "$xml_version"; then
	AC_PATH_PROG(XML_CONFIG, xml-config)
	if test ! -z "$XML_CONFIG"; then
                xml_version=libxml
		XML_INCLUDES=`$XML_CONFIG --cflags`
		XML_LIBS=`$XML_CONFIG --libs`
        fi
fi
if test -z "$xml_version"; then
	AC_CHECK_LIB(xerces-c1_3,Initialize__16XMLPlatformUtils,[xml_version=xerces1_3])
	if test "$xml_version" = "xerces1_3"; then
		XML_INCLUDES="-I ?"	# @TBD
		XML_LIBS="-l?"          # @TBD
		AC_MSG_ERROR([@TBD])
	fi
fi
dnl @BUG
dnl if test -z "$xml_version"; then
dnl 	AC_CHECK_LIB(xerces-c1_1,Initialize__16XMLPlatformUtils,[xml_version=xml4c3_1_0])
dnl 	if test "$xml_version" = "xml4c3_1_0"; then
dnl 		XML_INCLUDES="-I ?"	# @TBD
dnl 		XML_LIBS="-l?"          # @TBD
dnl		AC_MSG_ERROR([@TBD])
dnl 	fi
dnl fi
if test -z "$xml_version"; then
	AC_MSG_ERROR([Please install an XML development package.])
fi
AC_MSG_RESULT([Found XML development package: ${xml_version}])

AC_SUBST(XML_INCLUDES)
AC_SUBST(XML_LIBS)

dnl
dnl Make sure that the correct parser code is copied to the XMLLib directory.
dnl
filelist_libxml="XMLInterface.hpp XMLInterface.cpp"
filelist_xerces1_3="XMLInterface.hpp XMLInterface.cpp"
filelist_xml4c3_1_0="XMLInterface.hpp XMLInterface.cpp"
case ${xml_version} in
libxml*)	filelist=$filelist_libxml;	dir=libxml		;;
xerces1_3)	filelist=$filelist_xerces1_3;	dir=xerces-c1_3_0-linux	;;
xml4c3_1_0)	filelist=$filelist_xml4c3_1_0;	dir=xml4c3_1_0-linux	;;
*)		AC_MSG_ERROR([Error: should not be here!]); exit	;;
esac
cd XMLLib;
success=1
for i in ${filelist}; do
	if test ! -f ${i}; then
		success=0
		AC_MSG_RESULT([XMLParser setup: Missing: ${i}])
		break
	fi
	cmp -s ${i} ${dir}/${i}
	if test $? != 0; then
		success=0
		AC_MSG_RESULT([XMLParser setup: Different: ${i}])
		break
	fi
done
if test ${success} != 1; then
	for i in ${filelist}; do
		AC_MSG_RESULT([XMLParser setup: Copying: ${dir}/${i}])
		cp ${dir}/${i} .
	done		
fi
cd ..

dnl
dnl Test for CUPS enablement
dnl
AC_ARG_ENABLE(cups,
[  --enable-cups                 Build the cups integration],
[
case "${enableval}" in
yes|true)	cups=true							;;
no|false)	cups=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-cups])	;;
esac],
[cups=true])
BUILD_CUPS=
DEFAULTCUPSEXEROOT=/usr/lib/cups/filter
DEFAULTCUPSDATAROOT=/usr/share/cups/model/omni
if test x$cups = xtrue; then
	BUILD_CUPS=CUPS
	AC_CHECK_HEADERS(cups/cups.h cups/raster.h,,[BUILD_CUPS=])
	AC_CHECK_LIB(cups,ppdOpenFile,[true],[BUILD_CUPS=])
	AC_CHECK_LIB(cups,ppdClose,[true],[BUILD_CUPS=])
	AC_CHECK_LIB(cups,ppdFindAttr,[true],[BUILD_CUPS=])
	AC_CHECK_LIB(cupsimage,cupsRasterOpen,[true],[BUILD_CUPS=],-lcups)
	AC_CHECK_LIB(cupsimage,cupsRasterClose,[true],[BUILD_CUPS=],-lcups)
	AC_CHECK_LIB(cupsimage,cupsRasterReadPixels,[true],[BUILD_CUPS=],-lcups)
	AC_CHECK_LIB(cupsimage,cupsRasterReadHeader,[true],[BUILD_CUPS=],-lcups)
	dnl
	dnl @TODO test CUPS_CSPACE_W
	dnl
	BUILD_CUPS_RESULT=No
	if test ! -z "${BUILD_CUPS}"; then
		CUPS_LIBS="-lcups -lcupsimage"
		BUILD_CUPS_RESULT=Yes
	fi
fi
AC_MSG_RESULT([Building with CUPS: ${BUILD_CUPS_RESULT}])
AC_SUBST(BUILD_CUPS)
AC_SUBST(DEFAULTCUPSEXEROOT)
AC_SUBST(DEFAULTCUPSDATAROOT)
AC_SUBST(CUPS_LIBS)
AM_CONDITIONAL(CUPS, test x$cups = xtrue)

dnl
dnl Test for JobDialog enablement
dnl
AC_ARG_ENABLE(jobdialog,
[  --enable-jobdialog            Build the Job Dialog],
[
case "${enableval}" in
yes|true)	jobdialog=true							;;
no|false)	jobdialog=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-jobdialog])	;;
esac],
[jobdialog=true])
BUILD_JOB_DIALOG=
BUILD_DIALOG_RESULT=No
if test x$jobdialog = xtrue; then
	BUILD_JOB_DIALOG=JobDialog
	BUILD_DIALOG_RESULT=Yes
	JOB_DIALOG_CFLAGS=
	JOB_DIALOG_LFLAGS=
	dnl Test for gtkmm package
	AC_PATH_PROG(GTKMM_CONFIG, gtkmm-config)
	if test -z "$GTKMM_CONFIG"; then
		AC_MSG_WARN([Please install the gtkmm development package to build the JobDialog.])
		BUILD_JOB_DIALOG=
		BUILD_DIALOG_RESULT=No
	else
		JOB_DIALOG_CFLAGS=`$GTKMM_CONFIG --cflags`
		JOB_DIALOG_LFLAGS=`$GTKMM_CONFIG --libs`
	fi
	dnl Test for gtk package
	AC_PATH_PROG(GTK_CONFIG, gtk-config)
	if test -z "$GTK_CONFIG"; then
		AC_MSG_WARN([Please install the gtk development package to build the JobDialog.])
		BUILD_JOB_DIALOG=
		BUILD_DIALOG_RESULT=No
	else
		JOB_DIALOG_CFLAGS="$JOB_DIALOG_CFLAGS "`$GTK_CONFIG --cflags`
		JOB_DIALOG_LFLAGS="$JOB_DIALOG_LFLAGS "`$GTK_CONFIG --libs`
	fi
	dnl Test for sigc++ package
	AC_PATH_PROG(SIGC_CONFIG, sigc-config)
	if test -z "$SIGC_CONFIG"; then
		AC_MSG_WARN([Please install the sigc development package to build the JobDialog.])
		BUILD_JOB_DIALOG=
		BUILD_DIALOG_RESULT=No
	else
		JOB_DIALOG_CFLAGS="$JOB_DIALOG_CFLAGS "`$SIGC_CONFIG --cflags`
		JOB_DIALOG_LFLAGS="$JOB_DIALOG_LFLAGS "`$SIGC_CONFIG --libs`
	fi
fi

AC_MSG_RESULT([Building Omni job dialog: ${BUILD_DIALOG_RESULT}])
AC_SUBST(BUILD_JOB_DIALOG)
AC_SUBST(JOB_DIALOG_CFLAGS)
AC_SUBST(JOB_DIALOG_LFLAGS)
AM_CONDITIONAL(JOB_DIALOG, test x$jobdialog = xtrue)

dnl
dnl Test for DeviceCreationTool enablement
dnl
AC_ARG_ENABLE(xmltool,
[  --enable-xmltool              Build the XML Tool],
[
case "${enableval}" in
yes|true)	xmltool=true							;;
no|false)	xmltool=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-xmltool])	;;
esac],
[xmltool=true])
BUILD_XML_TOOL=
BUILD_XML_TOOL_RESULT=No
if test x$xmltool = xtrue; then
	BUILD_XML_TOOL=DeviceCreationTool
	BUILD_XML_TOOL_RESULT=Yes
	AC_PATH_PROG(PYTHON, python)
	if test -z "$PYTHON"; then
		AC_MSG_WARN([Please install Python to build the XmlTool.])
		BUILD_XML_TOOL=
		BUILD_XML_TOOL_RESULT=No
	else
		if $PYTHON -c "import gtk" 2>/dev/null; then
			true
		else
			AC_MSG_WARN([Python needs gtk to run XmlTool.])
			BUILD_XML_TOOL=
			BUILD_XML_TOOL_RESULT=No
		fi
		if $PYTHON -c "import gtk.glade" 2>/dev/null; then
			true
		else
			AC_MSG_WARN([Python needs gtk.glade to run XmlTool.])
			BUILD_XML_TOOL=
			BUILD_XML_TOOL_RESULT=No
		fi
		if $PYTHON -c "import gobject" 2>/dev/null; then
			true
		else
			AC_MSG_WARN([Python needs gobject to run XmlTool.])
			BUILD_XML_TOOL=
			BUILD_XML_TOOL_RESULT=No
		fi
		if $PYTHON -c "import types" 2>/dev/null; then
			true
		else
			AC_MSG_WARN([Python needs types to run XmlTool.])
			BUILD_XML_TOOL=
			BUILD_XML_TOOL_RESULT=No
		fi
		if $PYTHON -c "from xml.dom.ext.reader import Sax2" 2>/dev/null; then
			true
		else
			AC_MSG_WARN([Python needs PyXML to run XmlTool.])
			BUILD_XML_TOOL=
			BUILD_XML_TOOL_RESULT=No
		fi
	fi
fi

AC_MSG_RESULT([Building Omni XML device creation tool: ${BUILD_XML_TOOL_RESULT}])
AC_SUBST(BUILD_XML_TOOL)
AM_CONDITIONAL(XML_TOOL, test x$xmltool = xtrue)

dnl
dnl Test for DeviceCreationTool.C++ enablement
dnl
AC_ARG_ENABLE(xmltool2,
[  --enable-xmltool2             Build the C++ XML Tool],
[
case "${enableval}" in
yes|true)	xmltool2=true							;;
no|false)	xmltool2=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-xmltool2])	;;
esac],
[xmltool2=false])
BUILD_XML_TOOL2=
BUILD_XML_TOOL2_RESULT=No
if test x$xmltool2 = xtrue; then
	XML_TOOL2_CFLAGS=
	XML_TOOL2_LFLAGS=
	dnl Test for gtkmm package
	AC_PATH_PROG(GTKMM_CONFIG, gtkmm-config)
	if test -z "$GTKMM_CONFIG"; then
		AC_MSG_WARN([Please install the gtkmm development package to build the C++ XmlTool.])
		BUILD_XML_TOOL2=
		BUILD_XML_TOOL2_RESULT=No
	else
		XML_TOOL2_CFLAGS=`$GTKMM_CONFIG --cflags`
		XML_TOOL2_LFLAGS=`$GTKMM_CONFIG --libs`
	fi
	dnl Test for gtk package
	AC_PATH_PROG(GTK_CONFIG, gtk-config)
	if test -z "$GTK_CONFIG"; then
		AC_MSG_WARN([Please install the gtk development package to build the C++ XmlTool.])
		BUILD_XML_TOOL2=
		BUILD_XML_TOOL2_RESULT=No
	else
		XML_TOOL2_CFLAGS="$XML_TOOL2_CFLAGS "`$GTK_CONFIG --cflags`
		XML_TOOL2_LFLAGS="$XML_TOOL2_LFLAGS "`$GTK_CONFIG --libs`
	fi
	dnl Test for sigc++ package
	AC_PATH_PROG(SIGC_CONFIG, sigc-config)
	if test -z "$SIGC_CONFIG"; then
		AC_MSG_WARN([Please install the sigc development package to build the C++ XmlTool.])
		BUILD_XML_TOOL2=
		BUILD_XML_TOOL2_RESULT=No
	else
		XML_TOOL2_CFLAGS="$XML_TOOL2_CFLAGS "`$SIGC_CONFIG --cflags`
		XML_TOOL2_LFLAGS="$XML_TOOL2_LFLAGS "`$SIGC_CONFIG --libs`
	fi
dnl	dnl Test for xml++ package
dnl	AC_PATH_PROG(XMLPP_CONFIG, xml++-config)
dnl	if test -z "$XMLPP_CONFIG"; then
dnl		AC_MSG_WARN([Please install the xml++ development package to build the C++ XmlTool.])
dnl		BUILD_XML_TOOL2=
dnl		BUILD_XML_TOOL2_RESULT=No
dnl	else
dnl		XML_TOOL2_CFLAGS="$XML_TOOL2_CFLAGS "`$XMLPP_CONFIG --cflags`
dnl		XML_TOOL2_LFLAGS="$XML_TOOL2_LFLAGS "`$XMLPP_CONFIG --libs`
dnl	fi
	dnl if we are building the xml tools then build the xml library
	if test ! -z "$BUILD_XML_TOOL2"; then
		BUILD_XML_LIBRARY=XMLLib
	fi
fi

AC_MSG_RESULT([Building Omni C++ XML device creation tool: ${BUILD_XML_TOOL2_RESULT}])
AC_SUBST(BUILD_XML_TOOL2)
AC_SUBST(XML_TOOL2_CFLAGS)
AC_SUBST(XML_TOOL2_LFLAGS)
AM_CONDITIONAL(XML_TOOL2, test x$xmltool2 = xtrue)

dnl
dnl Test for device UPDF enablement
dnl
AC_ARG_ENABLE(deviceupdf,
[  --enable-deviceupdf           Build the devices as libraries],
[
case "${enableval}" in
yes|true)	deviceupdf=true							;;
no|false)	deviceupdf=false						;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-deviceupdf])	;;
esac],
[deviceupdf=false])
BUILD_UPDF_DEVICE=
if test x$deviceupdf = xtrue; then
	BUILD_UPDF_DEVICE=UPDF
fi

AC_SUBST(BUILD_UPDF_DEVICE)

dnl
dnl Test for the VendorSupplied directory
dnl
BUILD_VENDORSUPPLIED=
if test -d VendorSupplied; then
	VENDOR_DIRS=`find VendorSupplied/ -name Makefile.am | wc | awk '{print $1;}'`
	AC_MSG_RESULT([Found ${VENDOR_DIRS} Vendor directories.])
	if test ${VENDOR_DIRS} -gt 0; then
		BUILD_VENDORSUPPLIED=VendorSupplied
	fi
fi

AC_SUBST(BUILD_VENDORSUPPLIED)

dnl
dnl Test for device compilation enablement
dnl
AC_ARG_ENABLE(devicecompile,
[  --enable-devicecompile        Build the devices as libraries],
[
case "${enableval}" in
yes|true)	devicecompile=true							;;
no|false)	devicecompile=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-devicecompile])	;;
esac],
[devicecompile=false])
BUILD_COMPILED_DEVICE=
COMPILED_DEVICES=
if test x$devicecompile = xtrue; then

	BUILD_COMPILED_DEVICE=XMLParser

	dnl
	dnl Read in the list of device directories.
	dnl
	source ./devicesBuild.list

	dnl
	dnl Create a list of device directories
	dnl
	for i in ${DEVICELIST}; do
		DIR="`echo ${i} | sed -e 's/~//g' -e 's/_/ /g'`"
		NOSPACEDIR="`echo ${i} | sed -e 's/~//g' -e 's/_//g'`"

		COMPILED_DEVICES=${COMPILED_DEVICES}" "${NOSPACEDIR}

		#
		# Bug: Autotools cannot handle spaces in filenames!
		#      So create a spaceless directory name equivalent.
		#
		if test "${DIR}" != "${NOSPACEDIR}"; then
			if test ! -L "${NOSPACEDIR}"; then
				echo "Linking \"${DIR}\" to \"${NOSPACEDIR}\""
				ln -s "${DIR}" "${NOSPACEDIR}" 2>/dev/null
			fi
		fi
	done

	dnl if we are building compiled devices then build the xml library
	BUILD_XML_LIBRARY=XMLLib
fi

AC_SUBST(BUILD_COMPILED_DEVICE)
AC_SUBST(COMPILED_DEVICES)

dnl
dnl Test for device XML enablement
dnl
AC_ARG_ENABLE(devicexml,
[  --enable-devicexml            Use XML files directly],
[
case "${enableval}" in
yes|true)	devicexml=true							;;
no|false)	devicexml=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-devicexml])	;;
esac],
[devicexml=true])
BUILD_XML_DEVICE=
if test x$devicexml = xtrue; then
	BUILD_XML_DEVICE=XMLDevice

	dnl if we are building xml device support then build the xml library
	BUILD_XML_LIBRARY=XMLLib
fi

AC_SUBST(BUILD_XML_DEVICE)

dnl
dnl Test for test device enablement
dnl
AC_ARG_ENABLE(testdevice,
[  --enable-testdevice           Build the test device],
[
case "${enableval}" in
yes|true)	testdevice=true							;;
no|false)	testdevice=false						;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-testdevice])	;;
esac],
[testdevice=true])
BUILD_TEST_DEVICE=
if test x$testdevice = xtrue; then
	BUILD_TEST_DEVICE=test
fi

AC_SUBST(BUILD_TEST_DEVICE)

dnl
dnl Should we build the XML library?
dnl
BUILD_XML_LIBRARY_RESULT=No
if test ! -z "$BUILD_XML_LIBRARY"; then
	XML_INCLUDES="${XML_INCLUDES} -I\$(top_srcdir)/${BUILD_XML_LIBRARY}"

	BUILD_XML_LIBRARY_RESULT=Yes
fi
AC_MSG_RESULT([Building Omni XML library: ${BUILD_XML_LIBRARY_RESULT}])
AC_SUBST(BUILD_XML_LIBRARY)

dnl
dnl Specify a configuration file.
dnl
dnl configure.ac:344: warning: do not use m4_patsubst: use patsubst or m4_bpatsubst
AM_CONFIG_HEADER(config.h)

dnl
dnl Compiler options and definitions
dnl
INCLUDES="-I\$(top_srcdir) -I\$(top_srcdir)/hppcl3 ${GLIB_INCLUDES} ${XML_INCLUDES}"

AC_SUBST(INCLUDES)

dnl
dnl Compiler defines
dnl
CDEFINES="-DUSE_STANDARD_NAMES=1"

if test x$debug = xtrue; then
	CDEFINES=${CDEFINES}" -DDEBUG=1 -g"
else
	CDEFINES=${CDEFINES}" -DRETAIL=1"
fi

dnl CDEFINES=${CDEFINES}" -Dbindir=${bindir} -Dsbindir=${sbindir} -Dlibexecdir=${libexecdir} -Ddatadir=${datadir} -Dsysconfdir=${sysconfdir} -Dsharedstatedirdir=${sharedstatedir} -Dlocalstatedirdir=${sharedstatedir} -Dlibdir=${libdir} -Dinfodir=${infodir} -Dmandir=${mandir} -Dincludedir=${includedir} -Dpkgdatadir=${pkgdatadir} -Dpkglibdir=${pkglibdir}"
dnl Yet another autoconf bug. ${pkglibdir} and ${pkgdatadir} are not defined.
dnl
CDEFINES=${CDEFINES}" -DLIBRARY_PATH=${libdir}/${PACKAGE}/ -DSHARE_PATH=${datadir}/${PACKAGE}/ -DBIN_PATH=${bindir}/"

AC_SUBST(CDEFINES)

dnl
dnl Add our standard set of gcc flags if possible
dnl
if test x$ac_cv_prog_gcc = xyes; then
	dnl cc1plus: warning: "-Wmissing-declarations" is valid for C/ObjC but not for C++
        cflags_to_try="-Wall -Wstrict-prototypes -Wmissing-prototypes -fno-builtin -fno-common"
else
	cflags_to_try=
fi
AC_MSG_CHECKING([supported compiler flags])
old_cflags=$CFLAGS
echo
for flag in $cflags_to_try; do
	CFLAGS="$CFLAGS $flag"
	AC_TRY_COMPILE(, [return 0;], [
		echo "   $flag"
		GCFLAGS="$GCFLAGS $flag"
	])
	CFLAGS=$old_cflags
done
AC_MSG_RESULT([ ...done.])

CXXFLAGS="${CDEFINES} ${CFLAGS} ${GCFLAGS}"
AC_SUBST(CXXFLAGS)

dnl
dnl Only compile for dynamic linkage
dnl
AC_DISABLE_STATIC

dnl
dnl Checks for programs.
dnl
AC_PROG_AWK
dnl AC_PROG_CC
AC_PROG_CXX
dnl AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

dnl
dnl Checks for header files.
dnl
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl
dnl Checks for library functions.
dnl
AC_FUNC_MEMCMP
AC_FUNC_MMAP
dnl AC_FUNC_FORK
dnl AC_FUNC_MALLOC
dnl AC_FUNC_STAT
AC_CHECK_FUNCS(mkdir mkfifo putenv select strdup strstr)

dnl
dnl Starting with GCC 3.0, you must link C++ programs against either
dnl libstdc++ (shared by default), or libsupc++ (always static).  If
dnl you care about binary portability between Linux distributions,
dnl you need to either 1) build your own GCC with static C++ libraries
dnl or 2) link using gcc and libsupc++.
dnl
dnl Also, GCC 3.0.x still has problems compiling some code.  You may
dnl or may not have success with it.  USE 3.0.x WITH EXTREME CAUTION!
dnl
dnl Previous versions of GCC do not have the reliance on the stdc++
dnl or g++ libraries, so the extra supc++ library is not needed.
dnl
if test -n "$GCC"; then
	AC_MSG_CHECKING([if libsupc++ is required])

	SUPC="`$CXX -print-file-name=libsupc++.a 2>/dev/null`"
	case "$SUPC" in
	libsupc++.a*)
		# Library not found, so this is and older GCC...
		AC_MSG_RESULT(no)
		;;
	*)
		# This is gcc 3.x, and it knows of libsupc++, so we need it
		LIBS="$LIBS -lsupc++"
		AC_MSG_RESULT(yes)
		;;
	esac
fi

dnl
dnl Foomatic defaults
dnl
DEFAULTFOOMATICROOT=/usr/share/foomatic/db/source
AC_SUBST(DEFAULTFOOMATICROOT)

dnl
dnl Enable gomni backwards compatibility.  This is optional because some
dnl compilers will compile
dnl    void deleteDevice (Device *pDevice);
dnl to
dnl    void deleteDevice__FP6Device (Device *pDevice);
dnl so you cannot have two functions with the same name.
dnl
AC_ARG_ENABLE(gomni_compat,
[  --enable-gomni-compat         Turn on gomni backwards compatibility],
[
case "${enableval}" in
yes|true)	gomni_compat=true							;;
no|false)	gomni_compat=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-gomni-compat])	;;
esac],
[gomni_compat=false])
if test x$gomni_compat = xtrue; then
	AC_DEFINE(HAVE_BACKWARDS_COMPATIBILITY,1,[Maintain backwards compatibility with older gomnis])
fi

dnl
dnl Enable device font support.  This is optional.  This is not alpha level yet.
dnl
AC_ARG_ENABLE(device_fonts,
[  --enable-device-fonts         Turn on device font support.],
[
case "${enableval}" in
yes|true)	device_fonts=true							;;
no|false)	device_fonts=false							;;
*)		AC_MSG_ERROR([bad value ${enableval} for --enable-device-fonts])	;;
esac],
[device_fonts=false])
BUILD_DEVICE_FONT_SUPPORT=
if test x$device_fonts = xtrue; then

	if test -e DeviceFontSandBox; then

		BUILD_DEVICE_FONT_SUPPORT=DeviceFontSandBox
		AC_DEFINE(HAVE_DEVICE_FONT_SUPPORT,1,[Enable device font support])
	fi
fi

AC_SUBST(BUILD_DEVICE_FONT_SUPPORT)

dnl
dnl Show what options we are building
dnl
if test "x$BUILD_COMPILED_DEVICE" = "x"; then
	AC_MSG_RESULT([Not building compiled device support.])
else
	AC_MSG_RESULT([Building compiled device support.])
fi
if test "x$BUILD_XML_DEVICE" = "x"; then
	AC_MSG_RESULT([Not building device XML support.])
else
	AC_MSG_RESULT([Building device XML support.])
fi
if test "x$BUILD_UPDF_DEVICE" = "x"; then
	AC_MSG_RESULT([Not building device UPDF support.])
else
	AC_MSG_RESULT([Building device UPDF support.])
fi
if test "x$BUILD_VENDORSUPPLIED" = "x"; then
	AC_MSG_RESULT([Not using the VendorSupplied directory.])
else
	AC_MSG_RESULT([Using the VendorSupplied directory.])
fi
if test "x$BUILD_DEVICE_FONT_SUPPORT" = "x"; then
	AC_MSG_RESULT([Not building device font support.])
else
	AC_MSG_RESULT([Building device font support.])
fi

dnl
dnl Generate the Makefiles.  We do this because of an autotools bug where
dnl variable substitutions are not allowing within an AC_OUTPUT.  Otherwise
dnl it would be one line.
dnl

AC_CONFIG_FILES(
Makefile
hppcl3/Makefile
Foomatic/Makefile
tools/Makefile
XMLTools/Makefile
)

if test "x$BUILD_XML_LIBRARY" != "x"; then

AC_CONFIG_FILES(
XMLLib/Makefile
)

fi

if test "x$BUILD_UPDF_DEVICE" != "x"; then

AC_CONFIG_FILES(
UPDF/Makefile
)

fi

if test "x$BUILD_XML_DEVICE" != "x"; then

AC_CONFIG_FILES(
XMLDevice/Makefile
)

fi

if test "x$BUILD_JOB_DIALOG" != "x"; then

AC_CONFIG_FILES(
JobDialog/Makefile
)

fi

if test "x$BUILD_XML_TOOL" != "x"; then

AC_CONFIG_FILES(
DeviceCreationTool/Makefile
)

fi

if test "x$BUILD_XML_TOOL2" != "x"; then

AC_CONFIG_FILES(
DeviceCreationTool.C++/Makefile
)

fi

if test "x$BUILD_CUPS" != "x"; then

AC_CONFIG_FILES(
CUPS/Makefile
)

fi

if test "x$BUILD_DEVICE_FONT_SUPPORT" != "x"; then

AC_CONFIG_FILES(
DeviceFontSandBox/Makefile
)

fi

if test "x$BUILD_VENDORSUPPLIED" != "x" -a "x$BUILD_COMPILED_DEVICE" != "x"; then

AC_CONFIG_FILES(
Epson.PDC/Makefile
)

fi

if test "x$BUILD_COMPILED_DEVICE" != "x"; then

AC_CONFIG_FILES(
XMLParser/Makefile
)

fi

if test "x$BUILD_TEST_DEVICE" != "x"; then

AC_CONFIG_FILES(
test/Makefile
)

fi


if test "x$BUILD_COMPILED_DEVICE" != "x"; then

AC_CONFIG_FILES(
dnl @BEGIN-HACK-1
 Brother/Makefile Canon/Makefile Epson/Makefile HPLaserJet/Makefile IBM/Makefile KS/Makefile Kyocera/Makefile Okidata/Makefile Panasonic/Makefile Star/Makefile
dnl @END-HACK-1
)

fi

AC_OUTPUT()
