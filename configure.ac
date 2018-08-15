# Process this file with autoconf to produce a configure script.
AC_INIT(mg-tools, 3.2.0, consult@minigui.com)
AC_CONFIG_SRCDIR([vbfeditor/vbfeditor.c])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_LSTAT
AC_FUNC_MALLOC

dnl ========================================================================
dnl check for libminigui
threads_version="no"
procs_version="no"
standalone_version="no"

AC_CHECK_HEADERS(minigui/common.h, have_libminigui=yes, foo=bar)
AC_CHECK_HEADERS(freetype/freetype.h, have_libttf=yes, foo=bar)
AC_CHECK_HEADERS(openssl/md5.h, have_openssl=yes, foo=bar)

dnl ========================================================================
dnl check for non-UNIX system
unix="yes"

AC_CHECK_DECLS(__NOUNIX__, unix="no", foo=bar)

AC_ARG_WITH(lang, [  --with-lang=[en/zhcn/zhtw]])

case "$with_lang" in
  zhcn)
    AC_DEFINE(_LANG_ZHCN, 1, 
            [Use text strings for locale zh_CN in GB2312])
    ;;
  zhtw)
    AC_DEFINE(_LANG_ZHTW, 1, 
            [Use text strings for locale zh_TW in BIG5])
    ;;
  *)
    AC_DEFINE(_LANG_EN, 1, 
            [Use text strings for locale en_US in ISO8859-1])
    ;;
esac

dnl ========================================================================
dnl Write Output

if test "$ac_cv_prog_gcc" = "yes"; then
    CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -pipe"
fi

STDIO_TOOLS="common tools mgcfg-trans inner-res-trans cryptool product-id font-tools bdf2upf"

MINIGUI_LIBS=""
MINIGUI_TOOLS=""
if test "x$have_libminigui" != "xno"; then
  PKG_CHECK_MODULES([MINIGUI], [minigui >= 3.0.12])
  MINIGUI_TOOLS="ngux-tools rbfeditor vbfeditor qpfeditor upfeditor image-dumper"
fi

TTF_LIBS=""
TTF_TOOLS=""
if test "x$have_libttf" != "xno"; then
  AC_CHECK_LIB(ttf, TT_FreeType_Version, libttf=yes, libttf=no)
  if test "x$libttf" = "xyes"; then
    TTF_LIBS="-lttf"
    TTF_TOOLS="ttf2qpf ttf2upf"
  fi
fi

AC_SUBST(STDIO_TOOLS)
AC_SUBST(MINIGUI_LIBS)
AC_SUBST(MINIGUI_TOOLS)
AC_SUBST(TTF_LIBS)
AC_SUBST(TTF_TOOLS)

AC_OUTPUT(
    Makefile
    res/Makefile
    tools/Makefile
    font-tools/Makefile
    mgcfg-trans/Makefile
    inner-res-trans/Makefile
    ngux-tools/Makefile
    product-id/Makefile
    cryptool/Makefile
    rbfeditor/Makefile
    vbfeditor/Makefile
    qpfeditor/Makefile
    upfeditor/Makefile
    image-dumper/Makefile
    common/Makefile
    ttf2qpf/Makefile
    ttf2upf/Makefile
    bdf2upf/Makefile
    bdf2rbf/Makefile
)

