//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeatureDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFilteringDir.h>

namespace IDTMFile {

class HeaderDir;

/*---------------------------------------------------------------------------------**//**
* @description  Directory that stores indexed features. Attributes such as global extent,
*               tile extent, tile content extent, tile resolution, tile count and feature
*               count could be queried via this interface. For information about the
*               spatial index, use the spatial index directory. For information about
*               the filtering process, use the filtering directory.
*
*               User can query for the nature of stored data. Directories of this type
*               can store features of uniform/mixed type. User can also query whether
*               data is only simple points or complex features.
*
*
*               NTERAY: We can probably find a way to regroup tiles per resolution. Alain
*                       would have to specify the resolution ID with the tile ID when
*                       storing or reading data. Maybe resolution could be part of the
*                       id ??. Is that possible at his level?? We would only need
*                       to refer to underlying resolution directory. It would
*                       greatly enhance our capacity to store multi resolution data (not
*                       quad tree).
*
* @see          UniformFeatureDir
* @see          MixedFeatureDir
* @see          FeatureTileHandler
* @see          SpatialIndexDir
* @see          FilteringDir
* @see          PointDir
* @see          MetadataDir
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class FeatureDir : public HTGFF::Directory
    {
public:
     static uint32_t          s_GetVersion                       ();

    virtual                         ~FeatureDir                        () = 0;

     bool                     IsPointOnly                        () const;
     bool                     IsUniform                          () const;

    /*---------------------------------------------------------------------------------**//**
    * Sub directories
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool                     HasMetadataDir                     () const;
     const PrimitiveMetadataDir* 
                                    GetMetadataDir                     () const;
     PrimitiveMetadataDir*    GetMetadataDir                     ();
     PrimitiveMetadataDir*    CreateMetadataDir                  ();

    /*---------------------------------------------------------------------------------**//**
    * Spatial index accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool                     HasSpatialIndexDir                 () const;
     const SpatialIndexDir*   GetSpatialIndexDir                 () const;
     SpatialIndexDir*         GetSpatialIndexDir                 ();
     SpatialIndexDir*         CreateSpatialIndexDir              (const SpatialIndexDir::Options& 
                                                                        pi_rOptions);


    /*---------------------------------------------------------------------------------**//**
    * Filtering accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool                     HasFilteringDir                    () const;
     const FilteringDir*      GetFilteringDir                    () const;
     FilteringDir*            GetFilteringDir                    ();
     FilteringDir*            CreateFilteringDir                 (const FilteringDir::Options&
                                                                        pi_rOptions);

    /*---------------------------------------------------------------------------------**//**
    * Spatial information -> aliases for underlying point dir version
    +---------------+---------------+---------------+---------------+---------------+------*/

    size_t                          GetTileMaxPointCount               () const;
     uint64_t                 CountPoints                        () const;
     size_t                   CountPoints                        (TileID                  pi_ID) const;

     const Extent3d64f&       GetExtent                          (TileID                  pi_ID) const;
     Extent3d64f&             EditExtent                         (TileID                  pi_ID);

     ResolutionID             GetResolution                      (TileID                  pi_ID) const;
     void                     SetResolution                      (TileID                  pi_ID,
                                                                        ResolutionID            pi_Resolution);

    /*---------------------------------------------------------------------------------**//**
    * Tiles data accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     PointTypeID              GetPointType                       () const;
     FeatureHeaderTypeID      GetHeaderType                      () const;

     const HTGFF::Compression&
                                    GetPointCompressType               () const;
     size_t                   CountTiles                         () const;

     size_t                   GetTileMaxFeatureCount             () const;
     uint64_t                 CountFeatures                      () const;
     size_t                   CountFeatures                      (TileID                  pi_ID) const;



protected:
    typedef FeatureHeaderDir        HeaderDir;

    explicit                        FeatureDir                         ();

     PointDir&                GetPointDir                        () const;

     bool                     HasHeaderDir                       () const;
     HeaderDir&               GetHeaderDir                       () const;

     PointDir*                GetPointDirP                       () const;
     HeaderDir*               GetHeaderDirP                      () const;

    bool                            CreatePointOnlyFeatureHeaderDir    (HeaderDir*&         po_rpHeaderDir,
                                                                        FeatureType         pi_FeatureType,
                                                                        const CreateConfig& pi_rCreateConfig);
    bool                            LoadPointOnlyFeatureHeaderDir      (HeaderDir*&         po_rpHeaderDir,
                                                                        FeatureType         pi_FeatureType);

    bool                            CreateLinearFeatureHeaderDir       (HeaderDir*&         po_rpHeaderDir,
                                                                        const CreateConfig& pi_rCreateConfig);
    bool                            LoadLinearFeatureHeaderDir         (HeaderDir*&         po_rpHeaderDir);

    bool                            DefaultCreateFeatureHeaderDir      (HeaderDir*&         po_rpHeaderDir,
                                                                        const CreateConfig& pi_rCreateConfig);
    bool                            DefaultLoadFeatureHeaderDir        (HeaderDir*&         po_rpHeaderDir);

private:
    template <typename PointType, typename HeaderT>
    friend class                    FeatureTileHandler;
    template <typename HeaderT>
    friend class                    FeatureHeaderTileHandler;
    friend class                    PointPacketHandler;
    friend class                    FeatureHeaderPacketHandler;
    friend class                    FeaturePacketHandler;

    friend class                    HTGFF::Directory;

    // TDORAY: Transfer directory inheritance to pImpl??

    virtual bool                    _IsUniform                         () const = 0;

    virtual bool                    _CreateFeatureHeaderDir            (HeaderDir*&                 po_rpHeaderDir,
                                                                        const CreateConfig&         pi_rCreateConfig,
                                                                        const UserOptions*          pi_pUserOptions) = 0;
    virtual bool                    _LoadFeatureHeaderDir              (HeaderDir*&                 po_rpHeaderDir,
                                                                        const UserOptions*          pi_pUserOptions) = 0;

    virtual bool                    _Create                            (const CreateConfig&         pi_rCreateConfig,
                                                                        const UserOptions*          pi_pUserOptions) override;
    virtual bool                    _Load                              (const UserOptions*          pi_pUserOptions) override;


    struct                          Impl;
    std::auto_ptr<Impl>             m_pImpl; // Reserve some space for further use as this is a base class.
    HeaderDir*                      m_pHeaderDir;
    PointDir*                       m_pPointDir;
    };


} //End namespace IDTMFile

#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.hpp>
