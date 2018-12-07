//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFTileIDDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFTileIDDescriptor
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DExtent;

typedef list<uint64_t> HGFTileIDList;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author DM

    HGFTileIDDescriptor is a class that describes the position of each tile in
    an area. The tiles are numbered from Top-Right corner to Bottom-Left corner.
    This class is used to translate tile position to Index and Index to tile
    position.
    The tile has a position (X, Y) in the area.
    Ex.: Area Width = 600, Height = 600, TileSizeX,Y = 100
    Index 0 ' Tile position (0,0)
    Index 1 ' Tile position (100, 0)
    Index 5 ' Tile position (500, 0)
    Index 8 ' Tile position (200, 100)
    The tileID is a UInt32 type, the tileID is decomposed in 2 fields, 1 byte for
    the Level and the 3 other bytes for the Index.
    The Level field can be use for many things.
          ex.: use to specify the SubImage.

    -----------------------------------------------------------------------------
*/
class HGFTileIDDescriptor : public HFCShareableObject<HGFTileIDDescriptor>
    {
    HDECLARE_SEALEDCLASS_ID(HGFTileIDDescriptorId_Base)

public:

    // Constants

    IMAGEPP_EXPORT static  const uint64_t INDEX_NOT_FOUND;

    // Primary methods

    IMAGEPP_EXPORT HGFTileIDDescriptor     ();
    IMAGEPP_EXPORT HGFTileIDDescriptor(uint64_t              pi_Width,
                                                    uint64_t              pi_Height,
                                                    uint64_t              pi_TileSizeX,
                                                    uint64_t              pi_TileSizeY);
    IMAGEPP_EXPORT ~HGFTileIDDescriptor    ();

    HGFTileIDDescriptor     (const HGFTileIDDescriptor& pi_rObj);

    IMAGEPP_EXPORT HGFTileIDDescriptor&    operator=      (const HGFTileIDDescriptor& pi_rObj);

    // Compute Methods

    uint64_t      ComputeIndex            (uint64_t              pi_TilePosX,
                                            uint64_t              pi_TilePosY) const;
    uint64_t      ComputeID               (uint64_t              pi_TilePosX,
                                            uint64_t              pi_TilePosY,
                                            uint32_t               pi_Level = 0) const;
    uint64_t      ComputeIDFromIndex      (uint64_t              pi_Index,
                                            uint32_t               pi_Level = 0) const;

    // Get Methods

    uint32_t       GetLevel                (uint64_t              pi_TileID) const;
    uint64_t      GetIndex                (uint64_t              pi_TileID) const;
    void           GetPositionFromID       (uint64_t              pi_TileID,
                                            uint64_t*               po_pTilePosX,
                                            uint64_t*               po_pTilePosY);
    void           GetPositionFromIndex    (uint64_t              pi_Index,
                                            uint64_t*               po_pTilePosX,
                                            uint64_t*               po_pTilePosY);
    void           GetTileDataSize         (uint64_t              pi_TileIndex,
                                            uint64_t&               po_rDataWidth,
                                            uint64_t&               po_rDataHeight);

    // Tile and image dimension
    uint64_t      GetImageWidth           () const;
    uint64_t      GetImageHeight          () const;
    uint64_t      GetTileWidth            () const;
    uint64_t      GetTileHeight           () const;

    uint64_t      GetTileCount            () const;
    uint64_t      GetTileCount            (HGF2DExtent& pi_rExtent) const;

    // Query methods
    IMAGEPP_EXPORT uint64_t      GetFirstTileIndex       (const HGF2DExtent&     pi_rExtent);

    uint64_t      GetFirstTileIndex       (uint64_t              pi_XMin,
                                            uint64_t              pi_YMin,
                                            uint64_t              pi_XCount,
                                            uint64_t              pi_YCount);
    uint64_t      GetNextTileIndex        ();

    // Modify methods
    void           ChangeSize              (uint64_t              pi_Width,
                                            uint64_t              pi_Height);




#if 0
    // Debug function
    virtual void    PrintState(ostream& po_rOutput) const;
#endif

protected:
private:

    // Members

    // Image Size
    uint64_t       m_ImageWidth;
    uint64_t       m_ImageHeight;

    // TileSize
    uint64_t       m_TileSizeX;
    uint64_t       m_TileSizeY;

    uint64_t       m_NumberOfTileX;

    // Query information (not persistent)
    uint64_t       m_CurIndex;
    uint64_t       m_FirstCurIndex;    // Index in the current row
    uint64_t       m_LastCurIndex;
    uint64_t       m_LastFirstColumn;  // Index of the last row
    };

END_IMAGEPP_NAMESPACE

#include "HGFTileIDDescriptor.hpp"

