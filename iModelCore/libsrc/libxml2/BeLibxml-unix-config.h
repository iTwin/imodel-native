/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|  See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*
 * Normally this header file is generated through the GNU "./configure" process.  However, this causes
 * problems for cross-compilation.  This file originated by copying "config.h.in" and then modifying
 * #defines as appropriate for Unix (Android first, then iOS).  The starting point is that everything
 * is undefined.
 *
 * See Shaun Sewall if you have any questions.
 */
#undef PACKAGE
#undef VERSION
#undef HAVE_LIBZ
#undef HAVE_LIBM
#undef HAVE_ISINF
#undef HAVE_ISNAN
#undef HAVE_LIBHISTORY
#undef HAVE_LIBREADLINE
#undef HAVE_LIBPTHREAD
#undef HAVE_PTHREAD_H

/* Define if IPV6 support is there */
#undef SUPPORT_IP6

/* Define if getaddrinfo is there */
#undef HAVE_GETADDRINFO

/* Define to 1 if you have the <ansidecl.h> header file. */
#undef HAVE_ANSIDECL_H

/* Define to 1 if you have the <arpa/inet.h> header file. */
#undef HAVE_ARPA_INET_H

/* Define to 1 if you have the <arpa/nameser.h> header file. */
#undef HAVE_ARPA_NAMESER_H

/* Whether struct sockaddr::__ss_family exists */
#undef HAVE_BROKEN_SS_FAMILY

/* Define to 1 if you have the `class' function. */
#undef HAVE_CLASS

/* Define to 1 if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Define to 1 if you have the <dirent.h> header file. */
#undef HAVE_DIRENT_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Have dlopen based dso */
#undef HAVE_DLOPEN

/* Define to 1 if you have the <dl.h> header file. */
#undef HAVE_DL_H

/* Define to 1 if you have the <errno.h> header file. */
#undef HAVE_ERRNO_H

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H

/* Define to 1 if you have the `finite' function. */
#undef HAVE_FINITE

/* Define to 1 if you have the <float.h> header file. */
#undef HAVE_FLOAT_H

/* Define to 1 if you have the `fpclass' function. */
#undef HAVE_FPCLASS

/* Define to 1 if you have the `fprintf' function. */
#undef HAVE_FPRINTF

/* Define to 1 if you have the `fp_class' function. */
#undef HAVE_FP_CLASS

/* Define to 1 if you have the <fp_class.h> header file. */
#undef HAVE_FP_CLASS_H

/* Define to 1 if you have the `ftime' function. */
#undef HAVE_FTIME

/* Define if getaddrinfo is there */
#undef HAVE_GETADDRINFO

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* Define to 1 if you have the <ieeefp.h> header file. */
#undef HAVE_IEEEFP_H

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <inttypes.h.h> header file. */
#undef HAVE_INTTYPES_H_H

/* Define if isinf is there */
#undef HAVE_ISINF

/* Define if isnan is there */
#undef HAVE_ISNAN

/* Define to 1 if you have the `isnand' function. */
#undef HAVE_ISNAND

/* Define if history library is there (-lhistory) */
#undef HAVE_LIBHISTORY

/* Define if pthread library is there (-lpthread) */
// since we not defining LIBXML_THREAD_ENABLED, don't pretend we have this related define either.
#ifdef __MT__
#define HAVE_LIBPTHREAD
#endif

/* Define if readline library is there (-lreadline) */
#undef HAVE_LIBREADLINE

/* Have compression library */
#undef HAVE_LIBZ

/* Define to 1 if you have the <limits.h> header file. */
#undef HAVE_LIMITS_H

/* Define to 1 if you have the `localtime' function. */
#undef HAVE_LOCALTIME

/* Define to 1 if you have the <malloc.h> header file. */
#undef HAVE_MALLOC_H

/* Define to 1 if you have the <math.h> header file. */
#undef HAVE_MATH_H

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the <nan.h> header file. */
#undef HAVE_NAN_H

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#undef HAVE_NDIR_H

/* Define to 1 if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* Define to 1 if you have the <poll.h> header file. */
#undef HAVE_POLL_H

/* Define to 1 if you have the `printf' function. */
#undef HAVE_PRINTF

/* Define if <pthread.h> is there */
// since we not defining LIBXML_THREAD_ENABLED, don't pretend we have this related define either.
#ifdef __MT__
#define HAVE_PTHREAD_H
#endif

/* Define to 1 if you have the <resolv.h> header file. */
#undef HAVE_RESOLV_H

/* Have shl_load based dso */
#undef HAVE_SHLLOAD

/* Define to 1 if you have the `signal' function. */
#undef HAVE_SIGNAL

/* Define to 1 if you have the <signal.h> header file. */
#undef HAVE_SIGNAL_H

/* Define to 1 if you have the `snprintf' function. */
#if defined (_MSC_VER)
    #undef HAVE_SNPRINTF
#else
    #define HAVE_SNPRINTF 1
#endif

/* Define to 1 if you have the `sprintf' function. */
#define HAVE_SPRINTF 1

/* Define to 1 if you have the `sscanf' function. */
#define HAVE_SSCANF 1

/* Define to 1 if you have the `stat' function. */
#undef HAVE_STAT

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* (SS) Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#undef HAVE_STRDUP

/* Define to 1 if you have the `strerror' function. */
#undef HAVE_STRERROR

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strndup' function. */
#undef HAVE_STRNDUP

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#undef HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#undef HAVE_SYS_NDIR_H

/* Define to 1 if you have the <sys/select.h> header file. */
#undef HAVE_SYS_SELECT_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Whether va_copy() is available */
#if defined (__linux) || (defined (__APPLE__) && defined (__x86_64__))
    #define HAVE_VA_COPY
#else
    #undef HAVE_VA_COPY
#endif

/* Define to 1 if you have the `vfprintf' function. */
#undef HAVE_VFPRINTF

/* Define to 1 if you have the `vsnprintf' function. */
#undef HAVE_VSNPRINTF

/* Define to 1 if you have the `vsprintf' function. */
#undef HAVE_VSPRINTF

/* Define to 1 if you have the <zlib.h> header file. */
#undef HAVE_ZLIB_H

/* Define to 1 if you have the `_stat' function. */
#undef HAVE__STAT

/* Whether __va_copy() is available */
#undef HAVE___VA_COPY

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST const

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#undef LT_OBJDIR

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to 1 if the C compiler supports function prototypes. */
#undef PROTOTYPES

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Support for IPv6 */
#undef SUPPORT_IP6

/* Version number of package */
#undef VERSION

/* Determine what socket length (socklen_t) data type is */
#undef XML_SOCKLEN_T

/* Using the Win32 Socket implementation */
#undef _WINSOCKAPI_

/* Define like PROTOTYPES; this can be used by system headers. */
#undef __PROTOTYPES

/* Win32 Std C name mangling work-around */
#undef snprintf

/* ss_family is not defined here, use __ss_family instead */
#undef ss_family

/* Win32 Std C name mangling work-around */
#undef vsnprintf

/* Include multi-threading support like win32 already does. This triggers more in xmlversion.h. */
#define __MT__

/* Include XML_SYSCONFDIR for unix. As of 2.14.4 this Macro is no longer defined in libxml.h */
#ifndef XML_SYSCONFDIR
  #define XML_SYSCONFDIR "/etc"
#endif
