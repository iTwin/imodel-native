/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshDefs.h $
|    $RCSfile: ScalableMeshDefs.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/26 17:55:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/       
#pragma once

#define BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace ScalableMesh {
#define END_BENTLEY_SCALABLEMESH_NAMESPACE     }}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH    using namespace Bentley::ScalableMesh;

#ifndef BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
#define BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE namespace Import {
#define END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE  END_BENTLEY_SCALABLEMESH_NAMESPACE}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT using namespace Bentley::ScalableMesh::Import;
#endif //!BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

#ifdef __BENTLEYSTM_BUILD__ 
    #define BENTLEYSTM_EXPORT __declspec(dllexport)
#else
    #define BENTLEYSTM_EXPORT __declspec(dllimport)
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
    SCM_FILTER_TILE,
    SCM_FILTER_TIN,
    SCM_FILTER_PROGRESSIVE_DUMB,
    SCM_FILTER_PROGRESSIVE_TILE,
    SCM_FILTER_PROGRESSIVE_TIN,
    SCM_FILTER_DUMB_MESH = 6,    
    SCM_FILTER_GARLAND_SIMPLIFIER = 7, 
    SCM_FILTER_QTY,
    };

enum ScalableMeshMesherType
    {
    SCM_MESHER_2D_DELAUNAY = 0, //Suitable only for 2.5D data   
    SCM_MESHER_LMS_MARCHING_CUBE = 1,
    SCM_MESHER_3D_DELAUNAY = 2,
    SCM_MESHER_TETGEN = 3,
    SCM_MESHER_QTY,
    };

enum ScalableMeshSaveType
    {
    SCM_SAVE_STMFILE = 0,
    SCM_SAVE_DGNDB_TABLE = 1,
    SCM_SAVE_DGNDB_BLOB = 2,
    SCM_SAVE_QTY,
    };

enum DTMQueryType
    {
    DTM_QUERY_FULL_RESOLUTION = 0,      //Query the highest resolution data.
    DTM_QUERY_VIEW_DEPENDENT,                                           
    DTM_QUERY_FIX_RESOLUTION_VIEW,  //Return query which can be used to obtain view of a particular ScalableMesh's resolution. 
    DTM_QUERY_EXTENTS_BY_PARTS, // For tiled triangulation
    DTM_QUERY_ALL_POINTS_IN_EXTENT,
    DTM_QUERY_ALL_LINEARS_IN_EXTENT,
    DTM_QUERY_QTY
    };

//MST : Maybe query and data type should be combined eventually
enum DTMQueryDataType
    {
    DTM_QUERY_DATA_POINT = 0, 
    DTM_QUERY_DATA_LINEAR, 
    DTM_QUERY_DATA_QTY
    };

enum MeshQueryType
    {
    MESH_QUERY_FULL_RESOLUTION = 0,      //Query the highest resolution data.
    MESH_QUERY_VIEW_DEPENDENT,
    MESH_QUERY_PLANE_INTERSECT,
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
    
#define MEAN_SCREEN_PIXELS_PER_POINT 100


struct IDTMVolume abstract
{
protected:
virtual DTMStatusInt _ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector) = 0;
virtual DTMStatusInt _ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector) = 0;

public:
BENTLEYSTM_EXPORT DTMStatusInt ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
BENTLEYSTM_EXPORT DTMStatusInt ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector);
};

END_BENTLEY_SCALABLEMESH_NAMESPACE
/*__PUBLISH_SECTION_END__*/