/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageCatchment.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bcdtmDrainageTables.h"
int bcdtmDrainage_traceCatchmentForPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double maxPondDepth,bool* catchmentDeterminedP,long *catchmentClosureP,DPoint3d **catchmentPtsPP,long *numCatchmentPtsP,DPoint3d *sumpPointP) ;
int bcdtmDrainage_traceCatchmentForSumpLineDtmObject(BC_DTM_OBJ *dtmP,double sumpX,double sumpY,double sumpZ,long sumpPnt1,long sumpPnt2,double maxPondDepth,long useTables,DPoint3d **catchmentPtsPP,long *numCatchmentPtsP) ;
int bcdtmDrainage_traceCatchmentFromPointOnInternalSumpLineDtmObject(BC_DTM_OBJ *dtmP,double sumpX,double sumpY,double sumpZ,long sumpPoint1,long sumpPoint2,double maxPondDepth,long useTables,DPoint3d **catchmentPtsPP,long *numCatchmentPtsP) ;
int bcdtmDrainage_calculateTracePointsForTriangleDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long pnt3,DPoint3d tracePoints[]) ;
int bcdtmDrainage_refineTptrCatchmentPolygonDtmObject(BC_DTM_OBJ *dtmP,double sumpX,double sumpY,double sumpZ,long sumpPoint1,long sumpPoint2,long firstPoint,double maxPondDepth,DPoint3d **catchmentPtsPP,long *numCatchmentPtsP) ;
int bcdtmDrainage_traceMaximumAscentFromPointOnTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,double pointX,double pointY,double pointZ,long point1,long point2,long point3,long *ascentTracedP,long *highPoint1P,long *highPoint2P,DPoint3d **tracePtsPP,long *numTracePtsP) ;
int bcdtmDrainage_checkPointOnTinHullCanBeDeletedDtmObject(BC_DTM_OBJ *dtmP,long point,long *canBeDeletedP) ;
int bcdtmDrainage_calculateRadialIntersectOnOppositeTriangleEdgeDtmObject( BC_DTM_OBJ *dtmP,double r1X,double r1Y,double r2X,double r2Y,long pnt1,long pnt2,long pnt3,double *nextXP,double *nextYP,double *nextZP,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P) ;
int bcdtmDrainage_traceCatchmentFromInternalSumpPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double z,long sumpPnt,long useTables,DPoint3d **catchmentPtsPP,long *numCatchmentPtsP) ;
int bcdtmDrainage_refineTinForDrainageDtmObject(BC_DTM_OBJ *dtmP, bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts) ;
int bcdtmDrainage_traceCatchmentsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,DTMDrainageTables *drainageTablesP,double falseLowDepth,bool refineOption,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,DPoint3dCP fencePtsP,int numFencePts,void *userP,int& numCatchments) ;
int bcdtmDrainage_determineCatchmentsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,DTMDrainageTables *drainageTablesP,double falseLowDepth,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,DPoint3dCP fencePtsP,int  numFencePts,void *userP,int& numCatchments) ;
int bcdtmDrainage_polygoniseAndLoadTriangleIndexPolygonsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,DTMTriangleIndex* triangleIndexP,void *userP,int& numPolygons) ;

//  Code To Refine Catchments

int bcdtmDrainage_refineCatchmentBoundariesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *catchmentDtmP,double falseLowDepth,long descentTraceOverZeroSlope) ;
int bcdtmDrainage_insertCatchmentPolygonIntoDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *polygonPtsP,long numPolygonPts,long *startPointP) ;
int bcdtmDrainage_insertMaximumAscentLinesFromCatchmentDtmObject(BC_DTM_OBJ *dtmP,long startPoint) ;
int bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2,long p3,double angle) ;
int bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(BC_DTM_OBJ *dtmP,long point,long startPoint,long endPoint) ;
int bcdtmDrainage_scanBetweenPointsForRidgeLineDtmObject(BC_DTM_OBJ *dtmP,long Point,long Sp,long Ep,long *RidgePoint) ;
int bcdtmDrainage_scanBetweenPointsForMaximumAscentTriangleDtmObject(BC_DTM_OBJ *dtmP,long useTables,long point,long startPoint,long endPoint,long *trgPnt1P,long *trgPnt2P,double *trgAscentAngleP,double *trgSlopeP)  ;
int bcdtmDrainage_getNextMaximumAscentLineDtmObject(BC_DTM_OBJ *dtmP,long lastPoint,long point,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,long   *processP ) ;
int bcdtmDrainage_expandTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long  *startPointP) ;
int bcdtmDrainage_copyCatchmentTrianglesDtmObject(BC_DTM_OBJ *dtmP,long startPoint,BC_DTM_OBJ **catchDtmPP) ;
int bcdtmDrainage_determineRefinedCatchmentBoundaryDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *catchmentDtmP,DTMFeatureCallback loadFunctionP,double falseLowDepth,long catchmentID,void *userP,long *numCatchmentsP ) ;
int bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject(BC_DTM_OBJ *dtmP,long point) ;
int bcdtmDrainage_insertMaximumAscentLineFromPointOnTinLineDtmObject(BC_DTM_OBJ *dtmP,int point1,int point2,DPoint3d point) ;
int bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,long point3) ;
int bcdtmDrainage_genericCallBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTMFeatureId featureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;
int bcdtmDrainage_checkForMaximumAscentFlowLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,bool& maxAscent ) ;
int bcdtmDrainage_insertMaximumAscentLineFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2) ;
int bcdtmDrainage_insertMaximumAscentLinesFromPointOnSumpLineDtmObject(BC_DTM_OBJ *dtmP,int point1,int point2,DPoint3d point, long& drainPoint) ;
int bcdtmDrainage_insertMaximumAscentLinesFromSumpPointDtmObject(BC_DTM_OBJ *dtmP,int sumpPoint ) ;
