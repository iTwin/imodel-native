//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRARasterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRARasterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRARasterIterator.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>



//-----------------------------------------------------------------------------
// The constructor.  It takes a pointer to the raster to scan, and the region
// to scan.
//-----------------------------------------------------------------------------
HRARasterIterator::HRARasterIterator(const HFCPtr<HRARaster>&  pi_pRaster,
                                     const HRAIteratorOptions& pi_rOptions)
    : m_Options(pi_rOptions),
      m_pRaster(pi_pRaster)
    {
    // We need a personal version of the options.
    m_Options.CreatePrivateCopies();
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRARasterIterator::HRARasterIterator(const HRARasterIterator& pi_rObj)
    : m_Options(pi_rObj.m_Options),
      m_pRaster(pi_rObj.m_pRaster)
    {
    // We need a personal version of the options.
    m_Options.CreatePrivateCopies();
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRARasterIterator::~HRARasterIterator()
    {
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another iterator.
//-----------------------------------------------------------------------------
HRARasterIterator& HRARasterIterator::operator=(const HRARasterIterator& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Take a copy of the raster pointer
        m_pRaster = pi_rObj.m_pRaster;

        m_Options = pi_rObj.m_Options;

        // We need a personal version of the options.
        m_Options.CreatePrivateCopies();
        }
    return *this;
    }


//-----------------------------------------------------------------------------
// Returns the scanned region.
//-----------------------------------------------------------------------------
const HRAIteratorOptions& HRARasterIterator::GetOptions() const
    {
    return m_Options;
    }


//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRARasterIterator::PrintState (ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    po_rOutput

            << "HRARasterIterator"
            << endl;

#endif
    }
