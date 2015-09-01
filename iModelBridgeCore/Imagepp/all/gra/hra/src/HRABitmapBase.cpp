//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABitmapBase.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapBase
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRABitmapIterator.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRABlitter.h>
#include <Imagepp/all/h/HRAWarper.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRATransaction.h>

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HRABitmapBase::StretchWithHGS(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_pOptions, 
                                   const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const
    {
    HPRECONDITION(pi_rpTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID));

    HRABlitter blitter(pio_destSurface);
    blitter.SetSamplingMode(pi_pOptions.GetResamplingMode());
    
    if(pi_pOptions.ApplyAlphaBlend())
        blitter.SetAlphaBlend(true);
    if (pi_pOptions.ApplyGridShape())
        blitter.SetGridMode(true);
    if (pi_pOptions.GetOverviewMode())
        blitter.SetOverviewsMode(true);
       
    // create the descriptor for the source
    HFCPtr<HRPPixelType> PixelReplace(pi_pOptions.GetReplacingPixelType());
    HFCPtr<HGSSurfaceDescriptor> pSrcDescriptor(GetSurfaceDescriptor(&PixelReplace));
    HRASurface SrcSurface(pSrcDescriptor);

    blitter.BlitFrom(SrcSurface, *pi_rpTransfoModel, pi_pOptions.GetTransaction());
    }

//-----------------------------------------------------------------------------
// WarpWithHGS
//-----------------------------------------------------------------------------
void HRABitmapBase::WarpWithHGS(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_pOptions, 
                                const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const
    {
    HRAWarper warper(pio_destSurface);
    warper.SetSamplingMode(pi_pOptions.GetResamplingMode());
    if(pi_pOptions.ApplyAlphaBlend())
        warper.SetAlphaBlend(true);
    if (pi_pOptions.ApplyGridShape())
        warper.SetGridMode(true);
    
    // create the descriptor for the source
    HFCPtr<HRPPixelType> PixelReplace(pi_pOptions.GetReplacingPixelType());
    HFCPtr<HGSSurfaceDescriptor> pSrcDescriptor(GetSurfaceDescriptor(&PixelReplace));
    HRASurface SrcSurface(pSrcDescriptor);        

    warper.WarpFrom(SrcSurface, *pi_rpTransfoModel, pi_pOptions.GetTransaction());
    }