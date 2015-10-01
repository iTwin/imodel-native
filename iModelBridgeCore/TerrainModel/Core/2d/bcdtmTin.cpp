/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmTin.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel\Core\bcDTMBaseDef.h>
#include <TerrainModel\Core\dtmevars.h>
#include <TerrainModel\Core\bcdtminlines.h>
//#pragma optimize( "p", on )
#include <TerrainModel\Core\partitionarray.h>
#include <algorithm>
#include <list>
#include <mutex>




#include <thread>

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_createTinDtmObject
(
 BC_DTM_OBJ *dtmP,
 long  edgeOption,
 double maxSide,
 long numGraphicBreaks,
 long numContourLines,
 long numSoftBreaks,
 long numHardBreaks,
 long numVoids,
 long numIslands,
 long numHoles,
 long numBreakVoids,
 long numDrapeVoids,
 long numGroupSpots,
 long numRegions,
 long numHulls,
 long numDrapeHulls,
 long numHullLines
)
{
/*
** This is the controlling function for triangulating a Dtm object.
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long colinear=0,startTime,useMultiThread=TRUE ;
/*
** Write Entry Message
*/
// bcdtmWrite_message(0,0,0,"Triangulating DTM Object %p ** numPoints = %8ld",dtmP,dtmP->numPoints) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Triangulating DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP                       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"edgeOption                 = %8ld",edgeOption) ;
    bcdtmWrite_message(0,0,0,"maxSide                    = %8.2lf",maxSide) ;
    bcdtmWrite_message(0,0,0,"Number of Points           = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Graphic Breaks   = %8ld",numGraphicBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Contour Lines    = %8ld",numContourLines) ;
    bcdtmWrite_message(0,0,0,"Number Of Soft Breaks      = %8ld",numSoftBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Hard Breaks      = %8ld",numHardBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids            = %8ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Break Voids      = %8ld",numBreakVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Voids      = %8ld",numDrapeVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Island           = %8ld",numIslands) ;
    bcdtmWrite_message(0,0,0,"Number Of Holes            = %8ld",numHoles) ;
    bcdtmWrite_message(0,0,0,"Number Of Group Spots      = %8ld",numGroupSpots) ;
    bcdtmWrite_message(0,0,0,"Number Of Polygons         = %8ld",numRegions) ;
    bcdtmWrite_message(0,0,0,"Number Of Hulls            = %8ld",numHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Hulls      = %8ld",numDrapeHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Hull Lines       = %8ld",numHullLines) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol                = %20.16lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol                = %20.16lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol               = %20.16lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"DTM_NUM_PROCESSORS         = %8ld",DTM_NUM_PROCESSORS) ;

#ifndef _WIN32_WCE
    bcdtmWrite_message(0,0,0,"Floating Control Word      = 0x%.4x ",_control87(0,0)) ;
#endif

   }
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Check Dtm State
*/
 if( dtmP->dtmState != DTMState::DuplicatesRemoved )
   {
    bcdtmWrite_message(2,0,0,"DTM In Wrong State For Tin Creation") ;
    goto errexit ;
   }
/*
** Set DTM State To Tin
*/
  dtmP->dtmState = DTMState::Tin  ;
  dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Set Edge Option
*/
 if     ( numHulls      ) edgeOption = 4 ;
 else if( numDrapeHulls ) edgeOption = 5 ;
 else if( numHullLines  ) edgeOption = 6 ;
/*
** Single Thread Triangulate DTM Object
*/
 if( useMultiThread == FALSE || DTM_NUM_PROCESSORS == 1 || dtmP->numPoints < 1000 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Triangulating") ;
    startTime = bcdtmClock() ;
    if( bcdtmTin_triangulateDtmObject(dtmP,&colinear)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Triangulation Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    if( colinear == 1 )
      {
       bcdtmWrite_message(1,0,0,"All DTM Points Colinear") ;
       goto errexit ;
      }
   }
/*
** Multi Thread Triangulate DTM Object
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Triangulating") ;
    startTime = bcdtmClock() ;
    if( bcdtmTin_multiThreadTriangulateDtmObject(dtmP,&colinear)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Triangulation Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    if( colinear == 1 )
      {
       bcdtmWrite_message(1,0,0,"All DTM Points Colinear") ;
       goto errexit ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulation Completed") ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
/*
**   Check Topology
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
     if( bcdtmCheck_topologyDtmObject(dtmP,dbg))
       {
        bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ; goto errexit ;
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Tin Topology Valid") ;
/*
**   Check Precision
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
     if( bcdtmCheck_precisionDtmObject(dtmP,dbg) )
       {
        bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
        goto errexit ;
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
   }
/*
** Process Connected Lines
*/
 if( numGraphicBreaks) // Only need GraphicBreaks so ignore || numHardBreaks || numSoftBreaks || numContourLines )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Connected Lines") ;
    startTime = bcdtmClock() ;
    if( bcdtmTin_processConnectedLinesDtmObject(dtmP,numGraphicBreaks,0/*numHardBreaks*/,0 /*numSoftBreaks*/,0 /*numContourLines*/)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Connected Lines Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Remove Precision Slivers On Tin Hull
*/
 if( edgeOption != 4 )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Precision Removing Edge Sivers") ;
    if( bcdtmTin_precisionRemoveEdgeSliversDtmObject(dtmP,dtmP->ppTol)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Precision Slivers Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Process Contour Lines
*/
 if( numContourLines )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Contour Lines") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::ContourLine)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Zero Slope Triangles Along Contour Lines") ;
    if( bcdtmInsert_removeZeroSlopeTrianglesAlongContourLinesDtmObject(dtmP)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Contour Lines Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
  }
/*
** Process Soft Break Lines
*/
 if( numSoftBreaks )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Break Lines") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::SoftBreakline)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Soft Break Lines Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Process Break Lines
*/
 if( numHardBreaks )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Break Lines") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::Breakline)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating Along Break Lines") ;
    if( bcdtmInsert_retriangualteAlongBreakLinesDtmObject(dtmP) ) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Break Lines Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Process Drape Voids
*/
 if( numVoids )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Voids") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::Void)) goto errexit ;
   }
/*
** Process Voids
*/
 if( numBreakVoids )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Break Voids") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::BreakVoid)) goto errexit ;
   }
/*
** Process Holes
*/
 if( numHoles )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Holes") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::Hole)) goto errexit ;
   }
/*
** Process Islands
*/
 if( numIslands )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Islands") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::Island)) goto errexit ;
   }
/*
** Add An Interior Point To Single Triangle Voids
*/
// if( numVoids || numBreakVoids || numHoles )
//   {
//    if( dbg ) bcdtmWrite_message(0,0,0,"Adding An Interior Point To All Single Triangle Voids") ;
//    if( bcdtmTin_addInteriorPointToSingleTriangleVoidsDtmObject(dtmP)) goto errexit ;
//   }
/*
** Process Group Spots
*/
 if( numGroupSpots )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Group Spots") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::GroupSpots)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Group Spot Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Process Polygons
*/
 if( numRegions )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Polygons") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::Region)) goto errexit ;
   }
/*
** Dissolve External Edge Triangles
*/
  startTime = bcdtmClock() ;
  if( dbg ) bcdtmWrite_message(0,0,0,"edgeOption = %2ld",edgeOption) ;
  switch ( edgeOption )
    {
     case  1 :      /* Do Nothing  */
     break   ;

     case  2 :      /* Sliver Remove External Triangles  */
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Edge Sliver Triangles") ;
       if( bcdtmTin_removeExternalSliverTrianglesDtmObject(dtmP) ) goto errexit ;
     break   ;

     case  3 :      /* Max Side Remove Sliver Triangles  */
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Edge Max Side Triangles") ;
       if( bcdtmTin_removeExternalMaxSideTrianglesDtmObject(dtmP,maxSide) ) goto errexit ;
     break   ;

     case  4 :      /* Boundary Polygon  */
       if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Polygon") ;
       if( bcdtmTin_clipTinToBoundaryPolygonDtmObject(dtmP) ) goto errexit ;
     break   ;

     case  5 :      /* Drape Boundary Polygon  */
       if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Polygon") ;
       if( bcdtmTin_clipTinToDrapeBoundaryPolygonDtmObject(dtmP) ) goto errexit ;
     break   ;

     case  6 :     /*  Boundary Lines  */
       if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Lines") ;
       if( bcdtmTin_clipTinToBoundaryLinesDtmObject(dtmP,maxSide) ) goto errexit ;
    break   ;
    } ;
  if( tdbg ) bcdtmWrite_message(0,0,0,"** Edge Option Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Process Drape Voids
*/
 if( numDrapeVoids )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Drape Voids") ;
    if( bcdtmTin_insertDtmFeatureTypeIntoDtmObject(dtmP,DTMFeatureType::DrapeVoid)) goto errexit ;
   }
/*
** Mark Internal Void Points
*/
 if( numVoids || numHoles || numBreakVoids || numDrapeVoids || numIslands )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points") ;
    if( bcdtmTin_markInternalVoidPointsDtmObject(dtmP)) goto errexit ;
/*
** Clip Dtm Features To Void Boundaries
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features To Voids") ;
    if( bcdtmTin_clipDtmFeaturesToVoidsDtmObject(dtmP)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Mark Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Compact Tin Structure
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP))                 goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP))                  goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Circular List") ;
 if( bcdtmTin_compactCircularListDtmObject(dtmP))                 goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Point And Node Tables") ;
 if( bcdtmTin_compactPointAndNodeTablesDtmObject(dtmP))           goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Compact Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Reset Bounding Cube
*/
 if( edgeOption == 4 || edgeOption == 5 )
   {
    if( bcdtmMath_setBoundingCubeDtmObject(dtmP)) goto errexit ;
   }
/*
** Resize DTM Memory
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Memory") ;
 if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Resize Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Resort Tin
*/
 if( dtmP->numSortedPoints != dtmP->numPoints )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Resorting Tin Structure") ;
    if( bcdtmTin_resortTinStructureDtmObject(dtmP)) goto errexit ;
    if( dtmP->hullPoint == dtmP->nullPnt ) bcdtmList_setConvexHullDtmObject(dtmP) ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Resort Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
**  Count Tin Lines And Triangles
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Tin Lines And Triangles") ;
 if( bcdtmList_countTrianglesAndLinesDtmObject(dtmP,&dtmP->numTriangles,&dtmP->numLines) ) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Tin Points    = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Tin Lines     = %8ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"Number Of Tin Triangles = %8ld",dtmP->numTriangles) ;
   }
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Count Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Set Modified Time
*/
#ifndef _WIN32_WCE
 bcdtmObject_updateLastModifiedTime (dtmP) ;
#endif
/*
** Check Tin
*/
 if( cdbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Tin Checking Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object %p Error",dtmP) ;
// if( ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object %p Completed",dtmP) ;
// if( ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object %p Error",dtmP) ;

 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 dtmP->dtmState = DTMState::TinError ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_triangulateDtmObject(BC_DTM_OBJ *dtmP,long *colinearPtsP )
/*
** This is the controlling function for sorting dtm points into tiles and then triangulating them
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long p,node,leftPnt=0,rightPnt=0,topPnt=0,bottomPnt=0 ;
 long startTime,tinTime ;
 long *sortOfsP=NULL,*longP ;
// DTM_TIN_NODE *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object %p",dtmP) ;
 startTime = tinTime = bcdtmClock() ;
/*
** Allocate Array For Saving Sorted Point Order
*/
 sortOfsP = ( long * ) malloc( dtmP->numPoints * sizeof(long)) ;
 if( sortOfsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set Point Offsets
*/
 for( p = 0 , longP = sortOfsP ; p < dtmP->numPoints ; ++p , ++longP ) *longP = p ;
/*
** Sort Points Into Tiles
*/
 startTime = bcdtmClock() ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Tile Sorting %9ld Dtm Points",dtmP->numPoints) ;
 bcdtmSort_taggedPointsIntoTilesForTriangulationDtmObject(dtmP,sortOfsP,0,dtmP->numPoints) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Tile Sorting Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Allocate Nodes Memory For Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Nodes Memory For Dtm Object") ;
 if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
/*
** Set Set Order Into Nodes Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Sort Order Into Nodes Array") ;
 startTime = bcdtmClock() ;
 for( p = 0 , longP = sortOfsP ; p < dtmP->numPoints ; ++p , ++longP  )
   {
    nodeAddrP(dtmP,p)->fPtr = *longP ;
   }
/*
** Free Sort Order Pointer
*/
 free(sortOfsP) ;
 sortOfsP = NULL ;
/*
** Initialise Circular List Parameters
*/
 dtmP->cListPtr = 0 ;
 dtmP->cListDelPtr = dtmP->nullPtr ;
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
**  Allocate Circular List Memory For Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Circular List Memory For Dtm Object") ;
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Delaunay Triangulate Dtm Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Delaunay Triangulating") ;
 if( bcdtmTin_delaunayTriangulateDtmObject(dtmP,0,dtmP->numPoints,DTM_X_AXIS,&leftPnt,&rightPnt,&bottomPnt,&topPnt,colinearPtsP) != DTM_SUCCESS ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"**  Dtm Delaunay  Time       = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( *colinearPtsP ) goto cleanup ;
/*
** Reset Convex Hull
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reseting Convex Hull") ;
 dtmP->hullPoint      = leftPnt ;
 dtmP->nextHullPoint  = nodeAddrP(dtmP,leftPnt)->hPtr ;
 for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->hPtr = dtmP->nullPnt ;
 if( bcdtmTin_setConvexHullDtmObject(dtmP,dtmP->hullPoint,dtmP->nextHullPoint) != DTM_SUCCESS ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"**  Dtm Reset Hull Time      = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Reconstruct Sort Order
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reconstructing Sort Order") ;
 if( bcdtmTin_reconstructSortOrderDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"**  Dtm Reconstruction  Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Reset Convex Hull First Pointers
*/
 dtmP->hullPoint = 0 ;
 dtmP->nextHullPoint  = nodeAddrP(dtmP,dtmP->hullPoint)->hPtr ;
/*
** Cleanup
*/
 cleanup :
 if( sortOfsP != NULL ) free(sortOfsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object Error") ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Triangulation Time        = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),tinTime)) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret =  DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
void bcdtmTin_multiThreadTriangulationWorkerDtmObject (DTM_MULTI_THREAD *trgParametersP)
/*
    ** This function uses a divide and conquer strategy to triangulate tiled points
    **
    ** Author : Rob Cormack  November 2008  Rob.Cormack@Bentley.com
    **
    */
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    long   p, *longP;
    BC_DTM_OBJ threadDtm;
    __time32_t startTime;
    /*
    ** Initialise variables
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** startPoint = %8ld numPoints = %8ld", trgParametersP->thread, trgParametersP->startPoint, trgParametersP->numPoints);
    /*
    ** Sort Points Into Tiles
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tile Sorting %9ld Dtm Points", trgParametersP->thread, trgParametersP->numPoints);
    bcdtmSort_taggedPointsIntoTilesForTriangulationDtmObject (trgParametersP->dtmP, trgParametersP->sortOfsP, trgParametersP->startPoint, trgParametersP->numPoints);
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tile Sorting Time = %8.3lf Seconds", trgParametersP->thread, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Check For Check Stop Termination
    */
    if (bcdtmTin_checkForTriangulationTermination (trgParametersP->dtmP)) return;
    /*
    ** Set Set Order Into Nodes Array
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Setting Sort Order Into Nodes Array");
    for (p = trgParametersP->startPoint, longP = trgParametersP->sortOfsP + trgParametersP->startPoint; p < trgParametersP->startPoint + trgParametersP->numPoints; ++p, ++longP)
        {
        nodeAddrP (trgParametersP->dtmP, p)->fPtr = *longP;
        }
    /*
    ** Delaunay Triangulate Dtm Object
    */
    startTime = bcdtmClock ();
    memcpy (&threadDtm, trgParametersP->dtmP, BCDTMSize);
    threadDtm.cListPtr = trgParametersP->startPoint * 6;
    if (dbg) bcdtmWrite_message (0, 0, 0, "Delaunay Triangulating");
    if (bcdtmTin_delaunayTriangulateDtmObject (&threadDtm, trgParametersP->startPoint, trgParametersP->numPoints, DTM_X_AXIS, &trgParametersP->leftMostPoint, &trgParametersP->rightMostPoint, &trgParametersP->bottomMostPoint, &trgParametersP->topMostPoint, &trgParametersP->isColinear) != DTM_SUCCESS)
        return;
    if (tdbg) bcdtmWrite_message (0, 0, 0, "**  Dtm Delaunay  Time       = %8.3lf Seconds", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Set Return Cyclic List Parameter Values
    */
    trgParametersP->cListPtr = threadDtm.cListPtr;
    trgParametersP->cListDelPtr = threadDtm.cListDelPtr;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_multiThreadTriangulateDtmObject(BC_DTM_OBJ *dtmP,long *colinearPtsP )
/*
** This is the controlling function for To Multi Thread A DTM Triangulation
**
** Author : Rob Cormack  November 2008  Rob.Cormack@Bentley.com
**
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long n,p,node,numDtmPoints,numClist ;
 long startTime,tinTime ;
 long *sortOfsP=NULL,*longP ;
 long startPoint, numThreadPoints = 0;
 bvector<long> numThreadArrayPoints;
 long p1l,p1r,col1,p2l,p2r,col2,cListPtr,cListDelPtr ;
 std::vector<std::thread> thread;
 bvector<DTM_MULTI_THREAD> multiThread;
 /*
 ** Resize arrays
 */
 numThreadArrayPoints.resize (DTM_NUM_PROCESSORS);
 thread.resize (DTM_NUM_PROCESSORS);;
 multiThread.resize (DTM_NUM_PROCESSORS);
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Triangulating Dtm Object %p",dtmP) ;
 startTime = tinTime = bcdtmClock() ;
/*
** Initialise
*/
 numDtmPoints = dtmP->numPoints ;
/*
** Allocate Array For Saving Sorted Point Order
*/
 sortOfsP = ( long * ) malloc( dtmP->numPoints * sizeof(long)) ;
 if( sortOfsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set Point Offsets
*/
 for( p = 0 , longP = sortOfsP ; p < dtmP->numPoints ; ++p , ++longP ) *longP = p ;
/*
** Determine Number Of Points Per Thread
*/
 numThreadPoints = dtmP->numPoints / DTM_NUM_PROCESSORS + 1 ;
 if( dtmP->numPoints % DTM_NUM_PROCESSORS == 0 ) --numThreadPoints ;
 for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) numThreadArrayPoints[n] =  numThreadPoints ;
 if( dtmP->numPoints % DTM_NUM_PROCESSORS != 0 ) numThreadArrayPoints[DTM_NUM_PROCESSORS-1] = dtmP->numPoints - (DTM_NUM_PROCESSORS-1) * numThreadPoints ;
 if( dbg ) for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n  ) bcdtmWrite_message(0,0,0,"Thread[%2ld] ** numPoints = %6ld",n,numThreadArrayPoints[n]) ;
/*
** Initialise Multi Thread Triangulation Array
*/
 startPoint = 0 ;
 for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
   {
    multiThread[n].thread          = n ;
    multiThread[n].dtmP            = dtmP ;
    multiThread[n].startPoint      = startPoint ;
    multiThread[n].numPoints       = numThreadArrayPoints[n] ;
    multiThread[n].sortOfsP        = sortOfsP ;
    multiThread[n].leftMostPoint   = 0 ;
    multiThread[n].rightMostPoint  = 0 ;
    multiThread[n].topMostPoint    = 0 ;
    multiThread[n].bottomMostPoint = 0 ;
    multiThread[n].isColinear      = 0 ;
    multiThread[n].cListPtr        = startPoint * 6 ;
    multiThread[n].cListDelPtr     = dtmP->nullPtr  ;
    startPoint = startPoint + numThreadArrayPoints[n] ;
   }
/*
**  Allocate Nodes Memory For Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Nodes Memory For Dtm Object") ;
 if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Allocate Circular List Memory For Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Circular List Memory For Dtm Object") ;
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Initialise Circular List Parameters
*/
 dtmP->cListPtr = 0 ;
 dtmP->cListDelPtr = dtmP->nullPtr ;
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Create Threads To Triangulate Dtm Object
*/
 for( n = 1 ; n < DTM_NUM_PROCESSORS ; ++n )
   thread[n] = std::thread (bcdtmTin_multiThreadTriangulationWorkerDtmObject, &multiThread[n]);
/*
** Wait For All Threads To Complete
*/
 bcdtmTin_multiThreadTriangulationWorkerDtmObject(&multiThread[0]);
 for (n = 1; n < DTM_NUM_PROCESSORS; ++n)
     thread[n].join ();
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Merge Threaded Triangulations
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Threaded Triangulations") ;
/*
**  Set Merge Extents For Left Triangulation
*/
 p1l  = multiThread[0].leftMostPoint ;
 p1r  = multiThread[0].rightMostPoint ;
 col1 = multiThread[0].isColinear  ;
 numClist = multiThread[0].numPoints * 6 ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"p1l = %8ld p1r = %8ld col2 = %2ld",p1l,p1r,col1) ;
 dtmP->numPoints   = numDtmPoints ;
 dtmP->cListPtr    = multiThread[0].cListPtr ;
 dtmP->cListDelPtr = multiThread[0].cListDelPtr ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmP->cListPtr = %10ld dtmP->cListDelPtr = %10ld",dtmP->cListPtr,dtmP->cListDelPtr) ;
 for( cListPtr = dtmP->cListPtr ; cListPtr <  numClist ; ++cListPtr )
   {
    if( dtmP->cListDelPtr == dtmP->nullPtr )
      {
       dtmP->cListDelPtr = cListPtr ;
       clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr = dtmP->nullPtr ;
      }
    else
      {
       clistAddrP(dtmP,cListPtr)->nextPtr = dtmP->cListDelPtr ;
       dtmP->cListDelPtr = cListPtr ;
      }
   }
/*
** Write Delete List For Development Purposes
*/
 if( dbg == 2 )
   {
    cListDelPtr = dtmP->cListDelPtr ;
    while ( cListDelPtr != dtmP->nullPtr )
      {
       bcdtmWrite_message(0,0,0,"** cListDelPtr = %10ld cListDelPtr->nextPtr = %10ld",cListDelPtr,clistAddrP(dtmP,cListDelPtr)->nextPtr) ;
       cListDelPtr = clistAddrP(dtmP,cListDelPtr)->nextPtr ;
      }
   }
/*
** Merge Left And Right Triangulations
*/
 for( n = 1 ; n < DTM_NUM_PROCESSORS ; ++n )
   {
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
**  Set Merge Extents For Right Triangulation
*/
    p2l  = multiThread[n].leftMostPoint ;
    p2r  = multiThread[n].rightMostPoint ;
    col2 = multiThread[n].isColinear  ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"p2l = %8ld p2r = %8ld col2 = %2ld",p2l,p2r,col2) ;
/*
**  Map Unused Cyclic List Entries Back To Left Triangulation
*/
    dtmP->cListPtr = multiThread[n].cListPtr ;
    cListDelPtr    = multiThread[n].cListDelPtr ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmP->cListPtr = %10ld cListDelPtr = %10ld",dtmP->cListPtr,cListDelPtr) ;
    while( cListDelPtr != dtmP->nullPtr )
      {
       if( dtmP->cListDelPtr == dtmP->nullPtr )
         {
          dtmP->cListDelPtr = cListDelPtr ;
          clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr = dtmP->nullPtr ;
         }
       else
         {
          clistAddrP(dtmP,cListDelPtr)->nextPtr = dtmP->cListDelPtr ;
          dtmP->cListDelPtr = cListDelPtr ;
         }
       cListDelPtr = clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr ;
      }
/*
**  Write Delete List For Development Purposes
*/
    if( dbg == 2 )
      {
       cListDelPtr = dtmP->cListDelPtr ;
       while ( cListDelPtr != dtmP->nullPtr )
         {
          bcdtmWrite_message(0,0,0,"== cListDelPtr = %10ld cListDelPtr->nextPtr = %10ld",cListDelPtr,clistAddrP(dtmP,cListDelPtr)->nextPtr) ;
          cListDelPtr = clistAddrP(dtmP,cListDelPtr)->nextPtr ;
         }
      }
/*
**  Merge Triangulations
*/
    if( bcdtmTin_mergeTrianglesDtmObject(dtmP,p1l,p1r,col1,p2l,p2r,col2,DTM_X_AXIS,colinearPtsP)) goto errexit ;
    p1r  = p2r ;
    col1 = *colinearPtsP ;
/*
**  Map Unused Cyclic List Entries Back To Left Hand
*/
    numClist = numClist + multiThread[n].numPoints * 6 ;
    if( n < DTM_NUM_PROCESSORS - 1 )
      {
       for( cListPtr = dtmP->cListPtr ; cListPtr < numClist ; ++cListPtr )
         {
          if( dtmP->cListDelPtr == dtmP->nullPtr )
            {
             dtmP->cListDelPtr = cListPtr ;
             clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr = dtmP->nullPtr ;
            }
          else
            {
             clistAddrP(dtmP,cListPtr)->nextPtr = dtmP->cListDelPtr ;
             dtmP->cListDelPtr = cListPtr ;
            }
         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Threaded Triangulations Completed") ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Delaunay Triangulation Time     = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),tinTime)) ;
/*
** Reset Convex Hull
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reseting Convex Hull") ;
 dtmP->hullPoint      = p1l ;
 dtmP->nextHullPoint  = nodeAddrP(dtmP,p1l)->hPtr ;
 for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->hPtr = dtmP->nullPnt ;
 if( bcdtmTin_setConvexHullDtmObject(dtmP,dtmP->hullPoint,dtmP->nextHullPoint) != DTM_SUCCESS ) goto errexit ;
 if( tdbg == 2  ) bcdtmWrite_message(0,0,0,"**  Dtm Reset Hull Time      = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
/*
**  Check Topology
*/
    bcdtmWrite_message(0,0,0,"Checking Triangulation Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(0,0,0,"Triangulation Topology Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Topology Valid") ;
/*
**  Check Precision
*/
    bcdtmWrite_message(0,0,0,"Checking Triangulation Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(0,0,0,"Triangulation Precision Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Precision Valid") ;
   }
/*
** Reconstruct Sort Order
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reconstructing Sort Order") ;
 if( bcdtmTin_reconstructSortOrderDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Tin Reconstruction  Time        = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Reset Convex Hull First Pointers
*/
 dtmP->hullPoint = 0 ;
 dtmP->nextHullPoint  = nodeAddrP(dtmP,dtmP->hullPoint)->hPtr ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Cleanup
*/
 cleanup :
 if( sortOfsP != NULL ) free(sortOfsP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Total Triangulation Time        = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),tinTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Triangulating Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Triangulating Dtm Object Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret =  DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private long bcdtmTin_leftMostPointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Returns The Left Most Point
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 if( p1P->x < p2P->x || ( p1P->x == p2P->x &&  p1P->y <  p2P->y ) ) return(P1) ;
 return(P2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private long bcdtmTin_rightMostPointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Returns The Right Most Point
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 if( p1P->x > p2P->x || ( p1P->x == p2P->x &&  p1P->y >  p2P->y ) ) return(P1) ;
 return(P2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private long bcdtmTin_bottomMostPointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Returns The Bottom Most Point
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 if( p1P->y < p2P->y || ( p1P->y == p2P->y &&  p1P->x < p2P->x ) ) return(P1) ;
 return(P2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private long bcdtmTin_topMostPointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Returns The Bottom Most Point
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 if( p1P->y > p2P->y || ( p1P->y == p2P->y &&  p1P->x > p2P->x ) ) return(P1) ;
 return(P2) ;
}

BENTLEYDTM_Private void bcdtmTin_lrbtMostPointDtmObject (BC_DTM_OBJ* dtmP, long P1, long P2, long* lpnt, long* rpnt, long* bpnt, long* tpnt)
    {
    DTM_TIN_POINT *p1P, *p2P;
    p1P = pointAddrP (dtmP, P1);
    p2P = pointAddrP (dtmP, P2);
    if (p1P->x < p2P->x || (p1P->x == p2P->x &&  p1P->y < p2P->y))
        {
        *lpnt = P1; *rpnt = P2;
        }
    else
        {
        *lpnt = P2; *rpnt = P1;
        }
    //if (p1P->x > p2P->x || (p1P->x == p2P->x &&  p1P->y >  p2P->y))
    //    *rpnt = P1;
    //else
    //    *rpnt = P2;
    if (p1P->y < p2P->y || (p1P->y == p2P->y &&  p1P->x < p2P->x))
        {
        *bpnt = P1; *tpnt = P2;
        }
    else
        {
        *bpnt = P2; *tpnt = P1;
        }
    //if (p1P->y > p2P->y || (p1P->y == p2P->y &&  p1P->x > p2P->x))
    //    *tpnt = P1;
    //else
    //    *tpnt = P2;
    }

BENTLEYDTM_Private void bcdtmTin_lrbtMostPointFrom3PointsDtmObject (BC_DTM_OBJ* dtmP, long P1, long P2, long P3, long* lpnt, long* rpnt, long* bpnt, long* tpnt)
    {
    DTM_TIN_POINT *p1P, *p2P, *p3P;
    p1P = pointAddrP (dtmP, P1);
    p2P = pointAddrP (dtmP, P2);
    p3P = pointAddrP (dtmP, P3);

    if (p1P->x < p2P->x || (p1P->x == p2P->x &&  p1P->y < p2P->y))
        {
        *lpnt = (p1P->x < p3P->x || (p1P->x == p3P->x &&  p1P->y <  p3P->y)) ? P1 : P3;
        if (*lpnt == P3)
            *rpnt = P2;
        else
            *rpnt = (p2P->x < p3P->x || (p2P->x == p3P->x &&  p2P->y < p3P->y)) ? P3 : P2;
        }
    else
        {
        *lpnt = (p2P->x < p3P->x || (p2P->x == p3P->x &&  p2P->y <  p3P->y)) ? P2 : P3;
        if (*lpnt == P3)
            *rpnt = P1;
        else
            *rpnt = (p1P->x < p3P->x || (p1P->x == p3P->x &&  p1P->y < p3P->y)) ? P3 : P1;
        }

    //if (p1P->x > p2P->x || (p1P->x == p2P->x &&  p1P->y >  p2P->y))
    //    *rpnt = (p1P->x > p3P->x || (p1P->x == p3P->x &&  p1P->y >  p3P->y)) ? P1 : P3;
    //else
    //    *rpnt = (p2P->x > p3P->x || (p2P->x == p3P->x &&  p2P->y >  p3P->y)) ? P2 : P3;

    if (p1P->y < p2P->y || (p1P->y == p2P->y &&  p1P->x < p2P->x))
        {
        *bpnt = (p1P->y < p3P->y || (p1P->y == p3P->y &&  p1P->x < p3P->x)) ? P1 : P3;
        if (*bpnt == P3)
            *tpnt = P2;
        else
            *tpnt = (p2P->y < p3P->y || (p2P->y == p3P->y &&  p2P->x < p3P->x)) ? P3 : P2;
        }
    else
        {
        *bpnt = (p2P->y < p3P->y || (p2P->y == p3P->y &&  p2P->x < p3P->x)) ? P2 : P3;
        if (*bpnt == P3)
            *tpnt = P1;
        else
            *tpnt = (p1P->y < p3P->y || (p1P->y == p3P->y &&  p1P->x < p3P->x)) ? P3 : P1;
        }

    //if (p1P->y > p2P->y || (p1P->y == p2P->y &&  p1P->x > p2P->x))
    //    *tpnt = (p1P->y > p3P->y || (p1P->y == p3P->y &&  p1P->x > p3P->x)) ? P1 : P3;
    //else
    //    *tpnt = (p2P->y > p3P->y || (p2P->y == p3P->y &&  p2P->x > p3P->x)) ? P2 : P3;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_setConvexHullDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 long lp,p3 ;
 lp = p2 ;
 nodeAddrP(dtmP,p1)->hPtr = p2 ;
 do
   {
    if( (p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p1)) < 0 ) return(DTM_ERROR) ;
    nodeAddrP(dtmP,p2)->hPtr = p3 ;
    p1 = p2 ;
    p2 = p3 ;
   } while ( p3 != lp ) ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_delaunayTriangulateDtmObject(BC_DTM_OBJ *dtmP,long iofs,long ns,long axis,long *lpnt,long *rpnt,long *bpnt,long *tpnt,long *icol)
/*
** This function uses a divide and conquer strategy to triangulate tiled points
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 int    iside ;
 long   ns1,ns2,iofs1,iofs2,p1l,p1r,p1t,p1b,p2l,p2r,p2t,p2b ;
 long   icol1,icol2,m1l=0,m1r=0,m1col,m2l=0,m2r=0,m2col ;
/*
** Initialise variables
*/
 *icol = 0 ;
/*
** Two data points
*/
 if( ns == 2 )
   {
    *icol = 1 ;
    if( bcdtmList_insertLineDtmObject(dtmP,iofs,iofs+1)) goto errexit  ;
    bcdtmTin_lrbtMostPointDtmObject (dtmP, iofs, iofs + 1, lpnt, rpnt, bpnt, tpnt);
    //*lpnt = bcdtmTin_leftMostPointDtmObject(dtmP,iofs,iofs+1) ;
    //*rpnt = bcdtmTin_rightMostPointDtmObject(dtmP,iofs,iofs+1) ;
    //*bpnt = bcdtmTin_bottomMostPointDtmObject(dtmP,iofs,iofs+1) ;
    //*tpnt = bcdtmTin_topMostPointDtmObject(dtmP,iofs,iofs+1) ;
    nodeAddrP(dtmP,iofs)->hPtr = iofs+1 ;
   }
/*
** Three data points
*/
 else if( ns == 3 )
   {
    if( bcdtmList_insertLineDtmObject(dtmP,iofs,iofs+1))   goto errexit  ;
    if( bcdtmList_insertLineDtmObject(dtmP,iofs+1,iofs+2)) goto errexit  ;
    iside = bcdtmMath_pointSideOfDtmObject(dtmP,iofs,iofs+1,iofs+2) ;
    if( iside ==  0 )
      {
       *icol = 1 ;
       nodeAddrP(dtmP,iofs)->hPtr   = iofs+1 ;
       nodeAddrP(dtmP,iofs+1)->hPtr = iofs+2 ;
      }
    else
      {
       if( bcdtmList_insertLineDtmObject(dtmP,iofs,iofs+2) ) goto errexit  ;
       if( iside == -1 ) bcdtmTin_setConvexHullDtmObject(dtmP,iofs,iofs+2) ;
       if( iside ==  1 ) bcdtmTin_setConvexHullDtmObject(dtmP,iofs,iofs+1) ;
      }
    bcdtmTin_lrbtMostPointFrom3PointsDtmObject (dtmP, iofs, iofs + 1, iofs + 2, lpnt, rpnt, bpnt, tpnt);
 //   *lpnt = *rpnt = iofs ;
 //   *lpnt = bcdtmTin_leftMostPointDtmObject(dtmP,*lpnt,iofs+1)  ;
 //   *lpnt = bcdtmTin_leftMostPointDtmObject(dtmP,*lpnt,iofs+2)  ;
 //   *rpnt = bcdtmTin_rightMostPointDtmObject(dtmP,*rpnt,iofs+1) ;
 //   *rpnt = bcdtmTin_rightMostPointDtmObject(dtmP,*rpnt,iofs+2) ;
    //*tpnt = *bpnt = iofs ;
 //   *bpnt = bcdtmTin_bottomMostPointDtmObject(dtmP,*bpnt,iofs+1) ;
 //   *bpnt = bcdtmTin_bottomMostPointDtmObject(dtmP,*bpnt,iofs+2) ;
 //   *tpnt = bcdtmTin_topMostPointDtmObject(dtmP,*tpnt,iofs+1) ;
 //   *tpnt = bcdtmTin_topMostPointDtmObject(dtmP,*tpnt,iofs+2) ;
   }
/*
** More than three data points so divide and conquer
*/
 else
   {
    ns1 = ns / 2  ;
    ns2 = ns - ns1 ;
    iofs1 = iofs   ;
    iofs2 = iofs + ns1 ;
    if( bcdtmTin_delaunayTriangulateDtmObject(dtmP,iofs1,ns1,1-axis,&p1l,&p1r,&p1b,&p1t,&icol1) ) goto errexit  ;
    if( bcdtmTin_delaunayTriangulateDtmObject(dtmP,iofs2,ns2,1-axis,&p2l,&p2r,&p2b,&p2t,&icol2) ) goto errexit  ;
/*
**  Set Limits For Hull Merge
*/
    if( axis == DTM_Y_AXIS  ) { m1l = p2t ; m1r = p2b ; m1col = icol2 ; m2l = p1t ; m2r = p1b ; m2col = icol1 ; }
    else                      { m1l = p1l ; m1r = p1r ; m1col = icol1 ; m2l = p2l ; m2r = p2r ; m2col = icol2 ; }
/*
**  Merge Triangulated Hulls
*/
    if( bcdtmTin_mergeTrianglesDtmObject(dtmP,m1l,m1r,m1col,m2l,m2r,m2col,axis,icol) != DTM_SUCCESS ) goto errexit  ;
/*
**  Reset Hull Exterimities
*/
    *lpnt = bcdtmTin_leftMostPointDtmObject(dtmP,p1l,p2l) ;
    *rpnt = bcdtmTin_rightMostPointDtmObject(dtmP,p1r,p2r) ;
    *bpnt = bcdtmTin_bottomMostPointDtmObject(dtmP,p1b,p2b) ;
    *tpnt = bcdtmTin_topMostPointDtmObject(dtmP,p1t,p2t) ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
/*
 bcdtmWrite_message(0,0,0,"trg axis = %1ld",axis) ;
 bcdtmWrite_message(0,0,0,"m1l = %10ld  ** %10.4lf %10.4lf",m1l,pointAddrP(dtmP,m1l)->x,pointAddrP(dtmP,m1l)->y) ;
 bcdtmWrite_message(0,0,0,"m1r = %10ld  ** %10.4lf %10.4lf",m1r,pointAddrP(dtmP,m1r)->x,pointAddrP(dtmP,m1r)->y) ;
 bcdtmWrite_message(0,0,0,"m2l = %10ld  ** %10.4lf %10.4lf",m2l,pointAddrP(dtmP,m2l)->x,pointAddrP(dtmP,m2l)->y) ;
 bcdtmWrite_message(0,0,0,"m2r = %10ld  ** %10.4lf %10.4lf",m2r,pointAddrP(dtmP,m2r)->x,pointAddrP(dtmP,m2r)->y) ;
*/
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_mergeTrianglesDtmObject(BC_DTM_OBJ *dtmP,long p1l,long p1r,long colinear1,long p2l,long p2r,long colinear2,long axis,long *colinearP)
/*
** This function merges the two hulls created by the divide and conquer
** startegy for a cluster sorted tin object
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 long   l,r,l1,r1,l2,r2,llp,lrp,ulp,urp ;
 long   right=1,left=1,ap=0,bp=0 ;
/*
** Write Entry Message
*/
//bcdtmWrite_message(0,0,0,"axis = %2ld ** p1l = %9ld p1r = %9ld col1 = %2ld p2l = %9ld p2r = %9ld col2 = %2ld",axis,p1l,p1r,colinear1,p2l,p2r,colinear2) ;
/*
** Check For Colinear Hulls
*/
 *colinearP = 0 ;
 if( colinear1 == 1 && colinear2 == 1 )
   {
    if( ( bcdtmMath_pointSideOfDtmObject(dtmP,p1l,p1r,p2l) == 0 && bcdtmMath_pointSideOfDtmObject(dtmP,p1l,p1r,p2r) == 0  )  ||
        ( bcdtmMath_pointSideOfDtmObject(dtmP,p2r,p2l,p1r) == 0 && bcdtmMath_pointSideOfDtmObject(dtmP,p2r,p2l,p1l) == 0  )      )
      {
       *colinearP = 1 ;
       if(bcdtmList_insertLineDtmObject(dtmP,p1r,p2l)) goto errexit ;
       nodeAddrP(dtmP,p1r)->hPtr = p2l ;
       return(0) ;
      }
    else
      {
       if( bcdtmTin_getLcTangentDtmObject(dtmP,p1l,p1r,p2l,p2r,colinear1,colinear2,&llp,&lrp) ) goto errexit ;
       if( bcdtmTin_getUcTangentDtmObject(dtmP,p1l,p1r,p2l,p2r,colinear1,colinear2,&ulp,&urp) ) goto errexit ;
       if( llp == ulp && lrp == urp )
         {
          *colinearP = 1 ;
          if(bcdtmList_insertLineDtmObject(dtmP,p1r,p2l)) goto errexit ;
          nodeAddrP(dtmP,p1r)->hPtr = p2l ;
          return(0) ;
         }
      }
   }
/*
** Get Upper and Lower Common Tangents
*/
 if( bcdtmTin_getLcTangentDtmObject(dtmP,p1l,p1r,p2l,p2r,colinear1,colinear2,&llp,&lrp) ) goto errexit ;
// if( bcdtmTin_getUcTangentDtmObject(dtmP,p1l,p1r,p2l,p2r,colinear1,colinear2,&ulp,&urp) ) goto errexit ;
/*
** Get Before Point For Line Insertion Purposes
*/
 if( colinear1 == 1 )
   {
    if( axis == DTM_X_AXIS )
      {
       if     ( llp == p1l ) bp = p1l + 1 ;
       else if( llp == p1r ) bp = p1r - 1 ;
       else                  bp = llp - 1 ;
      }
    else //if( axis == DTM_Y_AXIS )
      {
       if( llp == p1l )
         {
          if( p1l < p1r ) bp = p1l + 1 ;
          else            bp = p1l - 1 ;
         }
       else if( llp == p1r )
         {
          if( p1r < p1l ) bp = p1r + 1 ;
          else            bp = p1r - 1 ;
         }
      }
   }
 else if(( bp = bcdtmList_nextClkDtmObject(dtmP,llp,nodeAddrP(dtmP,llp)->hPtr)) < 0 ) goto errexit ;
/*
** Get After Point For Line Insertion Purposes
*/
 if( colinear2 == 1 )
   {
    if( axis == DTM_X_AXIS )
      {
       if     ( lrp == p2l ) ap = p2l + 1 ;
       else if( lrp == p2r ) ap = p2r - 1 ;
       else                  ap = lrp - 1 ;
      }
    else //if( axis == DTM_Y_AXIS )
      {
       if( lrp == p2l )
         {
          if( p2l < p2r ) ap = p2l + 1 ;
          else            ap = p2l - 1 ;
         }
       else if( lrp == p2r )
         {
          if( p2r < p2l ) ap = p2r + 1 ;
          else            ap = p2r - 1 ;
         }
      }
   }
 else ap = nodeAddrP(dtmP,lrp)->hPtr ;
/*
** Merge Data Sets
*/
 l = llp ; r = lrp ;
 while ( left || right )
   {
    left = right = 1 ;
/*
** Insert Line
*/
    if( bcdtmList_insertLineBeforePointDtmObject(dtmP,l,r,bp) ) goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,r,l,ap) ) goto errexit ;
/*
** Remove Lines To The Left If Delaunay Criteria Applies
*/
     if( (l1 = bcdtmList_nextAntDtmObject(dtmP,l,r)) < 0 ) goto errexit ;
     if( bcdtmMath_pointSideOfDtmObject(dtmP,l,r,l1) > 0 )
       {
        if( colinear1 == 0 )
          {
           if( ( l2 = bcdtmList_nextAntDtmObject(dtmP,l,l1) ) < 0 ) goto errexit ;
           while ( bcdtmTin_inCircleTestDtmObject(dtmP,1,l,r,l1,l2) )
             {
              if( bcdtmList_deleteLineDtmObject(dtmP,l,l1) ) goto errexit ;
              l1 = l2  ;
              if( ( l2 = bcdtmList_nextAntDtmObject(dtmP,l,l1) ) < 0 ) goto errexit ;
             }
          }
       }
     else left = 0 ;
/*
** Remove Lines To The Right If Delaunay Criteria Applies
*/
    if( ( r1 = bcdtmList_nextClkDtmObject(dtmP,r,l)) < 0 )    goto errexit ;
    if( bcdtmMath_pointSideOfDtmObject(dtmP,l,r,r1) > 0 )
      {
       if( colinear2 == 0 )
         {
          if( ( r2 = bcdtmList_nextClkDtmObject(dtmP,r,r1)) < 0 ) goto errexit ;
          while( bcdtmTin_inCircleTestDtmObject(dtmP,2,l,r,r1,r2) )
            {
             if( bcdtmList_deleteLineDtmObject(dtmP,r,r1) ) goto errexit ;
             r1 = r2 ;
             if( ( r2 = bcdtmList_nextClkDtmObject(dtmP,r,r1)) < 0 ) goto errexit ;
            }
         }
      }
    else right = 0 ;
/*
** Determine Next Line To Insert
*/
    if(   left && ! right ) { bp = ap = l ; l = l1 ; }
    else if( ! left &&   right ) { bp = ap = r ; r = r1 ; }
    else if(   left &&   right )
      {
       if     ( bcdtmMath_pointSideOfDtmObject(dtmP,l,l1,r1) >= 0 )  { bp = ap = l ; l = l1 ; }
       else if( bcdtmMath_pointSideOfDtmObject(dtmP,r,r1,l1) <= 0 )  { bp = ap = r ; r = r1 ; }
       else if( bcdtmTin_inCircleTestDtmObject(dtmP,3,l,r,l1,r1))    { bp = ap = r ; r = r1 ; }
       else                                                          { bp = ap = l ; l = l1 ; }
      }
   }
/*
** Set Upper Tangent
*/
 ulp = l ; urp = r ;
/*
** Set Convex Hull Pointers
*/
 if( colinear1 == 1 || colinear2 == 1 )
   {
    for( l = p1l ; l <= p2r ; ++l ) nodeAddrP(dtmP,l)->hPtr = dtmP->nullPnt ;
    bcdtmTin_setConvexHullDtmObject(dtmP,llp,lrp) ;
   }
 else
   {
    nodeAddrP(dtmP,llp)->hPtr = lrp ;
    nodeAddrP(dtmP,urp)->hPtr = ulp ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_getLcTangentDtmObject(BC_DTM_OBJ *dtmP,long p1l,long p1r,long p2l,long p2r,long colinear1, long colinear2, long *left,long *right)
/*
** This function get the lower common tangent of two adjacent hulls
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 long s,nl,nr,loop=1 ;
/*
** Initialise
*/
 *left  = p1r ;
 *right = p2l ;
 if( colinear1 ) nl = p1l ;
 else    {  nl = nodeAddrP(dtmP,*left)->hPtr ; if(( nl = bcdtmList_nextClkDtmObject(dtmP,*left,nl)) < 0 ) goto  errexit ; }
 if( colinear2 ) nr = p2r ;
 else       nr = nodeAddrP(dtmP,*right)->hPtr ;
/*
** Loop Until Lower Common Tangent Found
*/
 while ( loop )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,*left,*right,nr) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,*right,nr,*left) < 0 )
      {
       if( colinear2 ) {  s = *right ; *right = nr ; nr = s ; }
       else            { *right = nr ; nr = nodeAddrP(dtmP,*right)->hPtr ; }
      }
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,*left,*right,nl) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,*right,nl,*left) < 0 )
      {
       if( colinear1 ) {  s = *left ; *left = nl ; nl = s ; }
       else            {  s = nl ; if( (nl = bcdtmList_nextClkDtmObject(dtmP,nl,*left)) < 0 ) goto errexit ; *left = s ; }
      }
    else loop = 0 ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"lct ** p1l = %6ld p1r = %6ld p2l = %6ld p2r = %6ld",p1l,p1r,p2l,p2r) ;
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_getUcTangentDtmObject(BC_DTM_OBJ *dtmP,long p1l,long p1r,long p2l,long p2r,long col1, long col2, long *l,long *r)
/*
** This function get the upper common tangent of two adjacent hulls
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 long  s,nl,nr,loop=1 ;
/*
** Initialise
*/
 *l = p1r ; *r = p2l ;
 if( col1 ) nl = p1l ;
 else       nl = nodeAddrP(dtmP,*l)->hPtr ;
 if( col2 ) nr = p2r ;
 else    {  nr = nodeAddrP(dtmP,*r)->hPtr ; if( (nr = bcdtmList_nextClkDtmObject(dtmP,*r,nr)) < 0 ) return(1) ; }
/*
** Loop Until Upper Common Tangent Found
*/
 while ( loop )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,*l,*r,nr) > 0 && bcdtmMath_pointSideOfDtmObject(dtmP,*r,nr,*l) > 0 )
      {
       if( col2 ) {  s = *r ; *r = nr ; nr = s ; }
       else       {  s = nr ; if( (nr = bcdtmList_nextClkDtmObject(dtmP,nr,*r)) < 0 ) goto errexit ; *r = s ; }
      }
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,*l,*r,nl) > 0 &&  bcdtmMath_pointSideOfDtmObject(dtmP,*r,nl,*l) > 0 )
      {
       if ( col1 ) {  s = *l ; *l = nl ; nl = s ; }
       else        { *l = nl ; nl = nodeAddrP(dtmP,*l)->hPtr ; }
      }
    else loop = 0 ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"uct ** p1l = %6ld p1r = %6ld p2l = %6ld p2r = %6ld",p1l,p1r,p2l,p2r) ;
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_inCircleTestDtmObject(BC_DTM_OBJ *dtmP,long maxMinTest,long P1,long P2,long P3,long P4)
/*
** Returns a positive value if P4 lies in the circle passing through P1 P2
** and P3. P1 P2 and P3 must be in an anticlockwise direction
*/
{
 double adx,bdx,cdx,ady,bdy,cdy;
 double bdxcdy,cdxbdy,cdxady,adxcdy,adxbdy,bdxady;
 double det,aval,bval,cval;
 DTM_TIN_POINT  *p1P,*p2P,*p3P,*p4P ;
/*
** Get Point Addresses
*/

 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p4P = pointAddrP(dtmP,P4) ;
/*
** Initialise
*/
 adx = p1P->x - p4P->x ;
 bdx = p2P->x - p4P->x ;
 ady = p1P->y - p4P->y ;
 bdy = p2P->y - p4P->y ;
 adxbdy = adx * bdy;
 bdxady = bdx * ady;
/*
** Test For Validity Of Triangle P1P2P4
*/
 if( adxbdy - bdxady <= 0 ) return(FALSE) ;

 p3P = pointAddrP(dtmP,P3) ;
 cdx = p3P->x - p4P->x ;
 cdy = p3P->y - p4P->y ;

 cval  = cdx * cdx + cdy * cdy;

 bdxcdy = bdx * cdy;
 cdxbdy = cdx * bdy;
 aval   = adx * adx + ady * ady;

 cdxady = cdx * ady;
 adxcdy = adx * cdy;
 bval   = bdx * bdx + bdy * bdy;

 det = aval * (bdxcdy - cdxbdy) + bval * (cdxady - adxcdy) + cval * (adxbdy - bdxady) ;
/*
** RobC - July 2004
** If the determinat value is less than a certain value
** use the maxMin test as it it more precision tolerant.
** Not quite sure what the value should be before switching
** to the maxMin test
*/
 if( fabs(det) < 0.001 )   // Possibilty Of Precision Problem So Use MaxMinTest
   {
    if( maxMinTest == 1 ) return(bcdtmTin_maxMinTestDtmObject(dtmP,P2,P4,P1,P3));
    if( maxMinTest == 2 ) return(bcdtmTin_maxMinTestDtmObject(dtmP,P1,P4,P2,P3));
    if( maxMinTest == 3 ) return(bcdtmTin_maxMinTestDtmObject(dtmP,P1,P4,P2,P3));
   }
/*
** Job Completed
*/
 if( det > 0.0 ) return(TRUE) ;
 else            return(FALSE) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_maxMinTestDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2,long p3,long p4)
{
/*
** This Function Determines If The Exchange Of Two Triangles is necessary
** on the Basis Of Maximizing The Mininum Angle
**
** P1,P2,P3,P4 Form a Quadrilateral with P3 and P4 Connected Diagonally
**
** Return Values = 0 No Excahnge
**               = 1 Exchange
*/
 double x1,y1,x2,y2,x3,y3,x4,y4 ;
 double a1sq,a2sq,b1sq,b2sq,c1sq,c2sq ;
 double s1sq,s2sq,s3sq,s4sq,s1max,s2max,s3max,s4max ;
 double s1min,s2min,u1,u2,u3,u4 ;
 DTM_TIN_POINT  *p1P,*p2P,*p3P,*p4P ;
/*
** Get Point Addresses
*/
 p1P = pointAddrP(dtmP,p1) ;
 p2P = pointAddrP(dtmP,p2) ;
 p3P = pointAddrP(dtmP,p3) ;
 p4P = pointAddrP(dtmP,p4) ;
/*
** Normalise Coordinates
*/
 x1 = 0.0 ;
 y1 = 0.0 ;
 x2 = p2P->x - p1P->x ;
 y2 = p2P->y - p1P->y ;
 x3 = p3P->x - p1P->x ;
 y3 = p3P->y - p1P->y ;
 x4 = p4P->x - p1P->x ;
 y4 = p4P->y - p1P->y ;
/*
** Caculate Triangle Areas
*/
 u3 = (y2-y3)*(x1-x3) - (x2-x3)*(y1-y3) ;
 u4 = (y1-y4)*(x2-x4) - (x1-x4)*(y2-y4) ;
 if( u3 * u4 <= 0.0 ) return(0) ;
 u1 = (y3-y1)*(x4-x1) - (x3-x1)*(y4-y1) ;
 u2 = (y4-y2)*(x3-x2) - (x4-x2)*(y3-y2) ;
/*
** Calculate Square Of Triangle Edges
*/
 a1sq = (x1-x3)*(x1-x3) + (y1-y3)*(y1-y3) ;
 b1sq = (x4-x1)*(x4-x1) + (y4-y1)*(y4-y1) ;
 c1sq = (x3-x4)*(x3-x4) + (y3-y4)*(y3-y4) ;
 a2sq = (x2-x4)*(x2-x4) + (y2-y4)*(y2-y4) ;
 b2sq = (x3-x2)*(x3-x2) + (y3-y2)*(y3-y2) ;
 c2sq = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) ;
/*
** Determine Longest Triangle Edges
*/
 if( a1sq > b1sq ) s1max = a1sq ; else s1max = b1sq ;
 if( a2sq > b2sq ) s2max = a2sq ; else s2max = b2sq ;
 if( b2sq > a1sq ) s3max = b2sq ; else s3max = a1sq ;
 if( b1sq > a2sq ) s4max = b1sq ; else s4max = a2sq ;
/*
** Determine Angles
*/
 s1sq = u1 * u1 / ( c1sq * s1max ) ;
 s2sq = u2 * u2 / ( c1sq * s2max ) ;
 s3sq = u3 * u3 / ( c2sq * s3max ) ;
 s4sq = u4 * u4 / ( c2sq * s4max ) ;

 if( s1sq < s2sq ) s1min = s1sq ; else s1min = s2sq ;
 if( s3sq < s4sq ) s2min = s3sq ; else s2min = s4sq ;

 if( s1min < s2min ) return(1) ;

 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_reconstructSortOrderDtmObject(BC_DTM_OBJ *dtmP)
/*
** This reconstructs the XY sort order for the Dtm Points after tile triangulation
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p,ofs,clPtr ;
 DTM_CIR_LIST  *clistP  ;
 DTM_TIN_POINT dtmPoint ;
 DTM_TIN_NODE  dtmNode  ;
/*
** Write Entry message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reconstructing Dtm Sort Order") ;
/*
** Reset Hull Pointers
*/
 p = dtmP->hullPoint ;
 do
   {
    ofs = nodeAddrP(dtmP,p)->hPtr ;
    nodeAddrP(dtmP,p)->hPtr = nodeAddrP(dtmP,nodeAddrP(dtmP,p)->hPtr)->fPtr  ;
    p = ofs ;
   } while ( p != dtmP->hullPoint ) ;
/*
** Adjust Circular List Table Point Numbers
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Circular List") ;
 for( clPtr = 0  ; clPtr < dtmP->cListPtr ; ++clPtr )
   {
    clistP = clistAddrP(dtmP,clPtr) ;
    if( clistP->pntNum >= 0 && clistP->pntNum < dtmP->numPoints ) clistP->pntNum = nodeAddrP(dtmP,clistP->pntNum)->fPtr ;
   }
/*
** Allocate Memory For Sort Pointers
*/
 LongArray sortP;

 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Sort Pointer") ;
 sortP.resize(dtmP->numPoints);

/*
** Calculate Sort Position For Each Dtm Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Sort Position Indexes") ;
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    *(sortP+nodeAddrP(dtmP,ofs)->fPtr) = ofs  ;
   }
/*
** Place Dtm Points In Sort Order
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing DTM Points In Sort Order") ;
 LongArray::iterator sP;
 for( ofs = 0 , sP = sortP.start() ; ofs < dtmP->numPoints ; ++ofs , ++sP )
   {
    if( ofs != *sP )
      {
/*
**     Swap Points
*/
       dtmPoint = *pointAddrP(dtmP,ofs) ;
       *pointAddrP(dtmP,ofs) = *pointAddrP(dtmP,*sP) ;
       *pointAddrP(dtmP,*sP) = dtmPoint ;
       dtmNode = *nodeAddrP(dtmP,ofs) ;
       *nodeAddrP(dtmP,ofs) = *nodeAddrP(dtmP,*sP) ;
       *nodeAddrP(dtmP,*sP) = dtmNode ;
/*
**     Update Sort Pointer
*/
       *(sortP+nodeAddrP(dtmP,*sP)->fPtr) = *sP ;
      }
   }
/*
** Null out saved sort order
*/
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs ) nodeAddrP(dtmP,ofs)->fPtr = dtmP->nullPtr ;
/*
** Check Sort Order
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Sort Order After Reconstruction") ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(2,0,0,"Dtm Sort Order Error") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Sort Order After Reconstruction Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reconstructing Dtm Sort Order Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reconstructing Dtm Sort Order Error") ;
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
BENTLEYDTM_Public int bcdtmTin_precisionRemoveEdgeSliversDtmObject(BC_DTM_OBJ *dtmP,double plTol)
/*
** This Function removes Tin Hull Slivers
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,process ;
 double d1,d2,d3,n1 ;
 DTM_TIN_POINT *p3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Precision Removing Edge Slivers ** plTol = %12.5lf",plTol) ;
/*
** Scan Hull And Check For Coincident Points
*/
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    p1 = dtmP->hullPoint ;
    do
      {
       p2 = nodeAddrP(dtmP,p1)->hPtr ;
       if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ;
       if( nodeAddrP(dtmP,p3)->hPtr == dtmP->nullPnt )
         {
          d1 = bcdtmMath_pointDistanceDtmObject(dtmP,p1,p2) ;
          d2 = bcdtmMath_pointDistanceDtmObject(dtmP,p1,p3) ;
          d3 = bcdtmMath_pointDistanceDtmObject(dtmP,p2,p3) ;
          if( d2 < d1 && d3 < d1 )
            {
             p3P = pointAddrP(dtmP,p3) ;
             if( ( n1 = bcdtmMath_normalDistanceToLineDtmObject(dtmP,p1,p2,p3P->x,p3P->y)) < plTol )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Deleting %6ld %6ld ** ND = %20.15lf",p1,p2,n1) ;
                if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) goto errexit  ;
                nodeAddrP(dtmP,p1)->hPtr = p3 ;
                nodeAddrP(dtmP,p3)->hPtr = p2 ;
                process = 1 ;
               }
            }
         }
       p1 = p2 ;
      }while ( p1 != dtmP->hullPoint ) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Precision Removing Edge Slivers Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Precision Removing Edge Slivers Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret =  DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long firstCall,BC_DTM_FEATURE **dtmFeaturePP,long *dtmFeatureNumP )
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature ;
 thread_local static long lastFeature=-1 ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence") ;
/*
** Initialise
*/
 *dtmFeaturePP   = NULL ;
 *dtmFeatureNumP = 0    ;
/*
** Do First Call Processing
*/
 if( firstCall == TRUE ) lastFeature = -1 ;
/*
** Scan feature For Next Occurrence
*/
 for( feature = lastFeature + 1 ; feature < dtmP->numFeatures && *dtmFeaturePP == NULL ; ++feature )
   {
    dtmFeatureP =  ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted  && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
      {
       *dtmFeaturePP   = dtmFeatureP ;

       if (((*(dtmFeaturePP))->dtmFeaturePts).pointsPI < 1000)
        {
        *dtmFeaturePP   = dtmFeatureP ;
        }

       *dtmFeatureNumP = feature ;
       lastFeature     = feature ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence Error") ;
 return(ret) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_countNumberOfDtmFeatureTypeOccurrencesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long *numOccurencesP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature,partNum,partOfs ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences") ;
/*
** Initialise
*/
 *numOccurencesP = 0 ;
/*
** Only Count If Features Are Present
*/
 if( dtmP->numFeatures > 0 )
   {
/*
** Scan Features Array
*/
    partNum = 0 ;
    partOfs = 0 ;
    dtmFeatureP = dtmP->fTablePP[partNum] ;
    for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
      {
       if( dtmFeatureP->dtmFeatureType == dtmFeatureType && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted   && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback) ++*numOccurencesP ;
       ++partOfs ;
       if( partOfs == dtmP->featurePartitionSize )
         {
          partOfs = 0 ;
          ++partNum   ;
          dtmFeatureP = dtmP->fTablePP[partNum] ;
         }
       else  ++dtmFeatureP ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences Error") ;
 return(ret) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_processConnectedLinesDtmObject(BC_DTM_OBJ *dtmP,long numGraphicBreaks,long numHardBreaks,long numSoftBreaks,long numContourLines)
/*
** This Function Inserts Graphic Break Lines
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Processing Connected Lines") ;
    bcdtmWrite_message(0,0,0,"numGraphicBreaks = %8ld",numGraphicBreaks) ;
    bcdtmWrite_message(0,0,0,"numContourLines  = %8ld",numContourLines) ;
    bcdtmWrite_message(0,0,0,"numHardBreaks    = %8ld",numHardBreaks) ;
   }
/*
** Connect Lines
*/
 if( numGraphicBreaks > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Connecting Graphic Breaks") ;
    if( bcdtmTin_connectLinesDtmObject(dtmP,DTMFeatureType::GraphicBreak,numGraphicBreaks)) goto errexit ;
   }
 if( numContourLines  > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Connecting Contour Lines") ;
    if( bcdtmTin_connectLinesDtmObject(dtmP,DTMFeatureType::ContourLine,numContourLines))   goto errexit ;
   }
 if( numSoftBreaks    > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Connecting Soft Breaks") ;
    if( bcdtmTin_connectLinesDtmObject(dtmP,DTMFeatureType::SoftBreakline,numSoftBreaks))       goto errexit ;
   }
 if( numHardBreaks    > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Connecting Hard Breaks") ;
    if( bcdtmTin_connectLinesDtmObject(dtmP,DTMFeatureType::Breakline,numHardBreaks))       goto errexit ;
   }
 /*
** Clean Up
*/
 cleanup:
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Connected Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Connected Lines Error") ;
 return(ret) ;
/*
** Errexit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_connectLinesDtmObject
(
 BC_DTM_OBJ *dtmP ,
 DTMFeatureType dtmFeatureType,
 long numFeatures
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,p1,p2,loop,feature,lineConnected ;
 long numConnectLines=0,numConnected=0,numUnconnected=0,lastNumUnconnected=0;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Connecting %8d Features Of dtmFeatureType = %4ld",numFeatures,dtmFeatureType) ;
/*
** Only Connect If Features Are Present
*/
 if( dtmP->numFeatures > 0 )
   {
    loop = 0 ;
    numConnectLines = 0 ;
    numUnconnected = numFeatures ;
    lastNumUnconnected = numFeatures * 100 ;
/*
**  Loop Through Features Until All Lines Connected
*/
     while  (  numUnconnected > 0 && numUnconnected < lastNumUnconnected  &&  loop < 40 )
      {
       ++loop ;
       lastNumUnconnected = numUnconnected ;
       numUnconnected = 0 ;
       numConnected   = 0 ;
/*
**     Scan Features Array
*/
        for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
         {
          dtmFeatureP = ftableAddrP(dtmP,feature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray && dtmFeatureP->dtmFeatureType == dtmFeatureType )
            {
            long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
             for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts - 1 ; ++n )
               {
                if( loop == 1 ) ++numConnectLines ;
                p1 = offsetP[n] ;
                p2 = offsetP[n+1] ;
/*
**              Test If Line Is Connected
*/
                if( bcdtmList_testLineDtmObject(dtmP,p1,p2)) ++numConnected ;
/*
**              Line Is Un Connected - Attempt To Connect Line
*/
                else
                  {
                   if( bcdtmTin_swapTinLinesThatIntersectConnectLineDtmObject(dtmP,p1,p2,&lineConnected)) goto errexit ;
                   if( lineConnected ) ++numConnected ;
                   else ++numUnconnected ;
                  }
               }
            }
         }
/*
**     Set lastNumUnconnected
*/
       if( loop == 1 ) lastNumUnconnected = numUnconnected + 1 ;
/*
**     Write States
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Loop[%2ld] ** numConnectLines = %8ld numConnected = %8ld numUnconnected = %8ld lastNumUnconnected = %8ld",loop,numConnectLines,numConnected,numUnconnected,lastNumUnconnected) ;
      }
/*
**  Reset Point Offsets
*/
/*
    for( feature = 0 ; feature < 0 ; ++feature )
      {
       dtmFeatureP = ftableAddrP(dtmP,feature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray && dtmFeatureP->dtmFeatureType == dtmFeatureType )
         {
          for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts - 1 ; ++n )
            {
             if( bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] < 0 ) bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] = - bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] - 1 ;
            }
         }
      }
*/
   }
/*
** Clean Up
*/
 cleanup:
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Lines Error") ;
 return(ret) ;
/*
** Errexit
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
BENTLEYDTM_Public int bcdtmTin_swapTinLinesThatIntersectConnectLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 long startPnt,
 long lastPnt,
 long *lineConnectedP
)
/*
** This Function Swaps Tin Lines That Intersect A Break Line
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int    sd1,sd2,sd3 ;
 long   L1,L2,Sp,P1,P2,P3,P4 ;
 double d1,d2,X1,X2,Y1,Y2 ;
/*
** Write Entry Message
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Swapping Tin Lines That Intersect Connect Line %6ld %6ld",startPnt,lastPnt) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"startPnt = %6ld ** %10.4lf %10.4lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
    bcdtmWrite_message(0,0,0,"lastPnt  = %6ld ** %10.4lf %10.4lf %10.4lf",lastPnt,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,lastPnt)->z) ;
    bcdtmWrite_message(0,0,0,"Angle startPntlastPnt = %12.10lf",bcdtmMath_getAngle(pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y)) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,startPnt) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,lastPnt) ;
   }
/*
** Scan From startPnt To lastPnt And Swap Lines That Intersect line startPntlastPnt
*/
 *lineConnectedP = 1 ;
/*
** Get Start Triangle Or Point
*/
 P1 = Sp = startPnt ;
 if( bcdtmTin_getSwapTriangleDtmObject(dtmP,P1,lastPnt,&P2,&P3,&P4)) goto errexit ;
 if( P2 == dtmP->nullPnt ) { *lineConnectedP = 0 ; goto cleanup  ; }
 while ( P1 != lastPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sp = %10ld P1 = %10ld P2 = %10ld P3 = %10ld P4 = %10ld",Sp,P1,P2,P3,P4) ;
/*
**  Line Passes Through A Point
*/
    if( P3 == dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"SplastPnt Passes Through P1") ;
       Sp = P1 = P2 ;
       if( P1 != lastPnt ) if( bcdtmTin_getSwapTriangleDtmObject(dtmP,P1,lastPnt,&P2,&P3,&P4)) goto errexit ;
       if( P2 == dtmP->nullPnt ) { *lineConnectedP = 0 ; goto cleanup  ; }
      }
/*
**  Line Passes Through A Line
**  Check If Line Can Be Swapped
*/
    else
      {
       sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,P1,P4,P2) ;
       sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,P1,P4,P3) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"00 sd1 = %2ld sd2 = %2ld",sd1,sd2) ;
/*
**     Check Line Is Not Closer Than Point To Line Tolerance
*/
       d1  = bcdtmMath_distanceOfPointFromLine(&L1,pointAddrP(dtmP,Sp)->x,pointAddrP(dtmP,Sp)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,&X1,&Y1) ;
       d2  = bcdtmMath_distanceOfPointFromLine(&L2,pointAddrP(dtmP,Sp)->x,pointAddrP(dtmP,Sp)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,&X2,&Y2) ;
       if     ( pointAddrP(dtmP,P2)->x == X1 && pointAddrP(dtmP,P2)->y == Y1 ) sd1 = 0 ;
       else if( pointAddrP(dtmP,P3)->x == X2 && pointAddrP(dtmP,P3)->y == Y2 ) sd2 = 0 ;
       else if( d1 < d2 && d1 < dtmP->plTol ) sd1 = 0 ;
       else if( d2 < d1 && d2 < dtmP->plTol ) sd2 = 0 ;
/*
**     Swap Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"01 sd1 = %2ld sd2 = %2ld",sd1,sd2) ;
       if( sd1 >  0 && sd2 < 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Swapping %10ld %10ld With %10ld %10ld",P2,P3,P1,P4) ;
          if( bcdtmList_deleteLineDtmObject(dtmP,P2,P3) ) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,P4,P2) ) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P4,P1,P3) ) goto errexit ;
          sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P4) ;
          if( sd1 == 0 )
            {
             Sp = P1 = P4 ;
             if( Sp != lastPnt ) if( bcdtmTin_getSwapTriangleDtmObject(dtmP,P1,lastPnt,&P2,&P3,&P4)) goto errexit ;
             if( P2 == dtmP->nullPnt ) { *lineConnectedP = 0 ; goto cleanup  ; }
            }
          else
            {
             if( sd1 > 0 ) P2 = P4 ;
             if( sd1 < 0 ) P3 = P4 ;
             if( ( P4 = bcdtmList_nextAntDtmObject(dtmP,P2,P3)) < 0 ) goto errexit ;
            }
         }
/*
**     Line Cannot Be Swapped
*/
       else
         {
          *lineConnectedP = 0 ;
          sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P4) ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Can Not Swap Line %10ld %10ld",P2,P3) ;
             bcdtmWrite_message(0,0,0,"SideOf[Sp,lastPnt,P2] = %2d",bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P2)) ;
             bcdtmWrite_message(0,0,0,"SideOf[Sp,lastPnt,P3] = %2d",bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P3)) ;
             bcdtmWrite_message(0,0,0,"SideOf[Sp,lastPnt,P4] = %2d",bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P4)) ;
            }
          if( sd1 == 0 )
            {
             Sp = P1 = P4 ;
             if( Sp != lastPnt ) if( bcdtmTin_getSwapTriangleDtmObject(dtmP,P1,lastPnt,&P2,&P3,&P4)) goto errexit ;
             if( P2 == dtmP->nullPnt ) { *lineConnectedP = 0 ; goto cleanup  ; }
            }
          else
            {
             if( sd1 > 0 ) { P1 = P2 ; P2 = P4 ; }
             if( sd1 < 0 ) { P1 = P3 ; P3 = P4 ; }
             if( ( P4 = bcdtmList_nextAntDtmObject(dtmP,P2,P3)) < 0 ) goto errexit ;
/*
**           Check For Precion Problem - P2 And P3 Are On Opposite Side Of Sp-lastPnt
*/
             sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P2) ;
             sd3 = bcdtmMath_pointSideOfDtmObject(dtmP,Sp,lastPnt,P3) ;
             if( sd2 == sd3  ) { *lineConnectedP = 0 ; goto cleanup  ; }
            }
         }
      }
   }
/*
** Job Completed
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Structure") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) bcdtmWrite_message(0,0,0,"Tin Structure Corrupted") ;
    else                                        bcdtmWrite_message(0,0,0,"Tin Structure Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines That Intersect Connect Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines That Intersect Connect Line Error") ;
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
BENTLEYDTM_Public int bcdtmTin_getSwapTriangleDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long lastPnt,long *P1,long *P2,long *P3)
{
 int  ret=DTM_SUCCESS  ;
 long p1,p2,p3,clPtr ;
 double sd1,sd2    ;
/*
** Initiliase
*/
 *P1 = *P2 = *P3 = dtmP->nullPnt ;
/*
** Check Line Range
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints || lastPnt < 0 || lastPnt >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Swap Triangle Range Error") ;
    goto errexit ;
   }
/*
** Scan startPnt To Get Starting Line
*/
 clPtr = nodeAddrP(dtmP,startPnt)->cPtr ;
 if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,startPnt,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
 while( clPtr != dtmP->nullPtr )
   {
    p2  = clistAddrP(dtmP,clPtr)->pntNum ;
     clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    if( nodeAddrP(dtmP,startPnt)->hPtr != p1 )
      {
       sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,startPnt,lastPnt,p1) ;
       sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,startPnt,lastPnt,p2) ;
       if      ( sd1 ==  0 && sd2 <  0 ) { *P1 = p1 ; clPtr = dtmP->nullPtr ; }
       else if ( sd1 >   0 && sd2 == 0 ) { *P1 = p2 ; clPtr = dtmP->nullPtr ; }
       else
         {
          if( sd1 >  0 && sd2 < 0 )
            {
             if(( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2) ) < 0 ) goto errexit ;
             *P1 = p1 ; *P2 = p2 ; *P3 = p3 ;
             clPtr = dtmP->nullPtr ;
            }
         }
      }
    p1 = p2 ;
   }
/*
** Clean Up
*/
 cleanup:
/*
** Job Completed
*/
 return(ret) ;
/*
** Errexit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
std::mutex s_safeInsert;

BENTLEYDTM_Private int bcdtmTin_insertDtmFeatureTypeIntoDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType)
/*
** This Function Inserts Dtm Features Into A Dtm Object
*/
{
    //std::lock_guard<std::mutex> lck (s_safeInsert);

    
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    pnt,closeFlag,firstPnt,startPnt,nextPnt,insertError,dtmFeatureNum,flPtr,numPriorPts;
 long    *tempOffsetP=NULL,drapeOption,insertOption,internalPoint,validateResult ;
 long    numFeatures = 0, numFeaturesError = 0, numFeaturesInserted = 0, numHullPts = 0, numDrapeVoidPts = 0, featureNum = 0;
 DTMFeatureType insFeatureType;
 char    dtmFeatureTypeName[100] ;
 DPoint3d     *hullPtsP=NULL,*featPtsP=NULL,*drapeVoidPtsP=NULL ;
 DTMMemPnt featPtsPI = 0;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_FEATURE_LIST *flistP ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ) goto errexit ;
    bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Object %p",dtmFeatureTypeName,dtmP) ;
   }
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Tin Invalid At Entry") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin Valid") ;
/*
**  Check Point Offsets
*/
    bcdtmWrite_message(0,0,0,"Checking For Point Offset Range Errors") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
   }
/*
** Count Number Of Features
*/
 if( dbg == 1 )
   {
    if( bcdtmTin_countNumberOfDtmFeatureTypeOccurrencesDtmObject(dtmP,dtmFeatureType,&numFeatures)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Number Of %s Features = %8ld",dtmFeatureTypeName,numFeatures) ;
   }
/*
** Set Drape And Insert Options
**
** drapeOption  = 1   Insert As Drape Line
**              = 2   Insert As Break Line
** insertOption = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
*/
 drapeOption   = 1 ;
 insertOption  = 2 ;
 if( dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureType == DTMFeatureType::SoftBreakline ) insertOption = 1 ;
 if( dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureType == DTMFeatureType::SoftBreakline || dtmFeatureType == DTMFeatureType::ContourLine || dtmFeatureType == DTMFeatureType::BreakVoid ) drapeOption = 2 ;
/*
** Etract Tin Hull For Drape Voids
*/
 if( dtmFeatureType == DTMFeatureType::DrapeVoid )
   {
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin Hull Points = %6ld",numHullPts) ;
   }
/*
** Scan and Insert Feature Lines
*/
 featureNum =0 ;
 numFeaturesError   = 0 ;
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,TRUE,&dtmFeatureP,&dtmFeatureNum) ;
 while( dtmFeatureP != NULL )
   {
    insertError = 0 ;
    firstPnt = dtmP->nullPnt ;
    ++featureNum ;
    numPriorPts = dtmP->numPoints ;
    if( dbg ==  1 ) bcdtmWrite_message(0,0,0,"Inserting Feature %8ld",featureNum) ;
/*
**  Validate Polygonal Dtm Features
*/
    switch ( dtmFeatureType )
      {
       case DTMFeatureType::Void       :
       case DTMFeatureType::BreakVoid :
       case DTMFeatureType::Hole       :
       case DTMFeatureType::Island     :
       case DTMFeatureType::Region     :

         if( dbg ) bcdtmWrite_message(0,0,0,"Validating Feature Point Offsets") ;
         if( bcdtmTin_validatePolygonalOffsetFeatureDtmObject(dtmP,dtmFeatureP,&validateResult)) goto errexit ;
         if( validateResult == FALSE )
           {
            insertError = 1 ;
            if( dbg )
              {
               if( dtmFeatureP->numDtmFeaturePts < 3 ) bcdtmWrite_message(0,0,0,"Feature Has Less Than 3 Points") ;
               if( bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[0] != bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[dtmFeatureP->numDtmFeaturePts-1] )bcdtmWrite_message(0,0,0,"Feature Does Not Close") ;
              }
           }
       break ;

       case DTMFeatureType::DrapeVoid :
         if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Void") ;
         if( bcdtmTin_validateDrapeVoidDtmObject(dtmP,dtmFeatureP,hullPtsP,numHullPts,&validateResult,&drapeVoidPtsP,&numDrapeVoidPts)) goto errexit ;
         if( validateResult == FALSE )
           {
            insertError = 1 ;
            if( drapeVoidPtsP != NULL ) { free(drapeVoidPtsP) ; drapeVoidPtsP = NULL ; }
           }
       break   ;

       default :
       break   ;
      } ;
/*
**  Create Tptr List For Feature
*/
    if( ! insertError )
      {
/*
**     Get Point Offsets
*/
//       offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
/*
**     Insert Group Spots
*/
       if( dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Group Spot Feature") ;
/*
**        Check Point Offsets
*/
          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          firstPnt = startPnt = offsetP[0] ;
          if( dtmFeatureP->numDtmFeaturePts == 1 )
            {
             nodeAddrP(dtmP,startPnt)->tPtr = dtmP->nullPnt ;
            }
          else
            {
             for( pnt = 1 ; pnt < dtmFeatureP->numDtmFeaturePts && ! insertError ; ++pnt )
               {
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"startPnt = %8ld ** startPnt->tPtr = %8ld",startPnt,nodeAddrP(dtmP,startPnt)->tPtr) ;
                   bcdtmWrite_message(0,0,0,"nextPnt  = %8ld ** nextPnt->tPtr  = %8ld",offsetP[pnt],nodeAddrP(dtmP,offsetP[pnt])->tPtr) ;
                  }
                if( startPnt != offsetP[pnt] && nodeAddrP(dtmP,offsetP[pnt])->tPtr == dtmP->nullPnt )
                  {
                   nodeAddrP(dtmP,startPnt)->tPtr = offsetP[pnt] ;
                   startPnt = offsetP[pnt] ;
                  }
               }
            }
        }
/*
**     Insert Drape Voids
*/
       else if( dtmFeatureType == DTMFeatureType::DrapeVoid )
         {
          internalPoint = 1 ;
          if( ( insertError = bcdtmInsert_internalStringIntoDtmObject(dtmP,drapeOption,internalPoint,drapeVoidPtsP,numDrapeVoidPts,&firstPnt)) == 1 ) goto errexit  ;
          if( drapeVoidPtsP != NULL ) { free(drapeVoidPtsP) ; drapeVoidPtsP = NULL ; }
         }
/*
**     Insert All Other Dtm Feature Types
*/
       else
         {
/*
**        Write Out Feature Points
*/
          numPriorPts = dtmP->numPoints ;
          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          if( dbg == 2 )
            {
             for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
               {
                startPnt = offsetP[pnt] ;
                bcdtmWrite_message(0,0,0,"Segment[%6ld] ** Pnt = %8ld ** %12.4lf %12.4lf",pnt+1,startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y) ;
               }
            }
/*
**        Insert Lines Between Feature Points
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Feature Lines") ;
          firstPnt = dtmP->nullPnt ;
          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts - 1 && ! insertError ; ++pnt )
            {
             startPnt = offsetP[pnt] ;
             nextPnt  = offsetP[pnt+1] ;
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Segment %6ld of %6ld From Point %8ld To %8ld ** %12.4lf %12.4lf ** %12.4lf %12.4lf",pnt+1,dtmFeatureP->numDtmFeaturePts-1,startPnt,nextPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y) ;
/*
**           Insert Feature Line Segment Into Tin
*/
             if( startPnt != nextPnt )
               {
                if( ( insertError = bcdtmInsert_lineBetweenPointsDtmObject(dtmP,startPnt,nextPnt,drapeOption,insertOption)) == 1 ) goto errexit ;
                if ((insertError == 12 || insertError == 10)) // Check for polygonized feature.
                    {
                    if ( dtmFeatureType == DTMFeatureType::Void || dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureType == DTMFeatureType::Hole || dtmFeatureType == DTMFeatureType::Island )
                        {
                        // Can't split polygonal features.
                        }
                    else if( firstPnt != dtmP->nullPnt )
                        {
                        if (dtmP->extended && dtmP->extended->rollBackInfoP && bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                        // if startOffset = 0 then add this feature to rollback.
                        // Add the points processed so far to the feature, then create a new feature holder to store the rest in. then come out.
                        if (insertError == 10)
                            {
                            // If this isn't the first point then take the point before as well.
                            if (pnt != 1)
                                {
                                // Find Knot Point
                                int startPnt  = firstPnt ;
                                int knotPnt = dtmP->nullPnt;
                                while( nodeAddrP(dtmP, startPnt)->tPtr != dtmP->nullPnt  && nodeAddrP(dtmP, startPnt)->tPtr  >= 0 )
                                    {
                                    knotPnt = startPnt;
                                    int nextPnt = nodeAddrP(dtmP, startPnt)->tPtr ;
                                    nodeAddrP(dtmP, startPnt)->tPtr = -( nodeAddrP(dtmP, startPnt)->tPtr+1) ;
                                    startPnt = nextPnt;
                                    }
                                if (knotPnt != dtmP->nullPnt)
                                    nodeAddrP(dtmP, knotPnt)->tPtr = dtmP->nullPnt;
                                //  Reset Tptr List

                                startPnt = firstPnt;
                                while( nodeAddrP(dtmP, startPnt)->tPtr != dtmP->nullPnt )
                                    {
                                    int nextPnt = -(nodeAddrP(dtmP,startPnt)->tPtr + 1 ) ;
                                    nodeAddrP(dtmP, startPnt)->tPtr = nextPnt ;
                                    startPnt = nextPnt;
                                    }

                                --pnt;
                                }
                            }
                        BC_DTM_FEATURE *dtmFeature2P = NULL;
                        int dtmFeatureNum2 = 0;
                        long newDtmFeatureNum2 = 0;
                        if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature Table") ;
                        if( bcdtmInsert_addToFeatureTableDtmObject (dtmP,dtmFeature2P,dtmFeatureNum2,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmP->nullPnt,&newDtmFeatureNum2)) goto errexit  ;
                        // Reload the feature pointer as this may change.
                        dtmFeatureP = ftableAddrP(dtmP, dtmFeatureNum);
                        dtmFeature2P = ftableAddrP(dtmP, newDtmFeatureNum2) ;
                        dtmFeature2P->dtmFeatureState = dtmFeatureP->dtmFeatureState;
                        int newNumPts = dtmFeatureP->numDtmFeaturePts - pnt;
                        dtmFeature2P->numDtmFeaturePts = newNumPts;
                        dtmFeatureP->numDtmFeaturePts = pnt + 1;
                        dtmFeature2P->dtmFeaturePts.offsetPI = bcdtmMemory_allocate (dtmP,sizeof (long) * newNumPts);

                        long* newOffset = (long*)bcdtmMemory_getPointer (dtmP, dtmFeature2P->dtmFeaturePts.offsetPI);
                        memcpy (newOffset, &offsetP[pnt], sizeof (long) * newNumPts);
                        insertError = 0;
                        break;
                        }
                    }
                if( dbg && insertError ) bcdtmWrite_message(0,0,0,"Insert Error %2ld Processing Feature %8ld",insertError,featureNum) ;
                if( firstPnt == dtmP->nullPnt ) firstPnt = startPnt ;
               }
            }
          if( firstPnt == dtmP->nullPnt ) insertError = 1 ;
         }
      }
/*
**  Check That Single Line Feature Does Not Loop Back On Itself
*/
    if( ! insertError && dtmFeatureP->numDtmFeaturePts > 1 )
      {
       if( nodeAddrP(dtmP,nodeAddrP(dtmP,firstPnt)->tPtr)->tPtr == firstPnt )
         {
          nodeAddrP(dtmP,nodeAddrP(dtmP,firstPnt)->tPtr)->tPtr = dtmP->nullPnt ;
         }
      }
/*
**  Check Connectivity Of Inserted Feature
*/
    if( ! insertError && dtmFeatureType != DTMFeatureType::GroupSpots )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Feature Connectivity") ;
       if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,firstPnt,0))
         {
          insertError = 1 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Tptr List Connectivity Error ** Feature = %8ld",featureNum) ;
         }
      }
/*
**  Check Voids And Islands Do Not Intersect Inserted Voids And Islands
*/
    if( ! insertError && ( dtmFeatureType == DTMFeatureType::Void || dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureType == DTMFeatureType::Hole || dtmFeatureType == DTMFeatureType::Island ) )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersected Voids Or Islands") ;
       if( bcdtmTin_checkForIntersectionWithInsertedVoidsAndIslandsDtmObject(dtmP,firstPnt,&closeFlag,&insertError)) goto errexit ;
       if( dbg && insertError ) bcdtmWrite_message(0,0,0,"Intersecting Voids Or Islands") ;
       if( closeFlag == FALSE ) insertError = TRUE ;
      }
/*
**  Insert Feature Into Tin
*/
    if( ! insertError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Feature Into Tin") ;
/*
**     Save Point Offsets
*/
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       tempOffsetP = ( long *) malloc(dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
       if( tempOffsetP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       memcpy(tempOffsetP,offsetP,dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
/*
**     Add Feature To Tin
*/
       if( dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureType == DTMFeatureType::DrapeVoid ) insFeatureType = DTMFeatureType::Void ;
       else insFeatureType = dtmFeatureType ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,dtmFeatureP,dtmFeatureNum,insFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,firstPnt,1)) goto errexit ;
       if( dbg )
         {
          if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(insFeatureType,dtmFeatureTypeName) ) goto errexit ;
          bcdtmWrite_message(0,0,0,"Inserting %s Into Tin Completed",dtmFeatureTypeName) ;
         }
       ++numFeaturesInserted ;
/*
**     Scan Feature And Set Point Types
*/
       if( dtmFeatureType != DTMFeatureType::DrapeVoid )
         {
/*
**     Marked All Points as Inserted.
*/
          bcdtmList_setPntTypeForForDtmTinFeatureDtmObject(dtmP,dtmFeatureNum,2) ;
/*
**     Unmarked All Real Points.
*/
          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             flPtr = nodeAddrP(dtmP,tempOffsetP[pnt])->fPtr ;
             while( flPtr != dtmP->nullPtr )
               {
                flistP = flistAddrP(dtmP,flPtr) ;
                if( flistP->dtmFeature == dtmFeatureNum )
                  {
                   flistP->pntType = 1 ;
                   flPtr = dtmP->nullPtr ;
                  }
                else flPtr = flistP->nextPtr ;
               }
            }
         }
/*
**     Free Temporary Offsets Memory
*/
       if( tempOffsetP != NULL ) { free( tempOffsetP ) ; tempOffsetP = NULL ; }
      }
/*
**  Set Feature State To Tin Insert Error
*/
    else
      {
       ++numFeaturesError ;
       dtmFeatureP->dtmFeatureState = DTMFeatureState::TinError ;
       if( firstPnt != dtmP->nullPnt ) bcdtmList_nullTptrListDtmObject(dtmP,firstPnt) ;
/*
**     Copy Point Offset Array To Point Array
*/
       if( dtmFeatureType != DTMFeatureType::DrapeVoid )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Feature Insert Error") ;
          featPtsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d));
          featPtsP  = bcdtmMemory_getPointerP3D(dtmP, featPtsPI);
          if( featPtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }

          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             *(featPtsP+pnt) = *(( DPoint3d * ) pointAddrP(dtmP,offsetP[pnt])) ;
            }
          bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
          dtmFeatureP->dtmFeaturePts.pointsPI = featPtsPI ;
          featPtsP = NULL ;
         }
      }
/*
**  Get Next Feature
*/
    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,FALSE,&dtmFeatureP,&dtmFeatureNum) ;
   }
/*
** Report And Set To Null Non Null Tptr Values
*/
 bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
**  Print Feature Insertion Insertion Statistics
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Features Processed       = %6ld",numFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Features With Errors     = %6ld",numFeaturesError) ;
    bcdtmWrite_message(0,0,0,"Number Of Features Inserted        = %6ld",numFeaturesInserted) ;
   }
/*
** Cleanup
*/
 cleanup :
 if( hullPtsP      != NULL ) free(hullPtsP) ;
 if( featPtsP      != NULL ) bcdtmMemory_free(dtmP, featPtsPI) ;
 if( drapeVoidPtsP != NULL ) free(drapeVoidPtsP) ;
 if( tempOffsetP   != NULL ) { free( tempOffsetP ) ; tempOffsetP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Completed",dtmFeatureTypeName) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Error",dtmFeatureTypeName) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"Error Detected Inserting Feature %8ld",featureNum) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_validatePolygonalOffsetFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer Dtm Object               */
 BC_DTM_FEATURE *dtmFeatureP,      /* ==> Pointer Drape Void Dtm Feature   */
 long *validateResultP             /* <== Validate Result <TRUE,FALSE>     */
)
/*
** This Function Validates A DTM Polygoanl Offset Feature .
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTMDirection direction;
 double area ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Polygonal Offset Feature")  ;
/*
** Initialise
*/
 *validateResultP = FALSE ;
/*
** Check For None Null Dtm Feature Pointer
*/
 if( dtmFeatureP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Dtm Feature Pointer") ;
    goto errexit ;
   }
/*
** Polygonal Offset Feature Must Have Three Or More Points And Must Close, Test for 4 or more as the first and last would be the same.

*/
 long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
 if( dtmFeatureP->numDtmFeaturePts >= 4 && offsetP[0] == offsetP[dtmFeatureP->numDtmFeaturePts-1] )
   {
    *validateResultP = TRUE ;
/*
**  Determine Direction And Area Of Point Offset Polygonal Feature
*/
    bcdtmMath_getPointOffsetPolygonDirectionAndAreaDtmObject(dtmP,offsetP,dtmFeatureP->numDtmFeaturePts,&direction,&area) ;
/*
**  Reverse Direction Of Polygonal Feature If Its Direction Is Clockwise
*/
    if( direction == DTMDirection::Clockwise )
      {
       bcdtmMath_reversePointOffsetPolygonDirection(offsetP,dtmFeatureP->numDtmFeaturePts) ;
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygonal Offset Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygonal Offset Feature Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_validateDrapeVoidDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer Dtm Object                   */
 BC_DTM_FEATURE *dtmFeatureP,      /* ==> Pointer Drape Void Dtm Feature       */
 DPoint3d  *hullPtsP,                   /* ==> Pointer Tin Hull Points              */
 long numHullPts,                  /* ==> Number Of Tin Hull Points            */
 long *drapeVoidValidatedP,        /* <== Drape Void Validated <TRUE,FALSE>    */
 DPoint3d  **drapeVoidPtsPP,            /* <== Pointer To Drape Void Points         */
 long *numDrapeVoidPtsP            /* <== Number Of Drape Void Points          */
)
/*
** This Function Validates A Drape Void .
**
** As drape Voids are instered post triangulation the extent of a drape void
** can be external to the tin hull.
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTMDirection direction;
 long   intersectFlag ;
 double area ;
 DPoint3d    *p3dP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Void")  ;
/*
** Initialise
*/
 *numDrapeVoidPtsP = 0 ;
 if( *drapeVoidPtsPP != NULL ) { free(*drapeVoidPtsPP) ; *drapeVoidPtsPP = NULL ; }
 *drapeVoidValidatedP = FALSE ;
/*
**  Set Pointer To Drape Points
*/
 DPoint3d* pointsP = bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI);
/*
** Log Drape Void Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Void Points = %8ld",*numDrapeVoidPtsP) ;
    for( p3dP = pointsP ; p3dP < pointsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Drape Void Point[%8ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-pointsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
**  Drape Void Must Have Three Or More Points And Must Close
*/
 if( dtmFeatureP->numDtmFeaturePts > 3 &&
     pointsP->x == (pointsP+dtmFeatureP->numDtmFeaturePts-1)->x  &&
     pointsP->y == (pointsP+dtmFeatureP->numDtmFeaturePts-1)->y     )
   {
/*
**  Allocate Memory For Drape Void Points
*/
    *numDrapeVoidPtsP = dtmFeatureP->numDtmFeaturePts ;
    *drapeVoidPtsPP = ( DPoint3d * ) malloc( *numDrapeVoidPtsP * sizeof(DPoint3d)) ;
    if( *drapeVoidPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Drape Void Points
*/
    memcpy(*drapeVoidPtsPP,pointsP,*numDrapeVoidPtsP * sizeof(DPoint3d)) ;
/*
**  Write Drape Void Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Drape Void Points = %8ld",*numDrapeVoidPtsP) ;
       for( p3dP = *drapeVoidPtsPP ; p3dP < *drapeVoidPtsPP + *numDrapeVoidPtsP ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Drape Void Point[%8ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-*drapeVoidPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**  Validate Drape Void Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Void Points")  ;
    if( ! bcdtmMath_validatePointArrayPolygon(drapeVoidPtsPP,numDrapeVoidPtsP,dtmP->ppTol) )
      {
/*
**     Check For More Than Three Points
*/
       if( *numDrapeVoidPtsP > 3 )
         {
/*
**        Write Direction Of Drape Void Polygon
*/
          if( dbg )
            {
             bcdtmMath_getPolygonDirectionP3D(*drapeVoidPtsPP,*numDrapeVoidPtsP,&direction,&area) ;
             bcdtmWrite_message(0,0,0,"Drape Void Direction = %1ld Area = %10.4lf",direction,area) ;
            }
/*
**        Intersect Drape Void And Tin Hull
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Drape Void And Tin Hull")  ;
          if( bcdtmPolygon_intersectPointArrayPolygons(hullPtsP,numHullPts,*drapeVoidPtsPP,*numDrapeVoidPtsP,&intersectFlag,&polyP,dtmP->ppTol,dtmP->plTol)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Flag = %2ld",intersectFlag) ;
/*
**        Extract Intersected Drape Void
*/
          if( intersectFlag )
            {
             *drapeVoidValidatedP = TRUE ;
             free(*drapeVoidPtsPP) ;
             *drapeVoidPtsPP = NULL ;
             if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,0,drapeVoidPtsPP,numDrapeVoidPtsP)) goto errexit ;
             bcdtmPolygon_deletePolygonObject(&polyP) ;
             if( dbg )
               {
                bcdtmMath_getPolygonDirectionP3D(*drapeVoidPtsPP,*numDrapeVoidPtsP,&direction,&area) ;
                bcdtmWrite_message(0,0,0,"Intersected Drape Void Direction = %1ld Area = %10.4lf",direction,area) ;
               }
/*
**           Write Drape Void Points
*/
             if( dbg && intersectFlag == 1 )
               {
                bcdtmWrite_message(0,0,0,"Reduced Number Of Drape Void Points = %8ld",*numDrapeVoidPtsP) ;
                for( p3dP = *drapeVoidPtsPP ; p3dP < *drapeVoidPtsPP + *numDrapeVoidPtsP ; ++p3dP )
                  {
                   bcdtmWrite_message(0,0,0,"Drape Void Point[%8ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-*drapeVoidPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
                  }
               }
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Drape Void Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Drape Void Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numDrapeVoidPtsP = 0 ;
 *drapeVoidValidatedP = FALSE ;
 if( *drapeVoidPtsPP != NULL ) { free(*drapeVoidPtsPP) ; *drapeVoidPtsPP = NULL ;}
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_checkForIntersectionWithInsertedVoidsAndIslandsDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer Dtm Object                           */
 long firstPoint,                  /* ==> First Point Of Tptr List                     */
 long *closeFlagP,                 /* <== Feature Closes <TRUE,FALSE>                  */
 long *intersectResultP            /* <== Intersects Inserted Feature <TRUE,FALSE>     */
)
/*
** This Function For Intersection Of A Tptr Polygon With Inserted Voids Or Islands
** Common Hull Points Are Allowed
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  scanPnt,pointFound,nextPnt,priorPnt,testForTouch=FALSE  ;
 long  featurePriorPnt,featureNextPnt,featurePnt,externalHit,internalHit,numPointFeatures=0 ;
 DTM_TIN_POINT_FEATURES *featP,*pointFeaturesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) if( bcdtmWrite_message(0,0,0,"Checking For Intersection With Void Or Island Features") ) goto errexit ;
/*
** Initialise
*/
 *intersectResultP = FALSE ;
 *closeFlagP = TRUE ;
/*
** Check For Touching Feature Point
*/
 if( testForTouch == TRUE )
   {
    pointFound = FALSE ;
    scanPnt    = firstPoint ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %9ld scanPnt = %9ld",firstPoint,scanPnt) ;
       if( bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,scanPnt)) pointFound = scanPnt ;
       scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
      } while ( scanPnt != firstPoint && pointFound == FALSE && scanPnt != dtmP->nullPnt ) ;
   }
/*
** Check For Intersection With A Feature
*/
 else
   {
    pointFound = FALSE ;
    scanPnt    = firstPoint ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %9ld scanPnt = %9ld",firstPoint,scanPnt) ;
       if( bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,scanPnt))
         {
/*
**        Log Touch Point
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Touch Point Found Point[%8ld] = %12.5lf %12.5lf %10.4lf",scanPnt,pointAddrP(dtmP,scanPnt)->x,pointAddrP(dtmP,scanPnt)->y,pointAddrP(dtmP,scanPnt)->z) ;
/*
**        Get Next And Prior Points
*/
          nextPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
          priorPnt = nextPnt ;
          while ( nodeAddrP(dtmP,priorPnt)->tPtr != scanPnt )
            {
             if( ( priorPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,priorPnt)) < 0 ) goto errexit ;
            }
/*
**        Get Features At Point
*/
          if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,scanPnt,&pointFeaturesP,&numPointFeatures)) goto errexit ;
/*
**        Scan Features And Determine If The Prior And Next Points Are Both External To A Void/Island Feature
*/
          for( featP = pointFeaturesP ; featP < pointFeaturesP + numPointFeatures && pointFound == FALSE ; ++featP )
            {
             if( featP->dtmFeatureType == DTMFeatureType::Void || featP->dtmFeatureType == DTMFeatureType::Island || featP->dtmFeatureType == DTMFeatureType::Void )
               {
                 featurePriorPnt = featP->priorPoint ;
                 featureNextPnt  = featP->nextPoint  ;
                 externalHit = 0 ;
                 internalHit = 0 ;
/*
**               Scan Externally
*/
                 if( ( featurePnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,featureNextPnt)) < 0 ) goto errexit ;
                 while ( featurePnt != featurePriorPnt )
                   {
                    if( featurePnt == priorPnt ) ++externalHit ;
                    if( featurePnt == nextPnt  ) ++externalHit ;
                    if( ( featurePnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,featurePnt)) < 0 ) goto errexit ;
                   }
/*
**               Scan Internally
*/
                 if(( featurePnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,featurePriorPnt)) < 0 ) goto errexit ;
                 while ( featurePnt != featureNextPnt )
                   {
                    if( featurePnt == priorPnt ) ++internalHit ;
                    if( featurePnt == nextPnt  ) ++internalHit ;
                    if( ( featurePnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,featurePnt)) < 0 ) goto errexit ;
                   }
/*
**               Check For Intersection
*/
                 if( externalHit && internalHit ) pointFound = TRUE ;
               }
            }
/*
**        Free Memory
*/
          if( pointFeaturesP != NULL ) { free(pointFeaturesP) ; pointFeaturesP = NULL ; }
         }
       scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
      } while ( scanPnt != firstPoint && pointFound == FALSE && scanPnt != dtmP->nullPnt ) ;
   }
/*
** Set Closure Flag
*/
 if( scanPnt == dtmP->nullPnt ) *closeFlagP = FALSE ;
/*
** Set Intersect Result
*/
 if( pointFound ) *intersectResultP = TRUE ;
 if( dbg )
   {
    if( pointFound ) bcdtmWrite_message(0,0,0,"Touch Or Intersection Found") ;
   }
/*
** Cleanup
*/
 cleanup :
 if( pointFeaturesP != NULL ) { free(pointFeaturesP) ; pointFeaturesP = NULL ; }

/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Intersection With Void Or Island Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Intersection With Void Or Island Features Error") ;
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
BENTLEYDTM_Private int bcdtmTin_addInteriorPointToSingleTriangleVoidsDtmObject(BC_DTM_OBJ *dtmP )
/*
** This Function Adds An Interior Point To Single Triangle Voids
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numPts,dtmFeature,nextPoint ;
 double x,y,z ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding An Interior Point To Single Triangle Voids") ;
/*
** Scan Features For Single Triangle Voids Or Holes
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
         {
          if( bcdtmList_countNumberOfPointsForDtmTinFeatureDtmObject(dtmP,dtmFeature,&numPts)) goto errexit ;
/*
**        Insert Interior Point Into Triangle
*/
          if( numPts == 4 )
            {
/*
**           Get Triangle Centroid
*/
             x = 0.0 ;
             y = 0.0 ;
             z = 0.0 ;
             nextPoint = ( long ) dtmFeatureP->dtmFeaturePts.firstPoint ;
             do
               {
                pntP = pointAddrP(dtmP,nextPoint) ;
                x = x + pntP->x ;
                y = y + pntP->y ;
                z = z + pntP->z ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,nextPoint,&nextPoint)) goto errexit ;
               } while( nextPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
             x =  x / 3.0 ;
             y =  y / 3.0 ;
             z =  z / 3.0 ;
/*
**           Insert Point Into Tin
*/
             if( bcdtmInsert_storePointInDtmObject(dtmP,2,1,x,y,z,&numPts)) goto errexit ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Function Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding An Interior Point To Single Triangle Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding An Interior Point To Single Triangle Voids Error") ;
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
BENTLEYDTM_Public int bcdtmTin_compactFeatureTableDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Compacts the Feature Table.
** It Removes Deleted Dtm Features
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ofs1,ofs2,ftable,flist,delCount = 0,dtmFeature ;
 char   dtmFeatureTypeName[50] ;
 LongArray::iterator ofsP;
 // BC_DTM_FEATURE   *ftableP ;
 DTM_FEATURE_LIST   *flistP ;
 BC_DTM_FEATURE     *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
/*
** Only Process If Dtm Features Exist
*/
 if( dtmP->numFeatures > 0 )
   {
/*
**  Write Out Features
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of DTM Features = %8ld",dtmP->numFeatures) ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature[%6ld] ** Type = %20s State = %2ld Id = %8I64d offsetP = %p",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeaturePts.offsetPI) ;
         }
      }
/*
** Check Dtm Feature End Points
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Before Compacting Feature Table") ;
       if( bcdtmCheck_dtmFeatureEndPointsDtmObject(dtmP,0))
         {
          bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
          goto errexit  ;
         }
      }
/*
** Delete Features In Feature State DTMFeatureState::OffsetsArray
*/
    PartitionArray<BC_DTM_FEATURE, DTM_PARTITION_SHIFT_FEATURE, MAllocAllocator> ftableArray(dtmP->fTablePP, dtmP->numFeatures, dtmP->numFeaturePartitions, dtmP->featurePartitionSize);
    PartitionArray<BC_DTM_FEATURE, DTM_PARTITION_SHIFT_FEATURE, MAllocAllocator>::iterator ftableP = ftableArray.start();
/*
**  Count Number Of Deleted Features
*/
    delCount = 0 ;
    for( ftable = 0 ; ftable < dtmP->numFeatures ; ++ftable, ++ftableP )
      {
/*
**     Delete Any Temporary Features
*/
       if( ftableP->dtmFeatureState == DTMFeatureState::OffsetsArray )
         {
          if( ftableP->dtmFeaturePts.pointsPI != 0)
            {
             bcdtmMemory_free(dtmP,ftableP->dtmFeaturePts.pointsPI) ;
             ftableP->dtmFeaturePts.pointsPI = 0;
            }
          ftableP->numDtmFeaturePts = 0 ;
          ftableP->dtmFeatureState = DTMFeatureState::Deleted ;
         }
/*
**     Test For Deleted Feature
*/
       if( ftableP->dtmFeatureState == DTMFeatureState::Deleted )
         {
          if( dbg )
            {
             bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(ftableP->dtmFeatureType,dtmFeatureTypeName) ;
             bcdtmWrite_message(0,0,0,"dtmFeature = %6ld dtmFeatureType = %20s dtmFeatureState = %2ld offsetP = %p",ftable,dtmFeatureTypeName,ftableP->dtmFeatureState,ftableP->dtmFeaturePts.offsetPI) ;
            }
          ++delCount ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Deleted Features = %6ld",delCount) ;
/*
**  Compact Feature Table If Deleted Features Exist
*/
    if( delCount > 0 )
      {
/*
**     Allocate Memory For Dtm Feature Offsets
*/
       LongArray tempP;
       if( tempP.resize(dtmP->numFeatures) != 0)
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       for( ofsP = tempP.start() ; ofsP != tempP.end()  ; ++ofsP ) *ofsP = 0 ;
/*
**     Mark All Deleted Records
*/
       ftableP = ftableArray.start();
       for( ftable = 0 ; ftable < dtmP->numFeatures ; ++ftable, ++ftableP )
         {
          if( ftableP->dtmFeatureState == DTMFeatureState::Deleted )
            {
             *(tempP+ftable) = 1 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Feature %8ld Deleted",ftable) ;
            }
         }
/*
**     Copy Over Deleted Records
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying Over Deleted Records") ;
       PartitionArray<BC_DTM_FEATURE, DTM_PARTITION_SHIFT_FEATURE, MAllocAllocator>::iterator ftable2P = ftableArray.start();
       ftableP = ftableArray.start();
       for( ofs1 = ofs2 = 0 ; ofs2 < dtmP->numFeatures ; ++ofs2, ++ftableP )
         {
          if( ! *(tempP+ofs2) )
            {
             if( ofs1 != ofs2 ) *ftable2P = *ftableP; //*(ftableAddrP(dtmP,ofs1)) = *(ftableAddrP(dtmP,ofs2)) ;
             ++ofs1 ;
             ++ftable2P;
            }
         }
/*
**     Adjust Pointers
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Pointers") ;
       for( ofsP = tempP.start() + 1 ; ofsP < tempP.start() + dtmP->numFeatures ; ++ofsP ) *ofsP = *ofsP + *(ofsP-1) ;
       dtmP->numFeatures  = dtmP->numFeatures - delCount ;
/*
**     Adjust Feature List Feature Numbers
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature List Numbers") ;
        for( flist = 0 ; flist < dtmP->numFlist ; ++flist )
         {
          flistP = flistAddrP(dtmP,flist) ;
          if( flistP->dtmFeature != dtmP->nullPnt ) flistP->dtmFeature -= (long)*(tempP+flistP->dtmFeature) ;
         }
      }
/*
**  Check Dtm Feature End Points
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points After Compacting") ;
       if( bcdtmCheck_dtmFeatureEndPointsDtmObject(dtmP,0))
         {
          bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
          goto errexit  ;
         }
       bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
       if( ! bcdtmCheck_topologyDtmObject(dtmP,0)) bcdtmWrite_message(0,0,0,"Tin Topology Valid") ;
       else
         {
          bcdtmWrite_message(0,0,0,"Tin Topology Invalid") ;
          goto errexit ;
         }
       bcdtmWrite_message(0,0,0,"Checking Topology Dtm Features") ;
       if( ! bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,0)) bcdtmWrite_message(0,0,0,"Tin Feature Topology Valid") ;
       else
         {
          bcdtmWrite_message(0,0,0,"Tin Topology Invalid") ;
          goto errexit ;
         }
      }
/*
**  Write Out Features
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of DTM Features = %8ld",dtmP->numFeatures) ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature[%6ld] ** Type = %20s State = %2ld Id = %8I64d offsetP = %p",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeaturePts.offsetPI) ;
         }
      }
   }
/*
**  Update Modified Time
 */
 if (delCount != 0)
   bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Error") ;
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
BENTLEYDTM_Public int bcdtmTin_compactFeatureListDtmObject(BC_DTM_OBJ *dtmP)
/*
** This routine Compacts the Feature List
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   node,fl1Ptr,fl2Ptr,delCount,*tempP=NULL;
 DTM_TIN_NODE  *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
/*
** Check If Feature List Can be Compacted
*/
 if( dtmP->fListDelPtr != dtmP->nullPtr )
   {
    PartitionArray<DTM_FEATURE_LIST, DTM_PARTITION_SHIFT_FLIST, MAllocAllocator> fList(dtmP->fListPP, dtmP->numFlist, dtmP->numFlistPartitions, dtmP->flistPartitionSize);
    LongArray tempP;
    delCount = 0 ;
    fl1Ptr = dtmP->fListDelPtr ;
/*
**  Allocate memory For Counts
*/
    if( tempP.resize(dtmP->numFlist) != 0)
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    LongArray::iterator ofsP;
    for( ofsP = tempP.start(); ofsP != tempP.end() ; ++ofsP ) *ofsP = 0;
/*
**  Mark All Deleted Records
*/
    while( fl1Ptr != dtmP->nullPtr )
      {
       *(tempP+fl1Ptr) = 1 ;
       fl1Ptr = fList[fl1Ptr].nextPtr ;
       ++delCount;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"delCount = %8ld",delCount) ;
/*
**  Copy Over Deleted Records
*/
    for( fl1Ptr = fl2Ptr = 0 ; fl2Ptr < dtmP->numFlist ; ++fl2Ptr )
      {
       if( ! *(tempP+fl2Ptr) )
         {
          if( fl1Ptr != fl2Ptr ) fList[fl1Ptr] = fList[fl2Ptr] ;
          ++fl1Ptr ;
         }
      }
/*
**  Adjust Pointers
*/
    for( ofsP = tempP.start() + 1 ; ofsP != tempP.end() ; ++ofsP ) *ofsP += *(ofsP-1) ;
    for( fl1Ptr = 0 ; fl1Ptr < dtmP->numFlist ; ++fl1Ptr )
      {
       fl2Ptr = fList[fl1Ptr].nextPtr ;
       if( fl2Ptr != dtmP->nullPtr ) fList[fl1Ptr].nextPtr -= (long)*(tempP+fl2Ptr) ;
      }
    dtmP->numFlist    = dtmP->numFlist - delCount ;
    dtmP->fListDelPtr = dtmP->nullPtr ;
/*
** Adjust Feature List Pointers
*/
   for( node = 0 ; node < dtmP->numPoints ; ++node )
     {
      nodeP = nodeAddrP(dtmP,node) ;
      if( nodeP->fPtr != dtmP->nullPtr ) nodeP->fPtr -= (long)*(tempP+nodeP->fPtr) ;
     }
  }
/*
** Clean Up
*/
cleanup :
 if( tempP != NULL ) free(tempP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature List Error") ;
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
BENTLEYDTM_Public int bcdtmTin_compactCircularListDtmObject(BC_DTM_OBJ *dtmP)
/*
** This routine Compacts the Circular List
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  node,cl1Ptr,cl2Ptr,delCount;
/*
 PartitionArray<unsigned short, 11, MAllocAllocator>::iterator ofsP;
 PartitionArray<unsigned short, 11, MAllocAllocator> tempP;
 PartitionArray<unsigned short, 11, MAllocAllocator>::iterator end;
*/

 PartitionArray<long, 11, MAllocAllocator>::iterator ofsP;
 PartitionArray<long, 11, MAllocAllocator> tempP;
 PartitionArray<long, 11, MAllocAllocator>::iterator end;

 DTM_TIN_NODE  *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Circular List ** dtmP = %p dtmP->cListDelPtr = %9ld",dtmP,dtmP->cListDelPtr) ;
/*
** Check If Circular List Can be Compacted
*/
 if( dtmP->cListDelPtr != dtmP->nullPtr )
   {
/*
**  Allocate Memory For Counts
*/
    if( tempP.resize(dtmP->cListPtr) != 0)
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**   Initialise
*/
    end = tempP.end();
    for( ofsP = tempP.start(); ofsP != end; ++ofsP )*ofsP = 0 ;
/*
** Mark All Deleted Records
*/
    delCount = 0 ;
    cl1Ptr = dtmP->cListDelPtr ;
    while( cl1Ptr != dtmP->nullPtr )
      {
       *(tempP+cl1Ptr) = 1 ;
       cl1Ptr = clistAddrP(dtmP,cl1Ptr)->nextPtr ;
       ++delCount ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Deleted Clist Records = %6ld",delCount) ;
/*
** Copy Over Deleted Records
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Over Deleted Clist Records") ;
    for( cl1Ptr = cl2Ptr = 0 ; cl2Ptr < dtmP->cListPtr ; ++cl2Ptr )
      {
       if( ! *(tempP+cl2Ptr) )
         {
          if( cl1Ptr != cl2Ptr ) *(clistAddrP(dtmP,cl1Ptr)) = *(clistAddrP(dtmP,cl2Ptr)) ;
          ++cl1Ptr ;
         }
      }
/*
**  Adjust Circular List Pointers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Circular List Next Pointers") ;
    for( ofsP = tempP.start() + 1 ; ofsP != end; ++ofsP ) *ofsP = *ofsP + *(ofsP-1) ;
    for( cl1Ptr = 0 ; cl1Ptr < dtmP->cListPtr ; ++cl1Ptr )
      {
       cl2Ptr = clistAddrP(dtmP,cl1Ptr)->nextPtr ;
       if( cl2Ptr != dtmP->nullPtr ) clistAddrP(dtmP,cl1Ptr)->nextPtr -= (long)*(tempP+cl2Ptr) ;
      }
    dtmP->cListPtr    = dtmP->cListPtr - delCount ;
    dtmP->cListDelPtr = dtmP->nullPtr ;
/*
** Adjust Node Circular List Pointers
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Node Circular List Pointers") ;
   for( node = 0 ; node < dtmP->numPoints ; ++node )
     {
      nodeP = nodeAddrP(dtmP,node) ;
      if( nodeP->cPtr != dtmP->nullPtr ) nodeP->cPtr -= (long)*(tempP+nodeP->cPtr) ;
     }
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Comapcting Circular List ** dtmP = %p dtmP->cListDelPtr = %9ld Completed",dtmP,dtmP->cListDelPtr) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Comapcting Circular List ** dtmP = %p dtmP->cListDelPtr = %9ld Error",dtmP,dtmP->cListDelPtr) ;
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
BENTLEYDTM_Public int bcdtmTin_compactPointAndNodeTablesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This routine Compacts the Points And Nodes Arrays
*/
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long              delCount;
 long              ofs1,ofs2,node,ftable,flist,clist,hullFeature ;
 DTM_CIR_LIST      *clistP ;
 BC_DTM_FEATURE    *ftableP  ;
 DTM_FEATURE_LIST  *flistP ;
 DTM_TIN_NODE      *nodeP  ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Compacting Point And Nodes Table") ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->memPoints = %8ld dtmP->numSortedPoints = %8ld",dtmP->numPoints,dtmP->memPoints,dtmP->numSortedPoints) ;
   }
/*
** Count Deleted Tin Points
*/
 delCount = 0 ;
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    if( nodeAddrP(dtmP,node)->cPtr == dtmP->nullPtr ) ++delCount ;
    if( dbg == 2 )if( nodeAddrP(dtmP,node)->cPtr == dtmP->nullPtr ) bcdtmWrite_message(0,0,0,"Point %8ld deleted",node) ;
   }
/*
** Write Number Of Deleted Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Deleted Points = %8ld",delCount) ;
/*
** Only Process If Deleted Tin Points Exist
*/
 if( delCount > 0 )
   {
/*
**  Allocate And Initialise Memory For Deleted Point Count
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Delete Point Count") ;
    LongArray tempP;
    if(tempP.resize(dtmP->numPoints) != DTM_SUCCESS)
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    LongArray::iterator ofsP;
    for( ofsP = tempP.start() ; ofsP != tempP.end(); ++ofsP ) *ofsP = 0 ;
/*
**  Mark All Deleted Node Records
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Deleted Point Records") ;
    for( node = 0 , ofsP = tempP.start(); node <  dtmP->numPoints ; ++node , ++ofsP )
      {
       if( nodeAddrP(dtmP,node)->cPtr == dtmP->nullPtr )  *ofsP = 1 ;
      }
/*
**  Copy Over Deleted Point And Node Records
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Over Deleted Point Records") ;
    for( ofs1 = ofs2 = 0 , ofsP = tempP.start() ; ofs2 < dtmP->numPoints ; ++ofs2 , ++ofsP )
      {
       if( ! *ofsP )
         {
          if( ofs1 != ofs2 )
            {
             *(nodeAddrP(dtmP,ofs1))  = *(nodeAddrP(dtmP,ofs2))  ;
             *(pointAddrP(dtmP,ofs1)) = *(pointAddrP(dtmP,ofs2)) ;
            }
          ++ofs1 ;
         }
      }
/*
**  Adjust Pointers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Pointers") ;
    for( ofsP = tempP.start() + 1 ; ofsP != tempP.end(); ++ofsP ) *ofsP += *(ofsP-1) ;
/*
**  Reset Number Of Points
*/
    if( dtmP->numSortedPoints <= 0 ) dtmP->numSortedPoints = 1 ;
    dtmP->numPoints        = dtmP->numPoints       - delCount ;
    dtmP->numSortedPoints  = dtmP->numSortedPoints - *(tempP+(dtmP->numSortedPoints-1));
    if( dtmP->numSortedPoints <= 0 ) dtmP->numSortedPoints = 1 ;
/*
**  Adjust Feature Table First Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature Table") ;
    hullFeature = dtmP->nullPnt ;
    for( ftable = 0 ; ftable < dtmP->numFeatures ; ++ftable )
      {
       ftableP = ftableAddrP(dtmP,ftable) ;
       if( ftableP->dtmFeatureState == DTMFeatureState::Tin )
         {
          if( ftableP->dtmFeatureType == DTMFeatureType::Hull ) hullFeature = ftable ;
          else if( ftableP->dtmFeaturePts.firstPoint != dtmP->nullPnt ) ftableP->dtmFeaturePts.firstPoint -= *(tempP+(long)ftableP->dtmFeaturePts.firstPoint) ;
         }
      }
/*
**  Adjust Feature List Next Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature List") ;
    for( flist = 0 ; flist < dtmP->numFlist ; ++flist )
      {
       flistP = flistAddrP(dtmP,flist) ;
       if( flistP->nextPnt != dtmP->nullPnt ) flistP->nextPnt -= (long)*(tempP+flistP->nextPnt) ;
      }
/*
**  Adjust Tin Pointers To Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Hull Pointers") ;
    if( dtmP->hullPoint     >= 0 && dtmP->hullPoint     < dtmP->numPoints ) dtmP->hullPoint      = dtmP->hullPoint     - *(tempP+dtmP->hullPoint) ;
    if( dtmP->nextHullPoint >= 0 && dtmP->nextHullPoint < dtmP->numPoints ) dtmP->nextHullPoint  = dtmP->nextHullPoint - *(tempP+dtmP->nextHullPoint) ;
/*
**  Adjust Node Hull Pointers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Node Hull Pointers") ;
    for( node = 0 ; node <  dtmP->numPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->hPtr != dtmP->nullPnt )
         {
          nodeP->hPtr -= (long)*(tempP+nodeP->hPtr) ;
          if( nodeP->hPtr < 0 ) nodeP->hPtr = dtmP->nullPnt ;
         }
      }
/*
**  Adjust Circular List Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Circular List cListPtr = %8ld clistDelPtr - %9ld",dtmP->cListPtr,dtmP->cListDelPtr) ;
    for( clist = 0 ; clist < dtmP->cListPtr ; ++clist )
      {
       clistP = clistAddrP(dtmP,clist) ;
       if( clistP->pntNum != dtmP->nullPnt ) clistP->pntNum -= (long)*(tempP+clistP->pntNum) ;
      }
/*
**  Rset First Point For Hull Ferature
*/
    if( hullFeature != dtmP->nullPnt )
      {
       ftableAddrP(dtmP,hullFeature)->dtmFeaturePts.firstPoint = 0 ;
      }
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
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_removeExternalSliverTrianglesDtmObject(BC_DTM_OBJ *dtmP)
/*
** Delete Sliver Triangles Along Edge
*/
{
 int     ret=DTM_SUCCESS ;
 long    hullPnt,nextPnt=0,antPnt,process ;
 double  s1s,s2s,bls,sliverRatio=0.025 ;
/*
** Initialise
*/
 hullPnt = dtmP->hullPoint ;
 do
   {
    process = 1 ;
    while (process)
      {
       process = 0 ;
       nextPnt = nodeAddrP(dtmP,hullPnt)->hPtr ;
       if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,hullPnt,nextPnt)) < 0 ) goto errexit ;
       if( nodeAddrP(dtmP,antPnt)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,hullPnt,nextPnt) )
         {
          bls = bcdtmMath_pointDistanceDtmObject(dtmP,hullPnt,nextPnt)  ;
          s1s = bcdtmMath_pointDistanceDtmObject(dtmP,hullPnt,antPnt)  ;
          s2s = bcdtmMath_pointDistanceDtmObject(dtmP,antPnt,nextPnt) ;
          if( bls > s1s && bls > s2s )
            {
             if(( s1s + s2s - bls ) / bls  <= sliverRatio ) process = 1 ;
             if( process )
               {
                if((bcdtmList_deleteLineDtmObject(dtmP,hullPnt,nextPnt))) goto errexit ;
                nodeAddrP(dtmP,hullPnt)->hPtr = antPnt ;
                nodeAddrP(dtmP,antPnt)->hPtr  = nextPnt ;
               }
            }
         }
      }
    hullPnt = nextPnt ;
   } while ( hullPnt != dtmP->hullPoint ) ;
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
/*------------------------------------------------------------+
|                                                             |
|                                                             |
|                                                             |
+------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_removeExternalMaxSideTrianglesDtmObjectOld(BC_DTM_OBJ *dtmP,double maxSide)
/*
** This Function Removes Edge Triangles Greater Than Maxside
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p,np,ap,p1=0,p2=0,p3=0,process,numRemoved=0 ;
 double b1s,scanside ;
 double maxSideSquared ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles ** maxSide = %10.4lf",maxSide) ;
/*
** Scan Around Edge And Remove Largest Edge
*/
 if( maxSide > 0.0 )
   {
    maxSideSquared = maxSide * maxSide;
    process = 1 ;
    while ( process )
      {
       process  = 0  ;
       scanside = 0.0 ;
       p = dtmP->hullPoint ;
/*
**     Scan Boundary Line to get largest line
*/
       do
         {
          np = nodeAddrP(dtmP,p)->hPtr ;
          if (np >= 0)
              {
              if( (ap = bcdtmList_nextAntDtmObject(dtmP,p,np) ) < 0 ) goto errexit ;
              if( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p,np) )
                {
                 b1s = bcdtmMath_pointDistanceSquaredDtmObject(dtmP,p,np)  ; //
                 if( b1s > maxSideSquared )
                   {
                    if( b1s > scanside )
                      {
                       p1 = p ;
                       p2 = np ;
                       p3 = ap ;
                       scanside = b1s ;
                      }
                    process = 1 ;
                     }
                 else
                     nodeAddrP(dtmP,p)->hPtr = -np - 1;
                }
                else
                    nodeAddrP(dtmP,p)->hPtr = -np - 1;
              p = np ;
            }
          else
              p = -np - 1;
         } while ( p != dtmP->hullPoint ) ;
/*
**     Delete Largest Hull Line
*/
       if( process )
         {
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Deleting Line %6ld %6ld  %6ld ** Length  = %10.4lf",p1,p2,p3,sqrt(scanside)) ;
             bcdtmWrite_message(0,0,0,"P1 = %6ld  ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
             bcdtmWrite_message(0,0,0,"P2 = %6ld  ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
             bcdtmWrite_message(0,0,0,"P3 = %6ld  ** %10.4lf %10.4lf %10.4lf",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z) ;
            }
              if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) goto errexit ;
              nodeAddrP(dtmP,p1)->hPtr = p3 ;
              nodeAddrP(dtmP,p3)->hPtr = p2 ;
              ++numRemoved ;
         }
      }
   }
/*
** Write Stats On Number Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lines Removed = %6ld",numRemoved) ;
/*
**     Reset back the hull points
*/
       p = dtmP->hullPoint ;
       do
         {
          np = nodeAddrP(dtmP,p)->hPtr ;
          if (np < 0)
              {
              np = -np -1;
              nodeAddrP(dtmP,p)->hPtr = np;
              }
          p = np ;
         } while ( p != dtmP->hullPoint ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Check Tin
*/
 if( cdbg )
   {
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(0,0,0,"Tin Corrupted After Removing Max Side Triangles") ;
       goto errexit  ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

struct removeEdge
    {
    double distance;
    long p1;
    long p2;
    long p3;

    removeEdge()
        {
        }
    removeEdge(double distance, long p1, long p2, long p3)
        {
        this->distance = distance;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        }
    };

BENTLEYDTM_Public int bcdtmTin_removeExternalMaxSideTrianglesDtmObject(BC_DTM_OBJ *dtmP,double maxSide)
/*
** This Function Removes Edge Triangles Greater Than Maxside
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p,np,ap,p1=0,p2=0,p3=0,/*process,*/numRemoved=0 ;
 double b1s,scanside ;
 double maxSideSquared ;
 std::list<removeEdge> removeEdges;
 long startTime;

/*
** Write Entry Message
*/
 startTime  = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles ** maxSide = %10.4lf",maxSide) ;
/*
** Scan Around Edge And Remove Largest Edge
*/
 if( maxSide > 0.0 )
   {
    maxSideSquared = maxSide * maxSide;
    p = dtmP->hullPoint ;
/*
**     Scan Boundary Line to get largest line
*/
    do
     {
      np = nodeAddrP(dtmP,p)->hPtr ;
      if( (ap = bcdtmList_nextAntDtmObject(dtmP,p,np) ) < 0 ) goto errexit ;
      if( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p,np) )
        {
         b1s = bcdtmMath_pointDistanceSquaredDtmObject(dtmP,p,np)  ;
         if( b1s > maxSideSquared )
           {
           removeEdges.push_back (removeEdge(b1s, p, np, ap));
           }
        }
      p = np ;
     } while ( p != dtmP->hullPoint ) ;
/*
**     Delete Largest Hull Line
*/
     while (removeEdges.size())
         {
         std::list<removeEdge>::iterator it = removeEdges.begin();
         std::list<removeEdge>::iterator largest;
         scanside = 0;

        while (it != removeEdges.end())
          {
          if (it->distance > scanside)
            {
            scanside = it->distance;
            largest = it;
            }
          it++;
          }

        p1 = largest->p1;
        p2 = largest->p2;
        p3 = largest->p3;

        removeEdges.erase (largest);

        if (nodeAddrP (dtmP, p3)->hPtr == dtmP->nullPnt)
            {
            if( dbg )
             {
             bcdtmWrite_message(0,0,0,"Deleting Line %6ld %6ld  %6ld ** Length  = %10.4lf",p1,p2,p3,sqrt(scanside)) ;
             bcdtmWrite_message(0,0,0,"P1 = %6ld  ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
             bcdtmWrite_message(0,0,0,"P2 = %6ld  ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
             bcdtmWrite_message(0,0,0,"P3 = %6ld  ** %10.4lf %10.4lf %10.4lf",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z) ;
             }
           if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) goto errexit ;
           nodeAddrP(dtmP,p1)->hPtr = p3 ;
           nodeAddrP(dtmP,p3)->hPtr = p2 ;

           if( (ap = bcdtmList_nextAntDtmObject(dtmP,p1,p3) ) < 0 ) goto errexit ;
           if( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p3) )
            {
             b1s = bcdtmMath_pointDistanceSquaredDtmObject(dtmP, p1, p3)  ;
             if( b1s > maxSideSquared )
               {
               removeEdges.push_back (removeEdge(b1s, p1, p3, ap));
               }
            }

           if( (ap = bcdtmList_nextAntDtmObject(dtmP,p3,p2) ) < 0 ) goto errexit ;
           if( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p3,p2) )
            {
             b1s = bcdtmMath_pointDistanceSquaredDtmObject(dtmP,p3,p2)  ;
             if( b1s > maxSideSquared )
               {
               removeEdges.push_back (removeEdge(b1s, p3, p2, ap));
               }
            }

           ++numRemoved ;
          }
      }
   }
/*
** Write Stats On Number Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lines Removed = %6ld",numRemoved) ;
/*
** Check Tin
*/
 if( cdbg )
   {
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(0,0,0,"Tin Corrupted After Removing Max Side Triangles") ;
       goto errexit  ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Max Side Triangles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_clipTinToBoundaryPolygonDtmObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long   loop,pnt,pnt1,firstPnt = 0,startPnt,nextPnt,insertError,dtmFeature ;
 long   lineNum,knotPoint=0,knotDetected=FALSE,cleanTptrList=TRUE ;
 long   *offsetP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Boundary Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %15ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol     = %15.12lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol     = %15.12lf",dtmP->plTol) ;
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0))
      {
       bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ;
       goto errexit ;
      }
/*
**   Check Precision
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
     if( bcdtmCheck_precisionDtmObject(dtmP,0) )
       {
        bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
        goto errexit ;
       }
   }
/*
** Null Tptr Values
*/
 if( bcdtmList_nullTptrValuesDtmObject(dtmP)) goto errexit ;
/*
** Get Hull Feature
*/
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Hull,TRUE,&dtmFeatureP,&dtmFeature) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"hullFeature = %8ld ** hullFeatureId = %I64d",dtmFeature,dtmFeatureP->dtmFeatureId) ;
/*
** Log Hull Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Hull Points = %8ld",dtmFeatureP->numDtmFeaturePts) ;
    offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
    for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
      {
       bcdtmWrite_message(0,0,0,"Hull Point[%8ld][%8ld] = %12.5lf %12.5lf %10.4lf",pnt,offsetP[pnt],pointAddrP(dtmP,offsetP[pnt])->x,pointAddrP(dtmP,offsetP[pnt])->y,pointAddrP(dtmP,offsetP[pnt])->z) ;
      }
   }
/*
**  Insert Hull Feature
*/
 if( dtmFeatureP != NULL )
   {
/*
**  Try Five Times To Insert Boundary Polygon
*/
    loop = 5 ; insertError = 1 ;
    while ( loop > 0 && insertError )
      {
/*
**     Insert Boundary Polygon Into Tin Object
*/
       lineNum = 0 ;
       insertError = 0 ;
       firstPnt = dtmP->nullPnt ;
       offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts - 1 && ! insertError ; ++pnt )
         {
          startPnt = offsetP[pnt] ;
          nextPnt  = offsetP[pnt+1] ;
          if( firstPnt == dtmP->nullPnt ) firstPnt = startPnt ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Inserting Line[%6ld] ** %8ld %8ld",lineNum,startPnt,nextPnt) ;
          ++lineNum ;
/*
**        Insert Boundary Line Segment Into Tin
*/
          if( ( insertError = bcdtmInsert_lineBetweenPointsDtmObject(dtmP,startPnt,nextPnt,1,2)) == 1 ) goto errexit ;
          if( dbg && insertError ) bcdtmWrite_message(0,0,0,"Insert Error = %2ld",insertError) ;
         }
/*
**     Null Tpr List If Error Detected
*/
       if( insertError ) if( bcdtmList_nullTptrListDtmObject(dtmP,firstPnt)) goto errexit ;
/*
**     Check Connectivity Of Inserted Boundary Polygon
*/
       if( ! insertError )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Inserted Boundary Polygon") ;
          if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,firstPnt,0) ) insertError = 1 ;
         }
/*
**     Retry
*/
       if( insertError ) --loop ;
      }
/*
**  Use Site Insertion Functions If Boundary Polygon Was Not Inserted
*/
    if( insertError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Using Site Insert Functions For Boundary Polygon") ;
       lineNum = 1 ;
       insertError  = 0 ;
       knotDetected = FALSE ;
       firstPnt = dtmP->nullPnt ;
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts - 1 ; ++pnt )
         {
          startPnt = offsetP[pnt] ;
          nextPnt  = offsetP[pnt+1] ;
          if( firstPnt == dtmP->nullPnt ) firstPnt = startPnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line %6ld of %6ld ** %8ld %8ld",lineNum,dtmFeatureP->numDtmFeaturePts-1,startPnt,nextPnt) ;
/*
**        Insert Boundary Line Segment Into Tin Using Site Insertion Functions
*/
          if( bcdtmSite_lineBetweenPointsDtmObject(dtmP,startPnt,nextPnt,1,&knotPoint) == 1 ) goto errexit ;
          if( knotPoint != dtmP->nullPnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Knot Detected At Point %7ld ** %12.6lf %12.6lf %10.4lf",knotPoint,pointAddrP(dtmP,knotPoint)->x,pointAddrP(dtmP,knotPoint)->y,pointAddrP(dtmP,knotPoint)->z) ;
             if( knotPoint != dtmP->nullPnt ) knotDetected = TRUE ;
            }
          ++lineNum ;
/*
**        Log Inserted Segment
*/
          if( dbg == 1 )
            {
             loop = 1 ;
             pnt1 = startPnt ;
             while( loop )
               {
                bcdtmWrite_message(0,0,0,"startPnt = %8ld nextPnt = %8ld ** pnt = %8ld",startPnt,nextPnt,pnt1) ;
                if( pnt1 == nextPnt || pnt1 == dtmP->nullPnt ) loop = 0 ;
                else                                           pnt1 = nodeAddrP(dtmP,pnt1)->tPtr ;
               }
            }
/*
**        Check Tin Precision
*/
          if( cdbg )
            {
             if( bcdtmCheck_precisionDtmObject(dtmP,1) )
               {
                bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
                goto errexit ;
               }
            }
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Boundary Polygon Inserted") ;
      }
/*
**  Check Boundary Polygon Is Present
*/
    if( firstPnt == dtmP->nullPnt ) insertError = 1 ;
/*
**  If Knots Detected Check And Clean Tptr Polygon
*/
    if( ! insertError && knotDetected == TRUE )
      {
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPnt,1)) insertError = 1 ;
       else if( bcdtmList_cleanTptrPolygonDtmObject(dtmP,firstPnt))          insertError = 1 ;
      }
/*
**  Check For Connectivity Errors
*/
   if( ! insertError )
     {
      if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPnt,1)) insertError = 1 ;
     }
/*
**  If Errors Return
*/
    if( insertError )
      {
       bcdtmWrite_message(1,0,0,"Error In Boundary Polygon") ;
//       goto errexit ;
       goto cleanup ;
      }
/*
**  Clip Features External To Boundary Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping External To Boundary Polygon") ;
    if( dbg == 1 ) bcdtmList_writeTptrListDtmObject(dtmP,firstPnt) ;
    if( cleanTptrList == TRUE ) if( bcdtmList_cleanTptrPolygonDtmObject(dtmP,firstPnt)) goto errexit ;

    if( dtmP->extended && dtmP->extended->rollBackInfoP && dtmP->extended->rollBackInfoP->rollBackDtmP != NULL  && bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes)) if( bcdtmInsert_rollBackDtmFeaturesExternalToTptrPolygonDtmObject(dtmP,firstPnt)) goto errexit ;
    if( bcdtmClip_externalToTptrPolygonDtmObject(dtmP,firstPnt) )
      {
       bcdtmWrite_message(1,0,0,"Error Clipping External To Boundary Polygon") ;
       goto errexit ;
      }
/*
**  Check Tin
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Checking Tin After Inserting Boundary Polygon") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP))
         {
          bcdtmWrite_message(1,0,0,"Tin Corrupted After Inserting Boundary Polygon") ;
          goto errexit  ;
         }
      }
/*
**  Set Hull Feature To Tin State
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Hull Feature To Tin state") ;
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeaturePts.offsetPI != 0 )
      {
       bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
       dtmFeatureP->dtmFeaturePts.offsetPI = 0 ;
      }
   dtmFeatureP->dtmFeatureState = DTMFeatureState::Tin ;
   dtmFeatureP->dtmFeaturePts.firstPoint = firstPnt ;
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Boundary Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Boundary Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}

BENTLEYDTM_Private bool bcdtmInsert_rollBackMapSortCompare (DTM_ROLLBACK_FEATURE_MAP& m1, DTM_ROLLBACK_FEATURE_MAP& m2)
    {
    return m1.featureId < m2.featureId;
    }

BENTLEYDTM_Public int bcdtmInsert_rollBackDtmFeatureDtmObject(
 BC_DTM_OBJ *dtmP,                      // ==> Pointer To DTM Object
 DTMFeatureId featureId)
    {
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    dtmFeature,numFeaturePts ;
 DPoint3d     *featurePtsP=NULL;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

 if (dbg)
     {
     bcdtmWrite_message(0,0,0,"bcdtmInsert_rollBackDtmFeatureDtmObject") ;
     bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
     bcdtmWrite_message(0,0,0,"rollBackInfo->rollBackDtmP = %p",rollBackInfo->rollBackDtmP) ;
     bcdtmWrite_message(0,0,0,"rollBackInfo->rollBackDtmP = %d",rollBackInfo->rollBackMapInitialized) ;
     bcdtmWrite_message(0,0,0,"featureId     = %8ld",featureId) ;
     }

 if (!rollBackInfo->rollBackMapInitialized)
     {
     DTM_ROLLBACK_FEATURE_MAP mapEntry;

     rollBackInfo->rollBackMap.clear();
     rollBackInfo->rollBackMap.empty();
     if (dbg) bcdtmWrite_message(0,0,0,"Loading the featureMap for rollback") ;
     for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
       {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       mapEntry.featureId = dtmFeatureP->dtmFeatureId;
       mapEntry.featureIndex = dtmFeature;
       rollBackInfo->rollBackMap.push_back (mapEntry);
       }
     std::sort (rollBackInfo->rollBackMap.begin(), rollBackInfo->rollBackMap.end(), &bcdtmInsert_rollBackMapSortCompare);
     rollBackInfo->rollBackMapInitialized = true;
     }

  for( dtmFeature = 0 ; dtmFeature < rollBackInfo->rollBackDtmP->numFeatures ; ++dtmFeature )
     {
     dtmFeatureP = ftableAddrP(rollBackInfo->rollBackDtmP,dtmFeature) ;
     if (dtmFeatureP->dtmFeatureId == featureId)
         {
         if (dbg) bcdtmWrite_message(0,0,0," -- Already Exists") ;

         goto cleanup;
         }
     }

  if (rollBackInfo->rollBackMap.size() != 0)
      {
      // Find first element which has this featureId.
  DtmRollBackFeatureMap::iterator feat1P = rollBackInfo->rollBackMap.begin() ;
  DtmRollBackFeatureMap::iterator feat2P = rollBackInfo->rollBackMap.end() - 1 ;
    if( featureId > feat1P->featureId)
      {
      if ( featureId <= feat2P->featureId )
          {
          if ( featureId == feat2P->featureId )
              feat1P = feat2P;
          else
              {
               DtmRollBackFeatureMap::iterator feat3P = feat1P + ((feat2P - feat1P) / 2);
               while( feat3P != feat1P && feat3P != feat2P )
                 {
                  if( featureId == feat3P->featureId )
                      {
                      feat1P = feat3P;
                      break;
                      }

                 if      ( featureId < feat3P->featureId ) feat2P = feat3P ;
                 else if ( featureId > feat3P->featureId ) feat1P = feat3P ;
                 feat3P = feat1P + ((feat2P - feat1P) / 2);
                 }
              }
          while (featureId == feat1P->featureId)
              feat1P--;
          feat1P++;
          }
      }

    while (feat1P < rollBackInfo->rollBackMap.end() && feat1P->featureId == featureId)
       {
       dtmFeature = feat1P->featureIndex;
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if (dbg)
            bcdtmWrite_message(0,0,0," --- featureId = %8ld", dtmFeatureP->dtmFeatureId) ;

        if( featureId != dtmFeatureP->dtmFeatureId)
            {
            bcdtmWrite_message(1,0,0,"Feature Id's don't match") ;
            goto errexit;
            }

        if( (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin) && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
            {
            if (dbg)
                bcdtmWrite_message(0,0,0," ----- Adding featureId to rollback = %d %d %d", (long)dtmFeatureP->dtmFeatureId, (long)dtmFeatureP->dtmUserTag, (long)dtmFeatureP->numDtmFeaturePts) ;
            if( bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
            if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
            }
        feat1P++;
        }
       }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM Features External To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM Features External To Tptr Polygon Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_rollBackDtmFeaturesExternalToTptrPolygonDtmObject
(
 BC_DTM_OBJ *dtmP,                      // ==> Pointer To DTM Object
 long       startPnt                    // ==> Tptr Polygon Start Point
 )
/*
** This Function Rolls Back Dtm Features External To A Tptr Polygon
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    node,clc,spnt,fspnt,dtmFeature,numFeatures ;
 DTMFeatureType dtmFeatureType;
 long    concaveSpan,numMarked,numPts,mark=-987654321 ;
 DPoint3d     *featurePtsP=NULL,*markP,*markedPtsP=NULL ;
 DTM_TIN_NODE       *nodeP ;
 DTM_TIN_POINT      *pointP ;
 BC_DTM_FEATURE     *dtmFeatureP ;
 DTMFeatureId     nullFeatureId=DTM_NULL_FEATURE_ID  ;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

 if( !( dtmP->extended && dtmP->extended->rollBackInfoP && dtmP->extended->rollBackInfoP->rollBackDtmP != NULL  && bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes)))
   goto cleanup;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Rolling Back DTM Features External To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt     = %8ld",startPnt) ;
   }
/*
** Mark Points External To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Clip Polygon") ;
 bcdtmMark_externalTptrPolygonPointsDtmObject(dtmP,startPnt,mark,&numMarked) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External Points = %6ld",numMarked) ;
/*
**  Copy Tptr List To Sptr and Null Out Tptr
*/
 if( bcdtmList_nullSptrValuesDtmObject(dtmP)) goto errexit ;
 if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Scan Feature Lists And Detect And Fix Feature Lines That Span Concave Clip Boundaries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping And Fixing Concave Sections") ;
 if( bcdtmClip_scanFeatureListAndDetectAndFixConcaveSpansDtmObject(dtmP,mark) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping And Fixing Concave Sections Completed") ;
/*
** Roll Back Features That Will Be Clipped
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature Table For Marked Features") ;
 numFeatures = dtmP->numFeatures ;
 for( dtmFeature = 0 ; dtmFeature < numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       dtmFeatureType = dtmFeatureP->dtmFeatureType ;
/*
**     Initialise Scan Variables
*/
       numPts = 1 ;
       numMarked = 0 ;
       fspnt = spnt  = (long ) dtmFeatureP->dtmFeaturePts.firstPoint  ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %2ld firstPoint = %8ld",dtmFeatureType,fspnt) ;
       clc   = nodeAddrP(dtmP,fspnt)->fPtr ;
       if( nodeAddrP(dtmP,fspnt)->tPtr == mark ) ++numMarked ;
/*
**     Scan Feature List Pointers
*/
       concaveSpan = FALSE ;
       while ( clc != dtmP->nullPtr )
         {
          while ( clc != dtmP->nullPtr  && flistAddrP(dtmP,clc)->dtmFeature != dtmFeature ) clc = flistAddrP(dtmP,clc)->nextPtr ;
          if( clc != dtmP->nullPtr )
            {
             spnt = flistAddrP(dtmP,clc)->nextPnt ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %2ld firstPoint = %8ld nextPoint = %8ld",dtmFeatureType,fspnt,spnt) ;
             if( spnt != dtmP->nullPnt )
               {
                ++numPts ;
                if( nodeAddrP(dtmP,spnt)->tPtr == mark ) ++numMarked ;
                if( nodeAddrP(dtmP,spnt)->tPtr != mark && nodeAddrP(dtmP,spnt)->tPtr != dtmP->nullPnt ) concaveSpan = TRUE ;
                clc = nodeAddrP(dtmP,spnt)->fPtr ;
               }
             if( spnt == dtmP->nullPnt || spnt == fspnt ) clc = dtmP->nullPtr ;
            }
         }
/*
**     Roll Back Feature
*/
       if( ( numMarked == 0 && concaveSpan ) || numMarked > 0 )
         {
/*
**        Write Feature To Roll Back DTM
*/
          if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
         }
      }
   }
/*
** Roll Back Random Points That Will Be Clipped
*/
  markedPtsP = ( DPoint3d *) malloc(1000*sizeof(DPoint3d)) ;
  if( markedPtsP == NULL )
    {
     bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
     goto errexit ;
    }
  numMarked = 0 ;
  markP = markedPtsP ;
  for( node = 0  ; node < dtmP->numPoints ; ++node )
    {
     nodeP = nodeAddrP(dtmP,node) ;
     if( nodeP->tPtr == mark && nodeP->fPtr == dtmP->nullPtr  )
       {
        pointP = pointAddrP(dtmP,node) ;
        markP->x = pointP->x ;
        markP->y = pointP->y ;
        markP->z = pointP->z ;
        nodeP->tPtr = dtmP->nullPnt ;
        if( numMarked == 1000 )
          {
           if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,markedPtsP,numMarked)) goto errexit ;
           numMarked = 0 ;
           markP = markedPtsP ;
          }
       }
    }
  if( numMarked > 0 )
    {
     if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,markedPtsP,numMarked)) goto errexit ;
     numMarked = 0 ;
     markP = markedPtsP ;
    }
/*
** Restore Polygon Pointers
*/
 if( bcdtmList_nullTptrValuesDtmObject(dtmP)) goto errexit ;
 if( bcdtmList_copySptrListToTptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_nullSptrListDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( markedPtsP  != NULL ) { free(markedPtsP)  ; markedPtsP  = NULL ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM Features External To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM Features External To Tptr Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_clipTinToDrapeBoundaryPolygonDtmObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long   dtmFeature,firstPnt=0 ;
 DPoint3d    *p3dP,*hullPtsP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId dtmFeatureId;
 DTMUserTag dtmUserTag;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Drape Boundary Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %15ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol     = %15.12lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol     = %15.12lf",dtmP->plTol) ;
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0))
      {
       bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ;
       goto errexit ;
      }
/*
**   Check Precision
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
     if( bcdtmCheck_precisionDtmObject(dtmP,0) )
       {
        bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
        goto errexit ;
       }
   }
/*
** Get Hull Feature
*/
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::DrapeHull,TRUE,&dtmFeatureP,&dtmFeature) ;
/*
**  Insert Drape Hull Feature
*/
 if( dtmFeatureP != NULL )
   {
   dtmFeatureId = dtmFeatureP->dtmFeatureId;
   dtmUserTag = dtmFeatureP->dtmUserTag;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"hullFeature   = %8ld ** hullFeatureId = %I64d",dtmFeature,dtmFeatureP->dtmFeatureId) ;
       bcdtmWrite_message(0,0,0,"Feature State = %2ld ** numPoints     = %8ld",dtmFeatureP->dtmFeatureState,dtmFeatureP->numDtmFeaturePts) ;
       hullPtsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
       for( p3dP = hullPtsP ; p3dP < hullPtsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Hull Point[%6ld] = %12.5lf %12.5lf %12.5lf",(long)(p3dP-hullPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**  Clip DTM To Drape Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM To Drape Hull") ;
    if (bcdtmClip_toPolygonDtmObject (dtmP, (DPoint3d *)bcdtmMemory_getPointerP3D (dtmP, dtmFeatureP->dtmFeaturePts.pointsPI), dtmFeatureP->numDtmFeaturePts, DTMClipOption::External)) goto errexit;
/*
**  Set Hull Feature To Tin State
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Hull Feature To Tin state") ;
    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::DrapeHull,TRUE,&dtmFeatureP,&dtmFeature) ;
    if (dtmFeatureP != NULL)
        {
        if( dtmFeatureP->dtmFeaturePts.offsetPI != 0 )
          {
           bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
           dtmFeatureP->dtmFeaturePts.offsetPI = 0 ;
          }

       dtmFeatureP->dtmFeatureType  = DTMFeatureType::Hull ;
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Tin ;
       dtmFeatureP->dtmFeaturePts.firstPoint = firstPnt ;
        }
    else
        {
        if( dbg && dtmFeatureP == NULL) bcdtmWrite_message(0,0,0,"Feature no longer exists in the dtm.") ;
        if( bcdtmInsert_addToFeatureTableDtmObject (dtmP, dtmFeatureP, 0, DTMFeatureType::Hull, dtmUserTag, dtmFeatureId, firstPnt, &dtmFeature)) goto errexit;
        }
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Drape Boundary Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Drape Boundary Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_clipTinToBoundaryLinesDtmObject(BC_DTM_OBJ *dtmP,double maxSide)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     count,feature,numHullPts,numTinFeatures,numBeforeJoin,numAfterJoin,dtmFeatureNum ;
 long     numJoinUserTags ;
 DPoint3d      *hullPtsP=NULL ;
 DTM_JOIN_USER_TAGS *joinUserTagsP=NULL ;
 BC_DTM_OBJ     *dataP=NULL   ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxSide = %10.4lf",maxSide) ;
   }
/*
** Initialise
*/
 numTinFeatures = dtmP->numFeatures ;
/*
** Count Number Of Boundary Line Points
*/
 count = 0 ;
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::HullLine,TRUE,&dtmFeatureP,&dtmFeatureNum) ;
 while( dtmFeatureP != NULL )
   {
    count = count + dtmFeatureP->numDtmFeaturePts ;
    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::HullLine,FALSE,&dtmFeatureP,&dtmFeatureNum) ;
   }
/*
** Only Process If More Than Two Boundary Line Points
*/
 if( count > 2 )
   {
/*
**  Create Data Object
*/
    if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dataP,count,count) ;
/*
**  Copy Boundary Lines To Data Object
*/
    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::HullLine,TRUE,&dtmFeatureP,&dtmFeatureNum) ;
    while( dtmFeatureP != NULL )
      {
       if( bcdtmObject_getPointsForDtmFeatureDtmObject(
           dtmP,
           dtmFeatureNum,
           ((DTM_TIN_POINT **)&hullPtsP),
           &numHullPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::HullLine,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
       bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::HullLine,FALSE,&dtmFeatureP,&dtmFeatureNum) ;
      }
/*
**  Only Process If Boundary Lines Present
*/
    if( dataP->numPoints > 0 )
      {
/*
**     Join Boundary Lines
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Joining Boundary Lines") ;
       if( bcdtmJoin_dtmFeatureTypeDtmObject(dataP,dtmP->ppTol*2.0,DTMFeatureType::HullLine,DTMFeatureType::HullLine,&numBeforeJoin,&numAfterJoin,&joinUserTagsP,&numJoinUserTags)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Number Boundary Lines Before Join = %4ld",numBeforeJoin) ;
          bcdtmWrite_message(0,0,0,"Number Boundary Lines After  Join = %4ld",numAfterJoin) ;
         }
/*
**     Write Out Data Object
*/
       if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"bdylines.dat") ;
/*
**     Remove Boundary Lines From Feature Table
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Boundary Line Features") ;
       for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
         {
          dtmFeatureP = ftableAddrP(dtmP,feature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine && dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Boundary Line Feature %8ld",feature) ;
             bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
             dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
            }
         }
/*
**     Insert Boundary Lines Into DTMFeatureState::Tin
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Boundary Lines ** dtmP->numPoints = %6ld dtmP->numFeatures = %6ld",dtmP->numPoints,dtmP->numFeatures) ;
       if( bcdtmTin_insertBoundaryLinesDtmObject(dtmP,dataP)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Boundary Lines Inserted  ** dtmP->numPoints = %6ld dtmP->numFeatures = %6ld",dtmP->numPoints,dtmP->numFeatures) ;
/*
**     Write Out Tin With Boundary Lines Set To Hard Breaks
*/
       if( dbg )
         {
          for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
            {
             if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Breakline )ftableAddrP(dtmP,feature)->dtmFeatureType = (DTMFeatureType)-999 ;
             if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::HullLine  )ftableAddrP(dtmP,feature)->dtmFeatureType = DTMFeatureType::Breakline ;
            }
          bcdtmWrite_toFileDtmObject(dtmP,L"bdyLines.tin") ;
          for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
            {
             if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Breakline )ftableAddrP(dtmP,feature)->dtmFeatureType = DTMFeatureType::HullLine ;
             if (ftableAddrP (dtmP, feature)->dtmFeatureType == (DTMFeatureType)-999)ftableAddrP (dtmP, feature)->dtmFeatureType = DTMFeatureType::Breakline;
            }
         }
/*
**     Do Max Side Processing
*/
       if( maxSide > 0.0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing External Max Side Triangles") ;
          if( bcdtmTin_removeExternalMaxSideTrianglesDtmObject(dtmP,maxSide) ) goto errexit ;
         }
/*
**     Clip Tin To Boundary Lines
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin Hull To Boundary Lines") ;
       if( bcdtmTin_clipHullToBoundaryLinesDtmObject(dtmP,numTinFeatures)) goto errexit ;
/*
**     Remove Boundary Line Features From Tin
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Boundary Lines Features From Tin") ;
       for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
         {
          if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,feature)) goto errexit ;
         }
/*
**     Check Tin
*/
       if( cdbg )
         {
          if(bcdtmCheck_tinComponentDtmObject(dtmP))
            {
             bcdtmWrite_message(0,0,0,"Tin Corrupted After Processing Boundary Lines") ;
             goto errexit  ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dataP         != NULL ) bcdtmObject_destroyDtmObject(&dataP) ;
 if( joinUserTagsP != NULL ) free(joinUserTagsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Tin To Boundary Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_insertBoundaryLinesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *dataP)
/*
** This Function Inserts Boundary Lines Into A Tin Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  error = 0,sp,np,numFeatures,dtmFeature ;
 long  cp1,cp2,point,firstPoint,lastPoint,firstNewPoint ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *p1P,*p2P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Boundary Lines Into Tin Dtm Object") ;
/*
** Write Hull Line Features
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Boundary Line Features = %8ld",dataP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine )
         {
          bcdtmWrite_message(0,0,0,"Boundary Line Feature = %8ld",dtmFeature) ;
         }
      }
   }
/*
** Null Out Temporary Pointers
*/
 numFeatures = dtmP->numFeatures ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Insert Boundary Lines Into Tin
*/
 for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine )
      {
       firstPoint = (long) dtmFeatureP->dtmFeaturePts.firstPoint ;
       lastPoint  = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
       firstNewPoint = dtmP->nullPnt ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Boundary Line Feature %8ld ** firstPoint = %8ld lastPoint = %8ld",dtmFeature,firstPoint,lastPoint) ;
       for( point = firstPoint + 1 ; point <= lastPoint ; ++point )
         {
          p1P = pointAddrP(dataP,point-1) ;
          p2P = pointAddrP(dataP,point) ;
          bcdtmFind_closestPointDtmObject(dtmP,p1P->x,p1P->y,&cp1) ;
          bcdtmFind_closestPointDtmObject(dtmP,p2P->x,p2P->y,&cp2) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Boundary Line Segment ** %12.5lf %12.5lf ** %12.5lf %12.5lf",p1P->x,p1P->y,p2P->x,p2P->y) ;
/*
**        Insert Line
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points ** %8ld %8ld",cp1,cp2) ;
          if( cp1 != cp2 )
            {
             if( firstNewPoint == dtmP->nullPnt ) firstNewPoint = cp1 ;
             if( ( error = bcdtmInsert_lineBetweenPointsDtmObject(dtmP,cp1,cp2,1,2)) == 1 ) goto errexit ;
             if( dbg && error ) bcdtmWrite_message(0,0,0,"Insert Error = %2ld ** Fp = %6ld Lp = %6ld",error,cp1,cp2) ;
             if( error == 2  ) point = lastPoint ;
            }
         }
/*
**     Check Line Inserted
*/
       if( firstNewPoint != dtmP->nullPnt && nodeAddrP(dtmP,firstNewPoint)->tPtr != dtmP->nullPnt )
         {
          error = 0 ;
/*
**        Check Boundary Line For Loop Back
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Boundary Line For Loop Back") ;
          if( nodeAddrP(dtmP,nodeAddrP(dtmP,firstNewPoint)->tPtr)->tPtr == firstNewPoint )
            {
             nodeAddrP(dtmP,nodeAddrP(dtmP,firstNewPoint)->tPtr)->tPtr = dtmP->nullPnt ;
             error = 1 ;
            }
/*
**        Clean Tptr Feature
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Tptr Polygon") ;
          sp = firstNewPoint ;
          do
            {
             np = nodeAddrP(dtmP,sp)->tPtr ;
             if( np != dtmP->nullPnt )
               {
                nodeAddrP(dtmP,sp)->tPtr = -(nodeAddrP(dtmP,sp)->tPtr+1) ;
                if( np != firstNewPoint && nodeAddrP(dtmP,np)->tPtr < 0 )
                  {
                   nodeAddrP(dtmP,np)->tPtr = dtmP->nullPnt ;
                   error = 1 ;
                  }
               }
             sp = np ;
            } while ( sp != firstNewPoint && sp != dtmP->nullPnt )  ;

          sp = firstNewPoint ;
          do
            {
             if( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) nodeAddrP(dtmP,sp)->tPtr = -(nodeAddrP(dtmP,sp)->tPtr + 1 ) ;
             sp = nodeAddrP(dtmP,sp)->tPtr ;
            } while ( sp != firstNewPoint && sp != dtmP->nullPnt )  ;
/*
**        Insert Boundary Line Into Feature Tables
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::HullLine,dtmP->nullUserTag,dtmP->nullFeatureId,firstNewPoint,1) ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"00 Feature = %5ld Utag = %9I64d Spnt = %6ld",dtmP->numFeatures-1,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmUserTag,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmFeaturePts.firstPoint) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"01 Feature = %5ld Utag = %9I64d Spnt = %6ld",dtmP->numFeatures-1,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmUserTag,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmFeaturePts.firstPoint) ;
         }
/*
**     If Error Null Out Tptr Values
*/
       if( error ) bcdtmList_nullTptrValuesDtmObject(dtmP) ;
      }
   }
/*
** Write Out Boundary Lines ** Developement Only
*/
 if( dbg )
   {
    for( sp = numFeatures ; sp < dtmP->numFeatures ; ++sp )
      {
       bcdtmWrite_message(0,0,0,"Feature = %6ld Fpnt = %6ld Utag = %12I64d Type = %2ld",sp,ftableAddrP(dtmP,sp)->dtmFeaturePts.firstPoint,ftableAddrP(dtmP,sp)->dtmUserTag,ftableAddrP(dtmP,sp)->dtmFeatureType) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Boundary Lines Into Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Boundary Lines Into Tin Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_clipHullToBoundaryLinesDtmObject(BC_DTM_OBJ *dtmP, long numTinFeatures)
/*
** This Function Clips The Hull To Boundary Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,feature,process,numAssigned,numRemoved,totalNumRemoved ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Clipping Tin Hull To Boundary Lines") ;
    bcdtmWrite_message(0,0,0,"tinp           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures    = %6ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"numTinFeatures = %6ld",numTinFeatures) ;
   }
/*
** Only Process If Boundary Lines Present
*/
 if( numTinFeatures < dtmP->numFeatures )
   {
/*
**  Write Out Boundary Lines
*/
    if( dbg == 2 )
      {
       for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
         {
          if( ( sp = (long ) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
            {
             bcdtmWrite_message(0,0,0,"Boundary Line Feature = %6ld ** Direction = %9I64d",feature,ftableAddrP(dtmP,feature)->dtmUserTag) ;
             do
               {
                bcdtmWrite_message(0,0,0,"**** Point[%9ld] fTableP = %9ld ** %12.5lf %12.5lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
                sp = np ;
               } while ( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
            }
         }
      }
/*
**  Remove Slivers From Tin Hull Prior To Clipping
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Slivers From Tin Hull") ;
    if( bcdtmTin_removeSliversPriorToBoundaryLineClipDtmObject(dtmP,&numRemoved)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slivers Removed = %6ld",numRemoved) ;
/*
**  Remove Hull Lines With A Boundary Point Vertex
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Hull Lines With A Boundary Point Vertex") ;
    if( bcdtmTin_removeHullLinesWithBoundaryVextexDtmObject(dtmP,numTinFeatures,&numRemoved)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Hull Lines Removed = %6ld",numRemoved) ;
/*
**  Repeatatively Scan Boundary Lines Until No More Edge Triangles Are Removed
*/
    process = 1 ;
    while ( process  )
      {
       process = 0 ;
       totalNumRemoved = 0 ;
/*
**     Assign Direction To Boundary Lines From Two Hull Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Two Hull Points") ;
       if( bcdtmTin_assignDirectionToBoundaryLinesFromTwoHullPointsDtmObject(dtmP,numTinFeatures,&numAssigned) ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Assigned = %6ld",numAssigned) ;
/*
**     Remove Triangles External To Boundary Lines
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Triangles External To Boundary Lines") ;
       for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
         {
          if ( ftableAddrP(dtmP,feature)->dtmUserTag == 1 )
            {
             if( bcdtmTin_removeTrianglesExternalToBoundaryLineDtmObject(dtmP,feature,&numRemoved)) goto errexit ;
             totalNumRemoved =  totalNumRemoved + numRemoved ;
            }
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Removed  = %6ld",totalNumRemoved) ;
/*
**     Set Rescan
*/
       if( totalNumRemoved ) process = 1 ;
/*
**     Remove Hull Lines With A Boundary Point Vertex
*/
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing Hull Lines With A Boundary Point Vertex") ;
          if( bcdtmTin_removeHullLinesWithBoundaryVextexDtmObject(dtmP,numTinFeatures,&numRemoved)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Hull Lines Removed = %6ld",numRemoved) ;
          if( numRemoved ) process = 1 ;
/*
**        Assign Direction From A Single Hull Point
*/
          else
            {
             if( dbg  ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Hull Points") ;
             if( bcdtmTin_assignDirectionToBoundaryLinesFromHullPointDtmObject(dtmP,numTinFeatures,&numAssigned) ) goto errexit ;
             if( dbg  ) bcdtmWrite_message(0,0,0,"Number Assigned = %6ld",numAssigned) ;
             if( numAssigned ) process = 1 ;
            }
         }
      }
   }
/*
** Write Statistics On Processed Boundary Lines
*/
 if( dbg )
   {
    numRemoved =0 ;
    numAssigned = 0 ;
    totalNumRemoved = 0 ;
    for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
      {
       if ( ftableAddrP(dtmP,feature)->dtmUserTag == dtmP->nullUserTag ) ++numRemoved ;
       if ( ftableAddrP(dtmP,feature)->dtmUserTag == 1 ) ++numAssigned ;
       if ( ftableAddrP(dtmP,feature)->dtmUserTag == 2 ) ++totalNumRemoved ;
      }
    bcdtmWrite_message(0,0,0,"**** Number Of Boundary Lines   = %6ld",dtmP->numFeatures-numTinFeatures) ;
    bcdtmWrite_message(0,0,0,"**** Number Not Processed       = %6ld",numRemoved) ;
    bcdtmWrite_message(0,0,0,"**** Number Partially Processed = %6ld",numAssigned) ;
    bcdtmWrite_message(0,0,0,"**** Number Fully  Processed    = %6ld",totalNumRemoved) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Clipping Tin Hull To Boundary Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Clipping Tin Hull To Boundary Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*--------------------------------------------------------------------+
|                                                                     |
|                                                                     |
|                                                                     |
+--------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_removeSliversPriorToBoundaryLineClipDtmObject(BC_DTM_OBJ *dtmP,long *numRemovedP)
/*
** This Function Removes Slivers Prior To Boundary Line Clipping
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,np,pp,fsp,flp,process,removeFlag ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Eliminate Hull Lines That Are External To A Start Or End Point
*/
 process = 1 ;
 while( process )
   {
    process = 0 ;
    sp = dtmP->hullPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->hPtr ;
       if( nodeAddrP(dtmP,sp)->tPtr == dtmP->nullPnt && nodeAddrP(dtmP,np)->tPtr == dtmP->nullPnt )
         {
          if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,np) )
            {
             if( ( pp = bcdtmList_nextAntDtmObject(dtmP,sp,np) ) < 0 ) goto errexit ;
             if( nodeAddrP(dtmP,pp)->hPtr == dtmP->nullPnt )
               {
                if( nodeAddrP(dtmP,pp)->tPtr != dtmP->nullPnt  )
                  {
                   if( bcdtmList_getFirstAndLastPointForDtmTinFeatureDtmObject(dtmP,nodeAddrP(dtmP,pp)->tPtr,&fsp,&flp) ) goto errexit ;
                   if( pp == fsp || pp == flp )
                     {
                      bcdtmList_deleteLineDtmObject(dtmP,sp,np) ;
                      nodeAddrP(dtmP,sp)->hPtr = pp ;
                      nodeAddrP(dtmP,pp)->hPtr = np  ;
                       process = 1 ;
                      ++*numRemovedP ;
                     }
                  }
               }
            }
         }
       sp = np ;
      } while ( sp != dtmP->hullPoint ) ;
   }
/*
**
**  Eliminate Hull Lines That Conect To The Same Boundary Line
**
*/
 process = 1 ;
 while( process )
   {
    process = 0 ;
    sp = dtmP->hullPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->hPtr ;
       if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,np) )
         {
          if( nodeAddrP(dtmP,sp)->tPtr == nodeAddrP(dtmP,np)->tPtr && nodeAddrP(dtmP,np)->tPtr != dtmP->nullPnt  )
            {
             if( ( fsp = bcdtmList_nextAntDtmObject(dtmP,sp,np) ) < 0 ) goto errexit ;
             if( nodeAddrP(dtmP,fsp)->hPtr == dtmP->nullPnt ) /* && nodeAddrP(dtmP,fsp)->tPtr == nodeAddrP(dtmP,sp)->tPtr ) */
               {
                if( bcdtmTin_testIfHullLineCanBeRemovedDtmObject(dtmP,nodeAddrP(dtmP,sp)->tPtr,sp,np,&removeFlag)) goto errexit ;
                if( removeFlag )
                  {
                   bcdtmList_deleteLineDtmObject(dtmP,sp,np) ;
                   nodeAddrP(dtmP,sp)->hPtr  = fsp ;
                   nodeAddrP(dtmP,fsp)->hPtr = np  ;
                   process = 1 ;
                   ++(*numRemovedP) ;
                  }
               }
            }
         }
       sp = np ;
      } while ( sp != dtmP->hullPoint ) ;
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
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_testIfHullLineCanBeRemovedDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long startPnt,long nextPnt,long *removeFlagP)
/*
** This Function Tests If A Hull Line Can Be Removed
** By Calculating The Area From startPnt To nextPnt Along dtmFeature Back To startPnt
**
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp,process,clPtr ;
 DTMDirection direction;
 double hullArea,dtmFeatureArea,sx,sy,x,y  ;
/*
** Initialise
*/
 *removeFlagP = 0 ;
/*
** Calculate Hull Area
*/
 bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,dtmP->hullPoint,&hullArea,&direction) ;
/*
** Test Boundary Line Can Be Traversed From startPnt To nextPnt
*/
 sp = startPnt ;
 process = 1 ;
 while ( sp != nextPnt && process )
   {
    if( ( clPtr = nodeAddrP(dtmP,sp)->fPtr ) == dtmP->nullPtr ) process = 0 ;
    else
      {
       while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
       if( clPtr == dtmP->nullPtr ) process = 0 ;
       else                         sp = flistAddrP(dtmP,clPtr)->nextPnt ;
      }
   }
/*
** Reverse dtmFeature If Necessary
*/
 if( ! process ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,dtmFeature)) goto errexit ;
/*
** Calculate Area Between Boundary Line And Hull Line
*/
 sp = startPnt ;
 dtmFeatureArea = 0.0 ;
 sx = pointAddrP(dtmP,sp)->x ;
 sy = pointAddrP(dtmP,sp)->y ;
 process = 1 ;
 do
   {

    if( ( clPtr = nodeAddrP(dtmP,sp)->fPtr ) == dtmP->nullPtr ) process = 0 ;
    else
      {
       while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
       if( clPtr == dtmP->nullPtr ) process = 0 ;
       else
         {
          sp = flistAddrP(dtmP,clPtr)->nextPnt ;
          x  = pointAddrP(dtmP,sp)->x - sx ;
          y  = pointAddrP(dtmP,sp)->y - sy ;
          dtmFeatureArea = dtmFeatureArea + x * y / 2.0 + x * pointAddrP(dtmP,sp)->y ;
          sx = pointAddrP(dtmP,sp)->x ;
          sy = pointAddrP(dtmP,sp)->y ;
         }
      }
   }  while ( sp != nextPnt && process ) ;
 if( ! process ) { bcdtmWrite_message(1,0,0,"Error Calculating Area Between Hull Line And Boundary Line") ; goto errexit ; }
 x  = pointAddrP(dtmP,startPnt)->x - sx ;
 y  = pointAddrP(dtmP,startPnt)->y - sy ;
 dtmFeatureArea = dtmFeatureArea + x * y / 2.0 + x * pointAddrP(dtmP,startPnt)->y ;
/*
** Set Remove Flag
*/
 if( dtmFeatureArea < 0.0 ) dtmFeatureArea = - dtmFeatureArea ;
 if( dtmFeatureArea < ( hullArea - dtmFeatureArea )) *removeFlagP = 1 ;
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
/*--------------------------------------------------------------------+
|                                                                     |
|                                                                     |
|                                                                     |
+--------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_removeHullLinesWithBoundaryVextexDtmObject(BC_DTM_OBJ *dtmP,long numTinFeatures,long *numRemovedP)
/*
** This Function Removes Hull Lines With Boundary Point Vertex
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,np,na,feature,pLine,nLine,mark=-56788765,process ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Hull Lines With Boundary Point Vertex") ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Scan Boundary Line Features And Mark Tin Points On Boundary Feature
*/
 for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if( ( sp = (long) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       if( ftableAddrP(dtmP,feature)->dtmUserTag == dtmP->nullUserTag )
         {
          do
            {
             nodeAddrP(dtmP,sp)->tPtr = mark ;
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
             sp = np ;
            } while( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
         }
      }
   }
/*
** Scan Hull And Remove Hull Lines
*/
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    sp = dtmP->hullPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->hPtr ;
       if( ( na = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
/*
**     Check If Hull Line Vextex Is A Boundary Line Point
*/
       if( nodeAddrP(dtmP,na)->tPtr == mark )
         {
/*
**        Test Hull Line Vertex Is Not A Hull Point
*/
          if( nodeAddrP(dtmP,na)->hPtr == dtmP->nullPnt && nodeAddrP(dtmP,na)->tPtr == mark )
            {
/*
**           Test Triangle Lines Are Not A Dtm Feature Lines
*/
             if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,np))
               {
                pLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,na) ;
                nLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,na,np) ;
                if( ( pLine && nLine ) || ( ! pLine && ! nLine ))
                  {
                   if( bcdtmList_deleteLineDtmObject(dtmP,sp,np)) goto errexit ;
                   nodeAddrP(dtmP,sp)->hPtr = na ;
                   nodeAddrP(dtmP,na)->hPtr = np ;
                   ++*numRemovedP ;
                   process = 1 ;
                  }
               }
            }
         }
       sp = np ;
      } while ( sp != dtmP->hullPoint ) ;
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Hull Lines With Boundary Point Vertex Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Hull Lines With Boundary Point Vertex Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*--------------------------------------------------------------------+
|                                                                     |
|                                                                     |
|                                                                     |
+--------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_assignDirectionToBoundaryLinesFromTwoHullPointsDtmObject(BC_DTM_OBJ *dtmP,long numTinFeatures,long *numAssignedP)
/*
** This Function Assigns A Direction To A Boundary Line
** If It Is Coincident With A Hull Point
*/
{
 int    ret=0,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,pp=0,fhp,shp,feature ;
 double x,y,area ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assign Direction To Boundary Lines From Two Hull Points") ;
/*
** Initialise
*/
 *numAssignedP = 0 ;
/*
** Scan Boundary Line Features For Hull Coincident Lines
*/
 for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if( ( sp = (long ) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       if( ftableAddrP(dtmP,feature)->dtmUserTag == dtmP->nullUserTag )
         {
/*
**       Write Out Feature Points
*/
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Boundary Line Feature = %6ld ** Direction = %9I64d",feature,ftableAddrP(dtmP,feature)->dtmUserTag) ;
             do
               {
                bcdtmWrite_message(0,0,0,"**** Point[%9ld] fTableP = %9ld ** %12.5lf %12.5lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
                sp = np ;
               } while ( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
             sp = (long) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
            }
/*
**        Scan To First Hull Point
*/
          fhp = dtmP->nullPnt ;
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
             if( nodeAddrP(dtmP,sp)->hPtr != dtmP->nullPnt ) fhp = sp ;
             sp = np ;
            } while ( fhp == dtmP->nullPnt && sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
/*
**        Scan To Second Hull Point
*/
          shp = dtmP->nullPnt ;
          if( fhp != dtmP->nullPnt && sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint )
            {
             do
               {
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
                if( nodeAddrP(dtmP,sp)->hPtr != dtmP->nullPnt ) shp = sp ;
                sp = np ;
               } while ( shp == dtmP->nullPnt && sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
            }
/*
**        Write Out Hull Points
*/
          if( dbg )
            {
             if( fhp != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"fhp = %9ld Fptr = %9ld ** %12.5lf %12.5lf %10.4lf",fhp,nodeAddrP(dtmP,fhp)->hPtr,pointAddrP(dtmP,fhp)->x,pointAddrP(dtmP,fhp)->y,pointAddrP(dtmP,fhp)->z) ;
             else                       bcdtmWrite_message(0,0,0,"fhp = %9ld",fhp ) ;
             if( shp != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"shp = %9ld Fptr = %9ld ** %12.5lf %12.5lf %10.4lf",shp,nodeAddrP(dtmP,shp)->hPtr,pointAddrP(dtmP,shp)->x,pointAddrP(dtmP,shp)->y,pointAddrP(dtmP,shp)->z) ;
             else                       bcdtmWrite_message(0,0,0,"shp = %9ld",shp ) ;
            }
/*
**        If Two Hull Points Found Determine Boundary Line Direction
*/
          if( fhp != dtmP->nullPnt && shp != dtmP->nullPnt )
            {
/*
**           Consecutive Hull Points
*/
             if( nodeAddrP(dtmP,fhp)->hPtr == shp || nodeAddrP(dtmP,shp)->hPtr == fhp )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Consecutive Hull Points") ;
                ftableAddrP(dtmP,feature)->dtmUserTag = 1 ;
                if( nodeAddrP(dtmP,shp)->hPtr == fhp ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                ++*numAssignedP ;
               }
/*
**           Non Consecutive Hull Points
*/
             else
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Non Consecutive Hull Points") ;
/*
**              Calculate Area Between Boundary Line And Tin Hull
*/
                area = 0.0 ;
                sp = fhp ;
                while ( sp != shp )
                  {
                   if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
                   x  = pointAddrP(dtmP,np)->x - pointAddrP(dtmP,sp)->x ;
                   y  = pointAddrP(dtmP,np)->y - pointAddrP(dtmP,sp)->y ;
                   area = area + x * y / 2.0 + x * pointAddrP(dtmP,sp)->y ;
                   pp = sp ;
                   sp = np ;
                  }
                while ( sp != fhp )
                  {
                   if(( pp = bcdtmList_nextClkDtmObject(dtmP,sp,pp)) < 0 ) goto errexit ;
                   while ( nodeAddrP(dtmP,pp)->hPtr != sp && nodeAddrP(dtmP,sp)->hPtr != pp )
                     {
                      if(( pp = bcdtmList_nextClkDtmObject(dtmP,sp,pp)) < 0 ) goto errexit ;
                     }
                   x  = pointAddrP(dtmP,pp)->x - pointAddrP(dtmP,sp)->x ;
                   y  = pointAddrP(dtmP,pp)->y - pointAddrP(dtmP,sp)->y ;
                   area = area + x * y / 2.0 + x * pointAddrP(dtmP,sp)->y ;
                   np = sp ;
                   sp = pp ;
                   pp = np ;
                  }
                if( dbg ) bcdtmWrite_message(0,0,0,"area = %12.4lf",area) ;
/*
**              Assign Direction
*/
                ftableAddrP(dtmP,feature)->dtmUserTag = 1 ;
                if( area > 0.0  ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                ++*numAssignedP ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Hull Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Hull Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*--------------------------------------------------------------------+
|                                                                     |
|                                                                     |
|                                                                     |
+--------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_assignDirectionToBoundaryLinesFromHullPointDtmObject(BC_DTM_OBJ *dtmP,long numTinFeatures,long *numAssignedP)
/*
** This Function Assigns A Direction To A Boundary Line
** If It Is Coincident With A Hull Point
*/
{
 int    ret=0,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,pp,hp,na,nc,nhp,php,feature,onFeature,numNextFeatures,numPriorFeatures ;
 double angHp=0.0,angNp=0.0,angPp=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assign Direction To Boundary Lines From Hull Points") ;
/*
** Initialise
*/
 *numAssignedP = 0 ;
/*
** Scan Boundary Line Features For Hull Coincident Lines
*/
 for( feature = numTinFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if( ( sp = (long) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       if( ftableAddrP(dtmP,feature)->dtmUserTag == dtmP->nullUserTag )
         {
/*
**       Write Out Feature Points
*/
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Boundary Line Feature = %6ld ** Direction = %9I64d",feature,ftableAddrP(dtmP,feature)->dtmUserTag) ;
             do
               {
                bcdtmWrite_message(0,0,0,"**** Point[%9ld] fTableP = %9ld ** %12.5lf %12.5lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
                sp = np ;
               } while ( sp != dtmP->nullPnt && sp != (long)ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
             sp = (long) ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
            }
/*
**        Scan To Hull Point
*/
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np)) goto errexit ;
             if( nodeAddrP(dtmP,sp)->hPtr != dtmP->nullPnt )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Direction To Feature %6ld",feature) ;
                if( dbg == 2 ) bcdtmList_writeCircularListForPointDtmObject(dtmP,sp) ;
                if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,feature,sp,&pp)) goto errexit ;
/*
**              Scan Anti Clockwise About Point Till Next Point Is On Feature
*/
                if( ( hp = bcdtmList_nextAntDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->hPtr)) < 0 ) goto errexit ;
                if( bcdtmList_testForPointInTinFeatureListDtmObject(dtmP,feature,hp,&onFeature)) goto errexit ;
                while ( ! onFeature && hp != nodeAddrP(dtmP,sp)->hPtr )
                  {
                   if( ( hp = bcdtmList_nextAntDtmObject(dtmP,sp,hp)) < 0 ) goto errexit ;
                   if( bcdtmList_testForPointInTinFeatureListDtmObject(dtmP,feature,hp,&onFeature)) goto errexit ;
                  }
/*
**              If Point Is On Feature Assign Direction To Boundary Line
*/
                if( onFeature )
                  {
                   if( dbg == 2 )
                     {
                      bcdtmWrite_message(0,0,0,"hp = %9ld ** %12.5lf %12.5lf %10.4lf",hp,pointAddrP(dtmP,hp)->x,pointAddrP(dtmP,hp)->y,pointAddrP(dtmP,hp)->z) ;
                      bcdtmWrite_message(0,0,0,"sp = %9ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                      if( pp != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pp = %9ld ** %12.5lf %12.5lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
                      else                     bcdtmWrite_message(0,0,0,"pp = %9ld",pp) ;
                      if( np != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"np = %9ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,hp)->y,pointAddrP(dtmP,np)->z) ;
                      else                     bcdtmWrite_message(0,0,0,"np = %9ld",np) ;
                     }
/*
**                 Hull Point Is Between Boundary Line End Points
*/
                   if(  pp != dtmP->nullPnt && np != dtmP->nullPnt )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Hull Point Between Boundary Line End Points") ;
                      ftableAddrP(dtmP,feature)->dtmUserTag = 1 ;
                      if( hp == pp ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                      np = dtmP->nullPnt ;
                      ++*numAssignedP ;
                     }
/*
**                 Hull Point Is A Boundary Line End Point
*/
                   else
                     {
                      nhp = nodeAddrP(dtmP,sp)->hPtr ;
                      if(( php = bcdtmList_nextClkDtmObject(dtmP,sp,nhp)) < 0 ) goto errexit ;
                      if( dbg )
                        {
                         bcdtmWrite_message(0,0,0,"php = %9ld Fptr = %9ld ** %12.5lf %12.5lf %10.4lf",php,nodeAddrP(dtmP,php)->hPtr,pointAddrP(dtmP,php)->x,pointAddrP(dtmP,php)->y,pointAddrP(dtmP,php)->z) ;
                         bcdtmWrite_message(0,0,0,"nhp = %9ld Fptr = %9ld ** %12.5lf %12.5lf %10.4lf",nhp,nodeAddrP(dtmP,nhp)->hPtr,pointAddrP(dtmP,nhp)->x,pointAddrP(dtmP,nhp)->y,pointAddrP(dtmP,nhp)->z) ;
                        }
/*
**                   Count Feature Lines Between Next And Prior Hull Points
*/
                      numNextFeatures  = 0 ;
                      nc = hp ;
                      while ( nc != nhp )
                        {
                         if(( nc = bcdtmList_nextClkDtmObject(dtmP,sp,nc))  < 0 ) goto errexit ;
                         if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,nc)) ++numNextFeatures  ;
                        }
                      numPriorFeatures = 0 ;
                      na = hp ;
                      while ( na != php )
                        {
                         if(( na = bcdtmList_nextAntDtmObject(dtmP,sp,na))  < 0 ) goto errexit ;
                         if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,na)) ++numPriorFeatures  ;
                        }
                      if( dbg )
                        {
                         bcdtmWrite_message(0,0,0,"numNextFeatures  = %4ld",numNextFeatures) ;
                         bcdtmWrite_message(0,0,0,"numPriorFeatures = %4ld",numPriorFeatures) ;
                        }
/*
**                    Cannot Process Boundary Line If there Are Features Eiher Side Of It
*/
                      if( ! numNextFeatures || ! numPriorFeatures )
                        {
/*
**                       Calculate Angles To Hull Lines From Hp
*/
                         if( ! numNextFeatures && ! numPriorFeatures )
                           {
                            angHp = bcdtmMath_getPointAngleDtmObject(dtmP,sp,hp) ;
                            angNp = bcdtmMath_getPointAngleDtmObject(dtmP,sp,nhp) ;
                            angPp = bcdtmMath_getPointAngleDtmObject(dtmP,sp,php) ;
                            if( angHp < angNp ) angHp += DTM_2PYE ;
                            angNp = angHp - angNp ;
                            if( angHp >= DTM_2PYE ) angHp -= DTM_2PYE ;
                            if( angPp < angHp ) angPp += DTM_2PYE ;
                            angPp = angPp - angHp ;
                            if( dbg )
                              {
                               bcdtmWrite_message(0,0,0,"angNp = %12.10lf",angNp) ;
                               bcdtmWrite_message(0,0,0,"angPp = %12.10lf",angPp) ;
                              }
                           }
/*
**                       Hull Point Is Boundary Line Start Point
*/
                         if(  pp == dtmP->nullPnt && np != dtmP->nullPnt )
                           {
                            if( dbg ) bcdtmWrite_message(0,0,0,"Hull Point Is Boundary Line Start Point") ;
                            ftableAddrP(dtmP,feature)->dtmUserTag = 1 ;
                            if( numPriorFeatures || numNextFeatures )
                              {
                               if( numNextFeatures ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                              }
                            else if( angPp < angNp ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                            np = dtmP->nullPnt ;
                            ++*numAssignedP ;
                           }
/*
**                       Hull Point Is Boundary Line End Point
*/
                         if(  pp != dtmP->nullPnt && np == dtmP->nullPnt )
                           {
                            if( dbg ) bcdtmWrite_message(0,0,0,"Hull Point Is Boundary Line End Point") ;
                            ftableAddrP(dtmP,feature)->dtmUserTag = 1 ;
                            if( numPriorFeatures || numNextFeatures )
                              {
                               if( numPriorFeatures ) if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                              }
                            else if( angNp < angPp )if( bcdtmList_reverseDtmTinFeatureDtmObject(dtmP,feature)) goto errexit ;
                            np = dtmP->nullPnt ;
                            ++*numAssignedP ;
                           }
                        }
                     }
                  }
               }
             sp = np ;
            } while ( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Hull Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Direction To Boundary Lines From Hull Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*--------------------------------------------------------------------+
|                                                                     |
|                                                                     |
|                                                                     |
+--------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTin_removeTrianglesExternalToBoundaryLineDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *numRemovedP)
/*
** This Function Triangles External To A Boundary Line
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,np,pp,na,nc,php=0,nhp=0,process ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Triangles External To Boundary Line") ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Only Process For Valid Boundary Line Feature
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 1 )
      {
/*
**    Repeatively Scan Boundary Line Until No External Triangles Are Removed
*/
       process = 1 ;
       while ( process )
         {
          process = 0 ;
          sp = (long) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
          if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&pp) ) goto errexit ;
/*
**        Scan To End Of Boundary Line And Remove External Triangles
*/
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
             if( nodeAddrP(dtmP,sp)->hPtr != dtmP->nullPnt )
               {
/*
**              Remove Next Lines
*/
                if( np != dtmP->nullPnt )
                  {
                   nhp = nodeAddrP(dtmP,sp)->hPtr ;
                   while ( nhp != np )
                     {
                      if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,nhp)) nhp = np ;
                      else
                        {
                         if(( na = bcdtmList_nextAntDtmObject(dtmP,sp,nhp)) < 0 ) goto errexit ;
                         if(nodeAddrP(dtmP,na)->hPtr != dtmP->nullPnt ) nhp = np ;
                         else
                           {
                            if( dbg )
                              {
                               bcdtmWrite_message(0,0,0,"Removing Hull Line P1 = %6ld P2 = %6ld",sp,php) ;
                               bcdtmWrite_message(0,0,0,"P1 = %6ld fTableP = %9ld ** %12.6lf %12.6lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                               bcdtmWrite_message(0,0,0,"P2 = %6ld fTableP = %9ld ** %12.6lf %12.6lf %10.4lf",php,nodeAddrP(dtmP,php)->hPtr,pointAddrP(dtmP,php)->x,pointAddrP(dtmP,php)->y,pointAddrP(dtmP,php)->z) ;
                              }
                            bcdtmList_deleteLineDtmObject(dtmP,sp,nhp) ;
                            nodeAddrP(dtmP,sp)->hPtr = na ;
                            nodeAddrP(dtmP,na)->hPtr = nhp ;
                            nhp = na ;
                            ++*numRemovedP ;
                            process = 1 ;
                           }
                        }
                     }
                  }
/*
**              Remove Previous Lines
*/
                if( pp != dtmP->nullPnt )
                  {
                   if(( php = bcdtmList_nextClkDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->hPtr)) < 0 ) goto errexit ;
                   while ( php != pp )
                     {
                      if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,php) ) php = pp ;
                      else
                        {
                         if(( nc = bcdtmList_nextClkDtmObject(dtmP,sp,php)) < 0 ) goto errexit ;
                         if  (nodeAddrP(dtmP,nc)->hPtr != dtmP->nullPnt ) php = pp ;
                         else
                           {
                            if( dbg )
                              {
                               bcdtmWrite_message(0,0,0,"Removing Hull Line P1 = %6ld P2 = %6ld",sp,nhp) ;
                               bcdtmWrite_message(0,0,0,"P1 = %6ld fTableP = %9ld ** %12.6lf %12.6lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                               bcdtmWrite_message(0,0,0,"P2 = %6ld fTableP = %9ld ** %12.6lf %12.6lf %10.4lf",nhp,nodeAddrP(dtmP,nhp)->hPtr,pointAddrP(dtmP,nhp)->x,pointAddrP(dtmP,nhp)->y,pointAddrP(dtmP,nhp)->z) ;
                              }
                            bcdtmList_deleteLineDtmObject(dtmP,sp,php) ;
                            nodeAddrP(dtmP,php)->hPtr = nc ;
                            nodeAddrP(dtmP,nc)->hPtr  = sp ;
                            php = nc ;
                            ++*numRemovedP ;
                            process = 1 ;
                           }
                        }
                     }
                  }
               }
/*
**           Set For Next Feature Point
*/
             pp = sp ;
             sp = np ;
            } while ( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
         }
/*
**     Mark Completed Boundary Lines
*/
       ftableAddrP(dtmP,dtmFeature)->dtmUserTag = 2  ;
       sp = (long)ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       do
         {
          if( nodeAddrP(dtmP,sp)->hPtr == dtmP->nullPnt ) ftableAddrP(dtmP,dtmFeature)->dtmUserTag = 1 ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
          sp = np ;
         } while  ( sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint && ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 2 ) ;
      }
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Triangles External To Boundary Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Triangles External To Boundary Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTin_markInternalVoidPointsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Functions Marks Void Points In The Tin
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  islandFeature,voidFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points") ;
/*
** Scan Island Features And Check That They Have A Surrounding Void
*/
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Island,TRUE,&dtmFeatureP,&islandFeature) ;
 while( dtmFeatureP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Surrounding Void For Island Feature %6ld",islandFeature) ;
    if( bcdtmList_getVoidExternalToIslandDtmObject(dtmP,islandFeature,&voidFeature)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Surrounding Void = %8ld",voidFeature) ;
/*
**  If No Surrounding Void Set Island Feature To A Polygon Feature
*/
//    if( voidFeature == dtmP->nullPnt ) dtmFeatureP->dtmFeatureType = DTMFeatureType::Region ;
/*
**  Get Next Island Feature
*/
    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Island,FALSE,&dtmFeatureP,&islandFeature) ;
   }
/*
** Mark Internal Void Points
*/
 if( bcdtmMark_voidPointsDtmObject(dtmP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Error") ;
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
BENTLEYDTM_Public int bcdtmTin_clipDtmFeaturesToVoidsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Clips DTM Features To Void Boundaries
*/
{
 int  ret=DTM_SUCCESS ;
 long dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP  ;
/*
** Scan Features And Clip To Void Boundaries
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine ))
      {
       if( bcdtmTin_clipVoidLinesFromDtmFeatureDtmObject(dtmP,dtmFeature)) goto errexit ;
      }
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
BENTLEYDTM_Public int bcdtmTin_clipVoidLinesFromDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature )
/*
** This Function Clips Void Lines From DTM Features
** Function Updated 29 May 2003 RobC
*/
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long           sp,np,fsp,spnt,tptr,process,voidLine;
 DTMFeatureType dtmFeatureType;
 DPoint3d            *featurePtsP=NULL ;
 DTMUserTag   userTag ;
 DTMFeatureId userFeatureId ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Dtm Feature %6ld",dtmFeature) ;
/*
** Initialise
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures  ) goto cleanup ;
 if( ( spnt = (long) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint) == dtmP->nullPnt ) goto cleanup ;
/*
** Set Feature Parameters
*/
 userTag        = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 userFeatureId       = ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
 dtmFeatureType = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType  ;
/*
** Scan For Feature Void Lines
*/
 sp = (long) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
    voidLine= 0 ;
    if( np != dtmP->nullPnt )
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine) )goto errexit  ;
      }
    if( ! voidLine) sp = np ;
   } while ( sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint && sp != dtmP->nullPnt && ! voidLine) ;
/*
** If Void Line - Delete Existing Feature And Insert Non Voids Segments Of Feature
*/
 if( voidLine)
   {
/*
** Write Void Line
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Void Line Detected ** dtmFeature = %6ld sp = %6ld np = %6ld",dtmFeature,sp,np) ;
/*
**  Copy DTM Features To Tptr Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature To Tptr Array") ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&spnt)) goto errexit  ;
    if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,spnt)) goto errexit  ;
    if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,spnt) ;
/*
**  Add DTM Feature To Roll Back DTM
*/
    if( bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes) && dtmP->extended && dtmP->extended->rollBackInfoP && dtmP->extended->rollBackInfoP->rollBackDtmP != NULL )
      {
      dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
      bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId);
      }
/*
** Delete Feature From Tin Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Feature") ;
    if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit  ;
/*
** Scan And Insert New Features
*/
    sp = spnt ;
    process = 1 ;
    while( process )
      {
/*
**     Scan To First Non Void Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To First Non Void Line") ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
       do
         {
          if( np != dtmP->nullPnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"sp = %6ld np = %6ld",sp,np) ;
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine)) goto errexit  ;
             if( voidLine) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
            }
         } while ( np != spnt && np != dtmP->nullPnt && voidLine) ;
/*
**     Scan To Next Void Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Next Void Line") ;
       if( np == spnt || np == dtmP->nullPnt ) process = 0 ;
       else
         {
          fsp = sp ;
          np = nodeAddrP(dtmP,sp)->tPtr ;
          do
            {
             if( np != dtmP->nullPnt )
               {
                if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine)) goto errexit  ;
                if( ! voidLine) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
               }
            } while ( np != spnt && np != dtmP->nullPnt &&  voidLine) ;
          tptr = nodeAddrP(dtmP,sp)->tPtr ;
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
/*
**        Store Dtm Feature
*/
          if( nodeAddrP(dtmP,fsp)->tPtr != dtmP->nullPnt )
            {
             if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,userTag,userFeatureId,fsp,1)) goto errexit  ;
            }
/*
**        Get Next Non Void Segment
*/
          nodeAddrP(dtmP,sp)->tPtr = tptr ;
         }
      }
/*
** Null Out Sptr And Tptr List
*/
    sp = spnt ;
    do
      {
       np = nodeAddrP(dtmP,sp)->sPtr ;
       nodeAddrP(dtmP,sp)->tPtr = nodeAddrP(dtmP,sp)->sPtr = dtmP->nullPnt ;
       sp = np ;
      } while ( sp != spnt && sp != dtmP->nullPnt ) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Function Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Dtm Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_EXPORT int bcdtmTin_maxsideRemoveExternalTrianglesDtmObject(BC_DTM_OBJ *dtmP,double maxSide)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Max Side Removing External Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxSide = %8.3lf",maxSide) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Remove MaxSide Triangles
*/
 if( bcdtmTin_removeExternalMaxSideTrianglesDtmObject(dtmP,maxSide)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Function Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Max Side Removing External Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Max Side Removing External Triangles Error") ;
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
#ifndef notdef
typedef PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> DtmTinPointArray;

BENTLEYDTM_Public int bcdtmTin_resortTinStructureDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Resorts the Tin Data Structures after the Insertion of Unsorted Points
**
**  2004/01/03  Rob Cormack  Rob.Cormack@Bentley.com
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     node,fTable,fList,cList ;
 long     hullFeature ;
// long     ofs,ofs1,ofs2;
 LongArray::iterator srP;
 DTM_TIN_NODE      *nodeP;
 DTM_CIR_LIST      *cListP ;
 BC_DTM_FEATURE    *fTableP ;
 DTM_FEATURE_LIST  *fListP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resorting Tin ** dtmP->numSortedPoints = %8ld dtmP->numPoints = %8ld",dtmP->numSortedPoints,dtmP->numPoints) ;
/*
** Set Number Of Sorted Points
*/
 if( bcdtmList_setNumberOfSortedPointsDtmObject(dtmP)) goto errexit ;
/*
** Only Resort If Additional Tin Points Have Been Inserted
*/
 if( dtmP->numSortedPoints <= 0 ) dtmP->numSortedPoints = 1 ;
 if( dtmP->numSortedPoints < dtmP->numPoints )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
     PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> pointsArray(dtmP->pointsPP, dtmP->numPoints, dtmP->numPointPartitions, dtmP->pointPartitionSize);
     XYPointArraySort<DTM_TIN_POINT, DtmTinPointArray > sorter;

    sorter.DoResort(pointsArray,dtmP->numSortedPoints, dtmP->numPoints-dtmP->numSortedPoints);
    LongArray& sortP = sorter.GetSortP();
    LongArray& tempP = sorter.GetTempP();
/*
**  Adjust Feature Table First Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature First Point Values") ;
    hullFeature = dtmP->nullPnt ;
    for( fTable = 0 ; fTable < dtmP->numFeatures ; ++fTable )
      {
       fTableP = ftableAddrP(dtmP,fTable) ;
       if( fTableP->dtmFeatureState == DTMFeatureState::Tin )
         {
          if( fTableP->dtmFeatureType == DTMFeatureType::Hull ) hullFeature = fTable ;
          else fTableP->dtmFeaturePts.firstPoint = *(tempP+(long)fTableP->dtmFeaturePts.firstPoint) ;
         }
      }
/*
**  Adjust Feature List Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature List Point Numbers") ;
    for( fList = 0 ; fList < dtmP->numFlist ; ++fList )
      {
       fListP = flistAddrP(dtmP,fList) ;
       if( fListP->nextPnt != dtmP->nullPnt ) fListP->nextPnt = *(tempP+fListP->nextPnt) ;
      }
/*
**  Adjust Node Hull Pointers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Hull Node Pointers") ;
    for( node = 0 ; node < dtmP->numPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->hPtr != dtmP->nullPnt ) nodeP->hPtr = *(tempP+nodeP->hPtr) ;
      }
/*
**  Adjust Circular List Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Circular List Point Numbers") ;
    for( cList = 0 ; cList < dtmP->cListPtr ; ++cList )
      {
       cListP = clistAddrP(dtmP,cList) ;
       if( cListP->pntNum >= 0 && cListP->pntNum < dtmP->numPoints ) cListP->pntNum = *(tempP+cListP->pntNum) ;
      }
/*
**  Reset Dtm Pointer To Tin Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Hull Pointers") ;
    if( dtmP->hullPoint >= 0 && dtmP->hullPoint < dtmP->numPoints )
      {
       dtmP->hullPoint    = *(tempP+dtmP->hullPoint) ;
       dtmP->nextHullPoint  = nodeAddrP(dtmP,dtmP->hullPoint)->hPtr ;
      }
    else
      {
       dtmP->hullPoint      = dtmP->nullPnt ;
       dtmP->nextHullPoint  = dtmP->nullPnt ;
      }
/*
**  Reset First Point For Hull Feature
*/
    if( hullFeature != dtmP->nullPnt ) ftableAddrP(dtmP,hullFeature)->dtmFeaturePts.firstPoint = 0 ;
/*
**  Set Number Of Tin Points For Binary Searching
*/
    dtmP->numSortedPoints = dtmP->numPoints ;
/*
**  Place Data In Sort Order
*/
    DTM_TIN_NODE tempNode;
    LongArray::iterator srP;
    int ofs;

    for( srP = sortP.start() , ofs = 0 ; ofs < dtmP->numPoints ; ++srP , ++ofs )
      {
      tempNode = *nodeAddrP(dtmP,ofs) ;
      *nodeAddrP(dtmP,ofs)  = *nodeAddrP(dtmP,*srP) ;
      *nodeAddrP(dtmP,*srP) = tempNode ;
      *(sortP+*(tempP+ofs)) = *srP ;
      *(tempP+*srP) = *(tempP+ofs) ;
      }
     }
/*
** Check Tin Points Are Sorted
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Points Sort Order") ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,1) )
      {
       bcdtmWrite_message(2,0,0,"Dtm Sort Order Invalid") ;
       goto errexit ;
      }
   }
/*
**  Clean Up
*/
 cleanup :
/*
** Job ComPleted
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_checkForTriangulationTermination
(
 BC_DTM_OBJ  *dtmP
)
{
 if( dtmP->extended != NULL && dtmP->extended->triangulationCheckStopCallBackP != NULL )
   {
    if( dtmP->extended->triangulationCheckStopCallBackP(DTMFeatureType::CheckStop)) return(1) ;
   }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTin_setTriangulationCheckStopCallBackFunction
(
 BC_DTM_OBJ  *dtmP,
 int         (*checkStopCallBackP) (DTMFeatureType dtmFeatureType)
)
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ_EXTENDED *dtmObjectExtendedP=NULL ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Extension Exists
*/
 dtmObjectExtendedP = ( BC_DTM_OBJ_EXTENDED * ) dtmP->extended ;
 if( dtmObjectExtendedP == NULL )
   {
    bcdtmObject_createDTMExtended(&dtmObjectExtendedP);
    dtmP->extended = dtmObjectExtendedP ;
    if( dtmObjectExtendedP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
   }
/*
**  Set Call Back Function Pointer
*/
 dtmP->extended->triangulationCheckStopCallBackP = checkStopCallBackP ;
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTin_clearTriangulationCheckStopCallBackFunction
(
 BC_DTM_OBJ  *dtmP
)
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Clear Check Stop Call Back
*/
 if( dtmP->extended != NULL )
   {
    dtmP->extended->triangulationCheckStopCallBackP = NULL ;
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
int bcdtmTin_insertDtmFeatureTypeIntoDtmObjectNew(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType)
/*
** This Function Inserts Dtm Features Into A Dtm Object
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     pnt,closeFlag,firstPnt,startPnt,nextPnt,insertError,dtmFeatureNum,dtmFeatureNum2,flPtr,numPriorPts;
 long     *tempOffsetP=NULL,drapeOption,insertOption,internalPoint,validateResult,lastPnt,priorLastPnt,featureRolledBack ;
 long     numFeatures = 0, numFeaturesError = 0, numFeaturesInserted = 0, numHullPts, numDrapeVoidPts, featureNum;
 DTMFeatureType insFeatureType;
 char     dtmFeatureTypeName[100] ;
 DPoint3d *hullPtsP=NULL,*featPtsP=NULL,*drapeVoidPtsP=NULL ;
 DTMMemPnt featPtsPI = 0;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_FEATURE_LIST *flistP ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ) goto errexit ;
    bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Object %p",dtmFeatureTypeName,dtmP) ;
   }
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Tin Invalid At Entry") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin Valid") ;
/*
**  Check Point Offsets
*/
    bcdtmWrite_message(0,0,0,"Checking For Point Offset Range Errors") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
   }
/*
** Count Number Of Features
*/
 if( dbg == 1 )
   {
    if( bcdtmTin_countNumberOfDtmFeatureTypeOccurrencesDtmObject(dtmP,dtmFeatureType,&numFeatures)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Number Of %s Features = %8ld",dtmFeatureTypeName,numFeatures) ;
   }
/*
** Set Drape And Insert Options
**
** drapeOption  = 1   Insert As Drape Line
**              = 2   Insert As Break Line
** insertOption = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
*/
 drapeOption   = 1 ;
 insertOption  = 2 ;
 if( dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureType == DTMFeatureType::SoftBreakline ) insertOption = 1 ;
 if( dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureType == DTMFeatureType::SoftBreakline || dtmFeatureType == DTMFeatureType::ContourLine || dtmFeatureType == DTMFeatureType::BreakVoid ) drapeOption = 2 ;
/*
** Etract Tin Hull For Drape Voids
*/
 if( dtmFeatureType == DTMFeatureType::DrapeVoid )
   {
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin Hull Points = %6ld",numHullPts) ;
   }
/*
** Scan and Insert Feature Lines
*/
 featureNum =0 ;
 numFeaturesError   = 0 ;
 bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,TRUE,&dtmFeatureP,&dtmFeatureNum) ;
 while( dtmFeatureP != NULL )
   {
    insertError = 0 ;
    firstPnt = dtmP->nullPnt ;
    ++featureNum ;
    numPriorPts = dtmP->numPoints ;
    if( dbg ==  1 ) bcdtmWrite_message(0,0,0,"Inserting Feature %8ld",featureNum) ;
/*
**  Validate Polygonal Dtm Features
*/
    switch ( dtmFeatureType )
      {
       case DTMFeatureType::Void       :
       case DTMFeatureType::BreakVoid :
       case DTMFeatureType::Hole       :
       case DTMFeatureType::Island     :
       case DTMFeatureType::Region     :

         if( dbg ) bcdtmWrite_message(0,0,0,"Validating Feature Point Offsets") ;
         if( bcdtmTin_validatePolygonalOffsetFeatureDtmObject(dtmP,dtmFeatureP,&validateResult)) goto errexit ;
         if( validateResult == FALSE )
           {
            insertError = 1 ;
            if( dbg )
              {
               if( dtmFeatureP->numDtmFeaturePts < 3 ) bcdtmWrite_message(0,0,0,"Feature Has Less Than 3 Points") ;
               if( bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[0] != bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[dtmFeatureP->numDtmFeaturePts-1] )bcdtmWrite_message(0,0,0,"Feature Does Not Close") ;
              }
           }
       break ;

       case DTMFeatureType::DrapeVoid :
         if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Void") ;
         if( bcdtmTin_validateDrapeVoidDtmObject(dtmP,dtmFeatureP,hullPtsP,numHullPts,&validateResult,&drapeVoidPtsP,&numDrapeVoidPts)) goto errexit ;
         if( validateResult == FALSE )
           {
            insertError = 1 ;
            if( drapeVoidPtsP != NULL ) { free(drapeVoidPtsP) ; drapeVoidPtsP = NULL ; }
           }
       break   ;

       default :
       break   ;
      } ;
/*
**  Create Tptr List For Feature
*/
    if( ! insertError )
      {
/*
**     Get Point Offsets
*/
//       offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
/*
**     Insert Group Spots
*/
       if( dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Group Spot Feature") ;
/*
**        Check Point Offsets
*/
          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          firstPnt = startPnt = offsetP[0] ;
          if( dtmFeatureP->numDtmFeaturePts == 1 )
            {
             nodeAddrP(dtmP,startPnt)->tPtr = dtmP->nullPnt ;
            }
          else
            {
             for( pnt = 1 ; pnt < dtmFeatureP->numDtmFeaturePts && ! insertError ; ++pnt )
               {
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"startPnt = %8ld ** startPnt->tPtr = %8ld",startPnt,nodeAddrP(dtmP,startPnt)->tPtr) ;
                   bcdtmWrite_message(0,0,0,"nextPnt  = %8ld ** nextPnt->tPtr  = %8ld",offsetP[pnt],nodeAddrP(dtmP,offsetP[pnt])->tPtr) ;
                  }
                if( nodeAddrP(dtmP,startPnt)->tPtr != dtmP->nullPnt || nodeAddrP(dtmP,offsetP[pnt])->tPtr != dtmP->nullPnt ) insertError = 1 ;
                else
                  {
                   nodeAddrP(dtmP,startPnt)->tPtr = offsetP[pnt] ;
                   startPnt = offsetP[pnt] ;
                  }
               }
            }
        }
/*
**     Insert Drape Voids
*/
       else if( dtmFeatureType == DTMFeatureType::DrapeVoid )
         {
          internalPoint = 1 ;
          if( ( insertError = bcdtmInsert_internalStringIntoDtmObject(dtmP,drapeOption,internalPoint,drapeVoidPtsP,numDrapeVoidPts,&firstPnt)) == 1 ) goto errexit  ;
          if( drapeVoidPtsP != NULL ) { free(drapeVoidPtsP) ; drapeVoidPtsP = NULL ; }
         }
/*
**     Insert All Other Dtm Feature Types
*/
       else
         {
/*
**        Write Out Feature Points
*/
          numPriorPts = dtmP->numPoints ;
          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          if( dbg == 2 )
            {
             for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
               {
                startPnt = offsetP[pnt] ;
                bcdtmWrite_message(0,0,0,"Segment[%6ld] ** Pnt = %8ld ** %12.4lf %12.4lf",pnt+1,startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y) ;
               }
            }

//        Insert Lines Between Feature Points

          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Feature Lines") ;
          firstPnt = dtmP->nullPnt ;
          featureRolledBack = FALSE ;
          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts - 1 && ! insertError ; ++pnt )
            {
             startPnt = offsetP[pnt] ;
             nextPnt  = offsetP[pnt+1] ;
             if( firstPnt == dtmP->nullPnt ) firstPnt = startPnt ;
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Segment %6ld of %6ld From Point %8ld To %8ld ** %12.4lf %12.4lf ** %12.4lf %12.4lf",pnt+1,dtmFeatureP->numDtmFeaturePts-1,startPnt,nextPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y) ;

//           Insert Feature Line Segment Into Tin

             if( startPnt != nextPnt )
               {
                if(( insertError = bcdtmInsert_lineBetweenPointsDtmObject(dtmP,startPnt,nextPnt,drapeOption,insertOption)) == 1 ) goto errexit ;

                //  If Insert Error Then Break Feature At Knot Point

                if( insertError == 10 || insertError == 12 )
                  {

                   //  Cannot Split DTM Polygonal Features

                   if( dtmFeatureType != DTMFeatureType::Void && dtmFeatureType != DTMFeatureType::BreakVoid && dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureType != DTMFeatureType::Hole && dtmFeatureType != DTMFeatureType::Island )
                     {

                      // Add Feature To Roll Back

                      if (dtmP->extended && dtmP->extended->rollBackInfoP && bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                      featureRolledBack = TRUE  ;

                      // Knot At Coincident Feature Point

                      if( insertError == 10 || insertError == 12 )
                        {
                         if( bcdtmList_getLastPointInTptrListDtmObject(dtmP,startPnt,&lastPnt,&priorLastPnt)) goto errexit ;
                         if( insertError == 10 ) nodeAddrP(dtmP,priorLastPnt)->tPtr = dtmP->nullPnt ;
                         if( bcdtmInsert_addToFeatureTableDtmObject(dtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,firstPnt,&dtmFeatureNum2)) goto errexit  ;

                         // Reset Variables

                         dtmFeatureP = ftableAddrP(dtmP,dtmFeatureNum);
                         if( insertError == 10 ) offsetP[pnt] = priorLastPnt ;
                         if( insertError == 12 ) offsetP[pnt] = lastPnt ;
                         firstPnt = dtmP->nullPnt ;
                         insertError = 0;
                         --pnt ;
                        }
                     }
                   if( dbg && insertError ) bcdtmWrite_message(0,0,0,"Insert Error %2ld Processing Feature %8ld",insertError,featureNum) ;
                  }
               }
            }

          // Check For Topology Insert Error

          if( ! insertError )
            {
             if( firstPnt == dtmP->nullPnt ) insertError = 1 ;
             else if ( nodeAddrP(dtmP,firstPnt)->tPtr == dtmP->nullPnt ) insertError = 1 ;
            }
         }
      }

//  Check That Single Line Feature Does Not Loop Back On Itself

    if( ! insertError && dtmFeatureP->numDtmFeaturePts > 1 )
      {
       if( nodeAddrP(dtmP,nodeAddrP(dtmP,firstPnt)->tPtr)->tPtr == firstPnt )
         {
          nodeAddrP(dtmP,nodeAddrP(dtmP,firstPnt)->tPtr)->tPtr = dtmP->nullPnt ;
         }
      }

//  Check Connectivity Of Inserted Feature

    if( ! insertError && dtmFeatureType != DTMFeatureType::GroupSpots )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Feature Connectivity") ;
       if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,firstPnt,0))
         {
          insertError = 1 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Tptr List Connectivity Error ** Feature = %8ld",featureNum) ;
         }
      }

//  Check Voids And Islands Do Not Intersect Inserted Voids And Islands

    if( ! insertError && ( dtmFeatureType == DTMFeatureType::Void || dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureType == DTMFeatureType::Hole || dtmFeatureType == DTMFeatureType::Island ) )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersected Voids Or Islands") ;
       if( bcdtmTin_checkForIntersectionWithInsertedVoidsAndIslandsDtmObject(dtmP,firstPnt,&closeFlag,&insertError)) goto errexit ;
       if( dbg && insertError ) bcdtmWrite_message(0,0,0,"Intersecting Voids Or Islands") ;
       if( closeFlag == FALSE ) insertError = TRUE ;
      }

//  Insert Feature Into Tin

    if( ! insertError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Feature Into Tin") ;

//     Save Point Offsets

       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       tempOffsetP = ( long *) malloc(dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
       if( tempOffsetP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       memcpy(tempOffsetP,offsetP,dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;

//     Add Feature To Tin

       insFeatureType = dtmFeatureType ;
       if( dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureType == DTMFeatureType::DrapeVoid ) insFeatureType = DTMFeatureType::Void ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,dtmFeatureP,dtmFeatureNum,insFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,firstPnt,1)) goto errexit ;
       if( dbg )
         {
          if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(insFeatureType,dtmFeatureTypeName) ) goto errexit ;
          bcdtmWrite_message(0,0,0,"Inserting %s Into Tin Completed",dtmFeatureTypeName) ;
         }
       ++numFeaturesInserted ;

//     Scan Feature And Set Point Types

       if( dtmFeatureType != DTMFeatureType::DrapeVoid )
         {

//        Marked All Points as Inserted.

          bcdtmList_setPntTypeForForDtmTinFeatureDtmObject(dtmP,dtmFeatureNum,2) ;

//        Unmarked All Real Points.

          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             flPtr = nodeAddrP(dtmP,tempOffsetP[pnt])->fPtr ;
             while( flPtr != dtmP->nullPtr )
               {
                flistP = flistAddrP(dtmP,flPtr) ;
                if( flistP->dtmFeature == dtmFeatureNum )
                  {
                   flistP->pntType = 1 ;
                   flPtr = dtmP->nullPtr ;
                  }
                else flPtr = flistP->nextPtr ;
               }
            }
         }

//     Free Temporary Offsets Memory

       if( tempOffsetP != NULL ) { free( tempOffsetP ) ; tempOffsetP = NULL ; }
      }

//  Set Feature State To Tin Insert Error

    else
      {
       ++numFeaturesError ;
       dtmFeatureP->dtmFeatureState = DTMFeatureState::TinError ;
       if( firstPnt != dtmP->nullPnt ) bcdtmList_nullTptrListDtmObject(dtmP,firstPnt) ;

//     Copy Point Offset Array To Point Array

       if( dtmFeatureType != DTMFeatureType::DrapeVoid )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Feature Insert Error") ;
          featPtsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d));
          featPtsP  = bcdtmMemory_getPointerP3D(dtmP, featPtsPI);
          if( featPtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }

          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          for( pnt = 0 ; pnt < dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             *(featPtsP+pnt) = *(( DPoint3d * ) pointAddrP(dtmP,offsetP[pnt])) ;
            }
          bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
          dtmFeatureP->dtmFeaturePts.pointsPI = featPtsPI ;
          featPtsP = NULL ;
         }
      }

//  Get Next Feature

    bcdtmTin_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,FALSE,&dtmFeatureP,&dtmFeatureNum) ;
   }

// Report And Set To Null Non Null Tptr Values

 bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;

//  Print Feature Insertion Insertion Statistics

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Features Processed       = %6ld",numFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Features With Errors     = %6ld",numFeaturesError) ;
    bcdtmWrite_message(0,0,0,"Number Of Features Inserted        = %6ld",numFeaturesInserted) ;
   }

// Cleanup

 cleanup :
 if( hullPtsP      != NULL ) free(hullPtsP) ;
 if( featPtsP      != NULL ) bcdtmMemory_free(dtmP, featPtsPI) ;
 if( drapeVoidPtsP != NULL ) free(drapeVoidPtsP) ;
 if( tempOffsetP   != NULL ) { free( tempOffsetP ) ; tempOffsetP = NULL ; }

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Completed",dtmFeatureTypeName) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting %s Features Into Dtm Error",dtmFeatureTypeName) ;
 return(ret) ;

// Error Exit

 errexit :
 if( dbg ) bcdtmWrite_message(0,0,0,"Error Detected Inserting Feature %8ld",featureNum) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
