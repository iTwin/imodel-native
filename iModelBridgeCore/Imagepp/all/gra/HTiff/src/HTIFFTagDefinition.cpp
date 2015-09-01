//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFTagDefinition.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFTagDefinition
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HTIFFTagDefinition.h>
#include <Imagepp/all/h/HTIFFUtils.h>




bool HTIFFTagInfo::IsBigTiffTag  (uint32_t FileTagNumber) const
    {
    switch(FileTagNumber)
        {
            // Only these tags support LONG64
        case TIFFTAG_STRIPOFFSETS:
        case TIFFTAG_STRIPBYTECOUNTS:
        case TIFFTAG_TILEOFFSETS:
        case TIFFTAG_TILEBYTECOUNTS:
        case TIFFTAG_FREEOFFSETS:
        case TIFFTAG_FREEBYTECOUNTS:
        case TIFFTAG_HMR_IMAGEINFORMATION:
        case TIFFTAG_HMR2_IMAGEINFORMATION:
        case TIFFTAG_HMR_PROJECTWISE_BLOB:
            return true;
        default:
            return false;
        }
    }

//
// NB: NB: THIS ARRAY IS ASSUMED TO BE SORTED BY TAG.
//
// If you add a Tag in this list, you must update the Enum in the
// file HTIFFTag.h. (the sequence must be keep)
//

const HTagInfo::Info HTIFFTagInfo::sTagInfo[] = {

    // EXIF Standard's GPS Tags
        {   TIFFTAG_GPS_VERSIONID, GPS_VERSIONID, 4, 4,
        BYTE, false, false, "GPSVersionID"
        },
        {   TIFFTAG_GPS_LATITUDEREF, GPS_LATITUDEREF, 2, 2,
        ASCII, false, false, "GPSLatitudeRef"
        },
        {   TIFFTAG_GPS_LATITUDE, GPS_LATITUDE, 3, 3,
        RATIONAL, false, false, "GPSLatitude"
        },
        {   TIFFTAG_GPS_LONGITUDEREF, GPS_LONGITUDEREF, 2, 2,
        ASCII, false, false, "GPSLongitudeRef"
        },
        {   TIFFTAG_GPS_LONGITUDE, GPS_LONGITUDE, 3, 3,
        RATIONAL, false, false, "GPSLongitude"
        },
        {   TIFFTAG_GPS_ALTITUDEREF, GPS_ALTITUDEREF, 1, 1,
        BYTE, false, false, "GPSAltitudeRef"
        },
        {   TIFFTAG_GPS_ALTITUDE, GPS_ALTITUDE, 1, 1,
        RATIONAL, false, false, "GPSAltitude"
        },
        {   TIFFTAG_GPS_TIMESTAMP, GPS_TIMESTAMP, 3, 3,
        RATIONAL, false, false, "GPSTimeStamp"
        },
        {   TIFFTAG_GPS_SATELLITES, GPS_SATELLITES, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "GPSSatellites"
        },
        {   TIFFTAG_GPS_STATUS, GPS_STATUS, 2, 2,
        ASCII, false, false, "GPSStatus"
        },
        {   TIFFTAG_GPS_MEASUREMODE, GPS_MEASUREMODE, 2, 2,
        ASCII, false, false, "GPSMeasureMode"
        },
        {   TIFFTAG_GPS_DOP, GPS_DOP, 1, 1,
        RATIONAL, false, false, "GPSDOP"
        },
        {   TIFFTAG_GPS_SPEEDREF, GPS_SPEEDREF, 2, 2,
        ASCII, false, false, "GPSSpeedRef"
        },
        {   TIFFTAG_GPS_SPEED, GPS_SPEED, 1, 1,
        RATIONAL, false, false, "GPSSpeed"
        },
        {   TIFFTAG_GPS_TRACKREF, GPS_TRACKREF, 2, 2,
        ASCII, false, false, "GPSTrackRef"
        },
        {   TIFFTAG_GPS_TRACK, GPS_TRACK, 1, 1,
        RATIONAL, false, false, "GPSTrack"
        },
        {   TIFFTAG_GPS_IMGDIRECTIONREF, GPS_IMGDIRECTIONREF, 2, 2,
        ASCII, false, false, "GPSImgDirectionRef"
        },
        {   TIFFTAG_GPS_IMGDIRECTION, GPS_IMGDIRECTION, 1, 1,
        RATIONAL, false, false, "GPSImgDirection"
        },
        {   TIFFTAG_GPS_MAPDATUM, GPS_MAPDATUM, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "GPSMapDatum"
        },
        {   TIFFTAG_GPS_DESTLATITUDEREF, GPS_DESTLATITUDEREF, 2, 2,
        ASCII, false, false, "GPSDestLatitudeRef"
        },
        {   TIFFTAG_GPS_DESTLATITUDE, GPS_DESTLATITUDE, 3, 3,
        RATIONAL, false, false, "GPSDestLatitude"
        },
        {   TIFFTAG_GPS_DESTLONGITUDEREF, GPS_DESTLONGITUDEREF, 2, 2,
        ASCII, false, false, "GPSDestLongitudeRef"
        },
        {   TIFFTAG_GPS_DESTLONGITUDE, GPS_DESTLONGITUDE, 3, 3,
        RATIONAL, false, false, "GPSDestLongitude"
        },
        {   TIFFTAG_GPS_DESTBEARINGREF, GPS_DESTBEARINGREF, 2, 2,
        ASCII, false, false, "GPSDestBearingRef"
        },
        {   TIFFTAG_GPS_DESTBEARING, GPS_DESTBEARING, 1, 1,
        RATIONAL, false, false, "GPSDestBearing"
        },
        {   TIFFTAG_GPS_DESTDISTANCEREF, GPS_DESTDISTANCEREF, 2, 2,
        ASCII, false, false, "GPSDestDistanceRef"
        },
        {   TIFFTAG_GPS_DESTDISTANCE, GPS_DESTDISTANCE, 1, 1,
        RATIONAL, false, false, "GPSDestDistance"
        },
        {   TIFFTAG_GPS_PROCESSINGMETHOD, GPS_PROCESSINGMETHOD, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "GPSProcessingMethod"
        },
        {   TIFFTAG_GPS_AREAINFORMATION, GPS_AREAINFORMATION, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "GPSAreaInformation"
        },
        {   TIFFTAG_GPS_DATESTAMP, GPS_DATESTAMP, 11, 11,
        ASCII, false, false, "GPSDateStamp"
        },
        {   TIFFTAG_GPS_DIFFERENTIAL, GPS_DIFFERENTIAL, 1, 1,
        SHORT, false, false, "GPSDifferential"
        },

        {   TIFFTAG_SUBFILETYPE, SUBFILETYPE,         1,              1,
        LONG,     true, false, "SubfileType"
        },
        {   TIFFTAG_SUBFILETYPE, SUBFILETYPE,         1,              1,
        SHORT,    true, false, "SubfileType"
        },
        {   TIFFTAG_OSUBFILETYPE, OSUBFILETYPE,       1,              1,
        SHORT,    true, false, "OldSubfileType"
        },
        {   TIFFTAG_IMAGEWIDTH, IMAGEWIDTH,           1,              1,
        LONG,     false,false, "ImageWidth"
        },
        {   TIFFTAG_IMAGEWIDTH, IMAGEWIDTH,           1,              1,
        SHORT,    false,false, "ImageWidth"
        },
        {   TIFFTAG_IMAGELENGTH, IMAGELENGTH,         1,              1,
        LONG,     true, false, "ImageLength"
        },
        {   TIFFTAG_IMAGELENGTH, IMAGELENGTH,         1,              1,
        SHORT,    true, false, "ImageLength"
        },
        {   TIFFTAG_BITSPERSAMPLE, BITSPERSAMPLE,     TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        SHORT,    false,false,    "BitsPerSample"
        },
        {   TIFFTAG_COMPRESSION, COMPRESSION,         TAG_IO_VARIABLE,1,
        SHORT,    false,false, "Compression"
        },
    //TR 312623 : The long definition for the TIFFTAG_COMPRESSION should be
    //after the short definition to ensure that Image++ writes a short
    //instead of long value (i.e. : the standard is short).
        {   TIFFTAG_COMPRESSION, COMPRESSION,         TAG_IO_VARIABLE,1,
        LONG,    false,false, "Compression"
        },
        {   TIFFTAG_PHOTOMETRIC, PHOTOMETRIC,         1,              1,
        SHORT,    false,false, "PhotometricInterpretation"
        },
        {   TIFFTAG_THRESHHOLDING, THRESHHOLDING,     1,              1,
        SHORT,    true, false, "Threshholding"
        },
        {   TIFFTAG_CELLWIDTH, CELLWIDTH,             1,              1,
        SHORT,    true, false, "CellWidth"
        },
        {   TIFFTAG_CELLLENGTH, CELLLENGTH,           1,              1,
        SHORT,    true, false, "CellLength"
        },
        {   TIFFTAG_FILLORDER, FILLORDER,             1,              1,
        SHORT,    false,false, "FillOrder"
        },
        {   TIFFTAG_DOCUMENTNAME, DOCUMENTNAME,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true,false, "DocumentName"
        },
        {   TIFFTAG_IMAGEDESCRIPTION, IMAGEDESCRIPTION,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true,false, "ImageDescription"
        },
        {   TIFFTAG_MAKE, MAKE,                       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true,false, "Make"
        },
        {   TIFFTAG_MODEL, MODEL,                     TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true,false, "Model"
        },
        {   TIFFTAG_STRIPOFFSETS, STRIPOFFSETS,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG,     false,false, "StripOffsets"
        },
        {   TIFFTAG_STRIPOFFSETS, STRIPOFFSETS,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG64,   false,false, "StripOffsets"
        },
        {   TIFFTAG_STRIPOFFSETS, STRIPOFFSETS,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,    false,false, "StripOffsets"
        },
        {   TIFFTAG_ORIENTATION, ORIENTATION,        1,              1,
        SHORT,    false,false,    "Orientation"
        },
        {   TIFFTAG_SAMPLESPERPIXEL, SAMPLESPERPIXEL, 1,              1,
        SHORT,    false,false, "SamplesPerPixel"
        },
        {   TIFFTAG_SAMPLESPERPIXEL, SAMPLESPERPIXEL, 1,              1,
        LONG,     false,false, "SamplesPerPixel"
        },
        {   TIFFTAG_ROWSPERSTRIP, ROWSPERSTRIP,       1,              1,
        LONG,     false,false, "RowsPerStrip"
        },
        {   TIFFTAG_ROWSPERSTRIP, ROWSPERSTRIP,       1,              1,
        SHORT,    false,false, "RowsPerStrip"
        },
        {   TIFFTAG_STRIPBYTECOUNTS, STRIPBYTECOUNTS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG,     false,false, "StripByteCounts"
        },
        {   TIFFTAG_STRIPBYTECOUNTS, STRIPBYTECOUNTS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG64,   false,false, "StripByteCounts"
        },
        {   TIFFTAG_STRIPBYTECOUNTS,STRIPBYTECOUNTS,  TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,    false,false, "StripByteCounts"
        },
        {   TIFFTAG_MINSAMPLEVALUE, MINSAMPLEVALUE,   TAG_IO_USE_SPP,TAG_IO_USE_SPP,
        SHORT,    true, false, "MinSampleValue"
        },
        {   TIFFTAG_MAXSAMPLEVALUE, MAXSAMPLEVALUE,   TAG_IO_USE_SPP,TAG_IO_USE_SPP,
        SHORT,    true, false, "MaxSampleValue"
        },
        {   TIFFTAG_XRESOLUTION, XRESOLUTION,         1,              1,
        RATIONAL, false,false, "XResolution"
        },
        {   TIFFTAG_YRESOLUTION, YRESOLUTION,         1,              1,
        RATIONAL, false,false,    "YResolution"
        },
        {   TIFFTAG_PLANARCONFIG, PLANARCONFIG,       1,              1,
        SHORT,    false,false,    "PlanarConfiguration"
        },
        {   TIFFTAG_PAGENAME, PAGENAME,               TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true, false, "PageName"
        },
        {   TIFFTAG_XPOSITION, XPOSITION,             1,              1,
        RATIONAL, true, false, "XPosition"
        },
        {   TIFFTAG_YPOSITION, YPOSITION,             1,              1,
        RATIONAL, true, false, "YPosition"
        },
        {   TIFFTAG_FREEOFFSETS, FREEOFFSETS,         TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG,     false,false, "FreeOffsets"
        },
        {   TIFFTAG_FREEOFFSETS, FREEOFFSETS,         TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG64,   false,false, "FreeOffsets"
        },
        {   TIFFTAG_FREEBYTECOUNTS,   FREEBYTECOUNTS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG,     false,false, "FreeByteCounts"
        },
        {   TIFFTAG_FREEBYTECOUNTS,   FREEBYTECOUNTS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG64,   false,false, "FreeByteCounts"
        },
        {   TIFFTAG_GRAYRESPONSEUNIT, GRAYRESPONSEUNIT,1,             1,
        SHORT,   true, false, "GrayResponseUnit"
        },
        {   TIFFTAG_GRAYRESPONSECURVE, GRAYRESPONSECURVE,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,   true, false, "GrayResponseCurve"
        },
        {   TIFFTAG_GROUP3OPTIONS, GROUP3OPTIONS,     1,              1,
        LONG,    false,false, "CCITTGroup3Options"
        },
        {   TIFFTAG_RESOLUTIONUNIT, RESOLUTIONUNIT,   1,              1,
        SHORT,   false,false, "ResolutionUnit"
        },
        {   TIFFTAG_PAGENUMBER, PAGENUMBER,           2,              2,
        SHORT,   true, false, "PageNumber"
        },
        {   TIFFTAG_COLORRESPONSEUNIT, COLORRESPONSEUNIT,1,           1,
        SHORT,   true, false, "ColorResponseUnit"
        },
        {   TIFFTAG_TRANSFERFUNCTION, TRANSFERFUNCTION,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,   true, false, "TransferFunction"
        },
        {   TIFFTAG_SOFTWARE, SOFTWARE,               TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,   true, false, "Software"
        },
        {   TIFFTAG_DATETIME, DATETIME,               20,             20,
        ASCII,   true, false, "DateTime"
        },
        {   TIFFTAG_ARTIST, ARTIST,                   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,   true, false, "Artist"
        },
        {   TIFFTAG_HOSTCOMPUTER, HOSTCOMPUTER,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,   true, false, "HostComputer"
        },
        {   TIFFTAG_PREDICTOR, PREDICTOR,         1,              1,
        SHORT,    false,false, "Predictor"
        },
        {   TIFFTAG_WHITEPOINT, WHITEPOINT,           2,              2,
        RATIONAL,true, false, "WhitePoint"
        },
        {   TIFFTAG_PRIMARYCHROMATICITIES,PRIMARYCHROMATICITIES,6,    6,
        RATIONAL,true, false, "PrimaryChromaticities"
        },
        {   TIFFTAG_COLORMAP, TCOLORMAP,              TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,   true, false, "ColorMap"
        },
        {   TIFFTAG_HALFTONEHINTS, HALFTONEHINTS,     2,              2,
        SHORT,   true, false, "HalftoneHints"
        },
        {   TIFFTAG_TILEWIDTH, TILEWIDTH,             1,              1,
        LONG,    false,false, "TileWidth"
        },
        {   TIFFTAG_TILEWIDTH, TILEWIDTH,             1,              1,
        SHORT,   false,false, "TileWidth"
        },
        {   TIFFTAG_TILELENGTH, TILELENGTH,           1,              1,
        LONG,    false,false, "TileLength"
        },
        {   TIFFTAG_TILELENGTH, TILELENGTH,           1,              1,
        SHORT,   false,false, "TileLength"
        },
        {   TIFFTAG_TILEOFFSETS, TILEOFFSETS,         TAG_IO_VARIABLE,1,
        LONG,    false,false, "TileOffsets"
        },
        {   TIFFTAG_TILEOFFSETS, TILEOFFSETS,         TAG_IO_VARIABLE,1,
        LONG64,  false,false, "TileOffsets"
        },
        {   TIFFTAG_TILEBYTECOUNTS, TILEBYTECOUNTS,   TAG_IO_VARIABLE,1,
        LONG,    false,false, "TileByteCounts"
        },
        {   TIFFTAG_TILEBYTECOUNTS, TILEBYTECOUNTS,   TAG_IO_VARIABLE,1,
        LONG64,    false,false, "TileByteCounts"
        },
        {   TIFFTAG_TILEBYTECOUNTS, TILEBYTECOUNTS,   TAG_IO_VARIABLE,1,
        SHORT,   false,false, "TileByteCounts"
        },
        {   TIFFTAG_SUBIFD, SUBIFD,                   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG,    true,  true, "SubIFD"
        },
        {   TIFFTAG_INKSET, INKSET,                   1,              1,
        SHORT,   false,false, "InkSet"
        },
        {   TIFFTAG_INKNAMES, INKNAMES,               TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,   true, false, "InkNames"
        },
        {   TIFFTAG_DOTRANGE, DOTRANGE,               2,              2,
        SHORT,   false,false, "DotRange"
        },
        {   TIFFTAG_TARGETPRINTER, TARGETPRINTER,     TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,   true, false, "TargetPrinter"
        },
        {   TIFFTAG_EXTRASAMPLES, EXTRASAMPLES,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,   false,false, "ExtraSamples"
        },
        {   TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT,   false,false, "SampleFormat"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        DOUBLE,   true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        FLOAT,    true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        RATIONAL, true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        LONG,     true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        SHORT,    true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMINSAMPLEVALUE,  SMINSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        BYTE,     true,   false,  "SMinSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        DOUBLE,   true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        FLOAT,    true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        RATIONAL, true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        LONG,     true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        SHORT,    true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_SMAXSAMPLEVALUE,  SMAXSAMPLEVALUE,    TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        BYTE,     true,   false,  "SMaxSampleValue"
        },
        {   TIFFTAG_JPEGTABLES, JPEGTABLES,            TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        UNDEFINED,  true, false, "JPEGTables"
        },
        {   TIFFTAG_JPEGTABLES, JPEGTABLES,            TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        BYTE,  true, false, "JPEGTables"
        },
        {   TIFFTAG_YCBCRCOEFFICIENTS, YCBCRCOEFFICIENTS, 3,          3,
        RATIONAL,false,false, "YCbCrCoefficients"
        },
        {   TIFFTAG_YCBCRSUBSAMPLING, YCBCRSUBSAMPLING,   2,          2,
        SHORT,   false,false, "YCbCrSubsampling"
        },
        {   TIFFTAG_YCBCRPOSITIONING, YCBCRPOSITIONING,   1,          1,
        SHORT,   false,false, "YCbCrPositioning"
        },
        {   TIFFTAG_REFERENCEBLACKWHITE, REFERENCEBLACKWHITE,6,       6,
        RATIONAL,true, false, "ReferenceBlackWhite"
        },
        {   TIFFTAG_REFERENCEBLACKWHITE, REFERENCEBLACKWHITE,6,       6,
        LONG,    true, false, "ReferenceBlackWhite"
        },

// HMR Tags
//
        {   TIFFTAG_HMR_IMAGECOORDINATESYSTEM, HMR_IMAGECOORDINATESYSTEM,TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII,   true, false, "HMR_ImageCoordinateSystem"
        },
        {   TIFFTAG_HMR_XORIGIN, HMR_XORIGIN,                                          1,               1,
        DOUBLE,  true, false, "HMR_XOrigin"
        },
        {   TIFFTAG_HMR_YORIGIN, HMR_YORIGIN,                                          1,               1,
        DOUBLE,  true, false, "HMR_YOrigin"
        },
        {   TIFFTAG_HMR_XPIXELSIZE, HMR_XPIXELSIZE,                                    1,               1,
        DOUBLE,  true, false, "HMR_XPixelSize"
        },
        {   TIFFTAG_HMR_YPIXELSIZE, HMR_YPIXELSIZE,                                    1,               1,
        DOUBLE,  true, false, "HMR_YPixelSize"
        },
        {   TIFFTAG_HMR2_3D_TRANSFO_MATRIX, HMR2_3D_TRANSFO_MATRIX,   TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        DOUBLE, true, true, "HMR2_3DTransformationMatrix"
        },
    // Well Known Text (WKT) Geocoding information
        {   TIFFTAG_HMR2_WELLKNOWNTEXT, HMR2_WELLKNOWNTEXT,   TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, true, false, "HMR2_WellKnownText"
        },
        {   TIFFTAG_HMR2_ONDEMANDRASTERS_INFO, HMR2_ONDEMANDRASTERS_INFO,  TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        BYTE, true, false, "HMR2_OnDemandRastersInfo"
        },
        {   TIFFTAG_HMR_HISTOGRAM, HMR_HISTOGRAM,                        TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG,    true, false, "HMR_Histogram"
        },
        {   TIFFTAG_HMR_HISTOGRAMDATETIME,HMR_HISTOGRAMDATETIME,                         20,              20,
        ASCII,   true, false, "HMR_HistogramDateTime"
        },
        {   TIFFTAG_HMR_PIXEL_TYPE_SPEC, HMR_PIXEL_TYPE_SPEC,                             1,               1,
        SHORT,   true, false, "HMR_Pixel_Type_Spec"
        },
        {   TIFFTAG_HMR_TRANSPARENCY_PALETTE, HMR_TRANSPARENCY_PALETTE,    TAG_IO_VARIABLE,  TAG_IO_VARIABLE,
        BYTE,    true, false, "HMR_Transparency_Palette"
        },
        {   TIFFTAG_HMR_THUMBNAIL_COMPOSED, HMR_THUMBNAIL_COMPOSED,                       1,               1,
        SHORT,    true, false, "HMR_Thumbnail_Composed"
        },
        {   TIFFTAG_HMR2_CHANNELS_WITH_NODATAVALUE, HMR2_CHANNELS_WITH_NODATAVALUE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG,    true, false, "HMR_ChannelsWithNoDataValue"
        },
        {   TIFFTAG_HMR2_CHANNELS_NODATAVALUE, HMR2_CHANNELS_NODATAVALUE,   TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        DOUBLE,  true, false, "HMR_ChannelsNoDataValue"
        },
        {   TIFFTAG_HMR_VERSION, HMR_VERSION,                                             1,               1,
        LONG,    true, false, "HMR_Version"
        },
        {   TIFFTAG_HMR_VERSION_MINOR, HMR_VERSION_MINOR,                                 1,               1,
        LONG,    true, false, "HMR_Minor_Version"
        },
        {   TIFFTAG_HMR_PADDING, HMR_PADDING,                                             1,               1,
        LONG,    true, false, "HMR_Padding"
        },

    // Logical Shape data
        {   TIFFTAG_HMR_LOGICALSHAPE, HMR_LOGICALSHAPE,   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, false, "HMR_LogicalShape"
        },

    // transparent Shape data
        {   TIFFTAG_HMR_TRANSPARENTSHAPE, HMR_TRANSPARENTSHAPE, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, false, "HMR_TransparentShape"
        },

    // User data
        {   TIFFTAG_HMR_USERDATA, HMR_USERDATA,           TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,  true, false, "HMR_UserData"
        },

    // Tile Flags
        {   TIFFTAG_HMR2_TILEFLAG, HMR2_TILEFLAG,         TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,  true, false, "HMR_TileFlagInfo"
        },

    // Decimation method
        {   TIFFTAG_HMR_DECIMATION_METHOD, HMR_DECIMATION_METHOD, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        BYTE, true, false, "HMR_Decimation_Method"
        },
        {   TIFFTAG_HMR_DECIMATION_METHOD, HMR_DECIMATION_METHOD, 1, 1,
        LONG, true, false, "HMR_Decimation_Method"
        },

    // AltaPhoto Blob
//    { TIFFTAG_HMR_ALTAPHOTO_BLOB, HMR_ALTAPHOTO_BLOB,       TAG_IO_VARIABLE,   TAG_IO_VARIABLE,
//      LONG, true, false, "HMR_AltaPhoto_Blob" },
    // ProjectWise Blob
        {   TIFFTAG_HMR_PROJECTWISE_BLOB, HMR_PROJECTWISE_BLOB,       TAG_IO_VARIABLE,   TAG_IO_VARIABLE,
        LONG, true, false, "HMR_ProjectWise_Blob"
        },
        {   TIFFTAG_HMR_PROJECTWISE_BLOB, HMR_PROJECTWISE_BLOB,       TAG_IO_VARIABLE,   TAG_IO_VARIABLE,
        LONG64, true, false, "HMR_ProjectWise_Blob"
        },

    // Filters
        {   TIFFTAG_HMR_FILTERS, HMR_FILTERS,      TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, true, false, "HMR_Filter"
        } ,

    // Tag used by the cache(CTiff) to keep synchro the source file with the cache.
        {   TIFFTAG_HMR_SOURCEFILE_CREATIONDATE,HMR_SOURCEFILE_CREATIONDATE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII,   true, false, "HMR_SourceFileCreationDateTime"
        },

    // Concurrence support, this field is used to synchronize the file access.
    // This tag contain the offset in file of a LONG, used as a Counter for the synchro
        {   TIFFTAG_HMR_SYNCHRONIZE_FIELD, HMR_SYNCHRONIZE_FIELD, 1,  1,
        LONG,   false, false, "HMR_SynchronizeField"
        },

// SGI tags
        {   TIFFTAG_MATTEING, MATTEING,               1,              1,
        SHORT,  false,false, "Matteing"
        },
        {   TIFFTAG_DATATYPE, DATATYPE,               TAG_IO_USE_SPP, TAG_IO_USE_SPP,
        SHORT,  false,false, "DataType"
        },
        {   TIFFTAG_IMAGEDEPTH, IMAGEDEPTH,           1,              1,
        LONG,   false,false, "ImageDepth"
        },
        {   TIFFTAG_IMAGEDEPTH, IMAGEDEPTH,           1,              1,
        SHORT,  false,false,  "ImageDepth"
        },
        {   TIFFTAG_TILEDEPTH, TILEDEPTH,             1,              1,
        LONG,   false,false,  "TileDepth"
        },
        {   TIFFTAG_TILEDEPTH, TILEDEPTH,             1,              1,
        SHORT,  false,false,  "TileDepth"
        },
// End

        {   TIFFTAG_COPYRIGHT, COPYRIGHT,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII,    true,false, "CopyRight"
        },

// GeoTiff 1/2
        {   TIFFTAG_GEOPIXELSCALE, GEOPIXELSCALE,     TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, true, "GeoPixelScale"
        },
        {   TIFFTAG_INTERGRAPH_RAGBAG, INTERGRAPH_RAGBAG,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT, true, true, "Intergraph RagBag Georeference"
        },
        {   TIFFTAG_INTERGRAPH_MATRIX, INTERGRAPH_MATRIX,TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, true, "Intergraph TransformationMatrix"
        },
        {   TIFFTAG_GEOTIEPOINTS, GEOTIEPOINTS,       TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, true, "GeoTiePoints"
        },
        {   HTIFFTAG_GEOTRANSMATRIX, GEOTRANSMATRIX,   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE, true, true, "GeoTransformationMatrix"
        },

//EXIF tags
        {   TIFFTAG_EXIF_FNUMBER, EXIF_FNUMBER, 1, 1,
        RATIONAL, false, false, "FNumber"
        },
        {   TIFFTAG_EXIF_EXPOSUREPROGRAM, EXIF_EXPOSUREPROGRAM, 1, 1,
        SHORT, false, false, "ExposureProgram"
        },


// Private tags registered to HMR inc
        {   TIFFTAG_HMR_IMAGEINFORMATION, HMR_IMAGEINFORMATION, 1,     1,
        LONG,  true,  false, "HMR_ImageInformation"
        },
        {   TIFFTAG_HMR_IMAGEINFORMATION, HMR_IMAGEINFORMATION, 1,     1,
        LONG64,  true,  false, "HMR_ImageInformation"
        },
        {   TIFFTAG_HMR2_IMAGEINFORMATION, HMR2_IMAGEINFORMATION, 1,     1,
        LONG,  true,  false, "HMR2_ImageInformation"
        },
        {   TIFFTAG_HMR2_IMAGEINFORMATION, HMR2_IMAGEINFORMATION, 1,     1,
        LONG64,  true,  false, "HMR2_ImageInformation"
        },

        {   TIFFTAG_EXIFDIRECTORY, EXIFDIRECTORY, 1, 1,
        LONG, false, false, "ExifDirectory"
        },

// GeoTiff  2/2
        {   HTIFFTAG_GEOKEYDIRECTORY, GEOKEYDIRECTORY, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT, true,  true, "GeoKeyDirectory"
        },
        {   HTIFFTAG_GEODOUBLEPARAMS, GEODOUBLEPARAMS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE,true,  true, "GeoDoubleParams"
        },
        {   HTIFFTAG_GEOASCIIPARAMS, GEOASCIIPARAMS,   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII, true,  false, "GeoASCIIParams"
        },

// EXIF tags
        {   TIFFTAG_EXIF_EXPOSURETIME, EXIF_EXPOSURETIME, 1, 1,
        RATIONAL, false, false, "ExposureTime"
        },

        {   TIFFTAG_GPSDIRECTORY, GPSDIRECTORY, 1, 1,
        LONG, false, false, "GpsDirectory"
        },

//EXIF tags
        {   TIFFTAG_EXIF_SPECTRALSENSITIVITY, EXIF_SPECTRALSENSITIVITY, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "SpectralSensitivity"
        },
        {   TIFFTAG_EXIF_ISOSPEEDRATINGS, EXIF_ISOSPEEDRATINGS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        SHORT, false, true, "ISOSpeedRatings"
        },
        {   TIFFTAG_EXIF_OECF, EXIF_OECF, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "OptoelectricConversionFactor"
        },
        {   TIFFTAG_EXIF_EXIFVERSION, EXIF_EXIFVERSION, 4, 4,
        UNDEFINED, false, false, "ExifVersion"
        },
        {   TIFFTAG_EXIF_DATETIMEORIGINAL, EXIF_DATETIMEORIGINAL, 20, 20,
        ASCII, false, false, "DateTimeOriginal"
        },
        {   TIFFTAG_EXIF_DATETIMEDIGITIZED, EXIF_DATETIMEDIGITIZED, 20, 20,
        ASCII, false, false, "DateTimeDigitized"
        },
        {   TIFFTAG_EXIF_COMPONENTSCONFIGURATION, EXIF_COMPONENTSCONFIGURATION, 4, 4,
        UNDEFINED, false, false, "ComponentsConfiguration"
        },
        {   TIFFTAG_EXIF_COMPRESSEDBITSPERPIXEL, EXIF_COMPRESSEDBITSPERPIXEL, 1, 1,
        RATIONAL, false, false, "CompressedBitsPerPixel"
        },
        {   TIFFTAG_EXIF_SHUTTERSPEEDVALUE, EXIF_SHUTTERSPEEDVALUE, 1, 1,
        SRATIONAL, false, false, "ShutterSpeedValue"
        },
        {   TIFFTAG_EXIF_APERTUREVALUE, EXIF_APERTUREVALUE, 1, 1,
        RATIONAL, false, false, "ApertureValue"
        },
        {   TIFFTAG_EXIF_BRIGHTNESSVALUE, EXIF_BRIGHTNESSVALUE, 1, 1,
        SRATIONAL, false, false, "BrightnessValue"
        },
        {   TIFFTAG_EXIF_EXPOSUREBIASVALUE, EXIF_EXPOSUREBIASVALUE, 1, 1,
        SRATIONAL, false, false, "ExposureBiasValue"
        },
        {   TIFFTAG_EXIF_MAXAPERTUREVALUE, EXIF_MAXAPERTUREVALUE, 1, 1,
        RATIONAL, false, false, "MaxApertureValue"
        },
        {   TIFFTAG_EXIF_SUBJECTDISTANCE, EXIF_SUBJECTDISTANCE, 1, 1,
        RATIONAL, false, false, "SubjectDistance"
        },
        {   TIFFTAG_EXIF_METERINGMODE, EXIF_METERINGMODE, 1, 1,
        SHORT, false, false, "MeteringMode"
        },
        {   TIFFTAG_EXIF_LIGHTSOURCE, EXIF_LIGHTSOURCE, 1, 1,
        SHORT, false, false, "LightSource"
        },
        {   TIFFTAG_EXIF_FLASH, EXIF_FLASH, 1, 1,
        SHORT, false, false, "Flash"
        },
        {   TIFFTAG_EXIF_FOCALLENGTH, EXIF_FOCALLENGTH, 1, 1,
        RATIONAL, false, false, "FocalLength"
        },
        {   TIFFTAG_EXIF_SUBJECTAREA, EXIF_SUBJECTAREA, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        SHORT, false, true, "SubjectArea"
        },
        {   TIFFTAG_EXIF_MAKERNOTE, EXIF_MAKERNOTE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "MakerNote"
        },
        {   TIFFTAG_EXIF_USERCOMMENT, EXIF_USERCOMMENT, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "UserComment"
        },
        {   TIFFTAG_EXIF_SUBSECTIME, EXIF_SUBSECTIME, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "SubSecTime"
        },
        {   TIFFTAG_EXIF_SUBSECTIME_ORIGINAL, EXIF_SUBSECTIME_ORIGINAL, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "SubSecTime_Original"
        },
        {   TIFFTAG_EXIF_SUBSECTIME_DIGITIZED, EXIF_SUBSECTIME_DIGITIZED, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCII, false, false, "SubSecTime_Digitized"
        },
        {   TIFFTAG_EXIF_FLASHPIXVERSION, EXIF_FLASHPIXVERSION, 4, 4,
        UNDEFINED, false, false, "FlashpixVersion"
        },
        {   TIFFTAG_EXIF_COLORSPACE, EXIF_COLORSPACE, 1, 1,
        SHORT, false, false, "ColorSpace"
        },
        {   TIFFTAG_EXIF_PIXELXDIMENSION, EXIF_PIXELXDIMENSION, 1, 1,
        LONG, false, false, "PixelXDimension"
        },
        {   TIFFTAG_EXIF_PIXELXDIMENSION, EXIF_PIXELXDIMENSION, 1, 1,
        SHORT, false, false, "PixelXDimension"
        },
        {   TIFFTAG_EXIF_PIXELYDIMENSION, EXIF_PIXELYDIMENSION, 1, 1,
        LONG, false, false, "PixelYDimension"
        },
        {   TIFFTAG_EXIF_PIXELXDIMENSION, EXIF_PIXELYDIMENSION, 1, 1,
        SHORT, false, false, "PixelYDimension"
        },
        {   TIFFTAG_EXIF_RELATEDSOUNDFILE, EXIF_RELATEDSOUNDFILE, 13, 13,
        ASCII, false, false, "RelatedSoundFile"
        },
        {   TIFFTAG_EXIF_FLASHENERGY, EXIF_FLASHENERGY, 1, 1,
        RATIONAL, false, false, "FlashEnergy"
        },
        {   TIFFTAG_EXIF_SPATIALFREQUENCYRESPONSE, EXIF_SPATIALFREQUENCYRESPONSE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "SpatialFrequencyResponse"
        },
        {   TIFFTAG_EXIF_FOCALPLANEXRESOLUTION, EXIF_FOCALPLANEXRESOLUTION, 1, 1,
        RATIONAL, false, false, "FocalPlaneXResolution"
        },
        {   TIFFTAG_EXIF_FOCALPLANEYRESOLUTION, EXIF_FOCALPLANEYRESOLUTION, 1, 1,
        RATIONAL, false, false, "FocalPlaneYResolution"
        },
        {   TIFFTAG_EXIF_FOCALPLANERESOLUTIONUNIT, EXIF_FOCALPLANERESOLUTIONUNIT, 1, 1,
        SHORT, false, false, "FocalPlaneResolutionUnit"
        },
        {   TIFFTAG_EXIF_SUBJECTLOCATION, EXIF_SUBJECTLOCATION, 2, 2,
        SHORT, false, false, "SubjectLocation"
        },
        {   TIFFTAG_EXIF_EXPOSUREINDEX, EXIF_EXPOSUREINDEX, 1, 1,
        RATIONAL, false, false, "ExposureIndex"
        },
        {   TIFFTAG_EXIF_SENSINGMETHOD, EXIF_SENSINGMETHOD, 1, 1,
        SHORT, false, false, "SensingMethod"
        },
        {   TIFFTAG_EXIF_FILESOURCE, EXIF_FILESOURCE, 1, 1,
        UNDEFINED, false, false, "FileSource"
        },
        {   TIFFTAG_EXIF_SCENETYPE, EXIF_SCENETYPE, 1, 1,
        UNDEFINED, false, false, "SceneType"
        },
        {   TIFFTAG_EXIF_CFAPATTERN, EXIF_CFAPATTERN, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "CFAPattern"
        },
        {   TIFFTAG_EXIF_CUSTOMRENDERED, EXIF_CUSTOMRENDERED, 1, 1,
        SHORT, false, false, "CustomRendered"
        },
        {   TIFFTAG_EXIF_EXPOSUREMODE, EXIF_EXPOSUREMODE, 1, 1,
        SHORT, false, false, "ExposureMode"
        },
        {   TIFFTAG_EXIF_WHITEBALANCE, EXIF_WHITEBALANCE, 1, 1,
        SHORT, false, false, "WhiteBalance"
        },
        {   TIFFTAG_EXIF_DIGITALZOOMRATIO, EXIF_DIGITALZOOMRATIO, 1, 1,
        RATIONAL, false, false, "DigitalZoomRatio"
        },
        {   TIFFTAG_EXIF_FOCALLENGTHIN35MMFILM, EXIF_FOCALLENGTHIN35MMFILM, 1, 1,
        SHORT, false, false, "FocalLengthIn35mmFilm"
        },
        {   TIFFTAG_EXIF_SCENECAPTURETYPE, EXIF_SCENECAPTURETYPE, 1, 1,
        SHORT, false, false, "SceneCaptureType"
        },
        {   TIFFTAG_EXIF_GAINCONTROL, EXIF_GAINCONTROL, 1, 1,
        SHORT, false, false, "GainControl"
        },
        {   TIFFTAG_EXIF_CONTRAST, EXIF_CONTRAST, 1, 1,
        SHORT, false, false, "Contrast"
        },
        {   TIFFTAG_EXIF_SATURATION, EXIF_SATURATION, 1, 1,
        SHORT, false, false, "Saturation"
        },
        {   TIFFTAG_EXIF_SHARPNESS, EXIF_SHARPNESS, 1, 1,
        SHORT, false, false, "Sharpness"
        },
        {   TIFFTAG_EXIF_DEVICESETTINGDESCRIPTION, EXIF_DEVICESETTINGDESCRIPTION, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        UNDEFINED, false, true, "DeviceSettingDescription"
        },
        {   TIFFTAG_EXIF_SUBJECTDISTANCERANGE, EXIF_SUBJECTDISTANCERANGE, 1, 1,
        SHORT, false, false, "SubjectDistanceRange"
        },
        {   TIFFTAG_EXIF_IMAGEUNIQUEID, EXIF_IMAGEUNIQUEID, 33, 33,
        ASCII, false, false, "ImageUniqueID"
        },
    //GDAL_NODATA tag
        { TIFFTAG_GDAL_NODATA, GDALNODATA,   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
          ASCII, true,  false, "GDAL_NoData" 
		},      
//
// These tags are not saved in file
//
        {   TIFFTAG_JPEGQUALITY, COMPRESSION_QUALITY, 1,     1,
        LONG,  true,  false, "Compression Quality"
        },

        {   TIFFTAG_JPEGQUALITY, COMPRESSION_JPEGOPTIMIZECODING, 1,     1,
        SHORT,  true,  false, "Compression JPEGOptimizeCoding"
        },

        {   0, EndOfTag,  0,0,
        _NOTYPE, true, true, ""
        },
    };


const uint32_t HTIFFTagInfo::sNumberOfDef = (sizeof (sTagInfo) / sizeof (sTagInfo[0])) - 1;
