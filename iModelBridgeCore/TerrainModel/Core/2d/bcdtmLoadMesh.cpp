/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLoadMesh.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
/*==============================================================================*//**
* @memo   Loads The Dtm Tin Triangles As Consecutive 3 Tuple Point Indices
* @doc    Loads The Dtm Tin Triangles As Consecutive 3 Tuple Point Indices
* @param  dtmP                  ==> Pointer To DTM Dtm object        
* @param  useFence              ==> Use Fence  <TRUE,FALSE>            
* @param  fencePts              ==> DPoint3d Array Of Fence Points         
* @param  numFencePts           ==> Number Of Fence Points            
* @param  numTrianglesP         <== Number Of  Point Indicies
* @param  trianglesPP           <== Pointer To Points Indices
* @param  dtmPtsPPP             <== Pointer To Dtm Point Arrays ( Dpoint3d )
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - September 2007 - rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_tinTrianglesFromDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Dtm Object Pointer                             */
 long useFence,              /* ==> Use Fence  <TRUE,FALSE>                        */    
 DTMFenceType fenceType,             /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DPoint3d *fencePtsP,             /* ==> Pointer To Fence Points                        */
 long numFencePts,           /* ==> Number Of Fence Points                         */
 long *numTrianglesP,        /* <== Number Of Triangles                            */
 long **trianglesPP,         /* <== Pointer To Triangle Point Indicies             */
 DTM_TIN_POINT ***dtmPtsPPP  /* <== Pointer To Dtm Points Array ( Dpoint3d )       */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1,pnt2,pnt3,clPtr,memTriangles=0,memTrianglesInc=0 ;
 bool voidTriangle, loadTriangle, voidsInDtm;
 DTMFenceOption triangleExtent;
 double xMin,yMin,xMax,yMax ;
 DPoint3d    trgPts[4] ; 
 BC_DTM_OBJ *clipDtmP=NULL ; 
 DTM_TIN_POINT *p1P,*p2P,*p3P ;
 DTM_CIR_LIST  *clistP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Dtm Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"useFence      = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType     = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fencePtsP     = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numTrianglesP = %8ld",*numTrianglesP) ;
    bcdtmWrite_message(0,0,0,"trianglesPP   = %p",*trianglesPP) ;
    bcdtmWrite_message(0,0,0,"dtmPtsPPP     = %p",*dtmPtsPPP) ;
   }
/*
** Initialise
*/
 *numTrianglesP =  0 ;
 if( *trianglesPP != NULL ) 
   { 
    free(*trianglesPP)  ;
    *trianglesPP = NULL ; 
   } 
 *dtmPtsPPP = NULL ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Set Pointer To Dtm Points Array
*/
 *dtmPtsPPP = dtmP->pointsPP ;
/*
** Validate Fence 
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
/*
** Set Meory Allocation Parameters For Storing Triangles
*/
 memTriangles = 0 ;
 if( useFence == FALSE ) memTrianglesInc = dtmP->numTriangles * 3 ;
 else                    memTrianglesInc = dtmP->numTriangles * 3 / 4 ;
 if( memTrianglesInc <= 0 ) memTrianglesInc = 100 ;
/*
** Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
   }
/*
** Scan And Check For Presence Of Voids
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
/*
** Scan Dtm For Triangle Triangles
*/
 for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
   {
    if( ( clPtr = nodeAddrP(dtmP,pnt1)->cPtr ) != dtmP->nullPtr ) 
      {
       if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ; 
       while ( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          pnt3   = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
/*
**        Test For Triangle
*/        
          if( pnt2 > pnt1 && pnt3 > pnt1 && nodeAddrP(dtmP,pnt1)->hPtr != pnt2 )
            {
/*
**           Test For Void Triangle
*/  
             voidTriangle = false ; 
             if( voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**           Process If None Void Triangle
*/
             if( voidTriangle == false )  
               {
/*
**              Check If Triangle Overlaps Fence
*/
                loadTriangle = true ;
                if( useFence == TRUE )
                  { 
/*
**                 Set Triangle Point Addresses
*/ 
                   p1P = pointAddrP(dtmP,pnt1) ;
                   p2P = pointAddrP(dtmP,pnt2) ;
                   p3P = pointAddrP(dtmP,pnt3) ;
/*
**                 Get Bounding Rectangle For Triangle
*/
                   xMin = xMax = p1P->x ;
                   yMin = yMax = p1P->y ;
                   if( p2P->x < xMin ) xMin = p2P->x ;
                   if( p2P->x > xMax ) xMax = p2P->x ;
                   if( p2P->y < yMin ) yMin = p2P->y ;
                   if( p2P->y > yMax ) yMax = p2P->y ;
                   if( p3P->x < xMin ) xMin = p3P->x ;
                   if( p3P->x > xMax ) xMax = p3P->x ;
                   if( p3P->y < yMin ) yMin = p3P->y ;
                   if( p3P->y > yMax ) yMax = p3P->y ;
/*
**                 Check For Triangle External To Fence
*/
                   if( xMin > clipDtmP->xMax || xMax < clipDtmP->xMin || yMin > clipDtmP->yMax || yMax < clipDtmP->yMin ) loadTriangle = FALSE ;
                   else if( fenceType != DTMFenceType::Block ) 
                     {
                      trgPts[0].x = p1P->x ; trgPts[0].y = p1P->y ;
                      trgPts[1].x = p2P->x ; trgPts[1].y = p2P->y ;
                      trgPts[2].x = p3P->x ; trgPts[2].y = p3P->y ;
                      trgPts[3].x = p1P->x ; trgPts[3].y = p1P->y ;
                      if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,trgPts,4,&triangleExtent)) goto errexit ; 
                      if( triangleExtent == DTMFenceOption::Outside ) loadTriangle = FALSE ;
                     }
                  }
/*
**              Store Point Indices For Triangle
*/
                if( loadTriangle == true)
                  {
/*
**                 Check Memory
*/
                   if( *numTrianglesP + 2 >= memTriangles )
                     {
                      memTriangles = memTriangles + memTrianglesInc ;
                      if ( *trianglesPP == NULL ) *trianglesPP = ( long *) malloc ( memTriangles * sizeof(long)) ;
                      else                        *trianglesPP = ( long *) realloc ( *trianglesPP , memTriangles * sizeof(long)) ;
                      if( trianglesPP == NULL )
                        {
                         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                         goto errexit ;
                        }
                     }
/*
**                 Store Triangle Point Indices
*/
                   *(*trianglesPP+*numTrianglesP) = pnt1 + 1 ;
                   ++*numTrianglesP ;
                   *(*trianglesPP+*numTrianglesP) = pnt2 + 1 ;
                   ++*numTrianglesP ;
                   *(*trianglesPP+*numTrianglesP) = pnt3 + 1 ;
                   ++*numTrianglesP ;
                  }
               }
            }
/*
**        Set For Next Triangle
*/
          pnt2 = pnt3 ;
         }
      }
   }
/*
** Realloc Memory
*/
 if( *numTrianglesP >  0 && *numTrianglesP < memTriangles ) *trianglesPP = ( long *) realloc ( *trianglesPP , *numTrianglesP * sizeof(long)) ;
 if( *numTrianglesP == 0 && *trianglesPP   != NULL        ) { free(*trianglesPP) ; *trianglesPP = NULL ; }
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Dtm Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Dtm Triangles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 *numTrianglesP = 0 ;
 if( *trianglesPP != NULL ) { free(*trianglesPP)   ; *trianglesPP   = NULL ; }
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Tin Edges As Consecutive 2 Tuple Point Indices
* @doc    Loads The Tin Edges As Consecutive 2 Tuple Point Indices
* @param  *dtmP                 ==> Pointer To DTM Tin object        
* @param  useFence              ==> Use Fence  <TRUE,FALSE>            
* @param  fencePts              ==> DPoint3d Array Of Fence Points         
* @param  numFencePts           ==> Number Of Fence Points            
* @param  edgesPP               <== Pointer To Points Indices
* @param  numEdgesP             <== Number Of  Point Indicies
* @param  dtmPtsPPP             <== Pointer To Tin Point Arrays ( Dpoint3d )
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - September 2007 - rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_tinEdgesFromDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Tin Object Pointer                             */
 long useFence,              /* ==> Use Fence  <TRUE,FALSE>                        */    
 DPoint3d *fencePtsP,             /* ==> Pointer To Fence Points                        */
 long numFencePts,           /* ==> Number Of Fence Points                         */
 long *numEdgesP,            /* <== Number Of Edges                                */
 long **edgesPP,             /* <== Pointer To Edge Point Indicies                 */
 DTM_TIN_POINT ***dtmPtsPPP  /* <== Pointer To Tin Points Array ( Dpoint3d )       */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  pnt1,pnt2,listPtr,dtmFeature,memEdges=0,memEdgesInc=0 ;
 bool isVoids,voidLine;
 DTMFenceOption loadLine;
 DPoint3d   edgePts[2] ; 
 BC_DTM_OBJ *clipTinP=NULL ; 
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Tin Edges") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"useFence    = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP   = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numEdgesP   = %8ld",*numEdgesP) ;
    bcdtmWrite_message(0,0,0,"edgesPP     = %p",*edgesPP) ;
    bcdtmWrite_message(0,0,0,"dtmPtsPP    = %p",*dtmPtsPPP) ;
   }
/*
** Initialise
*/
 *numEdgesP =  0 ;
 if( *edgesPP != NULL ) 
   { 
    free(*edgesPP)  ;
    *edgesPP = NULL ; 
   } 
 *dtmPtsPPP = NULL ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Pointer To Tin Points Array
*/
 *dtmPtsPPP = dtmP->pointsPP ;
/*
** Validate Fence 
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
/*
** Set Meory Allocation Parameters For Storing Edges
*/
 memEdges = 0 ;
 if( useFence == FALSE ) memEdgesInc = dtmP->numLines * 2 ;
 else                    memEdgesInc = dtmP->numLines / 2 ;
 if( memEdgesInc <= 0  ) memEdgesInc = 100 ;
/*
** Build Clipping Tin For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipTinP,fencePtsP,numFencePts)) goto errexit ;
   }
/*
** Scan And Check For Presence Of Voids
*/
 isVoids = false;
 if( dtmP->numFeatures > 0 )
   {
    for( dtmFeature = 0  ; dtmFeature < dtmP->numFeatures && isVoids == false; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) isVoids = true ;
         }  
      } 
   }
/*
** Scan Tin For Triangle Edges
*/
 for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
   {
    if( ( listPtr = nodeAddrP(dtmP,pnt1)->cPtr ) != dtmP->nullPtr ) 
      {
       while ( listPtr != dtmP->nullPtr )
         {
          pnt2    = clistAddrP(dtmP,listPtr)->pntNum ;
          listPtr = clistAddrP(dtmP,listPtr)->nextPtr ;
/*
**        Test For Line
*/        
          if( pnt2 > pnt1 )
            {
/*
**           Test For Void Line
*/  
             voidLine = false ; 
             if( isVoids == true ) if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidLine)) goto errexit ;
/*
**           Process If None Void Line
*/
             if( voidLine == false )  
               {
/*
**              Check If Line Overlaps Fence
*/
                loadLine = DTMFenceOption::Inside;
                if( useFence == TRUE )
                  { 
                  edgePts[0].x = pointAddrP(dtmP,pnt1)->x ; edgePts[0].y = pointAddrP(dtmP,pnt1)->y ;
                  edgePts[1].x = pointAddrP(dtmP,pnt2)->x ; edgePts[1].y = pointAddrP(dtmP,pnt2)->y ;
                  if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipTinP,edgePts,2,&loadLine)) goto errexit ; 
                  if( loadLine == DTMFenceOption::Outside ) loadLine = DTMFenceOption::None ;
                 }
/*
**              Store Point Indices For Line
*/
                if( loadLine == DTMFenceOption::Inside)
                  {
/*
**                 Check Memory
*/
                   if( *numEdgesP + 1 >= memEdges )
                     {
                      memEdges = memEdges + memEdgesInc ;
                      if ( *edgesPP == NULL ) *edgesPP = ( long *) malloc ( memEdges * sizeof(long)) ;
                      else                    *edgesPP = ( long *) realloc ( *edgesPP , memEdges * sizeof(long)) ;
                      if( edgesPP == NULL )
                        {
                         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                         goto errexit ;
                        }
                     }
/*
**                 Store Line Point Indices
*/
                    *(*edgesPP+*numEdgesP) = pnt1 ;
                    ++*numEdgesP ;
                    *(*edgesPP+*numEdgesP) = pnt2 ;
                    ++*numEdgesP ;
                  }
               }
            }
         }
      }
   }
/*
** Realloc Memory
*/
 if( *numEdgesP >  0 && *numEdgesP < memEdges ) *edgesPP = ( long *) realloc ( *edgesPP , *numEdgesP * sizeof(long)) ;
 if( *numEdgesP == 0 && *edgesPP   != NULL    ) { free(*edgesPP) ; *edgesPP = NULL ; }
/*
** Clean Up
*/
 cleanup :
 if( clipTinP != NULL ) bcdtmObject_destroyDtmObject(&clipTinP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Tin Edges Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Tin Edges Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 *numEdgesP = 0 ;
 if( *edgesPP != NULL ) { free(*edgesPP)   ; *edgesPP   = NULL ; }
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @doc    Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @notes  This function returns a maximum number of mesh faces per call
* @notes  and must be called repeatively untill it returns zero mesh faces.
* @notes  It is done this way to reduce memory fragmentation associated with 
* @notes  allocating and freeing large memory arrays. 
* @notes  For the first call set firstCall to TRUE. 
* @notes  For second and subsequent calls set firstCall to FALSE.
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesPP
* @notes  The meshPtsPP and meshFacesPP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesPP,3,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  dtmP                  ==> Pointer To Dtm object        
* @param  firstCall             ==> First Call <TRUE,FALSE>  
* @param  maxMeshSize           ==> Maximum Number Of Mesh Faces To Be returned per call 
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >         
* @param  fencePts              ==> DPoint3d Array Of Fence Points         
* @param  numFencePts           ==> Number Of Fence Points            
* @param  meshPtsPP             <== Pointer To Mesh Points
* @param  numMeshPtsP           <== Number Of Mesh Points
* @param  meshFacesPP           <== Pointer To Mesh Faces 
* @param  numMeshFacesP         <== Number Of Mesh Faces
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - November 2004 - rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLoad_tinMeshFromDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To Dtm Object                          */
 long firstCall,             /* ==> TRUE for first call FALSE For subsequent calls */
 long maxTriangles,          /* ==> Maximum Number Of Triangles To Load            */
 long useFence,              /* ==> Use Fence  <TRUE,FALSE>                        */    
 DTMFenceType fenceType,             /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,           /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d *fencePtsP,             /* ==> Pointer To Fence Points                        */
 long numFencePts,           /* ==> Number Of Fence Points                         */
 DPoint3d  **meshPtsPP,           /* <== Pointer To Mesh Points                         */
 long *numMeshPtsP,          /* <== Number Of Mesh Points                          */
 long **meshFacesPP,         /* <== Pointer To Mesh Faces                          */
 long *numMeshFacesP         /* <== Number Of Mesh Faces                           */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  p1,p2,p3,lp1=0,lp2=0,lp3=0,*faceP,clPtr,numTriangles=0,numTrianglesMarked=0 ;
 bool voidTriangle, voidsInDtm = false, loadTriangle, firstTriangle;
 long  node, startPnt, lastPnt, nextPnt = 0;
 DTMFenceOption triangleExtent;
 DPoint3d   *p3dP,trgPts[4] ; 
 double xMin,xMax,yMin,yMax ;
 static long sp1=0,sp2=0,sp3=0,scanStartPnt=0,scanLastPnt=0,totalTriangles=0,startTime  ; 
 static long useWindow = 0;
 static DTMFenceType windowType = DTMFenceType::Block;
 static DTMFenceOption windowOption = DTMFenceOption::Overlap;
 static BC_DTM_OBJ *clipDtmP=NULL ;
 DTM_TIN_POINT *pntP ;
 DTM_TIN_NODE  *nodeP ;
/*
** Write Entry Message
*/
  if( dbg && firstCall == TRUE )
   {
    bcdtmWrite_message(0,0,0,"Loading Dtm Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"firstCall        = %8ld",firstCall) ;
    bcdtmWrite_message(0,0,0,"maxTriangles     = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType        = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption      = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePoints   = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"meshPtsPP        = %p",*meshPtsPP) ;
    bcdtmWrite_message(0,0,0,"numMeshPtsP      = %8ld",*numMeshPtsP) ;
    bcdtmWrite_message(0,0,0,"meshFacesPP      = %p",*meshFacesPP) ;
    bcdtmWrite_message(0,0,0,"numMeshFacesP    = %8ld",*numMeshFacesP) ;
    if( useFence && numFencePts > 2 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         } 
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",dtmP->xMin,dtmP->xMax,dtmP->xRange) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",dtmP->yMin,dtmP->yMax,dtmP->yRange) ;
       bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",dtmP->zMin,dtmP->zMax,dtmP->zRange) ;
      }
   }
/*
** Initialise
*/
 numTriangles = 0 ;
 *numMeshPtsP = *numMeshFacesP = 0 ;
 if( *meshPtsPP   != NULL ) { free(*meshPtsPP)   ; *meshPtsPP   = NULL ; }
 if( *meshFacesPP != NULL ) { free(*meshFacesPP) ; *meshFacesPP = NULL ; } 
/*
** Check Calls Are Correctly Synchronised Or Last Call
*/
 if( firstCall == FALSE && sp1 == 0 && sp2 == 0 ) goto cleanup ;
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 250000 ; 
/*
** Do First Call Processing
*/
 if( firstCall == TRUE )
   {
    startTime = bcdtmClock() ;
/*
**  Check For Valid Dtm Object
*/
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
    if( dtmP->dtmState != DTMState::Tin )
      {
       bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
       goto errexit ;
      }
/*
**  Delete Clip Dtm If It Exists
*/
    if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
**  Initialise Triangle Points
*/
    sp1 = sp2 = sp3 = 0 ;
    totalTriangles  = 0 ;
    scanStartPnt = 0 ;
    scanLastPnt  = dtmP->numPoints ;
/*
**  Validate Fence 
*/  
    if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
    if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
/*
**  Build Clipping Dtm For Fence
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Before Testing Extents ** useFence = %2ld",useFence) ;
    if( useFence == TRUE ) 
      {
       if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
/*
**     Check Fence Bounding Rectangle And DTM Bounding Rectangle Overlap
*/
       if( clipDtmP->xMin < dtmP->xMax && clipDtmP->xMax > dtmP->xMin && clipDtmP->yMin < dtmP->yMax && clipDtmP->yMax > dtmP->yMin ) 
         {
/*
**        Check Fence Is Internal To DTM
*/
          if( dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax )useFence = FALSE ;
/*
**        Get Start And End Points For Scanning Triangles
*/
          else
            {
             startTime = bcdtmClock() ;
             bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&scanStartPnt) ;
             while( scanStartPnt > 0 && pointAddrP(dtmP,scanStartPnt)->x >= clipDtmP->xMin ) --scanStartPnt ;
             if( pointAddrP(dtmP,scanStartPnt)->x < clipDtmP->xMin ) ++scanStartPnt ;
             bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&scanLastPnt) ;
             while( scanLastPnt < dtmP->numPoints && pointAddrP(dtmP,scanLastPnt)->x <= clipDtmP->xMax ) ++scanLastPnt ;
             if( dbg ) 
               { 
                bcdtmWrite_message(0,0,0,"scanStartPnt = %8ld scanStartPnt->x = %12.5lf",scanStartPnt,pointAddrP(dtmP,scanStartPnt)->x) ;
                bcdtmWrite_message(0,0,0,"scanLastPnt  = %8ld scanLastPnt->x  = %12.5lf",scanLastPnt,pointAddrP(dtmP,scanLastPnt)->x) ;
                bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
                bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
               }
             sp1 = scanStartPnt ;
            }          
         }
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Fence Does Not Overlap DTM") ;
          goto cleanup ;
         }  
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"After Testing Extents ** useFence = %2ld",useFence) ;
/*
**  Set Static Window Values
*/
    useWindow = useFence ;
    windowType = fenceType ;
    windowOption = fenceOption ;
/*
**  Scan Dtm Features And Check For The Presence Of Voids 
*/
    voidsInDtm = FALSE ;
    bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
   }
/*
** Scan Dtm And Mark Dtm Points
*/
 lastPnt = 0 ;
 startPnt = sp1 ;
 numTriangles = 0 ;
 firstTriangle = true ;
 for( p1 = sp1 ; p1 < scanLastPnt && numTriangles < maxTriangles ; ++p1 )
   {
    if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr ) 
      {
/*
**     Scan To First Load Triangle
*/
       if( firstTriangle == true ) 
         {
          firstTriangle = false ;
          if( sp2 != 0 || sp3 != 0 )
            {
             if(( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
             do
               {
                p2 = p3 ; 
                p3 = clistAddrP(dtmP,clPtr)->pntNum ;
                clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
               } while( p2 != sp2 && p3 != sp3 && clPtr != dtmP->nullPtr ) ;
             p2 = p3 ;
            }
          else if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
          if( dbg ) bcdtmWrite_message(0,0,0,"First Triangle Found ** p1 = %8ld p2 = %8ld p3 = %8ld",p1,p2,p3) ;
         }
/*
**     Get P2
*/
       else if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
/*
**     Continue Scan With Next Triangle
*/
       while ( clPtr != dtmP->nullPtr && numTriangles < maxTriangles )
         {
          p3  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Next Triangle Found ** p1 = %8ld p2 = %8ld p3 = %8ld",p1,p2,p3) ;
          if( ( ( p2 > p1 && p3 > p1 ) || ( p2 < scanStartPnt || p3 < scanStartPnt ))  && nodeAddrP(dtmP,p1)->hPtr != p2 )
            {
/*
**           Test For Void Triangle
*/  
             voidTriangle = false ; 
             if( voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidTriangle)) goto errexit ;
/*
**           Process If Valid Triangle
*/
             if( voidTriangle == false )  
               {
/*
**              Check If Triangle Overlaps Fence
*/
                loadTriangle = true ;
                if( useWindow == TRUE )
                  { 
                   trgPts[0].x = pointAddrP(dtmP,p1)->x ; trgPts[0].y = pointAddrP(dtmP,p1)->y ;
                   trgPts[1].x = pointAddrP(dtmP,p2)->x ; trgPts[1].y = pointAddrP(dtmP,p2)->y ;
                   trgPts[2].x = pointAddrP(dtmP,p3)->x ; trgPts[2].y = pointAddrP(dtmP,p3)->y ;
                   trgPts[3].x = pointAddrP(dtmP,p1)->x ; trgPts[3].y = pointAddrP(dtmP,p1)->y ;
                   if( windowType == DTMFenceType::Block )
                     {
                      triangleExtent = DTMFenceOption::Outside ;
                      xMin = xMax = trgPts[0].x ;
                      yMin = yMax = trgPts[0].y ;
                      if( trgPts[1].x < xMin ) xMin = trgPts[1].x ;
                      if( trgPts[1].x > xMax ) xMax = trgPts[1].x ;
                      if( trgPts[2].x < xMin ) xMin = trgPts[2].x ;
                      if( trgPts[2].x > xMax ) xMax = trgPts[2].x ;
                      if( trgPts[1].y < yMin ) yMin = trgPts[1].y ;
                      if( trgPts[1].y > yMax ) yMax = trgPts[1].y ;
                      if( trgPts[2].y < yMin ) yMin = trgPts[2].y ;
                      if( trgPts[2].y > yMax ) yMax = trgPts[2].y ;
                      if( xMin < clipDtmP->xMax && xMax > clipDtmP->xMin && yMin < clipDtmP->yMax && yMax > clipDtmP->yMin ) triangleExtent = DTMFenceOption::Inside ;
                     }
                   else
                     {
                      if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,trgPts,4,&triangleExtent)) goto errexit ; 
                     } 
                   if( triangleExtent == DTMFenceOption::Outside ) loadTriangle = FALSE ;
                  }
/*
**              Mark Triangle Load Points
*/
                if( loadTriangle == true )
                  {
                   if( p2 < startPnt ) startPnt = p2 ;
                   if( p3 < startPnt ) startPnt = p3 ;
                   if( p2 > lastPnt ) lastPnt = p2 ;
                   if( p3 > lastPnt ) lastPnt = p3 ;
                   nodeAddrP(dtmP,p1)->sPtr = 1 ;
                   nodeAddrP(dtmP,p2)->sPtr = 1 ;
                   nodeAddrP(dtmP,p3)->sPtr = 1 ;
                   ++numTriangles ;
                  }
               }
            }
          lp1 = p1 ;
          lp2 = p2 ;
          lp3 = p3 ;   
          p2 = p3 ; 
         }
      }
   }
/*
** Write Number Of Triangles Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"00 Number Of Triangles Marked = %8ld",numTriangles) ;
/*
** Create Mesh Arrays
*/
 if( numTriangles > 0 )
   {
/*
**  Count Number Of Mesh Points
*/
    *numMeshPtsP = 0 ;
    for( node = startPnt ; node <= lastPnt ; ++node ) 
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->sPtr != dtmP->nullPnt )
         {
          ++*numMeshPtsP ;
          nodeP->sPtr = *numMeshPtsP ; 
         }
      }  
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** *numMeshPtsP = %8ld",*numMeshPtsP) ;
    *meshPtsPP = ( DPoint3d * ) malloc( *numMeshPtsP * sizeof(DPoint3d) ) ;
    if( *meshPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Populate Mesh Points Array
*/
    nextPnt = 1 ;              // One Based Adressing For Polyface Mesh
    p3dP = *meshPtsPP ;
    for( node = startPnt ; node <= lastPnt ; ++node ) 
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->sPtr != dtmP->nullPnt )
         {
          pntP    = pointAddrP(dtmP,node) ; 
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          nodeP->sPtr = nextPnt ;
          ++nextPnt ;
          ++p3dP ;
         }
      }  
/*
**  Allocate Memory For Mesh Faces Array
*/
    *numMeshFacesP = numTriangles ;
    *meshFacesPP = (long * ) malloc( *numMeshFacesP * 3 * sizeof(long)) ;
    if( *meshFacesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Polulate The Mesh Faces Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Faces") ;
    faceP = *meshFacesPP ;
    firstTriangle = true ;
    numTrianglesMarked = 0 ;
    for( p1 = sp1 ; p1 < scanLastPnt && numTrianglesMarked < numTriangles ; ++p1 )
      {
       if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr ) 
         {
/*
**        Scan To First Load Triangle
*/
          if( firstTriangle == TRUE ) 
            {
             firstTriangle = FALSE ;
             if( sp2 != 0 || sp3 != 0 )
               {
                if(( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
                do
                  {
                   p2 = p3 ; 
                   p3 = clistAddrP(dtmP,clPtr)->pntNum ;
                   clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                  } while( p2 != sp2 && p3 != sp3 && clPtr != dtmP->nullPtr ) ;
                p2 = p3 ;
               }
             else if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
            }
/*
**        Get P2
*/
          else if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;  
/*
**        Continue Scan With Next Triangle
*/
          while ( clPtr != dtmP->nullPtr && numTrianglesMarked < numTriangles )
            {
             p3 = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if( (( p2 > p1 && p3 > p1 ) || ( p2 < scanStartPnt || p3 < scanStartPnt )) && nodeAddrP(dtmP,p1)->hPtr != p2 )
               {
                if( nodeAddrP(dtmP,p1)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,p2)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,p3)->sPtr != dtmP->nullPnt )
                  {
                   voidTriangle = false ;
                   if( voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidTriangle)) goto errexit ;
                   if( voidTriangle == false )
                     { 
                      *faceP = nodeAddrP(dtmP,p1)->sPtr  ; ++faceP ;
                      *faceP = nodeAddrP(dtmP,p2)->sPtr  ; ++faceP ;
                      *faceP = nodeAddrP(dtmP,p3)->sPtr  ; ++faceP ;
                      ++numTrianglesMarked ;
                     }
                  }
               }
             p2 = p3 ; 
            }
         }
      }
/*
**   Write Number Of Triangles Marked
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Marked = %8ld",numTrianglesMarked) ;
   }
/*
** Reset Static Point Numbers If All Triangles Loaded
*/
 if( p1 >= scanLastPnt || numTriangles == 0  ) 
   {
    sp1 = sp2 = sp3 = 0 ;
    if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
   }
 else
   {
    sp1 = lp1 ;
    sp2 = lp2 ;
    sp3 = lp3 ;
   }
/*
** Reset SPtr Values To Null
*/
 for( node = startPnt ; node < lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
** Increment Total Triangles Loaded
*/
 totalTriangles = totalTriangles + numTriangles ;
/*
** Clean Up
*/
 cleanup :
 if( tdbg ) bcdtmWrite_message(0,0,0,"*********** Total Triangles Loaded  = %10ld Time = %8.3lf seconds ** dtmP->numTriangles = %10ld",totalTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime),dtmP->numTriangles) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Dtm Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Dtm Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 sp1 = sp2 = sp3 = 0 ;
 *numMeshPtsP = *numMeshFacesP = 0 ;
 if( *meshPtsPP    != NULL ) { free(*meshPtsPP)   ; *meshPtsPP   = NULL ; }
 if( *meshFacesPP  != NULL ) { free(*meshFacesPP) ; *meshFacesPP = NULL ; } 
 if( clipDtmP      != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Lattice As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @doc    Loads The Lattice As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @notes  This function returns a maximum number of mesh faces per call
* @notes  and must be called repeatively untill it returns zero mesh faces.
* @notes  It is done this way to reduce memory fragmentation associated with 
* @notes  allocating and freeing large memory arrays. 
* @notes  For the first call set firstCall to TRUE. 
* @notes  For second and subsequent calls set firstCall to FALSE.
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesPP
* @notes  The meshPtsPP and meshFacesPP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesPP,4,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  *latticeP             ==> Pointer To DTM Lattice object        
* @param  firstCall             ==> First Call <TRUE,FALSE>  
* @param  maxMeshSize           ==> Maximum Number Of Mesh Faces To Be returned per call 
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >         
* @param  fencePts              ==> DPoint3d Array Of Fence Points         
* @param  numFencePts           ==> Number Of Fence Points            
* @param  meshPtsPP             <== Pointer To Mesh Points
* @param  numMeshPtsP           <== Number Of Mesh Points
* @param  meshFacesPP           <== Pointer To Mesh Faces 
* @param  numMeshFacesP         <== Number Of Mesh Faces
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - November 2004 - rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_latticeMeshFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,             /* ==> Pointer To Lattice Object */
 long firstCall,                    /* ==> First Call <TRUE,FALSE>   */ 
 long maxMeshSize,                  /* ==> Maximum Number Of Mesh Faces To Be returned per call */
 long useFence,                     /* ==> Use Fence  <TRUE,FALSE>   */    
 DPoint3d  *fencePtsP,                   /* ==> Pointer To Fence Points   */
 long numFencePts,                  /* ==> Number Of Fence Points    */
 DPoint3d  **meshPtsPP,                  /* <== Pointer To Mesh Points    */
 long *numMeshPtsP,                 /* <== Number Of Mesh Points     */
 long **meshFacesPP,                /* <== Pointer To Mesh Faces     */
 long *numMeshFacesP                /* <== Number Of Mesh Faces      */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    i,j,li=0,lj=0,f1,f2,f3,f4,*pntP,*pntNumP=NULL,loadCell ;
 long    *faceP, numPoints = 0, numFaces = 0;
 DTMFenceOption latticeExtent;
 double  x1,y1,x2,y2,z1,z2,z3,z4 ;
 float   nullValue=(float) 0.0 ;
 DPoint3d     *p3dP,latticePts[5] ;
 static long si=0,sj=0 ; 
 static BC_DTM_OBJ *clipDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg && firstCall == TRUE )
   {
    bcdtmWrite_message(0,0,0,"Loading Lattice Mesh") ;
    bcdtmWrite_message(0,0,0,"latticeP         = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"firstCall        = %8ld",firstCall) ;
    bcdtmWrite_message(0,0,0,"maxMeshSize      = %8ld",maxMeshSize) ;
    bcdtmWrite_message(0,0,0,"Use Fence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"Fence Points     = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"Num Fence Points = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"Mesh Pts Array   = %p",*meshPtsPP) ;
    bcdtmWrite_message(0,0,0,"Num Mesh Pts     = %8ld",*numMeshPtsP) ;
    bcdtmWrite_message(0,0,0,"Mesh Faces Array = %p",*meshFacesPP) ;
    bcdtmWrite_message(0,0,0,"Num Mesh Faces   = %8ld",*numMeshFacesP) ;
    bcdtmWrite_message(0,0,0,"si               = %8ld",si) ;
    bcdtmWrite_message(0,0,0,"sj               = %8ld",sj) ;
   }
/*
** Free Memory If Necessary
*/
 *numMeshPtsP   = 0 ;
 *numMeshFacesP = 0 ;
 if( *meshPtsPP   != NULL ) { free(*meshPtsPP)   ; *meshPtsPP   = NULL ; }
 if( *meshFacesPP != NULL ) { free(*meshFacesPP) ; *meshFacesPP = NULL ; }
/*
** Check Calls Are Correctly Synchronised
*/
 if( firstCall == FALSE && si == 0 && sj == 0 )
   {
    if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
    goto cleanup ;
   }
/*
** Validate Mesh Size
*/
 if( maxMeshSize <= 0 ) maxMeshSize = 100000 ; 
/*
** Do First Call Processing
*/
 if( firstCall == TRUE )
   {
/*
** Test For Valid Lattice Object
*/
    if(bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Delete Clip Tin If It Exists
*/
    if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Validate Fence 
*/
    if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
    if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
/*
** Build Clipping Tin For Fence
*/
    if( useFence == TRUE ) 
      {
       if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Fence Range ** xMin = %12.5lf xMax = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
          bcdtmWrite_message(0,0,0,"            ** yMin = %12.5lf yMax = %12.5lf",clipDtmP->yMin,clipDtmP->yMax) ;
         }
      }
   }
/*
** Initialise
*/
 nullValue = latticeP->NULLVAL ;
/*
** Allocate Memory For Face Vertices
*/
 pntNumP = (long *) malloc( latticeP->NXL * latticeP->NYL * sizeof(long)) ;
 if( pntNumP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( pntP = pntNumP ; pntP < pntNumP + latticeP->NXL * latticeP->NYL ; ++pntP ) *pntP = 0 ;
/*
** Count Number Of Vertices And Faces In Lattice
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Vertices And Faces In Lattice") ;
 numFaces  = 0 ;
 for( i = si ; i < latticeP->NYL - 1 && numFaces < maxMeshSize ; ++i )
   {
    li = i ;
    for( j = sj ; j < latticeP->NXL - 1 && numFaces < maxMeshSize ; ++j )
      {
       lj = j ;
       z1 = *(latticeP->LAT + j*latticeP->NYL + i) ;
       z2 = *(latticeP->LAT + j*latticeP->NYL + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL + i) ; 
       z4 = *(latticeP->LAT + (j+1)*latticeP->NYL + i + 1) ;
       if( z1 != nullValue && z2 != nullValue && z3 != nullValue && z4 != nullValue )
         {
/*
**        Set Cell Coordinates
*/
          x1 = latticeP->LXMIN + latticeP->DX * i ; 
          x2 = x1 + latticeP->DX ;
          y1 = latticeP->LYMIN + latticeP->DY * j ;
          y2 = y1 + latticeP->DY ;
/*
**        Check If Cell Overlaps Fence
*/
          loadCell = TRUE ;
          if( useFence == TRUE )
            { 
             latticePts[0].x = x1 ; latticePts[0].y = y1 ;
             latticePts[1].x = x2 ; latticePts[1].y = y1 ;
             latticePts[2].x = x2 ; latticePts[2].y = y2 ;
             latticePts[3].x = x1 ; latticePts[3].y = y2 ;
             latticePts[4].x = x1 ; latticePts[4].y = y1 ;
             if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,latticePts,5,&latticeExtent)) goto errexit ; 
             if( latticeExtent == DTMFenceOption::Outside ) loadCell = FALSE ;
            } 
/*
**        Set Cell Markers
*/
          if( loadCell == TRUE )  
            {
             *(pntNumP+j*latticeP->NYL+i)       = 1 ;
             *(pntNumP+j*latticeP->NYL+i+1)     = 1 ;
             *(pntNumP+(j+1)*latticeP->NYL+i)   = 1 ;
             *(pntNumP+(j+1)*latticeP->NYL+i+1) = 1 ;
             ++numFaces ;
            }
         }
      }
    sj = 0 ;
   }
/*
** Reset Static Lattice Start Scan Points
*/
 ++lj ;
 if( lj >= latticeP->NXL - 1 ) { ++li ; lj = 0 ; }
 if( li >= latticeP->NYL - 1 ) li = 0 ;
 si = li ;
 sj = lj ;
/*
** Count And Assign Vertice Offsets
*/
 numPoints = 0 ;
 for( i = 0 ; i < latticeP->NYL  ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL  ; ++j )
      { 
       if( *(pntNumP+j*latticeP->NYL+i) ) 
         { 
          ++numPoints ; 
          *(pntNumP+j*latticeP->NYL+i) = numPoints ; 
         }
      }
   }
/*
** Write Statistics
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Num Points = %8ld ** Num Faces = %8ld",numPoints,numFaces) ;
/*
** Allocate Memory For Mesh Points
*/
 *numMeshPtsP = numPoints ;
 *meshPtsPP = ( DPoint3d * ) malloc( *numMeshPtsP * sizeof(DPoint3d)) ;
 if( *meshPtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Populate Mesh Points Array
*/
 p3dP = *meshPtsPP ;
 for( i = 0 ; i < latticeP->NYL  ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL  ; ++j )
      { 
       if( *(pntNumP+j*latticeP->NYL+i) ) 
         { 
          p3dP->x = latticeP->LXMIN + latticeP->DX * i ;  
          p3dP->y = latticeP->LYMIN + latticeP->DY * j ;
          p3dP->z = *(latticeP->LAT+j*latticeP->NYL+i)  ;
          ++p3dP ; 
         }
      }
   }
/*
** Allocate Memory For Mesh Faces
*/
 *numMeshFacesP = numFaces ;
 *meshFacesPP = (long * ) malloc( *numMeshFacesP * 4 * sizeof(long)) ;
 if( *meshFacesPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Polulate The Mesh Faces Array
*/
 faceP = *meshFacesPP ;
 for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
      {
       f1 = *(pntNumP+j*latticeP->NYL+i) ;
       f2 = *(pntNumP+j*latticeP->NYL+i+1) ;
       f3 = *(pntNumP+(j+1)*latticeP->NYL+i+1) ;
       f4 = *(pntNumP+(j+1)*latticeP->NYL+i) ;
       if( f1 && f2 && f3 && f4 )
         {
          *faceP = f1  ; ++faceP ;
          *faceP = f2  ; ++faceP ;
          *faceP = f3  ; ++faceP ;
          *faceP = f4  ; ++faceP ;
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( pntNumP != NULL ) { free(pntNumP) ; pntNumP = NULL ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Mesh Completed ** Mesh Points = %10ld Mesh Faces = %10ld",numPoints,numFaces) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Mesh Completed Error") ;
 return(ret) ;
/*
** Error exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 si = sj = 0 ;
 goto cleanup ;
}
