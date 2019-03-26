/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/dtm2dfnsXM.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once
#ifdef XM
#include <TerrainModel/TerrainModel.h>

///////// bcdtmCheck/////////
BENTLEYDTM_Public                      int bcdtmCheck_dtmFeatureEndPointsTinObject (DTM_TIN_OBJ *tinP, long reportErrors); 
BENTLEYDTM_Public                      int bcdtmCheck_dtmFeatureListTinObject (DTM_TIN_OBJ *tinP, long reportErrors); 
BENTLEYDTM_Public                      int bcdtmCheck_forClosePointsTinObject (DTM_TIN_OBJ *Tin, double Pptol, long MessageFlag, long *NoClosePoints); 
BENTLEYDTM_Public                      int bcdtmCheck_forIntersectingTinHullLinesTinObject (DTM_TIN_OBJ *Tin, long MessageFlag); 
BENTLEYDTM_Public                      int bcdtmCheck_forKnotInDtmFeatureTinObject (DTM_TIN_OBJ *Tin, long Feature); 
BENTLEYDTM_Public                      int bcdtmCheck_forSliverTrianglesTinObject (DTM_TIN_OBJ *Tin, double Pltol, long MessageFlag, long *NoSliverTriangles); 
BENTLEYDTM_EXPORT                      int bcdtmCheck_integrityTinObject (DTM_TIN_OBJ *Tin); 
BENTLEYDTM_Public                      int bcdtmCheck_pointPrecisionTinObject (DTM_TIN_OBJ *Tin, long Point, long MessageFlag); 
BENTLEYDTM_Public                      int bcdtmCheck_precisionTinObject (DTM_TIN_OBJ *Tin, long MessageFlag); 
BENTLEYDTM_Public                      int bcdtmCheck_sortOrderDataObject (DTM_DAT_OBJ *Data); 
BENTLEYDTM_Public                      int bcdtmCheck_sortOrderTinObject (DTM_TIN_OBJ *Tin); 
BENTLEYDTM_Public                      int bcdtmCheck_topologyDtmFeaturesTinObject (DTM_TIN_OBJ *tinP, long messageFlag); 
BENTLEYDTM_Public                      int bcdtmCheck_topologyTinObject (DTM_TIN_OBJ *Tin, long MessageFlag, long WarnFlag); 

///////// bcdtmData/////////
BENTLEYDTM_EXPORT                      int bcdtmData_joinAdjacentDtmObjects (DTM_DAT_OBJLIST *joinObjectsP, long numJoinObjects, double xyTolerance, double zTolerance);

///////// bcdtmGeopak/////////
BENTLEYDTM_Public                      int bcdtmGeopak_copyDataObjectToDtmObject (DTM_DAT_OBJ *dataP, BC_DTM_OBJ *dtmP);
BENTLEYDTM_EXPORT                      int bcdtmGeopak_copyDtmObjectToTinObject (BC_DTM_OBJ *dtmP, DTM_TIN_OBJ **tinPP);
BENTLEYDTM_EXPORT                      int bcdtmGeopak_copyTinObjectToDtmObject (DTM_TIN_OBJ *tinP, BC_DTM_OBJ *dtmP);

///////// bcdtmInsert/////////
BENTLEYDTM_Public                      int bcdtmInsert_addDtmFeatureToTinObject (DTM_TIN_OBJ *tinP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTM_GUID userFeatureId, long startPoint, long clearFlag); 
BENTLEYDTM_Private                     int bcdtmInsert_addToFeatureListTinObject (DTM_TIN_OBJ *tinP, long tableEntry, long startPoint, long clearFlag); 
BENTLEYDTM_Public                      int bcdtmInsert_addToFeatureTableTinObject (DTM_TIN_OBJ *tinP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTM_GUID userFeatureId, long startPoint, long *tableEntryP); 
BENTLEYDTM_Public                      int bcdtmInsert_checkAndFixLastPointOfDtmFeaturesTinObject (DTM_TIN_OBJ *tinP); 

///////// bcdtmList/////////
BENTLEYDTM_Private                     int bcdtmList_addDtmFeatureToTptrListTinObject (DTM_TIN_OBJ *tinP, long dtmFeature, long *firstPntP, long *lastPntP, long *numAddedP); 
BENTLEYDTM_Public                      int bcdtmList_checkConnectivityOfDtmFeatureTinObject (DTM_TIN_OBJ *Tin, long dtmFeature, long MessageFlag); 
BENTLEYDTM_Public                      int bcdtmList_checkConnectivityTptrListTinObject (DTM_TIN_OBJ *tinP, long startPnt, long messageFlag); 
BENTLEYDTM_Public                      int bcdtmList_checkConnectivityTptrPolygonTinObject (DTM_TIN_OBJ *tinP, long startPnt, long messageFlag); 
BENTLEYDTM_EXPORT                      int bcdtmList_copyDtmFeaturePointsToPointArrayTinObject (DTM_TIN_OBJ *tinP, long dtmFeature, DPoint3d **featurePtsPP, long *numFeaturePtsP); 
BENTLEYDTM_Public                      int bcdtmList_copyDtmFeatureToTptrListTinObject (DTM_TIN_OBJ *tinP, long dtmFeature, long *startPnt); 
BENTLEYDTM_Public                      int bcdtmList_getNextPointForDtmFeatureTinObject (DTM_TIN_OBJ *tinP, long dtmFeature, long currentPnt , long *nextPnt); 
BENTLEYDTM_Public                      int bcdtmList_getPriorPointForDtmFeatureTinObject (DTM_TIN_OBJ *tinP, long dtmFeature, long currentPnt, long *priorPnt); 
BENTLEYDTM_Public                      int bcdtmList_getVoidExternalToIslandTinObject (DTM_TIN_OBJ *tinP, long islandFeature, long *voidFeatureP); 
BENTLEYDTM_Private                     int bcdtmList_getVoidFeatureExternalToIslandFeatureTinObject (DTM_TIN_OBJ *tinP, long islandFeature, long *voidFeatureP); 
BENTLEYDTM_EXPORT                     long bcdtmList_nextAntTinObject (DTM_TIN_OBJ *Tin, long p1, long p2); 
BENTLEYDTM_Public                     long bcdtmList_nextClockTinObject (DTM_TIN_OBJ *Tin, long p1, long p2); 
BENTLEYDTM_Public                      int bcdtmList_nullOutTptrListTinObject (DTM_TIN_OBJ *Tin, long Spnt); 
BENTLEYDTM_Public                      int bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject (DTM_TIN_OBJ *Tin, long reportFlag); 
BENTLEYDTM_Public                      int bcdtmList_reverseTptrPolygonTinObject (DTM_TIN_OBJ *Tin, long Spnt); 
BENTLEYDTM_Public                      int bcdtmList_testForIslandHullLineTinObject (DTM_TIN_OBJ *Tin, long P1, long P2); 
BENTLEYDTM_Private                     int bcdtmList_testForSurroundingDtmFeatureTypeTinObject (DTM_TIN_OBJ *tinP, long featurePnt, long featureNextPnt, long featurePriorPnt, DTMFeatureType dtmFeatureType, long *surroundFeatureP); 
BENTLEYDTM_Public                      int bcdtmList_testForVoidOrHoleHullLineTinObject (DTM_TIN_OBJ *Tin, long P1, long P2); 
BENTLEYDTM_Public                      int bcdtmList_testIfPointOnDtmFeatureTinObject (DTM_TIN_OBJ *Tin, DTMFeatureType DtmFeatureType, long P, long *Feature); 
BENTLEYDTM_Public                      int bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject (DTM_TIN_OBJ *Tin, long P1); 
BENTLEYDTM_Public                      int bcdtmList_testLineTinObject (DTM_TIN_OBJ *Tin, long p1, long p2); 
BENTLEYDTM_Public                      int bcdtmList_writeCircularListForPointTinObject (DTM_TIN_OBJ *Tin, long P); 
BENTLEYDTM_Public                      int bcdtmList_writeDtmFeaturesForPointTinObject (DTM_TIN_OBJ *tinP, long point); 
BENTLEYDTM_Public                      int bcdtmList_writePointsForDtmFeatureTinObject (DTM_TIN_OBJ *tinP, long dtmFeature); 
BENTLEYDTM_Public                      int bcdtmList_writeTptrListTinObject (DTM_TIN_OBJ *Tin, long Point); 

///////// bcdtmMark/////////
BENTLEYDTM_Public                      int bcdtmMark_pointsExternalToIslandTinObject (DTM_TIN_OBJ *Tin, long islandFeature); 
BENTLEYDTM_Public                      int bcdtmMark_pointsInternalToVoidTinObject (DTM_TIN_OBJ *Tin, long VoidFeature); 
BENTLEYDTM_Public                      int bcdtmMark_voidPointsExternalToTptrIslandPolygonTinObject (DTM_TIN_OBJ *Tin, long sPnt, long *numMarked); 
BENTLEYDTM_Public                      int bcdtmMark_voidPointsInternalToTptrVoidPolygonTinObject (DTM_TIN_OBJ *Tin, long sPnt, long *numMarked); 
BENTLEYDTM_Public                      int bcdtmMark_voidPointsTinObject (DTM_TIN_OBJ *Tin); 

///////// bcdtmMath/////////
BENTLEYDTM_Public                      int bcdtmMath_allPointSideOfTinObject (DTM_TIN_OBJ *Tin, long p1, long p2, long p3); 
BENTLEYDTM_Public                      int bcdtmMath_calculateAreaAndDirectionSptrPolygonTinObject (DTM_TIN_OBJ *Tin, long Spnt, double *Area, DTMDirection* direction); 
BENTLEYDTM_Public                      int bcdtmMath_calculateAreaAndDirectionTptrPolygonTinObject (DTM_TIN_OBJ *Tin, long Spnt, double *Area, DTMDirection* direction); 
BENTLEYDTM_Public                   double bcdtmMath_pointDistanceTinObject (DTM_TIN_OBJ *Tin, long p1, long p2); 

///////// bcdtmNormal/////////
BENTLEYDTM_Public                      int bcdtmNormal_calculateMachinePrecisionForTinObject (DTM_TIN_OBJ *Tin, double *Mpptol); 

///////// bcdtmObject/////////
BENTLEYDTM_Public                      int bcdtmObject_allocateMemoryDataObject (DTM_DAT_OBJ *dataP); 
BENTLEYDTM_Public                      int bcdtmObject_allocateMemoryFeatureListTinObject (DTM_TIN_OBJ *tinP); 
BENTLEYDTM_Public                      int bcdtmObject_allocateMemoryFeatureTableTinObject (DTM_TIN_OBJ *tinP); 
BENTLEYDTM_EXPORT                      int bcdtmObject_copyDataObjectToDtmObject (DTM_DAT_OBJ *dataP, BC_DTM_OBJ **dtmPP); 
BENTLEYDTM_EXPORT                      int bcdtmObject_copyTinObjectToDtmObject (DTM_TIN_OBJ *tinP, BC_DTM_OBJ **dtmPP); 
BENTLEYDTM_EXPORT                      int bcdtmObject_createDataObject (DTM_DAT_OBJ **dataPP); 
BENTLEYDTM_EXPORT                      int bcdtmObject_createTinObject (DTM_TIN_OBJ **Tin); 
BENTLEYDTM_EXPORT                      int bcdtmObject_deleteDataObject (DTM_DAT_OBJ **Data); 
BENTLEYDTM_EXPORT                      int bcdtmObject_deleteTinObject (DTM_TIN_OBJ **Tin); 
BENTLEYDTM_Public                      int bcdtmObject_deSortDataObject (DTM_DAT_OBJ *dataP); 
BENTLEYDTM_Public                      int bcdtmObject_freeMemoryDataObject (DTM_DAT_OBJ *DataObj); 
BENTLEYDTM_Public                      int bcdtmObject_freeMemoryTinObject (DTM_TIN_OBJ *tinP); 
BENTLEYDTM_Public                      int bcdtmObject_initialiseDataObject (DTM_DAT_OBJ *dataP); 
BENTLEYDTM_Public                      int bcdtmObject_initialiseTinObject (DTM_TIN_OBJ *tinP); 
BENTLEYDTM_EXPORT                      int bcdtmObject_setMemoryAllocationParametersDataObject (DTM_DAT_OBJ *dataP, long inimemPolyPts, long incmemPolyPts); 
BENTLEYDTM_EXPORT                      int bcdtmObject_storeDtmFeatureInDataObject (DTM_DAT_OBJ *dataP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTM_GUID userFeatureId, DPoint3d *featurePtsP, long numFeaturePts); 
BENTLEYDTM_EXPORT                      int bcdtmObject_storePointInDataObject (DTM_DAT_OBJ *dataP, long featureCode, DTMUserTag userTag, DTM_GUID userFeatureId, double x, double y, double z); 
BENTLEYDTM_Public                      int bcdtmObject_testForValidDataObject (DTM_DAT_OBJ *Data); 
BENTLEYDTM_EXPORT                      int bcdtmObject_testForValidTinObject (DTM_TIN_OBJ *Tin); 

///////// bcdtmRead/////////
BENTLEYDTM_EXPORT                      int bcdtmRead_atFilePositionDataObject (DTM_DAT_OBJ **dataPP, FILE *fpDATA, long filePosition); 
BENTLEYDTM_EXPORT                      int bcdtmRead_atFilePositionTinObject (DTM_TIN_OBJ **tinPP, FILE *tinFP, long filePosition); 
BENTLEYDTM_EXPORT                      int bcdtmRead_atFilePositionTinObject_custom (DTM_TIN_OBJ **Tin, WCharCP fileNameP, double FilePosition); 
BENTLEYDTM_Public                      int bcdtmRead_dataFileASCIIToDataObject (DTM_DAT_OBJ *Data, WCharCP DataFile); 
BENTLEYDTM_Public                      int bcdtmRead_dataFileBinaryToDataObject (DTM_DAT_OBJ *Data, WCharCP DataFile); 
BENTLEYDTM_EXPORT                      int bcdtmRead_dataFileToDataObject (DTM_DAT_OBJ *Data, WCharCP DataFileName); 

///////// bcdtmReadStream/////////
BENTLEYDTM_EXPORT                       int bcdtmReadStream_atFilePositionDataObject (DTM_DAT_OBJ **dataPP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long filePosition); 
BENTLEYDTM_EXPORT                       int bcdtmReadStream_atFilePositionTinObject (DTM_TIN_OBJ **tinPP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long filePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer3TinObject (DTM_TIN_OBJ *tinP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long filePosition);
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer400DataObject (DTM_DAT_OBJ *dataP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long FilePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer400TinObject (DTM_TIN_OBJ *Tin, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long FilePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer500DataObject (DTM_DAT_OBJ *dataP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long FilePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer500TinObject (DTM_TIN_OBJ *tinP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long FilePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer501DataObject (DTM_DAT_OBJ *dataP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long FilePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer501TinObject (DTM_TIN_OBJ *tinP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long filePosition); 
BENTLEYDTM_Public                       int bcdtmReadStream_atFilePositionVer502DataObject (DTM_DAT_OBJ *dataP, BENTLEY_NAMESPACE_NAME::TerrainModel::IBcDtmStream* dtmStreamP, long filePosition); 

///////// bcdtmString/////////
BENTLEYDTM_Public                       int bcdtmString_extractDtmFeatureTypeStringsFromDataObject (DTM_DAT_OBJ *dataP, DTMFeatureType dtmFeatureType, DTM_POINT_ARRAY ***featureArrayPPP, long *numFeatureArrayP); 

///////// bcdtmTin/////////
BENTLEYDTM_Public                       int bcdtmTin_compactFeatureListTinObject (DTM_TIN_OBJ *Tin); 
BENTLEYDTM_Public                       int bcdtmTin_compactFeatureTableTinObject (DTM_TIN_OBJ *tinP); 
BENTLEYDTM_EXPORT                       int bcdtmTin_markInternalVoidPointsTinObject (DTM_TIN_OBJ *tinP); 

///////// bcdtmUtility/////////
BENTLEYDTM_EXPORT                       int bcdtmUtility_setCurrentDataObject (DTM_DAT_OBJ *dtmP, WCharCP dtmFileP);
BENTLEYDTM_EXPORT                       int bcdtmUtility_setCurrentTinObject (DTM_TIN_OBJ *dtmP, WCharCP dtmFileP);
BENTLEYDTM_Public                       int bcdtmUtility_setNullValuesForBackwardsCompatibilityTinObject (DTM_TIN_OBJ *tinP);
BENTLEYDTM_Public                       int bcdtmUtility_setNullValuesForForwardsCompatibilityTinObject (DTM_TIN_OBJ *tinP);

///////// bcdtmWrite/////////
BENTLEYDTM_EXPORT                       int bcdtmWrite_dataFileFromDataObject (DTM_DAT_OBJ *Data, WCharCP DataFile); 
#endif
