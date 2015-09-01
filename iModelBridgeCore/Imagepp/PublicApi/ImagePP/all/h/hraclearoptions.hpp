//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/hraclearoptions.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRAClearOptions
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HRAClearOptions::HRAClearOptions()
    : m_pShape(0),
      m_ApplyRasterClipping(false),
      m_pScanlines(0),
      m_pRLEMask(0),
      m_pRawData(0),
      m_ForceLoadingData(false)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HRAClearOptions::HRAClearOptions(const HRAClearOptions& pi_rObj)
    : m_pShape(pi_rObj.m_pShape),
      m_ApplyRasterClipping(pi_rObj.m_ApplyRasterClipping),
      m_pScanlines(pi_rObj.m_pScanlines),
      m_pRLEMask(pi_rObj.m_pRLEMask),
      m_pRawData(pi_rObj.m_pRawData),
      m_ForceLoadingData(pi_rObj.m_ForceLoadingData)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HRAClearOptions::~HRAClearOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HRAClearOptions&  HRAClearOptions::operator=(const HRAClearOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pShape                = pi_rObj.m_pShape;
        m_ApplyRasterClipping   = pi_rObj.m_ApplyRasterClipping;
        m_pRawData              = pi_rObj.m_pRawData;
        m_pRLEMask              = pi_rObj.m_pRLEMask;
        m_pScanlines            = pi_rObj.m_pScanlines;
        m_ForceLoadingData      = pi_rObj.m_ForceLoadingData;
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// HasShape
//-----------------------------------------------------------------------------
inline bool HRAClearOptions::HasShape() const
    {
    return m_pShape != 0;
    }

//-----------------------------------------------------------------------------
// public
// GetShape
//-----------------------------------------------------------------------------
inline const HVEShape* HRAClearOptions::GetShape() const
    {
    return m_pShape;
    }

//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetShape(const HVEShape*    pi_pShape)
    {
    HPRECONDITION(!HasScanlines() && !HasRLEMask());

    m_pShape = pi_pShape;
    }

//-----------------------------------------------------------------------------
// public
// SetApplyRasterClipping
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetApplyRasterClipping(bool pi_ApplyRasterClipping)
    {
    m_ApplyRasterClipping = pi_ApplyRasterClipping;
    }

//-----------------------------------------------------------------------------
// public
// HasApplyRasterClipping
//-----------------------------------------------------------------------------
inline bool HRAClearOptions::HasApplyRasterClipping() const
    {
    return m_ApplyRasterClipping;
    }

//-----------------------------------------------------------------------------
// public
// HasScanlines
//-----------------------------------------------------------------------------
inline bool HRAClearOptions::HasScanlines() const
    {
    return m_pScanlines != 0;
    }

//-----------------------------------------------------------------------------
// public
// SetScanlines
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetScanlines(const HGFScanLines* pi_pScanlines)
    {
    HPRECONDITION(!HasShape() && !HasRLEMask());

    m_pScanlines = pi_pScanlines;
    }

//-----------------------------------------------------------------------------
// public
// GetScanlines
//-----------------------------------------------------------------------------
inline const HGFScanLines* HRAClearOptions::GetScanlines() const
    {
    return m_pScanlines;
    }


//-----------------------------------------------------------------------------
// public
// HasRLEMask
//-----------------------------------------------------------------------------
inline bool HRAClearOptions::HasRLEMask() const
    {
    return m_pRLEMask != 0;
    }

//-----------------------------------------------------------------------------
// public
// SetRLEMask
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetRLEMask(const HCDPacketRLE* pi_pRLEMask)
    {
    HPRECONDITION(m_pScanlines == 0 && m_pShape == 0);

    m_pRLEMask = pi_pRLEMask;
    }

//-----------------------------------------------------------------------------
// public
// GetRLEMask
//-----------------------------------------------------------------------------
inline const HCDPacketRLE* HRAClearOptions::GetRLEMask() const
    {
    return m_pRLEMask;
    }

//-----------------------------------------------------------------------------
// public
// SetRawDataValue
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetRawDataValue(const void* pi_pRawData)
    {
    m_pRawData = pi_pRawData;
    }

//-----------------------------------------------------------------------------
// public
// GetRawDataValue
//-----------------------------------------------------------------------------
inline const void* HRAClearOptions::GetRawDataValue() const
    {
    return m_pRawData;
    }

//-----------------------------------------------------------------------------
// public
// HasLoadingData
//-----------------------------------------------------------------------------
inline bool HRAClearOptions::HasLoadingData() const
    {
    return m_ForceLoadingData;
    }

//-----------------------------------------------------------------------------
// public
// SetLoadingData
//-----------------------------------------------------------------------------
inline void HRAClearOptions::SetLoadingData(bool pi_ForceLoadingData)
    {
    m_ForceLoadingData = pi_ForceLoadingData;
    }
END_IMAGEPP_NAMESPACE