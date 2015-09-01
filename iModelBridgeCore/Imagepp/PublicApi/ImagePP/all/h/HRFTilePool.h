//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTilePool.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFTilePool
//-----------------------------------------------------------------------------

#pragma once

#include "HRFTile.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE
class HRFTilePool : public HFCExclusiveKey
    {
public:
    //--------------------------------------
    // Map of tiles
    //--------------------------------------

    typedef map<uint64_t, HFCPtr<HRFTile> >
    TileMap;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFTilePool();
    virtual         ~HRFTilePool();


    //--------------------------------------
    // Tile Methods
    //--------------------------------------

    // Adds a tile to the pool
    void            AddTile     (uint64_t        pi_TileID,
                                 HFCPtr<HRFTile>& pi_rpTile);

    // Creates a new tile in the pool for future reference
    HFCPtr<HRFTile> CreateTile  (uint64_t pi_TileID,
                                 uint64_t pi_TileIndex,
                                 uint64_t pi_TilePosX,
                                 uint64_t pi_TilePosY,
                                 unsigned short pi_TileResolution);

    // Removes a tile from the pool
    void            RemoveTile  (uint64_t pi_TileID);

    // Extract a tile from the pool
    HFCPtr<HRFTile> GetTile     (uint64_t pi_TileID) const;

    // Removes all tiles from the pool
    void            Clear       ();

    // Invalidates all the tiles in the pool.
    void            Invalidate  ();
    void            InvalidateTilesNotIn(const HGFTileIDList&   pi_rTiles,
                                         bool                  pi_RemoveTiles);
    void            InvalidateTiles     (const HGFTileIDList&   pi_rTiles,
                                         bool                  pi_RemoveTiles);


    //--------------------------------------
    // Iteration
    //--------------------------------------

    // The tile pool object must be claimed by
    // the caller of this method
    const TileMap&  GetTiles() const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Map of tiles with the ID as key
    TileMap         m_TileMap;
    };
END_IMAGEPP_NAMESPACE

#include "HRFTilePool.hpp"

