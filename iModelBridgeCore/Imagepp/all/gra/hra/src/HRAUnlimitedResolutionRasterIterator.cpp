//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAUnlimitedResolutionRasterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Methods for class HRAUnlimitedResolutionRasterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAUnlimitedResolutionRasterIterator.h>
#include <Imagepp/all/h/HRAUnlimitedResolutionRaster.h>


//-----------------------------------------------------------------------------
// public
// HRAPyramidRasterIterator - Full featured constructor.
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRasterIterator::HRAUnlimitedResolutionRasterIterator(const HFCPtr<HRAUnlimitedResolutionRaster>&  pi_pRaster,
                                                                           const HRAIteratorOptions&                    pi_rOptions,
                                                                           double                                      pi_Resolution)
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
HRAUnlimitedResolutionRasterIterator::~HRAUnlimitedResolutionRasterIterator()
    {
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapEditor - Copy constructor
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRasterIterator::HRAUnlimitedResolutionRasterIterator(const HRAUnlimitedResolutionRasterIterator& pi_rIterator)
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
HRAUnlimitedResolutionRasterIterator& HRAUnlimitedResolutionRasterIterator::operator=(const HRAUnlimitedResolutionRasterIterator& pi_rIterator)
    {
    if (this != &pi_rIterator)
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
void HRAUnlimitedResolutionRasterIterator::CreateIteratorResolution ()
    {
    // Create options for the source iterator. We need to clip
    // using our effective shape.
    HRAIteratorOptions SourceOptions(GetOptions());
    SourceOptions.ClipRegionToRaster(GetRaster());

    m_pRasterIterator = ((const HFCPtr<HRAUnlimitedResolutionRaster>&)m_pRaster)->CreateSubImage(m_Resolution)->CreateIterator(SourceOptions);
    }
