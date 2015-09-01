//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRATiledRasterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Methods for class HRATiledRasterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRATiledRaster.h>
#include <Imagepp/all/h/HRATiledRasterIterator.h>


//-----------------------------------------------------------------------------
// public
// HRATiledRasterIterator - Full featured constructor.
//-----------------------------------------------------------------------------
HRATiledRasterIterator::HRATiledRasterIterator(const HFCPtr<HRATiledRaster>& pi_pTileRaster,
                                               const HRAIteratorOptions&     pi_rOptions)
    : HRARasterIterator ( (HFCPtr<HRARaster>&) pi_pTileRaster, pi_rOptions),
      m_TileDescription(*pi_pTileRaster->GetPtrTileDescription())
    {
    HPRECONDITION (pi_pTileRaster != 0);

    // m_Index will be set when calling Reset...

    m_MaxIndex  = m_TileDescription.GetTileCount();

    m_pTileRaster = pi_pTileRaster;

    // Calculate the region to process.
    HFCPtr<HVEShape> pTotalShapeToProcess(pi_rOptions.CalculateClippedRegion((HFCPtr<HRARaster>&) pi_pTileRaster));
    pTotalShapeToProcess->ChangeCoordSys(m_pTileRaster->GetPhysicalCoordSys());

    m_IDIterator.SetParameters(&m_TileDescription, pTotalShapeToProcess);

    // Try-Catch, because the HRFFile can throw an error.
    //
    try {
        Reset ();
        }
    catch(...)
        {
        m_pTileRaster = 0;
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// ~HRATiledRasterIterator - Destructor
//-----------------------------------------------------------------------------
HRATiledRasterIterator::~HRATiledRasterIterator()
    {
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapEditor - Copy constructor
//-----------------------------------------------------------------------------
HRATiledRasterIterator::HRATiledRasterIterator(const HRATiledRasterIterator& pi_rIterator)
    : HRARasterIterator (pi_rIterator),
      m_TileDescription(*pi_rIterator.m_pTileRaster->GetPtrTileDescription())
    {
    m_Index     = pi_rIterator.m_Index;
    m_MaxIndex  = pi_rIterator.m_MaxIndex;

    m_pTileRaster = pi_rIterator.m_pTileRaster;

    // Calculate the region to process.
    HFCPtr<HVEShape> pTotalShapeToProcess(GetOptions().CalculateClippedRegion(GetRaster()));
    pTotalShapeToProcess->ChangeCoordSys(m_pTileRaster->GetPhysicalCoordSys());

    m_IDIterator.SetParameters(&m_TileDescription, pTotalShapeToProcess);

    PrepareCurrentTile();
    }

//-----------------------------------------------------------------------------
// public
// operator= - Assignment operator
//-----------------------------------------------------------------------------
HRATiledRasterIterator& HRATiledRasterIterator::operator=(const HRATiledRasterIterator& pi_rIterator)
    {
    if(this != &pi_rIterator)
        {
        HRARasterIterator::operator=(pi_rIterator);

        m_Index       = pi_rIterator.m_Index;
        m_MaxIndex    = pi_rIterator.m_MaxIndex;

        m_pTileRaster = pi_rIterator.m_pTileRaster;

        // Calculate the region to process.
        HFCPtr<HVEShape> pTotalShapeToProcess(GetOptions().CalculateClippedRegion(GetRaster()));
        pTotalShapeToProcess->ChangeCoordSys(m_pTileRaster->GetPhysicalCoordSys());

        m_TileDescription = *m_pTileRaster->GetPtrTileDescription();

        m_IDIterator.SetParameters(&m_TileDescription, pTotalShapeToProcess);

        PrepareCurrentTile();
        }

    return(*this);
    }
