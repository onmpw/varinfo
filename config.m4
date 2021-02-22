dnl $Id$
dnl config.m4 for extension varinfo

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(varinfo, for varinfo support,
dnl Make sure that the comment is aligned:
dnl [  --with-varinfo             Include varinfo support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(varinfo, whether to enable varinfo support,
Make sure that the comment is aligned:
[  --enable-varinfo           Enable varinfo support])

if test "$PHP_VARINFO" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-varinfo -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/varinfo.h"  # you most likely want to change this
  dnl if test -r $PHP_VARINFO/$SEARCH_FOR; then # path given as parameter
  dnl   VARINFO_DIR=$PHP_VARINFO
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for varinfo files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       VARINFO_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$VARINFO_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the varinfo distribution])
  dnl fi

  dnl # --with-varinfo -> add include path
  dnl PHP_ADD_INCLUDE($VARINFO_DIR/include)

  dnl # --with-varinfo -> check for lib and symbol presence
  dnl LIBNAME=varinfo # you may want to change this
  dnl LIBSYMBOL=varinfo # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $VARINFO_DIR/$PHP_LIBDIR, VARINFO_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_VARINFOLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong varinfo lib version or lib not found])
  dnl ],[
  dnl   -L$VARINFO_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(VARINFO_SHARED_LIBADD)

  PHP_NEW_EXTENSION(varinfo, varinfo.c, $ext_shared)
fi
