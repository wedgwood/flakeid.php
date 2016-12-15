dnl $Id$
dnl config.m4 for extension flakeid

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(flakeid, for flakeid support,
dnl Make sure that the comment is aligned:
[  --with-flakeid             Include flakeid support])

dnl Otherwise use enable:

dnl  PHP_ARG_ENABLE(flakeid, whether to enable flakeid support,
dnl  Make sure that the comment is aligned:
dnl  [  --enable-flakeid           Enable flakeid support])

if test "$PHP_FLAKEID" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-flakeid -> check with-path
  SEARCH_PATH="./libflakeid /usr/local /usr"
  SEARCH_FOR="/include/flakeid.h"
  if test -r $PHP_FLAKEID/$SEARCH_FOR; then
   FLAKEID_DIR=$PHP_FLAKEID
  else # search default path list
   AC_MSG_CHECKING([for flakeid files in default path])
   for i in $SEARCH_PATH ; do
     if test -r $i/$SEARCH_FOR; then
       FLAKEID_DIR=$i
       AC_MSG_RESULT(found in $i)
     fi
   done
  fi

  if test -z "$FLAKEID_DIR"; then
   AC_MSG_RESULT([not found])
   AC_MSG_ERROR([Please reinstall the flakeid distribution])
  fi

  dnl # --with-flakeid -> add include path
  PHP_ADD_INCLUDE($FLAKEID_DIR/include)

  dnl # --with-flakeid -> check for lib and symbol presence
  LIBNAME=flakeid

  PHP_CHECK_LIBRARY($LIBNAME,flakeid_generate,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FLAKEID_DIR/lib, FLAKEID_SHARED_LIBADD)
    AC_DEFINE(HAVE_FLAKEIDLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong flakeid lib version or lib not found])
  ],[
    -L$FLAKEID_DIR/lib -lm
  ])

  dnl  PHP_CHECK_LIBRARY($LIBNAME,flakeid_ctx_create,
  dnl  [
    dnl  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FLAKEID_DIR/lib, FLAKEID_SHARED_LIBADD)
    dnl  AC_DEFINE(HAVE_FLAKEIDLIB,1,[ ])
  dnl  ],[
    dnl  AC_MSG_ERROR([wrong flakeid lib version or lib not found])
  dnl  ],[
    dnl  -L$FLAKEID_DIR/lib -lm
  dnl  ])

  dnl  PHP_CHECK_LIBRARY($LIBNAME,flakeid_ctx_create_with_spoof,
  dnl  [
    dnl  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FLAKEID_DIR/lib, FLAKEID_SHARED_LIBADD)
    dnl  AC_DEFINE(HAVE_FLAKEIDLIB,1,[ ])
  dnl  ],[
    dnl  AC_MSG_ERROR([wrong flakeid lib version or lib not found])
  dnl  ],[
    dnl  -L$FLAKEID_DIR/lib -lm
  dnl  ])

  PHP_SUBST(FLAKEID_SHARED_LIBADD)
  PHP_NEW_EXTENSION(flakeid, flakeid.c, $ext_shared)
fi
