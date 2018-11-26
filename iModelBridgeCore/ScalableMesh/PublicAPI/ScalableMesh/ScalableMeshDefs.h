/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshDefs.h $
|    $RCSfile: ScalableMeshDefs.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/26 17:55:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/       
#pragma once

#include <Bentley/Bentley.h>

#define BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace ScalableMesh {
#define END_BENTLEY_SCALABLEMESH_NAMESPACE     }}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH    using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh;

#ifndef BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
#define BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE namespace Import {
#define END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE  END_BENTLEY_SCALABLEMESH_NAMESPACE}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Import;
#endif //!BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

#if _WIN32
#ifdef __BENTLEYSTM_BUILD__ 
    #define BENTLEY_SM_EXPORT __declspec(dllexport)
#else
    #define BENTLEY_SM_EXPORT __declspec(dllimport)
#endif
#else
    #define BENTLEY_SM_EXPORT
#endif
    

#if _WIN32
#ifdef __BENTLEYSTMIMPORT_BUILD__ 
#define BENTLEY_SM_IMPORT_EXPORT __declspec(dllexport)
#else
#define BENTLEY_SM_IMPORT_EXPORT __declspec(dllimport)
#endif
#else
    #define BENTLEY_SM_IMPORT_EXPORT
#endif

#if !defined(_WIN32)
typedef uint8_t byte;
#endif

#ifdef VANCOUVER_API
#define IMAGEPP_NAMESPACE_NAME
#else
#define IMAGEPP_NAMESPACE_NAME ImagePP
#endif



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

enum ScalableMeshCompressionType
    {
    SCM_COMPRESSION_NONE = 0,
    SCM_COMPRESSION_DEFLATE,
    SCM_COMPRESSION_QTY
    };

enum ScalableMeshFilterType
    {
    SCM_FILTER_DUMB = 0,    
    SCM_FILTER_PROGRESSIVE_DUMB = 1,    
    SCM_FILTER_DUMB_MESH = 2,        
    SCM_FILTER_CGAL_SIMPLIFIER = 3,
    SCM_FILTER_QTY,
    };

enum ScalableMeshMesherType
    {
    SCM_MESHER_2D_DELAUNAY = 0, //Suitable only for 2.5D data       
    SCM_MESHER_3D_DELAUNAY = 2,
    SCM_MESHER_TETGEN = 3,
    SCM_MESHER_QTY,
    };

enum ScalableMeshCreationMethod
    {
    SCM_CREATION_METHOD_ONE_SPLIT = 0,     //Old method indexing the data with the final split threshold (e.g. : 10000)
    SCM_CREATION_METHOD_BIG_SPLIT_CUT, //New method which uses bigger split during the indexing phase.
    SCM_CREATION_METHOD_QTY,
    };

enum ScalableMeshCreationCompleteness
    {
    SCM_CREATION_COMPLETENESS_INDEX_ONLY = 0,  //Only create the index without any meshing operations.
    SCM_CREATION_COMPLETENESS_FULL,            //Do complete creation.
    SCM_CREATION_COMPLETENESS_QTY,
    };

enum ScalableMeshSaveType
    {
    SCM_SAVE_STMFILE = 0,
    SCM_SAVE_DGNDB_TABLE = 1,
    SCM_SAVE_DGNDB_BLOB = 2,
    SCM_SAVE_QTY,
    };

//NEEDS_WORK_MST : Must be renamed SCMQueryType
enum ScalableMeshQueryType
    {
    SCM_QUERY_FULL_RESOLUTION = 0,      //Query the highest resolution data.
    SCM_QUERY_VIEW_DEPENDENT,                                           
    SCM_QUERY_FIX_RESOLUTION_VIEW,  //Return query which can be used to obtain view of a particular ScalableMesh's resolution. 
    SCM_QUERY_EXTENTS_BY_PARTS, // For tiled triangulation
    SCM_QUERY_ALL_POINTS_IN_EXTENT,
    SCM_QUERY_ALL_LINEARS_IN_EXTENT,
    SCM_QUERY_QTY
    };

enum MeshQueryType
    {
    MESH_QUERY_FULL_RESOLUTION = 0,      //Query the highest resolution data.
    MESH_QUERY_VIEW_DEPENDENT,
    MESH_QUERY_PLANE_INTERSECT,
    MESH_QUERY_CONTEXT,
    MESH_QUERY_QTY
    };

enum DTMSourceDataType
    {
    DTM_SOURCE_DATA_MIX = 0, 
    DTM_SOURCE_DATA_DTM, 
    DTM_SOURCE_DATA_POINT,
    DTM_SOURCE_DATA_BREAKLINE,
    DTM_SOURCE_DATA_CLIP,
    DTM_SOURCE_DATA_MASK,    
    DTM_SOURCE_DATA_HARD_MASK,
    DTM_SOURCE_DATA_IMAGE,
    DTM_SOURCE_DATA_MESH,
    DTM_SOURCE_DATA_QTY
    };

enum DTMSourceGroupOperation
    {
    DTM_SOURCE_GROUP_OP_REPLACE = 0, 
    DTM_SOURCE_GROUP_OP_APPEND, 
    DTM_SOURCE_GROUP_OP_QTY
    };

enum ScalableMeshState
    {
    SCM_STATE_EMPTY = 0, 
    SCM_STATE_DIRTY, 
    SCM_STATE_UP_TO_DATE, 
    SCM_STATE_QTY        
    };

enum DTMSourceMonikerType
    {
    DTM_SOURCE_MONIKER_FULL_PATH = 0, 
    DTM_SOURCE_MONIKER_MSDOCUMENT = 1, 
    DTM_SOURCE_MONIKER_DATABASE = 2,   
    DTM_SOURCE_MONIKER_QTY    
    };

enum DTMStatus
    {   
    DTMSTATUS_SUCCESS = 0,
    DTMSTATUS_ERROR,
    DTMSTATUS_UNSUPPORTED_VERSION,
    DTMSTATUS_QTY                
    };

enum SMStatus
    {
    S_SUCCESS = 0,
    S_ERROR,
    S_ERROR_COULD_NOT_OPEN,
    S_ERROR_NOT_SUPPORTED,
    S_ERROR_NOT_FOUND,
    S_ERROR_DOES_NOT_FIT_MATHEMATICAL_DOMAIN,
	S_ERROR_CANCELED_BY_USER,
    S_WARNING_NODE_NOT_FOUND,
    S_QTY,
    };

enum class SMNonDestructiveClipType
    {
    Mask = 0,
    Boundary,
    Qty
    };

enum class SMClipGeometryType
    {
    Polygon = 0,
    ComplexPolygon,
    BoundedVolume,
    Qty
    };

enum class SMTextureType
    {
    None = 0,
    Embedded,
    Streaming
    };

enum class SMNodeViewStatus
    {
    NotVisible = 0,
    TooCoarse,
    Fine
    };
	
struct SMRasterTile
    {
    uint64_t m_posX;          //In pixels
    uint64_t m_posY;          //In pixels
    uint64_t m_sizeX;         //In pixels
    uint64_t m_sizeY;         //In pixels
    uint64_t m_resolutionInd;
    };
    

enum class SMGenerateOperation
    {
    SCM_FILTER_DUMB = 0,
    SCM_FILTER_PROGRESSIVE_DUMB = 1,
    SCM_FILTER_DUMB_MESH = 2,
    SCM_FILTER_CGAL_SIMPLIFIER = 3,
    SCM_FILTER_QTY,
    };


#define MEAN_SCREEN_PIXELS_PER_POINT 100


END_BENTLEY_SCALABLEMESH_NAMESPACE
/*__PUBLISH_SECTION_END__*/