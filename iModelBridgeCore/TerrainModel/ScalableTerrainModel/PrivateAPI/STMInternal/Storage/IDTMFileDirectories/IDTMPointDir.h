//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/IDTMFileDirectories/PointTypes.h>

#include <STMInternal/Storage/HTGFFSubDirHelpers.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h>

#include <STMInternal/Storage/HTGFFSubDirHelpers.h>
#include <STMInternal/Storage/HPUArray.h>

namespace HTGFF {
class DataType;
class Compression;

} // END namespace HTGFF


namespace IDTMFile {

class FeatureDir;


 size_t                   GetTypeSize                        (PointTypeID                 pi_Type);
 const HTGFF::DataType&   GetTypeDescriptor                  (PointTypeID                 pi_Type);


/*---------------------------------------------------------------------------------**//**
* @description  Directory used to store mass points data indexed per tile. Also store
*               attributes about this data and its organization.
*
* @see          PointTileHandler
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PointDir : public HTGFF::Directory
    {
public:
     static uint32_t      s_GetVersion                       ();

     PointTypeID          GetPointType                       () const;
     const HTGFF::Compression&
                                GetCompressType                    () const;

     double               GetCompressionRatio                () const;

    /*---------------------------------------------------------------------------------**//**
    * Global directory information accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     size_t               CountTiles                         () const;

    /*---------------------------------------------------------------------------------**//**
    * Spatial information
    +---------------+---------------+---------------+---------------+---------------+------*/
     size_t               GetTileMaxPointCount               () const;
     uint64_t             CountPoints                        () const;
     size_t               CountPoints                        (TileID                  pi_ID) const;

     const Extent3d64f&   GetExtent                          (TileID                  pi_ID) const;
     Extent3d64f&         EditExtent                         (TileID                  pi_ID);

     ResolutionID         GetResolution                      (TileID                  pi_ID) const;
     void                 SetResolution                      (TileID                  pi_ID,
                                                                    ResolutionID            pi_Resolution);

    explicit                    PointDir                           ();      // Should be private, Android problem.

protected:
    friend class                HTGFF::Directory;

    template <typename PointType>
    friend class                PointTileHandler;
    template <typename PointType, typename HeaderT>
    friend class                FeatureTileHandler;
    template <typename HeaderT>
    friend class                FeatureHeaderTileHandler;

    friend class                PointPacketHandler;

    /*---------------------------------------------------------------------------------**//**
    * Untyped data accessors. Only available through PointTileHandler.
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool                 GetPoints                          (TileID                  pi_ID,
                                                                    Packet&                 po_rPoints) const;
     bool                 SetPoints                          (TileID                  pi_ID,
                                                                    const Packet&           pi_rPoints);
     bool                 AddPoints                          (TileID&                 po_rID,
                                                                    const Packet&           pi_rPoints);
     bool                 RemovePoints                       (TileID                  pi_ID);


    // TDORAY: Transfer directory inheritance to pImpl??

    virtual bool                _Create                            (const CreateConfig&     pi_rCreateConfig,
                                                                    const UserOptions*      pi_pUserOptions) override;
    virtual bool                _Load                              (const UserOptions*      pi_pUserOptions) override;
    virtual bool                _Save                              () override;


    typedef HTGFF::AttributeSubDir<Extent3d64f>
                                ExtentsDir;
    typedef HTGFF::AttributeSubDir<ResolutionID>
                                ResolutionsDir;

    ExtentsDir*                 m_pExtentsDir;
    ResolutionsDir*             m_pResolutionsDir;

    PointTypeID                 m_PointType;
    size_t                      m_PointTypeSize;
    };

} //End namespace IDTMFile