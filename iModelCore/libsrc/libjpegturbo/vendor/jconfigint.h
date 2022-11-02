/* libjpeg-turbo build number */
#define BUILD  "0"

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#ifndef INLINE
    #if defined(__GNUC__)
        #define INLINE inline __attribute__((always_inline))
    #elif defined(_MSC_VER)
        #define INLINE __forceinline
    #else
        #define INLINE
    #endif
#endif

/* How to obtain thread-local storage */
#ifdef _MSC_VER
    #define THREAD_LOCAL __declspec(thread)
#else
    #define THREAD_LOCAL __thread
#endif

/* Define to the full name of this package. */
#define PACKAGE_NAME  "libjpeg-turbo"

/* Version number of package */
#define VERSION  "2.1.3"

/* The size of `size_t', as computed by sizeof. */
#if !defined(SIZEOF_SIZE_T)
    #if defined (_M_X64) || defined(__LP64__)
        #define SIZEOF_SIZE_T 8
    #else
        #define SIZEOF_SIZE_T 4
    #endif
#endif

/* Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t). */
#ifndef _WIN32
    #define HAVE_BUILTIN_CTZL
#endif

/* Define to 1 if you have the <intrin.h> header file. */
#ifdef _WIN32
    #define HAVE_INTRIN_H  1
#endif

#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)
#define HAVE_BITSCANFORWARD64
#elif (SIZEOF_SIZE_T == 4)
#define HAVE_BITSCANFORWARD
#endif
#endif

#define BMP_SUPPORTED
#define PPM_SUPPORTED
#if defined(__has_attribute)
#if __has_attribute(fallthrough)
#define FALLTHROUGH  __attribute__((fallthrough));
#else
#define FALLTHROUGH
#endif
#else
#define FALLTHROUGH
#endif
