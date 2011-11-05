# Initialize autoconf

AC_PREREQ([2.68])
AC_INIT([netnotify-daemon], [0.0.4], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/netnotify-daemon.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_INSTALL

# Checks for libraries.
REQ_GLIB_VERSION=2.16
REQ_DBUS_VERSION=0.88

pkg_modules="glib-2.0 >= $REQ_GLIB_VERSION, gio-2.0 >= $REQ_GLIB_VERSION, dbus-1 >= $REQ_DBUS_VERSION, dbus-glib-1 >= $REQ_DBUS_VERSION"
PKG_CHECK_MODULES(NETNOTIFY_DAEMON, $pkg_modules)
AC_SUBST(NETNOTIFY_DAEMON_CFLAGS)
AC_SUBST(NETNOTIFY_DAEMON_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([netdb.h stdlib.h])

# D-bus stuff
AS_AC_EXPAND(DATADIR, $datadir)
DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
AC_SUBST(DBUS_SERVICES_DIR)
AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for DBUS is])

AS_AC_EXPAND(BINPATH, $bindir)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 data/Makefile
                 data/org.freedesktop.Notifications.service])
AC_OUTPUT
