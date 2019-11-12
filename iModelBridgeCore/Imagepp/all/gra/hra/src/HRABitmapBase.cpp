//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapBase
// ----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRABitmapBase.h>
#include <ImagePP/all/h/HRABitmapIterator.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HRATransaction.h>

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
    m_XPosInRaster  = pi_rBitmap.m_XPosInRaster;
    m_YPosInRaster  = pi_rBitmap.m_YPosInRaster;
    m_dataWidth = pi_rBitmap.m_dataWidth;
    m_dataHeight = pi_rBitmap.m_dataHeight;
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
