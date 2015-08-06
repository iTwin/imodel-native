/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/image.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
#include <Geom/IntegerTypes/BSIRect.h>
#endif

#include <DgnPlatform/DgnPlatform.r.h>

typedef struct rgbFile RGBFile;

BEGIN_BENTLEY_DGN_NAMESPACE

/*----------------------------------------------------------------------+
|                                   |
|   Video File Formats                          |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    VIDEOFILE_FLI           = 14,/* Replace the now obsolete IMAGEFILE_FLI*/
    VIDEOFILE_FLC           = 15,/* Replace the now obsolete IMAGEFILE_FLC*/
    VIDEOFILE_AVI           = 26,/* Replace the now obsolete IMAGEFILE_AVI*/
    VIDEOFILE_WMV           = 53,/* Replace the now obsolete IMAGEFILE_WMV*/
    //Add new type here

    } VideoFileFormat;


/*----------------------------------------------------------------------+
|                                   |
|   Image File Formats                          |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    IMAGEFILE_UNKNOWN       =-1,
    IMAGEFILE_RAWTIFF       = 0,
    IMAGEFILE_RGB           = 1,
    IMAGEFILE_TARGA         = 2,
    IMAGEFILE_TIFF          = 3,
    IMAGEFILE_PICT          = 4,
    IMAGEFILE_PCX           = 5,
    IMAGEFILE_POSTSCRIPT    = 6,
    IMAGEFILE_GIF           = 7,
    IMAGEFILE_WPG           = 8,
    IMAGEFILE_BMP           = 9,
    IMAGEFILE_SUNRASTER     = 10,
    IMAGEFILE_IMGMAPPED     = 11,
    IMAGEFILE_IMGRGB        = 12,
    IMAGEFILE_RLE           = 13,
    _IMAGEFILE_FLI          = VIDEOFILE_FLI, /* IMAGEFILE_FLI is now obsolete in mdlImage - use mdlVideo API and VIDEOFILE_FLI*/
    _IMAGEFILE_FLC          = VIDEOFILE_FLC, /* IMAGEFILE_FLC is now obsolete in mdlImage - use mdlVideo API and VIDEOFILE_FLC*/
    IMAGEFILE_BUMP          = 16,
    IMAGEFILE_COT           = 17,
    IMAGEFILE_JPEG          = 18,
    IMAGEFILE_CIT           = 19,
    IMAGEFILE_TG4           = 20,
    IMAGEFILE_FAX           = 21,
    IMAGEFILE_CALS          = 22,
    IMAGEFILE_INGR          = 23,
    IMAGEFILE_RLC           = 24,
    IMAGEFILE_PACKBYTE      = 25,
    _IMAGEFILE_AVI          = VIDEOFILE_AVI, /* IMAGEFILE_AVI is now obsolete in mdlImage - use mdlVideo API and VIDEOFILE_AVI*/
    IMAGEFILE_PROCEDURE     = 27,
    IMAGEFILE_PNG           = 28,
    IMAGEFILE_GEOTIFF       = 29,
    IMAGEFILE_HMR           = 30,
    IMAGEFILE_ITIFF         = 31,
    IMAGEFILE_C29           = 32,
    IMAGEFILE_MRSID         = 33,
    IMAGEFILE_ERMAPPER      = 34,
    IMAGEFILE_TIFFINTGR     = 35,
    IMAGEFILE_MULTICHANNEL  = 36,
    IMAGEFILE_C30           = 37,
    IMAGEFILE_C31           = 38,
    IMAGEFILE_EPX           = 39,
    IMAGEFILE_BIL           = 40,
    IMAGEFILE_JPEG2000      = 41,
    IMAGEFILE_RGBCOMPRESSED = 42,
    IMAGEFILE_MPF           = 43,
    IMAGEFILE_CRL           = 44,
    IMAGEFILE_LRD           = 45,
    IMAGEFILE_DOQ           = 46,
    IMAGEFILE_WBMP          = 47,
    IMAGEFILE_ERDASIMG      = 48,
    IMAGEFILE_NITF          = 49,
    IMAGEFILE_DTED          = 50,
    IMAGEFILE_ITIFF64       = 51,
    IMAGEFILE_XWMS          = 52,
    _IMAGEFILE_WMV          = VIDEOFILE_WMV, /* IMAGEFILE_WMV is now obsolete in mdlImage - use mdlVideo API and VIDEOFILE_WMV*/
    IMAGEFILE_PDF           = 54,
    IMAGEFILE_BINGMAPS      = 55,
    IMAGEFILE_BSB           = 56,
    IMAGEFILE_XWCS          = 57,
    IMAGEFILE_USGSDEM       = 58,
    IMAGEFILE_USGSSDTSDEM   = 59,
    IMAGEFILE_XORA          = 60,
    IMAGEFILE_SPOTDIMAP     = 61,
    IMAGEFILE_AIG           = 62,
    IMAGEFILE_AAIG          = 63,
    //Add new type here

    } ImageFileFormat;

#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const long IMAGEFILE_ALL           =-1;
static const long IMAGEFILE_ALLIMAGEFILES = 98;
static const long IMAGEFILE_ALLFILES      = 99;

    // Image COM file plugin ID will dynamically be assigned in this range.
static const long IMAGEFILE_MIN_COM_RESERVED = 1000;
static const long IMAGEFILE_MAX_COM_RESERVED = 2000;
#else
#define IMAGEFILE_ALL           (-1)
#define IMAGEFILE_ALLIMAGEFILES (98)
#define IMAGEFILE_ALLFILES      (99)
#endif

/*----------------------------------------------------------------------+
|                                   |
|   Image Formats                           |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    IMAGEFORMAT_BitMap              = 1,
    IMAGEFORMAT_RLEBitMap           = 2,
    IMAGEFORMAT_ByteMap             = 3,
    IMAGEFORMAT_GreyScale           = 4,
    IMAGEFORMAT_RGBSeparate         = 5,
    IMAGEFORMAT_RGB                 = 6,
    IMAGEFORMAT_RGBA                = 7,
    IMAGEFORMAT_PackByte            = 8,
    IMAGEFORMAT_RLEByteMap          = 9,
    IMAGEFORMAT_BGRA                = 10, // Same as Windows DIB
    IMAGEFORMAT_RGBASeparate        = 11,
    IMAGEFORMAT_BGRSeparate         = 12
    } ImageFormat;

/*----------------------------------------------------------------------+
|                                   |
|   Degrees of Compression - used for JPEG compression          |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    MINCOMPRESSION         = 90,
    LOWCOMPRESSION         = 75,
    MEDCOMPRESSION         = 55,
    HIGHCOMPRESSION        = 40
    } CompressionRatio;

#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const CompressionRatio DEFAULTCOMPRESSION     = MINCOMPRESSION;
#else
#define DEFAULTCOMPRESSION     MINCOMPRESSION
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Ratios of Compression - used for JPEG2000 compression               |
|                                                                       |
+----------------------------------------------------------------------*/
typedef enum
    {
    JPEG2000LOSSLESS            = 1,
    JPEG2000MINCOMPRESSION      = 3,
    JPEG2000LOWCOMPRESSION      = 19,
    JPEG2000MEDCOMPRESSION      = 35,
    JPEG2000HIGHCOMPRESSION     = 50
    } CompressionRatioJPeg2000;


#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const CompressionRatioJPeg2000 DEFAULTJPEG2000COMPRESSION  =JPEG2000LOSSLESS;
#else
#define DEFAULTJPEG2000COMPRESSION  JPEG2000LOSSLESS
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Ratios of Compression - used for JPEG2000 compression               |
|                                                                       |
+----------------------------------------------------------------------*/
typedef enum
    {
    ECWMINCOMPRESSION      = 3,
    ECWLOWCOMPRESSION      = 10,
    ECWMEDCOMPRESSION      = 17,
    ECWHIGHCOMPRESSION     = 25
    } CompressionRatioECW;

#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const CompressionRatioECW DEFAULTECWCOMPRESSION  = ECWMINCOMPRESSION;
#else
#define DEFAULTECWCOMPRESSION  ECWMINCOMPRESSION
#endif

/*----------------------------------------------------------------------+
|                                   |
|   Compression Algorithms (use TIFF types where possible)      |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    COMPRESSTYPE_UNKNOWN         = -99,
    COMPRESSTYPE_TIFFNONE        =  -1,           /* dump mode - tiff uncompressed */
    COMPRESSTYPE_NONE            =   0,           /* no compression (old false) */
    COMPRESSTYPE_DEFAULT         =   1,           /* default compression (old true) */
    COMPRESSTYPE_CCITTRLE        =   2,           /* CCITT modified Huffman RLE */
    COMPRESSTYPE_CCITTFAX3       =   3,           /* CCITT Group 3 fax encoding */
    COMPRESSTYPE_CCITTFAX4       =   4,           /* CCITT Group 4 fax encoding */
    COMPRESSTYPE_LZW             =   5,           /* Lempel-Ziv  & Welch */
    COMPRESSTYPE_OJPEG           =   6,           /* !6.0 JPEG */
    COMPRESSTYPE_JPEG            =   7,           /* %JPEG DCT compression */
    COMPRESSTYPE_BMPRLE8         =   8,           /* BMP RLE8 */
    COMPRESSTYPE_BMPRLE4         =   9,           /* BMP RLE4 */
    COMPRESSTYPE_GIFLZW          =  10,           /* GIF LZW */
    COMPRESSTYPE_NEXT            = 32766,         /* NeXT 2-bit RLE */
    COMPRESSTYPE_CCITTRLEW       = 32771,         /* #1 w/ word alignment */
    COMPRESSTYPE_PACKBITS        = 32773,         /* Macintosh RLE */
    COMPRESSTYPE_THUNDERSCAN     = 32809,         /* ThunderScan RLE */
    COMPRESSTYPE_PIXARFILM       = 32908,         /* Pixar companded 10bit LZW */
    COMPRESSTYPE_DEFLATE         = 32946,         /* Deflate compression */
    COMPRESSTYPE_JBIG            = 34661,         /* ISO JBIG */

    COMPRESSTYPE_JPEG2000        = 34712,       /* JPEG2000 */
    COMPRESSTYPE_JPEG2000LOSSLESS= 34713,       /* JPEG2000 lossless */
    COMPRESSTYPE_JPEG2000MIN     = 34714,       /* JPEG2000 min ratio */
    COMPRESSTYPE_JPEG2000LOW     = 34715,       /* JPEG2000 low ratio */
    COMPRESSTYPE_JPEG2000MED     = 34716,       /* JPEG2000 med ratio */
    COMPRESSTYPE_JPEG2000HIGH    = 34717,       /* JPEG2000 high ratio */

    COMPRESSTYPE_ECW             = 34811,           /* ECW ERMAPPER*/
    COMPRESSTYPE_ECWMIN          = 34812,           /* ECW min ratio */
    COMPRESSTYPE_ECWLOW          = 34813,           /* ECW low ratio */
    COMPRESSTYPE_ECWMED          = 34814,           /* ECW med ratio */
    COMPRESSTYPE_ECWHIGH         = 34815,           /* ECW high ratio */

    COMPRESSTYPE_JPEGMIN        = MINCOMPRESSION,  /* JPEG type */
    COMPRESSTYPE_JPEGLOW        = LOWCOMPRESSION,  /* JPEG type */
    COMPRESSTYPE_JPEGMED        = MEDCOMPRESSION,  /* JPEG type */
    COMPRESSTYPE_JPEGHIGH       = HIGHCOMPRESSION, /* JPEG type */
    COMPRESSTYPE_RLE1           = 40000,           /* RLE1 compression */
    COMPRESSTYPE_RLE8           = 40001,           /* RLE8 compression */
    COMPRESSTYPE_ZLIB           = 40002,           /* ZLIB compression */
    COMPRESSTYPE_FLASHPIX       = 40003,           /* FLASHPIX compression */
    COMPRESSTYPE_TGARLE         = 40213,           /* TARGA RLE compression*/
    COMPRESSTYPE_LRD            = 40214,           /* LRD (special RLE ) compression*/
    COMPRESSTYPE_FLIRLE8        = 40215,           /* FLI (special RLE ) compression*/
    COMPRESSTYPE_CRL8           = 40216           /* CRL (special RLE ) compression*/
    } CompressionType;

/*----------------------------------------------------------------------+
|                                   |
|   Orientations (as returned by mdlImage_extractIngrAttach)        |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    INGR_ORIENT_UpperLeftVertical        = 0,
    INGR_ORIENT_UpperRightVertical       = 1,
    INGR_ORIENT_LowerLeftVertical        = 2,
    INGR_ORIENT_LowerRightVertical       = 3,
    INGR_ORIENT_UpperLeftHorizontal      = 4,
    INGR_ORIENT_UpperRightHorizontal     = 5,
    INGR_ORIENT_LowerLeftHorizontal      = 6,
    INGR_ORIENT_LowerRightHorizontal     = 7
    } ImageIngrOrientation;

/*----------------------------------------------------------------------+
|                                   |
|   Orientations (as returned by  mdlImage_readFileParams       |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    TOP_LEFT             = 0,
    TOP_RIGHT            = 1,
    LOWER_LEFT           = 2,
    LOWER_RIGHT          = 3
    } ImageOriginOrientation;

/*----------------------------------------------------------------------+
|                                   |
|   Return status for mdlImage_checkStop if processing aborted      |
|                                   |
+----------------------------------------------------------------------*/
#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const long MDLIMAGE_PROCESSABORTED      = 1;
#else
#define MDLIMAGE_PROCESSABORTED      1
#endif


/*----------------------------------------------------------------------+
|                                   |
|   Miscellaneous                           |
|                                   |
+----------------------------------------------------------------------*/
#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
static const double MONOCHROME_THRESHOLD  = 128.0;
#else
#define MONOCHROME_THRESHOLD  128.0
#endif
#define BITMAP_ROWBYTES(nColumns)   (((nColumns) + 7) >> 3)
#define BITMAP_BYTES(nColumns, nRows)   (nRows * BITMAP_ROWBYTES(nColumns))

//__PUBLISH_SECTION_END__

/*----------------------------------------------------------------------+
|                                   |
|   Movie Frame Transition Modes                    |
|                                   |
+----------------------------------------------------------------------*/
typedef enum
    {
    MOVIE_FrameCut           = 0,
    MOVIE_HorizontalWipe     = 1,
    MOVIE_VerticalWipe       = 2,
    MOVIE_Fade               = 3
    } MovieFrameTransition;

/*----------------------------------------------------------------------+
|                                   |
|   Movies Structure                            |
|                                   |
+----------------------------------------------------------------------*/
#if ! defined (resource)
typedef struct msMovieFrame
    {
    Byte *dataP;
    WChar     fileName[DGNPLATFORM_RESOURCE_MAXFILELENGTH];
    int         imageFormat;
    BSIRect     changeRect;
    struct msMovieFrame *nextP;
    } MSMovieFrame;

typedef struct msMovies
    {
    MSMovieFrame    *firstFrameP;
    Point2d     size;
    int         nFrames;
    int         speed;
    Byte redMap[256];
    Byte grnMap[256];
    Byte bluMap[256];
    int         paletteSize;
    double      gammaCorrection;
    bool        buffered;
    } MSMovie;

#endif

//__PUBLISH_SECTION_START__

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
