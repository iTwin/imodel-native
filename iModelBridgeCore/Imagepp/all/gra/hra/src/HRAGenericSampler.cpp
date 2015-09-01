//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAGenericSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAGenericSampler.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>


/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_rSampleDimension
-----------------------------------------------------------------------------*/
HRAGenericSampler::HRAGenericSampler(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                                     const HGF2DRectangle&                 pi_rSampleDimension,
                                     double                                pi_DeltaX,
                                     double                                pi_DeltaY)
    : m_SampleDimension(pi_rSampleDimension),
      m_DeltaX(pi_DeltaX),
      m_DeltaY(pi_DeltaY),
      m_StretchByLine(HDOUBLE_EQUAL_EPSILON(pi_DeltaY, 0.0))   // optimization
    {
    m_Width               = pi_rMemorySurface.GetWidth();
    m_Height              = pi_rMemorySurface.GetHeight();
    m_PixelType           = pi_rMemorySurface.GetPixelType();

    // Only add things that are generic to all sampler.
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRAGenericSampler::~HRAGenericSampler()
    {
    }


/**----------------------------------------------------------------------------
   Retrieve the pixeltype in which the sampled data will be returned. NULL
   will be returned if we sample in the source pixeltype.
   ----------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HRAGenericSampler::GetOutputPixelType() const
    {
    // Default = use source pixeltype
    return 0;
    }


/**----------------------------------------------------------------------------
   Try to sample in the specified pixeltype. This can only be used if the
   basic output pixeltype is different than the source pixeltype.
   If TryToUse returns true, following calls to GetOutputPixelType() should
   return the given pixeltype.

   @see GetOutputPixelType()
   -----------------------------------------------------------------------------
*/
bool HRAGenericSampler::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
    {
    // Default = source pixeltype
    return false;
    }

/**----------------------------------------------------------------------------
 Get the sample dimension

 @return const HGF2DRectangle&
-----------------------------------------------------------------------------*/
const HGF2DRectangle& HRAGenericSampler::GetSampleDimension() const
    {
    return m_SampleDimension;
    }

/**----------------------------------------------------------------------------
Set scaling

-----------------------------------------------------------------------------*/
void HRAGenericSampler::SetScale(double pi_ScaleX, double pi_ScaleY)
    {
    m_DeltaX = pi_ScaleX;
    m_DeltaY = pi_ScaleY;
    }