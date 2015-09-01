//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPyramidRaster.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRAPyramidRaster<T>
//-----------------------------------------------------------------------------


#include "HRAPyramidRasterIterator.h"
#include "HGFTileIDDescriptor.h"
#include "HRATiledRaster.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// CreateIterator    - Create an iterator.
//-----------------------------------------------------------------------------
inline HRARasterIterator* HRAPyramidRaster::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    return new HRAPyramidRasterIterator (HFCPtr<HRAPyramidRaster>((HRAPyramidRaster*)this),
                                         pi_rOptions,
                                         FindTheBestResolution(pi_rOptions.GetPhysicalCoordSys()));
    }


//-----------------------------------------------------------------------------
// public
// Create an editor
//-----------------------------------------------------------------------------
inline HRARasterEditor* HRAPyramidRaster::CreateEditor (HFCAccessMode pi_Mode)
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Create a shaped editor
//-----------------------------------------------------------------------------
inline HRARasterEditor* HRAPyramidRaster::CreateEditor (const HVEShape& pi_rShape,
                                                        HFCAccessMode  pi_Mode)
    {
    return 0;
    }



inline HRARasterEditor* HRAPyramidRaster::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// CountSubImages - Returns the number of sub resolution(image)
//                  The main image is included.
//-----------------------------------------------------------------------------
inline unsigned short HRAPyramidRaster::CountSubImages () const
    {
    return (unsigned short)m_pSubImageList.BufSize;
    }


//-----------------------------------------------------------------------------
// public
// GetSubImagesResolution - Returns the resolution for a specify sub-image.
//-----------------------------------------------------------------------------
inline double HRAPyramidRaster::GetSubImagesResolution (unsigned short pi_SubImageIndex) const
    {
    HPRECONDITION (pi_SubImageIndex < m_pSubImageList.BufSize);

    return m_pSubImageList.pData[pi_SubImageIndex].m_ImageResolution;
    }


//-----------------------------------------------------------------------------
// public
// HasSinglePixeltype -
//-----------------------------------------------------------------------------
inline bool HRAPyramidRaster::HasSinglePixelType  () const
    {
    return m_SinglePixelType;
    }


//-----------------------------------------------------------------------------
// private
// GetSubImage - Get a Raster pointer for a specify sub-image.
//-----------------------------------------------------------------------------
inline  HFCPtr<HRATiledRaster> HRAPyramidRaster::GetSubImage (unsigned short pi_Index) const
    {
    if (pi_Index < m_pSubImageList.BufSize)
        return m_pSubImageList.pData[pi_Index].m_pSubImage;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// public
// MakeSubImageDescriptor - Make a list of SubImageDescription for the current
//                          object.
//-----------------------------------------------------------------------------
inline HRAPyramidRaster::SubImageDescription* HRAPyramidRaster::MakeSubImageDescriptor()
    {
    // Make a list of SubImageDescriptor
    SubImageDescription* pSubImageDesc = new SubImageDescription[m_pSubImageList.BufSize-1];

    uint64_t BitmapWidth;
    uint64_t BitmapHeight;
    m_pSubImageList.pData[0].m_pSubImage->GetSize(&BitmapWidth, &BitmapHeight);

    CHECK_HUINT64_TO_HDOUBLE_CONV(BitmapWidth)

    double ImgWidth  = (double)BitmapWidth;

    for (unsigned short i=1; i<m_pSubImageList.BufSize; i++)
        {
        uint64_t BitmapWidth;
        uint64_t BitmapHeight;
        m_pSubImageList.pData[i].m_pSubImage->GetSize(&BitmapWidth, &BitmapHeight);
        pSubImageDesc[i-1].Resolution = 1.0 / round(ImgWidth / (double)BitmapWidth);
        }

    return (pSubImageDesc);
    }



//-----------------------------------------------------------------------------
// public
// UseOnlyFirstResolution
//-----------------------------------------------------------------------------
inline void HRAPyramidRaster::UseOnlyFirstResolution(bool pi_UseOnlyFirstRes)
    {
    m_UseOnlyFirstRes = pi_UseOnlyFirstRes;
    }


//-----------------------------------------------------------------------------
// public
// InvalidateRaster
//-----------------------------------------------------------------------------
inline void HRAPyramidRaster::InvalidateRaster()
    {
    //Should set the volatile layers to each tiled raster
    for (unsigned short ResInd = 0; ResInd < m_pSubImageList.BufSize; ResInd++)
        {
        m_pSubImageList.pData[ResInd].m_pSubImage->InvalidateRaster();
        }
    }

//-----------------------------------------------------------------------------
// protected
// EnableLookAhead
//-----------------------------------------------------------------------------
inline void HRAPyramidRaster::EnableLookAhead (bool pi_ByBlock)
    {
    HASSERT(GetStore() != 0);

    m_LookAheadEnabled = true;

   for (uint32_t i = 0; i < m_pSubImageList.BufSize; i++)
        m_pSubImageList.pData[i].m_pSubImage->EnableLookAhead(pi_ByBlock);
    }
END_IMAGEPP_NAMESPACE
