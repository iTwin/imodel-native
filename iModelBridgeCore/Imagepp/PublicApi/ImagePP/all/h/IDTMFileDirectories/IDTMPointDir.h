//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMPointDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <Imagepp/all/h/HTGFFDirectory.h>
#include <Imagepp/all/h/IDTMFileDirectories/PointTypes.h>

#include <Imagepp/all/h/HTGFFSubDirHelpers.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMMetadataDir.h>

#include <Imagepp/all/h/HTGFFSubDirHelpers.h>
#include <Imagepp/all/h/HPUArray.h>

namespace HTGFF {
class DataType;
class Compression;

} // END namespace HTGFF


namespace IDTMFile {

class FeatureDir;


_HDLLg size_t                   GetTypeSize                        (PointTypeID                 pi_Type);
_HDLLg const HTGFF::DataType&   GetTypeDescriptor                  (PointTypeID                 pi_Type);


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
    _HDLLg static uint32_t      s_GetVersion                       ();

    _HDLLg PointTypeID          GetPointType                       () const;
    _HDLLg const HTGFF::Compression&
                                GetCompressType                    () const;

    _HDLLg double               GetCompressionRatio                () const;

    /*---------------------------------------------------------------------------------**//**
    * Global directory information accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg size_t               CountTiles                         () const;

    /*---------------------------------------------------------------------------------**//**
    * Spatial information
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg size_t               GetTileMaxPointCount               () const;
    _HDLLg uint64_t             CountPoints                        () const;
    _HDLLg size_t               CountPoints                        (TileID                  pi_ID) const;

    _HDLLg const Extent3d64f&   GetExtent                          (TileID                  pi_ID) const;
    _HDLLg Extent3d64f&         EditExtent                         (TileID                  pi_ID);

    _HDLLg ResolutionID         GetResolution                      (TileID                  pi_ID) const;
    _HDLLg void                 SetResolution                      (TileID                  pi_ID,
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
    _HDLLg bool                 GetPoints                          (TileID                  pi_ID,
                                                                    Packet&                 po_rPoints) const;
    _HDLLg bool                 SetPoints                          (TileID                  pi_ID,
                                                                    const Packet&           pi_rPoints);
    _HDLLg bool                 AddPoints                          (TileID&                 po_rID,
                                                                    const Packet&           pi_rPoints);
    _HDLLg bool                 RemovePoints                       (TileID                  pi_ID);


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