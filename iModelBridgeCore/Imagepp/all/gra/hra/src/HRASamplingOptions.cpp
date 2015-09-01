//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRASamplingOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRASamplingOptions.h>
/** -----------------------------------------------------------------------------
    Default constructor
    -----------------------------------------------------------------------------
*/
HRASamplingOptions::HRASamplingOptions()
    {
    // default values
    m_PixelsToScan = 25;
    m_TilesToScan = 100;
    m_PyramidImageSize = 20;
    m_pFilter = 0;
    }

/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HRASamplingOptions::HRASamplingOptions(const HRASamplingOptions& pi_rSamplingOptions)
    {
    DeepCopy(pi_rSamplingOptions);
    }

/** -----------------------------------------------------------------------------
    Assignment
    -----------------------------------------------------------------------------
*/
HRASamplingOptions& HRASamplingOptions::operator=(const HRASamplingOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // We don't delete the filter and border generator,
        // we don't own them.

        DeepCopy(pi_rObj);
        }

    return *this;
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HRASamplingOptions::~HRASamplingOptions()
    {
    // We don't delete the filter and border generator,
    // we don't own them.
    }

/** -----------------------------------------------------------------------------
// public
// DeepCopy
    -----------------------------------------------------------------------------
*/
void HRASamplingOptions::DeepCopy(const HRASamplingOptions& pi_rObj)
    {
    m_PixelsToScan      = pi_rObj.m_PixelsToScan;
    m_TilesToScan       = pi_rObj.m_TilesToScan;
    m_PyramidImageSize  = pi_rObj.m_PyramidImageSize;

    // CAUTION: Dumb pointer copy of the filter and generator. For
    // performance only. Because of this, we should never keep
    // a SamplingOptions object.
    m_pFilter = pi_rObj.m_pFilter;

    if (pi_rObj.m_pRegionToScan != 0)
        m_pRegionToScan = new HVEShape(*pi_rObj.m_pRegionToScan);
    else
        m_pRegionToScan = 0;

    if (pi_rObj.m_pSrcPixelTypeReplacer != 0)
        m_pSrcPixelTypeReplacer = (HRPPixelType*)pi_rObj.m_pSrcPixelTypeReplacer->Clone();
    }
