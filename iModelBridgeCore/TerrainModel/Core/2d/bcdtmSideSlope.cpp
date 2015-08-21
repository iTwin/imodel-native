/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmSideSlope.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"
#include "bcdtmSideSlope.h"

BENTLEYDTM_Public int bcdtmInsert_removeDtmFeatureFromDtmObject2 (BC_DTM_OBJ *dtmP, long dtmFeature, bool clearup = false);
thread_local static long processingLimits = 0;
thread_local static BC_DTM_OBJ *benchTinP = nullptr;



/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_createSideSlopesForSideSlopeTableDtmObject      |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_createSideSlopesForSideSlopeTableDtmObject
(
 DTM_SIDE_SLOPE_TABLE **SideSlopeTable,  /* ==> Table Containing The Parameters For Determing The Side Slopes */
 long *SideSlopeTableSize,               /* ==> Size Of Or Number Of Entries In The Side Slope Table  */
 long SideSlopeDirection,                /* ==> Direction To Calculate The Side Slopes 1 = Right 2 = Left */
 DTM_SLOPE_TABLE *SlopeTable,            /* ==> Slope Table For Assigning Side Slopes Slope  */
 long SlopeTableSize,                    /* ==> Size Of Or Number Of Entries In The Slope Table */
 long CornerOption,                      /* ==> Rounded Or Square Side Slope Corners. 1 = Rounded 2 = Square  */
 long StrokeCorners,                     /* ==> Stroke Side Slope Corners. 0  Dont Stroke 1 Stroke */
 double CornerStrokeTolerance,           /* ==> Linear Tolerance For Stroking Side Slope Corners  */
 double Pptol,                           /* ==> Point To Point Tolerance For Determing The Side Slopes */
 DPoint3d *ParallelEdgePts,                   /* ==> Parallel Edge Points For Truncating Side Slope Radials    */
 long NumParallelEdgePts,                /* ==> Number Of Parallel Edge Points */
 DTMUserTag UserRadialTag,             /* ==> User Tag To Be Assigned To Side Slope Radial Break Lines */
 DTMUserTag UserElementTag,            /* ==> User Tag To Be Assigned To Side Slope Element  */
 BC_DTM_OBJ* **DataObjects,              /* <== Array Of Pointers To The Created Side Slope Data Objects */
 long *NumberOfDataObjects               /* <== Size Of or Number Of Array Of Data Object Pointers  */
)
/*
**
** This Function Calculates Sides Slopes For A Side Slope Table
**
**
** Arguements
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    n,point,closedElement,numElemPts ;
 double  dist,angle ;
 wchar_t ssasc[10],SideSlopeFile[]=L"sideslopes0000.dat",SideSlopeElementFile[]=L"sideslopeElement00.dat" ;
 DPoint3d     *elemPtsP=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radial ;
 BC_DTM_OBJ *tempObjP=nullptr ;
 long    start ;
 static long numSideSlopes=0,numSideSlopeElements=0 ;
 DTM_TIN_POINT   *pointP ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
 // FILE *xyzFP=nullptr ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope OccurrenceIn A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
/*
** Set Value of DTM_PYE
*/
 DTM_PYE = atan2(0.0,-1.0) ;
 if( DTM_PYE < 0.0 ) DTM_PYE = -DTM_PYE ;
 DTM_2PYE = DTM_PYE * 2.0 ;
/*
** Timing Information For Side Slopes
*/
 start = bcdtmClock() ;
/*
** Write Status Message
*/
// bcdtmWrite_message(0,0,0,"Creating Side Slopes ** seqdbg = %8ld",seqdbg) ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"EXT Creating Side Slopes ** seqdbg = %8ld",seqdbg) ;
    bcdtmWrite_message(0,0,0,"SideSlopeTable            = %p",*SideSlopeTable) ;
    bcdtmWrite_message(0,0,0,"SideSlopeTableSize        = %8ld",*SideSlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"SideSlopeDirection        = %8ld",SideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"SlopeTable                = %p",SlopeTable) ;
    bcdtmWrite_message(0,0,0,"SlopeTableSize            = %8ld",SlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"CornerOption              = %8ld",CornerOption) ;
    bcdtmWrite_message(0,0,0,"Stroke Corners            = %8ld",StrokeCorners) ;
    bcdtmWrite_message(0,0,0,"CornerStrokeTolerance     = %8.4lf",CornerStrokeTolerance) ;
    bcdtmWrite_message(0,0,0,"Pptol                     = %8.6lf",Pptol) ;
    bcdtmWrite_message(0,0,0,"UserRadialTag             = %10I64d",UserRadialTag) ;
    bcdtmWrite_message(0,0,0,"UserElementTag            = %10I64d",UserElementTag) ;
    bcdtmWrite_message(0,0,0,"Pointer To Data Objects   = %p",*DataObjects) ;
    bcdtmWrite_message(0,0,0,"Number Of Data Objects    = %8ld",*NumberOfDataObjects) ;
    bcdtmWrite_message(0,0,0,"Parallel Points           = %p",ParallelEdgePts) ;
    bcdtmWrite_message(0,0,0,"Number Of Parallel Points = %8ld",NumParallelEdgePts) ;
   }
/*
** Write Out Side Slope Parameters
*/
 if( dbg == 1 )
   {
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** sideSlopeOption = %2ld cutFillOption = %2ld cutFillTin = %p **  %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->sideSlopeOption,radial->cutFillOption,radial->cutFillTin,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
    bcdtmWrite_message(0,0,0,"") ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** sideSlopeOption = %2ld cutSlope = %10.4lf fillSlope = %10.4lf forceSlope = %10.4lf **  %12.5lf %12.5lf %10.4lf" ,(long)(radial-*SideSlopeTable),radial->sideSlopeOption,radial->cutSlope,radial->fillSlope,radial->forcedSlope,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
 radial = *SideSlopeTable ;
 bcdtmWrite_message(0,0,0,"Radial[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
 radial = *SideSlopeTable + *SideSlopeTableSize - 1 ;
 bcdtmWrite_message(0,0,0,"Radial[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
*/
/*
** Write Side Slope Element
*/
 if( dbg == 1 )
   {
    if( bcdtmObject_createDtmObject(&tempObjP)) goto errexit ;
    if( bcdtmSideSlope_copySideSlopeElementPointsToPointArray(*SideSlopeTable,*SideSlopeTableSize,&elemPtsP,&numElemPts)) goto errexit ;

//xyzFP = bcdtmFile_open("alignment.xyz",L"wb") ;
//fwrite(elemPtsP,sizeof(DPoint3d),numElemPts,xyzFP) ;
//fclose(xyzFP) ;

    if( bcdtmObject_storeDtmFeatureInDtmObject(tempObjP,DTMFeatureType::Breakline,tempObjP->nullUserTag,1,&tempObjP->nullFeatureId,elemPtsP,numElemPts)) goto errexit ;
    if( elemPtsP != nullptr ) { free(elemPtsP) ; elemPtsP = nullptr ; }
    swprintf(ssasc,10,L"%2ld",seqdbg) ;
    if( ssasc[0] == ' ' ) ssasc[0] = '0' ;
    SideSlopeElementFile[16] = ssasc[0] ;
    SideSlopeElementFile[17] = ssasc[1] ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(tempObjP,SideSlopeElementFile)) goto errexit ;
    if( tempObjP != nullptr ) bcdtmObject_destroyDtmObject(&tempObjP) ;
   }
/*
** Write Parallel Edges
*/
 if( dbg == 3 )
   {
    if( ParallelEdgePts != nullptr && NumParallelEdgePts > 0 )
      {
       if( bcdtmObject_createDtmObject(&tempObjP)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(tempObjP,DTMFeatureType::Breakline,tempObjP->nullUserTag,1,&tempObjP->nullFeatureId,ParallelEdgePts,NumParallelEdgePts)) goto errexit ;
       if( tempObjP != nullptr ) bcdtmObject_destroyDtmObject(&tempObjP) ;
      }
   }
/*
** Initialise
*/
 benchTinP=nullptr ;
 if( dbg ) bcdtmWrite_message(0,0,0,"00 Number Of Side Slopes = %8ld ** sideSlopeArrayP = %p",*NumberOfDataObjects,*DataObjects) ;
 *NumberOfDataObjects = 0 ;
 if( *DataObjects != nullptr ) { free(*DataObjects) ; *DataObjects = nullptr ; }
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
   {
    radial->radialStatus     = 1 ;
    radial->radialType       = 0 ;
    radial->radialGenesis    = 1 ;
    radial->radialSolution   = 0 ;
    radial->radialAngle      = 0.0 ;
    radial->radialSlope      = 0.0 ;
    radial->radialEndPoint.x = 0.0 ;
    radial->radialEndPoint.y = 0.0 ;
    radial->radialEndPoint.z = 0.0 ;
    radial->surfaceZ         = 0.0 ;
   }
/*
** Validate Input Parameters
*/
 if( CornerOption       < 1 || CornerOption       > 2 ) { bcdtmWrite_message(1,0,0,"Invalid Value %4ld For Corner Option",CornerOption) ; goto errexit ; }
 if( Pptol <= 0.0 ) { bcdtmWrite_message(1,0,0,"Invalid Value %10.8lf For Pptol",Pptol) ; goto errexit ; }
/*
** Pull Element Points Onto Tin ** Fudge Fix For MS Conversion Of Coordinates From V7 To V8
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Pulling Side Slope Element Points Onto Tin") ;
 if( bcdtmSideSlope_pullSideSlopePointsOntoTin(*SideSlopeTable,*SideSlopeTableSize)) goto errexit ;
/*
** Test For Closed Side Slope Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Closed Side Slope Element") ;
 closedElement = 0 ;
 if( bcdtmMath_distance((*SideSlopeTable)->radialStartPoint.x,(*SideSlopeTable)->radialStartPoint.y,(*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.x,(*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.y) < Pptol )
   {
    (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.x = (*SideSlopeTable)->radialStartPoint.x ;
    (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.y = (*SideSlopeTable)->radialStartPoint.y ;
   }
 if( (*SideSlopeTable)->radialStartPoint.x == (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.x &&
     (*SideSlopeTable)->radialStartPoint.y == (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.y     ) closedElement = 1 ;
 if( closedElement ) (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.z =  (*SideSlopeTable)->radialStartPoint.z ;
 if( closedElement && *SideSlopeTableSize == 3 ) { closedElement = 0 ; --*SideSlopeTableSize ; }
 if( dbg )
   {
    if( closedElement ) bcdtmWrite_message(0,0,0,"Side Slope Element Closed") ;
    else                bcdtmWrite_message(0,0,0,"Side Slope Element Open") ;
   }
/*
**  Validate The Side Slope Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Side Slope Element") ;
 if( bcdtmSideSlope_validateSideSlopeElement(*SideSlopeTable,SideSlopeTableSize,Pptol))  goto errexit ;
/*
**  Set Force Slope For Element Points Not On Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Element Points Not On Tin") ;
 if( bcdtmSideSlope_setForceSlopeForVerticesNotOnTin(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection,closedElement)) goto errexit ;
/*
**  Set Force Slope For Element Segments Not On Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Element Segments Not On Tin") ;
 if( bcdtmSideSlope_setForceSlopeForSegmentsNotOnTin(*SideSlopeTable,*SideSlopeTableSize)) goto errexit ;
/*
** If Element Open And Parallel Points Present, Extend Ends Of Parallel points
*/
 if( ! closedElement && ParallelEdgePts != nullptr )
   {
    dist  = bcdtmMath_distance((ParallelEdgePts+1)->x,(ParallelEdgePts+1)->y,ParallelEdgePts->x,ParallelEdgePts->y) ;
    angle = bcdtmMath_getAngle((ParallelEdgePts+1)->x,(ParallelEdgePts+1)->y,ParallelEdgePts->x,ParallelEdgePts->y) ;
    ParallelEdgePts->x = (ParallelEdgePts+1)->x + (dist+0.5) * cos(angle) ;
    ParallelEdgePts->y = (ParallelEdgePts+1)->y + (dist+0.5) * sin(angle) ;
    dist  = bcdtmMath_distance((ParallelEdgePts+NumParallelEdgePts-2)->x,(ParallelEdgePts+NumParallelEdgePts-2)->y,(ParallelEdgePts+NumParallelEdgePts-1)->x,(ParallelEdgePts+NumParallelEdgePts-1)->y) ;
    angle = bcdtmMath_getAngle((ParallelEdgePts+NumParallelEdgePts-2)->x,(ParallelEdgePts+NumParallelEdgePts-2)->y,(ParallelEdgePts+NumParallelEdgePts-1)->x,(ParallelEdgePts+NumParallelEdgePts-1)->y) ;
    (ParallelEdgePts+NumParallelEdgePts-1)->x = (ParallelEdgePts+NumParallelEdgePts-2)->x + (dist+0.5) * cos(angle) ;
    (ParallelEdgePts+NumParallelEdgePts-1)->y = (ParallelEdgePts+NumParallelEdgePts-2)->y + (dist+0.5) * sin(angle) ;
   }
/*
** Insert Elevation Transitions Points Into Side Slope Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Elevation Transition Points Into SideSlopeTable") ;
 if( bcdtmSideSlope_insertVerticesAtElevationTransitions(SideSlopeTable,SideSlopeTableSize)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Mark Radials Intactive That Cannot Satisfy A Forced Slope To A Delta Elevation
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Radials Intactive That Cannot Satisfy A Forced Slope To A Delta Elevation") ;
 if( bcdtmSideSlope_markInactiveDeltaVerticalRadials(*SideSlopeTable,*SideSlopeTableSize)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Insert Cut/Fill Transitions Points Into Side Slope Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Cut/Fill Transition Points Into SideSlopeTable") ;
 if( bcdtmSideSlope_insertVerticesAtCutFillTransitions(SideSlopeTable,SideSlopeTableSize,Pptol)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Insert Transition Points For Slope To Object Into Side Slope Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Transition Points For Slope To Object") ;
 if( bcdtmSideSlope_insertTransitionVerticesForSlopeToObject(SideSlopeTable,SideSlopeTableSize,Pptol)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Create Side Slopes For Active Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Active Radials") ;
 if( bcdtmSideSlope_createSideSlopesForActiveRadials(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection,SlopeTable,SlopeTableSize,CornerOption,StrokeCorners,CornerStrokeTolerance,Pptol,ParallelEdgePts,NumParallelEdgePts,UserRadialTag,UserElementTag,DataObjects,NumberOfDataObjects)) goto errexit ;
/*
** Round Side Slope Elevation Values
*/
 for( n = 0 ; n < *NumberOfDataObjects ; ++n )
   {
    if( ( tempObjP = *(*DataObjects+n)) != nullptr )
      {
       for(  point = 0  ; point  < tempObjP->numPoints ; ++point )
         {
          pointP = pointAddrP(tempObjP,point) ;
          pointP->z = bcdtmMath_roundToDecimalPoints(pointP->z,8) ;
         }
       tempObjP = nullptr ;
      }
   }
/*
** Write Slope Toes From Side Slope Objects Development Only
*/
 if( dbg == 1 )
   {
    BC_DTM_FEATURE *dtmFeatureP ;
    long numFeaturePts=0 ;
    DPoint3d  *p3dP,*featurePtsP=nullptr ;
    for( n = 0 ; n < *NumberOfDataObjects ; ++n )
      {
       if( ( tempObjP = *(*DataObjects+n)) != nullptr )
         {
          bcdtmWrite_message(0,0,0,"Side Slope[%4ld]",n+1) ;
          for(  point = 0  ; point  < tempObjP->numFeatures ; ++point )
            {
             dtmFeatureP = ftableAddrP(tempObjP,point) ;
             if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::SlopeToe )
               {
                if( ! bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(tempObjP,point,&featurePtsP,&numFeaturePts))
                  {
                   for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
                     {
                      bcdtmWrite_message(0,0,0,"**** Slope Toe Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
                     }
                  }
                if( featurePtsP != nullptr )
                  {
                   free(featurePtsP) ;
                   featurePtsP = nullptr ;
                  }
               }
            }
         }
         tempObjP = nullptr ;
      }
   }
/*
** Write Data Objects To File ** For Debugging Purposes
*/
 if( dbg == 1  )
   {
    numSideSlopes = 0 ;
    bcdtmWrite_message(0,0,0,"Number Of Data Objects = %2ld",*NumberOfDataObjects) ;
    for( n = 0 ; n < *NumberOfDataObjects ; ++n )
      {
       if( n < 99 )
         {
          ++numSideSlopes ;
          swprintf(ssasc,10,L"%2ld%2ld",seqdbg,numSideSlopes) ;
          if( ssasc[0] == ' ' ) ssasc[0] = '0' ;
          if( ssasc[2] == ' ' ) ssasc[2] = '0' ;
          SideSlopeFile[10] = ssasc[0] ;
          SideSlopeFile[11] = ssasc[1] ;
          SideSlopeFile[12] = ssasc[2] ;
          SideSlopeFile[13] = ssasc[3] ;
          bcdtmWrite_geopakDatFileFromDtmObject(*(*DataObjects+n),SideSlopeFile) ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( elemPtsP != nullptr ) { free(elemPtsP) ; elemPtsP = nullptr ; }
 if( tempObjP != nullptr ) bcdtmObject_destroyDtmObject(&tempObjP) ;
/*
** Timing Information
*/
// bcdtmWrite_message(0,0,0,"Time To Create Side Slopes = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),start)) ;
/*
** Normal Return
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Side Slopes = %8ld ** sideSlopeArrayP = %p",*NumberOfDataObjects,*DataObjects) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Side Slopes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Side Slopes Error") ;
// if( ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Side Slopes Completed") ;
// if( ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Side Slopes Error") ;
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
int bcdtmSideSlope_copySideSlopeElementPointsToPointArray
(
 DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,
 long                 sideSlopeTableSize,
 DPoint3d                  **elemPtsPP,
 long                 *numElemPtsP
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 DPoint3d *p3dP ;
 DTM_SIDE_SLOPE_TABLE *tableP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Side Slope Element Points To Point Array") ;
/*
** Initialise
*/
 *numElemPtsP = 0 ;
 if( *elemPtsPP != nullptr ) { free(*elemPtsPP) ; *elemPtsPP = nullptr ; }
 if( sideSlopeTableP == nullptr || sideSlopeTableSize <= 0 )
   {
    bcdtmWrite_message(2,0,0,"Empty Side Slope Table") ;
    goto errexit ;
   }
 *numElemPtsP = sideSlopeTableSize ;
/*
** Allocate Memory
*/
 *elemPtsPP = ( DPoint3d * ) malloc( *numElemPtsP * sizeof(DPoint3d)) ;
 if( *elemPtsPP == nullptr )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Error") ;
    goto errexit ;
   }
/*
** Copy Points
*/
 for( p3dP = *elemPtsPP , tableP = sideSlopeTableP ; tableP < sideSlopeTableP + sideSlopeTableSize ; ++p3dP , ++tableP )
   {
    p3dP->x = tableP->radialStartPoint.x ;
    p3dP->y = tableP->radialStartPoint.y ;
    p3dP->z = tableP->radialStartPoint.z ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Side Slope Element Points To Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Side Slope Element Points To Point Array Error") ;
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
int bcdtmSideSlope_pullSideSlopePointsOntoTin(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize)
/*
** This Function Pulls A Side Slope Point Onto The Cut/Fill Or Slope To Tin
** If The Distance Of The Side Slope Point From A Tin Point
** Is Less Than Or Equal To The Machine Precision Of The Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long   cp ;
 double dp ;
 DTM_SIDE_SLOPE_TABLE *radial ;
 BC_DTM_OBJ *dtmP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Pulling Side Slope Points Onto Tin") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Determine Cut/Fill Transition Points
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize - 1 ; ++radial )
   {
/*
** Test For Cut Fill Tin
*/
    if( radial->cutFillTin != nullptr )
      {
       dtmP = (BC_DTM_OBJ *) radial->cutFillTin ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"cutFillTin = %p",dtmP) ;
       if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
       if( bcdtmFind_closestPointDtmObject(dtmP,radial->radialStartPoint.x,radial->radialStartPoint.y,&cp) == 1 )
         {
          radial->radialStartPoint.x = pointAddrP(dtmP,cp)->x ;
          radial->radialStartPoint.y = pointAddrP(dtmP,cp)->y ;
         }
       else if( ( dp = bcdtmMath_distance(radial->radialStartPoint.x,radial->radialStartPoint.y,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y)) <= dtmP->mppTol )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Side Slope Point Pulled Onto Cut/Fill Tin ** dp = %20.15lf Mpptol = %20.15lf",dp,radial->cutFillTin->mppTol ) ;
          radial->radialStartPoint.x = pointAddrP(dtmP,cp)->x ;
          radial->radialStartPoint.y = pointAddrP(dtmP,cp)->y ;
         }
      }
/*
** Test For Slope To Tin
*/
    if( radial->slopeToTin != nullptr )
      {
       dtmP = (BC_DTM_OBJ *) radial->slopeToTin ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"slopeToTin = %p",dtmP) ;
       if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
       if( bcdtmFind_closestPointDtmObject(dtmP,radial->radialStartPoint.x,radial->radialStartPoint.y,&cp) == 1 )
         {
          radial->radialStartPoint.x = pointAddrP(dtmP,cp)->x ;
          radial->radialStartPoint.y = pointAddrP(dtmP,cp)->y ;
         }
       else if( ( dp = bcdtmMath_distance(radial->radialStartPoint.x,radial->radialStartPoint.y,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y)) <= dtmP->mppTol )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Side Slope Point Pulled Onto Slope To Tin ** dp = %20.15lf Mpptol = %20.15lf",dp,radial->slopeToTin->mppTol ) ;
          radial->radialStartPoint.x = pointAddrP(dtmP,cp)->x ;
          radial->radialStartPoint.y = pointAddrP(dtmP,cp)->y ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pulling Side Slope Points Onto Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pulling Side Slope Points Onto Tin Error") ;
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
|  bcdtmSideSlope_validateSideSlopeElement()                           |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_validateSideSlopeElement(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long *SideSlopeTableSize,double Pptol)
/*
** This Function Validates a Side Slope Element
*/
{
 int     ret=0 ;
 long    dbg=0,KnotDetected ;
 DTMDirection Direction;
 double  Area ;
 DTM_SIDE_SLOPE_TABLE *radial ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Side Slope Element") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of SideSlopeTable Points = %6ld",*SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] = %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Eliminate Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Eliminating Duplicate Side Slope Element Points") ;
 bcdtmSideSlope_deleteDuplicateSideSlopeElementPoints(SideSlopeTable,SideSlopeTableSize,Pptol) ;
/*
** Check For Knots
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Knots In Side Slope Element") ;
 bcdtmSideSlope_checkForKnotsInSideSlopeElement(SideSlopeTable,*SideSlopeTableSize,&KnotDetected ) ;
 if( KnotDetected )
   {
    bcdtmWrite_message(1,0,0,"Knots Detected In Side Slope Element") ;
    goto errexit  ;
   }
/*
** If SideSlopeTable Closed Check And Set Direction Of SideSlopeTable Anti Clockwise
*/
 if( (SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.x == SideSlopeTable->radialStartPoint.x &&  (SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.y == SideSlopeTable->radialStartPoint.y )
   {
/*
** Get SideSlopeTable Direction
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking SideSlopeTable Direction") ;
    bcdtmSideSlope_getSideSlopeElementDirection(SideSlopeTable,*SideSlopeTableSize,&Direction,&Area) ;
/*
** Reverse Direction Of SideSlopeTable If Clockwise
*/
    if( Direction == DTMDirection::Clockwise )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Element Direction Anti Clockwise") ;
       bcdtmSideSlope_reverseSideSlopeTableDirection(SideSlopeTable,*SideSlopeTableSize) ;
      }
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Validating Side Slope Element Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Validating Side Slope Element Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_deleteDuplicateSideSlopeElementPoints(DTM_SIDE_SLOPE_TABLE *Points,long *numPts,double Pptol)
{
 double dx,dy ;
 DTM_SIDE_SLOPE_TABLE  *p3d1,*p3d2   ;
/*
** Initialise Variables
*/
 if( Pptol < 0.0 ) Pptol = -Pptol ;
/*
** Eliminate Points Within Pptol
*/
 for( p3d1 = Points, p3d2 = p3d1 + 1 ; p3d2 < Points + *numPts ; ++p3d2 )
   {
    dx = p3d2->radialStartPoint.x - p3d1->radialStartPoint.x ;
    dy = p3d2->radialStartPoint.y - p3d1->radialStartPoint.y ;
    if( sqrt(dx*dx + dy*dy) > Pptol )
      {
       ++p3d1 ;
       if( p3d1 != p3d2 ) *p3d1 = *p3d2 ;
      }
   }
 *numPts = (long) (p3d1-Points) + 1 ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_checkForKnotsInSideSlopeElement                      |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_checkForKnotsInSideSlopeElement(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long *KnotFlag )
/*
** This Function Checks For Knots In The Pad Element Table
*/
{
 int             ret=0 ;
 long            dbg=0,StringPtsNe,KnotPtsNe ;
 DPoint3d             *p3d,*StringPts=nullptr ;
 DTM_STR_INT_PTS *KnotPts=nullptr ;
 DTM_SIDE_SLOPE_TABLE *pad ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Knots In Side Slope Element ** Number Element Segments = %6ld",SideSlopeTableSize-1) ;
/*
** Allocate Memory For DPoint3d String
*/
 StringPtsNe = SideSlopeTableSize ;
 StringPts = (DPoint3d *)  malloc( StringPtsNe * sizeof(DPoint3d)) ;
 if( StringPts == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Pad Points To String Points
*/
 for( pad = SideSlopeTable , p3d = StringPts ; pad < SideSlopeTable + SideSlopeTableSize ; ++pad , ++p3d )
   {
    p3d->x = pad->radialStartPoint.x ;
    p3d->y = pad->radialStartPoint.y ;
    p3d->z = pad->radialStartPoint.z ;
   }
/*
**  Check DPoint3d String For Knots
*/
 if( bcdtmClean_checkP3DStringForKnots(StringPts,StringPtsNe,&KnotPts,&KnotPtsNe) ) goto errexit ;
/*
** Set Knot Flag
*/
 *KnotFlag = KnotPtsNe ;
/*
** Clean Up
*/
 cleanup :
 if( KnotPts   != nullptr ) free(KnotPts) ;
 if( StringPts != nullptr ) free(StringPts) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Checking For Knots In Side Slope Element Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Checking For Knots In Side Slope Element Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_getSideSlopeElementDirection(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,DTMDirection *Direction,double *Area)
/*
**  This Function Determines The Direction Of A SideSlopeTable
*/
 {
  double x,y   ;
  DTM_SIDE_SLOPE_TABLE *radial ;
/*
** Initialise Varaibles
*/
  *Area = 0.0 ;
/*
** Sum Area Of SideSlopeTable
*/
  for ( radial = SideSlopeTable + 1 ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
    {
     x = radial->radialStartPoint.x - (radial-1)->radialStartPoint.x ;
     y = radial->radialStartPoint.y - (radial-1)->radialStartPoint.y ;
     *Area = *Area + x * y / 2.0 + x * (radial-1)->radialStartPoint.y ;
    }
/*
** Set Direction
*/
 if( *Area >= 0.0 ) *Direction = DTMDirection::Clockwise ;         /* ClockWise */
 else  { *Direction = DTMDirection::AntiClockwise ; *Area = -*Area ; } /* Anti Clockwise */
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
int bcdtmSideSlope_reverseSideSlopeTableDirection(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize)
/*
**  This Function Reverses The Direction Of a SideSlopeTable
*/
{
 DTM_SIDE_SLOPE_TABLE *radial1,*radial2,*radial3,radial ;
/*
** Swap Coordinates
*/
 radial1 = SideSlopeTable ;
 radial2 = SideSlopeTable + SideSlopeTableSize - 1 ;
 radial3 = &radial ;
 while ( radial1 < radial2 )
   {
    *radial3 = *radial1 ;
    *radial1 = *radial2 ;
    *radial2 = *radial3 ;
    ++radial1 ; --radial2 ;
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
int bcdtmSideSlope_setForceSlopeForVerticesNotOnTin(DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,long sideSlopeTableSize,long sideSlopeDirection,long closeFlag)
/*
** This Function Checks If The Side Slope Vertices Are On The Tin
** The Force Slope And Force Slope Option Is Set For Any Vertice Not
** On The Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   drapeResult,intersectType=0,p1,p2 ;
 double x,y,z,dx,dy,dd,sx,sy,ex,ey,nextAngle=0.0,priorAngle=0.0,radialAngle ;
 DTM_SIDE_SLOPE_TABLE *radP,*radialP,*nextRadialP,*priorRadialP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Vertices Not On Tin") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of  Vertices = %6ld",sideSlopeTableSize) ;
    for( radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++radialP )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] = %10.4lf %10.4lf %10.4lf",(long)(radialP-sideSlopeTableP),radialP->radialStartPoint.x,radialP->radialStartPoint.y,radialP->radialStartPoint.z) ;
      }
   }
/*
** Scan Side Slope Table For Vertices Not On Tin
*/
 for( radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++radialP )
   {
    if( ! radialP->isForceSlope )
      {
       if( radialP->sideSlopeOption  == 1 || ( radialP->sideSlopeOption  >= 5 && radialP->sideSlopeOption  <= 7 ) )
         {
          if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *) radialP->slopeToTin,radialP->radialStartPoint.x,radialP->radialStartPoint.y,&z,&drapeResult)) goto errexit ;
          if( drapeResult != 1 )
            {
/*
**           Write Vertice
*/
             bcdtmWrite_message(3,0,0,"Side Slope Element Vertice External To Tin Or In Void ** %10.4lf %10.4lf %10.4lf",radialP->radialStartPoint.x,radialP->radialStartPoint.y,radialP->radialStartPoint.z) ;
/*
**           Get Next And Prior Radials
*/
             nextRadialP  = radialP + 1 ;
             priorRadialP = radialP - 1 ;
             if( nextRadialP >= sideSlopeTableP + sideSlopeTableSize )
               {
                if( closeFlag ) nextRadialP = sideSlopeTableP + 1 ;
                else            nextRadialP = nullptr ;
               }
             if( priorRadialP < sideSlopeTableP )
               {
                if( closeFlag ) priorRadialP = sideSlopeTableP + sideSlopeTableSize - 2 ;
                else            priorRadialP = nullptr ;
               }
/*
**           Calculate Angles To Next And Prior Radials
*/
             if( nextRadialP  != nullptr ) nextAngle  = bcdtmMath_getAngle(radialP->radialStartPoint.x,radialP->radialStartPoint.y,nextRadialP->radialStartPoint.x,nextRadialP->radialStartPoint.y) ;
             if( priorRadialP != nullptr ) priorAngle = bcdtmMath_getAngle(radialP->radialStartPoint.x,radialP->radialStartPoint.y,priorRadialP->radialStartPoint.x,priorRadialP->radialStartPoint.y) ;
/*
**           Calculate Radial Side Slope Angle
*/
             if( nextRadialP == nullptr )
               {
                if( sideSlopeDirection == 1 ) radialAngle = priorAngle + DTM_PYE / 2.0 ;
                else                          radialAngle = priorAngle - DTM_PYE / 2.0 ;
               }
             else if( priorRadialP == nullptr )
               {
                if( sideSlopeDirection == 1 ) radialAngle = nextAngle - DTM_PYE / 2.0 ;
                else                          radialAngle = nextAngle + DTM_PYE / 2.0 ;
               }
             else
               {
                if( sideSlopeDirection == 1 )
                  {
                   if( nextAngle < priorAngle ) nextAngle = nextAngle + DTM_2PYE ;
                   radialAngle = ( priorAngle+nextAngle) / 2.0 ;
                  }
                else
                  {
                   if( priorAngle < nextAngle ) priorAngle = priorAngle + DTM_2PYE ;
                   radialAngle = ( priorAngle+nextAngle) / 2.0 ;
                  }
               }
/*
**           Normalise Radial Angle
*/
             radialAngle = bcdtmMath_normaliseAngle(radialAngle) ;
/*
**           Calculate Radial Endpoints For Intersecting With Tin Hull
*/
             dx = radialP->slopeToTin->xMax - radialP->slopeToTin->xMin ;
             dy = radialP->slopeToTin->yMax - radialP->slopeToTin->yMin ;
             dd = sqrt( dx*dx + dy*dy ) ;
             sx = radialP->radialStartPoint.x ;
             sy = radialP->radialStartPoint.y ;
             ex = sx + dd * cos(radialAngle)  ;
             ey = sy + dd * sin(radialAngle)  ;
/*
**           Truncate Radial To Side Slope Element
*/
             for( radP = sideSlopeTableP ; radP < sideSlopeTableP + sideSlopeTableSize - 1 ; ++radP )
               {
                if( radP != radialP && radP+1 != radialP  && ( ! closeFlag || ( closeFlag &&  ( radialP != sideSlopeTableP || radP+1 != sideSlopeTableP + sideSlopeTableSize - 1 ) &&  ( radialP != sideSlopeTableP + sideSlopeTableSize - 1 && radP != sideSlopeTableP) )))
                  {
/*
**                 Check For Intersection With Side Slope Segment
*/
                   if( bcdtmMath_checkIfLinesIntersect(sx,sy,ex,ey,radP->radialStartPoint.x,radP->radialStartPoint.y,(radP+1)->radialStartPoint.x,(radP+1)->radialStartPoint.y) )
                     {
/*
**                    Intersect Lines
*/
                      bcdtmMath_normalIntersectCordLines(sx,sy,ex,ey,radP->radialStartPoint.x,radP->radialStartPoint.y,(radP+1)->radialStartPoint.x,(radP+1)->radialStartPoint.y,&x,&y) ;
                      ex = x ;
                      ey = y ;
                     }
                  }
               }
/*
**           Intersect Radial With Tin Hull
*/
             if( drapeResult == 0 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Intecept With Tin Hull") ;
                p1 = p2 = radialP->slopeToTin->nullPnt ;
                if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject((BC_DTM_OBJ *)radialP->slopeToTin,sx,sy,ex,ey,&intersectType,&p1,&p2,&x,&y,&z) ) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Intercept Type = %2ld ** Intercept Point = %12.5lf %12.5lf %10.4lf",intersectType,x,y,z) ;
               }
/*
**           Intersect Radial With Void Hull
*/
             if( drapeResult == 2 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Intecept With Void Hull") ;
                p1 = p2 = radialP->slopeToTin->nullPnt ;
                if( bcdtmDrape_findClosestLineInterceptWithVoidHullDtmObject((BC_DTM_OBJ *)radialP->slopeToTin,sx,sy,ex,ey,&intersectType,&p1,&p2,&x,&y,&z) ) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Intercept Type = %2ld ** Intercept Point = %12.5lf %12.5lf %10.4lf",intersectType,x,y,z) ;
               }
/*
**           If Intersection Not Return Error
*/
             if( intersectType == 0 )
               {
               // Flag the radial as  "Do Not Calculate Side Slope"
               radialP->radialStatus = 0;
               }
/*
**           Turn Force Slope On For Subsequent Radial Intersection With Tin Surface
*/
             else
               {
                radialP->isForceSlope = 1 ;
                if( z >= radialP->radialStartPoint.z ) radialP->forcedSlope =  radialP->cutSlope ;
                else                                   radialP->forcedSlope = -radialP->fillSlope ;
               }
            }
         }
      }
   }
/*
**  Adjust For Closed Element
*/
 if( closeFlag )
   {
    (sideSlopeTableP+sideSlopeTableSize-1)->isForceSlope = sideSlopeTableP->isForceSlope ;
    (sideSlopeTableP+sideSlopeTableSize-1)->forcedSlope  = sideSlopeTableP->forcedSlope  ;
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Vertices Not On Tin Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Vertices Not On Tin Error") ;
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
int bcdtmSideSlope_setForceSlopeForSegmentsNotOnTin(DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,long sideSlopeTableSize)
/*
** This Function Checks If The Side Slope Segments Are On The Tin
** The Force Slope And Force Slope Option Is Set For Segment Vertices
** If The Segment Is Not On The Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   drapeResult,numDrapePts ;
 double z ;
 DPoint3d    segPtsP[2] ;
 DTM_DRAPE_POINT      *drapeP,*drapePtsP=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radialP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Segments Not On Tin") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of  Vertices = %6ld",sideSlopeTableSize) ;
    for( radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++radialP )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] = %10.4lf %10.4lf %10.4lf",(long)(radialP-sideSlopeTableP),radialP->radialStartPoint.x,radialP->radialStartPoint.y,radialP->radialStartPoint.z) ;
      }
   }
/*
** Check For Side Slope Segments Not On Tin
*/
 for( radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize - 1 ; ++radialP )
   {
    if( ! radialP->isForceSlope && ! (radialP+1)->isForceSlope )
      {
       if( ( radialP->sideSlopeOption      == 1 || ( radialP->sideSlopeOption      >= 5 && radialP->sideSlopeOption      <= 7 ) )  &&
           ( (radialP+1)->sideSlopeOption  == 1 || ( (radialP+1)->sideSlopeOption  >= 5 && (radialP+1)->sideSlopeOption  <= 7 ) )      )
         {
/*
**        Drape Element Segment On Tin
*/
          segPtsP[0].x = radialP->radialStartPoint.x ;
          segPtsP[0].y = radialP->radialStartPoint.y ;
          segPtsP[1].x = (radialP+1)->radialStartPoint.x ;
          segPtsP[1].y = (radialP+1)->radialStartPoint.y ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Segment ** %12.5lf %12.5lf ** %12.5lf %12.5lf",segPtsP[0].x,segPtsP[0].y,segPtsP[1].x,segPtsP[1].y) ;
          if( bcdtmDrape_stringDtmObject((BC_DTM_OBJ *)radialP->slopeToTin,segPtsP,2,false,&drapePtsP,&numDrapePts)) goto errexit ;
/*
**       Check For Segment Or Part Of Not On Tin
*/
          drapeP = drapePtsP ;
          while (  drapeP < drapePtsP + numDrapePts && drapeP->drapeType != DTMDrapedLineCode::External ) ++drapeP ;
          if( drapeP >= drapePtsP + numDrapePts ) --drapeP ;
/*
**       Set Force Slope If Drape Is Not On Tin Surface
*/
          if( drapeP->drapeType == DTMDrapedLineCode::External)
            {
             bcdtmWrite_message(3,0,0,"Segment Not On Tin Surface ** %12.5lf %12.5lf ** %12.5lf %12.5lf",segPtsP[0].x,segPtsP[0].y,segPtsP[1].x,segPtsP[1].y) ;
             radialP->isForceSlope = 1 ;
             if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *) radialP->slopeToTin,radialP->radialStartPoint.x,radialP->radialStartPoint.y,&z,&drapeResult)) goto errexit ;
             if( z >= radialP->radialStartPoint.z ) radialP->forcedSlope =  radialP->cutSlope ;
             else                                   radialP->forcedSlope = -radialP->fillSlope ;
             (radialP+1)->isForceSlope = 1 ;
             if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)(radialP+1)->slopeToTin,(radialP+1)->radialStartPoint.x,(radialP+1)->radialStartPoint.y,&z,&drapeResult)) goto errexit ;
             if( z >= (radialP+1)->radialStartPoint.z ) (radialP+1)->forcedSlope =  (radialP+1)->cutSlope ;
             else                                       (radialP+1)->forcedSlope = -(radialP+1)->fillSlope ;
            }
          if( drapePtsP != nullptr ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
         }
      }
   }
/*
** Job Completed
*/
 cleanup :
 if( drapePtsP != nullptr ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Segments Not On Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Force Slope For Segments Not On Tin Error") ;
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
|  bcdtmSideSlope_insertVerticesAtElevationTransitions                 |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_insertVerticesAtElevationTransitions(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize)
/*
** This Function Inserts Points Into The Side Slope Table At Elevation Transitions
**
**  SideSlopeTable        ==>  Pointer To Side Slope Table
**  SideSlopeTableSize    ==>  Size Of Side Slope Table
**
*/
{
 int       ret=0 ;
 long      dbg=0,ofs,MemSideSlopeTableSize,MemRadInc=100 ;
 double    dx,dy,dz,dz1,dz2,Xt,Yt,Zt ;
 DTM_SIDE_SLOPE_TABLE *radial,*radialofs ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Side Slope Table Elevation Transitions") ;
/*
** Write Elevation Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Initialise
*/
 MemSideSlopeTableSize = *SideSlopeTableSize ;
/*
** Determine Transition Points
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize - 1 ; ++radial )
   {
    if( ( radial->sideSlopeOption == 2 && (radial+1)->sideSlopeOption == 2 ) || ( radial->sideSlopeOption == 5 && (radial+1)->sideSlopeOption == 5 ) )
      {
/*
** Write Radials That Are Being Tested For Possibilty Of Elevation Transition Point
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Testing For Elevation Transition Points Between Radials") ;
          bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Genesis = %2ld Tin = %p ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialGenesis,radial->slopeToTin,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
          bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Genesis = %2ld Tin = %p ** %10.4lf %10.4lf %10.4lf",(long)(radial+1-*SideSlopeTable),(radial+1)->radialGenesis,(radial+1)->slopeToTin,(radial+1)->radialStartPoint.x,(radial+1)->radialStartPoint.y,(radial+1)->radialStartPoint.z) ;
         }
/*
** Get Elevation Differences
*/
       dz1 = radial->radialStartPoint.z - radial->toElev ;
       dz2 = (radial+1)->radialStartPoint.z - (radial+1)->toElev ;
/*
** Check For Elevation Transition Point
*/
       if(( dz1 > 0.0 && dz2 < 0.0 ) || ( dz1 < 0.0 && dz2 > 0.0 ) )
         {
/*
** Calculate Elevation Transition Point
*/
          if( dz1 < 0.0 ) dz1 = -dz1 ;
          if( dz2 < 0.0 ) dz2 = -dz2 ;
          dx = (radial+1)->radialStartPoint.x - radial->radialStartPoint.x ;
          dy = (radial+1)->radialStartPoint.y - radial->radialStartPoint.y ;
          dz = (radial+1)->radialStartPoint.z - radial->radialStartPoint.z ;
          Xt = radial->radialStartPoint.x + dx * dz1 / ( dz1+dz2) ;
          Yt = radial->radialStartPoint.y + dy * dz1 / ( dz1+dz2) ;
          Zt = radial->radialStartPoint.z + dz * dz1 / ( dz1+dz2) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Transition Point Found ** %10.4lf %10.4lf %10.4lf",Xt,Yt,Zt) ;
/*
** Check And Reallocate If Necessary, Radial Table Memmory
*/
          ofs = (long) ( radial - *SideSlopeTable ) ;
          if( *SideSlopeTableSize == MemSideSlopeTableSize )
            {
             MemSideSlopeTableSize =  MemSideSlopeTableSize + MemRadInc ;
             *SideSlopeTable = (DTM_SIDE_SLOPE_TABLE*) realloc( *SideSlopeTable , MemSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
             if( *SideSlopeTable == nullptr ) { free( SideSlopeTable) ; bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
** Copy Radial Table Entries Down One Entry From Radofs
*/
          radial = *SideSlopeTable + *SideSlopeTableSize ;
          radialofs = *SideSlopeTable + ofs ;
          while( radial > radialofs + 1 ) { *radial = *(radial-1) ; --radial ; }
/*
** Store Transition Point
*/
          *radial = *(radial-1) ;
          radial->radialStartPoint.x = Xt ;
          radial->radialStartPoint.y = Yt ;
          radial->radialStartPoint.z = Zt ;
          radial->radialGenesis = 2 ;
          ++*SideSlopeTableSize ;
         }
      }
   }
/*
** Write Elevation Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld toElev = %10.4lf isForceSlope = %2ld forcedSlope = %8.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->toElev,radial->isForceSlope,radial->forcedSlope,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Modify For Force Slope To Elevation
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
   {
    if( ( radial->sideSlopeOption == 2 || radial->sideSlopeOption == 5 ) && radial->isForceSlope )
      {
       if( radial->forcedSlope  > 0.0 &&  radial->toElev  < radial->radialStartPoint.z ) radial->radialStatus = 0 ;
       if( radial->forcedSlope  < 0.0 &&  radial->toElev  > radial->radialStartPoint.z ) radial->radialStatus = 0 ;
       if( radial->forcedSlope == 0.0 &&  radial->toElev != radial->radialStartPoint.z ) radial->radialStatus = 0 ;
      }
    if( radial->radialStatus ) radial->radialStatus = 1 ;
   }
/*
** Write Elevation Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld toElev = %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->toElev,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Determing Side Slope Table Elevation Transitions Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Determing Side Slope Table Elevation Transitions Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_markInactiveDeltaVerticalRadials                     |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_markInactiveDeltaVerticalRadials(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize)
/*
** Mark Radials Inactive That Satisfy A Forced Slope To A Delta Elevation
*/
{
 long dbg=0;
 DTM_SIDE_SLOPE_TABLE *radial ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Inactive Delta Vertical Radials") ;
/*
** Mark Radials Intactive That Cannot Satisfy A Forced Slope To A Delta Elevation
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
    if( ( radial->sideSlopeOption == 4 || radial->sideSlopeOption == 7 ) && radial->isForceSlope )
      {
       if( radial->forcedSlope  > 0.0 &&  radial->toDeltaElev  < 0.0 ) radial->radialStatus = 0 ;
       if( radial->forcedSlope  < 0.0 &&  radial->toDeltaElev  > 0.0 ) radial->radialStatus = 0 ;
       if( radial->forcedSlope == 0.0 &&  radial->toDeltaElev != 0.0 ) radial->radialStatus = 0 ;
      }
    if( radial->radialStatus ) radial->radialStatus = 1 ;
   }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Inactive Delta Vertical Radials Completed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_insertVerticesAtCutFillTransitions                   |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_insertVerticesAtCutFillTransitions(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize,double Pptol)
/*
** This Function Inserts Vertices At Cut/Fill Transitions For The Cut/Fill Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   DrapeFlag,NumDrapePts,MemSideSlopeTableSize,MemRadInc=100 ;
 long   ofs1,ofs2,pofs,segmentInternal,zf1,zf2 ;
 double d1,d2,dd,ddx,ddy,ddz,dz1,dz2,dsz1,dsz2,sx1,sy1,sz1,sx2,sy2,sz2,sln,sdx,sdy,sdz,Xt,Yt,Zt,Zs=0.0 ;
 DPoint3d    p3dPts[2] ;
 DTM_DRAPE_POINT *drapeP,*DrapePts=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radial,*radialofs ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Cut/Fill Transitions Into Side Slope Table") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Initialise
*/
 MemSideSlopeTableSize = *SideSlopeTableSize ;
/*
** Determine Cut/Fill Transition Points
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize - 1 ; ++radial )
   {
/*
**  Test For Cut Fill Option
*/
    if( radial->cutFillOption && (radial+1)->cutFillOption &&
        radial->cutFillTin == (radial+1)->cutFillTin &&
        radial->radialStatus  &&  (radial+1)->radialStatus )
      {
/*
**     Write Radials That Are Being Tested For Possibilty Of Transition Point
*/
        if( dbg )
          {
           bcdtmWrite_message(0,0,0,"Testing For Transition Points Between Radials") ;
           bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Genesis = %2ld Tin = %p ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialGenesis,radial->slopeToTin,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
           bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Genesis = %2ld Tin = %p ** %10.4lf %10.4lf %10.4lf",(long)(radial+1-*SideSlopeTable),(radial+1)->radialGenesis,(radial+1)->slopeToTin,(radial+1)->radialStartPoint.x,(radial+1)->radialStartPoint.y,(radial+1)->radialStartPoint.z) ;
          }
/*
**      Set Radial Status
*/
        radial->radialStatus = 2 ;
        (radial+1)->radialStatus = 2 ;
/*
**      Only Test If Not Force Slope
*/
        NumDrapePts = 0 ;
        segmentInternal = 1 ;
        if( ! radial->isForceSlope && ! (radial+1)->isForceSlope )
          {
/*
**         Initialise Drape Line Coordinates
*/
           p3dPts[0].x = radial->radialStartPoint.x ;
           p3dPts[0].y = radial->radialStartPoint.y ;
           p3dPts[0].z = radial->radialStartPoint.z ;
           p3dPts[1].x = (radial+1)->radialStartPoint.x ;
           p3dPts[1].y = (radial+1)->radialStartPoint.y ;
           p3dPts[1].z = (radial+1)->radialStartPoint.z ;
/*
**         Drape Side Slope Segment On Tin
*/
           if( DrapePts != nullptr )bcdtmDrape_freeDrapePointMemory(&DrapePts,&NumDrapePts) ;
           if( bcdtmDrape_stringDtmObject((BC_DTM_OBJ *) radial->cutFillTin,p3dPts,2,false,&DrapePts,&NumDrapePts)) goto errexit ;
/*
**         Check Side Slope Segment Is Within Tin Hull And Doesn't Pass Through Voids
*/
           for( drapeP = DrapePts ; drapeP < DrapePts + NumDrapePts ; ++drapeP )
             {
              if( drapeP->drapeType == DTMDrapedLineCode::External)
                {
                 segmentInternal = 0 ;
                 bcdtmWrite_message(1,0,0,"Side Slope Segment External To Tin Hull Or In Void") ;
                 bcdtmWrite_message(0,0,0,"Radial = %12.6lf %12.6lf %12.6lf",radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
                 goto errexit ;
                }
             }
         }
/*
**     If Side Slope Element Segment Internal To Tin Look For Cut/Fill Transitions
*/
       if( segmentInternal )
         {
/*
**        Set Parameters For Side Slope Segment
*/
          sx1 = radial->radialStartPoint.x ;
          sy1 = radial->radialStartPoint.y ;
          sz1 = radial->radialStartPoint.z ;
          sx2 = (radial+1)->radialStartPoint.x ;
          sy2 = (radial+1)->radialStartPoint.y ;
          sz2 = (radial+1)->radialStartPoint.z ;
          sdx = sx2 - sx1 ;
          sdy = sy2 - sy1 ;
          sdz = sz2 - sz1 ;
          sln = bcdtmMath_distance(sx1,sy1,sx2,sy2) ;
/*
**       Scan Drape Points For Transition Points
*/
          for( ofs1 = 0 , ofs2 = 1 ; ofs1 < NumDrapePts - 1 ; ++ofs1 , ++ofs2 )
            {
/*
**           Calculate Length Of Drape Section
*/
             dd = bcdtmMath_distance((DrapePts+ofs1)->drapeX,(DrapePts+ofs1)->drapeY,(DrapePts+ofs2)->drapeX,(DrapePts+ofs2)->drapeY) ;
             if( dd > 0.0 )
               {
/*
**              Calculate z values On Element Segment At Drape Points
*/
                d1 = bcdtmMath_distance(sx1,sy1,(DrapePts+ofs1)->drapeX,(DrapePts+ofs1)->drapeY) ;
                d2 = bcdtmMath_distance(sx1,sy1,(DrapePts+ofs2)->drapeX,(DrapePts+ofs2)->drapeY) ;
                dsz1 = sz1 + sdz * d1 / sln ;
                dsz2 = sz1 + sdz * d2 / sln ;
/*
**              Check If Drape Points Are Above Or Below Element Segment
*/
                zf1 = zf2 = 0 ;
                if( dsz1 > (DrapePts+ofs1)->drapeZ ) zf1 =  1 ;
                if( dsz1 < (DrapePts+ofs1)->drapeZ ) zf1 = -1 ;
                if( dsz2 > (DrapePts+ofs2)->drapeZ ) zf2 =  1 ;
                if( dsz2 < (DrapePts+ofs2)->drapeZ ) zf2 = -1 ;
/*
**              Check For Transition Point
*/
                if( zf1 == -zf2 || ( zf1 != 0 && zf2 == 0 ) || ( zf1 == 0 && zf2 != 0 ))
                  {
/*
**                 Calculate Transition Point
*/
                   dz1 = dsz1 - (DrapePts+ofs1)->drapeZ ;
                   dz2 = dsz2 - (DrapePts+ofs2)->drapeZ ;
                   if( dz1 < 0.0 ) dz1 = -dz1 ;
                   if( dz2 < 0.0 ) dz2 = -dz2 ;
                   ddx = (DrapePts+ofs2)->drapeX - (DrapePts+ofs1)->drapeX ;
                   ddy = (DrapePts+ofs2)->drapeY - (DrapePts+ofs1)->drapeY ;
                   ddz = (DrapePts+ofs2)->drapeZ - (DrapePts+ofs1)->drapeZ ;
                   Xt  = (DrapePts+ofs1)->drapeX + ddx * dz1 / (dz1+dz2)  ;
                   Yt  = (DrapePts+ofs1)->drapeY + ddy * dz1 / (dz1+dz2)  ;
                   Zt  = (DrapePts+ofs1)->drapeZ + ddz * dz1 / (dz1+dz2)  ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Transition Point Found ** %10.4lf %10.4lf %10.4lf",Xt,Yt,Zt) ;
                   if( dsz1 == dsz2 ) Zt = dsz1 ;
/*
**                 Drape Point On Object And Test For Differences
*/
                   if( dbg )
                     {
                      if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,Xt,Yt,&dz1,&zf1)) goto errexit ;
                      bcdtmWrite_message(0,0,0,"Transition Point z = %10.4lf Surface z = %10.4lf Difference = %20.15lf",Zt,dz1,fabs(Zt-dz1)) ;
                     }
/*
**                 Check Transition Point Is Greater Than ppTol From Prior And Next Radial Point
*/
                   d1 = bcdtmMath_distance(Xt,Yt,radial->radialStartPoint.x,radial->radialStartPoint.y) ;
                   d2 = bcdtmMath_distance(Xt,Yt,(radial+1)->radialStartPoint.x,(radial+1)->radialStartPoint.y) ;
                   if( d1 > Pptol && d2 > Pptol )
                     {
/*
**                    Check And Reallocate If Necessary, Radial Table Memmory
*/
                      pofs = (long) ( radial - *SideSlopeTable ) ;
                      if( *SideSlopeTableSize == MemSideSlopeTableSize )
                        {
                         MemSideSlopeTableSize =  MemSideSlopeTableSize + MemRadInc ;
                         *SideSlopeTable = (DTM_SIDE_SLOPE_TABLE*) realloc( *SideSlopeTable , MemSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
                         if( *SideSlopeTable == nullptr ) { free( SideSlopeTable) ; bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                        }
/*
**                    Copy Radial Table Entries Down One Entry From Radofs
*/
                      radial = *SideSlopeTable + *SideSlopeTableSize ;
                      radialofs = *SideSlopeTable + pofs ;
                      while( radial > radialofs + 1 ) { *radial = *(radial-1) ; --radial ; }
/*
**                    Store Transition Point In SideSlopeTable Table
*/
                      *radial = *(radial-1) ;
                      radial->radialStartPoint.x = Xt ;
                      radial->radialStartPoint.y = Yt ;
                      radial->radialStartPoint.z = Zt ;
                      radial->radialStatus  = 2 ;
                      radial->radialGenesis = 3 ;
                      ++*SideSlopeTableSize ;
                     }
                  }
               }
            }
         }
      }
   }
/*
** Write Cut Fill Transitions
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
      if( radial->radialStatus == 2  )
        {
         if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->cutFillTin,radial->radialStartPoint.x,radial->radialStartPoint.y,&Zs,&DrapeFlag)) goto errexit ;
         bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld Zs = %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,Zs,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
        }
      }
   }
/*
** Determine Radial Cut/Fill Type ** Cut = 2 Cut , Fill == 3
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
   {
    if( radial->radialStatus == 2  )
      {
       if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->cutFillTin,radial->radialStartPoint.x,radial->radialStartPoint.y,&Zs,&DrapeFlag)) goto errexit ;
       if( fabs( radial->radialStartPoint.z - Zs ) < Pptol / 100.0 ) Zs = radial->radialStartPoint.z  ;
       if     ( radial->radialStartPoint.z == Zs ) radial->radialStatus = 1 ;
       else if( radial->radialStartPoint.z <  Zs ) radial->radialStatus = 2 ;
       else                                        radial->radialStatus = 3 ;
      }
   }
/*
** Write Cut Fill Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,Zs,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Mark Radials Not To Be Processed
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
   {
    if( radial->radialStatus == 2  && radial->cutFillOption == 2 ) radial->radialStatus = 0 ;
    if( radial->radialStatus == 3  && radial->cutFillOption == 1 ) radial->radialStatus = 0 ;
    if( radial->radialStatus ) radial->radialStatus = 1 ;
   }
/*
** Write Cut Fill Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( DrapePts != nullptr ) bcdtmDrape_freeDrapePointMemory(&DrapePts,&NumDrapePts) ;
 *SideSlopeTable = (DTM_SIDE_SLOPE_TABLE *) realloc(*SideSlopeTable,*SideSlopeTableSize*sizeof(DTM_SIDE_SLOPE_TABLE)) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Cut/Fill Transitions Into Side Slope Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Cut/Fill Transitions Into Side Slope Table Error") ;
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
|    bcdtmSideSlope_insertTransitionVerticesForSlopeToObject           |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_insertTransitionVerticesForSlopeToObject(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize,double Pptol)
/*
** This Function Inserts Vertices At Cut/Fill Transitions For The Slope To Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   NumDrapePts,MemSideSlopeTableSize,MemRadInc=100 ;
 long   ofs1,ofs2,pofs,sideSlopeOption,segmentInternal,zf1,zf2,DrapeFlag ;
 double d1,d2,dd,ddx,ddy,ddz,dz1,dz2,dsz1,dsz2,sx1,sy1,sz1,sx2,sy2,sz2,sln,sdx,sdy,sdz,Xt,Yt,Zt ;
 DPoint3d    p3dPts[2] ;
 DTM_DRAPE_POINT *drapeP,*DrapePts=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radial,*radialOfs ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Transition Vertices For Slope To Tin") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Initialise
*/
 MemSideSlopeTableSize = *SideSlopeTableSize ;
/*
** Determine Cut/Fill Transition Points
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize - 1 ; ++radial )
   {
/*
**  Test For Slope To Option
*/
    sideSlopeOption = 0 ;
    if( ( radial->sideSlopeOption     == 1 || ( radial->sideSlopeOption     >= 5 && radial->sideSlopeOption     <= 8 ) )  &&
        ( (radial+1)->sideSlopeOption == 1 || ( (radial+1)->sideSlopeOption >= 5 && (radial+1)->sideSlopeOption <= 8 ) )  &&
        ( radial->slopeToTin     == (radial+1)->slopeToTin ) &&
          radial->radialStatus   &&   (radial+1)->radialStatus        ) sideSlopeOption = 1 ;
    if( sideSlopeOption )
      {
/*
**      Write Radials That Are Being Tested For Possibilty Of Transition Point
*/
        if( dbg )
          {
           bcdtmWrite_message(0,0,0,"Testing For Transition Points Between Radials") ;
           if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,radial->radialStartPoint.x,radial->radialStartPoint.y,&dz1,&DrapeFlag)) goto errexit ;
           bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Genesis = %2ld Tin = %p ** %10.4lf %10.4lf %10.4lf ** Surface z = %10.4lf",(long)(radial-*SideSlopeTable),radial->radialGenesis,radial->slopeToTin,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,dz1) ;
           if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,(radial+1)->radialStartPoint.x,(radial+1)->radialStartPoint.y,&dz1,&DrapeFlag)) goto errexit ;
          }
/*
**      Only Test If Not Force Slope
*/
        NumDrapePts = 0 ;
        segmentInternal = 1 ;
        if( ! radial->isForceSlope && ! (radial+1)->isForceSlope )
          {
/*
**         Initialise Drape Line Coordinates
*/
           p3dPts[0].x = radial->radialStartPoint.x ;
           p3dPts[0].y = radial->radialStartPoint.y ;
           p3dPts[0].z = radial->radialStartPoint.z ;
           p3dPts[1].x = (radial+1)->radialStartPoint.x ;
           p3dPts[1].y = (radial+1)->radialStartPoint.y ;
           p3dPts[1].z = (radial+1)->radialStartPoint.z ;
/*
**         Drape Side Slope Segment On Tin
*/
           if( bcdtmDrape_stringDtmObject((BC_DTM_OBJ *)radial->slopeToTin,p3dPts,2,false,&DrapePts,&NumDrapePts)) goto errexit ;
/*
**         Check Side Slope Segment Is Within Tin Hull And Doesn't Pass Through Voids
*/
           for( drapeP = DrapePts ; drapeP < DrapePts + NumDrapePts ; ++drapeP )
             {
             if (drapeP->drapeType == DTMDrapedLineCode::External)
                {
                 segmentInternal = 0 ;
                 bcdtmWrite_message(1,0,0,"Side Slope Segment External To Tin Hull Or In Void") ;
                 bcdtmWrite_message(0,0,0,"Radial = %12.6lf %12.6lf %12.6lf",radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
                 goto errexit ;
                }
             }
          }
/*
**     If Side Slope Element Segment Internal To Tin Look For Cut/Fill Transitions
*/
       if( segmentInternal )
         {
/*
**        Set Parameters For Side Slope Segment
*/
          sx1 = radial->radialStartPoint.x ;
          sy1 = radial->radialStartPoint.y ;
          sz1 = radial->radialStartPoint.z ;
          sx2 = (radial+1)->radialStartPoint.x ;
          sy2 = (radial+1)->radialStartPoint.y ;
          sz2 = (radial+1)->radialStartPoint.z ;
          sdx = sx2 - sx1 ;
          sdy = sy2 - sy1 ;
          sdz = sz2 - sz1 ;
          sln = bcdtmMath_distance(sx1,sy1,sx2,sy2) ;
/*
**        Scan Drape Points For Transition Points
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",NumDrapePts) ;
          for( ofs1 = 0 , ofs2 = 1 ; ofs1 < NumDrapePts - 1 ; ++ofs1 , ++ofs2 )
            {
/*
**           Write Drape Points
*/
             if( dbg == 2  )
               {
                bcdtmWrite_message(0,0,0,"Testing For Transition Point Between Drape Points") ;
                bcdtmWrite_message(0,0,0,"Drape Point[%6ld] ** %10.4lf %10.4lf %10.4lf",ofs1,(DrapePts+ofs1)->drapeX,(DrapePts+ofs1)->drapeY,(DrapePts+ofs1)->drapeZ) ;
                bcdtmWrite_message(0,0,0,"Drape Point[%6ld] ** %10.4lf %10.4lf %10.4lf",ofs2,(DrapePts+ofs2)->drapeX,(DrapePts+ofs2)->drapeY,(DrapePts+ofs2)->drapeZ) ;
               }
/*
**           Calculate Length Of Drape Section
*/
             dd = bcdtmMath_distance((DrapePts+ofs1)->drapeX,(DrapePts+ofs1)->drapeY,(DrapePts+ofs2)->drapeX,(DrapePts+ofs2)->drapeY) ;
             if( dd > 0.0 )
               {
/*
**              Calculate z values On Element Segment At Drape Points
*/
                d1 = bcdtmMath_distance(sx1,sy1,(DrapePts+ofs1)->drapeX,(DrapePts+ofs1)->drapeY) ;
                d2 = bcdtmMath_distance(sx1,sy1,(DrapePts+ofs2)->drapeX,(DrapePts+ofs2)->drapeY) ;
                dsz1 = sz1 + sdz * d1 / sln ;
                dsz2 = sz1 + sdz * d2 / sln ;
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"z At Ofs1 On Side Slope Segment = %10.4lf",dsz1) ;
                   bcdtmWrite_message(0,0,0,"z At Ofs2 On Side Slope Segment = %10.4lf",dsz2) ;
                  }
/*
**              Check If Drape Points Are Above Or Below Element Segment
*/
                zf1 = zf2 = 0 ;
                if( dsz1 > (DrapePts+ofs1)->drapeZ ) zf1 =  1 ;
                if( dsz1 < (DrapePts+ofs1)->drapeZ ) zf1 = -1 ;
                if( dsz2 > (DrapePts+ofs2)->drapeZ ) zf2 =  1 ;
                if( dsz2 < (DrapePts+ofs2)->drapeZ ) zf2 = -1 ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Zf1 = %2ld Zf2 = %2ld",zf1,zf2) ;
/*
**              Check For Transition Point
*/
                if( zf1 == -zf2 || ( zf1 != 0 && zf2 == 0 ) || ( zf1 == 0 && zf2 != 0 ))
                  {
/*
**                 Calculate Transition Point
*/
                   dz1 = dsz1 - (DrapePts+ofs1)->drapeZ ;
                   dz2 = dsz2 - (DrapePts+ofs2)->drapeZ ;
                   if( dz1 < 0.0 ) dz1 = -dz1 ;
                   if( dz2 < 0.0 ) dz2 = -dz2 ;
                   ddx = (DrapePts+ofs2)->drapeX - (DrapePts+ofs1)->drapeX ;
                   ddy = (DrapePts+ofs2)->drapeY - (DrapePts+ofs1)->drapeY ;
                   ddz = (DrapePts+ofs2)->drapeZ - (DrapePts+ofs1)->drapeZ ;
                   Xt  = (DrapePts+ofs1)->drapeX + ddx * dz1 / (dz1+dz2)  ;
                   Yt  = (DrapePts+ofs1)->drapeY + ddy * dz1 / (dz1+dz2)  ;
                   Zt  = (DrapePts+ofs1)->drapeZ + ddz * dz1 / (dz1+dz2)  ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Transition Point Found ** %10.4lf %10.4lf %10.4lf",Xt,Yt,Zt) ;
                   if( dsz1 == dsz2 ) Zt = dsz1 ;
/*
**                 Drape Point On Object And Test For Differences
*/
                   if( dbg )
                     {
                      if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,Xt,Yt,&dz1,&zf1)) goto errexit ;
                      bcdtmWrite_message(0,0,0,"Transition Point z = %10.4lf Surface z = %10.4lf Difference = %20.15lf",Zt,dz1,fabs(Zt-dz1)) ;
                     }
/*
**                 Check Transition Point Is Greater Than ppTol From Prior And Next Radial Point
*/
                   d1 = bcdtmMath_distance(Xt,Yt,radial->radialStartPoint.x,radial->radialStartPoint.y) ;
                   d2 = bcdtmMath_distance(Xt,Yt,(radial+1)->radialStartPoint.x,(radial+1)->radialStartPoint.y) ;
                   if( d1 > Pptol && d2 > Pptol )
                     {
/*
**                    Check And Reallocate If Necessary, Radial Table Memmory
*/
                      pofs = (long) ( radial - *SideSlopeTable ) ;
                      if( *SideSlopeTableSize == MemSideSlopeTableSize )
                        {
                         MemSideSlopeTableSize =  MemSideSlopeTableSize + MemRadInc ;
                         *SideSlopeTable = ( DTM_SIDE_SLOPE_TABLE*) realloc( *SideSlopeTable , MemSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
                         if( *SideSlopeTable == nullptr ) { free( SideSlopeTable) ; bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                        }
/*
**                    Copy Radial Table Entries Down One Entry From Radofs
*/
                      radial = *SideSlopeTable + *SideSlopeTableSize ;
                      radialOfs = *SideSlopeTable + pofs ;
                      while( radial > radialOfs + 1 ) { *radial = *(radial-1) ; --radial ; }
/*
**                    Store Transition Point In SideSlopeTable Table
*/
                      *radial = *(radial-1) ;
                      radial->radialStartPoint.x = Xt ;
                      radial->radialStartPoint.y = Yt ;
                      radial->radialStartPoint.z = Zt ;
                      radial->radialStatus  = 1 ;
                      radial->radialGenesis = 4 ;
                      ++*SideSlopeTableSize ;
                     }
                  }
               }
            }
         }
/*
**     Free Drape Points Memory
*/
       if( DrapePts != nullptr ) bcdtmDrape_freeDrapePointMemory(&DrapePts,&NumDrapePts) ;
      }
   }
/*
** Write Slope To Object Transitions
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial [%6ld] ** Status = %2ld Genesis = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( DrapePts != nullptr ) bcdtmDrape_freeDrapePointMemory(&DrapePts,&NumDrapePts) ;
 *SideSlopeTable = ( DTM_SIDE_SLOPE_TABLE *) realloc(*SideSlopeTable,*SideSlopeTableSize*sizeof(DTM_SIDE_SLOPE_TABLE)) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Transition Vertices For Slope To Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Transition Vertices For Slope To Object Error") ;
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
|  bcdtmSideSlope_createSideSlopesForActiveRadials                   |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_createSideSlopesForActiveRadials(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long SideSlopeDirection,DTM_SLOPE_TABLE *SlopeTable,long SlopeTableSize,long CornerOption,long StrokeCorners,double CornerStrokeTolerance,double Pptol,DPoint3d *ParallelEdgePts,long NumParallelEdgePts,DTMUserTag UserRadialTag,DTMUserTag UserElementTag,BC_DTM_OBJ* **DataObjects,long *NumberOfDataObjects)
/*
**
** This Function Calculates Sides Slopes For Active Side Slope Table Radials
**
**
** Arguements
*/
{
 int      ret=0,dbg=0 ;
 long     n,CloseFlag,MemDataObjects=0,MemDataObjectsInc=100  ;
 long     RightSideSlopeTableSize,LeftSideSlopeTableSize,BenchFlag,NumberOfBenchObjects=0 ;
 BC_DTM_OBJ  *SideSlopes=nullptr ;
 BC_DTM_OBJ* *BenchObjects=nullptr ;
 BC_DTM_OBJ   *BenchTin=nullptr ;
 DTM_SIDE_SLOPE_TABLE  *radial,*radial1,*radial2,*radial3,*RightSideSlopeTable=nullptr,*LeftSideSlopeTable=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Active Radials") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",SideSlopeTableSize) ;
    for( radial1 = SideSlopeTable ; radial1 < SideSlopeTable + SideSlopeTableSize  ; ++radial1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] S = %2ld G = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial1-SideSlopeTable),radial1->radialStatus,radial1->radialGenesis,radial1->radialStartPoint.x,radial1->radialStartPoint.y,radial1->radialStartPoint.z) ;
      }
   }
/*
** Initialise
*/
 *NumberOfDataObjects = 0 ;
/*
** Check For Closure
*/
 CloseFlag = 0 ;
 if( SideSlopeTable->radialStartPoint.x == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.x && (SideSlopeTable)->radialStartPoint.y == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.y ) CloseFlag = 1 ;
/*
** If Closure Reorder Side Slope Table So First Radial Is Active
*/
 if( CloseFlag ) if( bcdtmSideSlope_reorderSideSlopeTable(SideSlopeTable,SideSlopeTableSize)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",SideSlopeTableSize) ;
    for( radial1 = SideSlopeTable ; radial1 < SideSlopeTable + SideSlopeTableSize  ; ++radial1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] S = %2ld G = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial1-SideSlopeTable),radial1->radialStatus,radial1->radialGenesis,radial1->radialStartPoint.x,radial1->radialStartPoint.y,radial1->radialStartPoint.z) ;
      }
   }
/*
** Iteratively Side Slope From Active Radials
*/
 radial1 = SideSlopeTable ;
 while ( radial1 < SideSlopeTable + SideSlopeTableSize )
   {
/*
** Get Side Slope Element Section For Side Slope Calculations
*/
    while ( radial1 < SideSlopeTable + SideSlopeTableSize && ! radial1->radialStatus ) ++radial1 ;
    radial2 = radial1 ;
    while ( radial2 < SideSlopeTable + SideSlopeTableSize &&   radial2->radialStatus ) ++radial2 ;
    --radial2 ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Section Start =  [%6ld] ** Status = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial1-SideSlopeTable),radial1->radialStatus,radial1->radialStartPoint.x,radial1->radialStartPoint.y,radial1->radialStartPoint.z) ;
       bcdtmWrite_message(0,0,0,"Section End   =  [%6ld] ** Status = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial2-SideSlopeTable),radial2->radialStatus,radial2->radialStartPoint.x,radial2->radialStartPoint.y,radial2->radialStartPoint.z) ;
      }
/*
** Create Side Slope For Side Slope Element Section
*/
    if( radial2 > radial1 )
      {
/*
**     Initialise
*/
       RightSideSlopeTableSize = 0 ;
       LeftSideSlopeTableSize  = 0 ;
/*
**     Create Right Side Slope Table
*/
       if( SideSlopeDirection == 1 || SideSlopeDirection == 3 )
         {
/*
**        Allocate Memory For Side Slope Element Section
*/
          RightSideSlopeTableSize = (long)(radial2-radial1) + 1 ;
          RightSideSlopeTable     = (DTM_SIDE_SLOPE_TABLE * ) malloc ( RightSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
          if( RightSideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**        Copy Side Slope Table To  Side Slope Element Section
*/
          for( radial = radial1 , radial3 = RightSideSlopeTable ; radial <= radial2 ; ++radial , ++radial3 )
            {
             *radial3 = *radial ;
            }
         }
/*
**     Create Left Side Slope Table
*/
       if( SideSlopeDirection == 2 || SideSlopeDirection == 3 )
         {
/*
**        Allocate Memory For Element Section
*/
          LeftSideSlopeTableSize = (long)(radial2-radial1) + 1 ;
          LeftSideSlopeTable     = (DTM_SIDE_SLOPE_TABLE * ) malloc ( LeftSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
          if( LeftSideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**        Copy Side Slope Table To Element Section
*/
          for( radial = radial1 , radial3 = LeftSideSlopeTable ; radial <= radial2 ; ++radial , ++radial3 )
            {
             *radial3 = *radial ;
            }
         }
/*
**     Check For Benches
*/
       BenchFlag = 0 ;
/*
       for( radial = RightSideSlopeTable ; radial < RightSideSlopeTable + RightSideSlopeTableSize && ! BenchFlag ; ++radial  )
         {
          if( radial->sideSlopeOption == 5 || radial->sideSlopeOption == 6 || radial->sideSlopeOption == 7 )
            {  BenchFlag = 1 ;  benchTinP = BenchTin  = (BC_DTM_OBJ *)radial->slopeToTin ; }
         }
       for( radial = LeftSideSlopeTable ; radial < LeftSideSlopeTable + LeftSideSlopeTableSize && ! BenchFlag ; ++radial  )
         {
          if( radial->sideSlopeOption == 5 || radial->sideSlopeOption == 6 || radial->sideSlopeOption == 7 )
            {  BenchFlag = 1 ;  benchTinP = BenchTin  = (BC_DTM_OBJ *)radial->slopeToTin ; }
         }
*/
/*
**     Create Side Slopes
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Creating Side Slopes") ;
       if( SideSlopeDirection == 1 || SideSlopeDirection == 3 ) if( bcdtmSideSlope_createSideSlopes(&RightSideSlopeTable,&RightSideSlopeTableSize,1,SlopeTable,SlopeTableSize,CornerOption,StrokeCorners,CornerStrokeTolerance,Pptol) ) goto errexit ;
       if( SideSlopeDirection == 2 || SideSlopeDirection == 3 ) if( bcdtmSideSlope_createSideSlopes(&LeftSideSlopeTable,&LeftSideSlopeTableSize,2,SlopeTable,SlopeTableSize,CornerOption,StrokeCorners,CornerStrokeTolerance,Pptol) ) goto errexit ;

/*
**     Create Data Object For Side Slopes
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Creating Data Object For Side Slopes") ;
       if( SideSlopes != nullptr ) if( bcdtmObject_destroyDtmObject(&SideSlopes)) goto errexit ;
       if( bcdtmObject_createDtmObject(&SideSlopes)) goto errexit ;
/*
**     Resolve Overlapping Radials
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Overlapping Radials") ;
       if( bcdtmSideSlope_resolveOverlappingSideSlopeRadials(RightSideSlopeTable,RightSideSlopeTableSize,LeftSideSlopeTable,LeftSideSlopeTableSize,ParallelEdgePts,NumParallelEdgePts,SideSlopeDirection,CornerStrokeTolerance,Pptol*2.0,UserRadialTag,UserElementTag,Pptol,Pptol,SideSlopes)) goto errexit ;
/*
**     Free Memory For Side Slope Tables
*/
       if( RightSideSlopeTable != nullptr ) { free(RightSideSlopeTable) ; RightSideSlopeTable = nullptr ; }
       if( LeftSideSlopeTable  != nullptr ) { free(LeftSideSlopeTable)  ; LeftSideSlopeTable  = nullptr ; }
/*
**     If Truncation Points  Remove Holes And Boundary Polygons From Side Slopes
*/
       if( ParallelEdgePts != nullptr )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing Holes And Boundary Polygons") ;
          bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(SideSlopes,DTMFeatureType::Hole) ;
          bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(SideSlopes,DTMFeatureType::Hull) ;
         }
/*
**     Extract Benches
*/
       NumberOfBenchObjects = 0 ;
       if( BenchFlag )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Benches") ;
          if( bcdtmSideSlope_extractBenchesFromSlopeToesAndStoreInSeparateDataObjects(BenchTin,SideSlopes,&BenchObjects,&NumberOfBenchObjects)) goto errexit  ;
          bcdtmObject_destroyDtmObject(&SideSlopes) ;
          SideSlopes = nullptr ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Bench Objects = %2ld",NumberOfBenchObjects) ;
/*
**     Store Pointer To Side Slope Data Object
*/
       if( NumberOfBenchObjects == 0  )
         {
          if( *NumberOfDataObjects == MemDataObjects )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Data Object Pointers") ;
             MemDataObjects = MemDataObjects + MemDataObjectsInc ;
             if( *DataObjects == nullptr ) *DataObjects = (BC_DTM_OBJ **) malloc ( MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             else                       *DataObjects = (BC_DTM_OBJ **) realloc( *DataObjects,MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             if( *DataObjects == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
          *(*DataObjects+*NumberOfDataObjects) = SideSlopes ;
          ++*NumberOfDataObjects ;
          SideSlopes = nullptr ;
         }
       else
         {
          for( n = 0 ; n < NumberOfBenchObjects ; ++n )
            {
             if( *NumberOfDataObjects == MemDataObjects )
               {
                MemDataObjects = MemDataObjects + MemDataObjectsInc ;
                if( *DataObjects == nullptr ) *DataObjects = (BC_DTM_OBJ **) malloc ( MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
                else                       *DataObjects = (BC_DTM_OBJ **) realloc( *DataObjects,MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
                if( *DataObjects == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
               }
             *(*DataObjects+*NumberOfDataObjects) = *(BenchObjects+n) ;
             ++*NumberOfDataObjects ;
            }
          if( BenchObjects != nullptr ) { free(BenchObjects) ; BenchObjects = nullptr ; }
          NumberOfBenchObjects = 0 ;
         }
      }
/*
** Set Up To Get Next Side Slope Element Section
*/
    radial1 = radial2 + 1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( RightSideSlopeTable != nullptr ) free(RightSideSlopeTable) ;
 if( LeftSideSlopeTable  != nullptr ) free(LeftSideSlopeTable) ;
 if( SideSlopes   != nullptr ) bcdtmObject_destroyDtmObject(&SideSlopes) ;
 if( BenchObjects != nullptr )
   {
    for( n = 0 ; n < NumberOfBenchObjects ; ++n )
      {
       SideSlopes = *(BenchObjects+n) ;
       bcdtmObject_destroyDtmObject(&SideSlopes) ;
      }
    free(BenchObjects) ;
   }
/*
** Reallocate If No Errors Or Delete Data Objects If Errors
*/
 if( ! ret ) *DataObjects = (BC_DTM_OBJ **) realloc( *DataObjects,*NumberOfDataObjects * sizeof(BC_DTM_OBJ *)) ;
 else
   {
    for( n = 0 ; n < *NumberOfDataObjects ; ++n )
      {
       SideSlopes = *(*DataObjects+n) ;
       bcdtmObject_destroyDtmObject(&SideSlopes) ;
      }
    free(*DataObjects) ;
    *DataObjects = nullptr ;
    *NumberOfDataObjects = 0 ;
   }
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Active Radials Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Active Radials Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_reorderSideSlopeTable                                |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_reorderSideSlopeTable(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize)
/*
** This Function Reorders A Closed Side Slope Table So That
** The First Radial Is  An Active Radial
*/
{
 int       ret=0 ;
 long      dbg=0 ;
 DTM_SIDE_SLOPE_TABLE *radial,*radial1,*radial2,*sideSlopeTable=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Table") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Size Of Side Slope Table = %6ld",SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] S = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStatus,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Scan To Inactive Radial
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize && radial->radialStatus ; ++radial ) ;
 if( dbg && radial < SideSlopeTable + SideSlopeTableSize ) bcdtmWrite_message(0,0,0,"No Solution Point [%6ld] ** Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStatus,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
/*
** Scan To Active Radial
*/
 if( radial < SideSlopeTable + SideSlopeTableSize )
   {
    while ( radial < SideSlopeTable + SideSlopeTableSize && ! radial->radialStatus ) ++radial ;
    if( dbg && radial < SideSlopeTable + SideSlopeTableSize ) bcdtmWrite_message(0,0,0,"Solution    Point [%6ld] ** Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStatus,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
/*
** Allocate Memory For Temporary Side Slope Table
*/
    if( radial < SideSlopeTable + SideSlopeTableSize )
      {
       sideSlopeTable   = (DTM_SIDE_SLOPE_TABLE * ) malloc ( SideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
       if( sideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Radials Starting At Active Radial To Temporary Side Slope Table
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Copy Point [%6ld] ** Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStatus,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
       radial1 = sideSlopeTable ;
       radial2 = radial ;
       while ( radial2 < SideSlopeTable + SideSlopeTableSize )  { *radial1 = *radial2 ; ++radial1 ; ++radial2 ; }
       radial2 = SideSlopeTable + 1 ;
       while ( radial2 <= radial )  {  *radial1 = *radial2 ; ++radial1 ; ++radial2 ; }
/*
** Copy Temporary
*/
       for( radial1 = SideSlopeTable , radial2 = sideSlopeTable  ; radial1 < SideSlopeTable + SideSlopeTableSize ; ++radial1 , ++radial2 ) *radial1 = *radial2 ;
      }
   }
/*
** Write Status
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Side Slope Table After Reodereding") ;
    bcdtmWrite_message(0,0,0,"Size Of Side Slope Table = %6ld",SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Status = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialStatus,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( sideSlopeTable != nullptr ) free(sideSlopeTable) ;
/*
** Normal Exit
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_createSideSlopes                                   |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_createSideSlopes(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize,long SideSlopeDirection,DTM_SLOPE_TABLE *SlopeTable,long SlopeTableSize,long CornerOption,long StrokeCorners,double CornerStrokeTolerance,double Pptol)
/*
**
** This Function Calculates Sides Slopes
**
**
** Arguements
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 DTM_SIDE_SLOPE_TABLE  *radial ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope OccurrenceIn A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Active Side Slopes ** seqdbg = %4ld",seqdbg) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"SideSlopeTable        = %p",*SideSlopeTable) ;
    bcdtmWrite_message(0,0,0,"SideSlopeTableSize    = %6ld",*SideSlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"SideSlopeDirection    = %6ld",SideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"SlopeTable            = %p",SlopeTable) ;
    bcdtmWrite_message(0,0,0,"SlopeTableSize        = %p",SlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"Corner Option         = %6ld",CornerOption) ;
    bcdtmWrite_message(0,0,0,"StrokeCorners         = %6ld",StrokeCorners) ;
    bcdtmWrite_message(0,0,0,"CornerStrokeTolerance = %10.4lf",CornerStrokeTolerance) ;
    bcdtmWrite_message(0,0,0,"Pptol                 = %10.4lf",Pptol) ;
   }
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
//       bcdtmWrite_message(0,0,0,"Radial[%6ld] O = %2ld S = %2ld G = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->sideSlopeOption,radial->radialStatus,radial->radialGenesis,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
//       bcdtmWrite_message(0,0,0,"Radial[%6ld] O = %2ld toElev = %10.4lf",(long)(radial-*SideSlopeTable),radial->sideSlopeOption,radial->toElev) ;
       bcdtmWrite_message(0,0,0,"Radial[%6ld] O = %2ld toDist = %10.4lf slope = %10.4lf ** %10.4lf %10.4lf %10.4lf ",(long)(radial-*SideSlopeTable),radial->sideSlopeOption,radial->toHorizOffset,radial->radialSlope,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
    bcdtmSideSlope_writeElementToBinaryDTMFile(*SideSlopeTable,*SideSlopeTableSize,L"SideSlopeElement.dat") ;
   }
/*
** Assign Radial Types
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Radial Types") ;
 if( bcdtmSideSlope_assignRadialTypesToSideSlopeTablePoints(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection) ) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Insert Normal Radials At Convex Corners
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Convex Corners") ;
 if( bcdtmSideSlope_insertNormalRadialsAtConvexCorners(SideSlopeTable,SideSlopeTableSize) ) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Insert Normal Radials At Concave Corners
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Concave Corners") ;
// if( bcdtmSideSlope_insertNormalRadialsAtConcaveCorners(SideSlopeTable,SideSlopeTableSize,Pptol) ) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Assign Slopes And Angles To Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Slopes And Angles To Radials") ;
 if( bcdtmSideSlope_assignSlopesAndAnglesToRadials(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection,SlopeTable,SlopeTableSize) ) goto errexit ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       radial->radialEndPoint.x = radial->radialStartPoint.x + 25.0 * cos(radial->radialAngle) ;
       radial->radialEndPoint.y = radial->radialStartPoint.y + 25.0 * sin(radial->radialAngle) ;
       radial->radialEndPoint.z = radial->radialStartPoint.z + 25.0 * radial->radialSlope ;
      }
    bcdtmSideSlope_writeRadialsToBinaryDTMFile(*SideSlopeTable,*SideSlopeTableSize,1,L"Angles&Slopes.dat") ;
   }
/*
 ** Check For Zero Slope To An Elevation Or Delta Elevation
 */
  for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
    {
     if( radial->radialSlope == 0.0 && ( radial->sideSlopeOption == 2 || radial->sideSlopeOption == 4 ))
       {
        bcdtmWrite_message(2,0,0,"Cannot Side Slope To An Elevation Or Delta Elevation At A Zero Slope") ;
        goto errexit ;
       }
    }
/*
** Check For Processing Limits
** Added 18/10/2004  Rob Cormack
*/
 processingLimits = 0 ;
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize && ! processingLimits ; ++radial )
   {
    if( ( radial->sideSlopeOption >= 3 && radial->sideSlopeOption <= 4 ) ||
        ( radial->sideSlopeOption >= 6 && radial->sideSlopeOption <= 7 )    ) processingLimits = 1 ;
   }
/*
** Convert Delta Vertical To A Horizontal Distance
** Added 18/10/2004  Rob Cormack
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Changing Delta Vertical Limits To Horizontal Limits") ;
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial )
   {
    if( radial->sideSlopeOption == 4 || radial->sideSlopeOption == 7 )
      {
       if( radial->sideSlopeOption == 4 ) radial->sideSlopeOption = 3 ;
       if( radial->sideSlopeOption == 7 ) radial->sideSlopeOption = 6 ;
       radial->toHorizOffset = fabs(radial->toDeltaElev) / fabs(radial->radialSlope) ;
      }
   }
/*
** Adjust Slopes And Angles Of Radials For Calculation Method
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Slopes And Angles For Calculation Method") ;
 if( bcdtmSideSlope_adjustSlopesAndAnglesForCalculationMethod(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection,CornerOption) ) goto errexit ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       radial->radialEndPoint.x = radial->radialStartPoint.x + 25.0 * cos(radial->radialAngle) ;
       radial->radialEndPoint.y = radial->radialStartPoint.y + 25.0 * sin(radial->radialAngle) ;
       radial->radialEndPoint.z = radial->radialStartPoint.z + 25.0 * radial->radialSlope ;
      }
    bcdtmSideSlope_writeRadialsToBinaryDTMFile(*SideSlopeTable,*SideSlopeTableSize,1,L"AdjustedAngles&Slopes.dat") ;
   }
/*
** Extend Horizontal Limits For Corner Radials
*/
/*
** Commented Out 18/10/2004 Rob Cormack. Extension Amount Now Determined In
** Function "bcdtmSideSlope_getReflexRadialAngleAndSlopeForLimit"
*/
// if( dbg ) bcdtmWrite_message(0,0,0,"Extending Horizontal Limits For Corner Radials") ;
// if( bcdtmSideSlope_extendHorizontalLimitForCornerRadials(*SideSlopeTable,*SideSlopeTableSize,SideSlopeDirection)) goto errexit ;
/*
** Project Radials To Limit Or Intersect Tin Surface
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Projecting Radials To Limit And/Or Intersecting Tin Surface") ;
 if( bcdtmSideSlope_intersectRadialsWithSurface(*SideSlopeTable,*SideSlopeTableSize,1) ) goto errexit ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Start %10.4lf %10.4lf %10.4lf End  %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,radial->radialEndPoint.x,radial->radialEndPoint.y,radial->radialEndPoint.z) ;
 //      if( bcdtmMath_distance(radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialEndPoint.x,radial->radialEndPoint.y )> 50.0 ) goto errexit ;
      }
    bcdtmSideSlope_writeRadialsToBinaryDTMFile(*SideSlopeTable,*SideSlopeTableSize,1,L"ProjectedRadials.dat") ;
   }
/*
** Stoke Convex Corners
*/
 if( StrokeCorners )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Stroking Convex Corners") ;
    if( bcdtmSideSlope_strokeConvexCorners(SideSlopeTable,SideSlopeTableSize,SideSlopeDirection,CornerOption,CornerStrokeTolerance) ) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
       for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
         {
          bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld S = %2ld Slope = %10.4lf Angle = %8.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStatus,radial->radialSlope,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
         }
      }
/*
** Project Stroked Corner Radials To Limit Or Intersect Tin Surface
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Projecting Stroked Corner Radials To Limit And/Or Intersecting Tin Surface") ;
    if( bcdtmSideSlope_intersectRadialsWithSurface(*SideSlopeTable,*SideSlopeTableSize,2) ) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
       for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
         {
          bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Start %10.4lf %10.4lf %10.4lf End  %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,radial->radialEndPoint.x,radial->radialEndPoint.y,radial->radialEndPoint.z) ;
         }
       bcdtmSideSlope_writeRadialsToBinaryDTMFile(*SideSlopeTable,*SideSlopeTableSize,1,L"StrokedCornerRadials.dat") ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Active Side Slopes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Active Side Slope Error") ;
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
int bcdtmSideSlope_assignRadialTypesToSideSlopeTablePoints(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long SideSlopeDirection)
/*
** This Function Assigns Radial Types To The Side Slope Table Points
**
** Radial Type == 1  Concave Corner
**             == 2  Normal To Edge
**             == 3  Convex Corner
**
*/
{
 int       ret=0,sdof ;
 long      dbg=0,CloseFlag,CornerFlag ;
 double    n1,pang,nang,dang ;
 DTM_SIDE_SLOPE_TABLE *radialp,*radial,*radialn,*tradial=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Point Types To SideSlopeTable Points") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"SideSlopeTable Points Before Point Type Assignment") ;
    bcdtmWrite_message(0,0,0,"Number Of SideSlopeTable Points = %2ld",SideSlopeTableSize)  ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld]  T = %1ld  ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Check For Closed Or Open Side Slope Element
*/
 CloseFlag = 0 ;
 if( SideSlopeTable->radialStartPoint.x == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.x &&
     SideSlopeTable->radialStartPoint.y == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.y    ) CloseFlag = 1 ;
/*
** Scan SideSlopeTable And Assign Point Types
*/
 CornerFlag = 0 ;
 radialp = SideSlopeTable + SideSlopeTableSize - 2 ;
 for ( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize - CloseFlag ; ++radial )
   {
    radialn = radial + 1 ;
    if( ! CloseFlag && ( radial == SideSlopeTable || radial == SideSlopeTable + SideSlopeTableSize - 1 )) radial->radialType = 2 ;
    else
      {
       n1 = bcdtmMath_normalDistanceToCordLine(radialp->radialStartPoint.x,radialp->radialStartPoint.y,radialn->radialStartPoint.x,radialn->radialStartPoint.y,radial->radialStartPoint.x,radial->radialStartPoint.y) ;
       pang = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,radialp->radialStartPoint.x,radialp->radialStartPoint.y) ;
       nang = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,radialn->radialStartPoint.x,radialn->radialStartPoint.y) ;
       if( nang < pang ) dang = (nang+DTM_2PYE)-pang ;
       else              dang = nang-pang ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial[%6ld] %10.4lf %10.4lf ** Type = %2ld pang = %10.8lf nang = %10.8lf dang = %10.8lf",(long)(radial-SideSlopeTable),radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialType,pang,nang,dang) ;
       if( dang >= (DTM_PYE-0.0001) && dang <= (DTM_PYE+0.0001))   radial->radialType = 2 ;
       else
         {
          sdof = bcdtmMath_sideOf(radialp->radialStartPoint.x,radialp->radialStartPoint.y,radialn->radialStartPoint.x,radialn->radialStartPoint.y,radial->radialStartPoint.x,radial->radialStartPoint.y) ;
          if( ( sdof > 0 && SideSlopeDirection == 1 ) || ( sdof < 0 && SideSlopeDirection == 2 ) ) { radial->radialType = 1 ; CornerFlag = 1 ; }
          if( ( sdof < 0 && SideSlopeDirection == 1 ) || ( sdof > 0 && SideSlopeDirection == 2 ) ) { radial->radialType = 3 ; CornerFlag = 1 ; }
          if( sdof == 0 ) radial->radialType = 2 ;
         }
      }
    radialp = radial ;
   }
/*
** If Closed Set Last Type Equal To First Type
*/
 if( CloseFlag ) (SideSlopeTable+SideSlopeTableSize-1)->radialType = SideSlopeTable->radialType ;
/*
** Test For Non Zero Area Side Slope Table Polygon
*/
 if( CloseFlag && ! CornerFlag ) { bcdtmWrite_message(1,0,0,"Zero Area Element Polygon") ; goto errexit ; }
/*
** Organise Points So First Point Is A corner Point
*/
 if( CloseFlag && SideSlopeTable->radialType != 1 && SideSlopeTable->radialType != 3 )
   {
/*
** Allocate Memory
*/
    tradial = ( DTM_SIDE_SLOPE_TABLE * ) malloc( SideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
    if ( tradial == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan To First Type 1 Point
*/
    radial = SideSlopeTable ;
    while( radial->radialType != 1 && radial->radialType != 3 ) ++radial ;
/*
**  Copy Points To Temporary Structure
*/
    radialp = tradial ;
    radialn = radial  ;
    while ( radialn < SideSlopeTable + SideSlopeTableSize - 1  ) { *radialp = *radialn ; ++radialp ; ++radialn ; }
    radialn = SideSlopeTable ;
    while ( radialn <= radial ) { *radialp = *radialn ; ++radialp ; ++radialn ; }
/*
**  Copy Points From Temporary Structure
*/
    radialp = tradial ;
    radialn = SideSlopeTable  ;
    while ( radialn < SideSlopeTable + SideSlopeTableSize  ) { *radialn = *radialp ; ++radialn ; ++radialp ; }
/*
** Set Last Point To First Point
*/
    (SideSlopeTable+SideSlopeTableSize-1)->radialType = SideSlopeTable->radialType ;
   }
/*
** Write SideSlopeTable Point Types
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"SideSlopeTable Points After Point Type Assignment") ;
    bcdtmWrite_message(0,0,0,"Number Of SideSlopeTable Points = %2ld",SideSlopeTableSize)  ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld]  T = %1ld  ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( tradial != nullptr ) free(tradial) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Assigning Point Types To SideSlopeTable Points Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Assigning Point Types To SideSlopeTable Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_insertNormalRadialsAtConvexCorners(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize)
/*
** This Function Inserts Normal Radials At Convex Corner Points
*/
{
 int       ret=0 ;
 long      dbg=0,numConvexCorners,sideSlopeTableSize,CloseFlag ;
 DTM_SIDE_SLOPE_TABLE *pad,*padn,*sideSlopeTable=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Convex Corners") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"SideSlopeTable Points After Point Type Assignment") ;
    bcdtmWrite_message(0,0,0,"Number Of SideSlopeTable Points = %2ld",SideSlopeTableSize)  ;
    for( pad = *SideSlopeTable ; pad < *SideSlopeTable + *SideSlopeTableSize ; ++pad )
      {
       bcdtmWrite_message(0,0,0,"SideSlopeTable Point[%6ld]  T = %1ld  ** %10.4lf %10.4lf %10.4lf",(long)(pad-*SideSlopeTable),pad->radialType,pad->radialStartPoint.x,pad->radialStartPoint.y,pad->radialStartPoint.z) ;
      }
   }
/*
** Check For Closed Or Open Side Slope Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Closed Or Open Side Slope Element") ;
 CloseFlag = 0 ;
 if( (*SideSlopeTable)->radialStartPoint.x == (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.x &&
     (*SideSlopeTable)->radialStartPoint.y == (*SideSlopeTable+*SideSlopeTableSize-1)->radialStartPoint.y    ) CloseFlag = 1 ;
/*
** Count Number Of Convex Corners
*/
 numConvexCorners = 0 ;
 for( pad = *SideSlopeTable ; pad < *SideSlopeTable + *SideSlopeTableSize - CloseFlag ; ++pad )
   {
    if( pad->radialType == 3 ) ++numConvexCorners ;
   }
/*
** Insert Radials At Convex Corners
*/
 if( numConvexCorners )
   {
/*
** Assign Memory To Temporary Side Slope Table
*/
    sideSlopeTableSize = *SideSlopeTableSize + numConvexCorners * 2 ;
    sideSlopeTable = ( DTM_SIDE_SLOPE_TABLE * ) malloc( sideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
    if( sideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Side Slope Table And Insert Normal Radials At Convex Corners
*/
    padn = sideSlopeTable ;
    for( pad = *SideSlopeTable ; pad < *SideSlopeTable + *SideSlopeTableSize - CloseFlag ; ++pad )
      {
       if( pad->radialType == 3 ) {  *padn = *pad ; padn->radialType = 2 ; ++padn ; }
       *padn = *pad ; ++padn ;
       if( pad->radialType == 3 ) {  *padn = *pad ; padn->radialType = 2 ; ++padn ; }
      }
    if( CloseFlag ) *padn = *sideSlopeTable ;
/*
**  Free Memory Assigned To Side Slope Table
*/
    free(*SideSlopeTable) ;
    *SideSlopeTable = sideSlopeTable ;
    *SideSlopeTableSize = sideSlopeTableSize ;
    sideSlopeTable  = nullptr ;
   }
/*
** Clean Up
*/
 cleanup :
 if( sideSlopeTable != nullptr ) free(sideSlopeTable) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Convex Corners Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Convex Corners Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_insertNormalRadialsAtConcaveCorners(DTM_SIDE_SLOPE_TABLE **sideSlopeTablePP,long *sideSlopeTableSizeP,double p2pTol)
/*
** This Function Inserts Normal Radials At Concave Corner Points
*/
{
 int       ret=DTM_SUCCESS,dbg=0 ;
 double    angle ;
 long      numConcaveCorners,tempSideSlopeTableSize,closeFlag ;
 DTM_SIDE_SLOPE_TABLE *radP,*rad1P,*temP,*tempSideSlopeTableP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Concave Corners") ;
/*
** Check For Closed Or Open Side Slope Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Closed Or Open Side Slope Element") ;
 closeFlag = 0 ;
 if( (*sideSlopeTablePP)->radialStartPoint.x == (*sideSlopeTablePP+*sideSlopeTableSizeP-1)->radialStartPoint.x &&
     (*sideSlopeTablePP)->radialStartPoint.y == (*sideSlopeTablePP+*sideSlopeTableSizeP-1)->radialStartPoint.y    ) closeFlag = 1 ;
/*
** Count Number Of Concave Corners
*/
 numConcaveCorners = 0 ;
 for( radP = *sideSlopeTablePP ; radP < *sideSlopeTablePP + *sideSlopeTableSizeP - closeFlag ; ++radP )
   {
    if( radP->radialType == 1 ) ++numConcaveCorners ;
   }
/*
** Insert Radials At Concave Corners
*/
 if( numConcaveCorners )
   {
/*
**  Assign Memory To Temporary Side Slope Table
*/
    tempSideSlopeTableSize = *sideSlopeTableSizeP + numConcaveCorners * 2 ;
    tempSideSlopeTableP = ( DTM_SIDE_SLOPE_TABLE * ) malloc( tempSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
    if( tempSideSlopeTableP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Scan Side Slope Table And Insert Normal Radials At Concave Corners
*/
    temP =tempSideSlopeTableP ;
    for( radP = *sideSlopeTablePP ; radP < *sideSlopeTablePP + *sideSlopeTableSizeP - closeFlag ; ++radP )
      {
/*
**     Insert Prior Radial
*/
       if( radP->radialType == 1 )
         {
          rad1P = radP - 1 ;
          if( rad1P < *sideSlopeTablePP ) rad1P = *sideSlopeTablePP + *sideSlopeTableSizeP - 2 ;
          if( rad1P->radialType == 2 && bcdtmMath_distance(radP->radialStartPoint.x,radP->radialStartPoint.y,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y) > p2pTol * 10.0 )
            {
             *temP = *radP ;
             angle = bcdtmMath_getAngle(radP->radialStartPoint.x,radP->radialStartPoint.y,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y) ;
             temP->radialStartPoint.x = radP->radialStartPoint.x + p2pTol * 10.0 * cos(angle) ;
             temP->radialStartPoint.y = radP->radialStartPoint.y + p2pTol * 10.0 * sin(angle) ;
             bcdtmMath_interpolatePointOnLine(radP->radialStartPoint.x,radP->radialStartPoint.y,radP->radialStartPoint.z,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y,rad1P->radialStartPoint.z,temP->radialStartPoint.x,temP->radialStartPoint.y,&temP->radialStartPoint.z) ;
             temP->radialType = 2 ;
             ++temP ;
            }
         }
/*
**     Store Current Radial
*/
       *temP = *radP ;
       ++temP ;
/*
**     Insert Next Radial
*/
       if( radP->radialType == 1 )
         {
          rad1P = radP + 1 ;
          if( rad1P >= *sideSlopeTablePP + *sideSlopeTableSizeP ) rad1P = *sideSlopeTablePP + 1 ;
          if( rad1P->radialType == 2 && bcdtmMath_distance(radP->radialStartPoint.x,radP->radialStartPoint.y,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y) > p2pTol * 10.0 )
            {
             *temP = *radP ;
             angle = bcdtmMath_getAngle(radP->radialStartPoint.x,radP->radialStartPoint.y,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y) ;
             temP->radialStartPoint.x = radP->radialStartPoint.x + p2pTol * 10.0 * cos(angle) ;
             temP->radialStartPoint.y = radP->radialStartPoint.y + p2pTol * 10.0 * sin(angle) ;
             bcdtmMath_interpolatePointOnLine(radP->radialStartPoint.x,radP->radialStartPoint.y,radP->radialStartPoint.z,rad1P->radialStartPoint.x,rad1P->radialStartPoint.y,rad1P->radialStartPoint.z,temP->radialStartPoint.x,temP->radialStartPoint.y,&temP->radialStartPoint.z) ;
             temP->radialType = 2 ;
             ++temP ;
            }
         }
      }
/*
**  Close Side Slope Table
*/
    if( closeFlag )
      {
       *temP = *tempSideSlopeTableP ;
       ++temP ;
      }
/*
**  Resize Side Slope Table
*/
    tempSideSlopeTableSize = ( long)(temP-tempSideSlopeTableP) ;
    tempSideSlopeTableP = ( DTM_SIDE_SLOPE_TABLE * ) realloc( tempSideSlopeTableP, tempSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
/*
**  Free Memory Assigned To Side Slope Table
*/
    free(*sideSlopeTablePP) ;
    *sideSlopeTablePP    = tempSideSlopeTableP ;
    *sideSlopeTableSizeP = tempSideSlopeTableSize ;
    tempSideSlopeTableP  = nullptr ;
   }
/*
** Clean Up
*/
 cleanup :
 if(tempSideSlopeTableP != nullptr ) free(tempSideSlopeTableP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Concave Corners Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Normal Radials At Concave Corners Error") ;
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
int bcdtmSideSlope_assignSlopesAndAnglesToRadials(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long SideSlopeDirection,DTM_SLOPE_TABLE *SlopeTable,long SlopeTableSize)
/*
** This Function Assigns Radial Types To The Side Slope Table Points
*/
{
 int   ret=0,dbg=0 ;
 long  CloseFlag,DrapeFlag ;
 double dz,angp,angn ;
 DTM_SIDE_SLOPE_TABLE *radial,*priorRadial,*nextRadial ;
 DTM_SLOPE_TABLE *slp ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Slopes And Angles To Radials") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Radials = %6ld",SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] Option = %2ld Force %2ld SlopeTable = %2ld ** Cut = %10.4lf Fill = %10.4lf Force = %10.4lf",(long)(radial-SideSlopeTable),radial->sideSlopeOption,radial->isForceSlope,radial->useSlopeTable,radial->cutSlope,radial->fillSlope,radial->forcedSlope) ;
      }
   }
/*
** Set Correct Sign For Cut And Fill Slopes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Cut Fill Slopes To Absolute Values") ;
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
    if( radial->cutSlope  < 0.0 ) radial->cutSlope  = -radial->cutSlope  ;
    if( radial->fillSlope > 0.0 ) radial->fillSlope = -radial->fillSlope ;
   }
/*
** Check For Closed Or Open Side Slope Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Closed Or Open Side Slope Element") ;
 CloseFlag = 0 ;
 if( SideSlopeTable->radialStartPoint.x == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.x &&
     SideSlopeTable->radialStartPoint.y == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.y    ) CloseFlag = 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"CloseFlag = %2ld",CloseFlag) ;
/*
** Scan Side Slope Element And Deterine Surface z Of Element Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping Side Slope Element On Slope To Tin") ;
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
    if( ! radial->isForceSlope && radial->slopeToTin != nullptr )
      {
       if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,radial->radialStartPoint.x,radial->radialStartPoint.y,&radial->surfaceZ,&DrapeFlag)) goto errexit ;
       if( DrapeFlag == 0 || DrapeFlag == 2 ) { bcdtmWrite_message(1,0,0,"Side Slope Element Point %10.4lf %10.4lf External To Tin Or In Void",radial->radialStartPoint.x,radial->radialStartPoint.y) ; goto errexit ; }
      }
   }
/*
** Scan Side Slope Element And Assign Slope And Angles To Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Slopes And Angles To Radials") ;
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
/*
** Assign Radial Angle To User Value
*/
    if( radial->isRadialDir ) radial->radialAngle = radial->radialDir ;
/*
** Calculate Radial Angle From Slope Element Direction
*/
    else
      {
/*
** Calculate Angle For Corner Radial
*/
       if( radial->radialType == 1 || radial->radialType == 3 )
         {
/*
** Get Prior And Next Side Slope Element Points
*/
          priorRadial = nextRadial = radial ;
          while ( priorRadial->radialStartPoint.x == radial->radialStartPoint.x && priorRadial->radialStartPoint.y == radial->radialStartPoint.y ) { --priorRadial ; if( priorRadial < SideSlopeTable ) priorRadial = SideSlopeTable + SideSlopeTableSize - 1  ; }
          while ( nextRadial->radialStartPoint.x == radial->radialStartPoint.x && nextRadial->radialStartPoint.y == radial->radialStartPoint.y ) { ++nextRadial ; if( nextRadial > SideSlopeTable + SideSlopeTableSize - 1 ) nextRadial = SideSlopeTable  ; }
/*
** Calculate Angles To Prior And Next Side Slope Element Points
*/
          angp = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,priorRadial->radialStartPoint.x,priorRadial->radialStartPoint.y) ;
          angn = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,nextRadial->radialStartPoint.x,nextRadial->radialStartPoint.y) ;
/*
** Calculate Radial Angle
*/
          if( SideSlopeDirection == 1 )
            {
             if ( angn < angp ) angn += DTM_2PYE ;
             radial->radialAngle  = ( angp + angn ) / 2.0 ;
             if( radial->radialAngle >= DTM_2PYE ) radial->radialAngle -= DTM_2PYE ;
            }
          if( SideSlopeDirection == 2 )
            {
             if ( angp < angn ) angp += DTM_2PYE ;
             radial->radialAngle  = ( angp + angn ) / 2.0 ;
             if( radial->radialAngle >= DTM_2PYE ) radial->radialAngle -= DTM_2PYE ;
            }
         }
/*
** Calculate Angle Edge Radials
*/
       if( radial->radialType == 2 )
         {
/*
** Get Prior Radial
*/
          priorRadial = radial - 1 ;
          if ( priorRadial < SideSlopeTable )
            {
             if( ! CloseFlag ) priorRadial = nullptr ;
             else              priorRadial = SideSlopeTable + SideSlopeTableSize - 2 ;
            }
          if( priorRadial != nullptr ) if( priorRadial->radialStartPoint.x == radial->radialStartPoint.x && priorRadial->radialStartPoint.y == radial->radialStartPoint.y ) priorRadial = nullptr ;
/*
** Get Next Radial
*/
          nextRadial = radial + 1 ;
          if( nextRadial > SideSlopeTable + SideSlopeTableSize - 1 )
            {
             if( ! CloseFlag ) nextRadial = nullptr ;
             else              nextRadial = SideSlopeTable + 1 ;
            }
          if( nextRadial != nullptr ) if( nextRadial->radialStartPoint.x == radial->radialStartPoint.x && nextRadial->radialStartPoint.y == radial->radialStartPoint.y ) nextRadial = nullptr ;
/*
** Calculate Angles And Slopes For Edge Radial
*/
          if( priorRadial != nullptr && nextRadial != nullptr ) nextRadial = nullptr ;
          if( priorRadial != nullptr )
            {
             angp = bcdtmMath_getAngle(priorRadial->radialStartPoint.x,priorRadial->radialStartPoint.y,radial->radialStartPoint.x,radial->radialStartPoint.y) ;
             if( SideSlopeDirection == 1 ) radial->radialAngle = angp - ( DTM_PYE / 2.0 ) ;
             else                          radial->radialAngle = angp + ( DTM_PYE / 2.0 ) ;
            }
          else
            {
             angp = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,nextRadial->radialStartPoint.x,nextRadial->radialStartPoint.y) ;
             if( SideSlopeDirection == 1 ) radial->radialAngle = angp - ( DTM_PYE / 2.0 ) ;
             else                          radial->radialAngle = angp + ( DTM_PYE / 2.0 ) ;
            }
         }
      }
/*
** Normalise Angle
*/
    radial->radialAngle = bcdtmMath_normaliseAngle(radial->radialAngle) ;
/*
**  Assign Slope
*/
    if     ( radial->isForceSlope ) radial->radialSlope = radial->forcedSlope ;
/*
** Slope To An Object
*/
    else if( radial->sideSlopeOption == 1 || radial->sideSlopeOption == 6 )
      {
/*
** Assign Slope From Slope Table
*/
       if( radial->useSlopeTable )
         {
          dz = radial->radialStartPoint.z - radial->surfaceZ ;
          for( slp = SlopeTable ; slp < SlopeTable + SlopeTableSize ; ++slp )
            {
             if( dz >= slp->Low && dz < slp->High )
               {
                if( dz >= 0.0 ) radial->radialSlope = -slp->Slope ;
                else            radial->radialSlope =  slp->Slope ;
                slp = SlopeTable + SlopeTableSize ;
               }
            }
         }
/*
** Assign Slope Depending On Cut Fill Of Side Slope Element Point
*/
       else
         {
          if( radial->radialStartPoint.z  <= radial->surfaceZ ) radial->radialSlope = radial->cutSlope ;
          else                                                  radial->radialSlope = radial->fillSlope ;
         }
      }
/*
** Slope To An Elevation
*/
    else if( radial->sideSlopeOption == 2 ||  radial->sideSlopeOption == 5 )
      {
       if( radial->radialStartPoint.z  <= radial->toElev ) radial->radialSlope = radial->cutSlope ;
       else                                                radial->radialSlope = radial->fillSlope ;
      }
/*
** Slope To A Delta Elevation
*/
    else if( radial->sideSlopeOption == 4 )
      {
       if(  radial->toDeltaElev >= 0.0 ) radial->radialSlope = radial->cutSlope ;
       else                              radial->radialSlope = radial->fillSlope ;
      }
    else if( radial->sideSlopeOption == 7 )
      {
       if( radial->surfaceZ < radial->radialStartPoint.z && radial->toDeltaElev >= 0.0 )  radial->toDeltaElev = -radial->toDeltaElev ;
       if( radial->surfaceZ > radial->radialStartPoint.z && radial->toDeltaElev <= 0.0 )  radial->toDeltaElev = -radial->toDeltaElev ;
       if(  radial->toDeltaElev >= 0.0 ) radial->radialSlope = radial->cutSlope ;
       else                              radial->radialSlope = radial->fillSlope ;
      }
/*
** Slope To A Delta Horizontal
*/
    else if( radial->sideSlopeOption == 3 )
      {
       radial->radialSlope = radial->forcedSlope ;
bcdtmWrite_message(0,0,0,"radialSlope = %10.4lf radial->forcedSlope = %10.4lf",radial->radialSlope,radial->forcedSlope) ;
      }
   }
/*
** Write Angles And Slopes
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Radials = %6ld",SideSlopeTableSize) ;
    for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld Angle = %12.10lf Slope = %10.6lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialAngle,radial->radialSlope) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Assigning Slopes And Angles To Radials Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Assigning Slopes And Angles To Radials Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_adjustSlopesAndAnglesForCalculationMethod(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long SideSlopeDirection,long CornerOption)
/*
** This Function Assigns Radial Types To The Side Slope Table Points
*/
{
 int    ret=0,dbg=0 ;
 long   Solution ;
 double saveSlope,angle,slope,horRatio ;
 DTM_SIDE_SLOPE_TABLE *radial  ;
 double *slpP,*slopesP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Slopes And Angles For Calculation Method") ;
/*
**
** Allocate Array For Temporary Save Of Radial Slopes For Limit Calculations
** RobC - March 2007
**
*/
 slopesP = ( double * ) malloc ( SideSlopeTableSize * sizeof(double)) ;
 if( slopesP == nullptr )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( slpP = slopesP , radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++slpP , ++radial )
   {
    *slpP = radial->radialSlope ;
   }
/*
** Adjust Angles And Slopes Of Corner Radials For Calculation Method
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
/*
** Additional Condition Added 16 Sep 2004 RobC To Only Adjust The Angles For
** Non Limit Side Slopes
*/
// Robc March 2007    if( radial->sideSlopeOption == 1 )
//    if( radial->sideSlopeOption == 1 )
// RobC - Modified March 2012 To Include To Elevation, To Delta Elevation and Out Horizontal Distance
    if( radial->sideSlopeOption >= 1 && radial->sideSlopeOption <= 4  )
      {
       if( ! radial->isRadialDir && ( radial->radialOption == 1 || radial->radialOption == 2 ) && ( radial->radialType == 1 || radial->radialType == 3 ) )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Corner Radial[%6ld] T = %2ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
          saveSlope = radial->radialSlope ;
          if( bcdtmSideSlope_calculateAngleAndSlopeForCornerRadial(SideSlopeTable,SideSlopeTableSize,radial,SideSlopeDirection,&Solution)) goto errexit ;
          if( CornerOption == 1 && radial->radialType == 3  ) radial->radialSlope = saveSlope ;
 //         if( CornerOption == 1 && radial->radialType == 3  ) radial->radialSlope = (radial->radialSlope + saveSlope)/2.0 ;
          radial->radialSolution = Solution ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Adjusted  Corner Radial[%6ld] T = %2ld Solution = %1ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialSolution,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;

//        RobC - 09Mar2012 - Extend Horizontal Distance

          if( CornerOption == 2 && radial->radialType == 3 && radial->sideSlopeOption >= 3 && radial->sideSlopeOption <= 4 )
            {
             angle = fabs((radial+1)->radialAngle - (radial-1)->radialAngle) ;
             while( angle > DTM_PYE ) angle = angle - DTM_PYE ;
             angle = angle / 2.0 ;
             radial->toHorizOffset = radial->toHorizOffset / sin(angle) ;
            }
         }
      }
/*
** Adjust Slope And Angle Of Reflex Radial For Limit Applications
*/
    else if ( radial->radialType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld ** %12.4lf %12.4lf %10.4lf ** Angle = %12.10lf Slope = %10.8lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,radial->radialAngle,radial->radialSlope) ;
       if( bcdtmSideSlope_getReflexRadialAngleAndSlopeForLimit(SideSlopeTable,SideSlopeTableSize,radial,SideSlopeDirection,slopesP,&angle,&slope,&horRatio)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"============== Adjusted Angle = %12.10lf Adjusted Slope = %10.8lf Extension Ratio = %8.4lf",angle,slope,horRatio) ;
       radial->radialAngle = angle ;
       if     ( radial->radialSlope > 0.0 && slope > 0.0 ) radial->radialSlope = slope ;
       else if( radial->radialSlope < 0.0 && slope < 0.0 ) radial->radialSlope = slope ;
       radial->toHorizOffset = radial->toHorizOffset * horRatio ;
      }
   }
/*
** Adjust Angles And Slopes Of Edge Radials For Calculation Method
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
    if( ! radial->isRadialDir && radial->radialOption == 2  &&  radial->radialType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Edge Radial[%6ld] T = %2ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
       if( bcdtmSideSlope_calculateAngleAndSlopeForEdgeRadial(SideSlopeTable,SideSlopeTableSize,radial,SideSlopeDirection,&Solution)) goto errexit ;
       radial->radialSolution = Solution ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Adjusted  Edge Radial[%6ld] T = %2ld Solution = %1ld Slope = %10.4lf Angle = %8.4lf Degs %11.8lf Rads ** %10.4lf %10.4lf %10.4lf",(long)(radial-SideSlopeTable),radial->radialType,radial->radialSolution,radial->radialSlope,radial->radialAngle*360.0/DTM_2PYE,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( slopesP != nullptr ) free(slopesP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Adjusting Slopes And Angles For Calculation Method Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Adjusting Slopes And Angles For Calculation Method Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
int bcdtmSideSlope_getReflexRadialAngleAndSlopeForLimit(DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,long sideSlopeTableSize,DTM_SIDE_SLOPE_TABLE *radialP,long sideSlopeDirection,double *radialSlopesP,double *angleP,double *slopeP,double *horRatioP)
{
 int    ret=DTM_SUCCESS,sd1,sd2,dbg=0 ;
 double x=0.0,y=0.0,pz,nz,px1,py1,pz1,px2,py2,pz2,nx1,ny1,nz1,nx2,ny2,nz2;
 bool intersectionFound ;
 double angp,angn,offset ;
 DTM_SIDE_SLOPE_TABLE *priorRadialP,*nextRadialP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Reflex Radial Angle And Slope For Limit Completed") ;
/*
** Initialise
*/
 *horRatioP = 1.0 ;
 *angleP = *slopeP = 0.0 ;
 if( radialP < sideSlopeTableP || radialP >= sideSlopeTableP + sideSlopeTableSize )
   {
    bcdtmWrite_message(2,0,0,"Side Slope Radial Range Error") ;
    goto errexit ;
   }
/*
** Get Prior And Next Side Slope Element Radials
*/
 priorRadialP = nextRadialP = radialP ;
 while ( priorRadialP->radialStartPoint.x == radialP->radialStartPoint.x && priorRadialP->radialStartPoint.y == radialP->radialStartPoint.y ) { --priorRadialP ; if( priorRadialP < sideSlopeTableP ) priorRadialP = sideSlopeTableP + sideSlopeTableSize - 1  ; }
 while ( nextRadialP->radialStartPoint.x  == radialP->radialStartPoint.x && nextRadialP->radialStartPoint.y == radialP->radialStartPoint.y ) { ++nextRadialP  ; if( nextRadialP  > sideSlopeTableP + sideSlopeTableSize - 1 ) nextRadialP = sideSlopeTableP  ; }
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Prior Radial = %6ld ** %12.4lf %12.4lf %10.4lf ** Angle = %12.10lf Slope = %10.4lf",(long)(priorRadialP-sideSlopeTableP),priorRadialP->radialStartPoint.x,priorRadialP->radialStartPoint.y,priorRadialP->radialStartPoint.z,priorRadialP->radialAngle,priorRadialP->radialSlope) ;
    bcdtmWrite_message(0,0,0,"Radial       = %6ld ** %12.4lf %12.4lf %10.4lf ** Angle = %12.10lf Slope = %10.4lf",(long)(radialP-sideSlopeTableP),radialP->radialStartPoint.x,radialP->radialStartPoint.y,radialP->radialStartPoint.z,radialP->radialAngle,radialP->radialSlope) ;
    bcdtmWrite_message(0,0,0,"Next Radial  = %6ld ** %12.4lf %12.4lf %10.4lf ** Angle = %12.10lf Slope = %10.4lf",(long)(nextRadialP-sideSlopeTableP),nextRadialP->radialStartPoint.x,nextRadialP->radialStartPoint.y,nextRadialP->radialStartPoint.z,nextRadialP->radialAngle,nextRadialP->radialSlope) ;
   }
/*
** Calculate Prior And Next Angles
*/
 angp = bcdtmMath_getAngle(priorRadialP->radialStartPoint.x,priorRadialP->radialStartPoint.y,radialP->radialStartPoint.x,radialP->radialStartPoint.y) ;
 angn = bcdtmMath_getAngle(radialP->radialStartPoint.x,radialP->radialStartPoint.y,nextRadialP->radialStartPoint.x,nextRadialP->radialStartPoint.y) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"angp = %12.10lf angn = %12.10lf",angp,angn) ;
/*
** Modify Angles For Side Slope Direction
*/
 if( sideSlopeDirection == 1 )
   {
    angp -=  DTM_PYE / 2.0 ;
    angn -=  DTM_PYE / 2.0 ;
   }
 else
   {
    angp +=  DTM_PYE / 2.0 ;
    angn +=  DTM_PYE / 2.0 ;
   }
 angp = bcdtmMath_normaliseAngle(angp) ;
 angn = bcdtmMath_normaliseAngle(angn) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"angp = %12.10lf angn = %12.10lf",angp,angn) ;
/*
** Calculate Offset Lines To Side Slope Element At 100 Units
*/
 offset = 100 ;
 px1 = priorRadialP->radialStartPoint.x + offset * cos(angp) ;
 py1 = priorRadialP->radialStartPoint.y + offset * sin(angp) ;
 px2 = radialP->radialStartPoint.x + offset * cos(angp) ;
 py2 = radialP->radialStartPoint.y + offset * sin(angp) ;
 nx1 = radialP->radialStartPoint.x + offset * cos(angn) ;
 ny1 = radialP->radialStartPoint.y + offset * sin(angn) ;
 nx2 = nextRadialP->radialStartPoint.x + offset * cos(angn) ;
 ny2 = nextRadialP->radialStartPoint.y + offset * sin(angn) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Prior Offset Line = %12.4lf %12.4lf ** %12.4lf %12.4lf",px1,py1,px2,py2) ;
    bcdtmWrite_message(0,0,0,"Next  Offset Line = %12.4lf %12.4lf ** %12.4lf %12.4lf",nx1,ny1,nx2,ny2) ;
   }
/*
** Intersect Prior And Next Offset Lines
*/
 intersectionFound = false ;
 while ( intersectionFound == false )
   {
    sd1 = bcdtmMath_sideOf(nx1,ny1,nx2,ny2,px1,py1) ;
    sd2 = bcdtmMath_sideOf(nx1,ny1,nx2,ny2,px2,py2) ;
    if( sd1 != sd2 )
      {
       sd1 = bcdtmMath_sideOf(px1,py1,px2,py2,nx1,ny1) ;
       sd2 = bcdtmMath_sideOf(px1,py1,px2,py2,nx2,ny2) ;
       if( sd1 != sd2  )
         {
          intersectionFound = true ;
          bcdtmMath_normalIntersectCordLines(px1,py1,px2,py2,nx1,ny1,nx2,ny2,&x,&y) ;
         }
      }
    if( intersectionFound == false )
      {
       offset = offset / 2.0 ;
       px1 = priorRadialP->radialStartPoint.x + offset * cos(angp) ;
       py1 = priorRadialP->radialStartPoint.y + offset * sin(angp) ;
       px2 = radialP->radialStartPoint.x + offset * cos(angp) ;
       py2 = radialP->radialStartPoint.y + offset * sin(angp) ;
       nx1 = radialP->radialStartPoint.x + offset * cos(angn) ;
       ny1 = radialP->radialStartPoint.y + offset * sin(angn) ;
       nx2 = nextRadialP->radialStartPoint.x + offset * cos(angn) ;
       ny2 = nextRadialP->radialStartPoint.y + offset * sin(angn) ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Prior Offset Line = %12.4lf %12.4lf ** %12.4lf %12.4lf",px1,py1,px2,py2) ;
          bcdtmWrite_message(0,0,0,"Next  Offset Line = %12.4lf %12.4lf ** %12.4lf %12.4lf",nx1,ny1,nx2,ny2) ;
         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Point ** x = %12.4lf y = %12.4lf",x,y) ;
/*
** Calculate Elevation Values Of Offset Line End Points
** RobC - March 2007
*/
 pz1 = priorRadialP->radialStartPoint.z + *(radialSlopesP+(long)(priorRadialP-sideSlopeTableP)) * offset ;
 pz2 = radialP->radialStartPoint.z      + *(radialSlopesP+(long)(priorRadialP-sideSlopeTableP)) * offset ;
 nz1 = radialP->radialStartPoint.z      + *(radialSlopesP+(long)(nextRadialP-sideSlopeTableP))  * offset ;
 nz2 = nextRadialP->radialStartPoint.z  + *(radialSlopesP+(long)(nextRadialP-sideSlopeTableP))  * offset ;
// pz1 = priorRadialP->radialStartPoint.z + priorRadialP->radialSlope * offset ;
// pz2 = radialP->radialStartPoint.z      + priorRadialP->radialSlope * offset ;
// nz1 = radialP->radialStartPoint.z      + nextRadialP->radialSlope  * offset ;
// nz2 = nextRadialP->radialStartPoint.z  + nextRadialP->radialSlope  * offset ;
// if( dbg ) dtmWrite_message(0,0,0,"pz1 = %10.4lf pz2 = %10.4lf ** nz1 = %10.4lf nz2 = %10.4lf",pz1,pz2,nz1,nz2) ;
/*
** Interpolate Intersect Point On Both Offset Lines
*/
 bcdtmMath_interpolatePointOnLine(px1,py1,pz1,px2,py2,pz2,x,y,&pz) ;
 bcdtmMath_interpolatePointOnLine(nx1,ny1,nz1,nx2,ny2,nz2,x,y,&nz) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"x = %12.4lf y = %12.4lf PZ = %10.4lf Nz = %10.4lf",x,y,pz,nz) ;
/*
** Calculate Horizontal Offset Extension Ratio
*/
 *horRatioP = bcdtmMath_distance(x,y,radialP->radialStartPoint.x,radialP->radialStartPoint.y) / offset ;
/*
** Calculate Radial Angle
*/
 *angleP = bcdtmMath_getAngle(radialP->radialStartPoint.x,radialP->radialStartPoint.y,x,y) ;
/*
** Calculate Radial Slope
*/
 *slopeP = radialP->radialSlope ;
 *slopeP = ( (pz+nz)/2.0 - radialP->radialStartPoint.z ) / bcdtmMath_distance(x,y,radialP->radialStartPoint.x,radialP->radialStartPoint.y) ;
 if     ( radialP->radialSlope < 0.0 && *slopeP > 0.0 ) *slopeP = radialP->radialSlope ;
 else if( radialP->radialSlope > 0.0 && *slopeP < 0.0 ) *slopeP = radialP->radialSlope ;
/*
** Write Angle And Slope
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Angle = %12.10lf Slope = %12.6lf Extension Ratio = %12.8ld",*angleP,*slopeP,*horRatioP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg &&  ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Reflex Radial Angle And Slope For Limit Condition Completed") ;
 if( dbg &&  ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Reflex Radial Angle And Slope For Limit Condition Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
int bcdtmSideSlope_calculateAngleAndSlopeForCornerRadial(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,DTM_SIDE_SLOPE_TABLE *Radial,long SideSlopeDirection,long *PlaneSolution)
/*
**
** This Is The Controlling Routine For Calculating
** A Corner Radial Angle And Slope. Firstly Try For A Planar Solution,
** If No Solution Then Adjust Angle And Slope For A Level Side Slope Element
**
*/
{
 long   dbg=0,Solution ;
 double spz,snz,ang,priorAngle,nextAngle,deltaAngle,Slope,Angle,levelSlope,levelAngle ;
 DTM_SIDE_SLOPE_TABLE *priorRadial=nullptr,*nextRadial=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Angle And Slope For Corner Radial") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Radial = [%4ld]  Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(Radial-SideSlopeTable),Radial->radialType,Radial->radialStartPoint.x,Radial->radialStartPoint.y,Radial->radialStartPoint.z) ;
/*
** Initialise
*/
 *PlaneSolution = 1 ;
/*
** Get Prior Radial
*/
 priorRadial = Radial ; while ( priorRadial->radialStartPoint.x == Radial->radialStartPoint.x && priorRadial->radialStartPoint.y == Radial->radialStartPoint.y ) { --priorRadial ; if( priorRadial < SideSlopeTable )  priorRadial = SideSlopeTable + SideSlopeTableSize - 1 ; }
/*
** Get Next Radial
*/
 nextRadial = Radial  ; while ( nextRadial->radialStartPoint.x == Radial->radialStartPoint.x && nextRadial->radialStartPoint.y == Radial->radialStartPoint.y ) { ++nextRadial ; if( nextRadial == SideSlopeTable + SideSlopeTableSize ) nextRadial = SideSlopeTable + 1 ; }
/*
** Calculate Angles From Radial To Previous Radial And Radial To Next Radial
*/
 priorAngle = bcdtmMath_getAngle(Radial->radialStartPoint.x,Radial->radialStartPoint.y,priorRadial->radialStartPoint.x,priorRadial->radialStartPoint.y) ;
 nextAngle  = bcdtmMath_getAngle(Radial->radialStartPoint.x,Radial->radialStartPoint.y,nextRadial->radialStartPoint.x,nextRadial->radialStartPoint.y) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"priorAngle = %12.8lf nextAngle = %12.8lf",priorAngle,nextAngle) ;
/*
** Calculate Planar Angles And Slope For Side Slope Element
*/
 if( bcdtmSideSlope_calculatePlanarAngleAndSlopeForCornerRadial(SideSlopeTable,SideSlopeDirection,priorRadial,Radial,nextRadial,&Angle,&Slope ) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Planar Solution ** Angle = %10.8lf Slope = %10.4lf",Angle,Slope) ;
/*
** Calculate Planar Angles And Slope For A Level Side Slope Element
*/
 spz =  priorRadial->radialStartPoint.z ;
 snz =  nextRadial->radialStartPoint.z ;
 priorRadial->radialStartPoint.z = nextRadial->radialStartPoint.z = Radial->radialStartPoint.z ;
 if( bcdtmSideSlope_calculatePlanarAngleAndSlopeForCornerRadial(SideSlopeTable,SideSlopeDirection,priorRadial,Radial,nextRadial,&levelAngle,&levelSlope ) ) goto errexit ;
 priorRadial->radialStartPoint.z = spz ;
 nextRadial->radialStartPoint.z  = snz ;
/*
** Check Planar Solution Is Between Normals
*/
 Solution = 1 ;
 if( ( Radial->radialSlope > 0.0 && Slope < 0.0 ) || ( Radial->radialSlope < 0.0 && Slope > 0.0 ) ) Solution = 0 ;
 else  bcdtmSideSlope_checkForSolution(SideSlopeDirection,Radial->radialType,Angle,priorAngle,nextAngle,&Solution) ;
 if( dbg &&   Solution ) bcdtmWrite_message(0,0,0,"Planar Solution Found") ;
 if( dbg && ! Solution ) bcdtmWrite_message(0,0,0,"No Planar Solution Found") ;
/*
** Check Solution Is Within 10 Degrees Of Level Solution
*/
 if( Solution )
   {
    ang = Angle ;
    if( ang < levelAngle ) ang += DTM_2PYE ;
    deltaAngle = ang - levelAngle ;
    while( deltaAngle > DTM_PYE ) deltaAngle -= DTM_PYE  ;
    if( deltaAngle > DTM_PYE / 2.0 ) deltaAngle = DTM_PYE - deltaAngle ;
    if( deltaAngle > 10.0 / 360.0 * DTM_2PYE ) Solution = 0 ;
//    bcdtmWrite_message(0,0,0,"deltaAngle = %10.4lf",deltaAngle * 360.0 / DTM_2PYE ) ;
   }
/*
** Switch To Level Solution If Solution Not Found
*/
 if( ! Solution )
   {
    *PlaneSolution = 0 ;
    Angle = levelAngle ;
    Slope = levelSlope ;
   }
/*
** Set Angle And Slope For Radial
*/
 Radial->radialAngle = Angle ;
 Radial->radialSlope = Slope ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Solution = %1ld Angle = %10.8lf Slope = %10.4lf",Solution,Angle,Slope) ;
 if( dbg && ! Solution) bcdtmWrite_message(0,0,0,"******** Solution Not Found") ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Return
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmSideSlope_calculatePlanarAngleAndSlopeForCornerRadial         |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_calculatePlanarAngleAndSlopeForCornerRadial(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeDirection,DTM_SIDE_SLOPE_TABLE *PriorRadial,DTM_SIDE_SLOPE_TABLE *Radial,DTM_SIDE_SLOPE_TABLE *NextRadial,double *Angle,double *Slope )
/*
** This Function Calculates The Planar Angles And Slopes For A Corner Radial
** By Determing The Line Of Intersection Between The Prior And Next Planes At
** The Corner Radial
*/
{
 long    dbg=0 ;
 double  Xr,Yr,Zr ;
 double  IntAngle=0.0,priorAngle,nextAngle,orthoAngle,priorAngleOffset,nextAngleOffset,Pzr,Nzr ;
 double  A0,B0,C0,D0,A1,B1,C1,D1 ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calculating Planar Angle And Slope For Corner Radial") ;
    bcdtmWrite_message(0,0,0,"SideDirection = %2ld",SideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"Slope         = %10.4lf",Radial->radialSlope) ;
    bcdtmWrite_message(0,0,0,"Angle         = %10.4lf",Radial->radialAngle) ;
    bcdtmWrite_message(0,0,0,"priorRadial = %6ld radial = %6ld nextRadial = %6ld",(long)(PriorRadial-SideSlopeTable),(long)(Radial-SideSlopeTable),(long)(NextRadial-SideSlopeTable)) ;
   }
/*
** Initialise
*/
 *Angle = *Slope = 0.0 ;
/*
** Calculate Angles From Corner Radial To Prior And Next Radials
*/
 priorAngle = bcdtmMath_getAngle(Radial->radialStartPoint.x,Radial->radialStartPoint.y,PriorRadial->radialStartPoint.x,PriorRadial->radialStartPoint.y) ;
 nextAngle  = bcdtmMath_getAngle(Radial->radialStartPoint.x,Radial->radialStartPoint.y,NextRadial->radialStartPoint.x,NextRadial->radialStartPoint.y) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"priorAngle = %12.8lf nextAngle = %12.8lf",priorAngle,nextAngle) ;
/*
** Determine Orthogonal Angle Offset At Corner
*/
 if( SideSlopeDirection == 1 ) { nextAngleOffset = -DTM_PYE / 2.0 ; priorAngleOffset =  DTM_PYE / 2.0 ; }
 else                          { nextAngleOffset =  DTM_PYE / 2.0 ; priorAngleOffset = -DTM_PYE / 2.0 ; }
/*
** Calculate Plane Coefficients For Prior Plane
*/
 orthoAngle = bcdtmMath_normaliseAngle(priorAngle + priorAngleOffset) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Prior Ortho Angle = %12.8lf",orthoAngle) ;
 Xr = Radial->radialStartPoint.x + 200.0 * cos(orthoAngle) ;
 Yr = Radial->radialStartPoint.y + 200.0 * sin(orthoAngle) ;
 Zr = Radial->radialStartPoint.z + 200.0 * Radial->radialSlope   ;
 bcdtmMath_calculatePlaneCoefficients(Radial->radialStartPoint.x,Radial->radialStartPoint.y,Radial->radialStartPoint.z,PriorRadial->radialStartPoint.x,PriorRadial->radialStartPoint.y,PriorRadial->radialStartPoint.z,Xr,Yr,Zr,&A0,&B0,&C0,&D0) ;
/*
** Calculate Plane Coefficients For Next Plane
*/
 orthoAngle = bcdtmMath_normaliseAngle(nextAngle + nextAngleOffset) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Next Ortho Angle  = %12.8lf",orthoAngle) ;
 Xr = Radial->radialStartPoint.x + 200.0 * cos(orthoAngle) ;
 Yr = Radial->radialStartPoint.y + 200.0 * sin(orthoAngle) ;
 Zr = Radial->radialStartPoint.z + 200.0 * Radial->radialSlope   ;
 bcdtmMath_calculatePlaneCoefficients(Radial->radialStartPoint.x,Radial->radialStartPoint.y,Radial->radialStartPoint.z,NextRadial->radialStartPoint.x,NextRadial->radialStartPoint.y,NextRadial->radialStartPoint.z,Xr,Yr,Zr,&A1,&B1,&C1,&D1) ;
/*
** Calculate Angle Between Planes
**
** Special Case ** Level Side Slope Element And Zero Slope
*/
 if( Radial->radialSlope == 0.0 && PriorRadial->radialStartPoint.z == Radial->radialStartPoint.z && Radial->radialStartPoint.z == NextRadial->radialStartPoint.z )
   {
    if( SideSlopeDirection == 1 )
      {
       IntAngle = nextAngle ;
       if( IntAngle < priorAngle ) IntAngle += DTM_2PYE ;
       IntAngle = ( IntAngle + priorAngle ) / 2.0 ;
       IntAngle = bcdtmMath_normaliseAngle(IntAngle) ;
      }
    if( SideSlopeDirection == 2 )
      {
       IntAngle = priorAngle ;
       if( IntAngle < nextAngle ) IntAngle += DTM_2PYE ;
       IntAngle = ( IntAngle + nextAngle ) / 2.0 ;
       IntAngle = bcdtmMath_normaliseAngle(IntAngle) ;
      }
   }
/*
**  Calcualte Intersection Angle Of Prior And Next Planes
*/
 else  bcdtmSideSlope_calculateXYAngleBetweenPlanes(&IntAngle,A0,B0,C0,A1,B1,C1) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Angle Between Planes = %12.8lf",IntAngle) ;
/*
** Normalise Intersection Angle ( Ensure It Is On Correct Side Of Side Slope Element )
*/
 if( SideSlopeDirection == 1 )
   {
    if( nextAngle < priorAngle ) nextAngle += DTM_2PYE ;
    if( IntAngle  < priorAngle ) IntAngle  += DTM_2PYE ;
    if( IntAngle  > nextAngle  ) IntAngle  += DTM_PYE  ;
   }
 if( SideSlopeDirection == 2 )
   {
    if( priorAngle < nextAngle  ) priorAngle += DTM_2PYE ;
    if( IntAngle   < nextAngle  ) IntAngle   += DTM_2PYE ;
    if( IntAngle   > priorAngle ) IntAngle   += DTM_PYE  ;
   }
 IntAngle = bcdtmMath_normaliseAngle(IntAngle) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Normalised Angle Between Planes = %12.8lf",IntAngle) ;
/*
** Calculate Slope
*/
 Xr = Radial->radialStartPoint.x + 200.0 * cos(IntAngle) ;
 Yr = Radial->radialStartPoint.y + 200.0 * sin(IntAngle) ;
 bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A0,B0,C0,D0) ;
 *Angle = IntAngle ;
 *Slope = (Zr-Radial->radialStartPoint.z) / 200.0 ;
/*
** Check Interpolated z Value For Both Planes Along Intersection Line Of Intersection
*/
 if( dbg )
   {
    bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Pzr,A0,B0,C0,D0) ;
    bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Nzr,A1,B1,C1,D1) ;
    if( fabs(Pzr-Nzr) > 0.0001 )
      {
       bcdtmWrite_message(1,0,0,"Error With Interpolated Values Along Intersection Line Of Planes") ;
       bcdtmWrite_message(0,0,0,"Radial = %5ld ** Pzr = %10.5lf Nzr = %10.5lf",(long)(Radial-SideSlopeTable),Pzr,Nzr) ;
       goto errexit ;
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_calculateXYAngleBetweenPlanes                        |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_calculateXYAngleBetweenPlanes(double *Angle,double A0,double B0,double C0,double A1,double B1,double C1)
/*
** This Function Calculates The Angle Between Two Planes
*/
{
 double A,B,C ;
/*
** Calculate Coefficients Of Vector Parallel To Line Of
** Intersection Of The Two Planes
*/
 A = B0*C1 - B1*C0 ;
 B = C0*A1 - C1*A0 ;
 C = A0*B1 - A1*B0 ;
/*
** Get Angle Of Vector Projected On XY Plane
*/
 *Angle = atan2(B,A) ;
 if( *Angle < 0.0 ) *Angle += DTM_2PYE ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_checkForLeftSolution                                 |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_checkForSolution(long SideSlopeDirection,long RadialType,double Angle,double PriorAngle,double NextAngle,long *SolutionFound)
/*
** This Function Checks If The Solution Is Between The Two Normals At The Corner Point
*/
{
 long   dbg=0 ;
 double angp,angn,angr,angs,priorAngleOffset,nextAngleOffset ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Solution ** D = %2ld T = %2ld  Angle = %12.8lf Prior Angle = %12.8lf Next Angle = %12.8lf",SideSlopeDirection,RadialType,Angle,PriorAngle,NextAngle) ;
/*
** Initialise
*/
 *SolutionFound = 1 ;
 angp = PriorAngle ;
 angr = Angle ;
 angn = NextAngle ;
/*
** Determine Orthogonal Angle Offset At Corner
*/
 if( SideSlopeDirection == 1 ) { nextAngleOffset = -DTM_PYE / 2.0 ; priorAngleOffset =  DTM_PYE / 2.0 ; }
 else                          { nextAngleOffset =  DTM_PYE / 2.0 ; priorAngleOffset = -DTM_PYE / 2.0 ; }
/*
**  Check Right Side Slope
*/
 if( SideSlopeDirection == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Right") ;
/*
** Check Solution Is Between Prior And Next Angle In An Anti Clockwise Direction
*/
    if( angn < angp ) angn += DTM_2PYE ;
    if( angr < angp ) angr += DTM_2PYE ;
    if( angr >= angn || angr <= angp ) *SolutionFound = 0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Solution Right Between Prior And Next Radials = %2ld",*SolutionFound) ;
/*
**  Check Soultion Is Between Normals
*/
    if( *SolutionFound )
      {
       angr = bcdtmMath_normaliseAngle(angr) ;
       angn = angn + nextAngleOffset ;
       angn = bcdtmMath_normaliseAngle(angn) ;
       angp = angp + priorAngleOffset ;
       angp = bcdtmMath_normaliseAngle(angp) ;
       if( RadialType == 1 ) { angs = angn ; angn = angp ; angp = angs ; }
       if( angn < angp ) angn += DTM_2PYE ;
       if( angr < angp ) angr += DTM_2PYE ;
       if( angr <= angp || angr >= angn ) *SolutionFound = 0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Solution Right Between Normals = %2ld",*SolutionFound) ;
      }
   }
/*
**  Check Left Side Slope
*/
 if( SideSlopeDirection == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Left") ;
/*
** Check Solution Is Between Next And Prior Angle In An Anti Clockwise Direction
*/
    if( angp < angn ) angp += DTM_2PYE ;
    if( angr < angn ) angr += DTM_2PYE ;
    if( angr >= angp || angr <= angn ) *SolutionFound = 0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Solution Left Between Next And Prior Radials = %2ld",*SolutionFound) ;
/*
**  Check Soultion Is Between Normals
*/
    if( *SolutionFound )
      {
       angr = bcdtmMath_normaliseAngle(angr) ;
       angn = angn + nextAngleOffset ;
       angn = bcdtmMath_normaliseAngle(angn) ;
       angp = angp + priorAngleOffset ;
       angp = bcdtmMath_normaliseAngle(angp) ;
       if( RadialType == 1 ) { angs = angn ; angn = angp ; angp = angs ; }
       if( angp < angn ) angp += DTM_2PYE ;
       if( angr < angn ) angr += DTM_2PYE ;
       if( angr >= angp || angr <= angn ) *SolutionFound = 0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Solution Left Between Normals = %2ld",*SolutionFound) ;
      }
   }
/*
** Write Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Solution = %1ld Direction = %1ld RadialType = %1ld Pang = %12.10lf Angle  = %12.10lf Nang = %12.10lf",*SolutionFound,SideSlopeDirection,RadialType,PriorAngle,Angle,NextAngle) ;
/*
** Job Completed
*/
 return(0) ;
}
/*----------------------------------------------------------------------+
|                                                                       |
|  bcdtmSideSlope_calculateAngleAndSlopeForEdgeRadial                     |
|                                                                       |
+----------------------------------------------------------------------*/
int bcdtmSideSlope_calculateAngleAndSlopeForEdgeRadial(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,DTM_SIDE_SLOPE_TABLE *Radial,long SideSlopeDirection,long *PlaneSolution)
/*
**
** This Is The Controlling Routine For Calculating
** The Angle And Slope For An Edge Radial. Firstly Try For A Planar Solution,
** If No Solution Then Adjust Angle And Slope For A Level Side Slope Element
**
*/
{
 long      dbg=0,Solution,CloseFlag ;
 double    ang,Slope,Angle ;
 DTM_SIDE_SLOPE_TABLE *priorRadial=nullptr,*nextRadial=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Angle And Slope For Edge Radial") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Radial = [%4ld]  Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(Radial-SideSlopeTable),Radial->radialType,Radial->radialStartPoint.x,Radial->radialStartPoint.y,Radial->radialStartPoint.z) ;
/*
** Initialise
*/
 *PlaneSolution = 1 ;
/*
** Check For Closed Or Open Side Slope Element
*/
 CloseFlag = 0 ;
 if( SideSlopeTable->radialStartPoint.x == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.x &&
     SideSlopeTable->radialStartPoint.y == (SideSlopeTable+SideSlopeTableSize-1)->radialStartPoint.y    ) CloseFlag = 1 ;
 /*
** Get Prior Radial
*/
 priorRadial = Radial - 1 ;
 if( priorRadial < SideSlopeTable )
   {
    if(! CloseFlag ) priorRadial = nullptr ;
    else             priorRadial = SideSlopeTable + SideSlopeTableSize - 1 ;
   }
 if( priorRadial != nullptr && priorRadial->radialStartPoint.x == Radial->radialStartPoint.x && priorRadial->radialStartPoint.y == Radial->radialStartPoint.y ) priorRadial = nullptr ;
/*
** Get Next Radial
*/
 nextRadial = Radial + 1 ;
 if( nextRadial > SideSlopeTable + SideSlopeTableSize - 1 )
   {
    if(! CloseFlag ) nextRadial = nullptr ;
    else             nextRadial = SideSlopeTable  ;
   }
 if( nextRadial != nullptr && nextRadial->radialStartPoint.x == Radial->radialStartPoint.x && nextRadial->radialStartPoint.y == Radial->radialStartPoint.y ) nextRadial = nullptr ;
/*
** Write Prior And Next Radials Radials
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"priorRadial = %p nextRadial = %p",priorRadial,nextRadial) ;
    if( priorRadial != nullptr ) bcdtmWrite_message(0,0,0,"prior Radial Offset = %6ld",(long)(priorRadial-SideSlopeTable)) ;
    if( nextRadial  != nullptr ) bcdtmWrite_message(0,0,0,"next  Radial Offset = %6ld",(long)(nextRadial-SideSlopeTable)) ;
   }
/*
** If Corner Type 2 Radial Just Leave Current Angles
*/
 if( priorRadial == nullptr && nextRadial == nullptr )  Radial->radialSolution = 1 ;
/*
** Check For Slope Transition
*/
 else if( priorRadial != nullptr && ( ( priorRadial->radialSlope > 0.0 && Radial->radialSlope < 0.0 ) || ( priorRadial->radialSlope < 0.0 && Radial->radialSlope > 0.0 ))) Radial->radialSolution = 1 ;
 else if( nextRadial  != nullptr && ( ( nextRadial->radialSlope  > 0.0 && Radial->radialSlope < 0.0 ) || ( nextRadial->radialSlope  < 0.0 && Radial->radialSlope > 0.0 ))) Radial->radialSolution = 1 ;
/*
** If Edge Radial Calculate Planar Angles And Slope
*/
 else
   {
    if( bcdtmSideSlope_calculatePlanarAngleAndSlopeForEdgeRadial(SideSlopeTable,SideSlopeDirection,priorRadial,Radial,nextRadial,&Solution,&Angle,&Slope ) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Planar Solution ** Angle = %10.8lf Slope = %10.4lf",Angle,Slope) ;
/*
** Check For Solution
*/
    Solution = 1 ;
    if( ( Radial->radialSlope > 0.0 && Slope < 0.0 ) || ( Radial->radialSlope < 0.0 && Slope > 0.0 ) ) Solution = 0 ;
    else
      {
/*
**     Allow A 15 Degree Variation In Angle ** 15 Degrees Arbitrarily Chosen
*/
       ang = fabs(Radial->radialAngle - Angle) ;
       if( ang > DTM_PYE ) ang -= DTM_PYE ;
       if( ang > DTM_PYE * 15.0 / 180.0 )
         {
          Solution = 0 ;
          Slope = Radial->radialSlope ;
          Angle = Radial->radialAngle ;
         }
/*
** Allow A 25% Variation In Slope ** 25% variation Arbitrarily Chosen
*/
       if( (Radial->radialSlope-Slope) / Radial->radialSlope > 0.25 )
         {
          Solution = 0 ;
          if( Slope > Radial->radialSlope ) Slope = Radial->radialSlope + Radial->radialSlope * 0.25 ;
          if( Slope < Radial->radialSlope ) Slope = Radial->radialSlope - Radial->radialSlope * 0.25 ;
         }
      }
    if( dbg &&   Solution ) bcdtmWrite_message(0,0,0,"Planar Solution Found") ;
    if( dbg && ! Solution ) bcdtmWrite_message(0,0,0,"No Planar Solution Found") ;
/*
** Set Angle And Slope For Radial
*/
    Radial->radialAngle = Angle ;
    Radial->radialSlope = Slope ;
    Radial->radialSolution = Solution ;
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Solution = %1ld Angle = %10.8lf Slope = %10.4lf",Solution,Angle,Slope) ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Return
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmSideSlope_calculatePlanarAngleAndSlopeForEdgeRadial           |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_calculatePlanarAngleAndSlopeForEdgeRadial(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeDirection,DTM_SIDE_SLOPE_TABLE *PriorRadial,DTM_SIDE_SLOPE_TABLE *Radial,DTM_SIDE_SLOPE_TABLE *NextRadial,long *PlaneSolution,double *Angle,double *Slope )
/*
** This Function Calculates The Planar Angles And Slopes For A Corner Radial
** By Determing The Line Of Intersection Between The Prior And Next Planes At
** The Corner Radial
*/
{
 long    dbg=0,priorSolution,nextSolution ;
 double  x,y,Z1,Z2,A0=0.0,B0=0.0,C0=0.0,D0=0.0,A1=0.0,B1=0.0,C1=0.0,D1=0.0,angle ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Planar Angle And Slope For Edge Radial") ;
/*
** Initialise
*/
 *PlaneSolution = 0 ;
 *Angle = *Slope = 0.0 ;
/*
** Check Prior And/Or Next Radials Are Active
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"priorRadial = %p nextRadial = %p",PriorRadial,NextRadial) ;
    if( dbg && PriorRadial != nullptr ) bcdtmWrite_message(0,0,0,"prior Radial Offset = %6ld",(long)(PriorRadial-SideSlopeTable)) ;
    if( dbg && NextRadial  != nullptr ) bcdtmWrite_message(0,0,0,"next  Radial Offset = %6ld",(long)(NextRadial-SideSlopeTable)) ;
   }
 if( PriorRadial == nullptr && NextRadial == nullptr )
   {
    bcdtmWrite_message(1,0,0,"No Active Radials For Edge Radial Planar Calculations") ;
    goto errexit ;
   }
/*
**  Calculate Plane Coefficients
*/
 priorSolution = nextSolution = 0 ;
 if( PriorRadial != nullptr ) if( bcdtmSideSlope_calculatePlaneCoefficientsForEdgeRadial(PriorRadial,Radial,SideSlopeDirection,&priorSolution,&A0,&B0,&C0,&D0)) goto errexit ;
 if( NextRadial  != nullptr ) if( bcdtmSideSlope_calculatePlaneCoefficientsForEdgeRadial(Radial,NextRadial ,SideSlopeDirection,&nextSolution, &A1,&B1,&C1,&D1)) goto errexit ;
/*
** Check For A Solution
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"PriorSolution = %1ld NextSolution = %1ld",priorSolution,nextSolution) ;
 if( priorSolution || nextSolution )
   {
    *PlaneSolution = 1 ;
/*
** Calculate Radial Endpoints
*/
    angle = Radial->radialAngle ;
    x = Radial->radialStartPoint.x + 200.0 * cos(angle) ;
    y = Radial->radialStartPoint.y + 200.0 * sin(angle) ;
/*
** Calculate Slope
*/
 if     (   priorSolution && ! nextSolution )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Prior Plane Solution") ;
    bcdtmMath_interpolatePointOnPlane(x,y,&Z1,A0,B0,C0,D0) ;
    *Slope = ( Z1 - Radial->radialStartPoint.z ) / 200.0 ;
    *Angle = angle ;
   }
 else if(   priorSolution &&   nextSolution )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Interpolating Prior And Next Plane Solution") ;
    bcdtmMath_interpolatePointOnPlane(x,y,&Z1,A0,B0,C0,D0) ;
    bcdtmMath_interpolatePointOnPlane(x,y,&Z2,A1,B1,C1,D1) ;
    *Angle = angle ;
    *Slope = ( (Z1+Z2) / 2.0  - Radial->radialStartPoint.z ) / 200.0 ;
   }
 else if( ! priorSolution &&   nextSolution )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Next Plane Solution") ;
    bcdtmMath_interpolatePointOnPlane(x,y,&Z2,A1,B1,C1,D1) ;
    *Slope = ( Z2 - Radial->radialStartPoint.z ) / 200.0 ;
    *Angle = angle ;
   }
/*
** Write Slope For Development Purposes
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"****Slope = %10.4lf",*Slope) ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_calculatePlaneCoefficientsForEdgeRadials             |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_calculatePlaneCoefficientsForEdgeRadial(DTM_SIDE_SLOPE_TABLE *Radial1,DTM_SIDE_SLOPE_TABLE *Radial2,long SideSlopeDirection,long *PlaneSolution,double *A,double *B,double *C,double *D)
{
 long   dbg=0 ;
 double x,y,z,dx,dy,dz,dd,d1,angle,slope,alpha,pslope,sloperatio ;
/*
** Initialise
*/
 *PlaneSolution = 1 ;
 *A = *B = *C = *D = 0.0 ;
 angle = Radial1->radialAngle ;
 slope = Radial1->radialSlope ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Radial Angle = %10.8lf Radial Slope = %10.8lf",angle,slope) ;
/*
** Calculate Parameters
*/
 dx = Radial2->radialStartPoint.x - Radial1->radialStartPoint.x ;
 dy = Radial2->radialStartPoint.y - Radial1->radialStartPoint.y ;
 dz = Radial2->radialStartPoint.z - Radial1->radialStartPoint.z ;
 dd = sqrt(dx*dx+dy*dy) ;
/*
** Check For Solution
*/
 if( slope != 0.0 ) d1 = dz/slope ;
 else               d1 = 0.0 ;
 if( fabs(d1) > dd ) *PlaneSolution = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"PlaneSolution = %1ld dz = %10.4lf  dd = %10.4lf d1 = %10.4lf",*PlaneSolution,dz,dd,d1) ;
/*
** Ammend Slope For Non Level Line Between Radial1 And Radial2
*/
 if( *PlaneSolution )
   {
    if( fabs(dz) > 0.0 && slope != 0.0 )
      {
/*
** Calculate Angle Of Normal To Plane With XY Plane
*/
       alpha = acos(d1/dd) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"00 ** dz = %10.5lf ** alpha = %10.8lf",dz,alpha) ;
       alpha = DTM_PYE/2.0 - alpha ;
       if( dbg ) bcdtmWrite_message(0,0,0,"01 ** dz = %10.5lf ** alpha = %10.8lf",dz,alpha) ;
/*
** Calculate Angle Of Radial Along Normal
*/
       if( ( SideSlopeDirection == 1 && Radial1 < Radial2 ) || ( SideSlopeDirection == 2 && Radial1 > Radial2 ) )
         {
          if( dz >= 0.0 ) angle = angle + alpha ;
          else            angle = angle - alpha ;
         }
       if( ( SideSlopeDirection == 1 && Radial1 > Radial2 ) || ( SideSlopeDirection == 2 && Radial1 < Radial2 ) )
         {
          if( dz >= 0.0 ) angle = angle - alpha ;
          else            angle = angle + alpha ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"angle = %10.8lf",angle) ;
      }
/*
** Calculate Third Point On Plane
*/
    x = Radial1->radialStartPoint.x + 200.0 * cos(angle) ;
    y = Radial1->radialStartPoint.y + 200.0 * sin(angle) ;
    z = Radial1->radialStartPoint.z + 200.0 * slope  ;
/*
** Calculate Plane Coefficients
*/
    bcdtmMath_calculatePlaneCoefficients(Radial1->radialStartPoint.x,Radial1->radialStartPoint.y,Radial1->radialStartPoint.z,Radial2->radialStartPoint.x,Radial2->radialStartPoint.y,Radial2->radialStartPoint.z,x,y,z,A,B,C,D) ;
/*
** Calulate Plane Slope For Development And Checking Purpose
*/
    if( dbg )
      {
       pslope = tan( acos(*C / sqrt( (*A)*(*A)  + (*B)*(*B) + (*C)*(*C) ))) ;
       if( slope == 0.0 ) sloperatio = fabs(pslope-slope) ;
       else               sloperatio = fabs(pslope-slope)/slope ;
       if( sloperatio < 0.01 )   bcdtmWrite_message(0,0,0,"Rad->radialStartPoint.Slope = %10.4lf Plane Slope = %10.4lf",slope,pslope) ;
       else                    { bcdtmWrite_message(0,0,0,"**DTM_ERROR** Rad->radialStartPoint.Slope = %10.4lf Plane Slope = %10.4lf",slope,pslope) ; goto errexit ; }
      }
/*
** Calculate Slope For Normal ** Development And Checking Only
*/
    if( dbg )
      {
       angle = Radial1->radialAngle ;
       x = Radial1->radialStartPoint.x + 200 * cos(angle) ;
       y = Radial1->radialStartPoint.y + 200 * sin(angle) ;
       bcdtmMath_interpolatePointOnPlane(x,y,&z,*A,*B,*C,*D) ;
       slope = ( z - Radial1->radialStartPoint.z ) / 200 ;
       bcdtmWrite_message(0,0,0,"Slope Of Normal = %10.4lf",slope) ;
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_extendHorizontalLimitForCornerRadials(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long SideSlopeDirection)
{
 int      dbg=0 ;
 double   ang,angp,angn,angb ;
 DTM_SIDE_SLOPE_TABLE  *radial,*priorRadial,*nextRadial ;

/*
** Scan Side Slope Table And Extend Horizontal Limits For Corner Radials
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial )
   {
    if( ( radial->radialType == 1 || radial->radialType == 3 ) &&
        ( radial->sideSlopeOption == 3 || radial->sideSlopeOption == 6 ) )

      {
       priorRadial = nextRadial = radial ;
       while( priorRadial->radialStartPoint.x == radial->radialStartPoint.x && priorRadial->radialStartPoint.y == radial->radialStartPoint.y ) { --priorRadial ; if( priorRadial <  SideSlopeTable ) priorRadial = SideSlopeTable + SideSlopeTableSize - 2 ; }
       while( nextRadial->radialStartPoint.x  == radial->radialStartPoint.x && nextRadial->radialStartPoint.y  == radial->radialStartPoint.y ) { ++nextRadial  ; if( nextRadial  >= SideSlopeTable + SideSlopeTableSize ) nextRadial = SideSlopeTable  + 1 ; }
       angp = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,priorRadial->radialStartPoint.x,priorRadial->radialStartPoint.y) ;
       angn = bcdtmMath_getAngle(radial->radialStartPoint.x,radial->radialStartPoint.y,nextRadial->radialStartPoint.x,nextRadial->radialStartPoint.y) ;
/*
**  Extend To Right
*/
       if( SideSlopeDirection == 1 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Extending To Right") ;
          if( angn < angp ) angn = angn + DTM_2PYE ;
          angb = ( angn + angp ) / 2.0 ;
          if( angb > DTM_2PYE ) angb -= DTM_2PYE ;
          ang  = angn - angp ;
          if( ang < DTM_PYE )
            {
/*
** Extend Limit At Bisector Angle
*/
             ang = ang / 2.0 ;
             radial->toHorizOffset = radial->toHorizOffset / sin(ang) ;
             if( radial->toHorizOffset < 0.0 ) radial->toHorizOffset = -radial->toHorizOffset ;
/*
** Extend Limit At Plane Intersection Angle
*/
             if( angb < radial->radialAngle ) angb += DTM_2PYE ;
             angb = angb - radial->radialAngle ;
             if( angb >= DTM_2PYE ) angb -= DTM_2PYE ;
             if( angb >  DTM_PYE  ) angb -= DTM_PYE  ;
             radial->toHorizOffset = radial->toHorizOffset / cos(angb) ;
             if( radial->toHorizOffset < 0.0 ) radial->toHorizOffset = -radial->toHorizOffset ;
            }
         }
/*
** Extend To Left
*/
       if( SideSlopeDirection == 2 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Extending To Left") ;
          if( angp < angn ) angp = angp + DTM_2PYE ;
          ang  = angp - angn ;
          angb = ( angn + angp ) / 2.0 ;
          if( angb > DTM_2PYE ) angb -= DTM_2PYE ;
          if( ang < DTM_PYE )
            {
/*
** Extend Limit At Bisector Angle
*/
             ang = ang / 2.0 ;
             radial->toHorizOffset = radial->toHorizOffset / sin(ang) ;
             if( radial->toHorizOffset < 0.0 ) radial->toHorizOffset = -radial->toHorizOffset ;
/*
** Extend Bisector Limit At Plane Intersection Angle
*/
             if( angb < radial->radialAngle) angb += DTM_2PYE ;
             angb = angb - radial->radialAngle ;
             if( angb >= DTM_2PYE ) angb -= DTM_2PYE ;
             if( angb >  DTM_PYE  ) angb -= DTM_PYE  ;
             radial->toHorizOffset = radial->toHorizOffset / cos(angb) ;
             if( radial->toHorizOffset < 0.0 ) radial->toHorizOffset = -radial->toHorizOffset ;
            }
         }
      }
   }
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
int bcdtmSideSlope_intersectRadialsWithSurface(DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,long Status )
/*
** This Function Calculates The Radial Intersections With The Tin Surface
** Only Radials That Match The Status Are Intersected
*/
{
 int     ret=0,dbg=0 ;
 long    StartFlag,EndFlag ;
 double  sX,sY,sZ,hX,hY,hZ,endValue ;
 DTM_SIDE_SLOPE_TABLE  *radial   ;
/*
** Scan Radial Table And Calculate Radial Intersions
*/
 for( radial = SideSlopeTable ; radial < SideSlopeTable + SideSlopeTableSize ; ++radial)
   {
    if( radial->radialStatus == Status )
      {
       if     ( radial->sideSlopeOption == 2 || radial->sideSlopeOption == 5 ) endValue = radial->toElev ;
       else if( radial->sideSlopeOption == 3 || radial->sideSlopeOption == 6 ) endValue = radial->toHorizOffset ;
       else if( radial->sideSlopeOption == 4 || radial->sideSlopeOption == 7 ) endValue = radial->toDeltaElev ;
       else   endValue = 0.0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Start Radial[%6ld] Slope Option = %2ld ** %10.4lf %10.4lf %10.4lf ** End Value = %10.4lf",(long)(radial-SideSlopeTable),radial->sideSlopeOption,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,endValue) ;
/*
** Set Radial Start Point
*/
       sX = radial->radialStartPoint.x ;
       sY = radial->radialStartPoint.y ;
       sZ = radial->radialStartPoint.z ;
/*
** Project Radial To Tin Hull If Necessary
*/
       if( radial->isForceSlope && ( radial->sideSlopeOption == 1 || ( radial->sideSlopeOption >= 5 && radial->sideSlopeOption <=7 ))  )
         {
          if( bcdtmSideSlope_projectVectorStartToHullDtmObject((BC_DTM_OBJ *)radial->slopeToTin,sX,sY,sZ,radial->radialAngle,radial->radialSlope,&hX,&hY,&hZ) ) goto errexit ;
          sX = hX ;
          sY = hY ;
          sZ = hZ ;
         }
/*
** Intersect Tin Surface
*/
       if( bcdtmSideSlope_intersectSurfaceDtmObject((BC_DTM_OBJ *)radial->slopeToTin,sX,sY,sZ,radial->radialAngle,radial->radialSlope,radial->sideSlopeOption,endValue,&radial->radialEndPoint.x,&radial->radialEndPoint.y,&radial->radialEndPoint.z,&StartFlag,&EndFlag)) goto errexit ;
/*
** Drape Radial End Point On slopeTo Tin For slopeTo Options With Limits
*/
       if( radial->sideSlopeOption == 5 || radial->sideSlopeOption == 6 || radial->sideSlopeOption == 7 )
         {
         long flag = 0;
          if( bcdtmDrape_pointDtmObject((BC_DTM_OBJ *)radial->slopeToTin,radial->radialEndPoint.x,radial->radialEndPoint.y,&radial->surfaceZ,&EndFlag)) goto errexit ;
         if (EndFlag == 0 && flag == 0)
                EndFlag = 1; // Outside
         }
/*
** Reset Radial Status To One - Stroked Corner Radials have a radialStatus of 2
*/
       radial->radialStatus = (StartFlag == 0 && (EndFlag == 0 || EndFlag == 3)) ? 1:0;
      }
   }
/*
** Job Completed
*/
 cleanup :
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_projectVectorStartToHullDtmObject(BC_DTM_OBJ *tinP,double Sx,double Sy,double Sz,double xyAngle,double slope,double *Hx, double *Hy,double *Hz)
{
/*
** This Function Extends The Vector Start Point To The Tin Hull
** Assumes Vector Start Point Is External To Hull Or In Void
*/
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   ofs,numDrapePts ;
 long   pntType,P1,P2,P3,startFlag,inVoidFlag ;
 double dl,z,radius ;
 DPoint3d    p3dPts[2] ;
 DTM_DRAPE_POINT *drapeP,*drapePts=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Projecting Start Point Onto Hull") ;
    bcdtmWrite_message(0,0,0,"xyAngle = %15.5lf",xyAngle) ;
    bcdtmWrite_message(0,0,0,"Sx = 15.5lf Sy = %15.5lf Sz = %15.5lf",Sx,Sy,Sz) ;
   }
/*
** Initialise
*/
 *Hx = Sx ;
 *Hy = Sy ;
 *Hz = Sz ;
/*
** Find Location Of Vector Start Point
*/
 startFlag = 0 ;
 bcdtmFind_triangleForPointDtmObject(tinP,Sx,Sy,&z,&pntType,&P1,&P2,&P3) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Sz = %10.4lf DrapeZ = %10.4lf Sz-DrapeZ = %10.4lf",Sz,z,(Sz-z)) ;
 if ( ! pntType    ) startFlag = 2 ;
/*
** Check If Start Point In Void
*/
 if( pntType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(tinP,P1)->PCWD) ) startFlag = 1 ;
 if( pntType == 2 || pntType == 3 )
   {
    if( bcdtmList_testForVoidLineDtmObject(tinP,P1,P2,&inVoidFlag)) goto errexit ;
    if( inVoidFlag ) startFlag = 1 ;
   }
 if( pntType == 4 )
   {
    if( bcdtmList_testForVoidTriangleDtmObject(tinP,P1,P2,P3,&inVoidFlag)) goto errexit ;
    if( inVoidFlag )  startFlag = 1 ;
   }
/*
**  Write Location Of Start Point
*/
 if( dbg )
   {
    if( startFlag == 0 ) bcdtmWrite_message(0,0,0,"Start Point Internal To Tin") ;
    if( startFlag == 1 ) bcdtmWrite_message(0,0,0,"Start Point In Void") ;
    if( startFlag == 2 ) bcdtmWrite_message(0,0,0,"Start Point External To Tin") ;
   }
/*
** Project Vector Start To Tin Hull
*/
 if( startFlag  )
   {
/*
**  Determine Arbitary Vector End Points
*/
    radius = sqrt(tinP->xRange*tinP->xRange+tinP->yRange*tinP->yRange) * 10 ;
    p3dPts[1].x = Sx + radius * cos(xyAngle) ;
    p3dPts[1].y = Sy + radius * sin(xyAngle) ;
/*
**  Drape Vector On Tin
*/
    p3dPts[0].x = Sx ;
    p3dPts[0].y = Sy ;
    if( bcdtmDrape_stringDtmObject(tinP,p3dPts,2,false,&drapePts,&numDrapePts)) goto errexit ;
/*
**  Scan To First Drape Point On Tin
*/
    drapeP = drapePts ;
    while (drapeP < drapePts + numDrapePts && drapeP->drapeType == DTMDrapedLineCode::External) ++drapeP;
    if( drapeP >= drapePts + numDrapePts ) --drapeP ;
/*
**  Set Projected Point Marginally Inside Tin Hull
*/
    ofs = (long) ( drapeP-drapePts ) ;
    *Hx = (drapePts+ofs)->drapeX + 0.00001 * cos(xyAngle) ;
    *Hy = (drapePts+ofs)->drapeY + 0.00001 * sin(xyAngle) ;
    dl  = bcdtmMath_distance(Sx,Sy,*Hx,*Hy) ;
    *Hz = Sz + dl * slope ;
/*
** Adjust Hull z For Slope
*/
    bcdtmFind_triangleForPointDtmObject(tinP,*Hx,*Hy,&z,&pntType,&P1,&P2,&P3) ;
    if( slope < 0.0 && *Hz < z ) *Hz = z ;
    if( slope > 0.0 && *Hz > z ) *Hz = z ;
/*
** Write Point Projection On Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Hx = %15.5lf Hy = %15.5lf Hz = %15.5lf",*Hx,*Hy,*Hz) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePts != nullptr ) bcdtmDrape_freeDrapePointMemory(&drapePts,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting Start Point Onto Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting Start Point Onto Hull Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
//
// This is a copy of the private dtm function bcdtmDrape_findClosestHullLineDtmObject
// SS3 branche only.
// Need to make bcdtmDrape_findClosestHullLineDtmObject public
//
int bcdtmSideSlope_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *pnt1P,long *pnt2P)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 long   p1,p2,isw,lf ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn  ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Initialise
*/
 *pnt1P = *pnt2P = dtmP->nullPnt ;
/*
** Find Closest Hull Line
*/
 isw = 1 ;
 p1  = dtmP->hullPoint ;
 p1P = pointAddrP(dtmP,p1) ;
 do
   {
    p2  = nodeAddrP(dtmP,p1)->hPtr ;
    p2P = pointAddrP(dtmP,p2) ;
        if( bcdtmMath_sideOf(p1P->x, p1P->y, p2P->x, p2P->y, x, y) < 0 )
          {
           d1 = bcdtmMath_distance(p1P->x, p1P->y, x, y);
           d2 = bcdtmMath_distance(p2P->x, p2P->y, x, y);
           d3 = bcdtmMath_distance((p1P->x+p2P->x) / 2.0,(p1P->y+p2P->y)/2.0, x, y);
       d4 = bcdtmMath_distanceOfPointFromLine(&lf, p1P->x, p1P->y, p2P->x, p2P->y, x, y, &Xn, &Yn);
           if( isw )
             {
                  *pnt1P = p1 ;
                  *pnt2P = p2 ;
          dn = d1 ;
                  if( d2 < dn ) dn = d2 ;
                  if( d3 < dn ) dn = d3 ;
          if( lf && d4 < dn ) dn = d4 ;
                  isw = 0 ;
             }
       else
             {
                  if( d1 < dn || d2 < dn || d3 < dn || ( lf && d4 < dn ) )
                    {
                     *pnt1P = p1 ;
                         *pnt2P = p2 ;
                     if( d1 < dn ) dn = d1 ;
                     if( d2 < dn ) dn = d2 ;
                     if( d3 < dn ) dn = d3 ;
             if( lf && d4 < dn ) dn = d4 ;
                        }
                 }
      }
    p1  = p2 ;
        p1P = p2P ;
   } while ( p1 != dtmP->hullPoint ) ;
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
int bcdtmSideSlope_intersectSurfaceDtmObject(BC_DTM_OBJ *Tin,double Sx,double Sy,double Sz,double Angle,double Slope,long SideSlopeFlag,double EndValue,double *Lx, double *Ly,double *Lz,long *StartFlag,long *EndFlag)
/*
** This Function Intersects a 3D Vector With A Tin Surface
**
**  Return Values  =  0 Succesfull
**                 =  1 User Error
**                 =  2 Bentley Civil DTM Error ( Terminate )
**  Sx             ==>  x Coordinate 3D Vector
**  Sy             ==>  y Coordinate 3D Vector
**  Sz             ==>  z Coordiante 3D Vector
**  Angle          ==>  Vector Angle in XY Plane expressed in Radians
**  Slope          ==>  Cut/Fill Slope expressed as ratio of rise/run
**  SideSlopeFlag  ==   1 Intersect TIN Surface
**                 ==   2 Calculate Radial To  Elevation in EndValue At Slope
**                 ==   3 Calculate Radial Out Distance  in EndValue
**                 ==   4 Calculate Radial Up/Down Vertical Distance In EndValue
**                 ==   5 Calculate Radial To  Elevation in EndValue At Slope or TIN Surface Whichever Comes First
**                 ==   6 Calculate Radial Out Distance  in EndValue or TIN Surface Whichever Comes First
**                 ==   7 Calculate Radial Up/Down Vertical Distance  in EndValue or TIN Surface Whichever Comes First
**                 ==   8 Calculate Radial Out Distance  in EndValue
**  EndValue       ==   Terminating Value To be used for Side Slope Flag.
**  Lx             <==  x Coordinate Of Vector Intersect With Tin
**  Ly             <==  y Coordinate Of Vector Intersect With Tin
**  Lz             <==  z Coordinate Of Vector Intersect With Tin
**  StartFlag      <==  0 Vector Point Internal to TIN
**                 <==  1 Vector Point In Void
**                 <==  2 Vector Point External To Tin
**                 <==  3 Slope Solution Not Possible
**  EndFlag        <==  0 - Slope Vector Terminated On Tin Surface
**                      1 - Slope Vector Terminated On Void Hull
**                      2 - Slope Vector Terminated On Tin  Hull
**                      3 - Slope Vector Not Intersected
*/
{
 long   dbg=0,P1,P2,P3,Ptype=0,process ;
 double z,Ex,Ey,Ez,radius ;
 double px,py,pzt,pzs,nx=0.0,ny=0.0,nzt=0.0,nzs=0.0,zdp,zdn ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sx = %10.4lf Sy = %10.4lf Sz = %10.4lf ** Slope = %10.5lf Angle = %10.8lf SideSlope Flag = %2ld End Value = %10.4lf",Sx,Sy,Sz,Slope,Angle,SideSlopeFlag,EndValue) ;
/*
** Initialise Variables
*/
 zdp = zdn = 0.0 ;
 *StartFlag = 0 ; *EndFlag = 3 ;
 *Lx = *Ly = *Lz = 0.0 ;
/*
** Adjust Limit Values To Positive
*/
 if( SideSlopeFlag == 3 || SideSlopeFlag == 4 || SideSlopeFlag == 8 ||
     SideSlopeFlag == 6 || SideSlopeFlag == 7                          )
   {
    if( EndValue < 0.0 ) EndValue = -EndValue ;
   }
/*
** Process Non TIN Intersection Radials
*/
 if( ( SideSlopeFlag >= 2 && SideSlopeFlag <= 4 ) || SideSlopeFlag == 8  )
   {
/*
**  Radial Extension To Set Elevation
*/
    if( SideSlopeFlag == 2 )
      {
       *Lz = EndValue ;
       if( Slope != 0.0 ) radius = fabs(*Lz-Sz) / fabs(Slope) ;
       else               { *Lz = Sz ;  radius = 10000.0 ; }
       *Lx = Sx + radius * cos(Angle) ;
       *Ly = Sy + radius * sin(Angle) ;
       return(0) ;
      }
/*
** Extend Radial Out For A Distance At Angle And Slope
*/
   if( SideSlopeFlag == 3 || SideSlopeFlag == 8 )
     {
      *Lz = Sz +  Slope * EndValue ;
      *Lx = Sx + EndValue * cos(Angle) ;
      *Ly = Sy + EndValue * sin(Angle) ;
/*
**  Write Distance Out ** Development Only
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Horizontal Extension = %10.6lf",bcdtmMath_distance(Sx,Sy,*Lx,*Ly)) ;
       return(0) ;
      }
/*
** Extend Radial Out At Slope And Angle For A Vertical Distance
*/
    if( SideSlopeFlag == 4 )
      {
       if( Slope >= 0.0 ) EndValue = Sz + EndValue ;
       else               EndValue = Sz - EndValue ;
       *Lz = EndValue ;
       radius = fabs(*Lz-Sz) / fabs(Slope) ;
       *Lx = Sx + radius * cos(Angle) ;
       *Ly = Sy + radius * sin(Angle) ;
       return(0) ;
      }
   }
/*
** Find Point On Tin Surface
*/
 bcdtmFind_triangleForPointDtmObject(Tin,Sx,Sy,&z,&Ptype,&P1,&P2,&P3) ;
 if (Ptype == 0)
     {
    long drapeFlag;
     bcdtmDrape_pointDtmObject (Tin,Sx,Sy,&z, &drapeFlag);
     if (drapeFlag == 0)
        {
        *StartFlag = 2;
        return(0);
         }
     if (drapeFlag == 2)
        {
        *StartFlag = 1;
        return(0);
        }

     bcdtmSideSlope_findClosestHullLineDtmObject(Tin, Sx, Sy, &P1, &P2) ;
     if( P1 != Tin->nullPnt && P2 != Tin->nullPnt )
        {
        DTM_TIN_POINT *pnt1P = pointAddrP (Tin, P1);
        DTM_TIN_POINT *pnt2P = pointAddrP (Tin, P2);
        if( bcdtmMath_distance (Sx, Sy, pnt1P->x, pnt1P->y) <= Tin->ppTol )
            {
            P2 = Tin->nullPnt;
            Ptype = 1;
            }
        else if( bcdtmMath_distance(Sx, Sy, pnt2P->x, pnt2P->y) <= Tin->ppTol )
            {
            P1 = P2;
            P2 = Tin->nullPnt;
            Ptype = 1;
            }
        else if (bcdtmMath_normalDistanceToLineDtmObject(Tin, P1, P2, Sx, Sx) <= Tin->ppTol )
            Ptype = 3;
        }
     }

   if( dbg ) bcdtmWrite_message(0,0,0,"Sz = %10.4lf DrapeZ = %10.4lf Sz-DrapeZ = %10.4lf",Sz,z,(Sz-z)) ;
/*
** Check If Slope Solution Possible For SideSlopeFlag Options 1 & 3
*/
 if( SideSlopeFlag == 2 || SideSlopeFlag == 5 )
   {
    if( Slope == 0.0 &&  EndValue != Sz ) { *StartFlag = 3 ; return(0) ; }
    if( Slope >  0.0 &&  EndValue <  Sz ) { *StartFlag = 3 ; return(0) ; }
    if( Slope <  0.0 &&  EndValue >  Sz ) { *StartFlag = 3 ; return(0) ; }
   }
/*
** Test If Vector Point On Surface
*/
 if( fabs(z-Sz) < 0.0001 ) { *Lx = Sx ; *Ly = Sy ; *Lz = Sz ; *EndFlag = Ptype == 4 ? 2 :0 ; return(0) ; }
/*
** Calculate Arbitrary Endpoints For Vector
*/
 radius = sqrt(Tin->xRange*Tin->xRange+Tin->yRange*Tin->yRange) * 2.0 ;
 Ex = Sx + radius * cos(Angle) ;
 Ey = Sy + radius * sin(Angle) ;
 Ez = Sz + radius * Slope      ;
/*
** Set Point Types
*/
 if( Ptype == 2 || Ptype == 3 ) Ptype = 2 ;
 if( Ptype == 4 )  Ptype = 3 ;
/*
** Determine Intersection Point With Tin Surface
*/
 process = 1 ;
 px = Sx ; py = Sy ; pzs = Sz ;  pzt = z  ;
 Ex = Ex ; Ey = Ey ; Ez = Ez  ;
 while ( process )
   {
    if( bcdtmSideSlope_getNextInterceptDtmObject(Tin,px,py,&Ptype,Ex,Ey,&P1,&P2,&P3,&nx,&ny,&nzt)) return(1) ;
    if( Ptype > 3 ) process = 0 ;
    else
      {
       bcdtmMath_interpolatePointOnLine(Sx,Sy,Sz,Ex,Ey,Ez,nx,ny,&nzs) ;
       zdp = pzt - pzs ; zdn = nzt - nzs ;
       if( zdp == 0.0 || zdn == 0.0 || ( zdp > 0.0 && zdn < 0.0 ) || ( zdp < 0.0 && zdn > 0.0 )) process = 0 ;
       else { px = nx ; py = ny ; pzt = nzt ; pzs = nzs ; }
      }
   }
/*
** Radial Terminated On A Hull Boundary
*/
 if( Ptype > 3 )
   {
    *Lx = px ; *Ly = py ; *Lz = pzs ;
    if( Ptype == 4 ) *EndFlag = 2 ;
    if( Ptype == 5 ) *EndFlag = 1 ;
   }
/*
** Radial Terminated Internally To Tin
*/
 else
   {
    *EndFlag = 0 ;
    if( zdp == 0.0 ) { *Lx = px ; *Ly = py ; *Lz = pzt ; return(0) ; }
    if( zdn == 0.0 ) { *Lx = nx ; *Ly = ny ; *Lz = nzt ; return(0) ; }
    if( zdn < 0.0 ) zdn = -zdn ;
    if( zdp < 0.0 ) zdp = -zdp ;
    *Lx = px  + ( nx  - px  ) * zdp / (zdp+zdn) ;
    *Ly = py  + ( ny  - py  ) * zdp / (zdp+zdn) ;
    *Lz = pzs + ( nzs - pzs ) * zdp / (zdp+zdn) ;
   }
/*
** Check If End Conditions Are Reached Before TIN Surface
*/
 if( SideSlopeFlag >= 5 && SideSlopeFlag <= 7 )
   {
/*
** Check If Elevation Is Reached Before TIN Surface
*/
    if( SideSlopeFlag == 5 )
      {
       if( *Lz != Sz )
         {
          if( ( Slope > 0.0 && EndValue < *Lz ) || ( Slope < 0.0 && EndValue > *Lz ) )
            {
             *Lx = Sx  + (*Lx - Sx ) * ( EndValue - Sz ) / ( *Lz - Sz) ;
             *Ly = Sy  + (*Ly - Sy ) * ( EndValue - Sz ) / ( *Lz - Sz) ;
             *Lz = EndValue ;
              // End has been adjusted: reassign EndFlag to 0
              *EndFlag = 0;
            }
         }
/*
**  Write Slope Value ** Development Only
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Slope = %10.6lf",(*Lz-Sz) / bcdtmMath_distance(Sx,Sy,*Lx,*Ly)) ;
      }
/*
** Check If Distance Is Reached Before TIN Surface
*/
    if( SideSlopeFlag == 6 )
      {
       if( EndValue < ( zdn = bcdtmMath_distance(Sx,Sy,*Lx,*Ly)) )
         {
          *Lx = Sx  + (*Lx - Sx ) *  EndValue / zdn ;
          *Ly = Sy  + (*Ly - Sy ) *  EndValue / zdn ;
          *Lz = Sz  + (*Lz - Sz ) *  EndValue / zdn ;
          // End has been adjusted: reassign EndFlag to 0
          *EndFlag = 0;
         }
/*
**  Write Slope Value ** Development Only
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"SideSlopeFlag = 6 ** Lx = %10.4lf Ly = %10.4lf Lz = %10.4lf Lenght = %10.4lf Slope = %10.6lf",*Lx,*Ly,*Lz,bcdtmMath_distance(Sx,Sy,*Lx,*Ly),(*Lz-Sz)/bcdtmMath_distance(Sx,Sy,*Lx,*Ly) ) ;
      }
/*
** Check If Elevation Offset Reached Before TIN Surface
*/
    if( SideSlopeFlag == 7 )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"SideSlope Flag == 7 ** Slope = %10.4lf EndValue = %10.4lf",Slope,EndValue) ;
          bcdtmWrite_message(0,0,0,"Sx = %10.4lf Sy = %10.4lf Sz = %10.4lf",Sx,Sy,Sz) ;
          bcdtmWrite_message(0,0,0,"Lx = %10.4lf Ly = %10.4lf Lz = %10.4lf",*Lx,*Ly,*Lz) ;
         }
       if( Slope >= 0.0 ) EndValue = Sz + EndValue ;
       else               EndValue = Sz - EndValue ;
       if( dbg ) bcdtmWrite_message(0,0,0,"EndValue = %10.4lf",EndValue) ;
       if( *Lz != EndValue )
         {
          if( ( Slope > 0.0 && EndValue < *Lz ) || ( Slope < 0.0 && EndValue > *Lz ) )
            {
             *Lx = Sx  + (*Lx - Sx ) * ( EndValue - Sz ) / ( *Lz - Sz) ;
             *Ly = Sy  + (*Ly - Sy ) * ( EndValue - Sz ) / ( *Lz - Sz) ;
             *Lz = EndValue ;
              // End has been adjusted: reassign EndFlag to 0
              *EndFlag = 0;
            }
         }
/*
**  Write Slope Value ** Development Only
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Lx = %10.4lf Ly = %10.4lf Lz = %10.4lf",*Lx,*Ly,*Lz) ;
          bcdtmWrite_message(0,0,0,"SideSlopeFlag = 7 Slope = %10.6lf Distance = %10.6lf",(*Lz-Sz) / bcdtmMath_distance(Sx,Sy,*Lx,*Ly),bcdtmMath_distance(Sx,Sy,*Lx,*Ly)) ;
         }
      }
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
int bcdtmSideSlope_getNextInterceptDtmObject(BC_DTM_OBJ *Tin,double Px,double Py,long *Ptype,double Ex,double Ey,long *P1,long *P2,long *P3,double *Nx,double *Ny, double *Nz )
/*
** This Function Gets The Next Triangle Intercept with Line PxPy  ExEy
*/
{
 long pa,pc,pn,sp1,sp2,sp3,clc ;
/*
** Initialise Varaibles
*/
 *Nx = *Ny = *Nz = 0.0 ;
/*
** Get Next Intercept on Basis Of Last Intercept
*/
 switch( *Ptype )
   {
    case 1 :  /* Last Intercept Was Point P1 */
/*
**  Find Points connected to P1 that are either side of  Line
*/
      clc = nodeAddrP(Tin,*P1)->cPtr  ;
      pc  = clistAddrP(Tin,clc)->pntNum ;
      if( ( pa = bcdtmList_nextAntDtmObject(Tin,*P1,pc)) < 0 ) return(1) ;
      while ( clc  != Tin->nullPtr )
        {
         pc  = clistAddrP(Tin,clc)->pntNum ;
         clc = clistAddrP(Tin,clc)->nextPtr ;
         if( bcdtmList_testLineDtmObject(Tin,pc,pa) && bcdtmMath_pointSideOfDtmObject(Tin,pc,pa,*P1) > 0 )
           {
            sp1 = bcdtmMath_sideOf(pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,Ex,Ey,pointAddrP(Tin,pc)->x,pointAddrP(Tin,pc)->y) ;
            sp2 = bcdtmMath_sideOf(pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,Ex,Ey,pointAddrP(Tin,pa)->x,pointAddrP(Tin,pa)->y) ;
            sp3 = bcdtmMath_sideOf(pointAddrP(Tin,pc)->x,pointAddrP(Tin,pc)->y,pointAddrP(Tin,pa)->x,pointAddrP(Tin,pa)->y,Ex,Ey) ;
            if( sp1 <= 0 && sp2 >= 0 && sp3 <= 0 )
              {
               if( sp1 == 0 ){ *Ptype = 1 ; *P1 = pc ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,pc)->x ; *Ny = pointAddrP(Tin,pc)->y ; *Nz = pointAddrP(Tin,pc)->z ; return(0) ; }
               if( sp2 == 0 ){ *Ptype = 1 ; *P1 = pa ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,pa)->x ; *Ny = pointAddrP(Tin,pa)->y ; *Nz = pointAddrP(Tin,pa)->z ; return(0) ; }
               bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,pc)->x,pointAddrP(Tin,pc)->y,pointAddrP(Tin,pa)->x,pointAddrP(Tin,pa)->y,Nx,Ny) ;
               if( pointAddrP(Tin,pc)->x == *Nx && pointAddrP(Tin,pc)->y == *Ny )
                 { *Ptype = 1 ; *P1 = pc ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,pc)->x ; *Ny = pointAddrP(Tin,pc)->y ; *Nz = pointAddrP(Tin,pc)->z ; return(0) ; }
               if( pointAddrP(Tin,pa)->x == *Nx && pointAddrP(Tin,pa)->y == *Ny )
                 { *Ptype = 1 ; *P1 = pa ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,pa)->x ; *Ny = pointAddrP(Tin,pa)->y ; *Nz = pointAddrP(Tin,pa)->z ; return(0) ; }
               bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,pa,pc) ;
               *Ptype = 2 ; *P1 = pa ; *P2 = pc ; *P3 = Tin->nullPnt ;
               return(0) ;
              }
           }
         pa  = pc ;
        }
      *Ptype = 4 ; return(0)  ;
    break  ;

    case 2 :  /* Last Intercept Was Line P1-P2 */
/*
**    Check For Going External
*/
      sp1 = *P1;
      sp2 = *P2;
      if( nodeAddrP(Tin,*P1)->hPtr == *P2 )
        {
         sp3 = bcdtmMath_sideOf(pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,Ex,Ey) ;
         if( sp3 <  0 )
           {
            *Ptype = 5 ;
            return (0);
           }
        }
      if( nodeAddrP(Tin,*P2)->hPtr == *P1 )
        {
         sp3 = bcdtmMath_sideOf(pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,Ex,Ey) ;
         if( sp3 <  0 )
           {
            *Ptype = 5 ;
            return (0);
           }
        }
/*
**    Calculate Intersection On Opposite Side Of Triangle
*/

      pa = pc = pn = Tin->nullPnt ;
      if( ( pa = bcdtmList_nextAntDtmObject(Tin,*P1,*P2))   < 0 ) return(1) ;
      if( ( pc = bcdtmList_nextClkDtmObject(Tin,*P1,*P2)) < 0 ) return(1) ;
      if( ! bcdtmList_testLineDtmObject(Tin,pa,*P2) ) pa = Tin->nullPnt ;
      if( ! bcdtmList_testLineDtmObject(Tin,pc,*P2) ) pc = Tin->nullPnt ;
      sp1 = bcdtmMath_sideOf(pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,Ex,Ey) ;
      if( sp1 == 0 )
        {
         if( bcdtmMath_distance(Ex,Ey,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y) <= bcdtmMath_distance(Ex,Ey,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y) )
              { *Ptype = 1 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
         else { *Ptype = 1 ; *P1 = *P2 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
        }
      if( sp1 >  0 ) pn = pa ;
      if( sp1 <  0 ) pn = pc ;
      if( pn == Tin->nullPnt ) { *Ptype = 4 ; return(0) ; } /* On Tin Hull */
      if( bcdtmList_testForVoidHullLineDtmObject(Tin,*P1,*P2)){ *Ptype = 5 ; return(0) ; } ;
      *P3 = pn ;
      sp3 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y) ;
      if( sp3 == 0 ) { *Ptype = 1 ; *P1 = *P3 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
      sp1 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y) ;
      sp2 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y) ;
      if( sp1 != sp3 )
        {
         *Ptype = 2 ;
         bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y,Nx,Ny) ;
         if( pointAddrP(Tin,*P1)->x == *Nx && pointAddrP(Tin,*P1)->y == *Ny )
           { *Ptype = 1 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
         if( pointAddrP(Tin,*P3)->x == *Nx && pointAddrP(Tin,*P3)->y == *Ny )
           { *Ptype = 1 ; *P1 = *P3 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
         bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,*P1,*P3) ;
             *P1 = *P1 ; *P2 = *P3 ; *P3 = Tin->nullPnt ;
              return(0) ;
        }
      if( sp2 != sp3 )
        {
         *Ptype = 2 ;
         bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y,Nx,Ny) ;
         if( pointAddrP(Tin,*P2)->x == *Nx && pointAddrP(Tin,*P2)->y == *Ny )
           { *Ptype = 1 ; *P1 = *P2 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
         if( pointAddrP(Tin,*P3)->x == *Nx && pointAddrP(Tin,*P3)->y == *Ny )
           { *Ptype = 1 ; *P1 = *P3 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
         bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,*P2,*P3) ;
             *P1 = *P2 ; *P2 = *P3 ; *P3 = Tin->nullPnt ;
             return(0) ;
        }
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 2 ") ;
      return(1) ;
    break  ;

    case 3 :  /* Last Intercept Was In Triangle P1-P2-P3 */
      if( bcdtmMath_pointSideOfDtmObject(Tin,*P1,*P2,*P3) > 0 ) { pn = *P2 ; *P2 = *P3 ; *P3 = pn ; }
      sp1 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y) ;
      if( sp1 == 0 ) { *Ptype = 1 ; *P1 = *P1 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
      sp2 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y) ;
      if( sp2 == 0 ) { *Ptype = 1 ; *P1 = *P2 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
      sp3 = bcdtmMath_sideOf(Px,Py,Ex,Ey,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y) ;
      if( sp3 == 0 ) { *Ptype = 1 ; *P1 = *P3 ; *P2 = *P3 = Tin->nullPnt ; *Nx = pointAddrP(Tin,*P1)->x ; *Ny = pointAddrP(Tin,*P1)->y ; *Nz = pointAddrP(Tin,*P1)->z ; return(0) ; }
      if( sp1 > 0 && sp2 < 0 )
        {
         *Ptype = 2 ; *P3 = Tin->nullPnt ;
         bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,Nx,Ny) ;
         bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,*P1,*P2) ;
         return(0) ;
        }
      if( sp2 > 0 && sp3 < 0 )
        {
         *Ptype = 2 ;
         bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,*P2)->x,pointAddrP(Tin,*P2)->y,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y,Nx,Ny) ;
         bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,*P2,*P3) ;
         *P1 = *P2 ; *P2 = *P3 ; *P3 = Tin->nullPnt ;
         return(0) ;
        }
      if( sp3 > 0 && sp1 < 0 )
        {
         *Ptype = 2 ;
         bcdtmMath_normalIntersectCordLines(Px,Py,Ex,Ey,pointAddrP(Tin,*P3)->x,pointAddrP(Tin,*P3)->y,pointAddrP(Tin,*P1)->x,pointAddrP(Tin,*P1)->y,Nx,Ny) ;
         bcdtmMath_interpolatePointOnLineDtmObject(Tin,*Nx,*Ny,Nz,*P3,*P1) ;
         *P2 = *P1 ; *P1 = *P3 ; *P3 = Tin->nullPnt ;
         return(0) ;
        }
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 3 ") ;
      return(1) ;
    break  ;

    default :
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 4 ") ;
      return(1) ;
    break ;

   } ;
/*
** Job Completed
*/
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_strokeConvexCorners(DTM_SIDE_SLOPE_TABLE **SideSlopeTable,long *SideSlopeTableSize,long SideSlopeDirection,long CornerOption,double CornerStrokeTolerance)
/*
** This Function Strokes Convex Corners
*/
{
 int       ret=0 ;
 long      dbg=0,radialOfs,numStrokeRadials,MemSideSlopeTableSize,MemTableInc=100 ;
 double    X1,Y1,Z1,X2,Y2,Z2,A=0.0,B=0.0,C=0.0,D=0.0,dist,angle,slope,angleinc=0.0,slopeinc,hor,horinc  ;
 DTM_SIDE_SLOPE_TABLE *radial,*pradial,*nradial,*bradial,*tradial ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Stroking Convex Corners") ;
/*
** Write Side Slope Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Corner Option = %2ld",CornerOption) ;
    bcdtmWrite_message(0,0,0,"SideSlopeDirection = %2ld",SideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld S = %2ld Slope = %10.4lf Angle = %8.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStatus,radial->radialSlope,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
** Initialise
*/
 MemSideSlopeTableSize = *SideSlopeTableSize ;
/*
** Scan Side Slope Table And Stroke Convex Corners
*/
 for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize ; ++radial)
   {
    if( radial->radialStatus && radial->radialType == 3 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"********* Current Radial[%6ld] T = %2ld S = %2ld Slope = %10.4lf Angle = %12.8lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStatus,radial->radialSlope,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
/*
**  Get Prior Radial
*/
       pradial = radial - 1 ;
       if( pradial < *SideSlopeTable ) pradial = *SideSlopeTable + *SideSlopeTableSize - 2 ;
/*
**  Get Prior Distance
*/
       dist = bcdtmMath_distance(pradial->radialEndPoint.x,pradial->radialEndPoint.y,radial->radialEndPoint.x,radial->radialEndPoint.y) ;
       numStrokeRadials = (long)(dist / CornerStrokeTolerance) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Prior Stroke Radials = %6ld",numStrokeRadials) ;
/*
**  Insert Corner Stroke Radials Into Side Slope Table
*/
       if( numStrokeRadials > 0 )
         {
/*
**  Determine Plane Parameters For Straight Corners
*/
          if( CornerOption == 2 )
            {
             X1 = radial->radialStartPoint.x + 200.0 * cos(radial->radialAngle) ;
             Y1 = radial->radialStartPoint.y + 200.0 * sin(radial->radialAngle) ;
             Z1 = radial->radialStartPoint.z + 200.0 * radial->radialSlope ;
             X2 = pradial->radialStartPoint.x + 200.0 * cos(pradial->radialAngle) ;
             Y2 = pradial->radialStartPoint.y + 200.0 * sin(pradial->radialAngle) ;
             Z2 = pradial->radialStartPoint.z + 200.0 * pradial->radialSlope ;
             bcdtmMath_calculatePlaneCoefficients(radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,X1,Y1,Z1,X2,Y2,Z2,&A,&B,&C,&D) ;
            }
/*
**  Determine Horizontal Limit
*/
          hor = pradial->toHorizOffset ;
          horinc = ( radial->toHorizOffset - pradial->toHorizOffset ) / (double) (numStrokeRadials+1) ;
/*
**  Determine Slope Increment
*/
          slope    = pradial->radialSlope ;
          slopeinc = ( radial->radialSlope - pradial->radialSlope ) / (double) (numStrokeRadials+1) ;
/*
**  Determine Angle Increment
*/
          angle  = pradial->radialAngle ;
          if( SideSlopeDirection == 1 )
            {
             if( radial->radialAngle < pradial->radialAngle ) angleinc = ( radial->radialAngle + DTM_2PYE - pradial->radialAngle ) / (double) (numStrokeRadials+1) ;
             else                                             angleinc = ( radial->radialAngle - pradial->radialAngle ) / (double) (numStrokeRadials+1) ;
            }
          if( SideSlopeDirection == 2 )
            {
             if( radial->radialAngle > pradial->radialAngle ) angleinc = ( radial->radialAngle - DTM_2PYE - pradial->radialAngle ) / (double) (numStrokeRadials+1) ;
             else                                             angleinc = ( radial->radialAngle - pradial->radialAngle ) / (double) (numStrokeRadials+1) ;
            }
/*
**  Check Memory And Reallocate If Necessary
*/
          if( *SideSlopeTableSize + numStrokeRadials >= MemSideSlopeTableSize )
            {
             radialOfs = (long)(radial-*SideSlopeTable) ;
             while( MemSideSlopeTableSize <= *SideSlopeTableSize + numStrokeRadials ) MemSideSlopeTableSize += MemTableInc ;
             *SideSlopeTable = ( DTM_SIDE_SLOPE_TABLE *) realloc ( *SideSlopeTable,MemSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
             if ( *SideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
             radial = *SideSlopeTable + radialOfs ;
            }
/*
**  Copy Down numStrokeRadials From Radial
*/
          tradial = *SideSlopeTable + *SideSlopeTableSize + numStrokeRadials - 1 ;
          bradial = *SideSlopeTable + *SideSlopeTableSize - 1 ;
          while ( bradial >= radial ) { *tradial = *bradial ; --bradial ; --tradial ; }
/*
**  Insert Stroke Radials
*/
          ++tradial ;
          hor = hor + horinc ;
          angle = angle + angleinc ;
          slope = slope + slopeinc ;
          while ( radial < tradial )
             {
              *radial = *tradial ;
              radial->radialType   = 2 ;
              radial->radialStatus = 2 ;
              radial->radialAngle  = bcdtmMath_normaliseAngle(angle) ;
              radial->radialSlope  = slope ;
              radial->toHorizOffset = hor ;
              if( CornerOption == 2 )
                {
                 X1 = radial->radialStartPoint.x + 200.0 * cos(radial->radialAngle) ;
                 Y1 = radial->radialStartPoint.y + 200.0 * sin(radial->radialAngle) ;
                 bcdtmMath_interpolatePointOnPlane(X1,Y1,&Z1,A,B,C,D) ;
                 radial->radialSlope = (Z1 - radial->radialStartPoint.z ) / 200.0 ;
                }
              angle = angle + angleinc ;
              slope = slope + slopeinc ;
              hor   = hor   + horinc   ;
              ++radial ;
             }
          *SideSlopeTableSize += numStrokeRadials ;
         }
/*
**  Get Next Radial
*/
       nradial = radial + 1 ;
       if( nradial >= *SideSlopeTable + *SideSlopeTableSize ) nradial = *SideSlopeTable + 1 ;
/*
**  Get Next Distance
*/
       dist = bcdtmMath_distance(nradial->radialEndPoint.x,nradial->radialEndPoint.y,radial->radialEndPoint.x,radial->radialEndPoint.y) ;
       numStrokeRadials = (long)(dist / CornerStrokeTolerance) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Next  Stroke Radials = %6ld",numStrokeRadials) ;
/*
**  Insert Corner Stroke Radials Into Side Slope Table
*/
       if( numStrokeRadials > 0 )
         {
/*
**  Determine Plane Parameters For Straight Corners
*/
          if( CornerOption == 2 )
            {
             X1 = radial->radialStartPoint.x + 200.0 * cos(radial->radialAngle) ;
             Y1 = radial->radialStartPoint.y + 200.0 * sin(radial->radialAngle) ;
             Z1 = radial->radialStartPoint.z + 200.0 * radial->radialSlope ;
             X2 = nradial->radialStartPoint.x + 200.0 * cos(nradial->radialAngle) ;
             Y2 = nradial->radialStartPoint.y + 200.0 * sin(nradial->radialAngle) ;
             Z2 = nradial->radialStartPoint.z + 200.0 * nradial->radialSlope ;
             bcdtmMath_calculatePlaneCoefficients(radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z,X1,Y1,Z1,X2,Y2,Z2,&A,&B,&C,&D) ;
            }
/*
**  Determine Horizontal Limit
*/
          hor = radial->toHorizOffset ;
          horinc = ( nradial->toHorizOffset - radial->toHorizOffset ) / (double) (numStrokeRadials+1) ;
/*
**  Determine Slope Increment
*/
          slope    = radial->radialSlope ;
          slopeinc = ( nradial->radialSlope - radial->radialSlope ) / (double) (numStrokeRadials+1) ;
/*
**  Determine Angle Increment
*/
          angle  = radial->radialAngle ;
          if( SideSlopeDirection == 1 )
            {
             if( nradial->radialAngle <  radial->radialAngle ) angleinc = ( nradial->radialAngle + DTM_2PYE - radial->radialAngle ) / (double) (numStrokeRadials+1) ;
             else                                              angleinc = ( nradial->radialAngle - radial->radialAngle ) / (double) (numStrokeRadials+1) ;
            }
          if( SideSlopeDirection == 2 )
            {
             if( nradial->radialAngle > radial->radialAngle ) angleinc = ( nradial->radialAngle - DTM_2PYE - radial->radialAngle ) / (double) (numStrokeRadials+1) ;
             else                                             angleinc = ( nradial->radialAngle - radial->radialAngle ) / (double) (numStrokeRadials+1) ;
            }
/*
**  Check Memory And Reallocate If Necessary
*/
          if( *SideSlopeTableSize + numStrokeRadials >= MemSideSlopeTableSize )
            {
             radialOfs = (long)(radial-*SideSlopeTable) ;
             while( MemSideSlopeTableSize <= *SideSlopeTableSize + numStrokeRadials ) MemSideSlopeTableSize += MemTableInc ;
             *SideSlopeTable = ( DTM_SIDE_SLOPE_TABLE *) realloc ( *SideSlopeTable,MemSideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
             if ( *SideSlopeTable == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
             radial = *SideSlopeTable + radialOfs ;
            }
/*
**  Copy Down numStrokeRadials From Radial
*/
          tradial = *SideSlopeTable + *SideSlopeTableSize + numStrokeRadials - 1 ;
          bradial = *SideSlopeTable + *SideSlopeTableSize - 1 ;
          while ( bradial >  radial ) { *tradial = *bradial ; --bradial ; --tradial ; }
/*
**  Insert Stroke Radials
*/
          ++radial ;
          ++tradial ;
          hor = hor + horinc ;
          angle = angle + angleinc ;
          slope = slope + slopeinc ;
          while ( radial < tradial )
            {
             *radial = *bradial ;
             radial->radialType = 2 ;
             radial->radialStatus = 2 ;
             radial->radialAngle  = bcdtmMath_normaliseAngle(angle) ;
             radial->radialSlope  = slope ;
             radial->toHorizOffset = hor ;
              if( CornerOption == 2 )
                {
                 X1 = radial->radialStartPoint.x + 200.0 * cos(radial->radialAngle) ;
                 Y1 = radial->radialStartPoint.y + 200.0 * sin(radial->radialAngle) ;
                 bcdtmMath_interpolatePointOnPlane(X1,Y1,&Z1,A,B,C,D) ;
                 radial->radialSlope = (Z1 - radial->radialStartPoint.z  ) / 200.0 ;
                }
             hor = hor + horinc ;
             angle = angle + angleinc ;
             slope = slope + slopeinc ;
             ++radial ;
            }
          *SideSlopeTableSize += numStrokeRadials ;
         }
      }
   }
/*
** Reallocate Side Slope Table Memory
*/
 *SideSlopeTable = ( DTM_SIDE_SLOPE_TABLE *) realloc( *SideSlopeTable,*SideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE)) ;
/*
** Write Side Slope Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Element Radials = %6ld",*SideSlopeTableSize) ;
    for( radial = *SideSlopeTable ; radial < *SideSlopeTable + *SideSlopeTableSize  ; ++radial )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld S = %2ld Slope = %10.4lf Angle = %8.4lf ** %10.4lf %10.4lf %10.4lf",(long)(radial-*SideSlopeTable),radial->radialType,radial->radialStatus,radial->radialSlope,radial->radialAngle,radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialStartPoint.z) ;
      }
   }
/*
**
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Stroking Convex Corners Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Stroking Convex Corners Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writeRadialsToBinaryDTMFile
(
 DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,
 long                 sideSlopeTableSize,
 long                 writeSideSlopeElement,
 wchar_t              *dataFileP
)
/*
** This Function Writes The radials To A Binary DTM Data File
*/
{
 int  ret=DTM_SUCCESS ;
 long numElemPts=0 ;
 DPoint3d  radialPts[2],*elemPtsP=nullptr ;
 DTM_SIDE_SLOPE_TABLE  *radialP ;
 BC_DTM_OBJ *dataP=nullptr ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
/*
** Store Radials In DTM Object
*/
 for( radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++radialP )
   {
    radialPts[0].x = radialP->radialStartPoint.x ;
    radialPts[0].y = radialP->radialStartPoint.y ;
    radialPts[0].z = radialP->radialStartPoint.z ;
    radialPts[1].x = radialP->radialEndPoint.x ;
    radialPts[1].y = radialP->radialEndPoint.y ;
    radialPts[1].z = radialP->radialEndPoint.z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,radialPts,2)) goto errexit ;
   }
/*
** Store Element In DTM Object
*/
 if( writeSideSlopeElement )
   {
    if( bcdtmSideSlope_copySideSlopeElementPointsToPointArray(sideSlopeTableP,sideSlopeTableSize,&elemPtsP,&numElemPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,elemPtsP,numElemPts)) goto errexit ;
   }
/*
** Write Data Object To File
*/
 if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,dataFileP)) goto errexit ;
/*
** Delete Data Object
*/
 cleanup :
 if( elemPtsP != nullptr ) { free(elemPtsP) ; elemPtsP = nullptr ; }
 if( dataP    != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
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
int bcdtmSideSlope_writeElementToBinaryDTMFile
(
 DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,
 long                 sideSlopeTableSize,
 wchar_t              *dataFileP
)
/*
** This Function Writes The Side Slope Element To A Binary DTM Data File
*/
{
 int ret=DTM_SUCCESS ;
 long numElemPts=0 ;
 DPoint3d  *elemPtsP=nullptr ;
 BC_DTM_OBJ *dataP=nullptr ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
 /*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
/*
** Store Element In DTM Object
*/
 if( bcdtmSideSlope_copySideSlopeElementPointsToPointArray(sideSlopeTableP,sideSlopeTableSize,&elemPtsP,&numElemPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,elemPtsP,numElemPts)) goto errexit ;
/*
** Write DTM Object To File
*/
 if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,dataFileP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( elemPtsP != nullptr ) { free(elemPtsP) ; elemPtsP = nullptr ; }
 if( dataP    != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
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
|  bcdtmSideSlope_resolveOverlappingSideSlopeRadials                 |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_resolveOverlappingSideSlopeRadials(DTM_SIDE_SLOPE_TABLE *RightSideSlopeTable,long RightSideSlopeTableSize,DTM_SIDE_SLOPE_TABLE *LeftSideSlopeTable,long LeftSideSlopeTableSize,DPoint3d *ParallelEdgePts,long NumParallelEdgePts,long SideDirection,double CornerTolerance,double RadialExtension,DTMUserTag RadialTag,DTMUserTag ElementTag,double Pptol,double Pltol,BC_DTM_OBJ *SideSlopes)
/*
** This Is The Controlling Function For Resolving OverLapping Side Slope Radials
** This Function Has been Completely Rewritten From the older Resolve Side Slope Function
** That Was Initially Developed In March To November 98. This Function Incorporates All the
** Initial Development As Well As the Enhancemnets And Techniques That Have Been Developed Since 98.
** This Function Also Incorporates Fast Spatial Indexing Techniques For Line Intersections
**
** Author : Rob Cormack
** Date   : October 2002
**
*/
{
 int     ret=0,dbg=0 ;
 bool    useNewAlgorithm=true ;
 long    sideSlopeElementType,NumRghtRadials=0,NumLeftRadials=0,RadialIntersectFlag,numRemoved ;
 double  tolerance=0.05 ;
 DTM_OVERLAP_RADIAL_TABLE  *RghtRadials=nullptr,*LeftRadials=nullptr ;
 long start=0,finish=0,resStart=0 ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Resolving Overlapping Side Slope Radials") ;
    resStart = bcdtmClock() ;
   }
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"RightSideSlopeTable     = %p",RightSideSlopeTable) ;
    bcdtmWrite_message(0,0,0,"RightSideSlopeTableSize = %8ld",RightSideSlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"LeftSideSlopeTable      = %p",LeftSideSlopeTable) ;
    bcdtmWrite_message(0,0,0,"LeftSideSlopeTableSize  = %8ld",LeftSideSlopeTableSize) ;
    bcdtmWrite_message(0,0,0,"ParallelEdgePts         = %p",ParallelEdgePts) ;
    bcdtmWrite_message(0,0,0,"NumParallelEdgePts      = %8ld",NumParallelEdgePts) ;
    bcdtmWrite_message(0,0,0,"SideDirection           = %8ld",SideDirection) ;
    bcdtmWrite_message(0,0,0,"CornerTolerance         = %8.5lf",CornerTolerance) ;
    bcdtmWrite_message(0,0,0,"RadialExtension         = %8.5lf",RadialExtension) ;
    bcdtmWrite_message(0,0,0,"RadialTag               = %10I64d",RadialTag) ;
    bcdtmWrite_message(0,0,0,"ElementTag              = %10I64d",ElementTag) ;
    bcdtmWrite_message(0,0,0,"Pptol                   = %10.5lf",Pptol) ;
    bcdtmWrite_message(0,0,0,"Pltol                   = %10.5lf",Pltol) ;
    bcdtmWrite_message(0,0,0,"SideSlopes              = %p",SideSlopes) ;
   }
/*
** Write Out Radials For Development Purposes
*/
 if( dbg == 1 )
   {
    if( SideDirection == 1 || SideDirection == 3) bcdtmSideSlope_writeRadialsToBinaryDTMFile(RightSideSlopeTable,RightSideSlopeTableSize,0,L"RightSideSlopeRadials.dat") ;
    if( SideDirection == 2 || SideDirection == 3) bcdtmSideSlope_writeRadialsToBinaryDTMFile(LeftSideSlopeTable,LeftSideSlopeTableSize,0,L"LeftSideSlopeRadials.dat") ;
   }
/*
** Determine Side Slope Element Type, 1 For Open Element 2 For Closed Element
*/
 sideSlopeElementType = 1 ;
 if( SideDirection == 1 || SideDirection == 3 )
   {
    if( RightSideSlopeTable->radialStartPoint.x == (RightSideSlopeTable+RightSideSlopeTableSize-1)->radialStartPoint.x  &&
        RightSideSlopeTable->radialStartPoint.y == (RightSideSlopeTable+RightSideSlopeTableSize-1)->radialStartPoint.y     ) sideSlopeElementType = 2 ;
   }
 else
   {
    if( LeftSideSlopeTable->radialStartPoint.x == (LeftSideSlopeTable+LeftSideSlopeTableSize-1)->radialStartPoint.x  &&
        LeftSideSlopeTable->radialStartPoint.y == (LeftSideSlopeTable+LeftSideSlopeTableSize-1)->radialStartPoint.y     ) sideSlopeElementType = 2 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Side Slope Element Type = %2ld",sideSlopeElementType) ;
/*
** Resolve Right Radials
*/
 if( SideDirection == 1 || SideDirection == 3 )
   {
/*
** Extend Radials At Transistion Points If Necessary
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Extending Radials At Transistion Points") ;
    bcdtmSideSlope_extendRadialsAtTransistionPoints(sideSlopeElementType,RightSideSlopeTable,RightSideSlopeTableSize,RadialExtension) ;
/*
**  Create Right Radial Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Right Radial Overlap Table") ;
    if( bcdtmSideSlope_createRadialOverlapTable(sideSlopeElementType,RightSideSlopeTable,RightSideSlopeTableSize,&RghtRadials,&NumRghtRadials)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials00.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Creation Of Overlap Table") ;
/*
** Truncate Radials At Parallel Boundary Edge
*/
    if( ParallelEdgePts != nullptr && NumParallelEdgePts > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Right Radials At Parallel Boundary Edge") ;
       if( dbg ) start = bcdtmClock() ;
       if( bcdtmSideSlope_truncateSideSlopeRadialsAtParallelBoundaryEdge(ParallelEdgePts,NumParallelEdgePts,RghtRadials,&NumRghtRadials) )
       if( dbg ) finish = bcdtmClock() ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Time To Truncate Right Radials At Parallel Boundary Edge = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials01.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Truncation At Parrallel Edge") ;
      }
/*
** Truncate Radials At Pad Edges
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Right Radials At Pad Edge") ;
    if( bcdtmSideSlope_truncateSideSlopeRadialsAtPadEdge(RghtRadials,NumRghtRadials,sideSlopeElementType)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials02.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Truncation At Pad Edge") ;

/*
**
** Remove Adjacent Convex Corner Radials That Are Within Grainular Tolerance
** RobC - Added Following Function 15/11/2007 To Counter Densification Of Verices While Benching
**
*/
   if( bcdtmSideSlope_removeAdjacentRadialsToConvexRadials(sideSlopeElementType,RghtRadials,&NumRghtRadials,tolerance,&numRemoved)) goto errexit ;
   if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials03.dat") ;
/*
** Remove Adjacent Convex Corner Radials That Are Within Grainular Tolerance
**
**  Commented Out Rob Cormack 30/10/2007
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Adjacent Convex Corner Radials") ;
    if( bcdtmSideSlope_removeAdjacentSideSlopeRadialsWithinTolerance(RghtRadials,&NumRghtRadials,CornerTolerance)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,"RightResolveRadials03.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Removal Of Adjacent Convex Corner Radials") ;
*/
/*
** Truncate Element Radials With End Radials
*/
    if( sideSlopeElementType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Left Radials By Element End Radials") ;
       if( bcdtmSideSlope_truncateElementRadialsWithElementEndRadials(1,RghtRadials,NumRghtRadials) ) goto errexit ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials04.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Truncation At Pad Edge") ;
      }
/*
** Intersect  Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials") ;
    if( dbg ) start = bcdtmClock() ;
    if( bcdtmSideSlope_intersectSideSlopeRadials(RghtRadials,NumRghtRadials,sideSlopeElementType,&RadialIntersectFlag) ) goto errexit ;
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Intersect Radials = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials05.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Radial Intersection") ;
/*
**  Truncate Intersected Radials
*/
    if( RadialIntersectFlag )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Intersected Radials") ;
       if( bcdtmSideSlope_truncateIntersectedSideSlopeRadials(1,RghtRadials,NumRghtRadials)) goto errexit ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials06.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Radial Truncation") ;
      }
/*
** Intersect Radials With Base Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials With Base Lines") ;
    if( dbg ) start = bcdtmClock() ;
    if( bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines(1,RghtRadials,NumRghtRadials)) goto errexit ;
    if( bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines(1,RghtRadials,NumRghtRadials)) goto errexit ;
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Intersect Radials With Base Lines = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials07.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Base Lines") ;
/*
**  Set Elevations Of Intersected Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Elevations Of Intersected Radials") ;
    bcdtmSideSlope_setElevationOfIntersectedSideSlopeRadials(RghtRadials,NumRghtRadials) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials08.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(RghtRadials,NumRghtRadials,"Right After Set Elevations") ;
/*
**  Mark Truncated Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Truncated Radials") ;
    if( bcdtmSideSlope_markTruncatedRadials(RghtRadials,NumRghtRadials)) goto errexit ;
/*
**  Truncate Non Truncated Radials The Intersect Truncated Slope Toes
**  RobC Following Code added 9/11/2007
*/
    if( bcdtmSideSlope_truncateRadialsInsideTruncatedSlopeToe(RghtRadials,NumRghtRadials,sideSlopeElementType)) goto errexit ;
/*
**  Terminate Radials With Toe Points Inside Slope Toes
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Terminating Radials With Toe Points Inside Slope Toes") ;
    if( processingLimits ) if( bcdtmSideSlope_truncateRadialsWithToePointsInsideSlopeToe(RghtRadials,NumRghtRadials,1)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"RightResolveRadials09.dat") ;
/*
**  Truncate Truncating Radials
*/
    if( bcdtmSideSlope_truncateTruncatingRadials(RghtRadials,NumRghtRadials)) goto errexit ;
   }
/*
** Resolve Left Radials
*/
 if( SideDirection == 2 || SideDirection == 3 )
   {
/*
** Extend Radials At Transistion Points If Necessary
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Extending Radials At Transistion Points") ;
    bcdtmSideSlope_extendRadialsAtTransistionPoints(sideSlopeElementType,LeftSideSlopeTable,LeftSideSlopeTableSize,RadialExtension) ;
/*
**  Create Left Radial Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Left Radial Overlap Table") ;
    if( bcdtmSideSlope_createRadialOverlapTable(sideSlopeElementType,LeftSideSlopeTable,LeftSideSlopeTableSize,&LeftRadials,&NumLeftRadials)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials00.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Creation Of Overlap Table") ;
/*
** Truncate Radials At Parallel Boundary Edge
*/
    if( ParallelEdgePts != nullptr && NumParallelEdgePts > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Left Radials At Parallel Boundary Edge") ;
       if( dbg ) start = bcdtmClock() ;
       if( bcdtmSideSlope_truncateSideSlopeRadialsAtParallelBoundaryEdge(ParallelEdgePts,NumParallelEdgePts,LeftRadials,&NumLeftRadials) )
       if( dbg ) finish = bcdtmClock() ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Time To Truncate Left Radials At Parallel Boundary Edge = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials01.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Truncation At Parrallel Edge") ;
      }
/*
** Truncate Radials At Pad Edges
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Left Radials At Pad Edge") ;
    if( dbg ) start = bcdtmClock() ;
    if( bcdtmSideSlope_truncateSideSlopeRadialsAtPadEdge(LeftRadials,NumLeftRadials,sideSlopeElementType)) goto errexit ;
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Truncate Left Radials At Pad Edges = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials02.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Truncation At Pad Edge") ;
/*
**
** Remove Adjacent Convex Corner Radials That Are Within Grainular Tolerance
** RobC - Added Following Function 15/11/2007 To Counter Densification Of Verices While Benching
**
*/
   if( bcdtmSideSlope_removeAdjacentRadialsToConvexRadials(sideSlopeElementType,LeftRadials,&NumLeftRadials,tolerance,&numRemoved)) goto errexit ;
   if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials03.dat") ;
/*
** Remove Adjacent Convex Corner Radials That Are Within Grainular Tolerance
*/
/*
** Commented Out Rob 30/10/2007
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Adjacent Convex Corner Left Radials") ;
    if( bcdtmSideSlope_removeAdjacentSideSlopeRadialsWithinTolerance(LeftRadials,&NumLeftRadials,CornerTolerance)) goto errexit ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,"LeftResolveRadials03.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Right After Removal Of Adjacent Convex Corner Radials") ;
*/

/*
** Truncate Radials With Element End Radials
*/
    if( sideSlopeElementType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Left Radials By Element End Radials") ;
       if( bcdtmSideSlope_truncateElementRadialsWithElementEndRadials(2,LeftRadials,NumLeftRadials) ) goto errexit ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials04.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Truncation With Edge Radials") ;
      }
/*
** Intersect  Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials") ;
    if( dbg ) start = bcdtmClock() ;
    if( bcdtmSideSlope_intersectSideSlopeRadials(LeftRadials,NumLeftRadials,sideSlopeElementType,&RadialIntersectFlag) ) goto errexit ;
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Intersect Radials = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials05.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Radial Intersection") ;
/*
**  Truncate Intersected Radials
*/
    if( RadialIntersectFlag )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Intersected Radials") ;
       if( bcdtmSideSlope_truncateIntersectedSideSlopeRadials(2,LeftRadials,NumLeftRadials)) goto errexit ;
       if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials06.dat") ;
       if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Radial Truncation") ;
      }
/*
** Intersect Radials With Base Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials With Base Lines") ;
    start = bcdtmClock() ;
    if( bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines(sideSlopeElementType,LeftRadials,NumLeftRadials)) goto errexit ;
    if( bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines(sideSlopeElementType,LeftRadials,NumLeftRadials)) goto errexit ;
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Intersect Radials With Base Lines = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials07.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Base Lines") ;
/*
**  Set Elevations Of Intersected Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Elevations Of Intersected Radials") ;
    bcdtmSideSlope_setElevationOfIntersectedSideSlopeRadials(LeftRadials,NumLeftRadials) ;
    if( dbg ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials08.dat") ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(LeftRadials,NumLeftRadials,"Left After Setting Elevations") ;
/*
**  Mark Truncated Radials
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Truncated Radials") ;
    if( bcdtmSideSlope_markTruncatedRadials(LeftRadials,NumLeftRadials)) goto errexit ;
/*
**  Truncate Non Truncated Radials The Intersect Truncated Slope Toes
**  Robc - Following Code Added 9/11/2007
*/
    if( bcdtmSideSlope_truncateRadialsInsideTruncatedSlopeToe(LeftRadials,NumLeftRadials,sideSlopeElementType)) goto errexit ;
/*
**  Terminate Radials With Toe Points Inside Slope Toes
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Terminating Radials With Toe Points Inside Slope Toes") ;
    if( processingLimits ) if( bcdtmSideSlope_truncateRadialsWithToePointsInsideSlopeToe(LeftRadials,NumLeftRadials,2)) goto errexit ;
    if( dbg == 2 ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"LeftResolveRadials09.dat") ;
/*
**  Truncate Truncating Radials
*/
    if( bcdtmSideSlope_truncateTruncatingRadials(LeftRadials,NumLeftRadials)) goto errexit ;
   }
/*
** Remove Type2 Radials Coincident With Type1
*/
// if( RghtRadials != nullptr ) bcdtmSideSlope_removeType2RadialsCoincicidentWithType1(&RghtRadials,&NumRghtRadials) ;
// if( LeftRadials != nullptr ) bcdtmSideSlope_removeType2RadialsCoincicidentWithType1(&LeftRadials,&NumLeftRadials) ;
/*
** Write Side Slope Radials To Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Side Slope Radials To Data Object") ;
 if( bcdtmSideSlope_writeSideSlopeRadialsToDataObject(sideSlopeElementType,SideDirection,RghtRadials,NumRghtRadials,LeftRadials,NumLeftRadials,RadialTag,ElementTag,SideSlopes)) goto errexit ;
/*
** Write Boundary Polygon And Slope Toes Data Object
**
** RobC 25/10/2007 ** Following Commented Functions Replaced With New Code Below
*/
 if( useNewAlgorithm == false )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Boundary Polygon To Data Object") ;
    if( dbg ) start = bcdtmClock() ;
    if( sideSlopeElementType == 1 ) { if( bcdtmSideSlope_writeOpenSideSlopeElementBoundaryPolygonToDataObject(RghtRadials,NumRghtRadials,LeftRadials,NumLeftRadials,SideDirection,SideSlopes,Pptol,Pltol)) goto errexit ; }
    if( sideSlopeElementType == 2 ) { if( bcdtmSideSlope_writeClosedSideSlopeElementBoundaryPolygonToDataObject(RghtRadials,NumRghtRadials,LeftRadials,NumLeftRadials,SideDirection,SideSlopes,Pptol,Pltol)) goto errexit ; }
    if( dbg ) finish = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Determine Boundary Polygon = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start) ) ;
/*
** Write Holes To Data Object
*/
    if( sideSlopeElementType == 2 && ( SideDirection == 2 || SideDirection == 3 ) )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Writing Left Holes To Data Object") ;
       if( dbg ) start = bcdtmClock() ;
       if( bcdtmSideSlope_writePadSideSlopeHolesToDataObject(LeftRadials,NumLeftRadials,SideSlopes,Pptol,Pltol)) goto errexit ;
       if( dbg ) finish = bcdtmClock() ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Time To Determine Left Holes = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
      }
   }
/*
** Write Boundary Polygon And SlopeToes To Data Object
**
** RobC 25/10/2007 - New Functions Added
*/
  if( useNewAlgorithm == true )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Boundary Polygon To Data Object") ;
    if( sideSlopeElementType == 1 )
        {
        if( bcdtmSideSlope_getBoundaryPolygonAndSlopeToesForOpenSideSlopeElementDataObject
            (SideSlopes,sideSlopeElementType,SideDirection,RightSideSlopeTable, RghtRadials, NumRghtRadials, LeftSideSlopeTable, LeftRadials,NumLeftRadials))
            goto errexit ;
        }
    if( sideSlopeElementType == 2 )
        {
        if( bcdtmSideSlope_getBoundaryPolygonAndSlopeToesForClosedSideSlopeElementDataObject(SideSlopes,sideSlopeElementType,SideDirection, RightSideSlopeTable, RghtRadials, NumRghtRadials, LeftSideSlopeTable, LeftRadials,NumLeftRadials))
            goto errexit ;
        }
   }
/*
** Free Memory
*/
 cleanup :
 if( RghtRadials != nullptr ) free(RghtRadials) ;
 if( LeftRadials != nullptr ) free(LeftRadials) ;
/*
** Write Debug Information
*/
 if( dbg && ! ret )
   {
    finish = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Time To Resolve Overlaps = %7.3lf seconds",bcdtmClock_elapsedTime(finish,resStart) ) ;
    bcdtmWrite_geopakDatFileFromDtmObject(SideSlopes,L"resolvedSideSlopes.dat") ;
   }
/*
** Job Completed
*/
 if( dbg && ! ret )  bcdtmWrite_message(0,0,0,"Resolving Overlapping Side Slope Radials Completed") ;
 if( dbg &&   ret )  bcdtmWrite_message(0,0,0,"Resolving Overlapping Side Slope Radials Error") ;
// if( seqdbg == 0 ) return(DTM_ERROR) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeType2RadialsCoincicidentWithType1
(
 DTM_OVERLAP_RADIAL_TABLE **sideSlopeRadialsPP,    /* ==> Pointer To Side Slope Radials            */
 long                     *numSideSlopeRadialsP    /* ==> Number Of Side Slope Radials             */
)
{
 int dbg=0 ;
 long numRemoved=0 ;
// double intX,intY,intZ ;
 DTM_OVERLAP_RADIAL_TABLE *radP,*pradP,*nradP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Coincident Type 2 Radials") ;
    bcdtmWrite_message(0,0,0,"*sideSlopeRadialsPP   = %p",*sideSlopeRadialsPP) ;
    bcdtmWrite_message(0,0,0,"*numSideSlopeRadialsP = %8ld",*numSideSlopeRadialsP) ;
   }
/*
** Scan And Mark Type 2 Radials To Remove
*/
 for( radP = *sideSlopeRadialsPP ; radP < *sideSlopeRadialsPP + *numSideSlopeRadialsP ; ++radP )
   {
    if( radP->Type == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"radial[%5ld] = Type 1",(long)(radP-*sideSlopeRadialsPP)) ;
       pradP = radP - 1 ;
       if( pradP < *sideSlopeRadialsPP ) pradP = *sideSlopeRadialsPP + *numSideSlopeRadialsP - 1 ;
       pradP->Type = 9 ;
       nradP = radP + 1 ;
       if( nradP >= *sideSlopeRadialsPP + *numSideSlopeRadialsP ) nradP = *sideSlopeRadialsPP ;
       nradP->Type = 9 ;
       numRemoved = numRemoved + 2 ;
/*
**     Adjust Slope Toe Of Type 1 Radial
*/
//       if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,pradP->Gx,pradP->Gy,nradP->Gx,nradP->Gy))
//         {
//          bcdtmMath_intersectCordLines(radP->Px,radP->Py,radP->Gx,radP->Gy,pradP->Gx,pradP->Gy,nradP->Gx,nradP->Gy,&intX,&intY)  ;
//          bcdtmMath_interpolatePointOnLine(radP->Px,radP->Py,radP->Pz,radP->Gx,radP->Gy,radP->Gz,intX,intY,&intZ) ;
//          radP->Gx = intX ;
//          radP->Gy = intY ;
//          radP->Gz = intZ ;
//         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numRemoved = %6ld",numRemoved) ;
/*
** Remove Radials
*/
 if( numRemoved )
   {
    for( radP = pradP = *sideSlopeRadialsPP ; pradP < *sideSlopeRadialsPP + *numSideSlopeRadialsP  ; ++pradP )
      {
       if( pradP->Type != 9 )
         {
          *radP = *pradP ;
          ++radP ;
         }
      }
/*
**  Set Number Of Radials
*/
    *numSideSlopeRadialsP = (long )( radP - *sideSlopeRadialsPP ) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"*numSideSlopeRadialsP = %6ld",*numSideSlopeRadialsP) ;
/*
**  Reallocate Memory
*/
    *sideSlopeRadialsPP = ( DTM_OVERLAP_RADIAL_TABLE * ) realloc ( *sideSlopeRadialsPP , *numSideSlopeRadialsP * sizeof(DTM_OVERLAP_RADIAL_TABLE)) ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_extendRadialsAtTransitionPoints                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_extendRadialsAtTransistionPoints(long SideSlopeElementType,DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,double RadialExtension)
/*
** This Function Creates The Overlap Table
*/
{
 long   dbg=0  ;
 DTM_SIDE_SLOPE_TABLE *radial ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extending Radials At Transition Points ** Radial Extension = %12.8lf",RadialExtension) ;
 if( RadialExtension < 0.05 ) RadialExtension = 0.05 ; // Added Robc 5/11/2007
/*
** Scan Side Slope Table For Transition Points And Extend Radials If Necessary
*/
 if( SideSlopeElementType == 1 )
   {
    for( radial =  SideSlopeTable ; radial <  SideSlopeTable +  SideSlopeTableSize ; ++radial )
      {
       if( radial->radialGenesis != 1 )
         {
          if( bcdtmMath_distance(radial->radialStartPoint.x,radial->radialStartPoint.y,radial->radialEndPoint.x,radial->radialEndPoint.y ) < RadialExtension )
            {
             radial->radialEndPoint.x = radial->radialEndPoint.x + RadialExtension * cos(radial->radialAngle) ;
             radial->radialEndPoint.y = radial->radialEndPoint.y + RadialExtension * sin(radial->radialAngle) ;
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_createRadialOverlapTable                             |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_createRadialOverlapTable(long sideSlopeElementType,DTM_SIDE_SLOPE_TABLE *SideSlopeTable,long SideSlopeTableSize,DTM_OVERLAP_RADIAL_TABLE **Radials,long *NumRadials)
/*
** This Function Creates The Overlap Table
*/
{
 long   dbg=0,PadType=0  ;
 DTM_SIDE_SLOPE_TABLE *rad ;
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Radial Overlap Table") ;
 if( dbg )
   {
    for( rad = SideSlopeTable ; rad < SideSlopeTable + SideSlopeTableSize  ; ++rad )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %2ld S = %2ld G = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(rad-SideSlopeTable),rad->radialStatus,rad->radialType,rad->radialGenesis,rad->radialStartPoint.x,rad->radialStartPoint.y,rad->radialStartPoint.z) ;
      }
   }
/*
** Initialise For Pad Type
*/
 if( sideSlopeElementType == 2 ) PadType = 1 ;
/*
** Count Number Of Overlap Table Entries
*/
 *NumRadials = 0 ;
 for( rad = SideSlopeTable ; rad < SideSlopeTable + SideSlopeTableSize - PadType ; ++rad )
     ++*NumRadials ;
/*
** Allocate memory
*/
 *Radials = (DTM_OVERLAP_RADIAL_TABLE *) malloc( *NumRadials * sizeof(DTM_OVERLAP_RADIAL_TABLE)) ;
 if( *Radials == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Store Points In Ovelap Table
*/
 ovl = *Radials ;
 for( rad = SideSlopeTable ; rad < SideSlopeTable + SideSlopeTableSize - PadType ; ++rad )
   {
    ovl->Ofs    = (long)(rad-SideSlopeTable) ;
    ovl->Type   = rad->radialType ;
    ovl->Status = 1 ;
    ovl->TruncatingRadial = DTM_NULL_PNT ;
    ovl->TruncatingEdge   = DTM_NULL_PNT ;
    ovl->Px     = rad->radialStartPoint.x  ;
    ovl->Py     = rad->radialStartPoint.y  ;
    ovl->Pz     = rad->radialStartPoint.z  ;
    ovl->Gx     = rad->radialEndPoint.x ;
    ovl->Gy     = rad->radialEndPoint.y ;
    ovl->Gz     = rad->radialEndPoint.z ;
    ovl->Nx     = rad->radialEndPoint.x ;
    ovl->Ny     = rad->radialEndPoint.y ;
    ovl->Nz     = rad->radialEndPoint.z ;
    ovl->EdgeZ  = 0.0     ;
    ++ovl ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeAdjacentRadialsToConvexRadials(long sideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *radialsP,long *numRadialsP,double tolerance,long *numRemovedP)
/*
** This Function Removes Adjacent Radials At Convex Corners On The Side Slope Element
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 double d1,d2;
 DTM_OVERLAP_RADIAL_TABLE *radP,*radpP,*radnP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Adjacent Radials To Convex Radials") ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Mark radialsP Coincident With tolerance To Type 3 radialsP
*/
 for( radP =  radialsP ; radP < radialsP + *numRadialsP ; ++radP )
   {
    if( radP->Type == 3 )
      {
/*
**     Get Prior Radial
*/
       radpP = radP - 1 ;
       if( radpP <  radialsP )
         {
          radpP = nullptr ;
          if( sideSlopeElementType == 2 ) radpP = radialsP + *numRadialsP - 1 ;
         }
/*
**     Get Next Radial
*/
       radnP = radP + 1 ;
       if( radnP >=  radialsP + *numRadialsP  )
         {
          radnP = nullptr ;
          if( sideSlopeElementType == 2 ) radnP = radialsP  ;
         }
/*
**     Calculate Length Of Radial
*/
       d1 = bcdtmMath_distance(radP->Px,radP->Py,radP->Gx,radP->Gy) ;
/*
**     Check If Prior Radial Can Be Removed
*/
       if( radpP != nullptr )
         {
          d2 = bcdtmMath_distance(radP->Gx,radP->Gy,radpP->Gx,radpP->Gy) ;
          if( d2 / d1 <= tolerance )
            {
             radpP->Status = 2 ;
             ++*numRemovedP ;
            }
         }
/*
**     Check If Next Radial Can Be Removed
*/
       if( radnP != nullptr )
         {
          d2 = bcdtmMath_distance(radP->Gx,radP->Gy,radnP->Gx,radnP->Gy) ;
          if( d2 / d1 <= tolerance )
            {
             radnP->Status = 2 ;
             ++*numRemovedP ;
            }
         }
      }
   }
/*
** Remove Marked radialsP
*/
 if( *numRemovedP )
   {
    for( radpP = radnP = radialsP ; radnP < radialsP + *numRadialsP ; ++radnP )
      {
       if( radnP->Status != 2 )
         {
          if( radpP != radnP ) *radpP = *radnP ;
          ++radpP ;
         }
      }
    *numRadialsP = (long)(radpP - radialsP) ;
   }
/*
** Job Completed
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_removeAdjacentSideSlopeRadialsWithinTolerance      |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeAdjacentSideSlopeRadialsWithinTolerance(DTM_OVERLAP_RADIAL_TABLE *Radials,long *NumRadials,double Tolerance)
/*
** This Function Removes Adjacent Radials At Convex Corners On The Side Slope Element
*/
{
 long   dbg=0 ;
 double Sx,Sy,ratio,type3Ratio=0.02 ;
 DTM_OVERLAP_RADIAL_TABLE *ovl,*ovlp,*ovln,*ovlpp,*ovlnn ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Adjacent Side Slope Radials Within Tolerance") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Num Radials = %6ld Tolerance = %10.4lf",*NumRadials,Tolerance) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Overlap Radials Before Removal = %6ld",*NumRadials) ;
    for( ovl = Radials  ; ovl < Radials + *NumRadials ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"O = [%4ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl-Radials),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovl->Gx,ovl->Gy) ;
      }
   }
/*
** Mark Radials Coincident With Tolerance To Type 3 Radials
*/
 for( ovl =  Radials ; ovl < Radials + *NumRadials ; ++ovl )
   {
    if( ovl->Type == 3 )
      {
/*
** Scan Backwards From Type Three Radial To Get First Convex Corner Radial
*/
       ovlp = ovl - 1 ; if( ovlp <  Radials ) ovlp = Radials + *NumRadials - 1 ;
       while( ovlp->Px == ovl->Px && ovlp->Py == ovl->Py )
         {
          --ovlp ;
          if( ovlp <  Radials ) ovlp = Radials + *NumRadials - 1 ;
         }
       ++ovlp ; if( ovlp >= Radials + *NumRadials ) ovlp = Radials  ;
       ovlpp = ovlp ;
/*
** Scan Forwards From Type Three Radial To Get Last Convex Corner Radial
*/
       ovln = ovl + 1 ; if( ovln >= Radials + *NumRadials ) ovln = Radials ;
       while( ovln->Px == ovl->Px && ovln->Py == ovl->Py )
         {
          ++ovln ;
          if( ovln >= Radials + *NumRadials ) ovln = Radials  ;
         }
       --ovln ; if( ovln <  Radials ) ovln = Radials + *NumRadials - 1 ;
       ovlnn = ovln ;
/*
** Write Radial Extent
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Ovlpp Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovlp-Radials),ovlpp->Type,ovlpp->Status,ovlpp->Px,ovlpp->Py,ovlpp->Pz,ovlpp->Nx,ovlpp->Ny,ovlpp->Nz) ;
          bcdtmWrite_message(0,0,0,"Ovl   Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovl-Radials),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Pz,ovl->Nx,ovl->Ny,ovl->Nz) ;
          bcdtmWrite_message(0,0,0,"Ovlnn Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovln-Radials),ovlnn->Type,ovlnn->Status,ovlnn->Px,ovlnn->Py,ovlnn->Pz,ovlnn->Nx,ovlnn->Ny,ovlnn->Nz) ;
         }
/*
** Scan Backwards From Type Three Radial To First Convex Corner Radial And
** Mark Radials With Tolerance
*/
       if( ovlpp != ovl )
         {
          Sx = ovl->Nx ;
          Sy = ovl->Ny ;
          ovlp = ovl - 1  ;
          if( ovlp < Radials ) ovlp = Radials + *NumRadials - 1 ;
          while( ovlp != ovlpp )
            {
             if( bcdtmMath_distance(ovlp->Nx,ovlp->Ny,Sx,Sy) <= Tolerance )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Prior Radial %10ld",(long)(ovlp-Radials)) ;
                ovlp->Status = 0 ;
               }
             else
               {
                Sx = ovlp->Nx ;
                Sy = ovlp->Ny ;
               }
             --ovlp ;
             if( ovlp < Radials ) ovlp = Radials + *NumRadials - 1 ;
            }
          ratio = bcdtmMath_distance(ovlpp->Nx,ovlpp->Ny,Sx,Sy)/bcdtmMath_distance(ovl->Px,ovl->Py,Sx,Sy) ;
          if( ratio < type3Ratio )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Ratio Prior Radial %10ld",(long)(ovlpp-Radials)) ;
             ovlpp->Status = 0 ;
            }
         }
/*
**
** Scan Forwards From Type Three Radial To Last Convex Corner Radial And
** Mark Radials Within Tolernace
**
*/
       if( ovlnn != ovl )
         {
          Sx = ovl->Nx ;
          Sy = ovl->Ny ;
          ovln = ovl + 1 ;
          if( ovln >= Radials + *NumRadials ) ovln = Radials  ;
          while ( ovln != ovlnn )
            {
             if( bcdtmMath_distance(ovln->Nx,ovln->Ny,Sx,Sy) <= Tolerance )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Next Radial %10ld",(long)(ovln-Radials)) ;
                ovln->Status = 0 ;
               }
             else
               {
                Sx = ovln->Nx ;
                Sy = ovln->Ny ;
               }
             ++ovln ;
             if( ovln >= Radials + *NumRadials ) ovln = Radials  ;
            }
          ratio = bcdtmMath_distance(ovlnn->Nx,ovlnn->Ny,Sx,Sy)/bcdtmMath_distance(ovl->Px,ovl->Py,Sx,Sy) ;
          if( ratio < type3Ratio )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Ratio Next Radial %10ld",(long)(ovlnn-Radials)) ;
             ovlnn->Status = 0 ;
            }
         }

/*
       if( ovln != ovl )
         {
          Sx = ovl->Nx ;
          Sy = ovl->Ny ;
          ovln = ovl ;
          while ( ovln != ovlnn )
            {
             ++ovln ;
             if( ovln >= Radials + *NumRadials ) ovln = Radials  ;
             if( bcdtmMath_distance(ovln->Nx,ovln->Ny,Sx,Sy) <= Tolerance ) ovln->Status = 0 ;
             else                                                        { Sx = ovln->Nx ; Sy = ovln->Ny ; }
            }
         }
       else
         {
          ratio = bcdtmMath_distance(ovlnn->Nx,ovlnn->Ny,ovl->Nx,ovl->Ny)/bcdtmMath_distance(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny) ;
          if( ratio < type3Ratio ) ovlnn->Status = 0 ;
         }
*/
/*
** Write Radial Extent
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Ovlp Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovlp-Radials),ovlp->Type,ovlp->Status,ovlp->Px,ovlp->Py,ovlp->Pz,ovlp->Nx,ovlp->Ny,ovlp->Nz) ;
          bcdtmWrite_message(0,0,0,"Ovl  Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovl-Radials),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Pz,ovl->Nx,ovl->Ny,ovl->Nz) ;
          bcdtmWrite_message(0,0,0,"Ovln Radial[%6ld] T = %2ld S = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovln-Radials),ovln->Type,ovln->Status,ovln->Px,ovln->Py,ovln->Pz,ovln->Nx,ovln->Ny,ovln->Nz) ;
         }
      }
   }
/*
** Remove Marked Radials
*/
 for( ovlp = ovln = Radials ; ovln < Radials + *NumRadials ; ++ovln )
   {
    if( ovln->Status )
      {
       if( ovlp != ovln ) *ovlp = *ovln ;
       ++ovlp ;
      }
   }
/*
** Reset Number Of Radials
*/
 *NumRadials = (long)(ovlp - Radials) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Num Radials = %6ld",*NumRadials) ;
/*
** Write Radials - Development Only
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Overlap Radials After Removal = %6ld",*NumRadials) ;
    for( ovl = Radials  ; ovl < Radials + *NumRadials ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"O = [%4ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl-Radials),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovl->Gx,ovl->Gy) ;
      }
   }
/*
** Job Completed
*/
// bcdtmWrite_message(0,0,0,"minType3Ratio = %20.15lf maxType3Ratio = %20.15lf",minType3Ratio,maxType3Ratio) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Adjacent Side Slope Radials Within Tolerance Completed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile
(
 DTM_OVERLAP_RADIAL_TABLE *ovlTablePtsP,
 long                     ovlTableSize,
 wchar_t                  *dataFileP
)
/*
** This Function Writes The radials To A Binary DTM Data File
*/
{
 int  ret=DTM_SUCCESS ;
 DPoint3d  radialPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE *radialP ;
 BC_DTM_OBJ *dataP=nullptr ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
/*
** Store Radials In DTM Object
*/
 for( radialP = ovlTablePtsP ; radialP < ovlTablePtsP + ovlTableSize ; ++radialP )
   {
    radialPts[0].x = radialP->Px ;
    radialPts[0].y = radialP->Py ;
    radialPts[0].z = radialP->Pz ;
    radialPts[1].x = radialP->Nx ;
    radialPts[1].y = radialP->Ny ;
    radialPts[1].z = radialP->Nz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,radialPts,2)) goto errexit ;
   }
/*
** Write Data Object To File
*/
 if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,dataFileP)) goto errexit ;
/*
** Delete Data Object
*/
 cleanup :
 if( dataP    != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
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
/*----------------------------------------------------------------------+
|                                                                       |
| bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile                      |
|                                                                       |
+----------------------------------------------------------------------*/
int bcdtmSideSlope_writeOverlapRadialTableToDTMLogFile(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,char *Message)
/*
** This Function Write The Overlap Radial Table To The DTM Log File
*/
{
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
 bcdtmWrite_message(0,0,0,"%s",Message) ;
 bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",NumOvlPts) ;
 for( ovl =  OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
   {
    bcdtmWrite_message(0,0,0,"Radial[%6ld] ** T = %1ld S = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateSideSlopeRadialsAtParallelBoundaryEdge       |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateSideSlopeRadialsAtParallelBoundaryEdge(DPoint3d *PadPts,long NumPadPts,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long *NumOvlPts)
/*
**
** This Function Truncates The Radials At User Provided Boundary Edge
** This Is Mainly For Copy Parallel Applications
**
*/
{
 int     ret=0 ;
 long    dbg=0,IntTableNe,IntPtsNe,IntPtsMe,IntPtsMinc ;
 DPoint3d     *p3d ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl,*ovp ;
 DTM_STR_INT_TAB *pint,*IntTable=nullptr ;
 DTM_STR_INT_PTS *pinp,*IntPts=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radials At Parallel Boundary Edge") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",*NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + *NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld]  Type = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Boundary Edges = %6ld",NumPadPts) ;
        for( p3d = PadPts ; p3d < PadPts + NumPadPts ; ++p3d )
          {
       bcdtmWrite_message(0,0,0,"Parallel Edge[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d-PadPts),p3d->x,p3d->y,p3d->z) ;
      }
   }
/*
** Build Radial Parallel Edge Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Edge Intersection Table") ;
 if( bcdtmSideSlope_buildRadialParallelEdgeIntersectionTable(OvlPts,*NumOvlPts,PadPts,NumPadPts,&IntTable,&IntTableNe) )  goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Radial Parallel Edge Intersection Table Entries = %4ld",IntTableNe) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radial Parallel Edge Intersection Table Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** Pad = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
      }
   }
/*
** Scan Intersection Table And For Intersections
*/
 IntPtsMinc = IntTableNe / 10 ;
 if( IntPtsMinc < 1000 ) IntPtsMinc = 1000 ;
 IntPtsNe = IntPtsMe = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ;
 if( bcdtmSideSlope_scanForRadialParallelEdgeIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersections = %4ld",IntPtsNe) ;
/*
** Truncate Radials
*/
 if( IntPtsNe > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(IntPts,IntPtsNe,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
/*
** Write Intersection Points
*/
    if( dbg == 2  )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
       for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(pinp-IntPts),pinp->String1,pinp->Segment1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z,pinp->Z2) ;
                 }
      }
/*
**  Truncate Radials
*/
    for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
      {
       if( pinp == IntPts || ( pinp->Segment1 != (pinp-1)->Segment1 ))
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radial = %6ld",pinp->Segment1) ;
          ovl = OvlPts + pinp->Segment1 ;
          ovl->Nx = ovl->Gx = pinp->x ;
          ovl->Ny = ovl->Gy = pinp->y ;
          ovl->Nz = ovl->Gz = pinp->z ;
         }
      }
   }
/*
** Set Overlay Status
*/
 for( ovl = OvlPts ; ovl < OvlPts + *NumOvlPts ; ++ovl ) ovl->Status = 1 ;
/*
** Remove Radials That On Either Side Of Type 3  ** For Copy Parralel Applications Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Radials On Either Side Of Type 3") ;
 for( ovl = ovp = OvlPts ; ovp < OvlPts + *NumOvlPts ; ++ovp )
   {
    if( ovp->Type == 3 )
      {
       ovl = ovp - 1 ;
       if( ovl < OvlPts ) ovl = OvlPts + *NumOvlPts - 1 ;
       ovl->Status = 0 ;
       ovl = ovp + 1 ;
       if( ovl >= OvlPts + *NumOvlPts ) ovl = OvlPts ;
       ovl->Status = 0 ;
      }
   }
/*
** Remove Redundant Radials
*/
 for( ovl = ovp = OvlPts ; ovp < OvlPts + *NumOvlPts ; ++ovp )
   {
    if( ovp->Status )
      {
       if( ovp != ovl ) *ovl = *ovp ;
       ++ovl ;
      }
   }
/*
** Reset Number Of Radials
*/
 *NumOvlPts = (long)(ovl - OvlPts) ;
/*
** Write Truncated
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Truncated Radials = %6ld",*NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + *NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld]  Type = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( IntTable != nullptr ) free(IntTable) ;
 if( IntPts   != nullptr ) free(IntPts) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Truncating Radials At Parallel Boundary Edge Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Truncating Radials At Parallel Boundary Edge Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmSideSlope_buildRadialParallelEdgeIntersectionTable                 |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_buildRadialParallelEdgeIntersectionTable(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DPoint3d *ParallelPts,long NumParallelPts,DTM_STR_INT_TAB **IntTable,long *IntTableNe)
{
 int    ret=0 ;
 long   dbg=0,IntTableMe,IntTableMinc  ;
 double cord ;
 DPoint3d    *p3d ;
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
 DTM_STR_INT_TAB *pint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Parallel Intersection Table") ;
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 IntTableMinc = NumOvlPts * 2  ;
/*
** Store Parallel Edge Segments In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
   {
/*
**  Check For Memory Allocation
*/
    if( *IntTableNe == IntTableMe )
      {
       IntTableMe = IntTableMe + IntTableMinc ;
       if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store Pad Line
*/
    (*IntTable+*IntTableNe)->String  = 1 ;
    (*IntTable+*IntTableNe)->Segment = (long)(ovl-OvlPts) ;
    (*IntTable+*IntTableNe)->Type = 1   ;
    (*IntTable+*IntTableNe)->Direction = 1 ;
    (*IntTable+*IntTableNe)->X1 = ovl->Px ;
    (*IntTable+*IntTableNe)->Y1 = ovl->Py ;
    (*IntTable+*IntTableNe)->Z1 = ovl->Pz ;
    (*IntTable+*IntTableNe)->X2 = ovl->Gx ;
    (*IntTable+*IntTableNe)->Y2 = ovl->Gy ;
    (*IntTable+*IntTableNe)->Z2 = ovl->Gz ;
    ++*IntTableNe ;
   }
/*
** Store Parallel Edges In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Parallel Edge Segments In Intersection Table") ;
 for( p3d = ParallelPts ; p3d < ParallelPts + NumParallelPts - 1 ; ++p3d )
   {
/*
**  Check For Memory Allocation
*/
    if( *IntTableNe == IntTableMe )
      {
       IntTableMe = IntTableMe + IntTableMinc ;
       if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store Ovl Line
*/
    (*IntTable+*IntTableNe)->String  = 0  ;
    (*IntTable+*IntTableNe)->Segment = (long)(p3d-ParallelPts) ;
    (*IntTable+*IntTableNe)->Type    = 0  ;
    (*IntTable+*IntTableNe)->Direction = 1 ;
    (*IntTable+*IntTableNe)->X1 = p3d->x ;
    (*IntTable+*IntTableNe)->Y1 = p3d->y ;
    (*IntTable+*IntTableNe)->Z1 = p3d->z ;
    (*IntTable+*IntTableNe)->X2 = (p3d+1)->x ;
    (*IntTable+*IntTableNe)->Y2 = (p3d+1)->y ;
    (*IntTable+*IntTableNe)->Z2 = (p3d+1)->z ;
    ++*IntTableNe ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( pint = *IntTable ; pint < *IntTable + *IntTableNe ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *)) bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Radial Parallel Edge Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Radial Parallel Edge Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForRadialParallelEdgeIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Parallel Edge Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Scan Intersection Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineRadialParallelEdgeIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_determineRadialParallelEdgeIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x=0.0,y=0.0 ;
 DTM_STR_INT_TAB  *alp,*slp,*rlp,*plp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
**  Only Compare Radials Against Parralel Edges
*/
    if( ( slp->String == 0 && alp->String == 1 ) || ( slp->String == 1 && alp->String == 0 ))
      {
       if( slp->String == 1 )  { rlp = slp ; plp = alp ; }
       else                    { rlp = alp ; plp = slp ; }
/*
** Check Lines Intersect
*/
       if( bcdtmMath_checkIfLinesIntersect(plp->X1,plp->Y1,plp->X2,plp->Y2,rlp->X1,rlp->Y1,rlp->X2,rlp->Y2))
         {
/*
** Intersect Lines
*/
          bcdtmMath_normalIntersectCordLines(plp->X1,plp->Y1,plp->X2,plp->Y2,rlp->X1,rlp->Y1,rlp->X2,rlp->Y2,&x,&y) ;
/*
** Check Memory
*/
          if( *IntPtsNe >= *IntPtsMe )
            {
             *IntPtsMe = *IntPtsMe + IntPtsMinc ;
             if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             if( *IntPts == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
** Calculate Distances For Alp
*/
          if( rlp->Direction == 1 ) { Xs = rlp->X1 ; Ys = rlp->Y1 ; Zs = rlp->Z1 ; Xe = rlp->X2 ; Ye = rlp->Y2 ; Ze = rlp->Z2 ; }
          if( rlp->Direction == 2 ) { Xs = rlp->X2 ; Ys = rlp->Y2 ; Zs = rlp->Z2 ; Xe = rlp->X1 ; Ye = rlp->Y1 ; Ze = rlp->Z1 ; }
          dz = Ze - Zs ;
          di = bcdtmMath_distance(Xs,Ys,x,y) ;
          dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point Alp
*/
          (*IntPts+*IntPtsNe)->String1  = rlp->String  ;
          (*IntPts+*IntPtsNe)->Segment1 = rlp->Segment ;
          (*IntPts+*IntPtsNe)->String2  = plp->String  ;
          (*IntPts+*IntPtsNe)->Segment2 = plp->Segment ;
          (*IntPts+*IntPtsNe)->Distance = di ;
          (*IntPts+*IntPtsNe)->x = x ;
          (*IntPts+*IntPtsNe)->y = y ;
          (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
          (*IntPts+*IntPtsNe)->Z2 = 0.0 ;
          ++*IntPtsNe ;
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateRadialsAtPadEdge                                   |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateSideSlopeRadialsAtPadEdge(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,long sideSlopeElementType)
/*
** This Function Truncates The Radials At The Pad Edge
*/
{
 int     ret=0 ;
 long    dbg=0,IntTableNe,IntPtsNe,IntPtsMe,IntPtsMinc ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl ;
 DTM_STR_INT_TAB *pint,*IntTable=nullptr ;
 DTM_STR_INT_PTS *pinp,*pinpn,*IntPts=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radials At Pad Edge") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld]  Type = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Build Pad Line Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Edge Intersection Table") ;
 if( bcdtmSideSlope_buildRadialEdgeIntersectionTable(OvlPts,NumOvlPts,sideSlopeElementType,&IntTable,&IntTableNe) )  goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Radial Edge Intersection Table Entries = %4ld",IntTableNe) ;
/*
** Write Intersection Table
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Pad Line Intersection Table Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** String = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
      }
   }
/*
** Scan Intersection Table And For Intersections
*/
 IntPtsMinc = IntTableNe / 10 ;
 if( IntPtsMinc < 1000 ) IntPtsMinc = 1000 ;
 IntPtsNe = IntPtsMe = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ;
 if( bcdtmSideSlope_scanForRadialEdgeIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersections = %4ld",IntPtsNe) ;
/*
** Sort Intersection Points
*/
 if( IntPtsNe > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(IntPts,IntPtsNe,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
/*
** Write Intersection Points
*/
    if( dbg == 1  )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
       for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf",(long)(pinp-IntPts),pinp->String1,pinp->Segment1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z) ;
                 }
      }
/*
** Truncate Radials
*/
    for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
      {
/*
**  If Radial Intersection Point Then Set Parameters In Overlap Table
*/
       if(  pinp->String1 == 1 )
         {
          ovl = OvlPts + pinp->Segment1 ;
          ovl->Nx = pinp->x ;
          ovl->Ny = pinp->y ;
          ovl->Nz = pinp->z ;
          ovl->TruncatingEdge  = 1 ;
          ovl->EdgeZ = pinp->Z2 ;
          ovl->Status = 0 ;  // added 29/10/2007 RobC
          ovl->Gx = pinp->x ;
          ovl->Gy = pinp->y ;
          ovl->Gz = pinp->z ;
/*
** Scan To End Of Intersection Points For Radial
*/
          pinpn = pinp ;
          while ( pinpn < IntPts + IntPtsNe && pinpn->Segment1 == pinp->Segment1 ) ++pinpn ;
          --pinpn ;
          pinp = pinpn ;
         }
      }
   }
/*
** Write Truncated Radials
*/
 if( dbg == 2  )
   {
    for( ovl =  OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld] Type = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( IntTable != nullptr ) free(IntTable) ;
 if( IntPts   != nullptr ) free(IntPts) ;
/*
** Job Completed
*/
 if( dbg && ! ret )  bcdtmWrite_message(0,0,0,"Truncating Radials At Pad Edge Completed") ;
 if( dbg &&   ret )  bcdtmWrite_message(0,0,0,"Truncating Radials At Pad Edge Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmSideSlope_buildRadialEdgeIntersectionTable                   |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_buildRadialEdgeIntersectionTable(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,long sideSlopeElementType,DTM_STR_INT_TAB **IntTable,long *IntTableNe)
{
 int    ret=0 ;
 long   dbg=0,IntTableMe,IntTableMinc,sideSlopeType  ;
 double cord ;
 DTM_OVERLAP_RADIAL_TABLE *ovl,*ovn ;
 DTM_STR_INT_TAB *pint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Edge Intersection Table") ;
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 IntTableMinc = NumOvlPts * 2  ;
/*
** Store Pad Segments In Intersection Table
*/
 if( sideSlopeElementType == 1 ) sideSlopeType = 1 ;
 else                            sideSlopeType = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Pad Segments In Intersection Table") ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts - sideSlopeType ; ++ovl )
   {
    ovn = ovl + 1 ;
    if( ovn >= OvlPts + NumOvlPts ) ovn = OvlPts ;
    if( ovl->Px != ovn->Px || ovl->Py != ovn->Py )
      {
/*
**  Check For Memory Allocation
*/
       if( *IntTableNe == IntTableMe )
         {
          IntTableMe = IntTableMe + IntTableMinc ;
          if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
          else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
          if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
/*
**  Store Pad Line
*/
       (*IntTable+*IntTableNe)->String  = 0 ;
       (*IntTable+*IntTableNe)->Segment = (long)(ovl-OvlPts) ;
       (*IntTable+*IntTableNe)->Type = 1   ;
       (*IntTable+*IntTableNe)->Direction = 1 ;
       (*IntTable+*IntTableNe)->X1 = ovl->Px ;
       (*IntTable+*IntTableNe)->Y1 = ovl->Py ;
       (*IntTable+*IntTableNe)->Z1 = ovl->Pz ;
       (*IntTable+*IntTableNe)->X2 = ovn->Px ;
       (*IntTable+*IntTableNe)->Y2 = ovn->Py ;
       (*IntTable+*IntTableNe)->Z2 = ovn->Pz ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Pad Segment[%6ld] = %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(*IntTable+*IntTableNe)->Segment,(*IntTable+*IntTableNe)->X1,(*IntTable+*IntTableNe)->Y1,(*IntTable+*IntTableNe)->Z1,(*IntTable+*IntTableNe)->X2,(*IntTable+*IntTableNe)->Y2,(*IntTable+*IntTableNe)->Z2) ;
       ++*IntTableNe ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Pad Segments Stored In Intersection Table = %6ld",*IntTableNe) ;
/*
** Store Radials In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
   {
/*
**  Check For Memory Allocation
*/
    if( *IntTableNe == IntTableMe )
      {
       IntTableMe = IntTableMe + IntTableMinc ;
       if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store Ovl Line
*/
    (*IntTable+*IntTableNe)->String  = 1 ;
    (*IntTable+*IntTableNe)->Segment = (long)(ovl-OvlPts) ;
    (*IntTable+*IntTableNe)->Type    = 2   ;
    (*IntTable+*IntTableNe)->Direction = 1 ;
    (*IntTable+*IntTableNe)->X1 = ovl->Px ;
    (*IntTable+*IntTableNe)->Y1 = ovl->Py ;
    (*IntTable+*IntTableNe)->Z1 = ovl->Pz ;
    (*IntTable+*IntTableNe)->X2 = ovl->Nx ;
    (*IntTable+*IntTableNe)->Y2 = ovl->Ny ;
    (*IntTable+*IntTableNe)->Z2 = ovl->Nz ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Radial Segment[%6ld] = %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(*IntTable+*IntTableNe)->Segment,(*IntTable+*IntTableNe)->X1,(*IntTable+*IntTableNe)->Y1,(*IntTable+*IntTableNe)->Z1,(*IntTable+*IntTableNe)->X2,(*IntTable+*IntTableNe)->Y2,(*IntTable+*IntTableNe)->Z2) ;
    ++*IntTableNe ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( pint = *IntTable ; pint < *IntTable + *IntTableNe ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForRadialEdgeIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Base Line Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineRadialEdgeIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_determineRadialEdgeIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y,Rx,Ry ;
 DTM_STR_INT_TAB  *alp,*slp,*rlp,*plp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
** Only Compare Pad Segments Against Radial Segments
*/
    if( ( slp->String == 0 && alp->String == 1 ) || ( slp->String == 1 && alp->String == 0 ) )
      {
       if( slp->String == 1 ) { rlp = slp ; plp = alp ; }
       else                   { rlp = alp ; plp = slp ; }
/*
**  Check Radial Is Not From A Pad Segment End Point
*/
       if( rlp->Direction == 1 ) { Rx = rlp->X1 ; Ry = rlp->Y1 ; }
       else                      { Rx = rlp->X2 ; Ry = rlp->Y2 ; }
       if( ( Rx != plp->X1 || Ry != plp->Y1 ) && ( Rx != plp->X2 || Ry != plp->Y2 ) )
         {
/*
** Check Lines Intersect
*/
          if( bcdtmMath_checkIfLinesIntersect(plp->X1,plp->Y1,plp->X2,plp->Y2,rlp->X1,rlp->Y1,rlp->X2,rlp->Y2))
            {
/*
** Intersect Lines
*/
             bcdtmMath_normalIntersectCordLines(plp->X1,plp->Y1,plp->X2,plp->Y2,rlp->X1,rlp->Y1,rlp->X2,rlp->Y2,&x,&y) ;
/*
** Check Memory
*/
             if( *IntPtsNe >= *IntPtsMe )
               {
                *IntPtsMe = *IntPtsMe + IntPtsMinc ;
                if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                if( *IntPts == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
               }
/*
** Calculate Distances For rlp
*/
             if( rlp->Direction == 1 ) { Xs = rlp->X1 ; Ys = rlp->Y1 ; Zs = rlp->Z1 ; Xe = rlp->X2 ; Ye = rlp->Y2 ; Ze = rlp->Z2 ; }
             if( rlp->Direction == 2 ) { Xs = rlp->X2 ; Ys = rlp->Y2 ; Zs = rlp->Z2 ; Xe = rlp->X1 ; Ye = rlp->Y1 ; Ze = rlp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Radial Intersection Point
*/
             (*IntPts+*IntPtsNe)->String1  = rlp->String  ;
             (*IntPts+*IntPtsNe)->Segment1 = rlp->Segment ;
             (*IntPts+*IntPtsNe)->String2  = plp->String  ;
             (*IntPts+*IntPtsNe)->Segment2 = plp->Segment ;
             (*IntPts+*IntPtsNe)->Distance = di ;
             (*IntPts+*IntPtsNe)->x = x ;
             (*IntPts+*IntPtsNe)->y = y ;
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
             ++*IntPtsNe ;
/*
** Calculate Distances For Pad
*/
             if( plp->Direction == 1 ) { Xs = plp->X1 ; Ys = plp->Y1 ; Zs = plp->Z1 ; Xe = plp->X2 ; Ye = plp->Y2 ; Ze = plp->Z2 ; }
             if( plp->Direction == 2 ) { Xs = plp->X2 ; Ys = plp->Y2 ; Zs = plp->Z2 ; Xe = plp->X1 ; Ye = plp->Y1 ; Ze = plp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Z2 Values For Radial
*/
             (*IntPts+*IntPtsNe-1)->Z2 =  Zs + dz * di / dl ;
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateElementRadialsWithElementEndRadials                |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateElementRadialsWithElementEndRadials(long SideDirection,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Truncates The Radials With The Element End Radials
*/
{
 int     sd1,sd2  ;
 long    dbg=0,n,intflag ;
 double  d1,d2,dz ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl,*ovp ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Element Radials With Element End Radials") ;
/*
** Scan Radials And Detect Intersection With End Radials
*/
 for( n = 0 ; n < 2 ; ++n )
   {
    if( ! n ) ovl = OvlPts ;
    else      ovl = OvlPts + NumOvlPts - 1 ;
    for( ovp = OvlPts + 1 ; ovp < OvlPts + NumOvlPts - 1 ; ++ovp )
      {
       sd1 = bcdtmMath_sideOf(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovp->Px,ovp->Py) ;
       sd2 = bcdtmMath_sideOf(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovp->Nx,ovp->Ny) ;
       if( sd1 != sd2 )
         {
          sd1 = bcdtmMath_sideOf(ovp->Px,ovp->Py,ovp->Nx,ovp->Ny,ovl->Px,ovl->Py) ;
          sd2 = bcdtmMath_sideOf(ovp->Px,ovp->Py,ovp->Nx,ovp->Ny,ovl->Nx,ovl->Ny) ;
          if( sd1 != sd2 )
            {
/*
**  Check Intersecting Radial On Correct Side Of End Radial
*/
             intflag = 0 ;
             if( ( n == 0 && SideDirection == 1 ) || n == 1 && SideDirection == 2 )
               {
                sd1 = bcdtmMath_sideOf(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovp->Px,ovp->Py) ;
                if( sd1 == 1 ) intflag = 1 ;
               }
             if( ( n == 1 && SideDirection == 1 ) || n == 0 && SideDirection == 2 )
               {
                sd1 = bcdtmMath_sideOf(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovp->Px,ovp->Py) ;
                if( sd1 == -1 ) intflag = 1 ;
               }
/*
**  If Radial On Correct Side Truncate Radial
*/
             if( intflag )
               {
                dz = ovl->Gz - ovl->Pz ;
                d1 = bcdtmMath_distance(ovl->Px,ovl->Py,ovl->Gx,ovl->Gy) ;
                bcdtmMath_normalIntersectCordLines(ovl->Px,ovl->Py,ovl->Nx,ovl->Ny,ovp->Px,ovp->Py,ovp->Nx,ovp->Ny,&ovp->Nx,&ovp->Ny) ;
                d2 = bcdtmMath_distance(ovl->Px,ovl->Py,ovp->Nx,ovp->Ny) ;
                ovp->Nz = ovl->Pz + dz * d2 / d1 ;
                ovp->Status = 0 ;
                ovp->TruncatingRadial = (long)(ovl-OvlPts) ;
               }
            }
         }
      }
   }
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Element Radials With Element End Radials Completed") ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_intersectRadials                                     |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_intersectSideSlopeRadials(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,long sideSlopeElementType,long *RadialIntersectFlag)
/*
** This Function Intersects Radials
*/
{
 int    ret=0 ;
 long   dbg=0,IntTableNe,IntPtsNe,IntPtsMe,IntPtsMinc ;
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
 DTM_STR_INT_TAB *pint,*IntTable=nullptr ;
 DTM_STR_INT_PTS *pinp,*IntPts=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials ** Number Of Radials = %6ld",NumOvlPts) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld]  Type = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Initialise
*/
 *RadialIntersectFlag = 0 ;
/*
** Create And Build Radial Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Intersection Table") ;
 if( bcdtmSideSlope_buildRadialIntersectionTable(OvlPts,NumOvlPts,&IntTable,&IntTableNe)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Radial Intersection Table Entries = %6ld",IntTableNe) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Pad Line Intersection Table Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** Pad = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
      }
   }
/*
** Scan For Intersections
*/
 IntPtsMinc = IntTableNe * 2 ;
 if( IntPtsMinc < 10000 ) IntPtsMinc = 10000 ;
 IntPtsNe = IntPtsMe = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Radial Intersections") ;
 if( bcdtmSideSlope_scanForRadialIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Radial Intersections = %4ld",IntPtsNe) ;
/*
** Process Intersection Points
*/
 if( IntPtsNe > 0 )
   {
    *RadialIntersectFlag = 1 ;
/*
** Sort Intersection Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(IntPts,IntPtsNe,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_radialRadialIntersectionPointsCompareFunction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
/*
** Write Intersection Points
*/
    if( dbg == 2  )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
       for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%5ld] ** Str1 = %4ld Seg1 = %7ld Str2 = %4ld Seg2 = %7ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(pinp-IntPts),pinp->String1,pinp->Segment1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z,pinp->Z2) ;
                 }
      }
/*
** Truncate Radials Using Intersection Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radials Using Intersection Table") ;
    if( bcdtmSideSlope_truncateRadialsUsingIntersectionPoints(sideSlopeElementType,OvlPts,NumOvlPts,IntPts,IntPtsNe) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radials Using Truncation Table Completed") ;
   }
/*
** Write Radial Truncations
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Radials = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** %10.4lf %10.4lf %10.4lf ** %10.4lf TR = %9ld",(long)(ovl-OvlPts),ovl->Px,ovl->Py,ovl->Pz,ovl->Nz,ovl->TruncatingRadial) ;
      }
   }
/*
** CleanUp
*/
 cleanup :
 if( IntTable != nullptr ) free(IntTable) ;
 if( IntPts   != nullptr ) free(IntPts) ;
/*
** Normal Exit
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Intersecting Radials Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Intersecting Radials Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_radialRadialIntersectionPointsCompareFunction(const DTM_STR_INT_PTS *Tp1,const DTM_STR_INT_PTS  *Tp2)
/*
** Compare Function For Qsort Of Radial Intersection Points
*/
{
 if     (  Tp1->Segment1  < Tp2->Segment1 ) return(-1) ;
 else if(  Tp1->Segment1  > Tp2->Segment1 ) return( 1) ;
 else if(  Tp1->Distance  < Tp2->Distance ) return(-1) ;
 else if(  Tp1->Distance  > Tp2->Distance ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_buildRadialIntersectionTable(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_TAB **IntTable,long *IntTableNe)
{
 int    ret=0 ;
 long   dbg=0,IntTableMe,IntTableMinc  ;
 double cord ;
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
 DTM_STR_INT_TAB *pint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Intersection Table") ;
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 IntTableMinc = NumOvlPts * 2  ;
/*
** Store Radials In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
   {
/*
**  Check For Memory Allocation
*/
    if( *IntTableNe == IntTableMe )
      {
       IntTableMe = IntTableMe + IntTableMinc ;
       if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store Ovl Line
*/
    (*IntTable+*IntTableNe)->String  = 1 ;
    (*IntTable+*IntTableNe)->Segment = (long)(ovl-OvlPts) ;
    (*IntTable+*IntTableNe)->Type    = 1   ;
    (*IntTable+*IntTableNe)->Direction = 1 ;
    (*IntTable+*IntTableNe)->X1 = ovl->Px ;
    (*IntTable+*IntTableNe)->Y1 = ovl->Py ;
    (*IntTable+*IntTableNe)->Z1 = ovl->Pz ;
    (*IntTable+*IntTableNe)->X2 = ovl->Nx ;
    (*IntTable+*IntTableNe)->Y2 = ovl->Ny ;
    (*IntTable+*IntTableNe)->Z2 = ovl->Nz ;
    ++*IntTableNe ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( pint = *IntTable ; pint < *IntTable + *IntTableNe ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForRadialIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineRadialIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_determineRadialIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y,Ax,Ay,Sx,Sy ;
 DTM_STR_INT_TAB  *alp,*slp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
**  Check Radial Start Points Are Not Coincident
*/
    if( alp->Direction == 1 ) { Ax = alp->X1 ; Ay = alp->Y1 ; }
    else                      { Ax = alp->X2 ; Ay = alp->Y2 ; }
    if( slp->Direction == 1 ) { Sx = slp->X1 ; Sy = slp->Y1 ; }
    else                      { Sx = slp->X2 ; Sy = slp->Y2 ; }
    if( Ax != Sx || Ay != Sy )
      {
/*
** Check Lines Intersect
*/
       if( bcdtmMath_checkIfLinesIntersect(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2))
         {
/*
** Intersect Lines
*/
          bcdtmMath_normalIntersectCordLines(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2,&x,&y) ;
/*
** Check Memory
*/
          if( *IntPtsNe + 1 >= *IntPtsMe )
            {
             *IntPtsMe = *IntPtsMe + IntPtsMinc ;
             if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             if( *IntPts == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
** Calculate Distances For Alp
*/
          if( alp->Direction == 1 ) { Xs = alp->X1 ; Ys = alp->Y1 ; Zs = alp->Z1 ; Xe = alp->X2 ; Ye = alp->Y2 ; Ze = alp->Z2 ; }
          if( alp->Direction == 2 ) { Xs = alp->X2 ; Ys = alp->Y2 ; Zs = alp->Z2 ; Xe = alp->X1 ; Ye = alp->Y1 ; Ze = alp->Z1 ; }
          dz = Ze - Zs ;
          di = bcdtmMath_distance(Xs,Ys,x,y) ;
          dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point Alp
*/
          (*IntPts+*IntPtsNe)->String1  = *IntPtsNe  ;
          (*IntPts+*IntPtsNe)->Segment1 = alp->Segment ;
          (*IntPts+*IntPtsNe)->String2  = *IntPtsNe + 1  ;
          (*IntPts+*IntPtsNe)->Segment2 = slp->Segment ;
          (*IntPts+*IntPtsNe)->Distance = di ;
          (*IntPts+*IntPtsNe)->x = x ;
          (*IntPts+*IntPtsNe)->y = y ;
          (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
          ++*IntPtsNe ;
/*
** Calculate Distances For Slp
*/
          if( slp->Direction == 1 ) { Xs = slp->X1 ; Ys = slp->Y1 ; Zs = slp->Z1 ; Xe = slp->X2 ; Ye = slp->Y2 ; Ze = slp->Z2 ; }
          if( slp->Direction == 2 ) { Xs = slp->X2 ; Ys = slp->Y2 ; Zs = slp->Z2 ; Xe = slp->X1 ; Ye = slp->Y1 ; Ze = slp->Z1 ; }
          dz = Ze - Zs ;
          di = bcdtmMath_distance(Xs,Ys,x,y) ;
          dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point For Slp
*/
          (*IntPts+*IntPtsNe)->String1  = *IntPtsNe  ;
          (*IntPts+*IntPtsNe)->Segment1 = slp->Segment ;
          (*IntPts+*IntPtsNe)->String2  = *IntPtsNe - 1  ;
          (*IntPts+*IntPtsNe)->Segment2 = alp->Segment ;
          (*IntPts+*IntPtsNe)->Distance = di ;
          (*IntPts+*IntPtsNe)->x = x ;
          (*IntPts+*IntPtsNe)->y = y ;
          (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
          ++*IntPtsNe ;
/*
** Store Z2 Values
*/
          (*IntPts+*IntPtsNe-2)->Z2 = (*IntPts+*IntPtsNe-1)->z ;
          (*IntPts+*IntPtsNe-1)->Z2 = (*IntPts+*IntPtsNe-2)->z ;
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateRadialsUsingIntersectionPoints               |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateRadialsUsingIntersectionPoints(long sideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_PTS *IntPts,long NumIntPts)
{
 int     ret=0 ;
 long    dbg=0,process,loop,NumIntersections,Offset,TruncatedRadial ;
 long    *IntIndex=nullptr,*TmpIndex=nullptr,*pind,*RadIndex=nullptr ;
 double  dz1,dz2,Nx,Ny,Nz1,Nz2 ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl1,*ovl2 ;
 DTM_STR_INT_PTS *pinp,*pinp1 ;
 long    Radial1,Radial2,Radial2Ofs,Radial1End,Radial2End,Radial2Truncated ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radials Using Intersection Points") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Side Slope Element Type = %2ld",sideSlopeElementType) ;
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",NumOvlPts) ;
    for( ovl1 =  OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%5ld] ** T = %1ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Type,ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Gx,ovl1->Gy,ovl1->Gz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
      }
   }
/*
** Allocate memory For Index
*/
 IntIndex = ( long * ) malloc( NumIntPts * sizeof(long) ) ;
 if( IntIndex == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
 TmpIndex = ( long * ) malloc( NumIntPts * sizeof(long) ) ;
 if( TmpIndex == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Intersection Points Table And Build Index To Corresponding Radial For Recursive Searching
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Index To Corresponding Intersection Points Table Entry") ;
 for( pinp1 = IntPts ; pinp1 < IntPts + NumIntPts ; ++pinp1 )
   {
    *(TmpIndex+pinp1->String1) = (long)(pinp1-IntPts) ;
   }
 for( pinp1 = IntPts ; pinp1 < IntPts + NumIntPts ; ++pinp1 )
   {
    *(IntIndex+(long)(pinp1-IntPts)) = *(TmpIndex+pinp1->String2) ;
    pinp1->String1 = pinp1->String2 = 0 ;
   }
/*
** Free Temp Index Memory
*/
 free(TmpIndex) ; TmpIndex = nullptr ;
/*
**  Write Intersection Point Index Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %4ld",NumIntPts) ;
    for( pinp1 = IntPts ,pind = IntIndex ; pinp1 < IntPts + NumIntPts ; ++pinp1 , ++pind )
      {
           bcdtmWrite_message(0,0,0,"Point[%6ld] ** Segment1 = %6ld Segment2 = %6ld Index = %9ld",(long)(pinp1-IntPts),pinp1->Segment1,pinp1->Segment2,*pind) ;
      }
   }
/*
** Build Radial Index
*/
 RadIndex = ( long * ) malloc ( NumOvlPts * sizeof(long)) ;
 if( RadIndex == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Null Out Radial Index
*/
 for( pind = RadIndex ; pind < RadIndex + NumOvlPts ; ++pind ) *pind = DTM_NULL_PNT ;
/*
** Scan Intersection Point Table And Set Index For Radials To Intersection Points Table
*/
 for( pinp1 = IntPts ; pinp1 < IntPts + NumIntPts ; ++pinp1 )
   {
    if( *(RadIndex + pinp1->Segment1) == DTM_NULL_PNT ) *(RadIndex + pinp1->Segment1) = (long)(pinp1-IntPts) ;
   }
/*
** Write Out Radial Index
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radial Indexs = %4ld",NumOvlPts) ;
    for( pind = RadIndex ; pind < RadIndex + NumOvlPts; ++pind )
      {
           bcdtmWrite_message(0,0,0,"Radial[%9ld] **  Index = %9ld",(long)(pind-RadIndex),*pind) ;
      }
   }
/*
** Scan Radial Index Table And Truncate Radials
*/
 loop = 0 ;
 process = 1 ;
 NumIntersections=0 ;
 while ( process && loop < 100 )
   {
    ++loop ;
    process = 0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"===========Loop = %3ld===========",loop) ;
/*
** Scan Intersection Points Looking For Radial Truncations
*/
    for( pinp = IntPts ; pinp < IntPts + NumIntPts ; ++pinp )
      {
       if( pinp->String1 != DTM_NULL_PNT && pinp->String2 != DTM_NULL_PNT )
         {
          Radial1    = pinp->Segment1 ;
          Radial2    = pinp->Segment2 ;
          Radial2Ofs = *(RadIndex+Radial2) ;
          Radial1End = (long)(pinp-IntPts) ;
          Radial2End = *(IntIndex+Radial1End) ;
          Radial2Truncated = bcdtmSideSlope_checkForPriorTruncationOfTruncatingRadial(OvlPts,IntPts,Radial2Ofs,Radial2End) ;
/*
** If Either Radial Not Previously Truncated Test For Possible Truncation
*/
          if( ! Radial2Truncated )
            {
             ovl1 = OvlPts + Radial1 ;
             ovl2 = OvlPts + Radial2 ;
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Intersecting Radials %6ld %6ld",(long)(ovl1-OvlPts),(long)(ovl2-OvlPts)) ;
                bcdtmWrite_message(0,0,0,"**** Ovl1 = [%6ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl1-OvlPts),ovl1->Type,ovl1->Status,ovl1->Px,ovl1->Py,ovl1->Nx,ovl1->Ny,ovl1->Gx,ovl1->Gy) ;
                bcdtmWrite_message(0,0,0,"**** Ovl2 = [%6ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl2-OvlPts),ovl2->Type,ovl2->Status,ovl2->Px,ovl2->Py,ovl2->Nx,ovl2->Ny,ovl2->Gx,ovl2->Gy) ;
               }
/*
** Set Intersection Variables
*/
             ++process  ;
             ++NumIntersections ;
             TruncatedRadial = DTM_NULL_PNT ;
             Nx  = pinp->x ;
             Ny  = pinp->y ;
             dz1 = ovl1->Gz - ovl1->Pz ;
             Nz1 = pinp->z ;
             dz2 = ovl2->Gz - ovl2->Pz ;
             Nz2 = pinp->Z2 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"fabs(Nz1-Nz2) = %20.10lf ** Nx = %10.4lf Ny = %10.4lf Nz1 = %10.4lf Nz2 = %10.4lf",fabs(Nz1-Nz2),Nx,Ny,Nz1,Nz2) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"dz1 = %20.15lf ** dz2 = %20.15lf",dz1,dz2) ;

//             if( fabs(Nz1-Nz2) < 0.0001 ) Nz1 = Nz2 ;
             if( fabs(Nz1-Nz2) < 0.01 ) Nz1 = Nz2 ;
/*
**  Radials Truncate At Same Elevation
*/
             if( Nz1 == Nz2 )
               {
                if( ovl1->Type == 1 && ovl2->Type != 1 ) { ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ; TruncatedRadial = 2 ; }
                if( ovl1->Type != 1 && ovl2->Type == 1 ) { ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz2 ; TruncatedRadial = 1 ; }
                if( ( ovl1->Type == 1 && ovl2->Type == 1 ) ||
                    ( ovl1->Type != 1 && ovl2->Type != 1 ) ||
                    ( ovl1->Type == 1 && sideSlopeElementType == 1 && ( ovl2 == OvlPts || ovl2 == OvlPts + NumOvlPts - 1 ) )  ||
                    ( ovl2->Type == 1 && sideSlopeElementType == 1 && ( ovl1 == OvlPts || ovl1 == OvlPts + NumOvlPts - 1 ) )      )
                  {
                   ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz2 ;
                   ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ;
                   TruncatedRadial = 3 ;
                  }

               }
/*
**  Both Radials Level
*/
             else if( dz1 == 0.0 && dz2 == 0.0 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Both Radials Level") ;
                if( Nz1 >  Nz2 ) { ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz1 ; TruncatedRadial = 1 ; }
                if( Nz2 >  Nz1 ) { ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ; TruncatedRadial = 2 ; }
               }
/*
**  Both Radials Going Down
*/
             else if( dz1 <= 0.0 && dz2 <= 0.0 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Both Radials Going Down") ;
                if( Nz1 <  Nz2 ) { ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz1 ; TruncatedRadial = 1 ; }
                if( Nz2 <  Nz1 ) { ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ; TruncatedRadial = 2 ; }
               }
/*
**  Both Radials Going Up
*/
             else if( dz1 >= 0.0 && dz2 >= 0.0 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Both Radials Going Up") ;
                if( Nz1 > Nz2 ) { ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz1 ; TruncatedRadial = 1 ; }
                if( Nz2 > Nz1 ) { ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ; TruncatedRadial = 2 ; }
               }
/*
**  One Radial Going Up And One Radial Going Down ( Caused By Small Slopes From Uneven Pads )
*/
             else if( ( dz1 >= 0.0 && dz2 <= 0.0 ) || ( dz1 <= 0.0 && dz2 >= 0.0 ) )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"One Radial Up One Radial Down") ;
                if     ( dz2 >= 0.0 ) { ovl1->Nx = Nx ; ovl1->Ny = Ny ; ovl1->Nz = Nz1 ; TruncatedRadial = 1 ; }
                else if( dz1 >= 0.0 ) { ovl2->Nx = Nx ; ovl2->Ny = Ny ; ovl2->Nz = Nz2 ; TruncatedRadial = 2 ; }
               }
/*
** Reset Intersection Table For Truncation Of Radial 1
*/
             if( TruncatedRadial == 1 || TruncatedRadial == 3 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radial One") ;
                ovl1->TruncatingRadial = (long)(ovl2-OvlPts) ;
                Offset = (long)(pinp-IntPts) ;
                if( bcdtmSideSlope_removeActiveIntersectionPointsForRadialFromOffset(Radial1,IntPts,NumIntPts,Offset,IntIndex) ) goto errexit ;
               }
/*
** Reset Intersection Table For Truncation Of Radial 2
*/
             if( TruncatedRadial == 2 || TruncatedRadial == 3 )
               {
                if( dbg )  bcdtmWrite_message(0,0,0,"Truncating Radial Two") ;
                ovl2->TruncatingRadial = (long)(ovl1-OvlPts) ;
                Offset = *(IntIndex+(long)(pinp-IntPts)) ;
                if( bcdtmSideSlope_removeActiveIntersectionPointsForRadialFromOffset(Radial2,IntPts,NumIntPts,Offset,IntIndex) ) goto errexit ;
               }
/*
** Write Truncated Radials
*/
             if( dbg && TruncatedRadial != DTM_NULL_PNT )
               {
                bcdtmWrite_message(0,0,0,"Truncated Radial =  %2ld",TruncatedRadial) ;
                bcdtmWrite_message(0,0,0,"Ovl1 = [%6ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl1-OvlPts),ovl1->Type,ovl1->Status,ovl1->Px,ovl1->Py,ovl1->Nx,ovl1->Ny,ovl1->Gx,ovl1->Gy) ;
                bcdtmWrite_message(0,0,0,"Ovl2 = [%6ld] T = %1ld S = %1ld ** %8.2lf %8.2lf ** %8.2lf %8.2lf ** %8.2lf %8.2lf",(long)(ovl2-OvlPts),ovl2->Type,ovl2->Status,ovl2->Px,ovl2->Py,ovl2->Nx,ovl2->Ny,ovl2->Gx,ovl2->Gy) ;
               }
            }
         }
      }
   }
/*
** Write Of Number Of Loops
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Truncation Loops = %6ld Number Of Intersections = %6ld",loop,NumIntersections) ;
/*
** Clean Up
*/
 cleanup :
 if( IntIndex != nullptr ) free(IntIndex) ;
 if( TmpIndex != nullptr ) free(TmpIndex) ;
 if( RadIndex != nullptr ) free(RadIndex) ;
/*
** Normal Exit
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Truncating Radials Using Intersection Points Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Truncating Radials Using Intersection Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_checkForPriorTruncationOfTruncatingRadial            |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_checkForPriorTruncationOfTruncatingRadial(DTM_OVERLAP_RADIAL_TABLE *OvlPts,DTM_STR_INT_PTS *RadIntPts,long RadStartOfs,long RadEndOfs)
/*
** This Function Checks For A Possible Prior Truncation Of The Truncating Radial
*/
{
 long   dbg=0,RadOfs;
 double dz1,dz2,z1,z2 ;
 DTM_OVERLAP_RADIAL_TABLE *ovl1,*ovl2 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Prior Truncation Of Truncating Radial ** %4ld",(RadIntPts+RadStartOfs)->Segment1 ) ;
/*
** Initialise
*/
 ovl1 = OvlPts + (RadIntPts+RadStartOfs)->Segment1 ;
 dz1  = ovl1->Gz - ovl1->Pz  ;
/*
** Scan Radial Intersection Points Table
*/
 for( RadOfs = RadStartOfs ; RadOfs < RadEndOfs && (RadIntPts+RadOfs)->String1 != DTM_NULL_PNT ; ++RadOfs )
   {
    if( (RadIntPts+RadOfs)->String2 != DTM_NULL_PNT )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial1 = %6ld Radial2 = %6ld",(RadIntPts+RadOfs)->Segment1,(RadIntPts+RadOfs)->Segment2) ;
       z1   = (RadIntPts+RadOfs)->z ;
       ovl2 = OvlPts + (RadIntPts+RadOfs)->Segment2 ;
       dz2  = ovl2->Gz - ovl2->Pz  ;
       z2   = (RadIntPts+RadOfs)->Z2 ;
       if( fabs(z1-z2) < 0.001 ) z1 = z2 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"dz1 = %10.4lf dz2 = %10.4lf ** Nz1 = %10.4lf Nz2 = %10.4lf",dz1,dz2,z1,z2) ;
/*
**  Radials Intersect At Same Elevation
*/
       if( z1 == z2 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Same Elevation ovl1->type = %1ld  ovl1->type = %1ld",ovl1->Type,ovl2->Type) ;
          if( ovl2->Type == 1 ) return(1) ;
          if( ovl2->Type != 1 && ovl1->Type != 1 ) return(1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"No Prior Truncation Same Elevatio By ovl1") ;
         }
/*
**  Both Radials Level
*/
       else if( dz1 == 0.0 && dz2 == 0.0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Lvel ovl1->type = %1ld  ovl1->type = %1ld",ovl1->Type,ovl2->Type) ;
          if( z2 >  z1 ) return(1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"No Prior Truncation Going Down By ovl1") ;
         }
/*
**  Both Radials Going Down
*/
       else if( dz1 <= 0.0 && dz2 <= 0.0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Going Down ovl1->type = %1ld  ovl1->type = %1ld",ovl1->Type,ovl2->Type) ;
          if( z2 >  z1 ) return(1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"No Prior Truncation Going Down By ovl1") ;
         }
/*
**  Both Radials Going Up
*/
       else if( dz1 >= 0.0 && dz2 >= 0.0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Going Up ovl1->type = %1ld  ovl1->type = %1ld",ovl1->Type,ovl2->Type) ;
          if( z2 < z1 ) return(1)  ;
          if( dbg ) bcdtmWrite_message(0,0,0,"No Prior Truncation Going Up By ovl1") ;
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_removeActiveIntersectionPointsForRadial                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeActiveIntersectionPointsForRadialFromOffset(long Radial,DTM_STR_INT_PTS *IntPts,long NumIntPts,long Offset,long *IntIndex)
{
 long dbg=0 ;
 DTM_STR_INT_PTS *pinp1,*pinp2 ;
/*
** Initialise Scan Start Point
*/
 pinp1 = IntPts + Offset ;
/*
** Check Radial Corresponds With Offset
*/
 if( Radial != pinp1->Segment1 )
   {
    bcdtmWrite_message(0,0,0,"Radial = %6ld  Scan Offset Radial = %6ld",Radial,pinp1->Segment1) ;
    goto errexit ;
   }
/*
** Scan Intersection Points And Mark Active Intersection Points As Inactive
*/
 while ( pinp1 < IntPts + NumIntPts && pinp1->Segment1 == Radial )
   {
    pinp2 = IntPts + *(IntIndex+(long)(pinp1-IntPts)) ;
    pinp1->String1 = DTM_NULL_PNT ;
    pinp2->String2 = DTM_NULL_PNT ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"pinp1 ****  Seg1 = %6ld Seg2 = %6ld Str1 = %9ld Str2 = %9ld",pinp1->Segment1,pinp1->Segment2,pinp1->String1,pinp1->String2) ;
       bcdtmWrite_message(0,0,0,"pinp2 ****  Seg1 = %6ld Seg2 = %6ld Str1 = %9ld Str2 = %9ld",pinp2->Segment1,pinp2->Segment2,pinp2->String1,pinp2->String2) ;
      }
    ++pinp1 ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit   :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateIntersectedSideSlopeRadials                  |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateIntersectedSideSlopeRadials(long Direction,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Truncates Intersected Radials
** Direction = 1  Right
** Direction = 2  Left ;
*/
{
 int     sd1 ;
 long    dbg=0,ofs,ofsP,ofsN,process=0;
 bool tryNewCode=true ;
 double  dz1,dz2 ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl1,*ovl2,*ovl3,*ovlP,*ovlN ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Intersected Radials ** Direction = %2ld",Direction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",NumOvlPts) ;
    for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
      }
   }
 if( tryNewCode == true) goto newcode ;
/*
** Scan Radials And Truncate Intersected Radials
*/
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
   {
    if( ovl1->TruncatingRadial != DTM_NULL_PNT )
      {
           ovl2 = OvlPts+ovl1->TruncatingRadial ;
       ovl3 = nullptr ;
       if( ovl2->TruncatingRadial != DTM_NULL_PNT ) ovl3 = OvlPts+ovl2->TruncatingRadial ;
/*
**  Check If Radial Can Be Truncated
*/
       ofs = (long)(ovl1-ovl2) ;
       if( ovl3 != ovl1 && ( ofs < 1 || ofs > 1 ) )
         {
/*
**  Radial Not Truncated By Type One Radial
*/
          if( ovl2->Type != 1 )
                {
             if( dbg ) bcdtmWrite_message(0,0,0,"Radial %6ld Truncated By Radial %6ld",(long)(ovl1-OvlPts),ovl1->TruncatingRadial) ;
             sd1 = bcdtmMath_sideOf(ovl2->Px,ovl2->Py,ovl2->Gx,ovl2->Gy,ovl1->Px,ovl1->Py) ;
             if( Direction == 1 )
               {
                        if( sd1 >= 0 ) ovl3 = ovl2 + 1 ;
                        else           ovl3 = ovl2 - 1 ;
               }
             if( Direction == 2 )
               {
                        if( sd1 <= 0 ) ovl3 = ovl2 + 1 ;
                        else           ovl3 = ovl2 - 1 ;
               }
                     if( ovl3 <  OvlPts ) ovl3 = OvlPts + NumOvlPts - 1 ;
                     if( ovl3 >= OvlPts + NumOvlPts ) ovl3 = OvlPts ;
             dz1 = ovl1->Nz - ovl1->Pz ;
             dz2 = ovl2->Nz - ovl2->Pz ;
             if( dbg ) bcdtmWrite_message(0,0,0,"dz1 = %20.15lf dz2 = %20.15lf",dz1,dz2) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Ovl1 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Ovl2 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl2-OvlPts),ovl2->Px,ovl2->Py,ovl2->Pz,ovl2->Nx,ovl2->Ny,ovl2->Nz) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Ovl3 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl3-OvlPts),ovl3->Px,ovl3->Py,ovl3->Pz,ovl3->Nx,ovl3->Ny,ovl3->Nz) ;
             if( dz1 < 0.0 && dz2 < 0.0 ) bcdtmSideSlope_truncateSideSlopeRadial(OvlPts,ovl1,ovl2,ovl3,3) ;
             if( dz1 > 0.0 && dz2 > 0.0 ) bcdtmSideSlope_truncateSideSlopeRadial(OvlPts,ovl1,ovl2,ovl3,3) ;
             ovl1->TruncatingRadial = DTM_NULL_PNT ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Ovl1 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
            }
         }
      }
   }
/*
**  New Code
*/
 newcode :
/*
** Scan Radials And Truncate Intersected Radials
** New Code Added 12/March/2004  RobC
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"***** Truncating Radials") ;
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
      {
       if( ovl1->TruncatingRadial != DTM_NULL_PNT )
         {
          ofs  = (long)(ovl1-OvlPts) ;
              ovl2 = OvlPts+ovl1->TruncatingRadial ;
/*
**  Radial Not Truncated By Type One Radial
*/
          if( ovl2->Type != 1 && ovl2->TruncatingRadial == DTM_NULL_PNT )
            {
             ovlP = ovl2 - 1 ;
             if( ovlP < OvlPts ) ovlP = OvlPts + NumOvlPts - 1 ;
             ofsP  = (long)(ovlP-OvlPts) ;
             ovlN = ovl2 + 1 ;
             if( ovlN >= OvlPts + NumOvlPts ) ovlN = OvlPts ;
             ofsN  = (long)(ovlN-OvlPts) ;
/*
**  Check If Radial Can Be Truncated
*/
             ovl3 = nullptr ;
             if( ovlP->TruncatingRadial == ofs ) ovl3 = ovlP ;
             if( ovlN->TruncatingRadial == ofs ) ovl3 = ovlN ;
             if( ovl3 != nullptr )
               {
                dz1 = ovl1->Nz - ovl1->Pz ;
                dz2 = ovl2->Nz - ovl2->Pz ;
                if( dbg ) bcdtmWrite_message(0,0,0,"dz1 = %20.15lf dz2 = %20.15lf",dz1,dz2) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Ovl1 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Ovl2 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl2-OvlPts),ovl2->Px,ovl2->Py,ovl2->Pz,ovl2->Nx,ovl2->Ny,ovl2->Nz) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Ovl3 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl3-OvlPts),ovl3->Px,ovl3->Py,ovl3->Pz,ovl3->Nx,ovl3->Ny,ovl3->Nz) ;
                if( ( dz1 < 0.0 && dz2 < 0.0 ) || ( dz1 > 0.0 && dz2 > 0.0 ) )
                  {
                   bcdtmSideSlope_truncateSideSlopeRadial(OvlPts,ovl1,ovl2,ovl3,4) ;
                   ovl1->TruncatingRadial = DTM_NULL_PNT ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Ovl1 Radial[%6ld] ** %12.6lf %12.6lf %10.4lf ** %12.6lf %12.6lf %10.4lf",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz) ;
                   process = 1 ;
                  }
               }
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_truncateSideSlopeRadial                              |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateSideSlopeRadial(DTM_OVERLAP_RADIAL_TABLE *OvlPts,DTM_OVERLAP_RADIAL_TABLE *Ovl1,DTM_OVERLAP_RADIAL_TABLE *Ovl2,DTM_OVERLAP_RADIAL_TABLE *Ovl3,long Flag)
/*
** This Function Truncates Intersected Side Slope Radials
*/
{
 long   dbg=0 ;
 double dx,dy,dz,dz1,dz2,Ca,Cb,Cc,Cd,Zs,Zn,Sx,Sy,Sz,Xmin,Xmax,Ymin,Ymax,Zmin,Zmax ;
 double Snx,Sny,Snz,d1,d2,z,Bx,By,Bz,Rz ;
/*
** Write
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Ovl1 = %6ld Ovl2 = %6ld Ovl3 = %6ld Flag = %2ld",(long)(Ovl1-OvlPts),(long)(Ovl2-OvlPts),(long)(Ovl3-OvlPts),Flag) ;
    bcdtmWrite_message(0,0,0,"** Px = %10.4lf %10.4lf %10.4lf",Ovl1->Px,Ovl1->Py,Ovl1->Pz) ;
    bcdtmWrite_message(0,0,0,"** Nx = %10.4lf %10.4lf %10.4lf",Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
   }
/*
** Save End Point
*/
 Snx = Ovl1->Nx ; Sny = Ovl1->Ny ; Snz = Ovl1->Nz ;
 if( Ovl1->Px <= Ovl1->Nx ) { Xmin = Ovl1->Px ; Xmax = Ovl1->Nx ; }
 else                       { Xmax = Ovl1->Px ; Xmin = Ovl1->Nx ; }
 if( Ovl1->Py <= Ovl1->Ny ) { Ymin = Ovl1->Py ; Ymax = Ovl1->Ny ; }
 else                       { Ymax = Ovl1->Py ; Ymin = Ovl1->Ny ; }
 if( Ovl1->Pz <= Ovl1->Nz ) { Zmin = Ovl1->Pz ; Zmax = Ovl1->Nz ; }
 else                       { Zmax = Ovl1->Pz ; Zmin = Ovl1->Nz ; }
/*
** Both Radials Going Up
*/
 if( Flag == 1 ) return(0) ;
/*
** Radial Going Down
*/
 if( Flag == 2 )
   {
/*
** Truncate Radial
*/
    bcdtmMath_intersectCordLines(Ovl1->Px,Ovl1->Py,Ovl1->Gx,Ovl1->Gy,Ovl3->Px,Ovl3->Py,Ovl3->Gx,Ovl3->Gy,&Sx,&Sy) ;
    bcdtmMath_interpolatePointOnLine(Ovl1->Px,Ovl1->Py,Ovl1->Pz,Ovl1->Gx,Ovl1->Gy,Ovl1->Gz,Sx,Sy,&Sz) ;
    bcdtmMath_calculatePlaneCoefficients(Ovl2->Px,Ovl2->Py,Ovl2->Pz,Ovl2->Gx,Ovl2->Gy,Ovl2->Gz,Ovl3->Gx,Ovl3->Gy,Ovl3->Gz,&Ca,&Cb,&Cc,&Cd) ;
    if( Cc == 0.0 ) Cc = 0.000000001 ;
    Zn = - ( Ca * Ovl1->Nx + Cb * Ovl1->Ny + Cd ) / Cc ;
    Zs = - ( Ca * Sx + Cb * Sy + Cd ) / Cc ;
    dz1 = fabs(Zn-Ovl1->Nz) ;
    dz2 = fabs(Zs-Sz) ;
    dx = Ovl1->Nx - Sx  ;
    dy = Ovl1->Ny - Sy  ;
    dz = Ovl1->Nz - Sz  ;
    Ovl1->Nx = Sx + dx * dz2/(dz1+dz2) ;
    Ovl1->Ny = Sy + dy * dz2/(dz1+dz2) ;
    Ovl1->Nz = Sz + dz * dz2/(dz1+dz2) ;
   }
 if( Flag == 3 )
   {
/*
** Truncate Radial
*/
    if( Ovl2->Px != Ovl2->Nx || Ovl2->Py != Ovl2->Ny ) bcdtmMath_calculatePlaneCoefficients(Ovl2->Px,Ovl2->Py,Ovl2->Pz,Ovl2->Nx,Ovl2->Ny,Ovl2->Nz,Ovl3->Nx,Ovl3->Ny,Ovl3->Nz,&Ca,&Cb,&Cc,&Cd) ;
    else                                               bcdtmMath_calculatePlaneCoefficients(Ovl2->Px,Ovl2->Py,Ovl2->Pz,Ovl3->Px,Ovl3->Py,Ovl3->Pz,Ovl3->Nx,Ovl3->Ny,Ovl3->Nz,&Ca,&Cb,&Cc,&Cd) ;
    if( Cc == 0.0 ) Cc = 0.000000001 ;
    Zn = - ( Ca * Ovl1->Nx + Cb * Ovl1->Ny + Cd ) / Cc ;
    Zs = - ( Ca * Ovl1->Px + Cb * Ovl1->Py + Cd ) / Cc ;
    dz1 = fabs(Zn-Ovl1->Nz) ;
    dz2 = fabs(Zs-Ovl1->Pz) ;
    dx = Ovl1->Nx - Ovl1->Px  ;
    dy = Ovl1->Ny - Ovl1->Py  ;
    dz = Ovl1->Nz - Ovl1->Pz  ;
    Ovl1->Nx = Ovl1->Px + dx * dz2/(dz1+dz2) ;
    Ovl1->Ny = Ovl1->Py + dy * dz2/(dz1+dz2) ;
    Ovl1->Nz = Ovl1->Pz + dz * dz2/(dz1+dz2) ;
   }

/*
**  Added   12/March/04  RobC
*/
 if( Flag == 4 )
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Ovl1 = %6ld  Truncating Radial = %6ld",(long)(Ovl1-OvlPts),Ovl1->TruncatingRadial) ;
       bcdtmWrite_message(0,0,0,"Ovl2 = %6ld  Truncating Radial = %6ld",(long)(Ovl2-OvlPts),Ovl2->TruncatingRadial) ;
       bcdtmWrite_message(0,0,0,"Ovl3 = %6ld  Truncating Radial = %6ld",(long)(Ovl3-OvlPts),Ovl3->TruncatingRadial) ;
       d1 = bcdtmMath_normalDistanceToCordLine(Ovl2->Px,Ovl2->Py,Ovl2->Nx,Ovl2->Ny,Ovl1->Nx,Ovl1->Ny) ;
       d2 = bcdtmMath_normalDistanceToCordLine(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny,Ovl3->Nx,Ovl3->Ny) ;
       bcdtmWrite_message(0,0,0,"Distance Ovl1->Ovl2 = %10.4lf",d1) ;
       bcdtmWrite_message(0,0,0,"Distance Ovl3->Ovl1 = %10.4lf",d2) ;
       if( d1 > 0.0001 ) bcdtmWrite_message(0,0,0,"DTM_ERROR D1") ;
       if( d2 > 0.0001 ) bcdtmWrite_message(0,0,0,"DTM_ERROR D2") ;
      }
/*
**  Calculate Plane Coefficients
*/
    bcdtmMath_calculatePlaneCoefficients(Ovl2->Px,Ovl2->Py,Ovl2->Pz,Ovl2->Nx,Ovl2->Ny,Ovl2->Nz,Ovl3->Nx,Ovl3->Ny,Ovl3->Nz,&Ca,&Cb,&Cc,&Cd) ;
    if( Cc == 0.0 ) Cc = 0.000000001 ;
/*
**  Calculate z at Point[Ovl3->Nx,Ovl3->Ny] On Ovl1
*/
    d1 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl3->Nx,Ovl3->Ny) ;
    d2 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny) ;
    if( dbg )
      {
       if( d1 >= d2 ) bcdtmWrite_message(0,0,0,"00 DTM_ERROR") ;
       bcdtmWrite_message(0,0,0,"00 d1 = %10.6lf d2 = %10.6lf ** Nx = %10.4lf %10.4lf %10.4lf",d1,d2,Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
      }
    dz = Ovl1->Nz - Ovl1->Pz  ;
    z  = Ovl1->Pz + dz * d1 / d2 ;
/*
**  Calculate z On Plane
*/
    Zn = - ( Ca * Ovl1->Nx + Cb * Ovl1->Ny + Cd ) / Cc ;
    Zs = - ( Ca * Ovl3->Nx + Cb * Ovl3->Ny + Cd ) / Cc ;
    dz1 = fabs(Zs-z) ;
    dz2 = fabs(Zn-Ovl1->Nz) ;
/*
** Truncate Radial
*/
    dx = Ovl1->Nx - Ovl3->Nx  ;
    dy = Ovl1->Ny - Ovl3->Ny  ;
    Ovl1->Nx = Ovl3->Nx + dx * dz1/(dz1+dz2) ;
    Ovl1->Ny = Ovl3->Ny + dy * dz1/(dz1+dz2) ;
    d1 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny) ;
    if( dbg )
      {
       if( d1 >= d2 ) bcdtmWrite_message(0,0,0,"01 DTM_ERROR") ;
       bcdtmWrite_message(0,0,0,"01 d1 = %10.6lf d2 = %10.6lf ** Nx = %10.4lf %10.4lf %10.4lf",d1,d2,Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
      }
    Ovl1->Nz = Ovl1->Pz + dz * d1 / d2 ;
   }
/*
**  Added 15/March/04 - RobC For Radials Intersecting Base Lines
*/
 if( Flag == 5 )
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] **[P] %12.5lf %12.5lf %10.4lf **[N] %12.5lf %12.5lf %10.4lf",(long)(Ovl1-OvlPts),Ovl1->Px,Ovl1->Py,Ovl1->Pz,Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
       bcdtmWrite_message(0,0,0," Base1[%6ld] **[P] %12.5lf %12.5lf %10.4lf **[N] %12.5lf %12.5lf %10.4lf",(long)(Ovl2-OvlPts),Ovl2->Px,Ovl2->Py,Ovl2->Pz,Ovl2->Nx,Ovl2->Ny,Ovl2->Nz) ;
       bcdtmWrite_message(0,0,0," Base2[%6ld] **[P] %12.5lf %12.5lf %10.4lf **[N] %12.5lf %12.5lf %10.4lf",(long)(Ovl3-OvlPts),Ovl3->Px,Ovl3->Py,Ovl3->Pz,Ovl3->Nx,Ovl3->Ny,Ovl3->Nz) ;
      }
/*
** Get Intersection Point
*/
    bcdtmMath_normalIntersectCordLines(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny,Ovl2->Nx,Ovl2->Ny,Ovl3->Nx,Ovl3->Ny,&Bx,&By) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Point = %12.5lf %12.5lf",Bx,By) ;
/*
** Calculate z Of Intersection Point On Base Line
*/
    d1 = bcdtmMath_distance(Ovl2->Nx,Ovl2->Ny,Bx,By) ;
    d2 = bcdtmMath_distance(Ovl2->Nx,Ovl2->Ny,Ovl3->Nx,Ovl3->Ny) ;
    dz = Ovl3->Nz - Ovl2->Nz  ;
    Bz = Ovl2->Nz + dz * d1 / d2 ;
/*
** Calculate z Of Intersection Point On Radial
*/
    d1 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Bx,By) ;
    d2 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny) ;
    dz = Ovl1->Nz - Ovl1->Pz  ;
    Rz = Ovl1->Pz + dz * d1 / d2 ;
/*
**  Calculate Plane Coefficients
*/
    bcdtmMath_calculatePlaneCoefficients(Ovl2->Nx,Ovl2->Ny,Ovl2->Nz,Ovl3->Nx,Ovl3->Ny,Ovl3->Nz,(Ovl2->Px+Ovl3->Px)/2.0,(Ovl2->Py+Ovl3->Py)/2.0,(Ovl2->Pz+Ovl3->Pz)/2.0,&Ca,&Cb,&Cc,&Cd) ;
    if( Cc == 0.0 ) Cc = 0.000000001 ;
/*
**  Calculate z Of Radial End Point On Plane
*/
    Zn = - ( Ca * Ovl1->Nx + Cb * Ovl1->Ny + Cd ) / Cc ;
/*
**  Calculate Elevation Differences Of Intersection Point And Radial End Point
*/
    dz1 = Bz - Rz ;
    dz2 = Zn - Ovl1->Nz ;
/*
**  Calculate Point Where Radial Intesects Plane
*/
    if( ( dz1 < 0.0 && dz2 > 0.0 ) || ( dz1 > 0.0 && dz2 < 0.0 ) )
      {
       dz1 = fabs(dz1) ;
       dz2 = fabs(dz2) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial Intersects Plane")  ;
       dx = Ovl1->Nx - Bx ;
       dy = Ovl1->Ny - By ;
       Ovl1->Nx = Bx + dx * dz1/(dz1+dz2) ;
       Ovl1->Ny = By + dy * dz1/(dz1+dz2) ;
       d1 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny) ;
       Ovl1->Nz = Ovl1->Pz + dz * d1 / d2 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"End Point = %12.5lf %12.5lf %10.4lf",Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial Does Not Intersect Plane")  ;
       Ovl1->Nx = Bx  ;
       Ovl1->Ny = By  ;
       Ovl1->Nz = Bz  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"End Point = %12.5lf %12.5lf %10.4lf",Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
      }
   }
/*
** Check Truncation Is Correct
*/
 if( Ovl1->Nx < Xmin || Ovl1->Nx > Xmax || Ovl1->Ny < Ymin || Ovl1->Ny > Ymax     )
   {
    d1 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Ovl1->Nx,Ovl1->Ny) ;
    d2 = bcdtmMath_distance(Ovl1->Px,Ovl1->Py,Snx,Sny) ;
    if( fabs(d1-d2) > 0.00001 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"d1 = %12.5lf d2 = %12.5lf",d1,d2) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Truncation Error %10.4lf %10.4lf %10.4lf",Ovl1->Nx,Ovl1->Ny,Ovl1->Nz) ;
      }
    Ovl1->Nx = Snx ;
    Ovl1->Ny = Sny ;
    Ovl1->Nz = Snz ;
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines               |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_intersectSideSlopeRadialsWithBaseLines(long sideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Detects Intersections Of Radials With Base Lines
*/
{
 int     ret=0 ;
 long    dbg=0,PadType,IntTableNe,IntPtsNe,IntPtsMe,IntPtsMinc ;
 double  Nx,Ny,Nz ;
 DPoint3d     p3dPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl,*base1,*base2 ;
 DTM_STR_INT_TAB *pint,*IntTable=nullptr ;
 DTM_STR_INT_PTS *pinp,*IntPts=nullptr ;
 BC_DTM_OBJ *Data=nullptr ;
 long  loop=0;
 bool onlyLoopOnce=true ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Radials With Base Lines") ;
/*
** Determine Pad Type
*/
 PadType = 0 ;
 if( sideSlopeElementType == 1 ) PadType = 1 ;
/*
** Write Radials - Development Only
*/
 if( dbg == 2 )
   {
    if( PadType == 1 )  bcdtmWrite_message(0,0,0,"Base Line Type = Open Element") ;
    else                bcdtmWrite_message(0,0,0,"Base Line Type = Closed Element") ;
    for( ovl = OvlPts  ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%4ld] T = %1ld S = %1ld T = %9ld [P] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Status,ovl->TruncatingRadial,ovl->Px,ovl->Py,ovl->Pz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Write Radial Truncations - Development Only
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Radials = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** %10.4lf %10.4lf %10.4lf ** %10.4lf TR = %9ld",(long)(ovl-OvlPts),ovl->Px,ovl->Py,ovl->Pz,ovl->Nz,ovl->TruncatingRadial) ;
      }
   }
/*
** Build Radial Base Line Intersection Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table") ;
 if( bcdtmSideSlope_buildRadialBaseLineIntersectionTable(sideSlopeElementType,OvlPts,NumOvlPts,&IntTable,&IntTableNe) )  goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Radial Base Line Intersection Entries = %6ld",IntTableNe ) ;
/*
** Write Base Lines To Data File For Checking Purposes
*/
 if( dbg == 2 )
   {
    if( bcdtmObject_createDtmObject(&Data) ) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(Data,IntTableNe,IntTableNe) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       if( pint->Type == 1 )
         {
          p3dPts[0].x = pint->X1 ;
          p3dPts[0].y = pint->Y1 ;
          p3dPts[0].z = pint->Z1 ;
          p3dPts[1].x = pint->X2 ;
          p3dPts[1].y = pint->Y2 ;
          p3dPts[1].z = pint->Z2 ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
         }
      }
    bcdtmWrite_geopakDatFileFromDtmObject(Data,L"BaseLines.dat") ;
    bcdtmObject_destroyDtmObject(&Data) ;
   }
/*
** Write Intersection Table ** Development Only
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radial Base Line Intersection Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** String = %4ld Sergment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf ** %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->X2,pint->Y2) ;
      }
   }
/*
** Scan Point Table And Look For Intersections
*/
 IntPtsMinc = IntTableNe / 10 ;
 if( IntPtsMinc < 1000 ) IntPtsMinc = 1000 ;
 IntPtsNe = IntPtsMe = 0 ;
/*
** Scan For Intersections
*/
 IntPtsNe = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ;
 if( bcdtmSideSlope_scanForRadialBaseLineIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
    for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
      {
       ovl   = OvlPts + pinp->String1 ;
       base1 = OvlPts + pinp->String2 ;
       base2 = OvlPts + pinp->Segment2 ;
       bcdtmWrite_message(0,0,0,"Radial[%6ld] base1 = %6ld base2 = %6ld ** base1->tRadial = %9ld base2->tRadial = %9ld",pinp->String1,pinp->String2,pinp->Segment2,base1->TruncatingRadial,base2->TruncatingRadial) ;
      }
   }
/*
** Process Intersections
*/
 loop = 0 ;
 while( IntPtsNe > 0 && loop < 1 )
   {
/*
**  Sort Intersection Points Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Points Table") ;
    qsort(IntPts,IntPtsNe,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
/*
** Write Intersections
*/
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
       for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
         {
          bcdtmWrite_message(0,0,0,"Intersection Point[%4ld] ** Radial = %6ld BaseOfs1 = %6ld BaseOfs2 = %6ld Distance = %10.4lf x = %10.4lf y = %10.4lf z = %10.4lf",(long)(pinp-IntPts),pinp->String1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z) ;
         }
      }
/*
** Truncate Intersected Radials
*/
    for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
      {
       ovl   = OvlPts + pinp->String1 ;
       base1 = OvlPts + pinp->String2 ;
       base2 = OvlPts + pinp->Segment2 ;
       Nx = ovl->Nx ;
       Ny = ovl->Ny ;
       Nz = ovl->Nz ;
       if( dbg )  bcdtmWrite_message(0,0,0,"Before Truncating Radial[%4ld] T = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
//       bcdtmSideSlope_truncateSideSlopeRadial(OvlPts,ovl,base1,base2,3) ;
       if( pinp->Distance > 0.001 ) bcdtmSideSlope_truncateSideSlopeRadial(OvlPts,ovl,base1,base2,5) ;
       if( Nx != ovl->Nx || Ny != ovl->Ny ) ovl->TruncatingRadial = DTM_NULL_PNT ;
       pinp->x = ovl->Nx ;
       pinp->y = ovl->Ny ;
       pinp->z = ovl->Nz ;
       ovl->Nx = Nx ;
       ovl->Ny = Ny ;
       ovl->Nz = Nz ;
       if( dbg )  bcdtmWrite_message(0,0,0,"After  Truncating Radial[%4ld] T = %2ld [P] %10.4lf %10.4lf %10.4lf [G] %10.4lf %10.4lf %10.4lf [N] %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
/*
** Modify Radials
*/
    for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
      {
       ovl   = OvlPts + pinp->String1 ;
       ovl->Nx = pinp->x ;
       ovl->Ny = pinp->y ;
       ovl->Nz = pinp->z ;
      }
/*
**
** Not Sure Whether More Than One Loop Is Necessary
** Currently Will Limit The Loops To One. The Old cde had two loops. May have been necessary for some prior reason.
** Will come back to This
**
*/
 if( onlyLoopOnce == true ) goto endloop ;
/*
** Build Radial Base Line Intersection Tables
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table") ;
    if( bcdtmSideSlope_buildRadialBaseLineIntersectionTable(sideSlopeElementType,OvlPts,NumOvlPts,&IntTable,&IntTableNe) )  goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Radial Base Line Intersection Entries = %6ld",IntTableNe ) ;
/*
**  Sort Intersection Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(IntTable,IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Scan For Intersections
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ;
    IntPtsMinc = IntPtsNe ;
    IntPtsNe = 0 ;
    if( bcdtmSideSlope_scanForRadialBaseLineIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
/*
** Set For Rescan
*/
endloop :
    ++loop ;
   }
/*
**  Write Number Of Loops
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Loops = %6ld",loop) ;
/*
** Clean Up
*/
 cleanup :
 if( IntTable != nullptr )  free(IntTable) ;
 if( IntPts   != nullptr )  free(IntPts) ;
 if( Data     != nullptr )  bcdtmObject_destroyDtmObject(&Data) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Intersecting Radials With Base Lines Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Intersecting Radials With Base Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmSideSlope_buildRadialBaseLineIntersectionTable               |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_buildRadialBaseLineIntersectionTable(long SideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_TAB **IntTable,long *IntTableNe)
{
 int    ret=0 ;
 long   dbg=0,ofs,IntTableMe,IntTableMinc  ;
 double cord ;
 DTM_OVERLAP_RADIAL_TABLE *ovl1,*ovl2,*trovl2,*bovl1,*bovl2 ;
 DTM_STR_INT_TAB *prb ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table") ;
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ;*IntTable = nullptr ; }
 IntTableMinc = NumOvlPts * 3 ;
/*
** Write Radials To Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( ovl1 = OvlPts , ofs = 0 ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 , ++ofs )
   {
    ovl2 = nullptr ;
    if( ovl1->TruncatingRadial != DTM_NULL_PNT )
      {
       ovl2 = OvlPts + ovl1->TruncatingRadial ;
       if( ovl2->TruncatingRadial != DTM_NULL_PNT ) ovl2 = OvlPts + ovl2->TruncatingRadial ;
      }
    if( ovl2 != ovl1 )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Storing Radial = %6ld",(long)(ovl1-OvlPts)) ;
       if( bcdtmSideSlope_storeRadialBaseLineInRadialBaseLineIntersectionTable(ofs,ofs,2,1,ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nx,ovl1->Ny,ovl1->Nz,IntTable,IntTableNe,&IntTableMe,IntTableMinc) ) goto errexit ;
      }
   }
/*
** Write Base Lines To Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Base Lines") ;
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts - 1 ; ++ovl1 )
   {
    ovl2 = ovl1 + 1   ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ovl1->TruncatingRadial = %9ld ovl2->TruncatingRadial = %9ld",ovl1->TruncatingRadial,ovl2->TruncatingRadial) ;
/*
**  Check Radials Are Not Truncated By The Same Radial
*/
    if( ovl1->TruncatingRadial != ovl2->TruncatingRadial || ovl1->TruncatingRadial == DTM_NULL_PNT || ovl2->TruncatingRadial == DTM_NULL_PNT )
      {
/*
**  Check If Either Radial Is Truncated By A Type 1 Radial ( Concave Corner )
**  If So Switch That Radial To The Type 1 Truncating Radial
*/
       bovl1 = ovl1 ;
       bovl2 = ovl2 ;
       if( ovl1->TruncatingRadial != DTM_NULL_PNT  )
         {
          if( (OvlPts+ovl1->TruncatingRadial)->Type == 1 ) bovl1 = OvlPts+ovl1->TruncatingRadial ;
         }
       if( ovl2->TruncatingRadial != DTM_NULL_PNT  )
         {
          if( (OvlPts+ovl2->TruncatingRadial)->Type == 1 ) bovl2 = OvlPts+ovl2->TruncatingRadial ;
         }
/*
**  Store Base Line
*/
       if( bovl1 == ovl1 && bovl2 == ovl2 )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Storing Base Line ** Radial1 = %6ld Radial2 = %6ld",(long)(bovl1-OvlPts),(long)(bovl2-OvlPts)) ;
          if( bcdtmSideSlope_storeRadialBaseLineInRadialBaseLineIntersectionTable((long)(bovl1-OvlPts),(long)(bovl2-OvlPts),1,1,bovl1->Nx,bovl1->Ny,bovl1->Nz,bovl2->Nx,bovl2->Ny,bovl2->Nz,IntTable,IntTableNe,&IntTableMe,IntTableMinc ) ) goto errexit ;
         }
      }
   }
/*
**  Write Next Base Line From Non Truncated Type One Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Base Lines From Type 1 Radials") ;
 for( ovl1 = OvlPts ;  ovl1 < OvlPts + NumOvlPts - 1 ; ++ovl1 )
   {
    if( ovl1->Type == 1 && ovl1->TruncatingRadial == DTM_NULL_PNT )
      {
/*
** Scan Forward To Next Base Line Point
*/
       ovl2 = ovl1 + 1 ;
       if( ovl2 >= OvlPts + NumOvlPts )  ovl2 = OvlPts ;
       trovl2 = nullptr ;
       if( ovl2->TruncatingRadial != DTM_NULL_PNT ) trovl2 = OvlPts + ovl2->TruncatingRadial ;
       while ( trovl2 == ovl1 )
         {
          ovl2 = ovl2 + 1 ;
          if( ovl2 >= OvlPts + NumOvlPts )
            {
             if( SideSlopeElementType == 2 )  ovl2 = OvlPts ;
             else                             ovl2 = trovl2 = nullptr ;
            }
          if( ovl2 != nullptr )
            {
             if( ovl2->TruncatingRadial != DTM_NULL_PNT ) trovl2 = OvlPts + ovl2->TruncatingRadial ;
             else                                   trovl2 = nullptr ;
            }
         }
/*
**  Store Base Line
*/
       if( ovl2 != nullptr )
         {
          if( trovl2 == nullptr || ( trovl2 != nullptr && trovl2->Type != 1 ) )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Base Line Radial1 = %6ld Radial2 = %6ld",(long)(ovl1-OvlPts),(long)(ovl2-OvlPts)) ;
             if( bcdtmSideSlope_storeRadialBaseLineInRadialBaseLineIntersectionTable((long)(ovl1-OvlPts),(long)(ovl2-OvlPts),1,1,ovl1->Nx,ovl1->Ny,ovl1->Nz,ovl2->Nx,ovl2->Ny,ovl2->Nz,IntTable,IntTableNe,&IntTableMe,IntTableMinc ) ) goto errexit ;
            }
         }
      }
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( prb = *IntTable ; prb < *IntTable + *IntTableNe ; ++prb )
   {
    if( prb->X1 > prb->X2 || ( prb->X1 == prb->X2 && prb->Y1 > prb->Y2 ) )
      {
       prb->Direction = 2 ;
       cord = prb->X1 ; prb->X1 = prb->X2 ; prb->X2 = cord ;
       cord = prb->Y1 ; prb->Y1 = prb->Y2 ; prb->Y2 = cord ;
       cord = prb->Z1 ; prb->Z1 = prb->Z2 ; prb->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmSideSlope_storeRadialBaseLineInRadialBaseLineIntersectionTable                |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_storeRadialBaseLineInRadialBaseLineIntersectionTable(long Ofs1,long Ofs2,long Type,long Direction,double X1,double Y1,double Z1,double X2,double Y2,double Z2, DTM_STR_INT_TAB **IntTable,long *IntTableNe,long *IntTableMe,long IntTableMinc )
{
 int dbg=0 ;
/*
**  Check For Memory Allocation
*/
 if( *IntTableNe == *IntTableMe )
   {
    *IntTableMe = *IntTableMe + IntTableMinc ;
    if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( *IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,*IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
   }
/*
**  Store Base Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Ofs1 = %4ld Ofs2 = %4ld  Type = %2ld ** %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf",Ofs1,Ofs2,Type,X1,Y1,Z1,X2,Y2,Z2) ;
 (*IntTable+*IntTableNe)->String  = Ofs1 ;
 (*IntTable+*IntTableNe)->Segment = Ofs2 ;
 (*IntTable+*IntTableNe)->Type = Type   ;
 (*IntTable+*IntTableNe)->Direction = Direction ;
 (*IntTable+*IntTableNe)->X1 = X1 ;
 (*IntTable+*IntTableNe)->Y1 = Y1 ;
 (*IntTable+*IntTableNe)->Z1 = Z1 ;
 (*IntTable+*IntTableNe)->X2 = X2 ;
 (*IntTable+*IntTableNe)->Y2 = Y2 ;
 (*IntTable+*IntTableNe)->Z2 = Z2 ;
  ++*IntTableNe ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForRadialBaseLineIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Base Line Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineActiveRadialBaseLineIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_determineActiveRadialBaseLineIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y ;
 DTM_STR_INT_TAB  *alp,*slp,*rlp,*blp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List Only If Last Line Is Of Type Radial
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
** Only Test Radial Against A Base line
*/
    if( ( alp->Type == 2 && slp->Type == 1 ) || ( alp->Type == 1 && slp->Type == 2 ) )
      {
/*
** Set Radial And Base Pointers
*/
       if( alp->Type == 2 ) { rlp = alp ; blp = slp ; }
       else                 { rlp = slp ; blp = alp ; }
       if( rlp->Direction == 1 ) { Xs = rlp->X1 ; Ys = rlp->Y1 ; Zs = rlp->Z1 ; Xe = rlp->X2 ; Ye = rlp->Y2 ; Ze = rlp->Z2 ; }
       if( rlp->Direction == 2 ) { Xs = rlp->X2 ; Ys = rlp->Y2 ; Zs = rlp->Z2 ; Xe = rlp->X1 ; Ye = rlp->Y1 ; Ze = rlp->Z1 ; }
/*
** Check That End Of Radial Is Not Coincident Either End Of Base Line
*/
       if( rlp->String != blp->String && rlp->String != blp->Segment )
         {
/*
** Check If Lines Intersect
*/
          if( bcdtmMath_checkIfLinesIntersect(rlp->X1,rlp->Y1,rlp->X2,rlp->Y2,blp->X1,blp->Y1,blp->X2,blp->Y2))
            {
/*
** Intersect Lines
*/
             bcdtmMath_normalIntersectCordLines(rlp->X1,rlp->Y1,rlp->X2,rlp->Y2,blp->X1,blp->Y1,blp->X2,blp->Y2,&x,&y) ;
/*
** Check Memory
*/
             if( *IntPtsNe == *IntPtsMe )
               {
                *IntPtsMe = *IntPtsMe + IntPtsMinc ;
                if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                if( *IntPts == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
               }
/*
** Calculate Distances
*/
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point
*/
             (*IntPts+*IntPtsNe)->String1  = rlp->String  ;
             (*IntPts+*IntPtsNe)->Segment1 = rlp->Segment ;
             (*IntPts+*IntPtsNe)->String2  = blp->String  ;
             (*IntPts+*IntPtsNe)->Segment2 = blp->Segment ;
             (*IntPts+*IntPtsNe)->Distance = di ;
             (*IntPts+*IntPtsNe)->x = x ;
             (*IntPts+*IntPtsNe)->y = y ;
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
             ++*IntPtsNe ;
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmSideSlope_setElevationOfIntersectedSideSlopeRadials           |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_setElevationOfIntersectedSideSlopeRadials(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Sets The Elevation Of Intersected Radials To That Of The Intersecting Radial
*/
{
 int  dbg=0 ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl1,*ovl2 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Elevations Of Intersected Radials") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Radials = %6ld",NumOvlPts) ;
    for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** %10.4lf %10.4lf %10.4lf ** %10.4lf TR = %9ld",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nz,ovl1->TruncatingRadial) ;
      }
   }
/*
**  Copy Truncating Radials
*/
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 ) ovl1->TruncatingEdge = ovl1->TruncatingRadial ;
/*
** Save Intersecting Radials For Resolving Boundary Polygons and Holes
** Modified 13/6/2002 Rob Cormack
*/
/*
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 ) ovl1->Rad = ovl1->TruncatingRadial ;
*/
/*
** Scan Radials And Truncate Intersected Radials
*/
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
   {
    if( ovl1->TruncatingRadial != DTM_NULL_PNT )
      {
           ovl2 = OvlPts + ovl1->TruncatingRadial ;
           bcdtmSideSlope_setIntersectedSideSlopeRadialElevation(OvlPts,ovl1,ovl2) ;
      }
   }
/*
** Write Out Radials
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Radials = %6ld",NumOvlPts) ;
    for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** %10.4lf %10.4lf %10.4lf ** %10.4lf Truncating Radial = %9ld",(long)(ovl1-OvlPts),ovl1->Px,ovl1->Py,ovl1->Pz,ovl1->Nz,ovl1->TruncatingRadial) ;
      }
   }
/*
**  Copy Truncating Radials
*/
 for( ovl1 = OvlPts ; ovl1 < OvlPts + NumOvlPts ; ++ovl1 ) ovl1->TruncatingRadial = ovl1->TruncatingEdge ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Elevations Of Intersected Radials Completed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmSideSlope_setIntersectedSideSlopeRadialElevation              |
|                                                                    |
+-------------------------------------------------------------------*/
void bcdtmSideSlope_setIntersectedSideSlopeRadialElevation(DTM_OVERLAP_RADIAL_TABLE *Ovl,DTM_OVERLAP_RADIAL_TABLE *Ovl1,DTM_OVERLAP_RADIAL_TABLE *Ovl2)
{
 int    dbg=0 ;
 double d1,d2  ;
 DTM_OVERLAP_RADIAL_TABLE *Ovl3=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"ovl1 = %4ld ovl2 = %4ld",(long)(Ovl1-Ovl),(long)(Ovl2-Ovl)) ;
/*
** If Ovl2 Is Intersected Set Intersected Elevation For Radial 2
*/
 if( Ovl2->TruncatingRadial != DTM_NULL_PNT && Ovl2->TruncatingRadial >= 0 )
   {
    Ovl3 = Ovl + Ovl2->TruncatingRadial ;
    Ovl2->TruncatingRadial = - (Ovl2->TruncatingRadial+1) ;
    if( Ovl3 != Ovl1 ) bcdtmSideSlope_setIntersectedSideSlopeRadialElevation(Ovl,Ovl2,Ovl3) ;
   }
/*
**  Set Intersected Elevation For Ovl1
*/
 if( Ovl2->TruncatingRadial < 0 ) Ovl2->TruncatingRadial = -(Ovl2->TruncatingRadial+1) ;
 if( Ovl3 == Ovl1 ) { Ovl1->TruncatingRadial = Ovl2->TruncatingRadial = DTM_NULL_PNT ; }
 else
   {
    d1 = bcdtmMath_distance(Ovl2->Px,Ovl2->Py,Ovl1->Nx,Ovl1->Ny) ;
    d2 = bcdtmMath_distance(Ovl2->Px,Ovl2->Py,Ovl2->Nx,Ovl2->Ny) ;
    if( d1 < d2 ) Ovl1->Nz = Ovl2->Pz + (Ovl2->Nz - Ovl2->Pz) * d1 / d2 ;
    else if( dbg && fabs(d1-d2) > 0.00001 )
      {
       bcdtmWrite_message(0,0,0,"Elevation Setting Error ** Ovl1 = %6ld Ovl2 = %6ld",(long)(Ovl1-Ovl),(long)(Ovl2-Ovl)) ;
       bcdtmWrite_message(0,0,0,"D1 = %12.5lf  D2 = %12.5lf",d1,d2) ;
      }
    Ovl1->TruncatingRadial = DTM_NULL_PNT ;
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_writeSideSlopeRadialsToDataObject                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writeSideSlopeRadialsToDataObject(long SideSlopeElementType,long SideDirection,DTM_OVERLAP_RADIAL_TABLE *RghtRadials,long NumRghtRadials,DTM_OVERLAP_RADIAL_TABLE *LeftRadials,long NumLeftRadials,DTMUserTag UserRadialTag,DTMUserTag UserElementTag,BC_DTM_OBJ *SideSlopes)
/*
** This Function Writes The Side Slopes To The Data Object
*/
{
 int   ret=DTM_SUCCESS ;
 long  padType ;
 DPoint3d   p3dPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE *ovl,*ovn ;
/*
** Initialise
*/
 if( SideSlopeElementType == 1 ) padType = 1 ;
 else                            padType = 0 ;
/*
** Process Right Side Or External Side Slopes
*/
 if( SideDirection == 1 || SideDirection == 3 )
   {
    for( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials - padType ; ++ovl )
      {
       ovn = ovl + 1 ;
       if( ovn == RghtRadials + NumRghtRadials ) ovn = RghtRadials ;
       if( ovl->Px != ovn->Px || ovl->Py != ovn->Py )
         {
          p3dPts[0].x = ovl->Px ;
          p3dPts[0].y = ovl->Py ;
          p3dPts[0].z = ovl->Pz ;
          p3dPts[1].x = ovn->Px ;
          p3dPts[1].y = ovn->Py ;
          p3dPts[1].z = ovn->Pz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Breakline,UserElementTag,1,&SideSlopes->nullFeatureId,p3dPts,2)) goto errexit ;
         }
      }
    for( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials ; ++ovl )
      {
       p3dPts[0].x = ovl->Px ;
       p3dPts[0].y = ovl->Py ;
       p3dPts[0].z = ovl->Pz ;
       p3dPts[1].x = ovl->Nx ;
       p3dPts[1].y = ovl->Ny ;
       p3dPts[1].z = ovl->Nz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Breakline,UserRadialTag,1,&SideSlopes->nullFeatureId,p3dPts,2)) goto errexit ;
      }
   }
/*
** Process Left Side Or Internal Side Slopes
*/
 if( SideDirection == 2 || SideDirection == 3 )
   {
    for( ovl = LeftRadials ; ovl < LeftRadials + NumLeftRadials - padType  ; ++ovl )
      {
       ovn = ovl + 1 ;
       if( ovn == LeftRadials + NumLeftRadials ) ovn = LeftRadials ;
       if( ovl->Px != ovn->Px || ovl->Py != ovn->Py )
         {
          p3dPts[0].x = ovl->Px ;
          p3dPts[0].y = ovl->Py ;
          p3dPts[0].z = ovl->Pz ;
          p3dPts[1].x = ovn->Px ;
          p3dPts[1].y = ovn->Py ;
          p3dPts[1].z = ovn->Pz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Breakline,UserElementTag,1,&SideSlopes->nullFeatureId,p3dPts,2)) goto errexit ;
         }
      }
    for( ovl = LeftRadials ; ovl < LeftRadials + NumLeftRadials ; ++ovl )
      {
       p3dPts[0].x = ovl->Px ;
       p3dPts[0].y = ovl->Py ;
       p3dPts[0].z = ovl->Pz ;
       p3dPts[1].x = ovl->Nx ;
       p3dPts[1].y = ovl->Ny ;
       p3dPts[1].z = ovl->Nz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Breakline,UserRadialTag,1,&SideSlopes->nullFeatureId,p3dPts,2)) goto errexit ;
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
int bcdtmSideSlope_writeOpenSideSlopeElementBoundaryPolygonToDataObject
(
 DTM_OVERLAP_RADIAL_TABLE *RghtRadials,
 long NumRghtRadials,
 DTM_OVERLAP_RADIAL_TABLE *LeftRadials,
 long NumLeftRadials,
 long SideDirection,
 BC_DTM_OBJ *SideSlopes,
 double ppTol,
 double plTol
)
/*
** This Function Writes The Boundary Polygon To The Data Object
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    sp,np,numHullPts,LeftTruncatedRadials,RightTruncatedRadials ;
 DPoint3d     p3dPts[2],*hullPtsP=nullptr ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl,*ovn ;
 BC_DTM_OBJ *Data=nullptr,*openRightP=nullptr,*openLeftP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Open Element Boundary Polygon To Data Object") ;
/*
** Write Out Radials For Debugging Purposes
*/
 if( dbg ) { if( SideDirection == 1 || SideDirection == 3 ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(RghtRadials,NumRghtRadials,L"openElmRadialsRight.dat") ; }
 if( dbg ) { if( SideDirection == 2 || SideDirection == 3 ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(LeftRadials,NumLeftRadials,L"openElmRadialsleft.dat") ; }
/*
** Check For Right Truncated Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Truncated Radials") ;
 RightTruncatedRadials = 0 ;
 if( SideDirection == 1 || SideDirection == 3 )
 for( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials ; ++ovl )
   {
    if( ! ovl->Status ) ++RightTruncatedRadials ;
   }
/*
** Check For Left Truncated Radials
*/
 LeftTruncatedRadials = 0 ;
 if( SideDirection == 2 || SideDirection == 3 )
 for( ovl = LeftRadials ; ovl < LeftRadials + NumLeftRadials ; ++ovl )
   {
    if( ! ovl->Status ) ++LeftTruncatedRadials ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Side Slope Direction = %2ld RightTruncatedRadials = %4ld LeftTruncatedRadials = %4ld",SideDirection,RightTruncatedRadials,LeftTruncatedRadials) ;
/*
**  Create Data Object For Toe Slopes
*/
 if( bcdtmObject_createDtmObject(&Data) ) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(Data,NumRghtRadials+NumLeftRadials*2,1000) ;
/*
**  Write Right Toe Slopes
*/
 if( SideDirection == 1 || SideDirection == 3 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Right Slope Toes") ;
    if( ! RightTruncatedRadials )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Writing None Truncated Right Slope Toes") ;
       for( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials - 1 ; ++ovl )
         {
          ovn = ovl + 1 ;
          p3dPts[0].x = ovl->Nx ;
          p3dPts[0].y = ovl->Ny ;
          p3dPts[0].z = ovl->Nz ;
          p3dPts[1].x = ovn->Nx ;
          p3dPts[1].y = ovn->Ny ;
          p3dPts[1].z = ovn->Nz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
         }
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Writing Truncated Right Slope Toes") ;
       if( ! processingLimits ) { if( bcdtmSideSlope_writeSlopeToesToDataObject(1,Data,RghtRadials,NumRghtRadials)) goto errexit ; }
       else                     { if( bcdtmSideSlope_writeLimitSlopeToesToDataObject(1,Data,RghtRadials,NumRghtRadials)) goto errexit ; }
      }
    if( bcdtmObject_cloneDtmObject(Data,&openRightP)) goto errexit ;
    if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(Data,L"openElmRightSlopeToes.dat") ;
   }
/*
**  Write Left Toe Slopes
*/
 if( SideDirection == 2 || SideDirection == 3 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Left Slope Toes") ;
    bcdtmSideSlope_reverseOrderOfSideSlopeRadials(LeftRadials,NumLeftRadials) ;
    if( ! LeftTruncatedRadials )
      {
       for( ovl = LeftRadials ; ovl < LeftRadials + NumLeftRadials - 1 ; ++ovl )
         {
          ovn = ovl + 1 ;
          p3dPts[0].x = ovl->Nx ;
          p3dPts[0].y = ovl->Ny ;
          p3dPts[0].z = ovl->Nz ;
          p3dPts[1].x = ovn->Nx ;
          p3dPts[1].y = ovn->Ny ;
          p3dPts[1].z = ovn->Nz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
         }
      }
    else
      {
       if( ! processingLimits ) { if( bcdtmSideSlope_writeSlopeToesToDataObject(1,Data,LeftRadials,NumLeftRadials)) goto errexit ; }
       else                     { if( bcdtmSideSlope_writeLimitSlopeToesToDataObject(1,Data,LeftRadials,NumLeftRadials)) goto errexit ; }
      }
    if( bcdtmObject_cloneDtmObject(Data,&openLeftP)) goto errexit ;
    if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(Data,L"openElmLeftSlopeToes.dat") ;
   }
/*
**  Element Pad With Right Side Slopes Only
**  Write Start Side , Element Boundary and End Side
*/
 if( SideDirection == 1 )
   {
    ovl = RghtRadials  ;
    p3dPts[0].x = ovl->Nx ;
    p3dPts[0].y = ovl->Ny ;
    p3dPts[0].z = ovl->Nz ;
    p3dPts[1].x = ovl->Px ;
    p3dPts[1].y = ovl->Py ;
    p3dPts[1].z = ovl->Pz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
    for ( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials - 1 ; ++ovl )
      {
       ovn = ovl + 1 ;
       p3dPts[0].x = ovl->Px ;
       p3dPts[0].y = ovl->Py ;
       p3dPts[0].z = ovl->Pz ;
       p3dPts[1].x = ovn->Px ;
       p3dPts[1].y = ovn->Py ;
       p3dPts[1].z = ovn->Pz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
      }
    p3dPts[0].x = ovl->Px ;
    p3dPts[0].y = ovl->Py ;
    p3dPts[0].z = ovl->Pz ;
    p3dPts[1].x = ovl->Nx ;
    p3dPts[1].y = ovl->Ny ;
    p3dPts[1].z = ovl->Nz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
   }
/*
** Element Pad With Left Side Slopes Only
** Write Start Side , Element Boundary and End Side
*/
 if( SideDirection == 2 )
   {
    ovl = LeftRadials + NumLeftRadials - 1 ;
    p3dPts[0].x = ovl->Nx ;
    p3dPts[0].y = ovl->Ny ;
    p3dPts[0].z = ovl->Nz ;
    p3dPts[1].x = ovl->Px ;
    p3dPts[1].y = ovl->Py ;
    p3dPts[1].z = ovl->Pz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
    for ( ovl = LeftRadials + NumLeftRadials - 1 ; ovl > LeftRadials  ; --ovl )
      {
       ovn = ovl - 1 ;
       p3dPts[0].x = ovl->Px ;
       p3dPts[0].y = ovl->Py ;
       p3dPts[0].z = ovl->Pz ;
       p3dPts[1].x = ovn->Px ;
       p3dPts[1].y = ovn->Py ;
       p3dPts[1].z = ovn->Pz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
      }
    p3dPts[0].x = ovl->Px ;
    p3dPts[0].y = ovl->Py ;
    p3dPts[0].z = ovl->Pz ;
    p3dPts[1].x = ovl->Nx ;
    p3dPts[1].y = ovl->Ny ;
    p3dPts[1].z = ovl->Nz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
   }
/*
**  Element Pad With Right And Left Side Slopes
**  Write Start and End Sides
*/
 if( SideDirection == 3 )
   {
    ovl = RghtRadials ;
    p3dPts[0].x = ovl->Nx ;
    p3dPts[0].y = ovl->Ny ;
    p3dPts[0].z = ovl->Nz ;
    p3dPts[1].x = ovl->Px ;
    p3dPts[1].y = ovl->Py ;
    p3dPts[1].z = ovl->Pz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
    ovl = LeftRadials  ;
    p3dPts[0].x = ovl->Px ;
    p3dPts[0].y = ovl->Py ;
    p3dPts[0].z = ovl->Pz ;
    p3dPts[1].x = ovl->Nx ;
    p3dPts[1].y = ovl->Ny ;
    p3dPts[1].z = ovl->Nz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
    ovl = LeftRadials + NumLeftRadials - 1 ;
    p3dPts[0].x = ovl->Nx ;
    p3dPts[0].y = ovl->Ny ;
    p3dPts[0].z = ovl->Nz ;
    p3dPts[1].x = ovl->Px ;
    p3dPts[1].y = ovl->Py ;
    p3dPts[1].z = ovl->Pz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
    ovl = RghtRadials + NumRghtRadials - 1 ;
    p3dPts[0].x = ovl->Px ;
    p3dPts[0].y = ovl->Py ;
    p3dPts[0].z = ovl->Pz ;
    p3dPts[1].x = ovl->Nx ;
    p3dPts[1].y = ovl->Ny ;
    p3dPts[1].z = ovl->Nz ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
   }
/*
** Triangulate Data Object
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(Data,L"elmbndy.dat") ;
 plTol = ppTol ;
 Data->ppTol = ppTol ;
 Data->plTol = plTol ;
 if( bcdtmObject_createTinDtmObject(Data,1,0.0)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"elmbndy00.tin") ;
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(Data)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"elmbndy01.tin") ;
/*
** Store Tin Hull As Break Lines In Side Slopes Object
*/
 sp = Data->hullPoint ;
 do
   {
    np = nodeAddrP(Data,sp)->hPtr ;
    p3dPts[0].x = pointAddrP(Data,sp)->x ;
    p3dPts[0].y = pointAddrP(Data,sp)->y ;
    p3dPts[0].z = pointAddrP(Data,sp)->z ;
    p3dPts[1].x = pointAddrP(Data,np)->x ;
    p3dPts[1].y = pointAddrP(Data,np)->y ;
    p3dPts[1].z = pointAddrP(Data,np)->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Breakline,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,p3dPts,2)) goto errexit ;
    sp = np ;
   } while ( sp != Data->hullPoint ) ;
/*
** Store Tin Hull As Boundary Polygon In Side Slopes Object
*/
 if( bcdtmList_extractHullDtmObject(Data,&hullPtsP,&numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::Hull,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
** Mark All Points On Tin Hull
*/
 sp = Data->hullPoint ;
 do
   {
    nodeAddrP(Data,sp)->tPtr = 1 ;
    sp = nodeAddrP(Data,sp)->hPtr ;
   } while ( sp != Data->hullPoint ) ;
/*
** Un Mark Points On Tin Hull That Are Not Gound Intersection Points
*/
 if( SideDirection == 1 )
   {
    for( ovl = RghtRadials ; ovl < RghtRadials + NumRghtRadials ; ++ovl )
      {
       bcdtmFind_closestPointDtmObject(Data,ovl->Px,ovl->Py,&sp) ;
       nodeAddrP(Data,sp)->tPtr = 0 ;
      }
   }
 if( SideDirection == 2 )
   {
    for( ovl = LeftRadials ; ovl < LeftRadials + NumLeftRadials ; ++ovl )
      {
       bcdtmFind_closestPointDtmObject(Data,ovl->Px,ovl->Py,&sp) ;
       nodeAddrP(Data,sp)->tPtr = 0 ;
      }
   }
 if( SideDirection == 3 )
   {
    bcdtmFind_closestPointDtmObject(Data,LeftRadials->Px,LeftRadials->Py,&sp) ;
    nodeAddrP(Data,sp)->tPtr = 0 ;
    bcdtmFind_closestPointDtmObject(Data,(LeftRadials+NumLeftRadials-1)->Px,(LeftRadials+NumLeftRadials-1)->Py,&sp) ;
    nodeAddrP(Data,sp)->tPtr = 0 ;
   }
/*
**  Scan To Non Toe Slope Point On Hull
*/
 np = Data->nullPnt ;
 sp = Data->hullPoint ;
 do
   {
    if( ! nodeAddrP(Data,sp)->tPtr ) np = sp ;
    sp = nodeAddrP(Data,sp)->hPtr ;
   } while ( sp != Data->hullPoint && np == Data->nullPnt ) ;
/*
** Write Slope Toes
*/
 if( np != Data->nullPnt ) Data->hullPoint = np ;
 if( SideDirection == 1 || SideDirection == 3 )
   {
    sp = Data->hullPoint ;
    do
      {
       if( ! nodeAddrP(Data,sp)->tPtr )
         {
          if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
          if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::SlopeToe,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
          if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
       }
       else
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(Data,sp)->x,pointAddrP(Data,sp)->y,pointAddrP(Data,sp)->z) ) goto errexit ;
         }
       sp = nodeAddrP(Data,sp)->hPtr ;
      } while ( sp != Data->hullPoint ) ;
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::SlopeToe,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
 else
   {
    sp = Data->hullPoint ;
    do
      {
       if( ! nodeAddrP(Data,sp)->tPtr )
         {
          if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
          if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::SlopeToe,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
          if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
         }
       else
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(Data,sp)->x,pointAddrP(Data,sp)->y,pointAddrP(Data,sp)->z) ) goto errexit ;
         }
       if(( sp = bcdtmList_nextClkDtmObject(Data,sp,nodeAddrP(Data,sp)->hPtr)) < 0 ) goto errexit ;
      } while ( sp != Data->hullPoint ) ;
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SideSlopes,DTMFeatureType::SlopeToe,SideSlopes->nullUserTag,1,&SideSlopes->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
/*
** Mark Hull Points
*/
 sp = Data->hullPoint ;
 do { nodeAddrP(Data,sp)->PRGN = 1 ; sp = nodeAddrP(Data,sp)->hPtr ; } while ( sp != Data->hullPoint ) ;
/*
**  Look For And Write Holes In Boundary Polygon To Data Object
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Looking For Holes In Boundary Polygon") ;
 if( ! processingLimits ) { if( bcdtmSideSlope_writeInternalPadHolesToDataObject(Data,SideSlopes,1)) goto errexit ; }
 else                     { if( bcdtmSideSlope_writeLimitInternalPadHolesToDataObject(Data,SideSlopes,1)) goto errexit ; }
 if(dbg) bcdtmWrite_message(0,0,0,"Looking For Holes In Boundary Polygon Completed") ;
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
 if( Data     != nullptr ) bcdtmObject_destroyDtmObject(&Data) ;
/*
** Write Status Message
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To Data Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To Data Object Error") ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_reverseOderOfSideSlopeRadials                              |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_reverseOrderOfSideSlopeRadials(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Reverses The Order Of The Radials
*/
{
 DTM_OVERLAP_RADIAL_TABLE *opb,*opt,*ovl,Ovl ;
/*
** Initialise
*/
 opb = OvlPts ;
 opt = OvlPts + NumOvlPts - 1 ;
 ovl = &Ovl ;
/*
** Reverse Order
*/
 while ( opb < opt ) { *ovl = *opb ; *opb = *opt ; *opt = *ovl ; ++opb ; --opt ; }
/*
** Reset Truncating Radials
*/
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
   {
    if( ovl->TruncatingRadial != DTM_NULL_PNT ) ovl->TruncatingRadial = NumOvlPts - ovl->TruncatingRadial - 1 ;
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
int bcdtmSideSlope_removeExternalLoopsFromBoundaryDtmObject( BC_DTM_OBJ *tinP )
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long hullPnt,clPtr,priorPnt,nextPnt,startPnt,breakPnt,savPnt ;
 long process,numBreaks=0,numPts ;
 DTMDirection direction,polyDirection;
 double polyArea,length,markedLength,unMarkedLength,totalLength,clkLength,antLength ;
 BC_DTM_OBJ *dataP=nullptr ;
 DPoint3d  *ptsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Removing External Loops From Boundary Tin") ;
/*
** Remove Dangling Breaks
*/
 if( bcdtmSideSlope_removeDanglingBreaksDtmObject(tinP)) goto errexit ;
/*
** Scan Tin Hull And Until A Hull Point Is Found That Is Part Of More Than Two Break Lines
*/
 hullPnt  = tinP->nullPnt ;
 startPnt = tinP->hullPoint ;
 do
   {
    numBreaks = 0 ;
    clPtr = nodeAddrP(tinP,startPnt)->cPtr ;
    while( clPtr != tinP->nullPtr)
      {
       nextPnt = clistAddrP(tinP,clPtr)->pntNum ;
       clPtr   = clistAddrP(tinP,clPtr)->nextPtr ;
       if( bcdtmList_testForBreakLineDtmObject(tinP,startPnt,nextPnt)) ++numBreaks ;
      }
    if( numBreaks > 2 ) hullPnt = startPnt ;
    startPnt = nodeAddrP(tinP,startPnt)->hPtr ;
   } while ( startPnt != tinP->hullPoint && hullPnt == tinP->nullPnt ) ;
/*
** Write Hull Points
*/
 if( dbg )
   {
    if( hullPnt != tinP->nullPnt )bcdtmWrite_message(0,0,0,"hullPnt = %8ld ** %12.5lf %12.5lf %10.4lf ** numBreaks = %8ld",hullPnt,pointAddrP(tinP,hullPnt)->x,pointAddrP(tinP,hullPnt)->y,pointAddrP(tinP,hullPnt)->z,numBreaks) ;
    else                          bcdtmWrite_message(0,0,0,"No Hull Point On More Than Two break Lines") ;
   }
/*
** If Max Breaks Is Greater Than Two Remove Outside Break Line
*/
 if( hullPnt != tinP->nullPnt )
   {
/*
**  Scan Hull Point For Internal Break Line
*/
    if( dbg )bcdtmWrite_message(0,0,0,"Scanning From hullPnt = %8ld",hullPnt) ;
    nextPnt = nodeAddrP(tinP,hullPnt)->hPtr ;
    if( ( priorPnt = bcdtmList_nextClkDtmObject(tinP,hullPnt,nextPnt)) < 0 ) goto errexit ;
    if( ( breakPnt = bcdtmList_nextClkDtmObject(tinP,hullPnt,priorPnt)) < 0 ) goto errexit ;
    while( breakPnt != nextPnt &&  !bcdtmList_testForBreakLineDtmObject(tinP,hullPnt,breakPnt) )
      {
       if( ( breakPnt = bcdtmList_nextClkDtmObject(tinP,hullPnt,breakPnt)) < 0 ) goto errexit ;
      }
/*
**   Set Tptr Polygon
*/
    if( breakPnt != nextPnt )
      {
       startPnt  = hullPnt ;
       nextPnt   = breakPnt ;
       nodeAddrP(tinP,startPnt)->tPtr = nextPnt ;
       direction = DTMDirection::Clockwise ;
       process   = 1 ;
       while( process )
         {
          do
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"startPnt = %8ld ** nextPnt = %8ld ** %12.5lf %12.5lf %10.4lf",startPnt,nextPnt,pointAddrP(tinP,nextPnt)->x,pointAddrP(tinP,nextPnt)->y,pointAddrP(tinP,nextPnt)->z) ;
             if( direction == DTMDirection::Clockwise ) { if( ( startPnt = bcdtmList_nextClkDtmObject(tinP,nextPnt,startPnt) ) < 0 ) goto errexit ; }
             else                             { if( ( startPnt = bcdtmList_nextAntDtmObject(tinP,nextPnt,startPnt) )   < 0 ) goto errexit ; }
             while ( ! bcdtmList_testForBreakLineDtmObject(tinP,nextPnt,startPnt))
               {
                if( direction == DTMDirection::Clockwise ) { if( ( startPnt = bcdtmList_nextClkDtmObject(tinP,nextPnt,startPnt) ) < 0 ) goto errexit ; }
                else                             { if( ( startPnt = bcdtmList_nextAntDtmObject(tinP,nextPnt,startPnt) )   < 0 ) goto errexit ; }
               }
/*
**           Check For Break In Break Lines
*/
             if( startPnt != hullPnt && nodeAddrP(tinP,startPnt)->tPtr != tinP->nullPnt )
               {
                bcdtmWrite_message(1,0,0,"Break In SlopeToes At Pnt %8ld ** %12.5lf %12.5lf %10.4lf",nextPnt,pointAddrP(tinP,nextPnt)->x,pointAddrP(tinP,nextPnt)->y,pointAddrP(tinP,nextPnt)->z) ;
                goto cleanup ;
               }
             nodeAddrP(tinP,nextPnt)->tPtr = startPnt ;
             savPnt   = startPnt ;
             startPnt = nextPnt  ;
             nextPnt  = savPnt   ;
            } while( nextPnt != hullPnt ) ;
/*
**        Calculate Tptr Area
*/
          if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(tinP,hullPnt) ;
          if( bcdtmList_checkConnectivityTptrPolygonDtmObject(tinP,hullPnt,1)) goto errexit ;
          bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(tinP,hullPnt,&polyArea,&polyDirection) ;
          if( dbg )bcdtmWrite_message(0,0,0,"area = %15.4lf ** direction = %2ld",polyArea,polyDirection) ;
          if( polyDirection == DTMDirection::Clockwise ) if ( bcdtmList_reverseTptrPolygonDtmObject(tinP,hullPnt)) goto errexit ;
/*
**        Write Tptr Polygon To Data Object - Development Purposes
*/
          if( dbg )
            {
             if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
             if( bcdtmList_copyTptrListToPointArrayDtmObject(tinP,hullPnt,&ptsP,&numPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,ptsP,numPts)) goto errexit ;
             free(ptsP) ;
             ptsP = nullptr ;
             if( direction == DTMDirection::Clockwise )     if( bcdtmWrite_toFileDtmObject(dataP,L"clkBdy.dat")) goto errexit ;
             if( direction == DTMDirection::AntiClockwise ) if( bcdtmWrite_toFileDtmObject(dataP,L"antBdy.dat")) goto errexit ;
             if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
            }
/*
**        Scan Tin Hull And Calculate Marked And UnMarked Lengths Of Tin Hull
*/
          if( dbg )bcdtmWrite_message(0,0,0,"Calculating Marked Hull Lengths") ;
          markedLength   = 0.0 ;
          unMarkedLength = 0.0 ;
          totalLength    = 0.0 ;
          startPnt = tinP->hullPoint ;
          do
            {
             nextPnt = nodeAddrP(tinP,startPnt)->hPtr ;
             length  = bcdtmMath_pointDistanceDtmObject(tinP,startPnt,nextPnt);
             totalLength = totalLength + length ;
             if(nodeAddrP(tinP,startPnt)->tPtr == nextPnt ) markedLength   = markedLength   + length ;
             else                                        unMarkedLength = unMarkedLength + length ;
             startPnt = nextPnt ;
            } while( startPnt != tinP->hullPoint);

          if( dbg )bcdtmWrite_message(0,0,0,"totalLength = %15.4lf ** markedLength = %15.4lf ** unMarkedLength = %15.4lf",totalLength,markedLength,unMarkedLength) ;
/*
**        Set Lengths
*/
          if( direction == DTMDirection::Clockwise ) clkLength = markedLength ;
          else                             antLength = markedLength ;
/*
**        Check For Number Of Scans
*/
          if( direction == DTMDirection::Clockwise )
            {
             if( bcdtmList_copyTptrListToSptrListDtmObject(tinP,hullPnt) ) goto errexit ;
             bcdtmList_nullTptrListDtmObject(tinP,hullPnt);
             direction = DTMDirection::AntiClockwise ;
             startPnt  = hullPnt ;
             nextPnt   = breakPnt ;
             nodeAddrP(tinP,startPnt)->tPtr = nextPnt ;
            }
          else  process = 0 ;
         }
/*
**     Set New Hull Into Tptr List
*/
       if( dbg )bcdtmWrite_message(0,0,0,"Setting New Hull Into Tptr Polygon") ;
       if( clkLength >= antLength )
         {
          if( bcdtmList_nullTptrListDtmObject(tinP,hullPnt)) goto errexit ;
          if( bcdtmList_copySptrListToTptrListDtmObject(tinP,hullPnt) ) goto errexit ;
         }
       else
         {
          if( bcdtmList_nullSptrListDtmObject(tinP,hullPnt)) goto errexit ;
         }
/*
**     Clip Tin To Tptr Polygon
*/
       if( dbg )bcdtmWrite_message(0,0,0,"Clippint Tin To Tptr Polygon") ;
       if( bcdtmClip_toTptrPolygonDtmObject(tinP,hullPnt,DTMClipOption::External)) goto errexit ;
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( ptsP  != nullptr ) free(ptsP) ;
 if( dataP != nullptr )bcdtmObject_destroyDtmObject(&dataP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )bcdtmWrite_message(0,0,0,"Removing External Loops From Boundary Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS )bcdtmWrite_message(0,0,0,"Removing External Loops From Boundary Tin Error") ;
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
int bcdtmSideSlope_writeClosedSideSlopeElementBoundaryPolygonToDataObject
(
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,
 long                     numRightRadials,
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,
 long                     numLeftRadials,
 long                     sideSlopeDirection,
 BC_DTM_OBJ               *sideSlopesP,
 double                   ppTol,
 double                   plTol
)
/*
** This Function Writes The Boundary Polygon To The Side Slopes DTM Object
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    sp,numHullPts,numTruncatedRadials ;
 DPoint3d     p3dPts[2],*hullPtsP=nullptr ;
 BC_DTM_OBJ *dtmP=nullptr ;
 DTM_OVERLAP_RADIAL_TABLE  *ovlP,*ovnP ;

// char ssasc[100] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Writing Closed Side Slope Element Boundary Polygon To DTM Object") ;
    bcdtmWrite_message(0,0,0,"rightRadialsP      =  %p",rightRadialsP) ;
    bcdtmWrite_message(0,0,0,"numRightRadials    =  %8ld",numRightRadials) ;
    bcdtmWrite_message(0,0,0,"leftRadialsP       =  %p",leftRadialsP) ;
    bcdtmWrite_message(0,0,0,"numLeftRadials     =  %8ld",numLeftRadials) ;
    bcdtmWrite_message(0,0,0,"sideSlopeDirection =  %8ld",sideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"rightRadialsP      =  %p",rightRadialsP) ;
    bcdtmWrite_message(0,0,0,"sideSlopesP        =  %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"ppTol              =  %12.8lf",ppTol) ;
    bcdtmWrite_message(0,0,0,"plTol              =  %12.8lf",plTol) ;
   }
/*
** Pad With Internal Side Slopes Only
*/
 if( sideSlopeDirection == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Pad As Boundary Polygon") ;
/*
**  Store Pad As Boundary Polygon In Data Object
*/
    for( ovlP = leftRadialsP ; ovlP < leftRadialsP + numLeftRadials ; ++ovlP )
      {
       if( ovlP == leftRadialsP || ( ovlP->Px != (ovlP-1)->Px || ovlP->Py != (ovlP-1)->Py) )
         {
          if( bcdtmLoad_storePointInCache(ovlP->Px,ovlP->Py,ovlP->Pz) ) goto errexit ;
         }
      }
/*
**  Check Boundary Polygon Closes
*/
    --ovlP ;
    if( ovlP->Px != leftRadialsP->Px || ovlP->Py != leftRadialsP->Py )
      {
       if( bcdtmLoad_storePointInCache(leftRadialsP->Px,leftRadialsP->Py,leftRadialsP->Pz) ) goto errexit ;
      }
/*
**  Store Cache Points As Hull
*/
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
**  Clean Up Before Returning
*/
    goto cleanup ;
   }
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,2*numRightRadials,1000) ;
/*
** Check For Truncated External rightRadialsP
*/
 numTruncatedRadials = 0 ;
 for( ovlP = rightRadialsP ; ovlP < rightRadialsP + numRightRadials ; ++ovlP )
   {
    if( ! ovlP->Status ) ++numTruncatedRadials ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numRightRadials = %6ld numTruncatedRadials = %6ld",numRightRadials,numTruncatedRadials) ;
/*
** If No Right Radials Truncated , Just Write Toe Slopes From Overlay Table
*/
 if( numTruncatedRadials == 0  )
   {
/*
** Write As Individual Break Lines
*/
    for( ovlP = rightRadialsP ; ovlP < rightRadialsP + numRightRadials ; ++ovlP )
      {
       ovnP = ovlP + 1 ;
       if( ovnP >= rightRadialsP + numRightRadials ) ovnP = rightRadialsP ;
       p3dPts[0].x = ovlP->Nx ;
       p3dPts[0].y = ovlP->Ny ;
       p3dPts[0].z = ovlP->Nz ;
       p3dPts[1].x = ovnP->Nx ;
       p3dPts[1].y = ovnP->Ny ;
       p3dPts[1].z = ovnP->Nz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Breakline,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,p3dPts,2)) goto errexit ;
      }
/*
**  Write As Boundary Polygon And Slope Toes
*/
    for( ovlP = rightRadialsP ; ovlP < rightRadialsP + numRightRadials ; ++ovlP )
      {
       if( bcdtmLoad_storePointInCache(ovlP->Nx,ovlP->Ny,ovlP->Nz) ) goto errexit ;
      }
    if( bcdtmLoad_storePointInCache(rightRadialsP->Nx,rightRadialsP->Ny,rightRadialsP->Nz) ) goto errexit ;
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
**  Clean Up Before Returning
*/
    goto cleanup ;
   }
/*
**  Write Slope Toe Data To Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Pad Slope Toes To Data Object") ;
 if( ! processingLimits ) { if( bcdtmSideSlope_writeSlopeToesToDataObject(2,dtmP,rightRadialsP,numRightRadials) ) goto errexit ; }
 else                     { if( bcdtmSideSlope_writeLimitSlopeToesToDataObject(2,dtmP,rightRadialsP,numRightRadials) ) goto errexit ; }
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"exttslopetoes.dat") ;
/*
** Triangulate Data Object
*/
 plTol = ppTol ;
 dtmP->ppTol = ppTol ;
 dtmP->plTol = plTol ;
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"extSlopeToes00.tin") ;
/*
** Check For Broken Side Slopes
** Following Code Added 14/Mar/2005 RobC - To Fix Breaks In Slope Toes
*/
 if( bcdtmSideSlope_detectAndFixBreaksInSlopeToes(dtmP) ) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"fixedexttslopetoes.tin") ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"extSlopeToes01.tin") ;
/*
** Remove External Loops From Boundary Tin
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing External Loops From Boundary Tin") ;
// if( bcdtmSideSlope_removeExternalLoopsFromBoundaryDtmObject(dtmP,)) goto errexit ;
// bcdtmSideSlope_removeExternalLoopsFromBoundaryDtmObject(dtmP,) ;
/*
** Store Tin Hull As Boundary Polygon In Side Slopes Object
*/
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Breakline,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
** Mark Hull Points
*/
 sp = dtmP->hullPoint ;
 do { nodeAddrP(dtmP,sp)->PRGN = 1 ; sp = nodeAddrP(dtmP,sp)->hPtr ; } while ( sp != dtmP->hullPoint ) ;
/*
**  Look For And Write Holes In Boundary Polygon To Side Slopes Object
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Looking For Holes In Boundary Polygon") ;
 if( ! processingLimits ) { if( bcdtmSideSlope_writeInternalPadHolesToDataObject(dtmP,sideSlopesP,1)) goto errexit ; }
 else                     { if( bcdtmSideSlope_writeLimitInternalPadHolesToDataObject(dtmP,sideSlopesP,1)) goto errexit ; }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
 if( dtmP     != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Write Status Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Closed Side Slope Element Boundary Polygon To DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Closed Side Slope Element Boundary Polygon To DTM Object Error") ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Return
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
int bcdtmSideSlope_detectAndFixBreaksInSlopeToes(BC_DTM_OBJ *tinP)
/*
** This Function Detects And Fixes Breaks In The Slope Toes
** This Is Not A Generic Function
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   sp,np,featureNum,clist,numBreaks,spNum,npNum,dtmFeature ;
 bool first;
 double dd,dmin ;
 BC_DTM_FEATURE  *fP ;
 DTM_TIN_NODE    *dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting And Fixing Breaks In Slope Toes") ;
/*
** Initialise
*/
 bcdtmList_nullTptrValuesDtmObject(tinP) ;
/*
** Scan Feature Table For Slope Toes Breaks That Are Not Connected At Both Ends
*/
 numBreaks = 0 ;
 for( dtmFeature = 0 ; dtmFeature < tinP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(tinP,dtmFeature) ;
    if( ( sp = fP->dtmFeaturePts.firstPoint ) != tinP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature %6ld ** First Pnt = %6ld ** %12.5lf %12.5lf %10.4lf",dtmFeature,sp,pointAddrP(tinP,sp)->x,pointAddrP(tinP,sp)->y,pointAddrP(tinP,sp)->z) ;
/*
**     Count Number Of Breaks At Sp
*/
       spNum = 0 ;
       clist = nodeAddrP(tinP,sp)->cPtr ;
       while( clist != tinP->nullPtr )
         {
          if( bcdtmList_testForBreakLineDtmObject(tinP,sp,clistAddrP(tinP,clist)->pntNum)) ++spNum ;
          clist = clistAddrP(tinP,clist)->nextPtr ;
         }
/*
**     Scan Feature Points
*/
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(tinP,dtmFeature,sp,&np)) goto errexit ;
          if( np != tinP->nullPnt )
            {
/*
**           Count Number Of Breaks At Np
*/
             npNum = 0 ;
             clist = nodeAddrP(tinP,np)->cPtr ;
             while( clist != tinP->nullPtr )
               {
                if( bcdtmList_testForBreakLineDtmObject(tinP,np,clistAddrP(tinP,clist)->pntNum)) ++npNum ;
                clist = clistAddrP(tinP,clist)->nextPtr ;
               }
/*
**           Test For Break In Slope Toes
*/
             if( spNum == 1 && npNum <= 2  )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Sp Break Detected At %6ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(tinP,sp)->x,pointAddrP(tinP,sp)->y,pointAddrP(tinP,sp)->z) ;
                nodeAddrP(tinP,sp)->tPtr = np ;
                ++numBreaks ;
               }
             if( spNum <= 2 && npNum == 1  )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Np Break Detected At %6ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(tinP,np)->x,pointAddrP(tinP,np)->y,pointAddrP(tinP,np)->z) ;
                nodeAddrP(tinP,np)->tPtr = sp ;
                ++numBreaks ;
               }
             spNum = npNum ;
             npNum = 0 ;
            }
          sp = np ;
         } while( sp != fP->dtmFeaturePts.firstPoint && sp != tinP->nullPnt ) ;
      }
   }
/*
** Write Number Of Breaks Detected In Slope Toes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Breaks Detected = %4ld",numBreaks) ;
/*
**  Join Up Unconnected Breaks
*/
 if( numBreaks > 0 )
   {
    for( sp = 0 ; sp < tinP->numPoints ; ++sp )
      {
       dP = nodeAddrP(tinP,sp) ;
       if( dP->tPtr != tinP->nullPnt )
         {
          np = tinP->nullPnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Break At %6ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(tinP,sp)->x,pointAddrP(tinP,sp)->y,pointAddrP(tinP,sp)->z) ;
/*
**        Scan Sp Looking For Unconnected Break
*/
          first = true ;
          clist = nodeAddrP(tinP,sp)->cPtr ;
          while( clist != tinP->nullPtr )
            {
             if( nodeAddrP(tinP,clistAddrP(tinP,clist)->pntNum)->tPtr != tinP->nullPnt )
               {
                dd = bcdtmMath_pointDistanceDtmObject(tinP,sp,clistAddrP(tinP,clist)->pntNum) ;
                if( first == true || dd < dmin )
                  {
                   dmin = dd ;
                   first = false ;
                   np = clistAddrP(tinP,clist)->pntNum ;
                  }
               }
             clist = clistAddrP(tinP,clist)->nextPtr ;
            }
/*
**       If No Connection Found Scan For Closest break Point
*/
          if( np == tinP->nullPnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Connection Not Found ** Scanning For Closest Break Point") ;
             first = true ;
             clist = nodeAddrP(tinP,sp)->cPtr ;
             while( clist != tinP->nullPtr )
               {
                if( nodeAddrP(tinP,clistAddrP(tinP,clist)->pntNum)->tPtr == tinP->nullPnt )
                  {
                   if( ! bcdtmList_testForBreakLineDtmObject(tinP,sp,clistAddrP(tinP,clist)->pntNum))
                     {
                      if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(tinP,DTMFeatureType::Breakline,clistAddrP(tinP,clist)->pntNum,&featureNum)) goto errexit ;
                        {
                         dd = bcdtmMath_pointDistanceDtmObject(tinP,sp,clistAddrP(tinP,clist)->pntNum) ;
                         if( first == true || dd < dmin )
                           {
                            dmin = dd ;
                            first = false ;
                            np = clistAddrP(tinP,clist)->pntNum ;
                           }
                        }
                     }
                  }
                clist = clistAddrP(tinP,clist)->nextPtr ;
               }
            }
/*
**       Insert Sp-Np As A Break Line
*/
          if( np != tinP->nullPnt )
            {
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Inserting Break Line Between Tin Points") ;
                bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.5lf %12.5lf %10.4lf",sp,pointAddrP(tinP,sp)->x,pointAddrP(tinP,sp)->y,pointAddrP(tinP,sp)->z) ;
                bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.5lf %12.5lf %10.4lf",np,pointAddrP(tinP,np)->x,pointAddrP(tinP,np)->y,pointAddrP(tinP,np)->z) ;
               }
             nodeAddrP(tinP,sp)->tPtr = np ;
             nodeAddrP(tinP,np)->tPtr = tinP->nullPnt ;
             if( bcdtmInsert_addDtmFeatureToDtmObject(tinP,nullptr,0,DTMFeatureType::Breakline,tinP->nullUserTag,tinP->nullFeatureId,sp,1)) goto errexit ;
            }
          else
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Break Not Connected") ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(tinP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Detecting And Fixing Breaks In Slope Toes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Detecting And Fixing Breaks In Slope Toes Error") ;
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
int bcdtmSideSlope_writeSlopeToesToDataObject(long sideSlopeElementType,BC_DTM_OBJ *Data,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts)
/*
** This Function Writes The Radial Slope Toes To A Data Object
*/
{
 int     ret=0,dbg=0 ;
 long    padType,IntTableNe,IntPtsNe,IntPtsMe,IntPtsMinc,ToeFound ;
 double  Px=0.0,Py=0.0,Pz=0.0;
 long    *pl,*Index=nullptr ;
 DPoint3d     p3dPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE  *ovl,*ovln,*ovlp,*Ovl ;
 DTM_STR_INT_TAB *pint,*IntTable=nullptr ;
 DTM_STR_INT_PTS *pinp,*IntPts=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Slope Toes To Data Object") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Side SlopeElement Type = %2ld",sideSlopeElementType) ;
    bcdtmWrite_message(0,0,0,"Number Of Overlay Points = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"ovl[%4ld] T = %1ld S = %1ld Tr = %9ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Status,ovl->TruncatingRadial,ovl->Px,ovl->Py,ovl->Pz,ovl->Nx,ovl->Ny,ovl->Nz) ;
      }
   }
/*
** Create And Build Slope Toe Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Slope Toe Intersection Table") ;
 if( bcdtmSideSlope_buildSlopeToeIntersectionTable(OvlPts,NumOvlPts,&IntTable,&IntTableNe)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slope Toe Intersection Table Entries = %6ld",IntTableNe) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Pad Line Intersection Table Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** Pad = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
      }
   }
/*
** Scan For Intersections
*/
 IntPtsMinc = IntTableNe / 10 ;
 if( IntPtsMinc < 1000 ) IntPtsMinc = 1000 ;
 IntPtsNe = IntPtsMe = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Slope Toe Intersections") ;
 if( bcdtmSideSlope_scanForSlopeToeIntersections(IntTable,IntTableNe,&IntPts,&IntPtsNe,&IntPtsMe,IntPtsMinc) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slope Toe Intersections = %4ld",IntPtsNe) ;
 if( IntPtsNe > 0 )
   {
/*
** Sort Intersection Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(IntPts,IntPtsNe,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_radialRadialIntersectionPointsCompareFunction) ;
/*
** Write Intersection Points
*/
    if( dbg == 1  )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",IntPtsNe) ;
       for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(pinp-IntPts),pinp->String1,pinp->Segment1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z,pinp->Z2) ;
                 }
      }
   }
/*
** Build Index Table To Intersection Points
*/
 Index = (long *) malloc( NumOvlPts * sizeof(long)) ;
 if( Index == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( pl = Index ; pl < Index + NumOvlPts ; ++pl ) *pl = DTM_NULL_PNT ;
/*
** Place Index To Intersection Points In Index Table
*/
 for( pinp = IntPts ; pinp < IntPts + IntPtsNe ; ++pinp )
   {
    if( pinp == IntPts || ( pinp > IntPts && pinp->Segment1 != (pinp-1)->Segment1 ) )
      {
       *(Index + pinp->Segment1) = (long)(pinp-IntPts) ;
      }
   }
/*
** Write Index Entries
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"None Null Index Entries") ;
    for( pl = Index ; pl < Index + NumOvlPts ; ++pl )
      {
       if( *pl != DTM_NULL_PNT ) bcdtmWrite_message(0,0,0,"Index[%5ld] = %5ld",(long)(pl-Index),*pl) ;
      }
   }
/*
** Build Radial Pad Edge Intersection Table For Scanning Against Edges
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Radial Pad Edge Intersection Table") ;
 if( bcdtmSideSlope_buildRadialEdgeIntersectionTable(OvlPts,NumOvlPts,1,&IntTable,&IntTableNe) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Radial Pad Edge Intersection Table Entries = %6ld",IntTableNe) ;
/*
** Store Slope Toes In Data Object
*/
 if( sideSlopeElementType == 1 ) padType = 1 ;
 else                            padType = 0 ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts - padType ; ++ovl )
   {
/*
**  Only Process Non Truncated Radials
*/
    if( ovl->Status == 1 )
      {
       ovlp = ovl - 1 ;
       if( ovlp < OvlPts ) ovlp = OvlPts + NumOvlPts - 1 ;
       ovln = ovl + 1 ;
       if( ovln >= OvlPts + NumOvlPts ) ovln = OvlPts ;
       if( dbg && ( ovlp->Status == 0 || ovln->Status == 0 ))
         {
          bcdtmWrite_message(0,0,0,"ovlp = [%4ld] T = %1ld S = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(ovlp-OvlPts),ovlp->Type,ovlp->Status,ovlp->Px,ovlp->Py,ovlp->Pz) ;
          bcdtmWrite_message(0,0,0,"ovl  = [%4ld] T = %1ld S = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Type,ovl->Status,ovl->Px,ovl->Py,ovl->Pz) ;
          bcdtmWrite_message(0,0,0,"ovln = [%4ld] T = %1ld S = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(ovln-OvlPts),ovln->Type,ovln->Status,ovln->Px,ovln->Py,ovln->Pz) ;
         }
/*
**  Prior Side Slope Truncated
*/
       if( ovlp->Status == 0 )
         {
                  ToeFound = 0 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Truncated") ;
/*
**  If Valid Index Entry Get Coordinates
*/
                   if( *(Index+(long)(ovlp-OvlPts )) != DTM_NULL_PNT )
                    {
                         ToeFound = 1 ;
                         Px = (IntPts+*(Index+(long)(ovlp-OvlPts )))->x ;
                         Py = (IntPts+*(Index+(long)(ovlp-OvlPts )))->y ;
                     Pz = (IntPts+*(Index+(long)(ovlp-OvlPts )))->z ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Intersection Found ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
            }
          else
                    {
             bcdtmSideSlope_findFirstPriorNonTruncatedPadToePoint(sideSlopeElementType,OvlPts,NumOvlPts,IntTable,IntTableNe,ovl,&Ovl) ;
                         if( Ovl != nullptr )
                           {
                            ToeFound = 1 ;
                            Px = Ovl->Nx ;
                            Py = Ovl->Ny ;
                            Pz = Ovl->Nz ;
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Point Found") ;
                           }
/*
**           Check And Fix For Transition Point - RobC  Feb 2004
*/
             if( ovl->Px == ovl->Gx && ovl->Py == ovl->Gy )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Current Radial Is Transistion Point") ;
/*
**              Scan Forward From Transition Point
*/
                Ovl = ovlp ;
                while ( Ovl->Status != 1 )
                  {
                   --Ovl ;
                   if( Ovl < OvlPts ) Ovl = OvlPts + NumOvlPts - 1 ;
                  }
                ToeFound = 1 ;
                        Px = Ovl->Nx ;
                        Py = Ovl->Ny ;
                        Pz = Ovl->Nz ;
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Point Found For Transition Point ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
               }
                        }
          if( ToeFound )
            {
             p3dPts[0].x = Px ;
             p3dPts[0].y = Py ;
             p3dPts[0].z = Pz ;
             p3dPts[1].x = ovl->Gx ;
             p3dPts[1].y = ovl->Gy ;
             p3dPts[1].z = ovl->Gz ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Found ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
            }
          else if( dbg ) bcdtmWrite_message(0,0,0,"**** Prior Slope Toe Point Not Found") ;
         }
/*
**  Next Slope Toe Not Truncated
*/
       if( ovln->Status )
         {
          p3dPts[0].x = ovl->Gx ;
          p3dPts[0].y = ovl->Gy ;
          p3dPts[0].z = ovl->Gz ;
          p3dPts[1].x = ovln->Gx ;
          p3dPts[1].y = ovln->Gy ;
          p3dPts[1].z = ovln->Gz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
         }
/*
**  Next Slope Toe Truncated
*/
       else
         {
          ToeFound = 0 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Truncated") ;
/*
**        If Valid Index Entry Get Coordinates
*/
                   if( *(Index+(long)(ovl-OvlPts )) != DTM_NULL_PNT )
                    {
                         ToeFound = 1 ;
                         Px = (IntPts+*(Index+(long)(ovl-OvlPts )))->x ;
                         Py = (IntPts+*(Index+(long)(ovl-OvlPts )))->y ;
                     Pz = (IntPts+*(Index+(long)(ovl-OvlPts )))->z ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Intersection Found ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
            }
          else
                    {
             bcdtmSideSlope_findFirstNextNonTruncatedPadToePoint(sideSlopeElementType,OvlPts,NumOvlPts,IntTable,IntTableNe,ovl,&Ovl) ;
                         if( Ovl != nullptr )
                           {
                            ToeFound = 1 ;
                            Px = Ovl->Nx ;
                            Py = Ovl->Ny ;
                            Pz = Ovl->Nz ;
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Point Found ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
                           }
             else
               {
/*
**              Check And Fix For Transition Point - RobC  Feb 2004
*/
                if( ovl->Px == ovl->Gx && ovl->Py == ovl->Gy )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"**** Current Radial Is Transistion Point") ;
/*
**                 Scan Forward From Transition Point
*/
                   Ovl = ovln ;
                   while ( Ovl->Status != 1 )
                     {
                      ++Ovl ;
                      if( Ovl >= OvlPts + NumOvlPts ) Ovl = OvlPts ;
                     }
                               ToeFound = 1 ;
                               Px = Ovl->Nx ;
                               Py = Ovl->Ny ;
                               Pz = Ovl->Nz ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Point Found For Transition Point ** %10.4lf %10.4lf %10.4lf",Px,Py,Pz) ;
                  }
               }
                        }
          if( ToeFound )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Found **") ;
             p3dPts[0].x = ovl->Gx ;
             p3dPts[0].y = ovl->Gy ;
             p3dPts[0].z = ovl->Gz ;
             p3dPts[1].x = Px ;
             p3dPts[1].y = Py ;
             p3dPts[1].z = Pz ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::Breakline,Data->nullUserTag,1,&Data->nullFeatureId,p3dPts,2)) goto errexit ;
            }
          else if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Slope Toe Point Not Found") ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( IntTable != nullptr ) free(IntTable) ;
 if( IntPts   != nullptr ) free(IntPts)   ;
 if( Index    != nullptr ) free(Index)    ;
/*
** Job Completed
*/
 if( dbg && !ret ) bcdtmWrite_message(0,0,0,"Writing Slope Toes To Data Object Completed") ;
 if( dbg &&  ret ) bcdtmWrite_message(0,0,0,"Writing Slope Toes To Data Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_buildSlopeToeIntersectionTable(DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_TAB **IntTable,long *IntTableNe)
{
 int    ret=0,dbg=0 ;
 long   type1,type2,IntTableMe,IntTableMinc  ;
 double cord ;
 DTM_OVERLAP_RADIAL_TABLE *ovl ;
 DTM_STR_INT_TAB *pint ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Slope Toe Intersection Table") ;
/*
** Write Radials For Development Purposes
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Side Slope Radials = %6ld",NumOvlPts) ;
    for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts ; ++ovl )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] ** Status = %1ld ** %12.4lf %12.4lf %10.4lf ** %12.4lf %12.4lf %10.4lf",(long)(ovl-OvlPts),ovl->Status,ovl->Px,ovl->Py,ovl->Pz,ovl->Gx,ovl->Gy,ovl->Gz) ;
      }
   }
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 IntTableMinc = NumOvlPts / 2  ;
 if( IntTableMinc == 0 ) IntTableMinc = 100 ;
/*
** Store Slope Toes In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Slope Toes In Intersection Table") ;
 for( ovl = OvlPts ; ovl < OvlPts + NumOvlPts - 1 ; ++ovl )
   {
/*
**  Only Write Those Slope Toes That Start Or End From A Non Truncated Radial
*/
    if( ovl->Status || (ovl+1)->Status )
          {
/*
**  Only Write Those Slope Toes From Radials That Are Not Truncated By A Concave Radial (Type 1 )
*/
       type1 = type2 = 0 ;
           if( ovl->TruncatingRadial     != DTM_NULL_PNT ) type1 = (OvlPts+ovl->TruncatingRadial)->Type     ;
           if( (ovl+1)->TruncatingRadial != DTM_NULL_PNT ) type2 = (OvlPts+(ovl+1)->TruncatingRadial)->Type ;
           if( type1 != 1 && type2 != 1 )
             {
          if( dbg ) bcdtmWrite_message(0,0,0,"Storing Slope Toe %6ld %6ld",(long)(ovl-OvlPts),(long)(ovl-OvlPts+1)) ;
/*
**  Check For Memory Allocation
*/
          if( *IntTableNe == IntTableMe )
            {
             IntTableMe = IntTableMe + IntTableMinc ;
             if( *IntTable == nullptr ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
             else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
             if( *IntTable == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
**  Store Ovl Line
*/
          (*IntTable+*IntTableNe)->String  = 0 ;
          (*IntTable+*IntTableNe)->Segment = (long)(ovl-OvlPts) ;
          (*IntTable+*IntTableNe)->Type    = 1   ;
          (*IntTable+*IntTableNe)->Direction = 1 ;
          (*IntTable+*IntTableNe)->X1 = ovl->Gx ;
          (*IntTable+*IntTableNe)->Y1 = ovl->Gy ;
          (*IntTable+*IntTableNe)->Z1 = ovl->Gz ;
          (*IntTable+*IntTableNe)->X2 = (ovl+1)->Gx ;
          (*IntTable+*IntTableNe)->Y2 = (ovl+1)->Gy ;
          (*IntTable+*IntTableNe)->Z2 = (ovl+1)->Gz ;
          ++*IntTableNe ;
         }
      }
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( pint = *IntTable ; pint < *IntTable + *IntTableNe ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Slope Toe Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Slope Toe Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != nullptr ) { free(*IntTable) ; *IntTable = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForSlopeToeIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for SlopeToe Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineSlopeToeIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_determineSlopeToeIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y ;
 DTM_STR_INT_TAB  *alp,*slp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
**  Check Slope Toe Start Points Are Not Coincident
*/
    if( labs(alp->Segment-slp->Segment) > 1 )
      {
/*
** Check Lines Intersect
*/
       if( bcdtmMath_checkIfLinesIntersect(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2))
         {
/*
** Intersect Lines
*/
          bcdtmMath_normalIntersectCordLines(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2,&x,&y) ;
/*
** Check Memory
*/
          if( *IntPtsNe + 1 >= *IntPtsMe )
            {
             *IntPtsMe = *IntPtsMe + IntPtsMinc ;
             if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
             if( *IntPts == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
** Calculate Distances For Alp
*/
          if( alp->Direction == 1 ) { Xs = alp->X1 ; Ys = alp->Y1 ; Zs = alp->Z1 ; Xe = alp->X2 ; Ye = alp->Y2 ; Ze = alp->Z2 ; }
          if( alp->Direction == 2 ) { Xs = alp->X2 ; Ys = alp->Y2 ; Zs = alp->Z2 ; Xe = alp->X1 ; Ye = alp->Y1 ; Ze = alp->Z1 ; }
          dz = Ze - Zs ;
          di = bcdtmMath_distance(Xs,Ys,x,y) ;
          dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point Alp
*/
          (*IntPts+*IntPtsNe)->String1  = *IntPtsNe  ;
          (*IntPts+*IntPtsNe)->Segment1 = alp->Segment ;
          (*IntPts+*IntPtsNe)->String2  = *IntPtsNe + 1  ;
          (*IntPts+*IntPtsNe)->Segment2 = slp->Segment ;
          (*IntPts+*IntPtsNe)->Distance = di ;
          (*IntPts+*IntPtsNe)->x = x ;
          (*IntPts+*IntPtsNe)->y = y ;
          (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
          ++*IntPtsNe ;
/*
** Calculate Distances For Slp
*/
          if( slp->Direction == 1 ) { Xs = slp->X1 ; Ys = slp->Y1 ; Zs = slp->Z1 ; Xe = slp->X2 ; Ye = slp->Y2 ; Ze = slp->Z2 ; }
          if( slp->Direction == 2 ) { Xs = slp->X2 ; Ys = slp->Y2 ; Zs = slp->Z2 ; Xe = slp->X1 ; Ye = slp->Y1 ; Ze = slp->Z1 ; }
          dz = Ze - Zs ;
          di = bcdtmMath_distance(Xs,Ys,x,y) ;
          dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point For Slp
*/
          (*IntPts+*IntPtsNe)->String1  = *IntPtsNe  ;
          (*IntPts+*IntPtsNe)->Segment1 = slp->Segment ;
          (*IntPts+*IntPtsNe)->String2  = *IntPtsNe - 1  ;
          (*IntPts+*IntPtsNe)->Segment2 = alp->Segment ;
          (*IntPts+*IntPtsNe)->Distance = di ;
          (*IntPts+*IntPtsNe)->x = x ;
          (*IntPts+*IntPtsNe)->y = y ;
          (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
          ++*IntPtsNe ;
/*
** Store Z2 Values
*/
          (*IntPts+*IntPtsNe-2)->Z2 = (*IntPts+*IntPtsNe-1)->z ;
          (*IntPts+*IntPtsNe-1)->Z2 = (*IntPts+*IntPtsNe-2)->z ;
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|     bcdtmSideSlope_findFirstPrioNonTruncatedPadToePoint                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_findFirstPriorNonTruncatedPadToePoint(long sideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_OVERLAP_RADIAL_TABLE *Ovl,DTM_OVERLAP_RADIAL_TABLE **Ovp )
{
 long   dbg=0,IntFlag,NumTested  ;
 DTM_OVERLAP_RADIAL_TABLE *ovp ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding First Prior Non Truncated Toe Point Ovl = %6ld ** %10.4lf %10.4lf %10.4lf",(long)(Ovl-OvlPts),Ovl->Px,Ovl->Py,Ovl->Pz) ;
/*
** Initialise
*/
 *Ovp = nullptr ;
 NumTested = 0 ;
/*
** Scan Backwards Through Overlay Table Looking For Non Truncated Toe Point
*/
 ovp  = Ovl ;
 do
   {
    --ovp ;
    IntFlag = 1 ;
    if      ( ovp < OvlPts && sideSlopeElementType == 1 ) ovp = nullptr ;
    else if ( ovp < OvlPts && sideSlopeElementType == 2 ) ovp = OvlPts + NumOvlPts - 1 ;
    if( ovp != nullptr )
      {
/*
**  Check For Non Truncated Slope Toe Point
*/
       if( ovp->Nx == ovp->Gx && ovp->Ny == ovp->Gy )
         {
          ++NumTested ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Testing Prior Non Truncated Toe Point Ovp = %6ld ** %10.4lf %10.4lf %10.4lf",(long)(ovp-OvlPts),ovp->Px,ovp->Py,ovp->Pz) ;
/*
** Test If Line Connecting Ovl And Ovp Toe Points Is Intersected By A Radial Or Pad Edge
*/
          bcdtmSideSlope_checkForInterectionOfSlopeToeWithRadialOrPadEdge(OvlPts,ovp,Ovl,IntTable,IntTableNe,&IntFlag) ;
         }
      }
   } while ( IntFlag && ovp != Ovl && ovp != nullptr ) ;
/*
** Set Return Values
*/
 if( ! IntFlag  && ovp != Ovl && ovp != nullptr ) *Ovp = ovp ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding First Prior Non Truncated Toe Point Completed ** Numtested = %6ld",NumTested) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|     bcdtmSideSlope_findFirstNextNonTruncatedPadToePoint                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_findFirstNextNonTruncatedPadToePoint(long sideSlopeElementType,DTM_OVERLAP_RADIAL_TABLE *OvlPts,long NumOvlPts,DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_OVERLAP_RADIAL_TABLE *Ovl,DTM_OVERLAP_RADIAL_TABLE **Ovn )
{
 long   dbg=0,IntFlag,NumTested  ;
 DTM_OVERLAP_RADIAL_TABLE *ovn ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding First Next Non Truncated Toe Point Ovl = %6ld ** %10.4lf %10.4lf %10.4lf",(long)(Ovl-OvlPts),Ovl->Px,Ovl->Py,Ovl->Pz) ;
/*
** Initialise
*/
 *Ovn = nullptr ;
 NumTested = 0 ;
/*
** Scan Forwards Through Overlay Table Looking For Non Truncated Toe Point
*/
 ovn  = Ovl ;
 do
   {
    ++ovn ;
    IntFlag = 1 ;
    if      (  ovn >= OvlPts + NumOvlPts && sideSlopeElementType == 1 ) ovn = nullptr   ;
    else if (  ovn >= OvlPts + NumOvlPts && sideSlopeElementType == 2 ) ovn = OvlPts ;
    if( ovn != nullptr )
          {
/*
**  Check For Non Truncated Slope Toe Point
*/
       if( ovn->Nx == ovn->Gx && ovn->Ny == ovn->Gy )
         {
          ++NumTested ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Testing Non Truncated Next Toe Point Ovn = %6ld ** %10.4lf %10.4lf %10.4lf",(long)(ovn-OvlPts),ovn->Px,ovn->Py,ovn->Pz) ;
/*
** Test If Line Connecting Ovl And Ovp Toe Points Is Intersected By A Radial Or Pad Edge
*/
          bcdtmSideSlope_checkForInterectionOfSlopeToeWithRadialOrPadEdge(OvlPts,Ovl,ovn,IntTable,IntTableNe,&IntFlag) ;
         }
      }
   } while ( IntFlag && ovn != Ovl && ovn != nullptr ) ;
/*
** Set Return Values
*/
 if( ! IntFlag && ovn != Ovl && ovn != nullptr )  *Ovn = ovn ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding First Next Non Truncated Toe Point Completed ** NumTested = %6ld",NumTested) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_checkForInterectionOfSlopeToeWithRadialOrPadEdge(DTM_OVERLAP_RADIAL_TABLE *OvlPts,DTM_OVERLAP_RADIAL_TABLE *Ovl1,DTM_OVERLAP_RADIAL_TABLE *Ovl2,DTM_STR_INT_TAB *IntTable,long IntTableNe,long *IntFlag)
{
 long   dbg=0,ofs1,ofs2 ;
 double Sx,Sy,Ex,Ey ;
 DTM_STR_INT_TAB *pint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersection Of Slope Toe With Radials Or Pad Edge") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Ovl1 = %6ld ** T = %2ld S = %2ld %10.4lf %10.4lf %10.4lf",(long)(Ovl1-OvlPts),Ovl1->Type,Ovl1->Status,Ovl1->Px,Ovl1->Py,Ovl1->Pz) ;
    bcdtmWrite_message(0,0,0,"Ovl2 = %6ld ** T = %2ld S = %2ld %10.4lf %10.4lf %10.4lf",(long)(Ovl2-OvlPts),Ovl2->Type,Ovl2->Status,Ovl2->Px,Ovl2->Py,Ovl2->Pz) ;
   }
/*
** Initialise
*/
 *IntFlag = 0 ;
/*
** Get End Points Of Slope Toe
*/
 ofs1 = (long) (Ovl1-OvlPts) ;
 ofs2 = (long) (Ovl2-OvlPts) ;
 if( ( Ovl1->Gx < Ovl2->Gx ) || ( Ovl1->Gx == Ovl2->Gx && Ovl1->Gy < Ovl2->Gy )) { Sx = Ovl1->Gx ; Sy = Ovl1->Gy ; Ex = Ovl2->Gx ; Ey = Ovl2->Gy ; }
 else                                                                            { Sx = Ovl2->Gx ; Sy = Ovl2->Gy ; Ex = Ovl1->Gx ; Ey = Ovl1->Gy ; }
/*
** Write Offsets And Start End Scan Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Ofs1 = %6ld Ofs2 = %6ld",ofs1,ofs2) ;
        bcdtmWrite_message(0,0,0,"Sx = %10.4lf Sy = %10.4lf",Sx,Sy) ;
        bcdtmWrite_message(0,0,0,"Ex = %10.4lf Ey = %10.4lf",Ex,Ey) ;
   }
/*
** Scan Intersection Table looking For Intersection
*/
  for ( pint = IntTable ; pint < IntTable + IntTableNe &&  pint->X1 <= Ex && ! *IntFlag ; ++pint )
   {
/*
**  Check For Possible Intersection
*/
    if( pint->X2 >= Sx )
          {
/*
**  Do Not Compare With The Same Radial EndPoints
*/
       if( pint->Type != 2 || ( pint->Type == 2 && ( pint->Segment != ofs1 && pint->Segment != ofs2 ) ))
             {
/*
**  Check For Intersection
*/
          *IntFlag = bcdtmMath_checkIfLinesIntersect(Sx,Sy,Ex,Ey,pint->X1,pint->Y1,pint->X2,pint->Y2) ;
          if( dbg && *IntFlag ) bcdtmWrite_message(0,0,0,"Intersecting Element[%5ld] ** S = %6ld T = %2ld D = %2ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
                 }
          }
   }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersection Of Slope Toe With Radials Or Pad Edge Completed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writePadSideSlopeHolesToDataObject
(
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,
 long                     numLeftRadials,
 BC_DTM_OBJ               *sideSlopesP,
 double                   ppTol,
 double                   plTol
)
/*
** This Function Writes The Side Slope Hole Polygons For A Pad
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    sp,numHullPts,numTruncatedRadials ;
 DPoint3d     p3dPts[2],*hullPtsP=nullptr ;
 BC_DTM_OBJ  *dtmP=nullptr ;
 DTM_OVERLAP_RADIAL_TABLE *ovlP,*ovnP ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 2 ) dbg=0 ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Writing Internal Holes To Data Object") ;
    bcdtmWrite_message(0,0,0,"leftRadialsP    = %p",leftRadialsP) ;
    bcdtmWrite_message(0,0,0,"numLeftRadials  = %8ld",numLeftRadials) ;
    bcdtmWrite_message(0,0,0,"sideSlopesP     = %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"Pptol           = %12.8lf",ppTol) ;
    bcdtmWrite_message(0,0,0,"Pltol           = %12.8lf",plTol) ;
   }
/*
** Check For Internal Holes
*/
 numTruncatedRadials= 0 ;
 for( ovlP = leftRadialsP ; ovlP < leftRadialsP + numLeftRadials ; ++ovlP )
   {
    if( ! ovlP->Status ) ++numTruncatedRadials ;
   }
/*
** Write Status For Hole Presence
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numLeftRadials = %6ld numTruncatedRadials = %6ld",numLeftRadials,numTruncatedRadials) ;
    if(  numTruncatedRadials == numLeftRadials ) bcdtmWrite_message(0,0,0,"No Internal Holes") ;
    else                                         bcdtmWrite_message(0,0,0,"Internal Holes Present") ;
   }
/*
** If All Radials Truncated No Internal Holes
*/
 if( numTruncatedRadials == numLeftRadials ) goto cleanup ;
/*
** If No Radials Truncated , Just Write Toe Slopes From Overlay Table
*/
 if( numTruncatedRadials == 0 )
   {
/*
**  Write As Individual Graphic Break Lines For Tin Construction Purpose
*/
    for( ovlP = leftRadialsP ; ovlP < leftRadialsP + numLeftRadials ; ++ovlP )
      {
       ovnP = ovlP + 1 ;
       if( ovnP >= leftRadialsP + numLeftRadials ) ovnP = leftRadialsP ;
       p3dPts[0].x = ovlP->Nx ;
       p3dPts[0].y = ovlP->Ny ;
       p3dPts[0].z = ovlP->Nz ;
       p3dPts[1].x = ovnP->Nx ;
       p3dPts[1].y = ovnP->Ny ;
       p3dPts[1].z = ovnP->Nz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Breakline,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,p3dPts,2)) goto errexit ;
      }
/*
**  Write As Hole And Slope Toe
*/
    for( ovlP = leftRadialsP  ; ovlP < leftRadialsP + numLeftRadials ; ++ovlP )
      {
       if( bcdtmLoad_storePointInCache(ovlP->Nx,ovlP->Ny,ovlP->Nz) ) goto errexit ;
      }
    ovlP = leftRadialsP ;
    if( bcdtmLoad_storePointInCache(ovlP->Nx,ovlP->Ny,ovlP->Nz) ) goto errexit ;
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
** Clean Up And Return
*/
    goto cleanup ;
   }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,2*numLeftRadials,1000) ;
/*
**  Write Slope Toe Data To Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Slope Toes To Data Object") ;
// if( ! processingLimits ) { if( bcdtmSideSlope_writeSlopeToesToDataObject(2,dtmP,leftRadialsP,numLeftRadials) ) goto errexit ; }
 if( ! processingLimits ) { if( bcdtmSideSlope_writeLimitSlopeToesToDataObject(2,dtmP,leftRadialsP,numLeftRadials) ) goto errexit ; }
 else                     { if( bcdtmSideSlope_writeLimitSlopeToesToDataObject(2,dtmP,leftRadialsP,numLeftRadials) ) goto errexit ; }
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"leftSlopeToes.dat") ;
/*
** Write Side Slope Element To Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Pad Edge To Data Object") ;
 for( ovlP = leftRadialsP ; ovlP < leftRadialsP + numLeftRadials ; ++ovlP )
   {
    ovnP = ovlP + 1 ;
    if( ovnP >= leftRadialsP + numLeftRadials ) ovnP = leftRadialsP ;
    if( ovlP->Px != ovnP->Px || ovlP->Py != ovnP->Py )
      {
       p3dPts[0].x = ovlP->Px ;
       p3dPts[0].y = ovlP->Py ;
       p3dPts[0].z = ovlP->Pz ;
       p3dPts[1].x = ovnP->Px ;
       p3dPts[1].y = ovnP->Py ;
       p3dPts[1].z = ovnP->Pz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
      }
   }
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"intSlopeToes.dat") ;
/*
** Triangulate DTM Object
*/
 plTol = ppTol ;
 dtmP->ppTol = ppTol ;
 dtmP->plTol = plTol ;
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"intslopetoes.tin") ;
/*
** Mark Hull Points
*/
 sp = dtmP->hullPoint ;
 do { nodeAddrP(dtmP,sp)->PRGN = 1 ; sp = nodeAddrP(dtmP,sp)->hPtr ; } while ( sp != dtmP->hullPoint ) ;
/*
**  Look For And Write Internal Holes To Data Object
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Extracting Internal Holes ** Processing Limits = %2ld",processingLimits) ;
 if( ! processingLimits ) { if( bcdtmSideSlope_writeInternalPadHolesToDataObject(dtmP,sideSlopesP,2)) goto errexit ; }
 else                     { if( bcdtmSideSlope_writeLimitInternalPadHolesToDataObject(dtmP,sideSlopesP,2)) goto errexit ; }
 if(dbg) bcdtmWrite_message(0,0,0,"Extracting Internal Holes Completed") ;
/*
** Delete DTm Object
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Write Status Message
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Writing Internal Holes To Data Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Writing Internal Holes To Data Object Error") ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Return
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
int bcdtmSideSlope_writeInternalPadHolesToDataObject(BC_DTM_OBJ *Tin,BC_DTM_OBJ *Data,long HoleDirection)
{
 int    ret=0,dbg=0 ;
 long   sp,np,lp,clc,spnt,lpnt,node,nfeat=0,numPolyPts;
 DTMDirection Direction ;
 long   dtmFeature,numFeaturePts,HullCoincidentFlag,NoHoles,numTmpFeatureCodes ;
 double Area ;
 DPoint3d    *featurePtsP=nullptr ;
 DTM_TIN_NODE   *pd ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Sequence Debugging
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 1 ) dbg=0;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Internal Pad Holes To Data Object ** HoleDirection = %1ld",HoleDirection) ;
/*
** Initialise
*/
 NoHoles = 0 ;
 for( sp = 0 ; sp < Tin->numPoints ; ++sp )
   {
    pd = nodeAddrP(Tin,sp) ;
    pd->tPtr = Tin->nullPnt ;
    pd->PRGN = 0 ;
   }
/*
** Remove Dangling Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangles") ;
 if( bcdtmSideSlope_removeDanglingBreaksDtmObject(Tin)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(Tin,L"afterDanglesRemoved.tin") ;
/*
** Write Information On Number Of Features At Internal TIN Point ** Developement Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Possible Hole Start Tin Points") ;
    for( sp = 0 ; sp < Tin->numPoints ; ++sp )
      {
       pd = nodeAddrP(Tin,sp) ;
       if( ! pd->PRGN && pd->hPtr == Tin->nullPnt )
         {
          spnt = sp  ;
          if( bcdtmSideSlope_countNumberOfDtmFeaturesAtPointDtmObject(Tin,spnt,&nfeat)) goto errexit ;
          bcdtmWrite_message(0,0,0,"nfeat = %6ld spnt = %6ld ** %10.4lf %10.4lf %10.4lf",nfeat,spnt,pointAddrP(Tin,spnt)->x,pointAddrP(Tin,spnt)->y,pointAddrP(Tin,spnt)->z) ;
         }
      }
   }
/*
** Scan Tin For Hole Start Points
*/
 for( node = 0 ; node < Tin->numPoints ; ++node )
   {
    pd = nodeAddrP(Tin,node) ;
/*
**  Ignore Point If Previously Processed
*/
    if( ! pd->PRGN && pd->hPtr == Tin->nullPnt )
      {
       spnt = node ;
/*
**     Count Number Of DTM Features At Point
*/
       nfeat = 0 ;
       np = Tin->nullPnt ;
       clc = nodeAddrP(Tin,spnt)->fPtr ;
       while ( clc != Tin->nullPtr )
         {
          ++nfeat ;
          if( nfeat == 2 && flistAddrP(Tin,clc)->nextPnt != Tin->nullPnt ) np = flistAddrP(Tin,clc)->nextPnt ;
          clc = flistAddrP(Tin,clc)->nextPtr ;
         }
      if( dbg ) bcdtmWrite_message(0,0,0,"Point = %6ld ** Num Features = %4ld",node,nfeat) ;
/*
**     If Number Of Features Is Equal To 1 Then Scan Internal To Slope Toe
*/
       if( nfeat == 2 )
         {
          sp = spnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Hole Start Point = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(Tin,sp)->x,pointAddrP(Tin,sp)->y,pointAddrP(Tin,sp)->z ) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Hole Next  Point = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(Tin,np)->x,pointAddrP(Tin,np)->y,pointAddrP(Tin,np)->z ) ;
/*
**        Scan Internal To And Extract Toe Slope Polygon
*/
          numPolyPts = 0 ;
          do
            {
             ++numPolyPts  ;
             nodeAddrP(Tin,sp)->tPtr = np ;
             if( HoleDirection == 2 ) { if(( lp = bcdtmList_nextClkDtmObject(Tin,np,sp)) < 0  ) goto errexit ; }
             else                     { if(( lp = bcdtmList_nextAntDtmObject(Tin,np,sp)) < 0  ) goto errexit ; }
             while ( ! bcdtmList_testForBreakLineDtmObject(Tin,lp,np) )
               {
                if( HoleDirection == 2 ) { if(( lp = bcdtmList_nextClkDtmObject(Tin,np,lp)) < 0  ) goto errexit ; }
                else                     { if(( lp = bcdtmList_nextAntDtmObject(Tin,np,lp)) < 0  ) goto errexit ; }
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"[%4ld] lp = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",numPolyPts,lp,nodeAddrP(Tin,lp)->tPtr,pointAddrP(Tin,lp)->x,pointAddrP(Tin,lp)->y,pointAddrP(Tin,lp)->z) ;
             sp = np ;
             np = lp ;
            } while ( sp != spnt && nodeAddrP(Tin,sp)->tPtr == Tin->nullPnt ) ;
/*
**        Check For Loop Back
*/
          lpnt = Tin->nullPnt ;
          if( sp != spnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Loop Back Detected At Sp = %6ld Tptr = %6ld ** %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(Tin,sp)->tPtr,pointAddrP(Tin,sp)->x,pointAddrP(Tin,sp)->y,pointAddrP(Tin,sp)->z ) ;
             nodeAddrP(Tin,np)->tPtr = sp ;
             lpnt = spnt ;
             spnt = sp   ;
             numPolyPts = 0 ;
             do
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Sp = %6ld Tptr = %6ld",sp,nodeAddrP(Tin,sp)->tPtr) ;
                ++numPolyPts ;
                sp = nodeAddrP(Tin,sp)->tPtr ;
               } while ( sp != spnt ) ;
            }
/*
**        Check Direction Of Tptr Polgon
*/
          if( dbg )
            {
             bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(Tin,spnt,&Area,&Direction) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Direction = %1ld Area = %10.4lf",Direction,Area) ;
            }
/*
**        If Hole Boundary Is Coincident With TIN Hull Ignore Hole
*/
          HullCoincidentFlag = 0 ;
          sp = spnt ;
          do
            {
             np = nodeAddrP(Tin,sp)->tPtr ;
             if( nodeAddrP(Tin,sp)->hPtr == np || nodeAddrP(Tin,np)->hPtr == sp ) ++HullCoincidentFlag  ;
             sp = np ;
            } while ( sp != spnt ) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"HullCoincidentFlag = %6ld numPolyPts = %6ld",HullCoincidentFlag,numPolyPts) ;
          if( HullCoincidentFlag == numPolyPts || ( HoleDirection == 1 && HullCoincidentFlag ) ) numPolyPts = 2 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"numPolyPts = %6ld",numPolyPts) ;
/*
**        Copy Tptr Polygon To Data Object As A Hole
*/
          if( numPolyPts > 2 )
            {
             ++NoHoles ;
             if( bcdtmList_copyTptrListFromDtmObjectToDtmObject(Tin,Data,spnt,DTMFeatureType::Hole,Data->nullUserTag,Data->nullFeatureId)) goto errexit ;
/*
**           Mark Points On Hole
*/
             sp = spnt ;
             do
               {
                nodeAddrP(Tin,sp)->PRGN = 1 ;
                sp = nodeAddrP(Tin,sp)->tPtr ;
               } while ( sp != spnt) ;
            }
/*
**        Null Out Tptr List
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Tptr List") ;
          if( bcdtmList_nullTptrListDtmObject(Tin,spnt)) goto errexit;
/*
**        Null Out Lead To Loop
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Leading Tptr List") ;
          if( lpnt != Tin->nullPnt ) bcdtmList_nullTptrValuesDtmObject(Tin) ;
//          for( sp = Tin->nodesP ; pd < Tin->nodesP + Tin->numPoints ; ++pd ) pd->tPtr = Tin->nullPnt ;
         }
      }
   }
/*
** Validate Holes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Holes = %4ld",NoHoles) ;
 if( NoHoles > 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Holes ** No Holes = %4ld",NoHoles) ;
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes00.dat") ;
// TODO - RobC 30/5/28
//    if( bcdtmData_validateDtmPolygonalFeatureDataObject(Data,11,12)) return(1) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Holes Completed") ;
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes01.dat") ;
   }
/*
** Write Holes As Graphic Break Lines And Slope Toes
*/
 if( NoHoles > 0 )
   {
    numTmpFeatureCodes = Data->numFeatures ;
    for( dtmFeature = 0 ; dtmFeature < numTmpFeatureCodes ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(Data,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(Data,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::GraphicBreak,Data->nullUserTag,1,&Data->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::SlopeToe,Data->nullUserTag,1,&Data->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
         }
      }
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes02.dat") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Internal Pad Holes To Data Object Completed") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( dbg ) bcdtmWrite_message(1,0,0,"Error Writing Internal Pad Holes To Data Object") ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_countNumberOfDtmFeaturesAtPointDtmObject(BC_DTM_OBJ *Tin,long P,long *Count)
/*
** This Function Counts The Number Of DTM Features At Point P
*/
{
 long clc ;
/*
** Initialise
*/
 *Count = 0 ;
/*
** Check Point Range
*/
 if( P < 0 || P > Tin->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
/*
** Scan DTM Feature Lists For Point
*/
 clc = nodeAddrP(Tin,P)->fPtr ;
 while ( clc != Tin->nullPtr )
   {
    ++*Count ;
    clc = flistAddrP(Tin,clc)->nextPtr ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit : return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeDanglingBreaksDtmObject(BC_DTM_OBJ *Tin)
/*
**
** This Function Removes Internal Dangling Break Lines
** A Dangling Break Line Is One That Dosnt Have A Connecting
** Break Line At Either End
**
*/
{
 int  dbg=0 ;
 long p,fPtr,NumberOfBreaks,NumberOfDangles=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Breaks") ;
/*
** Scan Internal Tin Points And Look For Break Points That Are Only
** On One Break Line
*/
 for( p = 0 ; p < Tin->numPoints ; ++p )
   {
    if( nodeAddrP(Tin,p)->hPtr == Tin->nullPnt )
      {
       NumberOfBreaks = 0 ;
       fPtr = nodeAddrP(Tin,p)->fPtr ;
       while( fPtr != Tin->nullPtr )
         {
          if( ftableAddrP(Tin,flistAddrP(Tin,fPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) ++NumberOfBreaks ;
          fPtr = flistAddrP(Tin,fPtr)->nextPtr ;
         }
/*
**     If Point On Only One Break Line, Remove Break Line Segment
*/
       if( NumberOfBreaks == 1 )
         {
          ++NumberOfDangles ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Break At Point %6ld ** %10.4lf %10.4lf %10.4lf",p,pointAddrP(Tin,p)->x,pointAddrP(Tin,p)->y,pointAddrP(Tin,p)->z) ;
          if( bcdtmSideSlope_removeBreakLineSegmentAtTinPointDtmObject(Tin,p)) goto errexit ;
         }
      }
   }
/*
** Write Stats On Number Of Dangles Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dangles Removed = %6ld",NumberOfDangles) ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_countNumberOfDtmFeatureTypeForTinPointDtmObject(BC_DTM_OBJ *Tin,long TinPoint,DTMFeatureType DtmFeatureType,long *NumberOfFeatures)
/*
** This Function Counts The Number Of The DTM Features Types For A Tin Point
** This is a special adoption of the bcdtmList_countNumberOfDtmFeatureTypeForTinPointDtmObject function
** For Pad applications Only and should not be used for any other purposes
**
** Arguements
**
** Tin              ==> Tin Object
** TinPoint         ==> Tin Point Number
** DtmFeatureType   ==> Type Of Dtm Feature To Count
** NumberOfFeatures <== Number Of Features Of DtmFeatureType
**
** Arguement Validation
**
** No Validity Checking Of Arguements
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  20th December 2001
**
*/
{
 long clc,cln,pnt ;
/*
** Initialise
*/
 *NumberOfFeatures = 0 ;
/*
** Count Number Of Features At Point
*/
 if( DtmFeatureType == DTMFeatureType::Hull && nodeAddrP(Tin,TinPoint)->hPtr != DTM_NULL_PNT ) ++*NumberOfFeatures ;
/*
** Scan Feature List For Point
*/
 cln = nodeAddrP(Tin,TinPoint)->fPtr ;
 while( cln != Tin->nullPtr )
   {
    if( ftableAddrP(Tin,flistAddrP(Tin,cln)->dtmFeature)->dtmFeatureType == DtmFeatureType ) ++*NumberOfFeatures ;
    cln = flistAddrP(Tin,cln)->nextPtr ;
   }
/*
** Scan Tin Point For Linear Features For Which Tin Point Is Last Point
*/
 clc = nodeAddrP(Tin,TinPoint)->cPtr ;
 while( clc != Tin->nullPtr )
   {
    pnt = clistAddrP(Tin,clc)->pntNum ;
    clc = clistAddrP(Tin,clc)->nextPtr ;
/*
** Check If Circular List Point Has a Dtm Feature Whose Next Point Is The Tin Point
*/
    cln = nodeAddrP(Tin,pnt)->fPtr ;
    while( cln != Tin->nullPtr )
      {
       if(ftableAddrP(Tin,flistAddrP(Tin,cln)->dtmFeature)->dtmFeatureType == DtmFeatureType && flistAddrP(Tin,cln)->nextPnt == TinPoint ) ++*NumberOfFeatures ;
       cln = flistAddrP(Tin,cln)->nextPtr ;
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmSideSlope_removeBreakLineSegmentAtTinPointDtmObject()          |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeBreakLineSegmentAtTinPointDtmObject(BC_DTM_OBJ *Tin,long P)
/*
** This Function Removes The Break Line Segment At Tin Point P
** Assumes P Is On Only One Break line
*/
{
 long clc,cln,pnt,closeFlag,Feature,LastPoint,NumberOfPoints ;
/*
** Get Break Line Feature For Point
*/
 LastPoint = 0 ;
 Feature = Tin->nullPnt ;
 if(( cln = nodeAddrP(Tin,P)->fPtr ) != Tin->nullPtr ) Feature = flistAddrP(Tin,cln)->dtmFeature ;
 else
   {
    LastPoint = 1 ;
/*
** Scan Tin Point For Which Tin Point Is Last Point In Break Line Feature
*/
    clc = nodeAddrP(Tin,P)->cPtr ;
    while( clc != Tin->nullPtr && Feature == Tin->nullPnt )
      {
       pnt = clistAddrP(Tin,clc)->pntNum ;
       clc = clistAddrP(Tin,clc)->nextPtr ;
/*
** Check If Circular List Point Has a Dtm Feature Whose Next Point Is The Tin Point
*/
       cln = nodeAddrP(Tin,pnt)->fPtr ;
       while( cln != Tin->nullPtr && Feature == Tin->nullPnt )
         {
          if(ftableAddrP(Tin,flistAddrP(Tin,cln)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline && flistAddrP(Tin,cln)->nextPnt == P )
            {
             Feature = flistAddrP(Tin,cln)->dtmFeature ;
            }
          cln = flistAddrP(Tin,cln)->nextPtr ;
         }
      }
   }
/*
** Check Feature Found
*/
 if( Feature == Tin->nullPnt ) { bcdtmWrite_message(2,0,0,"Break Line Feature Not Found") ; goto errexit ; }
/*
** Count Number Of Points In Feature
*/
 bcdtmList_countNumberOfDtmFeaturePointsDtmObject(Tin,Feature,&NumberOfPoints,&closeFlag) ;
/*
** If Number Of Points Is Equal To Two Delete Feature From TIN
*/
 if      ( NumberOfPoints == 2 ) { if( bcdtmInsert_removeDtmFeatureFromDtmObject2(Tin,Feature, false)) goto errexit ; }
 else if ( NumberOfPoints >  2 ) { if( bcdtmInsert_removePointFromDtmFeatureDtmObject(Tin,P,Feature)) goto errexit ; }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*---------------------------------------------------------------------------------+
|                                                                                  |
|   bcdtmSideSlope_extractextractBenchesFromSlopeToesAndStoreInSeparateDataObjects   |
|                                                                                  |
+---------------------------------------------------------------------------------*/
int bcdtmSideSlope_extractBenchesFromSlopeToesAndStoreInSeparateDataObjects(BC_DTM_OBJ *Tin,BC_DTM_OBJ *SideSlopes,BC_DTM_OBJ* **DataObjects,long *NumberOfDataObjects)
/*
** This Function Extracts The Benches From The Slope Toes And Stores Them In Separate Data Objects
**
** Arguements
**
** Tin                 ==>  Tin Object To Determine If Slope Toes Are On Surface Or Not
** SideSlopes          ==>  Data Object Containing The Slope Toes
** DataObjects         <==  Array Of Data Object Pointers To The Data Objects Containing The Separate Benches
** NumberOfDataObjects <==  Number Of Entries In Array Of Data Object Pointers
**
** Author : Rob Cormack
** Date   : 16 January 2002
**
*/
{
 int     ret=0 ;
 long    dbg=0,fc,ss,numCachePts,NumSlopePts,SlopeToeClosed,PointOnSurface,NumTmpolyPtsP,DrapeFlag ;
 DTMDirection    direction;
 long IsPointOnSurface,IsPriorPointOnSurface,IsNextPointOnSurface,AnyPointOnSurface,MemIncDataObjects=10 ;
 DTMUserTag UserTag ;
 static  long MemDataObjects=0 ;
 double  area,Sz ;
 DPoint3d     *p3d,*p3dt,*SlopeToePts=nullptr,*TmpolyPtsP=nullptr ;
 DPoint3d     *cachePtsP=nullptr ;
 BC_DTM_OBJ *SlopeToeObject=nullptr ;
/*
** Write Status Message - Development Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Benches From Slope Toes ** Number Of Data Objects = %4ld",*NumberOfDataObjects) ;
 if( dbg ) bcdtmWrite_toFileDtmObject(SideSlopes,L"benches.dat") ;
/*
** Initialise
*/
 if( *DataObjects == nullptr ) MemDataObjects = 0 ;
/*
** Extract First Set Of Slope Toe Points
*/
 if( bcdtmGeopak_moveFirstOccurrenceOfDtmFeatureTypeToPointArrayDtmObject(SideSlopes,DTMFeatureType::SlopeToe,&UserTag,&SlopeToePts,&NumSlopePts) ) goto errexit ;
 if( NumSlopePts == 0 )
   {
    bcdtmWrite_message(2,0,0,"No Slope Toes Created From Side Slope") ;
    goto errexit ;
   }
/*
**  Set Slope Toe Point Array Polygon Anti Clockwise
*/
 if( SlopeToePts->x == (SlopeToePts+NumSlopePts-1)->x && SlopeToePts->y == (SlopeToePts+NumSlopePts-1)->y )
   {
    bcdtmMath_getPolygonDirectionP3D(SlopeToePts,NumSlopePts,&direction,&area) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Direction = %1ld",direction) ;
    if( direction == DTMDirection::Clockwise ) bcdtmMath_reversePolygonDirectionP3D(SlopeToePts,NumSlopePts) ;
   }
/*
** Set Slope Toe Elevations Within Point To Point Tolerance Of Tin Elevation To Tin Elevation
*/
 for( p3d = SlopeToePts ; p3d < SlopeToePts + NumSlopePts ; ++p3d)
   {
    if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
    if( DrapeFlag && fabs(Sz-p3d->z) < Tin->ppTol )p3d->z = bcdtmMath_roundToDecimalPoints(Sz,8) ;
   }
/*
**  Write Out Slope Toes
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Before Bench Number Of Slope Toe Points = %6ld",NumSlopePts) ;
    for( p3d = SlopeToePts ; p3d < SlopeToePts + NumSlopePts ; ++p3d )
      {
       bcdtmWrite_message(0,0,0,"Slope To Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3d-SlopeToePts),p3d->x,p3d->y,p3d->z) ;
      }
   }
/*
**  Break Slope Toe Into Bench And Ground Sections
*/
 while ( NumSlopePts > 0 )
   {
/*
**  Test For Closure
*/
    SlopeToeClosed = 0 ;
    if( SlopeToePts->x == (SlopeToePts+NumSlopePts-1)->x && SlopeToePts->y == (SlopeToePts+NumSlopePts-1)->y ) SlopeToeClosed = 1 ;
    if( dbg &&   SlopeToeClosed ) bcdtmWrite_message(0,0,0,"Slope Toe Closes") ;
    if( dbg && ! SlopeToeClosed ) bcdtmWrite_message(0,0,0,"Slope Toe Does Not Close") ;
/*
**  Write Out Slope Toes ** Development Only
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Slope Toe Points ** Number = %6ld",NumSlopePts) ;
       for( p3d = SlopeToePts ; p3d < SlopeToePts + NumSlopePts ; ++p3d)
         {
          if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
          PointOnSurface = 0 ;
          if( DrapeFlag && fabs(Sz-p3d->z) < 0.0001 ) PointOnSurface = 1 ;
          bcdtmWrite_message(0,0,0,"Point[%4ld] = %10.4lf %10.4lf %10.4lf ** DrapeFlag = %1ld On Surface = %1ld Surface z = %10.4lf",(long)(p3d-SlopeToePts),p3d->x,p3d->y,p3d->z,DrapeFlag,PointOnSurface,Sz) ;
         }
      }
/*
**  Reorder Closed Slope Toes So That The First Slope Toe Part Section Is Contiguous And On Object
*/
    AnyPointOnSurface = 0 ;
    if( SlopeToeClosed )
      {
       p3d = SlopeToePts + NumSlopePts - 1 ;
       if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
       PointOnSurface = 0 ;
       if( fabs(Sz-p3d->z) < 0.0001 ) PointOnSurface = 1 ;
       IsPointOnSurface = PointOnSurface ;
       while ( p3d > SlopeToePts && IsPointOnSurface == PointOnSurface )
         {
          --p3d ;
          if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
          IsPointOnSurface = 0 ;
          if( fabs(Sz-p3d->z) < 0.0001 ) IsPointOnSurface = 1 ;
         }
       if( IsPointOnSurface != PointOnSurface )
         {
          AnyPointOnSurface = 1 ;
          NumTmpolyPtsP = NumSlopePts - ((long)(p3d-SlopeToePts) + 1 ) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Last Section Offset = %6ld  NumTmpolyPtsP = %6ld",(long)(p3d-SlopeToePts)+1,NumTmpolyPtsP) ;
          TmpolyPtsP = ( DPoint3d * ) malloc( NumSlopePts * sizeof(DPoint3d)) ;
          if( TmpolyPtsP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
          ++p3d ;
          memcpy(TmpolyPtsP,p3d,NumTmpolyPtsP*sizeof(DPoint3d)) ;
          memcpy(TmpolyPtsP+NumTmpolyPtsP-1,SlopeToePts,((long)(p3d-SlopeToePts)+1)*sizeof(DPoint3d)) ;
          *(TmpolyPtsP+NumSlopePts-1) = *TmpolyPtsP ;
          free(SlopeToePts) ;
          SlopeToePts = TmpolyPtsP ;
          TmpolyPtsP = nullptr ;
/*
**        Write Out Slope Toes ** Development Only
*/
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Re Ordered Slope Toe Points ** Number = %6ld",NumSlopePts) ;
             for( p3d = SlopeToePts ; p3d < SlopeToePts + NumSlopePts ; ++p3d)
               {
                if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
                PointOnSurface = 0 ;
                if( DrapeFlag && fabs(Sz-p3d->z) < 0.0001 ) PointOnSurface = 1 ;
                bcdtmWrite_message(0,0,0,"Point[%4ld] = %10.4lf %10.4lf %10.4lf ** DrapeFlag = %1ld On Surface = %1ld Surface z = %10.4lf",(long)(p3d-SlopeToePts),p3d->x,p3d->y,p3d->z,DrapeFlag,PointOnSurface,Sz) ;
               }
            }
         }
      }
/*
**  Create Initial Data Object
*/
    if( bcdtmObject_createDtmObject(&SlopeToeObject)) goto errexit ;
/*
**  Determine If First Slope Toe Point Is On Tin Surface
*/
    p3d = SlopeToePts ;
    if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
    PointOnSurface = 0 ;
    if( fabs(Sz-p3d->z) < 0.0001 ) PointOnSurface = 1 ;
/*
** Scan Slope Toe Points And Place Ground And Bench Sections In Different Data Objects
*/
    fc = 40 ;
    while ( p3d < SlopeToePts + NumSlopePts - AnyPointOnSurface )
      {
       if( bcdtmDrape_pointDtmObject(Tin,p3d->x,p3d->y,&Sz,&DrapeFlag)) goto errexit ;
       IsPointOnSurface = 0 ;
       if( fabs(Sz-p3d->z) < 0.0001 ) IsPointOnSurface = 1 ;
       if( IsPointOnSurface == PointOnSurface )
         {
/*
**        If Start On Surface Write Link To Prior Bench Point First
*/
          if( IsPointOnSurface && fc == 40 )
            {
             if( bcdtmLoad_getCachePoints(&cachePtsP,&numCachePts)) goto errexit ;
             if( numCachePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SlopeToeObject,DTMFeatureType::SlopeToe,SlopeToeObject->nullUserTag,1,&SlopeToeObject->nullFeatureId,cachePtsP,numCachePts)) goto errexit ;
             if( cachePtsP != nullptr ) { free(cachePtsP) ; cachePtsP = nullptr ; }

             if( SlopeToeClosed ||  p3d > SlopeToePts )
               {
                p3dt = p3d - 1 ;
                if( p3dt < SlopeToePts ) p3dt = SlopeToePts + NumSlopePts - 2 ;
                if( bcdtmDrape_pointDtmObject(Tin,p3dt->x,p3dt->y,&Sz,&DrapeFlag)) goto errexit ;
                IsPriorPointOnSurface = 0 ;
                if( fabs(Sz-p3dt->z) < 0.0001 ) IsPriorPointOnSurface = 1 ;
                if( ! IsPriorPointOnSurface )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Offset = %4ld Surface Point = %1ld ** Fc = %1ld  %10.4lf %10.4lf %10.4lf",(long)(p3dt-SlopeToePts),PointOnSurface,fc,p3dt->x,p3dt->y,p3dt->z) ;
//                   if( bcdtmObject_storePointInDataObject(SlopeToeObject,fc,PointOnSurface,nullFeatureId,p3dt->x,p3dt->y,p3dt->z)) goto errexit ;
                   if( bcdtmLoad_storePointInCache(p3dt->x,p3dt->y,p3dt->z)) goto errexit ;
                   fc = 41 ;
                  }
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"Offset = %4ld Surface Point = %1ld ** Fc = %1ld  %10.4lf %10.4lf %10.4lf",(long)(p3d-SlopeToePts),PointOnSurface,fc,p3d->x,p3d->y,p3d->z) ;
//          if( bcdtmObject_storePointInDataObject(SlopeToeObject,fc,PointOnSurface,nullFeatureId,p3d->x,p3d->y,p3d->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(p3d->x,p3d->y,p3d->z)) goto errexit ;
          fc = 41 ;
/*
**        If Close Flag And Second Last Point Is On Surface And Next Point On Bench
**        Write Link To Bench
*/
          if( PointOnSurface && SlopeToeClosed && p3d == SlopeToePts + NumSlopePts - 2 )
            {
             p3dt = p3d + 1 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Offset = %4ld Surface Point = %1ld ** Fc = %1ld  %10.4lf %10.4lf %10.4lf",(long)(p3dt-SlopeToePts),PointOnSurface,fc,p3dt->x,p3dt->y,p3dt->z) ;
//             if( bcdtmObject_storePointInDataObject(SlopeToeObject,fc,PointOnSurface,nullFeatureId,p3dt->x,p3dt->y,p3dt->z)) goto errexit ;
             if( bcdtmLoad_storePointInCache(p3dt->x,p3dt->y,p3dt->z)) goto errexit ;
            }
         }
       else
         {
/*
**        If End On Surface Write Link To Next Point If Next Point On Bench
*/
          if( PointOnSurface )
            {
             if( SlopeToeClosed ||  p3d < SlopeToePts + NumSlopePts - 1 )
               {
                p3dt = p3d + 1 ;
                if( bcdtmDrape_pointDtmObject(Tin,p3dt->x,p3dt->y,&Sz,&DrapeFlag)) goto errexit ;
                IsNextPointOnSurface = 0 ;
                if( fabs(Sz-p3d->z) < 0.0001 ) IsNextPointOnSurface = 1 ;
                if( ! IsNextPointOnSurface )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Offset = %4ld Surface Point = %1ld ** Fc = %1ld  %10.4lf %10.4lf %10.4lf",(long)(p3d-SlopeToePts),PointOnSurface,fc,p3d->x,p3d->y,p3d->z) ;
//                   if( bcdtmObject_storePointInDataObject(SlopeToeObject,fc,PointOnSurface,nullFeatureId,p3d->x,p3d->y,p3d->z)) goto errexit ;
                   if( bcdtmLoad_storePointInCache(p3d->x,p3d->y,p3d->z)) goto errexit ;
                  }
               }
            }
/*
**       Get Slope Toe From cache
*/
         if( bcdtmLoad_getCachePoints(&cachePtsP,&numCachePts)) goto errexit ;
         if( dbg ) bcdtmWrite_message(0,0,0,"**** Number Of Cache Points = %8ld",numCachePts) ;
//         if( numCachePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SlopeToeObject,DTMFeatureType::SlopeToe,SlopeToeObject->nullUserTag,1,&SlopeToeObject->nullFeatureId,cachePtsP,numCachePts)) goto errexit ;
         if( numCachePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SlopeToeObject,DTMFeatureType::SlopeToe,PointOnSurface,1,&SlopeToeObject->nullFeatureId,cachePtsP,numCachePts)) goto errexit ;
         if( cachePtsP != nullptr ) { free(cachePtsP) ; cachePtsP = nullptr ; }
/*
**        Allocate Memory For Data Object Pointer Array
*/
          if( *NumberOfDataObjects == MemDataObjects )
            {
             MemDataObjects = MemDataObjects + MemIncDataObjects ;
             if( *DataObjects == nullptr ) *DataObjects  = ( BC_DTM_OBJ ** ) malloc ( MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             else                       *DataObjects  = ( BC_DTM_OBJ ** ) realloc ( *DataObjects,MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             if( *DataObjects == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
          if( SlopeToeObject->numPoints <= 1 ) bcdtmObject_destroyDtmObject(&SlopeToeObject) ;
          else
            {
             *(*DataObjects+*NumberOfDataObjects) = SlopeToeObject ;
             ++*NumberOfDataObjects ;
            }
/*
**        Reset For Next Section Of Slope Toe
*/
          SlopeToeObject = nullptr ;
          if( bcdtmObject_createDtmObject(&SlopeToeObject)) goto errexit ;
          --p3d ;
          PointOnSurface = IsPointOnSurface ;
          fc = 40 ;
         }
       ++p3d ;
      }
/*
** Store Pointer To Last Data Object
*/
    if( SlopeToeObject != nullptr )
      {
       if( bcdtmLoad_getCachePoints(&cachePtsP,&numCachePts)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Last Number Of Cache Points = %8ld",numCachePts) ;
//       if( numCachePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SlopeToeObject,DTMFeatureType::SlopeToe,SlopeToeObject->nullUserTag,1,&SlopeToeObject->nullFeatureId,cachePtsP,numCachePts)) goto errexit ;
       if( numCachePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(SlopeToeObject,DTMFeatureType::SlopeToe,PointOnSurface,1,&SlopeToeObject->nullFeatureId,cachePtsP,numCachePts)) goto errexit ;
       if( cachePtsP != nullptr ) { free(cachePtsP) ; cachePtsP = nullptr ; }

/*
**     Allocate Memory For Data Object Pointer Array
*/
       if( SlopeToeObject->numPoints <= 1 ) bcdtmObject_destroyDtmObject(&SlopeToeObject) ;
       else
         {
          if( *NumberOfDataObjects == MemDataObjects )
            {
             MemDataObjects = MemDataObjects + MemIncDataObjects ;
             if( *DataObjects == nullptr ) *DataObjects  = ( BC_DTM_OBJ ** ) malloc ( MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             else                       *DataObjects  = ( BC_DTM_OBJ ** ) realloc ( *DataObjects,MemDataObjects * sizeof(BC_DTM_OBJ *)) ;
             if( *DataObjects == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
          *(*DataObjects+*NumberOfDataObjects) = SlopeToeObject ;
          ++*NumberOfDataObjects ;
          SlopeToeObject = nullptr ;
         }
      }
/*
**  Write Out Slope Toes
*/
    if( dbg == 1  )
      {
       bcdtmWrite_message(0,0,0,"**** Number Of Data Object      = %6ld",*NumberOfDataObjects) ;
       bcdtmWrite_message(0,0,0,"**** Number Of Slope Toe Points = %6ld",NumSlopePts) ;
       for( p3d = SlopeToePts ; p3d < SlopeToePts + NumSlopePts ; ++p3d )
         {
          bcdtmWrite_message(0,0,0,"Slope To Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3d-SlopeToePts),p3d->x,p3d->y,p3d->z) ;
         }
      }
/*
** Get Next Slope Toes
*/
    if( bcdtmGeopak_moveFirstOccurrenceOfDtmFeatureTypeToPointArrayDtmObject(SideSlopes,DTMFeatureType::SlopeToe,&UserTag,&SlopeToePts,&NumSlopePts) ) goto errexit ;
   }
/*
** Reallocate memory
*/
 if( *NumberOfDataObjects < MemDataObjects )
   {
    *DataObjects  = ( BC_DTM_OBJ ** ) realloc ( *DataObjects,*NumberOfDataObjects * sizeof(BC_DTM_OBJ *)) ;
    MemDataObjects = *NumberOfDataObjects ;
   }
/*
** Cleanup
*/
 cleanup :
 if( cachePtsP   != nullptr ) { free(cachePtsP) ; cachePtsP = nullptr ; }
 if( SlopeToePts != nullptr ) free(SlopeToePts) ;
 if( TmpolyPtsP  != nullptr ) free(TmpolyPtsP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Extracting Benches From Slope Toes Completed ** Number Of Data Objects = %4ld",*NumberOfDataObjects) ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Extracting Benches From Slope Toes Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( SlopeToeObject != nullptr ) bcdtmObject_destroyDtmObject(&SlopeToeObject) ;
 if( *NumberOfDataObjects > 0 )
   {
    for( ss = 0 ; ss < *NumberOfDataObjects ; ++ss )
      {
       SlopeToeObject = *(*DataObjects+ss) ;
       bcdtmObject_destroyDtmObject(&SlopeToeObject) ;
      }
   }
 *NumberOfDataObjects = 0  ;
 if( *DataObjects != nullptr ) { free(*DataObjects) ; *DataObjects = nullptr ; }
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_markTruncatedRadials(DTM_OVERLAP_RADIAL_TABLE *radialTableP,long radialTableSize)
/*
** This Function Marks Truncated Radials
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 DTM_OVERLAP_RADIAL_TABLE  *radialP ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Truncated Radials") ;
/*
** Mark Truncated Radials
*/
 for( radialP = radialTableP ; radialP < radialTableP + radialTableSize ; ++radialP )
   {
    if( radialP->Gx == radialP->Nx && radialP->Gy == radialP->Ny ) radialP->Status = 1 ;
    else                                                           radialP->Status = 0 ;
   }
/*
** Write Radials
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Radials After Marking Truncated Radials") ;
    for( radialP = radialTableP ; radialP < radialTableP + radialTableSize ; ++radialP )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld]  T = %1ld S = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(radialP-radialTableP),radialP->Type,radialP->Status,radialP->Px,radialP->Py,radialP->Pz) ;
      }
   }
/*
** Job Completed
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_truncateRadialsWithToePointsInsideSlopeToe(DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,long numOvlPts,long direction)
/*
** This Function Truncates Radials With Toe Points Inside Truncated Slope Toes
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 bool   found;
 long intersectFlag ;
 DTM_OVERLAP_RADIAL_TABLE  *ovlP,*ovtP,*radP,*oneP ;
/*
** Set Static Debug Contol For Catching A Particular Side Slope Occurrence In A Sequence
*/
 static long seqdbg=0 ;
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Terminating Radials Within Slope Toe Point Inside Slope Toes") ;
/*
** Write Overlap Radials
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",numOvlPts) ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
      {
       bcdtmWrite_message(0,0,0,"Radial[%4ld] ** T = %2ld S = %2ld Tr = %9ld ** %12.4lf %12.4lf %12.4lf",(long)(ovlP-ovlPtsP),ovlP->Type,ovlP->Status,ovlP->TruncatingRadial,ovlP->Px,ovlP->Py,ovlP->Pz) ;
      }
   }
/*
** Loop Until No Toe Ponts Within Slope Toes Are Found
*/
 found = true ;
 while ( found == true )
   {
    found = false ;
/*
** Scan Overlap Table And Look For Truncated Slope Toes
*/
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts - 1 ; ++ovlP )
      {

       if( ( ovlP->Status == 1 && (ovlP+1)->Status == 0 ) ||
           ( ovlP->Status == 0 && (ovlP+1)->Status == 1 )     )
         {
/*
**  Write Slope Toes
*/
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Slope Toe Found") ;
             bcdtmWrite_message(0,0,0,"Radial = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovlP-ovlPtsP),ovlP->Status,ovlP->Type,ovlP->Px,ovlP->Py,ovlP->Pz) ;
             bcdtmWrite_message(0,0,0,"Radial = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovlP+1-ovlPtsP),(ovlP+1)->Status,(ovlP+1)->Type,(ovlP+1)->Px,(ovlP+1)->Py,(ovlP+1)->Pz) ;
            }
/*
**  Set Non Truncated Radial
*/
          if( ovlP->Status == 1 ) radP = ovlP ;
          else                    radP = ovlP + 1 ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Non Truncated Radial = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(radP-ovlPtsP),radP->Status,radP->Type,radP->Px,radP->Py,radP->Pz) ;
            }
/*
**  Scan Overlap Table And Look For Non Terminating Radial Within Truncated Slope Toe
*/
          for( ovtP = ovlPtsP ; ovtP < ovlPtsP + numOvlPts - 1  ; ++ovtP )
            {
             if( ovtP != radP && (ovtP+1) != radP )
               {
                if( ( ovtP->Status == 1 && (ovtP+1)->Status == 0 ) ||
                    ( ovtP->Status == 0 && (ovtP+1)->Status == 1 )     )
                  {
                   if(  ovtP->TruncatingRadial    != (long)(ovtP-ovlPtsP+1) &&
                       (ovtP+1)->TruncatingRadial != (long)(ovtP-ovlPtsP)       )
                             {
/*
**  Check Slope Toe Does Not Intersect A Type 1 Radial
*/
                      intersectFlag = 0 ;
                      for( oneP = ovlPtsP ; oneP < ovlPtsP + numOvlPts && ! intersectFlag  ; ++oneP )
                        {
                         if( oneP->Type == 1 )
                           {
                            if( bcdtmMath_checkIfLinesIntersect(oneP->Px,oneP->Py,oneP->Nx,oneP->Ny,ovtP->Gx,ovtP->Gy,(ovtP+1)->Gx,(ovtP+1)->Gy) ) intersectFlag = 1 ;
                           }
                        }
/*
**  If No Intersection Then Check For Toe Point
*/
                      if( ! intersectFlag )
                        {
/*
**  Write Slope Toes
*/
                         if( dbg )
                           {
                            bcdtmWrite_message(0,0,0,"**** Testing Against Slope Toe") ;
                            bcdtmWrite_message(0,0,0,"**** Radial = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovtP-ovlPtsP),ovtP->Status,ovtP->Type,ovtP->Px,ovtP->Py,ovtP->Pz) ;
                            bcdtmWrite_message(0,0,0,"**** Radial = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovtP+1-ovlPtsP),(ovtP+1)->Status,(ovtP+1)->Type,(ovtP+1)->Px,(ovtP+1)->Py,(ovtP+1)->Pz) ;
                           }
/*
**  Check If Non Truncated Radial Toe Point Is Within Slope Toe
*/
                         if( direction == 1 )     // Right
                           {
                            if( bcdtmMath_sideOf(ovtP->Px,ovtP->Py,ovtP->Gx,ovtP->Gy,radP->Gx,radP->Gy) > 0 )
                              {
                               if( bcdtmMath_sideOf(ovtP->Gx,ovtP->Gy,(ovtP+1)->Gx,(ovtP+1)->Gy,radP->Gx,radP->Gy) > 0 )
                                 {
                                  if( bcdtmMath_sideOf((ovtP+1)->Gx,(ovtP+1)->Gy,(ovtP+1)->Px,(ovtP+1)->Py,radP->Gx,radP->Gy) > 0 )
                                    {
                                     radP->Status = 0 ;
                                     found = true ;
                                     if( dbg )
                                       {
                                        bcdtmWrite_message(0,0,0,"Right Toe Point Found Inside Slope Toe") ;
                                        bcdtmWrite_message(0,0,0,"Radial = %6ld ** %12.4lf %12.4lf %10.4lf",(long)(ovtP-ovlPtsP),(ovtP+1)->Px,(ovtP+1)->Py,(ovtP+1)->Pz) ;
                                       }
                                    }
                                 }
                              }
                           }
                         if( direction == 2 )   // Left
                           {
                            if( bcdtmMath_sideOf(ovtP->Px,ovtP->Py,ovtP->Gx,ovtP->Gy,radP->Gx,radP->Gy) < 0 )
                              {
                               if( bcdtmMath_sideOf(ovtP->Gx,ovtP->Gy,(ovtP+1)->Gx,(ovtP+1)->Gy,radP->Gx,radP->Gy) < 0 )
                                 {
                                  if( bcdtmMath_sideOf((ovtP+1)->Gx,(ovtP+1)->Gy,(ovtP+1)->Px,(ovtP+1)->Py,radP->Gx,radP->Gy) < 0 )
                                    {
                                     radP->Status = 0 ;
                                     found = true ;
                                     if( dbg )
                                       {
                                        bcdtmWrite_message(0,0,0,"Left Toe Point Found Inside Slope Toe") ;
                                        bcdtmWrite_message(0,0,0,"Radial = %6ld ** %12.4lf %12.4lf %10.4lf",(long)(radP-ovlPtsP),radP->Px,radP->Py,radP->Pz) ;
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
/*
** Scan Overlap Table And Look For Type 1 Radials That Are Truncated By Type 2 Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Type 1 Radials Terminated By Type 2") ;
 for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
   {
    if( ovlP->Type == 1 && ovlP->TruncatingRadial != DTM_NULL_PNT )
      {
       radP = ovlPtsP + ovlP->TruncatingRadial ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Type 2 Radial %6ld Truncates Type 1 Radial %6ld",(long)(radP-ovlPtsP),(long)(ovlP-ovlPtsP)) ;
       if( radP->Type == 2 ) radP->Status = 0 ;
      }
   }

/*
** Write Overlap Radials
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",numOvlPts) ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
      {
       bcdtmWrite_message(0,0,0,"Radial[%4ld] ** Type = %2ld Status = %2ld ** %12.4lf %12.4lf %12.4lf",(long)(ovlP-ovlPtsP),ovlP->Type,ovlP->Status,ovlP->Px,ovlP->Py,ovlP->Pz) ;
      }
   }
/*
** Job Completed
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Terminating Type Two Radials Within Slope Toe Completed") ;
 return(ret) ;
}
/*==============================================================================*//**
* @memo   Copy Parallel 3D a DPoint3d Line String
* @doc    Copy Parallel 3D a DPoint3d Line String
* @notes
* @notes  1. For a closed line string a positive offset copies externally
* @notes     and a negative offset copies internally.
* @notes  2. For an  open line string a positive offset copies to the right
* @notes     and a negative offset copies to the left
* @notes
* @param pointsP                =>  Array  Of DPoint3d Points
* @param numPoints              =>  Number Of DPoint3d Points
* @param offset                 =>  Parallel offset
* @param slope                  =>  Parallel offset Slope
* @param copyMode,              =>  Copy Mode For Convex Corners , Mitre = DTM_MITRE_CORNER , Round = DTM_ROUND_CORNER
* @param cornerStrokeTolerance  =>  Stoke Tolerance For Rounded Corners
* @param parallelPtsPP          <=  Array  Of DPoint3d Parallel Points
* @param numParallelPtsP        <=  Number Of DPoint3d Parallel Points
* @author Rob Cormack February 2004 Rob.Cormack@bentley.com
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_copyParallel3D
(
 DPoint3d    *pointsP,             /* => Array  Of DPoint3d Points           */
 long   numPoints,            /* => Number Of DPoint3d Points           */
 double offset,               /* => Copy Parallel Offset           */
 double slope,                /* => Copy Parallel Slope            */
 long   copyMode,             /* => Copy Mode 1.Mitre = DTM_MITRE_CORNER ,2. Round = DTM_ROUND_CORNER */
 double cornerStrokeTolerance,/* => Stoke Tolerance For Rounded Corners */
 DPoint3d **parallelPtsPP,         /* <= Array  Of DPoint3d Parallel Points  */
 long *numParallelPtsP        /* <= Number Of DPoint3d Parallel Points  */
)
{
 int      ret=DTM_SUCCESS,dbg=0 ;
 long     numKnotPts ;
 DPoint3d      *p3dP ;
 DTM_STR_INT_PTS  *knotPtsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copy Parallel 3D") ;
    bcdtmWrite_message(0,0,0,"pointsP               = %p",pointsP) ;
    bcdtmWrite_message(0,0,0,"numPoints             = %6ld",numPoints) ;
    bcdtmWrite_message(0,0,0,"offset                = %10.4lf",offset) ;
    bcdtmWrite_message(0,0,0,"slope                 = %10.4lf",slope) ;
    bcdtmWrite_message(0,0,0,"copyMode              = %2ld",copyMode) ;
    bcdtmWrite_message(0,0,0,"cornerStrokeTolerance = %10.4lf",cornerStrokeTolerance) ;
    if( dbg == 2 )
      {
       for( p3dP = pointsP ; p3dP < pointsP + numPoints ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-pointsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Validate
*/
 if( pointsP == nullptr )   goto errexit ;
 if( numPoints < 2   )   goto errexit ;
 if( *parallelPtsPP != nullptr ) { free(*parallelPtsPP) ; *parallelPtsPP = nullptr ; }
/*
** Initialise
*/
 *numParallelPtsP = 0 ;
/*
** Normalise Line String Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Normalising") ;
 if( offset != 0.0 ) if( bcdtmSideSlope_normalisePointArray(pointsP,numPoints) ) goto errexit ;
/*
** Remove Duplicate Points From Line String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates") ;
 if( bcdtmSideSlope_removeDuplicatesPointArray(pointsP,&numPoints,0.0)) goto errexit ;
 if( numPoints < 2   )   goto errexit ;
/*
** Check For Knots In String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Knots") ;
 if( bcdtmSideSlope_detectKnotsPointArray(pointsP,numPoints,&knotPtsP,&numKnotPts)) goto errexit ;
 if( numKnotPts > 0 ) goto errexit ;
/*
** Write Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points After Duplicate Removal = %6ld",numPoints) ;
    for( p3dP = pointsP ; p3dP < pointsP + numPoints ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-pointsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Offset Points Parallel
*/
 if( bcdtmSideSlope_offsetCopyPointArray3D(pointsP,numPoints,offset,slope,copyMode,cornerStrokeTolerance,parallelPtsPP,numParallelPtsP) == DTM_ERROR ) goto errexit ;
/*
** Write Parallel Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Copy Parallel Points = %6ld",*numParallelPtsP) ;
    for( p3dP = *parallelPtsPP ; p3dP < *parallelPtsPP + *numParallelPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-*parallelPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Remove Knots From Parallel Points
*/
 if( bcdtmSideSlope_removeKnots(parallelPtsPP,numParallelPtsP,&knotPtsP,&numKnotPts)) goto errexit ;
/*
** Normalise Parallel Points
*/
 if( bcdtmSideSlope_normalisePointArray(*parallelPtsPP,*numParallelPtsP) ) goto errexit ;
/*
** Remove Duplicate Parallel Points
*/
 if( bcdtmSideSlope_removeDuplicatesPointArray(*parallelPtsPP,numParallelPtsP,0.0)) goto errexit ;
/*
** Write Parallel Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Copy Parallel Points With Knots Removed = %6ld",*numParallelPtsP) ;
    for( p3dP = *parallelPtsPP ; p3dP < *parallelPtsPP + *numParallelPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-*parallelPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }

/*
** Set Number Of Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Copy Points = %6ld",*numParallelPtsP) ;
    bcdtmWrite_message(0,0,0,"Start Point = %10.4lf %10.4lf %10.4lf",(*parallelPtsPP)->x,(*parallelPtsPP)->y,(*parallelPtsPP)->z) ;
    bcdtmWrite_message(0,0,0,"End   Point = %10.4lf %10.4lf %10.4lf",(*parallelPtsPP+*numParallelPtsP-1)->x,(*parallelPtsPP+*numParallelPtsP-1)->y,(*parallelPtsPP+*numParallelPtsP-1)->z) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( knotPtsP != nullptr ) { free(knotPtsP) ; knotPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Parallel 3D Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Copying Parallel 3D Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numParallelPtsP = 0 ;
 if( *parallelPtsPP != nullptr ) { free(*parallelPtsPP) ; *parallelPtsPP = nullptr ; }
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Consecutive Duplicate Point Removal Function
* @doc  Generic Line String Consecutive Duplicate Point Removal Function
* @notes  This function removes consecutive duplicate points from a line string
* @notes  Consecutive points within the Point To Point Tolerance will be removed
* @author Rob Cormack 26 February 2003 rob@geopak.com
* @param  *lineString       ==> Pointer To An Array Of Line String Points stored in structure DPoint3d
* @param  *sizeLineString  <==> Number Of Line Strings Points
* @param  pptol            <==  Point To Point Tolerance
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_removeDuplicatesPointArray
(
 DPoint3d      *lineString,
 long          *sizeLineString,
 double        pptol
)
{
 int         ret=DTM_SUCCESS ;
 long        removeFlag  ;
 DPoint3d    *p3d1,*p3d2 ;
/*
** Check For Presence Of Line String
*/
 if( sizeLineString <= 0 || lineString == nullptr ) goto errexit ;
/*
** Check Point To Point Tolerance Is Greater Than Or Equal To Zero
*/
 if( pptol < 0.0 ) pptol = 0.0 ;
/*
** Scan Points And Remove Points Within Point To Point Tolerance
*/
 for( p3d1 = lineString , p3d2 = lineString + 1 ; p3d2 < lineString + *sizeLineString ; ++p3d2 )
   {
/*
** Mark Points To Be Removed
*/
    removeFlag = 0 ;
    if( fabs(p3d1->x - p3d2->x) <= pptol &&  fabs(p3d1->y - p3d2->y) <= pptol )
      {
       if( bcdtmMath_distance(p3d1->x,p3d1->y,p3d2->x,p3d2->y) <= pptol ) removeFlag = 1 ;
      }
/*
** Copy Over Removed Points
*/
    if( ! removeFlag )
      {
       ++p3d1 ;
       if( p3d1 != p3d2 ) *p3d1 = *p3d2 ;
      }
   }
/*
** Reset String Size
*/
 *sizeLineString = (long)(p3d1 - lineString ) + 1 ;
/*
**  Clean Up
*/
 cleanup :
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Knot Detection Function
* @doc  Generic Line String Knot Detection Function
* @notes  This function detects knots in a line string
* @notes  After the first line string segment consecutive Duplicate Line String Points Will Be reported As a knot.
* @notes  Consecutive Duplicate Line String Points can be prior removed with function bcdtmSideSlope_removeDuplicatePoints
* @notes  The number of knots reported will be twice the actual number.
* @notes  The same knot will be reported for each line string segment
* @author Rob Cormack 26 February 2003 rob@geopak.com
* @param  *lineString      ==> Pointer To An Array Of Line String Points stored in structure DPoint3d
* @param  sizeLineString   ==> Number Of Line Strings Pointers
* @param  **knotPoints     <== Pointer To An Array Of Knot Points stored in structure DTM_STR_INT_PTS
* @param  *numknotPoints   <== Number Of Knot Points Detected
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_detectKnotsPointArray
(
 DPoint3d                   *lineString,
 long                  sizeLineString,
 DTM_STR_INT_PTS       **knotPoints,
 long                  *numKnotPoints
)
{
 int                   ret=DTM_SUCCESS ;
 DTM_P3D_LINE_STRING   knotString ;
 DTM_P3D_LINE_STRING*  pknotString ;
/*
** Initialise
*/
 *numKnotPoints = 0 ;
 if( *knotPoints != nullptr ) { free(*knotPoints) ; *knotPoints = nullptr ; }
/*
** Check For Presence Of Line String
*/
 if( lineString == nullptr ) goto errexit ;
 if( sizeLineString <= 2 ) goto cleanup ;
/*
** Check String For Knots
*/
 knotString.stringPts = lineString ;
 knotString.numStringPts = sizeLineString ;
 pknotString = &knotString ;
 if( bcdtmSideSlope_detectStringIntersections(&pknotString,1,knotPoints,numKnotPoints)) goto errexit ;
/*
** Sort Knots On String Segment Number
*/
 if( *numKnotPoints > 0 )
   {
    qsort(*knotPoints,*numKnotPoints,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
   }
/*
**  Clean Up
*/
 cleanup :
 if( *numKnotPoints == 0 && *knotPoints != nullptr ) { free(*knotPoints) ; *knotPoints = nullptr ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numKnotPoints = 0 ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Knot Detection Function
* @doc  Generic Line String Knot Detection Function
* @notes  This function detects and Inserts knots in a line string
* @notes  After the first line string segment consecutive Duplicate Line String Points Will Be reported As a knot.
* @notes  Consecutive Duplicate Line String Points can be prior removed with function bcdtmSideSlope_removeDuplicatePoints
* @notes  The number of knots reported will be twice the actual number.
* @notes  The same knot will be reported for each line string segment
* @author Rob Cormack 26 February 2003 rob@geopak.com
* @param  *lineString      ==> Pointer To An Array Of Line String Points stored in structure DPoint3d
* @param  sizeLineString   ==> Number Of Line Strings Pointers
* @param  **knotPoints     <== Pointer To An Array Of Knot Points stored in structure DTM_STR_INT_PTS
* @param  *numknotPoints   <== Number Of Knot Points Detected
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_insertKnots
(
 DPoint3d                 *lineString,
 long                sizeLineString,
 DTM_STR_INT_PTS     **knotPoints,
 long                *numKnotPoints
)
{
 int                   ret=DTM_SUCCESS ;
 DTM_P3D_LINE_STRING   knotString ;
 DTM_P3D_LINE_STRING*  pknotString ;
/*
** Initialise
*/
 *numKnotPoints = 0 ;
 if( *knotPoints != nullptr ) { free(*knotPoints) ; *knotPoints = nullptr ; }
/*
** Check For Presence Of Line String
*/
 if( lineString == nullptr ) goto errexit ;
 if( sizeLineString <= 2 ) goto cleanup ;
/*
** Check String For Knots
*/
 knotString.stringPts = lineString ;
 knotString.numStringPts = sizeLineString ;
 pknotString = &knotString ;
 if( bcdtmSideSlope_detectStringIntersections(&pknotString,1,knotPoints,numKnotPoints)) goto errexit ;
/*
** Sort Knots On String Segment Number
*/
 if( *numKnotPoints > 0 )
   {
    qsort(*knotPoints,*numKnotPoints,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
/*
** Insert Knot Points Into Line String
*/
   if( bcdtmSideSlope_insertIntersectionPointsIntoLineStrings(&pknotString,*knotPoints,*numKnotPoints) ) goto errexit ;
  }
/*
**  Clean Up
*/
 cleanup :
 if( *numKnotPoints == 0 && *knotPoints != nullptr ) { free(*knotPoints) ; *knotPoints = nullptr ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numKnotPoints = 0 ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Knot Removal Function
* @doc  Generic Line String Knot Removal Function
* @notes  1. This function detects and removes knots from a line string
* @notes  2. The number of knots reported will be twice the actual number.
* @notes  3. The same knot is reported for each line string segment
* @notes  4. Consecutive Duplicate Points are removed prior to knot removal
* @notes  5. The knot points locations returned are after duplicate point removal
* @notes     Therefore the caller should make a copy of the line string prior
* @notes     to calling this function if the caller wishes to insert the knot points
* @author Rob Cormack 28 February 2003 rob@geopak.com
* @param  **linePtsPP     <==> Pointer To An Array Of Line String Points stored in structure DPoint3d
* @param  *numLinePtsP    <==> Number Of Line Strings Pointers
* @param  **knotPtsPP     <==  Pointer To An Array Of Knot Points stored in structure DTM_STR_INT_PTS
* @param  *numknotPtsPP   <==  Number Of Knot Points Detected
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_removeKnots
(
 DPoint3d                 **linePtsPP,
 long                *numLinePtsP,
 DTM_STR_INT_PTS     **knotPtsPP,
 long                *numKnotPtsP
)
{
 int                 ret=DTM_SUCCESS,dbg=0 ;
 DTM_P3D_LINE_STRING knotString,*knotStringP ;
 DTM_STR_INT_PTS     *knotP ;
 DPoint3d                 *p3dP ;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Knotes From LineString") ;
    bcdtmWrite_message(0,0,0,"*linePtsPP   = %p",*linePtsPP) ;
    bcdtmWrite_message(0,0,0,"*numLinePtsP = %8ld",*numLinePtsP) ;
   }
/*
** Initialise
*/
 *numKnotPtsP = 0 ;
 if( *knotPtsPP != nullptr ) { free(*knotPtsPP) ; *knotPtsPP = nullptr ; }
/*
** Check For Presence Of Line String
*/
 if( *linePtsPP == nullptr ) goto errexit ;
 if( *numLinePtsP <= 2 ) goto cleanup ;
/*
** Remove Duplicate Points
*/
 if( bcdtmSideSlope_removeDuplicatesPointArray(*linePtsPP,numLinePtsP,0.0)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"*numLinePtsP = %8ld",*numLinePtsP) ;
 if( *numLinePtsP <= 2 ) goto cleanup ;
/*
** Check String For Knots
*/
 knotString.stringPts = *linePtsPP ;
 knotString.numStringPts = *numLinePtsP ;
 knotStringP = &knotString ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Line String For Knots") ;
 if( bcdtmSideSlope_detectStringIntersections(&knotStringP,1,knotPtsPP,numKnotPtsP)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Knots Detected = %6ld",*numKnotPtsP) ;
    for( knotP = *knotPtsPP ; knotP < *knotPtsPP + *numKnotPtsP ; ++knotP )
      {
       bcdtmWrite_message(0,0,0,"Knot[%6ld] ** Seg1 = %4ld Seg2 = %4ld Dist = %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(knotP-*knotPtsPP),knotP->Segment1,knotP->Segment2,knotP->Distance,knotP->x,knotP->y,knotP->z) ;
      }
   }
/*
** Process Knots
*/
 if( *numKnotPtsP > 0 )
   {
/*
** Average Intersection z Coordiante Values
*/
    for( knotP = *knotPtsPP ; knotP < *knotPtsPP + *numKnotPtsP ; ++knotP )
      {
       knotP->z = knotP->Z2 = ( knotP->z + knotP->Z2 ) / 2.0 ;
      }
/*
** Sort Knots On String Segment Number
*/
    qsort(*knotPtsPP,*numKnotPtsP,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
/*
** Write Out Sorted Knots
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Sorted Knots = %6ld",*numKnotPtsP) ;
       for( knotP = *knotPtsPP ; knotP < *knotPtsPP + *numKnotPtsP ; ++knotP )
         {
          bcdtmWrite_message(0,0,0,"Knot[%6ld] ** Seg1 = %4ld Seg2 = %4ld Dist = %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(knotP-*knotPtsPP),knotP->Segment1,knotP->Segment2,knotP->Distance,knotP->x,knotP->y,knotP->z) ;
         }
      }
/*
** Insert Knot Points Into Line String
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Knots Into Line String") ;
   if( bcdtmSideSlope_insertIntersectionPointsIntoLineStrings(&knotStringP,*knotPtsPP,*numKnotPtsP) ) goto errexit ;
   if( dbg )
     {
      bcdtmWrite_message(0,0,0,"Number Of Line String Points After Knot Insertion = %6ld",knotString.numStringPts) ;
      for( p3dP = knotString.stringPts ; p3dP < knotString.stringPts + knotString.numStringPts ; ++ p3dP )
        {
         bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-knotString.stringPts),p3dP->x,p3dP->y,p3dP->z) ;
        }
     }
/*
**  Remove Knots From String
*/
   *linePtsPP = knotString.stringPts ;
   *numLinePtsP = knotString.numStringPts ;
   if( dbg ) bcdtmWrite_message(0,0,0,"Removing Knots From Line String") ;
   if( bcdtmSideSlope_remove3DKnotsFromLineString(linePtsPP,numLinePtsP,*knotPtsPP,*numKnotPtsP) ) goto errexit ;
  }
/*
**  Clean Up
*/
 cleanup :
 if( *numKnotPtsP == 0 && *knotPtsPP != nullptr ) { free(*knotPtsPP) ; *knotPtsPP = nullptr ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numKnotPtsP = 0 ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Intersection Function
* @doc  Generic Line String Intersection Function
* @notes 1. This function determines the intersection points of a set of line strings
* @notes 2. The number of intersection points returned is twice the actual number
* @notes 3. The same intersection point is returned for each intersecting line strings
* @author Rob Cormack 24 February 2003 rob@geopak.com
* @param  *lineStrings      ==> Pointer To An Array Of Line String Pointers stored in structure DTM_P3D_LINE_STRING
* @param  numLineStrings    ==> Number Of Line Strings Pointers
* @param  **intPoints       <== Pointer To An Array Of Intersection Points stored in structure DTM_STR_INT_PTS
* @param  *numIntPoints     <== Number Of Intersection Points
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_detectStringIntersections
(
 DTM_P3D_LINE_STRING* *lineStrings,
 long                 numLineStrings,
 DTM_STR_INT_PTS      **intPoints,
 long                 *numIntPoints
)
{
 int                ret=DTM_SUCCESS,dbg=0 ;
 long               intTableSize=0,memIntPoints=0,memIntPointsInc=5000 ;
 DTM_STR_INT_TAB  *intTable=nullptr ;
/*
** Initialise
*/
 *numIntPoints = 0 ;
 if( *intPoints != nullptr ) { free(*intPoints) ; *intPoints = nullptr ; }
/*
** Check For Presence Of Line Strings
*/
 if( numLineStrings <= 0 || lineStrings == nullptr ) goto errexit ;
/*
** Build Line String Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Line String Intersection Table") ;
 if( bcdtmSideSlope_buildD3dLineStringIntersectionTable(lineStrings,numLineStrings,&intTable,&intTableSize) ) goto errexit ;
/*
** Scan For Line String Intersections
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Line String Intersections") ;
 if( bcdtmSideSlope_scanForStringLineIntersections(intTable,intTableSize,intPoints,numIntPoints,&memIntPoints,memIntPointsInc)) goto errexit ;
/*
** Reallocate Memory For Intersections Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory") ;
 if( *numIntPoints > 0 && *numIntPoints != memIntPoints ) *intPoints = ( DTM_STR_INT_PTS * ) realloc( *intPoints , *numIntPoints * sizeof( DTM_STR_INT_PTS )) ;
/*
** Sort Intersection Points On String Number
*/
 if( *numIntPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Points") ;
    qsort(*intPoints,*numIntPoints,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
   }
/*
**  Clean Up
*/
 cleanup :
 if( intTable != nullptr ) free(intTable) ;
 if( *numIntPoints == 0 && *intPoints != nullptr ) { free(*intPoints) ; *intPoints = nullptr ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numIntPoints = 0 ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Intersection Function
* @doc  Generic Line String Intersection Function
* @notes 1. This function determines the intersection points of a set of line strings
*           and inserts the intersection points into the strings
* @notes 2. The number of intersection points returned is twice the actual number
* @notes 3. The same intersection point is returned for each intersecting line strings
* @author Rob Cormack 24 February 2003 rob@geopak.com
* @param  *lineStrings      ==> Pointer To An Array Of Line String Pointers stored in structure DTM_P3D_LINE_STRING
* @param  numLineStrings    ==> Number Of Line Strings Pointers
* @param  **intPoints       <== Pointer To An Array Of Intersection Points stored in structure DTM_STR_INT_PTS
* @param  *numIntPoints     <== Number Of Intersection Points
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
int bcdtmSideSlope_insertStringIntersections
(
 DTM_P3D_LINE_STRING*        *lineStrings,
 long                     numLineStrings,
 DTM_STR_INT_PTS       **intPoints,
 long                     *numIntPoints
)
{
 int                ret=DTM_SUCCESS ;
 long               intTableSize=0,memIntPoints=0,memIntPointsInc=5000 ;
 DTM_STR_INT_TAB  *intTable=nullptr ;
/*
** Initialise
*/
 *numIntPoints = 0 ;
 if( *intPoints != nullptr ) { free(*intPoints) ; *intPoints = nullptr ; }
/*
** Check For Presence Of Line Strings
*/
 if( numLineStrings <= 0 || lineStrings == nullptr ) goto errexit ;
/*
** Build Line String Table
*/
 if( bcdtmSideSlope_buildD3dLineStringIntersectionTable(lineStrings,numLineStrings,&intTable,&intTableSize) ) goto errexit ;
/*
** Scan For Line String Intersections
*/
 if( bcdtmSideSlope_scanForStringLineIntersections(intTable,intTableSize,intPoints,numIntPoints,&memIntPoints,memIntPointsInc)) goto errexit ;
/*
** Reallocate Memory For Intersections Points
*/
 if( *numIntPoints > 0 && *numIntPoints != memIntPoints ) *intPoints = ( DTM_STR_INT_PTS * ) realloc( *intPoints , *numIntPoints * sizeof( DTM_STR_INT_PTS )) ;
/*
** Process Intersection Points
*/
 if( *numIntPoints > 0 )
   {
/*
** Sort Intersection Points On String Number
*/
    qsort(*intPoints,*numIntPoints,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
/*
** Insert Intersertion Points Into Line Strings
*/
   if( bcdtmSideSlope_insertIntersectionPointsIntoLineStrings(lineStrings,*intPoints,*numIntPoints) ) goto errexit ;
   }
/*
**  Clean Up
*/
 cleanup :
 if( intTable != nullptr ) free(intTable) ;
 if( *numIntPoints == 0 && *intPoints != nullptr ) { free(*intPoints) ; *intPoints = nullptr ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 *numIntPoints = 0 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmSideSlope_buildD3dLineStringIntersectionTable                |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmSideSlope_buildD3dLineStringIntersectionTable(DTM_P3D_LINE_STRING* *lineStrings,long numLineStrings,DTM_STR_INT_TAB **intTable,long *intTableSize)
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   numString,numStringPts,intTableMe,intTableMinc,CloseFlag  ;
 double cord ;
 DPoint3d          *p3d,*stringPts ;
 DTM_P3D_LINE_STRING* *pline ;
 DTM_STR_INT_TAB *pint  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Line String Intersection Table") ;
/*
** Initialise
*/
 *intTableSize = intTableMe = 0 ;
 if( *intTable != nullptr ) { free(*intTable) ; *intTable = nullptr ; }
/*
** Determine Size Of Line String Intersection Table
*/
 intTableMinc = 0 ;
 for( pline = lineStrings ; pline < lineStrings + numLineStrings ; ++pline )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Line String[%4ld] ** numStringPts = %6ld",(long)(pline-lineStrings), (*pline)->numStringPts) ;
    intTableMinc = intTableMinc + (*pline)->numStringPts - 1 ;
   }
/*
** Scan Line Strings And Build Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Line Strings And Building Intersection Table") ;
 for( pline = lineStrings ; pline < lineStrings + numLineStrings ; ++pline )
   {
/*
** Set String Parameters
*/
    numString    = (long)(pline-lineStrings) ;
    stringPts    = (*pline)->stringPts ;
    numStringPts = (*pline)->numStringPts ;
/*
** Check For Closure
*/
    CloseFlag = 0 ;
    if( stringPts->x == (stringPts+numStringPts-1)->x && stringPts->y == (stringPts+numStringPts-1)->y ) CloseFlag = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"CloseFlag = %2ld",CloseFlag) ;
/*
** Store String Segments In Intersection Table
*/
    for( p3d = stringPts ; p3d < stringPts + numStringPts - 1 ; ++p3d )
      {
/*
**  Check For Memory Allocation
*/
       if( *intTableSize == intTableMe )
         {
          intTableMe = intTableMe + intTableMinc ;
          if( *intTable == nullptr ) *intTable = ( DTM_STR_INT_TAB * ) malloc ( intTableMe * sizeof(DTM_STR_INT_TAB)) ;
          else                    *intTable = ( DTM_STR_INT_TAB * ) realloc ( *intTable,intTableMe * sizeof(DTM_STR_INT_TAB)) ;
          if( *intTable == nullptr ) goto errexit ;
         }
/*
**  Store String Line
*/
       (*intTable+*intTableSize)->String  = numString ;
       (*intTable+*intTableSize)->Segment = (long)(p3d-stringPts) ;
       if( ( p3d == stringPts && CloseFlag ) || ( p3d == stringPts + numStringPts - 2 && CloseFlag ) ) (*intTable+*intTableSize)->Type = 2   ;
       else                                                                                            (*intTable+*intTableSize)->Type = 1   ;
       (*intTable+*intTableSize)->Direction = 1 ;
       (*intTable+*intTableSize)->X1 = p3d->x ;
       (*intTable+*intTableSize)->Y1 = p3d->y ;
       (*intTable+*intTableSize)->Z1 = p3d->z ;
       (*intTable+*intTableSize)->X2 = (p3d+1)->x ;
       (*intTable+*intTableSize)->Y2 = (p3d+1)->y ;
       (*intTable+*intTableSize)->Z2 = (p3d+1)->z ;
       ++*intTableSize ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Line Strings And Building Intersection Table Completed") ;
/*
** Reallocate Intersection Table Memory
*/
 if( *intTableSize != intTableMe ) *intTable = ( DTM_STR_INT_TAB * ) realloc ( *intTable, *intTableSize * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Ordering Line Coordinates") ;
 for( pint = *intTable ; pint < *intTable + *intTableSize ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Qsorting Line Coordinates") ;
 qsort(*intTable,*intTableSize,sizeof(DTM_STR_INT_TAB),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionTableCompareFunction) ;
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
 *intTableSize = 0 ;
 if( *intTable != nullptr ) { free(*intTable) ; *intTable = nullptr ; }
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_intersectionTableCompareFunction(const DTM_STR_INT_TAB *Tp1,const DTM_STR_INT_TAB *Tp2)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  Tp1->X1  < Tp2->X1 ) return(-1) ;
 else if(  Tp1->X1  > Tp2->X1 ) return( 1) ;
 else if(  Tp1->Y1  < Tp2->Y2 ) return(-1) ;
 else if(  Tp1->Y1  > Tp2->Y2 ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_scanForStringLineIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Base Line Intersections
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Line String Intersections") ;
/*
** Scan Sorted Line String Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmSideSlope_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmSideSlope_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmSideSlope_determineActiveStringLineIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != nullptr ) free(ActIntTable) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Line String Intersections Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Line String Intersections Error") ;
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
int bcdtmSideSlope_deleteActiveStringLines(DTM_STR_INT_TAB *ActIntTable,long *ActIntTableNe,DTM_STR_INT_TAB *Pint)
/*
** This Functions Deletes Entries From The Active Line Intersection List
*/
{
 long              cnt=0 ;
 DTM_STR_INT_TAB *pal1,*pal2 ;
/*
** Scan Active Line List And Mark Entries For Deletion
*/
 for ( pal1 = ActIntTable ; pal1 < ActIntTable + *ActIntTableNe ; ++pal1 )
   {
    if( pal1->X2 < Pint->X1 ) { pal1->String = -9999 ; ++cnt ; }
   }
 if( cnt == 0 ) return(0) ;
/*
** Delete Marked Entries
*/
 if( cnt > 0 )
   {
    for( pal1 = pal2 = ActIntTable ; pal2 < ActIntTable + *ActIntTableNe ; ++pal2 )
      {
       if( pal2->String != -9999 )
         {
          if( pal1 != pal2 ) *pal1 = *pal2 ;
          ++pal1 ;
         }
      }
   }
/*
** Reset Number Of Active Entries
*/
 *ActIntTableNe = *ActIntTableNe - cnt ;
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
int bcdtmSideSlope_addActiveStringLine(DTM_STR_INT_TAB **ActIntTable,long *ActIntTableNe,long *ActIntTableMe,DTM_STR_INT_TAB *Pint)
/*
** This Functions Adds An Entry To The Active Line List
*/
{
 long MemInc=100 ;
/*
** Test For Memory
*/
 if( *ActIntTableNe == *ActIntTableMe )
   {
    *ActIntTableMe = *ActIntTableMe + MemInc ;
    if( *ActIntTable == nullptr ) *ActIntTable = ( DTM_STR_INT_TAB * ) malloc ( *ActIntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    else                       *ActIntTable = ( DTM_STR_INT_TAB * ) realloc( *ActIntTable, *ActIntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    if( *ActIntTable == nullptr )  return(1) ;
   }
/*
** Store Entry
*/
 *(*ActIntTable+*ActIntTableNe) = *Pint ;
 ++*ActIntTableNe ;
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
int bcdtmSideSlope_determineActiveStringLineIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc )
/*
** Determine Line Intersections
*/
{
 double             di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y ;
 DTM_STR_INT_TAB  *alp,*slp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
** Check Lines Are Not Consecutive Segments Of The Same Line String
*/
    if( alp->String != slp->String || ( alp->String == slp->String && labs(alp->Segment-slp->Segment) > 1 ) )
      {
/*
**  Check Lines Do Not Close Line String
*/
       if( alp->String != slp->String || ( alp->Type != 2 || slp->Type != 2 ) )
         {
/*
** Check Lines Intersect
*/
          if( bcdtmSideSlope_checkIfLinesIntersect(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2))
            {
/*
** Intersect Lines
*/
             bcdtmSideSlope_normalIntersectLines(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2,&x,&y) ;
/*
** Check Memory
*/
             if( *IntPtsNe + 1 >= *IntPtsMe )
               {
                *IntPtsMe = *IntPtsMe + IntPtsMinc ;
                if( *IntPts == nullptr ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                if( *IntPts == nullptr )  goto errexit ;
               }
/*
** Calculate Distances For Alp
*/
             if( alp->Direction == 1 ) { Xs = alp->X1 ; Ys = alp->Y1 ; Zs = alp->Z1 ; Xe = alp->X2 ; Ye = alp->Y2 ; Ze = alp->Z2 ; }
             if( alp->Direction == 2 ) { Xs = alp->X2 ; Ys = alp->Y2 ; Zs = alp->Z2 ; Xe = alp->X1 ; Ye = alp->Y1 ; Ze = alp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point Alp
*/
             (*IntPts+*IntPtsNe)->String1  = alp->String  ;
             (*IntPts+*IntPtsNe)->Segment1 = alp->Segment ;
             (*IntPts+*IntPtsNe)->String2  = slp->String  ;
             (*IntPts+*IntPtsNe)->Segment2 = slp->Segment ;
             (*IntPts+*IntPtsNe)->Distance = di ;
             (*IntPts+*IntPtsNe)->x = x ;
             (*IntPts+*IntPtsNe)->y = y ;
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
             ++*IntPtsNe ;
/*
** Calculate Distances For Slp
*/
             if( slp->Direction == 1 ) { Xs = slp->X1 ; Ys = slp->Y1 ; Zs = slp->Z1 ; Xe = slp->X2 ; Ye = slp->Y2 ; Ze = slp->Z2 ; }
             if( slp->Direction == 2 ) { Xs = slp->X2 ; Ys = slp->Y2 ; Zs = slp->Z2 ; Xe = slp->X1 ; Ye = slp->Y1 ; Ze = slp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point For Slp
*/
             (*IntPts+*IntPtsNe)->String1  = slp->String  ;
             (*IntPts+*IntPtsNe)->Segment1 = slp->Segment ;
             (*IntPts+*IntPtsNe)->String2  = alp->String  ;
             (*IntPts+*IntPtsNe)->Segment2 = alp->Segment ;
             (*IntPts+*IntPtsNe)->Distance = di ;
             (*IntPts+*IntPtsNe)->x = x ;
             (*IntPts+*IntPtsNe)->y = y ;
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ;
             ++*IntPtsNe ;
/*
** Store Z2 Values
*/
             (*IntPts+*IntPtsNe-2)->Z2 = (*IntPts+*IntPtsNe-1)->z ;
             (*IntPts+*IntPtsNe-1)->Z2 = (*IntPts+*IntPtsNe-2)->z ;
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_checkIfLinesIntersect(double x1,double y1,double x2,double y2,double x3,double y3,double x4,double y4)
/*
** This Function Checks If Two Lines Intersect
** Return Values  ==  0  No Intersection
**                ==  1  Intersection
**
*/
{
 int    sd1,sd2 ;
 double xn1,xm1,yn1,ym1,xn2,xm2,yn2,ym2 ;
/*
** Check If Bounding Rectangles Overlap
*/
 if( x1 <= x2 ) { xn1 = x1 ; xm1 = x2 ; } else { xn1 = x2 ; xm1 = x1 ; }
 if( y1 <= y2 ) { yn1 = y1 ; ym1 = y2 ; } else { yn1 = y2 ; ym1 = y1 ; }
 if( x3 <= x4 ) { xn2 = x3 ; xm2 = x4 ; } else { xn2 = x4 ; xm2 = x3 ; }
 if( y3 <= y4 ) { yn2 = y3 ; ym2 = y4 ; } else { yn2 = y4 ; ym2 = y3 ; }
 if( xn1 > xm2 || xm1 < xn2 || yn1 > ym2 || ym1 < yn2 ) return(0) ;
/*
** Calculate SideOf Values
*/
 sd1 = bcdtmSideSlope_sideOf(x3,y3,x4,y4,x1,y1) ;
 sd2 = bcdtmSideSlope_sideOf(x3,y3,x4,y4,x2,y2) ;
 if( sd1 == sd2 && sd1 != 0 ) return(0) ;
 sd1 = bcdtmSideSlope_sideOf(x1,y1,x2,y2,x3,y3) ;
 sd2 = bcdtmSideSlope_sideOf(x1,y1,x2,y2,x4,y4) ;
 if( sd1 == sd2 && sd1 != 0 ) return(0) ;
/*
** Lines Intersect
*/
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_sideOf(double x1,double y1,double x2,double y2,double x3,double y3)
{
 int ret=DTM_SUCCESS ;
 double sdof ;
 sdof = (x1-x3) * (y2-y3) - (y1-y3) * (x2-x3) ;
 if( sdof <  0.0 ) ret = -1 ; /* Right of Line */
 if( sdof == 0.0 ) ret =  0 ; /* On Line       */
 if( sdof >  0.0 ) ret =  1 ; /* Left of Line  */
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_normalIntersectLines(double X1,double Y1,double X2,double Y2,double X3,double Y3,double X4,double Y4,double *Xi,double *Yi)
/*
** This Function Calculates The Intersect Point Of Two Lines
*/
{
 double  n1,n2 ;
/*
** Check For Coincident End Points
*/
 if( X3 == X1 && Y3 == Y1 ) { *Xi = X1 ; *Yi = Y1 ; return(0) ; }
 if( X3 == X2 && Y3 == Y2 ) { *Xi = X2 ; *Yi = Y2 ; return(0) ; }
 if( X4 == X1 && Y4 == Y1 ) { *Xi = X1 ; *Yi = Y1 ; return(0) ; }
 if( X4 == X2 && Y4 == Y2 ) { *Xi = X2 ; *Yi = Y2 ; return(0) ; }
/*
** Calculate Variables
*/
 n1  = fabs(bcdtmSideSlope_normalDistanceToLine(X1,Y1,X2,Y2,X3,Y3)) ;
 n2  = fabs(bcdtmSideSlope_normalDistanceToLine(X1,Y1,X2,Y2,X4,Y4)) ;
 if( (n1+n2) != 0.0 )
   {
    *Xi = X3 + (X4-X3) * (n1/(n1+n2)) ;
    *Yi = Y3 + (Y4-Y3) * (n1/(n1+n2)) ;
   }
 else
   {
    n1 = bcdtmMath_distance(X1,Y1,X3,Y3) ;
    n2 = bcdtmMath_distance(X1,Y1,X4,Y4) ;
    if( n1 <= n2 ) { *Xi = X3 ; *Yi = Y3 ; }
    else           { *Xi = X4 ; *Yi = Y4 ; }
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
double bcdtmSideSlope_normalDistanceToLine(double X1,double Y1,double X2,double Y2, double x, double y )
/*
** This Function Return the Normal Distance Of a Point From A Line
*/
{
 double r,d,dx,dy,a1,a2,a3 ;
/*
** Initialise Variables
*/
 d = 0.0 ;
 dx = X2 - X1 ;
 dy = Y2 - Y1 ;
 r  = sqrt( dx*dx + dy*dy) ;
 if( r > 0.0 )
   {
    a1 =  dy / r ;
    a2 = -dx / r ;
    a3 = -a1 * X1 - a2 * Y1 ;
    d  = a1 * x + a2 * y + a3 ;
   }
 else d = sqrt( (x-X1)*(x-X1) + (y-Y1)*(y-Y1) ) ;
/*
** Job Completed
*/
 return(d) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_intersectionPointsCompareFunction(const DTM_STR_INT_PTS *Tp1,const DTM_STR_INT_PTS  *Tp2)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  Tp1->String1 < Tp2->String1  ) return(-1) ;
 else if(  Tp1->String1 > Tp2->String1  ) return( 1) ;
 else if( Tp1->Segment1 < Tp2->Segment1 ) return(-1) ;
 else if( Tp1->Segment1 > Tp2->Segment1 ) return( 1) ;
 else if( Tp1->Distance < Tp2->Distance ) return(-1) ;
 else if( Tp1->Distance > Tp2->Distance ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmSideSlope_insertIntersectionPointsIntoLineStrings(DTM_P3D_LINE_STRING* *lineStrings,DTM_STR_INT_PTS *intPoints,long numIntPoints)
{
 int    ret=DTM_SUCCESS ;
 long   numInts,numStringPts,strSegNo ;
 DPoint3d   *p3d1,*p3d2,*string=nullptr ;
 DTM_P3D_LINE_STRING*  *pline ;
 DTM_STR_INT_PTS *pinp,*ninp  ;
/*
** Initialie
*/
 pline = lineStrings ;
/*
** Scan Intersection Points
*/
 for( pinp = intPoints ; pinp < intPoints + numIntPoints ; ++pinp )
   {
/*
** Get First And Last Entry For String In Intersection Points Table
*/
    ninp = pinp + 1 ;
    while ( ninp < intPoints + numIntPoints && ninp->String1 == pinp->String1 ) ++ninp ;
    --ninp ;
/*
** Set Number Of Intersection Points
*/
    numInts = (long)( ninp - pinp ) + 1 ;
/*
** Set Pointer To Entry In String Line Table For Intersected String
*/
    pline = lineStrings + pinp->String1 ;
/*
** Allocate Memory For New Extended String
*/
    numStringPts = (*pline)->numStringPts + numInts ;
    string = ( DPoint3d *) malloc ( numStringPts * sizeof(DPoint3d)) ;
    if( string == nullptr ) goto errexit ;
/*
** Copy Old String And Intersection Points To New String
*/
    p3d2  = string ;
    for( p3d1 = (*pline)->stringPts , strSegNo = 0 ; p3d1 < (*pline)->stringPts + (*pline)->numStringPts ; ++p3d1 , ++strSegNo )
      {
/*
**     Copy String Point
*/
       *p3d2 = *p3d1 ;
       ++p3d2 ;
/*
** Copy Intersection Points
*/
       while( pinp <= ninp && pinp->Segment1 == strSegNo )
         {
          p3d2->x = pinp->x  ;
          p3d2->y = pinp->y  ;
          p3d2->z = pinp->z  ;
          ++p3d2 ;
          ++pinp ;
         }
      }
/*
** Reset String Size
*/
   if( (long)(p3d2-string) < numStringPts )
     {
      numStringPts = (long)(p3d2-string) ;
      string = ( DPoint3d *) realloc ( string,numStringPts * sizeof(DPoint3d)) ;
     }
/*
** Remove Duplicate Intersection Points From String
*/
   bcdtmSideSlope_removeDuplicatesPointArray(string,&numStringPts,0.0) ;
/*
** Replace Existing Line String with Line String With Inserted Intersection Points
*/
    free((*pline)->stringPts) ;
    (*pline)->stringPts = string ;
    (*pline)->numStringPts = numStringPts ;
    string = nullptr ;
    numStringPts = 0 ;
/*
** Reset For Next String
*/
    pinp = ninp ;
   }
/*
** Clean Up
*/
 cleanup :
 if( string != nullptr ) free(string) ;
/*
** Job Completed
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
int  bcdtmSideSlope_remove3DKnotsFromLineString(DPoint3d **linePtsPP,long *numLinePtsP,DTM_STR_INT_PTS *intPtsP,long numIntPts)
{
 int                ret=DTM_SUCCESS,dbg=0 ;
 long               *lP,*l1P,*l2P,*l3P,*l4P,loop,process,*knotFlagP=nullptr,numKnotPts,numKnots,closeFlag ;
 double             knotLength,startLength,endLength ;
 DPoint3d           *p3dP ;
 DTM_STR_INT_PTS *inpP,*inp1P,*inp2P,*knotPtsP=nullptr  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Knots From Line String") ;
/*
** Make A Local Copy Of The Knots
*/
 numKnotPts = numIntPts  ;
 knotPtsP = ( DTM_STR_INT_PTS * ) malloc( numKnotPts * sizeof(DTM_STR_INT_PTS)) ;
 if( knotPtsP == nullptr ) goto errexit ;
/*
** Copy The Knots
*/
 for( inp1P = knotPtsP , inp2P = intPtsP ; inp1P < knotPtsP + numKnotPts ; ++inp1P , ++inp2P ) *inp1P = *inp2P ;
/*
** Remove Duplicate Knots
*/
 for( inp1P = knotPtsP , inp2P = knotPtsP + 1  ; inp2P < knotPtsP + numKnotPts ; ++inp2P )
   {
    if( inp1P->x != inp2P->x || inp1P->y != inp2P->y )
      {
       ++inp1P ;
       if( inp1P != inp2P ) *inp1P = *inp2P ;
      }
   }
 numKnotPts = (long)(inp1P-knotPtsP) + 1 ;
/*
** Allocate Memory For Knot Flag Array
*/
 knotFlagP = ( long * ) malloc(*numLinePtsP * sizeof(long)) ;
 if( knotFlagP == nullptr ) goto errexit ;
/*
** Initialise Knot Flags
*/
 for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P ) *l1P = 0 ;
/*
** Mark Location Of Knots
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Location Of Knots") ;
 for( inpP = knotPtsP ; inpP < knotPtsP + numKnotPts ; ++inpP )
   {
    p3dP = *linePtsPP ;
    while( p3dP->x != inpP->x || p3dP->y != inpP->y ) ++p3dP ;
    if( ! *(knotFlagP+(long)(p3dP-*linePtsPP)) )
      {
       *(knotFlagP+(long)(p3dP-*linePtsPP)) = (long)(inpP-knotPtsP) + 1 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Knot %6ld Location Found = %4ld",(long)(inpP-knotPtsP+1),(long)(p3dP-*linePtsPP)) ;
      }
    ++p3dP ;
    while( p3dP->x != inpP->x || p3dP->y != inpP->y ) ++p3dP ;
    if( ! *(knotFlagP+(long)(p3dP-*linePtsPP)) )
      {
       *(knotFlagP+(long)(p3dP-*linePtsPP)) = (long)(inpP-knotPtsP) + 1 ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Knot %6ld Location Found = %4ld",(long)(inpP-knotPtsP+1),(long)(p3dP-*linePtsPP)) ;
      }
   }
/*
** Write Location Of Knots
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Knot Flags = %6ld",*numLinePtsP ) ;
    for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
      {
       p3dP = *linePtsPP + (long)(l1P-knotFlagP) ;
       if( *l1P > 0 ) bcdtmWrite_message(0,0,0,"KnotFlag[%6ld] = %4ld ** %12.6lf %12.6lf %12.6lf",(long)(l1P-knotFlagP),*l1P,p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Test If String Closes
*/
 closeFlag = 0 ;
 if( (*linePtsPP)->x == (*linePtsPP+*numLinePtsP-1)->x && (*linePtsPP)->y == (*linePtsPP+*numLinePtsP-1)->y ) closeFlag = 1 ;
/*
** Remove Knot Loops
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Knots") ;
 loop = 1  ;
 while ( loop )
   {
    loop = 0 ;
/*
** Count Number Of Knots
*/
    numKnots = 0 ;
    for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
      {
       if( *l1P > 0 ) ++numKnots ;
      }
    numKnots = numKnots / 2 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Knots = %4ld",numKnots) ;
/*
** Remove Simple Knots
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Simple Knots") ;
    process = 1 ;
    while ( process )
      {
       process = 0 ;
       for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
         {
          if( *l1P > 0 )
            {
             l2P = l1P + 1 ;
             while ( l2P < knotFlagP + *numLinePtsP && *l2P <= 0 ) ++l2P ;
             if( l2P < knotFlagP + *numLinePtsP )
               {
                if( *l2P == *l1P )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Removing Simple Knot %6ld",*l1P) ;
/*
**                 If String Does Not Close Eliminate Knot
*/
                   if( ! closeFlag )
                     {
                      *l1P = 0 ;
                      for( lP = l1P + 1 ; lP <= l2P ; ++lP ) *lP = -1 ;
                     }
/*
**                 Get Knot Length, Start Length And End Length
*/
                   else
                     {
                      knotLength  = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,l1P,l2P) ;
                      startLength = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,knotFlagP,l1P) ;
                      endLength   = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,l2P,knotFlagP+*numLinePtsP-1) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Knot Length = %10.4lf Start length = %10.4lf End Length = %10.4lf",knotLength,startLength,endLength) ;
/*
**                    If Knot Length Shorter Than The Sum Of Start And Ending Length Eliminate Knot
*/
                      if( knotLength < startLength + endLength )
                        {
                         *l1P = 0 ;
                         for( lP = l1P + 1 ; lP <= l2P ; ++lP ) *lP = -1 ;
                        }
/*
**                    If Knot Length Greater Than The Sum Of Start And Ending Length Eliminate Ends
*/
                      else
                       {
                        for( lP = knotFlagP ; lP < l1P ; ++lP ) *lP = -1 ;
                        for( lP = l2P + 1 ; lP < knotFlagP + *numLinePtsP ; ++lP ) *lP = -1 ;
                        *l1P = 0 ;
                        *l2P = 0 ;
                       }
                    }
/*
**                 Reset For Next Knot
*/
                   process = 1 ;
                   loop = 1 ;
                  }
               }
            }
         }
      }
/*
**  Write Location Of Knots
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Knot Flags = %6ld",*numLinePtsP ) ;
       for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
         {
          p3dP = *linePtsPP + (long)(l1P-knotFlagP) ;
          if( *l1P > 0 ) bcdtmWrite_message(0,0,0,"KnotFlag[%6ld] = %4ld ** %12.6lf %12.6lf %12.6lf",(long)(l1P-knotFlagP),*l1P,p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**   Remove Loop Back Knots
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Loop Back Knots") ;
    process = 1 ;
    while ( process )
      {
       process = 0 ;
       for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
         {
          if( *l1P > 0 )
            {
             l2P = l1P + 1 ;
             while ( l2P < knotFlagP + *numLinePtsP && *l2P <= 0 ) ++l2P ;
             if( l2P < knotFlagP + *numLinePtsP )
               {
                l3P = l2P + 1 ;
                while ( l3P < knotFlagP + *numLinePtsP && *l3P != *l2P ) ++l3P ;
                if( l3P < knotFlagP + *numLinePtsP )
                  {
                   l4P = l3P + 1 ;
                   while ( l4P < knotFlagP + *numLinePtsP && *l4P <= 0 ) ++l4P ;
                   if( l4P < knotFlagP + *numLinePtsP )
                     {
                      if( *l4P == *l1P )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Loop Back Knot l1P = %6ld l4P = %6ld ** Offset 1 = %6ld Offset 4 = %6ld",*l1P,*l4P,(long)(l1P-knotFlagP),(long)(l4P-knotFlagP)) ;
                         for( lP = l1P ; lP <  l2P ; ++lP ) *l1P = -*l2P ;
                         for( lP = l3P ; lP <= l4P ; ++lP ) *l1P = 0 ;
                        }
                     }
                  }
               }
            }
         }
      }
/*
**  Remove Non Loop Back Knots
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Loop Back Knots") ;
    process = 1 ;
    while ( process )
      {
       process = 0 ;
       for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
         {
          if( *l1P > 0 )
            {
             l2P = l1P + 1 ;
             while ( l2P < knotFlagP + *numLinePtsP && *l2P != *l1P ) ++l2P ;
             if( l2P < knotFlagP + *numLinePtsP )
               {
                if( *l2P == *l1P )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Loop Back Knot %6ld",*l1P) ;
/*
**                 If String Does Not Close Eliminate Knot
*/
                   if( ! closeFlag )
                     {
                      *l1P = 0 ;
                      for( lP = l1P + 1 ; lP <= l2P ; ++lP ) *lP = -1 ;
                     }
/*
**                 Get Knot Length, Start Length And End Length
*/
                   else
                     {
                      knotLength  = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,l1P,l2P) ;
                      startLength = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,knotFlagP,l1P) ;
                      endLength   = bcdtmSideSlope_lengthOfNonKnotSection(*linePtsPP,knotFlagP,l2P,knotFlagP+*numLinePtsP-1) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Knot Length = %10.4lf Start length = %10.4lf End Length = %10.4lf",knotLength,startLength,endLength) ;
/*
**                    If Knot Length Shorter Than The Sum Of Start And Ending Length Eliminate Knot
*/
                      if( knotLength < startLength + endLength )
                        {
                         *l1P = 0 ;
                         for( lP = l1P + 1 ; lP <= l2P ; ++lP ) *lP = -1 ;
                        }
/*
**                   If Knot Length Greater Than The Sum Of Start And Ending Length Eliminate Ends
*/
                      else
                        {
                         for( lP = knotFlagP ; lP < l1P ; ++lP ) *lP = -1 ;
                         for( lP = l2P + 1 ; lP < knotFlagP + *numLinePtsP ; ++lP ) *lP = -1 ;
                         *l1P = 0 ;
                         *l2P = 0 ;
                        }
                     }
                   process = 1 ;
                   loop = 1 ;
                  }
               }
            }
         }
      }
/*
** Write Location Of Knots
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Knot Flags = %6ld",*numLinePtsP ) ;
       for( l1P = knotFlagP ; l1P < knotFlagP + *numLinePtsP ; ++l1P )
         {
          p3dP = *linePtsPP + (long)(l1P-knotFlagP) ;
          if( *l1P > 0 ) bcdtmWrite_message(0,0,0,"KnotFlag[%6ld] = %4ld ** %12.6lf %12.6lf %12.6lf",(long)(l1P-knotFlagP),*l1P,p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }

/*
** Remove Deleted Line String Points
*/
 p3dP = *linePtsPP ;
 for( l1P = knotFlagP ; l1P < knotFlagP +*numLinePtsP ; ++l1P )
   {
    if( *l1P >= 0 )
      {
       *p3dP = *(*linePtsPP + (long)(l1P-knotFlagP)) ;
       ++p3dP ;
      }
   }
 *numLinePtsP = (long)(p3dP-*linePtsPP) ;
/*
** Clean Up
*/
 cleanup :
 if( knotPtsP   != nullptr ) free(knotPtsP) ;
 if( knotFlagP  != nullptr ) free(knotFlagP) ;
/*
** Job Completed
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
double  bcdtmSideSlope_lengthOfNonKnotSection(DPoint3d *linePtsP,long *knotFlagP,long *l1P,long *l2P)
{
 long     *lP ;
 double   length=0.0 ;
 DPoint3d *p3d1P,*p3d2P ;
/*
** Scan From Start To End
*/
 lP = l1P ;
 while( lP < l2P )
   {
    while( lP < l2P && *lP < 0 ) ++lP ;
    if( lP < l2P )
      {
       p3d1P = linePtsP + (long)(lP-knotFlagP) ;
       ++lP ;
       while( lP < l2P && *lP < 0 ) ++lP ;
       if( lP <= l2P )
         {
          p3d2P = linePtsP + (long)(lP-knotFlagP) ;
          length = length + bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) ;
         }
      }
   }
/*
** Job Completed
*/
 return(length) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_storePoint(double x,double y,double z,DPoint3d **pointsPP,long *numPtsP,long *memPtsP,long memPtsInc)
{
 int  dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing %12.6lf %12.6lf %12.6lf",x,y,z) ;
/*
** Check For Memory Allocation
*/
 if( *numPtsP == *memPtsP )
   {
    *memPtsP = *memPtsP + memPtsInc ;
    if( *pointsPP == nullptr ) *pointsPP = (DPoint3d * ) malloc ( *memPtsP * sizeof(DPoint3d)) ;
    else                    *pointsPP = (DPoint3d * ) realloc( *pointsPP,*memPtsP * sizeof(DPoint3d)) ;
    if( *pointsPP == nullptr ) goto errexit ;
   }
/*
** Store Point
*/
 (*pointsPP+*numPtsP)->x = x ;
 (*pointsPP+*numPtsP)->y = y ;
 (*pointsPP+*numPtsP)->z = z ;
 ++*numPtsP ;
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
int bcdtmSideSlope_normalisePointArray(DPoint3d *pointsP,long numPts)
{
 double   large ;
 DPoint3d *p3dP ;
/*
** Validate
*/
 if( pointsP == nullptr || numPts < 0 ) goto errexit ;
/*
** Scan Points And Get Largest Absolute Number
*/
 large = fabs(pointsP->x) ;
 for( p3dP = pointsP ; p3dP < pointsP + numPts ; ++p3dP )
   {
    if( fabs(p3dP->x) > large ) large = fabs(p3dP->x) ;
    if( fabs(p3dP->y) > large ) large = fabs(p3dP->y) ;
    if( fabs(p3dP->z) > large ) large = fabs(p3dP->z) ;
   }
/*
** Scan Points And Normalise Coordinate values
*/
 for( p3dP = pointsP ; p3dP < pointsP + numPts ; ++p3dP )
   {
    p3dP->x = (p3dP->x/large) * large ;
    p3dP->y = (p3dP->y/large) * large ;
    p3dP->z = (p3dP->z/large) * large ;
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
double bcdtmSideSlope_getAngle(double x1,double y1,double x2,double y2)
{
 double ang ;
 static double pye2=0.0 ;
 if( pye2 == 0.0 ) pye2 = atan2(0.0,-1.0) * 2.0 ;
 ang = atan2((y2-y1),(x2-x1)) ;
 if( ang < 0.0 ) ang += pye2 ;
 return( ang ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_getPolygonDirection(DPoint3d *polyPtsP,long numPolyPts,long *directionP,double *areaP)
/*
**  This Function Determines The Direction And Area Of A Polygon
*/
 {
  double   x,y   ;
  DPoint3d *p3dP ;
/*
** Initialise Varaibles
*/
 *areaP = 0.0 ;
 *directionP = 0 ;
/*
** Calculate Polygon Area
*/
 for ( p3dP = polyPtsP + 1 ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
   {
    x = p3dP->x - (p3dP-1)->x ;
    y = p3dP->y - (p3dP-1)->y ;
    *areaP = *areaP + x * y / 2.0 + x * (p3dP-1)->y ;
   }
/*
** Set Polygon Direction
*/
 if( *areaP >= 0.0 ) *directionP = 1 ;          /* ClockWise      */
 else  { *directionP = 2 ; *areaP = -*areaP ; } /* Anti Clockwise */
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
int bcdtmSideSlope_reversePolygonDirection(DPoint3d *polyPtsP,long numPolyPts)
/*
**  This Function Reverses The Direction Of A Polygon
*/
{
 DPoint3d *pt1P,*pt2P,*pt3P,tempPt ;
/*
** Swap Coordinates
*/
 pt1P = polyPtsP ;
 pt2P = polyPtsP + numPolyPts - 1 ;
 pt3P = &tempPt ;
 while ( pt1P < pt2P )
   {
    *pt3P = *pt1P ;
    *pt1P = *pt2P ;
    *pt2P = *pt3P ;
    ++pt1P ; --pt2P ;
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
int bcdtmSideSlope_breakPointArrayAtKnots(DPoint3d *pointsP,long numPoints,DTM_STR_INT_PTS *knotPtsP,long numKnotPts,DTM_P3D_LINE_STRING **stringsPP,long *numStringsP)
/*
** This Function Breaks A Point Array Into Two Or More Point Arrays At Knot Points
*/
{
 int      ret=DTM_SUCCESS,dbg=0 ;
 long     numTempStrings,numStringPts,memStrings=0,memStringsInc=100,numTempStringPts ;
 long     *lP,*l1P,*knotFlagP=nullptr ;
 DPoint3d *p3dP,*p3d1P,*p3d2P,*p3d3P,*stringPtsP=nullptr ;
 DTM_P3D_LINE_STRING *tempStringsP=nullptr ;
 DTM_STR_INT_PTS *inpP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Breaking Point Array At Knots") ;
/*
** Initialise
*/
 *numStringsP = 0 ;
 if( *stringsPP != nullptr ) goto errexit ;
/*
** Create An Array Of Point Array Of Size One
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating An Array Of Point Array Of Size 1") ;
 tempStringsP = ( DTM_P3D_LINE_STRING *) malloc ( sizeof(DTM_P3D_LINE_STRING)) ;
 if( tempStringsP == nullptr ) goto errexit ;
 tempStringsP->stringPts    = nullptr ;
 tempStringsP->numStringPts = 0 ;
/*
** Allocate Memory For Point Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Point Array") ;
 tempStringsP->stringPts = ( DPoint3d * ) malloc (numPoints * sizeof(DPoint3d)) ;
 if( tempStringsP->stringPts == nullptr ) goto errexit ;
/*
** Copy Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Points") ;
 numTempStrings = 0 ;
 tempStringsP->numStringPts = numPoints ;
 for( p3d1P = tempStringsP->stringPts , p3d2P = pointsP ; p3d2P < pointsP + numPoints ; ++p3d1P , ++p3d2P ) *p3d1P = *p3d2P ;
/*
** If No Knots Present Just Set Strings To Temp String
*/
 if( numKnotPts <= 0 )
   {
    *stringsPP = tempStringsP ;
    *numStringsP = 1 ;
    tempStringsP = nullptr ;
   }
/*
** Knots Present
*/
 else
   {
/*
** Sort Knots On String Segment Number
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Knots") ;
    qsort(knotPtsP,numKnotPts,sizeof(DTM_STR_INT_PTS),( int (__cdecl *)(const void *,const void *))bcdtmSideSlope_intersectionPointsCompareFunction) ;
/*
** Insert Knot Points Into Temp String
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Knots") ;
    if( bcdtmSideSlope_insertIntersectionPointsIntoLineStrings(&tempStringsP,knotPtsP,numKnotPts) ) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Point Array Pts = %6ld",tempStringsP->numStringPts) ;
       for( p3d1P = tempStringsP->stringPts ; p3d1P < tempStringsP->stringPts + tempStringsP->numStringPts ; ++p3d1P )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.6lf %12.6lf %12.6lf",(long)(p3d1P-tempStringsP->stringPts),p3d1P->x,p3d1P->y,p3d1P->z) ;
         }
      }
/*
** Allocate Memory For Knot Flag Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Knot Flags") ;
    numTempStringPts = tempStringsP->numStringPts ;
    knotFlagP = ( long * ) malloc(numTempStringPts * sizeof(long)) ;
    if( knotFlagP == nullptr ) goto errexit ;
/*
** Initialise Knot Flags
*/
    for( l1P = knotFlagP ; l1P < knotFlagP + numTempStringPts ; ++l1P ) *l1P = 0 ;
/*
** Mark Location Of Knots
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Location Of Knots") ;
    for( inpP = knotPtsP ; inpP < knotPtsP + numKnotPts ; ++inpP )
      {
       for( p3d1P = tempStringsP->stringPts ; p3d1P < tempStringsP->stringPts + tempStringsP->numStringPts ; ++p3d1P )
         {
          if( p3d1P->x == inpP->x && p3d1P->y == inpP->y )
            {
             *(knotFlagP+(long)(p3d1P-tempStringsP->stringPts)) = 1 ;
            }
         }
      }
    if( dbg )
      {
       for( lP = knotFlagP ; lP < knotFlagP + numTempStringPts ; ++lP )
         {
          if( *lP ) bcdtmWrite_message(0,0,0,"Knot[%4ld] = %2ld",(long)(lP-knotFlagP),*lP) ;
         }
      }
/*
** Copy String Sections To Array Of Point Arrays
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Sections To Point Arrays") ;
    lP = knotFlagP + 1 ;
    p3d1P = tempStringsP->stringPts ;
    while( lP < knotFlagP + numTempStringPts )
      {
/*
** Scan To Next Knot
*/
       while( lP < knotFlagP + numTempStringPts && ! *lP ) ++lP ;
       if( lP >= knotFlagP + numTempStringPts ) --lP ;
/*
**  Sets Offsets To String
*/
       p3d2P = tempStringsP->stringPts + (long)(lP-knotFlagP) ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"String Start[%4ld] = %12.6lf %12.6lf %12.6lf",(long)(p3d1P-tempStringsP->stringPts),p3d1P->x,p3d1P->y,p3d1P->z) ;
          bcdtmWrite_message(0,0,0,"String   End[%4ld] = %12.6lf %12.6lf %12.6lf",(long)(p3d2P-tempStringsP->stringPts),p3d2P->x,p3d2P->y,p3d2P->z) ;
         }
/*
** Dont Store Zero Length Strings
*/
       if( (long)(p3d2P-p3d1P) > 1 || bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) > 0.0 )
         {
          numStringPts = (long)(p3d2P-p3d1P) + 1 ;
          stringPtsP   = ( DPoint3d * ) malloc(numStringPts*sizeof(DPoint3d)) ;
          if( stringPtsP == nullptr ) goto errexit ;
          for( p3dP = stringPtsP , p3d3P = p3d1P ; p3d3P <= p3d2P ; ++p3dP ,++p3d3P )
             {
              *p3dP = *p3d3P ;
             }
/*
** Check And Allocate Memory If Necessary
*/
          if( *numStringsP == memStrings )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory") ;
             memStrings = memStrings + memStringsInc ;
             if( *stringsPP == nullptr ) *stringsPP = ( DTM_P3D_LINE_STRING * ) malloc(memStrings*sizeof(DTM_P3D_LINE_STRING)) ;
             else                     *stringsPP = ( DTM_P3D_LINE_STRING * ) realloc(*stringsPP,memStrings*sizeof(DTM_P3D_LINE_STRING)) ;
             if( *stringsPP == nullptr ) goto errexit ;
            }
/*
** Store String
*/
          (*stringsPP+*numStringsP)->stringPts    = stringPtsP ;
          (*stringsPP+*numStringsP)->numStringPts = numStringPts ;
          ++*numStringsP ;
          stringPtsP = nullptr ;
         }
/*
** Reset For Next String
*/
       p3d1P = p3d2P ;
       lP = lP + 1 ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( stringPtsP   != nullptr )  free(stringPtsP) ;
 if( tempStringsP != nullptr )
   {
    if( tempStringsP->stringPts != nullptr ) free(tempStringsP->stringPts) ;
    free(tempStringsP) ;
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Breaking Point Array At Knots Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Breaking Point Array At Knots Error") ;
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
int bcdtmSideSlope_offsetCopyPointArray3D(DPoint3d *pointsP,long numPoints,double offset,double slope,long copyMode,double cornerStrokeTolerance,DPoint3d **parallelPtsPP,long *numParallelPtsP)
{
 int      i,ret=DTM_SUCCESS,dbg=0,sideOf=0 ;
 long     closeFlag,direction=0,numAngleIncs,memPts=0,memPtsInc=100 ;
 double   x,y,z,area,gpk2Pye,gpkPye,priorAngle,nextAngle,bisectorAngle ;
 double   deltaAngle=0.0,angleInc=0.0,adjAngleInc ;
 DPoint3d *cpP,*ppP,*npP,*p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Offset Copy Point Array") ;
/*
** Initialise
*/
 *numParallelPtsP = 0 ;
 if( *parallelPtsPP != nullptr ) { free(*parallelPtsPP) ; *parallelPtsPP = nullptr ; }
 gpkPye = atan2(0.0,-1.0) ;
 gpk2Pye = gpkPye * 2.0 ;
 memPtsInc =  numPoints + 100 ;
/*
** Validate
*/
 if( pointsP == nullptr ) goto errexit ;
 if( numPoints < 2   ) goto errexit ;
 if( cornerStrokeTolerance < 0.0 ) cornerStrokeTolerance = 0.0 ;
 if( copyMode != DTM_MITRE_CORNER && copyMode != DTM_ROUND_CORNER ) copyMode = DTM_MITRE_CORNER ;
/*
** Calculate Angle Increment For Rounded Corners
*/
 if( copyMode == DTM_ROUND_CORNER && cornerStrokeTolerance > 0.0 )
   {
    angleInc = atan(cornerStrokeTolerance/fabs(offset)) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Corner Angle Increment = %12.10lf",angleInc) ;
   }
/*
** Test For Closure
*/
 closeFlag = 0 ;
 if( pointsP->x == (pointsP+numPoints-1)->x && pointsP->y == (pointsP+numPoints-1)->y) closeFlag = 1 ;
 if( cornerStrokeTolerance < 0.0 ) cornerStrokeTolerance = 0.0 ;
 if( dbg &&   closeFlag ) bcdtmWrite_message(0,0,0,"Closed line string") ;
 if( dbg && ! closeFlag ) bcdtmWrite_message(0,0,0,"Open line string") ;
/*
**  Set Polygon Anti ( Counter ) Clockwise
*/
 if( closeFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction") ;
    bcdtmSideSlope_getPolygonDirection(pointsP,numPoints,&direction,&area) ;
    if( dbg && direction == 1 ) bcdtmWrite_message(0,0,0,"Reversing Direction") ;
    if( direction == 1 ) bcdtmSideSlope_reversePolygonDirection(pointsP,numPoints) ;
   }
/*
** Scan Points And Copy Parallel
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Parallel") ;
 *numParallelPtsP = 0 ;
 if( closeFlag ) ppP = pointsP + numPoints - 2 ;
 else            ppP = nullptr ;
 for( cpP = pointsP  ; cpP < pointsP + numPoints - closeFlag ; ++cpP )
   {
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Offseting From Point %6ld ** %12.6lf %12.6lf %12.6lf",(long)(cpP-pointsP),cpP->x,cpP->y,cpP->z) ;
    npP = cpP + 1 ;
    if( npP >= pointsP + numPoints )
      {
       if( closeFlag ) npP = pointsP + 1 ;
       else            npP = nullptr ;
      }
    priorAngle = nextAngle = 0.0 ;
    if( ppP != nullptr ) priorAngle = bcdtmSideSlope_getAngle(cpP->x,cpP->y,ppP->x,ppP->y) ;
    if( npP != nullptr ) nextAngle  = bcdtmSideSlope_getAngle(cpP->x,cpP->y,npP->x,npP->y) ;
    if( ppP != nullptr && npP != nullptr ) sideOf = bcdtmSideSlope_sideOf(ppP->x,ppP->y,npP->x,npP->y,cpP->x,cpP->y) ;
/*
** zero offset
*/
    if    ( offset == 0.0 )
      {
       x = cpP->x;
       y = cpP->y;
       z = z = cpP->z + slope * offset ;
       if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
      }
/*
** offset To Right Or External
*/
    else if( offset > 0.0 )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"offsetting To Right") ;
       if( ppP == nullptr || npP == nullptr )
         {
          if( ppP == nullptr ) deltaAngle = nextAngle  - gpkPye / 2.0 ;
          if( npP == nullptr ) deltaAngle = priorAngle + gpkPye / 2.0 ;
          x = cpP->x + offset * cos(deltaAngle) ;
          y = cpP->y + offset * sin(deltaAngle) ;
          z = z = cpP->z + slope * offset ;
          if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
         }
       else
         {
          if( nextAngle < priorAngle ) nextAngle += gpk2Pye ;
          bisectorAngle = ( priorAngle + nextAngle ) / 2.0 ;
          if( nextAngle > gpk2Pye ) nextAngle -= gpk2Pye ;
          if( sideOf < 0 )         /* Convex Corner To Right */
            {
             if( copyMode == DTM_MITRE_CORNER )
               {
                deltaAngle  =  nextAngle - priorAngle - gpkPye ;
                while ( deltaAngle > gpk2Pye ) deltaAngle -= gpk2Pye ;
                while ( deltaAngle < 0.0     ) deltaAngle += gpk2Pye ;
                if( deltaAngle > gpkPye ) deltaAngle -= gpkPye  ;
                deltaAngle = deltaAngle / 2.0 ;
                x = cpP->x + offset/cos(deltaAngle) * cos(bisectorAngle) ;
                y = cpP->y + offset/cos(deltaAngle) * sin(bisectorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
               }
             if( copyMode == DTM_ROUND_CORNER )
               {
                priorAngle = priorAngle + gpkPye/2.0 ;
                nextAngle  = nextAngle  - gpkPye/2.0 ;
                while( priorAngle >= gpk2Pye ) priorAngle -= gpk2Pye ;
                while( priorAngle <  0.0     ) priorAngle += gpk2Pye ;
                while( nextAngle  >= gpk2Pye ) nextAngle  -= gpk2Pye ;
                while( nextAngle  <  0.0     ) nextAngle  += gpk2Pye ;
                while( bisectorAngle >= gpk2Pye ) bisectorAngle -= gpk2Pye ;
                while( bisectorAngle <  0.0     ) bisectorAngle += gpk2Pye ;
                x = cpP->x + offset * cos(priorAngle) ;
                y = cpP->y + offset * sin(priorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                if( angleInc > 0.0 )
                  {
                   if( bisectorAngle >= priorAngle ) numAngleIncs = (long)(( bisectorAngle-priorAngle) / angleInc )  ;
                   else                              numAngleIncs = (long)(( bisectorAngle+gpk2Pye-priorAngle) / angleInc )  ;
                   if( bisectorAngle >= priorAngle ) adjAngleInc  = ( bisectorAngle-priorAngle) / (double) numAngleIncs ;
                   else                              adjAngleInc  = ( bisectorAngle+gpk2Pye-priorAngle) / (double) numAngleIncs ;
                   for( i = 1 ; i < numAngleIncs ; ++i )
                     {
                      x = cpP->x + offset * cos(priorAngle+i*adjAngleInc) ;
                      y = cpP->y + offset * sin(priorAngle+i*adjAngleInc) ;
                      z = z = cpP->z + slope * offset ;
                      if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                     }
                  }
                x = cpP->x + offset * cos(bisectorAngle) ;
                y = cpP->y + offset * sin(bisectorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                if( angleInc > 0.0 )
                  {
                   if( nextAngle >= bisectorAngle ) numAngleIncs = (long) (( nextAngle-bisectorAngle) / angleInc ) ;
                   else                             numAngleIncs = (long) (( nextAngle+gpk2Pye-bisectorAngle) / angleInc ) ;
                   if( nextAngle >= bisectorAngle ) adjAngleInc  = ( nextAngle-bisectorAngle) / (double) numAngleIncs ;
                   else                             adjAngleInc  = ( nextAngle+gpk2Pye-bisectorAngle) / (double) numAngleIncs ;
                   for( i = 1 ; i < numAngleIncs ; ++i )
                     {
                      x = cpP->x + offset * cos(bisectorAngle+i*adjAngleInc) ;
                      y = cpP->y + offset * sin(bisectorAngle+i*adjAngleInc) ;
                      z = z = cpP->z + slope * offset ;
                      if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                     }
                  }
                x = cpP->x + offset * cos(nextAngle) ;
                y = cpP->y + offset * sin(nextAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
               }
            }
          else if( sideOf == 0 )      /* Colinear Lines */
            {
             x = cpP->x + offset * cos(bisectorAngle) ;
             y = cpP->y + offset * sin(bisectorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
            }
          else  if( sideOf > 0 )       /* Concave Corner To Right */
            {
             priorAngle =  priorAngle + gpkPye/2.0 ;
             x = cpP->x + offset * cos(priorAngle) ;
             y = cpP->y + offset * sin(priorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
             nextAngle  = nextAngle - gpkPye/2.0 ;
             x = cpP->x + offset * cos(nextAngle) ;
             y = cpP->y + offset * sin(nextAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
/*
**  The following code Places A Single Copy Parallel Point At The Bisector
**  Angle Of The Concave Angle. The above code places two points orthongal
**  to the prior and next lines. This forces a knot in the copy parallel
**  that is subsequently removed. If problems occur in concave corners
**  uncomment the above code and comment out the following code
*/
/*
             deltaAngle = priorAngle-bisectorAngle  ;
             if( deltaAngle > gpk2Pye ) deltaAngle -= gpk2Pye ;
             if( deltaAngle < 0.0     ) deltaAngle += gpk2Pye ;
             if( deltaAngle > gpkPye  ) deltaAngle -= gpkPye  ;
             x = cpP->x + offset/sin(deltaAngle) * cos(bisectorAngle) ;
             y = cpP->y + offset/sin(deltaAngle) * sin(bisectorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
*/
            }
         }
      }
/*
** offset To Left Or Internal
*/
    else  if( offset < 0.0 )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"offsetting To Left") ;
       if( priorAngle < nextAngle ) priorAngle += gpk2Pye ;
       bisectorAngle = ( priorAngle + nextAngle ) / 2.0 ;
       if( priorAngle > gpk2Pye ) priorAngle -= gpk2Pye ;
       if( ppP == nullptr || npP == nullptr )
         {
          if( ppP == nullptr ) deltaAngle = nextAngle  + gpkPye / 2.0 ;
          if( npP == nullptr ) deltaAngle = priorAngle - gpkPye / 2.0 ;
          x = cpP->x - offset * cos(deltaAngle) ;
          y = cpP->y - offset * sin(deltaAngle) ;
          z = z = cpP->z + slope * offset ;
          if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
         }
       else
         {
          if( sideOf < 0 )           /* Concave Corner To Left */
            {
             priorAngle =  priorAngle - gpkPye/2.0 ;
             x = cpP->x - offset * cos(priorAngle) ;
             y = cpP->y - offset * sin(priorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
             if( cpP != pointsP + numPoints - 1 )
               {

                nextAngle  = nextAngle + gpkPye/2.0 ;
                x = cpP->x - offset * cos(nextAngle) ;
                y = cpP->y - offset * sin(nextAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
               }
/*
**  The following code Places A Single Copy Parallel Point At The Bisector
**  Angle Of The Concave Angle. The above code places two points orthongal
**  to the prior and next lines. This forces a knot in the copy parallel
**  that is subsequently removed. If problems occur in concave corners
**  uncomment the above code and comment out the following code
*/
/*
             deltaAngle = priorAngle-bisectorAngle  ;
             if( deltaAngle > gpk2Pye ) deltaAngle -= gpk2Pye ;
             if( deltaAngle < 0.0     ) deltaAngle += gpk2Pye ;
             if( deltaAngle > gpkPye  ) deltaAngle -= gpkPye  ;
             x = cpP->x - offset/sin(deltaAngle) * cos(bisectorAngle) ;
             y = cpP->y - offset/sin(deltaAngle) * sin(bisectorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
*/
            }
          else if( sideOf == 0 )        /* Colinear Lines */
            {
             x = cpP->x - offset * cos(bisectorAngle) ;
             y = cpP->y - offset * sin(bisectorAngle) ;
             z = z = cpP->z + slope * offset ;
             if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
            }
          else  if( sideOf > 0 )          /* Convex Corner To Left */
            {
             if( copyMode == DTM_MITRE_CORNER )
               {
                deltaAngle  =  priorAngle - nextAngle - gpkPye ;
                while ( deltaAngle > gpk2Pye ) deltaAngle -= gpk2Pye ;
                while ( deltaAngle < 0.0     ) deltaAngle += gpk2Pye ;
                if( deltaAngle > gpkPye ) deltaAngle -= gpkPye  ;
                deltaAngle = deltaAngle / 2.0 ;
                x = cpP->x - offset/cos(deltaAngle) * cos(bisectorAngle) ;
                y = cpP->y - offset/cos(deltaAngle) * sin(bisectorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
               }
             if( copyMode == DTM_ROUND_CORNER )
               {
                priorAngle = priorAngle - gpkPye/2.0 ;
                nextAngle  = nextAngle  + gpkPye/2.0 ;
                while( priorAngle >= gpk2Pye ) priorAngle -= gpk2Pye ;
                while( priorAngle <  0.0     ) priorAngle += gpk2Pye ;
                while( nextAngle  >= gpk2Pye ) nextAngle  -= gpk2Pye ;
                while( nextAngle  <  0.0     ) nextAngle  += gpk2Pye ;
                while( bisectorAngle >= gpk2Pye ) bisectorAngle -= gpk2Pye ;
                while( bisectorAngle <  0.0     ) bisectorAngle += gpk2Pye ;
                x = cpP->x - offset * cos(priorAngle) ;
                y = cpP->y - offset * sin(priorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                if( angleInc > 0.0 )
                  {
                   if( priorAngle >= bisectorAngle ) numAngleIncs = (long)(( priorAngle - bisectorAngle) / angleInc )  ;
                   else                              numAngleIncs = (long)(( priorAngle+gpk2Pye-bisectorAngle) / angleInc )  ;
                   if( priorAngle >= bisectorAngle ) adjAngleInc =  (priorAngle - bisectorAngle) / (double) numAngleIncs   ;
                   else                              adjAngleInc =  (priorAngle+gpk2Pye-bisectorAngle) / (double) numAngleIncs   ;
                   for( i = 1 ; i < numAngleIncs ; ++i )
                     {
                      x = cpP->x - offset * cos(priorAngle-i*adjAngleInc) ;
                      y = cpP->y - offset * sin(priorAngle-i*adjAngleInc) ;
                      z = z = cpP->z + slope * offset ;
                      if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                     }
                  }
                x = cpP->x - offset * cos(bisectorAngle) ;
                y = cpP->y - offset * sin(bisectorAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                if( angleInc > 0.0 )
                  {
                   if( bisectorAngle >= nextAngle ) numAngleIncs = (long) (( bisectorAngle - nextAngle ) / angleInc ) ;
                   else                             numAngleIncs = (long) (( bisectorAngle+gpk2Pye-nextAngle) / angleInc ) ;
                   if( bisectorAngle >= nextAngle ) adjAngleInc = ( bisectorAngle - nextAngle ) / (double) numAngleIncs ;
                   else                             adjAngleInc = ( bisectorAngle+gpk2Pye-nextAngle) / (double) numAngleIncs  ;
                   for( i = 1 ; i <= numAngleIncs ; ++i )
                     {
                      x = cpP->x - offset * cos(bisectorAngle-i*adjAngleInc) ;
                      y = cpP->y - offset * sin(bisectorAngle-i*adjAngleInc) ;
                      z = z = cpP->z + slope * offset ;
                      if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
                     }
                  }
                x = cpP->x - offset * cos(nextAngle) ;
                y = cpP->y - offset * sin(nextAngle) ;
                z = z = cpP->z + slope * offset ;
                if( bcdtmSideSlope_storePoint(x,y,z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
               }
            }
         }
      }
    ppP = cpP ;
   }
/*
** Write First Point As Last Point For Closing Element
*/
 if( closeFlag )
   {
    if( bcdtmSideSlope_storePoint((*parallelPtsPP)->x,(*parallelPtsPP)->y,(*parallelPtsPP)->z,parallelPtsPP,numParallelPtsP,&memPts,memPtsInc)) goto errexit ;
    if( direction == 1 ) bcdtmSideSlope_reversePolygonDirection(*parallelPtsPP,*numParallelPtsP) ;
   }
/*
** Write Parallel Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Copy Parallel Points = %6ld",*numParallelPtsP) ;
    for( p3dP = *parallelPtsPP ; p3dP < *parallelPtsPP + *numParallelPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-*parallelPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Offset Copy Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Offset Copy Point Array Error") ;
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
int bcdtmSideSlope_copyParallelSideSlopeElement
(
 DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,  /* ==> Table Containing The Parameters For Determing The Side Slopes */
 long sideSlopeTableSize,                /* ==> Size Of Or Number Of Entries In The Side Slope Table  */
 long sideSlopeDirection,                /* ==> Direction To Calculate The Side Slopes 1 = Right 2 = Left */
 long cornerOption,                      /* ==> Rounded Or Square Side Slope Corners. 1 = Rounded 2 = Square  */
 double cornerStrokeTolerance,           /* ==> Linear Tolerance For Stroking Side Slope Corners  */
 BC_DTM_OBJ* **dataObjectsPPP,          /* <== Array Of Pointers To The Created Side Slope Data Objects */
 long *numberOfDataObjectsP              /* <== Size Of or Number Of Array Of Data Object Pointers  */
)
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long copyMode,numElementPts,numParallelPts ;
 double slope,horOffset ;
 DPoint3d  *p3dP,*elementPtsP=nullptr,*parallelPtsP=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radialP ;
 BC_DTM_OBJ *dataP=nullptr ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
 /*
** Allocate Memory For Side Slope Element Points
*/
 numElementPts = sideSlopeTableSize ;
 elementPtsP   = (DPoint3d *) malloc( numElementPts * sizeof(DPoint3d)) ;
 if( elementPtsP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Element Points
*/
 for( p3dP = elementPtsP , radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++p3dP , ++radialP )
   {
    p3dP->x   = radialP->radialStartPoint.x ;
    p3dP->y   = radialP->radialStartPoint.y ;
    p3dP->z   = radialP->radialStartPoint.z ;
   }
/*
** Set Slope And Horizontal Offset
*/
 if( sideSlopeTableP->isForceSlope ) slope = sideSlopeTableP->forcedSlope ;
 else                                slope = sideSlopeTableP->radialSlope ;
 if( sideSlopeTableP->sideSlopeOption == 3 ) horOffset = fabs(sideSlopeTableP->toHorizOffset) ;
 if( sideSlopeTableP->sideSlopeOption == 4 ) horOffset = fabs(sideSlopeTableP->toDeltaElev)/fabs(slope) ;
 if( sideSlopeDirection == 2 ) horOffset = - horOffset ;
/*
** Copy Parallel Element Points
*/
 copyMode = DTM_MITRE_CORNER ;
 if( cornerOption == 1 ) copyMode = DTM_ROUND_CORNER ;
 if( bcdtmSideSlope_copyParallel3D(elementPtsP,numElementPts,horOffset,slope,copyMode,cornerStrokeTolerance,&parallelPtsP,&numParallelPts)) goto errexit ;
/*
** Create Data Object To Store The Parallel Points
*/
 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dataP,numParallelPts,10) ;
/*
** Copy Parallel Points To Data Object As A Slope Toe
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,parallelPtsP,numParallelPts)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dataP,L"copyParallel.dat") ;
/*
** Create Data Object Pointer Array
*/
 *numberOfDataObjectsP = 1 ;
 *dataObjectsPPP = (BC_DTM_OBJ **) malloc (*numberOfDataObjectsP * sizeof(BC_DTM_OBJ *)) ;
 if( *dataObjectsPPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 *(*dataObjectsPPP) = dataP ;
 dataP = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( elementPtsP  != nullptr ) { free(elementPtsP)  ; elementPtsP  = nullptr ; }
 if( parallelPtsP != nullptr ) { free(parallelPtsP) ; parallelPtsP = nullptr ; }
 if( dataP != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copy Parallel Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copy Parallel Side Slope Element Error") ;
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
int bcdtmSideSlope_copyParallelSideSlopeElementToPointArray
(
 DTM_SIDE_SLOPE_TABLE *sideSlopeTableP,  /* ==> Table Containing The Parameters For Determing The Side Slopes */
 long sideSlopeTableSize,                /* ==> Size Of Or Number Of Entries In The Side Slope Table  */
 long sideSlopeDirection,                /* ==> Direction To Calculate The Side Slopes 1 = Right 2 = Left */
 long cornerOption,                      /* ==> Rounded Or Square Side Slope Corners. 1 = Rounded 2 = Square  */
 double cornerStrokeTolerance,           /* ==> Linear Tolerance For Stroking Side Slope Corners  */
 DPoint3d  **paraElmemPolyPtsPP,              /* ==> Point Array To Store Parallel Points  */
 long *numParaElmemPolyPtsP              /* ==> Number Of Points In Point Array */
)
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long copyMode,numElementPts,numParallelPts ;
 double slope,horOffset ;
 DPoint3d  *p3dP,*elementPtsP=nullptr,*parallelPtsP=nullptr ;
 DTM_SIDE_SLOPE_TABLE *radialP ;
/*
** Write stroke
/*
** Allocate Memory For Side Slope Element Points
*/
 numElementPts = sideSlopeTableSize ;
 elementPtsP   = (DPoint3d *) malloc( numElementPts * sizeof(DPoint3d)) ;
 if( elementPtsP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Element Points
*/
 for( p3dP = elementPtsP , radialP = sideSlopeTableP ; radialP < sideSlopeTableP + sideSlopeTableSize ; ++p3dP , ++radialP )
   {
    p3dP->x   = radialP->radialStartPoint.x ;
    p3dP->y   = radialP->radialStartPoint.y ;
    p3dP->z   = radialP->radialStartPoint.z ;
   }
/*
** Set Slope And Horizontal Offset
*/
 if( sideSlopeTableP->isForceSlope ) slope = sideSlopeTableP->forcedSlope ;
 else                                slope = sideSlopeTableP->radialSlope ;
 if( sideSlopeTableP->sideSlopeOption == 6 ) horOffset = sideSlopeTableP->toHorizOffset ;
 if( sideSlopeTableP->sideSlopeOption == 7 ) horOffset = fabs(slope)/fabs(sideSlopeTableP->toDeltaElev) ;
 if( sideSlopeDirection == 2 ) horOffset = - horOffset ;
/*
** Copy Parallel Element Points
*/
 copyMode = DTM_MITRE_CORNER ;
 if( cornerOption == 1 ) copyMode = DTM_ROUND_CORNER ;
 if( bcdtmSideSlope_copyParallel3D(elementPtsP,numElementPts,horOffset,slope,copyMode,cornerStrokeTolerance,&parallelPtsP,&numParallelPts)) goto errexit ;
/*
** Copy Parallel Pts To Parallel Element Points Array
*/
 *paraElmemPolyPtsPP   = parallelPtsP  ;
 *numParaElmemPolyPtsP = numParallelPts ;
  parallelPtsP   = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( elementPtsP  != nullptr ) { free(elementPtsP)  ; elementPtsP  = nullptr ; }
 if( parallelPtsP != nullptr ) { free(parallelPtsP) ; parallelPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copy Parallel Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copy Parallel Side Slope Element Error") ;
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
int bcdtmSideSlope_writeLimitSlopeToesToDataObject(long sideSlopeType,BC_DTM_OBJ *dataP,DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,long numOvlPts)
/*
** This Function Writes The Slope Toes To A Data Object
*/
{
 int    ret=DTM_SUCCESS,dbg=0;
 bool newDev=false ;
 long   padType,intersectionFound,ovlOnSurface,nextOnSurface,ovnOnSurface,priorOnSurface,drapeFlag;
 double  ovlX,ovlY,ovlZ,nextX,nextY,nextZ,priorX,priorY,priorZ,ovnX,ovnY,ovnZ,intX,intY,intZ,intZ1,intZ2 ;
 DTM_OVERLAP_RADIAL_TABLE *ovlP,*ovnP,*startOvlP,*nextP,*priorP ;
 DPoint3d             pnt,p3dPts[2],*p3dP,*p3dLastP,*slopeToePtsP=nullptr ;
 long            numSlopeToePts,numKnotPts,direction ;
 DTM_STR_INT_PTS *knotPtsP=nullptr ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object") ;
    if( sideSlopeType == 0 ) bcdtmWrite_message(0,0,0,"Closed Side Slope Element") ;
    else                     bcdtmWrite_message(0,0,0,"Open Side Slope Element") ;
   }
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",numOvlPts) ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
      {
//       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %1ld S = %1ld Tr = %9ld ** %10.4lf %10.4lf %10.4lf",(long)(ovlP-ovlPtsP),ovlP->Type,ovlP->Status,ovlP->TruncatingRadial,ovlP->Px,ovlP->Py,ovlP->Pz) ;
       bcdtmWrite_message(0,0,0,"Radial[%6ld] = %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf ",(long)(ovlP-ovlPtsP),ovlP->Px,ovlP->Py,ovlP->Pz,ovlP->Nx,ovlP->Ny,ovlP->Nz) ;
      }
   }
/*
** Set Pad Type
*/
 if( sideSlopeType == 1 ) padType = 1 ;  /* Open Side Slope Element   */
 else                     padType = 0 ;  /* Closed Side Slope Element */
 if( dbg ) bcdtmWrite_message(0,0,0,"padType = %2ld ** numOvlPts = %6ld",padType,numOvlPts) ;
/*
** Use New Development To Get Slope Toes
*/
 if( newDev == true )
   {
/*
**  Allocate Memory
*/
    numSlopeToePts  = numOvlPts ;
    if( ! padType ) ++numSlopeToePts ;
    slopeToePtsP = ( DPoint3d * ) malloc( numSlopeToePts * sizeof(DPoint3d)) ;
    if( slopeToePtsP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Faailure") ;
       goto errexit ;
      }
/*
**  Store Slope To Points
*/
    numSlopeToePts = 0 ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
      {
       (slopeToePtsP+numSlopeToePts)->x = ovlP->Gx ;
       (slopeToePtsP+numSlopeToePts)->y = ovlP->Gy ;
       (slopeToePtsP+numSlopeToePts)->z = ovlP->Gz ;
       ++numSlopeToePts ;
      }
    if( ! padType )
      {
       (slopeToePtsP+numSlopeToePts)->x = ovlPtsP->Gx ;
       (slopeToePtsP+numSlopeToePts)->y = ovlPtsP->Gy ;
       (slopeToePtsP+numSlopeToePts)->z = ovlPtsP->Gz ;
       ++numSlopeToePts ;
      }
/*
**  Determine Side Slope Direction
*/
    direction = 0 ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts - 1 && ! direction ; ++ovlP )
      {
       if( ovlP->Px != (ovlP+1)->Px || ovlP->Py != (ovlP+1)->Py )
         {
          if( ovlP->Gx != ovlP->Px || ovlP->Gy != ovlP->Py )
            {
             direction = bcdtmMath_sideOf(ovlP->Px,ovlP->Py,(ovlP+1)->Px,(ovlP+1)->Py,ovlP->Gx,ovlP->Gy) ;
            }
         }
      }
    if( dbg )bcdtmWrite_message(0,0,0,"PadType = %2ld ** Side Slope Direction = %2ld",padType,direction) ;
/*
** Reverse Slope Toe Points If Direction Is Positive
*/
   if( ( padType == 1 && direction > 0 ) || ( padType == 0 && direction > 0 ) )
     {
      p3dP = slopeToePtsP ;
      p3dLastP = slopeToePtsP + numSlopeToePts - 1 ;
      while( p3dP < p3dLastP )
        {
         pnt = *p3dP ;
         *p3dP = *p3dLastP ;
         *p3dLastP = pnt ;
         ++p3dP ;
         --p3dLastP ;
        }
     }
/*
**  Remove Knots
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Knots ** slopeToePtsP = %p ** numSlopePtsP = %8ld",slopeToePtsP,numSlopeToePts) ;
    if( bcdtmSideSlope_removeKnots(&slopeToePtsP,&numSlopeToePts,&knotPtsP,&numKnotPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slope Toe Points = %6ld",numSlopeToePts) ;
/*
**  Copy Slope Toe Points To
*/
    if( numSlopeToePts >= 2 )
      {
       for( p3dP = slopeToePtsP ; p3dP < slopeToePtsP + numSlopeToePts - 1 ; ++p3dP )
         {
          p3dPts[0].x = p3dP->x ;
          p3dPts[0].y = p3dP->y ;
          p3dPts[0].z = p3dP->z ;
          p3dPts[1].x = (p3dP+1)->x ;
          p3dPts[1].y = (p3dP+1)->y ;
          p3dPts[1].z = (p3dP+1)->z ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,p3dP->x,p3dP->y,p3dP->z)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,(p3dP+1)->x,(p3dP+1)->y,(p3dP+1)->z)) goto errexit ;
         }
      }
/*
** Go To Cleanup
*/
   goto endup ;
  }
/*
** Scan To First Non Truncated Slope Toe
*/
 startOvlP = nullptr ;
 for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts - padType && startOvlP == nullptr ; ++ovlP )
   {
    if( ovlP->Status ) startOvlP = ovlP ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Start Slope Toe Radial = %6ld",(long)(startOvlP-ovlPtsP)) ;
/*
** Store Slope Toes In Data Object
*/
 if( startOvlP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Slope Toes In Data Object") ;
    ovlP = startOvlP ;
/*
**  Check For Prior Slope Toe From Truncated Radial Of Open Side Slope Element
*/
    if( padType == 1 && startOvlP != ovlPtsP )
      {
       ovlX = ovlP->Gx ;
       ovlY = ovlP->Gy ;
       ovlZ = ovlP->Gz ;
       ovlOnSurface = 0 ;
       if( benchTinP != nullptr )
         {
          if( bcdtmDrape_pointDtmObject(benchTinP,ovlX,ovlY,&intZ1,&drapeFlag)) goto errexit ;
          if( fabs(intZ1-ovlZ) < 0.0001 ) ovlOnSurface = 1 ;
         }
       priorP = startOvlP - 1 ;
       priorX = priorP->Gx ;
       priorY = priorP->Gy ;
       priorZ = priorP->Gz ;
       priorOnSurface = 0 ;
       if( benchTinP != nullptr )
         {
          if( bcdtmDrape_pointDtmObject(benchTinP,priorX,priorY,&intZ1,&drapeFlag)) goto errexit ;
          if( fabs(intZ1-priorZ) < 0.0001 ) priorOnSurface = 1 ;
         }
       bcdtmSideSlope_findFirstIntersectionWithSideSlopeRadial(ovlPtsP,numOvlPts,1,ovlX,ovlY,ovlZ,priorX,priorY,priorZ,priorP,ovlP,&intersectionFound,&intX,&intY,&intZ) ;
       if( intersectionFound )
         {
          priorX = intX ;
          priorY = intY ;
          priorZ = intZ ;
          if( ovlOnSurface && priorOnSurface ) if( bcdtmDrape_pointDtmObject(benchTinP,priorX,priorY,&priorZ,&drapeFlag)) goto errexit ;
         }
       p3dPts[0].x = priorX ;
       p3dPts[0].y = priorY ;
       p3dPts[0].z = priorZ ;
       p3dPts[1].x = ovlX ;
       p3dPts[1].y = ovlY ;
       p3dPts[1].z = ovlZ ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,priorX,priorY,priorZ)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,ovlX,ovlY,ovlZ)) goto errexit ;
      }
/*
**  Scan Side Slope Radials
*/
    do
      {
/*
**     Set Next Radial
*/
       ovnP = ovlP + 1 ;
       if( ovnP >= ovlPtsP + numOvlPts )
         {
          if( padType == 1 ) ovnP = nullptr ;
          else               ovnP = ovlPtsP ;
         }
/*
**     Check Next Radial Found
*/
       if( ovnP != nullptr )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Processing Slope Toe %6ld ** %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
/*
**        Write Non Truncated Slope Toe
*/
          if( ovlP->Status && ovnP->Status )
                {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing None Truncated Slope Toe %6ld %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
             p3dPts[0].x = ovlP->Gx ;
             p3dPts[0].y = ovlP->Gy ;
             p3dPts[0].z = ovlP->Gz ;
             p3dPts[1].x = ovnP->Gx ;
             p3dPts[1].y = ovnP->Gy ;
             p3dPts[1].z = ovnP->Gz ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//             if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,ovlP->Gx,ovlP->Gy,ovlP->Gz)) goto errexit ;
//             if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,ovnP->Gx,ovnP->Gy,ovnP->Gz)) goto errexit ;
            }
/*
**        Scan To First Non Truncated Radial
*/
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Processing Truncated Slope Toe ** ovl = %6ld ovn = %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
             if( bcdtmSideSlope_findFirstNonTruncatedRadial(ovlPtsP,numOvlPts,padType,ovlP,&ovnP)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"** Processing Slope Toe %6ld ** %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;

/*
**           Check Next Radial Found
*/
             if( ovnP != nullptr )
               {
/*
**              Set Coordinates
*/
                ovlX = ovlP->Gx ;
                ovlY = ovlP->Gy ;
                ovlZ = ovlP->Gz ;
                ovlOnSurface = 0 ;
                if( benchTinP != nullptr )
                  {
                   if( bcdtmDrape_pointDtmObject(benchTinP,ovlX,ovlY,&intZ1,&drapeFlag)) goto errexit ;
                   if( fabs(intZ1-ovlZ) < 0.0001 ) ovlOnSurface = 1 ;
                  }
/*
**              Get Next Slope Toe From ovlP
*/
                nextP = ovlP + 1 ;
                if( nextP >= ovlPtsP + numOvlPts ) nextP = ovlPtsP ;
                nextX = nextP->Gx ;
                nextY = nextP->Gy ;
                nextZ = nextP->Gz ;
                nextOnSurface = 0 ;
                if( benchTinP != nullptr )
                  {
                   if( bcdtmDrape_pointDtmObject(benchTinP,nextX,nextY,&intZ1,&drapeFlag)) goto errexit ;
                   if( fabs(intZ1-nextZ) < 0.0001 ) nextOnSurface = 1 ;
                  }
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"L ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",ovlX,ovlY,ovlZ,nextX,nextY,nextZ) ;
/*
**              Check For Intersection With Side Slope Radials
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Finding Intersection With Side Slope Radial") ;
                bcdtmSideSlope_findFirstIntersectionWithSideSlopeRadial(ovlPtsP,numOvlPts,0,ovlX,ovlY,ovlZ,nextX,nextY,nextZ,ovlP,ovnP,&intersectionFound,&intX,&intY,&intZ) ;
                if( intersectionFound )
                  {
                   nextX = intX ;
                   nextY = intY ;
                   nextZ = intZ ;
                   if( ovlOnSurface && nextOnSurface ) if( bcdtmDrape_pointDtmObject(benchTinP,nextX,nextY,&nextZ,&drapeFlag)) goto errexit ;
                   if( dbg == 1 ) bcdtmWrite_message(0,0,0,"L ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",ovlX,ovlY,ovlZ,nextX,nextY,nextZ) ;
                  }
/*
**              Get Prior Slope Toe To ovnP
*/
                ovnX = ovnP->Gx ;
                ovnY = ovnP->Gy ;
                ovnZ = ovnP->Gz ;
                ovnOnSurface = 0 ;
                if( benchTinP != nullptr )
                  {
                   if( bcdtmDrape_pointDtmObject(benchTinP,ovnX,ovnY,&intZ1,&drapeFlag)) goto errexit ;
                   if( fabs(intZ1-ovnZ) < 0.0001 ) ovnOnSurface = 1 ;
                  }
                priorP = ovnP - 1 ;
                if( priorP < ovlPtsP ) priorP = ovlPtsP + numOvlPts - 1 ;
                priorX = priorP->Gx ;
                priorY = priorP->Gy ;
                priorZ = priorP->Gz ;
                priorOnSurface = 0 ;
                if( benchTinP != nullptr )
                  {
                   if( bcdtmDrape_pointDtmObject(benchTinP,priorX,priorY,&intZ1,&drapeFlag)) goto errexit ;
                   if( fabs(intZ1-priorZ) < 0.0001 ) priorOnSurface = 1 ;
                  }
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"N ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",priorX,priorY,priorZ,ovnX,ovnY,ovnZ) ;
/*
**              Check For Intersection With Side Slope Radials
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Finding Intersection With Side Slope Radial") ;
                bcdtmSideSlope_findFirstIntersectionWithSideSlopeRadial(ovlPtsP,numOvlPts,1,ovnX,ovnY,ovnZ,priorX,priorY,priorZ,ovnP,ovlP,&intersectionFound,&intX,&intY,&intZ) ;
                if( intersectionFound )
                  {
                   priorX = intX ;
                   priorY = intY ;
                   priorZ = intZ ;
                   if( ovnOnSurface && priorOnSurface ) if( bcdtmDrape_pointDtmObject(benchTinP,priorX,priorY,&priorZ,&drapeFlag)) goto errexit ;
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"N ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",priorX,priorY,priorZ,ovnX,ovnY,ovnZ) ;
                  }
/*
**              Check For Intersection Of Next And Prior Slope Toes
*/
                intersectionFound = false ;
                if( ovlP != ovnP ) intersectionFound = bcdtmMath_intersectCordLines(ovlX,ovlY,nextX,nextY,priorX,priorY,ovnX,ovnY,&intX,&intY) ;
                if( intersectionFound )
                  {
                   bcdtmMath_interpolatePointOnLine(ovlX,ovlY,ovlZ,nextX,nextY,nextZ,intX,intY,&intZ1) ;
                   bcdtmMath_interpolatePointOnLine(priorX,priorY,priorZ,ovnX,ovnY,ovnZ,intX,intY,&intZ2) ;
                   intZ = ( intZ1 + intZ2 ) / 2.0 ;
                   if( nextOnSurface && priorOnSurface ) if( bcdtmDrape_pointDtmObject(benchTinP,intX,intY,&intZ,&drapeFlag)) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"INT ** %12.5lf %12.5lf %10.4lf",intX,intY,intZ) ;
                  }
/*
**              Write Slope Toe
*/
                p3dPts[0].x = ovlX ;
                p3dPts[0].y = ovlY ;
                p3dPts[0].z = ovlZ ;
//                if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,ovlX,ovlY,ovlZ)) goto errexit ;
                if( intersectionFound == false )
                  {
                   p3dPts[1].x = nextX ;
                   p3dPts[1].y = nextY ;
                   p3dPts[1].z = nextZ ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//                   if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,nextX,nextY,nextZ)) goto errexit ;
                   p3dPts[0].x = nextX ;
                   p3dPts[0].y = nextY ;
                   p3dPts[0].z = nextZ ;
                   p3dPts[1].x = priorX ;
                   p3dPts[1].y = priorY ;
                   p3dPts[1].z = priorZ ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//                   if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,nextX,nextY,nextZ)) goto errexit ;
//                   if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,priorX,priorY,priorZ)) goto errexit ;
                   p3dPts[0].x = priorX ;
                   p3dPts[0].y = priorY ;
                   p3dPts[0].z = priorZ ;
//                   if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,priorX,priorY,priorZ)) goto errexit ;
                  }
                else
                  {
                   p3dPts[1].x = intX ;
                   p3dPts[1].y = intY ;
                   p3dPts[1].z = intZ ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//                   if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,intX,intY,intZ)) goto errexit ;
                   p3dPts[0].x = intX ;
                   p3dPts[0].y = intY ;
                   p3dPts[0].z = intZ ;
//                   if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,intX,intY,intZ)) goto errexit ;
                  }
                p3dPts[1].x = ovnX ;
                p3dPts[1].y = ovnY ;
                p3dPts[1].z = ovnZ ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//                if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,ovnX,ovnY,ovnZ)) goto errexit ;
               }
            }
         }
/*
**     Set For Next Slope Toe
*/
       if( ovlP == ovnP ) ovnP = nullptr ;
       ovlP = ovnP ;
      } while ( ovlP != startOvlP && ovlP != nullptr ) ;
/*
**  Check For Next Slope Toe From None Truncated Radial Of Open Side Slope Element
*/
    if( padType == 1 && !(ovlPtsP+numOvlPts-1)->Status )
      {
/*
**     Scan Back To Get First None Truncated Radial
*/
       ovlP = nullptr ;
       for( ovnP = ovlPtsP+numOvlPts-1 ; ovnP >= ovlPtsP && ovlP == nullptr ; --ovnP )
         {
          if( ovnP->Status ) ovlP = ovnP ;
         }
/*
**     Get Next Slope Toe
*/
       if( ovlP == nullptr )
         {
          ovlX = ovlP->Gx ;
          ovlY = ovlP->Gy ;
          ovlZ = ovlP->Gz ;
          ovlOnSurface = 0 ;
          if( benchTinP != nullptr )
            {
             if( bcdtmDrape_pointDtmObject(benchTinP,ovlX,ovlY,&intZ1,&drapeFlag)) goto errexit ;
             if( fabs(intZ1-ovlZ) < 0.0001 ) ovlOnSurface = 1 ;
            }
          nextP = ovlP + 1 ;
          if( nextP >= ovlPtsP + numOvlPts ) nextP = ovlPtsP ;
          nextX = nextP->Gx ;
          nextY = nextP->Gy ;
          nextZ = nextP->Gz ;
          nextOnSurface = 0 ;
          if( benchTinP != nullptr )
            {
             if( bcdtmDrape_pointDtmObject(benchTinP,nextX,nextY,&intZ1,&drapeFlag)) goto errexit ;
             if( fabs(intZ1-nextZ) < 0.0001 ) nextOnSurface = 1 ;
            }
/*
**        Check For Intersection With Side Slope Radials
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Finding Intersection With Side Slope Radial") ;
          bcdtmSideSlope_findFirstIntersectionWithSideSlopeRadial(ovlPtsP,numOvlPts,0,ovlX,ovlY,ovlZ,nextX,nextY,nextZ,ovlP,ovlP,&intersectionFound,&intX,&intY,&intZ) ;
          if( intersectionFound )
            {
             nextX = intX ;
             nextY = intY ;
             nextZ = intZ ;
             if( ovlOnSurface && nextOnSurface ) if( bcdtmDrape_pointDtmObject(benchTinP,nextX,nextY,&nextZ,&drapeFlag)) goto errexit ;
            }
          p3dPts[0].x = ovlX ;
          p3dPts[0].y = ovlY ;
          p3dPts[0].z = ovlZ ;
          p3dPts[1].x = nextX ;
          p3dPts[1].y = nextY ;
          p3dPts[1].z = nextZ ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,ovlX,ovlY,ovlZ)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,nextX,nextY,nextZ)) goto errexit ;
         }
      }
   }
/*
** Write Slope Toes To Data File
*/
 endup :
 if( dbg )bcdtmWrite_toFileDtmObject(dataP,L"limitSlopeToes.dat") ;
/*
** Clean Up
*/
 cleanup :
 if( knotPtsP     != nullptr ) free(knotPtsP)  ;
 if( slopeToePtsP != nullptr ) free(slopeToePtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object Error") ;
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
int bcdtmSideSlope_findFirstNonTruncatedRadial
(
 DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,
 long numOvlPts,
 long padType,
 DTM_OVERLAP_RADIAL_TABLE *ovlP,
 DTM_OVERLAP_RADIAL_TABLE **ovnPP
)
{
 int ret=DTM_SUCCESS;
/*
**  Scan Forwards From ovlP ;
*/
 *ovnPP = ovlP + 1 ;
 if( *ovnPP >= ovlPtsP + numOvlPts )
   {
    if( padType == 1 ) *ovnPP = nullptr ;
    else               *ovnPP = ovlPtsP ;
   }
while ( *ovnPP != nullptr && (*ovnPP)->Status != 1 )
   {
    ++*ovnPP ;
    if( *ovnPP >= ovlPtsP + numOvlPts )
      {
       if( padType == 1 ) *ovnPP = nullptr ;
       else               *ovnPP = ovlPtsP ;
      }
   }
/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_findFirstIntersectionWithSideSlopeRadial
(
 DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,
 long                     numOvlPts,
 long                     direction,          /* Scan Direction 0 = Forward , 1 = Reverse  */
 double                   slopeToe1X,
 double                   slopeToe1Y,
 double                   slopeToe1Z,
 double                   slopeToe2X,
 double                   slopeToe2Y,
 double                   slopeToe2Z,
 DTM_OVERLAP_RADIAL_TABLE *startP,
 DTM_OVERLAP_RADIAL_TABLE *endP,
 long                     *intersectionFoundP,
 double                   *intXP,
 double                   *intYP,
 double                   *intZP
)
{
 int ret=DTM_SUCCESS ;
 long pointerInc=0 ;
 double intZ1,intZ2 ;
 DTM_OVERLAP_RADIAL_TABLE *nextP ;
/*
** Initialise
*/
 *intXP = 0.0 ;
 *intYP = 0.0 ;
 *intZP = 0.0 ;
 *intersectionFoundP = false ;
 if( direction == 0 ) pointerInc =  1 ;
 else                 pointerInc = -1 ;
/*
**  Scan From startP ;
*/
 nextP = startP + pointerInc ;
 if( nextP >= ovlPtsP + numOvlPts ) nextP = ovlPtsP ;
 if( nextP <  ovlPtsP             ) nextP = ovlPtsP + numOvlPts - 1 ;
 while( nextP != endP )
   {
    *intersectionFoundP = bcdtmMath_intersectCordLines(slopeToe1X,slopeToe1Y,slopeToe2X,slopeToe2Y,nextP->Px,nextP->Py,nextP->Nx,nextP->Ny,intXP,intYP) ;
    if( *intersectionFoundP )
      {
       bcdtmMath_interpolatePointOnLine(slopeToe1X,slopeToe1Y,slopeToe1Z,slopeToe2X,slopeToe2Y,slopeToe2Z,*intXP,*intYP,&intZ1) ;
       bcdtmMath_interpolatePointOnLine(nextP->Px,nextP->Py,nextP->Pz,nextP->Nx,nextP->Ny,nextP->Nz,*intXP,*intYP,&intZ2) ;
       *intZP = ( intZ1 + intZ2 ) / 2.0 ;
       nextP = endP ;
      }
    else
      {
       nextP = nextP + pointerInc ;
       if( nextP >= ovlPtsP + numOvlPts ) nextP = ovlPtsP ;
       if( nextP <  ovlPtsP             ) nextP = ovlPtsP + numOvlPts - 1 ;
      }
   }
/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writeLimitSlopeToesToDataObjectOld(long sideSlopeType,BC_DTM_OBJ *dataP,DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,long numOvlPts)
/*
** This Function Writes The Slope Toes To A Data Object
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   padType,intersectionFound ;
 double x,y,xInt=0.0,yInt=0.0,zInt=0.0,dist,dmin=0.0 ;
 DPoint3d    p3dPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE *ovlP,*ovnP,*trn1P,*trn2P,*rad1P,*rad2P,*rad3P=nullptr,*rad4P=nullptr,*crad1P,*crad2P ;
 double angle,rad2X,rad2Y,rad4X,rad4Y ;
 static long seqdbg=0 ;
/*
** Write Entry Message
*/
 ++seqdbg ;
 if( seqdbg == 2 ) dbg=0 ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object") ;
    if( sideSlopeType == 1 ) bcdtmWrite_message(0,0,0,"Closed Side Slope Element") ;
    else                     bcdtmWrite_message(0,0,0,"Open Side Slope Element") ;
   }
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Radials = %6ld",numOvlPts) ;
    for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts ; ++ovlP )
      {
       bcdtmWrite_message(0,0,0,"Radial[%6ld] T = %1ld S = %1ld Tr = %9ld ** %10.4lf %10.4lf %10.4lf",(long)(ovlP-ovlPtsP),ovlP->Type,ovlP->Status,ovlP->TruncatingRadial,ovlP->Px,ovlP->Py,ovlP->Pz) ;
      }
   }
/*
** Set Pad Type
*/
 if( sideSlopeType == 1 ) padType = 1 ;  /* Open Side Slope Element   */
 else                     padType = 0 ;  /* Closed Side Slope Element */
/*
** Store Slope Toes In Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Slope Toes In Data Object") ;
 for( ovlP = ovlPtsP ; ovlP < ovlPtsP + numOvlPts - padType ; ++ovlP )
   {
/*
**  Set Next Radial
*/
    ovnP = ovlP + 1 ;
    if( ovnP >= ovlPtsP + numOvlPts ) ovnP = ovlPtsP ;
/*
**  Write Non Truncated Slope Toe
*/
    if( ovlP->Status || ovnP->Status )
          {
/*
**    Only Write Those Slope Toes That Start Or End From A Non Truncated Radial
*/
       if( ovlP->Status && ovnP->Status )
             {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Storing None Truncated Slope Toe %6ld %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP+1)) ;
          p3dPts[0].x = ovlP->Gx ;
          p3dPts[0].y = ovlP->Gy ;
          p3dPts[0].z = ovlP->Gz ;
          p3dPts[1].x = ovnP->Gx ;
          p3dPts[1].y = ovnP->Gy ;
          p3dPts[1].z = ovnP->Gz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,ovlP->Gx,ovlP->Gy,ovlP->Gz)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,ovnP->Gx,ovnP->Gy,ovnP->Gz)) goto errexit ;
         }
/*
**    Write Truncated Slope Toe.
**    Firstly Find Closest Intersection With Corresponding Truncated Slope Toe
**    If No Intersection Extend Truncated Radial To Closest Intersection Point
*/
       else if ( ovlP->Status || ovnP->Status )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Truncated Slope Toe ** ovl = %6ld ovn = %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
/*
**        Order Slope Toe From Non Truncated End To Truncated End
*/
          if( ovlP->Status )  { rad1P = ovlP ; rad2P = ovnP ; }
          else                { rad1P = ovnP ; rad2P = ovlP ; }
          intersectionFound = false ;
/*
**        Only Process If Rad2 Is Not Truncated By Rad1
*/
          if( rad2P->TruncatingRadial != (long)(rad1P-ovlPtsP) )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Processing Truncated Slope Toe ** %6ld %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Re ordered Truncated Slope Toe ** %6ld %6ld",(long)(rad1P-ovlPtsP),(long)(rad2P-ovlPtsP)) ;
/*
**           Check For Connection To Truncating Radial Of Rad2
*/
             if( rad2P->TruncatingRadial != DTM_NULL_PNT )
               {
                if( (ovlPtsP+rad2P->TruncatingRadial)->Status == 1 )
                  {
                   x = (ovlPtsP+rad2P->TruncatingRadial)->Gx ;
                   y = (ovlPtsP+rad2P->TruncatingRadial)->Gy ;
                   dist = bcdtmMath_distance(rad1P->Gx,rad1P->Gy,x,y) ;
                   if( intersectionFound == false || dist < dmin )
                     {
                      dmin = dist ;
                      xInt = x ;
                      yInt = y ;
                      bcdtmMath_interpolatePointOnLine(rad1P->Gx,rad1P->Gy,rad1P->Gz,rad2P->Gx,rad2P->Gy,rad2P->Gz,x,y,&zInt) ;
                      crad1P = rad3P ;
                      crad2P = rad4P ;
                      intersectionFound = true ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"**** Intersection Found With Truncating Radial %6ld of Rad2",rad2P->TruncatingRadial) ;
                     }
                  }
               }
/*
**         Only Scan Those Slope Toes From Radials That Are Not Truncated Radial 1
*/
            if( ! intersectionFound )
              {
                   if( rad2P->TruncatingRadial != (long)(rad1P-ovlPtsP) )
                 {
/*
**                Scan Radials To Find Corresponding Truncated Slope Toe
*/
                  for( trn1P = ovlPtsP ; trn1P < ovlPtsP + numOvlPts - padType ; ++trn1P )
                    {
                     trn2P = trn1P + 1 ;
                     if( trn2P >= ovlPtsP + numOvlPts ) trn2P = ovlPtsP ;
/*
**                   Exclude Current Slope Toe And Adjoining Slope Toes
*/
                     if( trn1P != ovlP && trn2P != ovlP && trn1P != ovnP && trn2P != ovnP )
                       {
/*
**                      Only Process Truncated Slope Toe
*/
                        if( ( trn1P->Status && ! trn2P->Status ) || ( ! trn1P->Status && trn2P->Status ))
                          {
                           if( trn1P->Status )  { rad3P = trn1P ; rad4P = trn2P ; }
                           else                 { rad3P = trn2P ; rad4P = trn1P ; }
/*
**                         Exclude Slope Toes Where One Radial Truncates Its Next Neighbour
*/
                               if( rad4P->TruncatingRadial != (long)(rad3P-ovlPtsP) )
                             {
                              if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Checking Against Truncated Slope Toe ** %6ld %6ld",(long)(trn1P-ovlPtsP),(long)(trn2P-ovlPtsP)) ;
/*
**                            Extend Truncated Slope Toes At Truncated End - Required For Intersection Purposes
**                            To Fudge Intersections For Near Misses
*/
                              angle = bcdtmMath_getAngle(rad1P->Gx,rad1P->Gy,rad2P->Gx,rad2P->Gy) ;
                              dist  = bcdtmMath_distance(rad1P->Gx,rad1P->Gy,rad2P->Gx,rad2P->Gy) ;
                              rad2X = rad1P->Gx + cos(angle) * dist * 1.5 ;
                              rad2Y = rad1P->Gy + sin(angle) * dist * 1.5 ;
                              angle = bcdtmMath_getAngle(rad3P->Gx,rad3P->Gy,rad4P->Gx,rad4P->Gy) ;
                              dist  = bcdtmMath_distance(rad3P->Gx,rad3P->Gy,rad4P->Gx,rad4P->Gy) ;
                              rad4X = rad3P->Gx + cos(angle) * dist * 1.5 ;
                              rad4Y = rad3P->Gy + sin(angle) * dist * 1.5 ;
/*
**                           Get Intersection Point Of Truncated Slope Toes
*/
                              if( bcdtmMath_intersectCordLines(rad1P->Gx,rad1P->Gy,rad2X,rad2Y,rad3P->Gx,rad3P->Gy,rad4X,rad4Y,&x,&y) )
                                {
                                 dist = bcdtmMath_distance(rad1P->Gx,rad1P->Gy,x,y) ;
                                 if( intersectionFound == false || dist < dmin )
                                   {
                                    dmin = dist ;
                                    xInt = x ;
                                    yInt = y ;
                                    bcdtmMath_interpolatePointOnLine(rad1P->Gx,rad1P->Gy,rad1P->Gz,rad2P->Gx,rad2P->Gy,rad2P->Gz,x,y,&zInt) ;
                                    crad1P = rad3P ;
                                    crad2P = rad4P ;
                                    intersectionFound = true ;
                                    if( dbg ) bcdtmWrite_message(0,0,0,"**** Intersection Found With Slope Toe %6ld %6ld",(long)(trn1P-ovlPtsP),(long)(trn2P-ovlPtsP)) ;
                                   }
                                }
                             }
                          }
                       }
                    }
                 }
              }
/*
**           If Intersection Not Found Scan To Next Non Truncated Radial
*/
             if( intersectionFound == false )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Finding First None Trucated Slope Radial") ;
                if(bcdtmSideSlope_limitFindFirstNonTruncatedSlopeToe(ovlPtsP,numOvlPts,padType,rad1P,ovlP,ovnP,&intersectionFound,&xInt,&yInt,&zInt)) goto errexit ;
               }
/*
**           Check Intersection Found
*/
             if( intersectionFound == false )
               {
                bcdtmWrite_message(2,0,0,"==== No Intersection Found For Truncated Slope Toe") ;
                bcdtmWrite_message(0,0,0,"Slope Toe = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovlP-ovlPtsP),ovlP->Type,ovlP->Status,ovlP->Px,ovlP->Py,ovlP->Pz) ;
                bcdtmWrite_message(0,0,0,"Slope Toe = %6ld S = %1ld T = %1ld ** %12.4lf %12.4lf %10.4lf",(long)(ovnP-ovlPtsP),ovnP->Type,ovnP->Status,ovnP->Px,ovnP->Py,ovnP->Pz) ;
                bcdtmWrite_toFileDtmObject(dataP,L"slopeToes.dat") ;
                goto errexit ;
               }
/*
**           Write Slope Toe
*/
             else
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"==== Storing Truncated Slope Toe %6ld %6ld",(long)(ovlP-ovlPtsP),(long)(ovnP-ovlPtsP)) ;
                p3dPts[0].x = rad1P->Gx ;
                p3dPts[0].y = rad1P->Gy ;
                p3dPts[0].z = rad1P->Gz ;
                p3dPts[1].x = xInt ;
                p3dPts[1].y = yInt ;
                p3dPts[1].z = zInt ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,p3dPts,2)) goto errexit ;
//                if( bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//                if( bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullFeatureId,xInt,yInt,zInt)) goto errexit ;
               }
            }
         }
      }
   }
/*
** Write Slope Toes To Data File
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(dataP,L"slopeToes.dat") ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Limit Slope Toes To Data Object Error") ;
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
int bcdtmSideSlope_limitFindFirstNonTruncatedSlopeToe
(
 DTM_OVERLAP_RADIAL_TABLE *ovlPtsP,
 long numOvlPts,
 long padType,
 DTM_OVERLAP_RADIAL_TABLE *radP,
 DTM_OVERLAP_RADIAL_TABLE *ovlP,
 DTM_OVERLAP_RADIAL_TABLE *ovnP,
 long      *intersectionFoundP,
 double    *xIntP,
 double    *yIntP,
 double    *zIntP
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 DTM_OVERLAP_RADIAL_TABLE *scanP ;
/*
** Initialise
*/
 *intersectionFoundP = false ;
/*
**  Scan Forwards
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"==== Scanning Forwards") ;
 if( radP == ovlP )
   {
    scanP = ovnP ;
    while ( scanP != nullptr && scanP->Status != 1 )
      {
       ++scanP ;
       if( scanP >= ovlPtsP + numOvlPts )
         {
          if( padType == 1 ) scanP = nullptr ;
          else               scanP = ovlPtsP ;
         }
      }
    if( scanP != nullptr )
      {
       *xIntP = scanP->Gx ;
       *yIntP = scanP->Gy ;
       *zIntP = scanP->Gz ;
       *intersectionFoundP = true ;
      }
    else if( padType == 1 )
      {
       *xIntP = (ovlPtsP+numOvlPts-1)->Nx ;
       *yIntP = (ovlPtsP+numOvlPts-1)->Ny ;
       *zIntP = (ovlPtsP+numOvlPts-1)->Nz ;
       *intersectionFoundP = true ;
      }
   }
/*
**  Scan Backwards
*/
 if( radP == ovnP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"==== Scanning Backwards") ;
    scanP = ovlP ;
    while ( scanP != nullptr && scanP->Status != 1 )
      {
       --scanP ;
       if( scanP < ovlPtsP )
         {
          if( padType == 1 ) scanP = nullptr ;
          else               scanP = ovlPtsP + numOvlPts - 1 ;
         }
      }
    if( scanP != nullptr )
      {
       *xIntP = scanP->Gx ;
       *yIntP = scanP->Gy ;
       *zIntP = scanP->Gz ;
       *intersectionFoundP = true ;
      }
    else if( padType == 1 )
      {
       *xIntP = ovlPtsP->Nx ;
       *yIntP = ovlPtsP->Ny ;
       *zIntP = ovlPtsP->Nz ;
       *intersectionFoundP = true ;
      }
  }
/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_writeLimitInternalPadHolesToDataObject(BC_DTM_OBJ *Tin,BC_DTM_OBJ *Data,long HoleDirection)
{
 int    ret=0,dbg=0 ;
 long   sp,np,lp,clc,spnt,lpnt,node,nfeat=0,dtmFeature,numPolyPts,numFeaturePts ;
 DTMDirection Direction;
 long HullCoincidentFlag,NoHoles,numTmpFeatureCodes ;
 double Area ;
 DPoint3d    *featurePtsP=nullptr ;
 DTM_TIN_NODE   *pd ;
 static long seqdbg=0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
 /*
** Write Status Message
*/
 ++seqdbg ;
 if( seqdbg == 0 ) dbg=0;
  if( dbg ) bcdtmWrite_message(0,0,0,"Writing Internal Pad Holes To Data Object ** HoleDirection = %1ld",HoleDirection) ;
/*
** Initialise
*/
 NoHoles = 0 ;
 for( node = 0 ; node < Tin->numPoints ; ++node )
   {
    pd = nodeAddrP(Tin,node) ;
    pd->tPtr = Tin->nullPnt ;
    pd->PRGN = 0 ;
   }
/*
** Remove Dangling Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangles") ;
 if( dbg ) bcdtmWrite_toFileDtmObject(Tin,L"beforeDanglesRemoved.tin") ;
 if( bcdtmSideSlope_removeLimitDanglingBreaksDtmObject(Tin)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(Tin,L"afterDanglesRemoved.tin") ;
/*
** Scan Tin For Hole Start Points
*/
 for( node = 0 ; node < Tin->numPoints ; ++node )
   {
    pd = nodeAddrP(Tin,node) ;
/*
**  Ignore Point If Previously Processed
*/
    if( ! pd->PRGN && pd->hPtr == Tin->nullPnt )
      {
       spnt = node ;
/*
** Count Number Of DTM Features At Point
*/
       nfeat = 0 ;
       np = Tin->nullPnt ;
       clc = nodeAddrP(Tin,spnt)->fPtr ;
       while ( clc != Tin->nullPtr )
         {
          ++nfeat ;
          if( np == Tin->nullPnt ) np = flistAddrP(Tin,clc)->nextPnt ;
          clc = flistAddrP(Tin,clc)->nextPtr ;
         }
      if( dbg ) bcdtmWrite_message(0,0,0,"Point = %6ld ** Num Features = %4ld",node,nfeat) ;
/*
** If Number Of Features Is Equal To 1 Then Scan Internal To Slope Toe
*/
       if( np != Tin->nullPnt )
         {
          sp = spnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Hole Start Point = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(Tin,sp)->x,pointAddrP(Tin,sp)->y,pointAddrP(Tin,sp)->z ) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Hole Next  Point = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(Tin,np)->x,pointAddrP(Tin,np)->y,pointAddrP(Tin,np)->z ) ;
/*
** Scan Internal To And Extract Toe Slope Polygon
*/
          numPolyPts = 0 ;
          do
            {
             ++numPolyPts  ;
             nodeAddrP(Tin,sp)->tPtr = np ;
             if( HoleDirection == 2 ) { if(( lp = bcdtmList_nextClkDtmObject(Tin,np,sp)) < 0  ) goto errexit ; }
             else                     { if(( lp = bcdtmList_nextAntDtmObject(Tin,np,sp)) < 0  ) goto errexit ; }
             while ( ! bcdtmList_testForBreakLineDtmObject(Tin,lp,np) )
               {
                if( HoleDirection == 2 ) { if(( lp = bcdtmList_nextClkDtmObject(Tin,np,lp)) < 0  ) goto errexit ; }
                else                     { if(( lp = bcdtmList_nextAntDtmObject(Tin,np,lp)) < 0  ) goto errexit ; }
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"[%4ld] lp = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",numPolyPts,lp,nodeAddrP(Tin,lp)->tPtr,pointAddrP(Tin,lp)->x,pointAddrP(Tin,lp)->y,pointAddrP(Tin,lp)->z) ;
             sp = np ;
             np = lp ;
            } while ( sp != spnt && nodeAddrP(Tin,sp)->tPtr == Tin->nullPnt ) ;
/*
** Check For Loop Back
*/
          lpnt = Tin->nullPnt ;
          if( sp != spnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Loop Back Detected At Sp = %6ld Tptr = %6ld ** %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(Tin,sp)->tPtr,pointAddrP(Tin,sp)->x,pointAddrP(Tin,sp)->y,pointAddrP(Tin,sp)->z ) ;
             nodeAddrP(Tin,np)->tPtr = sp ;
             lpnt = spnt ;
             spnt = sp   ;
             numPolyPts = 0 ;
             do
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Sp = %6ld Tptr = %6ld",sp,nodeAddrP(Tin,sp)->tPtr) ;
                ++numPolyPts ;
                sp = nodeAddrP(Tin,sp)->tPtr ;
               } while ( sp != spnt ) ;
            }
/*
** Check Direction Of Tptr Polgon
*/
          if( dbg )
            {
             bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(Tin,spnt,&Area,&Direction) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Direction = %1ld Area = %10.4lf",Direction,Area) ;
            }
/*
** If Hole Boundary Is Coincident With TIN Hull Ignore Hole
*/
          HullCoincidentFlag = 0 ;
          sp = spnt ;
          do
            {
             np = nodeAddrP(Tin,sp)->tPtr ;
             if( nodeAddrP(Tin,sp)->hPtr == np || nodeAddrP(Tin,np)->hPtr == sp ) ++HullCoincidentFlag  ;
             sp = np ;
            } while ( sp != spnt ) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"HullCoincidentFlag = %6ld numPolyPts = %6ld",HullCoincidentFlag,numPolyPts) ;
          if( HullCoincidentFlag == numPolyPts || ( HoleDirection == 1 && HullCoincidentFlag ) ) numPolyPts = 2 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"numPolyPts = %6ld",numPolyPts) ;
/*
** Copy Tptr Polygon To Data Object As A Hole
*/
          if( numPolyPts > 2 )
            {
             ++NoHoles ;
             if( bcdtmList_copyTptrListFromDtmObjectToDtmObject(Tin,Data,spnt,DTMFeatureType::Hole,DTM_NULL_USER_TAG,nullFeatureId)) goto errexit ;
/*
** Mark Points On Hole
*/
             sp = spnt ;
             do
               {
                nodeAddrP(Tin,sp)->PRGN = 1 ;
                sp = nodeAddrP(Tin,sp)->tPtr ;
               } while ( sp != spnt) ;
            }
/*
** Null Out Tptr List
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Tptr List") ;
          if( bcdtmList_nullTptrListDtmObject(Tin,spnt)) goto errexit;
/*
** Null Out Lead To Loop
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Leading Tptr List") ;
          if( lpnt != Tin->nullPnt )  bcdtmList_nullTptrValuesDtmObject(Tin) ;
         }
      }
   }
/*
** Validate Holes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Holes = %4ld",NoHoles) ;
 if( NoHoles > 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Holes ** No Holes = %4ld",NoHoles) ;
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes00.dat") ;
// TODO - RobC 30/5/28
//    if( bcdtmData_validateDtmPolygonalFeatureDataObject(Data,11,12)) return(1) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Holes Completed") ;
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes01.dat") ;
   }
/*
** Write Holes As Graphic Break Lines And Slope Toes
*/
 if( NoHoles > 0 )
   {
    numTmpFeatureCodes = Data->numFeatures ;
    for( dtmFeature = 0 ; dtmFeature < numTmpFeatureCodes ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(Data,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(Data,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::GraphicBreak,Data->nullUserTag,1,&Data->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(Data,DTMFeatureType::SlopeToe,Data->nullUserTag,1,&Data->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
         }
      }
    if( dbg ) bcdtmWrite_toFileDtmObject(Data,L"holes02.dat") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Internal Pad Holes To Data Object Completed") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( dbg ) bcdtmWrite_message(1,0,0,"Error Writing Internal Pad Holes To Data Object") ;
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_removeLimitDanglingBreaksDtmObject(BC_DTM_OBJ *tinP)
/*
**
** This Function Removes Internal Dangling Break Lines
** A Dangling Break Line Is One That Does Not Have A Connecting
** Break Line At Either End
**
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  p1,p2,listPtr,numDangles;
 bool process ;
 static long seqdbg=0 ;
/*
** Remove Dangling Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Break Lines") ;
/*
** Initialise
*/
 for ( p1 = 0 ; p1 < tinP->numPoints ; ++p1 ) nodeAddrP(tinP,p1)->sPtr = 0 ;
/*
** Count Number Of Break Lines At Each Point
*/
 for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
   {
    nodeAddrP(tinP,p1)->sPtr = 0 ;
    listPtr = nodeAddrP(tinP,p1)->cPtr ;
    while ( listPtr != tinP->nullPtr )
      {
       p2      = clistAddrP(tinP,listPtr)->pntNum ;
       listPtr = clistAddrP(tinP,listPtr)->nextPtr ;
       if( nodeAddrP(tinP,p1)->hPtr != p2 && nodeAddrP(tinP,p2)->hPtr != p1 )
         {
          if( bcdtmList_testForLineOnDtmFeatureTypeDtmObject(tinP,DTMFeatureType::Breakline,p1,p2)) ++nodeAddrP(tinP,p1)->sPtr ;
         }
      }
   }
/*
** Write Number Of Break Lines At Each Point
*/
 if( dbg )
   {
    for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
      {
       if(nodeAddrP(tinP,p1)->sPtr > 0 )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] ** NumBreaks = %6ld ** %12.4lf %12.4lf %10.4lf",p1,nodeAddrP(tinP,p1)->sPtr,pointAddrP(tinP,p1)->x,pointAddrP(tinP,p1)->y,pointAddrP(tinP,p1)->z) ;
         }
      }
   }
/*
** Remove Dangling Break Lines
*/
 numDangles = 0 ;
 process = true ;
 while ( process == true )
   {
    process = false ;
    for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
      {
       if( nodeAddrP(tinP,p1)->sPtr == 1 )
         {
/*
**        Get Other End Point Of Break Line
*/
          listPtr = nodeAddrP(tinP,p1)->cPtr ;
          while ( listPtr != tinP->nullPtr )
            {
             p2      = clistAddrP(tinP,listPtr)->pntNum ;
             listPtr = clistAddrP(tinP,listPtr)->nextPtr ;
             if( bcdtmList_testForLineOnDtmFeatureTypeDtmObject(tinP,DTMFeatureType::Breakline,p1,p2))
               {
                ++numDangles ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Break At Point %6ld ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(tinP,p1)->x,pointAddrP(tinP,p1)->y,pointAddrP(tinP,p1)->z) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"                                 %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(tinP,p2)->x,pointAddrP(tinP,p2)->y,pointAddrP(tinP,p2)->z) ;
                if( bcdtmSideSlope_removeBreakLineSegmentDtmObject(tinP,p1,p2)) goto errexit ;
                --nodeAddrP(tinP,p1)->sPtr ;
                --nodeAddrP(tinP,p2)->sPtr ;
                listPtr = tinP->nullPtr  ;
                process = true ;
               }
            }
/*
**        Recount Breaks
*/
          if( process == true )
            {
             for ( p1 = 0 ; p1 < tinP->numPoints ; ++p1 ) nodeAddrP(tinP,p1)->sPtr = 0 ;
             for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
               {
                nodeAddrP(tinP,p1)->sPtr = 0 ;
                listPtr = nodeAddrP(tinP,p1)->cPtr ;
                while ( listPtr != tinP->nullPtr )
                  {
                   p2      = clistAddrP(tinP,listPtr)->pntNum ;
                   listPtr = clistAddrP(tinP,listPtr)->nextPtr ;
                   if( nodeAddrP(tinP,p1)->hPtr != p2 && nodeAddrP(tinP,p2)->hPtr != p1 )
                     {
                      if( bcdtmList_testForLineOnDtmFeatureTypeDtmObject(tinP,DTMFeatureType::Breakline,p1,p2)) ++nodeAddrP(tinP,p1)->sPtr ;
                     }
                  }
               }
            }
         }
      }
   }
/*
** Count Number Of Break Lines For Internal Points
*/
 for ( p1 = 0 ; p1 < tinP->numPoints ; ++p1 ) nodeAddrP(tinP,p1)->sPtr = 0 ;
 for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
   {
    nodeAddrP(tinP,p1)->sPtr = 0 ;
    listPtr = nodeAddrP(tinP,p1)->cPtr ;
    while ( listPtr != tinP->nullPtr )
      {
       p2      = clistAddrP(tinP,listPtr)->pntNum ;
       listPtr = clistAddrP(tinP,listPtr)->nextPtr ;
       if( nodeAddrP(tinP,p1)->hPtr != p2 && nodeAddrP(tinP,p2)->hPtr != p1 )
         {
          if( bcdtmList_testForLineOnDtmFeatureTypeDtmObject(tinP,DTMFeatureType::Breakline,p1,p2)) ++nodeAddrP(tinP,p1)->sPtr ;
         }
      }
   }
/*
** Check All Dangling Breaks Have Been Removed
*/
 for( p1 = 0 ; p1 < tinP->numPoints ; ++p1 )
   {
    if( nodeAddrP(tinP,p1)->hPtr == tinP->nullPnt )
      {
       if( nodeAddrP(tinP,p1)->sPtr == 1 )
         {
          ret = DTM_ERROR ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Dangling Break Not Removed At Point %6ld ** %12.4lf %12.4lf %10.4lf",p1,pointAddrP(tinP,p1)->x,pointAddrP(tinP,p1)->y,pointAddrP(tinP,p1)->z) ;
         }
      }
   }
/*
** Error Exit If Dangling Breaks Remain
*/
 if( ret != DTM_SUCCESS )
   {
    bcdtmWrite_message(2,0,0,"Not All Dangling Breaks Removed") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(tinP) ;
/*
** Write Stats On Number Of Dangles Removed
*/
 if( dbg && ret == DTM_SUCCESS )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dangles Removed = %6ld",numDangles) ;
    bcdtmWrite_message(0,0,0,"Removing Dangling Break Lines Completed") ;
   }
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Dangling Break Lines Error") ;
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
int bcdtmSideSlope_removeBreakLineSegmentDtmObject(BC_DTM_OBJ *tinP,long pnt1,long pnt2)
/*
** This Function Removes The Last Break Line Segment pnt1-pnt2 or pnt2-pnt1
** This Is Not A Generic Function
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long fPnt,nPnt,sPnt,listPtr,dtmFeature ;
 bool reverse;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Last Break Line Segment") ;
/*
** Scan Point 1 To Get Dtm Feature
*/
 reverse = false ;
 dtmFeature = tinP->nullPnt ;
 listPtr = nodeAddrP(tinP,pnt1)->fPtr ;
 while ( listPtr != tinP->nullPtr && dtmFeature == tinP->nullPnt )
   {
    if( flistAddrP(tinP,listPtr)->nextPnt == pnt2 )
      {
       dtmFeature = flistAddrP(tinP,listPtr)->dtmFeature ;
       listPtr = tinP->nullPtr ;
      }
    else  listPtr = flistAddrP(tinP,listPtr)->nextPtr ;
   }
/*
** Scan Point 2 To Get Dtm Feature
*/
 if( dtmFeature == tinP->nullPnt )
   {
    reverse = true ;
    listPtr = nodeAddrP(tinP,pnt2)->fPtr ;
    while ( listPtr != tinP->nullPtr && dtmFeature == tinP->nullPnt )
      {
       if( flistAddrP(tinP,listPtr)->nextPnt == pnt1 )
         {
          dtmFeature = flistAddrP(tinP,listPtr)->dtmFeature ;
          listPtr = tinP->nullPtr ;
         }
       else  listPtr = flistAddrP(tinP,listPtr)->nextPtr ;
      }
   }
/*
** Write Dtm Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature = %6ld",dtmFeature) ;
/*
** Check Feature Found
*/
 if( dtmFeature == tinP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature For Line Segment Not Found") ;
    goto errexit ;
   }
/*
** Check For Reversal Of Line Segment Direction
*/
 if( reverse == true )
   {
    long temp = pnt1 ;
    pnt1 = pnt2 ;
    pnt2 = temp;
   }
/*
** Copy Dtm Feature To Tptr List
*/
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(tinP,dtmFeature,&fPnt)) goto errexit ;
/*
** Remove Dtm Feature From Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %6ld",dtmFeature) ;
 if( bcdtmInsert_removeDtmFeatureFromDtmObject2(tinP,dtmFeature, false)) goto errexit ;
/*
** Scan To Pnt1 ;
*/
 sPnt = fPnt ;
 while( sPnt != pnt1 && sPnt != tinP->nullPnt  )
   {
    sPnt = nodeAddrP(tinP,sPnt)->tPtr ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"sPnt = %9ld",sPnt) ;
/*
** Add New Features
*/
 if( sPnt != tinP->nullPnt )
   {
    nPnt = nodeAddrP(tinP,sPnt)->tPtr ;
    nodeAddrP(tinP,sPnt)->tPtr = tinP->nullPnt ;
    if( nodeAddrP(tinP,fPnt)->tPtr != tinP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Feature ** fPnt = %9ld",fPnt) ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(tinP,nullptr,0,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,nullFeatureId,fPnt,1)) goto errexit ;
      }
    if( nodeAddrP(tinP,nPnt)->tPtr != tinP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Feature ** nPnt = %9ld",nPnt) ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(tinP,nullptr,0,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,nullFeatureId,nPnt,1)) goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Last Break Line Segment Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Last Break Line Segment Error") ;
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
int bcdtmSideSlope_truncateTruncatingRadials(DTM_OVERLAP_RADIAL_TABLE *radialsP,long numRadials)
{
 double nullValue=-999999999.0 ;
 DTM_OVERLAP_RADIAL_TABLE *rad1P,*rad2P ;
/*
**  Initialise
*/
 for( rad1P = radialsP ; rad1P < radialsP + numRadials ; ++rad1P )
   {
    rad1P->Tx = rad1P->Ty = rad1P->Tz = nullValue ;
   }
/*
** Find Closest Radial Truncated By Truncating Radial
*/
 for( rad1P = radialsP ; rad1P < radialsP + numRadials ; ++rad1P )
   {
    if( rad1P->TruncatingRadial != DTM_NULL_PNT )
      {
       rad2P = radialsP + rad1P->TruncatingRadial ;
       if( bcdtmMath_distance(rad2P->Nx,rad2P->Ny,rad1P->Nx,rad1P->Ny) <
           bcdtmMath_distance(rad2P->Nx,rad2P->Ny,rad2P->Tx,rad2P->Ty)    )
          {
           rad2P->Tx = rad1P->Nx ;
           rad2P->Ty = rad1P->Ny ;
          }
      }
   }
/*
** Truncate Truncating Radials
*/
 for( rad1P = radialsP ; rad1P < radialsP + numRadials ; ++rad1P )
   {
    if( rad1P->Tx != nullValue && rad1P->Ty != nullValue )
      {
       bcdtmMath_interpolatePointOnLine(rad1P->Px,rad1P->Py,rad1P->Pz,rad1P->Nx,rad1P->Ny,rad1P->Nz,rad1P->Tx,rad1P->Ty,&rad1P->Tz) ;
       // D-132354
       rad1P->Status = 1 ;
       rad1P->Nx     = rad1P->Tx ;
       rad1P->Ny     = rad1P->Ty ;
       rad1P->Nz     = rad1P->Tz ;
      }
   }
/*
** Return
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_getBoundaryPolygonAndSlopeToesForClosedSideSlopeElementDataObject
(
 BC_DTM_OBJ              *sideSlopesP,          /* Data Object To Store Slope Toes                   */
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed                       */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_SIDE_SLOPE_TABLE  *RightSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* Pointer To Radials To Right Of Side Slope Element */
 long                     numRightRadials,       /* Number Of Right Radials                           */
 DTM_SIDE_SLOPE_TABLE  *LeftSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
 )
{
 int ret=DTM_SUCCESS,dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Boundary Polygon And Slope Toes For Closed Side Slope Element") ;
/*
** Get Internal Holes And Slope Toes
*/
 if( sideSlopeDirection == 2 || sideSlopeDirection == 3 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Internal Slope Toes For Closed Side Slope Element") ;
    if( bcdtmSideSlope_getInternalSlopeToesForClosedSideSlopeElementDataObject(sideSlopesP,sideSlopeElementType,sideSlopeDirection,leftRadialsP,numLeftRadials)) goto errexit ;
   }
/*
** Get External Boundary Polygon And Slope Toes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting External Slope Toes For Closed Side Slope Element") ;
 if( bcdtmSideSlope_getExternalSlopeToesForClosedSideSlopeElementDataObject(sideSlopesP,sideSlopeElementType,sideSlopeDirection,rightRadialsP,numRightRadials,leftRadialsP,numLeftRadials)) goto errexit ;

/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Boundary Polygon And Slope Toes For Closed Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Boundary Polygon And Slope Toes For Closed Side Slope Element Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_getInternalSlopeToesForClosedSideSlopeElementDataObject
(
 BC_DTM_OBJ              *sideSlopesP,          /* Data Object To Store Slope Toes                   */
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed                       */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
 )
 {
  int ret=DTM_SUCCESS,dbg=0 ;
  long    point,flPtr,numTruncated,numHullPts,process;
  long    startPoint,nextPoint,listPoint;
  DTMDirection polyDirection;
  bool useNewAlgorithm=false ;
  double  polyArea ;
  DPoint3d     *p3dP,slopeToe[2],*hullPtsP=nullptr ;
  BC_DTM_OBJ *dtmP=nullptr ;
  DTM_OVERLAP_RADIAL_TABLE *rad1P,*rad2P,*firstRadialP ;
  long loop=0 ;
  DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Internal Slope Toes For Closed Side Slope Element") ;
    bcdtmWrite_message(0,0,0,"sideSlopeP           = %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"sideSlopeElementType = %8ld",sideSlopeElementType) ;
    bcdtmWrite_message(0,0,0,"sideSlopeDirection   = %8ld",sideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"leftRadialsP         = %p",leftRadialsP) ;
    bcdtmWrite_message(0,0,0,"numLeftRadials       = %8ld",numLeftRadials) ;
   }
/*
**  Get Internal Slope Toes
*/
 if( sideSlopeElementType == 2 && ( sideSlopeDirection == 2 || sideSlopeDirection == 3  ) )
   {
/*
**  Check For Truncated Left Radials
*/
    numTruncated = 0 ;
    for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P ) ++numTruncated ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Truncated Left Radials = %8ld of %8ld",numTruncated,numLeftRadials) ;
/*
**  If No Radials Are Truncated Write Side Slope Toes As Hole Polygon And Slope Toes
*/
    if( numTruncated == 0 )
      {
       if( bcdtmSideSlope_copySideSlopeRadialToePointsToPointArray(sideSlopeElementType,1,leftRadialsP,numLeftRadials,&hullPtsP,&numHullPts)) goto errexit ;
/*
**     Write Hole Polygon
*/
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
/*
**     Free Memory
*/
      if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
     }
/*
**  Resolve Intersecting Side Slope Toes
*/
   else
     {
/*
**    Use New Algorithm
**    Developement Not Yet Completed Robc 5/Nov/2007
*/
      if( useNewAlgorithm == true )
        {
/*
**       Reverse Order Of Left Radials
*/
         bcdtmSideSlope_reverseOrderOfSideSlopeRadials(leftRadialsP,numLeftRadials) ;
/*
**       Process Until No Holes Found
*/
loop = 10 ;
         process = true ;
         while ( process && loop )
           {
--loop ;
            process = false ;
/*
**          Find First Non Truncated Radial
*/
            firstRadialP = nullptr ;
            for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials && firstRadialP == nullptr ; ++rad1P )
              {
               if( rad1P->Status == 1 ) firstRadialP = rad1P ;
              }
/*
**          If First Radial Found Then There Are Holes
*/
            if( firstRadialP != nullptr )
              {
               process = true ;
/*
**             Reorder Radials So First Radial Is Non truncated
*/
               if( bcdtmSideSlope_reorderSideSlopeRadials(leftRadialsP,numLeftRadials,firstRadialP)) goto errexit ;
/*
**             Get Hole Polygon Internal To Closed Side Slope Element
*/
               if( bcdtmSideSlope_getPolygonFromClosedSideSlopeElementToes(1,&leftRadialsP,&numLeftRadials,&hullPtsP,&numHullPts)) goto errexit ;
               bcdtmMath_getPolygonDirectionP3D(hullPtsP,numHullPts,&polyDirection,&polyArea) ;
               if( dbg ) bcdtmWrite_message(0,0,0,"polyArea = %10.2lf polyDirection = %2ld",polyArea,polyDirection) ;

/*
**             Write Hole Polygon
*/
               if (polyDirection == DTMDirection::Clockwise)
                 {
                  if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                  if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                  if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                 }
/*
**             Mark And Remove Side Slope Radails On Hole Polygon
*/
               for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
                 {
                  for( p3dP = hullPtsP ; p3dP < hullPtsP + numHullPts && rad1P->Status ; ++p3dP )
                    {
                     if( rad1P->Gx == p3dP->x && rad1P->Gy == p3dP->y )
                       {
                        rad1P->Status = 10 ;
                       }
                    }
                 }
               for( rad1P = rad2P = leftRadialsP ; rad2P < leftRadialsP + numLeftRadials ; ++rad2P )
                 {
                  if( rad2P->Status != 10 )
                    {
                     if( rad1P != rad2P ) *rad1P = *rad2P ;
                     ++rad1P ;
                    }
                 }
               numLeftRadials = (long)(rad1P-leftRadialsP)  ;
/*
**             Free Hull Points Memory
*/
               if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
              }
           }
        }
/*
**    Use Old Algorithm
*/
      if( useNewAlgorithm == false )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Using Old Algorithm") ;
/*
**       Create Data Object
*/
         if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
         bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numLeftRadials*2,numLeftRadials) ;
/*
**       Write Slope Toes To Data Object
*/
         for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
           {
            rad2P = rad1P + 1 ;
            if( rad2P >= leftRadialsP + numLeftRadials ) rad2P = leftRadialsP ;
            slopeToe[0].x = rad1P->Gx ;
            slopeToe[0].y = rad1P->Gy ;
            slopeToe[0].z = rad1P->Gz ;
            slopeToe[1].x = rad2P->Gx ;
            slopeToe[1].y = rad2P->Gy ;
            slopeToe[1].z = rad2P->Gz ;
           if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,slopeToe,2) ) goto errexit ;
           }
/*
**       Triangulate Data Object
*/
         if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
/*
**       Extract Slope Toes
*/
         if(dbg) bcdtmWrite_message(0,0,0,"Extracting Internal Slope Toes") ;
         for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials - 1 ; ++rad1P )
           {
            if( rad1P->Status == 1  )
              {
               bcdtmFind_closestPointDtmObject(dtmP,rad1P->Gx,rad1P->Gy,&point) ;
               if( nodeAddrP(dtmP,point)->tPtr == dtmP->nullPnt )
                 {
/*
**                Get Next Point For Slope Toe
*/
                  nextPoint = dtmP->nullPnt ;
                  flPtr = nodeAddrP(dtmP,point)->fPtr ;
                  while( flPtr != dtmP->nullPtr )
                    {
                     if( flistAddrP(dtmP,flPtr)->nextPnt != dtmP->nullPnt ) nextPoint = flistAddrP(dtmP,flPtr)->nextPnt ;
                     flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
                    }
                  if( dbg ) bcdtmWrite_message(0,0,0,"Start Point = %6ld ** Next Point = %6ld",point,nextPoint) ;
/*
**                Extract Slope Toe Polygon
*/
                  if( nextPoint != dtmP->nullPnt )
                    {
                     startPoint = point ;
                     do
                       {
                        if( ( listPoint = bcdtmList_nextAntDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
                        while( ! bcdtmList_testForBreakLineDtmObject(dtmP,listPoint,point) )
                          {
                           if( ( listPoint = bcdtmList_nextAntDtmObject(dtmP,point,listPoint)) < 0 ) goto errexit ;
                          }
                        nodeAddrP(dtmP,point)->tPtr = listPoint ;
                        nextPoint = point ;
                        point = listPoint ;
                       } while ( point != startPoint ) ;
/*
**                   Check Connectivity Of Tptr Polygon
*/
                     if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPoint,0)) goto errexit ;
/*
**                   Calculate Direction Of Tptr Polygon
*/
                     bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&polyArea,&polyDirection) ;
                     if( dbg ) bcdtmWrite_message(0,0,0,"area = %15.4lf ** direction = %2ld",polyArea,polyDirection) ;
/*
**                   If Polygon Direction Is Clockwise Then A Slope Toe Polygon Has Been Found
*/
                     if (polyArea > 0.001 && polyDirection == DTMDirection::Clockwise)
                       {
/*
**                      Copy Tptr Polygon To Point Array
*/
                        if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&hullPtsP,&numHullPts)) goto errexit ;
/*
**                      Write Hole Points
*/
                        if( dbg )
                          {
                           bcdtmWrite_message(0,0,0,"Number Of Hole Points = %8ld",numHullPts) ;
                           for( p3dP = hullPtsP ; p3dP < hullPtsP + numHullPts ; ++p3dP )
                             {
                              bcdtmWrite_message(0,0,0,"Hole Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-hullPtsP),p3dP->x,p3dP->y,p3dP->z) ;
                             }
                          }
/*
**                      Write Hole Polygon
*/
                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                        if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
                       }
                    }
                 }
              }
           }
        }
/*
**    Destroy Data Object
*/
      if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
     }
  }
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( hullPtsP != nullptr ) free(hullPtsP) ;
/*
** Write Status Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Internal Slope Toes For Closed Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Internal Slope Toes For Closed Side Slope Element Error") ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
 }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_reorderSideSlopeRadials( DTM_OVERLAP_RADIAL_TABLE *radialsP,long numRadials,DTM_OVERLAP_RADIAL_TABLE *firstRadialP )
{
/*
** This Function Reorders The Side Slope Toe Radials
** So First Radial Is The First Entry In The Side Slope Table
*/
 int       ret=DTM_SUCCESS,dbg=0 ;
 DTM_OVERLAP_RADIAL_TABLE *rad1P,*rad2P,tempRadial ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Radials") ;
/*
** Check range
*/
 if( firstRadialP < radialsP || firstRadialP >= radialsP + numRadials )
   {
    bcdtmWrite_message(2,0,0,"Slope Toe Radial Range Error") ;
    goto errexit ;
   }
/*
**  Copy Radials
*/
 else
   {
    while( firstRadialP != radialsP )
      {
       tempRadial = *radialsP ;
       for( rad1P = radialsP , rad2P = radialsP + 1 ; rad1P < radialsP + numRadials - 1 ; ++rad1P , ++rad2P )
         {
          *rad1P = *rad2P ;
         }
       *(radialsP+numRadials-1) = tempRadial ;
       --firstRadialP ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Radials Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Reodering Side Slope Radials Error") ;
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
int bcdtmSideSlope_getExternalSlopeToesForClosedSideSlopeElementDataObject
(
 BC_DTM_OBJ              *sideSlopesP,           /* Data Object To Store Slope Toes                   */
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed                       */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* Pointer To Radials To Right Of Side Slope Element */
 long                     numRightRadials,       /* Number Of Right Radials                           */
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
 )
 {
  int     ret=DTM_SUCCESS,dbg=0 ;
  long    point,startPoint,nextPoint,numHullPts;
  bool useNewAlgorithm=true ;
  long    flPtr,listPoint,numTruncated,pointOnHull ;
  DTMDirection polyDirection;
  double  polyArea ;
  DPoint3d     slopeToe[2],*hullPtsP=nullptr ;
  BC_DTM_OBJ *dtmP=nullptr ;
  DTM_OVERLAP_RADIAL_TABLE *rad1P,*rad2P;
  DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting External Slope Toes For Closed Side Slope Element") ;
    bcdtmWrite_message(0,0,0,"sideSlopeP           = %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"sideSlopeElementType = %8ld",sideSlopeElementType) ;
    bcdtmWrite_message(0,0,0,"sideSlopeDirection   = %8ld",sideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"rightRadialsP        = %p",rightRadialsP) ;
    bcdtmWrite_message(0,0,0,"numRightRadials      = %8ld",numRightRadials) ;
   }
/*
**  Get Boundary Polygon And External Slope Toes
*/
 if( sideSlopeElementType == 2 )
   {
/*
**  Internal Side Slopes
*/
    if(  sideSlopeDirection == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying Side Slope Radial Start Points To Point Array") ;
       if( bcdtmSideSlope_copySideSlopeRadialStartPointsToPointArray(sideSlopeElementType,leftRadialsP,numLeftRadials,&hullPtsP,&numHullPts)) goto errexit ;
/*
**     Write Boundary Polygon To Side Slopes Data Object
*/
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
/*
**     Free Memory
*/
       if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
      }
/*
**  External Side Slopes - Check For Truncated Radials
*/
    else                                             // External Side Slopes
      {
       numTruncated = 0 ;
       for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials ; ++rad1P ) if( ! rad1P->Status ) ++numTruncated ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Truncated Right Radials = %8ld of %8ld ",numTruncated,numRightRadials) ;
/*
**     If No Radials Are Truncated Write Side Slope Toes As Boundary Polygon And Slope Toes
*/
       if( numTruncated == 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Copying Side Slope Slope Toes To Point Array") ;
          if( bcdtmSideSlope_copySideSlopeRadialToePointsToPointArray(sideSlopeElementType,1,rightRadialsP,numRightRadials,&hullPtsP,&numHullPts)) goto errexit ;
/*
**        Write Boundary Polygon
*/
          if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
/*
**        Free Memory
*/
          if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
         }
/*
**     Resolve Intersecting Slope Toes
*/
       else
         {
/*
**        Use new Algorithm
*/
          if( useNewAlgorithm == true )
            {
/*
**           Get Boundary Polygon
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Getting Boundary Polygon From Closed Side Slope Element Slope Toes") ;
             if( bcdtmSideSlope_getPolygonFromClosedSideSlopeElementToes(2,&rightRadialsP,&numRightRadials,&hullPtsP,&numHullPts)) goto errexit ;
/*
**           Write Boundary Polygon To Side Slopes Data Object
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Writing Boundary Polygon To Side Slopes Object ** numHullPts = %8ld",numHullPts) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
            }
/*
**        Create Data Object
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Creating DTM Object For Internal Side Slope Holes") ;
          if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
          bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numRightRadials*2,numRightRadials) ;
/*
**        Write Slope Toes To Data Object
*/
           for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials ; ++rad1P )
            {
             rad2P = rad1P + 1 ;
             if( rad2P >= rightRadialsP + numRightRadials ) rad2P = rightRadialsP ;
             slopeToe[0].x = rad1P->Gx ;
             slopeToe[0].y = rad1P->Gy ;
             slopeToe[0].z = rad1P->Gz ;
             slopeToe[1].x = rad2P->Gx ;
             slopeToe[1].y = rad2P->Gy ;
             slopeToe[1].z = rad2P->Gz ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,slopeToe,2) ) goto errexit ;
            }
/*
**        Triangulate Data Object
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object For Internal Side Slope Holes") ;
          if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"externalSlopeToes.dat") ;
          dtmP->ppTol = 0.0001 ;
          dtmP->plTol = 0.0001 ;
          if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
          if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"d:\\BSW\\temp\\externalSlopeToes.tin") ;
/*
**        Remove None Feature Hull Lines
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
          if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
          if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"d:\\BSW\\temp\\closedElement.tin") ;
/*
**        Use Old Algorithm
*/
          if( useNewAlgorithm == false )
            {
             if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
**           Write Boundary Polygon To Side Slopes Data Object
*/
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
             if( hullPtsP != nullptr ) { free( hullPtsP ) ; hullPtsP = nullptr ; }
            }
/*
**        Look For Internal Slope Toes In External Side Slopes
*/
          if(dbg) bcdtmWrite_message(0,0,0,"Extracting Internal Slope Toes") ;
          for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials ; ++rad1P )
            {
             if( rad1P->Status == 1 )
               {
                bcdtmFind_closestPointDtmObject(dtmP,rad1P->Gx,rad1P->Gy,&point) ;
                if( nodeAddrP(dtmP,point)->hPtr == DTM_NULL_PNT && nodeAddrP(dtmP,point)->tPtr == dtmP->nullPnt )
                  {
/*
**                 Get Next Point For Slope Toe
*/
                   nextPoint = dtmP->nullPnt ;
                   flPtr = nodeAddrP(dtmP,point)->fPtr ;
                   while( flPtr != dtmP->nullPtr )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"point = %6ld nextPoint = %6ld",point,flistAddrP(dtmP,flPtr)->nextPnt) ;
                      if( flistAddrP(dtmP,flPtr)->nextPnt != dtmP->nullPnt ) nextPoint = flistAddrP(dtmP,flPtr)->nextPnt ;
                      flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
                     }
                   if( nextPoint == dtmP->nullPnt ) goto cleanup ;
/*
**                 Extract Slope Toe Polygon
*/
                   pointOnHull = false ;
                   startPoint = point ;
                   do
                     {
                      if( nodeAddrP(dtmP,point)->hPtr != DTM_NULL_PNT ) pointOnHull = true ;
                      if( ( listPoint = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
                      while( ! bcdtmList_testForBreakLineDtmObject(dtmP,listPoint,point) )
                        {
                         if( ( listPoint = bcdtmList_nextClkDtmObject(dtmP,point,listPoint)) < 0 ) goto errexit ;
                        }
                      nodeAddrP(dtmP,point)->tPtr = listPoint ;
                      nextPoint = point ;
                      point = listPoint ;
                     } while ( point != startPoint ) ;
/*
**                 Only Process If Hole Point Not On Tin Hull
*/
                   if( pointOnHull == false )
                     {
/*
**                    Check Connectivity Of Tptr Polygon
*/
                      if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPoint,0)) goto errexit ;
/*
**                    Calculate Direction Of Tptr Polygon
*/
                      bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&polyArea,&polyDirection) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf ** Direction = %2ld",polyArea,polyDirection) ;
/*
**                    If Polygon Direction Is Clockwise Then An Internal Hole Has Been Found
*/
                      if (polyArea > 0.001 && polyDirection == DTMDirection::Clockwise)
                        {
/*
**                       Copy Tptr Polygon To Point Array
*/
                         if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&hullPtsP,&numHullPts)) goto errexit ;
/*
**                       Write Hole Polygon
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Writing Hole") ;
                         if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                         if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                         if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                         if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
                        }
                     }
                  }
               }
            }
/*
**        Destroy Data Object
*/
          if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( hullPtsP != nullptr ) free(hullPtsP) ;
/*
** Write Status Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting External Slope Toes For Closed Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting External Slope Toes For Closed Side Slope Element Error") ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_copySideSlopeRadialStartPointsToPointArray
(
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed   */
 DTM_OVERLAP_RADIAL_TABLE *sideSlopeRadialsP,    /* ==> Pointer To Radials        */
 long                     numSideSlopeRadials,   /* ==> Number Of Radials         */
 DPoint3d                      **elemPtsPP,           /* <== Radial Element Points     */
 long                     *numElemPtsP           /* <== Number of Radials         */
 )
{
 int ret=DTM_SUCCESS ;
 DPoint3d *p3dP ;
 DTM_OVERLAP_RADIAL_TABLE *radP ;
/*
** Initialise
*/
 *numElemPtsP = numSideSlopeRadials ;
 if( sideSlopeElementType == 2 ) ++*numElemPtsP ;
/*
** Allocate Memory
*/
 *elemPtsPP = ( DPoint3d * ) malloc ( *numElemPtsP * sizeof(DPoint3d)) ;
 if( *elemPtsPP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store Points
*/
 for( p3dP = *elemPtsPP , radP = sideSlopeRadialsP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++p3dP , ++radP )
   {
    p3dP->x = radP->Px ;
    p3dP->y = radP->Py ;
    p3dP->z = radP->Pz ;
   }
 if( sideSlopeElementType == 2 )
   {
    p3dP->x = sideSlopeRadialsP->Px ;
    p3dP->y = sideSlopeRadialsP->Py ;
    p3dP->z = sideSlopeRadialsP->Pz ;
   }
/*
** CleanUp
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_copySideSlopeRadialToePointsToPointArray
(
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed              */
 long                     toePointOption,        /* Toe Point Option <1 Ground, 2 Truncated> */
 DTM_OVERLAP_RADIAL_TABLE *sideSlopeRadialsP,    /* ==> Pointer To Radials                   */
 long                     numSideSlopeRadials,   /* ==> Number Of Radials                    */
 DPoint3d                      **elemPtsPP,           /* <== Radial Element Points                */
 long                     *numElemPtsP           /* <== Number of Radials                    */
 )
{
 int ret=DTM_SUCCESS ;
 DPoint3d *p3dP ;
 DTM_OVERLAP_RADIAL_TABLE *radP ;
/*
** Initialise
*/
 *numElemPtsP = numSideSlopeRadials ;
 if( sideSlopeElementType == 2 ) ++*numElemPtsP ;
/*
** Allocate Memory
*/
 *elemPtsPP = ( DPoint3d * ) malloc ( *numElemPtsP * sizeof(DPoint3d)) ;
 if( *elemPtsPP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store Points
*/
 for( p3dP = *elemPtsPP , radP = sideSlopeRadialsP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++p3dP , ++radP )
   {
    if( toePointOption == 1 )
      {
       p3dP->x = radP->Gx ;
       p3dP->y = radP->Gy ;
       p3dP->z = radP->Gz ;
      }
    else
      {
       p3dP->x = radP->Nx ;
       p3dP->y = radP->Ny ;
       p3dP->z = radP->Nz ;
      }
   }
 if( sideSlopeElementType == 2 )
   {
    if( toePointOption == 1 )
      {
       p3dP->x = sideSlopeRadialsP->Gx ;
       p3dP->y = sideSlopeRadialsP->Gy ;
       p3dP->z = sideSlopeRadialsP->Gz ;
      }
    else
      {
       p3dP->x = sideSlopeRadialsP->Nx ;
       p3dP->y = sideSlopeRadialsP->Ny ;
       p3dP->z = sideSlopeRadialsP->Nz ;
      }
   }
/*
** CleanUp
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmSideSlope_getPolygonFromClosedSideSlopeElementToesOld
(
 DTMDirection               direction,            /* ==> Polygon Direction  < 1 = CW , 2 = CCw >  */
 DTM_OVERLAP_RADIAL_TABLE *sideSlopeRadialsP,    /* ==> Pointer To Side Slope Radials            */
 long                     numSideSlopeRadials,  /* ==> Number Of Side Slope Radials             */
 DPoint3d                      **hullPtsPP,           /* <== Pointer To Boundary Polygon Points       */
 long                     *numHullPtsP           /* <== Number Of Boundary Polygon Points        */
)
/*
**
** Notes :-
**
** 1. The Knot Removal Algorithm Is Direction Dependent.
** 2. To get An External Polygon The Polygon Direction Must Be Anti Clockwise
** 3. To get An Internal Polygon The Polygon Direction Must Be Clockwise
**
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    numSlopeToePts,numKnotPts;
 bool process;
 DTMDirection toeDirection ;
 double  toeArea ;
 DPoint3d     *p3dP,*slopeToePtsP=nullptr ;
 DTM_STR_INT_PTS *knotPtsP=nullptr ;
 DTM_OVERLAP_RADIAL_TABLE *radP,*startRightRadialP;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Polygon For Side Slope Toes") ;
/*
** Initialise
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != nullptr ) { free(*hullPtsPP) ; *hullPtsPP = nullptr ; }
/*
** Copy Slope Toe Points To Point Array
*/
 if( bcdtmSideSlope_copySideSlopeRadialToePointsToPointArray(2,1,sideSlopeRadialsP,numSideSlopeRadials,&slopeToePtsP,&numSlopeToePts)) goto errexit ;
 if( bcdtmMath_getPolygonDirectionP3D(slopeToePtsP,numSlopeToePts,&toeDirection,&toeArea)) goto errexit ;
 if ((direction == DTMDirection::Clockwise && toeDirection == DTMDirection::AntiClockwise) || (direction == DTMDirection::AntiClockwise && toeDirection == DTMDirection::Clockwise))
   {
    if( bcdtmMath_reversePolygonDirectionP3D(slopeToePtsP,numSlopeToePts)) goto errexit ;
   }
/*
** Remove Knots From Slope Toe Polygon
*/
 if( bcdtmSideSlope_removeKnots(&slopeToePtsP,&numSlopeToePts,&knotPtsP,&numKnotPts)) goto errexit ;
 if( knotPtsP != nullptr ) { free( knotPtsP) ; knotPtsP     = nullptr ; }
/*
** Remove Truncated Radials Whose Slope Toe Intersects Boundary Polygon
*/
 bcdtmWrite_message(0,0,0,"Removing Truncated Radials That Intersect Boundary Polygon") ;
 process = true ;
 while ( process == true )
   {
    process = false ;
    for( radP = sideSlopeRadialsP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )
      {
       if( radP->Status == 0 )
         {
          for( p3dP = slopeToePtsP ; p3dP < slopeToePtsP + numSlopeToePts ; ++p3dP )
            {
             if( ( radP->Gx != p3dP->x || radP->Gy != p3dP->y ) && ( radP->Gx != (p3dP+1)->x || radP->Gy != (p3dP+1)->y ))
               {
                if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y) )
                  {
                   radP->Status = 3 ;
                   process = true ;
                  }
               }
            }
         }
      }
/*
**  Recalculate Slope Toes
*/
    if( process == true )
      {
       if( slopeToePtsP != nullptr ) { free( slopeToePtsP) ; slopeToePtsP = nullptr ; }
       startRightRadialP = sideSlopeRadialsP ;
       while( startRightRadialP->Status == 3 ) ++startRightRadialP ;
       numSlopeToePts = 0 ;
       for( radP = startRightRadialP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )if( radP->Status != 3 ) ++numSlopeToePts ;
       ++numSlopeToePts ;
       slopeToePtsP = ( DPoint3d * ) malloc(numSlopeToePts*sizeof(DPoint3d)) ;
       if( slopeToePtsP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       p3dP = slopeToePtsP ;
       for( radP = startRightRadialP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )
         {
          if( radP->Status != 3 )
            {
             p3dP->x = radP->Gx ;
             p3dP->y = radP->Gy ;
             p3dP->z = radP->Gz ;
             ++p3dP ;
            }
         }
       for( radP = sideSlopeRadialsP ; radP <= startRightRadialP ; ++radP )
         {
          p3dP->x = radP->Gx ;
          p3dP->y = radP->Gy ;
          p3dP->z = radP->Gz ;
         }
       if( bcdtmSideSlope_removeKnots(&slopeToePtsP,&numSlopeToePts,&knotPtsP,&numKnotPts)) goto errexit ;
       if( knotPtsP     != nullptr ) { free( knotPtsP)     ; knotPtsP     = nullptr ; }
      }
   }
/*
** Remove Truncated Radials Whose Slope Toe Intersects Boundary Polygon
*/
 bcdtmWrite_message(0,0,0,"Adjust Slope Toes Of Non Truncated Radials That Intersect Boundary Polygon") ;
 process = true ;
 while ( process == true )
   {
    process = false ;
    for( radP = sideSlopeRadialsP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )
      {
       if( radP->Status == 1 )
         {
          for( p3dP = slopeToePtsP ; p3dP < slopeToePtsP + numSlopeToePts ; ++p3dP )
            {
             if( ( radP->Gx != p3dP->x || radP->Gy != p3dP->y ) && ( radP->Gx != (p3dP+1)->x || radP->Gy != (p3dP+1)->y ))
               {
                if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y) )
                  {
                       bcdtmMath_intersectCordLines(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y,&radP->Nx,&radP->Ny)  ;
                   bcdtmMath_interpolatePointOnLine(radP->Px,radP->Py,radP->Pz,radP->Gx,radP->Gy,radP->Gz,radP->Nx,radP->Ny,&radP->Nz) ;
                   radP->Gx = radP->Nx ;
                   radP->Gy = radP->Ny ;
                   radP->Gz = radP->Nz ;
                   process = true ;
                  }
               }
            }
         }
      }
/*
**  Recalculate Slope Toes
*/
    if( process == true )
      {
       if( slopeToePtsP != nullptr ) { free( slopeToePtsP) ; slopeToePtsP = nullptr ; }
       startRightRadialP = sideSlopeRadialsP ;
       while( startRightRadialP->Status == 3 ) ++startRightRadialP ;
       numSlopeToePts = 0 ;
       for( radP = startRightRadialP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )if( radP->Status != 3 ) ++numSlopeToePts ;
       ++numSlopeToePts ;
       slopeToePtsP = ( DPoint3d * ) malloc(numSlopeToePts*sizeof(DPoint3d)) ;
       if( slopeToePtsP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       p3dP = slopeToePtsP ;
       for( radP = startRightRadialP ; radP < sideSlopeRadialsP + numSideSlopeRadials ; ++radP )
         {
          if( radP->Status != 3 )
            {
             p3dP->x = radP->Gx ;
             p3dP->y = radP->Gy ;
             p3dP->z = radP->Gz ;
             ++p3dP ;
            }
         }
       for( radP = sideSlopeRadialsP ; radP <= startRightRadialP ; ++radP )
         {
          p3dP->x = radP->Gx ;
          p3dP->y = radP->Gy ;
          p3dP->z = radP->Gz ;
         }
       if( bcdtmSideSlope_removeKnots(&slopeToePtsP,&numSlopeToePts,&knotPtsP,&numKnotPts)) goto errexit ;
       if( knotPtsP     != nullptr ) { free( knotPtsP)     ; knotPtsP     = nullptr ; }
       process = false ;  // Only Do One Loop Robc 2/Nov/2007
      }
   }
/*
** Set Return Values
*/
 *hullPtsPP   = slopeToePtsP ;
 *numHullPtsP = numSlopeToePts ;
 slopeToePtsP = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( slopeToePtsP != nullptr ) { free( slopeToePtsP) ; slopeToePtsP = nullptr ; }
 if( knotPtsP     != nullptr ) { free( knotPtsP)     ; knotPtsP     = nullptr ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon For Side Slope Toes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon For Side Slope Toes Error") ;
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

int bcdtmSideSlope_writeOpenSideSlopeElementTruncatedSlopeToesToDataObject
(
 BC_DTM_OBJ              *sideSlopesP,           /* dataP Object To Store Slope Toes                  */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_SIDE_SLOPE_TABLE  *RightSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* Pointer To Radials To Right Of Side Slope Element */
 long                     numRightRadials,       /* Number Of Right Radials                           */
 DTM_SIDE_SLOPE_TABLE  *LeftSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
 )
    {
    int          ret=DTM_SUCCESS,dbg=0 ;
    long         flPtr;
    long         side,startPoint,listPoint,numRadials ;
    long         numHullPts ;
    double       polyArea ;
    DPoint3d          *hullPtsP=nullptr;
    BC_DTM_OBJ   *dataP=nullptr ;
    DTM_OVERLAP_RADIAL_TABLE *rad1P ;
    DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID;


    if( bcdtmSideSlope_getPolygonFromOpenSideSlopeElementToes(sideSlopeDirection,rightRadialsP,numRightRadials,leftRadialsP,numLeftRadials,&hullPtsP,&numHullPts,&dataP)) goto errexit ;
    /*
    **  Store Tin Hull As Boundary Polygon In Side Slopes Object
    */
    if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,DTMUserTagConst::DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTMUserTagConst::DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }

    /*
    **  Get External Right Slope Toes
    */
    if( sideSlopeDirection  == 1 || sideSlopeDirection == 3 )
        {
        int maxRadials = numRightRadials*2;
        DTM_OVERLAP_RADIAL_TABLE *endRad1P = rightRadialsP + numRightRadials - 1;
        startPoint = -1;
        for (int iRadial = 0 ; iRadial < numRightRadials; iRadial++)
            {
            rad1P = rightRadialsP + iRadial;
            if (rad1P->Status == 1 && (RightSideSlopeTable+rad1P->Ofs)->radialStatus != 0 )
                {
                if (startPoint == -1)
                    {
                    bcdtmFind_closestPointDtmObject(dataP, rad1P->Gx, rad1P->Gy, &startPoint);
                    continue;
                    }
                }
            if (((RightSideSlopeTable+rad1P->Ofs)->radialStatus == 0 || rad1P == endRad1P) && startPoint != -1)
                {
                DTM_OVERLAP_RADIAL_TABLE *radialP = ((RightSideSlopeTable+rad1P->Ofs)->radialStatus == 0) ? rad1P - 1 : rad1P;
                if (rad1P->Status == 0)
                    continue;
                if( nodeAddrP(dataP, startPoint)->tPtr == dataP->nullPnt && nodeAddrP(dataP,startPoint)->hPtr != dataP->nullPnt )
                    {
                    long lastPoint;
                    bcdtmFind_closestPointDtmObject (dataP, radialP->Gx, radialP->Gy, &lastPoint);
                    int iPoint = 0;
                    while( startPoint != lastPoint )
                        {
                        nodeAddrP (dataP,startPoint)->tPtr = nodeAddrP(dataP,startPoint)->hPtr ;
                        startPoint = nodeAddrP(dataP,startPoint)->tPtr ;
                        if( bcdtmLoad_storePointInCache(
                            pointAddrP(dataP,startPoint)->x,
                            pointAddrP(dataP,startPoint)->y,
                            pointAddrP(dataP,startPoint)->z))
                            goto errexit ;
                        iPoint++;
                        if (iPoint > maxRadials)
                            goto errexit;
                        }
                    if (bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
                    if (numHullPts > 0)
                        {
                        if (bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts))
                            goto errexit ;
                        }
                    if (hullPtsP != nullptr)
                        { free(hullPtsP) ; hullPtsP = nullptr ;}
                    }
                startPoint = -1;
                }
            }
        }
    if( sideSlopeDirection  == 2 || sideSlopeDirection == 3 )
        {
        int maxRadials = numLeftRadials*2;
        DTM_OVERLAP_RADIAL_TABLE *endRad1P = leftRadialsP + numLeftRadials - 1;
        startPoint = -1;
        for (int iRadial = 0 ; iRadial < numLeftRadials; iRadial++)
            {
            rad1P = leftRadialsP + iRadial;
            if (rad1P->Status == 1 && (LeftSideSlopeTable+rad1P->Ofs)->radialStatus != 0)
                {
                if (startPoint == -1)
                    {
                    bcdtmFind_closestPointDtmObject(dataP, rad1P->Gx, rad1P->Gy, &startPoint);
                    continue;
                    }
                }
            if (((LeftSideSlopeTable+rad1P->Ofs)->radialStatus == 0 || rad1P == endRad1P) && startPoint != -1)
                {
                if (rad1P->Status == 0)
                    continue;
                DTM_OVERLAP_RADIAL_TABLE *radialP = ((LeftSideSlopeTable + rad1P->Ofs)->radialStatus == 0) ? rad1P - 1 : rad1P;
                if( nodeAddrP(dataP, startPoint)->tPtr == dataP->nullPnt && nodeAddrP(dataP,startPoint)->hPtr != dataP->nullPnt )
                    {
                    long lastPoint;
                    bcdtmFind_closestPointDtmObject (dataP, radialP->Gx, radialP->Gy, &lastPoint);
                    long nextPoint = bcdtmList_nextClkDtmObject(dataP, startPoint,nodeAddrP(dataP,startPoint)->hPtr);

                    int iPoint = 0;
                    while( startPoint != lastPoint )
                        {
                        if( bcdtmLoad_storePointInCache (
                            pointAddrP(dataP,nextPoint)->x,
                            pointAddrP(dataP,nextPoint)->y,pointAddrP(dataP,nextPoint)->z))
                            goto errexit ;
                        startPoint = nextPoint ;
                        if ((nextPoint = bcdtmList_nextClkDtmObject(dataP, startPoint, nodeAddrP(dataP,startPoint)->hPtr)) < 0)
                            goto errexit ;
                        iPoint++;
                        if (iPoint > maxRadials)
                            goto errexit;
                       }
                    if (bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts))
                        goto errexit ;
                    if (numHullPts > 0)
                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP, DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts))
                            goto errexit ;
                    if( hullPtsP != nullptr )
                        { free(hullPtsP) ; hullPtsP = nullptr ;}
                    }
                startPoint = -1;
                }
            }
        }
    /*
    **  Get Internal Slope Toes
    */
    for( side = 0 ; side < 2 ; ++side )
        {
        DTM_OVERLAP_RADIAL_TABLE *radialsP;
        if( side == 0 ) { radialsP   = rightRadialsP ; numRadials = numRightRadials ; }
        else            { radialsP   = leftRadialsP  ; numRadials = numLeftRadials  ; }
        if( radialsP != nullptr )
            {
            if( dbg && side == 0 ) bcdtmWrite_message(0,0,0,"Getting Internal Right Slope Toes") ;
            if( dbg && side == 1 ) bcdtmWrite_message(0,0,0,"Getting Internal Left Slope Toes") ;
            for( rad1P = radialsP ; rad1P < radialsP + numRadials  ; ++rad1P )
                {
                if( rad1P->Status && (rad1P+1)->Status )  // Must Start On A Non Truncated Slope Toe
                    {
                    bcdtmFind_closestPointDtmObject(dataP,rad1P->Gx,rad1P->Gy,&startPoint) ;
                    if( nodeAddrP(dataP,startPoint)->tPtr == dataP->nullPnt && nodeAddrP(dataP,startPoint)->hPtr == dataP->nullPnt )
                        {
                        /*
                        **                 Get Next Point For Slope Toe
                        */
                        long nextPoint = dataP->nullPnt ;
                        flPtr = nodeAddrP(dataP,startPoint)->fPtr ;
                        while( flPtr != dataP->nullPtr )
                            {
                            if( flistAddrP(dataP,flPtr)->nextPnt != dataP->nullPnt ) nextPoint = flistAddrP(dataP,flPtr)->nextPnt ;
                            flPtr = flistAddrP(dataP,flPtr)->nextPtr ;
                            }
                        if( dbg ) bcdtmWrite_message(0,0,0,"startPoint = %6ld ** nextPoint = %9ld",startPoint,nextPoint) ;
                        /*
                        **                 Extract Slope Toe Polygon
                        */
                        long pointOnTinHull = FALSE ;
                        if( nextPoint != dataP->nullPnt )
                            {
                            long point = startPoint ;
                            do
                                {
                                if( nodeAddrP(dataP,point)->hPtr != dataP->nullPnt) pointOnTinHull = TRUE ;
                                if( ( listPoint = bcdtmList_nextAntDtmObject(dataP,point,nextPoint)) < 0 ) goto errexit ;
                                while( ! bcdtmList_testForBreakLineDtmObject(dataP,listPoint,point) )
                                    {
                                    if( ( listPoint = bcdtmList_nextAntDtmObject(dataP,point,listPoint)) < 0 ) goto errexit ;
                                    }
                                nodeAddrP(dataP,point)->tPtr = listPoint ;
                                nextPoint = point ;
                                point = listPoint ;
                                } while ( point != startPoint ) ;
                                /*
                                **                    Ignore If Point On Tin Hull
                                */
                                if( pointOnTinHull == FALSE )
                                    {
                                    /*
                                    **                       Check Connectivity Of Tptr Polygon
                                    */
                                    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dataP,startPoint,0)) goto errexit ;
                                    /*
                                    **                       Calculate Direction Of Tptr Polygon
                                    */
                                    DTMDirection polyDirection;
                                    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dataP,startPoint,&polyArea,&polyDirection) ;
                                    if( dbg ) bcdtmWrite_message(0,0,0,"polyArea = %15.4lf ** polyDirectiondirection = %2ld",polyArea,polyDirection) ;
                                    /*
                                    **                       If Polygon Direction Is Clockwise Then A Slope Toe Polygon Has Been Found
                                    */
                                    if( polyArea > 0.001 && polyDirection == DTMDirection::AntiClockwise )
                                        {
                                        /*
                                        **                          Copy Tptr Polygon To Point Array
                                        */
                                        if( bcdtmList_copyTptrListToPointArrayDtmObject(dataP,startPoint,&hullPtsP,&numHullPts)) goto errexit ;
                                        /*
                                        **                          Write Hole Polygon
                                        */
                                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hole,DTMUserTagConst::DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,DTMUserTagConst::DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                                        if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,DTMUserTagConst::DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
                                        if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
                                        }
                                    }
                            }
                        }
                    }
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    if( dataP    != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
    /*
    ** Write Status Message
    */
    if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To DTM Object Completed") ;
    if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To DTM Object Error") ;
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
int bcdtmSideSlope_getBoundaryPolygonAndSlopeToesForOpenSideSlopeElementDataObject
(
 BC_DTM_OBJ              *sideSlopesP,           /* dataP Object To Store Slope Toes                  */
 long                     sideSlopeElementType,  /* ==>  1 = Open ,  2 = Closed                       */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_SIDE_SLOPE_TABLE  *RightSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* Pointer To Radials To Right Of Side Slope Element */
 long                     numRightRadials,       /* Number Of Right Radials                           */
 DTM_SIDE_SLOPE_TABLE  *LeftSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
 )
/*
** This Function Writes The Boundary Polygon To Side Slopes Data Object
*/
{
 int          ret=DTM_SUCCESS, dbg=0 ;
 long         numLeftTruncatedRadials,numRightTruncatedRadials;
 DTM_OVERLAP_RADIAL_TABLE *rad1P ;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Slope Toes For Open Side Slope Element") ;
    bcdtmWrite_message(0,0,0,"sideSlopeP           = %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"sideSlopeElementType = %8ld",sideSlopeElementType) ;
    bcdtmWrite_message(0,0,0,"sideSlopeDirection   = %8ld",sideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"rightRadialsP        = %p",rightRadialsP) ;
    bcdtmWrite_message(0,0,0,"numRightRadials      = %8ld",numRightRadials) ;
    bcdtmWrite_message(0,0,0,"leftRadialsP         = %p",leftRadialsP) ;
    bcdtmWrite_message(0,0,0,"numLeftRadials       = %8ld",numLeftRadials) ;
   }
/*
** Check For Correct Side Slope Element Type
*/
 if( sideSlopeElementType != 1 )
   {
    bcdtmWrite_message(2,0,0,"Inorrect Side Slope Element Type") ;
    goto errexit ;
   }
/*
** Write Out Radials For Debugging Purposes
*/
 if( dbg == 2 ) { if( sideSlopeDirection  == 1 || sideSlopeDirection  == 3 ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(rightRadialsP,numRightRadials,L"openElmRadialsRight.dat") ; }
 if( dbg == 2 ) { if( sideSlopeDirection  == 2 || sideSlopeDirection  == 3 ) bcdtmSideSlope_writeOverlapRadialTableToBinaryDTMFile(leftRadialsP,numLeftRadials,L"openElmRadialsleft.dat") ; }
/*
** Log Left Radials
*/
 if( dbg )
   {
    for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
      {
       bcdtmWrite_message(0,0,0,"LeftRadial[%4ld] = %2ld ** P = %10.4lf %10.4lf %10.4lf ** G = %10.4lf %10.4lf %10.4lf ** N = %10.4lf %10.4lf %10.4lf",(long)(rad1P-leftRadialsP),rad1P->Status,
                          rad1P->Px,rad1P->Py,rad1P->Pz,
                          rad1P->Gx,rad1P->Gy,rad1P->Gz,
                          rad1P->Nx,rad1P->Ny,rad1P->Nz
                        ) ;
      }
   }


/*
** Check For Truncated Right Radials
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Truncated Radials") ;
 numRightTruncatedRadials = 0 ;
 for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials ; ++rad1P )
   {
    if( ! rad1P->Status ) ++numRightTruncatedRadials ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numRightTruncatedRadials = %6ld",numRightTruncatedRadials) ;
/*
** Check For Truncated Left Radials
*/
 numLeftTruncatedRadials = 0 ;
 for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
   {
    if( ! rad1P->Status ) ++numLeftTruncatedRadials ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numLeftTruncatedRadials  = %6ld",numLeftTruncatedRadials) ;
/*
** Write None Intersecting Slope Toes To Data Object
*/
 if( ! numRightTruncatedRadials && ! numLeftTruncatedRadials )
   {
    if( bcdtmSideSlope_writeOpenSideSlopeElementNoneTruncatedSlopeToesToDataObject(sideSlopesP,sideSlopeDirection, RightSideSlopeTable, rightRadialsP,numRightRadials, LeftSideSlopeTable,leftRadialsP,numLeftRadials)) goto errexit ;
   }
/*
** Resolve Intersecting Slope Toes
*/
 else
   {
   if (bcdtmSideSlope_writeOpenSideSlopeElementTruncatedSlopeToesToDataObject (sideSlopesP,
       sideSlopeDirection, RightSideSlopeTable, rightRadialsP,numRightRadials, LeftSideSlopeTable,leftRadialsP,numLeftRadials))
        goto errexit;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Write Status Message
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To DTM Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Writing Element Boundary Polygon To DTM Object Error") ;
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
int bcdtmSideSlope_writeOpenSideSlopeElementNoneTruncatedSlopeToesToDataObject
(
 BC_DTM_OBJ               *sideSlopesP,          /* Pointer To Data Object To Store Slope Toes        */
 long                     sideSlopeDirection,    /* ==>  1 = Right , 2 = Left , 3 = Right And Left    */
 DTM_SIDE_SLOPE_TABLE  *RightSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* Pointer To Radials To Right Of Side Slope Element */
 long                     numRightRadials,       /* Number Of Right Radials                           */
 DTM_SIDE_SLOPE_TABLE  *LeftSideSlopeTable,
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* Pointer To Radials To Left Of Side Slope Element  */
 long                     numLeftRadials         /* Number Of Left Radials                            */
)
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long numHullPts ;
 DPoint3d  *hullPtsP=nullptr ;
 DTM_OVERLAP_RADIAL_TABLE  *rad1P,*rad2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Writing None Truncated Slope Toes For Open Side Slope Element") ;
    bcdtmWrite_message(0,0,0,"sideSlopeP           = %p",sideSlopesP) ;
    bcdtmWrite_message(0,0,0,"sideSlopeDirection   = %8ld",sideSlopeDirection) ;
    bcdtmWrite_message(0,0,0,"rightRadialsP        = %p",rightRadialsP) ;
    bcdtmWrite_message(0,0,0,"numRightRadials      = %8ld",numRightRadials) ;
    bcdtmWrite_message(0,0,0,"leftRadialsP         = %p",leftRadialsP) ;
    bcdtmWrite_message(0,0,0,"numLeftRadials       = %8ld",numLeftRadials) ;
   }
/*
**  Write Right Slope Toes
*/
 if( numRightRadials > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Right Slope Toes") ;
//    fcode = 40 ;
    for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials - 1 ; ++rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = RightSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          {
            if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
            if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
            if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
            continue;
          }
       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
       rad2P = rad1P + 1 ;
       if( bcdtmLoad_storePointInCache(rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//       fcode = 41 ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
      }
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
    for( rad1P = rightRadialsP + numRightRadials - 1 ; rad1P > rightRadialsP  ; --rad1P )
      {
       rad2P = rad1P - 1 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
       fcode = 41 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
      }
*/
   }
/*
**  Write Left Slope Toes
*/
 if( numLeftRadials > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Left Slope Toes") ;
//    fcode = 40 ;

    for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = LeftSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          {
            if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
            if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
            if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
            continue;
          }
       if( dbg ) bcdtmWrite_message(0,0,0,"Left Slope Toe Point[%4ld] = %12.5 %12.5lf %10.4lf",(long)(rad1P-leftRadialsP),rad1P->Gx,rad1P->Gy,rad1P->Gz) ;
       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
      }


/*
    for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials - 1 ; ++rad1P )
      {
       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
       rad2P = rad1P + 1 ;
       if( bcdtmLoad_storePointInCache(rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
       rad2P = rad1P + 1 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
       fcode = 41 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
      }
*/
/*
    for( rad1P = leftRadialsP + numLeftRadials - 1 ; rad1P > leftRadialsP  ; --rad1P )
      {
       rad2P = rad1P - 1 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
       fcode = 41 ;
       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
      }
*/
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slope Toe Points = %8ld",numHullPts) ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::SlopeToe,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
/*
**  Write Boundary Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Boundary Polygon") ;
// fcode = 4 ;
 if( sideSlopeDirection == 1 )
   {
    rad1P = rightRadialsP  ;
    if( bcdtmLoad_storePointInCache(rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//    if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
    for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials ; ++rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = RightSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;

       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
      }
    for( rad1P = rightRadialsP + numRightRadials - 1 ; rad1P >= rightRadialsP  ; --rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = RightSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;

       if( bcdtmLoad_storePointInCache(rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
      }
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
 if( sideSlopeDirection == 2 )
   {
    rad1P = leftRadialsP  ;
    if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;//
//    if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
    for( rad1P = leftRadialsP  ; rad1P < leftRadialsP + numLeftRadials ; ++rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = LeftSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;
       if( bcdtmLoad_storePointInCache(rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
      }
    for( rad1P = leftRadialsP + numLeftRadials - 1  ; rad1P >= leftRadialsP  ; --rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = LeftSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;
   if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;//
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
      }
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
 if( sideSlopeDirection == 3 )
   {
    rad1P = leftRadialsP  ;
    if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
    if( bcdtmLoad_storePointInCache(rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//    if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//    if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
    for( rad1P = rightRadialsP  ; rad1P < rightRadialsP + numRightRadials  ; ++rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = RightSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;

       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
      }
    rad1P = leftRadialsP + numLeftRadials - 1 ;
    if( bcdtmLoad_storePointInCache(rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//    if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
    for( rad1P = leftRadialsP + numLeftRadials - 1  ; rad1P >= leftRadialsP  ; --rad1P )
      {
      DTM_SIDE_SLOPE_TABLE *radial = LeftSideSlopeTable + rad1P->Ofs;
      if (radial->radialStatus == 0)
          continue;
       if( bcdtmLoad_storePointInCache(rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//       if( bcdtmObject_storePointInDataObject(sideSlopesP,fcode,DTM_NULL_USER_TAG,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
      }
    if( bcdtmLoad_getCachePoints(&hullPtsP,&numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::Hull,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( numHullPts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(sideSlopesP,DTMFeatureType::GraphicBreak,sideSlopesP->nullUserTag,1,&sideSlopesP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing None Truncated Slope Toes For Open Side Slope Element Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing None Truncated Slope Toes For Open Side Slope Element Error") ;
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
int bcdtmSideSlope_getPolygonFromClosedSideSlopeElementToes
(
 long                      direction,            /* ==> Polygon Direction  < 1 = CW , 2 = CCw >  */
 DTM_OVERLAP_RADIAL_TABLE **sideSlopeRadialsPP,  /* ==> Pointer To Side Slope Radials            */
 long                     *numSideSlopeRadialsP, /* ==> Number Of Side Slope Radials             */
 DPoint3d                      **hullPtsPP,           /* <== Pointer To Boundary Polygon Points       */
 long                     *numHullPtsP           /* <== Number Of Boundary Polygon Points        */
)
/*
**
** Notes :-
**
*/
{
 int     ret=DTM_SUCCESS,dbg=0,loop=2 ;
 long    pnt,lpnt,cpnt ;
 DPoint3d     p3dPts[2] ;
 DTM_OVERLAP_RADIAL_TABLE *radP,*rad1P,*rad2P ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Polygon From Closed Side Slope Element Slope Toes") ;
/*
** Initialise
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != nullptr ) { free(*hullPtsPP) ; *hullPtsPP = nullptr ; }
/*
** Loop And Fix
*/
 loop = 2 ;
 while( loop )
   {
    --loop ;
/*
** Store Slope Toes In Data Object
*/
    if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
    if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,*numSideSlopeRadialsP*2,*numSideSlopeRadialsP) ;
    for( rad1P = *sideSlopeRadialsPP ; rad1P < *sideSlopeRadialsPP + *numSideSlopeRadialsP  ; ++rad1P )
      {
       rad2P = rad1P + 1 ;
       if( rad2P >= *sideSlopeRadialsPP + *numSideSlopeRadialsP ) rad2P = *sideSlopeRadialsPP ;
       p3dPts[0].x = rad1P->Gx ;
       p3dPts[0].y = rad1P->Gy ;
       p3dPts[0].z = rad1P->Gz ;
       p3dPts[1].x = rad2P->Gx ;
       p3dPts[1].y = rad2P->Gy ;
       p3dPts[1].z = rad2P->Gz ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
      }
/*
**  Triangulate Data Object
*/
    if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"closedPolyDat1.dat") ;
    dtmP->ppTol = 0.0001 ;
    dtmP->plTol = 0.0001 ;
    if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"closedPolyTin1.tin") ;
/*
**  Clean Tin Hull
*/
    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"closedPolyTin2.tin") ;
/*
**  Truncate Radials At Tin Hull
*/
    if( loop )
      {
/*
**     Truncate Radials That Cut Tin Hull
*/
       for( radP = *sideSlopeRadialsPP ; radP < *sideSlopeRadialsPP + *numSideSlopeRadialsP ; ++radP )
         {
          bcdtmFind_closestPointDtmObject(dtmP,radP->Gx,radP->Gy,&cpnt) ;
          if( nodeAddrP(dtmP,cpnt)->hPtr != dtmP->nullPnt )
            {
             pnt = dtmP->hullPoint ;
             do
               {
                lpnt = nodeAddrP(dtmP,pnt)->hPtr ;
                if( pnt != cpnt && lpnt != cpnt )
                  {
                   if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,lpnt)->x,pointAddrP(dtmP,lpnt)->y ))
                     {
                      radP->Status = 3 ;
                     }
                  }
                pnt = lpnt ;
               } while ( pnt != dtmP->hullPoint ) ;
            }
         }
/*
**     Remove Type 3 Radials
*/
       for( rad1P = rad2P = *sideSlopeRadialsPP ; rad2P < *sideSlopeRadialsPP + *numSideSlopeRadialsP  ; ++rad2P )
         {
          if( rad2P->Status != 3 )
            {
             *rad1P = *rad2P ;
             ++rad1P ;
            }
         }
       *numSideSlopeRadialsP = (long)(rad1P-*sideSlopeRadialsPP) ;
      }
   }
/*
** Extract Tin Hull As Boundary Polygon
*/
 if( bcdtmList_extractHullDtmObject(dtmP,hullPtsPP,numHullPtsP)) goto errexit ;
 /*
** Clean Up
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon From Closed Side Slope Element Slope Toes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon From Closed Side Slope Element Slope Toes Error") ;
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
int bcdtmSideSlope_getPolygonFromOpenSideSlopeElementToes
(
 long                     sideSlopeDirection,    /* ==> Side Slope Direction                     */
 DTM_OVERLAP_RADIAL_TABLE *rightRadialsP,        /* ==> Pointer To Side Slope Radials            */
 long                     numRightRadials,       /* ==> Number Of Side Slope Radials             */
 DTM_OVERLAP_RADIAL_TABLE *leftRadialsP,         /* ==> Pointer To Side Slope Radials            */
 long                     numLeftRadials,        /* ==> Number Of Side Slope Radials             */
 DPoint3d                      **hullPtsPP,           /* <== Pointer To Boundary Polygon Points       */
 long                     *numHullPtsP,          /* <== Number Of Boundary Polygon Points        */
 BC_DTM_OBJ               **dtmPP                /* <== Tin Object                               */
)
/*
**
** Notes :-
**
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    side,loop,numSlopeToePts,numRadials ;
 DPoint3d     *p3dP,*slopeToePtsP=nullptr,p3dPts[2] ;
 wchar_t    datFile[]=L"openElement00.dat" ;
 wchar_t    tinFile[]=L"openElement00.tin" ;
 DTM_OVERLAP_RADIAL_TABLE *radP,*rad1P,*rad2P,*radialsP;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Polygon For Open Side Slope Toes") ;
/*
** Initialise
*/
 if( *hullPtsPP != nullptr ) { free(*hullPtsPP) ; *hullPtsPP = nullptr ; }
 if( *dtmPP     != nullptr ) bcdtmObject_destroyDtmObject(dtmPP) ;
/*
** Loop A Number Of Times To Refine Slope Toes
*/
 loop = 3 ;
 while( loop )
   {
    --loop ;
    if( loop == 2 )
      {
       wcscpy(datFile,L"openElement01.dat") ;
       wcscpy(tinFile,L"openElement01.tin") ;
      }
     else
      {
       wcscpy(datFile,L"openElement02.dat") ;
       wcscpy(tinFile,L"openElement02.tin") ;
      }

/*
**  Create Data Object For Toe Slopes
*/
    if( bcdtmObject_createDtmObject(&dtmP) ) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numRightRadials+numLeftRadials*2,1000) ;
/*
**  Write Slope Toes To Data Object
*/
    if( sideSlopeDirection == 1 || sideSlopeDirection == 3)
      {
       for( rad1P = rightRadialsP ; rad1P < rightRadialsP + numRightRadials -1; rad1P++ )
         {
          rad2P = rad1P + 1 ;
          p3dPts[0].x = rad1P->Gx ;
          p3dPts[0].y = rad1P->Gy ;
          p3dPts[0].z = rad1P->Gz ;
          p3dPts[1].x = rad2P->Gx ;
          p3dPts[1].y = rad2P->Gy ;
          p3dPts[1].z = rad2P->Gz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
         }
        for( rad1P = rightRadialsP + numRightRadials - 1 ; rad1P > rightRadialsP; rad1P-- )
         {
          rad2P = rad1P - 1 ;
          p3dPts[0].x = rad1P->Px ;
          p3dPts[0].y = rad1P->Py ;
          p3dPts[0].z = rad1P->Pz ;
          p3dPts[1].x = rad2P->Px ;
          p3dPts[1].y = rad2P->Py ;
          p3dPts[1].z = rad2P->Pz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dtmP,2,nullUserTag,nullFeatureId,rad1P->Px,rad1P->Py,rad1P->Pz)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dtmP,3,nullUserTag,nullFeatureId,rad2P->Px,rad2P->Py,rad2P->Pz)) goto errexit ;
        }
      }
    if( sideSlopeDirection == 2  || sideSlopeDirection == 3 )
      {
       for( rad1P = leftRadialsP ; rad1P < leftRadialsP + numLeftRadials -1; rad1P++ )
         {
          rad2P = rad1P + 1 ;
          p3dPts[0].x = rad1P->Px ;
          p3dPts[0].y = rad1P->Py ;
          p3dPts[0].z = rad1P->Pz ;
          p3dPts[1].x = rad2P->Px ;
          p3dPts[1].y = rad2P->Py ;
          p3dPts[1].z = rad2P->Pz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
         }
       for( rad1P = leftRadialsP + numLeftRadials - 1  ; rad1P > leftRadialsP; rad1P-- )
         {
          rad2P = rad1P - 1 ;
          p3dPts[0].x = rad1P->Gx ;
          p3dPts[0].y = rad1P->Gy ;
          p3dPts[0].z = rad1P->Gz ;
          p3dPts[1].x = rad2P->Gx ;
          p3dPts[1].y = rad2P->Gy ;
          p3dPts[1].z = rad2P->Gz ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,p3dPts,2)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dtmP,2,nullUserTag,nullFeatureId,rad1P->Gx,rad1P->Gy,rad1P->Gz)) goto errexit ;
//          if( bcdtmObject_storePointInDataObject(dtmP,3,nullUserTag,nullFeatureId,rad2P->Gx,rad2P->Gy,rad2P->Gz)) goto errexit ;
         }
      }

/*
**  Triangulate Data Object
*/
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,datFile) ;
    dtmP->ppTol  = 0.0 ;
    dtmP->plTol  = 0.0 ;
    if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
/*
**  Remove None Feature Hull Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,tinFile) ;
/*
**  Get Slope Toe Points
*/
    if( bcdtmList_extractHullDtmObject(dtmP,&slopeToePtsP,&numSlopeToePts)) goto errexit ;
/*
**  Set Tin
*/
    if( *dtmPP != nullptr ) bcdtmObject_destroyDtmObject(dtmPP) ;
    *dtmPP = dtmP ;
    dtmP = nullptr ;
/*
**  Truncate Truncated Radials That Cut Slope Toes
*/
    if( loop  )
      {
       for( side = 0 ; side < 2 ; ++side )
         {
          if( side == 0 ) { radialsP   = rightRadialsP ; numRadials = numRightRadials ; }
          else            { radialsP   = leftRadialsP  ; numRadials = numLeftRadials  ; }
          if( radialsP != nullptr )
            {
             if( dbg && side == 0 ) bcdtmWrite_message(0,0,0,"Truncating Right Truncated Radials") ;
             if( dbg && side == 1 ) bcdtmWrite_message(0,0,0,"Truncating Left Truncated Radials") ;
             for( radP = radialsP + 1  ; radP < radialsP + numRadials - 1 ; ++radP )
               {
                if( radP->Status == 0 )
                  {
                   for( p3dP = slopeToePtsP ; p3dP < slopeToePtsP + numSlopeToePts - 1  ; ++p3dP )
                     {
                      if( bcdtmMath_distance(radP->Px,radP->Py,p3dP->x,p3dP->y) > 0.001 &&
                          bcdtmMath_distance(radP->Px,radP->Py,(p3dP+1)->x,(p3dP+1)->y) > 0.001      )
                        {
                        if( ( radP->Gx != p3dP->x || radP->Gy != p3dP->y ) && ( radP->Gx != (p3dP+1)->x || radP->Gy != (p3dP+1)->y ))
                           {
                            if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y) )
                              {
                               if( dbg ) bcdtmWrite_message(0,0,0,"Truncating Radial %6ld ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",(long)(radP-radialsP),radP->Px,radP->Py,radP->Pz,radP->Gx,radP->Gy,radP->Gz) ;
                               bcdtmMath_intersectCordLines(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y,&radP->Gx,&radP->Gy)  ;
                               bcdtmMath_interpolatePointOnLine(p3dP->x,p3dP->y,p3dP->z,(p3dP+1)->x,(p3dP+1)->y,(p3dP+1)->z,radP->Gx,radP->Gy,&radP->Gz) ;
/*
                                   bcdtmMath_intersectCordLines(radP->Px,radP->Py,radP->Gx,radP->Gy,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y,&radP->Nx,&radP->Ny)  ;
                               bcdtmMath_interpolatePointOnLine(radP->Px,radP->Py,radP->Pz,radP->Gx,radP->Gy,radP->Gz,radP->Nx,radP->Ny,&radP->Nz) ;
                               radP->Gx = radP->Nx ;
                               radP->Gy = radP->Ny ;
                               radP->Gz = radP->Nz ;
                               process = true ;
*/
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
/*
**  Free Slope Toes Points
*/
    if( *hullPtsPP != nullptr ) free(*hullPtsPP) ;
    if( slopeToePtsP != nullptr )
      {
       *hullPtsPP   = slopeToePtsP ;
       *numHullPtsP = numSlopeToePts ;
       slopeToePtsP = nullptr ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( slopeToePtsP != nullptr ) { free( slopeToePtsP) ; slopeToePtsP = nullptr ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon For Side Slope Toes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Polygon For Side Slope Toes Error") ;
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
int bcdtmSideSlope_truncateRadialsInsideTruncatedSlopeToe
(
 DTM_OVERLAP_RADIAL_TABLE *radialsP,
 long numRadials,
 long sideSlopeElementType
)
/*
** This Function Truncates Radials With Toe Points Inside Truncated Slope Toes
*/
{
 int    dbg=0 ;
 long   elementType ;
 double intX,intY,xMin,xMax,yMin,yMax ;
 DTM_OVERLAP_RADIAL_TABLE  *radP,*rad1P,*rad2P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Terminating Radials Inside Truncated Slope Toes") ;
/*
** Initialise
*/
 elementType = 0 ;
 if( sideSlopeElementType == 1 ) elementType = 1 ;
/*
** Scan Radial Table And Look For Non Truncated That Intersect A Truncated Slope Toe
*/
  for( radP = radialsP ; radP < radialsP + numRadials - 1 ; ++radP )
    {
     if( radP->Status == 1 )
       {
/*
**      Scan For Overlap With Truncated Slope Toe
*/
        for( rad1P = radialsP ; rad1P < radialsP + numRadials - elementType ; ++rad1P )
          {
           rad2P = rad1P + 1 ;
           if( rad2P >= radialsP + numRadials ) rad2P = radialsP ;
           if( rad1P != radP && rad2P != radP )
             {
              if( ( ! rad1P->Status && rad2P->Status ) || ( rad1P->Status && !rad2P->Status ) )
                {
                 if( bcdtmMath_checkIfLinesIntersect(radP->Px,radP->Py,radP->Gx,radP->Gy,rad1P->Nx,rad1P->Ny,rad2P->Nx,rad2P->Ny) )
                   {
                    if( dbg )
                      {
                       bcdtmWrite_message(0,0,0,"Intersection Found") ;
                       bcdtmWrite_message(0,0,0,"Checking Radial %6ld ** %12.5lf %12.5lf ** %12.5lf %12.5lf",(long)(radP-radialsP),radP->Px,radP->Py,radP->Gx,radP->Gy) ;
                       bcdtmWrite_message(0,0,0,"**** With Radial %6ld ** %12.5lf %12.5lf ** %12.5lf %12.5lf",(long)(rad1P-radialsP),rad1P->Px,rad1P->Py,rad1P->Nx,rad1P->Ny) ;
                       bcdtmWrite_message(0,0,0,"**** With Radial %6ld ** %12.5lf %12.5lf ** %12.5lf %12.5lf",(long)(rad2P-radialsP),rad2P->Px,rad2P->Py,rad2P->Nx,rad2P->Ny) ;
                      }
                    bcdtmMath_intersectCordLines(radP->Px,radP->Py,radP->Gx,radP->Gy,rad1P->Nx,rad1P->Ny,rad2P->Nx,rad2P->Ny,&intX,&intY)  ;
/*
**                  Check Intersection Is In Length Of Radial
*/
                    if( radP->Px <= radP->Gx ) { xMin = radP->Px ; xMax = radP->Gx ; }
                    else                       { xMax = radP->Px ; xMin = radP->Gx ; }
                    if( radP->Py <= radP->Gy ) { yMin = radP->Py ; yMax = radP->Gy ; }
                    else                       { yMax = radP->Py ; yMin = radP->Gy ; }
                    if( intX >= xMin && intX <= xMax && intY >= yMin && intY <= yMax )
                      {
                       radP->Status = 0 ;
                       radP->Nx = intX ;
                       radP->Ny = intY ;
                       bcdtmMath_interpolatePointOnLine(radP->Px,radP->Py,radP->Pz,radP->Gx,radP->Gy,radP->Gz,radP->Nx,radP->Ny,&radP->Nz) ;
                      }
                   }
                }
             }
          }
       }
    }
 return(DTM_SUCCESS) ;
}

