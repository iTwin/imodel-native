//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRANearestSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------------------
//:> Class HRANearestSampler
//:>---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRANearestSampler.h>

#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRANearestSampler::HRANearestSampler(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                                     const HGF2DRectangle&                 pi_rSampleDimension,
                                     double                                pi_DeltaX,
                                     double                                pi_DeltaY)
    : HRAGenericSampler(pi_rMemorySurface,
                        pi_rSampleDimension,
                        pi_DeltaX,
                        pi_DeltaY)
    {
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRANearestSampler::~HRANearestSampler()
    {
    }
