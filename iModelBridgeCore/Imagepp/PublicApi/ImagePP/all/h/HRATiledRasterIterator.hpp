//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATiledRasterIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Inline methods for class HRATiledRasterIterator
//-----------------------------------------------------------------------------

#include "HRATiledRaster.h"


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
inline void HRATiledRasterIterator::PrepareCurrentTile ()
    {
    if (m_Index >= m_MaxIndex)
        m_pCurrentTile = 0;
    else
        m_pCurrentTile = m_pTileRaster->GetTileByIndex(m_Index)->GetTile();
    }


//-----------------------------------------------------------------------------
// private
// SearchNextIndex - Search the next Index (Tile), if the Iterator is shaped
//                   this function eliminates the tile that not touch to the
//                   shape and set the member m_Index.
//-----------------------------------------------------------------------------
inline void HRATiledRasterIterator::SearchNextIndex (bool pi_FirstCall)
    {
    if (pi_FirstCall)
        m_Index = m_IDIterator.GetFirstTileIndex();
    else
        m_Index = m_IDIterator.GetNextTileIndex();
    }


//-----------------------------------------------------------------------------
// public
// Next
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRATiledRasterIterator::Next()
    {
    // Next tile
    SearchNextIndex();

    PrepareCurrentTile();

    return (HFCPtr<HRARaster>&) m_pCurrentTile;
    }

//-----------------------------------------------------------------------------
// public
// operator()
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRATiledRasterIterator::operator()()
    {
    return (HFCPtr<HRARaster>&) m_pCurrentTile;
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
inline void HRATiledRasterIterator::Reset()
    {
    // m_Index is set in the following method
    SearchNextIndex (true);

    PrepareCurrentTile();
    }
END_IMAGEPP_NAMESPACE
