//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTypes.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HRFUtility implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFTypes.h>

//-----------------------------------------------------------------------------
// HRFBlockType for persistence
//-----------------------------------------------------------------------------
HRFBlockType::HRFBlockType()
    {
    }

HRFBlockType::HRFBlockType(Block pi_BlockType)
    {
    m_BlockType = pi_BlockType;
    }

HRFBlockType::HRFBlockType(const HRFBlockType& pi_rObj)
    {
    m_BlockType = pi_rObj.m_BlockType;
    }


//-----------------------------------------------------------------------------
// HRFDownSamplingMethod for persistence
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRFDownSamplingMethod::HRFDownSamplingMethod(int32_t pi_ForegroundIndex)
    {
    m_ForegroundIndex = pi_ForegroundIndex;
    }

HRFDownSamplingMethod::HRFDownSamplingMethod(DownSamplingMethod pi_DownSamplingMethod, int32_t pi_ForegroundIndex)
    {
    m_DownSamplingMethod = pi_DownSamplingMethod;
    m_ForegroundIndex    = pi_ForegroundIndex;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRFDownSamplingMethod::HRFDownSamplingMethod(const HRFDownSamplingMethod& pi_rObj)
    {
    m_DownSamplingMethod = pi_rObj.m_DownSamplingMethod;
    m_ForegroundIndex    = pi_rObj.m_ForegroundIndex;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFDownSamplingMethod::operator==(const HRFDownSamplingMethod& pi_rObj) const
    {
    return (m_DownSamplingMethod == pi_rObj.m_DownSamplingMethod);
    };

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFDownSamplingMethod::operator!=(const HRFDownSamplingMethod& pi_rObj) const
    {
    return (m_DownSamplingMethod != pi_rObj.m_DownSamplingMethod);
    };

//-----------------------------------------------------------------------------
// HRFEncodingType for persistence
//-----------------------------------------------------------------------------

HRFEncodingType::HRFEncodingType()
    {
    }

HRFEncodingType::HRFEncodingType(EncodingType pi_EncodingType)
    {
    m_EncodingType = pi_EncodingType;
    }

HRFEncodingType::HRFEncodingType(const HRFEncodingType& pi_rObj)
    {
    m_EncodingType = pi_rObj.m_EncodingType;
    }

bool HRFEncodingType::operator==(const HRFEncodingType& pi_rObj) const
    {
    return (m_EncodingType == pi_rObj.m_EncodingType);
    };

bool HRFEncodingType::operator!=(const HRFEncodingType& pi_rObj) const
    {
    return (m_EncodingType != pi_rObj.m_EncodingType);
    };


//-----------------------------------------------------------------------------
// HRFImageSizeMode for persistence
//-----------------------------------------------------------------------------

HRFImageSizeMode::HRFImageSizeMode()
    {
    }

HRFImageSizeMode::HRFImageSizeMode(ImageSizeMode pi_ImageSizeMode)
    {
    m_ImageSizeMode = pi_ImageSizeMode;
    }

HRFImageSizeMode::HRFImageSizeMode(const HRFImageSizeMode& pi_rObj)
    {
    m_ImageSizeMode = pi_rObj.m_ImageSizeMode;
    }

bool HRFImageSizeMode::operator==(const HRFImageSizeMode& pi_rObj) const
    {
    return (m_ImageSizeMode == pi_rObj.m_ImageSizeMode);
    };

bool HRFImageSizeMode::operator!=(const HRFImageSizeMode& pi_rObj) const
    {
    return (m_ImageSizeMode != pi_rObj.m_ImageSizeMode);
    };


//-----------------------------------------------------------------------------
// HRFGeoreferenceFormat for persistence
//-----------------------------------------------------------------------------


HRFGeoreferenceFormat::HRFGeoreferenceFormat()
    {
    }

HRFGeoreferenceFormat::HRFGeoreferenceFormat(GeoreferenceFormat pi_GeoreferenceFormat)
    {
    m_GeoreferenceFormat = pi_GeoreferenceFormat;
    }

HRFGeoreferenceFormat::HRFGeoreferenceFormat(const HRFGeoreferenceFormat& pi_rObj)
    {
    m_GeoreferenceFormat = pi_rObj.m_GeoreferenceFormat;
    }

bool HRFGeoreferenceFormat::operator==(const HRFGeoreferenceFormat& pi_rObj) const
    {
    return (m_GeoreferenceFormat == pi_rObj.m_GeoreferenceFormat);
    };

bool HRFGeoreferenceFormat::operator!=(const HRFGeoreferenceFormat& pi_rObj) const
    {
    return (m_GeoreferenceFormat != pi_rObj.m_GeoreferenceFormat);
    };

//-----------------------------------------------------------------------------
// HRFBlockAccess for persistence
//-----------------------------------------------------------------------------


HRFBlockAccess::HRFBlockAccess()
    {
    }

HRFBlockAccess::HRFBlockAccess(Access pi_BlockAccess)
    {
    m_BlockAccess = pi_BlockAccess;
    }

HRFBlockAccess::HRFBlockAccess(const HRFBlockAccess& pi_rObj)
    {
    m_BlockAccess = pi_rObj.m_BlockAccess;
    }

//-----------------------------------------------------------------------------
// HRFScanlineOrientation for persistence
//-----------------------------------------------------------------------------

HRFScanlineOrientation::HRFScanlineOrientation()
    {
    }

HRFScanlineOrientation::HRFScanlineOrientation(Scanline pi_ScanlineOrientation)
    {
    m_ScanlineOrientation = pi_ScanlineOrientation;
    }

HRFScanlineOrientation::HRFScanlineOrientation(const HRFScanlineOrientation& pi_rObj)
    {
    m_ScanlineOrientation = pi_rObj.m_ScanlineOrientation;
    }

bool HRFScanlineOrientation::IsScanlineVertical() const
    {
    return (((m_ScanlineOrientation & 0x04) == 0x04) ? false : true);
    }

bool HRFScanlineOrientation::IsScanlineHorizontal() const
    {
    return (((m_ScanlineOrientation & 0x04) == 0x04) ? true  : false);
    }

bool HRFScanlineOrientation::IsUpper() const
    {
    return (((m_ScanlineOrientation & 0x02) == 0x02) ? false : true);
    }

bool HRFScanlineOrientation::IsLower() const
    {
    return (((m_ScanlineOrientation & 0x02) == 0x02) ? true  : false);
    }

bool HRFScanlineOrientation::IsRight() const
    {
    return (((m_ScanlineOrientation & 0x01) == 0x01) ? true  : false);
    }

bool HRFScanlineOrientation::IsLeft() const
    {
    return (((m_ScanlineOrientation & 0x01) == 0x01) ? false : true);
    }

//-----------------------------------------------------------------------------
// HRFInterleaveType for persistence
//-----------------------------------------------------------------------------

HRFInterleaveType::HRFInterleaveType()
    {
    }

HRFInterleaveType::HRFInterleaveType(InterleaveType pi_InterleaveType)
    {
    m_InterleaveType = pi_InterleaveType;
    }

HRFInterleaveType::HRFInterleaveType(const HRFInterleaveType& pi_rObj)
    {
    m_InterleaveType = pi_rObj.m_InterleaveType;
    }

//-----------------------------------------------------------------------------
// HRFCoordinateType for persistence
//-----------------------------------------------------------------------------

HRFCoordinateType::HRFCoordinateType()
    {
    }

HRFCoordinateType::HRFCoordinateType(CoordinateType pi_CoordinateType)
    {
    m_CoordinateType = pi_CoordinateType;
    }

HRFCoordinateType::HRFCoordinateType(const HRFCoordinateType& pi_rObj)
    {
    m_CoordinateType = pi_rObj.m_CoordinateType;
    }


//-----------------------------------------------------------------------------
// HRFClipShape for persistence
//-----------------------------------------------------------------------------
HPM_REGISTER_CLASS(HRFClipShape, HVEShape)


HRFClipShape::HRFClipShape()
    : HVEShape()
    {
    m_CoordinateType = HRFCoordinateType::PHYSICAL;
    }

HRFClipShape::HRFClipShape(const HRFClipShape& pi_rClipShape)
    : HVEShape(pi_rClipShape)
    {
    m_CoordinateType = pi_rClipShape.GetCoordinateType();
    }

HRFClipShape::HRFClipShape(const HRFClipShape&  pi_rClipShape,
                           HRFCoordinateType    pi_CoordinateType)
    : HVEShape(pi_rClipShape)
    {
    m_CoordinateType = pi_CoordinateType;
    }

HRFClipShape::HRFClipShape(const HVEShape&   pi_rObj,
                           HRFCoordinateType pi_CoordinateType) // allow to give a rectangle
    : HVEShape(pi_rObj)
    {
    m_CoordinateType = pi_CoordinateType;
    }

HRFClipShape::HRFClipShape(size_t*           po_pBufferLength,
                           double*          pi_pBuffer,
                           HRFCoordinateType pi_CoordinateType)
    : HVEShape(po_pBufferLength, pi_pBuffer, new HGF2DCoordSys())
    {
    m_CoordinateType = pi_CoordinateType;
    }

HRFCoordinateType  HRFClipShape::GetCoordinateType() const
    {
    return m_CoordinateType;
    }
