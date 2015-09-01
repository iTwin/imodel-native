//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTilePool.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFTilePool
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HRFTilePool::HRFTilePool()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HRFTilePool::~HRFTilePool()
    {
    Invalidate();

    // No need for this code anymore since the STL map bug
// is corrected (_Lockit)
#if 0
    Clear();
#endif
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::AddTile(uint64_t        pi_TileID,
                                 HFCPtr<HRFTile>& pi_rpTile)
    {
    HPRECONDITION(pi_rpTile != 0);
    HFCMonitor Monitor(this);

    m_TileMap.insert(TileMap::value_type(pi_TileID, pi_rpTile));
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCPtr<HRFTile> HRFTilePool::CreateTile(uint64_t pi_TileID,
                                               uint64_t pi_TileIndex,
                                               uint64_t pi_TilePosX,
                                               uint64_t pi_TilePosY,
                                               unsigned short pi_TileResolution)
    {
    HPRECONDITION(GetTile(pi_TileID) == 0);
    HFCMonitor Monitor(this);
    HFCPtr<HRFTile> pResult;

    // create the new tile
    pResult = new HRFTile(pi_TileID, pi_TileIndex, pi_TilePosX, pi_TilePosY, pi_TileResolution);
    HASSERT(pResult != 0);

    // Add it
    AddTile(pi_TileID, pResult);

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::RemoveTile(uint64_t pi_TileID)
    {
    HFCMonitor Monitor(this);

    TileMap::iterator Itr(m_TileMap.find(pi_TileID));
    if (Itr != m_TileMap.end())
        m_TileMap.erase(Itr);
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCPtr<HRFTile> HRFTilePool::GetTile(uint64_t pi_TileID) const
    {
    HFCMonitor Monitor(const_cast<HRFTilePool*>(this));
    HFCPtr<HRFTile> pResult;

    TileMap::const_iterator Itr(m_TileMap.find(pi_TileID));
    if (Itr != m_TileMap.end())
        pResult = Itr->second;

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::Clear()
    {
    HFCMonitor Monitor(this);

    while (m_TileMap.size() > 0)
        m_TileMap.erase(m_TileMap.begin());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::Invalidate()
    {
    HFCMonitor Monitor(this);

    TileMap::iterator Itr(m_TileMap.begin());
    while (Itr != m_TileMap.end())
        {
        Itr->second->Signal();
        Itr++;
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::InvalidateTilesNotIn(const HGFTileIDList&  pi_rTiles,
                                              bool                 pi_RemoveTiles)
    {
    HFCMonitor Monitor(this);

    TileMap::iterator TileItr(m_TileMap.begin());
    while (TileItr != m_TileMap.end())
        {
        if (find(pi_rTiles.begin(), pi_rTiles.end(), TileItr->first) == pi_rTiles.end())
            {
            TileItr->second->Signal();
            if (pi_RemoveTiles)
                TileItr = m_TileMap.erase(TileItr);
            else
                TileItr++;
            }
        else
            TileItr++;
        }
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HRFTilePool::InvalidateTiles(const HGFTileIDList&  pi_rTiles,
                                         bool                 pi_RemoveTiles)
    {
    HFCMonitor Monitor(this);

    TileMap::iterator TileItr;
    HGFTileIDList::const_iterator Itr(pi_rTiles.begin());
    while (Itr != pi_rTiles.end())
        {
        if ((TileItr = m_TileMap.find(*Itr)) != m_TileMap.end())
            {
            TileItr->second->Signal();
            if (pi_RemoveTiles)
                m_TileMap.erase(TileItr);
            }
        Itr++;
        }
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const HRFTilePool::TileMap&  HRFTilePool::GetTiles() const
    {
    return (m_TileMap);
    }
END_IMAGEPP_NAMESPACE
