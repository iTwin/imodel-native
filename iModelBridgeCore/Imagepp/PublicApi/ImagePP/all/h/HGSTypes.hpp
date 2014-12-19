//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSTypes.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class HGSMemoryAlignment
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSMemoryAlignment object.

 @param pi_Alignment
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment::HGSMemoryAlignment(MemoryAlignment pi_Alignment)
    {
    m_MemoryAlignment = pi_Alignment;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSMemoryAlignment object.

 @param pi_pRawData
 @param pi_BytesPerRow
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment::HGSMemoryAlignment(const void*   pi_pRawData,
                                              uint32_t     pi_BytesPerRow)
    {
    if (((((uint64_t)pi_pRawData) % 4) == 0) && (pi_BytesPerRow % 4) == 0)
        m_MemoryAlignment = DWORD;
    else
        m_MemoryAlignment = BYTE;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSMemoryAlignment object.
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment::HGSMemoryAlignment(const HGSMemoryAlignment& pi_rMemoryAlignment)
    {
    m_MemoryAlignment = pi_rMemoryAlignment.m_MemoryAlignment;
    }


/**----------------------------------------------------------------------------
 Destructor for the class HGSMemoryAlignment
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment::~HGSMemoryAlignment()
    {
    }


/**----------------------------------------------------------------------------
 Equality operator
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment& HGSMemoryAlignment::operator=(const HGSMemoryAlignment& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_MemoryAlignment = pi_rObj.m_MemoryAlignment;
        }
    return *this;
    }


/**----------------------------------------------------------------------------
 Comparaison operator.

  @see HGSMemoryAlignment::operator!=(const HGSMemoryAlignment&)
-----------------------------------------------------------------------------*/
inline bool HGSMemoryAlignment::operator==(const HGSMemoryAlignment& pi_rObj) const
    {
    return m_MemoryAlignment == pi_rObj.m_MemoryAlignment;
    }


/**----------------------------------------------------------------------------
 Comparaison operator.

  @see HGSMemoryAlignment::operator==(const HGSMemoryAlignment&)
-----------------------------------------------------------------------------*/
inline bool HGSMemoryAlignment::operator!=(const HGSMemoryAlignment& pi_rObj) const
    {
    return m_MemoryAlignment != pi_rObj.m_MemoryAlignment;
    }


/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSMemoryAlignment::Supports(const HGSMemoryAlignment& pi_rObj) const
    {
    return m_MemoryAlignment == pi_rObj.m_MemoryAlignment ||
           (m_MemoryAlignment == BYTE && pi_rObj.m_MemoryAlignment == DWORD);
    }


/**----------------------------------------------------------------------------
 GetMemoryAlignment.
-----------------------------------------------------------------------------*/
inline HGSMemoryAlignment::MemoryAlignment HGSMemoryAlignment::GetMemoryAlignment() const
    {
    return m_MemoryAlignment;
    }


//-----------------------------------------------------------------------------
// class HGSSurfaceType
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSSurfaceType object.
-----------------------------------------------------------------------------*/
inline HGSSurfaceType::HGSSurfaceType(SurfaceType pi_SurfaceType)
    {
    m_SurfaceType = pi_SurfaceType;
    }


/**----------------------------------------------------------------------------
 Constructor that creates a new HGSSurfaceType object.
-----------------------------------------------------------------------------*/
inline HGSSurfaceType::HGSSurfaceType(const HGSSurfaceType& pi_rSurfaceType)
    {
    m_SurfaceType = pi_rSurfaceType.m_SurfaceType;
    }


/**----------------------------------------------------------------------------
 Destructor for the class HGSSurfaceType.
-----------------------------------------------------------------------------*/
inline HGSSurfaceType::~HGSSurfaceType()
    {
    }


/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSSurfaceType& HGSSurfaceType::operator=(const HGSSurfaceType& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_SurfaceType = pi_rObj.m_SurfaceType;
        }
    return *this;
    }


/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSSurfaceType::operator!=(const HGSSurfaceType&)
-----------------------------------------------------------------------------*/
inline bool HGSSurfaceType::operator==(const HGSSurfaceType& pi_rObj) const
    {
    return m_SurfaceType == pi_rObj.m_SurfaceType;
    }


/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSSurfaceType::operator==(const HGSSurfaceType&)
-----------------------------------------------------------------------------*/
inline bool HGSSurfaceType::operator!=(const HGSSurfaceType& pi_rObj) const
    {
    return m_SurfaceType != pi_rObj.m_SurfaceType;
    }


/**----------------------------------------------------------------------------
 Supports.
 -----------------------------------------------------------------------------*/
inline bool HGSSurfaceType::Supports(const HGSSurfaceType& pi_rObj) const
    {
    return m_SurfaceType == pi_rObj.m_SurfaceType;
    }


/**----------------------------------------------------------------------------
 GetSurfaceType.
-----------------------------------------------------------------------------*/
inline HGSSurfaceType::SurfaceType HGSSurfaceType::GetSurfaceType() const
    {
    return m_SurfaceType;
    }


//-----------------------------------------------------------------------------
// class HGSTransform
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSTransform object.
-----------------------------------------------------------------------------*/
inline HGSTransform::HGSTransform(TransformMethod pi_TransformMethod)
    {
    m_TransformMethod = pi_TransformMethod;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSTransform object.
-----------------------------------------------------------------------------*/
inline HGSTransform::HGSTransform(const HGSTransform& pi_rObj)
    {
    m_TransformMethod = pi_rObj.m_TransformMethod;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSTransformType.
-----------------------------------------------------------------------------*/
inline HGSTransform::~HGSTransform()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSTransform& HGSTransform::operator=(const HGSTransform& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_TransformMethod = pi_rObj.m_TransformMethod;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSTransform::operator!=(const HGSTransform&)
-----------------------------------------------------------------------------*/
inline bool HGSTransform::operator==(const HGSTransform& pi_rObj) const
    {
    return m_TransformMethod == pi_rObj.m_TransformMethod;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSTransform::operator==(const HGSTransform&)
-----------------------------------------------------------------------------*/
inline bool HGSTransform::operator!=(const HGSTransform& pi_rObj) const
    {
    return m_TransformMethod != pi_rObj.m_TransformMethod;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSTransform::Supports(const HGSTransform& pi_rObj) const
    {
    return m_TransformMethod == pi_rObj.m_TransformMethod;
    }

/**----------------------------------------------------------------------------
 GetTransformMethod.
-----------------------------------------------------------------------------*/
inline HGSTransform::TransformMethod HGSTransform::GetTransformMethod() const
    {
    return m_TransformMethod;
    }


//-----------------------------------------------------------------------------
// class HGSResampling
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSResampling object.
-----------------------------------------------------------------------------*/
inline HGSResampling::HGSResampling(ResamplingMethod pi_ResamplingMethod)
    {
    m_ResamplingMethod = pi_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSResampling object.
-----------------------------------------------------------------------------*/
inline HGSResampling::HGSResampling(const HGSResampling& pi_rObj)
    {
    m_ResamplingMethod = pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSResampling.
-----------------------------------------------------------------------------*/
inline HGSResampling::~HGSResampling()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSResampling& HGSResampling::operator=(const HGSResampling& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_ResamplingMethod = pi_rObj.m_ResamplingMethod;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSResampling::operator!=(const HGSResampling&)
-----------------------------------------------------------------------------*/
inline bool HGSResampling::operator==(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod == pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSResampling::operator==(const HGSResampling&)
-----------------------------------------------------------------------------*/
inline bool HGSResampling::operator!=(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod != pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSResampling::Supports(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod == pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 GetResamplingMethod.
-----------------------------------------------------------------------------*/
inline HGSResampling::ResamplingMethod HGSResampling::GetResamplingMethod() const
    {
    return m_ResamplingMethod;
    }


//-----------------------------------------------------------------------------
// class HGSColorConversion
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSColorConversion object.
-----------------------------------------------------------------------------*/
inline HGSColorConversion::HGSColorConversion(ColorConversionMethod pi_ColorConversionMethod)
    {
    m_ColorConversionMethod = pi_ColorConversionMethod;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSColorConversion object.
-----------------------------------------------------------------------------*/
inline HGSColorConversion::HGSColorConversion(const HGSColorConversion& pi_rObj)
    {
    m_ColorConversionMethod = pi_rObj.m_ColorConversionMethod;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSColorConversion.
-----------------------------------------------------------------------------*/
inline HGSColorConversion::~HGSColorConversion()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSColorConversion& HGSColorConversion::operator=(const HGSColorConversion& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_ColorConversionMethod= pi_rObj.m_ColorConversionMethod;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSColorConversion::operator!=(const HGSColorConversion&)
-----------------------------------------------------------------------------*/
inline bool HGSColorConversion::operator==(const HGSColorConversion& pi_rObj) const
    {
    return m_ColorConversionMethod == pi_rObj.m_ColorConversionMethod;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSColorConversion::operator!=(const HGSColorConversion&)
-----------------------------------------------------------------------------*/
inline bool HGSColorConversion::operator!=(const HGSColorConversion& pi_rObj) const
    {
    return m_ColorConversionMethod != pi_rObj.m_ColorConversionMethod;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSColorConversion::Supports(const HGSColorConversion& pi_rObj) const
    {
    return m_ColorConversionMethod == pi_rObj.m_ColorConversionMethod;
    }

/**----------------------------------------------------------------------------
 GetColorConversionMethod.
-----------------------------------------------------------------------------*/
inline HGSColorConversion::ColorConversionMethod HGSColorConversion::GetColorConversionMethod() const
    {
    return m_ColorConversionMethod;
    }



//-----------------------------------------------------------------------------
// class HGSColor
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Default Constructor that creates a new HGSColor object.
-----------------------------------------------------------------------------*/
inline HGSColor::HGSColor()
    {
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSColor object.
-----------------------------------------------------------------------------*/
inline HGSColor::HGSColor(const Byte* pi_pRGBAValue)
    {
    // no value means all color
    if (pi_pRGBAValue != 0)
        {
        m_pRGBAValue = new Byte[4];
        memcpy(m_pRGBAValue, pi_pRGBAValue, 4);
        }
    }

/**----------------------------------------------------------------------------
 Copy constructor that creates a new HGSColor object.
-----------------------------------------------------------------------------*/
inline HGSColor::HGSColor(const HGSColor& pi_rObj)
    {
    if (pi_rObj.m_pRGBAValue != 0)
        {
        m_pRGBAValue = new Byte[4];
        memcpy(m_pRGBAValue, pi_rObj.m_pRGBAValue, 4);
        }
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSColor.
-----------------------------------------------------------------------------*/
inline HGSColor::~HGSColor()
    {
    }

/**----------------------------------------------------------------------------
 Assignment operator.
-----------------------------------------------------------------------------*/
inline HGSColor& HGSColor::operator=(const HGSColor& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        if (pi_rObj.m_pRGBAValue != 0)
            {
            m_pRGBAValue = new Byte[4];
            memcpy(m_pRGBAValue, pi_rObj.m_pRGBAValue, 4);
            }
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.
-----------------------------------------------------------------------------*/
inline bool HGSColor::operator==(const HGSColor& pi_rObj) const
    {
    return (m_pRGBAValue == 0 && pi_rObj.m_pRGBAValue == 0) ||
           (m_pRGBAValue != 0 && pi_rObj.m_pRGBAValue != 0 &&
            memcmp(m_pRGBAValue, pi_rObj.m_pRGBAValue, 4) == 0);
    }

/**----------------------------------------------------------------------------
 Comparaison operator.
-----------------------------------------------------------------------------*/
inline bool HGSColor::operator!=(const HGSColor& pi_rObj) const
    {
    return !operator==(pi_rObj);
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSColor::Supports(const HGSColor& pi_rObj) const
    {
    return m_pRGBAValue == 0 ||
           (pi_rObj.m_pRGBAValue != 0 && memcmp(m_pRGBAValue, pi_rObj.m_pRGBAValue, 4) == 0);
    }

/**----------------------------------------------------------------------------
 GetValue.
-----------------------------------------------------------------------------*/
inline const Byte* HGSColor::GetValue() const
    {
    return m_pRGBAValue;
    }


//-----------------------------------------------------------------------------
// class HGSScanline
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSScanlineMethod object.
-----------------------------------------------------------------------------*/
inline HGSScanlineMethod::HGSScanlineMethod(ScanlineMethod pi_ScanlineMethod)
    {
    m_ScanlineMethod = pi_ScanlineMethod;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSScanlineMethod object.
-----------------------------------------------------------------------------*/
inline HGSScanlineMethod::HGSScanlineMethod(const HGSScanlineMethod& pi_rObj)
    {
    m_ScanlineMethod = pi_rObj.m_ScanlineMethod;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSScanlineMethod.
-----------------------------------------------------------------------------*/
inline HGSScanlineMethod::~HGSScanlineMethod()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSScanlineMethod& HGSScanlineMethod::operator=(const HGSScanlineMethod& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_ScanlineMethod = pi_rObj.m_ScanlineMethod;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSScanlineMethod::operator!=(const HGSScanline&)
-----------------------------------------------------------------------------*/
inline bool HGSScanlineMethod::operator==(const HGSScanlineMethod& pi_rObj) const
    {
    return m_ScanlineMethod == pi_rObj.m_ScanlineMethod;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSScanlineMethod::operator==(const HGSScanline&)
-----------------------------------------------------------------------------*/
inline bool HGSScanlineMethod::operator!=(const HGSScanlineMethod& pi_rObj) const
    {
    return m_ScanlineMethod != pi_rObj.m_ScanlineMethod;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSScanlineMethod::Supports(const HGSScanlineMethod& pi_rObj) const
    {
    return m_ScanlineMethod == pi_rObj.m_ScanlineMethod;
    }

/**----------------------------------------------------------------------------
 GetScanlineMethod.
-----------------------------------------------------------------------------*/
inline HGSScanlineMethod::ScanlineMethod HGSScanlineMethod::GetScanlineMethod() const
    {
    return m_ScanlineMethod;
    }


//-----------------------------------------------------------------------------
// class HGSPurpose
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSPurpose object.
-----------------------------------------------------------------------------*/
inline HGSPurpose::HGSPurpose(Purposes pi_Purpose)
    {
    m_Purpose = pi_Purpose;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSPurpose object.
-----------------------------------------------------------------------------*/
inline HGSPurpose::HGSPurpose(const HGSPurpose& pi_rObj)
    {
    m_Purpose = pi_rObj.m_Purpose;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSPurpose.
-----------------------------------------------------------------------------*/
inline HGSPurpose::~HGSPurpose()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSPurpose& HGSPurpose::operator=(const HGSPurpose& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Purpose = pi_rObj.m_Purpose;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSPurpose::operator!=(const HGSPurpose&)
-----------------------------------------------------------------------------*/
inline bool HGSPurpose::operator==(const HGSPurpose& pi_rObj) const
    {
    return m_Purpose == pi_rObj.m_Purpose;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSPurpose::operator==(const HGSPurpose&)
-----------------------------------------------------------------------------*/
inline bool HGSPurpose::operator!=(const HGSPurpose& pi_rObj) const
    {
    return m_Purpose != pi_rObj.m_Purpose;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSPurpose::Supports(const HGSPurpose& pi_rObj) const
    {
    return m_Purpose == pi_rObj.m_Purpose;
    }

/**----------------------------------------------------------------------------
 GetScanlineMethod.
-----------------------------------------------------------------------------*/
inline HGSPurpose::Purposes HGSPurpose::GetPurpose() const
    {
    return m_Purpose;
    }
