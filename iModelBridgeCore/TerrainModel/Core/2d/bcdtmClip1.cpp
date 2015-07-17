/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmClip1.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_internalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long fillOption)
/*
** This Function Internally Clips A Dtm Object
** fillOption  = 0  Do Not Fill Clipped Section With Triangles 
**                  This is a normally only a merge requirement
**             = 1  Fill Clipped Section With Void Triangles
**                  Normal clipping operation
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   sp,cp,np,lp,node,clPtr,numMarked,mark=-98989898 ;
 DTMDirection direction;
 double area ;
 DTM_TIN_NODE  *nodeP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Internal To Tptr Polygon")  ;
/*
** Check Direction Of Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction Of Tptr Polygon") ;
 bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Mark Dtm Points Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Tptr Polygon") ;
 if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPnt,mark,&numMarked)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number marked = %6ld of %6ld",numMarked,dtmP->numPoints) ;
 for( node = 0 ; node < dtmP->numPoints ; ++node ) 
   {
    nodeP = nodeAddrP(dtmP,node) ;
    nodeP->PRGN = 0 ; 
    if( nodeP->tPtr == mark ) nodeP->PRGN = 1 ;
   }
/*
** Clip Dtm Features Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Features Internal To Clip Polygon")  ;
 if( bcdtmClip_dtmFeaturesInternalToTptrPolygonDtmObject(dtmP,startPnt,mark)) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Features Internal To Clip Polygon Completed")  ;
/*
** Check Topology Of DTM Features 
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"DTM Feature Topology Errors") ; goto errexit  ; }
    else                                                   bcdtmWrite_message(0,0,0,"DTM Feature Topology  OK") ;
   } 
/*
** Delete All Lines Connecting To Internal Dtm Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Connecting To Internal Points") ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->PRGN || nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       clPtr = nodeAddrP(dtmP,sp)->cPtr ;
       while ( clPtr != dtmP->nullPtr )
	     {
	      cp  = clistAddrP(dtmP,clPtr)->pntNum ;
	      clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
	      if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp) ) goto errexit  ;
	     }
       nodeP = nodeAddrP(dtmP,sp)  ;
       nodeP->hPtr = dtmP->nullPnt ;
       nodeP->tPtr = dtmP->nullPnt ;
       nodeP->sPtr = dtmP->nullPnt ;
       nodeP->cPtr = dtmP->nullPtr ;
       nodeP->fPtr = dtmP->nullPtr ;
      }
   }
/*
** Delete All Internal Lines Connected To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Internal To Clip Polygon")  ;
 sp = startPnt ; 
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit  ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit  ;
       if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit  ;
       cp = lp ;
      }
    sp = np ; 
   } while( sp != startPnt ) ;
/*
** Fill Clipped Section With Triangles
*/
 if( fillOption )
   {
/*
**  Fill Tptr Polygon With Triangles 
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Filling Tptr Polygon With Triangles")  ;
    if( bcdtmClip_fillTptrPolygonWithTrianglesDtmObject(dtmP,startPnt)) goto errexit  ;
/*
**  Set Void Bits
*/
 //   if( bcdtmMark_voidPointsDtmObject(dtmP)) goto errexit ;
/*
**  Insert As Void  
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Clip Void")  ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,startPnt,1)) goto errexit  ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Inserted Void") ;
    if( bcdtmList_checkConnectivityOfDtmFeatureDtmObject(dtmP,dtmP->numFeatures-1,1)) 
      {
       bcdtmWrite_message(2,0,0,"Connectivity Error In Inserted Void") ;
       goto errexit  ;
      }
    else if( dbg ) bcdtmWrite_message(0,0,0,"Inserted Void Connectivity OK") ;
/*
**  Resolve Adjoining Void Features
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Adjoining Polygonal Features")  ;
    if( bcdtmClip_findAndMergeAdjoiningVoidsDtmObject(dtmP,dtmP->numFeatures-1)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Inserted Void Internal To Another Void")  ;
    if( bcdtmClip_checkForInsertedVoidInternalToVoidDtmObject(dtmP)) goto errexit  ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Internal To Tptr Polygon Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Internal To Tptr Polygon Error")  ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
