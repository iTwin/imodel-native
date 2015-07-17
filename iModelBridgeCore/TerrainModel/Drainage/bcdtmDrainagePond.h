/*--------------------------------------------------------------------------------------+
|
| $Source: Drainage/bcdtmDrainagePond.h $
|
| $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

class DTMDrainageTables;
struct DTMZeroSlopePolygon;
typedef bvector<DTMZeroSlopePolygon> DTMZeroSlopePolygonVector ;
struct DTM_ZERO_SLOPE_SUMP_LINE;

int bcdtmDrainage_calculatePondDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double falseLowDepth,DTMFeatureCallback loadFunctionP,bool drawPond,bool*pondDeterminedP, double *pondElevationP,double *pondDepthP,double *pondAreaP,double *pondVolumeP,void *userP) ;
int bcdtmDrainage_calculatePondWithLowPointDtmObject (BC_DTM_OBJ *dtmP, double x, double y, double falseLowDepth, DTMFeatureCallback loadFunctionP, bool drawPond, bool* pondDeterminedP, double* pondElevationP, double* pondDepthP, double* pondAreaP, double* pondVolumeP, DPoint3d* lowPtP, void* userP);
int bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject (BC_DTM_OBJ *dtmP, long dtmPnt1, long dtmPnt2, long dtmPnt3, DTMFeatureCallback loadFunctionP, bool loadFlag, bool boundaryFlag, long *exitPointP, long *priorPointP, long *nextPointP, void *userP);
int bcdtmDrainage_traceIslandBoundaryDtmObject(BC_DTM_OBJ *dtmP,double pondElevation,long startPoint,long nextPoint,long mark,DTMFeatureCallback loadFunctionP,bool loadFlag,bool boundaryFlag,DTM_POLYGON_OBJ *polygonP, void* userP) ;
int bcdtmDrainage_tracePondBoundaryDtmObject(BC_DTM_OBJ *dtmP,double pondElevation,long startPoint,long nextPoint,DTMFeatureCallback loadFunctionP,bool loadFlag,bool boundaryFlag,DTM_POLYGON_OBJ *polygonP,DPoint3d **pondPtsPP,long *numPondPtsP,void* userP) ;
int bcdtmDrainage_extractPondBoundaryDtmObject(BC_DTM_OBJ *dtmP,double pondElevation,long startPoint,long nextPoint,DTMFeatureCallback loadFunctionP,bool loadFlag,bool boundaryFlag,DTM_POLYGON_OBJ **polygonPP,void* userP) ;
int bcdtmDrainage_placePolygonAroundZeroSlopeTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,long *startPointP) ;
int bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,DTMFeatureCallback loadFunctionP,bool loadFlag,bool boundaryFlag,long *exitPointP,long *priorPointP,long *nextPointP,DTM_POLYGON_OBJ **polygonPP,void *userP) ;

int bcdtmDrainage_determinePondsDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMFeatureCallback loadFunctionP,bool loadFlag,bool buildTable,void *userP) ;
int bcdtmDrainage_determineLowPointPondsDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector* zeroSlopePolygons,int *zeroSlopePointsIndexP,DTMFeatureCallback loadFunctionP,bool loadFlag,bool buildTable,void *userP,int& numLowPointPonds) ;
int bcdtmDrainage_determinePondAboutLowPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector* zeroSlopePolygons,int *zeroSlopePointsIndexP,DTMFeatureCallback loadFunctionP,long lowPoint,bool loadFlag,bool boundaryFlag,long *exitPointP,long *priorPointP,long *nextPointP,DTM_POLYGON_OBJ **polygonPP,void *userP) ;
int bcdtmDrainage_expandPondToExitPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector *zeroSlopePolygons,int *zeroSlopePointsIndexP,long startPoint,long *exitPointP,long *priorPointP,long *nextPointP);
int bcdtmDrainage_scanPondForExitPointDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long startPoint,double lowPointZ,long *exitPointFoundP,long *exitPointP,long *priorPointP,long *nextPointP) ;
int bcdtmDrainage_testForPondExitPointDtmObject(BC_DTM_OBJ *dtmP,long lowPoint,long *priorPointP,long *nextPointP,long *exitFromPointP) ;
int bcdtmDrainage_expandPondAboutPointDtmObject(BC_DTM_OBJ *dtmP,long lowPoint,double lastArea,long *startPointP,double *areaP,long *extStartPntP,long *extEndPntP) ;

int bcdtmDrainage_determineZeroSlopeSumpLinePondsDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector* zeroSlopePolygons,int *zeroSlopePointsIndexP,DTMFeatureCallback loadFunctionP,bool loadFlag,bool buildTable,void* userP,int& numSumpLinePonds) ;
int bcdtmDrainage_nullZeroSumpLineEntry(DTM_ZERO_SLOPE_SUMP_LINE *zeroSumpLinesP,long numZeroSumpLines,long sumpPoint1,long sumpPoint2) ;
int bcdtmDrainage_getZeroSumpLineOffsets(DTM_ZERO_SLOPE_SUMP_LINE *zeroSumpLinesP,long numZeroSumpLines,long sumpPoint,long *startOffsetP,long *endOffsetP) ;
int bcdtmDrainage_concatenateZeroSlopeSumpLines(DTM_ZERO_SLOPE_SUMP_LINE *zeroSumpLinesP,long numZeroSumpLines,long sumpPtsOffset,DTM_SUMP_LINES **sumpLinesPP,long *numSumpLinesP) ;
int bcdtmDrainage_determinePondAboutZeroSlopeSumpLinesDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector* zeroSlopePolygons,int *zeroSlopePointsIndexP,DTM_SUMP_LINES *sumpLinesP,long numSumpLines,DTMFeatureCallback loadFunctionP,bool loadFlag,bool boundaryFlag,long *exitPointP,long *priorPointP,long *nextPointP,DTM_POLYGON_OBJ **polygonPP,void *userP) ;

int bcdtmDrainage_determineZeroSlopeTrianglePondsDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTMZeroSlopePolygonVector *zeroSlopePolygonsP,int *zeroSlopePointsIndexP,DTMFeatureCallback loadFunctionP,bool loadFlag,bool buildTable,void* userP,int& numZeroSlopeTrianglePonds) ;
int bcdtmDrainage_placePolygonAroundZeroSlopePolygonDtmObject(BC_DTM_OBJ *dtmP,long *firstPointP) ;

int bcdtmDrainage_getSumpLineOffsetDtmObject(BC_DTM_OBJ *dtmP,long sumpPoint1,long sumpPoint2,long *sumpLineOffsetP) ;
int bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,int *zeroSlopePointsIndexP,DTMFeatureCallback loadFunctionP,long sP1,long sP2,bool loadFlag,bool boundaryFlag,long *exitPointP,long *priorPointP,long *nextPointP,DTM_SUMP_LINES **sumpLinesPP,long *numSumpLinesP,DTM_POLYGON_OBJ **polygonPP,void* userP,double* areaP) ;
int bcdtmDrainage_concatenateZeroSlopeSumpLinesDtmObject(BC_DTM_OBJ *dtmP,long sP1,long sP2,DTM_SUMP_LINES **sumpLinesPP,long *numSumpLinesTableP) ;
int bcdtmDrainage_checkForFlowOutFromSumpLinePointsDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,DTM_SUMP_LINES *sumpLinesP,long numSumpLines,long *exitPointP,long *priorPointP,long *nextPointP) ;
int bcdtmDrainage_placePolygonAroundSumpLinesDtmObject(BC_DTM_OBJ *dtmP,DTM_SUMP_LINES *sumpLinesP,long numSumpLines,long *startPointP) ;
int bcdtmDrainage_markZeroSlopeSumpLinesWithinTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPoint,DTM_SUMP_LINES **sumpLinesPP,long *numSumpLinesTableP) ;
int bcdtmDrainage_sumpPointsCompareFunction(const void *c1P,const void *c2P) ;
int bcdtmDrainage_scanSumpPointForSumpLineDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long *sumpPointP,double *slopeP) ;
int bcdtmDrainage_scanSumpPointForMaximumDescentTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,long point,long *trgPnt1P,long *trgPnt2P,double *trgSlopeP) ;

int bcdtmDrainage_markInternalZeroSlopePolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long **zeroSlopePtsPP,long *numMarkedP,long *zeroSlopeFeatureStartP) ;
int bcdtmDrainage_testForZeroSlopePolygonPointDtmObject(BC_DTM_OBJ *dtmP,long point) ;
int bcdtmDraiange_markPointsInternalToAZeroSlopeTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *zeroIndexP,long mark,long *numMarkedP ) ;

int bcdtmDrainage_placeSptrPolygonAroundZeroSlopeTrianglesDtmObject (BC_DTM_OBJ *dtmP, int point1, int point2, DTMDirection& direction, int& startPoint);
int bcdtmDrainage_getUnionOfPolygonsDtmObject(BC_DTM_OBJ *dtmP,int tPtrStartPoint,int sPtrStartPoint,long& startPoint) ;
int bcdtmDrainage_expandPondOverSlopeTrianglesDtmObject(BC_DTM_OBJ *dtmP,long *pondStartPointP,double lowPointZ) ;
int bcdtmDrainage_expandPondToOuterEdgeOfZeroSlopeTrianglesDtmObject(BC_DTM_OBJ *dtmP,long *pondStartPointP,double lowPointZ) ;
int bcdtmDrainage_placeSptrPolygonAtPointAroundZeroSlopeTrianglesDtmObject(BC_DTM_OBJ *dtmP,int point,double lowPointZ,double& sptrArea,DTMDirection& sPtrDirection,int& startPoint) ;
int bcdtmDrainage_countNumberOfZeroSlopeEdgesAtPointDtmObject(BC_DTM_OBJ *dtmP,int point,double lowPointZ,int& numZeroEdges) ;
int bcdtmDrainage_scanPondBoundaryForAZeroEdgeDtmObject(BC_DTM_OBJ *dtmP,long pondStartPoint,double lowPointZ,int& edgePnt1,int& edgePnt2,int& zeroEdgeType) ;
int bcdtmDrainage_validatePondDtmObject(BC_DTM_OBJ *dtmP,long exitPoint,bool& pondValid) ;
int bcdtmDrainage_checkForPondExitPointDtmObject(BC_DTM_OBJ *dtmP,int point,double lowPointZ,bool& exitPoint) ;
int bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObject(BC_DTM_OBJ *dtmP,long pondPoint,double lowPointZ,int& edgePnt1,int& edgePnt2,int& zeroEdgeType ) ;
int bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObjectTwo(BC_DTM_OBJ *dtmP,long priorPondPoint,long pondPoint,long nextPondPoint,double lowPointZ,int& edgePnt1,int& edgePnt2,int& zeroEdgeType ) ;
int bcdtmDrainage_expandPondOverZeroSlopeTrianglesFromZeroEdgeDtmObject( BC_DTM_OBJ *dtmP,long *pondStartPointP,int edgeType,int edgePnt1,int edgePnt2,double lowPointZ ) ;
int bcdtmDrainage_scanPondBoundarySectionForAZeroEdgeDtmObject(BC_DTM_OBJ *dtmP,long pondStartPoint,long pondEndPoint,double lowPointZ,int& edgePnt1,int& edgePnt2,int& zeroEdgeType ) ;
int bcdtmDrainage_countNumberOfExternalZeroEdgesAtPointDtmObject(BC_DTM_OBJ *dtmP,long priorPoint,long pondPoint,long nextPoint,double lowPointZ,int& numZeroEdges) ;
int bcdtmDrainage_polygoniseZeroSlopeTrianglesDtmObject(BC_DTM_OBJ *dtmP,DTMZeroSlopePolygonVector& zeroSlopePolygons) ;
int bcdtmDrainage_markPointsInternalToZeroSlopePolygonsDtmObject(BC_DTM_OBJ *dtmP,DTMZeroSlopePolygonVector& zeroSlopePolygons,int **pointIndexPP) ;
int bcdtmDrainage_indexAllPointsAtZeroSlopeElevationDtmObject(BC_DTM_OBJ *dtmP,DTMZeroSlopePolygonVector *zeroSlopePolygonsP,int *pointIndexP,int zeroSlopePond,int priorPoint,int exitPoint,int nextPoint) ;
int bcdtmDrainage_writeZeroSlopePolygonPointsDtmObject(BC_DTM_OBJ *dtmP,DTMZeroSlopePolygonVector *zeroSlopePolygonsP,int zeroSlopePolygon) ;
int bcdtmDrainage_createDepressionDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **depressionDtmPP,DTMFeatureCallback loadFunctionP,void *userP) ;
