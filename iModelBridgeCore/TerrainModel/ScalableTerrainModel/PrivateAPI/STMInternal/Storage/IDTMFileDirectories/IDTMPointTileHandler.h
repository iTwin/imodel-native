//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMPointTileHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : PointTileHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.h>

#include <STMInternal/Storage/HPUArray.h>

#include <STMInternal/Storage/HTGFFPacketIter.h>
#include <STMInternal/Storage/HTGFFPacketManager.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Handler for tiles stored in PointDir. Can only be created via its
*               CreateFrom factory. CreateFrom will return null when handler is
*               incompatible with handled directory.
*
*               Provide an iterator facade that enables efficient access to tile
*               properties/data when iterating all tiles of this directory. Provide
*               tile accessors that enable adding, setting and getting a tile.
*
* @see          PointDir
*
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
class PointTileHandler : public HTGFF::DirectoryHandler
    {
public:
    typedef HPU::Array<PointType>
    PointArray;

private:
    class TileEditor : public HTGFF::PacketEditorBase<PointTileHandler>
        {
    public:
        TileID                  GetID                              () const;

        const Extent3d64f&      GetExtent                          () const;
        ResolutionID            GetResolution                      () const;

        size_t                  CountPoints                        () const;

        bool                    GetPoints                          (PointArray&             po_rTilePoints) const;
        };

public:
    typedef BentleyApi::ImagePP::HFCPtr<PointTileHandler>    
                                Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<PointTileHandler>    
                                CPtr;

    typedef const TileEditor    CTile;
    typedef TileEditor          Tile;

    typedef HTGFF::PacketIter<CTile> 
                                TileCIter;
    typedef HTGFF::PacketIter<Tile> 
                                TileIter;


    static bool                 IsCompatibleWith                   (const PointDir&             pi_rDir);
    static bool                 IsCompatibleWith                   (const UniformFeatureDir&    pi_rDir);

    static Ptr                  CreateFrom                         (PointDir*                   pi_rpDir);
    static Ptr                  CreateFrom                         (UniformFeatureDir*          pi_rpDir);
    static CPtr                 CreateFrom                         (const PointDir*             pi_rpDir);
    static CPtr                 CreateFrom                         (const UniformFeatureDir*    pi_rpDir);

    const PointDir&             GetDir                             () const {
        return *m_pPointDir;
        }
    PointDir&                   GetDir                             () {
        return *m_pPointDir;
        }

    /*---------------------------------------------------------------------------------**//**
    * Tiles data accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
    TileCIter                   TilesBegin                         () const;
    TileCIter                   TilesEnd                           () const;

    TileIter                    TilesBeginEdit                     ();
    TileIter                    TilesEndEdit                       ();

    // TDORAY: Make available GetTile methods instead?? How would we check that the requested tile exist == TileIterEnd? Would
    //         that be efficient?

    bool                        GetPoints                          (TileID                      pi_ID,
                                                                    PointArray&                 po_rTilePoints) const;

    bool                        SetPoints                          (TileID                      pi_ID,
                                                                    const PointArray&           pi_rTilePoints);

    bool                        AddPoints                          (TileID&                     po_rID,
                                                                    const PointArray&           pi_rTilePoints);

    bool                        RemovePoints                       (TileID                      pi_ID);

private:
    friend class                Directory;

    /*---------------------------------------------------------------------------------**//**
    * Compile time listing of supported point types for this handler
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <PointTypeID TypeID>
    struct IsSupportedPointTypeTrait                            {
        enum {value = 0};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_XYZf64>         {
        enum {value = 1};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_XYZf64RGBIi8>   {
        enum {value = 1};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_XYZf64Gi32>     {
        enum {value = 1};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_XYZMf64>        {
        enum {value = 1};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_XYZMf64Gi32>    {
        enum {value = 1};
        };
    template <>                 struct IsSupportedPointTypeTrait<POINT_TYPE_INTEGER>
        {
        enum { value = 1 };
        };

    static PointTileHandler*    CreateFromImpl                 (PointDir*                   pi_pDir);

    explicit                    PointTileHandler               (PointDir*                   pi_rpPointDir)
        : m_pPointDir(pi_rpPointDir) {}

    virtual bool                _Save                          () override { return true; }
    virtual bool                _Load                          () override { return true; }

    PointDir*                   m_pPointDir;
    };


} //End namespace IDTMFile

#include "IDTMPointTileHandler.hpp"