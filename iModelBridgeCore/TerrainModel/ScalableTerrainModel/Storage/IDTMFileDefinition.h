//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDefinition.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFFile.h>
#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/HTGFFTagFile.h>

/*
 * Forward declarations
 */
namespace HTGFF {

class DataType;

} // END namespace HTGFF




namespace IDTMFile {

enum AttributeIdentifier
    {
    // TDORAY: Try to eliminate need for these...
    // Legacy HTIFF file tags. Not used.
    IDTM_ATTRIBUTEID_HTIFF_SUB_FILE_TYPE,
    IDTM_ATTRIBUTEID_HTIFF_SYNCRONIZATION_FIELD,
    IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V1,
    IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V2,
    IDTM_ATTRIBUTEID_HTIFF_HMR_DECIMATION_METHOD,
    IDTM_ATTRIBUTEID_HTIFF_GEOKEYDIRECTORY,
    IDTM_ATTRIBUTEID_HTIFF_GEODOUBLEPARAMS,
    IDTM_ATTRIBUTEID_HTIFF_GEOASCIIPARAMS,

    // HTGFF reserved tags
    // DTM tile indexing
    IDTM_ATTRIBUTEID_HTGFF_PACKET_OFFSETS,
    IDTM_ATTRIBUTEID_HTGFF_PACKET_BYTECOUNTS,
    IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_OFFSETS,
    IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_BYTECOUNTS,


    // Directory attributes
    IDTM_ATTRIBUTEID_HTGFF_VERSION,
    IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_TYPE,
    IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_ROLE,
    IDTM_ATTRIBUTEID_HTGFF_DATA_COMPRESS_TYPE,
    IDTM_ATTRIBUTEID_HTGFF_PACKET_COUNT,
    IDTM_ATTRIBUTEID_HTGFF_DATA_SIZE,
    IDTM_ATTRIBUTEID_HTGFF_UNCOMPRESSED_DATA_SIZE,
    IDTM_ATTRIBUTEID_HTGFF_MAX_PACKET_SIZE,
    IDTM_ATTRIBUTEID_HTGFF_MAX_UNCOMPRESSED_PACKET_SIZE,
    IDTM_ATTRIBUTEID_HTGFF_SUBDIRECTORY_LISTING,
    IDTM_ATTRIBUTEID_HTGFF_REMOVED_DIRECTORY_LISTING,

    // Root directory attributes
    IDTM_DIRECTORYID_ROOTDIR_METADATA_SUBDIR,
    IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS,
    IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR,

    // Layer directory attributes
    IDTM_DIRECTORYID_LAYERDIR_METADATA_SUBDIR,
    IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
    IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS,
    IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM,

    // Metadata directory attributes
    IDTM_ATTRIBUTEID_METADATADIR_SCHEMA_TYPE,
    IDTM_ATTRIBUTEID_METADATADIR_SCHEMA,
    IDTM_ATTRIBUTEID_METADATADIR_METADATA,

    // Feature directory attributes
    IDTM_DIRECTORYID_FEATUREDIR_METADATA_SUBDIR,
    IDTM_DIRECTORYID_FEATUREDIR_POINT_SUBDIR,
    IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR,
    IDTM_DIRECTORYID_FEATUREDIR_SPATIAL_INDEX_SUBDIR,
    IDTM_DIRECTORYID_FEATUREDIR_FILTERING_SUBDIR,

    // Points directory attributes
    IDTM_DIRECTORYID_POINTDIR_TILES_3D_EXTENT_SUBDIR,
    IDTM_DIRECTORYID_POINTDIR_TILES_RESOLUTION_SUBDIR,
    IDTM_ATTRIBUTEID_POINTDIR_EMPTY_SLOT_0,

    // Feature header directory attributes

    // Spatial index directory attributes
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_EMPTY_SLOT_0_SUBDIR, 
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR,
    IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TYPE,
    IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TOPNODEID,
    IDTM_ATTRIBUTEID_SPATIALINDEXDIR_SPLIT_TRESHOLD,
    IDTM_ATTRIBUTEID_SPATIALINDEXDIR_BALANCED,
    IDTM_ATTRIBUTEID_SPATIALINDEXDIR_PROGRESSIVE,

    // Filtering directory attributes
    IDTM_ATTRIBUTEID_FILTERINGDIR_TYPE,
    IDTM_DIRECTORYID_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR,

    // Source node directory attributes
    IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS,
    IDTM_DIRECTORYID_SOURCENODEDIR_SOURCES_SUBDIR,
    IDTM_ATTRIBUTEID_SOURCENODEDIR_LAST_MODIFIED_TIME,

    // Source directory attributes
    IDTM_ATTRIBUTEID_SOURCESDIR_LAST_MODIFIED_CHECK_TIME,
    IDTM_ATTRIBUTEID_SOURCESDIR_LAST_SYNC_TIME,
    IDTM_ATTRIBUTEID_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION,
    IDTM_ATTRIBUTEID_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION,
    IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION,
    IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION,

    // Source sequence directory attributes
    IDTM_DIRECTORYID_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR,
    IDTM_DIRECTORYID_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR,
    IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR,
    IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR,
    IDTM_ATTRIBUTEID_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS,

    // Mesh directory attributes
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR,
    IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR,
    // End of attributes
    IDTM_ATTRIBUTEID_END,
    };


/*---------------------------------------------------------------------------------**//**
* @description  Class defining the specific tag informations required by HTGFFFile to
*               work.
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class AttributeInfo : public HTGFF::FileDefinition::TagInfo
    {
public:

    virtual HTagID                  GetFreeOffsetsTagID                () const override {
        return IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_OFFSETS;
    }
    virtual HTagID                  GetFreeByteCountsTagID             () const override {
        return IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_BYTECOUNTS;
    }

    virtual HTagID                  GetPacketOffsetsTagID              () const override {
        return IDTM_ATTRIBUTEID_HTGFF_PACKET_OFFSETS;
    }
    virtual HTagID                  GetPacketByteCountsTagID           () const override {
        return IDTM_ATTRIBUTEID_HTGFF_PACKET_BYTECOUNTS;
    }

    virtual HTagID                  GetVersionTagID                    () const override {
        return IDTM_ATTRIBUTEID_HTGFF_VERSION;
    }

    virtual HTagID                  GetDataDimensionsTypeTagID         () const override {
        return IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_TYPE;
    }
    virtual HTagID                  GetDataDimensionsRoleTagID         () const override {
        return IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_ROLE;
    }
    virtual HTagID                  GetDataCompressTypeTagID           () const override {
        return IDTM_ATTRIBUTEID_HTGFF_DATA_COMPRESS_TYPE;
    }

    virtual HTagID                  GetPacketCountTagID                () const override {
        return IDTM_ATTRIBUTEID_HTGFF_PACKET_COUNT;
    }
    virtual HTagID                  GetDataSizeTagID                   () const override {
        return IDTM_ATTRIBUTEID_HTGFF_DATA_SIZE;
    }
    virtual HTagID                  GetUncompressedDataSizeTagID       () const override {
        return IDTM_ATTRIBUTEID_HTGFF_UNCOMPRESSED_DATA_SIZE;
    }
    virtual HTagID                  GetMaxPacketSizeTagID              () const override {
        return IDTM_ATTRIBUTEID_HTGFF_MAX_PACKET_SIZE;
    }
    virtual HTagID                  GetMaxUncompressedPacketSizeTagID  () const override {
        return IDTM_ATTRIBUTEID_HTGFF_MAX_UNCOMPRESSED_PACKET_SIZE;
    }

    virtual HTagID                  GetSubDirectoryListingTagID        () const override {
        return IDTM_ATTRIBUTEID_HTGFF_SUBDIRECTORY_LISTING;
    }
    virtual HTagID                  GetRemovedDirectoryListingTagID    () const override {
        return IDTM_ATTRIBUTEID_HTGFF_REMOVED_DIRECTORY_LISTING;
    }

    virtual HTagID                  GetSubFileTypeTagID                () const override {
        return IDTM_ATTRIBUTEID_HTIFF_SUB_FILE_TYPE;
    }
    virtual HTagID                  GetHMRSyncronizationTagID          () const override {
        return IDTM_ATTRIBUTEID_HTIFF_SYNCRONIZATION_FIELD;
    }
    virtual HTagID                  GetHMRDirectoryV1TagID             () const override {
        return IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V1;
    }
    virtual HTagID                  GetHMRDirectoryV2TagID             () const override {
        return IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V2;
    }

    virtual HTagID                  GetHMRDecimationMethodTagID        () const override {
        return IDTM_ATTRIBUTEID_HTIFF_HMR_DECIMATION_METHOD;
    }

    virtual HTagID                  GetGeoKeyDirectoryTagID            () const override {
        return IDTM_ATTRIBUTEID_HTIFF_GEOKEYDIRECTORY;
    }
    virtual HTagID                  GetGeoDoubleParamsTagID            () const override {
        return IDTM_ATTRIBUTEID_HTIFF_GEODOUBLEPARAMS;
    }
    virtual HTagID                  GetGeoAsciiParamsTagID             () const override {
        return IDTM_ATTRIBUTEID_HTIFF_GEOASCIIPARAMS;
    }

    virtual HTagID                  GetNotSavedTagIDBegin              () const override {
        return IDTM_ATTRIBUTEID_END;
    }
    virtual uint32_t                GetTagQty                          () const override {
        return IDTM_ATTRIBUTEID_END;
    }

    virtual size_t                  GetTagDefinitionQty                () const override {
        return sDefinitionQty;
    }
    virtual const Info*             GetTagDefinitionArray              () const override {
        return sDefinitionArray;
    }

private:
    static const Info               sDefinitionArray[];
    static const size_t             sDefinitionQty;
    };

/*---------------------------------------------------------------------------------**//**
* @description  Class defining the specific file informations required by HTGFFFile to
*               work.
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class FileDefinition : public HTGFF::FileDefinition
    {
public:
    /*---------------------------------------------------------------------------------**//**
    * @description  Returns the magic number for a little endian file
    * @bsimethod                                                  Raymond.Gauthier   5/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual unsigned short GetLittleEndianMagicNumber () const override
    {
        return 0x4d54; // ASCII -> "MT" (for T.M. (Terrain model) abreviation when reversed from natural order -> little endian)
    }

    /*---------------------------------------------------------------------------------**//**
    * @description  Returns the magic number for a big endian file
    * @bsimethod                                                  Raymond.Gauthier   5/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual unsigned short GetBigEndianMagicNumber () const override
    {
        return 0x544d; // ASCII -> "TM" (for T.M. (Terrain model) abreviation
    }

    virtual const TagInfo&  GetTagInfo                     () const override
    {
        static const AttributeInfo IDTM_ATTRIBUTE_INFO;
        return IDTM_ATTRIBUTE_INFO;
    }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static struct FileDefault
    {
    explicit                        FileDefault            ();

    const NodeID                    topNode;
    const ResolutionID              topTileResolution;

    const Extent2d64f               extent2d;
    const Extent3d64f               extent3d;

    const ResolutionID              resolution;
    const bool                      filtered;

    } IDTM_DEFAULT;



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ExtentTypeDef
    {
    enum DimensionRole
        {
        DR_X_MIN,
        DR_X_MAX,
        DR_Y_MIN,
        DR_Y_MAX,
        DR_Z_MIN,
        DR_Z_MAX,
        DR_QTY,
        };

public:
    static const HTGFF::DataType&   GetType                ();
    };




} //End namespace IDTMFile