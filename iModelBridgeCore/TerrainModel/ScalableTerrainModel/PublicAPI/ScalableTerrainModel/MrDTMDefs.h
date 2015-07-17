/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/MrDTMDefs.h $
|    $RCSfile: MrDTMDefs.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/26 17:55:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/       
#pragma once

#ifdef __BENTLEYSTM_BUILD__ 
    #define BENTLEYSTM_EXPORT __declspec(dllexport)
#else
    #define BENTLEYSTM_EXPORT __declspec(dllimport)
#endif
    
BEGIN_BENTLEY_MRDTM_NAMESPACE

enum MrDTMCompressionType
    {
    MRDTM_COMPRESSION_NONE = 0,
    MRDTM_COMPRESSION_DEFLATE,
    MRDTM_COMPRESSION_QTY
    };

enum MrDTMFilterType
    {
    MRDTM_FILTER_DUMB = 0,
    MRDTM_FILTER_TILE,
    MRDTM_FILTER_TIN,
    MRDTM_FILTER_PROGRESSIVE_DUMB,
    MRDTM_FILTER_PROGRESSIVE_TILE,
    MRDTM_FILTER_PROGRESSIVE_TIN,
    MRDTM_FILTER_QTY,
    };

enum DTMQueryType
    {
    DTM_QUERY_FULL_RESOLUTION = 0,      //Query the highest resolution data.
    DTM_QUERY_VIEW_DEPENDENT,                                           
    DTM_QUERY_FIX_RESOLUTION_VIEW,  //Return query which can be used to obtain view of a particular MrDTM's resolution. 
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

enum MrDTMState
    {
    MRDTM_STATE_EMPTY = 0, 
    MRDTM_STATE_DIRTY, 
    MRDTM_STATE_UP_TO_DATE, 
    MRDTM_STATE_QTY        
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

END_BENTLEY_MRDTM_NAMESPACE
/*__PUBLISH_SECTION_END__*/