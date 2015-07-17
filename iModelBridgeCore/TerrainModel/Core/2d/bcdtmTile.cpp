/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmTile.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
#include <thread>

static unsigned long TileRandomSeed=0 ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
void bcdtmTile_multiThreadTileWorkerDtmObject (DTM_MULTI_THREAD_TILE *tileParametersP)
/*
** This function uses a divide and conquer strategy to tile DTM Points
**
** Author : Rob Cormack  December 2009  Rob.Cormack@Bentley.com
**
*/
    {
    int    dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    long   startTime, numPointTiles = 0;
    DTM_POINT_TILE *pointTilesP = NULL;
    /*
    ** Initialise variables
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** startPoint = %8ld numPoints = %8ld", tileParametersP->thread, tileParametersP->startPoint, tileParametersP->numPoints);
    /*
    ** Sort Points Into Tiles
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tiling %9ld Dtm Points", tileParametersP->thread, tileParametersP->numPoints);
    if (tileParametersP->tagP == NULL)
        {
        if (bcdtmTile_pointsInDtmObject (tileParametersP->dtmP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    else
        {
        if (bcdtmTile_taggedPointsInDtmObject (tileParametersP->dtmP, tileParametersP->tagP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tile Time = %8.3lf Seconds", tileParametersP->thread, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Set Return values
    */
    tileParametersP->pointTilesP = pointTilesP;
    tileParametersP->numPointTiles = numPointTiles;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTile_pointsDtmObject
(
 BC_DTM_OBJ *dtmP,                       // ==> Pointer to DTM Object
 long       *tagP,                       // ==> Pointer To User Tag. Can Be set To Null 
 long       startPoint,                  // ==> Offset Of Start Point In DTM
 long       numPoints,                   // ==> Number Of Points To Tile
 long       minTilePoints,               // ==> Minimum Point Size Of Tiles
 DTM_POINT_TILE **pointTilesPP,          // <== Pointer to Tile Offsets Array
 long           *numPointTilesP          // <== Number Of Tiles
 )
/*
** This Function Tiles A Dtm Object By Alternate Cuts on The x and y axis
**
** Author : Rob Cormack  December 2009  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   n,startTime ;
 long   numThreadPoints=0,numThreadArrayPoints[DTM_MAX_PROCESSORS] ;
 std::thread thread[DTM_MAX_PROCESSORS] ;
 DTM_MULTI_THREAD_TILE multiThread[DTM_MAX_PROCESSORS] ;
 DTM_POINT_TILE *tileP,*tile1P ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %8ld",*numPointTilesP) ;
   }
/*
** Validate
*/
 startTime = bcdtmClock() ;
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For DTM In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(0,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   } 
/*
** Single Thread Tile
*/
 if( numPoints <= 1024 || minTilePoints > numPoints / 2 || minTilePoints > numPoints / DTM_NUM_PROCESSORS )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Tiling Dtm") ; 
    if( tagP == NULL )
      {
       if( bcdtmTile_pointsInDtmObject(dtmP,0,dtmP->numPoints,minTilePoints,pointTilesPP,numPointTilesP) ) goto errexit ; 
      }
    else
      {
       if( bcdtmTile_taggedPointsInDtmObject(dtmP,tagP,0,dtmP->numPoints,minTilePoints,pointTilesPP,numPointTilesP) ) goto errexit ; 
      } 
   }   
/*
** Multi Thread Tile
*/
 else
   {    
    if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Tiling Dtm") ;
/*
**  Determine Number Of Points Per Thread
*/
    numThreadPoints = dtmP->numPoints / DTM_NUM_PROCESSORS + 1 ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS == 0 ) --numThreadPoints ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) numThreadArrayPoints[n] =  numThreadPoints ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS != 0 ) numThreadArrayPoints[DTM_NUM_PROCESSORS-1] = dtmP->numPoints - (DTM_NUM_PROCESSORS-1) * numThreadPoints ;
    if( dbg == 2 ) for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n  ) bcdtmWrite_message(0,0,0,"Thread[%2ld] ** numPoints = %6ld",n,numThreadArrayPoints[n]) ;
/*
**  Initialise Multi Thread Tile Array
*/
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       multiThread[n].thread          = n ;
       multiThread[n].dtmP            = dtmP ;
       multiThread[n].tagP            = tagP ;
       multiThread[n].startPoint      = startPoint ;
       multiThread[n].numPoints       = numThreadArrayPoints[n] ;
       multiThread[n].minTilePts      = minTilePoints ;
       multiThread[n].pointTilesP     = NULL  ;
       multiThread[n].numPointTiles   = 0  ;
       startPoint = startPoint + numThreadArrayPoints[n] ;
      }
/*
**  Create Threads To Tile Dtm Object
*/
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
      thread[n] = std::thread (bcdtmTile_multiThreadTileWorkerDtmObject, &multiThread[n]);
      // CreateThread (NULL,0,(LPTHREAD_START_ROUTINE) bcdtmTile_multiThreadTileWorkerDtmObject,&multiThread[n],0,&threadId[n]) ;
      }
/*
**  Wait For All Threads To Complete
*/
    for (n = 0; n < DTM_NUM_PROCESSORS; ++n)
        thread[n].join ();
    //WaitForMultipleObjects (DTM_NUM_PROCESSORS, thread, 1, INFINITE);
    //for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) CloseHandle(thread[n]) ;
/*
**  Count Total Number Of Tiles
*/
    *numPointTilesP = 0 ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"pointTilesP = %p ** numPointTiles = %8ld",multiThread[n].pointTilesP,multiThread[n].numPointTiles) ;
       *numPointTilesP = *numPointTilesP + multiThread[n].numPointTiles ;
      }
/*
**  Allocate Memory
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc (*numPointTilesP*sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }  
/*
**  Copy Tile Offset
*/
    tile1P = *pointTilesPP ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       for( tileP = multiThread[n].pointTilesP ; tileP < multiThread[n].pointTilesP + multiThread[n].numPointTiles ; ++tileP )
         {
          *tile1P = *tileP ;
          ++tile1P ;
         } 
/*
**     Free Memory
*/
       free(multiThread[n].pointTilesP) ;
       multiThread[n].pointTilesP = NULL ;
      }  
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Multi Thread Sort = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Public unsigned long bcdtmTile_randomNumber(unsigned long size)
/*
** This Function Generates A Random Number Between 0 and size
*/
{
// TileRandomSeed = (TileRandomSeed * 1366l + 150889l) % 714025l;
// return ( TileRandomSeed / (714025l / size + 1)) ;
 return(size >> 1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTile_pointsInDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE **pointTilesPP,long *numPointTilesP) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lastStartPoint=-1,memPointTiles=0,axis ;
 long   numTilePoints,numPointLevelTiles ;
 double xRange,yRange,zRange ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tiling Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %p",*numPointTilesP) ;
   }
/*
** Initialise
*/
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Create Two Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Calculate Number Of Point Tiles
*/
    numTilePoints = numPoints ;
    memPointTiles = 1 ;
    numPointLevelTiles = 1 ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
    while( numTilePoints > minTilePoints * 2 )
      {
       memPointTiles = memPointTiles * 2 ;
       numTilePoints = numTilePoints / 2 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
      }  
/*
**  Allocate Memory For Point Tiles
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( memPointTiles * sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Get x and y Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,startPoint,numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"xrange = %12.4lf yRange = %12.4lf zRange = %12.4lf",xRange,yRange,zRange) ;
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Tile By Divide And Conquer Method
*/
    if( bcdtmTile_sortAndTilePointsDtmObject(dtmP,startPoint,numPoints,minTilePoints,axis,*pointTilesPP,numPointTilesP,&lastStartPoint,memPointTiles) ) goto errexit ;
   }
/*
** Create One Tile
*/
 else
   {
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*pointTilesPP)->tileOffset = startPoint ; 
    (*pointTilesPP)->numTilePts = numPoints ; 
    *numPointTilesP = 1 ;
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
BENTLEYDTM_Private int bcdtmTile_sortAndTilePointsDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long minTilePoints,long axis,DTM_POINT_TILE *pointTilesP,long *numPointTilesP,long *lastStartPointP,long memPointTiles)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing onextLefty two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
/*
** Divide Data Set
*/
 if( numPoints > minTilePoints * 2  ) 
   {
    divider = numPoints >> 1 ;
/*
**  Median Sort Tile Points 
*/
    bcdtmTile_medianSortPointsDtmObject(dtmP,startPoint,numPoints,divider,axis) ;
/*
**  Create Two Data Sets
*/
    if( bcdtmTile_sortAndTilePointsDtmObject(dtmP,startPoint,divider,minTilePoints,1-axis,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
    if( bcdtmTile_sortAndTilePointsDtmObject(dtmP,startPoint+divider,numPoints-divider,minTilePoints,1-axis,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
   }
/*
** Add New Tile Entry
*/
 if( startPoint > *lastStartPointP )
   {
    if( *numPointTilesP+1 > memPointTiles )
      {
       bcdtmWrite_message(1,0,0,"Point Tile Memory Exhausted") ;
       goto errexit ;
      }
    else
      {
      (pointTilesP+*numPointTilesP)->tileOffset = startPoint ;
      (pointTilesP+*numPointTilesP)->numTilePts = numPoints ;
       ++*numPointTilesP ;
      }
/*
**  Set Last Start Point
*/
    *lastStartPointP = startPoint ;
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
BENTLEYDTM_Private void bcdtmTile_medianSortPointsDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot,swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DTM_TIN_POINT   *p1P,*p2P,temp;
/*
** Two Points
*/
 if( numPoints == 2) 
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = FALSE ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = TRUE ;
    if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
    if( swap == TRUE )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) bcdtmTile_randomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPoints;
    while (left < right) 
	  {
/* 
**     Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
	       scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+left) ;
           x = p1P->x ; 
	       y = p1P->y ; 
	       if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = TRUE ;
	       if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = TRUE ;
         } while( left <= right && scan == TRUE ) ;
/* 
**    Search for a point whose coordinate is too small for the right. 
*/
       do 
	     {
          --right ;
	       scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+right) ;
           x = p1P->x ; 
	       y = p1P->y ; 
	       if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = TRUE ;
	       if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = TRUE ;
         } while( left <= right &&  scan == TRUE ) ;
/* 
**     Swap the left and right points 
*/
       if( left < right ) 
	     {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmTile_medianSortPointsDtmObject(dtmP,startPoint,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmTile_medianSortPointsDtmObject(dtmP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTile_taggedPointsInDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE **pointTilesPP,long *numPointTilesP) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lastStartPoint=-1,memPointTiles=0,axis ;
 long   numTilePoints,numPointLevelTiles ;
 double xRange,yRange,zRange ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tiling Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %p",*numPointTilesP) ;
   }
/*
** Initialise
*/
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Create Two Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Calculate Number Of Point Tiles
*/
    numTilePoints = numPoints ;
    memPointTiles = 1 ;
    numPointLevelTiles = 1 ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
    while( numTilePoints > minTilePoints * 2 )
      {
       memPointTiles = memPointTiles * 2 ;
       numTilePoints = numTilePoints / 2 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
      }  
/*
**  Allocate Memory For Point Tiles
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( memPointTiles * sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Get x and y Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,startPoint,numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"xrange = %12.4lf yRange = %12.4lf zRange = %12.4lf",xRange,yRange,zRange) ;
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Tile By Divide And Conquer Method
*/
    if( bcdtmTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint,numPoints,minTilePoints,axis,*pointTilesPP,numPointTilesP,&lastStartPoint,memPointTiles) ) goto errexit ;
   }
/*
** Create One Tile
*/
 else
   {
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*pointTilesPP)->tileOffset = startPoint ; 
    (*pointTilesPP)->numTilePts = numPoints ; 
    *numPointTilesP = 1 ;
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
BENTLEYDTM_Private int bcdtmTile_sortAndTileTaggedPointsDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long minTilePoints,long axis,DTM_POINT_TILE *pointTilesP,long *numPointTilesP,long *lastStartPointP,long memPointTiles)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing onextLefty two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
/*
** Divide Into Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
    divider = numPoints >> 1 ;
/*
**  Median Sort Tile Points 
*/
    bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,startPoint,numPoints,divider,axis) ;
/*
**  Create Two Data Sets
*/
    if( bcdtmTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint,divider,minTilePoints,1-axis,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
    if( bcdtmTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint+divider,numPoints-divider,minTilePoints,1-axis,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
   }
/*
** Add New Tile Entry
*/
 if( startPoint > *lastStartPointP )
   {
    if( *numPointTilesP+1 > memPointTiles )
      {
       bcdtmWrite_message(1,0,0,"Point Tile Memory Exhausted") ;
       goto errexit ;
      }
/*
**  Store Tile Entry
*/ 
    (pointTilesP+*numPointTilesP)->tileOffset = startPoint ;
    (pointTilesP+*numPointTilesP)->numTilePts = numPoints ;
    ++*numPointTilesP ;
/*
**  Set Last Start Point
*/
    *lastStartPointP = startPoint ;
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
BENTLEYDTM_Private void bcdtmTile_medianSortTaggedPointsDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot;
 bool    swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DTM_TIN_POINT   *p1P,*p2P,temp;
 long    tag ; 
/*
** Two Points
*/
 if( numPoints == 2) 
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = false ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = true ;
    else if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
    if( swap == true )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
       tag   = *(tagP+startPoint) ;
       *(tagP+startPoint) = *(tagP+startPoint+1) ;
       *(tagP+startPoint+1) = tag ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) bcdtmTile_randomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    else if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPoints;
    while (left < right) 
	  {
/* 
**     Search for a point whose coordinate is too large for the left. 
*/
       scan = false ;
       do 
         {
          ++left ;
           if (left <= right)
               {
               p1P = pointAddrP(dtmP,startPoint+left) ;
           x = p1P->x ; 
	       y = p1P->y ; 
	       if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = true ;
           else if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = true ;
               }
         } while( scan == true ) ;
/* 
**    Search for a point whose coordinate is too small for the right. 
*/
               scan = false ;
       do 
	     {
          --right ;
           if (left <= right)
               {
               p1P = pointAddrP(dtmP,startPoint+right) ;
           x = p1P->x ; 
	       y = p1P->y ; 
	       if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = true ;
           else if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = true;
               }
         } while( scan == true ) ;
/* 
**     Swap the left and right points 
*/
       if( left < right ) 
	     {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
          tag  = *(tagP+startPoint+left) ;
          *(tagP+startPoint+left)  = *(tagP+startPoint+right) ;
          *(tagP+startPoint+right) = tag ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,startPoint,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}

/*------------------------------------------------------------------ - +
| |
| |
| |
+------------------------------------------------------------------ - */
void bcdtmMedianTile_multiThreadTileWorkerDtmObject (DTM_MULTI_THREAD_TILE *tileParametersP)
/*
** This function uses a divide and conquer strategy to tile DTM Points
**
** Author : Rob Cormack  December 2009  Rob.Cormack@Bentley.com
**
*/
    {
    int    dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    long   startTime, numPointTiles = 0;
    DTM_POINT_TILE *pointTilesP = NULL;
    /*
    ** Initialise variables
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** startPoint = %8ld numPoints = %8ld", tileParametersP->thread, tileParametersP->startPoint, tileParametersP->numPoints);
    /*
    ** Sort Points Into Tiles
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tiling %9ld Dtm Points", tileParametersP->thread, tileParametersP->numPoints);
    if (tileParametersP->tagP == NULL)
        {
        if (bcdtmMedianTile_pointsInDtmObject (tileParametersP->dtmP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    else
        {
        if (bcdtmMedianTile_taggedPointsInDtmObject (tileParametersP->dtmP, tileParametersP->tagP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tile Time = %8.3lf Seconds", tileParametersP->thread, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Set Return values
    */
    tileParametersP->pointTilesP = pointTilesP;
    tileParametersP->numPointTiles = numPointTiles;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMedianTile_pointsDtmObject
(
 BC_DTM_OBJ *dtmP,                       // ==> Pointer to DTM Object
 long       *tagP,                       // ==> Pointer To User Tag. Can Be set To Null 
 long       startPoint,                  // ==> Offset Of Start Point In DTM
 long       numPoints,                   // ==> Number Of Points To Tile
 long       minTilePoints,               // ==> Minimum Point Size Of Tiles
 DTM_POINT_TILE **pointTilesPP,          // <== Pointer to Tile Offsets Array
 long           *numPointTilesP          // <== Number Of Tiles
 )
/*
** This Function Multi Thread Tiles A Dtm Object
**
** Author : Rob Cormack  December 2009  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   n,axis,startTime ;
 long   startSortPoint,numSortPoints,medianSortPoint ;
 long   numThreadPoints=0,numThreadArrayPoints[DTM_MAX_PROCESSORS] ;
 double xRange,yRange,zRange ;
 std::thread thread[DTM_MAX_PROCESSORS] ;
 DTM_MULTI_THREAD_TILE multiThread[DTM_MAX_PROCESSORS] ;
 DTM_POINT_TILE *tileP,*tile1P ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %8ld",*numPointTilesP) ;
   }
/*
** Validate
*/
 startTime = bcdtmClock() ;
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For DTM In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(0,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   } 
/*
** Single Thread Tile
*/
 if( numPoints <= 1024 || minTilePoints > numPoints / 2 || minTilePoints > numPoints / DTM_NUM_PROCESSORS )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Tiling Dtm") ; 
    if( tagP == NULL )
      {
       if( bcdtmMedianTile_pointsInDtmObject(dtmP,startPoint,numPoints,minTilePoints,pointTilesPP,numPointTilesP) ) goto errexit ; 
      }
    else
      {
       if( bcdtmMedianTile_taggedPointsInDtmObject(dtmP,tagP,startPoint,numPoints,minTilePoints,pointTilesPP,numPointTilesP) ) goto errexit ; 
      } 
   }   
/*
** Multi Thread Tile
*/
 else
   {    
    if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Tiling Dtm") ;
/*
**  Get x and y Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,startPoint,numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"xrange = %12.4lf yRange = %12.4lf zRange = %12.4lf",xRange,yRange,zRange) ;
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Determine Number Of Points Per Thread
*/
    numThreadPoints = dtmP->numPoints / DTM_NUM_PROCESSORS + 1 ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS == 0 ) --numThreadPoints ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) numThreadArrayPoints[n] =  numThreadPoints ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS != 0 ) numThreadArrayPoints[DTM_NUM_PROCESSORS-1] = dtmP->numPoints - (DTM_NUM_PROCESSORS-1) * numThreadPoints ;
    if( dbg == 1 ) for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n  ) bcdtmWrite_message(0,0,0,"Thread[%2ld] ** numPoints = %6ld",n,numThreadArrayPoints[n]) ;
/*
**  Initialise Multi Thread Tile Array
*/
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       multiThread[n].thread          = n ;
       multiThread[n].dtmP            = dtmP ;
       multiThread[n].tagP            = tagP ;
       multiThread[n].startPoint      = startPoint ;
       multiThread[n].numPoints       = numThreadArrayPoints[n] ;
       multiThread[n].minTilePts      = minTilePoints ;
       multiThread[n].pointTilesP     = NULL  ;
       multiThread[n].numPointTiles   = 0  ;
       startPoint = startPoint + numThreadArrayPoints[n] ;
      }
/*
**  Sort Points
*/
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Median Sorting Points For Multiple Processors") ;
    startSortPoint = 0 ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS - 1 ; ++n )
      {
       numSortPoints   = dtmP->numPoints - startSortPoint  ;
       medianSortPoint =  multiThread[n].numPoints - 1  ; 
       bcdtmTile_medianSortPointsDtmObject(dtmP,startSortPoint,numSortPoints,medianSortPoint,axis) ;
       startSortPoint  = startSortPoint + multiThread[n].numPoints ; 
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Sort %8ld Points = %8.3lf Secs",dtmP->numPoints,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
**  Create Threads To Tile Dtm Object
*/
    for ( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
      thread[n] = std::thread (bcdtmMedianTile_multiThreadTileWorkerDtmObject, &multiThread[n]);
//           CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) bcdtmMedianTile_multiThreadTileWorkerDtmObject,&multiThread[n],0,&threadId[n]) ;
      }
/*
**  Wait For All Threads To Complete
*/
    for (n = 0; n < DTM_NUM_PROCESSORS; ++n)
        thread[n].join ();
//    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) CloseHandle(thread[n]) ;
/*
**  Count Total Number Of Tiles
*/
    *numPointTilesP = 0 ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"pointTilesP = %p ** numPointTiles = %8ld",multiThread[n].pointTilesP,multiThread[n].numPointTiles) ;
       *numPointTilesP = *numPointTilesP + multiThread[n].numPointTiles ;
      }
/*
**  Allocate Memory
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc (*numPointTilesP*sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }  
/*
**  Copy Tile Offset
*/
    tile1P = *pointTilesPP ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       for( tileP = multiThread[n].pointTilesP ; tileP < multiThread[n].pointTilesP + multiThread[n].numPointTiles ; ++tileP )
         {
          *tile1P = *tileP ;
          ++tile1P ;
         } 
/*
**     Free Memory
*/
       free(multiThread[n].pointTilesP) ;
       multiThread[n].pointTilesP = NULL ;
      }  
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Multi Thread Sort = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Public int bcdtmMedianTile_getXYZCoordinateRangesDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,double *xRangeP,double *yRangeP,double *zRangeP)
{
 int ret=DTM_SUCCESS ;
 long point ;
 double xMin,yMin,zMin,xMax,yMax,zMax ;
 DTM_TIN_POINT *pointP ;
/*
** Initialise
*/
 *xRangeP = 0.0 ;
 *yRangeP = 0.0 ;
 *zRangeP = 0.0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Scan DTM
*/
 pointP = pointAddrP(dtmP,startPoint) ;
 xMin = xMax = pointP->x ;
 yMin = yMax = pointP->y ;
 zMin = zMax = pointP->z ;
 for( point = startPoint + 1 ; point < startPoint + numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    if( pointP->x < xMin ) xMin = pointP->x ;
    if( pointP->x > xMax ) xMax = pointP->x ;
    if( pointP->y < yMin ) yMin = pointP->y ;
    if( pointP->y > yMax ) yMax = pointP->y ;
    if( pointP->z < zMin ) zMin = pointP->z ;
    if( pointP->z > zMax ) zMax = pointP->z ;
   }
 *xRangeP = xMax - xMin ;
 *yRangeP = yMax - yMin ;
 *zRangeP = zMax - zMin ;
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
BENTLEYDTM_Public int bcdtmMedianTile_pointsInDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE **pointTilesPP,long *numPointTilesP) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lastStartPoint=-1,memPointTiles=0 ;
 long   numTilePoints,numPointLevelTiles ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tiling Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %p",*numPointTilesP) ;
   }
/*
** Initialise
*/
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Create Two Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Calculate Number Of Point Tiles
*/
    numTilePoints = numPoints ;
    memPointTiles = 1 ;
    numPointLevelTiles = 1 ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
    while( numTilePoints > minTilePoints * 2 )
      {
       memPointTiles = memPointTiles * 2 ;
       numTilePoints = numTilePoints / 2 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
      }  
/*
**  Allocate Memory For Point Tiles
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( memPointTiles * sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Tile By Divide And Conquer Method
*/
    if( bcdtmMedianTile_sortAndTilePointsDtmObject(dtmP,startPoint,numPoints,minTilePoints,*pointTilesPP,numPointTilesP,&lastStartPoint,memPointTiles) ) goto errexit ;
   }
/*
** Create One Tile
*/
 else
   {
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*pointTilesPP)->tileOffset = startPoint ; 
    (*pointTilesPP)->numTilePts = numPoints ; 
    *numPointTilesP = 1 ;
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
BENTLEYDTM_Private int bcdtmMedianTile_sortAndTilePointsDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE *pointTilesP,long *numPointTilesP,long *lastStartPointP,long memPointTiles)
/* 
**
** Sorts points into Tiles
** Sorts by x-coordinate if x Coordinate Range Is Greater Than The y Coordinate Range  
** Sorts by y-coordinate if y Coordinate Range Is Greater Than The x Coordinate Range    
**                                           
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   axis,divider ;
 double xRange,yRange,zRange ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Tiling Tile ** startPoint = %8ld  numPoints = %8ld",startPoint,numPoints) ;
   }
/*
** Divide Into Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Set Divider
*/
    divider = numPoints >> 1 ;
/*
**  Get Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,startPoint,numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Median Sort Tile Points 
*/
    bcdtmTile_medianSortPointsDtmObject(dtmP,startPoint,numPoints,divider,axis) ;
/*
**  Tile The Tiles
*/
    if( bcdtmMedianTile_sortAndTilePointsDtmObject(dtmP,startPoint,divider,minTilePoints,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
    if( bcdtmMedianTile_sortAndTilePointsDtmObject(dtmP,startPoint+divider,numPoints-divider,minTilePoints,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
   }
/*
**  Create New Tile
*/
 if( startPoint > *lastStartPointP )
   {
    if( *numPointTilesP+1 > memPointTiles )
      {
       bcdtmWrite_message(1,0,0,"Point Tile Memory Exhausted") ;
       goto errexit ;
      }
    else
      {
      (pointTilesP+*numPointTilesP)->tileOffset = startPoint ;
      (pointTilesP+*numPointTilesP)->numTilePts = numPoints ;
       ++*numPointTilesP ;
      }
/*
**  Set Last Start Point
*/
    *lastStartPointP = startPoint ;
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
BENTLEYDTM_Public int bcdtmMedianTile_taggedPointsInDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE **pointTilesPP,long *numPointTilesP) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lastStartPoint=-1,memPointTiles=0 ;
 long   numTilePoints,numPointLevelTiles ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tiling Tagged Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"tagP            = %p",tagP) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints       = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints   = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*pointTilesPP   = %p",*pointTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numPointTilesP = %p",*numPointTilesP) ;
   }
/*
** Initialise
*/
 *numPointTilesP = 0 ;
 if( *pointTilesPP != NULL ) { free(*pointTilesPP) ; *pointTilesPP = NULL ; }
/*
** Create Two Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Calculate Number Of Point Tiles
*/
    numTilePoints = numPoints ;
    memPointTiles = 1 ;
    numPointLevelTiles = 1 ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
    while( numTilePoints > minTilePoints * 2 )
      {
       memPointTiles = memPointTiles * 2 ;
       numTilePoints = numTilePoints / 2 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numTilePoints = %8ld minTilePoints = %8ld memPointTiles = %8ld",numTilePoints,minTilePoints,memPointTiles) ;
      }  
/*
**  Allocate Memory For Point Tiles
*/
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( memPointTiles * sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Tile By Divide And Conquer Method
*/
    if( bcdtmMedianTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint,numPoints,minTilePoints,*pointTilesPP,numPointTilesP,&lastStartPoint,memPointTiles) ) goto errexit ;
   }
/*
** Create One Tile
*/
 else
   {
    *pointTilesPP = ( DTM_POINT_TILE * ) malloc( sizeof(DTM_POINT_TILE)) ;
    if( *pointTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*pointTilesPP)->tileOffset = startPoint ; 
    (*pointTilesPP)->numTilePts = numPoints ; 
    *numPointTilesP = 1 ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tiling Tagged Points DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tiling Tagged Points DTM Object Error") ;
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
BENTLEYDTM_Private int bcdtmMedianTile_sortAndTileTaggedPointsDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long minTilePoints,DTM_POINT_TILE *pointTilesP,long *numPointTilesP,long *lastStartPointP,long memPointTiles)
/* 
**
** Sorts Points into Tiles
** Sorts by x-coordinate if The x Coordiante Range Is Greater Than The y Coordinate Range  
** Sorts by y-coordinate if The y Coordiante Range Is Greater Than The x Coordinate Range     
**                                           
*/
{
 int    dbg=DTM_TRACE_VALUE(0) ;
 long   axis,divider ;
 double xRange,yRange,zRange ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Tiling Tile ** startPoint = %8ld  numPoints = %8ld",startPoint,numPoints) ;
   }
/*
** Divide Into Data Sets
*/
 if( numPoints > minTilePoints * 2  ) 
   {
/*
**  Get Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,startPoint,numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Create Two Data Sets
*/
    divider = numPoints >> 1 ;
/*
**  Median Sort Tile Points 
*/
    bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,startPoint,numPoints,divider,axis) ;
/*
**  Tile The Tiles
*/
    if( bcdtmMedianTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint,divider,minTilePoints,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
    if( bcdtmMedianTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,startPoint+divider,numPoints-divider,minTilePoints,pointTilesP,numPointTilesP,lastStartPointP,memPointTiles) ) goto errexit ;
  }
/*
** Add New Tile Entry
*/
 if( startPoint > *lastStartPointP )
   {
    if( *numPointTilesP+1 > memPointTiles )
      {
       bcdtmWrite_message(1,0,0,"Point Tile Memory Exhausted") ;
       goto errexit ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Tile ** startPoint = %8ld numPoints = %8ld",startPoint,numPoints) ;
      (pointTilesP+*numPointTilesP)->tileOffset = startPoint ;
      (pointTilesP+*numPointTilesP)->numTilePts = numPoints ;
       ++*numPointTilesP ;
      }
/*
**  Set Last Start Point
*/
    *lastStartPointP = startPoint ;
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
void bcdtmQuadTreeTile_multiThreadTileWorkerDtmObject (DTM_MULTI_THREAD_QUAD_TREE_TILE *tileParametersP)
/*
** This function uses a divide and conquer strategy to tile DTM Points
**
** Author : Rob Cormack  December 2009  Rob.Cormack@Bentley.com
**
*/
    {
    int    dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    long   startTime, numPointTiles = 0;
    DTM_QUAD_TREE_TILE *pointTilesP = NULL;
    /*
    ** Initialise variables
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** startPoint = %8ld numPoints = %8ld", tileParametersP->thread, tileParametersP->startPoint, tileParametersP->numPoints);
    /*
    ** Sort Points Into Tiles
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tiling %9ld Dtm Points", tileParametersP->thread, tileParametersP->numPoints);
    if (tileParametersP->tagP == NULL)
        {
        if (bcdtmQuadTreeTile_pointsInDtmObject (tileParametersP->dtmP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    else
        {
        if (bcdtmQuadTreeTile_taggedPointsInDtmObject (tileParametersP->dtmP, tileParametersP->tagP, tileParametersP->startPoint, tileParametersP->numPoints, tileParametersP->minTilePts, &pointTilesP, &numPointTiles))
            return;
        }
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Tile Time = %8.3lf Seconds", tileParametersP->thread, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Set Return values
    */
    tileParametersP->pointTilesP = pointTilesP;
    tileParametersP->numPointTiles = numPointTiles;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmQuadTreeTile_pointsDtmObject
(
 BC_DTM_OBJ *dtmP,                       // ==> Pointer to DTM Object
 long       *tagP,                       // ==> Pointer To User Tag. Can Be set To Null 
 long       startPoint,                  // ==> Offset Of Start Point In DTM
 long       numPoints,                   // ==> Number Of Points To Tile
 long       minTilePoints,               // ==> Minimum Point Size Of Tiles
 DTM_QUAD_TREE_TILE **quadTreeTilesPP,   // <== Pointer to Tile Offsets Array
 long           *numQuadTreeTilesP       // <== Number Of Tiles
 )
/*
** This Function Quad Tree Tiles A Dtm Object
**
** Author : Rob Cormack  February 2010  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   n,pnt,axis,startTime ;
 long   startSortPoint,numSortPoints,medianSortPoint ;
 long   numThreadPoints=0,numThreadArrayPoints[DTM_MAX_PROCESSORS] ;
 double xRange,yRange,zRange ;
 std::thread thread[DTM_MAX_PROCESSORS] ;
 DTM_MULTI_THREAD_QUAD_TREE_TILE multiThread[DTM_MAX_PROCESSORS] ;
 DTM_QUAD_TREE_TILE *tileP,*tile1P ;
 DTM_TIN_POINT *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Quadtree Tiling Points Dtm Object") ; 
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint         = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints          = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints      = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*quadTreeTilesPP   = %p",*quadTreeTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numQuadTreeTilesP = %8ld",*numQuadTreeTilesP) ;
   }
/*
** Validate
*/
 startTime = bcdtmClock() ;
 *numQuadTreeTilesP = 0 ;
 if( *quadTreeTilesPP != NULL ) { free(*quadTreeTilesPP) ; *quadTreeTilesPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For DTM In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(0,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   } 
/*
** Single Thread Tile
*/
 if( 1 /* numPoints <= 1024 */ )   // Multi Threading Not Yet Implemented
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Quad Tree Tiling Dtm") ; 
    if( tagP == NULL )
      {
       if( bcdtmQuadTreeTile_pointsInDtmObject(dtmP,0,dtmP->numPoints,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP) ) goto errexit ; 
      }
    else
      {
       if( bcdtmQuadTreeTile_taggedPointsInDtmObject(dtmP,tagP,0,dtmP->numPoints,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP) ) goto errexit ; 
      } 
   }   
/*
** Multi Thread Tile
*/
 else
   {    
    if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Quad Tree Tiling Dtm") ;
/*
**  Determine Number Of Points Per Thread
*/
    numThreadPoints = dtmP->numPoints / DTM_NUM_PROCESSORS + 1 ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS == 0 ) --numThreadPoints ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) numThreadArrayPoints[n] =  numThreadPoints ;
    if( dtmP->numPoints % DTM_NUM_PROCESSORS != 0 ) numThreadArrayPoints[DTM_NUM_PROCESSORS-1] = dtmP->numPoints - (DTM_NUM_PROCESSORS-1) * numThreadPoints ;
    if( dbg == 1 ) for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n  ) bcdtmWrite_message(0,0,0,"Thread[%2ld] ** numPoints = %6ld",n,numThreadArrayPoints[n]) ;
/*
**  Initialise Multi Thread Tile Array
*/
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       multiThread[n].thread          = n ;
       multiThread[n].dtmP            = dtmP ;
       multiThread[n].tagP            = tagP ;
       multiThread[n].startPoint      = startPoint ;
       multiThread[n].numPoints       = numThreadArrayPoints[n] ;
       multiThread[n].minTilePts      = minTilePoints ;
       multiThread[n].pointTilesP     = NULL  ;
       multiThread[n].numPointTiles   = 0  ;
       startPoint = startPoint + numThreadArrayPoints[n] ;
      }
/*
**  Get x and y Coordinate Ranges
*/
    if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,0,dtmP->numPoints,&xRange,&yRange,&zRange)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"xrange = %12.4lf yRange = %12.4lf zRange = %12.4lf",xRange,yRange,zRange) ;
/*
**  Set Axis
*/
    if( xRange > yRange ) axis = DTM_X_AXIS ;
    else                  axis = DTM_Y_AXIS ;
/*
**  Sort Points
*/
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Median Sorting Points For Multiple Processors") ;
    startSortPoint = 0 ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS - 1 ; ++n )
      {
       numSortPoints   = dtmP->numPoints - startSortPoint  ;
       medianSortPoint =  multiThread[n].numPoints - 1  ; 
       bcdtmTile_medianSortPointsDtmObject(dtmP,startSortPoint,numSortPoints,medianSortPoint,axis) ;
       startSortPoint  = startSortPoint + multiThread[n].numPoints ; 
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Sort %8ld Points = %8.3lf Secs",dtmP->numPoints,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
**  Create Threads To Tile Dtm Object
*/
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
      thread[n] = std::thread (bcdtmQuadTreeTile_multiThreadTileWorkerDtmObject, &multiThread[n]);
           //CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) bcdtmQuadTreeTile_multiThreadTileWorkerDtmObject,&multiThread[n],0,&threadId[n]) ;
      }
/*
**  Wait For All Threads To Complete
*/
    for (n = 0; n < DTM_NUM_PROCESSORS; ++n)
        thread[n].join();
    //WaitForMultipleObjects (DTM_NUM_PROCESSORS, thread, 1, INFINITE);
    //for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) CloseHandle(thread[n]) ;
/*
**  Count Total Number Of Tiles
*/
    *numQuadTreeTilesP = 0 ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"pointTilesP = %p ** numPointTiles = %8ld",multiThread[n].pointTilesP,multiThread[n].numPointTiles) ;
       *numQuadTreeTilesP = *numQuadTreeTilesP + multiThread[n].numPointTiles ;
      }
/*
**  Allocate Memory
*/
    *quadTreeTilesPP = ( DTM_QUAD_TREE_TILE * ) malloc (*numQuadTreeTilesP*sizeof(DTM_QUAD_TREE_TILE)) ;
    if( *quadTreeTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }  
/*
**  Copy Tile Offset
*/
    tile1P = *quadTreeTilesPP ;
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       for( tileP = multiThread[n].pointTilesP ; tileP < multiThread[n].pointTilesP + multiThread[n].numPointTiles ; ++tileP )
         {
          *tile1P = *tileP ;
          ++tile1P ;
         } 
/*
**     Free Memory
*/
       free(multiThread[n].pointTilesP) ;
       multiThread[n].pointTilesP = NULL ;
      }  
   }
/*
**  Set Quad Tree Tile Extents
*/
 for( tileP = *quadTreeTilesPP ; tileP  < *quadTreeTilesPP + *numQuadTreeTilesP ; ++tileP )
   {
    pntP = pointAddrP(dtmP,tileP->tileOffset) ;
    tileP->xMin = tileP->xMax = pntP->x ;
    tileP->yMin = tileP->yMax = pntP->y ;
    tileP->zMin = tileP->zMax = pntP->z ;
    for( pnt = tileP->tileOffset ; pnt < tileP->tileOffset + tileP->numTilePts ; ++pnt )
      {
       pntP = pointAddrP(dtmP,pnt) ;
       if( pntP->x < tileP->xMin ) tileP->xMin = pntP->x ;
       if( pntP->x > tileP->xMax ) tileP->xMax = pntP->x ;
       if( pntP->y < tileP->yMin ) tileP->yMin = pntP->y ;
       if( pntP->y > tileP->yMax ) tileP->yMax = pntP->y ;
       if( pntP->z < tileP->zMin ) tileP->zMin = pntP->z ;
       if( pntP->z > tileP->zMax ) tileP->zMax = pntP->z ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
// if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Multi Thread Sort = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Quadtree Tiling Points Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Quadtree Tiling Points Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Private int bcdtmQuadTreeTile_createQuadTreeChildNodes
(
 long tileNumber ,                     // ==> Tile Number Of Parent Node
 DTM_QUAD_TREE_TILE **quadTreeTilesPP, // ==> Quad Tree Tile Array
 long *numQuadTreeTilesP,              // ==> Number Of Quadtree Tiles
 long  memQuadTreeTiles                // ==> Memory Allocated For quad Tree Tiles
)
{
int dbg = 0;
 long n,divider,offset,numPoints ;
/*
**  Check Memory
*/
 if( *numQuadTreeTilesP + 4 > memQuadTreeTiles )
   {
    bcdtmWrite_message(1,0,0,"Quad Tree Memory Exhausted") ;
    goto errexit ;
   }
/*
**  Create And Store Child Nodes
*/ 
 offset    = (*quadTreeTilesPP+tileNumber)->tileOffset ;
 numPoints = (*quadTreeTilesPP+tileNumber)->numTilePts ;
 divider = numPoints >> 2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"offset = %8ld ** numPoints = %8ld ** divider = %8ld",offset,numPoints,divider) ;
 for( n = 0 ; n < 4 ; ++n )
   {
    (*quadTreeTilesPP+*numQuadTreeTilesP)->tileOffset = offset + n * divider ;
    if( n != 3 ) (*quadTreeTilesPP+*numQuadTreeTilesP)->numTilePts = divider  ;
    else         (*quadTreeTilesPP+*numQuadTreeTilesP)->numTilePts = numPoints - 3 * divider  ; 
    (*quadTreeTilesPP+*numQuadTreeTilesP)->quadTreeLevel = (*quadTreeTilesPP+tileNumber)->quadTreeLevel + 1 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->tileNumber    = *numQuadTreeTilesP ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->childNodes[0] = -1 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->childNodes[1] = -1 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->childNodes[2] = -1 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->childNodes[3] = -1 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->parentNode = tileNumber ;
    (*quadTreeTilesPP+tileNumber)->childNodes[n] = *numQuadTreeTilesP ; 
    (*quadTreeTilesPP+*numQuadTreeTilesP)->xMin = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->xMax = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->yMin = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->yMax = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->zMin = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->zMax = 0.0 ;
    (*quadTreeTilesPP+*numQuadTreeTilesP)->dtmP = NULL ;
     ++*numQuadTreeTilesP ;
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
BENTLEYDTM_Public int bcdtmQuadTreeTile_pointsInDtmObject
(
 BC_DTM_OBJ *dtmP,
 long startPoint,
 long numPoints,
 long minTilePoints,
 DTM_QUAD_TREE_TILE **quadTreeTilesPP,
 long *numQuadTreeTilesP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numQuadTreePoints=0,numQuadTreeTiles=0,numQuadTreeLevelTiles=1,memQuadTreeTiles=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Quad Tree Tiling Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint         = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints          = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints      = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*quadTreeTilesP    = %p",*quadTreeTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numQuadTreeTilesP = %p",*numQuadTreeTilesP) ;
   }
/*
** Initialise
*/
 *numQuadTreeTilesP = 0 ;
 if( *quadTreeTilesPP != NULL ) { free(*quadTreeTilesPP) ; *quadTreeTilesPP = NULL ; }
/*
** Create Four Data Sets
*/
 if( numPoints > minTilePoints * 4  ) 
   {
/*
**  Calculate Number Of Quadtree Tiles
*/
    numQuadTreePoints = numPoints ;
    numQuadTreeTiles  = 1 ;
    numQuadTreeLevelTiles = 1 ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numQuadTreePoints = %8ld minTilePoints = %8ld numQuadTreeTiles = %8ld",numQuadTreePoints,minTilePoints,numQuadTreeTiles) ;
    while( numQuadTreePoints > minTilePoints * 4 )
      {
       numQuadTreeLevelTiles = numQuadTreeLevelTiles * 4 ;
       numQuadTreeTiles = numQuadTreeTiles + numQuadTreeLevelTiles ;
       numQuadTreePoints = numQuadTreePoints / 4 ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numQuadTreePoints = %8ld minTilePoints = %8ld numQuadTreeTiles = %8ld",numQuadTreePoints,minTilePoints,numQuadTreeTiles) ;
      }  
/*
**  Allocate Memory For QuadTree Tiles
*/
    memQuadTreeTiles = numQuadTreeTiles  ;
    *quadTreeTilesPP = ( DTM_QUAD_TREE_TILE * ) malloc( memQuadTreeTiles * sizeof(DTM_QUAD_TREE_TILE)) ;
    if( *quadTreeTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Create Quadtree Header Node
*/
    (*quadTreeTilesPP)->tileOffset    = startPoint ; 
    (*quadTreeTilesPP)->numTilePts    = numPoints ; 
    (*quadTreeTilesPP)->tileNumber    = 0 ; 
    (*quadTreeTilesPP)->quadTreeLevel = 0 ;
    (*quadTreeTilesPP)->childNodes[0] = -1 ;
    (*quadTreeTilesPP)->childNodes[1] = -1 ;
    (*quadTreeTilesPP)->childNodes[2] = -1 ;
    (*quadTreeTilesPP)->childNodes[3] = -1 ;
    (*quadTreeTilesPP)->parentNode    = -1 ;
    (*quadTreeTilesPP)->xMin = 0.0 ;
    (*quadTreeTilesPP)->xMax = 0.0 ;
    (*quadTreeTilesPP)->yMin = 0.0 ;
    (*quadTreeTilesPP)->yMax = 0.0 ;
    (*quadTreeTilesPP)->zMin = 0.0 ;
    (*quadTreeTilesPP)->zMax = 0.0 ;
    (*quadTreeTilesPP)->dtmP = NULL ;
    *numQuadTreeTilesP = 1 ;
/*
**  Create Quad Tree Tiles
*/
   if( bcdtmQuadTreeTile_sortAndTilePointsDtmObject(dtmP,*quadTreeTilesPP,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles) ) goto errexit ; 
  }
/*
** Create One Tile
*/
 else
   {
    *quadTreeTilesPP = ( DTM_QUAD_TREE_TILE * ) malloc( sizeof(DTM_QUAD_TREE_TILE)) ;
    if( *quadTreeTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*quadTreeTilesPP)->tileOffset   = 0 ; 
    (*quadTreeTilesPP)->numTilePts   = numPoints ; 
    (*quadTreeTilesPP)->quadTreeLevel =  0 ;
    (*quadTreeTilesPP)->childNodes[0] = -1 ;
    (*quadTreeTilesPP)->childNodes[1] = -1 ;
    (*quadTreeTilesPP)->childNodes[2] = -1 ;
    (*quadTreeTilesPP)->childNodes[3] = -1 ;
    (*quadTreeTilesPP)->parentNode    = -1 ;
    (*quadTreeTilesPP)->xMin = 0.0 ;
    (*quadTreeTilesPP)->xMax = 0.0 ;
    (*quadTreeTilesPP)->yMin = 0.0 ;
    (*quadTreeTilesPP)->yMax = 0.0 ;
    (*quadTreeTilesPP)->zMin = 0.0 ;
    (*quadTreeTilesPP)->zMax = 0.0 ;
    (*quadTreeTilesPP)->dtmP = NULL ;
    *numQuadTreeTilesP = 1 ;
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
BENTLEYDTM_Private int bcdtmQuadTreeTile_sortAndTilePointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTM_QUAD_TREE_TILE *quadTreeTileP,
 long minTilePoints,
 DTM_QUAD_TREE_TILE **quadTreeTilesPP,
 long *numQuadTreeTilesP,
 long memQuadTreeTiles
)
/* 
** Uses A Divide And Conquer Startegy To Quad Tree Tile Points
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   axis,divider,numTilePts,tileOffset ;
 double xRange,yRange,zRange ;
 DTM_QUAD_TREE_TILE *qtreeP ;
/*
** Only Process If There Are No Child Nodes
*/
 if( quadTreeTileP->childNodes[0] == -1 )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Parent Node ** Tile = %8ld ** offset = %8ld numTilePts = %8ld",quadTreeTileP->tileNumber,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts) ;
/*
**  Divide Into Data Sets
*/
    if( quadTreeTileP->numTilePts > minTilePoints * 4  ) 
      {
/*
**     Get Coordinate Ranges
*/
       if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts,&xRange,&yRange,&zRange)) goto errexit ; 
/*
**     Set Axis
*/
       if( xRange > yRange ) axis = DTM_X_AXIS ;
       else                  axis = DTM_Y_AXIS ;
/*
**     Create Four Child Nodes
*/
       if( bcdtmQuadTreeTile_createQuadTreeChildNodes(quadTreeTileP->tileNumber,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles)) goto errexit ;
/*
**     Median Sort Tile Points. Firstly Divide The Whole Set And Secondly The Half Sets 
*/
       qtreeP = *quadTreeTilesPP + *numQuadTreeTilesP - 4 ;
       divider = qtreeP->numTilePts + (qtreeP+1)->numTilePts ;
       bcdtmTile_medianSortPointsDtmObject(dtmP,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts,divider,axis) ;
       numTilePts = divider ;
       divider    = qtreeP->numTilePts ;
       bcdtmTile_medianSortPointsDtmObject(dtmP,quadTreeTileP->tileOffset,numTilePts,divider,axis) ;
       tileOffset = quadTreeTileP->tileOffset + numTilePts ;
       divider    = (qtreeP+2)->numTilePts ;
       numTilePts = (qtreeP+2)->numTilePts + (qtreeP+3)->numTilePts ;  
       bcdtmTile_medianSortPointsDtmObject(dtmP,tileOffset,numTilePts,divider,axis) ;
/*
**     Divide and Conquer 
*/
       for( qtreeP = *quadTreeTilesPP + *numQuadTreeTilesP - 4 ; qtreeP < *quadTreeTilesPP + *numQuadTreeTilesP ; ++qtreeP )
         { 
          if( bcdtmQuadTreeTile_sortAndTilePointsDtmObject(dtmP,qtreeP,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles) ) goto errexit ; 
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
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_Public int bcdtmQuadTreeTile_taggedPointsInDtmObject
(
 BC_DTM_OBJ *dtmP,
 long *tagP,
 long startPoint,
 long numPoints,
 long minTilePoints,
 DTM_QUAD_TREE_TILE **quadTreeTilesPP,
 long *numQuadTreeTilesP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numQuadTreePoints=0,numQuadTreeTiles=1,numQuadTreeLevelTiles=1,memQuadTreeTiles=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Quad Tree Tiling Points DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint         = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"numPoints          = %8ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"minTilePoints      = %8ld",minTilePoints) ;
    bcdtmWrite_message(0,0,0,"*quadTreeTilesP    = %p",*quadTreeTilesPP) ;
    bcdtmWrite_message(0,0,0,"*numQuadTreeTilesP = %p",*numQuadTreeTilesP) ;
   }
/*
** Initialise
*/
 *numQuadTreeTilesP = 0 ;
 if( *quadTreeTilesPP != NULL ) { free(*quadTreeTilesPP) ; *quadTreeTilesPP = NULL ; }
/*
** Create Four Data Sets
*/
 if( numPoints > minTilePoints * 4  ) 
   {
/*
**  Calculate Number Of Quadtree Tiles
*/
    numQuadTreePoints = numPoints ;
    numQuadTreeTiles  = 1 ;
    numQuadTreeLevelTiles = 1 ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numQuadTreePoints = %8ld minTilePoints = %8ld numQuadTreeTiles = %8ld",numQuadTreePoints,minTilePoints,numQuadTreeTiles) ;
    while( numQuadTreePoints > minTilePoints * 4 )
      {
       numQuadTreeLevelTiles = numQuadTreeLevelTiles * 4 ;
       numQuadTreeTiles = numQuadTreeTiles + numQuadTreeLevelTiles ;
       numQuadTreePoints = numQuadTreePoints / 4 ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numQuadTreePoints = %8ld minTilePoints = %8ld numQuadTreeTiles = %8ld",numQuadTreePoints,minTilePoints,numQuadTreeTiles) ;
      }  
/*
**  Allocate Memory For QuadTree Tiles
*/
    memQuadTreeTiles = numQuadTreeTiles  ;
    *quadTreeTilesPP = ( DTM_QUAD_TREE_TILE * ) malloc( memQuadTreeTiles * sizeof(DTM_QUAD_TREE_TILE)) ;
    if( *quadTreeTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Create Quadtree Header Node
*/
    (*quadTreeTilesPP)->tileOffset    = startPoint ; 
    (*quadTreeTilesPP)->numTilePts    = numPoints ; 
    (*quadTreeTilesPP)->tileNumber    = 0 ; 
    (*quadTreeTilesPP)->quadTreeLevel = 0 ;
    (*quadTreeTilesPP)->childNodes[0] = -1 ;
    (*quadTreeTilesPP)->childNodes[1] = -1 ;
    (*quadTreeTilesPP)->childNodes[2] = -1 ;
    (*quadTreeTilesPP)->childNodes[3] = -1 ;
    (*quadTreeTilesPP)->parentNode    = -1 ;
    (*quadTreeTilesPP)->xMin = 0.0 ;
    (*quadTreeTilesPP)->xMax = 0.0 ;
    (*quadTreeTilesPP)->yMin = 0.0 ;
    (*quadTreeTilesPP)->yMax = 0.0 ;
    (*quadTreeTilesPP)->zMin = 0.0 ;
    (*quadTreeTilesPP)->zMax = 0.0 ;
    (*quadTreeTilesPP)->dtmP = NULL ;
    *numQuadTreeTilesP = 1 ;
/*
**  Create Quad Tree Tiles
*/
   if( bcdtmQuadTreeTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,*quadTreeTilesPP,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles) ) goto errexit ; 
  }
/*
** Create One Tile
*/
 else
   {
    *quadTreeTilesPP = ( DTM_QUAD_TREE_TILE * ) malloc( sizeof(DTM_QUAD_TREE_TILE)) ;
    if( *quadTreeTilesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    (*quadTreeTilesPP)->tileOffset   = 0 ; 
    (*quadTreeTilesPP)->numTilePts   = numPoints ; 
    (*quadTreeTilesPP)->quadTreeLevel =  0 ;
    (*quadTreeTilesPP)->childNodes[0] = -1 ;
    (*quadTreeTilesPP)->childNodes[1] = -1 ;
    (*quadTreeTilesPP)->childNodes[2] = -1 ;
    (*quadTreeTilesPP)->childNodes[3] = -1 ;
    *numQuadTreeTilesP = 1 ;
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
BENTLEYDTM_Private int bcdtmQuadTreeTile_sortAndTileTaggedPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long *tagP,
 DTM_QUAD_TREE_TILE *quadTreeTileP,
 long minTilePoints,
 DTM_QUAD_TREE_TILE **quadTreeTilesPP,
 long *numQuadTreeTilesP,
 long memQuadTreeTiles
)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if The x Coordiante Range Is Greater Than The y Coordinate Range  
** Sorts by y-coordinate if The y Coordiante Range Is Greater Than The x Coordinate Range     
**                                           
*/
{
/* 
** Uses A Divide And Conquer Startegy To Quad Tree Tile Points
*/
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   axis,divider,numTilePts,tileOffset ;
 double xRange,yRange,zRange ;
 DTM_QUAD_TREE_TILE *qtreeP ;
/*
** Only Process If There Are No Child Nodes
*/
 if( quadTreeTileP->childNodes[0] == -1 )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Parent Node ** Tile = %8ld ** offset = %8ld numTilePts = %8ld",quadTreeTileP->tileNumber,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts) ;
/*
**  Divide Into Data Sets
*/
    if( quadTreeTileP->numTilePts > minTilePoints * 4  ) 
      {
/*
**     Get Coordinate Ranges
*/
       if( bcdtmMedianTile_getXYZCoordinateRangesDtmObject(dtmP,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts,&xRange,&yRange,&zRange)) goto errexit ; 
/*
**     Set Axis
*/
       if( xRange > yRange ) axis = DTM_X_AXIS ;
       else                  axis = DTM_Y_AXIS ;
/*
**     Create Four Child Nodes
*/
       if( bcdtmQuadTreeTile_createQuadTreeChildNodes(quadTreeTileP->tileNumber,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles)) goto errexit ;
/*
**     Median Sort Tile Points. Firstly Divide The Whole Set And Secondly The Half Sets 
*/
       qtreeP = *quadTreeTilesPP + *numQuadTreeTilesP - 4 ;
       divider = qtreeP->numTilePts + (qtreeP+1)->numTilePts ;
       bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,quadTreeTileP->tileOffset,quadTreeTileP->numTilePts,divider,axis) ;
       numTilePts = divider ;
       divider    = qtreeP->numTilePts ;
       bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,quadTreeTileP->tileOffset,numTilePts,divider,axis) ;
       tileOffset = quadTreeTileP->tileOffset + numTilePts ;
       divider    = (qtreeP+2)->numTilePts ;
       numTilePts = (qtreeP+2)->numTilePts + (qtreeP+3)->numTilePts ;  
       bcdtmTile_medianSortTaggedPointsDtmObject(dtmP,tagP,tileOffset,numTilePts,divider,axis) ;
/*
**     Divide and Conquer 
*/
       for( qtreeP = *quadTreeTilesPP + *numQuadTreeTilesP - 4 ; qtreeP < *quadTreeTilesPP + *numQuadTreeTilesP ; ++qtreeP )
         { 
          if( bcdtmQuadTreeTile_sortAndTileTaggedPointsDtmObject(dtmP,tagP,qtreeP,minTilePoints,quadTreeTilesPP,numQuadTreeTilesP,memQuadTreeTiles) ) goto errexit ; 
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
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
