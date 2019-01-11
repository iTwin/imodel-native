//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASamplingOptions.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRASamplingOptions
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Get the percentage of pixels to scan
    -----------------------------------------------------------------------------
*/
inline Byte HRASamplingOptions::GetPixelsToScan() const
    {
    return(m_PixelsToScan);
    }

/** -----------------------------------------------------------------------------
    Set the percentage of pixels to scan
    -----------------------------------------------------------------------------
*/
inline void HRASamplingOptions::SetPixelsToScan(Byte pi_PixelsToScan)
    {
    m_PixelsToScan = pi_PixelsToScan;
    }

/** -----------------------------------------------------------------------------
    Get the percentage of tiles to scan
    -----------------------------------------------------------------------------
*/
inline Byte HRASamplingOptions::GetTilesToScan() const
    {
    return(m_TilesToScan);
    }

/** -----------------------------------------------------------------------------
    Get the percentage of pyramid image to scan
    -----------------------------------------------------------------------------
*/
inline Byte HRASamplingOptions::GetPyramidImageSize() const
    {
    return(m_PyramidImageSize);
    }

/** -----------------------------------------------------------------------------
    Set the percentage of tiles to scan
    -----------------------------------------------------------------------------
*/
inline void HRASamplingOptions::SetTilesToScan(Byte pi_TilesToScan)
    {
    m_TilesToScan = pi_TilesToScan;
    }

/** -----------------------------------------------------------------------------
    Get the possible source pixel type replacer
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HRPPixelType>& HRASamplingOptions::GetSrcPixelTypeReplacer() const
    {
    return m_pSrcPixelTypeReplacer;
    }

/** -----------------------------------------------------------------------------
    Set the possible source pixel type replacer
    -----------------------------------------------------------------------------
*/
inline void HRASamplingOptions::SetSrcPixelTypeReplacer(const HFCPtr<HRPPixelType>& pi_pPixelType)
    {
    m_pSrcPixelTypeReplacer = pi_pPixelType;
    }

/** -----------------------------------------------------------------------------
    SetPyramidImageSize
    -----------------------------------------------------------------------------
*/
inline void HRASamplingOptions::SetPyramidImageSize(Byte pi_ImageSize)
    {
    m_PyramidImageSize = pi_ImageSize;
    }

/** -----------------------------------------------------------------------------
    Get the region of interest. The returned pointer can be null, in which
    case the region is unspecified. In this case, there will be no
    restriction based on the logical position of pixels.
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HVEShape>& HRASamplingOptions::GetRegionToScan() const
    {
    return m_pRegionToScan;
    }

/** -----------------------------------------------------------------------------
    Set the region of interest.
    -----------------------------------------------------------------------------
*/
inline void HRASamplingOptions::SetRegionToScan(const HFCPtr<HVEShape>& pi_rpRegion)
    {
    m_pRegionToScan = pi_rpRegion;
    }

END_IMAGEPP_NAMESPACE
