//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRASampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HRASampler
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGSTypes.h>
#include <Imagepp/all/h/HRASampler.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>

#include <Imagepp/all/h/HRAAverageSamplerN8.h>
#include <Imagepp/all/h/HRABicubicSamplerN8.h>
#include <Imagepp/all/h/HRABilinearSamplerN8.h>
#include <Imagepp/all/h/HRAGenericAverageSampler.h>
#include <Imagepp/all/h/HRANearestSamplerN1.h>
#include <Imagepp/all/h/HRANearestSamplerN8.h>
#include <Imagepp/all/h/HRANearestSamplerRLE1.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRASampler::HRASampler(HGSResampling const&                   pi_SamplingMode,
                       HGSMemoryBaseSurfaceDescriptor const&  pi_rMemorySurface,
                       const HGF2DRectangle&                  pi_rSampleDimension,
                       double                                 pi_DeltaX,
                       double                                 pi_DeltaY)
    {
  

    if(pi_rMemorySurface.IsCompatibleWith(HGSMemoryRLESurfaceDescriptor::CLASS_ID))
        {
        m_pSampler = new HRANearestSamplerRLE1Line(static_cast<HGSMemoryRLESurfaceDescriptor const&>(pi_rMemorySurface), pi_rSampleDimension, pi_DeltaX, pi_DeltaY);
        }
    else if (pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
             pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        {
        // Nothing better than nearest for RLE1
        m_pSampler = new HRANearestSamplerRLE1(static_cast<HGSMemorySurfaceDescriptor const&>(pi_rMemorySurface), pi_rSampleDimension, pi_DeltaX, pi_DeltaY);
        }
    else
        {
        HGSMemorySurfaceDescriptor const& rMemorySurface = static_cast<HGSMemorySurfaceDescriptor const&>(pi_rMemorySurface);
        if (pi_SamplingMode.GetResamplingMethod() == HGSResampling::AVERAGE &&
            !(HDOUBLE_EQUAL_EPSILON(pi_rSampleDimension.GetXMax() - pi_rSampleDimension.GetXMin(), 1.0) &&
              HDOUBLE_EQUAL_EPSILON(pi_rSampleDimension.GetYMax() - pi_rSampleDimension.GetYMin(), 1.0)))
            {
            if (pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == true)
                {
                if (pi_rMemorySurface.GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != 0)
                    {
                    m_pSampler = new HRAGenericAverageSparseDataSampler<float, double>(rMemorySurface,
                                                                                         pi_rSampleDimension,
                                                                                         pi_DeltaX,
                                                                                         pi_DeltaY);
                    }
                else
                    {
                    m_pSampler = new HRAGenericAverageSampler<float, double>(rMemorySurface,
                                                                               pi_rSampleDimension,
                                                                               pi_DeltaX,
                                                                               pi_DeltaY);
                    }
                }
            else if (pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == true)
                {
                if (pi_rMemorySurface.GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != 0)
                    {
                    m_pSampler = new HRAGenericAverageSparseDataSampler<short, int64_t>(rMemorySurface,
                                                                                         pi_rSampleDimension,
                                                                                         pi_DeltaX,
                                                                                         pi_DeltaY);
                    }
                else
                    {
                    m_pSampler = new HRAGenericAverageSamplerInteger<short, int64_t>(rMemorySurface,
                                                                                      pi_rSampleDimension,
                                                                                      pi_DeltaX,
                                                                                      pi_DeltaY);
                    }
                }
            else
                {
                // N8 supports N8 and N1 :-)
                m_pSampler = new HRAAverageSamplerN8(rMemorySurface,
                                                     pi_rSampleDimension,
                                                     pi_DeltaX,
                                                     pi_DeltaY);
                }
            }
        else
            {
            if (pi_SamplingMode.GetResamplingMethod() == HGSResampling::CUBIC_CONVOLUTION)
                {
                //HRAGenericBicubicSampler should be used instead. Should be fixed when TR 267979 is fixed.
                HASSERT((pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == false) &&
                        (pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == false));

                // N8 supports N8 and N1 :-)
                m_pSampler = new HRABicubicSamplerN8(rMemorySurface,
                                                     pi_rSampleDimension,
                                                     pi_DeltaX,
                                                     pi_DeltaY);
                }
            else
                {
                if (pi_SamplingMode.GetResamplingMethod() == HGSResampling::BILINEAR)
                    {
                    //HRAGenericBilinearSampler should be used instead. Should be fixed when TR 267979 is fixed.
                    HASSERT((pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == false) &&
                            (pi_rMemorySurface.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == false));

                    // N8 supports N8 and N1 :-)
                    m_pSampler = new HRABilinearSamplerN8(rMemorySurface,
                                                          pi_rSampleDimension,
                                                          pi_DeltaX,
                                                          pi_DeltaY);
                    }
                else
                    {
                    if (pi_rMemorySurface.GetPixelType()->CountPixelRawDataBits() < 8)
                        {
                        m_pSampler = new HRANearestSamplerN1(rMemorySurface,
                                                             pi_rSampleDimension,
                                                             pi_DeltaX,
                                                             pi_DeltaY);
                        }
                    else
                        {
                        m_pSampler = new HRANearestSamplerN8(rMemorySurface,
                                                             pi_rSampleDimension,
                                                             pi_DeltaX,
                                                             pi_DeltaY);
                        }
                    }
                }
            }
        }
    }