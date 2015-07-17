/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageList.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>

extern int DrainageDebug ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainageList_copyTptrListToPointListDtmObject
(
 BC_DTM_OBJ     *dtmP,                        // ==> Pointer To DTM Object
 long           startPoint,                   // ==> Start Point On Tptr List
 DTMPointList&  pointList                     // <== Point List
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   numPointList,*pointListP=NULL ;   

// Log Entry Parameters

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Copying Tptr List To Point List") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint  = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"pointList   = %p",pointList) ;
   }

// Initialise   

   if( pointList.pointsP != NULL ) 
     {
      delete [] pointList.pointsP ;
      pointList.pointsP = NULL ;
     }
   pointList.numPoints = 0 ;  
   
// Copy Tptr List

  if( bcdtmList_copyTptrListToPointListDtmObject(dtmP,startPoint,&pointListP,&numPointList)) goto errexit ;
  
// Assign To Point List Structure

  pointList.pointsP   = ( int * ) pointListP ;
  pointList.numPoints = numPointList ;
  pointListP          = NULL ;
     
// Clean Up

 cleanup :
 if( pointListP != NULL ) free(pointListP) ;

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tptr List To Point List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tptr List To Point List Error") ;
 return(ret) ;

// Error Exit

 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainageList_copySptrListToPointListDtmObject
(
 BC_DTM_OBJ     *dtmP,                        // ==> Pointer To DTM Object
 long           startPoint,                   // ==> Start Point On Sptr List
 DTMPointList&  pointList                     // <== Point List
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   numPointList,*pointListP=NULL ;   

// Log Entry Parameters

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Copying Sptr List To Point List") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint  = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"pointList   = %p",pointList) ;
   }

// Initialise   

   if( pointList.pointsP != NULL ) 
     {
      delete [] pointList.pointsP ;
      pointList.pointsP = NULL ;
     }
   pointList.numPoints = 0 ;  
   
// Copy Tptr List

  if( bcdtmList_copySptrListToPointListDtmObject(dtmP,startPoint,&pointListP,&numPointList)) goto errexit ;
  
// Assign To Point List Structure

  pointList.pointsP   = ( int * ) pointListP ;
  pointList.numPoints = numPointList ;
  pointListP          = NULL ;
     
// Clean Up

 cleanup :
 if( pointListP != NULL ) free(pointListP) ;

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Sptr List To Point List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Sptr List To Point List Error") ;
 return(ret) ;

// Error Exit

 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainageList_copyPointListToTptrListDtmObject
(
 BC_DTM_OBJ     *dtmP,                        // ==> Pointer To DTM Object
 DTMPointList&  pointList,                    // ==> Point List
 long           *startPointP                  // <== Start Point On Tptr List
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   pnt,npnt ;

// Log Entry Parameters

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Copying Point List To Tptr List") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointList   = %p",pointList) ;
    bcdtmWrite_message(0,0,0,"startPointP = %8ld",*startPointP) ;
   }

// Initialise   

 *startPointP = dtmP->nullPnt ;
 
// Copy Point List

 if( pointList.numPoints > 1 )
   {
   
//  Check For Point Range Error
   
    if( pointList.pointsP[0] < 0 || pointList.pointsP[0] >= dtmP->numPoints )
      {
       bcdtmWrite_message(1,0,0,"Point Range Error") ;
       goto errexit ;
      } 
    pnt = *startPointP = pointList.pointsP[0] ;
    for( int n = 1 ; n < pointList.numPoints ; ++n )
      {
       npnt = pointList.pointsP[n] ;
   
//     Check For Point Range Error
   
       if( npnt < 0 || npnt >= dtmP->numPoints )
         {
          bcdtmWrite_message(1,0,0,"Point Range Error") ;
          goto errexit ;
         } 
       nodeAddrP(dtmP,pnt)->tPtr = npnt ;
       pnt  = npnt ;
      }
   }
   
// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Point List To Tptr List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Point List To Tptr List Error") ;
 return(ret) ;

// Error Exit

 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainageList_copyPointListToSptrListDtmObject
(
 BC_DTM_OBJ     *dtmP,                        // ==> Pointer To DTM Object
 DTMPointList&  pointList,                    // ==> Point List
 long           *startPointP                  // <== Start Point On Sptr List
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   pnt,npnt ;

// Log Entry Parameters

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Copying Point List To Sptr List") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointList   = %p",pointList) ;
    bcdtmWrite_message(0,0,0,"startPointP = %8ld",*startPointP) ;
   }

// Initialise   

 *startPointP = dtmP->nullPnt ;
 
// Copy Point List

 if( pointList.numPoints > 1 )
   {
   
//  Check For Point Range Error
   
    if( pointList.pointsP[0] < 0 || pointList.pointsP[0] >= dtmP->numPoints )
      {
       bcdtmWrite_message(1,0,0,"Point Range Error") ;
       goto errexit ;
      } 
      
    pnt = *startPointP = pointList.pointsP[0] ;
    for( int n = 1 ; n < pointList.numPoints ; ++n )
      {
       npnt = pointList.pointsP[n] ;
   
//     Check For Point Range Error
   
       if( npnt < 0 || npnt >= dtmP->numPoints )
         {
          bcdtmWrite_message(1,0,0,"Point Range Error") ;
          goto errexit ;
         } 
       nodeAddrP(dtmP,pnt)->sPtr = npnt ;
       pnt  = npnt ;
      }
   }
   
// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Point List To Sptr List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Point List To Sptr List Error") ;
 return(ret) ;

// Error Exit

 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainageList_expandTptrPolygonAtPointDtmObject
( 
 BC_DTM_OBJ *dtmP,                        // ==> Pointer To DTM Object
 long       *pointP,                      // <=> Start Pointer Of TpTr Polygon
 long       *extStartPntP,                // <== Start Point Of Extended Section
 long       *extEndPntP                   // <== End Point Of Extended Section
)

// This Function Expands A Tptr Polygon At A Tptr Polygon Point

{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(1) ;
 long   expandType,pnt,antPoint,nextPnt,cleanPnt,pPoint ;   
 long   point,priorPoint,nextPoint,startPoint,expansionNextPoint,expansionPriorPoint ;
 double beforeArea = 0,afterArea ;
 DTMDirection direction ;


// Log Entry Parameters

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointP      = %8ld",*pointP) ;
    bcdtmWrite_message(0,0,0,"extStartPnt = %8ld",*extStartPntP) ;
    bcdtmWrite_message(0,0,0,"extEndPnt   = %8ld",*extEndPntP) ;
    if( dbg == 2 ) 
      {
       bcdtmList_writeTptrListDtmObject(dtmP,*pointP) ;
       DPoint3d *tptrPtsP=NULL ;
       BC_DTM_OBJ *temP=NULL ;
       long     numTptrPts=0 ;
       if( bcdtmObject_createDtmObject(&temP)) goto errexit ;
       if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,*pointP,&tptrPtsP,&numTptrPts) ) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(temP,DTMFeatureType::Breakline,temP->nullUserTag,1,&temP->nullFeatureId,tptrPtsP,numTptrPts)) goto errexit ;
       if( bcdtmWrite_geopakDatFileFromDtmObject(temP,L"expansionPolygon.dat")) goto errexit ;
       if( tptrPtsP != NULL ) free(tptrPtsP) ;
       if( temP != NULL ) bcdtmObject_destroyDtmObject(&temP) ; 
      }
   }

// Initialise   

 *extStartPntP = dtmP->nullPnt ;
 *extEndPntP   = dtmP->nullPnt ;

// Only Expand For An Internal Point

 if( nodeAddrP(dtmP,*pointP)->hPtr == dtmP->nullPnt )
   {   

//  Perform Connectivity And Area Checks

    if( cdbg )
      {
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*pointP,0))
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Connectivity Error In Tptr Polygon") ;
          bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
          goto errexit ;
         }
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pointP,&beforeArea,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon Area      = %15.5lf",beforeArea) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon Direction = %2ld",direction) ;
      }

//  Initialise

    point = *pointP ;
    startPoint = dtmP->nullPnt ;
    priorPoint = nextPoint = nodeAddrP(dtmP,point)->tPtr ;
    while( nodeAddrP(dtmP,priorPoint)->tPtr != point)
      {
       if(( priorPoint = bcdtmList_nextClkDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
      }

//  Log Point Stats

    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"priorPoint = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",priorPoint,nodeAddrP(dtmP,priorPoint)->tPtr,pointAddrP(dtmP,priorPoint)->x,pointAddrP(dtmP,priorPoint)->y,pointAddrP(dtmP,priorPoint)->z) ;
       bcdtmWrite_message(0,0,0,"point      = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",point,nodeAddrP(dtmP,point)->tPtr,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
       bcdtmWrite_message(0,0,0,"nextPoint  = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",nextPoint,nodeAddrP(dtmP,nextPoint)->tPtr,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
      }

//  Log Tptr Polygon About Expand Point

    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Logging Anti Clockwise From Prior Point Tptr Polygon About Expansion Point") ;
       pnt = priorPoint ;
       bcdtmWrite_message(0,0,0,"pnt = %8ld ** pnt->tptr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
       do
         {
          if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
          bcdtmWrite_message(0,0,0,"pnt = %8ld ** pnt->tptr = %10ld pnt->z = %10.4lf",pnt,nodeAddrP(dtmP,pnt)->tPtr,pointAddrP(dtmP,pnt)->z) ;
         } while( pnt != nextPoint ) ;
      }

//  Determine Expand Type ( 1 = Simple , 2 = Complex )

    if( dbg ) bcdtmWrite_message(0,0,0,"Determing Point Expansion Type") ;
    expandType = 1 ;
    if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
    while( pnt != priorPoint && expandType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"** pnt = %8ld pnt->tPtr = %10ld ** %12.5lf %12.5lf %10.4lf",pnt,nodeAddrP(dtmP,pnt)->tPtr,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
       if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) 
           {
           expandType = 2 ;
           }
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"** pnt = %8ld pnt->tPtr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"expandType = %2ld",expandType) ;

//  Simple Expansion

    if( expandType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"***** Expanding Simple") ;

//     Set Tptr About Expansion Point

       pPoint =  priorPoint ;
       do
         {
          if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,point,pPoint)) < 0 ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"pPoint = %8ld antPoint = %8ld antPnt->tPtr = %9ld",pPoint,antPoint,nodeAddrP(dtmP,antPoint)->tPtr ) ;
          nodeAddrP(dtmP,pPoint)->tPtr = antPoint ;
          pPoint = antPoint ;
         } while ( pPoint != nextPoint && nodeAddrP(dtmP,antPoint)->tPtr != pPoint  ) ;

//     Removed Old Sections Of Tptr Polygon

       nodeAddrP(dtmP,point)->tPtr = dtmP->nullPnt ;
       startPoint = nextPoint ;
        
//     Set Added Section Start And End Points

       *extStartPntP = priorPoint ;
       *extEndPntP   = nextPoint ;
       
//     Log Expansion Completion       
       
       if( dbg ) bcdtmWrite_message(0,0,0,"***** Expanding Simple Completed") ;
       
//     Check Connectivity Of Tptr Polygon
        
       if( cdbg )
         {
          if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPoint,0))
            {
             bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
             goto errexit ;
            }
         }

      }

//  Complex Expansion

    if( expandType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"***** Expanding Complex") ;

//     Determine Closure Directions
  
       if( dbg ) bcdtmWrite_message(0,0,0,"Determining Closure Direction") ;
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"** pnt = %8ld pnt->tPtr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
             nodeAddrP(dtmP,point)->tPtr = pnt ;
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,point,&afterArea,&direction)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"pnt = %8ld ** closure = %2ld",pnt,direction) ;
             nodeAddrP(dtmP,pnt)->sPtr = (long)direction ;
             nodeAddrP(dtmP,point)->tPtr = nextPoint ;
            }  
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }

//     Set Next Point For Expansion

       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Next Point For Expansion") ;
       expansionNextPoint = nextPoint ; 
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          if( nodeAddrP(dtmP,pnt)->sPtr == 2 ) expansionNextPoint = pnt ;
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }

//     Set Prior Point For Expansion

       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Prior Point For Expansion") ;
       expansionPriorPoint = priorPoint ; 
       if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
       while( pnt != expansionNextPoint )
         {
          if( nodeAddrP(dtmP,pnt)->sPtr == 1 ) expansionPriorPoint = pnt ;
          if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }

//     Set Point To Clean Replaced Section Of Tptr Polygon

       cleanPnt = nodeAddrP(dtmP,expansionPriorPoint)->tPtr ;
       if( dbg ) bcdtmWrite_message(0,0,0,"expansionNext = %8ld expansionPrior = %8ld claenPnt = %8ld",expansionNextPoint,expansionPriorPoint,cleanPnt) ;

//     Set Tptr About Expansion Point

       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Tptr About Expansion Point") ;
       pPoint = expansionPriorPoint ;
       do
         {
          if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,point,pPoint)) < 0 ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"pPoint = %8ld antPoint = %8ld antPnt->tPtr = %9ld",pPoint,antPoint,nodeAddrP(dtmP,antPoint)->tPtr ) ;
          nodeAddrP(dtmP,pPoint)->tPtr = antPoint ;
          pPoint = antPoint ;
         } while ( pPoint != expansionNextPoint ) ;

//     Remove Old Sections Of Tptr Polygon

       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Old Sections Of Tptr Polygon") ;
       while( cleanPnt != expansionNextPoint )
         {
          nextPnt = nodeAddrP(dtmP,cleanPnt)->tPtr ;
          nodeAddrP(dtmP,cleanPnt)->tPtr = dtmP->nullPnt ;
          cleanPnt = nextPnt ;
         } 

//     Clean Sptr Settings
 
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }

//     Set New Index Point For Tptr Polygon
      
       startPoint = expansionNextPoint ;
       
//     Set Added Section Start And End Points

       *extStartPntP = expansionPriorPoint ;
       *extEndPntP   = expansionNextPoint ;
      }

//   Set Return Value

    *pointP = startPoint ;
    if( dbg ) bcdtmWrite_message(0,0,0,"*pointP = %8ld",*pointP) ;

//  Perform Area Checks

    if( cdbg)
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Performing Connectivity Check") ;
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*pointP,0))
         {
          bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
          goto errexit ;
         }
       if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*pointP) ;         
       if( dbg ) bcdtmWrite_message(0,0,0,"Performing Area Checks") ;
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pointP,&afterArea,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon Area      = %15.5lf",afterArea) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon Direction = %2ld",direction) ;
       if( afterArea < beforeArea || direction == DTMDirection::Clockwise)
         {
          bcdtmWrite_message(0,0,0,"Tptr Polygon Area Has Decreased") ;
          bcdtmWrite_message(0,0,0,"Expansion Point = %8ld ** beforeArea = %15.5lf afterArea = %15.5lf",point,beforeArea,afterArea) ;
          bcdtmList_writeTptrListDtmObject(dtmP,*pointP) ;
          goto errexit ;   
         } 
      }
   }   

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point Error") ;
 return(ret) ;

// Error Exit

 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
bool bcdtmDrainageList_checkForVoidsInDtmObject(BC_DTM_OBJ *dtmP)

    //   This Function Checks For Void In The DTM

    {
    long ofs,dtmFeature,partitionNum ;
    bool voidsInDtm=false ;
    BC_DTM_FEATURE *dtmFeatureP ;

    // Scan Dtm Features

    if( dtmP->fTablePP != NULL )
        {
        ofs = 0 ;
        partitionNum = 0 ;
        dtmFeatureP = dtmP->fTablePP[partitionNum] ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures &&  ! voidsInDtm ; ++dtmFeature )
             {
              if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
                   {
                    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   || 
                        dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole   || 
                        dtmFeatureP->dtmFeatureType == DTMFeatureType::Island     ) 
                        { 
                        voidsInDtm = true  ;
                        }
                   }
              ++ofs ;
              if( ofs == dtmP->featurePartitionSize ) 
                  {
                  ofs = 0 ;
                  ++partitionNum ;
                  dtmFeatureP = dtmP->fTablePP[partitionNum] ;
                  }
              else 
                  ++dtmFeatureP ; 
            }
        }

    // Return

    return(voidsInDtm) ;

    }

