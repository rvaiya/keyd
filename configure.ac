AC_PREREQ([2.63b])
AC_INIT([keyd],
  dnl Get the version from Git.  The m4_dquote macro is used instead of quotes to ensure that the
  dnl command is only run once.  The command outputs m4 quotes to prevent incidental expansion (the
  dnl m4_esyscmd macro does not quote the command output itself, so the command output is subject to
  dnl expansion).
  m4_dquote(m4_esyscmd([printf "[%s]" "$(build-aux/git-version-gen .dist-version)"])),
  [https://github.com/rvaiya/keyd/issues],
  [],
  [https://github.com/rvaiya/keyd])
AC_SUBST([CONFIGURE_DEPENDENCIES], ['$(top_srcdir)/build-aux/git-version-gen'])
AC_CONFIG_SRCDIR([src/keyd.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC
AC_MSG_CHECKING([whether C11 is supported])
_keyd_cver=`
  AS_ECHO(["$ac_prog_cc_stdc"]) |
  [sed -e '
    s/^c\([0-9]\{4\}\)$/\1/; t
    s/^c\([8-9][0-9]\)$/19\1/; t
    s/^c\([0-9]\{2\}\)$/20\1/; t
    s/.*/0/
  ']
`
# Assumption: Future versions of the C standard are always backwards compatible.
AS_IF([test "${_keyd_cver}" -ge 2011],
  [AC_MSG_RESULT([yes ($ac_prog_cc_stdc)])],
  [
    _keyd_cver_msg=$ac_prog_cc_stdc
    AS_IF([test "x$ac_prog_cc_stdc" != xno], [_keyd_cver_msg="no ($_keyd_cver_msg)"])
    AC_MSG_RESULT([$_keyd_cver_msg])
    AC_MSG_FAILURE([a C compiler that supports C11 is required])
  ])

AC_CONFIG_HEADERS([autoconfig.h])
AC_PATH_PROG([SCDOC], [scdoc], [])
AS_IF([test -z "$SCDOC"], [AC_MSG_FAILURE([scdoc is required])])
AM_PATH_PYTHON([3.8])

AC_ARG_WITH([vkbd],
  [AS_HELP_STRING(
    [--with-vkbd=ARG],
    [the virtual keyboard driver to use, one of: stdout uinput usb-gadget @<:@default=uinput@:>@])],
  [],
  [with_vkbd=uinput])
AC_SUBST([VKBD], [$with_vkbd])
AM_CONDITIONAL([USB_GADGET], [test "x${with_vkbd}" = xusb-gadget])
AS_IF(
  [test "x${with_vkbd}" = xusb-gadget], [
    AC_PATH_PROG([BASH], [bash], [])
    AS_IF([test -z "$BASH"], [AC_MSG_FAILURE([bash is required for --with-vkbd=usb-gadget])])
  ])

AC_ARG_WITH([systemd-system-unit],
  [AS_HELP_STRING(
    [--with-systemd-system-unit@<:@=DIR@:>@],
    [install systemd system unit files to DIR; 'yes' means autodetect, 'no' or blank means do not
     install any systemd system unit files @<:@default=yes@:>@])],
  [],
  [with_systemd_system_unit=yes])
AS_IF(
  [test "x$with_systemd_system_unit" = xno], [with_systemd_system_unit=],
  [test "x$with_systemd_system_unit" = xyes], [
    m4_ifdef([PKG_CHECK_VAR], [
      PKG_CHECK_VAR([SYSTEMD_SYSTEM_UNIT_DIR], [systemd], [systemdsystemunitdir], [
        with_systemd_system_unit=$SYSTEMD_SYSTEM_UNIT_DIR
      ], [
        AC_MSG_ERROR([m4_normalize([
          failed to locate systemd system unit directory.  If your system does not use systemd,
          please pass --without-systemd-system-unit to $srcdir/configure
        ])])
      ])
    ], [
      AC_MSG_ERROR([m4_normalize([
        unable to detect systemd system unit directory because the pkg-config autoconf macros were
        not available when the configure script was built.  Please install pkg-config and run
        $srcdir/autogen to rebuild $srcdir/configure, then re-run $srcdir/configure.
        Alternatively, pass --without-systemd-system-unit to $srcdir/configure to disable the
        creation and installation of the systemd system unit files
      ])])
    ])
  ])
AC_MSG_CHECKING([whether to install systemd system unit files])
AS_IF([test -n "$with_systemd_system_unit"], [AC_MSG_RESULT([yes, to $with_systemd_system_unit])],
      [AC_MSG_RESULT([no])])
AC_SUBST([systemdsystemunitdir], [$with_systemd_system_unit])
AM_CONDITIONAL([SYSTEMD_SYSTEM_UNIT], [test -n "$with_systemd_system_unit"])

AC_CONFIG_FILES([
  Makefile
])
AC_OUTPUT
