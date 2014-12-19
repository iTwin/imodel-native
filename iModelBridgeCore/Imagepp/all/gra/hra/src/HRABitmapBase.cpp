//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABitmapBase.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapBase
// ----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRABitmapIterator.h>
#include <Imagepp/all/h/HRPPixelType.h>

HPM_REGISTER_ABSTRACT_CLASS(HRABitmapBase, HRAStoredRaster)

//-----------------------------------------------------------------------------
// public
// Default constructor. Create a HRABitmap (1,1) pixel (24 bits).
//-----------------------------------------------------------------------------
HRABitmapBase::HRABitmapBase (HRPPixelType* pi_pPixelType,
                              bool         pi_Resizable)
    : HRAStoredRaster (pi_pPixelType,
                       pi_Resizable),
    m_XPosInRaster(0),
    m_YPosInRaster(0)
    {
    m_SLO           = UPPER_LEFT_HORIZONTAL;
    m_BitsAlignment = 8;
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapBase::HRABitmapBase - Full featured constructor.
//-----------------------------------------------------------------------------
HRABitmapBase::HRABitmapBase(size_t                         pi_WidthPixels,
                             size_t                         pi_HeightPixels,
                             const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                             const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                             const HFCPtr<HRPPixelType>&    pi_rpPixel,
                             uint32_t                       pi_BitsAlignment,
                             HRABitmapBase::SLO             pi_SLO,
                             bool                          pi_Resizable)
    : HRAStoredRaster(pi_WidthPixels,
                      pi_HeightPixels,
                      pi_pModelCSp_CSl,
                      pi_rpLogicalCoordSys,
                      pi_rpPixel,
                      pi_Resizable),
    m_XPosInRaster(0),
    m_YPosInRaster(0)
    {
    HPRECONDITION(pi_rpPixel != 0);

    m_BitsAlignment = pi_BitsAlignment;
    m_SLO           = pi_SLO;
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapBase::~HRABitmapBase - Destructor
//-----------------------------------------------------------------------------
HRABitmapBase::~HRABitmapBase()
    {
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmapBase::HRABitmapBase - Copy constructor
//-----------------------------------------------------------------------------
HRABitmapBase::HRABitmapBase(const HRABitmapBase& pi_rBitmap)
    : HRAStoredRaster(pi_rBitmap)
    {
    m_BitsAlignment = pi_rBitmap.m_BitsAlignment;
    m_SLO           = pi_rBitmap.m_SLO;
    m_XPosInRaster  = pi_rBitmap.m_XPosInRaster;
    m_YPosInRaster  = pi_rBitmap.m_YPosInRaster;
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmapBase::operator= - Assignment operator
//-----------------------------------------------------------------------------
HRABitmapBase& HRABitmapBase::operator=(const HRABitmapBase& pi_rBitmap)
    {
    if (this != &pi_rBitmap)
        {
        HRAStoredRaster::operator=(pi_rBitmap);

        m_BitsAlignment = pi_rBitmap.m_BitsAlignment;
        m_SLO           = pi_rBitmap.m_SLO;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// HRABitmap::CreateIterator
//-----------------------------------------------------------------------------
HRARasterIterator* HRABitmapBase::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    HRARasterIterator* pItr = new HRABitmapIterator(HFCPtr<HRABitmapBase>((HRABitmapBase*)this), pi_rOptions);

    return pItr;
    }