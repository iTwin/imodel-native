//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDefinition.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFDataDescriptor.h>

#include "IDTMFileDefinition.h"

#define ARRAY_SIZE(Array) (sizeof(Array)/sizeof(Array[0]))
#define ARRAY_BEGIN(Array) (Array)
#define ARRAY_END(Array) ((Array) + ARRAY_SIZE(Array))

namespace IDTMFile {

enum IDTMTagID
    {
    // Legacy HTIFF file tags (Not used)            -> Reserved 0 - 1000
    IDTMTAG_HTIFF_SUB_FILE_TYPE           = 0,
    IDTMTAG_HTIFF_SYNCRONIZATION_FIELD,
    IDTMTAG_HTIFF_HMRDIR_V1,
    IDTMTAG_HTIFF_HMRDIR_V2,
    IDTMTAG_HTIFF_HMR_DECIMATION_METHOD,
    IDTMTAG_HTIFF_GEOKEYDIRECTORY,
    IDTMTAG_HTIFF_GEODOUBLEPARAMS,
    IDTMTAG_HTIFF_GEOASCIIPARAMS,


    // TDORAY: Reserve 2000 range for these tags at the beginning
    // HTGFF reserved tags                          -> Reserved 1000 - 2000
    // DTM tile indexing
    IDTMTAG_HTGFF_SUB_DIRS_BEGIN = 1000,

    IDTMTAG_HTGFF_ATTRIBUTES_BEGIN = 1100,
    IDTMTAG_HTGFF_PACKET_OFFSETS         = IDTMTAG_HTGFF_ATTRIBUTES_BEGIN,
    IDTMTAG_HTGFF_PACKET_BYTECOUNTS,
    IDTMTAG_HTGFF_FREE_PACKET_OFFSETS,
    IDTMTAG_HTGFF_FREE_PACKET_BYTECOUNTS,


    // Directory attributes
    IDTMTAG_HTGFF_VERSION,
    IDTMTAG_HTGFF_DATA_DIMENSIONS_TYPE,
    IDTMTAG_HTGFF_DATA_DIMENSIONS_ROLE,
    IDTMTAG_HTGFF_DATA_COMPRESS_TYPE,
    IDTMTAG_HTGFF_PACKET_COUNT,
    IDTMTAG_HTGFF_DATA_SIZE,
    IDTMTAG_HTGFF_UNCOMPRESSED_DATA_SIZE,
    IDTMTAG_HTGFF_MAX_PACKET_SIZE,
    IDTMTAG_HTGFF_MAX_UNCOMPRESSED_PACKET_SIZE,
    IDTMTAG_HTGFF_SUB_DIRECTORY_LISTING,
    IDTMTAG_HTGFF_REMOVED_DIRECTORY_LISTING,


    // Root directory attributes                    -> Reserved 2000 - 3000
    IDTMTAG_ROOTDIR_SUB_DIRS_BEGIN = 2000,
    IDTMTAG_ROOTDIR_METADATA_SUBDIR = IDTMTAG_ROOTDIR_SUB_DIRS_BEGIN,
    IDTMTAG_ROOTDIR_LAYER_SUBDIRS,
    IDTMTAG_ROOTDIR_SOURCES_SUBDIR,

    IDTMTAG_ROOTDIR_ATTRIBUTES_BEGIN = 2100,


    // Layer directory attributes                   -> Reserved 3000 - 4000
    IDTMTAG_LAYERDIR_SUB_DIRS_BEGIN = 3000,
    IDTMTAG_LAYERDIR_METADATA_SUBDIR = IDTMTAG_LAYERDIR_SUB_DIRS_BEGIN,
    IDTMTAG_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
    IDTMTAG_LAYERDIR_MIXED_FEATURE_SUBDIRS,

    IDTMTAG_LAYERDIR_ATTRIBUTES_BEGIN = 3100,
    IDTMTAG_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM = IDTMTAG_LAYERDIR_ATTRIBUTES_BEGIN,


    // Metadata directory attributes                -> Reserved 4000 - 5000
    IDTMTAG_METADATADIR_SUB_DIRS_BEGIN = 4000,

    IDTMTAG_METADATADIR_ATTRIBUTES_BEGIN = 4100,
    IDTMTAG_METADATADIR_SCHEMA_TYPE = IDTMTAG_METADATADIR_ATTRIBUTES_BEGIN,
    IDTMTAG_METADATADIR_SCHEMA,
    IDTMTAG_METADATADIR_METADATA,


    // Feature directory attributes                 -> Reserved 5000 - 6000
    IDTMTAG_FEATUREDIR_SUB_DIRS_BEGIN = 5000,
    IDTMTAG_FEATUREDIR_METADATA_SUBDIR = IDTMTAG_FEATUREDIR_SUB_DIRS_BEGIN,
    IDTMTAG_FEATUREDIR_POINT_SUBDIR,
    IDTMTAG_FEATUREDIR_HEADER_SUBDIR,
    IDTMTAG_FEATUREDIR_SPATIAL_INDEX_SUBDIR,
    IDTMTAG_FEATUREDIR_FILTERING_SUBDIR,

    IDTMTAG_FEATUREDIR_ATTRIBUTES_BEGIN = 5100,


    // Points directory attributes                  -> Reserved 6000 - 7000
    IDTMTAG_POINTDIR_SUB_DIRS_BEGIN = 6000,
    IDTMTAG_POINTDIR_TILES_3D_EXTENT_SUBDIR = IDTMTAG_POINTDIR_SUB_DIRS_BEGIN,
    IDTMTAG_POINTDIR_TILES_RESOLUTION_SUBDIR,

    IDTMTAG_POINTDIR_ATTRIBUTES_BEGIN = 6100,
    IDTMTAG_POINTDIR_EMPTY_SLOT_0 = IDTMTAG_POINTDIR_ATTRIBUTES_BEGIN,


    // Feature header directory attributes          -> Reserved 7000 - 8000
    IDTMTAG_FEATUREHEADERDIR_SUB_DIRS_BEGIN = 7000,
    IDTMTAG_FEATUREHEADERDIR_ATTRIBUTES_BEGIN = 7100,


    // Spatial index directory attributes           -> Reserved 8000 - 9000
    IDTMTAG_SPATIALINDEXDIR_SUB_DIRS_BEGIN = 8000,
    IDTMTAG_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR = IDTMTAG_SPATIALINDEXDIR_SUB_DIRS_BEGIN,
    IDTMTAG_SPATIALINDEXDIR_EMPTY_SLOT_0_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR,	
	IDTMTAG_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR,
    IDTMTAG_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR,

    IDTMTAG_SPATIALINDEXDIR_ATTRIBUTES_BEGIN = 8100,
    IDTMTAG_SPATIALINDEXDIR_TYPE = IDTMTAG_SPATIALINDEXDIR_ATTRIBUTES_BEGIN,
    IDTMTAG_SPATIALINDEXDIR_TOPNODEID,
    IDTMTAG_SPATIALINDEXDIR_SPLIT_TRESHOLD,
    IDTMTAG_SPATIALINDEXDIR_BALANCED,
    IDTMTAG_SPATIALINDEXDIR_PROGRESSIVE,


    // Filtering directory attributes               -> Reserved 9000 - 10000
    IDTMTAG_FILTERINGDIR_SUB_DIRS_BEGIN = 9000,
    IDTMTAG_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR = IDTMTAG_FILTERINGDIR_SUB_DIRS_BEGIN,

    IDTMTAG_FILTERINGDIR_ATTRIBUTES_BEGIN = 9100,
    IDTMTAG_FILTERINGDIR_TYPE = IDTMTAG_FILTERINGDIR_ATTRIBUTES_BEGIN,


    // Source node directory attributes             -> Reserved 10000 - 10500
    IDTMTAG_SOURCENODEDIR_SUB_DIRS_BEGIN = 10000,
    IDTMTAG_SOURCENODEDIR_NODE_SUBDIRS = IDTMTAG_SOURCENODEDIR_SUB_DIRS_BEGIN,
    IDTMTAG_SOURCENODEDIR_SOURCES_SUBDIR,

    IDTMTAG_SOURCENODEDIR_ATTRIBUTES_BEGIN = 10100,
    IDTMTAG_SOURCENODEDIR_LAST_MODIFIED_TIME = IDTMTAG_SOURCENODEDIR_ATTRIBUTES_BEGIN,

    // Source directory attributes               -> Reserved 10500 - 11000
    IDTMTAG_SOURCESDIR_SUB_DIRS_BEGIN = 10500,

    IDTMTAG_SOURCESDIR_ATTRIBUTES_BEGIN = 10600,
    IDTMTAG_SOURCESDIR_LAST_MODIFIED_CHECK_TIME = IDTMTAG_SOURCESDIR_ATTRIBUTES_BEGIN,
    IDTMTAG_SOURCESDIR_LAST_SYNC_TIME,
    IDTMTAG_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION,
    IDTMTAG_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION,
    IDTMTAG_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION,
    IDTMTAG_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION,


    // Source sequence directory attributes               -> Reserved 11000 - 12000
    IDTMTAG_SOURCESEQUENCEDIR_SUB_DIRS_BEGIN = 11000,
    IDTMTAG_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR = IDTMTAG_SOURCESEQUENCEDIR_SUB_DIRS_BEGIN,
    IDTMTAG_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR,
    IDTMTAG_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR,
    IDTMTAG_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR,

    IDTMTAG_SOURCESEQUENCEDIR_ATTRIBUTES_BEGIN = 11100,
    IDTMTAG_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS = IDTMTAG_SOURCESEQUENCEDIR_ATTRIBUTES_BEGIN,


    };


//
// NB: THIS ARRAY IS ASSUMED TO BE SORTED BY TAG.
//
// If you add a Tag in this list, you must update the Enum in the
// file HTIFFTag.h. (the sequence must be keep)
//

const AttributeInfo::Info AttributeInfo::sDefinitionArray[] =
    {
//------------------ Legacy HTIFF attributes ----------------------------------------
    // TDORAY: Try to eliminate need for this...
    // Legacy HTIFF file tags. Not used.
        {   IDTMTAG_HTIFF_SUB_FILE_TYPE, IDTM_ATTRIBUTEID_HTIFF_SUB_FILE_TYPE, 1,  1,
        LONG,     true, false, "HTIFFSubfileType"
        },
        {   IDTMTAG_HTIFF_SYNCRONIZATION_FIELD, IDTM_ATTRIBUTEID_HTIFF_SYNCRONIZATION_FIELD, 1,  1,
        LONG,   false, false, "HTIFFSynchronizationField"
        },
        {   IDTMTAG_HTIFF_HMRDIR_V1, IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V1, 1,     1,
        LONG64,  true,  false, "HTIFFHMRDirectoryV1"
        },
        {   IDTMTAG_HTIFF_HMRDIR_V2, IDTM_ATTRIBUTEID_HTIFF_HMRDIR_V2, 1,     1,
        LONG64,  true,  false, "HTIFFHMRDirectoryV2"
        },
        {   IDTMTAG_HTIFF_HMR_DECIMATION_METHOD, IDTM_ATTRIBUTEID_HTIFF_HMR_DECIMATION_METHOD, 1, 1,
        LONG, true, false, "HTIFFHMRDecimationMethod"
        },
        {   IDTMTAG_HTIFF_GEOKEYDIRECTORY, IDTM_ATTRIBUTEID_HTIFF_GEOKEYDIRECTORY, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        SHORT, true,  true, "HTIFFGeoKeyDirectory"
        },
        {   IDTMTAG_HTIFF_GEODOUBLEPARAMS, IDTM_ATTRIBUTEID_HTIFF_GEODOUBLEPARAMS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        DOUBLE,true,  true, "HTIFFGeoDoubleParams"
        },
        {   IDTMTAG_HTIFF_GEOASCIIPARAMS, IDTM_ATTRIBUTEID_HTIFF_GEOASCIIPARAMS,   TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        ASCII, true,  false, "HTIFFGeoASCIIParams"
        },


//------------------ Attributes specific to HTGFFFile --------------------------------
    // HTGFF reserved tags
        {   IDTMTAG_HTGFF_PACKET_OFFSETS, IDTM_ATTRIBUTEID_HTGFF_PACKET_OFFSETS, TAG_IO_VARIABLE, 1,
        LONG64, false, false, "HTGFFPacketOffsets"
        },
        {   IDTMTAG_HTGFF_PACKET_BYTECOUNTS, IDTM_ATTRIBUTEID_HTGFF_PACKET_BYTECOUNTS, TAG_IO_VARIABLE, 1,
        LONG64, false, false, "HTGFFPacketByteCounts"
        },
        {   IDTMTAG_HTGFF_FREE_PACKET_OFFSETS, IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_OFFSETS, TAG_IO_VARIABLE,TAG_IO_VARIABLE,
        LONG64,   false,false, "HTGFFFreePacketOffsets"
        },
        {   IDTMTAG_HTGFF_FREE_PACKET_BYTECOUNTS, IDTM_ATTRIBUTEID_HTGFF_FREE_PACKET_BYTECOUNTS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG64,   false,false, "HTGFFFreePacketByteCounts"
        },


        {   IDTMTAG_HTGFF_VERSION, IDTM_ATTRIBUTEID_HTGFF_VERSION, 1, 1,
        LONG, true, false, "HTGFFVersion"
        },

    // The type of data stored in the directory
    // TDORAY: List
        {   IDTMTAG_HTGFF_DATA_DIMENSIONS_TYPE, IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_TYPE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "HTGFFDataDimensionsType"
        },

        {   IDTMTAG_HTGFF_DATA_DIMENSIONS_ROLE, IDTM_ATTRIBUTEID_HTGFF_DATA_DIMENSIONS_ROLE, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "HTGFFDataDimensionsRole"
        },

        {   IDTMTAG_HTGFF_DATA_COMPRESS_TYPE, IDTM_ATTRIBUTEID_HTGFF_DATA_COMPRESS_TYPE, 1, 1,
        LONG, true, false, "HTGFFDataCompressType"
        },

        {   IDTMTAG_HTGFF_PACKET_COUNT, IDTM_ATTRIBUTEID_HTGFF_PACKET_COUNT, 1, 1,
        LONG64, true, false, "HTGFFPacketCount"
        },
        {   IDTMTAG_HTGFF_DATA_SIZE, IDTM_ATTRIBUTEID_HTGFF_DATA_SIZE, 1, 1,
        LONG64, true, false, "HTGFFDataSize"
        },
        {   IDTMTAG_HTGFF_UNCOMPRESSED_DATA_SIZE, IDTM_ATTRIBUTEID_HTGFF_UNCOMPRESSED_DATA_SIZE, 1, 1,
        LONG64, true, false, "HTGFFUncompressedDataSize"
        },
        {   IDTMTAG_HTGFF_MAX_PACKET_SIZE, IDTM_ATTRIBUTEID_HTGFF_MAX_PACKET_SIZE, 1, 1,
        LONG64, true, false, "HTGFFMaxPacketSize"
        },
        {   IDTMTAG_HTGFF_MAX_UNCOMPRESSED_PACKET_SIZE, IDTM_ATTRIBUTEID_HTGFF_MAX_UNCOMPRESSED_PACKET_SIZE, 1, 1,
        LONG64, true, false, "HTGFFMaxUncompressedPacketSize"
        },
        {   IDTMTAG_HTGFF_SUB_DIRECTORY_LISTING, IDTM_ATTRIBUTEID_HTGFF_SUBDIRECTORY_LISTING, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "HTGFFSubDirListing"
        },
        {   IDTMTAG_HTGFF_REMOVED_DIRECTORY_LISTING, IDTM_ATTRIBUTEID_HTGFF_REMOVED_DIRECTORY_LISTING, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "HTGFFRemovedDirListing"
        },


//------------------- Tags for storing root directory specific attributes ---------
        {   IDTMTAG_ROOTDIR_METADATA_SUBDIR, IDTM_DIRECTORYID_ROOTDIR_METADATA_SUBDIR, 1, 1,
        LONG, true, false, "RootDirMetadataSubDir"
        },

        {   IDTMTAG_ROOTDIR_LAYER_SUBDIRS, IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "RootDirLayerSubDirs"
        },

        {   IDTMTAG_ROOTDIR_SOURCES_SUBDIR, IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR, 1, 1,
        LONG, true, false, "RootDirSourcesSubDir"
        },


//------------------- Tags for storing layer directory specific attributes ---------
        {   IDTMTAG_LAYERDIR_METADATA_SUBDIR, IDTM_DIRECTORYID_LAYERDIR_METADATA_SUBDIR, 1, 1,
        LONG, true, false, "LayerDirMetadataSubDir"
        },

        {   IDTMTAG_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "LayerDirUniformFeatureSubDirs"
        },

        {   IDTMTAG_LAYERDIR_MIXED_FEATURE_SUBDIRS, IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "LayerDirMixedFeatureSubDirs"
        },

        {   IDTMTAG_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM, IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        ASCIIW, true, false, "LayerDirWktSpatialReferenceSystem"
        },

//------------------- Tags for storing metadata directory specific attributes ---------
        {   IDTMTAG_METADATADIR_SCHEMA_TYPE, IDTM_ATTRIBUTEID_METADATADIR_SCHEMA_TYPE, 1, 1,
        LONG, true, false, "MetadataDirSchemaType"
        },

        {   IDTMTAG_METADATADIR_SCHEMA, IDTM_ATTRIBUTEID_METADATADIR_SCHEMA, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        BYTE, true, false, "MetadataDirSchema"
        },

        {   IDTMTAG_METADATADIR_METADATA, IDTM_ATTRIBUTEID_METADATADIR_METADATA, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        BYTE, true, false, "MetadataDirMetadata"
        },

//------------------ Attributes specific to features directory ---------------------------
        {   IDTMTAG_FEATUREDIR_METADATA_SUBDIR, IDTM_DIRECTORYID_FEATUREDIR_METADATA_SUBDIR, 1, 1,
        LONG, true, false, "FeatureDirMetadataSubDir"
        },

        {   IDTMTAG_FEATUREDIR_POINT_SUBDIR, IDTM_DIRECTORYID_FEATUREDIR_POINT_SUBDIR, 1, 1,
        LONG, true, false, "FeatureDirPointSubDir"
        },

        {   IDTMTAG_FEATUREDIR_HEADER_SUBDIR, IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR, 1, 1,
        LONG, true, false, "FeatureDirHeaderSubDir"
        },

        {   IDTMTAG_FEATUREDIR_SPATIAL_INDEX_SUBDIR, IDTM_DIRECTORYID_FEATUREDIR_SPATIAL_INDEX_SUBDIR, 1, 1,
        LONG, true, false, "FeatureDirSpatialIndexSubDir"
        },

        {   IDTMTAG_FEATUREDIR_FILTERING_SUBDIR, IDTM_DIRECTORYID_FEATUREDIR_FILTERING_SUBDIR, 1, 1,
        LONG, true, false, "FeatureDirFilteringSubDir"
        },

//------------------ Attributes specific to points directory ---------------------------

        {   IDTMTAG_POINTDIR_TILES_3D_EXTENT_SUBDIR, IDTM_DIRECTORYID_POINTDIR_TILES_3D_EXTENT_SUBDIR, 1, 1,
        LONG, true, false, "PointDirTiles3DExtentSubDir"
        },

        {   IDTMTAG_POINTDIR_TILES_RESOLUTION_SUBDIR, IDTM_DIRECTORYID_POINTDIR_TILES_RESOLUTION_SUBDIR, 1, 1,
        LONG, true, false, "PointDirTilesResolutionSubDir"
        },

        { IDTMTAG_POINTDIR_EMPTY_SLOT_0, IDTM_ATTRIBUTEID_POINTDIR_EMPTY_SLOT_0, 6, 6,
        DOUBLE, TRUE, FALSE, "PointDirContentExtent"
        },

//------------------ Attributes specific to feature header directory ---------------------------

//------------------ Attributes specific to spatial index directory ----------------------

    //Tile childs list. Count : 4 X number of tiles. Can be 0, 1 or 4 childs. MAX_ULONG indicates no childs.
        { IDTMTAG_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_SUBNODES_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesSubNodesSubDir"
        },


        { IDTMTAG_SPATIALINDEXDIR_EMPTY_SLOT_0_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_EMPTY_SLOT_0_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesViewDependantMetricsSubDir"
        },

        {   IDTMTAG_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesContent3DExtentSubDir"
        },

        {   IDTMTAG_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_STATISTICS_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesStatisticsSubDir"
        },


    //Spatial index type identificator, see IDTMSpatialIndexType
        {   IDTMTAG_SPATIALINDEXDIR_TYPE, IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TYPE, 1, 1,
        LONG, true, false, "SpatialIndexDirType"
        },

    // Store the ID of the DTM's lowest resolution tile (top/root tile)
        {   IDTMTAG_SPATIALINDEXDIR_TOPNODEID, IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TOPNODEID, 1, 1,
        LONG, true, false, "SpatialIndexDirTopNodeID"
        },

    // The quantity of points that triggers a node split
        {   IDTMTAG_SPATIALINDEXDIR_SPLIT_TRESHOLD, IDTM_ATTRIBUTEID_SPATIALINDEXDIR_SPLIT_TRESHOLD, 1, 1,
        LONG, true, false, "SpatialIndexDirSplitTreshold"
        },

    // Whether the stored tree is balanced or not (1(true)/0(false))
        {   IDTMTAG_SPATIALINDEXDIR_BALANCED, IDTM_ATTRIBUTEID_SPATIALINDEXDIR_BALANCED, 1, 1,
        LONG, true, false, "SpatialIndexDirBalanced"
        },

    // Whether the stored tree is balanced or not (1(true)/0(false))
        {   IDTMTAG_SPATIALINDEXDIR_PROGRESSIVE, IDTM_ATTRIBUTEID_SPATIALINDEXDIR_PROGRESSIVE, 1, 1,
        LONG, true, false, "SpatialIndexDirProgressive"
        },

//------------------ Attributes specific to filtering directory ----------------------

    //Tile filtered (is filtered). Count : number of tiles.
        {   IDTMTAG_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR, IDTM_DIRECTORYID_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR, 1, 1,
        LONG, true, false, "FilteringDirNodesFilteredStatusSubDir"
        },

    //Filter type identificator, see IDTMFilterType
        {   IDTMTAG_FILTERINGDIR_TYPE, IDTM_ATTRIBUTEID_FILTERINGDIR_TYPE, 1, 1,
        LONG, true, false, "FilteringDirType"
        },

//------------------ Attributes specific to source node directory ----------------------

        {   IDTMTAG_SOURCENODEDIR_NODE_SUBDIRS, IDTM_DIRECTORYID_SOURCENODEDIR_NODE_SUBDIRS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG, true, false, "SourceNodeDirNodeSubDirs"
        },

        {   IDTMTAG_SOURCENODEDIR_SOURCES_SUBDIR, IDTM_DIRECTORYID_SOURCENODEDIR_SOURCES_SUBDIR, 1, 1,
        LONG, true, false, "SourceNodeDirSourcesSubDir"
        },

        {   IDTMTAG_SOURCENODEDIR_LAST_MODIFIED_TIME, IDTM_ATTRIBUTEID_SOURCENODEDIR_LAST_MODIFIED_TIME, 1, 1,
        LONG64, true, false, "SourcesDirLastModifiedTime"
        },


//------------------ Attributes specific to sources directory ----------------------

        {   IDTMTAG_SOURCESDIR_LAST_MODIFIED_CHECK_TIME, IDTM_ATTRIBUTEID_SOURCESDIR_LAST_MODIFIED_CHECK_TIME, 1, 1,
        LONG64, true, false, "SourcesDirLastModifiedCheckTime"
        },

        {   IDTMTAG_SOURCESDIR_LAST_SYNC_TIME, IDTM_ATTRIBUTEID_SOURCESDIR_LAST_SYNC_TIME, 1, 1,
        LONG64, true, false, "SourcesDirLastSyncTime"
        },

    { IDTMTAG_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION, IDTM_ATTRIBUTEID_SOURCESDIR_SERIALIZED_SOURCE_FORMAT_VERSION, 1, 1,
      LONG, TRUE, FALSE, "SerializedSourceFormatVersion" },

    { IDTMTAG_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION, IDTM_ATTRIBUTEID_SOURCESDIR_CONTENT_CONFIG_FORMAT_VERSION, 1, 1,
      LONG, TRUE, FALSE, "ContentConfigFormatVersion" },

    { IDTMTAG_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION, IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_SEQUENCE_FORMAT_VERSION, 1, 1,
      LONG, TRUE, FALSE, "ImportSequenceFormatVersion" },

    { IDTMTAG_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION, IDTM_ATTRIBUTEID_SOURCESDIR_IMPORT_CONFIG_FORMAT_VERSION, 1, 1,
      LONG, TRUE, FALSE, "ImportConfigFormatVersion" },

//------------------ Attributes specific to source sequence directory ----------------------

        {   IDTMTAG_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR, IDTM_DIRECTORYID_SOURCESEQUENCEDIR_SERIALIZEDSOURCES_SUBDIR, 1, 1,
        LONG, true, false, "SourceSequenceDirMonikersSubDir"
        },

        {   IDTMTAG_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR, IDTM_DIRECTORYID_SOURCESEQUENCEDIR_CONTENTCONFIGS_SUBDIR, 1, 1,
        LONG, true, false, "SourceSequenceDirContentConfigsSubDir"
        },

        {   IDTMTAG_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR, IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTSEQUENCES_SUBDIR, 1, 1,
        LONG, true, false, "SourceSequenceDirImportSequencesSubDir"
        },

        {   IDTMTAG_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR, IDTM_DIRECTORYID_SOURCESEQUENCEDIR_IMPORTCONFIGS_SUBDIR, 1, 1,
        LONG, true, false, "SourceSequenceDirImportConfigsSubDir"
        },


    //Sources types identificators.
        {   IDTMTAG_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS, IDTM_ATTRIBUTEID_SOURCESEQUENCEDIR_LAST_MODIFIED_TIME_STAMPS, TAG_IO_VARIABLE, TAG_IO_VARIABLE,
        LONG64, true, false, "SourceSequenceDirLastModifiedTimeStamps"
        },




        { IDTMTAG_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_IDS_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesMeshIdsSubDir"
        },

        { IDTMTAG_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesNeighborNodesSubDir"
        },


        { IDTMTAG_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_NEIGHBORNODES_VARDATA_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesNeighborNodesVarDataSubDir"
        },

        { IDTMTAG_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_PARENTNODE_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesParentNodeSubDir"
        },

        { IDTMTAG_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesMeshComponentsSubDir"
        },

        { IDTMTAG_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESHCOMPONENTS_VARDATA_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesMeshComponentsVarDataSubDir"
        },
        { IDTMTAG_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR, IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_MESH_STATISTICS_SUBDIR, 1, 1,
        LONG, true, false, "SpatialIndexDirNodesMeshStatisticsSubDir"
        },

//------------------ Attributes end ----------------------
        {   0, IDTM_ATTRIBUTEID_END,  0,0,
        _NOTYPE, true, true, ""
        },


    };


const size_t AttributeInfo::sDefinitionQty = (sizeof (sDefinitionArray) / sizeof (sDefinitionArray[0]));




FileDefault::FileDefault () :
    topNode(0),
    topTileResolution(0),
    extent2d(), //{0.0, 0.0, 0.0, 0.0}
    extent3d(), //{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    resolution(0),
    filtered(false)
    {

    }



const HTGFF::DataType& ExtentTypeDef::GetType ()
    {
    using HTGFF::Dimension;
    using HTGFF::DataType;

    static const Dimension DIMENSIONS[]   =
        {
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_X_MIN),
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_X_MAX),
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_Y_MIN),
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_Y_MAX),
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_Z_MIN),
        Dimension::CreateFrom(Dimension::TYPE_FLOAT_64,   DR_Z_MAX),
        };

    static const DataType DATA_TYPE (ARRAY_BEGIN(DIMENSIONS), ARRAY_END(DIMENSIONS));


    return DATA_TYPE;
    }



} //End namespace IDTMFile