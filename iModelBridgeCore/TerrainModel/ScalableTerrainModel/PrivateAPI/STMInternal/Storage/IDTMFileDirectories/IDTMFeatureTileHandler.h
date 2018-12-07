//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureTileHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointTileHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderTileHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.h>

#include <STMInternal/Storage/IDTMFeatureArray.h>

#include <STMInternal/Storage/HPUArray.h>
#include <STMInternal/Storage/HTGFFPacketIter.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Handler for tiles stored in FeatureDir. Can only be created via its
*               CreateFrom factory. CreateFrom will return null when handler is
*               incompatible with handled directory.
*
*               Provide an iterator facade that enables efficient access to tile
*               properties/data when iterating all tiles of this directory. Provide
*               tile accessors that enable adding, setting and getting a tile.
*
* @see          FeatureDir
*
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename HeaderT = FeatureHeader>
class FeatureTileHandler : public HTGFF::DirectoryHandler
    {
public:
    typedef IDTMFeatureArray<PointType, HeaderT>
    Array;
private:
    typedef FeatureTileHandler<PointType, HeaderT>
    FeatureHandler;
    typedef PointTileHandler<PointType>
    PointHandler;
    typedef FeatureHeaderTileHandler<HeaderT>
    HeaderHandler;

protected:
    class TileEditor : public HTGFF::PacketEditorBase<FeatureHandler>
        {
    public:
        TileID                  GetID                              () const;

        const Extent3d64f&      GetExtent                          () const;
        ResolutionID            GetResolution                      () const;

        size_t                  CountFeatures                      () const;
        size_t                  CountPoints                        () const;

        bool                    GetFeatures                        (Array&        po_rFeatures) const;
        };

public:
    typedef BentleyApi::ImagePP::HFCPtr<FeatureHandler>      
                                Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<FeatureHandler>      
                                CPtr;

    typedef const TileEditor    CTile;
    typedef TileEditor          Tile;

    typedef HTGFF::PacketIter<CTile> 
                                TileCIter;
    typedef HTGFF::PacketIter<Tile> 
                                TileIter;

    static bool                 IsCompatibleWith                   (const FeatureDir&                   pi_rFeatureDir);



    static Ptr                  CreateFrom                         (UniformFeatureDir*                  pi_rpFeatureDir);
    static Ptr                  CreateFrom                         (MixedFeatureDir*                    pi_rpFeatureDir);
    static CPtr                 CreateFrom                         (const UniformFeatureDir*            pi_rpFeatureDir);
    static CPtr                 CreateFrom                         (const MixedFeatureDir*              pi_rpFeatureDir);


    const FeatureDir&           GetDir                             () const {
        return *m_pFeatureDir;
        }
    FeatureDir&                 GetDir                             () {
        return *m_pFeatureDir;
        }

    TileCIter                   TilesBegin                         () const;
    TileCIter                   TilesEnd                           () const;

    TileIter                    TilesBeginEdit                     ();
    TileIter                    TilesEndEdit                       ();

    // TDORAY: Make available GetTile methods instead?? How would we check that the requested tile exist == TileIterEnd? Would
    //         that be efficient?

    bool                        GetFeatures                        (TileID                              pi_ID,
                                                                    Array&                              po_rFeatures) const;


    bool                        SetFeatures                        (TileID                              pi_ID,
                                                                    const Array&                        pi_rFeatures);


    bool                        AddFeatures                        (TileID&                             po_rID,
                                                                    const Array&                        pi_rFeatures);

    bool                        RemoveFeatures                     (TileID                              pi_ID);


protected:

    explicit                    FeatureTileHandler                 (FeatureDir*                         pi_rpFeatureDir,
                                                                    typename const HeaderHandler::Ptr&  pi_rpHeaderTileHandler,
                                                                    typename const PointHandler::Ptr&   pi_rpPointTileHandler);


private:
    template <typename DirT>
    static FeatureTileHandler*  CreateFromImpl                     (DirT*                               pi_rpDir);

    virtual bool                _Save                              () override { return true; }
    virtual bool                _Load                              () override { return true; }

    FeatureDir*                 m_pFeatureDir;
    typename HeaderHandler::Ptr m_pHeaderTileHandler;
    typename PointHandler::Ptr  m_pPointTileHandler;
    };


#include "IDTMFeatureTileHandler.hpp"

} //End namespace IDTMFile


