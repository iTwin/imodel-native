dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.63])

m4_define([version_macro], m4_normalize(m4_include([VERSION])))

AC_INIT([libxml2],[version_macro])
AC_CONFIG_SRCDIR([entities.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])
AC_CANONICAL_HOST

LIBXML_VERSION=version_macro
LIBXML_MAJOR_VERSION=`echo $LIBXML_VERSION | cut -d . -f 1`
LIBXML_MINOR_VERSION=`echo $LIBXML_VERSION | cut -d . -f 2`
LIBXML_MICRO_VERSION=`echo $LIBXML_VERSION | cut -d . -f 3`
LIBXML_MICRO_VERSION_SUFFIX=

# The oldest minor version we are compatible with.
LIBXML_MINOR_COMPAT=14
LIBXML_CURRENT=`expr $LIBXML_MAJOR_VERSION + $LIBXML_MINOR_VERSION`
LIBXML_AGE=`expr $LIBXML_MINOR_VERSION - $LIBXML_MINOR_COMPAT`
LIBXML_VERSION_INFO=$LIBXML_CURRENT:$LIBXML_MICRO_VERSION:$LIBXML_AGE

LIBXML_VERSION_NUMBER=`expr $LIBXML_MAJOR_VERSION \* 10000 + $LIBXML_MINOR_VERSION \* 100 + $LIBXML_MICRO_VERSION`

if test -d .git ; then
  extra=`git describe 2>/dev/null`
  echo extra=$extra
  if test "$extra" != ""
  then
      LIBXML_VERSION_EXTRA="-GIT$extra"
  fi
fi

AC_SUBST(LIBXML_MAJOR_VERSION)
AC_SUBST(LIBXML_MINOR_VERSION)
AC_SUBST(LIBXML_MICRO_VERSION)
AC_SUBST(LIBXML_VERSION)
AC_SUBST(LIBXML_VERSION_INFO)
AC_SUBST(LIBXML_VERSION_NUMBER)
AC_SUBST(LIBXML_VERSION_EXTRA)

AM_INIT_AUTOMAKE([1.16.3 foreign subdir-objects no-dist-gzip dist-xz])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_PATH_PROG(TAR, tar, /bin/tar)
AC_PATH_PROG(WGET, wget, /usr/bin/wget)
AC_PATH_PROG(XSLTPROC, xsltproc, /usr/bin/xsltproc)
PKG_PROG_PKG_CONFIG

LT_INIT([disable-static])
LT_LIB_M

dnl
dnl We process the AC_ARG_WITH first so that later we can modify
dnl some of them to try to prevent impossible combinations.  This
dnl also allows up so alphabetize the choices
dnl

AC_ARG_WITH(c14n,
[  --with-c14n             Canonical XML 1.0 support (on)])
AC_ARG_WITH(catalog,
[  --with-catalog          XML Catalogs support (on)])
AC_ARG_WITH(debug,
[  --with-debug            debugging module (on)])
AC_ARG_WITH(history,
[  --with-history          history support for xmllint shell (off)])
AC_ARG_WITH(readline,
[  --with-readline[[=DIR]]   use readline in DIR for shell (off)])
AC_ARG_WITH(html,
[  --with-html             HTML parser (on)])
AC_ARG_WITH(http,
[  --with-http             HTTP support (off)])
AC_ARG_WITH(iconv,
[  --with-iconv[[=DIR]]      iconv support (on)])
AC_ARG_WITH(icu,
[  --with-icu              ICU support (off)])
AC_ARG_WITH(iso8859x,
[  --with-iso8859x         ISO-8859-X support if no iconv (on)])
AC_ARG_WITH(lzma,
[  --with-lzma[[=DIR]]       use liblzma in DIR (off)])
AC_ARG_WITH(modules,
[  --with-modules          dynamic modules support (on)])
AC_ARG_WITH(output,
[  --with-output           serialization support (on)])
AC_ARG_WITH(pattern,
[  --with-pattern          xmlPattern selection interface (on)])
AC_ARG_WITH(push,
[  --with-push             push parser interfaces (on)])
AC_ARG_WITH(python,
[  --with-python           Python bindings (on)])
AC_ARG_WITH(reader,
[  --with-reader           xmlReader parsing interface (on)])
AC_ARG_WITH(regexps,
[  --with-regexps          regular expressions support (on)])
AC_ARG_WITH(relaxng,
[  --with-relaxng          RELAX NG support (on)])
AC_ARG_WITH(sax1,
[  --with-sax1             older SAX1 interface (on)])
AC_ARG_WITH(schemas,
[  --with-schemas          XML Schemas 1.0 support (on)])
AC_ARG_WITH(schematron,
[  --with-schematron       Schematron support (on)])
AC_ARG_WITH(threads,
[  --with-threads          multithreading support (on)])
AC_ARG_WITH(thread-alloc,
[  --with-thread-alloc     per-thread malloc hooks (off)])
AC_ARG_WITH(valid,
[  --with-valid            DTD validation support (on)])
AC_ARG_WITH(writer,
[  --with-writer           xmlWriter serialization interface (on)])
AC_ARG_WITH(xinclude,
[  --with-xinclude         XInclude 1.0 support (on)])
AC_ARG_WITH(xpath,
[  --with-xpath            XPath 1.0 support (on)])
AC_ARG_WITH(xptr,
[  --with-xptr             XPointer support (on)])
AC_ARG_WITH(zlib,
[  --with-zlib[[=DIR]]       use libz in DIR (off)])

AC_ARG_WITH(minimum,
[  --with-minimum          build a minimally sized library (off)])
AC_ARG_WITH(legacy,
[  --with-legacy           maximum ABI compatibility (off)])

AC_ARG_WITH(tls,
[  --with-tls              thread-local storage (off)])

dnl
dnl Legacy defaults
dnl
if test "$with_legacy" = "yes"; then
    if test "$with_zlib" = ""; then
        with_zlib=yes
    fi
fi

dnl
dnl hard dependencies on options
dnl
if test "$with_c14n" = "yes"; then
    if test "$with_output" = "no"; then
        echo WARNING: --with-c14n overrides --without-output
    fi
    with_output=yes
    if test "$with_xpath" = "no"; then
        echo WARNING: --with-c14n overrides --without-xpath
    fi
    with_xpath=yes
fi
if test "$with_relaxng" = "yes"; then
    if test "$with_regexps" = "no"; then
        echo WARNING: --with-relaxng overrides --without-regexps
    fi
    with_regexps=yes
    if test "$with_schemas" = "no"; then
        echo WARNING: --with-relaxng overrides --without-schemas
    fi
    with_schemas=yes
fi
if test "$with_schemas" = "yes"; then
    if test "$with_pattern" = "no"; then
        echo WARNING: --with-schemas overrides --without-pattern
    fi
    with_pattern=yes
    if test "$with_regexps" = "no"; then
        echo WARNING: --with-schemas overrides --without-regexps
    fi
    with_regexps=yes
fi
if test "$with_schematron" = "yes"; then
    if test "$with_pattern" = "no"; then
        echo WARNING: --with-schematron overrides --without-pattern
    fi
    with_pattern=yes
    if test "$with_xpath" = "no"; then
        echo WARNING: --with-schematron overrides --without-xpath
    fi
    with_xpath=yes
fi
if test "$with_reader" = "yes"; then
    if test "$with_push" = "no"; then
        echo WARNING: --with-reader overrides --without-push
    fi
    with_push=yes
fi
if test "$with_writer" = "yes"; then
    if test "$with_output" = "no"; then
        echo WARNING: --with-writer overrides --without-output
    fi
    with_output=yes
    if test "$with_push" = "no"; then
        echo WARNING: --with-writer overrides --without-push
    fi
    with_push=yes
fi
if test "$with_xptr" = "yes"; then
    if test "$with_xpath" = "no"; then
        echo WARNING: --with-xptr overrides --without-xpath
    fi
    with_xpath=yes
fi
if test "$with_history" = "yes"; then
    if test "$with_readline" = "no"; then
        echo WARNING: --with-history overrides --without-readline
        with_readline=yes
    fi
    if test "$with_readline" = ""; then
        with_readline=yes
    fi
fi

if test "$with_minimum" = "yes"; then
    dnl
    dnl option to build a minimal libxml2 library
    dnl
    echo "Configuring for a minimal library"
    test "$with_c14n" = "" && with_c14n=no
    test "$with_catalog" = "" && with_catalog=no
    test "$with_debug" = "" && with_debug=no
    test "$with_history" = "" && with_history=no
    test "$with_html" = "" && with_html=no
    test "$with_http" = "" && with_http=no
    test "$with_iconv" = "" && with_iconv=no
    test "$with_iso8859x" = "" && with_iso8859x=no
    test "$with_lzma" = "" && with_lzma=no
    test "$with_output" = "" && with_output=no
    test "$with_pattern" = "" && with_pattern=no
    test "$with_push" = "" && with_push=no
    test "$with_python" = "" && with_python=no
    test "$with_reader" = "" && with_reader=no
    test "$with_readline" = "" && with_readline=no
    test "$with_regexps" = "" && with_regexps=no
    test "$with_relaxng" = "" && with_relaxng=no
    test "$with_sax1" = "" && with_sax1=no
    test "$with_schemas" = "" && with_schemas=no
    test "$with_schematron" = "" && with_schematron=no
    test "$with_threads" = "" && with_threads=no
    test "$with_thread_alloc" = "" && with_thread_alloc=no
    test "$with_valid" = "" && with_valid=no
    test "$with_writer" = "" && with_writer=no
    test "$with_xinclude" = "" && with_xinclude=no
    test "$with_xpath" = "" && with_xpath=no
    test "$with_xptr" = "" && with_xptr=no
    test "$with_zlib" = "" && with_zlib=no
    test "$with_modules" = "" && with_modules=no
else
    dnl
    dnl Disable dependent modules
    dnl
    if test "$with_output" = "no"; then
        with_c14n=no
        with_writer=no
    fi
    if test "$with_pattern" = "no"; then
        with_schemas=no
        with_schematron=no
    fi
    if test "$with_push" = "no"; then
        with_reader=no
        with_writer=no
    fi
    if test "$with_regexps" = "no"; then
        with_relaxng=no
        with_schemas=no
    fi
    if test "$with_schemas" = "no"; then
        with_relaxng=no
    fi
    if test "$with_xpath" = "no"; then
        with_c14n=no
        with_schematron=no
        with_xptr=no
    fi
fi

dnl
dnl Extra flags
dnl
XML_LIBDIR='-L${libdir}'
XML_INCLUDEDIR='-I${includedir}/libxml2'
XML_CFLAGS=""
XML_STATIC_CFLAGS=""
XML_PC_CFLAGS_PRIVATE=""

XML_PRIVATE_LIBS=
XML_PRIVATE_CFLAGS=
XML_PC_LIBS=
XML_PC_REQUIRES=

AX_GCC_FUNC_ATTRIBUTE([destructor])

dnl
dnl Checks for header files.
dnl
AC_CHECK_HEADERS([stdint.h])

dnl Checks for library functions.
AC_CHECK_DECLS([getentropy], [], [], [#include <sys/random.h>])
AC_CHECK_DECLS([glob], [], [], [#include <glob.h>])
AC_CHECK_DECLS([mmap], [], [], [#include <sys/mman.h>])

AM_CONDITIONAL(WITH_GLOB, test "$ac_cv_have_decl_glob" = "yes")

dnl
dnl Checks for inet libraries
dnl
if test "$with_http" = "yes"; then
    AC_CHECK_HEADERS([poll.h])

    case "$host" in
        *-*-mingw*)
            dnl AC_SEARCH_LIBS doesn't work because of non-standard calling
            dnl conventions on 32-bit Windows.
            NET_LIBS="$NET_LIBS -lws2_32"
            ;;
        *)
            _libs=$LIBS
            AC_SEARCH_LIBS(gethostbyname, [nsl], [
                if test "$ac_cv_search_gethostbyname" != "none required"; then
                    NET_LIBS="$NET_LIBS $ac_cv_search_gethostbyname"
                fi], [:], [$NET_LIBS])
            AC_SEARCH_LIBS(connect, [bsd socket inet], [
                if test "$ac_cv_search_connect" != "none required"; then
                    NET_LIBS="$NET_LIBS $ac_cv_search_connect"
                fi], [:], [$NET_LIBS])
            LIBS=$_libs
            ;;
    esac

    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${NET_LIBS}"
    XML_PC_LIBS="${XML_PC_LIBS} ${NET_LIBS}"
fi

dnl Thread-local storage
if test "$with_tls" = "yes"; then
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([_Thread_local int v;]) ], [
        AC_DEFINE([XML_THREAD_LOCAL], [_Thread_local], [TLS specifier]) ], [
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([__thread int v;]) ], [
        AC_DEFINE([XML_THREAD_LOCAL], [__thread], [TLS specifier]) ], [
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([__declspec(thread) int v;]) ], [
        AC_DEFINE([XML_THREAD_LOCAL], [__declspec(thread)], [TLS specifier]) ], [
    ])])])
fi

dnl
dnl Workaround for native compilers
dnl  HP  : http://bugs.gnome.org/db/31/3163.html
dnl  DEC : Enable NaN/Inf
dnl
if test "${GCC}" != "yes" ; then
    case "${host}" in
          hppa*-*-hpux* )
	       AM_CFLAGS="${AM_CFLAGS} -Wp,-H30000"
	       ;;
          *-dec-osf* )
               AM_CFLAGS="${AM_CFLAGS} -ieee"
               ;;
	  alpha*-*-linux* )
	       AM_CFLAGS="${AM_CFLAGS} -ieee"
	       ;;
    esac
else
    # warnings we'd like to see
    AM_CFLAGS="${AM_CFLAGS} -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes"
    # warnings we'd like to suppress
    AM_CFLAGS="${AM_CFLAGS} -Wno-long-long -Wno-format-extra-args -Wno-array-bounds"
    case "${host}" in
          alpha*-*-linux* )
	       AM_CFLAGS="${AM_CFLAGS} -mieee"
	       ;;
	  alpha*-*-osf* )
	       AM_CFLAGS="${AM_CFLAGS} -mieee"
	       ;;
    esac
fi
case ${host} in
    *-*-solaris*)
        XML_LIBDIR="${XML_LIBDIR} -R${libdir}"
        ;;
    *-*-cygwin* | *-*-mingw* | *-*-msys* )
        # If the host is Windows, and shared libraries are disabled, we
        # need to add -DLIBXML_STATIC to AM_CFLAGS in order for linking to
        # work properly (without it, xmlexports.h would force the use of
        # DLL imports, which obviously aren't present in a static
        # library).
        XML_STATIC_CFLAGS="-DLIBXML_STATIC"
        if test "x$enable_shared" = "xyes"; then
            XML_PC_CFLAGS_PRIVATE="
Cflags.private:"
        else
            XML_CFLAGS="$XML_STATIC_CFLAGS"
            AM_CFLAGS="$AM_CFLAGS $XML_STATIC_CFLAGS"
        fi
        ;;
esac


dnl
dnl Simple API modules
dnl

if test "$with_http" != "yes" ; then
    WITH_HTTP=0
else
    echo Enabling HTTP support
    WITH_HTTP=1
fi
AC_SUBST(WITH_HTTP)
AM_CONDITIONAL(WITH_HTTP_SOURCES, test "$WITH_HTTP" = "1")

if test "$with_reader" = "no" ; then
    echo Disabling the xmlReader parsing interface
    WITH_READER=0
else
    WITH_READER=1
fi
AC_SUBST(WITH_READER)
AM_CONDITIONAL(WITH_READER_SOURCES, test "$WITH_READER" = "1")

if test "$with_writer" = "no" ; then
    echo Disabling the xmlWriter saving interface
    WITH_WRITER=0
else
    WITH_WRITER=1
fi
AC_SUBST(WITH_WRITER)
AM_CONDITIONAL(WITH_WRITER_SOURCES, test "$WITH_WRITER" = "1")

if test "$with_pattern" = "no" ; then
    echo Disabling the xmlPattern parsing interface
    WITH_PATTERN=0
else
    WITH_PATTERN=1
fi
AC_SUBST(WITH_PATTERN)
AM_CONDITIONAL(WITH_PATTERN_SOURCES, test "$WITH_PATTERN" = "1")

if test "$with_sax1" = "no" ; then
    echo Disabling the older SAX1 interface
    WITH_SAX1=0
else
    WITH_SAX1=1
fi
AC_SUBST(WITH_SAX1)
AM_CONDITIONAL(WITH_SAX1_SOURCES, test "$WITH_SAX1" = "1")

if test "$with_push" = "no" ; then
    echo Disabling the PUSH parser interfaces
    WITH_PUSH=0
else
    WITH_PUSH=1
fi
AC_SUBST(WITH_PUSH)

if test "$with_html" = "no" ; then
    echo Disabling HTML support
    WITH_HTML=0
else
    WITH_HTML=1
fi
AC_SUBST(WITH_HTML)
AM_CONDITIONAL(WITH_HTML_SOURCES, test "$WITH_HTML" = "1")

if test "$with_valid" = "no" ; then
    echo Disabling DTD validation support
    WITH_VALID=0
else
    WITH_VALID=1
fi
AC_SUBST(WITH_VALID)
AM_CONDITIONAL(WITH_VALID_SOURCES, test "$WITH_VALID" = "1")

if test "$with_catalog" = "no" ; then
    echo Disabling Catalog support
    WITH_CATALOG=0
else
    WITH_CATALOG=1
fi
AC_SUBST(WITH_CATALOG)
AM_CONDITIONAL(WITH_CATALOG_SOURCES, test "$WITH_CATALOG" = "1")

if test "$with_xptr" = "no" ; then
    echo Disabling XPointer support
    WITH_XPTR=0
else
    WITH_XPTR=1
fi
AC_SUBST(WITH_XPTR)
AM_CONDITIONAL(WITH_XPTR_SOURCES, test "$WITH_XPTR" = "1")

if test "$with_c14n" = "no" ; then
    echo Disabling C14N support
    WITH_C14N=0
else
    WITH_C14N=1
fi
AC_SUBST(WITH_C14N)
AM_CONDITIONAL(WITH_C14N_SOURCES, test "$WITH_C14N" = "1")

if test "$with_xinclude" = "no" ; then
    echo Disabling XInclude support
    WITH_XINCLUDE=0
else
    WITH_XINCLUDE=1
fi
AC_SUBST(WITH_XINCLUDE)
AM_CONDITIONAL(WITH_XINCLUDE_SOURCES, test "$WITH_XINCLUDE" = "1")

if test "$with_schematron" = "no" ; then
    echo "Disabling Schematron support"
    WITH_SCHEMATRON=0
else
    WITH_SCHEMATRON=1
fi
AC_SUBST(WITH_SCHEMATRON)
AM_CONDITIONAL(WITH_SCHEMATRON_SOURCES, test "$WITH_SCHEMATRON" = "1")

if test "$with_xpath" = "no" ; then
    echo Disabling XPATH support
    WITH_XPATH=0
else
    WITH_XPATH=1
fi
AC_SUBST(WITH_XPATH)
AM_CONDITIONAL(WITH_XPATH_SOURCES, test "$WITH_XPATH" = "1")

if test "$with_output" = "no" ; then
    echo Disabling serialization/saving support
    WITH_OUTPUT=0
else
    WITH_OUTPUT=1
fi
AC_SUBST(WITH_OUTPUT)
AM_CONDITIONAL(WITH_OUTPUT_SOURCES, test "$WITH_OUTPUT" = "1")

if test "$WITH_ICONV" != "1" && test "$with_iso8859x" = "no" ; then
    echo Disabling ISO8859X support
    WITH_ISO8859X=0
else
    WITH_ISO8859X=1
fi
AC_SUBST(WITH_ISO8859X)

if test "$with_relaxng" = "no" ; then
    echo "Disabling Relax-NG support"
    WITH_RELAXNG=0
else
    WITH_RELAXNG=1
fi
AC_SUBST(WITH_RELAXNG)
AM_CONDITIONAL(WITH_RELAXNG_SOURCES, test "$WITH_RELAXNG" = "1")

if test "$with_schemas" = "no" ; then
    echo "Disabling Schemas support"
    WITH_SCHEMAS=0
else
    WITH_SCHEMAS=1
fi
AC_SUBST(WITH_SCHEMAS)
AM_CONDITIONAL(WITH_SCHEMAS_SOURCES, test "$WITH_SCHEMAS" = "1")

if test "$with_regexps" = "no" ; then
    echo Disabling Regexps support
    WITH_REGEXPS=0
else
    WITH_REGEXPS=1
fi
AC_SUBST(WITH_REGEXPS)
AM_CONDITIONAL(WITH_REGEXPS_SOURCES, test "$WITH_REGEXPS" = "1")

if test "$with_debug" = "no" ; then
    echo Disabling DEBUG support
    WITH_DEBUG=0
else
    WITH_DEBUG=1
fi
AC_SUBST(WITH_DEBUG)
AM_CONDITIONAL(WITH_DEBUG_SOURCES, test "$WITH_DEBUG" = "1")

dnl
dnl Check for Python
dnl

AS_IF([test "x$with_python" != "xno"], [
    AM_PATH_PYTHON
    PKG_CHECK_MODULES([PYTHON], [python-${PYTHON_VERSION}])
])
AM_CONDITIONAL([WITH_PYTHON], [test "x$with_python" != "xno"])

dnl
dnl Extra Python flags for Windows
dnl
PYTHON_LDFLAGS=
if test "${PYTHON}" != ""; then
    case "$host" in
        *-*-mingw* )
            PYTHON_LDFLAGS="-no-undefined -shrext .pyd"
            ;;
        *-*-cygwin* |*-*-msys* )
            PYTHON_LDFLAGS="-no-undefined"
            ;;
    esac
fi
AC_SUBST(PYTHON_LDFLAGS)

dnl
dnl Check for DSO support
dnl
WITH_MODULES=0

if test "$with_modules" != "no" ; then
    case "$host" in
        *-*-cygwin* | *-*-msys* )
            MODULE_EXTENSION=".dll"
            ;;
        *-*-mingw*)
            MODULE_EXTENSION=".dll"
            WITH_MODULES=1
            ;;
        *-*-hpux*)
	    MODULE_EXTENSION=".sl"
	    ;;
        *)
	    MODULE_EXTENSION=".so"
	    ;;
    esac

    if test "$WITH_MODULES" = "0"; then
        _libs=$LIBS
        AC_SEARCH_LIBS([dlopen], [dl], [
            WITH_MODULES=1
            if test "$ac_cv_search_dlopen" != "none required"; then
                MODULE_LIBS=$ac_cv_search_dlopen
            fi
            AC_DEFINE([HAVE_DLOPEN], [], [Have dlopen based dso])], [
            AC_SEARCH_LIBS([shl_load], [dld], [
                WITH_MODULES=1
                if test "$ac_cv_search_shl_load" != "none required"; then
                    MODULE_LIBS=$ac_cv_search_shl_load
                fi
                AC_DEFINE([HAVE_SHLLOAD], [], [Have shl_load based dso])])])
        LIBS=$_libs
    fi

    if test "$WITH_MODULES" = "0"; then
        AC_MSG_ERROR([dlopen or equivalent not found])
    fi

    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${MODULE_LIBS}"
    XML_PC_LIBS="${XML_PC_LIBS} ${MODULE_LIBS}"
fi

AC_SUBST(WITH_MODULES)
AC_SUBST(MODULE_EXTENSION)
AM_CONDITIONAL(WITH_MODULES_SOURCES, test "$WITH_MODULES" = "1")

dnl
dnl Thread-related stuff
dnl
THREAD_LIBS=""
WITH_THREADS=0
WITH_THREAD_ALLOC=0

if test "$with_threads" = "no" ; then
    echo Disabling multithreaded support
else
    case $host_os in
        *mingw*)
            dnl Default to native threads on Windows
            WITH_THREADS="1"
            ;;
        *)
            dnl Use pthread by default in other cases
            _libs=$LIBS
            AC_CHECK_HEADERS(pthread.h,
                AC_SEARCH_LIBS([pthread_create], [pthread], [
                    WITH_THREADS="1"
                    if test "$ac_cv_search_pthread_create" != "none required"; then
                        THREAD_LIBS=$ac_cv_search_pthread_create
                    fi]))
            LIBS=$_libs
            ;;
    esac

    if test "$WITH_THREADS" = "0"; then
        AC_MSG_ERROR([libpthread not found])
    fi

    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${THREAD_LIBS}"
    XML_PC_LIBS="${XML_PC_LIBS} ${THREAD_LIBS}"
fi
if test "$with_thread_alloc" = "yes" && test "$WITH_THREADS" = "1" ; then
    WITH_THREAD_ALLOC=1
fi

AC_SUBST(THREAD_LIBS)
AC_SUBST(WITH_THREADS)
AC_SUBST(WITH_THREAD_ALLOC)

dnl
dnl Checks for readline and history libraries.
dnl
if test "$with_readline" != "no" && test "$with_readline" != ""; then
    WITH_READLINE=0
    WITH_HISTORY=0

    if test "$with_readline" != "yes"; then
        RDL_DIR=$with_readline
    fi

    if test "$RDL_DIR" = ""; then
        PKG_CHECK_MODULES([RDL], [readline], [ WITH_READLINE=1 ], [:])
    fi

    if test "$WITH_READLINE" = "0"; then
        _cppflags=$CPPFLAGS
        _libs=$LIBS

        if test "$RDL_DIR" != ""; then
            CPPFLAGS="${CPPFLAGS} -I$RDL_DIR/include"
            LIBS="${LIBS} -L$RDL_DIR/lib"
        fi
        AC_CHECK_HEADER(readline/readline.h,
            AC_CHECK_LIB(readline, readline, [
                WITH_READLINE=1
                RDL_LIBS="-lreadline"
            ], [
                AC_MSG_ERROR([libreadline not found])
            ]))

        CPPFLAGS=$_cppflags
        LIBS=$_libs
    fi

    AC_DEFINE([HAVE_LIBREADLINE], [],
              [Define if readline library is available])

    if test "$with_history" = "yes"; then
        if test "$RDL_DIR" = ""; then
            PKG_CHECK_MODULES([HISTORY], [history], [
                WITH_HISTORY=1
                RDL_CFLAGS="$HISTORY_CFLAGS $RDL_CFLAGS"
                RDL_LIBS="$HISTORY_LIBS $RDL_LIBS"
            ], [:])
        fi

        if test "$WITH_HISTORY" = "0"; then
            _cppflags=$CPPFLAGS
            _libs=$LIBS

            if test "$RDL_DIR" != ""; then
                CPPFLAGS="${CPPFLAGS} -I$RDL_DIR/include"
                LIBS="${LIBS} -L$RDL_DIR/lib"
            fi
            AC_CHECK_HEADER(readline/history.h,
                AC_CHECK_LIB(history, append_history, [
                    WITH_HISTORY=1
                    RDL_LIBS="-lhistory $RDL_LIBS"
                ], [
                    AC_MSG_ERROR([libhistory not found])
                ]))

            CPPFLAGS=$_cppflags
            LIBS=$_libs
        fi

        AC_DEFINE([HAVE_LIBHISTORY], [],
                  [Define if history library is available])
    fi

    if test "$RDL_DIR" != ""; then
        RDL_CFLAGS="-I$RDL_DIR/include"
        RDL_LIBS="-L$RDL_DIR/lib $RDL_LIBS"
    fi
fi
AC_SUBST(RDL_CFLAGS)
AC_SUBST(RDL_LIBS)

dnl
dnl Checks for zlib library.
dnl
WITH_ZLIB=0

if test "$with_zlib" != "no" && test "$with_zlib" != ""; then
    echo "Enabling zlib compression support"

    if test "$with_zlib" != "yes"; then
        Z_DIR=$with_zlib
    fi

    # Don't run pkg-config if with_zlib contains a path.
    if test "x$Z_DIR" = "x"; then
        # Try pkg-config first so that static linking works.
        PKG_CHECK_MODULES([Z],[zlib],
            [WITH_ZLIB=1; XML_PC_REQUIRES="${XML_PC_REQUIRES} zlib"],
            [:])
    fi

    if test "$WITH_ZLIB" = "0"; then
        _cppflags=$CPPFLAGS
        _libs=$LIBS
        if test "x$Z_DIR" != "x"; then
            CPPFLAGS="${CPPFLAGS} -I$Z_DIR/include"
            LIBS="${LIBS} -L$Z_DIR/lib"
        fi
        AC_CHECK_HEADERS(zlib.h,
            AC_CHECK_LIB(z, gzread,[
                WITH_ZLIB=1
                if test "x${Z_DIR}" != "x"; then
                    Z_CFLAGS="-I${Z_DIR}/include"
                    Z_LIBS="-L${Z_DIR}/lib -lz"
                    [case ${host} in
                        *-*-solaris*)
                            Z_LIBS="-L${Z_DIR}/lib -R${Z_DIR}/lib -lz"
                            ;;
                    esac]
                else
                    Z_LIBS="-lz"
                fi])
                XML_PC_LIBS="${XML_PC_LIBS} ${Z_LIBS}"
            )
        CPPFLAGS=$_cppflags
        LIBS=$_libs
    fi

    if test "$WITH_ZLIB" = "0"; then
        AC_MSG_ERROR([zlib not found])
    fi

    XML_PRIVATE_CFLAGS="${XML_PRIVATE_CFLAGS} ${Z_CFLAGS}"
    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${Z_LIBS}"
fi
AC_SUBST(WITH_ZLIB)

dnl
dnl Checks for lzma library.
dnl
WITH_LZMA=0

if test "$with_lzma" != "no" && test "$with_lzma" != ""; then
    echo "Enabling lzma compression support"

    if test "$with_lzma" != "yes"; then
        LZMA_DIR=$with_lzma
    fi

    # Don't run pkg-config if with_lzma contains a path.
    if test "x$LZMA_DIR" = "x"; then
        # Try pkg-config first so that static linking works.
        PKG_CHECK_MODULES([LZMA],[liblzma],
            [WITH_LZMA=1; XML_PC_REQUIRES="${XML_PC_REQUIRES} liblzma"],
            [:])
    fi

    # If pkg-config failed, fall back to AC_CHECK_LIB. This
    # will not pick up the necessary LIBS flags for liblzma's
    # private dependencies, though, so static linking may fail.
    if test "$WITH_LZMA" = "0"; then
        _cppflags=$CPPFLAGS
        _libs=$LIBS
        if test "x$LZMA_DIR" != "x"; then
            CPPFLAGS="${CPPFLAGS} -I$LZMA_DIR/include"
            LIBS="${LIBS} -L$LZMA_DIR/lib"
        fi
        AC_CHECK_HEADERS(lzma.h,
            AC_CHECK_LIB(lzma, lzma_code,[
                WITH_LZMA=1
                if test "x${LZMA_DIR}" != "x"; then
                    LZMA_CFLAGS="-I${LZMA_DIR}/include"
                    LZMA_LIBS="-L${LZMA_DIR}/lib -llzma"
                else
                    LZMA_LIBS="-llzma"
                fi])
                XML_PC_LIBS="${XML_PC_LIBS} ${LZMA_LIBS}"
            )
        CPPFLAGS=$_cppflags
        LIBS=$_libs
    fi

    if test "$WITH_LZMA" = "0"; then
        AC_MSG_ERROR([liblzma not found])
    fi

    XML_PRIVATE_CFLAGS="${XML_PRIVATE_CFLAGS} ${LZMA_CFLAGS}"
    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${LZMA_LIBS}"
fi
AC_SUBST(WITH_LZMA)
AM_CONDITIONAL(WITH_LZMA_SOURCES, test "$WITH_LZMA" = "1")

dnl
dnl Checks for iconv library.
dnl
WITH_ICONV=0

if test "$with_iconv" = "no" ; then
    echo Disabling ICONV support
else
    _cppflags=$CPPFLAGS
    _libs=$LIBS
    if test "$with_iconv" != "yes" && test "$with_iconv" != "" ; then
	ICONV_DIR=$with_iconv
	CPPFLAGS="$CPPFLAGS -I$ICONV_DIR/include"
	LIBS="$LIBS -L$ICONV_DIR/lib"
    fi
    AC_MSG_CHECKING([for libiconv])
    AC_LINK_IFELSE([
        AC_LANG_PROGRAM([#include <iconv.h>], [iconv_open(0,0);])
    ], [
        WITH_ICONV=1
        AC_MSG_RESULT([none required])
    ], [
        LIBS="$LIBS -liconv"
        AC_LINK_IFELSE([
            AC_LANG_PROGRAM([#include <iconv.h>], [iconv_open(0,0);])
        ], [
            WITH_ICONV=1
            ICONV_LIBS="-liconv"
            AC_MSG_RESULT([yes])
        ], [
            AC_MSG_RESULT([no])
        ])
    ])

    if test "$WITH_ICONV" = "0"; then
        AC_MSG_ERROR([libiconv not found])
    fi

    if test "$WITH_ICONV" = "1" && test "$ICONV_DIR" != ""; then
        ICONV_CFLAGS="-I$ICONV_DIR/include"
        ICONV_LIBS="-L$ICONV_DIR/lib $ICONV_LIBS"
    fi
    CPPFLAGS=$_cppflags
    LIBS=$_libs

    XML_PRIVATE_CFLAGS="${XML_PRIVATE_CFLAGS} ${ICONV_CFLAGS}"
    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${ICONV_LIBS}"
    XML_PC_LIBS="${XML_PC_LIBS} ${ICONV_LIBS}"
fi
AC_SUBST(WITH_ICONV)
AC_SUBST(ICONV_CFLAGS)

dnl
dnl Checks for ICU library.
dnl
WITH_ICU=0

if test "$with_icu" != "no" && test "$with_icu" != "" ; then
    echo Enabling ICU support

    # Try pkg-config first so that static linking works.
    # If this succeeeds, we ignore the WITH_ICU directory.
    PKG_CHECK_MODULES([ICU], [icu-uc], [
        WITH_ICU=1; XML_PC_REQUIRES="${XML_PC_REQUIRES} icu-uc"
        m4_ifdef([PKG_CHECK_VAR],
            [PKG_CHECK_VAR([ICU_DEFS], [icu-uc], [DEFS])])
        if test "x$ICU_DEFS" != "x"; then
            ICU_CFLAGS="$ICU_CFLAGS $ICU_DEFS"
        fi],[:])

    if test "$WITH_ICU" = "0"; then
        ICU_CONFIG=icu-config
        if ${ICU_CONFIG} --cflags >/dev/null 2>&1
        then
            WITH_ICU=1
            ICU_CFLAGS=`${ICU_CONFIG} --cflags`
            ICU_LIBS=`${ICU_CONFIG} --ldflags`
            XML_PC_LIBS="${XML_PC_LIBS} ${ICU_LIBS}"
        else
            _cppflags="${CPPFLAGS}"
            _libs="${LIBS}"
            if test "$with_icu" != "yes" ; then
                ICU_DIR=$with_icu
                CPPFLAGS="${CPPFLAGS} -I$ICU_DIR/include"
                LIBS="${LIBS} -L$ICU_DIR/lib"
            fi

            AC_CHECK_HEADER(unicode/ucnv.h, [
                AC_CHECK_LIB([icucore], [ucnv_open], [
                    WITH_ICU=1
                    ICU_LIBS=-licucore
                    if test "$ICU_DIR" != ""; then
                        ICU_CFLAGS="-I$ICU_DIR/include"
                        ICU_LIBS="-L$ICU_DIR/lib $ICU_LIBS"
                    fi])])
                    XML_PC_LIBS="${XML_PC_LIBS} ${ICU_LIBS}"
            CPPFLAGS=$_cppflags
            LIBS=$_libs
        fi
    fi

    if test "$WITH_ICU" = "0"; then
        AC_MSG_ERROR([ICU not found])
    fi

    XML_PRIVATE_CFLAGS="${XML_PRIVATE_CFLAGS} ${ICU_CFLAGS}"
    XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${ICU_LIBS}"
fi
AC_SUBST(WITH_ICU)

dnl
dnl Crypto libraries
dnl
case "$host" in
    *-*-mingw*)
        CRYPTO_LIBS="-lbcrypt"
        XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${CRYPTO_LIBS}"
        XML_PC_LIBS="${XML_PC_LIBS} ${CRYPTO_LIBS}"
        ;;
esac

XML_LIBS="-lxml2"
XML_PC_LIBS="${XML_PC_LIBS} ${LIBM}"
XML_PRIVATE_LIBS="${XML_PRIVATE_LIBS} ${LIBM}"

dnl When static-only:
dnl * Duplicate xml-config static --libs into --dynamic.
dnl * Fold pkg-config private fields into main fields.
if test "x$enable_shared" = "xno"; then
  XML_PRIVATE_LIBS_NO_SHARED="${XML_PRIVATE_LIBS}"
  XML_PC_PRIVATE=
  XML_PC_LIBS_PRIVATE=
  BUILD_SHARED_LIBS=0
else
  XML_PRIVATE_LIBS_NO_SHARED=
  XML_PC_PRIVATE=".private"
  XML_PC_LIBS_PRIVATE="
Libs.private:"
  BUILD_SHARED_LIBS=1
fi
AC_SUBST(XML_PRIVATE_LIBS_NO_SHARED)
AC_SUBST(XML_PC_PRIVATE)
AC_SUBST(XML_PC_LIBS_PRIVATE)
AC_SUBST(XML_PC_CFLAGS_PRIVATE)
AC_SUBST(BUILD_SHARED_LIBS)
AM_SUBST_NOTMAKE(XML_PRIVATE_LIBS_NO_SHARED)
AM_SUBST_NOTMAKE(XML_PC_PRIVATE)
AM_SUBST_NOTMAKE(XML_PC_LIBS_PRIVATE)
AM_SUBST_NOTMAKE(XML_PC_CFLAGS_PRIVATE)
AM_SUBST_NOTMAKE(BUILD_SHARED_LIBS)

AC_SUBST(XML_PC_LIBS)
AC_SUBST(XML_PC_REQUIRES)
AM_SUBST_NOTMAKE(XML_PC_LIBS)
AM_SUBST_NOTMAKE(XML_PC_REQUIRES)

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_STATIC_CFLAGS)

AC_SUBST(XML_LIBDIR)
AC_SUBST(XML_LIBS)
AC_SUBST(XML_PRIVATE_LIBS)
AC_SUBST(XML_PRIVATE_CFLAGS)
AC_SUBST(XML_INCLUDEDIR)

# for libxml2-config.cmake.in
AX_RECURSIVE_EVAL(["$bindir"], [INSTALL_BINDIR])
AX_RECURSIVE_EVAL(["$includedir"], [INSTALL_INCLUDEDIR])
AX_RECURSIVE_EVAL(["$libdir"], [INSTALL_LIBDIR])
AC_SUBST(INSTALL_BINDIR)
AC_SUBST(INSTALL_INCLUDEDIR)
AC_SUBST(INSTALL_LIBDIR)
AM_SUBST_NOTMAKE(INSTALL_BINDIR)
AM_SUBST_NOTMAKE(INSTALL_INCLUDEDIR)
AM_SUBST_NOTMAKE(INSTALL_LIBDIR)

AX_RECURSIVE_EVAL(["$sysconfdir"], [XML_SYSCONFDIR])
AC_DEFINE_UNQUOTED([XML_SYSCONFDIR], ["$XML_SYSCONFDIR"],
                   [System configuration directory (/etc)])

# keep on one line for cygwin c.f. #130896
AC_CONFIG_FILES([Makefile include/Makefile include/libxml/Makefile include/private/Makefile doc/Makefile doc/devhelp/Makefile example/Makefile fuzz/Makefile python/Makefile python/tests/Makefile xstc/Makefile include/libxml/xmlversion.h libxml-2.0.pc libxml2-config.cmake])
AC_CONFIG_FILES([python/setup.py], [chmod +x python/setup.py])
AC_CONFIG_FILES([xml2-config], [chmod +x xml2-config])
AC_OUTPUT
