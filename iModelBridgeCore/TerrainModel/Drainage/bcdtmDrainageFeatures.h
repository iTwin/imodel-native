/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageFeatures.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

int bcdtmDrainage_returnLowPointsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numLowPts) ;
int bcdtmDrainage_returnNoneFalseLowPointsDtmObject(BC_DTM_OBJ *dtmP,double falseLowDepth,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numLowPts) ;
int bcdtmDrainage_returnHighPointsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numHightPts) ;
int bcdtmDrainage_returnSumpLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType, DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numSumpLines ) ;
int bcdtmDrainage_returnZeroSlopeSumpLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType, DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numSumpLines ) ;
int bcdtmDrainage_returnRidgeLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType, DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int& numRidgeLines ) ;
int bcdtmDrainage_returnZeroSlopePolygonsDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,bool useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,const DPoint3d *fencePtsP,int numFencePts,void *userP,int&  numZeroSlopePolygons) ;
