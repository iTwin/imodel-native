/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMCoreDefs.h $
|    $RCSfile: MrDTMCoreDefs.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/08/02 14:59:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMCoreDefs.h    		  	    		    (C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Multi Resolution DTM                                                  |
|                                                                                       |
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#define NB_SAMPLE_CAMERA_ORIENTATIONS 9

typedef struct 
{
    double minX;
    double minY;
    double maxX;
    double maxY;
} BC_MRES_TILE_LIMIT; 

typedef struct
{
 long   tileNumber ;
 long   resolutionLevel ;
 long   numPoints ;
 long   fileOffset ;
} BC_MRES_TILE_FILE_OFFSETS ;

typedef struct
{ 
 long               overviewResolutionLevel;
 long               numPoints ;
 long               fileOffset ;
 BC_MRES_TILE_LIMIT overviewLimit2D ; 
 double             viewDependentMetric[NB_SAMPLE_CAMERA_ORIENTATIONS] ;
} BC_MRES_OVERVIEW_FILE_OFFSETS_M03 ;

typedef struct 
{
 long    tileNumber ;
 long    numTilePoints ;
 long    resolutionLevel ;
 long    tileOffset ;
 double  tileXMin,tileXMax,tileYMin,tileYMax,tileZMin,tileZMax ;
} BC_MRES_TILE ;

typedef struct
{
 long  dtmFileType ;
 long  dtmVersionNumber ;
 long  numTiles ;
 long  numResolutionLevels ;
 long  spatialIndexOffset ;
 long  tileIndexOffset ;
 long  numTileIndex ;
} BC_MRES_DTM ;

typedef struct
{ 
 /* BC_MRES_DTM  */
 long  dtmFileType ;
 long  dtmVersionNumber ;
 long  numTiles ;
 long  numResolutionLevels ;
 long  spatialIndexOffset ;
 long  tileIndexOffset ;
 long  numTileIndex ;

 /* BC_MRES_DTM extension */
 long  numOverviewLevels ;
 long  overviewIndexOffset ;
} BC_MRES_DTM_M03 ;

typedef struct
{
 long     tileNumber ;
 long     resolutionLevel ;
 long     numPoints ;
 __int64  fileOffset ;
 double   xMin ;
 double   yMin ;
 double   zMin ;
 double   xMax ;
 double   yMax ;
 double   zMax ; 
} BC_MRES_FILE_TILE_OFFSETS ;

typedef struct
{
 long     dtmFileType ;
 long     dtmVersionNumber ;
 long     numPoints ;
 long     numTiles ;
 long     numResolutionLevels ;
 double   xMin ;
 double   yMin ;
 double   zMin ;
 double   xMax ;
 double   yMax ;
 double   zMax ; 
 __int64  spatialIndexOffset ;
 __int64  tileIndexOffset ;
} BC_MRES2_DTM ;

typedef struct
{
 long     tileNumber ;
 __int64  fileOffset ;
 long     numPoints ;
 long     resolutionPoints[20] ;
 double   xMin ;
 double   yMin ;
 double   zMin ;
 double   xMax ;
 double   yMax ;
 double   zMax ; 
} BC_MRES2_TILE_FILE_OFFSETS ;

//Filtering related definitions
enum RelevanceEvaluationMethod
{
    RELEVANCE_EVAL_ELEV_DIFF_TIN = 0,        //Based on Rob's TIN decimation function.
    RELEVANCE_EVAL_ELEV_DIFF_TILE,           //Based on Rob's TILE decimation function. 
    RELEVANCE_EVAL_NEIGHBOR_TRI_NORMAL_DIFF, //Tri normal diff function.            
};
