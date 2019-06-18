AC_INIT(Zertz, 0.4.1)

AM_INIT_AUTOMAKE	

AM_MAINTAINER_MODE	

AM_CONDITIONAL(INSIDE_GNOME_COMMON, true)

PKG_CHECK_MODULES(ZERTZ,libgnomeui-2.0,glib-2.0)
AC_SUBST(ZERTZ_CFLAGS)
AC_SUBST(ZERTZ_LIBS)
	
AC_CONFIG_MACRO_DIRS([m4])	
	
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CPP
AC_STDC_HEADERS
AM_PROG_LIBTOOL
AM_PATH_CHECK

AC_PROG_AWK

AC_SEARCH_LIBS([floor], [m])
AC_SEARCH_LIBS([timer_delete], [rt])
AC_SEARCH_LIBS([subunit_test_fail], [subunit])


AC_PATH_PROG(PKG_CONFIG, pkg-config)
if test "!" -x "$PKG_CONFIG" ; then
   AC_MSG_ERROR([
*** You need the latest pkg-config.
*** Get the latest version of pkg-config from
*** <http://www.freedesktop.org/software/pkgconfig/>.])
fi

AC_SUBST(PKG_CONFIG)

AC_OUTPUT([
Makefile
src/Makefile
test/Makefile
])
		
