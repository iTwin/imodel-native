//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicToolAttribute.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Classes for graphic tool attribute
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HGSGraphicToolAttribute.h>

//-----------------------------------------------------------------------------
// Class : HGSGraphicToolAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute::~HGSGraphicToolAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute& HGSGraphicToolAttribute::operator=(const HGSGraphicToolAttribute& pi_rObj)
    {
    return *this;
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Default constructor

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute::HGSGraphicToolAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute::HGSGraphicToolAttribute(const HGSGraphicToolAttribute& pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// Class : HGSTransformAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSTransformAttribute::HGSTransformAttribute(const HGSTransform& pi_rTransform)
    : HGSGraphicToolAttribute(),
      m_Transform(pi_rTransform)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSTransformAttribute::HGSTransformAttribute(const HGSTransformAttribute& pi_rObj)
    : HGSGraphicToolAttribute(),
      m_Transform(pi_rObj.m_Transform)
    {
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSTransformAttribute::~HGSTransformAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSTransformAttribute::Clone() const
    {
    return (HGSGraphicToolAttribute*)new HGSTransformAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSTransformAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSTransformAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Transform == ((HGSTransformAttribute&)pi_rAttribute).m_Transform) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSTransformAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Transform.Supports(((HGSTransformAttribute&)pi_rAttribute).m_Transform)));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSTransformAttribute& HGSTransformAttribute::operator=(const HGSTransformAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);
        // copy members
        m_Transform = pi_rObj.m_Transform;
        }

    return *this;
    }


/**----------------------------------------------------------------------------
 GetTransform

 @see
-----------------------------------------------------------------------------*/
const HGSTransform& HGSTransformAttribute::GetTransform() const
    {
    return m_Transform;
    }



//-----------------------------------------------------------------------------
// Class : HGSResamplingAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rResampling

 @see
-----------------------------------------------------------------------------*/
HGSResamplingAttribute::HGSResamplingAttribute(const HGSResampling& pi_rResampling)
    : HGSGraphicToolAttribute(),
      m_Resampling(pi_rResampling)
    {
    }

/**----------------------------------------------------------------------------
 CopyConstructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSResamplingAttribute::HGSResamplingAttribute(const HGSResamplingAttribute& pi_rObj)
    : HGSGraphicToolAttribute(pi_rObj),
      m_Resampling(pi_rObj.m_Resampling)
    {
    }

/**----------------------------------------------------------------------------
 Destructor

 @see
-----------------------------------------------------------------------------*/
HGSResamplingAttribute::~HGSResamplingAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSResamplingAttribute::Clone() const
    {
    return new HGSResamplingAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
bool HGSResamplingAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSResamplingAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Resampling == ((HGSResamplingAttribute&)pi_rAttribute).m_Resampling) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSResamplingAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Resampling.Supports(((HGSResamplingAttribute&)pi_rAttribute).m_Resampling)));
    }

/**----------------------------------------------------------------------------
 operator=

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSResamplingAttribute& HGSResamplingAttribute::operator=(const HGSResamplingAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);
        // copy members
        m_Resampling = pi_rObj.m_Resampling;
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 GetResampling

 @see
-----------------------------------------------------------------------------*/
const HGSResampling& HGSResamplingAttribute::GetResampling() const
    {
    return m_Resampling;
    }



//-----------------------------------------------------------------------------
// Class : HGSColorConversionAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rColorConversion

 @see
-----------------------------------------------------------------------------*/
HGSColorConversionAttribute::HGSColorConversionAttribute(const HGSColorConversion& pi_rColorConversion)
    : HGSGraphicToolAttribute(),
      m_ColorConversion(pi_rColorConversion)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSColorConversionAttribute::HGSColorConversionAttribute(const HGSColorConversionAttribute& pi_rObj)
    : HGSGraphicToolAttribute(),
      m_ColorConversion(pi_rObj.m_ColorConversion)
    {
    }

/**----------------------------------------------------------------------------
 Destructor

 @see
-----------------------------------------------------------------------------*/
HGSColorConversionAttribute::~HGSColorConversionAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSColorConversionAttribute::Clone() const
    {
    return new HGSColorConversionAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorConversionAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorConversionAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_ColorConversion == ((HGSColorConversionAttribute&)pi_rAttribute).m_ColorConversion) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorConversionAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_ColorConversion.Supports(((HGSColorConversionAttribute&)pi_rAttribute).m_ColorConversion)));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSColorConversionAttribute& HGSColorConversionAttribute::operator=(const HGSColorConversionAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);
        // copy members
        m_ColorConversion = pi_rObj.m_ColorConversion;
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
const HGSColorConversion& HGSColorConversionAttribute::GetColorConversion() const
    {
    return m_ColorConversion;
    }


//-----------------------------------------------------------------------------
// Class : HGSColorAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rColor

 @see
-----------------------------------------------------------------------------*/
HGSColorAttribute::HGSColorAttribute(const HGSColor& pi_rColor)
    : HGSGraphicToolAttribute(),
      m_Color(pi_rColor)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSColorAttribute::HGSColorAttribute(const HGSColorAttribute& pi_rObj)
    : HGSGraphicToolAttribute(pi_rObj),
      m_Color(pi_rObj.m_Color)
    {
    }

/**----------------------------------------------------------------------------
 Destructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSColorAttribute::~HGSColorAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSColorAttribute::Clone() const
    {
    return new HGSColorAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Color == ((HGSColorAttribute&)pi_rAttribute).m_Color) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSColorAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Color.Supports(((HGSColorAttribute&)pi_rAttribute).m_Color)));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSColorAttribute& HGSColorAttribute::operator=(const HGSColorAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);

        // copy members
        m_Color = pi_rObj.m_Color;
        }
    return *this;
    }


/**----------------------------------------------------------------------------
 GetColor

 @see
-----------------------------------------------------------------------------*/
const HGSColor& HGSColorAttribute::GetColor() const
    {
    return m_Color;
    }



//-----------------------------------------------------------------------------
// class : HGSLineStyleAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Default constructor

 @see
-----------------------------------------------------------------------------*/
HGSLineStyleAttribute::HGSLineStyleAttribute()
    : HGSGraphicToolAttribute()
    {
    m_LineWidth = 1;
    }

/**----------------------------------------------------------------------------
 Constructor

 @param pi_LineWidth

 @see
-----------------------------------------------------------------------------*/
HGSLineStyleAttribute::HGSLineStyleAttribute(uint32_t pi_LineWidth)
    : HGSGraphicToolAttribute()
    {
    m_LineWidth = pi_LineWidth;
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSLineStyleAttribute::HGSLineStyleAttribute(const HGSLineStyleAttribute& pi_rObj)
    : HGSGraphicToolAttribute(pi_rObj)
    {
    m_LineWidth = pi_rObj.m_LineWidth;
    }

/**----------------------------------------------------------------------------
 Destructor

 @see
-----------------------------------------------------------------------------*/
HGSLineStyleAttribute::~HGSLineStyleAttribute()
    {
    }


/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSLineStyleAttribute::Clone() const
    {
    return new HGSLineStyleAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSLineStyleAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSLineStyleAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_LineWidth == ((HGSLineStyleAttribute&)pi_rAttribute).m_LineWidth) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSLineStyleAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_LineWidth == 0 && ((HGSLineStyleAttribute&)pi_rAttribute).m_LineWidth > 0) ||
            (m_LineWidth == ((HGSLineStyleAttribute&)pi_rAttribute).m_LineWidth));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSLineStyleAttribute& HGSLineStyleAttribute::operator=(const HGSLineStyleAttribute& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);

        // copy members
        m_LineWidth = pi_rObj.m_LineWidth;
        }

    return (*this);
    }

//-----------------------------------------------------------------------------
// public
// GetLineWidth
//-----------------------------------------------------------------------------
uint32_t HGSLineStyleAttribute::GetWidth() const
    {
    return m_LineWidth;
    }


//-----------------------------------------------------------------------------
// Class : HGSScanlinesAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rColorConversion

 @see
-----------------------------------------------------------------------------*/
HGSScanlinesAttribute::HGSScanlinesAttribute(const HGSScanlineMethod& pi_rScanlineMethod)
    : HGSGraphicToolAttribute(),
      m_ScanlineMethod(pi_rScanlineMethod)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSScanlinesAttribute::HGSScanlinesAttribute(const HGSScanlinesAttribute& pi_rObj)
    : HGSGraphicToolAttribute(),
      m_ScanlineMethod(pi_rObj.m_ScanlineMethod)
    {
    }

/**----------------------------------------------------------------------------
 Destructor

 @see
-----------------------------------------------------------------------------*/
HGSScanlinesAttribute::~HGSScanlinesAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSScanlinesAttribute::Clone() const
    {
    return new HGSScanlinesAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSScanlinesAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSScanlinesAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_ScanlineMethod == ((HGSScanlinesAttribute&)pi_rAttribute).m_ScanlineMethod) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSScanlinesAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_ScanlineMethod.Supports(((HGSScanlinesAttribute&)pi_rAttribute).m_ScanlineMethod)));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSScanlinesAttribute& HGSScanlinesAttribute::operator=(const HGSScanlinesAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);
        // copy members
        m_ScanlineMethod = pi_rObj.m_ScanlineMethod;
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
const HGSScanlineMethod& HGSScanlinesAttribute::GetScanlineMethod() const
    {
    return m_ScanlineMethod;
    }


//-----------------------------------------------------------------------------
// Class : HGSPurposeAttribute
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @see
-----------------------------------------------------------------------------*/
HGSPurposeAttribute::HGSPurposeAttribute(const HGSPurpose& pi_rPurpose)
    : HGSGraphicToolAttribute(),
      m_Purpose(pi_rPurpose)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor

 @see
-----------------------------------------------------------------------------*/
HGSPurposeAttribute::HGSPurposeAttribute(const HGSPurposeAttribute& pi_rObj)
    : HGSGraphicToolAttribute(),
      m_Purpose(pi_rObj.m_Purpose)
    {
    }

/**----------------------------------------------------------------------------
 Destructor

 @see
-----------------------------------------------------------------------------*/
HGSPurposeAttribute::~HGSPurposeAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSGraphicToolAttribute* HGSPurposeAttribute::Clone() const
    {
    return new HGSPurposeAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSPurposeAttribute::IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }

/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSPurposeAttribute::IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Purpose == ((HGSPurposeAttribute&)pi_rAttribute).m_Purpose) );
    }

/**----------------------------------------------------------------------------
 Supports

 @see
-----------------------------------------------------------------------------*/
bool HGSPurposeAttribute::Supports(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Purpose.Supports(((HGSPurposeAttribute&)pi_rAttribute).m_Purpose)));
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSPurposeAttribute& HGSPurposeAttribute::operator=(const HGSPurposeAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSGraphicToolAttribute::operator=(pi_rObj);
        // copy members
        m_Purpose = pi_rObj.m_Purpose;
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
const HGSPurpose& HGSPurposeAttribute::GetPurpose() const
    {
    return m_Purpose;
    }
