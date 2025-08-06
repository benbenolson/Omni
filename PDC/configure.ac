dnl Copyright (c) 2003 International Business Machines
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the "Software"),
dnl to deal in the Software without restriction, including without limitation
dnl the rights to use, copy, modify, merge, publish, distribute, sublicense,
dnl and/or sell copies of the Software, and to permit persons to whom the
dnl Software is furnished to do so, subject to the following conditions:
dnl
dnl The above copyright notice and this permission notice shall be included
dnl in all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
dnl CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
dnl TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
dnl SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

dnl Arguments:                    defaults
dnl      --enable-debug           false
dnl      --enable-ef-debug        false

PACKAGE="PDC"
AC_INIT(PDC, 0.0.1)
AC_CONFIG_SRCDIR(pdc.h)

dnl
dnl Load in all of the version numbers
dnl
. ./version.mak

AC_SUBST(PDC_MAJOR_VERSION)
AC_SUBST(PDC_MINOR_VERSION)
AC_SUBST(PDC_MICRO_VERSION)
AC_SUBST(PDC_CURRENT_INTERFACE)
AC_SUBST(PDC_INTERFACE_AGE)
AC_SUBST(PDC_BINARY_AGE)
AC_SUBST(PDC_VERSION)

dnl
dnl libtool versioning
dnl
LT_RELEASE=${PDC_MAJOR_VERSION}.$PDC_MINOR_VERSION}
LT_CURRENT=${PDC_CURRENT_INTERFACE}
LT_REVISION=${PDC_INTERFACE_AGE}
LT_AGE=${PDC_BINARY_AGE}
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl
dnl Initialize automake stuff.
dnl
AM_INIT_AUTOMAKE(${PACKAGE}, ${PDC_VERSION})

dnl
dnl Enable debugging
dnl
AC_ARG_ENABLE(debug,
[  --enable-debug                Turn on debugging],
[
case "${enableval}" in
yes)	debug=true							;;
true)	debug=true							;;
no)	debug=false							;;
false)	debug=false							;;
*)	AC_MSG_ERROR([bad value ${enableval} for --enable-debug])	;;
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
yes)	ef_debug=true							;;
true)	ef_debug=true							;;
no)	ef_debug=false							;;
false)	ef_debug=false							;;
*)	AC_MSG_ERROR([bad value ${enableval} for --enable-ef-debug])	;;
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
	AC_MSG_ERROR([Please install glib development package.])
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

dnl
dnl Compiler options and definitions
dnl
INCLUDES="-I\$(top_srcdir) ${GLIB_INCLUDES}"

AC_SUBST(INCLUDES)

dnl
dnl Compiler defines
dnl
CDEFINES=

if test x$debug = xtrue; then
	CDEFINES=${CDEFINES}" -DDEBUG=1 -g"
else
	CDEFINES=${CDEFINES}" -DRETAIL=1"
fi

AC_SUBST(CDEFINES)

dnl
dnl Add our standard set of gcc flags if possible
dnl
if test x$ac_cv_prog_gcc = xyes; then
cflags_to_try="-Wall -Wstrict-prototypes -Wmissing-declarations \
-Wmissing-prototypes -fno-builtin -fno-common"
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
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

AC_OUTPUT(Makefile)
