/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmExport.cpp $
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
BENTLEYDTM_EXPORT int bcdtmExport_dtmObjectToDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **dtmPP,double ppTol,double plTol)
/*
** This Function Exports A DTM Object To A DTM Object. 
** Used To Retriangulate An Existing Triangulation With Different Tolerances
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    p1,p2,clc,dtmFeature,numFeatPts ;
 DPoint3d     tinLinePts[2],*featPtsP=NULL ;
 DTM_TIN_POINT   *pointP ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 DTMFeatureId   hullFeatureId ;
 DTMUserTag     hullUserTag ; 
 DTMFeatureId   nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Exporting Dtm Object To Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmPP                 = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"ppTol                 = %15.10lf",ppTol) ; 
    bcdtmWrite_message(0,0,0,"plTol                 = %15.10lf",plTol) ; 
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numLines        = %6ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"dtmP->numTriangles    = %6ld",dtmP->numTriangles) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures     = %6ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol           = %15.10lf",dtmP->ppTol) ;  
    bcdtmWrite_message(0,0,0,"dtmP->plTol           = %15.10lf",dtmP->plTol) ;
   }
/*
** Initialise
*/
 if( *dtmPP != NULL ) if( bcdtmObject_destroyDtmObject(dtmPP)) goto errexit ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; 
/*
** Check DTM Is In Triangulated State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&*dtmPP) ) goto errexit ; 
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(*dtmPP,dtmP->numLines*2,10000) ;
 (*dtmPP)->dtmFeatureIndex = dtmP->dtmFeatureIndex ;
/*
** Write Tin Lines To Dtm Object As Graphic Breaks
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Lines") ;
 for( p1 = 0 ; p1 < dtmP->numPoints; ++p1 )
   {
    clc = nodeAddrP(dtmP,p1)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( p2 > p1 )
         {
          pointP = pointAddrP(dtmP,p1) ;
          tinLinePts[0].x = pointP->x ; tinLinePts[0].y = pointP->y ; tinLinePts[0].z = pointP->z ; 
          pointP = pointAddrP(dtmP,p2) ;
          tinLinePts[1].x = pointP->x ; tinLinePts[1].y = pointP->y ; tinLinePts[1].z = pointP->z ; 
          if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,tinLinePts,2)) goto errexit ;  
         }
      } 
   }
/*
** Write Tin Features To Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Features") ;
 hullUserTag   = dtmP->nullUserTag ;
 hullFeatureId = dtmP->nullFeatureId ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) 
         {
          hullUserTag   = dtmFeatureP->dtmUserTag ; 
          hullFeatureId = dtmFeatureP->dtmFeatureId ;
         }
       else
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featPtsP,&numFeatPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,featPtsP,numFeatPts)) goto errexit ;  
          if( featPtsP != NULL )  { free(featPtsP) ; featPtsP = NULL ; }
         }
      }
   }
/*
** Write Tin Hull To DTM Object As Boundary Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Hull") ;
 if( bcdtmList_extractHullDtmObject(dtmP,&featPtsP,&numFeatPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::Hull,hullUserTag,1,&hullFeatureId,featPtsP,numFeatPts)) goto errexit ;  
 if( featPtsP != NULL )  { free(featPtsP) ; featPtsP = NULL ; }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Exported Dtm Object") ;
 (*dtmPP)->ppTol = ppTol ;
 (*dtmPP)->plTol = ppTol ;  // Robc - Need ppTOL and plTol To Be Same value
 if( bcdtmObject_createTinDtmObject(*dtmPP,1,0.0)) goto errexit ;
/*
** Write Some Stats For Exported DTM
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmPP->numPoints        = %6ld",(*dtmPP)->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmPP->numLines         = %6ld",(*dtmPP)->numLines) ;
    bcdtmWrite_message(0,0,0,"dtmPP->numTriangles     = %6ld",(*dtmPP)->numTriangles) ;
    bcdtmWrite_message(0,0,0,"dtmPP->numFeatures      = %6ld",(*dtmPP)->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmPP->ppTol            = %15.10lf",(*dtmPP)->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmPP->plTol            = %15.10lf",(*dtmPP)->plTol) ;
   }
/*
** Check Exported Dtm Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Exported DTM Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(*dtmPP))
      {
       bcdtmWrite_message(1,0,0,"Invalid Exported DTM Triangulation") ;
       goto errexit ;
      }
   bcdtmWrite_message(0,0,0,"Valid Exported DTM Triangulation") ;
  } 
/*
** Clean Up
*/
 cleanup :
 if( featPtsP != NULL )  { free(featPtsP) ; featPtsP = NULL ; }
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting Dtm Object To Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting Dtm Object To Dtm Object Error") ;
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ; 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
