dnl $Id$
dnl config.m4 for extension datrie

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(datrie, for datrie support,
dnl Make sure that the comment is aligned:
dnl [  --with-datrie             Include datrie support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(datrie, whether to enable datrie support,
dnl Make sure that the comment is aligned:
[  --enable-datrie           Enable datrie support])

if test "$PHP_DATRIE" != "no"; then
  dnl Write more examples of tests here...

  dnl # get library FOO build options from pkg-config output
  dnl AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  dnl AC_MSG_CHECKING(for libfoo)
  dnl if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists foo; then
  dnl   if $PKG_CONFIG foo --atleast-version 1.2.3; then
  dnl     LIBFOO_CFLAGS=`$PKG_CONFIG foo --cflags`
  dnl     LIBFOO_LIBDIR=`$PKG_CONFIG foo --libs`
  dnl     LIBFOO_VERSON=`$PKG_CONFIG foo --modversion`
  dnl     AC_MSG_RESULT(from pkgconfig: version $LIBFOO_VERSON)
  dnl   else
  dnl     AC_MSG_ERROR(system libfoo is too old: version 1.2.3 required)
  dnl   fi
  dnl else
  dnl   AC_MSG_ERROR(pkg-config not found)
  dnl fi
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBDIR, DATRIE_SHARED_LIBADD)
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)

  dnl # --with-datrie -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/trie.h"  # you most likely want to change this
  dnl if test -r $PHP_DATRIE/$SEARCH_FOR; then # path given as parameter
  dnl   TRIE_DIR=$PHP_DATRIE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for datrie files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       DATRIE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$DATRIE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the datrie distribution])
  dnl fi

  dnl # --with-datrie -> add include path
  dnl PHP_ADD_INCLUDE($DATRIE_DIR/include)

  dnl # --with-datrie -> check for lib and symbol presence
  dnl LIBNAME=trie # you may want to change this
  dnl LIBSYMBOL=trie # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DATRIE_DIR/$PHP_LIBDIR, DATRIE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_DATRIELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong datrie lib version or lib not found])
  dnl ],[
  dnl   -L$DATRIE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(DATRIE_SHARED_LIBADD)
  dnl
  dnl check datrie.so
#  DATRIE_DIR=/usr/local/libdatrie
#  AC_MSG_CHECKING([$DATRIE_DIR/lib/libdatrie.so is exists])
#  if test -f "$DATRIE_DIR/lib/libdatrie.so"; then
#    AC_MSG_RESULT([found])
#    else
#    AC_MSG_RESULT([not found])
#    AC_MSG_ERROR([The required $DATRIEDIR/lib/libdatrie.so file was not found.])
#  fi
#  dnl
#  dnl check datrie header file
#  LIBNAME=datrie
#  LIBSYMBOL=trie_new
#  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
#  [
#      PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DATRIE_DIR/lib, DATRIE_SHARED_LIBADD)
#      AC_DEFINE(HAVE_DATRIELIB,1,[libdatrie found and included])
#  ],[
#      AC_MSG_ERROR([wrong libdatrie version or lib not found])
#  ],[
#      -L$DATRIE_DIR/lib -ldatrie
#  ])

  PHP_ADD_INCLUDE(./datrie)
  PHP_SUBST(DATRIE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(datrie, \
  datrie/trie.c \
  datrie/fileutils.c \
  datrie/alpha-map.c \
  datrie/darray.c \
  datrie/tail.c \
  datrie/trie-string.c \
  datrie/dstring.c \
  php_datrie.c, $ext_shared)
  # PHP_NEW_EXTENSION(datrie, php_datrie.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi