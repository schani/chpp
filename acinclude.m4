AC_DEFUN(AM_PATH_MYSQL,
[
AC_ARG_WITH(mysql-prefix,[  --with-mysql-prefix=PFX  Prefix where mysql client is installed (optional)],
            mysql_config_prefix="$withval", mysql_config_prefix="")
AC_MSG_CHECKING(for libmysql)

no_mysql=""
if test x$mysql_config_prefix != x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I$mysql_config_prefix/include"
  LIBS="$LIBS -L$mysql_config_prefix/lib/mysql -lmysqlclient -lm"
  AC_TRY_LINK([#include <mysql/mysql.h>
],[mysql_connect(0, 0, 0, 0);], no_mysql="no", no_mysql="yes")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_mysql = x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I/usr/local/include"
  LIBS="$LIBS -L/usr/local/lib/mysql -lmysqlclient -lm"
  AC_TRY_LINK([#include <mysql/mysql.h>
],[mysql_connect(0, 0, 0, 0);], no_mysql="no"
  mysql_config_prefix="/usr/local")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_mysql = x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I/usr/include"
  LIBS="$LIBS -L/usr/lib/mysql -lmysqlclient -lm"
  AC_TRY_LINK([#include <mysql/mysql.h>
],[mysql_connect(0, 0, 0, 0);], no_mysql="no"
  mysql_config_prefix="/usr", no_mysql="yes")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_mysql = xno ; then
  AC_MSG_RESULT(yes)
  MYSQL_CFLAGS="-I$mysql_config_prefix/include"
  MYSQL_LIBS="-L$mysql_config_prefix/lib/mysql -lmysqlclient -lm"
  AC_DEFINE(HAVE_LIBMYSQLCLIENT)
else
  AC_MSG_RESULT(no)
  MYSQL_CFLAGS=""
  MYSQL_LIBS=""
fi
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)
])


AC_DEFUN(AM_PATH_MSQL,
[
AC_ARG_WITH(msql-prefix,[  --with-msql-prefix=PFX  Prefix where msql client is installed (optional)],
            msql_config_prefix="$withval", msql_config_prefix="")
AC_MSG_CHECKING(for libmsql)

no_msql=""
if test x$msql_config_prefix != x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I$msql_config_prefix/include"
  LIBS="$LIBS -L$msql_config_prefix/lib -lmsql"
  AC_TRY_LINK([#include <time.h>
#include <msql.h>
],[msqlConnect(0);], no_msql="no", no_msql="yes")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_msql = x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I/usr/local/Hughes/include"
  LIBS="$LIBS -L/usr/local/Hughes/lib -lmsql"
  AC_TRY_LINK([#include <time.h>
#include <msql.h>
],[msqlConnect(0);], no_msql="no"
  msql_config_prefix="/usr/local/Hughes")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_msql = x ; then
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS -I/usr/Hughes/include"
  LIBS="$LIBS -L/usr/Hughes/lib -lmsql"
  AC_TRY_LINK([#include <time.h>
#include <msql.h>
],[msqlConnect(0);], no_msql="no"
  msql_config_prefix="/usr/Hughes", no_msql="yes")
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
fi
if test x$no_msql = xno ; then
  AC_MSG_RESULT(yes)
  MSQL_CFLAGS="-I$msql_config_prefix/include"
  MSQL_LIBS="-L$msql_config_prefix/lib -lmsql -lm"
  AC_DEFINE(HAVE_LIBMSQL)
else
  AC_MSG_RESULT(no)
  MSQL_CFLAGS=""
  MSQL_LIBS=""
fi
AC_SUBST(MSQL_CFLAGS)
AC_SUBST(MSQL_LIBS)
])
