//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFTag.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTIFFFile
//-----------------------------------------------------------------------------

#pragma once


#define    TIFF_VERSION        42
#define    TIFF_VERSION_BIG    43      // Tiff larger than 4Gig


// See at the end for GeoKeys information.

//
// Flags pass to the PrintDirectory method.
//
#define    TIFFPRINT_NONE                    0x0
#define    TIFFPRINT_STRIPS                0x1
#define    TIFFPRINT_CURVES                0x2
#define    TIFFPRINT_COLORMAP                0x4
#define    TIFFPRINT_JPEGQTABLES            0x100
#define    TIFFPRINT_JPEGACTABLES            0x200
#define    TIFFPRINT_JPEGDCTABLES            0x200


// Support TAG HMR
#define TIFFPRINT_HMR_HISTOGRAM         0x400
#define TIFFPRINT_HMR_LOGICALSHAPE      0x800
#define TIFFPRINT_HMR_TRANSPARENTSHAPE  0x1000
#define TIFFPRINT_HMR_USERDATA          0x2000

#define TIFFPRINT_FREEBLOCKS            0x4000
#define TIFFPRINT_HMR2_TILEFLAG         0x8000


// GeoTiff
#define    TIFFPRINT_GEOKEYDIRECTORY        0x80000000
#define    TIFFPRINT_GEOKEYPARAMS            0x40000000

BEGIN_IMAGEPP_NAMESPACE

//
// Tag ID use by this librarie enclose in a namespace to
// avoid name clash.
//
// Must be in a incremental order, see the TAG list
// below.
//

enum TIFFTag {
    GPS_VERSIONID,
    GPS_LATITUDEREF,
    GPS_LATITUDE,
    GPS_LONGITUDEREF,
    GPS_LONGITUDE,
    GPS_ALTITUDEREF,
    GPS_ALTITUDE,
    GPS_TIMESTAMP,
    GPS_SATELLITES,
    GPS_STATUS,
    GPS_MEASUREMODE,
    GPS_DOP,
    GPS_SPEEDREF,
    GPS_SPEED,
    GPS_TRACKREF,
    GPS_TRACK,
    GPS_IMGDIRECTIONREF,
    GPS_IMGDIRECTION,
    GPS_MAPDATUM,
    GPS_DESTLATITUDEREF,
    GPS_DESTLATITUDE,
    GPS_DESTLONGITUDEREF,
    GPS_DESTLONGITUDE,
    GPS_DESTBEARINGREF,
    GPS_DESTBEARING,
    GPS_DESTDISTANCEREF,
    GPS_DESTDISTANCE,
    GPS_PROCESSINGMETHOD,
    GPS_AREAINFORMATION,
    GPS_DATESTAMP,
    GPS_DIFFERENTIAL,
    SUBFILETYPE,
    OSUBFILETYPE,
    IMAGEWIDTH,
    IMAGELENGTH,
    BITSPERSAMPLE,
    COMPRESSION,
    PHOTOMETRIC,
    THRESHHOLDING,
    CELLWIDTH,
    CELLLENGTH,
    FILLORDER,
    DOCUMENTNAME,
    IMAGEDESCRIPTION,
    MAKE,
    MODEL,
    STRIPOFFSETS,
    ORIENTATION,
    SAMPLESPERPIXEL,
    ROWSPERSTRIP,
    STRIPBYTECOUNTS,
    MINSAMPLEVALUE,
    MAXSAMPLEVALUE,
    XRESOLUTION,
    YRESOLUTION,
    PLANARCONFIG,
    PAGENAME,
    XPOSITION,
    YPOSITION,
    FREEOFFSETS,
    FREEBYTECOUNTS,
    GRAYRESPONSEUNIT,
    GRAYRESPONSECURVE,
    GROUP3OPTIONS,
    RESOLUTIONUNIT,
    PAGENUMBER,
    COLORRESPONSEUNIT,
    TRANSFERFUNCTION,
    SOFTWARE,
    DATETIME,
    ARTIST,
    HOSTCOMPUTER,
    PREDICTOR,
    WHITEPOINT,
    PRIMARYCHROMATICITIES,
    TCOLORMAP,
    HALFTONEHINTS,
    TILEWIDTH,
    TILELENGTH,
    TILEOFFSETS,
    TILEBYTECOUNTS,
    SUBIFD,
    INKSET,
    INKNAMES,
    DOTRANGE,
    TARGETPRINTER,
    EXTRASAMPLES,
    SAMPLEFORMAT,
    SMINSAMPLEVALUE,
    SMAXSAMPLEVALUE,
    JPEGTABLES,
    YCBCRCOEFFICIENTS,
    YCBCRSUBSAMPLING,
    YCBCRPOSITIONING,
    REFERENCEBLACKWHITE,
    HMR_IMAGECOORDINATESYSTEM,
    HMR_XORIGIN,
    HMR_YORIGIN,
    HMR_XPIXELSIZE,
    HMR_YPIXELSIZE,
    HMR2_3D_TRANSFO_MATRIX,
    HMR2_WELLKNOWNTEXT,
    HMR2_ONDEMANDRASTERS_INFO,
    HMR_HISTOGRAM,
    HMR_HISTOGRAMDATETIME,
    HMR_PIXEL_TYPE_SPEC,
    HMR_TRANSPARENCY_PALETTE,
    HMR_THUMBNAIL_COMPOSED,
    HMR2_CHANNELS_WITH_NODATAVALUE,
    HMR2_CHANNELS_NODATAVALUE,
    HMR_VERSION,
    HMR_VERSION_MINOR,
    HMR_PADDING,
    HMR_LOGICALSHAPE,
    HMR_TRANSPARENTSHAPE,
    HMR_USERDATA,
    HMR2_TILEFLAG,
    HMR_DECIMATION_METHOD,
//            HMR_ALTAPHOTO_BLOB,
    HMR_PROJECTWISE_BLOB,
    HMR_FILTERS,
    HMR_SOURCEFILE_CREATIONDATE,    // That information is only keep for back compatible between V8i, on Windows platform only.
    HMR_SYNCHRONIZE_FIELD,
    MATTEING,
    DATATYPE,
    IMAGEDEPTH,
    TILEDEPTH,
    COPYRIGHT,
    GEOPIXELSCALE,
    INTERGRAPH_RAGBAG,
    INTERGRAPH_MATRIX,
    GEOTIEPOINTS,
    GEOTRANSMATRIX,
    EXIF_EXPOSURETIME,              //33434L
    EXIF_FNUMBER,                   //33437L
    HMR_IMAGEINFORMATION,
    HMR2_IMAGEINFORMATION,
    EXIFDIRECTORY,                  //34665L
    GEOKEYDIRECTORY,
    GEODOUBLEPARAMS,
    GEOASCIIPARAMS,
    EXIF_EXPOSUREPROGRAM,           //34850L
    GPSDIRECTORY,                   //34853L
    EXIF_SPECTRALSENSITIVITY,
    EXIF_ISOSPEEDRATINGS,
    EXIF_OECF,
    EXIF_EXIFVERSION,
    EXIF_DATETIMEORIGINAL,
    EXIF_DATETIMEDIGITIZED,
    EXIF_COMPONENTSCONFIGURATION,
    EXIF_COMPRESSEDBITSPERPIXEL,
    EXIF_SHUTTERSPEEDVALUE,
    EXIF_APERTUREVALUE,
    EXIF_BRIGHTNESSVALUE,
    EXIF_EXPOSUREBIASVALUE,
    EXIF_MAXAPERTUREVALUE,
    EXIF_SUBJECTDISTANCE,
    EXIF_METERINGMODE,
    EXIF_LIGHTSOURCE,
    EXIF_FLASH,
    EXIF_FOCALLENGTH,
    EXIF_SUBJECTAREA,
    EXIF_MAKERNOTE,
    EXIF_USERCOMMENT,
    EXIF_SUBSECTIME,
    EXIF_SUBSECTIME_ORIGINAL,
    EXIF_SUBSECTIME_DIGITIZED,
    EXIF_FLASHPIXVERSION,
    EXIF_COLORSPACE,
    EXIF_PIXELXDIMENSION,
    EXIF_PIXELYDIMENSION,
    EXIF_RELATEDSOUNDFILE,
    EXIF_FLASHENERGY,
    EXIF_SPATIALFREQUENCYRESPONSE,
    EXIF_FOCALPLANEXRESOLUTION,
    EXIF_FOCALPLANEYRESOLUTION,
    EXIF_FOCALPLANERESOLUTIONUNIT,
    EXIF_SUBJECTLOCATION,
    EXIF_EXPOSUREINDEX,
    EXIF_SENSINGMETHOD,
    EXIF_FILESOURCE,
    EXIF_SCENETYPE,
    EXIF_CFAPATTERN,
    EXIF_CUSTOMRENDERED,
    EXIF_EXPOSUREMODE,
    EXIF_WHITEBALANCE,
    EXIF_DIGITALZOOMRATIO,
    EXIF_FOCALLENGTHIN35MMFILM,
    EXIF_SCENECAPTURETYPE,
    EXIF_GAINCONTROL,
    EXIF_CONTRAST,
    EXIF_SATURATION,
    EXIF_SHARPNESS,
    EXIF_DEVICESETTINGDESCRIPTION,
    EXIF_SUBJECTDISTANCERANGE,
    EXIF_IMAGEUNIQUEID,             //42016L

	GDALNODATA,

    // These Tags are not saved in the file.
    TAG_NOT_SAVED_FILE,
    COMPRESSION_QUALITY,    // map on TIFFTAG_JPEGQUALITY        65537
    COMPRESSION_JPEGOPTIMIZECODING,

    EndOfTag
    };
END_IMAGEPP_NAMESPACE


//
// This Tag list is kept only for information usage and used by the file
// HTIFFTagDefinition
//
#define    TIFFTAG_SUBFILETYPE        254                // subfile data descriptor
#define        FILETYPE_FULLIMAGE    0x0             // full image
#define        FILETYPE_REDUCEDIMAGE    0x1            // reduced resolution version
#define        FILETYPE_PAGE        0x2                // one page of many
#define        FILETYPE_MASK        0x4                // transparency mask
#define     FILETYPE_EMPTYPAGE  0x8             // for cTiff

#define    TIFFTAG_OSUBFILETYPE        255            // +kind of data in subfile
#define        OFILETYPE_IMAGE        1                // full resolution image data
#define        OFILETYPE_REDUCEDIMAGE    2            // reduced size image data
#define        OFILETYPE_PAGE        3                // one page of many
#define    TIFFTAG_IMAGEWIDTH        256                // image width in pixels
#define    TIFFTAG_IMAGELENGTH        257                // image height in pixels
#define    TIFFTAG_BITSPERSAMPLE        258            // bits per channel (sample)
#define    TIFFTAG_COMPRESSION        259                // data compression technique
#define        COMPRESSION_NONE        1            // dump mode
#define        COMPRESSION_CCITTRLE    2            // CCITT modified Huffman RLE
#define        COMPRESSION_CCITTFAX3    3            // CCITT Group 3 fax encoding
#define        COMPRESSION_CCITTFAX4    4            // CCITT Group 4 fax encoding
#define        COMPRESSION_LZW        5                // Lempel-Ziv  & Welch
#define        COMPRESSION_OJPEG        6            // !6.0 JPEG
#define        COMPRESSION_JPEG        7            // %JPEG DCT compression
#define        COMPRESSION_ADOBE_DEFLATE 8           // Same as COMPRESSION_DEFLATE
#define        COMPRESSION_NEXT        32766        // NeXT 2-bit RLE
#define        COMPRESSION_CCITTRLEW    32771        // #1 w/ word alignment
#define        COMPRESSION_PACKBITS    32773        // Macintosh RLE
#define        COMPRESSION_THUNDERSCAN    32809        // ThunderScan RLE
// compression codes 32908-32911 are reserved for Pixar
#define     COMPRESSION_PIXARFILM       32908   // Pixar companded 10bit LZW
#define     COMPRESSION_DEFLATE         32946       // Deflate compression
#define     COMPRESSION_JBIG            34661       // ISO JBIG
#define     COMPRESSION_JP2000      34712       // JPEG2000



// Reserved Tags HMR
#define COMPRESSION_HMR_FLASHPIX    34702   // Flashpix format (supports opacity)
#define COMPRESSION_HMR_FLASHPIX_OLD    35000
#define COMPRESSION_HMR_RLE1        34703
#define COMPRESSION_HMR_RLE1_OLD        35001
#define COMPRESSION_HMR_LURAWAVE_PADDED        34706
#define COMPRESSION_HMR_LURAWAVE_NON_PADDED    34707

#define TIFFTAG_PHOTOMETRIC         262     // photometric interpretation
#define PHOTOMETRIC_MINISWHITE      0       // min value is white
#define PHOTOMETRIC_MINISBLACK      1       // min value is black
#define PHOTOMETRIC_RGB             2       // RGB color model
#define PHOTOMETRIC_PALETTE         3       // color map indexed
#define PHOTOMETRIC_MASK            4       // $holdout mask
#define PHOTOMETRIC_SEPARATED       5       // !color separations
#define PHOTOMETRIC_YCBCR           6       // !CCIR 601
#define PHOTOMETRIC_CIELAB          8       // !1976 CIE L*a*b*
#define TIFFTAG_THRESHHOLDING       263     // +thresholding used on data
#define THRESHHOLD_BILEVEL          1       // b&w art scan
#define THRESHHOLD_HALFTONE         2       // or dithered scan
#define THRESHHOLD_ERRORDIFFUSE     3       // usually floyd-steinberg
#define TIFFTAG_CELLWIDTH           264     // +dithering matrix width
#define TIFFTAG_CELLLENGTH          265     // +dithering matrix height
#define TIFFTAG_FILLORDER           266     // data order within a byte
#define FILLORDER_MSB2LSB           1       // most significant -> least
#define FILLORDER_LSB2MSB           2       // least significant -> most
#define TIFFTAG_DOCUMENTNAME        269     // name of doc. image is from
#define TIFFTAG_IMAGEDESCRIPTION    270     // info about image
#define TIFFTAG_MAKE                271     // scanner manufacturer name
#define TIFFTAG_MODEL               272     // scanner model name/number
#define TIFFTAG_STRIPOFFSETS        273     // offsets to data strips
#define TIFFTAG_ORIENTATION         274     // +image orientation
#define ORIENTATION_TOPLEFT         1       // row 0 top, col 0 lhs
#define ORIENTATION_TOPRIGHT        2       // row 0 top, col 0 rhs
#define ORIENTATION_BOTRIGHT        3       // row 0 bottom, col 0 rhs
#define ORIENTATION_BOTLEFT         4       // row 0 bottom, col 0 lhs
#define ORIENTATION_LEFTTOP         5       // row 0 lhs, col 0 top
#define ORIENTATION_RIGHTTOP        6       // row 0 rhs, col 0 top
#define ORIENTATION_RIGHTBOT        7       // row 0 rhs, col 0 bottom
#define ORIENTATION_LEFTBOT         8       // row 0 lhs, col 0 bottom
#define TIFFTAG_SAMPLESPERPIXEL     277     // samples per pixel
#define TIFFTAG_ROWSPERSTRIP        278     // rows per strip of data
#define TIFFTAG_STRIPBYTECOUNTS     279     // bytes counts for strips
#define TIFFTAG_MINSAMPLEVALUE      280     // +minimum sample value
#define TIFFTAG_MAXSAMPLEVALUE      281     // +maximum sample value
#define TIFFTAG_XRESOLUTION         282     // pixels/resolution in x
#define TIFFTAG_YRESOLUTION         283     // pixels/resolution in y
#define TIFFTAG_PLANARCONFIG        284     // storage organization
#define PLANARCONFIG_CONTIG         1       // single image plane
#define PLANARCONFIG_SEPARATE       2       // separate planes of data
#define TIFFTAG_PAGENAME            285     // page name image is from
#define TIFFTAG_XPOSITION           286     // x page offset of image lhs
#define TIFFTAG_YPOSITION           287     // y page offset of image lhs
#define TIFFTAG_FREEOFFSETS         288     // +byte offset to free block
#define TIFFTAG_FREEBYTECOUNTS      289     // +sizes of free blocks
#define TIFFTAG_GRAYRESPONSEUNIT    290     // $gray scale curve accuracy
#define GRAYRESPONSEUNIT_10S        1       // tenths of a unit
#define GRAYRESPONSEUNIT_100S       2       // hundredths of a unit
#define GRAYRESPONSEUNIT_1000S      3       // thousandths of a unit
#define GRAYRESPONSEUNIT_10000S     4       // ten-thousandths of a unit
#define GRAYRESPONSEUNIT_100000S    5       // hundred-thousandths
#define TIFFTAG_GRAYRESPONSECURVE   291     // $gray scale response curve
#define TIFFTAG_GROUP3OPTIONS       292     // 32 flag bits
#define GROUP3OPT_2DENCODING        0x1     // 2-dimensional coding
#define GROUP3OPT_UNCOMPRESSED      0x2     // data not compressed
#define GROUP3OPT_FILLBITS          0x4     // fill to byte boundary
#define TIFFTAG_GROUP4OPTIONS       293     // 32 flag bits
#define GROUP4OPT_UNCOMPRESSED      0x2     // data not compressed
#define TIFFTAG_RESOLUTIONUNIT      296     // units of resolutions
#define RESUNIT_NONE                1       // no meaningful units
#define RESUNIT_INCH                2       // english
#define RESUNIT_CENTIMETER          3       // metric
#define TIFFTAG_PAGENUMBER          297     // page numbers of multi-page
#define TIFFTAG_COLORRESPONSEUNIT   300     // $color curve accuracy
#define COLORRESPONSEUNIT_10S       1       // tenths of a unit
#define COLORRESPONSEUNIT_100S      2       // hundredths of a unit
#define COLORRESPONSEUNIT_1000S     3       // thousandths of a unit
#define COLORRESPONSEUNIT_10000S    4       // ten-thousandths of a unit
#define COLORRESPONSEUNIT_100000S   5       // hundred-thousandths
#define TIFFTAG_TRANSFERFUNCTION    301     // !colorimetry info
#define TIFFTAG_SOFTWARE            305     // name & release
#define TIFFTAG_DATETIME            306     // creation date and time
#define TIFFTAG_ARTIST              315     // creator of image
#define TIFFTAG_HOSTCOMPUTER        316     // machine where created
#define TIFFTAG_PREDICTOR           317     // prediction scheme w/ LZW
#define TIFFTAG_WHITEPOINT          318     // image white point
#define TIFFTAG_PRIMARYCHROMATICITIES 319   // !primary chromaticities
#define TIFFTAG_COLORMAP            320     // RGB map for pallette image
#define TIFFTAG_HALFTONEHINTS       321     // !highlight+shadow info
#define TIFFTAG_TILEWIDTH           322     // !rows/data tile
#define TIFFTAG_TILELENGTH          323     // !cols/data tile
#define TIFFTAG_TILEOFFSETS         324     // !offsets to data tiles
#define TIFFTAG_TILEBYTECOUNTS      325     // !byte counts for tiles
#define TIFFTAG_BADFAXLINES         326     // lines w/ wrong pixel count
#define TIFFTAG_CLEANFAXDATA        327     // regenerated line info
#define CLEANFAXDATA_CLEAN          0       // no errors detected
#define CLEANFAXDATA_REGENERATED    1       // receiver regenerated lines
#define CLEANFAXDATA_UNCLEAN        2       // uncorrected errors exist
#define TIFFTAG_CONSECUTIVEBADFAXLINES 328  // max consecutive bad lines
#define TIFFTAG_SUBIFD              330     // subimage descriptors
#define TIFFTAG_INKSET              332     // !inks in separated image
#define INKSET_CMYK                 1       // !cyan-magenta-yellow-black
#define TIFFTAG_INKNAMES            333     // !ascii names of inks
#define TIFFTAG_DOTRANGE            336     // !0% and 100% dot codes
#define TIFFTAG_TARGETPRINTER       337     // !separation target
#define TIFFTAG_EXTRASAMPLES        338     // !info about extra samples
#define EXTRASAMPLE_UNSPECIFIED     0       // !unspecified data
#define EXTRASAMPLE_ASSOCALPHA      1       // !associated alpha data
#define EXTRASAMPLE_UNASSALPHA      2       // !unassociated alpha data
#define TIFFTAG_SAMPLEFORMAT        339     // !data sample format
#define SAMPLEFORMAT_UINT           1       // !unsigned integer data
#define SAMPLEFORMAT_INT            2       // !signed integer data
#define SAMPLEFORMAT_IEEEFP         3       // !IEEE floating point data
#define SAMPLEFORMAT_VOID           4       // !untyped data
#define TIFFTAG_SMINSAMPLEVALUE     340     // !variable MinSampleValue
#define TIFFTAG_SMAXSAMPLEVALUE     341     // !variable MaxSampleValue
#define TIFFTAG_JPEGTABLES          347     // %JPEG table stream
//
// Tags 512-521 are obsoleted by Technical Note #2
// which specifies a revised JPEG-in-TIFF scheme.

#define    TIFFTAG_JPEGPROC        512                // !JPEG processing algorithm
#define    JPEGPROC_BASELINE        1            // !baseline sequential
#define    JPEGPROC_LOSSLESS        14            // !Huffman coded lossless
#define    TIFFTAG_JPEGIFOFFSET        513            // !pointer to SOI marker
#define    TIFFTAG_JPEGIFBYTECOUNT        514            // !JFIF stream length
#define    TIFFTAG_JPEGRESTARTINTERVAL    515            // !restart interval length
#define    TIFFTAG_JPEGLOSSLESSPREDICTORS    517        // !lossless proc predictor
#define    TIFFTAG_JPEGPOINTTRANSFORM    518            // !lossless point transform
#define    TIFFTAG_JPEGQTABLES        519                // !Q matrice offsets
#define    TIFFTAG_JPEGDCTABLES        520            // !DCT table offsets
#define    TIFFTAG_JPEGACTABLES        521            // !AC coefficient offsets
#define    TIFFTAG_YCBCRCOEFFICIENTS    529            // !RGB -> YCbCr transform
#define    TIFFTAG_YCBCRSUBSAMPLING    530            // !YCbCr subsampling factors
#define    TIFFTAG_YCBCRPOSITIONING    531            // !subsample positioning
#define    YCBCRPOSITION_CENTERED    1            // !as in PostScript Level 2
#define    YCBCRPOSITION_COSITED    2            // !as in CCIR 601-1
#define    TIFFTAG_REFERENCEBLACKWHITE    532            // !colorimetry info
// tags 32952-32956 are private tags registered to Island Graphics
#define TIFFTAG_REFPTS            32953            // image reference points
#define TIFFTAG_REGIONTACKPOINT        32954        // region-xform tack point
#define TIFFTAG_REGIONWARPCORNERS    32955        // warp quadrilateral
#define TIFFTAG_REGIONAFFINE        32956        // affine transformation mat
// tags 32995-32999 are private tags registered to SGI
#define    TIFFTAG_MATTEING        32995            // $use ExtraSamples
#define    TIFFTAG_DATATYPE        32996            // $use SampleFormat
#define    TIFFTAG_IMAGEDEPTH        32997            // z depth of image
#define    TIFFTAG_TILEDEPTH        32998            // z depth/data tile

// tags 33300-33309 are private tags registered to Pixar
#define TIFFTAG_PIXAR_IMAGEFULLWIDTH    33300   // full image size in x
#define TIFFTAG_PIXAR_IMAGEFULLLENGTH   33301   // full image size in y
// tag 33405 is a private tag registered to Eastman Kodak
#define TIFFTAG_WRITERSERIALNUMBER      33405   // device serial number
// tag 33432 is listed in the 6.0 spec w/ unknown ownership
#define    TIFFTAG_COPYRIGHT        33432            // copyright string

//
// GeoTiff
//
// tags 33550 is a private tag registered to SoftDesk, Inc
#define TIFFTAG_GEOPIXELSCALE           33550

// Very old tag from Intergraph file.
#define TIFFTAG_INTERGRAPH_RAGBAG       33918

// tags 33920-33921 are private tags registered to Intergraph, Inc
#define TIFFTAG_INTERGRAPH_MATRIX       33920   // $use TIFFTAG_GEOTRANSMATRIX !
#define TIFFTAG_GEOTIEPOINTS            33922

// tags 34232-34236 are private tags registered to Texas Instruments
#define TIFFTAG_FRAMECOUNT              34232   // Sequence Frame Count

//
// GeoTiff
//

// tags 34263-34264 are private tags registered to NASA-JPL Carto Group
#define HTIFFTAG_GEOTRANSMATRIX         34264L  // New Matrix Tag replaces 33920
                                                // Add an 'H' in front to remove conflict with Ecw/Jpeg2000

//
// Private tags registered to HMR inc
// Donald  10/14/1996
//
// Tags registered
//
//          34460 --> TIFFTAG_HMR_IMAGEINFORMATION (HMR1)
//          34461 --> TIFFTAG_HMR_IMAGEINFORMATION (HMR2 <HMR++>)
//          34462 --> Not used
// Define the offset of the Private HMR Directory (Header)
#define TIFFTAG_HMR_IMAGEINFORMATION    34460L
#define TIFFTAG_HMR2_IMAGEINFORMATION   34461L

//
// GeoTiff
//
// tags 34735-3438 are private tags registered to SPOT Image, Inc
                                                // Add an 'H' in front to remove conflict with Ecw/Jpeg2000
#define HTIFFTAG_GEOKEYDIRECTORY         34735L
#define HTIFFTAG_GEODOUBLEPARAMS         34736L
#define HTIFFTAG_GEOASCIIPARAMS          34737L

//-----------------------------------------------------------------------------
//
// Defines Tags in the private HMR Directory
//
#define TIFFTAG_HMR_IMAGECOORDINATESYSTEM         31100L
#define TIFFTAG_HMR_XORIGIN                       31101L
#define TIFFTAG_HMR_YORIGIN                       31102L
#define TIFFTAG_HMR_XPIXELSIZE                    31103L
#define TIFFTAG_HMR_YPIXELSIZE                    31104L
#define TIFFTAG_HMR2_3D_TRANSFO_MATRIX            31105L
#define TIFFTAG_HMR2_WELLKNOWNTEXT                31106L
#define TIFFTAG_HMR2_ONDEMANDRASTERS_INFO         31107L

#define TIFFTAG_HMR_HISTOGRAM                   31110L
#define TIFFTAG_HMR_HISTOGRAMDATETIME           31111L
#define TIFFTAG_HMR_PIXEL_TYPE_SPEC             31112L
#define TIFFTAG_HMR_TRANSPARENCY_PALETTE        31113L
#define TIFFTAG_HMR_THUMBNAIL_COMPOSED          31114L
#define TIFFTAG_HMR2_CHANNELS_WITH_NODATAVALUE  31115L
#define TIFFTAG_HMR2_CHANNELS_NODATAVALUE       31116L

// Add Compression Option
#define TIFFTAG_HMR_VERSION                   31120L
#define TIFFTAG_HMR_VERSION_MINOR             31122L

// Padding line Optimization
#define TIFFTAG_HMR_PADDING                   31130L
// Si le TAG n'est pas present, alors VERSION 0
// Format Standard TIFF, (padding en Y en Bas)
// Dimension image reel. (sans padding)
// HMR and HMR++
#define     HMR_VERSION_TILE                  0L
// VERSION 1
// Format modifie (padding en Y en Haut)
// Dimension Image contient le padding
// HMR only (all file generated by Descarte)
#define     HMR_VERSION_TILEPADDING           1L
// VERSION 2
// Same as VERSION 1 but
// the origin is at the Top-Left (SLO4)
// HMR++ only
#define     HMR_VERSION_TILE_SLO4             2L
// VERSION 3
// the sub-res for 1bit image can be grayscale
#define     ITIFF_VERSION_1BIT_GRAYSCALE      3L
// VERSION 4
// Add 64 bits support
#define     ITIFF_VERSION_64BITS              4L
#define        ITIFF_LATEST_VERSION              ITIFF_VERSION_64BITS

#define     CTIFF_VERSION                     10002L

#define     CTIFF_PW_VERSION                  10010L
//
// Add logical shape for an image
#define TIFFTAG_HMR_LOGICALSHAPE              31141L

//
// Add transparent shapes for an image
#define TIFFTAG_HMR_TRANSPARENTSHAPE          31151L

//
// Add user data for an image
#define TIFFTAG_HMR_USERDATA                  31154L

// HMR++ Tags
//

// Tile Flags
//   All tiles of each sub-resolution are in this string.
//   Each byte represent a tile...
//   Presently, this list follow the defines in HRFTypes.h
//
//   0xFF --> End of Resolution.
//
#define TIFFTAG_HMR2_TILEFLAG                 31160L
// reserved                               0x00
#define HMR2_DATAFLAG_EMPTY               0x01
#define HMR2_DATAFLAG_LOADED              0x02
#define HMR2_DATAFLAG_OVERWRITTEN         0x04
#define HMR2_DATAFLAG_TOBECLEAR           0x08
#define HMR2_DATAFLAG_DIRTYFORSUBRES      0x10
#define HMR2_TILEFLAG_ENDRESOLUTION       0xff  // Mark, separate each resolution.

// Decimation method
//  All new decimation must be add at the end, because many files
//  can have the value now.
#define TIFFTAG_HMR_DECIMATION_METHOD         31165L
#define HMR_NEAREST_NEIGHBOUR_DECIMATION  0x00
#define HMR_AVERAGE_DECIMATION            0x01
#define HMR_VECTOR_AWARENESS_DECIMATION   0x02
#define HMR_UNDEFINED_DECIMATION          0x03
#define HMR_ORING4_DECIMATION             0x04
#define HMR_NONE_DECIMATION               0x05

// AltaPhoto Blob
//  each entry has 2 UInt32 --> [Offset][DataSize in byte]
//#define TIFFTAG_HMR_ALTAPHOTO_BLOB            31170L
#define TIFFTAG_HMR_PROJECTWISE_BLOB          31170L

// Filters
#define TIFFTAG_HMR_FILTERS                   31200L

// This tag used only in the Ctiff
// Keep the Creation date of the source file, to be able to determine
// if the source file has changed by a rename.
#define TIFFTAG_HMR_SOURCEFILE_CREATIONDATE   31250L

// Concurrence support, this field is used to synchronize the file access.
#define TIFFTAG_HMR_SYNCHRONIZE_FIELD         31999L

// HMR end
//-----------------------------------------------------------------------------

// tag 34750 is a private tag registered to Pixel Magic
#define    TIFFTAG_JBIGOPTIONS        34750            // JBIG options
// tags 34908-34914 are private tags registered to SGI
#define    TIFFTAG_FAXRECVPARAMS        34908        // encoded Class 2 ses. parms
#define    TIFFTAG_FAXSUBADDRESS        34909        // received SubAddr string
#define    TIFFTAG_FAXRECVTIME        34910            // receive time (secs)


#define    TIFFTAG_GDAL_NODATA        42113            // Unregistered tag used by the GDAL library, contains an ASCII encoded nodata or background pixel value. 
 

// tag 65535 is an undefined tag used by Eastman Kodak
#define TIFFTAG_DCSHUESHIFTVALUES       65535   // hue shift correction data



//
// The following are ``pseudo tags'' that can be
// used to control codec-specific functionality.
// These tags are not written to file.  Note that
// these values start at 0xffff+1 so that they'll
// never collide with Aldus-assigned tags.
//
// If you want your private pseudo tags ``registered''
// (i.e. added to this file), send mail to sam@sgi.com
// with the appropriate C definitions to add.

#define    TIFFTAG_FAXMODE            65536            // Group 3/4 format control
#define        FAXMODE_CLASSIC    0x0000                // default, include RTC
#define        FAXMODE_NORTC    0x0001                // no RTC at end of data
#define        FAXMODE_NOEOL    0x0002                // no EOL code at end of row
#define        FAXMODE_BYTEALIGN    0x0004            // byte align row
#define        FAXMODE_WORDALIGN    0x0008            // word align row
#define        FAXMODE_CLASSF    FAXMODE_NORTC        // TIFF Class F
#define    TIFFTAG_JPEGQUALITY        65537            // Compression quality level
// Note: quality level is on the IJG 0-100 scale.  Default value is 75
#define    TIFFTAG_JPEGCOLORMODE        65538        // Auto RGB<=>YCbCr convert?
#define        JPEGCOLORMODE_RAW    0x0000            // no conversion (default)
#define        JPEGCOLORMODE_RGB    0x0001            // do auto conversion
#define    TIFFTAG_JPEGTABLESMODE        65539        // What to put in JPEGTables
#define        JPEGTABLESMODE_QUANT 0x0001            // include quantization tbls
#define        JPEGTABLESMODE_HUFF    0x0002            // include Huffman tbls
// Note: default is JPEGTABLESMODE_QUANT | JPEGTABLESMODE_HUFF
#define    TIFFTAG_FAXFILLFUNC        65540            // G3/G4 fill function
#define    TIFFTAG_PIXARLOGDATAFMT        65549       // PixarLogCodec I/O data sz
#define        PIXARLOGDATAFMT_8BIT    0            // regular u_char samples
#define        PIXARLOGDATAFMT_8BITABGR    1        // ABGR-order u_chars
#define        PIXARLOGDATAFMT_10BITLOG    2        // 10-bit log-encoded (raw)
#define        PIXARLOGDATAFMT_12BITPICIO    3        // as per PICIO (1.0==2048)
#define        PIXARLOGDATAFMT_16BIT    4            // signed short samples
#define        PIXARLOGDATAFMT_FLOAT    5            // IEEE float samples
// 65550-65556 are allocated to Ken Murchison <ken@oceana.com>
#define TIFFTAG_DCSIMAGERTYPE           65550   // imager model & filter
#define     DCSIMAGERMODEL_M3           0       // M3 chip (1280 x 1024)
#define     DCSIMAGERMODEL_M5           1       // M5 chip (1536 x 1024)
#define     DCSIMAGERMODEL_M6           2       // M6 chip (3072 x 2048)
#define     DCSIMAGERFILTER_IR          0       // infrared filter
#define     DCSIMAGERFILTER_MONO        1       // monochrome filter
#define     DCSIMAGERFILTER_CFA         2       // color filter array
#define     DCSIMAGERFILTER_OTHER       3       // other filter
#define TIFFTAG_DCSINTERPMODE           65551   // interpolation mode
#define     DCSINTERPMODE_NORMAL        0x0     // whole image, default
#define     DCSINTERPMODE_PREVIEW       0x1     // preview of image (384x256)
#define TIFFTAG_DCSBALANCEARRAY         65552   // color balance values
#define TIFFTAG_DCSCORRECTMATRIX        65553   // color correction values
#define TIFFTAG_DCSGAMMA                65554   // gamma value
#define TIFFTAG_DCSTOESHOULDERPTS       65555   // toe & shoulder points
#define TIFFTAG_DCSCALIBRATIONFD        65556   // calibration file desc
// Note: quality level is on the ZLIB 1-9 scale. Default value is -1
#define    TIFFTAG_ZIPQUALITY        65557            // compression quality level




//
// --------------------------------------------------------------
// GeoTIFF definition...

#define TIFFGEO_VERSION     1
#define TIFFGEO_REV_MAJOR   1
#define    TIFFGEO_REV_MINOR   0


//
// EXIF and GPS IFDs
//
#define TIFFTAG_EXIFDIRECTORY         34665L
#define TIFFTAG_GPSDIRECTORY          34853L

//EXIF Tags
#define TIFFTAG_EXIF_EXPOSURETIME               33434L
#define TIFFTAG_EXIF_FNUMBER                    33437L
#define TIFFTAG_EXIF_EXPOSUREPROGRAM            34850L
#define TIFFTAG_EXIF_SPECTRALSENSITIVITY        34852L
#define TIFFTAG_EXIF_ISOSPEEDRATINGS            34855L
#define TIFFTAG_EXIF_OECF                       34856L
#define TIFFTAG_EXIF_EXIFVERSION                36864L
#define TIFFTAG_EXIF_DATETIMEORIGINAL           36867L
#define TIFFTAG_EXIF_DATETIMEDIGITIZED          36868L
#define TIFFTAG_EXIF_COMPONENTSCONFIGURATION    37121L
#define TIFFTAG_EXIF_COMPRESSEDBITSPERPIXEL     37122L
#define TIFFTAG_EXIF_SHUTTERSPEEDVALUE          37377L
#define TIFFTAG_EXIF_APERTUREVALUE              37378L
#define TIFFTAG_EXIF_BRIGHTNESSVALUE            37379L
#define TIFFTAG_EXIF_EXPOSUREBIASVALUE          37380L
#define TIFFTAG_EXIF_MAXAPERTUREVALUE           37381L
#define TIFFTAG_EXIF_SUBJECTDISTANCE            37382L
#define TIFFTAG_EXIF_METERINGMODE               37383L
#define TIFFTAG_EXIF_LIGHTSOURCE                37384L
#define TIFFTAG_EXIF_FLASH                      37385L
#define TIFFTAG_EXIF_FOCALLENGTH                37386L
#define TIFFTAG_EXIF_SUBJECTAREA                37396L
#define TIFFTAG_EXIF_MAKERNOTE                  37500L
#define TIFFTAG_EXIF_USERCOMMENT                37510L
#define TIFFTAG_EXIF_SUBSECTIME                 37520L
#define TIFFTAG_EXIF_SUBSECTIME_ORIGINAL        37521L
#define TIFFTAG_EXIF_SUBSECTIME_DIGITIZED       37522L
#define TIFFTAG_EXIF_FLASHPIXVERSION            40960L
#define TIFFTAG_EXIF_COLORSPACE                 40961L
#define TIFFTAG_EXIF_PIXELXDIMENSION            40962L
#define TIFFTAG_EXIF_PIXELYDIMENSION            40963L
#define TIFFTAG_EXIF_RELATEDSOUNDFILE           40964L
#define TIFFTAG_EXIF_FLASHENERGY                41483L
#define TIFFTAG_EXIF_SPATIALFREQUENCYRESPONSE   41484L
#define TIFFTAG_EXIF_FOCALPLANEXRESOLUTION      41486L
#define TIFFTAG_EXIF_FOCALPLANEYRESOLUTION      41487L
#define TIFFTAG_EXIF_FOCALPLANERESOLUTIONUNIT   41488L
#define TIFFTAG_EXIF_SUBJECTLOCATION            41492L
#define TIFFTAG_EXIF_EXPOSUREINDEX              41493L
#define TIFFTAG_EXIF_SENSINGMETHOD              41495L
#define TIFFTAG_EXIF_FILESOURCE                 41728L
#define TIFFTAG_EXIF_SCENETYPE                  41729L
#define TIFFTAG_EXIF_CFAPATTERN                 41730L
#define TIFFTAG_EXIF_CUSTOMRENDERED             41985L
#define TIFFTAG_EXIF_EXPOSUREMODE               41986L
#define TIFFTAG_EXIF_WHITEBALANCE               41987L
#define TIFFTAG_EXIF_DIGITALZOOMRATIO           41988L
#define TIFFTAG_EXIF_FOCALLENGTHIN35MMFILM      41989L
#define TIFFTAG_EXIF_SCENECAPTURETYPE           41990L
#define TIFFTAG_EXIF_GAINCONTROL                41991L
#define TIFFTAG_EXIF_CONTRAST                   41992L
#define TIFFTAG_EXIF_SATURATION                 41993L
#define TIFFTAG_EXIF_SHARPNESS                  41994L
#define TIFFTAG_EXIF_DEVICESETTINGDESCRIPTION   41995L
#define TIFFTAG_EXIF_SUBJECTDISTANCERANGE       41996L
#define TIFFTAG_EXIF_IMAGEUNIQUEID              42016L

//GPS Tags
#define TIFFTAG_GPS_VERSIONID           0L
#define TIFFTAG_GPS_LATITUDEREF         1L
#define TIFFTAG_GPS_LATITUDE            2L
#define TIFFTAG_GPS_LONGITUDEREF        3L
#define TIFFTAG_GPS_LONGITUDE           4L
#define TIFFTAG_GPS_ALTITUDEREF         5L
#define TIFFTAG_GPS_ALTITUDE            6L
#define TIFFTAG_GPS_TIMESTAMP           7L
#define TIFFTAG_GPS_SATELLITES          8L
#define TIFFTAG_GPS_STATUS              9L
#define TIFFTAG_GPS_MEASUREMODE        10L
#define TIFFTAG_GPS_DOP                11L
#define TIFFTAG_GPS_SPEEDREF           12L
#define TIFFTAG_GPS_SPEED              13L
#define TIFFTAG_GPS_TRACKREF           14L
#define TIFFTAG_GPS_TRACK              15L
#define TIFFTAG_GPS_IMGDIRECTIONREF    16L
#define TIFFTAG_GPS_IMGDIRECTION       17L
#define TIFFTAG_GPS_MAPDATUM           18L
#define TIFFTAG_GPS_DESTLATITUDEREF    19L
#define TIFFTAG_GPS_DESTLATITUDE       20L
#define TIFFTAG_GPS_DESTLONGITUDEREF   21L
#define TIFFTAG_GPS_DESTLONGITUDE      22L
#define TIFFTAG_GPS_DESTBEARINGREF     23L
#define TIFFTAG_GPS_DESTBEARING        24L
#define TIFFTAG_GPS_DESTDISTANCEREF    25L
#define TIFFTAG_GPS_DESTDISTANCE       26L
#define TIFFTAG_GPS_PROCESSINGMETHOD   27L
#define TIFFTAG_GPS_AREAINFORMATION    28L
#define TIFFTAG_GPS_DATESTAMP          29L
#define TIFFTAG_GPS_DIFFERENTIAL       30L



BEGIN_IMAGEPP_NAMESPACE

//
// GeoKey definitions
//
typedef enum {
    BaseGeoKey =                1024,           // First valid code

    GTModelType =                 1024,
    GTRasterType =                 1025,
    GTCitation =                 1026,
    GeographicType =             2048,
    GeogCitation =                 2049,
    GeogGeodeticDatum =         2050,
    GeogPrimeMeridian =         2051,
    GeogLinearUnits =             2052,
    GeogLinearUnitSize =         2053,
    GeogAngularUnits =             2054,
    GeogAngularUnitSize =         2055,
    GeogEllipsoid =             2056,
    GeogSemiMajorAxis =         2057,
    GeogSemiMinorAxis =         2058,
    GeogInvFlattening =         2059,
    GeogAzimuthUnits =             2060,
    GeogPrimeMeridianLong =     2061,
    ProjectedCSType =             3072,
    PCSCitation =                 3073,
    Projection =                 3074,
    ProjCoordTrans =             3075,
    ProjLinearUnits =             3076,
    ProjLinearUnitSize =         3077,
    ProjStdParallel1 =             3078,
    ProjStdParallel =           3078,   // Alias
    ProjStdParallel2 =             3079,
    ProjNatOriginLong =         3080,
    ProjOriginLong =            3080,   // Alias
    ProjNatOriginLat =             3081,
    ProjOriginLat =             3081,   // Alias
    ProjFalseEasting =             3082,
    ProjFalseNorthing =         3083,
    ProjFalseOriginLong =         3084,
    ProjFalseOriginLat =         3085,
    ProjFalseOriginEasting =       3086,
    ProjFalseOriginNorthing =     3087,
    ProjCenterLong =             3088,
    ProjCenterLat =             3089,
    ProjCenterEasting =         3090,
    ProjCenterNorthing =         3091,
    ProjScaleAtNatOrigin =         3092,
    ProjScaleAtOrigin =         3092,   // Alias
    ProjScaleAtCenter =         3093,
    ProjAzimuthAngle =             3094,
    ProjStraightVertPoleLong =     3095,
    ProjRectifiedGridAngle =    3096,
    VerticalCSType =             4096,
    VerticalCitation =             4097,
    VerticalDatum =             4098,
    VerticalUnits =             4099,

    ReservedEndGeoKey  =        32767,

    // Key space available for Private or internal use
    PrivateBaseGeoKey =         32768,      // Consistent with TIFF Private tags
    ProjectedCSTypeLong =       60000,

    PrivateEndGeoKey  =         65535,

    EndGeoKey =                 65535               // Largest Possible GeoKey ID
    } TIFFGeoKey;

END_IMAGEPP_NAMESPACE


// Universal key values -- defined for consistency
#define TIFFGeo_Undefined               0
#define TIFFGeo_UserDefined             32767

// 6.3.1.1 Model Type Codes
#define    TIFFGeo_ModelTypeProjected      1           // Projection Coordinate System
#define    TIFFGeo_ModelTypeGeographic     2           // Geographic latitude-longitude System
#define    TIFFGeo_ModelTypeGeocentric     3           // Geocentric (X,Y,Z) Coordinate System
#define    TIFFGeo_ModelProjected          ModelTypeProjected
#define    TIFFGeo_ModelGeographic         ModelTypeGeographic
#define    TIFFGeo_ModelGeocentric         ModelTypeGeocentric

#define TIFFGeo_ModelTypeSpecialMSSE    63554       // Special type created by MicroStation SE
#define TIFFGeo_ModelTypeSpecialMSJ     63248       // Special type created by MicroStation J  7.1.1.36

// 6.3.1.2 Raster Type Codes
#define TIFFGeo_RasterPixelIsArea       1   // Standard pixel-fills-grid-cell
#define    TIFFGeo_RasterPixelIsPoint      2   // Pixel-at-grid-vertex

#define TIFFGeo_Linear_Meter                          9001
#define TIFFGeo_Linear_Foot                         9002
#define TIFFGeo_Linear_Foot_US_Survey                 9003
#define TIFFGeo_British_Yard_1895                   9060

