//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This is the type used by the HRF
//-----------------------------------------------------------------------------
#pragma once

#include "HVEShape.h"

// Resolution Block Type
BEGIN_IMAGEPP_NAMESPACE
class HRFBlockType
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFBlockTypeId_Base)

    // File Type
    enum Block
        {
        LINE,
        TILE,
        STRIP,
        IMAGE,
        AUTO_DETECT
        };

    bool operator==(const HRFBlockType& pi_rObj) const
        {
        return (m_BlockType == pi_rObj.m_BlockType);
        };

    bool operator!=(const HRFBlockType& pi_rObj) const
        {
        return !operator==(pi_rObj);
        };

    explicit operator int() const
    {
        return (int)m_BlockType;
    };

    IMAGEPP_EXPORT HRFBlockType();
    IMAGEPP_EXPORT HRFBlockType(Block pi_BlockType);
    IMAGEPP_EXPORT HRFBlockType(const HRFBlockType& pi_rObj);

    Block m_BlockType;
    };


// Down Sampling Method
class HRFDownSamplingMethod
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFDownSamplingMethodId_Base)

    // File Type
    enum DownSamplingMethod
        {
        NEAREST_NEIGHBOUR   = 0x00,  // Default (nearest neigbor)
        AVERAGE             = 0x01,
        VECTOR_AWARENESS    = 0x02,
        UNKOWN              = 0x03,
        ORING4              = 0x04,
        NONE                = 0x05
        };

    IMAGEPP_EXPORT bool operator==(const HRFDownSamplingMethod& pi_rObj) const;
    bool operator!=(const HRFDownSamplingMethod& pi_rObj) const;

    IMAGEPP_EXPORT HRFDownSamplingMethod(int32_t pi_ForegroundIndex = -1);
    IMAGEPP_EXPORT HRFDownSamplingMethod(DownSamplingMethod pi_DownSamplingMethod, int32_t pi_ForegroundIndex = -1);
    IMAGEPP_EXPORT HRFDownSamplingMethod(const HRFDownSamplingMethod& pi_rObj);

    DownSamplingMethod m_DownSamplingMethod;
    int32_t            m_ForegroundIndex;
    };


// Encoding Type
class HRFEncodingType
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFEncodingTypeId_Base)

    // File Type
    enum EncodingType
        {
        STANDARD        = 0x00,
        MULTIRESOLUTION = 0x01,
        PROGRESSIVE     = 0x02
        };

    IMAGEPP_EXPORT bool operator==(const HRFEncodingType& pi_rObj) const;
    IMAGEPP_EXPORT bool operator!=(const HRFEncodingType& pi_rObj) const;

    IMAGEPP_EXPORT HRFEncodingType();
    IMAGEPP_EXPORT HRFEncodingType(EncodingType pi_EncodingType);
    IMAGEPP_EXPORT HRFEncodingType(const HRFEncodingType& pi_rObj);

    EncodingType m_EncodingType;
    };

// Image Size Mode
class HRFImageSizeMode
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFImageSizeModeId_Base)

    // File Type
    enum ImageSizeMode
        {
        ORIGINAL_SIZE = 0x00,
        RESAMPLE_SIZE = 0x01,
        CUSTOM_SIZE   = 0x02
        };

    bool operator==(const HRFImageSizeMode& pi_rObj) const;
    bool operator!=(const HRFImageSizeMode& pi_rObj) const;

    IMAGEPP_EXPORT HRFImageSizeMode();
    IMAGEPP_EXPORT HRFImageSizeMode(ImageSizeMode pi_ImageSizeMode);
    IMAGEPP_EXPORT HRFImageSizeMode(const HRFImageSizeMode& pi_rObj);

    ImageSizeMode m_ImageSizeMode;
    };


// Georeference Format
class HRFGeoreferenceFormat
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFGeoreferenceFormatId_Base)

    // File Type
    enum GeoreferenceFormat
        {
        GEOREFERENCE_IN_IMAGE       = 0x00,
        GEOREFERENCE_IN_HGR         = 0x01,
        GEOREFERENCE_IN_WORLD_FILE  = 0x02,
        };

    IMAGEPP_EXPORT bool operator==(const HRFGeoreferenceFormat& pi_rObj) const;
    bool operator!=(const HRFGeoreferenceFormat& pi_rObj) const;

    IMAGEPP_EXPORT HRFGeoreferenceFormat();
    IMAGEPP_EXPORT HRFGeoreferenceFormat(GeoreferenceFormat pi_GeoreferenceFormat);
    IMAGEPP_EXPORT HRFGeoreferenceFormat(const HRFGeoreferenceFormat& pi_rObj);

    GeoreferenceFormat m_GeoreferenceFormat;
    };


// Resolution Storage Access Type
class HRFBlockAccess
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFBlockAccessId_Base)

    // File Type
    enum Access
        {
        RANDOM,
        SEQUENTIAL
        };


    bool operator==(const HRFBlockAccess& pi_rObj) const
        {
        return (m_BlockAccess == pi_rObj.m_BlockAccess);
        };

    bool operator!=(const HRFBlockAccess& pi_rObj) const
        {
        return !operator==(pi_rObj);
        };

    IMAGEPP_EXPORT HRFBlockAccess();
    IMAGEPP_EXPORT HRFBlockAccess(Access pi_BlockAccess);
    IMAGEPP_EXPORT HRFBlockAccess(const HRFBlockAccess& pi_rObj);

    Access m_BlockAccess;
    };

// The order of value in the enum HRFScanlineOrientation is very important. Please do not change it.
// with this specific order you can examine the origine an the orientation easily .
// Bit 0 == 0 : Origin is at the left
// Bit 0 == 1 : Origin is at the right
// Bit 1 == 0 : Origin is at the upper
// Bit 1 == 1 : Origin is at the lower
// Bit 2 == 0 : Orientation is vertical
// Bit 2 == 1 : Orientation is horizontal


class HRFScanlineOrientation
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFScanlineOrientationId_Base)

    // File Type
    enum Scanline
        {
        UPPER_LEFT_VERTICAL = 0,
        UPPER_RIGHT_VERTICAL   ,
        LOWER_LEFT_VERTICAL    ,
        LOWER_RIGHT_VERTICAL   ,
        UPPER_LEFT_HORIZONTAL  ,
        UPPER_RIGHT_HORIZONTAL ,
        LOWER_LEFT_HORIZONTAL  ,
        LOWER_RIGHT_HORIZONTAL
        };


    bool operator==(const HRFScanlineOrientation& pi_rObj) const
        {
        return (m_ScanlineOrientation == pi_rObj.m_ScanlineOrientation);
        };

    bool operator!=(const HRFScanlineOrientation& pi_rObj) const
        {
        return !operator==(pi_rObj);
        };

    explicit operator int() const
        {
        return (int)m_ScanlineOrientation;
        };

    IMAGEPP_EXPORT bool IsScanlineVertical() const;
    IMAGEPP_EXPORT bool IsScanlineHorizontal() const;
    IMAGEPP_EXPORT bool IsUpper() const;
    IMAGEPP_EXPORT bool IsLower() const;
    IMAGEPP_EXPORT bool IsRight() const;
    IMAGEPP_EXPORT bool IsLeft() const;

    IMAGEPP_EXPORT HRFScanlineOrientation();
    IMAGEPP_EXPORT HRFScanlineOrientation(Scanline pi_ScanlineOrientation);
    IMAGEPP_EXPORT HRFScanlineOrientation(const HRFScanlineOrientation& pi_rObj);

    Scanline m_ScanlineOrientation;
    };

// Interleave Type
class HRFInterleaveType
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFInterleaveTypeId_Base)

    // File Type
    enum InterleaveType
        {
        PIXEL,
        LINE,
        PLANE
        };


    bool operator==(const HRFInterleaveType& pi_rObj) const
        {
        return (m_InterleaveType == pi_rObj.m_InterleaveType);
        };

    bool operator!=(const HRFInterleaveType& pi_rObj) const
        {
        return !operator==(pi_rObj);
        };

    IMAGEPP_EXPORT HRFInterleaveType();
    IMAGEPP_EXPORT HRFInterleaveType(InterleaveType pi_InterleaveType);
    IMAGEPP_EXPORT HRFInterleaveType(const HRFInterleaveType& pi_rObj);

    InterleaveType m_InterleaveType;
    };

// CoordinateType
class HRFCoordinateType
    {
public:
    HDECLARE_SEALEDCLASS_ID(HRFCoordinateTypeId_Base)

    // File Type
    enum CoordinateType
        {
        LOGICAL,
        PHYSICAL
        };


    bool operator==(const HRFCoordinateType& pi_rObj) const
        {
        return (m_CoordinateType == pi_rObj.m_CoordinateType);
        };

    bool operator!=(const HRFCoordinateType& pi_rObj) const
        {
        return !operator==(pi_rObj);
        };

    IMAGEPP_EXPORT HRFCoordinateType();
    IMAGEPP_EXPORT HRFCoordinateType(CoordinateType pi_CoordinateType);
    IMAGEPP_EXPORT HRFCoordinateType(const HRFCoordinateType& pi_rObj);

    CoordinateType m_CoordinateType;
    };

// HRFClipShape
class HRFClipShape : public HVEShape // first step to move to the HGF2DFence<double>
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRFClipShapeId)
public:
    // Constructor
    IMAGEPP_EXPORT HRFClipShape(); // for persistence
    IMAGEPP_EXPORT HRFClipShape(const HRFClipShape&  pi_rClipShape);
    IMAGEPP_EXPORT HRFClipShape(const HRFClipShape&  pi_rClipShape,
                        HRFCoordinateType    pi_CoordinateType);

    IMAGEPP_EXPORT HRFClipShape(const HVEShape&      pi_rObj,
                        HRFCoordinateType    pi_CoordinateType); // allow to give a rectangle

    IMAGEPP_EXPORT HRFClipShape(size_t*                        po_pBufferLength,
                        double*                       pi_pBuffer,
                        HRFCoordinateType              pi_CoordinateType);


    IMAGEPP_EXPORT HRFCoordinateType  GetCoordinateType() const;

protected:
    HRFCoordinateType  m_CoordinateType;
    };


// Data Flag
typedef Byte   HRFDataFlag;
// reserved                         0x00
//  The SetBlockDataFlag method mtually exclude these value
#define HRFDATAFLAG_EMPTY           0x01    // Exclusif
#define HRFDATAFLAG_LOADED          0x02    // Exclusif
#define HRFDATAFLAG_OVERWRITTEN     0x04    // Exclusif

#define HRFDATAFLAG_TOBECLEAR       0x08    // or
#define HRFDATAFLAG_DIRTYFORSUBRES  0x10    // or

#define HRF_EQUAL_TO_RESOLUTION_WIDTH    0
#define HRF_EQUAL_TO_RESOLUTION_HEIGHT   LONG_MAX
END_IMAGEPP_NAMESPACE


