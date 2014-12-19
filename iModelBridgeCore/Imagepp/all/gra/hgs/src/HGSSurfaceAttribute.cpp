//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceAttribute.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Classes for surface attribute
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HGSSurfaceAttribute.h>

//-----------------------------------------------------------------------------
// Class : HGSSurfaceAttribute
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurfaceAttribute::~HGSSurfaceAttribute()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGSSurfaceAttribute& HGSSurfaceAttribute::operator=(const HGSSurfaceAttribute& pi_rObj)
    {
    return *this;
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// protected
// Default Constructor
//-----------------------------------------------------------------------------
HGSSurfaceAttribute::HGSSurfaceAttribute()
    {
    }

//-----------------------------------------------------------------------------
// protected
// Copy Constructor
//-----------------------------------------------------------------------------
HGSSurfaceAttribute::HGSSurfaceAttribute(const HGSSurfaceAttribute& pi_rObj)
    {
    }


//---------------------------------------------------------------------------------------
// class : HGSPixelTypeAttribute
//---------------------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_PixelType

 @see
-----------------------------------------------------------------------------*/
HGSPixelTypeAttribute::HGSPixelTypeAttribute(HCLASS_ID pi_PixelType)
    : HGSSurfaceAttribute()
    {
    m_PixelType = pi_PixelType;
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSPixelTypeAttribute::HGSPixelTypeAttribute(const HGSPixelTypeAttribute& pi_rObj)
    : HGSSurfaceAttribute(pi_rObj)
    {
    m_PixelType = pi_rObj.m_PixelType;
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSPixelTypeAttribute::~HGSPixelTypeAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceAttribute* HGSPixelTypeAttribute::Clone() const
    {
    return new HGSPixelTypeAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSPixelTypeAttribute::IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }


/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSPixelTypeAttribute::IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_PixelType == ((HGSPixelTypeAttribute&)pi_rAttribute).m_PixelType) );
    }

/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSPixelTypeAttribute::Supports(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return (pi_rAttribute.GetClassID() == CLASS_ID) &&
           m_PixelType == ((HGSPixelTypeAttribute&)pi_rAttribute).m_PixelType;
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSPixelTypeAttribute& HGSPixelTypeAttribute::operator=(const HGSPixelTypeAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSSurfaceAttribute::operator=(pi_rObj);

        // copy members
        m_PixelType = pi_rObj.m_PixelType;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 GetPixelTypeClassID

 @param pi_rCapability

 @see
-----------------------------------------------------------------------------*/
HCLASS_ID HGSPixelTypeAttribute::GetPixelTypeClassID() const
    {
    return m_PixelType;
    }


//---------------------------------------------------------------------------------------
// class : HGSCompressionAttribute
//---------------------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_Compression

 @see
-----------------------------------------------------------------------------*/
HGSCompressionAttribute::HGSCompressionAttribute(HCLASS_ID pi_Compression)
    : HGSSurfaceAttribute()
    {
    m_Compression = pi_Compression;
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSCompressionAttribute::HGSCompressionAttribute(const HGSCompressionAttribute& pi_rObj)
    : HGSSurfaceAttribute(pi_rObj)
    {
    m_Compression = pi_rObj.m_Compression;
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSCompressionAttribute::~HGSCompressionAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceAttribute* HGSCompressionAttribute::Clone() const
    {
    return new HGSCompressionAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSCompressionAttribute::IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }


/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSCompressionAttribute::IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_Compression == ((HGSCompressionAttribute&)pi_rAttribute).m_Compression) );
    }


/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSCompressionAttribute::Supports(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return (pi_rAttribute.GetClassID() == CLASS_ID) &&
           m_Compression == ((HGSCompressionAttribute&)pi_rAttribute).m_Compression;
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSCompressionAttribute& HGSCompressionAttribute::operator=(const HGSCompressionAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSSurfaceAttribute::operator=(pi_rObj);

        // copy members
        m_Compression = pi_rObj.m_Compression;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 GetCompressionClassKey

 @see
-----------------------------------------------------------------------------*/
HCLASS_ID HGSCompressionAttribute::GetCompressionClassKey() const
    {
    return m_Compression;
    }


//---------------------------------------------------------------------------------------
// class : HGSSLOAttribute
//---------------------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_SLO

 @see
-----------------------------------------------------------------------------*/
HGSSLOAttribute::HGSSLOAttribute(HGFSLO pi_SLO)
    : HGSSurfaceAttribute()
    {
    m_SLO = pi_SLO;
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSSLOAttribute::HGSSLOAttribute(const HGSSLOAttribute& pi_rObj)
    : HGSSurfaceAttribute(pi_rObj)
    {
    m_SLO = pi_rObj.m_SLO;
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSSLOAttribute::~HGSSLOAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceAttribute* HGSSLOAttribute::Clone() const
    {
    return new HGSSLOAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSLOAttribute::IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }


/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSLOAttribute::IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_SLO == ((HGSSLOAttribute&)pi_rAttribute).m_SLO) );
    }


/**----------------------------------------------------------------------------
 Supportshrf
 src

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSLOAttribute::Supports(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return (pi_rAttribute.GetClassID() == CLASS_ID) &&
           m_SLO == ((HGSSLOAttribute&)pi_rAttribute).m_SLO;
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSSLOAttribute& HGSSLOAttribute::operator=(const HGSSLOAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSSurfaceAttribute::operator=(pi_rObj);

        // copy members
        m_SLO = pi_rObj.m_SLO;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 GetSLO

 @see
-----------------------------------------------------------------------------*/
HGFSLO HGSSLOAttribute::GetSLO() const
    {
    return m_SLO;
    }



//---------------------------------------------------------------------------------------
// class : HGSSurfaceTypeCapability
//---------------------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor

 @param pi_SurfaceType

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceTypeAttribute::HGSSurfaceTypeAttribute(const HGSSurfaceType& pi_rSurfaceType)
    : HGSSurfaceAttribute(),
      m_SurfaceType(pi_rSurfaceType)
    {
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceTypeAttribute::HGSSurfaceTypeAttribute(const HGSSurfaceTypeAttribute& pi_rObj)
    : HGSSurfaceAttribute(pi_rObj),
      m_SurfaceType(pi_rObj.m_SurfaceType)
    {
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSSurfaceTypeAttribute::~HGSSurfaceTypeAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceAttribute* HGSSurfaceTypeAttribute::Clone() const
    {
    return new HGSSurfaceTypeAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSurfaceTypeAttribute::IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }


/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSurfaceTypeAttribute::IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_SurfaceType == ((HGSSurfaceTypeAttribute&)pi_rAttribute).m_SurfaceType) );
    }


/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSSurfaceTypeAttribute::Supports(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return (pi_rAttribute.GetClassID() == CLASS_ID) &&
           m_SurfaceType.Supports(((HGSSurfaceTypeAttribute&)pi_rAttribute).m_SurfaceType);
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceTypeAttribute& HGSSurfaceTypeAttribute::operator=(const HGSSurfaceTypeAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSSurfaceAttribute::operator=(pi_rObj);

        // copy members
        m_SurfaceType = pi_rObj.m_SurfaceType;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 GetSurfaceType
-----------------------------------------------------------------------------*/
const HGSSurfaceType& HGSSurfaceTypeAttribute::GetSurfaceType() const
    {
    return m_SurfaceType;
    }


//---------------------------------------------------------------------------------------
// class : HGSMemoryAlignmentCapability
//---------------------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_MemoryAlignment

 @see
-----------------------------------------------------------------------------*/
HGSMemoryAlignmentAttribute::HGSMemoryAlignmentAttribute(const HGSMemoryAlignment& pi_rMemoryAlignment)
    : HGSSurfaceAttribute(),
      m_MemoryAlignment(pi_rMemoryAlignment)
    {
    }

/**----------------------------------------------------------------------------
 Copy Constructor

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSMemoryAlignmentAttribute::HGSMemoryAlignmentAttribute(const HGSMemoryAlignmentAttribute& pi_rObj)
    : HGSSurfaceAttribute(pi_rObj),
      m_MemoryAlignment(pi_rObj.m_MemoryAlignment)
    {
    }

/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSMemoryAlignmentAttribute::~HGSMemoryAlignmentAttribute()
    {
    }

/**----------------------------------------------------------------------------
 Clone

 @see
-----------------------------------------------------------------------------*/
HGSSurfaceAttribute* HGSMemoryAlignmentAttribute::Clone() const
    {
    return new HGSMemoryAlignmentAttribute(*this);
    }

/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSMemoryAlignmentAttribute::IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return pi_rAttribute.IsCompatibleWith(GetClassID());
    }


/**----------------------------------------------------------------------------
 IsEqual

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSMemoryAlignmentAttribute::IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return ((pi_rAttribute.GetClassID() == CLASS_ID) &&
            (m_MemoryAlignment == ((HGSMemoryAlignmentAttribute&)pi_rAttribute).m_MemoryAlignment) );
    }


/**----------------------------------------------------------------------------
 Supports

 @param pi_rAttribute

 @see
-----------------------------------------------------------------------------*/
bool HGSMemoryAlignmentAttribute::Supports(const HGSSurfaceAttribute& pi_rAttribute) const
    {
    return (pi_rAttribute.GetClassID() == CLASS_ID) &&
           m_MemoryAlignment.Supports(((HGSMemoryAlignmentAttribute&)pi_rAttribute).m_MemoryAlignment);
    }

/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj

 @see
-----------------------------------------------------------------------------*/
HGSMemoryAlignmentAttribute& HGSMemoryAlignmentAttribute::operator=(const HGSMemoryAlignmentAttribute& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGSSurfaceAttribute::operator=(pi_rObj);

        // copy members
        m_MemoryAlignment = pi_rObj.m_MemoryAlignment;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 GetMemoryAlignment

 @see
-----------------------------------------------------------------------------*/
const HGSMemoryAlignment& HGSMemoryAlignmentAttribute::GetMemoryAlignment() const
    {
    return m_MemoryAlignment;
    }









