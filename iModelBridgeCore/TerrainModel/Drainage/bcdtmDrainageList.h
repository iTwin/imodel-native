/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageList.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
int  bcdtmDrainageList_copyTptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,DTMPointList& pointList) ;
int  bcdtmDrainageList_copySptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,DTMPointList& pointList) ;
int  bcdtmDrainageList_copyPointListToTptrListDtmObject(BC_DTM_OBJ *dtmP,DTMPointList&  pointList,long *startPointP) ;
int  bcdtmDrainageList_copyPointListToSptrListDtmObject(BC_DTM_OBJ *dtmP,DTMPointList&  pointList,long *startPointP) ;
int  bcdtmDrainageList_expandTptrPolygonAtPointDtmObject(BC_DTM_OBJ *dtmP,long *pointP,long *extStartPntP,long *extEndPntP) ;
bool bcdtmDrainageList_checkForVoidsInDtmObject(BC_DTM_OBJ *dtmP) ;
