/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLoadRegions.cpp $
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
int bcdtmMark_checkForPointOnRegionHullDtmObject(BC_DTM_OBJ* dtmP, long pnt)
{
 long flistPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Scan Feature List For Point
*/
 flistPtr = nodeAddrP(dtmP,pnt)->fPtr ;
 while( flistPtr != dtmP->nullPtr )
   {
    dtmFeatureP = ftableAddrP(dtmP,flistAddrP(dtmP,flistPtr)->dtmFeature);
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region ) return(1) ;
    flistPtr = flistAddrP(dtmP,flistPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmMark_internalRegionPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long regionOption,      // ==>  Region Option < 1 Include Internal Regions , 2 Exclude Internal Regions >
 long startPnt,
 long mark,
 long *numMarkedP,
 long *minPntP,
 long *maxPntP 
 ) 
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
 *minPntP = dtmP->numPoints - 1 ;
 *maxPntP = 0 ;
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
/*
**     Check For Point On Internal Region Hull
*/
       if( nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt && regionOption == 2 )
         { 
          if( bcdtmMark_checkForPointOnRegionHullDtmObject(dtmP,antPnt))
            {
             nodeAddrP(dtmP,antPnt)->tPtr = dtmP->numPoints ;
             if( antPnt < *minPntP ) *minPntP = antPnt ;
             if( antPnt > *maxPntP ) *maxPntP = antPnt ;
            }
         } 
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
/*
**           Check For Point On Internal Region Hull
*/
             if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt && regionOption == 2 )
               {
                if( bcdtmMark_checkForPointOnRegionHullDtmObject(dtmP,clPnt))
                  {
                   nodeAddrP(dtmP,clPnt)->tPtr = dtmP->numPoints ;
                   if( clPnt < *minPntP ) *minPntP = clPnt ;
                   if( clPnt > *maxPntP ) *maxPntP = clPnt ;
                  }
               } 
/*
**           Mark Point
*/
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
/*
**     Check For Point On Internal Region Hull
*/
       if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt && regionOption == 2 )
         { 
          if( bcdtmMark_checkForPointOnRegionHullDtmObject(dtmP,clPnt)) 
            {
             nodeAddrP(dtmP,clPnt)->tPtr = dtmP->numPoints ;
             if( clPnt < *minPntP ) *minPntP = clPnt ;
             if( clPnt > *maxPntP ) *maxPntP = clPnt ;
            }
         } 
/*
**     Mark Point
*/
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
BENTLEYDTM_Private int bcdtmMark_checkAndMarkPolygonsDtmObject(BC_DTM_OBJ* dtmP, long pnt)
    {
    int ret = DTM_SUCCESS;
    long flistPtr = nodeAddrP(dtmP, pnt)->fPtr;

    while( flistPtr != dtmP->nullPtr)
        {
        long dtmFeature = flistAddrP(dtmP,flistPtr)->dtmFeature;
        BC_DTM_FEATURE *dtmFeatureP = ftableAddrP(dtmP,dtmFeature);

        if(dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon || dtmFeatureP->dtmFeatureType == DTMFeatureType::Region)
            {
            long startPnt;
            if((ret = bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP, dtmFeature, &startPnt)) != DTM_SUCCESS)
                goto errexit;
            bcdtmList_reverseTptrPolygonDtmObject(dtmP, startPnt);
            }

        flistPtr = flistAddrP(dtmP,flistPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmMark_internalTptrPolygonPointsDtmObject2(BC_DTM_OBJ *dtmP,long startPnt,long mark,long *numMarkedP, long *minPntP , long *maxPntP ) 
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
 *minPntP = dtmP->numPoints - 1 ;
 *maxPntP = 0 ;
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
          bcdtmMark_checkAndMarkPolygonsDtmObject(dtmP, antPnt);
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
              if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
                  bcdtmMark_checkAndMarkPolygonsDtmObject(dtmP, clPnt);
             if(nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
               {
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = clPnt ;  }
                else                      { nodeAddrP(dtmP,lastPnt)->tPtr = clPnt ; lastPnt = clPnt ; }
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
      if( nodeAddrP(dtmP,clPnt)->tPtr == dtmP->nullPnt )
          bcdtmMark_checkAndMarkPolygonsDtmObject(dtmP, clPnt);
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
BENTLEYDTM_Private int bcdtmFind_startAndEndPointTptrPolygonDtmObject(
    BC_DTM_OBJ *dtmP,
    long pt,
    long* startPt,
    long* endPt)
    {

    long ret=DTM_SUCCESS;
    long scanPnt;
    /*
    ** Initialise
    */
    *startPt = *endPt = pt;
    /*
    ** Check Start Point tPtr List Is Not Null
    */
    if( nodeAddrP(dtmP,pt)->tPtr == dtmP->nullPnt ) 
        {
        bcdtmWrite_message(2,0,0,"Tptr List Start Point Is Null") ; 
        goto errexit ;
        }
    /*
    ** Scan Around Tptr Polygon And Start and End Points
    */
    scanPnt = nodeAddrP(dtmP,pt)->tPtr ; 
    do
        {

        if( scanPnt < *startPt) *startPt = scanPnt;
        if( scanPnt > *endPt  ) *endPt = scanPnt;

        scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
        if(scanPnt == dtmP->nullPnt)
            {
            bcdtmWrite_message(2,0,0,"DTM Feature not Polygon") ;
            goto errexit ;
            }
        } while ( scanPnt != pt) ;
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
BENTLEYDTM_EXPORT int bcdtmLoad_trianglesFromRegionDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureId featureId,
 DTMFeatureCallback loadFunctionP,
 void* userP
 )
 {
  int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
  BC_DTM_FEATURE *dtmFeatureP ;
  long dtmFeature,numTriangles=0;
  long startPnt=0,lastPnt=0,minPnt,maxPnt ;
  long numMarked;
  long mark = -999999;
  DPoint3d             trgPts[4];
  long p1, p2, p3, clPtr;
  DTM_CIR_LIST    *clistP ;
  DTM_TIN_NODE    *node1P,*node2P,*node3P ;
  DTM_TIN_POINT   *p1P,*p2P,*p3P;
  bool voidTriangle,voidsInDtm=false;
/*
** Write Entry Message
*/
 if( dbg ) 
   { 
    bcdtmWrite_message(0,0,0,"Loading Triangles for Region") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureId     = %10I64d",featureId) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
**  Test For Voids In DTM
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
/*
**  Scan Dtm Features And Return All Triangles For DTMFeatureType::Polygon With The FeatureID
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureId == featureId && dtmFeatureP->dtmFeatureType == DTMFeatureType::Region && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin)
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureId = %10I64d dtmFeatureType = %3ld dtmFeatureState = %2ld",dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState) ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP, dtmFeature, &startPnt) != DTM_SUCCESS) goto errexit ;
/*
**     Check Connectivity Of Tptr Polygon - Development Only
*/
       if( cdbg )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
          if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
         }
/*
**     Mark Points Internal To Tptr Polygon
*/
       if( bcdtmMark_internalTptrPolygonPointsDtmObject2(dtmP,startPnt, mark, &numMarked,&minPnt,&maxPnt ) != DTM_SUCCESS) goto errexit ;
//       if( bcdtmMark_internalTptrPolygonPointsMinMaxDtmObject(dtmP,startPnt,mark,&numMarked,&minPnt,&maxPnt ) != DTM_SUCCESS) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"featureId = %10I64d ** numMarked = %8ld minPnt = %8ld maxPnt = %8ld",featureId,numMarked,minPnt,maxPnt) ;
/*
**     Get Start And End Points For Scan
*/ 
       if( bcdtmFind_startAndEndPointTptrPolygonDtmObject(dtmP, startPnt, &startPnt, &lastPnt) != DTM_SUCCESS) goto errexit;
       if( startPnt < minPnt ) minPnt = startPnt ;
       if( lastPnt  > maxPnt ) maxPnt = lastPnt ;
       if( dbg ) bcdtmWrite_message(0,0,0,"minPnt = %8ld ** lastPnt = %8ld",minPnt,maxPnt) ;
/*
**      Scan And Load Triangles
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangles") ;
       for( p1 = minPnt ; p1 <= maxPnt ; ++p1 )
         {
          node1P = nodeAddrP(dtmP,p1) ;
          if(( clPtr = node1P->cPtr) != dtmP->nullPtr)
            {
             if( node1P->tPtr != dtmP->nullPnt) 
               {
                p1P = pointAddrP(dtmP,p1) ;
                trgPts[0].x = p1P->x ;
                trgPts[0].y = p1P->y ;
                trgPts[0].z = p1P->z ;
                trgPts[3].x = p1P->x ;
                trgPts[3].y = p1P->y ;
                trgPts[3].z = p1P->z ;
                if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
                node2P = nodeAddrP(dtmP,p2) ;
                while ( clPtr != dtmP->nullPtr )
                  {
                   clistP = clistAddrP(dtmP,clPtr) ;
                   clPtr  = clistP->nextPtr ;
                   p3     = clistP->pntNum ;
                   node3P = nodeAddrP(dtmP,p3) ;
                   if( node1P->hPtr != p2 ) 
                     {
                      if( ( p2 > p1 && p3 > p1 && (node2P->tPtr == mark || node2P->tPtr != dtmP->nullPnt) && (node3P->tPtr == mark || node3P->tPtr != dtmP->nullPnt) ) )
                        {
                         voidTriangle = FALSE ;
                         if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidTriangle)) goto errexit ; }
                         if( voidTriangle == FALSE ) 
                           {
/*
**                          Set Point Addresses
*/
                            p2P = pointAddrP(dtmP,p2) ;
                            p3P = pointAddrP(dtmP,p3) ;
/*
**                          Set Point Coordinates
*/
                            trgPts[1].x = p2P->x ;
                            trgPts[1].y = p2P->y ;
                            trgPts[1].z = p2P->z ;
                            trgPts[2].x = p3P->x ;
                            trgPts[2].y = p3P->y ;
                            trgPts[2].z = p3P->z ;
                            ++numTriangles ; 
                            if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,trgPts,4,userP)) goto errexit ;
                           }
                        } 
                     }
                   p2     = p3 ;
                   node2P = node3P ;
                  } 
               } 
            }
          nodeAddrP(dtmP,p1)->tPtr = dtmP->nullPnt ;
         } 
      }
   }
/*
** Write Number Of Triangles Loaded To Log File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
/*
** Clean Up
*/
 cleanup :
 if( cdbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtmP->dtmState == DTMState::Tin ) bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLoad_triangleShadeMeshForRegionDtmObject
(
 BC_DTM_OBJ *dtmP,                         // ==> Pointer To DTM
 long       maxTriangles,                  // ==> Maximum Triangles To Return Per Callback Call 
 long       vectorOption,                  // ==> Vector Option <1=Surface Derivatives,2=Averaged Triangle Surface Normals> 
 double     zAxisFactor,                   // ==> Factor To Exaggerate The z Axis default value 1.0                            
 long       regionOption,                  // ==> Region Option < 1 = Include Internal Regions , 2 = Exclude Internal Regions >
 long       indexOption,                   // ==> Index Option  < 1 = Use User Tag , 2 = Use Feature Id >
 Int64      indexValue,                    // ==> Value For Region Selection
 DTMTriangleShadeMeshCallback loadFunctionP,            // ==> Call Back Function
 void*       userP                         // ==> User Pointer To Pass Back To Calling Function
 )

 {
  int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
  BC_DTM_FEATURE *dtmFeatureP ;
  long dtmFeature,numTriangles=0,numTrianglesLoaded=0;
  long startPnt=0,lastPnt=0,minPnt,maxPnt ;
  long numMarked=0;
  long mark = -999999;
  long p1, p2, p3, clPtr;
  DTM_CIR_LIST    *clistP ;
  DTM_TIN_NODE    *node1P,*node2P,*node3P ;
  bool   voidTriangle,voidsInDtm=false;
  long  *faceP,*meshFacesP=NULL,numMeshPts=0 ;
  DPoint3d   *meshPtsP=NULL,*meshVectorsP=NULL ; 
  long  meshVectorsSize=0,meshPtsSize=0;
/*
** Write Entry Message
*/
 if( dbg ) 
   { 
    bcdtmWrite_message(0,0,0,"Loading Triangle Shade Mesh for Region") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles  = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"vectorOption  = %8ld",vectorOption) ;
    bcdtmWrite_message(0,0,0,"zAxisFactor   = %8.3lf",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"regionOption  = %8ld",regionOption) ;
    bcdtmWrite_message(0,0,0,"indexOption   = %8ld",indexOption) ;
    bcdtmWrite_message(0,0,0,"indexValue    = %8I64d",indexValue) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ; 
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
 if( dbg ) bcdtmWrite_message(0,0,0,"maxTriangles = %8ld",maxTriangles) ;
/*
** Validate Normal Vector Option
*/
 if( vectorOption < 1 || vectorOption > 2 ) vectorOption = 2 ;
/*
** Validate z Axis Factor
*/
 if( zAxisFactor <= 0.0 ) zAxisFactor = 1.0 ; 
/*
** Validate Region Option
*/
 if( regionOption < 1 || regionOption > 2 ) regionOption = 1 ;
/*
** Validate Region Index
*/
 if( indexOption  < 1 || indexOption  > 2 ) indexOption  = 1 ;
//  Null Node Pointers

 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;

/*
**  Test For Voids In DTM
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
**  Scan Dtm Features And Return All Triangles For DTMFeatureType::Region With The Index Value
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( ( indexOption == 1 && dtmFeatureP->dtmUserTag == indexValue ) || ( indexOption == 2 && dtmFeatureP->dtmFeatureId == indexValue ))  )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"dtmUserTag = %10I64d dtmFeatureId = %10I64d dtmFeatureType = %3ld dtmFeatureState = %2ld",dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState) ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP, dtmFeature, &startPnt) != DTM_SUCCESS) goto errexit ;
/*
**     Check Connectivity Of Tptr Polygon - Development Only
*/
       if( cdbg )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
          if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
         }

//     Log Tptr Polygon To File

       if( dbg )
         {
          BC_DTM_OBJ *temP=NULL ;
          if( bcdtmObject_createDtmObject(&temP)) goto errexit ;
          if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPnt,&meshPtsP,&numMeshPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(temP,DTMFeatureType::Breakline,temP->nullUserTag,1,&temP->nullFeatureId,meshPtsP,numMeshPts)) goto errexit ;
          if( bcdtmWrite_geopakDatFileFromDtmObject(temP,L"regionBoundary.dat")) goto errexit ;
          if( temP     != NULL ) if( bcdtmObject_destroyDtmObject(&temP)) goto errexit ;
          if( meshPtsP != NULL ) { free(meshPtsP) ; meshPtsP = NULL ; } 
         } 
/*
**     Mark Points Internal To Region
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Region") ;
       if( bcdtmMark_internalRegionPointsDtmObject(dtmP,regionOption,startPnt,mark,&numMarked,&minPnt,&maxPnt) != DTM_SUCCESS ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"indexValue = %10I64d ** numMarked = %8ld minPnt = %8ld maxPnt = %8ld",indexValue,numMarked,minPnt,maxPnt) ;
/*
**     Get Start And End Points For Scan
*/ 
       if( bcdtmFind_startAndEndPointTptrPolygonDtmObject(dtmP, startPnt, &startPnt, &lastPnt) != DTM_SUCCESS) goto errexit;
       if( startPnt < minPnt ) minPnt = startPnt ;
       if( lastPnt  > maxPnt ) maxPnt = lastPnt ;
       if( dbg ) bcdtmWrite_message(0,0,0,"minPnt = %8ld ** lastPnt = %8ld",minPnt,maxPnt) ;
/*
**     Scan And Load Triangles
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangles") ;
       numTriangles = 0 ; 
       for( p1 = minPnt ; p1 <= maxPnt ; ++p1 )
         {
          node1P = nodeAddrP(dtmP,p1) ;
          if(( clPtr = node1P->cPtr) != dtmP->nullPtr)
            {
             if( node1P->tPtr != dtmP->nullPnt && node1P->tPtr != dtmP->numPoints ) 
               {
                if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
                node2P = nodeAddrP(dtmP,p2) ;
                while ( clPtr != dtmP->nullPtr )
                  {
                   clistP = clistAddrP(dtmP,clPtr) ;
                   clPtr  = clistP->nextPtr ;
                   p3     = clistP->pntNum ;
                   node3P = nodeAddrP(dtmP,p3) ;
                   if( node1P->hPtr != p2 ) 
                     {
                      if( node2P->tPtr != dtmP->nullPnt && node3P->tPtr != dtmP->nullPnt) 
                        {
                         voidTriangle = FALSE ;
                         if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidTriangle)) goto errexit ; }
                         if( voidTriangle == FALSE ) 
                           {
                            *faceP = p3  ; ++faceP ;
                            *faceP = p2  ; ++faceP ;
                            *faceP = p1  ; ++faceP ;
                            ++numTriangles ;
/*
**                          Check For Maximum Load Triangles
*/
                            if( numTriangles == maxTriangles )
                              {
/*
**                             Create Shade Mesh
*/
                               if( bcdtmLoad_createTriangleShadeMeshDtmObject(dtmP,numTriangles,zAxisFactor,meshFacesP,&meshPtsP,&numMeshPts,&meshPtsSize,&meshVectorsP,&meshVectorsSize)) goto errexit ;
/*
**                             Call Browse Function
*/
                               if( bcdtmLoad_callUserTriangleShadeMeshLoadFunction(loadFunctionP,DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                               faceP = meshFacesP ;
                               numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                               numTriangles = 0 ;
                              }
                           }
                        } 
                     }
                   p2     = p3 ;
                   node2P = node3P ;
                  } 
               } 
            }
          nodeAddrP(dtmP,p1)->tPtr = dtmP->nullPnt ;
         } 
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Remaining Triangles") ;
/*
**  Create Shade Mesh
*/
    if( bcdtmLoad_createTriangleShadeMeshDtmObject(dtmP,numTriangles,zAxisFactor,meshFacesP,&meshPtsP,&numMeshPts,&meshPtsSize,&meshVectorsP,&meshVectorsSize)) goto errexit ;
/*
**  Call Browse Function
*/
    if( bcdtmLoad_callUserTriangleShadeMeshLoadFunction(loadFunctionP,DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
   }
/*
** Write Number Of Triangles Loaded To Log File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTrianglesLoaded) ;
/*
** Clean Up
*/
 cleanup :
 if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
 if( meshFacesP   != NULL ) { free(meshFacesP)   ; meshFacesP     = NULL ; }
 if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
 if( cdbg ) 
   { 
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Shade Mesh for Region Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Shade Mesh for Region Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtmP->dtmState == DTMState::Tin ) 
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
   }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_createTriangleShadeMeshDtmObject
(
 BC_DTM_OBJ  *dtmP,
 long        numTriangles, 
 double      zAxisFactor,
 long        *meshFacesP,
 DPoint3d    **meshPtsPP,
 long        *numMeshPtsP,
 long        *meshPtsSizeP,
 DPoint3d    **meshVectorsPP,
 long        *meshVectorSizeP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long node,minMeshPnt,maxMeshPnt,*faceP ;
 DPoint3d      *p3dP,*vectP,normalVector ;
 DTM_TIN_POINT *pntP ;
 DTM_TIN_NODE  *nodeP ;
 double dz ;
/*
** Write Entry Message
*/ 
  if( dbg ) 
    {
     bcdtmWrite_message(0,0,0,"Creating Triangle Shade Mesh") ;
    } 
/*
**  Mark Mesh Points
*/
 minMeshPnt = dtmP->numPoints ;
 maxMeshPnt = -1 ;
 for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
   {
    if( *faceP < minMeshPnt ) minMeshPnt = *faceP ;
    if( *faceP > maxMeshPnt ) maxMeshPnt = *faceP ;
    nodeAddrP(dtmP,*faceP)->sPtr = 1 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"minMeshPoint = %8ld maxMeshPoint = %8ld",minMeshPnt,maxMeshPnt) ;
/*
** Count Number Of Mesh Points
*/
 *numMeshPtsP = 0 ;
 for( node = minMeshPnt ; node <= maxMeshPnt ; ++node ) 
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->sPtr == 1 )
      {
       ++*numMeshPtsP ;
       nodeP->sPtr = *numMeshPtsP ; 
      }
   }  
/*
** Allocate Memory For Mesh Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** *numMeshPtsP = %8ld",*numMeshPtsP) ;
 if( *meshPtsPP == NULL || *numMeshPtsP > *meshPtsSizeP )
   {
    if( *meshPtsPP ) { free(*meshPtsPP); *meshPtsPP = NULL ; }
    *meshPtsPP = ( DPoint3d * ) malloc( *numMeshPtsP * sizeof(DPoint3d) ) ;
    if( *meshPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    *meshPtsSizeP = *numMeshPtsP;
   }
/*
** Allocate Memory For Mesh Vectors
*/
 if( *meshVectorsPP == NULL || *numMeshPtsP > *meshVectorSizeP )
   {
    if(*meshVectorsPP) { free(*meshVectorsPP) ; *meshVectorsPP = NULL ; }
    *meshVectorsPP = ( DPoint3d * ) malloc( *numMeshPtsP * sizeof(DPoint3d) ) ;
    if( *meshVectorsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    *meshVectorSizeP = *numMeshPtsP;
   }
/*
**  Exaggerate z Axis
*/ 
  if( zAxisFactor != 1.0 )
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
     for( node = minMeshPnt ; node <= maxMeshPnt ; ++node ) 
       {
        nodeP = nodeAddrP(dtmP,node) ;
        if( nodeP->sPtr != dtmP->nullPnt )
          {
           pntP    = pointAddrP(dtmP,node) ; 
           dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
           pntP->z = dtmP->zMin + dz ;
          } 
       }
    } 
/*
** Populate Mesh Points Array And Mesh Vectors Array
*/
 p3dP  = *meshPtsPP ;
 vectP = *meshVectorsPP ;

//  Check Mesh Point Range

 if( cdbg )
   {
    if( minMeshPnt < 0 || maxMeshPnt >= dtmP->numPoints )
      {
       bcdtmWrite_message(1,0,0,"Mesh Points Range Error") ;
       goto errexit ;
      }
   }

// Scan Points

 for( node = minMeshPnt ; node <= maxMeshPnt ; ++node ) 
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->sPtr > 0 && nodeP->sPtr < dtmP->nullPnt  )
      {
       pntP    = pointAddrP(dtmP,node) ; 

//     Check For Memory Range Error

       if( cdbg )
         {
          if((long)(p3dP-*meshPtsPP) >= *numMeshPtsP )
            {
             bcdtmWrite_message(1,0,0,"Mesh Points Memory Range Error ** Mesh Point = %8ld ** Number Of Mesh Points = %8ld",(long)(p3dP-*meshPtsPP),*numMeshPtsP) ;
             goto errexit ;
            }  
         }
       
//     Store Points

       p3dP->x = pntP->x ;
       p3dP->y = pntP->y ;
       p3dP->z = pntP->z ;
       ++p3dP ;
       if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
       *vectP  = normalVector ;
       ++vectP ;
      }
   }
/*
**  De Exaggerate z Axis
*/ 
  if( zAxisFactor != 1.0 )
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"DE-Exaggerating z Axis") ;
     for( node = minMeshPnt ; node <= maxMeshPnt ; ++node ) 
       {
        nodeP = nodeAddrP(dtmP,node) ;
        if( nodeP->sPtr > 0 && nodeP->sPtr < dtmP->nullPnt  )
          {
           pntP    = pointAddrP(dtmP,node) ; 
           dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
           pntP->z = dtmP->zMin + dz ;
          } 
       }
    } 
/*
** Reset Point Indexes In Mesh Faces
*/
 for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
   {
    *faceP = nodeAddrP(dtmP,*faceP)->sPtr ;
   }
/*
** Null Sptr Values
*/
 for( node = minMeshPnt ; node <= maxMeshPnt ; ++node ) 
   {
    nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
   }
/*
** Clean Up
*/
 cleanup :
 if( cdbg ) 
   {
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Triangle Shade Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtmP->dtmState == DTMState::Tin ) bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
