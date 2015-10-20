/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmStockPile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
#include "bcdtmSideSlope.h"

/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmStockPile_createPointStockPileDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Pointer To Ground Surface Dtm Object
 DPoint3d        headCoordinates,       // ==> Conveyor Head Coordinates
 double     stockPileSlope,        // ==> Slope Of Stock Pile
 long       mergeOption,           // ==> If True Create Merged DTM Of StockPile And Ground Surface
 BC_DTM_OBJ **stockPileDtmPP,      // <== Pointer To Created Stock Pile DTM
 BC_DTM_OBJ **mergedDtmPP,         // <== Pointer To Merged StockPile And Ground Surface DTM
 double     *volumeP               // <== Volume Of Stock Pile
)

// This Function Creates A Stock Pile

{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   drapeFlag,startTime=bcdtmClock(),componentTime ;
 double surfaceElevation ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Point Stock Pile DTM");
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP);
    bcdtmWrite_message(0,0,0,"headCoordinates.x   = %12.5lf",headCoordinates.x);
    bcdtmWrite_message(0,0,0,"headCoordinates.y   = %12.5lf",headCoordinates.y);
    bcdtmWrite_message(0,0,0,"headCoordinates.z   = %12.5lf",headCoordinates.z);
    bcdtmWrite_message(0,0,0,"stockPileSlope      = %12.5lf",stockPileSlope);
    bcdtmWrite_message(0,0,0,"mergeOption         = %8d",mergeOption);
    bcdtmWrite_message(0,0,0,"stockPileDtmPP      = %p",*stockPileDtmPP);
    bcdtmWrite_message(0,0,0,"mergedDtmPP         = %p",*mergedDtmPP);
    bcdtmWrite_message(0,0,0,"volume              = %8.4lf",*volumeP);
   }

// Check For Valid Dtm Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

// Check DTM Is In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }

// Check Stock Pile DTM Is nullptr - Has Not Yet Been Creted

 if( *stockPileDtmPP != nullptr )
   {
    bcdtmWrite_message(1,0,0,"Stock Pile DTM Already Exists") ;
    goto errexit ;
   }

// Check Merged Stock Pile And Ground Surface DTM Is nullptr - Has Not Yet Been Creted

 if( mergeOption && *mergedDtmPP != nullptr )
   {
    bcdtmWrite_message(1,0,0,"Merged Stock Pile And Ground Surface DTM Already Exists") ;
    goto errexit ;
   }

// Check Conveyor Head Is Internal To DTM

 if( bcdtmDrape_pointDtmObject(dtmP,headCoordinates.x,headCoordinates.y,&surfaceElevation,&drapeFlag)) goto errexit ;
 if( drapeFlag != 1 )
   {
    bcdtmWrite_message(1,0,0,"Conveyor Head External To The DTM Or In A Void") ;
    goto errexit ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Head Elevation = %10.4lf Surface Elevation = %10.4lf Stock Pile Height = %10.4lf",headCoordinates.z,surfaceElevation,headCoordinates.z-surfaceElevation) ;  

// Create Stock Pile DTM

 componentTime = bcdtmClock() ;
 if( bcdtmStockPile_createPointStockPileToGroundSurfaceDtmObject(dtmP,headCoordinates,stockPileSlope,stockPileDtmPP,volumeP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Create  Point Stock Pile = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),componentTime)) ;

// Merge Stock Pile To Ground DTM

 if( mergeOption )
   {
    componentTime = bcdtmClock() ;
    if( bcdtmObject_cloneDtmObject(dtmP,mergedDtmPP)) goto errexit ;
    if( bcdtmMerge_dtmObjects(*mergedDtmPP,*stockPileDtmPP,0)) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_toFileDtmObject(*mergedDtmPP,L"mergedStockPileToGround.tin") ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Merge   Point Stock Pile = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),componentTime)) ;
   }

// Cleanup

 cleanup :

// Job Completed

 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Process Point Stock Pile = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Point Stock Pile DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Point Stock Pile DTM Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *stockPileDtmPP != nullptr ) bcdtmObject_destroyDtmObject(stockPileDtmPP) ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmStockPile_createPointStockPileToGroundSurfaceDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Pointer To Ground Surface Dtm Object
 DPoint3d        headCoordinates,       // ==> Conveyor Head Coordinates
 double     stockPileSlope,        // ==> Slope Of Stock Pile
 BC_DTM_OBJ **stockPileDtmPP,      // <== Pointer To Created Stock Pile DTM
 double     *volumeP               // <== Volume Of Stock Pile
)

// This Function Creates A Stock Pile To The Ground Surface

{

 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   n,startFlag,endFlag,drapeFlag,process ;
 DPoint3d    radial[2],surfacePoint[10] ;
 double angle,angleInc,length,fromArea,toArea,sZ,toleranceRatio=0.5/10.0,strokeTolerance=0.0  ;
 double ratio,angleBot,angleTop,lastLength,lastAngleInc ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID  ; 
 bvector<DPoint3d> slopeToePoints ;
 bvector<DPoint3d>::iterator stp ;
 bool angleIncNotFound=true ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Point Stock Pile To Ground Surface DTM");
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP);
    bcdtmWrite_message(0,0,0,"headCoordinates.x   = %12.5lf",headCoordinates.x);
    bcdtmWrite_message(0,0,0,"headCoordinates.y   = %12.5lf",headCoordinates.y);
    bcdtmWrite_message(0,0,0,"headCoordinates.z   = %12.5lf",headCoordinates.z);
    bcdtmWrite_message(0,0,0,"stockPileSlope      = %12.5lf",stockPileSlope);
    bcdtmWrite_message(0,0,0,"stockPileDtmPP      = %p",*stockPileDtmPP);
    bcdtmWrite_message(0,0,0,"volume              = %8.4lf",*volumeP);
   }

// Determine Stroking For Slope Toes

  if( bcdtmDrape_pointDtmObject(dtmP,headCoordinates.x,headCoordinates.y,&sZ,&drapeFlag)) goto errexit ;
  strokeTolerance = ( headCoordinates.z - sZ ) * toleranceRatio ;
  if( dbg ) bcdtmWrite_message(0,0,0,"Slope Toe Stroke Tolerance = %10.8lf",strokeTolerance) ;

// Calculate Angle Increment For Required Stroking Tolerance

  process  = TRUE ;
  angleBot = 0.0 ;
  angleTop = DTM_2PYE ;
  lastLength = 0 ;
  lastAngleInc = 0.0 ;
  while( process )
    {
     angleInc = ( angleTop - angleBot ) / 2.0 ;
     if( dbg ) bcdtmWrite_message(0,0,0,"**** angleInc = %12.10lf",angleInc) ;
     angle = 0.0 ;
     for( n = 0 ; n < 10 ; ++n )
       {
        if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,headCoordinates.x,headCoordinates.y,headCoordinates.z,angle,-stockPileSlope,1,0.0,&surfacePoint[n].x,&surfacePoint[n].y,&surfacePoint[n].z,&startFlag,&endFlag)) goto errexit ;
        angle = angle + angleInc ;
       }
     length = 0.0 ;
     for( n = 1 ; n < 10 ; ++n )
       {
        length = length + sqrt( ( surfacePoint[n].x - surfacePoint[n-1].x ) * ( surfacePoint[n].x - surfacePoint[n-1].x ) + ( surfacePoint[n].y - surfacePoint[n-1].y ) * ( surfacePoint[n].y - surfacePoint[n-1].y )) ;
       }
     length = length / 9.0 ;
     if( length > strokeTolerance )
       {
        lastLength = length ;
        lastAngleInc = angleInc ;
        if( length  >= strokeTolerance ) angleTop = angleInc ;
        else                             angleBot = angleInc ;
        if( dbg ) bcdtmWrite_message(0,0,0,"angleInc = %12.10lf ** length = %12.4lf strokeTolerance = %12.4lf",angleInc,length,strokeTolerance) ;
       }
     else
       {
        process = FALSE ;
       }
    }

//  Log Angle Increment Results

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"lastAngleInc = %12.10lf lastLength = %12.4lf",lastAngleInc,lastLength) ;
    bcdtmWrite_message(0,0,0,"    AngleInc = %12.10lf     Length = %12.4lf",angleInc,length) ;
   }

//  Caculate Angle Increment to Match Tolerance

 ratio = ( strokeTolerance - length ) / ( lastLength - length ) ;
 angleInc = angleInc + ratio * ( lastAngleInc - angleInc ) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"angleInc Matching Tolerance = %12.10lf",angleInc) ;

// Create DTM To Store Stock Pile

 if( bcdtmObject_createDtmObject(stockPileDtmPP)) goto errexit ;

// Scan Around Head And Shoot Off Radials To Surface

 angle = 0.0 ;
// angleInc = 0.05 ;
 radial[0].x = headCoordinates.x ;
 radial[0].y = headCoordinates.y ;
 radial[0].z = headCoordinates.z ;
 while( angle <= DTM_2PYE )
   {
    if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,radial[0].x,radial[0].y,radial[0].z,angle,-stockPileSlope,1,0.0,&radial[1].x,&radial[1].y,&radial[1].z,&startFlag,&endFlag)) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"angle = %12.10lf ** Ground = %12.5lf %12.5lf %10.4lf ** endFlag = %2ld",angle,radial[1].x,radial[1].y,radial[1].z,endFlag) ;
      }
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
    slopeToePoints.push_back(radial[1]) ;
    angle = angle + angleInc ;
   }

// Append Slope Toes To DTM

 for( stp = slopeToePoints.begin() ; stp < slopeToePoints.end() - 1  ; ++stp )
   {
    radial[0].x = stp->x ;
    radial[0].y = stp->y ;
    radial[0].z = stp->z ;
    radial[1].x = (stp+1)->x ;
    radial[1].y = (stp+1)->y ;
    radial[1].z = (stp+1)->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
   }
 stp = slopeToePoints.end() - 2 ;
 radial[0].x = stp->x ;
 radial[0].y = stp->y ;
 radial[0].z = stp->z ;
 stp = slopeToePoints.begin() ;
 radial[1].x = stp->x ;
 radial[1].y = stp->y ;
 radial[1].z = stp->z ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;

// Log Stock Pile DTM To File

 if( dbg )
   {
    bcdtmWrite_geopakDatFileFromDtmObject(*stockPileDtmPP,L"stockPile.dat") ;
   }

// Triangulate Stock Pile Object

 if( bcdtmObject_triangulateDtmObject(*stockPileDtmPP)) goto errexit ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(*stockPileDtmPP)) goto errexit ;

// Log Stock Pile DTM

 if( dbg )
   {
    bcdtmWrite_toFileDtmObject(*stockPileDtmPP,L"stockPile.tin") ;
   }

// Calculate Volume Of Stockpile

 if( bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects(*stockPileDtmPP,dtmP,nullptr,0,nullptr,nullptr,&fromArea,&toArea,volumeP)) goto errexit ;

// Cleanup

 cleanup :

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Point Stock Pile To Ground Surface DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Point Stock Pile To Ground Surface DTM Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmStockPile_createStringStockPileDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Pointer To Ground Surface Dtm Object
 DPoint3d        *headCoordinatesP,     // ==> Conveyor Head Coordinates
 long       numHeadCoordinates,    // ==> Number Of Conveyor Head Coordinates
 double     stockPileSlope,        // ==> Slope Of Stock Pile
 long       mergeOption,           // ==> If True Create Merged DTM Of StockPile And Ground Surface
 BC_DTM_OBJ **stockPileDtmPP,      // <== Pointer To Created Stock Pile DTM
 BC_DTM_OBJ **mergedDtmPP,         // <== Pointer To Merged StockPile And Ground Surface DTM
 double     *volumeP               // <== Volume Of Stock Pile
)

// This Function Creates A Stock Pile

{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numDrapePts ;
 DTM_DRAPE_POINT *drP,*drapePtsP=nullptr ;
 bvector<DTM_DRAPE_POINT> drapePts;
// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating String Stock Pile DTM");
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP);
    bcdtmWrite_message(0,0,0,"headCoordinatesP    = %p",headCoordinatesP);
    bcdtmWrite_message(0,0,0,"numHeadCoordinates  = %8ld",numHeadCoordinates);
    bcdtmWrite_message(0,0,0,"stockPileSlope      = %8.4lf",stockPileSlope);
    bcdtmWrite_message(0,0,0,"mergeOption         = %8d",mergeOption);
    bcdtmWrite_message(0,0,0,"stockPileDtmPP      = %p",*stockPileDtmPP);
    bcdtmWrite_message(0,0,0,"mergedDtmPP         = %p",*mergedDtmPP);
    bcdtmWrite_message(0,0,0,"volume              = %8.4lf",*mergedDtmPP);
   }

// Initialise

   *volumeP = 0.0 ;

// Check For Valid Dtm Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

// Check DTM Is In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }

// Check Stock Pile DTM Is nullptr - Has Not Yet Been Creted

 if( *stockPileDtmPP != nullptr )
   {
    bcdtmWrite_message(1,0,0,"Stock Pile DTM Already Exists") ;
    goto errexit ;
   }

// Check Merged Stock Pile And Ground Surface DTM Is nullptr - Has Not Yet Been Creted

 if( mergeOption && *mergedDtmPP != nullptr )
   {
    bcdtmWrite_message(1,0,0,"Merged Stock Pile And Ground Surface DTM Already Exists") ;
    goto errexit ;
   }

// Check Conveyor Head Is Internal To DTM

 if( bcdtmDrape_stringDtmObject(dtmP,headCoordinatesP,numHeadCoordinates,false,drapePts)) goto errexit ;
 drapePtsP = drapePts.data();
 numDrapePts = (long)drapePts.size();

 for( drP = drapePtsP ; drP < drapePtsP + numDrapePts ; ++drP )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"DrapePoint[%8ld] ** drapeType = %2ld ** x = %12.5lf y = %12.5lf z = %10.4lf",(long)(drP-drapePtsP),drP->drapeType,drP->drapePt.x,drP->drapePt.y,drP->drapePt.z) ;
    if (drP->drapeType == DTMDrapedLineCode::External || drP->drapeType == DTMDrapedLineCode::Void)
      {
       bcdtmWrite_message(1,0,0,"Conveyor Head Coordinate External To Tin Or In Void") ;
       goto errexit ;
      }
   }

// Create Stock Pile DTM

 if( bcdtmStockPile_createStringStockPileToGroundSurfaceDtmObject(dtmP,headCoordinatesP,numHeadCoordinates,stockPileSlope,stockPileDtmPP,volumeP)) goto errexit ;

// Merge Stock Pile To Ground DTM

 if( mergeOption )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Merging Stock Pile Into Ground Surface Tin") ;
    if( bcdtmObject_cloneDtmObject(dtmP,mergedDtmPP)) goto errexit ;
    if( bcdtmMerge_dtmObjects(*mergedDtmPP,*stockPileDtmPP,0)) goto errexit ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Writing Merging Stock Pile And Ground Surface Tin") ;
       bcdtmWrite_toFileDtmObject(*mergedDtmPP,L"mergedStringStockPileToGround.tin") ;
      }
   }

// Cleanup

 cleanup :

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating String Stock Pile DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating String Stock Pile DTM Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *stockPileDtmPP != nullptr ) bcdtmObject_destroyDtmObject(stockPileDtmPP) ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmStockPile_createStringStockPileToGroundSurfaceDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Pointer To Ground Surface Dtm Object
 DPoint3d        *headCoordinatesP,     // ==> Conveyor Head Coordinates
 long       numHeadCoordinates,    // ==> Number Of Conveyor Head Coordinates
 double     stockPileSlope,        // ==> Slope Of Stock Pile
 BC_DTM_OBJ **stockPileDtmPP,      // <== Pointer To Created Stock Pile DTM
 double     *volumeP               // <== Volume Of Stock Pile
)

// This Function Creates A Stock Pile To The Ground Surface

{

 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   startFlag,endFlag,closeFlag,sideSlopeTableSize,numDataObjects,numHullPts,numStrokePts ;
 DPoint3d    *p3dP,*p3d1P,p3dPoint,radial[2],*hullPtsP=nullptr,*strokePtsP=nullptr ;
 double angle,angleInc,startAngle,cornerStrokeTolerance=10.0,linearStrokeTolerance=10.0,fromArea,toArea  ;
 double dx,dy,dz,length,segmentLength ;
 bvector<DPoint3d> slopeToePoints ;
 bvector<DPoint3d>::iterator stp ;
 DTM_SIDE_SLOPE_TABLE *sstP,*sideSlopeTableP=nullptr ;
 BC_DTM_OBJ **dataObjectsPP=nullptr ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating String Stock Pile To Ground Surface DTM");
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP);
    bcdtmWrite_message(0,0,0,"headCoordinatesP    = %p",headCoordinatesP);
    bcdtmWrite_message(0,0,0,"numHeadCoordinates  = %8ld",numHeadCoordinates);
    bcdtmWrite_message(0,0,0,"stockPileSlope      = %8.4lf",stockPileSlope);
    bcdtmWrite_message(0,0,0,"stockPileDtmPP      = %p",*stockPileDtmPP);
    bcdtmWrite_message(0,0,0,"volume              = %8.4lf",*volumeP);
   }

// Log Stock Pile Head Coordinates

 if( dbg )
   {
    for( p3dP = headCoordinatesP ; p3dP < headCoordinatesP + numHeadCoordinates ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Head Coordinate[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-headCoordinatesP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }

// Check For Closure

   closeFlag = FALSE ;
   if(  headCoordinatesP->x == ( headCoordinatesP + numHeadCoordinates - 1)->x  &&
        headCoordinatesP->y == ( headCoordinatesP + numHeadCoordinates - 1)->y)
     {
      closeFlag = TRUE ;
     }
   if( dbg ) bcdtmWrite_message(0,0,0,"closeFlag = %2d",closeFlag) ;

// Stroke Linear Sections

   for( p3dP = headCoordinatesP ; p3dP < headCoordinatesP + numHeadCoordinates  ; ++p3dP )
     {
      slopeToePoints.push_back(*p3dP) ;
      if( p3dP < headCoordinatesP + numHeadCoordinates - 1 )
        {
         p3d1P = p3dP + 1 ;
         if( ( segmentLength = bcdtmMath_distance(p3dP->x,p3dP->y,p3d1P->x,p3d1P->y)) > linearStrokeTolerance )
           {
            dx = p3d1P->x - p3dP->x ;
            dy = p3d1P->y - p3dP->y ;
            dz = p3d1P->z - p3dP->z ;
            length = linearStrokeTolerance ;
            while( length < segmentLength )
              {
               p3dPoint.x = p3dP->x + dx * length / segmentLength ;
               p3dPoint.y = p3dP->y + dy * length / segmentLength ;
               p3dPoint.z = p3dP->z + dz * length / segmentLength ;
               slopeToePoints.push_back(p3dPoint) ;
               length = length + linearStrokeTolerance ;
              }
           }
        }
     }

//  Copy Stroked Points To Point Array

 numStrokePts =  ( long ) slopeToePoints.size() ;
 strokePtsP = ( DPoint3d * ) malloc ( numStrokePts * sizeof(DPoint3d)) ;
 if( strokePtsP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( p3dP = strokePtsP , stp = slopeToePoints.begin() ; stp < slopeToePoints.end() ; ++stp , ++p3dP )
   {
    *p3dP = *stp ;
   }

// Create Side Slope Table For Linear Stock Pile Design

 sideSlopeTableSize = numStrokePts ;
 sideSlopeTableP = ( DTM_SIDE_SLOPE_TABLE * ) malloc ( sideSlopeTableSize * sizeof(DTM_SIDE_SLOPE_TABLE )) ;
 if( sideSlopeTableP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }

// Populate Side Slope Table

 for( sstP = sideSlopeTableP , p3dP = strokePtsP  ; sstP < sideSlopeTableP + sideSlopeTableSize ; ++sstP , ++p3dP )
   {
    sstP->slopeToTin         = dtmP ;
    sstP->cutFillTin         = nullptr ;
    sstP->radialStartPoint.x = p3dP->x ;
    sstP->radialStartPoint.y = p3dP->y ;
    sstP->radialStartPoint.z = p3dP->z ;
    sstP->cutSlope           = stockPileSlope ;
    sstP->fillSlope          = stockPileSlope ;
    sstP->radialOption       = 1 ;
    sstP->sideSlopeOption    = 1 ;
    sstP->cutFillOption      = 0 ;
    sstP->isForceSlope       = 0 ;
    sstP->forcedSlope        = 0.0 ;
    sstP->useSlopeTable      = 0 ;
    sstP->isRadialDir        = 0 ;
    sstP->toElev             = 0.0 ;
   }

//  Create Stock Pile Side Slopes

 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Stock Pile") ;
 if( bcdtmSideSlope_createSideSlopesForSideSlopeTableDtmObject(&sideSlopeTableP,&sideSlopeTableSize,3,nullptr,0,1,1,cornerStrokeTolerance,0.00001,nullptr,0,DTM_NULL_USER_TAG,-10000,&dataObjectsPP,&numDataObjects) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Side Slopes For Stock Pile Completed ** numDataObjects = %4ld",numDataObjects) ;
 *stockPileDtmPP = *dataObjectsPP ;

// Log Side Slopes

 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(*stockPileDtmPP,L"stringStockPileSideSlopes.dat") ;

// Extract Hull From Side Slopes

 if( bcdtmList_extractHullDtmObject(*stockPileDtmPP,&hullPtsP,&numHullPts)) goto errexit ;

// Remove Slope Toe And Hull Features From Side Slopes

 if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(*stockPileDtmPP,DTMFeatureType::SlopeToe)) goto errexit ;
 if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(*stockPileDtmPP,DTMFeatureType::Hull)) goto errexit ;

// Scan Around Start Head Coordinate And Shoot Off Radials To Surface

 slopeToePoints.clear() ;
 startAngle = bcdtmMath_getAngle(headCoordinatesP[0].x,headCoordinatesP[0].y,headCoordinatesP[1].x,headCoordinatesP[1].y);
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"startAngle = %12.10lf",startAngle) ;
 angle = startAngle = startAngle + DTM_2PYE / 4 ;
 angleInc = 0.1 ;
 radial[0].x = headCoordinatesP[0].x ;
 radial[0].y = headCoordinatesP[0].y ;
 radial[0].z = headCoordinatesP[0].z ;
 while( angle < startAngle + DTM_2PYE / 2.0 )
   {
    if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,radial[0].x,radial[0].y,radial[0].z,angle,-stockPileSlope,1,0.0,&radial[1].x,&radial[1].y,&radial[1].z,&startFlag,&endFlag)) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"angle = %12.10lf ** Ground = %12.5lf %12.5lf %10.4lf ** endFlag = %2ld",angle,radial[1].x,radial[1].y,radial[1].z,endFlag) ;
      }
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
    slopeToePoints.push_back(radial[1]) ;
    angle = angle + angleInc ;
   }
 angle = startAngle + DTM_2PYE / 2.0 ;
 if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,radial[0].x,radial[0].y,radial[0].z,angle,-stockPileSlope,1,0.0,&radial[1].x,&radial[1].y,&radial[1].z,&startFlag,&endFlag)) goto errexit ;
 slopeToePoints.push_back(radial[1]) ;

// Append Slope Toes To DTM

 for( stp = slopeToePoints.begin() ; stp < slopeToePoints.end() - 1  ; ++stp )
   {
    radial[0].x = stp->x ;
    radial[0].y = stp->y ;
    radial[0].z = stp->z ;
    radial[1].x = (stp+1)->x ;
    radial[1].y = (stp+1)->y ;
    radial[1].z = (stp+1)->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
   }

// Scan Around End Head Coordinate And Shoot Off Radials To Surface

 slopeToePoints.clear() ;
 startAngle = bcdtmMath_getAngle(headCoordinatesP[numHeadCoordinates-1].x,headCoordinatesP[numHeadCoordinates-1].y,headCoordinatesP[numHeadCoordinates-2].x,headCoordinatesP[numHeadCoordinates-2].y) ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"endAngle   = %12.10lf",startAngle) ;
 angle = startAngle = startAngle + DTM_2PYE / 4 ;
 angle = startAngle ;
 angleInc = 0.1 ;
 radial[0].x = headCoordinatesP[numHeadCoordinates-1].x ;
 radial[0].y = headCoordinatesP[numHeadCoordinates-1].y ;
 radial[0].z = headCoordinatesP[numHeadCoordinates-1].z ;
 while( angle < startAngle + DTM_2PYE / 2.0 )
   {
    if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,radial[0].x,radial[0].y,radial[0].z,angle,-stockPileSlope,1,0.0,&radial[1].x,&radial[1].y,&radial[1].z,&startFlag,&endFlag)) goto errexit ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"angle = %12.10lf ** Ground = %12.5lf %12.5lf %10.4lf ** endFlag = %2ld",angle,radial[1].x,radial[1].y,radial[1].z,endFlag) ;
      }
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
    slopeToePoints.push_back(radial[1]) ;
    angle = angle + angleInc ;
   }
 angle = startAngle + DTM_2PYE / 2.0 ;
 if( bcdtmSideSlope_intersectSurfaceDtmObject(dtmP,radial[0].x,radial[0].y,radial[0].z,angle,-stockPileSlope,1,0.0,&radial[1].x,&radial[1].y,&radial[1].z,&startFlag,&endFlag)) goto errexit ;
 slopeToePoints.push_back(radial[1]) ;

// Append Slope Toes To DTM

 for( stp = slopeToePoints.begin() ; stp < slopeToePoints.end() - 1  ; ++stp )
   {
    radial[0].x = stp->x ;
    radial[0].y = stp->y ;
    radial[0].z = stp->z ;
    radial[1].x = (stp+1)->x ;
    radial[1].y = (stp+1)->y ;
    radial[1].z = (stp+1)->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,radial,2)) goto errexit ;
   }

// Append Hull Points To Stock Pile

 if( bcdtmObject_storeDtmFeatureInDtmObject(*stockPileDtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,hullPtsP,numHullPts)) goto errexit ;

// Log Stock Pile DTM To File

 if( dbg )
   {
    bcdtmWrite_geopakDatFileFromDtmObject(*stockPileDtmPP,L"stockPile.dat") ;
   }

// Triangulate Stock Pile Object

 if( bcdtmObject_triangulateDtmObject(*stockPileDtmPP)) goto errexit ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(*stockPileDtmPP)) goto errexit ;

// Calculate Volume Of Stockpile

 if( bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects(*stockPileDtmPP,dtmP,nullptr,0,nullptr,nullptr,&fromArea,&toArea,volumeP)) goto errexit ;

// Log Stock Pile DTM

 if( dbg )
   {
    bcdtmWrite_toFileDtmObject(*stockPileDtmPP,L"stockPile.tin") ;
   }

// Cleanup

 cleanup :
 if( hullPtsP        != nullptr ) { free(hullPtsP)        ; hullPtsP        = nullptr ; }
 if( strokePtsP      != nullptr ) { free(strokePtsP)      ; strokePtsP      = nullptr ; }
 if( dataObjectsPP   != nullptr ) { free(dataObjectsPP)   ; dataObjectsPP   = nullptr ; }
 if( sideSlopeTableP != nullptr ) { free(sideSlopeTableP) ; sideSlopeTableP = nullptr ; }

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating String Stock Pile To Ground Surface DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating String Stock Pile To Ground Surface DTM Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
