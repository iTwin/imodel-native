/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE
int  bcdtmDrainageList_copyTptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,DTMPointList& pointList) ;
int  bcdtmDrainageList_copySptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,DTMPointList& pointList) ;
int  bcdtmDrainageList_copyPointListToTptrListDtmObject(BC_DTM_OBJ *dtmP,DTMPointList&  pointList,long *startPointP) ;
int  bcdtmDrainageList_copyPointListToSptrListDtmObject(BC_DTM_OBJ *dtmP,DTMPointList&  pointList,long *startPointP) ;
int  bcdtmDrainageList_expandTptrPolygonAtPointDtmObject(BC_DTM_OBJ *dtmP,long *pointP,long *extStartPntP,long *extEndPntP) ;
bool bcdtmDrainageList_checkForVoidsInDtmObject(BC_DTM_OBJ *dtmP) ;
END_BENTLEY_TERRAINMODEL_NAMESPACE
