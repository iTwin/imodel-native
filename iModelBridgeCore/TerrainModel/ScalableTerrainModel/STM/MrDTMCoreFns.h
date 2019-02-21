/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMCoreFns.h $
|    $RCSfile: MrDTMCoreFns.h,v $
|   $Revision: 1.22 $
|       $Date: 2012/11/14 18:21:00 $
|     $Author: Daryl.Holmwood $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMCoreFns.h    		  	    		    (C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/MrDTMDefs.h>
#include "MrDTMCoreDefs.h"

//  Multi Resolution DTM Functions

int bcdtmMultiResolution_tileDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredDtmP) ;
int bcdtmMultiResolution_elevationDifferenceCompareFunction(const void *p1P,const void *p2P) ;
int bcdtmMultiResolution_elevationDifferenceTileCompareFunction(const void *p1P,const void *p2P) ;
int bcdtmMultiResolution_elevationDifferenceTileKeepCompareFunction(const void *p1P,const void *p2P);
int bcdtmMultiResolution_tileDecimateGroupSpotsDtmObject(BC_DTM_OBJ *dtmP,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredDtmP) ;

int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;
int bcdtmMultiResolution_tinDecimateGroupSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;      


int bcdtmMultiResolution_sortBinaryXYZFile(char *xyzFileNameP,char *outFileNameP,long sortPartitionSize,long *numSortedPointsP) ;

int bcdtmMultiResolution_checkSortOrderDtmObject(BC_DTM_OBJ *dtmP,long firstPoint,long lastPoint,long axis,long *isSortedP) ;
int bcdtmMultiResolution_checkForDuplicatesDtmObject(BC_DTM_OBJ *dtmP,long firstPoint,long lastPoint,long axis,long *isduplicatesP) ;
int bcdtmMultiResolution_filterTiledXYZFile(char *xyzFileNameP,char *mrDtmFileNameP,long numResolutionLevels,long numResolutionPoints,long decimationFactor,BC_MRES_FILE_TILE_OFFSETS *tileOffsetsP,long numTiles) ;
int bcdtmMultiResolution_filteredTilePointsCompareFunction(const void *p1P,const void *p2P) ;

int bcdtmMultiResolution_filterToLowestResolutionLevelDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *indexP,FILE *mrDtmFP,BC_MRES2_DTM mrDtmHeader,long numResolutionPoints,long decimationFactor,BC_MRES2_TILE_FILE_OFFSETS *tileOffsetsP,long numTiles) ;
int bcdtmMultiResolution_tileXYZFile( char *xyzFileNameP,__int64 tilePartitionSize,BC_MRES_FILE_TILE_OFFSETS **tileFileOffsetsPP,long *numTilesP) ;
int bcdtmMultiResolution_getRotationMatrix(double omega, double phi, double kappa, double* rotMatrixP) ;
int bcdtmMultiResolution_getSampleCameraOri(double viewportRotMatrix[][3], int* camOriP) ;
int bcdtmMultiResolution_getRootToViewScale(double rootToViewMatrix[][4], BC_MRES_TILE_LIMIT * tileLimitP, double* rootToViewScaleP) ;
int bcdtmMultiResolution_getTileAreaInCurrentView(double rootToViewMatrix[][4], DPoint3dP tileShapePts, int nbTileShapePts, double* rootToViewScaleP);

int bcdtmMultiResolution_tinZToleranceFilterRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,double filterTolerance,long *numFilteredPtsP,BC_DTM_OBJ *filteredPtsP) ;
int bcdtmMultiResolution_tinZToleranceFilterGroupSpotsDtmObject( BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,double filterTolerance,long *numFilteredPtsP,BC_DTM_OBJ *filteredPtsP) ;
int bcdtmMultiResolution_tileZToleranceFilterRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,double filterTolerance,long *numFilteredSpotsP,BC_DTM_OBJ *filteredDtmP) ;
