/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageTrace.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
class DTMDrainageTables;

int bcdtmDrainage_sumpLinePondTableCompareFunction(const void *void1P,const void *void2P ) ;

int bcdtmDrainage_scanPointForMaximumDescentDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *descentTypeP,long *descentPnt1P,long *descentPnt2P,double *descentSlopeP,double *descentAngleP)  ;
int bcdtmDrainage_scanPointForMaximumDescentSumpLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *sumpPointP,double *sumpAngleP,double *sumpSlopeP) ;
int bcdtmDrainage_scanPointForMaximumDescentTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *trgBasePnt1P,long *trgBasePnt2P,double *trgDescentAngleP,double *trgSlopeP) ; 
int bcdtmDrainage_scanBetweenPointsForMaximumDescentDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long startPoint,long endPoint,long *descentTypeP,long *descentPnt1P,long *descentPnt2P,double *descentSlopeP,double *descentAngleP) ;
int bcdtmDrainage_scanBetweenPointsForMaximumDescentSumpLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long startPoint,long endPoint,long *sumpPointP,double *sumpSlopeP,double *sumpAngleP) ;
int bcdtmDrainage_scanBetweenPointsForMaximumDescentTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long startPoint,long endPoint,long *trgPnt1P,long *trgPnt2P,double *trgDescentAngleP,double *trgSlopeP) ;

int bcdtmDrainage_startTraceDtmObject( BC_DTM_OBJ *dtmP, long   startTraceType, long   pnt1, long   pnt2, long   pnt3, double x, double y, double z, long   *startPointTypeP,long   *nextPnt1P,long   *nextPnt2P,long   *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,double *startTraceAngleP) ;
int bcdtmDrainage_traceMaximumDescentDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMFeatureCallback loadFunctionP,double  falseLowDepth,double  startX,double startY,void *userP) ;
int bcdtmDrainage_traceToLowPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMFeatureCallback loadFunctionP,double falseLowDepth,int zeroSlopeOption,bool loadFlag,long pnt1,long pnt2,long pnt3,double X,double Y,double Z,void *userP,long *lowPoint1P,long *lowPoint2P ) ;
int bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(BC_DTM_OBJ *dtmP,long flowDirection,long pnt1,long pnt2,long pnt3,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP)  ;
int bcdtmDrainage_getFirstTracePointFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,long flowDirection,long pnt1,long pnt2,long pnt3,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP) ;
int bcdtmDrainage_traceMaximumDescentFromTriangleEdgeDtmObject( BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,int zeroSlopeOption,long isFalseLow,double lastTraceAngle,long startPnt1, long startPnt2, long startPnt3, double startX, double startY, long *nextPnt1P, long *nextPnt2P, long *nextPnt3P, double *nextXP, double *nextYP, double *nextZP, long *tracePointFoundP, long *exitPointP, long *priorPointP, long *nextPointP ) ; 
int bcdtmDrainage_traceMaximumDescentFromTrianglePointDtmObject( BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP,int zeroSlopeOption,long isFalseLow,double lastAngle, long lastPnt, long startPnt, double startX, double startY, long *nextPnt1P, long *nextPnt2P, long *nextPnt3P, double *nextXP, double *nextYP, double *nextZP, long *processP, long *exitPointP, long *priorPointP, long *nextPointP ) ; 
int bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject( BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP, long priorPnt, long exitPnt, long nextPnt, double startX, double startY, long *nextPnt1P, long *nextPnt2P, long *nextPnt3P, double *nextXP, double *nextYP, double *nextZP, long *processP ) ; 

int bcdtmDrainage_traceMaximumAscentDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMFeatureCallback loadFunctionP,double falseHighElevation,double startX,double startY,void *userP) ;
int bcdtmDrainage_traceToHighPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMFeatureCallback loadFunctionP,double falseHighElevation,long traceOverZeroSlope,long loadFlag,long trgStartType, long pnt1,long pnt2,long pnt3,double X,double Y,double Z,void *userP,long *highPnt1P,long *highPnt2P,double *highXP,double *highYP,double *highZP) ;
int bcdtmDrainage_traceMaximumAscentFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long isFalseHigh,long traceOverZeroSlope,double lastAngle,long pnt1,long pnt2,long pnt3,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,long *tracePointFoundP ) ;
int bcdtmDrainage_traceMaximumAscentFromTrianglePointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long isFalseHigh,long traceOverZeroSlope,double lastAngle,long lastPnt,long startPnt,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,long *tracePointFoundP ) ;

int bcdtmDrainage_scanPointForMaximumAscentDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *ascentTypeP,long *ascentPnt1P,long *ascentPnt2P,double *ascentSlopeP,double *ascentAngleP) ;
int bcdtmDrainage_scanPointForMaximumAscentRidgeLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *ridgePointP,double *ridgeSlopeP) ;
int bcdtmDrainage_scanPointForMaximumAscentTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long excludePoint,long *trgBasePnt1P,long *trgBasePnt2P,double *trgAscentAngleP,double *trgSlopeP) ;

int bcdtmDrainage_scanLineEndPointsForAscentLinesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point1,long point2,DTM_ASCENT_LINE **ascentLinesPP,long *numAscentLinesP) ;
int bcdtmDrainage_ascentLinesSlopeCompareFunction(const void *aline1P,const void *aline2P) ;
int bcdtmDrainage_scanPointForAscentLinesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,DTM_ASCENT_LINE **ascentLinesPP,long *numAscentLinesP) ;
int bcdtmDrainage_scanPointForAscentSumpLinesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,DTM_ASCENT_LINE **ascentSumpLinesPP,long *numAscentSumpLinesP) ;
int bcdtmDrainage_scanPointForAscentRidgeLinesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,DTM_ASCENT_LINE **ridgeLinesPP,long *numRidgeLinesP) ;
int bcdtmDrainage_scanPointForAscentTrianglesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,DTM_ASCENT_LINE **ascentTrianglesPP,long *numAscentTrianglesP ) ;

int bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long linePoint1,long linePoint2,long antPoint,long clkPoint,DTMFeatureType *lineTypeP) ;
int bcdtmDrainage_checkForSumpOrRidgeLineDtmObject (BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP, int linePoint1, int linePoint2, DTMFeatureType& lineType);

// Functions For Tracing Point Catchments

int bcdtmDrainage_traceToSumpLineDtmObject(BC_DTM_OBJ *dtmP,long startType, long pnt1,long pnt2,long pnt3,double x,double y,double z,long *sumpPnt1P,long *sumpPnt2P,double *sumpXP,double *sumpYP,double *sumpZP) ;
int bcdtmDrainage_getFirstTrianglePointFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,long traceDirection,long pnt1,long pnt2,long *nxtPntP)  ;
int bcdtmDrainage_traceToSumpLineFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,double lastAngle,long startPnt1,long startPnt2,long startPnt3,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,long *processP) ;
int bcdtmDrainage_traceToSumpLineFromTrianglePointDtmObject(BC_DTM_OBJ *dtmP,double lastAngle,long lastPnt,long startPnt,double startX,double startY,long *nextPnt1P,long *nextPnt2P,long *nextPnt3P,double *nextXP,double *nextYP,double *nextZP,long *processP) ;
int bcdtmDrainage_getDescentZeroSlopeTriangleToTraceOverDtmObject(BC_DTM_OBJ *dtmP,long point,double lastAngle,long *trgPnt1P,long *trgPnt2P) ;
int bcdtmDrainage_checkTraceToSumpLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long checkOption,double falseLowDepth,bool traceOverZeroSlope,double X,double Y,double Z,double sumpX,double sumpY,double sumpZ,long sumpPoint1,long sumpPoint2,bool& tracedToSump) ;
int bcdtmDrainage_checkTraceToDrainPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,double falseLowDepth,bool traceOverZeroSlope,double X,double Y, double Z,int drainPoint, bool& tracedToDrain) ;
