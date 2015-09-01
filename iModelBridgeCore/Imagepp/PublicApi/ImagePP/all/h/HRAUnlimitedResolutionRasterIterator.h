//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAUnlimitedResolutionRasterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Class: HRAUnlimitedResolutionRasterIterator
// ----------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"
#include "HRAReferenceToRaster.h"

BEGIN_IMAGEPP_NAMESPACE
class HRAUnlimitedResolutionRaster;

// ----------------------------------------------------------------------------
//  HRAUnlimitedResolutionRasterIterator
// ----------------------------------------------------------------------------
class HRAUnlimitedResolutionRasterIterator : public HRARasterIterator

    {
public:
    // Primary methods

    HRAUnlimitedResolutionRasterIterator(const HFCPtr<HRAUnlimitedResolutionRaster>& pi_pRaster,
                                         const HRAIteratorOptions&                   pi_rOptions,
                                         double                                     pi_Resolution = 1.0);

    HRAUnlimitedResolutionRasterIterator(const HRAUnlimitedResolutionRasterIterator& pi_rObj);

    virtual         ~HRAUnlimitedResolutionRasterIterator(void);

    HRAUnlimitedResolutionRasterIterator&   operator=(const HRAUnlimitedResolutionRasterIterator& pi_rBitmap);

    // Inherited from HRARasterIterator

    virtual const HFCPtr<HRARaster>&    Next();
    virtual const HFCPtr<HRARaster>&    operator()();

    virtual void                        Reset();

protected:

private:
    // Members
    HFCPtr<HRARaster>                       m_pRaster;

    // Resolution used to select the good sub-image
    double                                 m_Resolution;

    // RasterIterator on the specify resolution.
    HAutoPtr<HRARasterIterator>             m_pRasterIterator;

    // Methods

    void CreateIteratorResolution ();
    };
END_IMAGEPP_NAMESPACE

#include "HRAUnlimitedResolutionRasterIterator.hpp"

