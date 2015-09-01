//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAPyramidRasterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Methods for class HRAPyramidRasterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRAPyramidRasterIterator.h>


//-----------------------------------------------------------------------------
// public
// HRAPyramidRasterIterator - Full featured constructor.
//-----------------------------------------------------------------------------
HRAPyramidRasterIterator::HRAPyramidRasterIterator(const HFCPtr<HRAPyramidRaster>& pi_pRaster,
                                                   const HRAIteratorOptions&       pi_rOptions,
                                                   double                         pi_Resolution)
    : HRARasterIterator ((HFCPtr<HRARaster>&) pi_pRaster, pi_rOptions),
      m_Resolution(pi_Resolution)
    {
    HPRECONDITION (pi_pRaster != 0);

    // Keep local copy of the pointer
    m_pRaster = pi_pRaster;

    CreateIteratorResolution ();
    }

//-----------------------------------------------------------------------------
// public
// ~HRAPyramidRasterIterator - Destructor
//-----------------------------------------------------------------------------
HRAPyramidRasterIterator::~HRAPyramidRasterIterator()
    {
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapEditor - Copy constructor
//-----------------------------------------------------------------------------
HRAPyramidRasterIterator::HRAPyramidRasterIterator(const HRAPyramidRasterIterator& pi_rIterator)
    : HRARasterIterator (pi_rIterator),
      m_Resolution(pi_rIterator.m_Resolution)
    {
    m_pRaster = pi_rIterator.m_pRaster;

    CreateIteratorResolution ();
    }

//-----------------------------------------------------------------------------
// public
// operator= - Assignment operator
//-----------------------------------------------------------------------------
HRAPyramidRasterIterator& HRAPyramidRasterIterator::operator=(const HRAPyramidRasterIterator& pi_rIterator)
    {
    if(this != &pi_rIterator)
        {
        HRARasterIterator::operator=(pi_rIterator);

        m_pRaster    = pi_rIterator.m_pRaster;
        m_Resolution = pi_rIterator.m_Resolution;

        // Delete the current Iterator and create a new one.
        m_pRasterIterator = 0;
        CreateIteratorResolution ();
        }

    return(*this);
    }



//-------------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// CreateIteratorResolution  -
//-----------------------------------------------------------------------------

void HRAPyramidRasterIterator::CreateIteratorResolution ()
    {
    unsigned short NbSubImage      = m_pRaster->CountSubImages();
    unsigned short i = 0;

    if (NbSubImage > 1)
        {
        // This will determine what span of requested resolutions a particular
        // raster resolution will serve.
        double ResolutionMultiplicator = 1.0 + (GetOptions().MaxResolutionStretchingFactor() / 100.0);

        // Find the good Resolution to use.
        for (i=1; (i<NbSubImage) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_pRaster->GetSubImagesResolution(i) * ResolutionMultiplicator,
                                              m_Resolution); i++);
        i--;
        }

    // Create options for the source iterator. We need to clip
    // using our effective shape.
    HRAIteratorOptions SourceOptions(GetOptions());
    SourceOptions.ClipRegionToRaster(GetRaster());

    // Do we ask for a sub-resolution
    if (i > 0)
        {
        if (SourceOptions.IsShaped())
            {
            HGF2DExtent Extent(SourceOptions.GetRegionToProcess()->GetExtent());
            m_pRaster->UpdateSubResolution(i, &Extent);
            }
        else
            {
            // Update SubResolution if needed
            m_pRaster->UpdateSubResolution(i);
            }
        }

    m_pRasterIterator = m_pRaster->GetSubImage(i)->CreateIterator(SourceOptions);
    }
