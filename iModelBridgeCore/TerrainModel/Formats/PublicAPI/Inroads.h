/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/Inroads.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Core/DTMDefs.h>

///////// bcdtmInroads/////////
                         DTMStatusInt bcdtmFormatInroads_clipUsingIslandFeatureIdDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureId dtmFeatureId);
                         DTMStatusInt bcdtmFormatInroads_deletePointDtmObject (BC_DTM_OBJ *dtmP, long point);
                                  int bcdtmFormatInroads_dtmFeaturePointsCallBackFunction (BC_DTM_OBJ *dtmP, const wchar_t* dtmFeatureName, const wchar_t* dtmFeatureDescription, const wchar_t* dtmFeatureStyle, DTMFeatureType dtmFeatureType, DPoint3d *dtmFeaturePointsP, long numDtmFeaturePoints, long excludeFromTriangulation);
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_exportBclibDtmToInroadsDtmFile (BC_DTM_OBJ *dtmP, const wchar_t *dtmFileNameP, const wchar_t *nameP, const wchar_t *descriptionP);
                         DTMStatusInt bcdtmFormatInroads_fixTopologyDtmObject (BC_DTM_OBJ *dtmP);
                                  int bcdtmFormatInroads_geopakCircularListCallBackFunction (BC_DTM_OBJ *dtmP,long pointIndex,long *cirPointIndexP,long numCirPointIndex);
                                  int bcdtmFormatInroads_geopakPointsCallBackFunction (BC_DTM_OBJ *dtmP, double x, double y, double z);
                         DTMStatusInt bcdtmFormatInroads_getInroadsTriangleNumberDtmObject (BC_DTM_OBJ *dtmP, DTM_MX_TRG_INDEX *trgIndexP, long trgPnt1, long trgPnt2, long trgPnt3, long *trgNumP);
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_importBclibDtmFromInroadsDtmFile (BC_DTM_OBJ **dtmPP, const wchar_t *dtmFileNameP);
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_importGeopakTinFromFile (const wchar_t *tinFileNameP, int (*tinStatsCallBackFunctionP) (long numRandomPoints,long numFeaturePoints,long numTriangles,long numFeatures), int (*tinRandomPointsCallBackFunctionP) (long pntIndex,double x,double y,double z), int (*tinFeaturePointsCallBackFunctionP) (long pntIndex,double x,double y,double z), int (*tinTrianglesCallBackFunctionP) (long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex), int (*tinFeaturesCallBackFunctionP) (DTMFeatureType dtmFeatureType,int64_t dtmUsertag,int64_t dtmFeatureId,long *pointIndicesP,long numPointIndices)); 
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_importTinFromDtmObject (BC_DTM_OBJ *dtmP, int (*tinStatsCallBackFunctionP) (long numRandomPoints,long numFeaturePoints,long numTriangles,long numFeatures), int (*tinRandomPointsCallBackFunctionP) (long pntIndex,double x,double y,double z), int (*tinFeaturePointsCallBackFunctionP) (long pntIndex,double x,double y,double z), int (*tinTrianglesCallBackFunctionP) (long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex), int (*tinFeaturesCallBackFunctionP) (DTMFeatureType dtmFeatureType,int64_t dtmUsertag,int64_t dtmFeatureId,long *pointIndicesP,long numPointIndices)); 
                         DTMStatusInt bcdtmFormatInroads_insertLineBetweenVerticesDtmObject (BC_DTM_OBJ *dtmP, long startPnt, long endPnt);
                         DTMStatusInt bcdtmFormatInroads_insertRectangleAroundTinDtmObject (BC_DTM_OBJ *dtmP, const double& xdec, const double& xinc, const double& ydec, const double& yinc, DTMFeatureId& islandFeatureIdP);
                         DTMStatusInt bcdtmFormatInroads_loadFeaturesFromDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureId hullFeatureId, int (*loadFunctionP) (DTMFeatureType dtmFeatureType,int64_t dtmUsertag,int64_t dtmFeatureId,long *pointIndicesP,long numPointIndices));
                         DTMStatusInt bcdtmFormatInroads_loadTrianglesFromDtmObject (BC_DTM_OBJ *dtmP, int (*loadFunctionP) (long trgNum, long trgPnt1, long trgPnt2,long trgPnt3, long voidTriangle, long side1Trg, long side2Trg, long side3Trg)); 
                         DTMStatusInt bcdtmFormatInroads_removeBoundingRectangleDtmObject (BC_DTM_OBJ *dtmP);
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_testForConnectedPointsDtmObject (BC_DTM_OBJ *dtmP,long point1,long point2);
                         DTMStatusInt bcdtmFormatInroads_trimToInroadsExteriorBoundaryDtmObject (BC_DTM_OBJ *dtmP);
