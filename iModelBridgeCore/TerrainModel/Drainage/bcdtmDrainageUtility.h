/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageUtility.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
int bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(BC_DTM_OBJ *dtmP, DTMDrainageTables *drainageTablesP,int trgPoint1,int trgPoint2,int trgPoint3,bool& voidTriangle,int& flowDirection ) ;
int bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject( BC_DTM_OBJ *dtmP,DTMDrainageTables *drainageTablesP,int trgPoint1,int trgPoint2,int trgPoint3,bool &voidTriangle,double& slope,double& descentAngle,double& ascentAngle) ;
int bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(BC_DTM_OBJ *dtmP,double sRadX,double sRadY,double eRadX,double eRadY,long dtmPnt1,long dtmPnt2,double *xP,double *yP,double *zP,long *intPntP) ;
int bcdtmDrainage_intersectCordLines(double X1,double Y1,double X2,double Y2,double X3,double Y3,double X4,double Y4,double *X5,double *Y5) ;
int bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double *descentAngleP,double *ascentAngleP,double *trgSlopeP) ;
int bcdtmDrainage_getTriangleFlowDirectionDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3 ) ;
int bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(BC_DTM_OBJ *dtmP,long apexPnt,long basePnt1,long basePnt2,double angle,double *xP,double *yP,double *zP,long *intPntP) ;
int bcdtmDrainage_internallyCleanPointArrayPolygon(DPoint3d **polyPtsPP,long *numPolyPtsP,double ppTol) ;
int bcdtmDrainage_calculateAngleIntersectOfRadialFromTriangleEdgeWithTriangleDtmObject(BC_DTM_OBJ *dtmP,long startPnt1,long startPnt2,long startPnt3,double startX,double startY,double angle,double *xP,double *yP,double *zP,long *nextPnt1P,long *nextPnt2P,long *nextApexP) ;
int bcdtmDrainage_insertPointIntoTinLineDtmObject(BC_DTM_OBJ *dtmP,long point,long linePoint1,long linePoint2) ;

