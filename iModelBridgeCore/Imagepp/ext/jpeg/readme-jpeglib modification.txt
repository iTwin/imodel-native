1) jmorecfg.h
    Surrond BITS_IN_JSAMPLE define with 
    #ifdef JPEGLIB_SUPPORT_12BITS
        #define BITS_IN_JSAMPLE  12	/* use 8 or 12 */
        #define NEED_SHORT_EXTERNAL_NAMES   /* In order to differentiate 8 bits and 12 bits calls.*/
    #else
        #define BITS_IN_JSAMPLE  8	/* use 8 or 12 */
    #endif

    *** Or eventually use NEED_12_BIT_NAMES if it ever make it to a release state.

2) jconfig.vc
    /* getenv is not available on WINRT. Anyway, we do not want it in all config we rely on jpeg_mem_init returned value*/
    #define NO_GETENV

    #ifndef INLINE
    #if defined(__GNUC__)
    #define INLINE __inline__
    #elif defined(_MSC_VER)
    #define INLINE __forceinline
    #else
    #define INLINE
    #endif
    #endif

3) Make sure to use jmemnobs.c. It is supported on all platform and we assume that we have all the memory we need.
   It used to be jmemansi.c but it relies on a temp file when out of memory. This is not platform independant.
   If we start using jmemansi.c again. Make sure to define DEFAULT_MAX_MEM to something bigger than the default(1MB) in jconfig.vc.

*** Jpeglib and JpegTurbo *** 
Since both lib will be use in the same environment they must we be using the same version. 
Basically, if you update jpeglib you need to update jpegturbo.

Don't forget the jpeg_license.txt
