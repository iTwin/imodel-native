//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASampler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline method for class HRASampler
//:>-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
inline HRASampler::~HRASampler()
    {
    }

/**----------------------------------------------------------------------------
 Get pixel

 @param pi_PosX
 @param pi_PosY

 @return void* A void pointer to the pixel
-----------------------------------------------------------------------------*/
inline void const* HRASampler::GetPixel(double pi_PosX, double pi_PosY) const
    {
    return m_pSampler->GetPixel(pi_PosX,
                                pi_PosY);
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location

 @param pi_pPositionsX
 @param pi_pPositionsY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
inline void HRASampler::GetPixels(const double*  pi_pPositionsX,
                                  const double*  pi_pPositionsY,
                                  size_t          pi_PixelCount,
                                  void*           po_pBuffer) const
    {
    m_pSampler->GetPixels(pi_pPositionsX,
                          pi_pPositionsY,
                          pi_PixelCount,
                          po_pBuffer);
    }

/**----------------------------------------------------------------------------
 Get pixels at specific location

 @param pi_PositionX
 @param pi_PositionY
 @param pi_DeltaX
 @param pi_DeltaY
 @param pi_PixelCount
 @param po_pBuffer
-----------------------------------------------------------------------------*/
inline void HRASampler::GetPixels(double         pi_PositionX,
                                  double         pi_PositionY,
                                  size_t          pi_PixelCount,
                                  void*           po_pBuffer) const
    {
    m_pSampler->GetPixels(pi_PositionX,
                          pi_PositionY,
                          pi_PixelCount,
                          po_pBuffer);
    }


/**----------------------------------------------------------------------------
   Retrieve the pixeltype in which the sampled data will be returned.
   -----------------------------------------------------------------------------
*/
inline HFCPtr<HRPPixelType> HRASampler::GetOutputPixelType() const
    {
    return m_pSampler->GetOutputPixelType();
    }


/**----------------------------------------------------------------------------
   Try to sample in the specified pixeltype. This can only be used if the
   basic output pixeltype is different than the source pixeltype.
   If TryToUse returns true, following calls to GetOutputPixelType() should
   return the given pixeltype.

   @see GetOutputPixelType()
   -----------------------------------------------------------------------------
*/
inline bool HRASampler::TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType)
    {
    return m_pSampler->TryToUse(pi_rpOutputPixelType);
    }

/**----------------------------------------------------------------------------
Set the current scale to the sampler
-----------------------------------------------------------------------------
*/
inline void HRASampler::SetScale(double pi_ScaleX, double pi_ScaleY)
    {
    m_pSampler->SetScale(pi_ScaleX, pi_ScaleY);
    }
END_IMAGEPP_NAMESPACE
