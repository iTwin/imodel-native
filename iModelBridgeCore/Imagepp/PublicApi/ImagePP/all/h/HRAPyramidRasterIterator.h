//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPyramidRasterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//
// Class: HRAPyramidRasterIterator
// ----------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"

BEGIN_IMAGEPP_NAMESPACE
class HRAPyramidRaster;

// ----------------------------------------------------------------------------
//  HRAPyramidRasterIterator
// ----------------------------------------------------------------------------
class HRAPyramidRasterIterator : public HRARasterIterator

    {
public:
    // Primary methods

    HRAPyramidRasterIterator   (const HFCPtr<HRAPyramidRaster>& pi_pRaster,
                                const HRAIteratorOptions&       pi_rOptions,
                                double                         pi_Resolution = 1.0);

    HRAPyramidRasterIterator   (const HRAPyramidRasterIterator& pi_rObj);

    virtual         ~HRAPyramidRasterIterator(void);

    HRAPyramidRasterIterator&   operator=(const HRAPyramidRasterIterator& pi_rBitmap);

    // Inherited from HRARasterIterator

    virtual const HFCPtr<HRARaster>&
    Next();
    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();

protected:

private:
    // Members

    // Keep a copy on the Raster pointer.
    HFCPtr<HRAPyramidRaster>  m_pRaster;

    // Resolution used to select the good sub-image
    double m_Resolution;

    // RasterIterator on the specify resolution.
    HAutoPtr<HRARasterIterator> m_pRasterIterator;

    // Methods

    void CreateIteratorResolution ();
    };
END_IMAGEPP_NAMESPACE

#include "HRAPyramidRasterIterator.hpp"

