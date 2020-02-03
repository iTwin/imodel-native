/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/CustomTypes.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "stdafx.h"

    #define _tcout std::wcout
    #define _tcerr std::wcerr
    typedef std::wistream itstream;
    typedef std::wostream otstream;
    typedef std::wfstream tfstream;

// Custom types and enums
typedef std::vector<WString> StringList;
typedef std::map<int, bool> BooleanMap;

enum AccessModes {
    Create,
    Read,
    Write
};

// Block Type Object
struct BlockObject
{
    int ID;
    WString label;
    long minimumWidth;
    long minimumHeight;
    long maximumWidth;
    long maximumHeight;
    long heightIncrement;
    long widthIncrement;
    long maxSizeInBytes;
    WString access;
    BooleanMap accessMode;
    bool isMultiResolution;
    bool isUnlimitedResolution;
};
typedef std::multimap<WString, BlockObject> BlockMap;
//The use of multimap here is not mandatory. In fact, a simpler data type could be used to simplify things.

// Compression Codec Object
struct CodecObject
{
    long ID;
    WString label;
    BooleanMap accessMode;
    BlockMap blockTypes;
};
typedef std::map<WString, CodecObject> CodecMap;

// DownSampling Methods
struct DownSamplingObject
{
    long ID;
    WString label;
};
typedef std::map<WString, DownSamplingObject> DownSamplingMap;

// Pixel Type Object
struct ColorSpaceObject
{
    WString label;
    long ID;
    BooleanMap accessMode;
    CodecMap supportedCompressions;
    DownSamplingMap downSamplingMethods;
};
typedef std::map<WString, ColorSpaceObject> ColorSpaceMap;

// Tag Object
struct TagObject
{
    WString tag;
    BooleanMap accessMode;
};
typedef std::map<WString, TagObject> TagMap;

// Scanline Orientation
struct ScanlineOrientation
{
    long ID;
    WString label;
};

// Transformation Model
struct TransfoModel 
{
    long ID;
    WString label;
    BooleanMap accessMode;
};
typedef std::map<WString, TransfoModel> TransfoModelMap;

// Interleave Type
struct InterleaveType
{
    long ID;
    WString label;
    BooleanMap accessMode;
};

// Thumbnail support
struct ThumbnailSupport
{
    bool isSupported;
    BooleanMap accessMode;
};

// MultiPage Support
struct MultiPageSupport
{
    bool isSupported;
    BooleanMap accessMode;
};

// Histogram Support
struct HistogramSupport
{
    bool isSupported;
    BooleanMap accessMode;
};

// Geocoding Supoprt
struct GeocodingSupport
{
    bool isSupported;
    BooleanMap accessMode;
}; 

// Unlimited Resolution
struct UnlimitedResolution
{
    bool isUnlimited;
};

// Object representing a supported format
struct ImageFormat
{
    long                    id;
    WString                 label;
    WString                 extensions;
    
    ColorSpaceMap           supportedColorSpace;
    GeocodingSupport        geocodingSupport;
    HistogramSupport        histogramSupport;
    InterleaveType          interleaveType;
    MultiPageSupport        multiPageSupport;
    ScanlineOrientation     SLO;
    StringList              geokeys;
    TagMap                  tags;
    ThumbnailSupport        thumbnailSupport;
    TransfoModelMap         transfoModels;
    UnlimitedResolution     unlimitedResolution;
};
typedef std::map<WString, ImageFormat> ImageFormatMap;
