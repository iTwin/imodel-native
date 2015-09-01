//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMBlendCorridorIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HIMBlendCorridorIterator.h>



/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HIMBlendCorridorIterator::HIMBlendCorridorIterator(const HFCPtr<HIMBlendCorridor>& pi_rpBlendCorridor,
                                                   const HRAIteratorOptions&       pi_rOptions)
    : HRARasterIterator( (HFCPtr<HRARaster>&) pi_rpBlendCorridor, pi_rOptions)
    {
    // Calculate the region to retrieve
    HFCPtr<HVEShape> pTotalShapeToProcess(pi_rOptions.CalculateClippedRegion((HFCPtr<HRARaster>&) pi_rpBlendCorridor));
    pTotalShapeToProcess->ChangeCoordSys(pi_rpBlendCorridor->GetPhysicalCoordSys());

    m_IDIterator.SetParameters(pi_rpBlendCorridor->GetTileDescriptor(), pTotalShapeToProcess);

    m_MaxIndex  = pi_rpBlendCorridor->GetTileDescriptor()->GetTileCount();

    Reset();
    }


/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HIMBlendCorridorIterator::HIMBlendCorridorIterator(const HIMBlendCorridorIterator& pi_rObj)
    : HRARasterIterator(pi_rObj)
    {
    m_Index     = pi_rObj.m_Index;
    m_MaxIndex  = pi_rObj.m_MaxIndex;

    // Calculate the region to process.
    HFCPtr<HVEShape> pTotalShapeToProcess(GetOptions().CalculateClippedRegion(GetRaster()));
    pTotalShapeToProcess->ChangeCoordSys(((HFCPtr<HIMBlendCorridor>&)GetRaster())->GetPhysicalCoordSys());

    m_IDIterator.SetParameters(((HFCPtr<HIMBlendCorridor>&)GetRaster())->GetTileDescriptor(), pTotalShapeToProcess);

    PrepareCurrentTile();
    }


/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HIMBlendCorridorIterator::~HIMBlendCorridorIterator()
    {
    }


/** -----------------------------------------------------------------------------
    Assignment
    -----------------------------------------------------------------------------
*/
HIMBlendCorridorIterator& HIMBlendCorridorIterator::operator=(
    const HIMBlendCorridorIterator& pi_rObj)
    {
    if(this != &pi_rObj)
        {
        HRARasterIterator::operator=(pi_rObj);

        m_Index       = pi_rObj.m_Index;
        m_MaxIndex    = pi_rObj.m_MaxIndex;

        // Calculate the region to process.
        HFCPtr<HVEShape> pTotalShapeToProcess(GetOptions().CalculateClippedRegion(GetRaster()));
        pTotalShapeToProcess->ChangeCoordSys(((HFCPtr<HIMBlendCorridor>&)GetRaster())->GetPhysicalCoordSys());

        m_IDIterator.SetParameters(((HFCPtr<HIMBlendCorridor>&)GetRaster())->GetTileDescriptor(), pTotalShapeToProcess);

        PrepareCurrentTile();
        }

    return(*this);
    }


/** -----------------------------------------------------------------------------
    Prepare the tile to return
    -----------------------------------------------------------------------------
*/
void HIMBlendCorridorIterator::PrepareCurrentTile ()
    {
    if (m_Index >= m_MaxIndex)
        m_pCurrentTile = 0;
    else
        m_pCurrentTile = ((HFCPtr<HIMBlendCorridor>&)GetRaster())->GetTile(m_Index);
    }


/** -----------------------------------------------------------------------------
    Search the next useful Index (Tile). The IDIterator object will reject
    IDs of tiles that don't touch the region to process.
    -----------------------------------------------------------------------------
*/
void HIMBlendCorridorIterator::SearchNextIndex (bool pi_FirstCall)
    {
    if (pi_FirstCall)
        m_Index = m_IDIterator.GetFirstTileIndex();
    else
        m_Index = m_IDIterator.GetNextTileIndex();
    }


/** -----------------------------------------------------------------------------
    Advance to the next tile
    -----------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMBlendCorridorIterator::Next()
    {
    // Next tile
    SearchNextIndex();

    PrepareCurrentTile();

    return (HFCPtr<HRARaster>&) m_pCurrentTile;
    }


/** -----------------------------------------------------------------------------
    Return the current tile
    -----------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMBlendCorridorIterator::operator()()
    {
    return m_pCurrentTile;
    }


/** -----------------------------------------------------------------------------
    Restart the iteration process.
    -----------------------------------------------------------------------------
*/
void HIMBlendCorridorIterator::Reset()
    {
    // m_Index is set in the following method
    SearchNextIndex (true);

    PrepareCurrentTile();
    }
