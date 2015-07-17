/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmMark.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
BENTLEYDTM_Public int bcdtmMark_internalTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long mark,long *numMarkedP ) 
/*
** This Function Marks All Points Internal To A tPtr Polygon.
**
** The tPtr Polygon Must Be Set AntiClockwise
*/
{
 long ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long priorPnt,scanPnt,nextPnt,antPnt,clPnt,clPtr,firstPnt,lastPnt ;
/*
** Initialise
*/
 firstPnt = lastPnt = dtmP->nullPnt ;
 *numMarkedP = 0 ;
/*
** Check Start Point tPtr List Is Not Null
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) 
   {
    bcdtmWrite_message(2,0,0,"Tptr List Start Point Is Null") ; 
    goto errexit ;
   }
/*
** Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal To Tptr Polygon") ;
 priorPnt = startPnt ;
 scanPnt = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    antPnt = nextPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
    while ( antPnt != priorPnt )
      {
       if( nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt )
         {
          nodeAddrP(dtmP,antPnt)->tPtr = mark ;
          ++(*numMarkedP) ;
          clPtr = nodeAddrP(dtmP,antPnt)->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
                else                           { nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ; lastPnt = clPnt ; }
                nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
               }
            }
         } 
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
      }
    priorPnt = scanPnt ;  
    scanPnt = nextPnt ; 
   } while ( priorPnt != startPnt ) ;
/*
** Scan Tptr List And Mark Connected Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tptr List Points") ;
 while ( firstPnt != lastPnt )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->tPtr ;
    nodeAddrP(dtmP,firstPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
    clPtr = nodeAddrP(dtmP,firstPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )  
         { 
		  nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ;
		  lastPnt = clPnt ; 
		  nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
	     }
      }
    firstPnt = nextPnt ;
   }
/*
** Mark Last Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Last Point") ;
 if( lastPnt != dtmP->nullPnt )
   {
    nodeAddrP(dtmP,lastPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_internalTptrPolygonPointsMinMaxDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long mark,long *numMarkedP, long *minPntP , long *maxPntP ) 
/*
** This Function Marks All Points Internal To A tPtr Polygon.
**
** The tPtr Polygon Must Be Set AntiClockwise
*/
{
 long ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long priorPnt,scanPnt,nextPnt,antPnt,clPnt,clPtr,firstPnt,lastPnt ;
/*
** Initialise
*/
 firstPnt = lastPnt = dtmP->nullPnt ;
 *numMarkedP = 0 ;
 *minPntP = dtmP->numPoints - 1 ;
 *maxPntP = 0 ;
/*
** Check Start Point tPtr List Is Not Null
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) 
   {
    bcdtmWrite_message(2,0,0,"Tptr List Start Point Is Null") ; 
    goto errexit ;
   }
/*
** Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal To Tptr Polygon") ;
 priorPnt = startPnt ;
 scanPnt = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    antPnt = nextPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
    while ( antPnt != priorPnt )
      {
       if( nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt )
         {
          nodeAddrP(dtmP,antPnt)->tPtr = mark ;
          ++(*numMarkedP) ;
          if( antPnt < *minPntP ) *minPntP = antPnt ;
          if( antPnt > *maxPntP ) *maxPntP = antPnt ;
          clPtr = nodeAddrP(dtmP,antPnt)->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
                else                           { nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ; lastPnt = clPnt ; }
                nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
               }
            }
         } 
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
      }
    priorPnt = scanPnt ;  
    scanPnt = nextPnt ; 
   } while ( priorPnt != startPnt ) ;
/*
** Scan Tptr List And Mark Connected Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tptr List Points") ;
 while ( firstPnt != lastPnt )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->tPtr ;
    nodeAddrP(dtmP,firstPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
    if( firstPnt < *minPntP ) *minPntP = firstPnt ;
    if( firstPnt > *maxPntP ) *maxPntP = firstPnt ;
    clPtr = nodeAddrP(dtmP,firstPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )  
         { 
		  nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ;
		  lastPnt = clPnt ; 
		  nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
	     }
      }
    firstPnt = nextPnt ;
   }
/*
** Mark Last Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Last Point") ;
 if( lastPnt != dtmP->nullPnt )
   {
    nodeAddrP(dtmP,lastPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
    if( lastPnt < *minPntP ) *minPntP = lastPnt ;
    if( lastPnt > *maxPntP ) *maxPntP = lastPnt ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_externalTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long mark,long *numMarkedP ) 
/*
** This Function Marks All Points External To A tPtr Polygon
** tPtr Polygon Must Be Set AntiClockwise
*/
{
 int  ret=DTM_SUCCESS ;
 long priorPnt,scanPnt,nextPnt,antPnt,clPnt,clPtr,firstPnt,lastPnt ;
/*
** Initialise
*/
 *numMarkedP = 0 ;
 firstPnt = lastPnt = dtmP->nullPnt ;
/*
** Scan Around Tptr Polygon And Mark External Points And Create Internal Tptr List
*/
 priorPnt = startPnt ;
 scanPnt = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    antPnt = nextPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
    if(( antPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
    while ( antPnt != priorPnt )
      {
       if( nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt )
         {
          nodeAddrP(dtmP,antPnt)->tPtr = mark ;
          ++(*numMarkedP) ;
          clPtr = nodeAddrP(dtmP,antPnt)->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
                else                           { nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ; lastPnt = clPnt ; }
                nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
               }
            }
         } 
       if(( antPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
      }
    priorPnt = scanPnt ;  
    scanPnt = nextPnt ; 
   } while ( priorPnt != startPnt ) ;
/*
** Scan Tptr List And Mark Points
*/
 while ( firstPnt != lastPnt )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->tPtr ;
    nodeAddrP(dtmP,firstPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
    clPtr = nodeAddrP(dtmP,firstPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )  
         { 
		  nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ; 
		  lastPnt = clPnt ;
		  nodeAddrP(dtmP,clPnt)->tPtr = clPnt ;
		 }
      }
    firstPnt = nextPnt ;
   }
/*
** Mark Last Point
*/
 if( lastPnt != dtmP->nullPnt )
   {
    nodeAddrP(dtmP,lastPnt)->tPtr = mark ;
    ++(*numMarkedP) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_voidPointsDtmObject(BC_DTM_OBJ *dtmP) 
/*
** This Function Marks Void Points
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  node,dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_NODE *nodeP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Voids Points Dtm Object") ;
/*
** Null Out Tptr Values And Void Bit Settings
*/
 for( node = 0 ; node < dtmP->numPoints ; ++node ) 
   { 
    nodeP = nodeAddrP(dtmP,node) ; 
    nodeP->tPtr = dtmP->nullPnt ;
//    nodeP->PCWD = 0 ;
    bcdtmFlag_clearVoidBitPCWD(&nodeP->PCWD)  ;
   }
/*
** Scan Feature Table And Process All Void Features
*/
 bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Void,TRUE,&dtmFeatureP,&dtmFeature) ;
 while( dtmFeatureP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Void Feature = %8ld",dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ) if( bcdtmMark_pointsInternalToVoidDtmObject(dtmP,dtmFeature)) goto errexit ;
    bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Void,FALSE,&dtmFeatureP,&dtmFeature) ;
   } 
/*
** Scan Feature Table And Process All Hole Features
*/
 bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Hole,TRUE,&dtmFeatureP,&dtmFeature) ;
 while( dtmFeatureP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Void Feature = %8ld",dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )if( bcdtmMark_pointsInternalToVoidDtmObject(dtmP,dtmFeature)) goto errexit ;
    bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Hole,FALSE,&dtmFeatureP,&dtmFeature) ;
   } 
/*
** Scan Feature Table And Process All Island Features
*/
 bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Island,TRUE,&dtmFeatureP,&dtmFeature) ;
 while( dtmFeatureP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Island Feature = %8ld",dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ) if( bcdtmMark_pointsExternalToIslandDtmObject(dtmP,dtmFeature)) goto errexit ;
    bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Island,FALSE,&dtmFeatureP,&dtmFeature) ;
   } 
/*
** Null Out Tptr Values 
*/
 bcdtmList_nullTptrValuesDtmObject (dtmP);
 /*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Voids Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_pointsInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature ) 
/*
** This Function Marks Points Internal To A Void As Void Points.
** voidFeature Is expressed as offset from Start Of Feature Table
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,spnt,numMarked ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Void Feature = %6ld",voidFeature) ;
/*
** Check For Non Null Tptr Values
*/
 if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check For Valid Void Feature
*/
 if( ( ftableAddrP(dtmP,voidFeature)->dtmFeatureType == DTMFeatureType::Void || ftableAddrP(dtmP,voidFeature)->dtmFeatureType == DTMFeatureType::Hole ) && ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt ) 
   {
/*
**  Copy Feature Points To Tptr Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copy Void Feature To Tptr Polygon") ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit   ;
/*
**  Mark Internal Void Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Points") ;
    if( bcdtmMark_voidPointsInternalToTptrVoidPolygonDtmObject(dtmP,spnt,&numMarked))  
/*
**  Mark Points On Void Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points On Void Hull") ;
    sp = spnt ;
    do
      {
       bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ;
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != spnt ) ;
/*
**  Null Out Tptr Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Tptr Void Polygon") ;
    if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit   ;
/*
**  Check For Non Null Tptr Values
*/
    if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Void Feature = %6ld Completed",voidFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Void Feature = %6ld Error",voidFeature) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmMark_voidPointsInternalToTptrVoidPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *numMarkedP ) 
/*
** This Function Marks Void Points Internal To a Tptr.
**
** Rob Cormack June 2003
** 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long nextPnt,priorPnt,listPntnt,listPnt,clc,pnt,firstPnt,lastPnt,hullPoint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon") ;
/*
** Initialise
*/
 *numMarkedP = 0 ;
 firstPnt = lastPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"firstPnt = %9ld lastPnt = %9ld",firstPnt,lastPnt) ;
   } 
 priorPnt= startPnt ;
 pnt = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    listPnt = nextPnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,nextPnt) )
      {
       if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; 
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( listPnt != priorPnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,listPnt))
         {
          if( bcdtmList_testForHullPointDtmObject(dtmP,listPnt,&hullPoint)) goto errexit ;   
          if( nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && ! hullPoint )
            {
             if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = listPnt ;  }
             else                           { nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ; lastPnt = listPnt ; }
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
            }
          if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; ;
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( listPnt != priorPnt )
      {
       listPnt = priorPnt  ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,priorPnt,pnt) )
         {
          if(( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; 
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( listPnt != nextPnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,listPnt,pnt))
            {
             if( bcdtmList_testForHullPointDtmObject(dtmP,listPnt,&hullPoint)) goto errexit ;   
             if( nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && ! hullPoint )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = listPnt ;  }
                else                           { nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ; lastPnt = listPnt ; }
                nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
               }
             if(( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
**   Reset For Next Point On Tptr Polygon
*/
    priorPnt = pnt ;  
    pnt  = nextPnt ; 
   } while ( priorPnt!= startPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %9ld lastPnt = %9ld",firstPnt,lastPnt) ;
 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       listPntnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          listPnt  = clistAddrP(dtmP,clc)->pntNum ;
          clc      = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_testForHullPointDtmObject(dtmP,listPnt,&hullPoint)) goto errexit ;   
          if(nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && ! hullPoint ) 
            { 
             nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ; 
             lastPnt = listPnt ; 
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ; 
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
            }
         }
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = nextPnt ;
      } while ( listPntnt != pnt  ) ;
   }
/*
** Mark And Null Out Internal Tptr List
*/
 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,pnt)->PCWD) ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       ++*numMarkedP ;
       pnt = nextPnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Write Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarkedP) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_pointsExternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature ) 
/*
** This Function Marks Points External To An Island As Void Points.
** islandFeature Is expressed as offset from Start Of Feature Table
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sp,spnt,numMarked ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Island Feature = %6ld",islandFeature) ;
/*
** Check For Non Null Tptr Values
*/
 if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check For Valid Island Feature
*/
 if( ftableAddrP(dtmP,islandFeature)->dtmFeatureType == DTMFeatureType::Island && ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt ) 
   {
/*
**  Copy Feature Points To Tptr Polygon
*/
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,islandFeature,&spnt)) goto errexit ;
/*
**  Mark Internal Island Points
*/
    if( bcdtmMark_voidPointsExternalToTptrPolygonDtmObject(dtmP,spnt,&numMarked)) goto errexit ; 
/*
**  Mark Points On Island Hull As None Void
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points On Island Hull As None Void") ;
    sp = spnt ;
    do
      {
       bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ;
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != spnt ) ;
/*
**  Null Out Tptr Polygon
*/
    if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**  Check For Non Null Tptr Values
*/
    if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmMark_voidPointsExternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *numMarkedP ) 
/*
** This Function Marks Void Points External To a Tptr Polygon.
**
** NOTE : Name Of Function Changed For None Partitioned DTM 
**        None Partitioned DTM Name =  bcdtmMark_voidPointsExternalToTptrIslandPolygonTinObject
** 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long nextPnt,priorPnt,scanPnt,clPnt,clPtr,pnt,firstPnt,lastPnt,hullPoint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon") ;
/*
** Initialise
*/
 *numMarkedP = 0 ;
 firstPnt = lastPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately External Points And Create Internal Tptr List
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Marking Points Immediately External To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"firstPnt = %9ld lastPnt = %9ld",firstPnt,lastPnt) ;
   } 
 priorPnt= startPnt ;
 pnt = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    clPnt = nextPnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,nextPnt,pnt) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,nextPnt) )
      {
       if(( clPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,clPnt)) < 0 ) goto errexit ; ;
/*
**    Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( clPnt != priorPnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,clPnt,pnt) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,clPnt) )
         {
          if( bcdtmList_testForHullPointDtmObject(dtmP,clPnt,&hullPoint)) goto errexit ;   
          if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt && ! hullPoint )
            {
             if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
             else                      { nodeAddrP(dtmP,lastPnt)->tPtr = -(clPnt+1) ; lastPnt = clPnt ; }
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld External To %6ld ** firstPnt = %9ld lastPnt = %9ld",clPnt,pnt,firstPnt,lastPnt) ;
            }
          if(( clPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,clPnt)) < 0 ) goto errexit ; ;
         }
      }
/*
**  Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( clPnt != priorPnt )
      {
       clPnt = priorPnt ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,priorPnt) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,priorPnt,pnt) )
         {
          if(( clPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,clPnt)) < 0 ) goto errexit ; ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( clPnt != nextPnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,clPnt) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,clPnt,pnt) )
            {
             if( bcdtmList_testForHullPointDtmObject(dtmP,clPnt,&hullPoint)) goto errexit ;   
             if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt && ! hullPoint )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
                else                      { nodeAddrP(dtmP,lastPnt)->tPtr = -(clPnt+1) ; lastPnt = clPnt ; }
                nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld External To %6ld ** firstPnt = %9ld lastPnt = %9ld",clPnt,pnt,firstPnt,lastPnt) ;
               }
             if(( clPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,clPnt)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
**  Reset For Next Point On Tptr Polygon
*/
    priorPnt = pnt ;  
    pnt  = nextPnt ; 
   } while ( priorPnt!= startPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %9ld lastPnt = %9ld",firstPnt,lastPnt) ;
 if( firstPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To External Marked Points") ;
    pnt = firstPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       scanPnt = pnt ;
       clPtr = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clPtr != dtmP->nullPtr )
         {
          clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( bcdtmList_testForHullPointDtmObject(dtmP,clPnt,&hullPoint)) goto errexit ;   
          if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt && ! hullPoint ) 
            { 
             if( dbg ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld External To %6ld ** firstPnt = %9ld lastPnt = %9ld",clPnt,pnt,firstPnt,lastPnt) ;
             nodeAddrP(dtmP,lastPnt)->tPtr = -(clPnt+1) ; 
             lastPnt = clPnt ; 
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ; 
            }
         }
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = nextPnt ;
      } while ( scanPnt != pnt  ) ;
   }
/*
** Mark Void Points And Null Out Internal Tptr List
*/
 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,pnt)->PCWD) ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       ++*numMarkedP ;
       pnt = nextPnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Write Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarkedP) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_setLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,unsigned char *lineFlagP)
{
 long  cPtr ;
/*
** Get Line Offset
*/
 cPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( cPtr != dtmP->nullPtr  )
   {
    if( clistAddrP(dtmP,cPtr)->pntNum == pnt2 ) 
      { 
	   bcdtmFlag_setFlag(lineFlagP,cPtr) ;
	   return(0) ; 
	  }
    cPtr = clistAddrP(dtmP,cPtr)->nextPtr ;
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_testLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,unsigned char *lineFlagP)
{
 long  cPtr ;
/*
** Get Line Offset
*/
 cPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( cPtr != dtmP->nullPtr )
   {
    if( clistAddrP(dtmP,cPtr)->pntNum == pnt2 )  return (bcdtmFlag_testFlag(lineFlagP,cPtr))  ; 
    cPtr = clistAddrP(dtmP,cPtr)->nextPtr ;
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_directionalVoidLinesExternalToTptrIslandPolygonDtmObject(BC_DTM_OBJ *dtmP,long sPnt,long *tinLinesP,long mark,long *numMarked ) 
/*
** This Function marks directional void lines exnternal To a tPtr Island Polygon.
** Directional means That Line P1P2 is different To Line P2P1
** This Is Used For Resolving Overlapping Polygonal DTM Features .
** The Void Lines Are Marked In This Manner For Polygonisation Purposes
**
** Rob Cormack June 2003
** 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long npnt,ppnt,lpnt,lp,clc,pnt,fPnt,lPnt,offset ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines External To Tptr Island Polygon") ;
/*
** Initialise
*/
 *numMarked = 0 ;
 fPnt = lPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,sPnt) ;
 if( nodeAddrP(dtmP,sPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately External Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately External To Tptr Polygon") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ; 
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,npnt,pnt) )
      {
       if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
            }
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
         }
      }
/*
**  Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,ppnt) )
         {
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
               }
             if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
** Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;  
    pnt  = npnt ; 
   } while ( ppnt!= sPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 if( fPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To External Marked Points") ;
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum  ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) ) 
            { 
             if( dbg ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
             nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; 
             lPnt = lp ; 
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ; 
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Mark Lines Connected To Marked Points
*/
 if( fPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Marked Points ") ;
    if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt ) ;
   }
/*
** Scan Tptr Polygon And Mark Lines Immediately External To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking External Lines Connected To Tptr Polygon") ;
 ppnt = sPnt ;
 pnt = nodeAddrP(dtmP,ppnt)->tPtr ; 
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,npnt,pnt) )
      {
       if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
         }
/*
**     Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
       if( lp != ppnt )
         {
          lp = ppnt  ;
          if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,ppnt) )
            {
             if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
/*
**           Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
             while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
               {
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
                *(tinLinesP+offset) = mark ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
                *(tinLinesP+offset) = mark ;
                ++*numMarked ; 
                if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
               }
            }
         }
      }
    ppnt = pnt ;
    pnt = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Scan Tptr Polygon And Mark Outside Of Tptr Polygon Lines 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking External Edge Tptr Polygon") ;
 pnt = sPnt ;
 do
   {
    npnt = nodeAddrP(dtmP,pnt)->tPtr ; 
    if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,npnt)) goto errexit ;
    *(tinLinesP+offset) = mark ;
    pnt = npnt ;
   } while ( pnt!= sPnt ) ;
/*
** Null Out Internal Tptr List
*/
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       pnt = npnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines External To Tptr Island Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines External To Tptr Island Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_directionalVoidLinesInternalToTptrVoidPolygonDtmObject(BC_DTM_OBJ *dtmP,long sPnt,long *tinLinesP,long mark,long *numMarked ) 
/*
** This Function marks directional void lines internal To a tPtr Void Polygon.
** Directional means That Line P1P2 is different To Line P2P1
** This Is Used For Resolving Overlapping Polygonal DTM Features .
** The Void Lines Are Marked In This Manner For Polygonisation Purposes
**
** Rob Cormack June 2003
** 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long npnt,ppnt,lpnt,lp,clc,pnt,fPnt,lPnt,offset ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines Internal To Tptr Void Polygon") ;
/*
** Initialise
*/
 *numMarked = 0 ;
 fPnt = lPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,sPnt) ;
 if( nodeAddrP(dtmP,sPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ; 
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,npnt,pnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; 
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z,pnt,fPnt,lPnt) ;
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,ppnt) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; 
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z,pnt,fPnt,lPnt) ;
               }
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
** Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;  
    pnt  = npnt ; 
   } while ( ppnt!= sPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) ) 
            { 
             nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; 
             lPnt = lp ; 
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ; 
             if( dbg ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z,pnt,fPnt,lPnt) ;
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Mark Lines Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Marked Points ") ;
 if( fPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt ) ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarked) ;
/*
** Scan Tptr Polygon And Mark Lines Immediately Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Internal To Tptr Polygon") ;
 ppnt = sPnt ;
 pnt = nodeAddrP(dtmP,ppnt)->tPtr ; 
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,npnt,pnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; 
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
         }
/*
**     Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
       if( lp != ppnt )
         {
          lp = ppnt  ;
          if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,ppnt) )
            {
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
/*
**           Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
             while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
               {
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
                *(tinLinesP+offset) = mark ;
                ++*numMarked ; 
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
                *(tinLinesP+offset) = mark ;
                ++*numMarked ; 
                if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ; ;
               }
            }
         }
      }
    ppnt = pnt ;
    pnt = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Scan Tptr Polygon And Mark Inside Of Tptr Polygon Lines 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Edge Tptr Polygon") ;
 pnt = sPnt ;
 do
   {
    npnt = nodeAddrP(dtmP,pnt)->tPtr ; 
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,npnt))
      {
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,pnt)) goto errexit ;
       *(tinLinesP+offset) = mark ;
       ++*numMarked ; 
      }  
    pnt = npnt ;
   } while ( pnt!= sPnt ) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarked) ;
/*
** Null Out Internal Tptr List
*/
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       pnt = npnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines Internal To Tptr Void Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Directional Void Lines Internal To Tptr Void Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_directionalTinLinesInternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long sPnt,long *tinLinesP,long mark,long *numMarked ) 
/*
** This Function marks directional tin lines internal To a tPtr Polygon.
** Directional means That Line P1P2 is different To Line P2P1
** The Tin Lines Are Marked In This Manner For Polygonisation Purposes
**
** The tPtr Polygon Must Be AntiClockwise
**
** sPnt = start Point Of Tptr Polygon List
** fPnt = first Point Of Internal Tptr List Of Marked Points
** lPnt = last  Point Of Internal Tptr List Of Marked Points
** npnt = next  point in list
** ppnt = prior point in list
** apnt = next anti clockwise point
** 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long npnt,apnt,ppnt,lpnt,lp,clc,pnt,fPnt,lPnt,offset ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tin Lines Internal To Tptr Polygon") ;
/*
** Initialise
*/
 *numMarked = 0 ;
 fPnt = lPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,sPnt) ;
 if( nodeAddrP(dtmP,sPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ; 
 do
   {
    apnt = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if(( apnt = bcdtmList_nextAntDtmObject(dtmP,pnt,apnt)) < 0 ) goto errexit ; ;
/*
** Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
    while ( apnt != ppnt)
      {
       if( nodeAddrP(dtmP,apnt)->tPtr == dtmP->nullPnt )
         {
          if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = apnt ;  }
          else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(apnt+1) ; lPnt = apnt ; }
          nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking Point %6ld Internal To %6ld ** fPnt = %9ld lPnt = %9ld",apnt,pnt,fPnt,lPnt) ;
         }
       if(( apnt = bcdtmList_nextAntDtmObject(dtmP,pnt,apnt)) < 0 ) goto errexit ; ;
      }
/*
** Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;  
    pnt  = npnt ; 
   } while ( ppnt!= sPnt ) ;
/*
** Scan Internal Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )  
            { 
             nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; 
             lPnt = lp ; 
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ; 
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Mark Lines Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Marked Points ") ;
 if( fPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,lp)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,lp,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt ) ;
   }
/*
** Scan Tptr Polygon And Mark Lines 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Tptr Polygon") ;
 ppnt = sPnt ;
 pnt = nodeAddrP(dtmP,ppnt)->tPtr ; 
 do
   {
    apnt = npnt  = nodeAddrP(dtmP,pnt)->tPtr ;
    if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,pnt)) goto errexit ;
    *(tinLinesP+offset) = mark ;
    ++*numMarked ; 
    if(( apnt = bcdtmList_nextAntDtmObject(dtmP,pnt,apnt)) < 0 ) goto errexit ; 
    while ( apnt != ppnt)
      {
       if( nodeAddrP(dtmP,apnt)->tPtr >= 0 )
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,apnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,apnt,pnt)) goto errexit ;
          *(tinLinesP+offset) = mark ;
          ++*numMarked ; 
         }
       if(( apnt = bcdtmList_nextAntDtmObject(dtmP,pnt,apnt)) < 0 ) goto errexit ; ;
      }
    ppnt = pnt ;
    pnt = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Null Out Internal Tptr List
*/
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       pnt = npnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Lines Internal To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Lines Internal To Tptr Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_setVoidPointsDtmObject
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Tin Object              */
 long   initOption,        /* ==> Unvoid All Tin Points <TRUE,FALSE> */ 
 long   startFeature,      /* ==> Scan From This Feature             */
 long   *numVoidPtsP       /* <== Number Of Void Points Set          */
 ) 
{
/*
**  This Function Can Be Used For Both Normal And Display Tins
**  For Normal  Tins ** Set  initOption = TRUE  and startFeature = 0 
**  For Display Tins ** Set  initOption = FALSE and startFeature to first Void/Island Feature In Tin representing An Object Boundary
**
*/
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  node,clPtr,antPnt,clkPnt,firstPnt,lastPnt ; 
 long  priorPnt,scanPnt,nextPnt,dtmFeature,onHull ;
 DTM_TIN_NODE   *nodeP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Setting Void Points") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"initOption   = %8ld",initOption)  ;
    bcdtmWrite_message(0,0,0,"startFeature = %8ld",startFeature)  ;
    bcdtmWrite_message(0,0,0,"*numVoidPtsP = %8ld",*numVoidPtsP)  ;
   }
/*
** Initialise
*/
 *numVoidPtsP = 0 ;
 firstPnt = dtmP->nullPnt ;
 lastPnt  = dtmP->nullPnt ;
/*
**  Mark All Points As None Void And Null Tin List Pointers
*/
 for( node = 0 ; node < dtmP->numPoints ; ++node )  
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( initOption == TRUE ) bcdtmFlag_clearVoidBitPCWD(&nodeP->PCWD) ;
    nodeP->tPtr = dtmP->nullPnt ;
    nodeP->sPtr = dtmP->nullPnt ;
   }
/*
** Mark All Points Immediately External To Island Hulls And Internal To Void Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Island And Void Features") ;   
 for( dtmFeature = startFeature ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Check For Island Feature
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
/*
**     Initialise Points For Scanning Island Hull
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking All Points External To Island Feature %6ld",dtmFeature) ; 
       priorPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,priorPnt,&scanPnt)) goto errexit ;   
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,scanPnt,&nextPnt))  goto errexit ; 
/*
**     Scan From Prior Point To Next Point Marking Void Points
*/
       do 
         {
/*
**        Check For Island Hull Line Coincident With Void Hull Line
*/
          if( ! bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,priorPnt,scanPnt) &&
              ! bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,scanPnt,nextPnt)       )
            { 
          
/*
**           Write Out Position On Island Hull
*/
             if( dbg == 2 )
               {
                bcdtmWrite_message(0,0,0,"Island Scan Points") ;
                bcdtmWrite_message(0,0,0,"priorPnt = %8ld ** %12.5lf %12.5lf %10.4lf",priorPnt,pointAddrP(dtmP,priorPnt)->x,pointAddrP(dtmP,priorPnt)->y,pointAddrP(dtmP,priorPnt)->z ) ;
                bcdtmWrite_message(0,0,0,"scanPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",scanPnt,pointAddrP(dtmP,scanPnt)->x,pointAddrP(dtmP,scanPnt)->y,pointAddrP(dtmP,scanPnt)->z ) ;
                bcdtmWrite_message(0,0,0,"nextPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z ) ;
               } 
             if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,priorPnt)) < 0 ) goto errexit ;
             while( antPnt != nextPnt )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** antPnt = %8ld ** %12.5lf %12.5lf %10.4lf",antPnt,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z ) ;
/*
**              Check If Point Has Already Been Set
*/
                if( nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt ) 
                  {
/*
**                 If Not Island Or Void Hull Set Point
*/
                   if( ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,antPnt)  &&
                       ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,antPnt)    &&
                       ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Hole,startFeature,antPnt)        )
                     {

                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"##### Marking antPnt") ;
                      if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = antPnt ; }
                      else                         { nodeAddrP(dtmP,lastPnt)->tPtr = antPnt ; lastPnt = antPnt ; }
                      nodeAddrP(dtmP,lastPnt)->tPtr = lastPnt ;
                     }
/*
**                 Test For Line On Void Hull
**                 RoC 30/10/06  - Following Lines Of Code Need To Be Enhanced A Bit More
**                 To Take Account Of Coincident Multiple Features
*/
                   else 
                     {
                      if( bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,scanPnt,antPnt) ||
                          bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Hole,startFeature,scanPnt,antPnt)    )
                        {
/*
**                       Scan Back From Next Point To First Hull Line
*/
                         if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,nextPnt)) < 0 ) goto errexit ;
                         while ( clkPnt != antPnt && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,scanPnt,clkPnt) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,clkPnt,scanPnt) )
                           {
                            if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,clkPnt)) < 0 ) goto errexit ;
                           }
                         antPnt = clkPnt ;
                        }
                     }
                  }
/*
**              Set Next Ant Point
*/
                if( antPnt != nextPnt ) if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ; 
               } 
            }
/*
**        Move To Next Point On Feature
*/
          priorPnt = scanPnt ;
          scanPnt  = nextPnt ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,scanPnt,&nextPnt))  goto errexit ;  
         } while ( priorPnt != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      } 
/*
**  Check For Void Feature
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking All Points Internal To Void   Feature %6ld",dtmFeature) ;   
       priorPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,priorPnt,&scanPnt)) goto errexit ;   
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,scanPnt,&nextPnt))  goto errexit ;  
/*
**     Scan From Prior Point To Next Point In A Clockwise Direction Marking Void Points
*/
       do 
         {
          if( ! bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,priorPnt,scanPnt) &&
              ! bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,scanPnt,nextPnt)       )
            { 
/*
**           Write Out Position On Island Hull
*/
             if( dbg == 2 )
               {
                bcdtmWrite_message(0,0,0,"Void Scan Points") ;
                bcdtmWrite_message(0,0,0,"priorPnt = %8ld ** %12.5lf %12.5lf %10.4lf",priorPnt,pointAddrP(dtmP,priorPnt)->x,pointAddrP(dtmP,priorPnt)->y,pointAddrP(dtmP,priorPnt)->z ) ;
                bcdtmWrite_message(0,0,0,"scanPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",scanPnt,pointAddrP(dtmP,scanPnt)->x,pointAddrP(dtmP,scanPnt)->y,pointAddrP(dtmP,scanPnt)->z ) ;
                bcdtmWrite_message(0,0,0,"nextPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z ) ;
               } 
             if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,priorPnt)) < 0 ) goto errexit ;
             while( clkPnt != nextPnt )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** clkPnt = %8ld ** %12.5lf %12.5lf %10.4lf",clkPnt,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z ) ;
/*
**              Check If Point Has Already Been Set
*/
                if( nodeAddrP(dtmP,clkPnt)->tPtr == dtmP->nullPnt ) 
                  {
/*
**                 If Not Island Or Void Hull Set Point
*/
                   if( ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,clkPnt)  &&
                       ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,clkPnt)    &&
                       ! bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Hole,startFeature,clkPnt)         )
                     {
                      if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clkPnt ; }
                      else                          { nodeAddrP(dtmP,lastPnt)->tPtr = clkPnt ; lastPnt = clkPnt ; }
                      nodeAddrP(dtmP,lastPnt)->tPtr = lastPnt ;
                     }
/*
**                 Test For Line On Island Hull
**                 RoC 30/10/06  - Following Lines Of Code Need To Be Enhanced A Bit More
**                 To Take Account Of Coincident Multiple Features
*/
                   else 
                     {
                      if( bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,scanPnt,clkPnt))
                        {
/*
**                       Scan Back From Next Point To First Hull Line
*/
                         if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,nextPnt)) < 0 ) goto errexit ;
                         while ( antPnt != clkPnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,scanPnt,antPnt) && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,antPnt,scanPnt) )
                           {
                            if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
                           }
                         clkPnt = antPnt ;
                        }
                     }
                  }
/*
**              Set Next Clock Point
*/
                if( clkPnt != nextPnt ) if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,clkPnt)) < 0 ) goto errexit ; 
               } 
            }
/*
**        Move To Next Point On Feature
*/
          priorPnt = scanPnt ;
          scanPnt  = nextPnt ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,scanPnt,&nextPnt))  goto errexit ;  
         } while ( priorPnt != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
   } 
/*
** Scan Tptr List And Set Void Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tptr List And Setting Void Points") ;
 while ( firstPnt != lastPnt )
   {
    ++*numVoidPtsP ;
    bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,firstPnt)->PCWD) ;
    nextPnt = nodeAddrP(dtmP,firstPnt)->tPtr ;
    clPtr   = nodeAddrP(dtmP,firstPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       scanPnt = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
       onHull  = 0 ; 
       if( bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Island,startFeature,scanPnt)) onHull = 1 ;
       if( ! onHull ) if( bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Void,startFeature,scanPnt)) onHull = 1  ;
       if( ! onHull ) if( bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject(dtmP,DTMFeatureType::Hole,startFeature,scanPnt)) onHull = 1 ;
       if(nodeAddrP(dtmP,scanPnt)->tPtr == dtmP->nullPnt && ! onHull )  
         { 
          nodeAddrP(dtmP,lastPnt)->tPtr = scanPnt ; 
          lastPnt = scanPnt ; 
          nodeAddrP(dtmP,scanPnt)->tPtr = scanPnt ; 
         }
      }
    firstPnt = nextPnt ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Void Points = %6ld",*numVoidPtsP) ;
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Void Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Void Points Eror") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_testForPointOnDtmFeatureTypeEqualOrAboveStartFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 long startFeature,
 long point
)
/*
** This Function Tests If point Is On A DTM Feature Type Equal Or Above The Start Feature
*/
{
 int  dbg=DTM_TRACE_VALUE(0)  ;
 long fPtr   ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Test For Point On Dtm Feature Type Equal Or Above Start Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"startFeature   = %8ld",startFeature) ;
    bcdtmWrite_message(0,0,0,"point          = %8ld",point) ;
   } 
/*
** Scan All Features For Point
*/
 if( point >= 0 && point < dtmP->numPoints ) 
   {
    if(nodeAddrP(dtmP,point)->fPtr != dtmP->nullPtr ) 
      { 
/*
**     Test For Hull Feature
*/
       if( dtmFeatureType == DTMFeatureType::Hull && nodeAddrP(dtmP,point)->fPtr != dtmP->nullPnt ) return(1)  ;
/*
**     Scanning Point Feature List 
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature List For Point") ;
       if( ( fPtr = nodeAddrP(dtmP,point)->fPtr ) != dtmP->nullPtr )
         {
          while ( fPtr != dtmP->nullPtr )
            {
             if( flistAddrP(dtmP,fPtr)->dtmFeature >= startFeature && ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) 
               {
                if( dbg )  bcdtmWrite_message(0,0,0,"Test For Point On Dtm Feature Type Equal Or Above Start Feature ** Passed") ;
                return(1)  ;
               }
             fPtr  = flistAddrP(dtmP,fPtr)->nextPtr ; 
            }
         }
      }
   }
/*
** Job Completed
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Test For Point On Dtm Feature Type Equal Or Above Start Feature ** Failed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_testForDtmFeatureTypeLineEqualOrAboveStartFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 long startFeature,
 long point1,
 long point2
)
/*
** This Function Tests If The Line Point1-Point2 Is A Dtm Feature Type Line
*/
{
 long clnPtr ;
/*
** Test For Dtm Feature Type Line Point1-Point2
*/
 clnPtr = nodeAddrP(dtmP,point1)->fPtr ;
 while ( clnPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clnPtr)->nextPnt == point2 )
      {
       if( flistAddrP(dtmP,clnPtr)->dtmFeature >= startFeature && ftableAddrP(dtmP,flistAddrP(dtmP,clnPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1)  ;
      }
    clnPtr = flistAddrP(dtmP,clnPtr)->nextPtr ;  
   } 
/*
** Test For Dtm Feature Type Line Point2-Point1
*/
 clnPtr = nodeAddrP(dtmP,point2)->fPtr ;
 while ( clnPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clnPtr)->nextPnt == point1 )
      {
       if( flistAddrP(dtmP,clnPtr)->dtmFeature >= startFeature && ftableAddrP(dtmP,flistAddrP(dtmP,clnPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1)  ;
      }
    clnPtr = flistAddrP(dtmP,clnPtr)->nextPtr ;  
   } 
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_prgnControlWordForFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 long       mark,
 long       *numMarkedP 
) 

// This Function Marks All Points On A DTM Polygon Feature

{
 int  ret=DTM_SUCCESS ;
 long sp,spnt ;
 BC_DTM_FEATURE *dtmFeatureP ;

// Initialise

 *numMarkedP = 0 ;

// Test For Legal Feature

 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {

//     Scan Feature And Mark PRGN Control Word

       spnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       sp = spnt ;
       nodeAddrP(dtmP,sp)->PRGN = ( unsigned short ) mark ; 
       ++*numMarkedP ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp)) goto errexit  ; 
       while ( sp != spnt && sp != dtmP->nullPnt ) 
         {
          nodeAddrP(dtmP,sp)->PRGN = ( unsigned short ) mark ; 
          ++*numMarkedP ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp)) goto errexit  ; 
         } 
      }
   }      
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_internalPolygonPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long dtmFeature,
 long       mark,
 long       *numMarkedP 
) 
/*
** This Function Marks All Points Internal To a DTM Polygon Feature
** The Polygon Feature Must Be AntiClockwise
*/
{
 int  ret=DTM_SUCCESS ;
 long pp,sp,np,ap,lp,clc,firstPnt,lastPnt,spnt ;
 BC_DTM_FEATURE *dtmFeatureP ;

// Initialise

 *numMarkedP = 0 ;
 firstPnt = lastPnt = spnt = dtmP->nullPnt ;

// Test For Legal Feature

 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {

       //  Scan Around dtmFeature Polygon And mark Internal Points And Create Internal Tptr List
       
       pp = spnt ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,pp,&sp)) goto errexit ; 
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
          ap =  np ;
          if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
          while ( ap != pp )
            {
             if( nodeAddrP(dtmP,ap)->tPtr != mark && ! bcdtmList_testIfPointOnDtmFeatureDtmObject(dtmP,ap,dtmFeature) )
               {
                nodeAddrP(dtmP,ap)->tPtr = mark ;
                ++(*numMarkedP) ;
               } 
             if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
            }
          pp = sp ;  
          sp = np ; 
         } while ( pp != spnt ) ;

//      Scan Internal Tptr List And Internal Points

        while ( firstPnt != lastPnt )
          {
           np = nodeAddrP(dtmP,firstPnt)->tPtr ;
           nodeAddrP(dtmP,firstPnt)->tPtr = mark ;
           ++(*numMarkedP) ;
           clc = nodeAddrP(dtmP,firstPnt)->cPtr ;
           while( clc != dtmP->nullPtr )
             {
              lp  = clistAddrP(dtmP,clc)->pntNum ;
              clc = clistAddrP(dtmP,clc)->nextPtr ;
              if( nodeAddrP(dtmP,lp)->tPtr != mark && ! bcdtmList_testIfPointOnDtmFeatureDtmObject(dtmP,lp,dtmFeature) )  
                { 
                 nodeAddrP(dtmP,lastPnt)->tPtr = lp ; 
                 lastPnt = lp ; 
                 nodeAddrP(dtmP,lp)->tPtr = lp ; 
                }
             }
           firstPnt = np ;
          }

//      Mark Last Point

        if( lastPnt != dtmP->nullPnt )
          {
           nodeAddrP(dtmP,lastPnt)->tPtr = mark ;
           ++(*numMarkedP) ;
          }
       }
    }      

// Clean Up

 cleanup :

// Return

 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
