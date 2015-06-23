/*--------------------------------------------------------------------------------------+
|
|     $Source: src/unmanaged/DTM/civilDTMext/bcdtmExtEdit.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "BcDTMEdit.h"

#define   DTM_FILE_SIZE               128

int DTM_MSG_LEVEL = 0;
/*
** Set Globals For Holding Edit Context
*/
static BC_DTM_OBJ  *editDtmP=NULL ;
static DTMFeatureType editDtmFeatureType=DTMFeatureType::None ;
static long editDtmFeatureNumber=DTM_NULL_PNT ;
static DPoint3d  *editFeaturePtsP=NULL ;
static long numEditFeaturePts=0 ;
static long editPnt1=DTM_NULL_PNT ;
static long editPnt2=DTM_NULL_PNT ;
static long editPnt3=DTM_NULL_PNT ;
static long editStatus=true ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_checkTinStructureDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Checks The Tin For Topology And Precision Errors
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Edit Tin") ;
/*
** Check For Pre 98 TIN
*/
 if( dtmP->ppTol == 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Convert Tin File") ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Null Out Temporary Pointers
*/
 bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,0) ;
 bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,0) ;
/*
** Check Tin Structure
*/
 if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Edit Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Edit Tin Error") ;
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
int bcdtmExtEdit_getDtmEditFeaturePoints(long *editPnt1P,long *editPnt2P,long *editPnt3P)
/*
** Needed For Geopak Tin Editor
*/
{
 *editPnt1P = editPnt1 ;
 *editPnt2P = editPnt2 ;
 *editPnt3P = editPnt3 ;
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_selectDtmEditFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,                  /* ==> Pointer To DTM Object               */
 DTMFeatureType dtmFeatureType,            /* ==> Type Of Dtm Feature To Snap To      */
 double  x,                         /* ==> Data Point x Coordinate Value       */
 double  y,                         /* ==> Data Point y Coordinate Value       */
 long    *featureFoundP,            /* <== Dtm Feature Found <true,false>      */
 DPoint3d     **featurePtsPP,            /* <== Feature Coordinates                 */
 long    *numFeaturePtsP            /* <== Number Of Feature Points            */
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Snapping to DTM Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x       = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y       = %12.5lf",y) ;
   }
/*
** Initialise
*/
 *featureFoundP = false ;
 if( *featurePtsPP != NULL ) { free(*featurePtsPP) ; *featurePtsPP = NULL ; }
 *numFeaturePtsP = 0 ;
/*
** Set Edit Context To NULL
*/
 editDtmP             = NULL ;
 editDtmFeatureType   = DTMFeatureType::None ;
 editDtmFeatureNumber = dtmP->nullPnt ;
 if( editFeaturePtsP != NULL ) { free(editFeaturePtsP) ; editFeaturePtsP = NULL ; }
 numEditFeaturePts    = 0 ;
 editPnt1             = dtmP->nullPnt ;
 editPnt2             = dtmP->nullPnt ;
 editPnt3             = dtmP->nullPnt ;
/*
** Test For Valid Dtm
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Switch On Dtm Feature Type
*/
 switch ( dtmFeatureType )
   {
    case DTMFeatureType::TinPoint :          // Snap To Tin Point
      if( bcdtmExtEdit_selectPointDtmObject(dtmP,x,y,featureFoundP,featurePtsPP,numFeaturePtsP,&editPnt1)) goto errexit ;
      editDtmP             = dtmP ;
      editDtmFeatureType   = DTMFeatureType::TinPoint ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Tin Point = %8ld",editPnt1) ;
    break ;

   case DTMFeatureType::TinLine :          // Snap To Tin Line
      if( bcdtmExtEdit_selectLineDtmObject(dtmP,x,y,featureFoundP,featurePtsPP,numFeaturePtsP,&editPnt1,&editPnt2)) goto errexit ;
      editDtmP             = dtmP ;
      editDtmFeatureType   = DTMFeatureType::TinLine ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Tin Line = %8ld-%8ld",editPnt1,editPnt2) ;
    break ;

    case DTMFeatureType::Triangle :          // Snap To Tin Triangle
      if( bcdtmExtEdit_selectTriangleDtmObject(dtmP,x,y,featureFoundP,featurePtsPP,numFeaturePtsP,&editPnt1,&editPnt2,&editPnt3)) goto errexit ;
      editDtmP             = dtmP ;
      editDtmFeatureType   = DTMFeatureType::Triangle ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Points = %8ld %8ld %8ld",editPnt1,editPnt2,editPnt3) ;
    break ;

    default :
      bcdtmWrite_message(1,0,0,"Snap Functionality To Dtm Feature Type Not yet Implemented") ;
      goto errexit ;
    break ;
   } ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to DTM Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to DTM Feature Error") ;
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
int bcdtmExtEdit_modifyDtmEditFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,                   /* ==> Pointer To DTM Object                     */
 long modifyOption,                  /* ==> Type Of Modification <1=Delete,,,,,>      */
 long *modifyResultP                 /* <== Modify Result <true,false>                */
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Modifying Dtm Edit Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"modifyOption = %8ld",modifyOption) ;
   }
/*
** Initialise
*/
 *modifyResultP = true ;
/*
** Check For Edit Dtm Feature
*/
 if( editDtmFeatureType == DTMFeatureType::None )
   {
    bcdtmWrite_message(2,0,0,"No Dtm Edit Feature") ;
    goto errexit ;
   }
/*
** Check Edit DTM and DTM are The Same
*/
 if( editDtmP != dtmP )
   {
    bcdtmWrite_message(2,0,0,"Dtm And Edit Dtm Are Not The Same") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm
*/
 if( bcdtmObject_testForValidDtmObject(editDtmP) ) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( editDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Edit DTM Is Not Triangulated") ;
    goto errexit ;
   }
/*
** Switch On Dtm Feature Type
*/
 switch ( editDtmFeatureType )
   {
    case DTMFeatureType::Triangle :          // Modify Triangle
      if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Triangle = %8ld %8ld %8ld",editPnt1,editPnt2,editPnt3) ;
      if( bcdtmExtEdit_deleteTriangleDtmObject(editDtmP,editPnt1,editPnt2,editPnt3)) goto errexit ;
    break ;

    default :
      bcdtmWrite_message(1,0,0,"Modify Functionality For Dtm Feature Type Not yet Implemented") ;
      goto errexit ;
    break ;
   } ;
/*
** Check Triangulation
*/
 if( cdbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted After Edit Transaction") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid After Edit Transaction") ;
   }
/*
** Clean Tin Object
*/
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted After Clean") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid After Clean") ;
   }
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Set Edit Context To NULL
*/
 editDtmP             = NULL ;
 editDtmFeatureType   = DTMFeatureType::None ;
 editDtmFeatureNumber = dtmP->nullPnt ;
 if( editFeaturePtsP != NULL ) { free(editFeaturePtsP) ; editFeaturePtsP = NULL ; }
 numEditFeaturePts    = 0 ;
 editPnt1             = dtmP->nullPnt ;
 editPnt2             = dtmP->nullPnt ;
 editPnt3             = dtmP->nullPnt ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Modifying Dtm Edit Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Modifying Dtm Edit Feature Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 editStatus = false ;
 *modifyResultP = false ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_selectPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x,
 double y,
 long   *pointFoundP,
 DPoint3d    **featurePtsPP,
 long   *numFeaturePtsP,
 long   *editPointP
)
/*
** This routine finds the closest Tin point to x,y
*/
{
 int ret=DTM_SUCCESS,dbg=0 ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Snapping To Point %10.4lf %10.4lf",x,y) ;
/*
** Initialise Variables
*/
 *pointFoundP = false ;
 *editPointP = dtmP->nullPnt ;
 *numFeaturePtsP = 0 ;
 if( *featurePtsPP != NULL ) { free(featurePtsPP) ; *featurePtsPP = NULL ; }
/*
** Find Edit Point
*/
 bcdtmFind_closestPointDtmObject(dtmP,x,y,editPointP) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Closest Point = %6ld ** %10.4lf %10.4lf %10.4lf",*editPointP,pointAddrP(dtmP,*editPointP)->x,pointAddrP(dtmP,*editPointP)->y,pointAddrP(dtmP,*editPointP)->z) ;
    bcdtmWrite_message(0,0,0,"Distance To Closet editPointP = %10.4lf",bcdtmMath_distance(x,y,pointAddrP(dtmP,*editPointP)->x,pointAddrP(dtmP,*editPointP)->y)) ;
    if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*editPointP)->PCWD)) bcdtmWrite_message(0,0,0,"Edit Point In Void") ;
   }
/*
** If Edit Point In Void Find Closest Edit Point On Void Hull
*/
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*editPointP)->PCWD) )
   {
    bcdtmExtEdit_findClosestNonVoidPointDtmObject(dtmP,x,y,editPointP) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"** Closent Edit Point            = %6ld ** %10.4lf %10.4lf %10.4lf",*editPointP,pointAddrP(dtmP,*editPointP)->x,pointAddrP(dtmP,*editPointP)->y,pointAddrP(dtmP,*editPointP)->z) ;
       bcdtmWrite_message(0,0,0,"** Distance To Closet Edit Point = %10.4lf",bcdtmMath_distance(x,y,pointAddrP(dtmP,*editPointP)->x,pointAddrP(dtmP,*editPointP)->y)) ;
       if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*editPointP)->PCWD)) bcdtmWrite_message(0,0,0,"** Edit Point In Void") ;
      }
   }
/*
** Allocate Memory For Point Coordinates
*/
 *featurePtsPP = ( DPoint3d *) malloc(sizeof(DPoint3d)) ;
 if( *featurePtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set Point Coordinates
*/
 (*featurePtsPP)->x = pointAddrP(dtmP,*editPointP)->x ;
 (*featurePtsPP)->y = pointAddrP(dtmP,*editPointP)->y ;
 (*featurePtsPP)->z = pointAddrP(dtmP,*editPointP)->z ;
 *pointFoundP = true ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *pointFoundP = false ;
 *editPointP = dtmP->nullPnt ;
 *numFeaturePtsP = 0 ;
 if( *featurePtsPP != NULL ) { free(featurePtsPP) ; *featurePtsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_findClosestNonVoidPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x,
 double y,
 long *editPointP
)
/*
** This routine finds the closest data point to p(x,y)
*/
{
 long    pt,ps ;
 double  dn,dd ;
/*
** Binary Scan x-Axis and find first x point value
** equal to x or greater than x
*/
 bcdtmFind_binaryScanDtmObject(dtmP,x,&pt) ;
 ps = *editPointP = pt ;
 if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*editPointP)->PCWD))
   {
    if( nodeAddrP(dtmP,pt)->cPtr != dtmP->nullPtr ) dn = bcdtmMath_distance(pointAddrP(dtmP,pt)->x,pointAddrP(dtmP,pt)->y,x,y) ;
    else                                            dn = sqrt(dtmP->xRange*dtmP->xRange + dtmP->yRange*dtmP->yRange) ;
    if( dn == 0 ) return(1) ;
   }
 else  dn = sqrt(dtmP->xRange*dtmP->xRange + dtmP->yRange*dtmP->yRange) ;
/*
** Scan Back Until x - x point value  > dn
*/
 for( pt = ps - 1 ; pt >= 0 && x-pointAddrP(dtmP,pt)->x < dn ; --pt )
   {
    if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pt)->PCWD) && fabs(y-pointAddrP(dtmP,pt)->y ) <= dn && nodeAddrP(dtmP,pt)->cPtr != dtmP->nullPtr )
      {
       dd = bcdtmMath_distance(pointAddrP(dtmP,pt)->x,pointAddrP(dtmP,pt)->y,x,y) ;
       if( dd < dn ) { dn = dd ; *editPointP = pt ; }
      }
   }
/*
** Scan Forwards Until x point value - x > dn
*/
 for( pt = ps + 1 ; pt < dtmP->numSortedPoints && pointAddrP(dtmP,pt)->x - x < dn ; ++pt )
   {
    if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pt)->PCWD) && fabs(y-pointAddrP(dtmP,pt)->y) <= dn && nodeAddrP(dtmP,pt)->cPtr != dtmP->nullPtr )
      {
       dd = bcdtmMath_distance(pointAddrP(dtmP,pt)->x,pointAddrP(dtmP,pt)->y,x,y) ;
       if( dd < dn ) { dn = dd ; *editPointP = pt ; }
      }
   }
/*
**  Scan Inserted Points
*/
 if( dtmP->numSortedPoints != dtmP->numPoints )
   {
    for( pt = dtmP->numSortedPoints ; pt < dtmP->numPoints ; ++pt )
      {
       if(! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pt)->PCWD) && fabs(x-pointAddrP(dtmP,pt)->x) <= dn && fabs(y-pointAddrP(dtmP,pt)->y) <= dn && nodeAddrP(dtmP,pt)->cPtr != dtmP->nullPtr )
         {
          dd = bcdtmMath_distance(pointAddrP(dtmP,pt)->x,pointAddrP(dtmP,pt)->y,x,y) ;
          if( dd < dn ) { dn = dd ; *editPointP = pt ; }
         }
      }
   }
/*
** Job Completed
*/
 if( dn == 0.0  ) return(1) ;
 else             return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_selectLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 double  x,
 double  y,
 long    *lineFoundP,               /* <== Line Found <true,false>                          */
 DPoint3d     **linePtsPP,               /* <== Coordinates For Line Vertices                    */
 long    *numLinePtsP,              /* <== Number Of Line Vertices                          */
 long    *pnt1P,                    /* <== Dtm Internal Line Vertice Number                 */
 long    *pnt2P                     /* <== Dtm Internal Line Vertice Number                 */
)
/*
** This routine finds the closest Tin Line to x,y
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   p1,p2,p3,fndType,voidLine,dtmFeature ;
 double Zs,n1,n2,n3    ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Snapping To Line %10.4lf %10.4lf",x,y) ;
/*
** Initialise Variables
*/
 *lineFoundP = false ;
 if( *linePtsPP != NULL ) { free(*linePtsPP) ; *linePtsPP = NULL ; }
 *numLinePtsP = 0 ;
 *pnt1P = *pnt2P = dtmP->nullPnt ;
/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&Zs,&fndType,&p1,&p2,&p3) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"00 ** fndType = %2ld",fndType) ;
/*
** If Point External Find Closest Hull Line
*/
 if( fndType == 0 )
   {
    if( bcdtmExtEdit_findClosestHullLineDtmObject(dtmP,x,y,&p1,&p2)) goto errexit ;
    fndType = 3 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"01 ** fndType = %2ld",fndType) ;
/*
** Test For Void Line
*/
 voidLine = 0 ;
 if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ) voidLine = 1 ;
 if( fndType == 2 || fndType == 3 ) if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit ;
 if( fndType == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidLine)) goto errexit ;
/*
** If Line In Void Find Enclosing Void
*/
 if( voidLine )
   {
    if( bcdtmExtEdit_findDtmFeatureTypeEnclosingPointDtmObject(dtmP,p1,DTMFeatureType::Void,&dtmFeature)) goto errexit ;
    if( bcdtmExtEdit_findClosestVoidLineDtmObject(dtmP,x,y,&p1,&p2,&dtmFeature)) goto errexit ;
    fndType = 2 ;
   }
/*
** Get Closest Line
*/
 switch(fndType)
   {
    case 1 :               /* Point Coincident With Point   */
      *pnt1P = p1 ; *pnt2P = clistAddrP(dtmP,nodeAddrP(dtmP,p1)->cPtr)->pntNum ;
    break ;

    case 2 :               /* Colinear with Line */
        case 3 :
          *pnt1P = p1 ; *pnt2P = p2 ;
    break ;

    case 4 :               /* Point In Triangle  */
      n1 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
      n2 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,x,y) ;
      n3 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y) ;
      if     ( n1 <= n2 && n1 <= n3 ) { *pnt1P = p1 ; *pnt2P = p2 ; }
          else if( n2 <= n1 && n2 <= n3 ) { *pnt1P = p2 ; *pnt2P = p3 ; }
          else                            { *pnt1P = p3 ; *pnt2P = p1 ; }
    break ;

    default :
    break   ;
   } ;
/*
** Allocate Memory For Line Coordinates
*/
 *linePtsPP = ( DPoint3d *) malloc(2*sizeof(DPoint3d)) ;
 if( *linePtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set Point Coordinates
*/
 (*linePtsPP)->x   = pointAddrP(dtmP,*pnt1P)->x ;
 (*linePtsPP)->y   = pointAddrP(dtmP,*pnt1P)->y ;
 (*linePtsPP)->z   = pointAddrP(dtmP,*pnt1P)->z ;
 (*linePtsPP+1)->x = pointAddrP(dtmP,*pnt2P)->x ;
 (*linePtsPP+1)->y = pointAddrP(dtmP,*pnt2P)->y ;
 (*linePtsPP+1)->z = pointAddrP(dtmP,*pnt2P)->z ;
 *lineFoundP = true ;
 *numLinePtsP = 2 ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *lineFoundP = false ;
 if( *linePtsPP != NULL ) { free(*linePtsPP) ; *linePtsPP = NULL ; }
 *numLinePtsP = 0 ;
 *pnt1P = *pnt2P = dtmP->nullPnt ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_selectTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,                  /* ==> Pointer To DTM Object                                */
 double  x,                         /* ==> x Coordinate Value                                   */
 double  y,                         /* ==> y Coordinate Value                                   */
 long    *trgFoundP,                /* <== Triangle Found <true,false>                          */
 DPoint3d     **trgPtsPP,                /* <== Coordinates For Triangle Vertices                    */
 long    *numTrgPtsP,               /* <== Number Of Triangle Vertices                          */
 long    *pnt1P,                    /* <== Dtm Internal Triangle Vertice Number                 */
 long    *pnt2P,                    /* <== Dtm Internal Triangle Vertice Number                 */
 long    *pnt3P                     /* <== Dtm Internal Triangle Vertice Number                 */
)
/*
** This Function Finds The closest Triangle to x,y
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   pnt1,pnt2,pnt3,fndType,voidFlag,voidFeature ;
 double Zs    ;
 DTM_TIN_POINT *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Snapping To Triangle") ;
/*
** Initialise Variables
*/
 *trgFoundP = false ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
 if( *trgPtsPP != NULL ) { free(*trgPtsPP) ; *trgPtsPP = NULL ; }
 *numTrgPtsP = 0 ;
 /*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&Zs,&fndType,&pnt1,&pnt2,&pnt3) ) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"00 fndType = %4ld",fndType) ;

/*
** If Point External Find Closest Hull Line
*/
 if( fndType == 0 )
   {
    if( bcdtmExtEdit_findClosestHullLineDtmObject(dtmP,x,y,&pnt1,&pnt2)) goto errexit  ;
    if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
    pointP = pointAddrP(dtmP,pnt1) ;
    x = ( pointAddrP(dtmP,pnt1)->x + pointAddrP(dtmP,pnt2)->x + pointAddrP(dtmP,pnt3)->x ) / 3.0 ;
    y = ( pointAddrP(dtmP,pnt1)->y + pointAddrP(dtmP,pnt2)->y + pointAddrP(dtmP,pnt3)->y ) / 3.0 ;
    fndType = 4 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"01 fndType = %4ld",fndType) ;
/*
** Test For Point In Void
*/
 voidFlag = 0 ;
 if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) voidFlag = 1 ;
 if( fndType == 2 || fndType == 3 ) if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,&voidFlag)) goto errexit  ;
 if( fndType == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,&voidFlag)) goto errexit  ;
/*
** If Point In Void Find Enclosing Void
*/
 if( voidFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point In Void Finding Surrounding Void") ;
    fndType = 0 ;
    if( bcdtmExtEdit_findClosestVoidLineDtmObject(dtmP,x,y,&pnt1,&pnt2,&voidFeature)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"VoidFeature = %4ld ** Closest Void Line = %12.5lf %12.5lf ** %12.5lf %12.5lf",voidFeature,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    if( voidFeature == dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"Can Not Find Closest Void Feature") ;
       goto errexit ;
      }
    if( ftableAddrP(dtmP,voidFeature)->dtmFeatureType == DTMFeatureType::Void )
      {
       if( nodeAddrP(dtmP,pnt1)->hPtr == pnt2 ) fndType = 0 ;
       else
         {
          if( ( pnt3 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
          if( ! bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2)) goto errexit ;
          fndType = 4 ;
         }
     }
    else if( ftableAddrP(dtmP,voidFeature)->dtmFeatureType == DTMFeatureType::Island )
      {
       if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
       if( ! bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2)) goto errexit  ;
       fndType = 4 ;
      }
   }
/*
** Set Triangle Vertices
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"02 fndType = %4ld",fndType) ;
 switch( fndType )
   {
    case 1 :               /* Point Coincident Triangle Vertex */
      *pnt1P = pnt1 ;
      *pnt2P = clistAddrP(dtmP,nodeAddrP(dtmP,pnt1)->cPtr)->pntNum ;
      if(( pnt3 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
      if( ! bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2) )
        {
         if(( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
        }
      *pnt3P = pnt3 ;
      *trgFoundP = true ;
     break ;

    case 2 :               /* Point Colinear with Triangle Edge */
        case 3 :
          *pnt1P = pnt1 ;
      *pnt2P = pnt2 ;
      if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
      if( ! bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2))
        {
         if( ( pnt3 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit  ;
        }
      *pnt3P = pnt3 ;
      *trgFoundP = true ;
    break ;

    case 4 :               /* Point In Triangle                   */
      *pnt1P = pnt1 ;
      *pnt2P = pnt2 ;
      *pnt3P = pnt3 ;
      *trgFoundP = true ;
    break ;

    default :
    break   ;
   } ;
/*
** Set Return Values
*/
 if( *trgFoundP != 0 )
   {
/*
**  Allocate Memory For Storing Triangle Vertices
*/
    *numTrgPtsP = 3 ;
    *trgPtsPP = ( DPoint3d * ) malloc( *numTrgPtsP * sizeof(DPoint3d)) ;
    if( *trgPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Store Triangle Vertices
*/
    pointP = pointAddrP(dtmP,*pnt1P) ;
    (*trgPtsPP)->x = pointP->x ;
    (*trgPtsPP)->y = pointP->y ;
    (*trgPtsPP)->z = pointP->z ;
    pointP = pointAddrP(dtmP,*pnt2P) ;
    (*trgPtsPP+1)->x = pointP->x ;
    (*trgPtsPP+1)->y = pointP->y ;
    (*trgPtsPP+1)->z = pointP->z ;
    pointP = pointAddrP(dtmP,*pnt3P) ;
    (*trgPtsPP+2)->x = pointP->x ;
    (*trgPtsPP+2)->y = pointP->y ;
    (*trgPtsPP+2)->z = pointP->z ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to Triangle Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numTrgPtsP = 0 ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
 if( *trgPtsPP != NULL ) { free(*trgPtsPP) ; *trgPtsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *hullPnt1P,long *hullPnt2P)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 long   p1,p2,isw,lf ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn  ;
/*
** Initialiase
*/
 *hullPnt1P = dtmP->nullPnt ;
 *hullPnt2P = dtmP->nullPnt ;
/*
** Find Closest Hull Line
*/
 isw = 1 ;
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
        if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) < 0 )
          {
           d1 = bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y) ;
           d2 = bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
           d3 = bcdtmMath_distance((pointAddrP(dtmP,p1)->x+pointAddrP(dtmP,p2)->x) / 2.0,(pointAddrP(dtmP,p1)->y+pointAddrP(dtmP,p2)->y)/2.0,x,y) ;
       d4 = bcdtmMath_distanceOfPointFromLine(&lf,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,&Xn,&Yn) ;
           if( isw )
             {
                  *hullPnt1P = p1 ; *hullPnt2P = p2 ;
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
                     *hullPnt1P = p1 ;
             *hullPnt2P = p2 ;
                     if( d1 < dn ) dn = d1 ;
                     if( d2 < dn ) dn = d2 ;
                     if( d3 < dn ) dn = d3 ;
             if( lf && d4 < dn ) dn = d4 ;
                        }
                 }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
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
int bcdtmExtEdit_findClosestHullLineForPointAddDtmObject
(
 BC_DTM_OBJ *dtmP,
 double     x,
 double     y,
 long       *hullPnt1P,
 long       *hullPnt2P
)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 int    dbg=0 ;
 long   p1,p2,isw,lf ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn  ;
 bool   p1Intersect,p2Intersect ;
/*
** Log Method Arguements
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Hull Line For Add Point") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x       = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y       = %12.5lf",y) ;
   }
/*
** Initialiase
*/
 *hullPnt1P = dtmP->nullPnt ;
 *hullPnt2P = dtmP->nullPnt ;
/*
** Find Closest Hull Line
*/
 isw = 1 ;
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
        if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) < 0 )
          {
//bcdtmWrite_message(0,0,0,"p1 = %8ld p2 = %8ld",p1,p2) ;
           d1 = bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y) ;
           d2 = bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
           d3 = bcdtmMath_distance((pointAddrP(dtmP,p1)->x+pointAddrP(dtmP,p2)->x) / 2.0,(pointAddrP(dtmP,p1)->y+pointAddrP(dtmP,p2)->y)/2.0,x,y) ;
       d4 = bcdtmMath_distanceOfPointFromLine(&lf,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,&Xn,&Yn) ;
       bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject(dtmP,p1,x,y,p1Intersect) ;
       bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject(dtmP,p2,x,y,p2Intersect) ;
//bcdtmWrite_message(0,0,0,"p1Intersect = %2d p2Intersect = %2d",p1Intersect,p2Intersect) ;
       if( p1Intersect == false && p2Intersect == false )
         {
          if( isw )
                {
                     *hullPnt1P = p1 ; *hullPnt2P = p2 ;
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
                        *hullPnt1P = p1 ;
                *hullPnt2P = p2 ;
                        if( d1 < dn ) dn = d1 ;
                        if( d2 < dn ) dn = d2 ;
                        if( d3 < dn ) dn = d3 ;
                if( lf && d4 < dn ) dn = d4 ;
                           }
                    }
                 }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
/*
** Log Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Closest Hull Line = %10ld %10ld",*hullPnt1P,*hullPnt1P) ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Line For Add Point Completed") ;
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       point,
 double     x,
 double     y,
 bool&      intersects
)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 int    pnt,nextPnt,sideOf1,sideOf2 ;
 double xp,yp ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;
 double xMin,yMin,xMax,yMax,xlMin,ylMin,xlMax,ylMax ;
/*
** Initialiase
*/
 intersects = false ;
 xp = pointAddrP(dtmP,point)->x ;
 yp = pointAddrP(dtmP,point)->y ;
 xMin = xMax = x ;
 yMin = yMax = y ;
 if( xp < xMin ) xMin = xp ;
 if( xp > xMax ) xMax = xp ;
 if( yp < yMin ) yMin = yp ;
 if( yp > yMax ) yMax = yp ;
/*
** Find Intersection With Hull Line
*/
 pnt = dtmP->hullPoint ;
 do
   {
    nextPnt = nodeAddrP(dtmP,pnt)->hPtr ;
    if( pnt != point && nextPnt != point )
      {
       pnt1P = pointAddrP(dtmP,pnt) ;
       pnt2P = pointAddrP(dtmP,nextPnt) ;
       xlMin = xlMax = pnt1P->x ;
       ylMin = ylMax = pnt1P->y ;
       if( pnt2P->x < xlMin ) xlMin = pnt2P->x ;
       if( pnt2P->x > xlMax ) xlMax = pnt2P->x ;
       if( pnt2P->y < ylMin ) ylMin = pnt2P->y ;
       if( pnt2P->y > ylMax ) ylMax = pnt2P->y ;
       if( xlMin <= xMax && xlMax >= xMin && ylMin <= yMax && ylMax >= yMin )
         {
          sideOf1 = bcdtmMath_sideOf(x,y,xp,yp,pnt1P->x,pnt1P->y) ;
          sideOf2 = bcdtmMath_sideOf(x,y,xp,yp,pnt2P->x,pnt2P->y) ;
          if( sideOf1 != sideOf2 )
            {
             sideOf1 = bcdtmMath_sideOf(pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,x,y) ;
             sideOf2 = bcdtmMath_sideOf(pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,xp,yp) ;
             if( sideOf1 != sideOf2 ) intersects = true ;
            }
         }
      }
    pnt = nextPnt ;
   } while ( pnt != dtmP->hullPoint && intersects == false ) ;
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
int bcdtmExtEdit_findClosestVoidLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x,
 double y,
 long *voidPnt1P,
 long *voidPnt2P,
 long *dtmFeatureP
)
/*
** This Routine Find the Closeset Void Line to x,y
** Assumes XY has been prior tested to be in a Void
*/
{
 int  ret=DTM_SUCCESS ;
 long n,p1,p2,p3,spnt,isw,lf,fndType ;
 long numIslandFeatures,voidFeature,islandFeature ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn,Zs  ;
 DTM_TIN_POINT_FEATURES *islandFeaturesP=NULL ;
/*
** Initialiase
*/
 *voidPnt1P   = dtmP->nullPnt ;
 *voidPnt2P   = dtmP->nullPnt ;
 *dtmFeatureP = dtmP->nullPnt ;
/*
** Find Triangle Enclosing Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&Zs,&fndType,&p1,&p2,&p3 ) ) goto errexit ;
/*
** Find Enclosing Void Feature
*/
 if( bcdtmExtEdit_findDtmFeatureTypeEnclosingPointDtmObject(dtmP,p1,DTMFeatureType::Void,&voidFeature)) goto errexit ;
/*
** Get List Of Islands Enclosed By Void
*/
 if( bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(dtmP,voidFeature,&islandFeaturesP,&numIslandFeatures) ) goto errexit ;
/*
** Scan Void Feature To Get Closest Line
*/
 isw = 1 ;
 p1 = spnt = ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,p1,&p2) ;
    if( nodeAddrP(dtmP,p1)->hPtr != p2 )
      {
           if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) > 0 )
             {
              d1 = bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y) ;
              d2 = bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
              d3 = bcdtmMath_distance((pointAddrP(dtmP,p1)->x+pointAddrP(dtmP,p2)->x) / 2.0,(pointAddrP(dtmP,p1)->y+pointAddrP(dtmP,p2)->y)/2.0,x,y) ;
          d4 = bcdtmMath_distanceOfPointFromLine(&lf,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,&Xn,&Yn) ;
              if( isw )
                {
                     *voidPnt1P = p1 ;
             *voidPnt2P = p2 ;
             *dtmFeatureP = voidFeature ;
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
                        *voidPnt1P = p1 ;
                *voidPnt2P = p2 ;
                *dtmFeatureP = voidFeature ;
                        if( d1 < dn ) dn = d1 ;
                        if( d2 < dn ) dn = d2 ;
                        if( d3 < dn ) dn = d3 ;
                if( lf && d4 < dn ) dn = d4 ;
               }
                        }
                 }
      }
    p1 = p2 ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,p1,&p2) ;
   } while ( p1 != spnt ) ;
/*
** Scan Island Features To Get Closest Line
*/
 for( n = 0 ; n < numIslandFeatures ; ++ n )
   {
    islandFeature = (islandFeaturesP+n)->dtmFeature ;
    spnt = p1 = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    do
      {
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,p1,&p2) ;
           if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) < 0 )
             {
              d1 = bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y) ;
              d2 = bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
              d3 = bcdtmMath_distance((pointAddrP(dtmP,p1)->x+pointAddrP(dtmP,p2)->x) / 2.0,(pointAddrP(dtmP,p1)->y+pointAddrP(dtmP,p2)->y)/2.0,x,y) ;
          d4 = bcdtmMath_distanceOfPointFromLine(&lf,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,&Xn,&Yn) ;
                  if( d1 < dn || d2 < dn || d3 < dn || ( lf && d4 < dn ) )
                    {
                     *voidPnt1P = p1 ; *voidPnt2P = p2 ;
             *dtmFeatureP = islandFeature ;
                     if( d1 < dn ) dn = d1 ;
                     if( d2 < dn ) dn = d2 ;
                     if( d3 < dn ) dn = d3 ;
             if( lf && d4 < dn ) dn = d4 ;
                    }
         }
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,p1,&p2) ;
       p1 = p2 ;
      } while ( p1 != spnt ) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandFeaturesP != NULL ) free(islandFeaturesP) ;
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
int bcdtmExtEdit_findDtmFeatureTypeEnclosingPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long point,
 DTMFeatureType dtmFeatureType,
 long *dtmFeatureP
)
/*
** This Routine Finds The Dtm Feature Enclosing A Point
*/
{
 int  ret=DTM_SUCCESS ;
 long clc,pn,pmn ;
/*
** Initialise
*/
 *dtmFeatureP = dtmP->nullPnt ;
 if( dtmP->numFeatures == 0 )
   {
    bcdtmWrite_message(1,0,0,"No Features") ;
    goto errexit ;
   }
 pmn = point ;
/*
** Scan Points Looking For Feature Boundary
*/
 while( pmn != dtmP->nullPnt )
  {
/*
** Scan Feature List For Point
*/
   if( ( clc = nodeAddrP(dtmP,pmn)->fPtr) !=  dtmP->nullPtr )
     {
      while( clc != dtmP->nullPtr )
        {
         if( dtmFeatureType == DTMFeatureType::Void && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island )
           {
            bcdtmExtEdit_getVoidExternalToIslandDtmObject(dtmP,flistAddrP(dtmP,clc)->dtmFeature,dtmFeatureP) ;
            return(0) ;
           }
         if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == dtmFeatureType && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
           {
            *dtmFeatureP = flistAddrP(dtmP,clc)->dtmFeature ;
            return(0) ;
           }
         clc = flistAddrP(dtmP,clc)->nextPtr ;
        }
     }
/*
** Get Next Point
*/
   pn  = pmn ;
   pmn = dtmP->nullPnt ;
   clc = nodeAddrP(dtmP,pn)->cPtr ;
   while ( clc != dtmP->nullPtr )
     {
      if( pointAddrP(dtmP,clistAddrP(dtmP,clc)->pntNum)->x < pointAddrP(dtmP,pn)->x )
        {
         if      ( pmn == dtmP->nullPnt ) pmn = clistAddrP(dtmP,clc)->pntNum ;
         else  if( pointAddrP(dtmP,clistAddrP(dtmP,clc)->pntNum)->x < pointAddrP(dtmP,pmn)->x ) pmn = clistAddrP(dtmP,clc)->pntNum ;
        }
      clc = clistAddrP(dtmP,clc)->nextPtr ;
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
int bcdtmExtEdit_testForPointOnDtmFeatureTypeDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 long point,
 long *dtmFeatureP,
 long *numFeaturesP
)
/*
** This Function Tests If Point Is On A DTM Feature Type
*/
{
 long clc ;
/*
** Initialiase
*/
 *numFeaturesP = 0 ;
 *dtmFeatureP  = dtmP->nullPnt ;
 if( point < 0 || point >= dtmP->numPoints ) return(0) ;
 if(nodeAddrP(dtmP,point)->cPtr == dtmP->nullPtr ) return(0) ;
/*
** Scan point
*/
 if( ( clc = nodeAddrP(dtmP,point)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == dtmFeatureType )
         { if( *dtmFeatureP == dtmP->nullPnt ) *dtmFeatureP = flistAddrP(dtmP,clc)->dtmFeature ; ++(*numFeaturesP) ; }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
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
int bcdtmExtEdit_getVoidExternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long *voidFeatureP)
/*
** This Function Gets The Void Immediately External To An Island
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long sp,np,pp,lp,pl,ph,fpnt,clc,cll ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island") ;
/*
** Initialise
*/
 *voidFeatureP = dtmP->nullPnt  ;
 pl = dtmP->numPoints ;
 ph = 0 ;
/*
** Check Feature Is A Island Feature
*/
 if( islandFeature < 0 || islandFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Island Feature Range Error") ;
    goto errexit ;
   }
/*
** Only Process If Island Feature
*/
 if( ftableAddrP(dtmP,islandFeature)->dtmFeatureType  == DTMFeatureType::Island  )
   {
/*
**  Null Out Tptr Values
*/
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
**  Scan Around Island And Look For Void
*/
    pp = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,pp,&sp)) goto errexit ;
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np)) goto errexit ;
    fpnt = np ;
    do
      {
       if( ( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit  ;
       while ( lp != pp && *voidFeatureP == dtmP->nullPnt )
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD))
               {
                if( lp < pl ) pl = lp ;
                if( lp > ph ) ph = lp ;
                nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
               }
             else
               {
                clc = nodeAddrP(dtmP,lp)->fPtr ;
                while ( clc != dtmP->nullPtr && *voidFeatureP == dtmP->nullPnt )
                  {
                   if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) *voidFeatureP = flistAddrP(dtmP,clc)->dtmFeature ;
                   clc = flistAddrP(dtmP,clc)->nextPtr ;
                  }
               }
            }
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit  ;
         }
       pp = sp ; sp = np ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np)) goto errexit ;
      } while ( np != fpnt && *voidFeatureP == dtmP->nullPnt ) ;

/*
**  Sequentially Scan Points
*/
    while( *voidFeatureP == dtmP->nullPnt )
      {
       for( sp = pl ; sp <= ph && *voidFeatureP == dtmP->nullPnt ; ++sp )
         {
          if( nodeAddrP(dtmP,sp)->tPtr == -dtmP->nullPnt )
            {
             clc = nodeAddrP(dtmP,sp)->cPtr ;
             while ( clc != dtmP->nullPtr && *voidFeatureP == dtmP->nullPnt )
               {
                lp  = clistAddrP(dtmP,clc)->pntNum ;
                clc = clistAddrP(dtmP,clc)->nextPtr ;
                if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
                  {
                   if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD)) nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
                  }
                else
                  {
                   cll = nodeAddrP(dtmP,lp)->fPtr ;
                   while ( cll != dtmP->nullPtr && *voidFeatureP == dtmP->nullPnt )
                     {
                      if( ftableAddrP(dtmP,flistAddrP(dtmP,cll)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) *voidFeatureP = flistAddrP(dtmP,cll)->dtmFeature ;
                      cll = flistAddrP(dtmP,cll)->nextPtr ;
                     }
                  }
               }
            }
          nodeAddrP(dtmP,sp)->tPtr = -dtmP->nullPnt + 10 ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island Error") ;
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
|    bcdtmExtEdit_cleanVoidTinObject()                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_cleanVoidDtmObject(BC_DTM_OBJ *dtmP,long VoidFeature)
/*
** This Function Removes Void Points From A Tin Object
*/
{
 long  *Islands=NULL,NumIslands=0 ;
/*
** Check Feature Is A Void Feature
*/
 if( VoidFeature < 0 || VoidFeature >= dtmP->numFeatures ) return(0) ;
 if( ftableAddrP(dtmP,VoidFeature)->dtmFeatureType  != DTMFeatureType::Void) return(0) ;
/*
** Get Island Features Internal To Void
*/
 if( bcdtmExtEdit_getIslandsInternalToVoidDtmObject(dtmP,VoidFeature,&Islands,&NumIslands))
   { if( Islands != NULL ) free(Islands) ; return(1) ; }
/*
** Remove Internal Void Points And Lines
*/
 if( bcdtmExtEdit_removeInternalVoidPointsAndLinesDtmObject(dtmP,VoidFeature,Islands,NumIslands))
   { if( Islands != NULL ) free(Islands) ; return(1) ; }
/*
** Retriangulate Void
*/
 if( bcdtmExtEdit_triangulateVoidDtmObject(dtmP,VoidFeature,Islands,NumIslands,dtmP->nullPnt) )
   { if( Islands != NULL ) free(Islands) ; return(1) ; }
/*
** Free Memory
*/
 if( Islands != NULL ) free(Islands) ;
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
int bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,DTM_TIN_POINT_FEATURES **islandsPP,long *numIslandsP)
/*
** This Function Gets The List Of All Island Feature Internal To A Void
**
** Rob Cormack - June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   memIslands=0,memIslandsInc=10,islandFeature ;
 long   ofs,lp,clc,pnt,ppnt,npnt,lpnt,fPnt,lPnt,sPnt,numLineFeatures ;
 DTM_TIN_POINT_FEATURES *lineFeaturesP=NULL,*featP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void") ;
/*
** Initialise
*/
 *numIslandsP = 0 ;
 if( *islandsPP != NULL ) { free(*islandsPP) ; *islandsPP = NULL ; }
/*
** Copy Void Feature To Tptr List
*/
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&sPnt)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 fPnt = lPnt = dtmP->nullPnt ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
      }
/*
** Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
               }
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
            }
         }
      }
/*
**  Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;
    pnt  = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Scan Internal Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
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
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp) )
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
** Scan Marked Points For Connection To An Island Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Marked Points For Connection To Island Feature") ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
/*
**     Scan Marked Point For Island Features
*/
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
         }
/*
**     Get Next Marked Point
*/
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Scan Void Hull For Direct Connection To Island
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Void Hull For Direct Connection To Islands") ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
/*
**  Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
       if( lp != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Island,pnt,lp,&lineFeaturesP,&numLineFeatures)) goto errexit ;
          for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
                for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
                  {
                   if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
                  }
               }
            if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
           }
         if( lp != npnt )
           {
            if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Island,lp,pnt,&lineFeaturesP,&numLineFeatures)) goto errexit ;
            for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
              {
               if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
              }
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
** Null Tptr List
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,sPnt)) goto errexit ;
/*
** Scan Detected Islands For Connection To Other Islands
*/
 for( ofs = 0 ; ofs < *numIslandsP ; ++ofs )
   {
    islandFeature = (*islandsPP+ofs)->dtmFeature ;
    sPnt = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,sPnt,&ppnt)) goto errexit ;
    do
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sPnt,&npnt)) goto errexit ;
       if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,npnt)) < 0 ) goto errexit ;
       while ( pnt != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,pnt,&lineFeaturesP,&numLineFeatures)) goto errexit ;
          for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(islandsPP,numIslandsP,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
          if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pnt)) < 0 ) goto errexit ;
         }
       ppnt = sPnt ;
       sPnt = npnt ;
      } while ( sPnt != ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( lineFeaturesP != NULL ) { free(lineFeaturesP) ; lineFeaturesP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(DTM_TIN_POINT_FEATURES **dtmFeatureListPP,long *numFeatureListP,long *memFeatureListP,long memFeatureListInc,long dtmFeature,DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTMFeatureId userFeatureId,long priorPoint,long nextPoint)
/*
** This Stores A Feature In A Dtm Feature List
**
** Rob Cormack - June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   storeFlag ;
 DTM_TIN_POINT_FEATURES *featP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List") ;
/*
** Scan Point Feature List For Already Included Feature
*/
 storeFlag = 1 ;
 for( featP = *dtmFeatureListPP ; featP < *dtmFeatureListPP + *numFeatureListP && storeFlag ; ++featP )
   {
    if( featP->dtmFeature == dtmFeature ) storeFlag = 0 ;
   }
/*
** Add Feature To Feature List
*/
 if( storeFlag )
   {
/*
** Allocate memory If Necessary
*/
    if( *numFeatureListP == *memFeatureListP )
      {
       *memFeatureListP = *memFeatureListP + memFeatureListInc ;
       if( *dtmFeatureListPP == NULL ) *dtmFeatureListPP = ( DTM_TIN_POINT_FEATURES * ) malloc ( *memFeatureListP * sizeof(DTM_TIN_POINT_FEATURES)) ;
       else                            *dtmFeatureListPP = ( DTM_TIN_POINT_FEATURES * ) realloc ( *dtmFeatureListPP , *memFeatureListP * sizeof(DTM_TIN_POINT_FEATURES)) ;
       if( *dtmFeatureListPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store Feature
*/
    (*dtmFeatureListPP + *numFeatureListP)->dtmFeature     =  dtmFeature ;
    (*dtmFeatureListPP + *numFeatureListP)->dtmFeatureType =  (DTMFeatureType)dtmFeatureType ;
    (*dtmFeatureListPP + *numFeatureListP)->userTag        =  userTag ;
    (*dtmFeatureListPP + *numFeatureListP)->userFeatureId  =  userFeatureId ;
    (*dtmFeatureListPP + *numFeatureListP)->nextPoint      =  nextPoint ;
    (*dtmFeatureListPP + *numFeatureListP)->priorPoint     =  priorPoint ;
    ++*numFeatureListP ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List Error") ;
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
int bcdtmExtEdit_deleteExternalTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,               // ==> Pointer To DTM Object
 long       trgPnt1,             // ==> Triangle Vertex 1
 long       trgPnt2,             // ==> Triangle Vertex 2
 long       trgPnt3              // ==> Triangle Vertex 3
)

// RobC - Added 10 October 2012
// This Is A simplier Implementation For The bcLIB TM Editor
// Delete External Triangle Than The Generic Delete Triangle Function
//

{
 int   ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 int   trgPnt,edgeCount,edgeP1,edgeP2,edgeP3 ;

//  Log Function Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting External Triangle" ) ;
    bcdtmWrite_message(0,0,0,"TrgPnt1 = %2ld ** %10.4lf %10.4lf %10.4lf",trgPnt1,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"TrgPnt2 = %2ld ** %10.4lf %10.4lf %10.4lf",trgPnt2,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"TrgPnt3 = %2ld ** %10.4lf %10.4lf %10.4lf",trgPnt3,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt3)->z) ;
   }

// Check For Valid Triangle

 if ( trgPnt1 < 0  || trgPnt1 >= dtmP->numPoints || trgPnt2 < 0 || trgPnt2 >= dtmP->numPoints || trgPnt3 < 0 || trgPnt3 >= dtmP->numPoints ) goto errexit ;
 if(nodeAddrP(dtmP,trgPnt1)->cPtr == dtmP->nullPtr || nodeAddrP(dtmP,trgPnt2)->cPtr == dtmP->nullPtr  || nodeAddrP(dtmP,trgPnt3)->cPtr == dtmP->nullPtr)  goto errexit    ;
 if( ! bcdtmList_testLineDtmObject(dtmP,trgPnt1,trgPnt2)) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,trgPnt2,trgPnt3)) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,trgPnt3,trgPnt1)) goto errexit ;

// Set Edge Triangle Clockwise

 if( bcdtmMath_pointSideOfDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3) > 0 )
   {
    trgPnt  = trgPnt2 ;
    trgPnt2 = trgPnt3 ;
    trgPnt3 = trgPnt ;
   }

// Determine External Edge Of Triangle

 edgeCount = edgeP1 = edgeP2 = edgeP3 = 0 ;
 if( nodeAddrP(dtmP,trgPnt1)->hPtr == trgPnt3 )
   {
    ++edgeCount ;
    edgeP1 = 1 ;
   }
 if( nodeAddrP(dtmP,trgPnt3)->hPtr == trgPnt2 )
   {
    ++edgeCount ;
    edgeP3 = 1 ;
   }
 if( nodeAddrP(dtmP,trgPnt2)->hPtr == trgPnt1 )
   {
    ++edgeCount ;
    edgeP2 = 1 ;
   }

// Delete External Edge Triangle

 if( edgeCount == 1 )
   {
    if( edgeP1 && nodeAddrP(dtmP,trgPnt2)->hPtr == dtmP->nullPnt )
      {
       if( bcdtmList_deleteLineDtmObject(dtmP,trgPnt1,trgPnt3)) goto errexit ;
       nodeAddrP(dtmP,trgPnt1)->hPtr = trgPnt2 ;
       nodeAddrP(dtmP,trgPnt2)->hPtr = trgPnt3 ;
      }

    if( edgeP3 && nodeAddrP(dtmP,trgPnt1)->hPtr == dtmP->nullPnt )
      {
       if( bcdtmList_deleteLineDtmObject(dtmP,trgPnt3,trgPnt2)) goto errexit ;
       nodeAddrP(dtmP,trgPnt3)->hPtr = trgPnt1 ;
       nodeAddrP(dtmP,trgPnt1)->hPtr = trgPnt2 ;
      }

    if( edgeP2 && nodeAddrP(dtmP,trgPnt3)->hPtr == dtmP->nullPnt )
      {
       if( bcdtmList_deleteLineDtmObject(dtmP,trgPnt2,trgPnt1)) goto errexit ;
       nodeAddrP(dtmP,trgPnt2)->hPtr = trgPnt3 ;
       nodeAddrP(dtmP,trgPnt3)->hPtr = trgPnt1 ;
      }

//  Check Tin Topology And Feature Topology

    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Checking Tin After Deleting Edge Triangle") ;
       if( bcdtmCheck_tinComponentDtmObject(editDtmP))
         {
          bcdtmWrite_message(2,0,0,"Tin Invalid After Deleting External Triangle") ;
          goto errexit ;
         }
       bcdtmWrite_message(2,0,0,"Tin Valid After Deleting External Triangle") ;
      }

//  Update Last Modified Time

    bcdtmObject_updateLastModifiedTime (dtmP) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting External Triangle Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting External Triangle Error") ;
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
int bcdtmExtEdit_deleteTriangleDtmObject(BC_DTM_OBJ *dtmP,long tinPnt1,long tinPnt2,long tinPnt3)
/*
** This Deletes A Triangle
** Rob Cormack July 2003
**
*/
{
 int   ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long  numDtmFeatures ;
 DPoint3d   trgPts[4] ;
 DTM_TIN_POINT *pointP ;
 BC_DTM_OBJ *voidDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Tin Triangle" ) ;
    bcdtmWrite_message(0,0,0,"Tin Point1 = %2ld ** %10.4lf %10.4lf %10.4lf",tinPnt1,pointAddrP(dtmP,tinPnt1)->x,pointAddrP(dtmP,tinPnt1)->y,pointAddrP(dtmP,tinPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"Tin Point2 = %2ld ** %10.4lf %10.4lf %10.4lf",tinPnt2,pointAddrP(dtmP,tinPnt2)->x,pointAddrP(dtmP,tinPnt2)->y,pointAddrP(dtmP,tinPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"Tin Point3 = %2ld ** %10.4lf %10.4lf %10.4lf",tinPnt3,pointAddrP(dtmP,tinPnt3)->x,pointAddrP(dtmP,tinPnt3)->y,pointAddrP(dtmP,tinPnt3)->z) ;
   }
/*
** Test For Valid Line
*/
 if ( tinPnt1 < 0  || tinPnt1 >= dtmP->numPoints || tinPnt2 < 0 || tinPnt2 >= dtmP->numPoints || tinPnt3 < 0 || tinPnt3 >= dtmP->numPoints ) goto errexit ;
 if(nodeAddrP(dtmP,tinPnt1)->cPtr == dtmP->nullPtr || nodeAddrP(dtmP,tinPnt2)->cPtr == dtmP->nullPtr  || nodeAddrP(dtmP,tinPnt3)->cPtr == dtmP->nullPtr)  goto errexit    ;
 if( ! bcdtmList_testLineDtmObject(dtmP,tinPnt1,tinPnt2)) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,tinPnt2,tinPnt3)) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,tinPnt3,tinPnt1)) goto errexit ;
/*
** Create Dtm Object To Store Void Polygon
*/
 if( bcdtmObject_createDtmObject(&voidDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidDtmP,10,10) ;
/*
** Store Triangle Boundary As Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Store Triangle Boundary As Void") ;
 pointP = pointAddrP(dtmP,tinPnt1) ;
 trgPts[0].x = pointP->x ; trgPts[0].y = pointP->y ; trgPts[0].z = pointP->z ;
 trgPts[3].x = pointP->x ; trgPts[3].y = pointP->y ; trgPts[3].z = pointP->z ;
 pointP = pointAddrP(dtmP,tinPnt3) ;
 trgPts[1].x = pointP->x ; trgPts[1].y = pointP->y ; trgPts[1].z = pointP->z ;
 pointP = pointAddrP(dtmP,tinPnt2) ;
 trgPts[2].x = pointP->x ; trgPts[2].y = pointP->y ; trgPts[2].z = pointP->z ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Void,voidDtmP->nullUserTag,1,&voidDtmP->nullFeatureId,trgPts,4)) goto errexit ;
/*
** Insert Void Into Edit Tin
*/
 numDtmFeatures = dtmP->numFeatures ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Void Into Edit Tin") ;
 if( bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject(dtmP,voidDtmP) ) goto errexit ;
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation After Inserting Triangle Void") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted After Inserting Triangle Void") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid") ;
   }
/*
** Remove Inserted Voids If On Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
 if( bcdtmExtEdit_removeInsertedVoidsOnTinHullDtmObject(dtmP,numDtmFeatures)) goto errexit ;
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( voidDtmP  != NULL ) bcdtmObject_destroyDtmObject(&voidDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Tin Triangle Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Tin Triangle Error") ;
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
int bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject
(
 BC_DTM_OBJ *dtmP,             /* ==> Pointer To Edit Dtm                          */
 BC_DTM_OBJ *dataDtmP          /* ==> Pointer To Object Containing Insert Features */
)
/*
**
** Notes :-
** 1. Assumes Any Islands Are Internal To Voids
** 2. Voids And Islands Are Clean
**
*/
{
 int     ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long    pp,np,pnt,lpnt,spnt,feature,numStartFeatures,numEndFeatures,numVoids,numIslands ;
 DTMDirection direction;
 long    point,dtmFeature ;
 long    numIntersectedFeatures,numVoidsFeatures,internalVoid,voidLine,numHullLines ;
 double  area ;
 char    dtmFeatureTypeName[50] ;
 DTM_TIN_POINT  *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT_FEATURES *intersectedFeaturesP=NULL,*vfP,*voidFeaturesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Edit Tin") ;
 bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Write Island And Void Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Void And Island Features") ;
    for( dtmFeature = 0 ; dtmFeature < dataDtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataDtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Insert dtmFeature[%4ld] = %s",dtmFeature,dtmFeatureTypeName) ;
          for( pnt = dtmFeatureP->dtmFeaturePts.firstPoint ; pnt < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             pointP = pointAddrP(dataDtmP,pnt) ;
             bcdtmWrite_message(0,0,0,"point[%4ld] = %12.5lf %12.5lf %10.4lf",pnt-dtmFeatureP->dtmFeaturePts.firstPoint,pointP->x,pointP->y,pointP->z) ;
            }
         }
      }
   }
/*
** Initialise
*/
 numStartFeatures = dtmP->numFeatures ;
/*
** Scan And Insert Voids And Islands
*/
 numVoids = numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dataDtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dataDtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void ||dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) )
      {
/*
**     Store Void Or Island Feature As Sptr Polygon
*/
       if( dbg )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Storing %s As Sptr Polygon",dtmFeatureTypeName) ;
         }
       spnt = lpnt = dtmP->nullPnt ;
       for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts  ; ++point )
         {
          pointP = pointAddrP(dataDtmP,point) ;
          bcdtmFind_closestPointDtmObject(dtmP,pointP->x,pointP->y,&pnt) ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Closest Point = %6ld ** %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
          if( lpnt != dtmP->nullPnt ) nodeAddrP(dtmP,lpnt)->sPtr = pnt ;
          if( spnt == dtmP->nullPnt ) spnt = pnt ;
          lpnt = pnt ;
         }
/*
**     Write Sptr List
*/
       if( dbg == 1 ) bcdtmList_writeSptrListDtmObject(dtmP,spnt) ;
/*
**     Check Connectivity Of Sptr Polygon
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Sptr Polygon") ;
       if( bcdtmList_checkConnectivitySptrPolygonDtmObject(dtmP,spnt,1)) goto errexit ;
/*
**     Check Direction Of Sptr Polygon
*/
       bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,spnt,&area,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Sptr Polygon Area = %10.4lf ** Direction = %1ld",area,direction) ;
/*
**     If Sptr Polygon Direction Clock Wise Reverse Direction
*/
       if( direction == DTMDirection::Clockwise ) bcdtmList_reverseSptrPolygonDtmObject(dtmP,spnt) ;
/*
**     Insert Sptr Polygon Into Tin
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Sptr Polygon") ;
       pnt = spnt ;
       do
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Inserting Line %6ld - %6ld",pnt,nodeAddrP(dtmP,pnt)->sPtr) ;
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,pnt,nodeAddrP(dtmP,pnt)->sPtr,1,2) ) goto errexit ;
          pnt = nodeAddrP(dtmP,pnt)->sPtr ;
         } while ( pnt != spnt ) ;
/*
**     Null Sptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Sptr Polygon") ;
       if( bcdtmList_nullSptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**     Insert Tptr Polygon As A Polygon Feature With The User Tag set to The Feature Type
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Feature To Tin") ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Polygon,(DTMUserTag)dtmFeatureP->dtmFeatureType,dtmP->dtmFeatureIndex,spnt,1)) goto errexit ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserted Feature ** dtmFeature = %4ld ** type = %2ld usertag = %2ld",dtmP->numFeatures-1,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmFeatureType,ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmUserTag) ;
      }
   }
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,cdbg) ;
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Number Of Voids And Islands ** Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   Inserted = %2ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands Inserted = %2ld",numIslands) ;
   }
/*
** Get List Of DTM Polygonal Features Intersected By Inserted Void And Island Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Features") ;
 if( bcdtmExtEdit_getIslandVoidHoleFeaturesWithACommonHullSegementDtmObject(dtmP,numStartFeatures,&intersectedFeaturesP,&numIntersectedFeatures)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersected Features = %3ld",numIntersectedFeatures) ;
    for( vfP = intersectedFeaturesP ; vfP < intersectedFeaturesP + numIntersectedFeatures ; ++vfP )
      {
       bcdtmWrite_message(0,0,0,"intersectedFeature[%4ld] ** dtmFeature = %4ld dtmFeatureType = %4ld",(long)(vfP-intersectedFeaturesP),vfP->dtmFeature,vfP->dtmFeatureType) ;
      }
   }
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** If No Intersecting Features Detected, Just Revert Feature Type Back To Correct Type
*/
 if( numIntersectedFeatures == 0 )
   {
    for( feature = numStartFeatures ; feature < dtmP->numFeatures ; ++feature )
      {
       dtmFeatureP = ftableAddrP(dtmP,feature) ;
       dtmFeatureP->dtmFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag ;
       dtmFeatureP->dtmUserTag     = dtmP->nullUserTag ;
       dtmFeatureP->dtmFeatureId   = dtmP->dtmFeatureIndex ;
       ++dtmP->dtmFeatureIndex ;
      }
   }
/*
** If Intersecting Features Detected Resolve Overlaps
*/
 if( numIntersectedFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Overlapping Islands Voids And Holes") ;
    if( bcdtmExtEdit_resolveOverlappingIslandsVoidsAndHolesDtmObject(dtmP,numStartFeatures,intersectedFeaturesP,numIntersectedFeatures)) goto errexit ;
    if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Polygon)) goto errexit ;
   }
/*
** Void Points Internal To Voids And External To Islands
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Voiding Points External To Islands And Internal To Voids") ;
 for( pnt = numStartFeatures ; pnt < dtmP->numFeatures ; ++pnt )
   {
    if( ftableAddrP(dtmP,pnt)->dtmFeatureType == DTMFeatureType::Void   ) if( bcdtmMark_pointsInternalToVoidDtmObject(dtmP,pnt)) goto errexit ;
    if( ftableAddrP(dtmP,pnt)->dtmFeatureType == DTMFeatureType::Island ) if( bcdtmMark_pointsExternalToIslandDtmObject(dtmP,pnt)) goto errexit ;
   }
 numEndFeatures = dtmP->numFeatures ;
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Clip Breaks And Contours To Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Breaks And Contours") ;
 for( feature = 0 ; feature < numStartFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine )
         {
          if( bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(dtmP,feature)) goto errexit ;
         }
      }
   }
/*
** Clip Group Spots Internal To Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Breaks And Contours") ;
 for( feature = 0 ; feature < numStartFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          if( bcdtmExtEdit_clipVoidPointsFromGroupSpotsDtmObject(dtmP,feature)) goto errexit ;
         }
      }
   }
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                  bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Delete Internal Void Points And Retriangulate Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"********* Deleting Internal Void Points And Lines And Retriangulating") ;
 for( feature = numStartFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if(ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if(ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Void || ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Hole )
         {
          if( bcdtmExtEdit_deleteInternalVoidPointsAndLinesAndRetriangulateVoidDtmObject(dtmP,feature)) goto errexit ;
         }
      }
   }
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                  bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Delete Voids Internal To Other Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Voids Internal To Voids") ;
 for( feature = numStartFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if(ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Void )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Void %6ld Is Internal To Other Voids",feature) ;
/*
**     Check For Internal Void ** Coincident Hull Line With Island Hull
*/
       internalVoid = 0 ;
       numHullLines = 0 ;
       spnt = ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
/*
**     Check For Internal Void ** Internal To Surrounding Void
*/
       if( ! internalVoid )
         {
/*
**        Set Void Feature To DTMFeatureType::Polygon
*/
          spnt = ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
          if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,feature,spnt,&pp)) goto errexit ;
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,spnt,&np)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"pp   = %6ld ** %10.4lf %10.4lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"spnt = %6ld ** %10.4lf %10.4lf %10.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"np   = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
             if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,spnt,np) && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,np,spnt) )
               {
                if( ! bcdtmList_testForVoidHullLineDtmObject(dtmP,np,spnt))
                  {
                   if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,spnt,np)) < 0 ) goto errexit ;
                   while( pnt != pp && ! internalVoid && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,spnt,pnt) && ! bcdtmList_testForVoidHullLineDtmObject(dtmP,pnt,spnt) )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"cp*pnt = %6ld ** %10.4lf %10.4lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
                      if( bcdtmList_testForVoidLineDtmObject(dtmP,spnt,pnt,&internalVoid)) goto errexit ;
                      if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,spnt,pnt)) < 0 ) goto errexit ;
                     }
                  }
                if( dbg && internalVoid) bcdtmWrite_message(0,0,0,"internalVoid") ;
               }
             if( ! internalVoid && pnt != pp )
               {
                if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,spnt,pp) && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pp,spnt) )
                  {
                   if( ! bcdtmList_testForVoidHullLineDtmObject(dtmP,pp,spnt))
                     {
                      if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,spnt,pp)) < 0 ) goto errexit ;
                      while( pnt != np && ! internalVoid && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,spnt) && ! bcdtmList_testForVoidHullLineDtmObject(dtmP,spnt,pnt) )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"ap*pnt = %6ld ** %10.4lf %10.4lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
                         if( bcdtmList_testForVoidLineDtmObject(dtmP,spnt,pnt,&internalVoid)) goto errexit ;
                         if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,spnt,pnt)) < 0 ) goto errexit ;
                        }
                     }
                  }
                if( dbg && internalVoid) bcdtmWrite_message(0,0,0,"internalVoid") ;
               }
             pp = spnt ;
             spnt = np ;
            } while ( spnt != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint && ! internalVoid ) ;
/*
**        Reset Feature Type To Void
*/
          if( dbg && internalVoid ) bcdtmWrite_message(0,0,0,"Void Coincident Connects To External Void") ;
         }
/*
**     Delete Void If Internal
*/
       if( internalVoid )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Void %6ld",feature) ;
/*
**        Mark Void Hull Points As Void Points
*/
          spnt = ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
          do
            {
             if( ! bcdtmList_testForPointOnIslandHullDtmObject(dtmP,spnt))
               {
                bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,spnt)->PCWD) ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,spnt,&pnt)) goto errexit ;
             spnt = pnt ;
            } while ( spnt != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
/*
**        Clip Dtm Features Coincident With Void Hull
*/
          spnt = ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,spnt,&pnt)) goto errexit ;
             if( bcdtmList_testForVoidLineDtmObject(dtmP,spnt,pnt,&voidLine)) goto errexit ;
             if( voidLine )
               {
                if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,spnt,pnt,&voidFeaturesP,&numVoidsFeatures) ) goto errexit ;
                for( vfP = voidFeaturesP ; vfP < voidFeaturesP + numVoidsFeatures ; ++vfP )
                  {
                   if( vfP->dtmFeatureType == DTMFeatureType::Breakline || vfP->dtmFeatureType == DTMFeatureType::ContourLine )
                     {
                      if( bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(dtmP,vfP->dtmFeature)) goto errexit ;
                     }
                  }
               }
             spnt = pnt ;
            } while ( spnt != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;
/*
**        Delete Void Feature
*/
          if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,feature)) goto errexit ;
         }
      }
   }
/*
** Clip Breaks And Contours To Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Breaks And Contours") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    if(ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if(ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Breakline || ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::ContourLine )
         {
          if( bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(dtmP,feature)) goto errexit ;
         }
      }
   }
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Dtm Feature Topolgy Errors") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Dtm Feature Topolgy OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( intersectedFeaturesP != NULL ) free(intersectedFeaturesP) ;
 if( voidFeaturesP        != NULL ) free(voidFeaturesP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Edit Tin Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Edit Tin Error") ;
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
int bcdtmExtEdit_removeInsertedVoidsOnTinHullDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures )
/*
** This Function Remove Voids Coincident With Tin Hull
** Voids Can Only Be Removed if :-
**
** 1. The Void has only one contiguous section coincident with Tin Hull
** 2. There are no Internal Islands
** 3. The void can not be totally contiguous with the Tin Hull
**
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long spnt,fsp1,lsp1,fsp2,lsp2,fPtr,coinFlag,voidFeature,numIslands  ;
 DTM_TIN_POINT_FEATURES *islandsP=NULL ;
 BC_DTM_FEATURE *voidFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
/*
**  Scan For Void Feature
*/
 for( voidFeature = numStartFeatures ; voidFeature < dtmP->numFeatures ; ++voidFeature )
   {
    voidFeatureP = ftableAddrP(dtmP,voidFeature) ;
    if( voidFeatureP->dtmFeatureType == DTMFeatureType::Void && voidFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
/*
**     Get Islands Internal To Void Feature
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Getting Islands Internal To Void") ;
       if( bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(dtmP,voidFeature,&islandsP,&numIslands)) goto errexit ;
       if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
**     If No Internal Islands Then Determine Number Of Contiguous Hull Sections
*/
       if( numIslands == 0 )
         {
/*
**        Copy Void Feature To Tptr List
*/
          if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
          if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,spnt) ;
/*
**        Check For One Only Contiguous Void Section With Hull
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking For One Coincident Void Section With Tin Hull") ;
          if( bcdtmExtEdit_checkForOneCoincidentTptrVoidPolygonSectionWithTinHullDtmObject(dtmP,spnt,&fsp1,&lsp1,&coinFlag) != DTM_SUCCESS ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"fsp1 = %9ld lsp1 = %9ld coinFlag = %2ld",fsp1,lsp1,coinFlag) ;
          if( dbg && fsp1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"fsp1->fTableP = %9ld lsp1->fTableP = %9ld",nodeAddrP(dtmP,fsp1)->hPtr,nodeAddrP(dtmP,lsp1)->hPtr,coinFlag) ;
/*
**        Get Next Contiguous CoinCident Section With Tin Hull
*/
          if( fsp1 != dtmP->nullPnt &&  lsp1 != dtmP->nullPnt && ! coinFlag)
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Void Has Only One Hull Coincident Section") ;
/*
**           Copy Void Feature To Tptr List
*/
             if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
/*
**           Delete All Points And Lines Internal To Tptr Polygon
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Deleting All Points And Lines Internal To Tptr Polygon") ;
             if( bcdtmExtEdit_deleteAllPointsAndLinesInternalToTptrPolygonDtmObject(dtmP,spnt) != DTM_SUCCESS ) goto errexit ;
/*
**           Delete Void Feature
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Void Feature") ;
             if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,voidFeature)) goto errexit ;
/*
**           Clip Dtm Features Coincident With Tin Hull
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Clip Dtm Features Coincident With Tin Hull") ;
             fsp2 = fsp1 ;
             do
               {
/*
**              Clip Features With Coincident Hull Sections
*/
                lsp2 = nodeAddrP(dtmP,fsp2)->tPtr ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Feature Coincident With Line %9ld %9ld",fsp2,lsp2) ;
                if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,spnt)) goto errexit ;
                if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
                if( bcdtmExtEdit_clipDtmFeaturesCoincidentWithTinLineDtmObject(dtmP,fsp2,lsp2) ) goto errexit ;
                if( bcdtmList_copySptrListToTptrListDtmObject(dtmP,spnt)) goto errexit;
                if( bcdtmList_nullSptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**              Remove Group Spots
*/
                if( ( fPtr = nodeAddrP(dtmP,fsp2)->fPtr ) != dtmP->nullPtr )
                  {
                   while( fPtr != dtmP->nullPtr )
                     {
                      if( ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::GroupSpots )
                        {
                         if( bcdtmExtInsert_removePointFromDtmFeatureDtmObject(dtmP,fsp2,flistAddrP(dtmP,fPtr)->dtmFeature)) goto errexit ;
                         fPtr = nodeAddrP(dtmP,fsp2)->fPtr ;
                        }
                      else fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
                     }
                  }
/*
**              Get Next Section Of Hull
*/
                fsp2 = nodeAddrP(dtmP,fsp2)->tPtr ;
               } while ( fsp2 != lsp1 ) ;
/*
**           Delete Tptr Lines Coincident With Tin Hull
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Tptr Lines Coincident With Tin Hull") ;
             fsp2 = fsp1 ;
             do
               {
                lsp2 = nodeAddrP(dtmP,fsp2)->tPtr ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Line %9ld %9ld",fsp2,lsp2) ;
                if( bcdtmList_deleteLineDtmObject(dtmP,fsp2,lsp2)) goto errexit ;
                fsp2 = nodeAddrP(dtmP,fsp2)->tPtr ;
               } while ( fsp2 != lsp1 ) ;
/*
**           Null Out Tptr List
*/
             if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**           Reset Convex Hull
*/
             if( bcdtmList_setConvexHullDtmObject(dtmP) ) goto errexit ;
            }
/*
**        Null Tptr List
*/
          else if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
         }
      }
   }
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Dtm Feature Topolgy Error") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Dtm Feature Topolgy OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Inserted Voids On Tin Hull Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Removing Inserted Voids On Tin Hull Error") ;
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
int bcdtmExtEdit_getListOfIntersectedIslandVoidHoleFeaturesDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTM_TIN_POINT_FEATURES **intersectedFeaturesPP,long *numIntersectedFeaturesP)
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  cln,fpnt,npnt,feature,storeFlag,dtmFeature,memFeatureTable=0,memInc=100 ;
 char  dtmFeatureTypeName[50] ;
 DTM_TIN_POINT_FEATURES *flP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features") ;
/*
** Initialise
*/
 *numIntersectedFeaturesP = 0 ;
 if( *intersectedFeaturesPP != NULL ) { free(*intersectedFeaturesPP) ; *intersectedFeaturesPP = NULL ; }
/*
** Scan For DTMFeatureType::Polygon And Get List Of Intersecting Islands,Voids and Holes
*/
 for( dtmFeature = numStartFeatures ; dtmFeature <= dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature %6ld",dtmFeature) ;
       fpnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       do
         {
/*
**        Scan Features Passing Through Point
*/
          cln = nodeAddrP(dtmP,fpnt)->fPtr ;
          while ( cln != dtmP->nullPtr )
            {
             feature = flistAddrP(dtmP,cln)->dtmFeature ;
             cln     = flistAddrP(dtmP,cln)->nextPtr ;
             if( dbg )
               {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(ftableAddrP(dtmP,feature)->dtmFeatureType,dtmFeatureTypeName) ;
                bcdtmWrite_message(0,0,0,"intersectedFeature = %6ld Type = %-20s ** x = %12.5lf y = %12.5lf",feature,dtmFeatureTypeName,pointAddrP(dtmP,fpnt)->x,pointAddrP(dtmP,fpnt)->y) ;
               }
/*
**           Test For Intersecting Feature
*/
             if( ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Island ||ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Void || ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Hole ))
               {
/*
**              Check Feature Not Already In List
*/
                storeFlag = 1 ;
                for( flP = *intersectedFeaturesPP ; flP < *intersectedFeaturesPP + *numIntersectedFeaturesP && storeFlag ; ++flP )
                  {
                   if( flP->dtmFeature == feature ) storeFlag = 0 ;
                  }

/*
**              Allocate memory If Necessary To Store Intersected Feature
*/
                if( storeFlag )
                  {
                   if( *numIntersectedFeaturesP == memFeatureTable )
                     {
                      memFeatureTable = memFeatureTable + memInc ;
                      if( *intersectedFeaturesPP == NULL )  *intersectedFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
                      else                                *intersectedFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc( *intersectedFeaturesPP,memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
                      if( *intersectedFeaturesPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                     }
/*
**                 Store Feature In List
*/
                   (*intersectedFeaturesPP+*numIntersectedFeaturesP)->dtmFeature = feature ;
                   (*intersectedFeaturesPP+*numIntersectedFeaturesP)->dtmFeatureType = ftableAddrP(dtmP,feature)->dtmFeatureType ;
                   ++*numIntersectedFeaturesP ;
                  }
               }
            }
/*
**         Get Next Point In Feature List
*/
           if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,fpnt,&npnt)) goto errexit ;
           fpnt = npnt ;
         } while ( fpnt != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
   }
/*
** Resize Memory If Necessary
*/
 if( *intersectedFeaturesPP != NULL && *numIntersectedFeaturesP != memFeatureTable )
   {
    *intersectedFeaturesPP = (DTM_TIN_POINT_FEATURES *) realloc ( *intersectedFeaturesPP , *numIntersectedFeaturesP * sizeof(DTM_TIN_POINT_FEATURES)) ;
   }
/*
** Write Intersected Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersected Features = %3ld",*numIntersectedFeaturesP) ;
    for( flP = *intersectedFeaturesPP ; flP < *intersectedFeaturesPP + *numIntersectedFeaturesP  ; ++flP )
      {
       bcdtmWrite_message(0,0,0,"Intersected Feature[%3ld] = %6ld Type = %4ld",(long)(flP-*intersectedFeaturesPP),flP->dtmFeature,flP->dtmFeatureType) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features Error") ;
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
int bcdtmExtEdit_getIslandVoidHoleFeaturesWithACommonHullSegementDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTM_TIN_POINT_FEATURES **intersectedFeaturesPP,long *numIntersectedFeaturesP)
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  fpnt,npnt,storeFlag,dtmFeature,numLineFeatures ;
 long  memFeatureTable=0,memInc=100 ;
 char  dtmFeatureTypeName[50] ;
 DTM_TIN_POINT_FEATURES *flP,*lineP,*lineFeaturesP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features") ;
/*
** Initialise
*/
 *numIntersectedFeaturesP = 0 ;
 if( *intersectedFeaturesPP != NULL ) { free(*intersectedFeaturesPP) ; *intersectedFeaturesPP = NULL ; }
/*
** Scan For DTMFeatureType::Polygon And Get List Of Intersecting Islands,Voids and Holes
*/
 for( dtmFeature = numStartFeatures ; dtmFeature <= dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature %6ld",dtmFeature) ;
       fpnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       do
         {
/*
**        Get Next Feature Point
*/
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,fpnt,&npnt)) goto errexit ;
          if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,fpnt,npnt,&lineFeaturesP,&numLineFeatures)) goto errexit ;
/*
**        Scan Features Passing Through Line
*/
          for( lineP = lineFeaturesP ; lineP < lineFeaturesP + numLineFeatures ; ++lineP )
            {
             if( dbg )
               {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(lineP->dtmFeatureType,dtmFeatureTypeName) ;
                bcdtmWrite_message(0,0,0,"intersectedFeature = %6ld Type = %-20s ** x = %12.5lf y = %12.5lf",lineP->dtmFeature,dtmFeatureTypeName,pointAddrP(dtmP,fpnt)->x,pointAddrP(dtmP,fpnt)->y) ;
               }
/*
**           Test For Intersecting Feature
*/
             if( ftableAddrP(dtmP,lineP->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ( lineP->dtmFeatureType == DTMFeatureType::Island || lineP->dtmFeatureType == DTMFeatureType::Void || lineP->dtmFeatureType == DTMFeatureType::Hole ))
               {
/*
**              Check Feature Not Already In List
*/
                storeFlag = 1 ;
                for( flP = *intersectedFeaturesPP ; flP < *intersectedFeaturesPP + *numIntersectedFeaturesP && storeFlag ; ++flP )
                  {
                   if( flP->dtmFeature == lineP->dtmFeature ) storeFlag = 0 ;
                  }
/*
**             Allocate memory If Necessary To Store Intersected Feature
*/
               if( storeFlag )
                 {
                  if( *numIntersectedFeaturesP == memFeatureTable )
                    {
                     memFeatureTable = memFeatureTable + memInc ;
                     if( *intersectedFeaturesPP == NULL )  *intersectedFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
                     else                                  *intersectedFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc( *intersectedFeaturesPP,memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
                     if( *intersectedFeaturesPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                    }
/*
**                 Store Feature In List
*/
                  (*intersectedFeaturesPP+*numIntersectedFeaturesP)->dtmFeature = lineP->dtmFeature ;
                  (*intersectedFeaturesPP+*numIntersectedFeaturesP)->dtmFeatureType = lineP->dtmFeatureType ;
                   ++*numIntersectedFeaturesP ;
                  }
/*
**              Free Memory
*/
                if( lineFeaturesP != NULL ) { free(lineFeaturesP) ; lineFeaturesP = NULL ; }
               }
            }
/*
**        Get Next Point In Feature List
*/
          fpnt = npnt ;
         } while ( fpnt != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
   }
/*
** Resize Memory If Necessary
*/
 if( *intersectedFeaturesPP != NULL && *numIntersectedFeaturesP != memFeatureTable )
   {
    *intersectedFeaturesPP = (DTM_TIN_POINT_FEATURES *) realloc ( *intersectedFeaturesPP , *numIntersectedFeaturesP * sizeof(DTM_TIN_POINT_FEATURES)) ;
   }
/*
** Write Intersected Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersected Features = %3ld",*numIntersectedFeaturesP) ;
    for( flP = *intersectedFeaturesPP ; flP < *intersectedFeaturesPP + *numIntersectedFeaturesP  ; ++flP )
      {
       bcdtmWrite_message(0,0,0,"Intersected Feature[%3ld] = %6ld Type = %4ld",(long)(flP-*intersectedFeaturesPP),flP->dtmFeature,flP->dtmFeatureType) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( lineFeaturesP != NULL ) { free(lineFeaturesP) ; lineFeaturesP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting List Of Intersected Island Void And Hole Features Error") ;
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
int bcdtmExtEdit_resolveOverlappingIslandsVoidsAndHolesDtmObject
(
 BC_DTM_OBJ             *dtmP,
 long                   numStartFeatures,
 DTM_TIN_POINT_FEATURES *intersectedFeaturesP,
 long                   numIntersectedFeatures
)
{
 int     ret=DTM_SUCCESS,dbg=0 /*DHDH1*/,cdbg=0 ;
 long    point,fpnt,npnt,tpnt,dtmFeature,numFeaturePts ;
 DPoint3d     *featurePtsP=NULL ;
 char    dtmFeatureTypeName[50] ;
 BC_DTM_OBJ *featuresDtmP=NULL,*resolvedFeaturesDtmP=NULL ;
 DTM_TIN_POINT_FEATURES *featP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pointP ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Overlapping Islands Voids And Holes") ;
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation After Before Resolving Overlapping Voids And Islands") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted After Inserting Triangle Void") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid") ;
   }
/*
** Initialse Features Object
*/
 if( bcdtmObject_createDtmObject(&featuresDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(featuresDtmP,1000,10000) ;
/*
** Store Tin Features In Features Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing New Tin Features In Data Object") ;
 for( dtmFeature = numStartFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(featuresDtmP,DTMFeatureType::Breakline,dtmFeatureP->dtmUserTag*100,1,&dtmP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
       if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
      }
   }
/*
** Store Intersected Features In Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Existing Intersected Tin Features") ;
 for( featP = intersectedFeaturesP ; featP < intersectedFeaturesP + numIntersectedFeatures ; ++featP )
   {
    dtmFeatureP = ftableAddrP(dtmP,featP->dtmFeature) ;
    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,featP->dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(featuresDtmP,DTMFeatureType::Breakline,(DTMUserTag)dtmFeatureP->dtmFeatureType,1,&dtmP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,featP->dtmFeature)) goto errexit ;
   }
/*
** Set Tolerances For Feature Object
*/
 featuresDtmP->ppTol = dtmP->mppTol ;
 featuresDtmP->plTol = dtmP->mppTol ;
/*
** Resolve Overlapping Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Islands Voids Holes") ;
 if( bcdtmExtEdit_resolveIslandsVoidsHolesDtmObject(featuresDtmP,&resolvedFeaturesDtmP)) goto errexit ;
/*
** Store Dtm Features In Tin
*/
 if( resolvedFeaturesDtmP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Resolved Dtm Features In Tin") ;
    for( dtmFeature = 0 ; dtmFeature < resolvedFeaturesDtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(resolvedFeaturesDtmP,dtmFeature) ;
       if( dbg )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Storing Dtm Feature %s",dtmFeatureTypeName) ;
         }
/*
**     Store Void Or Island Feature As Tptr Polygon
*/
       fpnt = npnt = dtmP->nullPnt ;
       for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
         {
          pointP = pointAddrP(resolvedFeaturesDtmP,point) ;
          bcdtmFind_closestPointDtmObject(dtmP,pointP->x,pointP->y,&tpnt) ;
          if( fpnt == dtmP->nullPnt )
            {
             fpnt = tpnt ;
             nodeAddrP(dtmP,fpnt)->tPtr = dtmP->nullPnt ;
            }
          else nodeAddrP(dtmP,npnt)->tPtr = tpnt ;
          if( tpnt != fpnt ) nodeAddrP(dtmP,tpnt)->tPtr = dtmP->nullPnt ;
          npnt = tpnt ;
         }
/*
**     Write Out Tptr List
*/
       if( dbg == 1  ) bcdtmList_writeTptrListDtmObject(dtmP,fpnt) ;
/*
**     Check Connectivity Of Tptr List
*/
       if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,fpnt,dbg))
         {
          bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr List") ;
          goto errexit ;
         }
/*
**     Store Feature In Tin
*/
       dtmFeatureId = dtmP->dtmFeatureIndex ;
       ++dtmP->dtmFeatureIndex ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmP->nullUserTag,dtmFeatureId,fpnt,1)) goto errexit ;
      }
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation After Resolving Overlapping Voids And Islands") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted After Resolving Overlapping Voids And Islands") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP          != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
 if( featuresDtmP         != NULL ) bcdtmObject_destroyDtmObject(&featuresDtmP) ;
 if( resolvedFeaturesDtmP != NULL ) bcdtmObject_destroyDtmObject(&resolvedFeaturesDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Overlapping Islands Voids And Holes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Overlapping Islands Voids And Holes Error") ;
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
int bcdtmExtEdit_resolveIslandsVoidsHolesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **resolvedFeaturesDtmPP)
/*
** This Function Resolves Overlapping Dtm Polygonal Features
*/
{
 int     ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long    pnt,fpnt,npnt,pass,dtmFeature,offset,numMarked,dtmMsgLevel,numFeaturePts ;
 long    *lineP,*featP,*tinLinesP=NULL,*featureLinesP=NULL,numTinLines ;
 long    numFeatures,iniPoints,incPoints,dtmFeatureMarkType ;
 DTMFeatureType dtmFeatureType;
 char    dtmFeatureTypeName[50] ;
 DPoint3d     *featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Islands Voids Holes") ;
/*
** Write Out Debug Information
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features = %6ld",dtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature[%4ld] = %s",dtmFeature,dtmFeatureTypeName) ;
          for( pnt = dtmFeatureP->dtmFeaturePts.firstPoint ; pnt < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++pnt )
            {
             pointP = pointAddrP(dtmP,pnt) ;
             bcdtmWrite_message(0,0,0,"point[%4ld] = %12.5lf %12.5lf %10.4lf",pnt-dtmFeatureP->dtmFeaturePts.firstPoint,pointP->x,pointP->y,pointP->z) ;
            }
         }
      }
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation Before Resolving Voids And Islands") ;
    if( bcdtmCheck_tinComponentDtmObject(editDtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Corrupted Before Resolving Voids And Islands") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Triangulation Valid") ;
   }
/*
** Initialise
*/
 dtmMsgLevel = DTM_MSG_LEVEL ;
 iniPoints   = (dtmP)->iniPoints ;
 incPoints   = (dtmP)->incPoints ;
/*
** Triangulate The Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating DTM") ;
 DTM_MSG_LEVEL = 0 ;
 // TODO: DH if( bcdtmExtObject_triangulateDtmObject(dtmP)) goto errexit ;
 DTM_MSG_LEVEL = dtmMsgLevel ;
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
/*
** Clean Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM") ;
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"islandVoidHole.dtm") ;
/*
** Reset Dtm Feature Types To Correct Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reseting Feature Types") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    dtmFeatureP->dtmFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureP->dtmFeatureType = %6ld",dtmFeatureP->dtmFeatureType) ;
   }
/*
** Allocate Memory For Tin Line And Feature Assignments
*/
 numTinLines = dtmP->cListPtr ;
 tinLinesP = ( long * ) malloc( numTinLines * sizeof(long)) ;
 if( tinLinesP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 featureLinesP = ( long * ) malloc( numTinLines * sizeof(long)) ;
 if( featureLinesP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Mark Tin And Feature Lines
*/
 for( lineP = tinLinesP     ; lineP < tinLinesP     + numTinLines ; ++lineP ) *lineP = 0 ;
 for( lineP = featureLinesP ; lineP < featureLinesP + numTinLines ; ++lineP ) *lineP = 1 ;
/*
** Do Three Passes And Directionally Mark Void Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Directionally Marking All Void Lines") ;
 for( pass = 0 ; pass < 3 ; ++pass )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if(dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
/*
**        Directionally Mark Void Lines
*/
          dtmFeatureMarkType = 0 ;
          if( pass == 0 && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island       ) dtmFeatureMarkType = 1 ;
          if( pass == 0 && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void         ) dtmFeatureMarkType = 2 ;
          if( pass == 0 && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole         ) dtmFeatureMarkType = 3 ;
          if( pass == 1 && dtmFeatureP->dtmFeatureType == (DTMFeatureType)(((int)DTMFeatureType::Void)   * 100) ) dtmFeatureMarkType = 4 ;
          if (pass == 2 && dtmFeatureP->dtmFeatureType == (DTMFeatureType)(((int)DTMFeatureType::Island) * 100)) dtmFeatureMarkType = 5;
/*
**        Marked Features
*/
          if( dtmFeatureMarkType )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Marking Feature Type %4ld",(long)dtmFeatureP->dtmFeatureType) ;
/*
**           Copy Feature To Tptr List
*/
             if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&fpnt)) goto errexit ;
/*
**           Mark Void Tin Lines
*/
             numMarked = 0 ;
             if( dtmFeatureMarkType == 1 ) if( bcdtmMark_directionalVoidLinesExternalToTptrIslandPolygonDtmObject(dtmP,fpnt,tinLinesP,1,&numMarked ) ) goto errexit ;
             if( dtmFeatureMarkType == 2 ) if( bcdtmMark_directionalVoidLinesInternalToTptrVoidPolygonDtmObject(dtmP,fpnt,tinLinesP,1,&numMarked ) ) goto errexit ;
             if( dtmFeatureMarkType == 3 ) if( bcdtmMark_directionalTinLinesInternalToTptrPolygonDtmObject(dtmP,fpnt,tinLinesP,1,&numMarked ) ) goto errexit ;
             if( dtmFeatureMarkType == 4 ) if( bcdtmMark_directionalTinLinesInternalToTptrPolygonDtmObject(dtmP,fpnt,tinLinesP,1,&numMarked ) ) goto errexit ;
             if( dtmFeatureMarkType == 5 ) if( bcdtmMark_directionalTinLinesInternalToTptrPolygonDtmObject(dtmP,fpnt,tinLinesP,1,&numMarked ) ) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Void Lines Marked = %6ld",numMarked) ;
/*
**           Modify Feature Lines
*/
             for( featP = featureLinesP , lineP = tinLinesP ; lineP < tinLinesP + numTinLines ; ++featP , ++lineP )
               {
                if( *lineP )
                  {
                   if( dtmFeatureMarkType == 1 ) if( *featP != 3 ) *featP = 2 ;
                   if( dtmFeatureMarkType == 2 ) if( *featP != 3 ) *featP = 2 ;
                   if( dtmFeatureMarkType == 3 ) *featP = 3 ;
                   if( dtmFeatureMarkType == 4 ) *featP = 2 ;
                   if( dtmFeatureMarkType == 5 ) *featP = 1 ;
                  }
               }
/*
**           Null Out Tptr List
*/
             if( bcdtmList_nullTptrListDtmObject(dtmP,fpnt)) goto errexit ;
/*
**           Null Out Tin Lines
*/
             for( lineP = tinLinesP ; lineP < tinLinesP + numTinLines ; ++lineP ) *lineP = 0 ;
            }
         }
      }
   }
/*
** Null Out All Lines On Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Directionally Nulling Out Tin Hull Lines") ;
 fpnt = dtmP->hullPoint ;
 do
   {
    npnt = nodeAddrP(dtmP,fpnt)->hPtr ;
    if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,fpnt,npnt) ) goto errexit ;
    *(featureLinesP+offset) = 0 ;
    if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,fpnt,npnt) )
      {
       while( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,fpnt,npnt))
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,fpnt,npnt) ) goto errexit ;
          *(featureLinesP+offset) = 0 ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,fpnt) ) goto errexit ;
          *(featureLinesP+offset) = 0 ;
          if(( npnt = bcdtmList_nextAntDtmObject(dtmP,fpnt,npnt)) < 0 ) goto errexit ;
         }
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,fpnt,npnt) ) goto errexit ;
       *(featureLinesP+offset) = 0 ;
       pass = nodeAddrP(dtmP,fpnt)->hPtr ;
       if(( npnt = bcdtmList_nextClkDtmObject(dtmP,pass,fpnt)) < 0 ) goto errexit ;
       while( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,pass,npnt))
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pass,npnt) ) goto errexit ;
          *(featureLinesP+offset) = 0 ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,pass) ) goto errexit ;
          *(featureLinesP+offset) = 0 ;
          if(( npnt = bcdtmList_nextClkDtmObject(dtmP,pass,npnt)) < 0 ) goto errexit ;
         }
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,pass) ) goto errexit ;
       *(featureLinesP+offset) = 0 ;
      }
    fpnt = nodeAddrP(dtmP,fpnt)->hPtr ;
   } while ( fpnt != dtmP->hullPoint ) ;
/*
** Polygonise Dtm Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Tin Feature Lines") ;
 numFeatures = dtmP->numFeatures ;
 if( bcdtmClean_polygoniseTinLinesDtmObject(dtmP,featureLinesP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygonised Features = %6ld",dtmP->numFeatures-numFeatures) ;
/*
** Extract Polygonised DTM Features
*/
 if( numFeatures < dtmP->numFeatures )
   {
/*
**  Create Dtm Object
*/
    if( bcdtmObject_createDtmObject(resolvedFeaturesDtmPP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(*resolvedFeaturesDtmPP,iniPoints,incPoints) ;
/*
**  Extract Features From Tin
*/
    for( dtmFeature = numFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dbg )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Feature Type %4ld ** %20s  UserTag = %4I64d",dtmFeatureP->dtmFeatureType,dtmFeatureTypeName,dtmFeatureP->dtmUserTag ) ;
         }
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon )
         {
/*
**        Assign Feature Type
*/
          dtmFeatureType = DTMFeatureType::Island ;
          if     ( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 1 ) dtmFeatureType = DTMFeatureType::Island ;
          else if( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 2 ) dtmFeatureType = DTMFeatureType::Void   ;
          else if( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 3 ) dtmFeatureType = DTMFeatureType::Hole   ;
/*
**        Store Feature
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(*resolvedFeaturesDtmPP,dtmFeatureType,(*resolvedFeaturesDtmPP)->nullUserTag,1,&(*resolvedFeaturesDtmPP)->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 DTM_MSG_LEVEL = dtmMsgLevel ;
 if( featurePtsP   != NULL ) { free(featurePtsP)   ; featurePtsP   = NULL ; }
 if( tinLinesP     != NULL ) { free(tinLinesP)     ; tinLinesP     = NULL ; }
 if( featureLinesP != NULL ) { free(featureLinesP) ; featureLinesP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Islands Voids Holes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Islands Voids Holes Object Error") ;
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
int bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature )
/*
** This Function Clips Void Lines From DTM Features
** Function Updated 29 May 2003 RobC
*/
{
 int         ret=DTM_SUCCESS,dbg=0 ;
 long        sp,np,fsp,spnt,tptr,process,voidLine ;
 DTMFeatureType type;
 DTMUserTag utag ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Void Lines From Dtm Feature %6ld",dtmFeature) ;
/*
** Initialise
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures  ) goto cleanup ;
 if( ( spnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint) == dtmP->nullPnt ) goto cleanup ;
 utag = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 type = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType  ;
/*
** Scan For Feature Void Lines
*/
 sp = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
        voidLine = 0 ;
    if( np != dtmP->nullPnt )
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine) )goto errexit  ;
      }
    if( ! voidLine ) sp = np ;
   } while ( sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint && sp != dtmP->nullPnt && ! voidLine ) ;
/*
** If Void Line - Delete Existing Feature And Insert Non Voids Segments Of Feature
*/
 if( voidLine )
   {
/*
**  Write Void Line
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
**  Delete Feature From Tin Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Feature") ;
    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit  ;
/*
**  Scan And Insert New Features
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
             if( voidLine ) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
            }
         } while ( np != spnt && np != dtmP->nullPnt && voidLine ) ;
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
                if( ! voidLine ) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
               }
            } while ( np != spnt && np != dtmP->nullPnt &&  voidLine ) ;
          tptr = nodeAddrP(dtmP,sp)->tPtr ;
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
/*
**       Store Dtm Feature
*/
          if( nodeAddrP(dtmP,fsp)->tPtr != dtmP->nullPnt )
            {
             if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,type,utag,dtmP->dtmFeatureIndex,fsp,1)) goto errexit  ;
             ++dtmP->dtmFeatureIndex ;
            }
/*
**        Get Next Non Void Segment
*/
          nodeAddrP(dtmP,sp)->tPtr = tptr ;
         }
      }
/*
**  Null Out Sptr And Tptr List
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
 if( ret == DTM_SUCCESS )ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_clipVoidPointsFromGroupSpotsDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature )
/*
** This Function Clips Void Points From DTM Features
** Function Updated 29 May 2003 RobC
*/
{
 int         ret=DTM_SUCCESS,dbg=0 ;
 long        sp,np,fsp,spnt,tptr,process,voidPoint ;
 DTMFeatureType type;
 DTMUserTag utag ;
 DTMFeatureId  featureId ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Group From Dtm Feature %6ld",dtmFeature) ;
/*
** Initialise
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures  ) goto cleanup ;
 if( ( spnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint) == dtmP->nullPnt ) goto cleanup ;
 utag = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 type = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType  ;
/*
** Scan For Feature Void Points
*/
 voidPoint = 0 ;
 sp = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ) voidPoint = 1 ;
    if( ! voidPoint )
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
       sp = np ;
      }
   } while ( sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint && sp != dtmP->nullPnt && ! voidPoint ) ;
/*
** If Void Point - Delete Existing Feature And Insert Non Void Points Of Feature
*/
 if( voidPoint )
   {
/*
**  Write Void Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Void Point Detected ** dtmFeature = %6ld sp = %6ld",dtmFeature,sp) ;
/*
**  Copy DTM Features To Tptr Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature To Tptr Array") ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&spnt)) goto errexit  ;
    if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,spnt)) goto errexit  ;
    if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,spnt) ;
/*
**  Delete Feature From Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Feature") ;
    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit  ;
/*
**  Scan And Insert New Features
*/
    sp = spnt ;
    process = 1 ;
    while( process )
      {
/*
**     Scan To First Non Void Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To First Non Void Point") ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
       do
         {
          if( np != dtmP->nullPnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"sp = %6ld np = %6ld",sp,np) ;
             if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ) voidPoint = 1 ;
             if( voidPoint ) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
            }
         } while ( np != spnt && np != dtmP->nullPnt && voidPoint ) ;
/*
**     Scan To Next Void Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Next Void Point") ;
       if( np == spnt || np == dtmP->nullPnt ) process = 0 ;
       else
         {
          fsp = sp ;
          np = nodeAddrP(dtmP,sp)->tPtr ;
          do
            {
             if( np != dtmP->nullPnt )
               {
                if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ) voidPoint = 1 ;
                if( ! voidPoint ) { sp = np ; np = nodeAddrP(dtmP,sp)->tPtr ; }
               }
            } while ( np != spnt && np != dtmP->nullPnt &&  voidPoint ) ;
          tptr = nodeAddrP(dtmP,sp)->tPtr ;
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
/*
**       Store Dtm Feature
*/
          if( nodeAddrP(dtmP,fsp)->tPtr != dtmP->nullPnt )
            {
             featureId = dtmP->dtmFeatureIndex ;
             ++dtmP->dtmFeatureIndex ;
             if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,type,utag,featureId,fsp,1)) goto errexit  ;
            }
/*
**        Get Next Non Void Segment
*/
          nodeAddrP(dtmP,sp)->tPtr = tptr ;
         }
      }
/*
**  Null Out Sptr And Tptr List
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
/*
** Function Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Void Points From Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Void Points From Dtm Feature %6ld Error",dtmFeature) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_deleteAllPointsAndLinesInternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long spnt )
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long cp,lp,np,sp,clc,mark=-98765432,numMarked,numLines=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting All Points And Lines Internal To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"spnt  = %8ld",spnt) ;
   }
/*
** Mark Internal Tptr Polygon Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Tptr Polygon Points") ;
 if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,spnt,mark,&numMarked)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",numMarked) ;
/*
** Delete Lines Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Connecting To Internal Points") ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->tPtr == mark )
      {
if( nodeAddrP(dtmP,sp)->fPtr != dtmP->nullPtr )
{
 bcdtmWrite_message(0,0,0,"XXXXX Deleting Internal Point With Feature List") ;
}
       clc = nodeAddrP(dtmP,sp)->cPtr ;
       while ( clc != dtmP->nullPtr )
         {
          cp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp) ) goto errexit  ;
          ++numLines ;
         }
       nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,sp)->sPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,sp)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,sp)->fPtr = dtmP->nullPtr ;
      }
   }
/*
** Delete All Internal Lines Connected To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Internal To Clip Polygon")  ;
 sp = spnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit  ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit  ;
       if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit  ;
       ++numLines ;
       cp = lp ;
      }
    sp = np ;
  } while( sp != spnt ) ;
/*
** Write Statistics
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Deleted = %6ld",numMarked) ;
    bcdtmWrite_message(0,0,0,"Number Of Lines  Deleted = %6ld",numLines) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Points And Lines Internal To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Points And Lines Internal To Tptr Polygon Error") ;
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
int bcdtmExtEdit_clipDtmFeaturesCoincidentWithTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2 )
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long p, p1, p2, sp, np, clc, fpnt, feature;
 DTMFeatureType type;
 DTMFeatureId  featureId ;
 DTMUserTag     utag ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features Coincident With Tin Line") ;
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Scan Point For Features
*/
 for( p = 0 ; p < 2 ; ++p )
   {
    if( p == 0 ) { p1 = P1 ; p2 = P2 ; }
    else         { p1 = P2 ; p2 = P1 ; }
    clc = nodeAddrP(dtmP,p1)->fPtr ;
    while( clc != dtmP->nullPtr )
      {
       if( flistAddrP(dtmP,clc)->nextPnt == p2 && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
          fpnt       = ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint ;
          feature    = flistAddrP(dtmP,clc)->dtmFeature ;
          type       = ftableAddrP(dtmP,feature)->dtmFeatureType ;
          utag       = ftableAddrP(dtmP,feature)->dtmUserTag ;
          featureId  = ftableAddrP(dtmP,feature)->dtmFeatureId ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Feature %6ld Type = %4ld",feature,type) ;
/*
**        Copy Feature To Tptr List
*/
          if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,feature,&fpnt)) goto errexit ;
/*
**        Scan To Start Of Clip Line
*/
          sp = fpnt ;
          do
            {
             np = nodeAddrP(dtmP,sp)->tPtr ;
             if( sp == p1 && np == p2  || sp == p2 && np == p1 )
               {
/*
**             Insert Clipped Feature
*/
                nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
                if( sp != fpnt ) if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,type,utag,featureId,fpnt,1)) goto errexit ;
                if( nodeAddrP(dtmP,np)->tPtr != dtmP->nullPnt ) if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,type,utag,dtmP->dtmFeatureIndex,np,1)) goto errexit ;
                sp = dtmP->nullPnt ;
               }
             else sp = np ;
            } while ( sp != dtmP->nullPnt ) ;
/*
**        Delete Feature
*/
          if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,feature, true)) goto errexit ;
/*
**        Rescan Features For Point
*/
          clc = nodeAddrP(dtmP,p1)->fPtr ;
         }
/*
**     Set For Next Feature In Point List
*/
       else clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features Coincident With Tin Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features Coincident With Tin Line Error") ;
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
int bcdtmExtEdit_checkForOneCoincidentTptrVoidPolygonSectionWithTinHullDtmObject(BC_DTM_OBJ *dtmP,long spnt,long *fsP,long *lsP,long *coincidentFlagP)
{
 int  ret=DTM_SUCCESS ;
 long cp,sp,np,ncp ;
/*
** Initialise
*/
 *fsP = *lsP = dtmP->nullPnt ;
 *coincidentFlagP = 0 ;
/*
** Get First Non Coincident Tptr Line With Tin Hull
*/
 sp = spnt ;
 ncp = dtmP->nullPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( nodeAddrP(dtmP,sp)->hPtr != np )  ncp = sp ;
    sp = np  ;
   }  while ( ncp == dtmP->nullPnt && sp != spnt && sp != dtmP->nullPnt ) ;
 if( sp == dtmP->nullPnt ) goto errexit ;
/*
** If No Non Coincident Lines Then Tptr List Is Coincident With Tin Hull
*/
 if( ncp == dtmP->nullPnt ) { *fsP = *lsP = spnt ; *coincidentFlagP = 1 ; }
/*
** Get First First Coincident Tptr Line With Tin Hull
*/
 else
   {
    sp = ncp ;
    cp = dtmP->nullPnt ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( nodeAddrP(dtmP,sp)->hPtr == np ) cp = sp ;
       sp = np ;
      }  while ( cp == dtmP->nullPnt && sp != ncp && sp != dtmP->nullPnt ) ;
/*
**  Get First Non Coincident Tptr Line With Tin Hull
*/
    if( cp != dtmP->nullPnt )
      {
       sp = *fsP = cp ;
       ncp = dtmP->nullPnt ;
       do
         {
          np = nodeAddrP(dtmP,sp)->tPtr ;
          if( nodeAddrP(dtmP,sp)->hPtr != np ) *lsP = sp ;
          sp = np ;
         }  while ( *lsP == dtmP->nullPnt && sp != *fsP && sp != dtmP->nullPnt ) ;
/*
**     Check For Coincident Tptr Points Between Non Coincident Tptr Lines
*/
       sp = nodeAddrP(dtmP,*lsP)->tPtr ;
       while ( sp != *fsP && *fsP != dtmP->nullPnt )
         {
          if( nodeAddrP(dtmP,sp)->hPtr != dtmP->nullPnt ) { *fsP = *lsP = dtmP->nullPnt ; }
          sp = nodeAddrP(dtmP,sp)->tPtr ;
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
int bcdtmExtEdit_deleteInternalVoidPointsAndLinesAndRetriangulateVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature)
/*
** This Function Deletes Point And Lines Internal To A Void And Retriangulates The Void
**
** Rob Cormack June 2003
*/
{
 int     ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long    lp,pp,spnt,npnt,ppnt,dtmFeature,dtmMsgLevel,numIslands,numFeaturePts ;
 long    p1Dtm,p2Dtm,p3Dtm ;
 DPoint3d     *featurePtsP=NULL ;
 BC_DTM_OBJ *voidDtmP=NULL ;
 DTM_TIN_POINT *pointP ;
 DTM_TIN_POINT_FEATURES *featP,*islandsP=NULL  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning And Retriangulating Void Feature = %6ld",voidFeature) ;
/*
** Initialise
*/
 dtmMsgLevel = DTM_MSG_LEVEL ;
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Feature Topolgy Errors") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Feature Topology OK") ;
   }
/*
** Get List Of Islands Internal To Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Islands Internal To Void") ;
 if( bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(dtmP,voidFeature,&islandsP,&numIslands)) goto errexit ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Internal Islands = %2ld",numIslands) ;
    for( featP = islandsP ; featP < islandsP + numIslands ; ++featP )
      {
       bcdtmWrite_message(0,0,0,"island[%2ld]  ** dtmFeature = %6ld dtmFeatureType = %2ld",(long)(featP-islandsP),featP->dtmFeature,featP->dtmFeatureType) ;
      }
   }
/*
** Delete Internal Void Points And Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines") ;
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
 if( bcdtmExtEdit_deleteInternalTptrVoidPolygonPointsAndLinesDtmObject(dtmP,spnt)) goto errexit ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
** Delete External Island Points And Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting External Island Points And Lines") ;
 for( featP = islandsP ; featP < islandsP + numIslands ; ++featP )
   {
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,featP->dtmFeature,&spnt)) goto errexit ;
    if( bcdtmExtEdit_deleteExternalTptrIslandPolygonPointsAndLinesDtmObject(dtmP,spnt)) goto errexit ;
    if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
   }
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&voidDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidDtmP,1000,1000) ;
/*
** Write Void Hull As A Break Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Void Hull To Data Object") ;
 if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,voidFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,1,1,&voidDtmP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
 if( featurePtsP != NULL ) { free( featurePtsP ) ; featurePtsP = NULL ; }
/*
** Write Island Hulls As Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Island Hulls To Data Object") ;
 for( featP = islandsP ; featP < islandsP + numIslands ; ++featP )
   {
    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,featP->dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,2,1,&voidDtmP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( featurePtsP != NULL ) { free( featurePtsP ) ; featurePtsP = NULL ; }
   }
/*
** Convert z Values to Point Numbers
*/
 for( npnt = 0 ; npnt < voidDtmP->numPoints ; ++npnt )
   {
    pointP = pointAddrP(voidDtmP,npnt) ;
    bcdtmFind_closestPointDtmObject(dtmP,pointP->x,pointP->y,&spnt) ;
    pointP->z = ( double ) spnt ;
   }
/*
** Triangulate The DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Before Triangulation ** voidDtmP->numPoints = %8ld",voidDtmP->numPoints ) ;
 if( dbg ) bcdtmWrite_toFileDtmObject(voidDtmP,L"untriangulatedVoid.dtm") ;
 DTM_MSG_LEVEL = 0 ;
 bcdtmObject_setTriangulationParametersDtmObject(voidDtmP,dtmP->mppTol*10.0,dtmP->mppTol*10.0,1,0.0) ;
 DTM_NORMALISE_OPTION = false ;
 // TODO: DH if( bcdtmExtObject_triangulateDtmObject(voidDtmP)) goto errexit ;
 DTM_MSG_LEVEL = dtmMsgLevel ;
 DTM_NORMALISE_OPTION = true ;
 if( dbg ) bcdtmWrite_message(0,0,0,"After  Triangulation ** voidDtmP->numPoints = %8ld",voidDtmP->numPoints ) ;
/*
** Remove Non Feature Hull Lines
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(voidDtmP)) goto errexit ;
/*
** Convert Break Features Back To Void And Island Features
*/
 for( dtmFeature = 0 ; dtmFeature < voidDtmP->numFeatures ; ++dtmFeature )
   {
    if     ( ftableAddrP(voidDtmP,dtmFeature)->dtmUserTag == 1 ) ftableAddrP(voidDtmP,dtmFeature)->dtmFeatureType = DTMFeatureType::Void ;
    else if( ftableAddrP(voidDtmP,dtmFeature)->dtmUserTag == 2 ) ftableAddrP(voidDtmP,dtmFeature)->dtmFeatureType = DTMFeatureType::Island ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Feature Type = %6ld",ftableAddrP(voidDtmP,dtmFeature)->dtmFeatureType) ;
   }
 if( dbg ) bcdtmWrite_toFileDtmObject(voidDtmP,L"triangulatedVoid.dtm") ;
/*
** Insert Lines Internal To Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines Internal To Void") ;
 for( dtmFeature = 0 ; dtmFeature < voidDtmP->numFeatures ; ++dtmFeature )
   {
    if( ftableAddrP(voidDtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Void )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines Internal To Void Feature %6ld",dtmFeature) ;
       spnt = ftableAddrP(voidDtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&ppnt)) goto errexit ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&npnt)) goto errexit ;
/*
**     Scan Feature Hull
*/
       do
         {
/*
**        Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Anti Clockwise") ;
          lp = voidDtmP->nullPnt ;
          if( ! bcdtmList_testForIslandHullLineDtmObject(voidDtmP,spnt,npnt) )
            {
             pp = npnt ;
             if(( lp = bcdtmList_nextAntDtmObject(voidDtmP,spnt,npnt)) < 0 ) goto errexit ;
             while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(voidDtmP,spnt,lp))
               {
                p1Dtm = (long)pointAddrP(voidDtmP,spnt)->z ;
                p2Dtm = (long)pointAddrP(voidDtmP,lp)->z   ;
                p3Dtm = (long)pointAddrP(voidDtmP,pp)->z   ;
                if( ! bcdtmList_testLineDtmObject(dtmP,p1Dtm,p2Dtm))
                  {
                   if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1Dtm,p2Dtm,p3Dtm)) goto errexit ;
                  }
                pp = lp ;
                if(( lp = bcdtmList_nextAntDtmObject(voidDtmP,spnt,lp)) < 0 ) goto errexit ;
               }
            }
/*
**        Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
          if( lp != ppnt )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Clockwise") ;
             if( ! bcdtmList_testForIslandHullLineDtmObject(voidDtmP,ppnt,spnt) )
               {
                pp = ppnt ;
                if(( lp = bcdtmList_nextClkDtmObject(voidDtmP,spnt,ppnt)) < 0 ) goto errexit ;
                while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(voidDtmP,lp,spnt))
                  {
                   p1Dtm = (long)pointAddrP(voidDtmP,spnt)->z ;
                   p2Dtm = (long)pointAddrP(voidDtmP,lp)->z   ;
                   p3Dtm = (long)pointAddrP(voidDtmP,pp)->z   ;
                   if( ! bcdtmList_testLineDtmObject(dtmP,p1Dtm,p2Dtm))
                     {
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,(long)pointAddrP(voidDtmP,spnt)->z,(long)pointAddrP(voidDtmP,lp)->z,(long)pointAddrP(voidDtmP,pp)->z)) goto errexit ;
                     }
                   pp = lp ;
                   if(( lp = bcdtmList_nextClkDtmObject(voidDtmP,spnt,lp)) < 0 ) goto errexit ;
                  }
               }
            }
/*
**        Reset For Next Feature Point
*/
          ppnt = spnt ;
          spnt = npnt ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&npnt)) goto errexit ;
         } while ( spnt != ftableAddrP(voidDtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
   }
/*
** Insert Lines External To Island
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines External To Islands") ;
 for( dtmFeature = 0 ; dtmFeature < voidDtmP->numFeatures ; ++dtmFeature )
   {
    if( ftableAddrP(voidDtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Island )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines External To Island Feature %6ld",dtmFeature) ;
       spnt = ftableAddrP(voidDtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&ppnt)) goto errexit ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&npnt)) goto errexit ;
/*
**     Scan Feature Hull
*/
       lp = dtmP->nullPnt ;
       do
         {
          if( dbg )
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          if( ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(voidDtmP,npnt,spnt) && ! bcdtmList_testForIslandHullLineDtmObject(voidDtmP,npnt,spnt))
            {
             pp = npnt ;
             if(( lp = bcdtmList_nextClkDtmObject(voidDtmP,spnt,npnt)) < 0 ) goto errexit ;
             while ( lp != ppnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(voidDtmP,lp,spnt))
               {
                if( ! bcdtmList_testLineDtmObject(dtmP,(long)pointAddrP(voidDtmP,spnt)->z,(long)pointAddrP(voidDtmP,lp)->z) )
                  {
                   if( bcdtmList_insertLineAfterPointDtmObject(dtmP,(long)pointAddrP(voidDtmP,spnt)->z,(long)pointAddrP(voidDtmP,lp)->z,(long)pointAddrP(voidDtmP,pp)->z )) goto errexit ;
                  }
                pp = lp ;
                if(( lp = bcdtmList_nextClkDtmObject(voidDtmP,spnt,lp)) < 0 ) goto errexit ;
               }
            }
/*
**       Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
          if( lp != ppnt )
            {
             if( ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(voidDtmP,spnt,ppnt) )
               {
                pp = ppnt ;
                if(( lp = bcdtmList_nextAntDtmObject(voidDtmP,spnt,ppnt)) < 0 ) goto errexit ;
                while ( lp != npnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(voidDtmP,spnt,lp))
                  {
                   if( ! bcdtmList_testLineDtmObject(dtmP,(long)pointAddrP(voidDtmP,spnt)->z,(long)pointAddrP(voidDtmP,lp)->z) )
                     {
                      if( bcdtmList_insertLineBeforePointDtmObject(dtmP,(long)pointAddrP(voidDtmP,spnt)->z,(long)pointAddrP(voidDtmP,lp)->z,(long)pointAddrP(voidDtmP,pp)->z)) goto errexit ;
                     }
                   pp = lp ;
                   if(( lp = bcdtmList_nextAntDtmObject(voidDtmP,spnt,lp)) < 0 ) goto errexit ;
                  }
               }
            }
/*
**        Reset For Next Point On Tptr Polygon
*/
          ppnt = spnt ;
          spnt = npnt ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(voidDtmP,dtmFeature,spnt,&npnt)) goto errexit ;
         } while ( spnt != ftableAddrP(voidDtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
   }
/*
** Check Tin Topology And Feature Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Feature Topolgy Errors") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Feature Topology OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 DTM_MSG_LEVEL = dtmMsgLevel ;
 DTM_NORMALISE_OPTION = true ;
 if( voidDtmP    != NULL ) bcdtmObject_destroyDtmObject(&voidDtmP) ;
 if( islandsP    != NULL ) { free(islandsP)    ; islandsP = NULL    ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines And Retriangulating Void Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines And Retriangulating Void Error") ;
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
int bcdtmExtEdit_deleteInternalTptrVoidPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt)
/*
** This Function Deletes Internal Voids Points And Lines
**
** Rob Cormack June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   lp,clc,pnt,npnt,ppnt,fPnt,lPnt,lpnt;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines") ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 fPnt = lPnt = dtmP->nullPnt ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
      }
/*
** Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
               }
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
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
** Scan Internal Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
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
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Write List Of Marked Points
*/
 if( dbg && fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marked Points") ;
    do
      {
       lpnt = pnt ;
       bcdtmWrite_message(0,0,0,"Marked Point = %6ld ** %10.4lf %10.4lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
       pnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
      } while ( pnt != lpnt ) ;
   }
/*
** Scan Marked Points And Delete All Connecting Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Marked Points And Deleting All Connecting Lines") ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
/*
**     Scan Marked Point And Delete Lines
*/
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lp)) goto errexit ;
         }
       nodeAddrP(dtmP,pnt)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,pnt)->fPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,pnt)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
/*
**     Get Next Marked Point
*/
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Delete Lines Connected To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Lines Connected To Tptr Polygon") ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,npnt) )
      {
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt) )
         {
          lpnt = lp ;
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lpnt)) goto errexit ;
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,ppnt,pnt) )
         {
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt) && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) )
            {
             lpnt = lp ;
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Deleted Line %6ld %6ld",pnt,lpnt) ;
             if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lpnt)) goto errexit ;
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
** Mark And Null Out Internal Tptr List
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
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Points And Lines Error") ;
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
int bcdtmExtEdit_deleteExternalTptrIslandPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt)
/*
** This Function Deletes External Islands Points And Lines
**
** Rob Cormack June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   lp,clc,pnt,npnt,ppnt,fPnt,lPnt,lpnt;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting External Island Points And Lines") ;
/*
** Mark Points Immediately External To Tptr Polygon
*/
 fPnt = lPnt = dtmP->nullPnt ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,npnt,pnt) )
      {
       if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,lp,pnt) && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
            }
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
      }
/*
**  Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,pnt,ppnt) && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**       Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,pnt,lp))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                        { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
               }
             if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
            }
         }
      }
/*
**  Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;
    pnt  = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To External Marked Points") ;
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
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Write List Of Marked Points
*/
 if( dbg && fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marked Points") ;
    do
      {
       lpnt = pnt ;
       bcdtmWrite_message(0,0,0,"Marked Point = %6ld ** %10.4lf %10.4lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
       pnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
      } while ( pnt != lpnt ) ;
   }
/*
** Scan Marked Points And Delete All Connecting Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Marked Points And Deleting All Connecting Lines") ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
/*
**     Scan Marked Point And Delete Lines
*/
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lp)) goto errexit ;
         }
       nodeAddrP(dtmP,pnt)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,pnt)->fPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,pnt)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
/*
**     Get Next Marked Point
*/
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Delete Lines Connected To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting External Lines Connected To Tptr Polygon") ;
 ppnt = sPnt ;
 pnt  = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,npnt))
      {
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Clockwise") ;
       if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
       while ( lp != ppnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,lp,pnt) && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,pnt,lp))
         {
          lpnt = lp ;
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Clk Deleted Line %6ld %6ld",pnt,lp) ;
          if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lpnt)) goto errexit ;
         }
      }
/*
**  Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,ppnt,pnt))
         {
/*
**        Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Anti Clockwise") ;
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          while ( lp != npnt && ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,pnt,lp)&& ! bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(dtmP,lp,pnt))
            {
             lpnt = lp ;
             if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Ant Deleted Line %6ld %6ld",pnt,lpnt) ;
             if( bcdtmList_deleteLineDtmObject(dtmP,pnt,lpnt)) goto errexit ;
            }
         }
      }
/*
**  Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;
    pnt  = npnt ;
   } while ( ppnt != sPnt ) ;
/*
** Mark And Null Out Internal Tptr List
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
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting External Island Points And Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting External Island Points And Lines Error") ;
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
int bcdtmExtEdit_moveVertexZDtmObject(BC_DTM_OBJ *dtmP,long point,double z)
/*
** This Function Moves The z Value For A Tin Point
*/
{
/*
** Move z Value
*/
 if( point == editPnt1 )
   {
    pointAddrP(dtmP,point)->z = z ;
    editPnt1 = DTM_NULL_PNT ;
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
int bcdtmExtEdit_checkPointXYCanBeMovedDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       point,
 double     x,
 double     y,
 long       &moveFlag
)
/*
** This Function Checks If point XY can be moved to x & y
** If so point XY is set to x & y
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   p1,p2,clc,nextPnt,priorPnt ;
 double sx,sy,px,py,nx,ny,p1x,p1y,p2x,p2y ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Point Can Be Moved") ;
    bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point     = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"x         = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y         = %12.5lf",y) ;
   }
/*
** Initialise Variables
*/
 moveFlag = 0 ;
 sx = pointAddrP(dtmP,point)->x ;
 sy = pointAddrP(dtmP,point)->y ;
/*
** Check Point Range
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Scan Point To See If It Can Be Moved
*/
 if( ( clc = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
   {
    moveFlag = 1 ;
    sx = pointAddrP(dtmP,point)->x ;
    sy = pointAddrP(dtmP,point)->y ;
    pointAddrP(dtmP,point)->x = x ;
    pointAddrP(dtmP,point)->y = y ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while( clc != dtmP->nullPtr && moveFlag )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testLineDtmObject(dtmP,p1,p2))
         {
          if( bcdtmMath_pointSideOfDtmObject(dtmP,point,p2,p1) < 1 ) moveFlag = 0 ;
         }
       p1 = p2 ;
      }
   }
/*
** If External Point Check Move Doesnt Intersect Tin Hull
*/
 if( moveFlag && ( nextPnt = nodeAddrP(dtmP,point)->hPtr ) != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Moving Hull Point") ;
    if( ( priorPnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPnt)) < 0 ) goto errexit ;
    nx = pointAddrP(dtmP,nextPnt)->x ;
    ny = pointAddrP(dtmP,nextPnt)->y ;
    px = pointAddrP(dtmP,priorPnt)->x ;
    py = pointAddrP(dtmP,priorPnt)->y ;
    p1 = nodeAddrP(dtmP,nextPnt)->hPtr ;
    p1x = pointAddrP(dtmP,p1)->x ;
    p1y = pointAddrP(dtmP,p1)->y ;
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    while( p2 != priorPnt && moveFlag )
      {
       p2x = pointAddrP(dtmP,p2)->x ;
       p2y = pointAddrP(dtmP,p2)->y ;
       if     ( bcdtmMath_checkIfLinesIntersect(x,y,nx,ny,p1x,p1y,p2x,p2y))
         {
          moveFlag = 0 ;
         }
       else if( bcdtmMath_checkIfLinesIntersect(x,y,px,py,p1x,p1y,p2x,p2y))
         {
          moveFlag = 0 ;
         }
       p1x = p2x ;
       p1y = p2y ;
       p1  = p2  ;
       p2  = nodeAddrP(dtmP,p1)->hPtr ;
      }
   }
/*
** Log Move Status
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"moveFlag = %2ld",moveFlag) ;
   }
/*
** Clean Up
*/
 cleanup :
 pointAddrP(dtmP,point)->x = sx ;
 pointAddrP(dtmP,point)->y = sy ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Moved Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Moved Error") ;
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
int bcdtmExtEdit_tempMoveVertexXYZDtmObject(BC_DTM_OBJ *dtmP,long Point,double x,double y,double z)
/*
** This Function Moves The Coordinates For A Point
*/
{
/*
** Check For Valid Point
*/
 if( Point >= 0 && Point < dtmP->numPoints )
   {
    pointAddrP(dtmP,Point)->x = x ;
    pointAddrP(dtmP,Point)->y = y ;
    pointAddrP(dtmP,Point)->z = z ;
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
int bcdtmExtEdit_moveVertexXYZDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       ResetFlag,
 long       Point,
 double     x,
 double     y,
 double     z
)
/*
** This Function Moves The Coordinates For A Point
** It Creates A New Point To Maintain The Point Sort Order
*/
{
 int   ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long  np,pp,hpp,clc,clp,dtmFeature,firstPoint ;
 long  nPnt,lPnt,moveFlag ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Moving Point XYZ") ;
    bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP ) ;
    bcdtmWrite_message(0,0,0,"ResetFlag = %8ld",ResetFlag) ;
    bcdtmWrite_message(0,0,0,"Point     = %8ld",Point ) ;
    bcdtmWrite_message(0,0,0,"x         = %12.5lf",x ) ;
    bcdtmWrite_message(0,0,0,"y         = %12.5lf",y ) ;
    bcdtmWrite_message(0,0,0,"z         = %12.5lf",z ) ;
   }
/*
** Check DTM Prior To Point Move
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Prior To Move") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"DTM Invalid Prior To Point Move") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"DTM Valid Prior To Point Move") ;
   }
/*
** Check For Valid Point
*/
 if( Point < 0 || Point > dtmP->numPoints ) goto errexit ;
/*
** Check Point Can Be Moved
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Point Can Be Moved") ;
    if( bcdtmExtEdit_checkPointXYCanBeMovedDtmObject (dtmP,Point,x,y,moveFlag)) goto errexit ;
    bcdtmWrite_message(0,0,0,"moveFlag = %2ld",moveFlag) ;
   }
/*
** Log Point Coordinates
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld dtmP->numPoints = %8ld",dtmP->numSortedPoints,dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Move Point[%ld] = %12.5lf %12.5lf %12.5lf",Point,pointAddrP(dtmP,Point)->x,pointAddrP(dtmP,Point)->y,pointAddrP(dtmP,Point)->z) ;
   }
/*
** Test For Reset Of Initial Coordinates
*/
 if( ResetFlag )
   {
    pointAddrP(dtmP,Point)->x = x ;
    pointAddrP(dtmP,Point)->y = y ;
    pointAddrP(dtmP,Point)->z = z ;
    goto cleanup ;
   }
/*
** Log Features For Point
*/
 if( dbg )
   {
    clc = nodeAddrP(dtmP,Point)->fPtr ;
    while( clc != dtmP->nullPtr )
      {
       bcdtmWrite_message(0,0,0,"Point %8ld Is Part Of Feature %8ld ** Next Point = %10ld",Point,flistAddrP(dtmP,clc)->dtmFeature,flistAddrP(dtmP,clc)->nextPnt) ;
       clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Add Point
*/
 if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&np)) goto errexit ;
/*
** If Hull Point Get Previous Point
*/
 hpp = dtmP->nullPnt ;
 if( nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt )
   {
    if( ( hpp = bcdtmList_nextClkDtmObject(dtmP,Point,nodeAddrP(dtmP,Point)->hPtr)) < 0 ) goto errexit ;
   }
/*
** Set Prior Pointers For Point Dtm Features To New Point
*/
 if( ( clc = nodeAddrP(dtmP,Point)->fPtr ) != dtmP->nullPtr )
   {
    while( clc != dtmP->nullPtr )
      {
       dtmFeature = flistAddrP(dtmP,clc)->dtmFeature ;

//     Copy Feature To Rptr List

       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&firstPoint)) goto errexit ;
       if( Point == firstPoint )
         {
          ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = np ;
         }

//     Scan Tptr List And And Set Prior Point To New Point

       lPnt = firstPoint ;
       nPnt = nodeAddrP(dtmP,firstPoint)->tPtr ;
       while( nPnt != dtmP->nullPnt )
         {
          if( nPnt == Point )
            {
             clp = nodeAddrP(dtmP,lPnt)->fPtr  ;
             while ( clp != dtmP->nullPtr )
               {
                if( flistAddrP(dtmP,clp)->nextPnt == Point )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** priorPoint = %8ld  nextPoint = %8ld",dtmFeature,lPnt,flistAddrP(dtmP,clp)->nextPnt) ;
                   flistAddrP(dtmP,clp)->nextPnt = np ;
                 }
                clp = flistAddrP(dtmP,clp)->nextPtr ;
               }
               break;
            }
          else if (nPnt == firstPoint)
              break;
          else
            {
             lPnt = nPnt ;
             nPnt = nodeAddrP(dtmP,lPnt)->tPtr ;
            }
         }

//     Get Next Feature For Point

       clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Copy Next Pointer For DtmFeatures To New Point
*/
 if( nodeAddrP(dtmP,Point)->fPtr != dtmP->nullPtr )
   {
    nodeAddrP(dtmP,np)->fPtr = nodeAddrP(dtmP,Point)->fPtr ;
    nodeAddrP(dtmP,Point)->fPtr = dtmP->nullPtr  ;
   }
/*
** Delete Circular List For Point And Insert For Np
*/
 clc = nodeAddrP(dtmP,Point)->cPtr ;
 while ( clc != dtmP->nullPtr )
   {
    pp  = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    clp = nodeAddrP(dtmP,pp)->cPtr  ;
    while ( clp != dtmP->nullPtr )
      {
       if(clistAddrP(dtmP,clp)->pntNum == Point ) { clistAddrP(dtmP,clp)->pntNum = np ; clp = dtmP->nullPtr ; }
       else clp = clistAddrP(dtmP,clp)->nextPtr ;
      }
   }
 nodeAddrP(dtmP,np)->cPtr = nodeAddrP(dtmP,Point)->cPtr ;
 nodeAddrP(dtmP,np)->hPtr = nodeAddrP(dtmP,Point)->hPtr ;
 if( hpp != dtmP->nullPnt ) nodeAddrP(dtmP,hpp)->hPtr = np ;
 nodeAddrP(dtmP,Point)->cPtr = dtmP->nullPtr ;
 nodeAddrP(dtmP,Point)->hPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,Point)->tPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,Point)->fPtr = dtmP->nullPtr ;
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Check DTM After Point Move
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM After Move") ;
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld dtmP->numPoints = %8ld",dtmP->numSortedPoints,dtmP->numPoints) ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"DTM Invalid After Point Move") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"DTM Valid After Point Move") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Point XYZ Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Point XYZ Error") ;
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
int  bcdtmExtEdit_checkPointCanBeDeletedDtmObject(BC_DTM_OBJ *dtmP,long Point,long UpdateFlag,long *Flag)
/*
** This function tests if A point can be deleted
**
** Return Value == 0 Success
**              == 1 Error
**        Flag  == 0 Point Cannot Be Deleted
**              == 1 Delete Point Retriangulate
**              == 2 Delete Point On Tin Hull
**              == 3 Delete Point Create Void
**              == 4 Delete Point On Void Hull
**              == 5 Delete Point On Island Hull
**
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long pp,np,pl,lp,sp,clc,feat,VoidFeature,IslandFeature,Feature ;
 long HullFlag=0,BrkFlag=0,ConFlag=0,VoidFlag=0,IslandFlag=0 ;
 long VoidLine1,VoidLine2,Direction ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Deleted") ;
/*
** Initialise Variables
*/
 *Flag = 0 ;
/*
** Test If Point Is On Tin Hull
*/
 if( nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt ) HullFlag = 1 ;
/*
** Determine Features Associated With Point
*/
 clc = nodeAddrP(dtmP,Point)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    feat = flistAddrP(dtmP,clc)->dtmFeature ;
    clc  = flistAddrP(dtmP,clc)->nextPtr ;
    if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Breakline   ) ++BrkFlag    ;
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::ContourLine ) ++ConFlag    ;
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Void         ) ++VoidFlag   ;
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Island       ) ++IslandFlag ;
      }
   }
/*
** Determine If Point Last Point On Break Or Contour Line
*/
  for( feat = 0 ; feat < dtmP->numFeatures ; ++feat )
    {
     if( ( sp = ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt && (ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Breakline || ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::ContourLine ))
       {
        np = lp = sp ;
        clc = nodeAddrP(dtmP,np)->fPtr ;
        while ( clc != dtmP->nullPtr )
          {
           while ( clc != dtmP->nullPtr  && flistAddrP(dtmP,clc)->dtmFeature != feat ) clc = flistAddrP(dtmP,clc)->nextPtr ;
           if( clc != dtmP->nullPtr )
             {
              lp = np ;
              np = flistAddrP(dtmP,clc)->nextPnt ;
              if( np != dtmP->nullPnt ) clc = nodeAddrP(dtmP,np)->fPtr ;
              if( np == dtmP->nullPnt || np == sp ) clc = dtmP->nullPtr  ;
             }
          }
/*
** Test If Last Point Is Equal To Delete Point
*/
        if( lp == Point )
          {
           if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Breakline   ) ++BrkFlag ;
           if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::ContourLine ) ++ConFlag ;
          }
       }
    }
/*
** Test For Simple Retriangulate
*/
  if( ! HullFlag && ! VoidFlag && ! IslandFlag && UpdateFlag == 1 ) { *Flag = 1 ; goto cleanup ; }
/*
**  Test For Point On No Features
*/
  if( ! HullFlag && /* ! BrkFlag && ! ConFlag && */ ! VoidFlag && ! IslandFlag )
    {
     if( UpdateFlag == 1 ) *Flag = 1 ;
     else                  *Flag = 3 ;
     if( *Flag == 3 )
       {
/*
**      Check Void Wont Have Coincident Island Points
*/
        clc = nodeAddrP(dtmP,Point)->cPtr ;
        while ( clc != dtmP->nullPtr && *Flag == 3 )
          {
           lp  = clistAddrP(dtmP,clc)->pntNum ;
           clc = clistAddrP(dtmP,clc)->nextPtr ;
           if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,lp,&Feature)) goto errexit ;
           if( Feature != dtmP->nullPnt ) *Flag = 0 ;
          }
/*
**      Check Void Wont Have Coincide Void Points
*/
        if( *Flag == 3 )
          {
           clc = nodeAddrP(dtmP,Point)->cPtr ;
           while ( clc != dtmP->nullPtr && *Flag == 3 )
             {
              lp  = clistAddrP(dtmP,clc)->pntNum ;
              clc = clistAddrP(dtmP,clc)->nextPtr ;
              if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,lp,&Feature)) goto errexit ;
              if( Feature != dtmP->nullPnt )
                {
                 if(( pp = bcdtmList_nextAntDtmObject(dtmP,Point,lp))   < 0 ) goto errexit ;
                 if(( np = bcdtmList_nextClkDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
                 bcdtmExtEdit_testForDtmFeatureHullLineDtmObject(dtmP,DTMFeatureType::Void,lp,pp,&VoidLine1,&Feature,&Direction) ;
                 bcdtmExtEdit_testForDtmFeatureHullLineDtmObject(dtmP,DTMFeatureType::Void,lp,np,&VoidLine2,&Feature,&Direction) ;
                 if( ! VoidLine1 && ! VoidLine2 ) *Flag = 0 ;
                }
             }
          }
       }
     goto cleanup ;
    }
/*
** Test If Hull Point Can Be Deleted
*/
 if( HullFlag && /* ! BrkFlag && ! ConFlag && */ ! VoidFlag && ! IslandFlag   )
   {
    np = nodeAddrP(dtmP,Point)->hPtr ;
        if( ( pp = bcdtmList_nextClkDtmObject(dtmP,Point,np)) < 0 ) goto errexit ;
        if( ( pl = bcdtmList_nextAntDtmObject(dtmP,Point,np)) < 0   ) goto errexit ;
        while ( pl != pp )
          {
           if( nodeAddrP(dtmP,pl)->hPtr != dtmP->nullPnt )  { *Flag = 0 ; goto cleanup ; }
           if( ( pl = bcdtmList_nextAntDtmObject(dtmP,Point,pl)) < 0   ) goto errexit ;
      }
    *Flag = 2 ;
    goto cleanup ;
   }
/*
** Test If Point On Void Hull can be Deleted
*/
 if( ! HullFlag && /* ! BrkFlag && ! ConFlag && */ VoidFlag && ! IslandFlag && VoidFlag == 1  )
   {
    *Flag = 4 ;
    VoidFeature = dtmP->nullPnt ;
    clc = nodeAddrP(dtmP,Point)->fPtr ;
    while ( clc != dtmP->nullPtr && VoidFeature == dtmP->nullPnt )
      {
       if(ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) VoidFeature = flistAddrP(dtmP,clc)->dtmFeature  ;
       else clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
    if( VoidFeature == dtmP->nullPnt ) goto errexit ;
    bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,VoidFeature,Point,&pp) ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,VoidFeature,Point,&np) ;
    if((lp = bcdtmList_nextClkDtmObject(dtmP,Point,np)) < 0 ) goto errexit ;
    while ( lp != pp )
      {
       if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,lp,&Feature)) goto errexit ;
       if( Feature != dtmP->nullPnt )
         {
          if(( pl = bcdtmList_nextAntDtmObject(dtmP,Point,lp))   < 0 ) goto errexit ;
          if(( sp = bcdtmList_nextClkDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
          bcdtmExtEdit_testForDtmFeatureHullLineDtmObject(dtmP,DTMFeatureType::Void,lp,pl,&VoidLine1,&Feature,&Direction) ;
          bcdtmExtEdit_testForDtmFeatureHullLineDtmObject(dtmP,DTMFeatureType::Void,lp,sp,&VoidLine2,&Feature,&Direction) ;
          if( ! VoidLine1 && ! VoidLine2 ) *Flag = 0 ;
         }
       if((lp = bcdtmList_nextClkDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
      }
   }
/*
** Test If Point On Island Hull can be Deleted
*/
 if( ! HullFlag && /*! BrkFlag && ! ConFlag && */ ! VoidFlag &&  IslandFlag && IslandFlag == 1  )
   {
    IslandFeature = dtmP->nullPnt ;
    clc = nodeAddrP(dtmP,Point)->fPtr ;
    while ( clc != dtmP->nullPtr && IslandFeature == dtmP->nullPnt )
      {
       if(ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) IslandFeature = flistAddrP(dtmP,clc)->dtmFeature  ;
       else clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
    if( IslandFeature == dtmP->nullPnt ) goto errexit ;
    bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,IslandFeature,Point,&pp) ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,IslandFeature,Point,&np) ;
    if((lp = bcdtmList_nextAntDtmObject(dtmP,Point,np)) < 0 ) goto errexit ;
    *Flag = 5 ;
    if( lp != pp )
      {
       while ( lp != pp && *Flag == 5 )
         {
          if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,lp,&Feature)) goto errexit ;
          if( Feature != dtmP->nullPnt ) *Flag = 0 ;
          if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,lp,&Feature)) goto errexit ;
          if( Feature != dtmP->nullPnt ) *Flag = 0 ;
          if((lp = bcdtmList_nextAntDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Deleted Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Deleted Error") ;
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
int bcdtmExtEdit_insertPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   pntType,
 long   dtmFeature,
 long   updateOption,                  // updateOption == 0 for dynamic mode otherwise 1
 long   P1,
 long   P2,
 long   P3,
 double x,
 double y,
 double z,
 long   *newPntP
)
/*
**
** This Function Inserts A Point Into A DTM Object
**
** The following set of point codes are relevent to the SS3 TM Editor Implementation
**
** pntType Codes  =  1  Coincident With Current Tin Point P1
**                =  2  Colinear With Internal Line P1-P2
**                =  3  Colinear With Hull Line Line P1-P2
**                =  4  Internal To Triangle P1-P2-P3
**                =  5  Internal To Void Hull At Void Hull Points P1-P2-P3
**                =  6  External To Tin Hull At Tin Hull Points P1-P2-P3
**                =  7  External To Island Hull At Island Hull Points P1-P2-P3
**
** Note. In dynamic mode no precision checking and or fixing is performed.
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long n,pp,ap,cp,pnt,spnt,*islandsP=NULL,numIslands=0,voidFeature=0,islandFeature=0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Log Method Parameters
*/
 if( dbg || cdbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Point Into Dtm") ;
    bcdtmWrite_message(0,0,0,"Dtm Object    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pntType       = %8ld",pntType) ;
    bcdtmWrite_message(0,0,0,"dtmFeature    = %8ld",dtmFeature) ;
    bcdtmWrite_message(0,0,0,"updateOption  = %8ld",updateOption) ;
    bcdtmWrite_message(0,0,0,"P1            = %8ld",P1) ;
    bcdtmWrite_message(0,0,0,"P2            = %8ld",P2) ;
    bcdtmWrite_message(0,0,0,"P3            = %8ld",P3) ;
    bcdtmWrite_message(0,0,0,"x             = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y             = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"z             = %12.5lf",z) ;
   }
/*
**  Check Edit
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Before Inserting Point")  ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"DTM Invalid Before Inserting Point") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"DTM Valid Before Inserting Point") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"beforePointInsert.tin") ;
   }
/*
** Set Global Insert Point Type
*/
 if( pntType < 1 || pntType > 7 )
   {
    bcdtmWrite_message(2,0,0,"Invalid Insert Point Type") ;
    goto errexit ;
   }
/*
** Insert Point Into Internal Triangle
*/
 if( pntType <  5 && updateOption == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Point") ;
    if( bcdtmInsert_storePointInDtmObject(dtmP,2,1,x,y,z,newPntP)) goto errexit ;
    if( dbg ) bcdtmList_writeCircularListForPointDtmObject(dtmP,*newPntP) ;
   }
/*
** Insert Point Into Internal Triangle
*/
 if( pntType <  5 && updateOption == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Dynamic Mode Inserting Internal Point") ;
    switch ( pntType )
      {
       case 1 :                   // Coincident Point
         *newPntP = P1 ;
       break  ;

       case 2 :                  //  Insert Into Internal Line
         if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP)) goto errexit ;
         if( ( ap = bcdtmList_nextAntDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
         if( ( cp = bcdtmList_nextClkDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
         if( bcdtmList_deleteLineDtmObject(dtmP,P1,P2)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,*newPntP,ap)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P2,*newPntP,cp)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,*newPntP,P2)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,ap,P1)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,*newPntP,P1)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,cp,P2)) goto errexit ;
         if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P1,P2) )
           {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,P1,P2,*newPntP)) goto errexit ;
           }
       break ;

       case  3 :                   // Insert Into External Line
         if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP)) goto errexit ;
             if( (ap = bcdtmList_nextAntDtmObject(dtmP,P1,P2))   < 0 ) goto errexit ;
             if(bcdtmList_deleteLineDtmObject(dtmP,P1,P2)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,*newPntP,ap)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
             if(bcdtmList_insertLineBeforePointDtmObject(dtmP,P2,*newPntP,ap)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,*newPntP,P2)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,ap,P1)) goto errexit ;
         if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P1,P2) )
           {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,P1,P2,*newPntP)) goto errexit ;
           }
             nodeAddrP(dtmP,P1)->hPtr = *newPntP ;
             nodeAddrP(dtmP,*newPntP)->hPtr = P2 ;
       break ;

       case  4 :                  // Insert Into Triangle
         if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,*newPntP,P2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P2,*newPntP,P3)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,P3,*newPntP,P1)) goto errexit ;
             if(bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P3,P2)) goto errexit ;
       break ;

           default :
             bcdtmWrite_message(2,0,0,"Illegal Point Type Code %6ld ",pntType) ;
             goto errexit ;
       break   ;
      } ;
     if( dbg ) bcdtmList_writeCircularListForPointDtmObject(dtmP,*newPntP) ;
   }
/*
** Insert Point External To Tin Hull
*/
 if( pntType == 6 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point External To Tin Hull") ;
/*
** Insert Point Into Tin
*/
    if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP)) goto errexit  ;
/*
** Insert Lines To External Point
*/
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,*newPntP,P2)) goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
    if( bcdtmList_insertLineBeforePointDtmObject(dtmP,P2,*newPntP,P1)) goto errexit ;
    if( P3 != dtmP->nullPnt )
      {
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P3,P2)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,P3,*newPntP,P2)) goto errexit ;
      }
/*
** Reset Hull Pointers
*/
    bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,P1,&pp) ;
    nodeAddrP(dtmP,P1)->hPtr = *newPntP ;
    if( P3 != dtmP->nullPnt )
      {
       if( dtmP->hullPoint == P2 ) dtmP->hullPoint = P1 ;
       nodeAddrP(dtmP,P2)->hPtr  = dtmP->nullPnt ;
       nodeAddrP(dtmP,*newPntP)->hPtr = P3 ;
      }
    else nodeAddrP(dtmP,*newPntP)->hPtr = P2 ;
   }
/*
** Insert Point Internal To Void Hull
*/
 if( pntType == 5 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point Internal To Void") ;
/*
**  Set Void Feature
*/
    voidFeature = dtmFeature ;
/*
**  Test For Update Flag
*/
    if( updateOption )
      {
/*
**     Insert Point Into Tin
*/
       if( bcdtmInsert_storePointInDtmObject(dtmP,1,1,x,y,z,newPntP)) goto errexit ;
       if( bcdtmList_copyDtmFeatureToSptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
/*
**     Insert New Void HuLL Lines Into Tin
*/
       if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,P1,*newPntP,1,1)) goto errexit ;
       if( P3 != dtmP->nullPnt )
         {
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,*newPntP,P3,1,1)) goto errexit ;
         }
       else
         {
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,*newPntP,P2,1,1)) goto errexit ;
         }
/*
**     Log New Hull Section
*/
       if( dbg )
         {
          pnt = P1 ;
          while( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt )
            {
             bcdtmWrite_message(0,0,0,"pnt = %8ld tPtr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
             pnt = nodeAddrP(dtmP,pnt)->tPtr ;
            }
         }
/*
**     Modify Tptr List To Include Modified Void Hull
*/
       if( P3 != dtmP->nullPnt )
          pnt = P3 ;
       else
          pnt = P2 ;
       while( pnt != P1 )
         {
          nodeAddrP(dtmP,pnt)->tPtr = nodeAddrP(dtmP,pnt)->sPtr ;
          pnt = nodeAddrP(dtmP,pnt)->sPtr ;
         }
/*
**    Null sPtr List And Check Connectivity Of Tptr
*/
      if( bcdtmList_nullSptrListDtmObject(dtmP,spnt)) goto errexit ;
      if( cdbg )
        {
         if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,P1,0))
           {
            bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr List") ;
            goto errexit ;
           }
        }
/*
**    Modified Feature And Remove Existing Feature
*/
      dtmFeatureP = ftableAddrP(dtmP,voidFeature) ;
      if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,P1,1)) goto errexit ;
      if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,voidFeature)) goto errexit ;
      }
/*
**  Dynamic Mode - Corrupt Tin For Purpose Of Speed And Display
*/
    else
      {
/*
**     Add Point Into Tin
*/
       bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP) ;
/*
**     Insert Lines Point
*/
       if( cdbg ) bcdtmWrite_message(0,0,0,"Inserting Lines About Point") ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,P1,*newPntP,P2)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P2,*newPntP,P1)) goto errexit ;
       if( P3 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineBeforePointDtmObject(dtmP,*newPntP,P3,P2)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P3,*newPntP,P2)) goto errexit ;
         }
/*
**     Reset Void Pointers
*/
       if( cdbg ) bcdtmWrite_message(0,0,0,"Resetting Void Pointers") ;
       if( P3 != dtmP->nullPnt )
         {
          if(ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint == P2 ) ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint = P1 ;
          if( bcdtmExtInsert_removePointFromDtmFeatureDtmObject(dtmP,P2,voidFeature)) goto errexit ;
          if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,voidFeature,P1,P3,*newPntP)) goto errexit ;
         }
       else  if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,voidFeature,P1,P2,*newPntP)) goto errexit ;
      }
   }
/*
** Insert Point External To Island Hull
*/
 if( pntType == 7 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point External To Island") ;
/*
**  Set Void Feature
*/
    islandFeature = dtmFeature ;
/*
**  Test For Update Flag
*/
    if( updateOption )
      {
/*
**     Insert Point Into Tin
*/
       if( bcdtmInsert_storePointInDtmObject(dtmP,1,1,x,y,z,newPntP)) goto errexit ;
       if( bcdtmList_copyDtmFeatureToSptrListDtmObject(dtmP,islandFeature,&spnt)) goto errexit ;
/*
**     Insert New Void HuLL Lines Into Tin
*/
       if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,P1,*newPntP,1,1)) goto errexit ;
       if( P3 != dtmP->nullPnt )
         {
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,*newPntP,P3,1,1)) goto errexit ;
         }
       else
         {
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,*newPntP,P2,1,1)) goto errexit ;
         }
/*
**     Log New Hull Section
*/
       if( dbg )
         {
          pnt = P1 ;
          while( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt )
            {
             bcdtmWrite_message(0,0,0,"pnt = %8ld tPtr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
             pnt = nodeAddrP(dtmP,pnt)->tPtr ;
            }
         }
/*
**     Modify Tptr List To Include Modified Void Hull
*/
       if( P3 != dtmP->nullPnt )
          pnt = P3 ;
       else
          pnt = P2 ;
       while( pnt != P1 )
         {
          nodeAddrP(dtmP,pnt)->tPtr = nodeAddrP(dtmP,pnt)->sPtr ;
          pnt = nodeAddrP(dtmP,pnt)->sPtr ;
         }
/*
**    Null sPtr List And Check Connectivity Of Tptr
*/
      if( bcdtmList_nullSptrListDtmObject(dtmP,spnt)) goto errexit ;
      if( cdbg )
        {
         if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,P1,0))
           {
            bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr List") ;
            goto errexit ;
           }
        }
/*
**    Modified Feature And Remove Existing Feature
*/
      dtmFeatureP = ftableAddrP(dtmP,islandFeature) ;
      if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,P1,1)) goto errexit ;
      if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,islandFeature)) goto errexit ;
      }
/*
**  Dynamic Mode - Corrupt Tin For Purpose Of Speed And Display
*/
    else
      {
/*
**     Insert Point Into Tin
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point To Tin") ;
       bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP) ;
/*
**     Insert Lines Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point Lines") ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P1,dtmP->nullPnt)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,*newPntP,P2)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P2,P1)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,P2,*newPntP,P1)) goto errexit ;
       if( P3 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*newPntP,P3,P2)) goto errexit ;
          if( bcdtmList_insertLineBeforePointDtmObject(dtmP,P3,*newPntP,P2)) goto errexit ;
         }
/*
**     Reset Island Pointers
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Island Pointers") ;
       if( P3 != dtmP->nullPnt )
         {
          if(ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint == P2 ) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = P1 ;
          if( bcdtmExtInsert_removePointFromDtmFeatureDtmObject(dtmP,P2,dtmFeature)) goto errexit ;
          if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,dtmFeature,P1,P3,*newPntP)) goto errexit ;
         }
       else  if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,dtmFeature,P1,P2,*newPntP)) goto errexit ;
      }
   }
/*
** Insert Point And Remove Void
*/
 if( pntType == 8 )             // Not Implemented in SS3 TM Editor
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point And Removing Void") ;
/*
**  Get Void feature External To Island
*/
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) bcdtmExtEdit_getVoidExternalToIslandDtmObject(dtmP,dtmFeature,&voidFeature) ;
    else                                                              voidFeature = dtmFeature ;
/*
**  Get Island dtmFeatures Internal To Void
*/
    if( bcdtmExtEdit_getIslandsInternalToVoidDtmObject(dtmP,voidFeature,&islandsP,&numIslands)) goto errexit ;
/*
**  Remove Internal Void Points And Lines
*/
    if( bcdtmExtEdit_removeInternalVoidPointsAndLinesDtmObject(dtmP,voidFeature,islandsP,numIslands))  goto errexit ;
/*
**  Insert Point Into Tin
*/
    bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,newPntP) ;
/*
**  Retriangulate Void
*/
    if( bcdtmExtEdit_triangulateVoidDtmObject(dtmP,voidFeature,islandsP,numIslands,*newPntP) )goto errexit ;
/*
**  Change Void And Island dtmFeature Types To Breaks
*/
    ftableAddrP(dtmP,voidFeature)->dtmFeatureType = (DTMFeatureType)-9999 ;
    for( n = 0 ; n < numIslands ; ++n ) ftableAddrP(dtmP,*(islandsP+n))->dtmFeatureType = (DTMFeatureType)-9999 ;
   }

// Clear Void Bit And Update Last Modified Time

 if( updateOption == 1 )
   {
    bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,*newPntP)->PCWD) ;
    bcdtmObject_updateLastModifiedTime (dtmP) ;
   }

//  Check Edit

 if( cdbg && updateOption == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM After Inserting Point") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"DTM Invalid") ;
       bcdtmWrite_message(0,0,0,"pntType = %2ld P1 = %10ld P2 = %10ld P3 = %10ld",pntType,P1,P2,P3) ;
       bcdtmWrite_message(0,0,0,"x = %12.5lf y = %12.5lf z = %12.5lf",x,y,z) ;
       bcdtmList_writeCircularListForPointDtmObject(dtmP,*newPntP) ;
       while( 1 ) { P1 = 1 ; }    //  Stop Here - Development Mode Only
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"DTM Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandsP != NULL ) free(islandsP) ;
/*
** Job Completed
*/
 if( ( dbg || cdbg ) && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Point Into DTM Completed") ;
 if( ( dbg || cdbg ) && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Point Into DTM Error") ;
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
int bcdtmExtEdit_removePointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       Point,
 long       Ptype,
 long       Feature,
 long       P1,
 long       P2,
 long       P3
 )
/*
** This Function Removes A Point From The Tin
**
** Note - Ptype must be set to Zero For None Dynamic Mode
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long lp,np,clc ;

// Log Entry Parameters

 if( dbg || cdbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Point DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP   = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Point  = %8ld",Point) ;
    bcdtmWrite_message(0,0,0,"Ptype  = %8ld",Ptype) ;
    bcdtmWrite_message(0,0,0,"P1     = %8ld",P1) ;
    bcdtmWrite_message(0,0,0,"P2     = %8ld",P2) ;
    bcdtmWrite_message(0,0,0,"P3     = %8ld",P3) ;
   }

//  Validate Point

 if( Point < 0 || Point >= dtmP->numPoints ) goto cleanup ;
 if( nodeAddrP(dtmP,Point)->cPtr == dtmP->nullPtr ) goto cleanup ;

// If External Point Reset Hull Pointers

 if( Ptype == 6 || ( Ptype == 0 && nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt) )
   {
    lp = nodeAddrP(dtmP,Point)->hPtr ;
    if(( np = bcdtmList_nextAntDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
    do
      {
       nodeAddrP(dtmP,np)->hPtr = lp ;
       lp = np ;
       if(( np = bcdtmList_nextAntDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
      } while ( np != nodeAddrP(dtmP,Point)->hPtr ) ;

//   Check And Reset Hull Point If Necessary

     if( Point == dtmP->hullPoint )
       dtmP->hullPoint = lp;
   }
/*
** Point On Void Hull
*/
 if( Ptype == 5 )
   {
    if( bcdtmExtInsert_removePointFromDtmFeatureDtmObject(dtmP,Point,Feature)) goto errexit ;
    if( P3 != dtmP->nullPnt ) if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,Feature,P1,P3,P2)) goto errexit ;
   }
/*
** Point On Island Hull
*/
 if( Ptype == 7 )
   {
    if( bcdtmExtInsert_removePointFromDtmFeatureDtmObject(dtmP,Point,Feature)) goto errexit ;
    if( P3 != dtmP->nullPnt ) if( bcdtmInsert_pointIntoDtmFeatureDtmObject(dtmP,Feature,P1,P3,P2)) goto errexit ;
   }

//  Remove Point From All DTM Features

 if( bcdtmExtInsert_removePointFromAllDtmFeaturesDtmObject(dtmP,Point)) goto errexit ;

//  Delete Circular List For Point

 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Circular List For Point %6ld",Point) ;
 clc = nodeAddrP(dtmP,Point)->cPtr ;
 while( clc != dtmP->nullPtr )
   {
    lp  = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    if( bcdtmList_deleteLineDtmObject(dtmP,Point,lp) ) goto errexit ;
   }
 nodeAddrP(dtmP,Point)->cPtr = dtmP->nullPtr ;
 nodeAddrP(dtmP,Point)->hPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,Point)->tPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,Point)->fPtr = dtmP->nullPtr ;

//  Reconnect Broken Triangle Edges

 if( Ptype == 2 || Ptype == 3  )
   {
    if( bcdtmList_insertLineDtmObject(dtmP,P1,P2)) goto errexit ;
    if( Ptype == 3 )
      {
       nodeAddrP(dtmP,P1)->hPtr = P2 ;
      }
   }

//  Decrement Point Count If Possible

 if( Point >= dtmP->numSortedPoints )
   {
    if( Point == dtmP->numPoints - 1 ) --dtmP->numPoints ;
   }

// Check DTM

 if( cdbg && Ptype )
   {
    bcdtmWrite_message(0,0,0,"Check DTM After Removing Point %8ld",Point) ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(0,0,0,"DTM Inavlid") ;
       while( 1 ) { P1 = 1 ; }      //  Stop Here - Development Mode Only
      }
    else bcdtmWrite_message(0,0,0,"DTM Valid") ;
   }

// Clean Up

 cleanup :

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Point Error") ;
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
int bcdtmExtEdit_insertTptrPolygonAroundPointDtmObject(BC_DTM_OBJ *dtmP,long Point,long *Spnt)
/*
** This Function Inserts An Anti ClockWise Tptr Polygon Around A Point
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  sp,lp,clc ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Tptr Polygon Around Point") ;
/*
** Initialise
*/
 *Spnt = dtmP->nullPnt ;
/*
** Test For Valid Tin Point
*/
 if( Point >= 0 && Point < dtmP->numPoints )
   {
    if( nodeAddrP(dtmP,Point)->cPtr != dtmP->nullPtr )
      {
/*
**     Test For Tin Hull Point
*/
       if(nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt )
         {
          sp = *Spnt = Point ;
          lp = nodeAddrP(dtmP,Point)->hPtr ;
          while ( nodeAddrP(dtmP,lp)->hPtr != Point )
            {
             nodeAddrP(dtmP,sp)->tPtr = lp ;
             sp = lp ;
             if(( lp = bcdtmList_nextAntDtmObject(dtmP,Point,lp)) < 0 ) goto errexit ;
            }
          nodeAddrP(dtmP,sp)->tPtr = lp ;
          nodeAddrP(dtmP,lp)->tPtr = Point ;
         }
/*
**     Set point Hull Into Tptr Polygon
*/
       else
         {
          clc = nodeAddrP(dtmP,Point)->cPtr ;
          sp = *Spnt = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             nodeAddrP(dtmP,sp)->tPtr = lp   ;
             sp  = lp ;
            }
          nodeAddrP(dtmP,sp)->tPtr = *Spnt   ;
/*
**        Set Tptr Polygon AntiClockwise
*/
          bcdtmList_reverseTptrPolygonDtmObject(dtmP,*Spnt) ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Tptr Polygon Around Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Tptr Polygon Around Point Error") ;
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
int bcdtmExtEdit_addPointToDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double z,long *Point )
/*
** This Function Finds The Triangle For A Point
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   P1,P2,P3,Ptype,VoidFlag ;
 double Zs ;
/*
** Initialise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point To Tin") ;
 *Point = dtmP->nullPnt ;
/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&Zs,&Ptype,&P1,&P2,&P3 ) ) goto errexit ;
/*
** Test For Point In Void
*/
 if( Ptype != 0 )
   {
    if( Ptype == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) ) Ptype = 6 ;
    if( Ptype == 2 || Ptype == 3 )
          {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,P1,P2,&VoidFlag) ) goto errexit ;
       if( ! VoidFlag ) Ptype = 6 ;
      }
    if(  Ptype == 4 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,&VoidFlag)) goto errexit ;
       if( VoidFlag ) Ptype = 6 ;
      }
   }
/*
** Store Point In Tin Object
*/
 if( Ptype >= 2 && Ptype <= 4 )
   {
    if( bcdtmInsert_storePointInDtmObject(dtmP,2,1,x,y,z,Point)) goto errexit ;
        goto cleanup ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Point To Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Point To Tin Error") ;
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
int bcdtmExtEdit_getIslandsInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long **islandsPP, long *numIslandsP)
/*
**
** This Function Gets The Set Of Islands Internal To A Void
** This Function Updated September 2003 - Rob Cormack
**
*/
{
 int   ret=DTM_SUCCESS ;
 long  numIslandFeatures ;
 DTM_TIN_POINT_FEATURES *ifP,*islandFeaturesP=NULL ;
/*
** Initialise
*/
 *numIslandsP = 0 ;
 if( *islandsPP != NULL ) { free(*islandsPP) ; *islandsPP = NULL ; }
/*
** Check For A Void Feature
*/
 if( voidFeature >= 0 && voidFeature < dtmP->numFeatures )
   {
    if( ftableAddrP(dtmP,voidFeature)->dtmFeatureType  == DTMFeatureType::Void && ftableAddrP(dtmP,voidFeature)->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Get Island Features Internal To Void Feature
*/
       if( bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(dtmP,voidFeature,&islandFeaturesP,&numIslandFeatures) ) goto errexit ;
/*
**      Allocate Memory For Islands
*/
       *islandsPP = ( long * ) malloc ( numIslandFeatures * sizeof(long)) ;
       if( *islandsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**      Copy Island Features
*/
       for( ifP = islandFeaturesP ; ifP < islandFeaturesP + numIslandFeatures ; ++ifP )
         {
          *(*islandsPP+*numIslandsP) = ifP->dtmFeature ;
          ++*numIslandsP ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( islandFeaturesP != NULL ) { free(islandFeaturesP) ; islandFeaturesP = NULL ; }
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numIslandsP = 0 ;
 if( *islandsPP != NULL ) { free(*islandsPP) ; *islandsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObjectDup(BC_DTM_OBJ *dtmP,long voidFeature,DTM_TIN_POINT_FEATURES **islands,long *numIslands)
/*
** This Function Gets The List Of All Island Feature Internal To A Void
**
** Rob Cormack - June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   memIslands=0,memIslandsInc=10,islandFeature ;
 long   ofs,lp,clc,pnt,ppnt,npnt,lpnt,fPnt,lPnt,sPnt,numLineFeatures ;
 DTM_TIN_POINT_FEATURES *lineFeatures=NULL,*featP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void") ;
/*
** Initialise
*/
 *numIslands = 0 ;
 if( *islands != NULL ) { free(*islands) ; *islands = NULL ; }
/*
** Copy Void Feature To Tptr List
*/
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&sPnt)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 fPnt = lPnt = dtmP->nullPnt ;
 ppnt = sPnt ;
 pnt  = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                        { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
      }
/*
** Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
               }
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
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
** Scan Internal Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
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
            }
         }
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Scan Marked Points For Connection To An Island Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Marked Points For Connection To Island Feature") ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
/*
**     Scan Marked Point For Island Features
*/
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeatures,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
         }
/*
**     Get Next Marked Point
*/
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Scan Void Hull For Direct Connection To Island
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Void Hull For Direct Connection To Islands") ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
/*
** Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
    if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeatures,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
       if( lp != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Island,pnt,lp,&lineFeatures,&numLineFeatures)) goto errexit ;
          for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,pnt,lp) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,lp,&lineFeatures,&numLineFeatures) ) goto errexit ;
                for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
                  {
                   if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
                  }
               }
            if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
           }
         if( lp != npnt )
           {
            if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Island,lp,pnt,&lineFeatures,&numLineFeatures)) goto errexit ;
            for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
              {
               if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
              }
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
** Null Out Tptr List
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,sPnt)) goto errexit ;
/*
** Scan Detected Islands For Connection To Other Islands
*/
 for( ofs = 0 ; ofs < *numIslands ; ++ofs )
   {
    islandFeature = (*islands+ofs)->dtmFeature ;
    sPnt = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,sPnt,&ppnt)) goto errexit ;
    do
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sPnt,&npnt)) goto errexit ;
       if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,npnt)) < 0 ) goto errexit ;
       while ( pnt != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Island,pnt,&lineFeatures,&numLineFeatures)) goto errexit ;
          for( featP = lineFeatures ; featP < lineFeatures + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storePointFeaturesInDtmFeatureList(islands,numIslands,&memIslands,memIslandsInc,featP->dtmFeature,DTMFeatureType::Island,dtmP->nullUserTag,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
          if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pnt)) < 0 ) goto errexit ;
         }
       ppnt = sPnt ;
       sPnt = npnt ;
      } while ( sPnt != ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( lineFeatures != NULL ) { free(lineFeatures) ; lineFeatures = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Getting Island Features Internal To Void Error") ;
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
int bcdtmExtEdit_storePointFeaturesInDtmFeatureList(DTM_TIN_POINT_FEATURES **bcdtmList,long *numDtmList,long *memDtmList,long memDtmListInc,long dtmFeature,DTMFeatureType dtmFeatureType,DTMUserTag userTag,long priorPoint,long nextPoint)
/*
** This Stores A Feature In A Dtm Feature List
**
** Rob Cormack - June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   storeFlag ;
 DTM_TIN_POINT_FEATURES *pfl ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List") ;
/*
** Scan Point Feature List For Already Included Feature
*/
 storeFlag = 1 ;
 for( pfl = *bcdtmList ; pfl < *bcdtmList + *numDtmList && storeFlag ; ++pfl )
   {
    if( pfl->dtmFeature == dtmFeature ) storeFlag = 0 ;
   }
/*
** Add Feature To Feature List
*/
 if( storeFlag )
   {
/*
** Allocate memory If Necessary
*/
    if( *numDtmList == *memDtmList )
      {
       *memDtmList = *memDtmList + memDtmListInc ;
       if( *bcdtmList == NULL ) *bcdtmList = ( DTM_TIN_POINT_FEATURES * ) malloc ( *memDtmList * sizeof(DTM_TIN_POINT_FEATURES)) ;
       else                   *bcdtmList = ( DTM_TIN_POINT_FEATURES * ) realloc ( *bcdtmList , *memDtmList * sizeof(DTM_TIN_POINT_FEATURES)) ;
       if( *bcdtmList == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
** Store Feature
*/
    (*bcdtmList + *numDtmList)->dtmFeature     =  dtmFeature ;
    (*bcdtmList + *numDtmList)->dtmFeatureType =  (DTMFeatureType)dtmFeatureType ;
    (*bcdtmList + *numDtmList)->userTag        =  userTag ;
    (*bcdtmList + *numDtmList)->nextPoint      =  nextPoint ;
    (*bcdtmList + *numDtmList)->priorPoint     =  priorPoint ;
    ++*numDtmList ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Storing Feature In Dtm Feature List Error") ;
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
int bcdtmExtEdit_testForDtmFeatureHullLineDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType DtmFeature,long P1,long P2,long *HullLine,long *Feature,long *Direction)
/*
** This Function Tests If Line P1-P2 Is a Feature Hull Line
*/
{
 long clc ;
/*
** Initialiase
*/
 *Feature  = dtmP->nullPnt ;
 *HullLine = *Direction = 0 ;
 if(! bcdtmList_testLineDtmObject(dtmP,P1,P2)) return(0) ;
/*
** Scan P1
*/
 if( ( clc = nodeAddrP(dtmP,P1)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DtmFeature && flistAddrP(dtmP,clc)->nextPnt == P2 )
         { *Feature = flistAddrP(dtmP,clc)->dtmFeature ; *HullLine = 1 ; *Direction = 1 ;  return(DTM_SUCCESS) ; }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Scan P2
*/
 if( ( clc = nodeAddrP(dtmP,P2)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DtmFeature && flistAddrP(dtmP,clc)->nextPnt == P1 )
         { *Feature = flistAddrP(dtmP,clc)->dtmFeature ; *HullLine = 1 ; *Direction = 2 ;  return(DTM_SUCCESS) ; }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
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
int bcdtmExtEdit_removeInternalVoidPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandsP, long numIslands)
/*
** This Function Removes Internal Void Points And Lines
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  sp,np,pp,lp,pl,ph,fpnt,clc,nvp,process,islandFeature  ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Internal Void Points And Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"voidFeature = %8ld",voidFeature) ;
    bcdtmWrite_message(0,0,0,"islandsP    = %p",islandsP) ;
    bcdtmWrite_message(0,0,0,"numIslands  = %8ld",numIslands) ;
   }
/*
** Initialise
*/
 pl = dtmP->numPoints ; ph = 0 ;
/*
** Null Tptr Values
*/
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Scan Around Void And Mark Internal Void Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Points") ;
 process = nvp = 0 ;
 pp = ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Void First Point = %8ld ** %12.5lf %12.5lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,pp,&sp) ;
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&np) ;
 fpnt = np ;
 do
   {
    if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( lp != pp  )
      {
       if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
         {
          if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD))
            {
             if( lp < pl ) pl = lp ;
             if( lp > ph ) ph = lp ;
             nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
             ++nvp ;
             process = 1 ;
            }
         }
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
      }
    pp = sp ; sp = np ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&np) ;
   } while ( np != fpnt ) ;
/*
** Scan Around islandsP And Mark External Void Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking External Island Points") ;
 for( np = 0 ; np < numIslands ; ++ np )
   {
    islandFeature = *(islandsP+np) ;
    pp = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,pp,&sp) ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
    fpnt = np ;
    do
      {
       if( ( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
       while ( lp != pp  )
         {
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD))
               {
                if( lp < pl ) pl = lp ;
                if( lp > ph ) ph = lp ;
                nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
                ++nvp ;
                process = 1 ;
               }
            }
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
         }
       pp = sp ; sp = np ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
      } while ( np != fpnt ) ;
   }
/*
** Sequentially Scan Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Marked Points") ;
 while( process )
   {
    process = 0 ;
    for( sp = pl ; sp <= ph ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == -dtmP->nullPnt )
         {
          clc = nodeAddrP(dtmP,sp)->cPtr ;
          while ( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD)) nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
                process = 1 ;
                ++nvp ;
               }
            }
          nodeAddrP(dtmP,sp)->tPtr = -dtmP->nullPnt + 10 ;
         }
      }
   }
/*
** Delete Void Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Void Points") ;
 if( nvp )
   {
    for( sp = pl ; sp <= ph ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr < 0 )
         {
          clc = nodeAddrP(dtmP,sp)->cPtr ;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( bcdtmList_deleteLineDtmObject(dtmP,sp,lp) ) goto errexit ;
            }
          nodeAddrP(dtmP,sp)->cPtr = dtmP->nullPtr ;
          nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
          nodeAddrP(dtmP,sp)->fPtr = dtmP->nullPtr ;
         }
      }
   }
/*
** Scan Around Void And Delete All Internal Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Void Lines") ;
 pp = ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ;
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,pp,&sp) ;
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&np) ;
 fpnt = sp ;
 do
   {
    if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( lp != pp )
      {
       bcdtmList_deleteLineDtmObject(dtmP,sp,lp) ;
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
      }
    pp = sp ;
    sp = np ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,np,&np) ;
   } while ( sp != fpnt ) ;
/*
** Scan Around Island And Delete External Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting External Island Lines") ;
 for( np = 0 ; np < numIslands ; ++np )
   {
    islandFeature = *(islandsP+np) ;
    pp = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,pp,&sp) ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
    fpnt = sp ;
    do
      {
       if( ( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
       while ( lp != pp  )
         {
          bcdtmList_deleteLineDtmObject(dtmP,sp,lp) ;
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
         }
       pp = sp ; sp = np ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
      } while ( sp != fpnt ) ;
   }
/*
** Null Out Tptr Values
*/
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Internal Void Points And Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Internal Void Points And Lines Error") ;
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
int bcdtmExtEdit_triangulateVoidDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       voidFeature,
 long       *islandFeaturesP,
 long       numIslandFeatures,
 long       internalPoint
)
/*
** This Function Fills A Void Feature With Triangles
*/
{
 int            ret=DTM_SUCCESS,dbg=0 ;
 long           sp,spnt,n,np,lp,pp,ps,pn,pl,pnt,point,numPoints,clc ;
 DPoint3d            *pointsP=NULL,intPoint ;
 DTM_TIN_POINT  *pntP ;
 BC_DTM_OBJ     *tempDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"ReTriangulating Void") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"voidFeature        = %8ld",voidFeature) ;
    bcdtmWrite_message(0,0,0,"islandFeaturesP    = %p",islandFeaturesP) ;
    bcdtmWrite_message(0,0,0,"numIslandFeatures  = %8ld",numIslandFeatures) ;
    bcdtmWrite_message(0,0,0,"internalPoint      = %8ld",internalPoint) ;
   }
/*
** Copy Void Feature Points To Point Array
*/
 if(bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,voidFeature,&pointsP,&numPoints)) goto errexit ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&tempDtmP))  goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPoints*numIslandFeatures+1,numPoints) ;
/*
** Store Internal Point In Dtm Object
*/
 if( internalPoint != dtmP->nullPnt )
   {
    pntP = pointAddrP(dtmP,internalPoint) ;
    intPoint.x = pntP->x ;
    intPoint.y = pntP->y ;
    intPoint.z = pntP->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,&intPoint,1)) goto errexit ;
   }
/*
** Store Void Feature As Break Lines
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pointsP,numPoints)) goto errexit ;
 if( pointsP != NULL )
   {
    free(pointsP) ;
    pointsP = NULL ;
   }
/*
** Write Islands Features To Dtm Object As Void Voids
*/
 for( n = 0 ; n < numIslandFeatures ; ++n )
   {
    if(bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,*(islandFeaturesP+n),&pointsP,&numPoints)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Void,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pointsP,numPoints)) goto errexit ;
    if( pointsP != NULL ) { free(pointsP) ; pointsP = NULL ; }
   }
/*
** Triangulate Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Void") ;
 DTM_NORMALISE_OPTION = false ;             // To Inhibit Normalisation Of Coordinates
 DTM_DUPLICATE_OPTION = false ;             // To Inhibit Removal Of None Identical Points
 tempDtmP->ppTol = tempDtmP->plTol = 0.0 ;
 if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION  = true ;
 DTM_DUPLICATE_OPTION = true ;
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing None Feature Tin Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
/*
** Set DTM Point Numbers Into TempDTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting DTM Point Numbers") ;
 for( pnt = 0 ; pnt < tempDtmP->numPoints ; ++pnt )
   {
    pntP = pointAddrP(tempDtmP,pnt) ;
    if( bcdtmFind_closestPointDtmObject(dtmP,pntP->x,pntP->y,&point)) goto errexit ;
    nodeAddrP(tempDtmP,pnt)->tPtr = point ;
   }
/*
** Copy Triangles On tempDtmP Hull To Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copy Triangles To Tin") ;
 sp = tempDtmP->hullPoint ;
 if(( np = bcdtmList_nextClkDtmObject(tempDtmP,sp,nodeAddrP(tempDtmP,sp)->hPtr)) < 0 ) goto errexit ;
 do
   {
    ps = nodeAddrP(tempDtmP,sp)->tPtr ;
    pn = nodeAddrP(tempDtmP,np)->tPtr ;
    if(( lp = bcdtmList_nextClkDtmObject(tempDtmP,sp,np)) < 0 ) goto errexit ;
    while ( lp != nodeAddrP(tempDtmP,sp)->hPtr )
      {
       pl = nodeAddrP(tempDtmP,lp)->tPtr ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ps,pl,pn)) goto errexit ;
       pn = pl ;
       if(( lp = bcdtmList_nextClkDtmObject(tempDtmP,sp,lp)) < 0 ) goto errexit ;
      }
    np = sp ; sp = nodeAddrP(tempDtmP,sp)->hPtr ;
   } while ( sp != tempDtmP->hullPoint ) ;
/*
** Copy External Triangles On tempDtmP islandFeaturesP To Tin
*/
 for ( n = 0 ; n < tempDtmP->numFeatures ; ++n )
   {
    if(ftableAddrP(tempDtmP,n)->dtmFeatureType == DTMFeatureType::Void && ( pp = ftableAddrP(tempDtmP,n)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       bcdtmList_getNextPointForDtmFeatureDtmObject(tempDtmP,n,pp,&sp) ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(tempDtmP,n,sp,&np) ;
       spnt = sp ;
       do
         {
          if(( lp = bcdtmList_nextClkDtmObject(tempDtmP,sp,np)) < 0 ) goto errexit ;
          ps = nodeAddrP(tempDtmP,sp)->tPtr ;
          pn = nodeAddrP(tempDtmP,np)->tPtr ;
          while ( lp != pp )
            {
             pl = nodeAddrP(tempDtmP,lp)->tPtr ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ps,pl,pn)) goto errexit ;
             pn = pl ;
             if(( lp = bcdtmList_nextClkDtmObject(tempDtmP,sp,lp)) < 0 ) goto errexit ;
            }
          pp = sp ; sp = np ; bcdtmList_getNextPointForDtmFeatureDtmObject(tempDtmP,n,np,&np) ;
         } while ( sp != spnt ) ;
      }
   }
/*
**  Insert Lines For internalPoint
*/
 if( internalPoint != dtmP->nullPnt )
   {
    if( bcdtmFind_closestPointDtmObject(tempDtmP,intPoint.x,intPoint.y,&sp)) goto errexit ;
    ps = nodeAddrP(tempDtmP,sp)->tPtr ;
    pp = dtmP->nullPnt ;
    clc = nodeAddrP(tempDtmP,sp)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       np  = clistAddrP(tempDtmP,clc)->pntNum ;
       clc = clistAddrP(tempDtmP,clc)->nextPtr ;
       pn  = nodeAddrP(tempDtmP,np)->tPtr  ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ps,pn,pp)) goto errexit ;
       pp = pn ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION  = true ;
 DTM_DUPLICATE_OPTION = true ;
 if( pointsP != NULL ) { free(pointsP) ; pointsP = NULL ; }
 bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating Void Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating Void Error") ;
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
int bcdtmExtEdit_deletePointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long tinPoint,
 long deleteFlag
)
/*
** This Function Deletes A Tin Point
*/
{
 int   ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long  spnt,numFeaturePts,drapeOption,insertOption,canDeleteFlag=0 ;
 long  numPointFeatures,nextHullPnt,priorHullPnt,numHullPnts;
 DPoint3d   *featurePtsP=NULL ;
 DTM_TIN_POINT_FEATURES *ppf,*pointFeatures=NULL ;
 BC_DTM_OBJ  *voidPolygonP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Tin Point") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"tinPoint   = %8ld",tinPoint) ;
    bcdtmWrite_message(0,0,0,"deleteFlag = %8ld",deleteFlag) ;
    bcdtmWrite_message(0,0,0,"Tin Point[%8ld] ** %10.4lf %10.4lf %10.4lf",tinPoint,pointAddrP(dtmP,tinPoint)->x,pointAddrP(dtmP,tinPoint)->y,pointAddrP(dtmP,tinPoint)->z) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Before Deleting Point") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Tin Valid") ;
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
   }
/*
** Check For Point Range Error
*/
 if( tinPoint < 0 || tinPoint >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Tin Point Range Error") ;
    goto errexit ;
   }
/*
** Check Point Can Be Deleted
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point Can Be Deleted") ;
    if( bcdtmExtEdit_checkPointCanBeDeletedDtmObject(dtmP,tinPoint,deleteFlag,&canDeleteFlag)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"canDeleteFlag = %2ld",canDeleteFlag) ;
   }

/*
** Reset Point Type
*/
 deleteFlag = 1 ;
 if( nodeAddrP(dtmP,tinPoint)->hPtr != dtmP->nullPnt ) deleteFlag = 2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"deleteFlag = %2ld",deleteFlag) ;
/*
** Get List Of DTM Features For Point
*/
 if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,tinPoint,&pointFeatures,&numPointFeatures) ) goto errexit ;
/*
** Write Point Features
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Point Features = %3ld",numPointFeatures) ;
    for( ppf = pointFeatures ; ppf < pointFeatures + numPointFeatures ; ++ppf )
      {
       bcdtmWrite_message(0,0,0,"Feature[%3ld] ** Feature = %6ld Type =  %4ld Prior = %9ld Next = %9ld UserTag = %I64d",(long)(ppf-pointFeatures),ppf->dtmFeature,ppf->dtmFeatureType,ppf->priorPoint,ppf->nextPoint,ppf->userTag) ;
      }
    if( dbg == 2 )
      {
       for( ppf = pointFeatures ; ppf < pointFeatures + numPointFeatures ; ++ppf )
         {
          bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,ppf->dtmFeature) ;
         }
      }
   }
/*
** Check Internal Point Can Be Deleted.
** All Features Must Have More Than Two Points
*/
 if( deleteFlag == 1 && numPointFeatures > 0 )
   {
    long  numPoints=0,closeFlag=0 ;
    bool canDelete = true ;
    for( ppf = pointFeatures ; ppf < pointFeatures + numPointFeatures && canDelete ; ++ppf )
      {
       if( ppf->dtmFeatureType != DTMFeatureType::GroupSpots )
         {
          if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,ppf->dtmFeature,&numPoints,&closeFlag)) goto errexit ;
          if( numPoints <= 2 ) canDelete = false ;
         }
      }
    if( canDelete == false ) goto cleanup ;
   }
/*
** Insert Tptr Polygon Around Point
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Inserting Tptr Polygon About Delete Point") ;
 if( bcdtmExtEdit_insertTptrPolygonAroundPointDtmObject(dtmP,tinPoint,&spnt)) goto errexit ;
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,spnt) ;
/*
** Check Point On Tin Hull Can Be Deleted
*/
 if( deleteFlag == 2 )
   {

//  Point Cannot Be Deleted If It Has Linear Features

    if( numPointFeatures > 0 )
      {
       for( ppf = pointFeatures ; ppf < pointFeatures + numPointFeatures ; ++ppf )
         {
          if( ppf->dtmFeatureType != DTMFeatureType::GroupSpots )
            {
             if( bcdtmList_nullTptrListDtmObject(dtmP,tinPoint)) goto errexit ;
             goto cleanup ;
            }
         }
      }

//  Point Cannot Be Deleted If The Tin Hull Is Corrupted After The Point Is Deleted

    nextHullPnt = nodeAddrP(dtmP,tinPoint)->hPtr ;
    if( ( priorHullPnt = bcdtmList_nextClkDtmObject(dtmP,tinPoint,nextHullPnt)) < 0 ) goto errexit ;
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,tinPoint)) goto errexit ;
    numHullPnts = 0 ;
    spnt = nodeAddrP(dtmP,priorHullPnt)->tPtr ;
    while( spnt != nextHullPnt )
      {
       if( nodeAddrP(dtmP,spnt)->hPtr != dtmP->nullPnt )
         {
          if( bcdtmList_nullTptrListDtmObject(dtmP,tinPoint)) goto errexit ;
          goto cleanup ;
         }
       spnt = nodeAddrP(dtmP,spnt)->tPtr ;
      }
   }
/*
** Delete Point
*/
 switch( deleteFlag )
   {
    case 1  : /* Delete Internal Point And Retriangulate */
      if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Internal Point And Retriangulating") ;
      if( bcdtmExtEdit_removePointDtmObject(dtmP,tinPoint,0,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
      if( bcdtmClip_fillTptrPolygonWithTrianglesDtmObject(dtmP,spnt)) goto errexit ;
      if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;

//   07Mar2012 RobC  Join Up Features With Broken Connections

    for( ppf = pointFeatures ; ppf < pointFeatures + numPointFeatures ; ++ppf )
      {
       if( dbg )bcdtmWrite_message(0,0,0,"Feature[%3ld] ** Feature = %6ld Type =  %4ld Prior = %9ld Next = %9ld UserTag = %I64d",(long)(ppf-pointFeatures),ppf->dtmFeature,ppf->dtmFeatureType,ppf->priorPoint,ppf->nextPoint,ppf->userTag) ;
       if( ppf->priorPoint != dtmP->nullPnt && ppf->nextPoint != dtmP->nullPnt )
         {
          if( ! bcdtmList_testLineDtmObject(dtmP,ppf->priorPoint,ppf->nextPoint ))
            {
             if( dbg )bcdtmWrite_message(0,0,0,"Feature[%3ld] Connection Broken ** priorPnt = %8ld nextPoint = %8ld",(long)(ppf-pointFeatures),ppf->priorPoint,ppf->nextPoint,ppf->userTag) ;
             if( ppf->dtmFeatureType != DTMFeatureType::GroupSpots )
               {
                drapeOption = 1 ;
                insertOption = 2 ;
                if( ppf->dtmFeatureType == DTMFeatureType::Breakline || ppf->dtmFeatureType == DTMFeatureType::SoftBreakline )
                  {
                   drapeOption  = 2 ;
                   insertOption = 1 ;
                  }

//              Remove And Re Insert Feature

                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,ppf->dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmInsert_internalStringIntoDtmObject(dtmP,drapeOption,insertOption,featurePtsP,numFeaturePts,&spnt)) goto errexit ;
                if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,spnt,0)) goto errexit ;
                if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,ppf->dtmFeatureType,ppf->userTag,ppf->userFeatureId,spnt,1)) goto errexit ;
                if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,ppf->dtmFeature)) goto errexit ;
               }
            }
         }
      }
    break   ;

    case 2  : /* Delete Point On Tin Hull */
      if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Point On Tin Hull") ;
      if( bcdtmList_nullTptrListDtmObject(dtmP,tinPoint)) goto errexit ;
      if( bcdtmExtEdit_removePointDtmObject(dtmP,tinPoint,0,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
    break ;

    default :
    break   ;
   } ;

/*
** Update Last Modified Time
*/
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin After Deleting Point") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Tin Invalid After Deleting Point") ;
       while( 1 ) { spnt = 1 ; }   // Stop Here - Development Only
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Tin Valid After Deleting Point") ;
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP   != NULL ) { free(featurePtsP)   ; featurePtsP = NULL   ; }
 if( pointFeatures != NULL ) { free(pointFeatures) ; pointFeatures = NULL ; }
 if( voidPolygonP  != NULL ) bcdtmObject_destroyDtmObject(&voidPolygonP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Tin Point Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Tin Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_breakDtmFeatureAtPointDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long breakPoint )
/*
** This Function break A DTM Features Into Two features At A Point
** The Point Is Excluded From Either Feature
*/
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    sp,np,pp,fpnt,pointFlag,closeFlag ;
 DTMFeatureType type;
 DTMUserTag utag ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Breaking Dtm Feature %6ld At Point %6ld",dtmFeature,breakPoint) ;
/*
** Check For Valid Dtm Feature
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures  )
   {
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
/*
**     Valid Feature Detected
*/
       fpnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint  ;
       utag = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
       type = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Feature Type = %4ld Fpnt = %6ld User Tag = %I64d",type,fpnt,utag) ;
/*
**    Check Point Is On Feature
*/
       sp = fpnt ;
       pointFlag = 0 ;
       do
         {
          if( sp == breakPoint ) pointFlag = 1 ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
          sp = np ;
         } while ( sp != dtmP->nullPnt && sp != fpnt && ! pointFlag ) ;
/*
**     Remove Point From Feature
*/
       if( pointFlag )
         {
/*
**       Check If Feature Closes
*/
          sp = fpnt ;
          closeFlag = false ;
          do
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
             sp = np ;
            } while ( sp != dtmP->nullPnt && sp != fpnt ) ;
          if( sp == fpnt ) closeFlag = true ;
          if( dbg ) bcdtmWrite_message(0,0,0,"closeFlag = %1ld",closeFlag) ;
/*
**        Get Prior And Next Points To Break Point
*/
          if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,breakPoint,&pp)) goto errexit ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,breakPoint,&np)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Prior Point = %6ld Next Point = %6ld",pp,np) ;
/*
**        Copy DTM Feature To Tptr Array
*/
          if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&fpnt) ) goto errexit  ;
          if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,fpnt) ;
/*
**        Delete DTM Feature From Tin Object
*/
          if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit  ;
/*
**         Break tPtr List At Break Point
*/
          nodeAddrP(dtmP,breakPoint)->tPtr = dtmP->nullPnt ;
          if( pp != dtmP->nullPnt ) nodeAddrP(dtmP,pp)->tPtr = dtmP->nullPnt ;
          if( nodeAddrP(dtmP,fpnt)->tPtr == breakPoint ) nodeAddrP(dtmP,fpnt)->tPtr = dtmP->nullPnt ;
/*
**        Store First Section Of Feature
*/
          if( nodeAddrP(dtmP,fpnt)->tPtr != dtmP->nullPnt )
            {
             if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,dtmP->numFeatures,type,utag,dtmP->dtmFeatureIndex,fpnt,1)) goto errexit ;
             ++dtmP->dtmFeatureIndex ;
            }
/*
**        Store Second Section Of Feature
*/
          if( np != dtmP->nullPnt )
            {
             if ( nodeAddrP(dtmP,np)->tPtr != dtmP->nullPnt )
                {
                 if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,dtmP->numFeatures,type,utag,dtmP->dtmFeatureIndex,np,1)) goto errexit ;
                 ++dtmP->dtmFeatureIndex ;
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
int bcdtmExtEdit_dataPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double     x,
 double     y,
 double     *z,
 long       *pntTypeP,
 long       *dtmFeatureP,
 long       *pnt1P,
 long       *pnt2P,
 long       *pnt3P
)
/*
** This Function Finds The Triangle For A Point
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   voidFlag,pt,p1,p2,p3,p4,sd3,sd4,count,closeFlag ;
 double n3,n4 ;
 bool   p3Intersect,p4Intersect ;
/*
** Initialise
*/
 *dtmFeatureP = dtmP->nullPnt ;
/*
** Log Method Arguements
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Data Point DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x        = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y        = %12.5lf",y) ;
   }
/*
** Find Triangle For Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Triangle For Point") ;
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,z,pntTypeP,pnt1P,pnt2P,pnt3P ) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"pntTypeP = %2ld ** pnt1P = %9ld pnt2P = %9ld pnt3P = %9ld",*pntTypeP,*pnt1P,*pnt2P,*pnt3P) ;
/*
** Test For Point In Void
*/
 if( *pntTypeP != 0 )
   {
    if( *pntTypeP == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*pnt1P)->PCWD) ) *pntTypeP = 5 ;
    if( *pntTypeP == 2 || *pntTypeP == 3 )
          {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,*pnt1P,*pnt2P,&voidFlag) ) goto errexit ;
       if( voidFlag ) *pntTypeP = 5 ;
      }
    if(  *pntTypeP == 4 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,*pnt1P,*pnt2P,*pnt3P,&voidFlag)) goto errexit ;
       if( voidFlag ) *pntTypeP = 5 ;
      }
   }
/*
** If Point External Find Closest Hull Line
*/
 if( *pntTypeP == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point External To Tin") ;
    if( bcdtmExtEdit_findClosestHullLineForPointAddDtmObject(dtmP,x,y,&p1,&p2)) goto errexit ;
//  bcdtmWrite_message(0,0,0,"p1[%8ld] = %12.5lf %12.5lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
//  bcdtmWrite_message(0,0,0,"p2[%8ld] = %12.5lf %12.5lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
    p4 = nodeAddrP(dtmP,p2)->hPtr ;
    bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject(dtmP,p3,x,y,p3Intersect) ;
    bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject(dtmP,p4,x,y,p4Intersect) ;
    if( ! p3Intersect && ! p4Intersect )
      {
       n3 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,x,y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
       n4 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y,x,y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
       sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
       sd4 = bcdtmMath_sideOf(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y) ;
       if( sd3 < 0 && sd4 > 0 )
         {
          if( n3 > n4 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
          else          { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
         }
       else if( sd3 <  0 && sd4 <= 0 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
       else if( sd3 >= 0 && sd4 >  0 ) { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
       else                            { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ; }
      }
    else if( ! p3Intersect &&  p4Intersect )
      {
       sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
       if( sd3 < 0 )
         { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
       else
         { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ; }
      }
    else if( p3Intersect && ! p4Intersect )
      {
       sd4 = bcdtmMath_sideOf(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y) ;
       if( sd4 > 0 )
         { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
       else
         { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ; }
      }
    else
      {
       *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ;
      }


    *pntTypeP = 6 ;
   }
/*
** If Point In Void Get Void Feature
*/
 if( *pntTypeP == 5 )
   {
    if( bcdtmExtEdit_findClosestVoidLineDtmObject(dtmP,x,y,&p1,&p2,dtmFeatureP)) goto errexit ;
    switch( ftableAddrP(dtmP,*dtmFeatureP)->dtmFeatureType )
      {
       case  DTMFeatureType::Void :
         bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,*dtmFeatureP,p1,&p3) ;
         bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,*dtmFeatureP,p2,&p4) ;
         n3 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,x,y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
         n4 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y,x,y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
         sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
         sd4 = bcdtmMath_sideOf(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y) ;
         if( sd3 > 0 && sd4 < 0 )
           {
            if( n3 > n4 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
            else          { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
           }
         else if( sd3 >  0 && sd4 >= 0 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
         else if( sd3 <= 0 && sd4 <  0 ) { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
         else                            { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ; }
         *pntTypeP = 5 ;
         bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,*dtmFeatureP,&count,&closeFlag) ;
         if( count == 3 ) *pnt3P = dtmP->nullPnt  ;
       break ;

       case  DTMFeatureType::Island :
         bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,*dtmFeatureP,p1,&p3) ;
         bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,*dtmFeatureP,p2,&p4) ;
         n3 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,x,y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
         n4 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y,x,y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
         sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,x,y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
         sd4 = bcdtmMath_sideOf(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y) ;
         if( sd3 < 0 && sd4 > 0 )
           {
            if( n3 > n4 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
            else          { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
           }
         else if( sd3 <  0 && sd4 <= 0 ) { *pnt1P = p3 ; *pnt2P = p1 ; *pnt3P = p2 ; }
         else if( sd3 >= 0 && sd4 >  0 ) { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p4 ; }
         else                            { *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = dtmP->nullPnt ; }
         *pntTypeP = 7 ;
       break ;

       default :
       break  ;

      } ;
   }
/*
** Set Lowest Point Number First
*/
 if( *pntTypeP == 4 ) while ( *pnt1P > *pnt2P || *pnt1P > *pnt3P ) { pt = *pnt1P ; *pnt1P = *pnt2P ; *pnt3P = pt ; }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Data Point For DTM Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Data Point For DTM Error") ;
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
int bcdtmExtEdit_deleteLineDtmObject(BC_DTM_OBJ *dtmP,long deleteFlag,long tinPnt1,long tinPnt2,long *tinPnt3,long *tinPnt4)
/*
** This routine deletes or swaps a Tin Line
** deleteFlag == 1 - delete line
** deleteFlag == 2 - swap line
**
** Rob Cormack July 2003
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  ap,cp,swapLine,numDtmFeatures,numLineFeatures,numPolyPts ;
 DPoint3d   *polyPtsP=NULL ;
 DTM_TIN_POINT_FEATURES *plf,*lineFeatures=NULL ;
 BC_DTM_OBJ *voidPolygonP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Tin Line ** Delete Flag = %2ld",deleteFlag ) ;
    bcdtmWrite_message(0,0,0,"Tin Point1 = %2ld ** %10.4lf %10.4lf %10.4lf",tinPnt1,pointAddrP(dtmP,tinPnt1)->x,pointAddrP(dtmP,tinPnt1)->y,pointAddrP(dtmP,tinPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"Tin Point2 = %2ld ** %10.4lf %10.4lf %10.4lf",tinPnt2,pointAddrP(dtmP,tinPnt2)->x,pointAddrP(dtmP,tinPnt2)->y,pointAddrP(dtmP,tinPnt2)->z) ;
   }
/*
** Initialise
*/
 *tinPnt3 = *tinPnt4 = -1 ;
/*
** Test For Valid Line
*/
 if( tinPnt1 < 0  || tinPnt1 >= dtmP->numPoints || tinPnt2 < 0 || tinPnt2 >= dtmP->numPoints ) goto errexit ;
 if( nodeAddrP(dtmP,tinPnt1)->cPtr == dtmP->nullPtr || nodeAddrP(dtmP,tinPnt2)->cPtr == dtmP->nullPtr )  goto errexit    ;
 if( ! bcdtmList_testLineDtmObject(dtmP,tinPnt1,tinPnt2)) goto errexit ;
/*
** Get Next Anti Clockwise And Clockwise Points For Line
*/
 ap = cp = dtmP->nullPnt ;
 if(( ap = bcdtmList_nextAntDtmObject(dtmP,tinPnt1,tinPnt2)) < 0 ) goto errexit ;
 if(( cp = bcdtmList_nextClkDtmObject(dtmP,tinPnt1,tinPnt2)) < 0 ) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,ap,tinPnt2)) ap = dtmP->nullPnt ;
 if( ! bcdtmList_testLineDtmObject(dtmP,cp,tinPnt2)) cp = dtmP->nullPnt ;
/*
** Get List Of DTM Features For Line
*/
 if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,tinPnt1,tinPnt2,&lineFeatures,&numLineFeatures) ) goto errexit ;
/*
** Write Line Features
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Line Features = %3ld",numLineFeatures) ;
    for( plf = lineFeatures ; plf < lineFeatures + numLineFeatures ; ++plf )
      {
       bcdtmWrite_message(0,0,0,"Feature[%3ld] ** Feature = %6ld Type =  %4ld Prior = %6ld Next = %6ld UserTag = %I64d",(long)(plf-lineFeatures),plf->dtmFeature,plf->dtmFeatureType,plf->priorPoint,plf->nextPoint,plf->userTag) ;
      }
    for( plf = lineFeatures ; plf < lineFeatures + numLineFeatures ; ++plf )
      {
       bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,plf->dtmFeature) ;
      }
   }
/*
** Perform Updates
*/
 switch ( deleteFlag )
   {
    case 1 :  /* Delete Line */
/*
**    Create Data Object For Void Polygon
*/
      if( bcdtmObject_createDtmObject(&voidPolygonP)) goto errexit ;
      bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidPolygonP,10,10) ;
/*
**    Place Polygon Around Line
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Polygon Around Line") ;
      if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,tinPnt1)->x,pointAddrP(dtmP,tinPnt1)->y,pointAddrP(dtmP,tinPnt1)->z)) goto errexit ;
      if( ap != dtmP->nullPnt ) if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z)) goto errexit ;
      if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,tinPnt2)->x,pointAddrP(dtmP,tinPnt2)->y,pointAddrP(dtmP,tinPnt2)->z)) goto errexit ;
      if( cp != dtmP->nullPnt ) if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z)) goto errexit ;
      if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,tinPnt1)->x,pointAddrP(dtmP,tinPnt1)->y,pointAddrP(dtmP,tinPnt1)->z)) goto errexit ;
/*
**    Get Cache Points
*/
      if( bcdtmLoad_getCachePoints(&polyPtsP,&numPolyPts)) goto errexit ;
/*
**    Store Poly Points In Void Polygon DTM
*/
      if( bcdtmObject_storeDtmFeatureInDtmObject(voidPolygonP,DTMFeatureType::Void,DTM_NULL_USER_TAG,1,&voidPolygonP->nullFeatureId,polyPtsP,numPolyPts)) goto errexit ;
/*
**    Insert Void Into Edit Tin
*/
      numDtmFeatures = dtmP->numFeatures ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Void Into Edit Tin") ;
      if( bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject(dtmP,voidPolygonP) ) goto errexit ;
      if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
**    Remove Inserted Voids If On Tin Hull
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
      if( bcdtmExtEdit_removeInsertedVoidsOnTinHullDtmObject(dtmP,numDtmFeatures)) goto errexit ;
      if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
    break ;

    case 2 : /* Swap Line */
      if( ap != dtmP->nullPnt && cp != dtmP->nullPnt )
        {
/*
**       Check Swap Line Is Not An Island,Void Or Hole Hull Line
*/
         swapLine = 1 ;
         for( plf = lineFeatures ; plf < lineFeatures + numLineFeatures ; ++plf )
           {
            if( plf->dtmFeatureType == DTMFeatureType::Island ||plf->dtmFeatureType == DTMFeatureType::Void || plf->dtmFeatureType == DTMFeatureType::Hole ) swapLine = 0 ;
           }
/*
**       Check Swap Line Can Be Swapped
*/
         if( swapLine )
           {
            if( bcdtmMath_pointSideOfDtmObject(dtmP,ap,cp,tinPnt1) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,ap,cp,tinPnt2) > 0 )
              {
/*
**             Break Dtm Features On Swap Line
*/
               if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features") ;
               if( bcdtmExtEdit_clipDtmFeaturesCoincidentWithTinLineDtmObject(dtmP,tinPnt1,tinPnt2) ) goto errexit ;
/*
**             Swap Line
*/
               if( dbg ) bcdtmWrite_message(0,0,0,"Swapping Line") ;
               if( bcdtmList_deleteLineDtmObject(dtmP,tinPnt1,tinPnt2)) goto errexit  ;
               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,cp,tinPnt2)) goto errexit  ;
               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,ap,tinPnt1)) goto errexit  ;
               *tinPnt3 = ap ;
               *tinPnt4 = cp ;
              }
           }
        }
    break ;

    default :
    break  ;
   }
    bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Check Tin Topology And Feature Topology
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( polyPtsP     != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; }
 if( lineFeatures != NULL ) { free(lineFeatures) ; lineFeatures = NULL ; }
 if( voidPolygonP != NULL ) bcdtmObject_destroyDtmObject(&voidPolygonP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Tin Line Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Tin Line Error") ;
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
int bcdtmExtEdit_polygonMoveZDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *polyPtsP,long numPolyPts,long moveOption,double elevation)
/*
** This Function Moves The z Values within A Polygon
*/
{
 int    ret=DTM_SUCCESS ;
 long   point ;
 double Xmin,Ymin,Xmax,Ymax ;
 DPoint3d    *p3dP ;
 DTM_TIN_POINT *pointP  ;
/*
** Get Bounding Rectangle For Polygon
*/
 Xmin = Xmax = polyPtsP->x ;
 Ymin = Ymax = polyPtsP->y ;
 for ( p3dP = polyPtsP + 1 ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
   {
    if( p3dP->x < Xmin ) Xmin = p3dP->x ;
    if( p3dP->x > Xmax ) Xmax = p3dP->x ;
    if( p3dP->y < Ymin ) Ymin = p3dP->y ;
    if( p3dP->y > Ymax ) Ymax = p3dP->y ;
   }
/*
** Scan And Modify Tin Points
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( nodeAddrP(dtmP,point)->cPtr != dtmP->nullPtr )
      {
       pointP = pointAddrP(dtmP,point);
       if( pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax )
         {
          if( bcdtmClip_pointInPointArrayPolygon(polyPtsP,numPolyPts,pointP->x,pointP->y))
            {
             if( moveOption == 1 ) pointP->z = pointP->z + elevation ;
             if( moveOption == 2 ) pointP->z = elevation ;
            }
         }
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
int bcdtmExtEdit_insertStringIntoDtmObject(BC_DTM_OBJ *dtmP,long drapeOption,wchar_t *stringFileNameP,long *startPntP)
/*
**
** drapeOption == 1  Drape On Tin Surface
**             == 2  Break On Tin Surface
**
** This Function Interactively Inserts a String Into an Existing Tin
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long numstringPtsP=0  ;
 DPoint3d  *p3dP,*stringPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin ** drapeOption = %2ld",drapeOption) ;
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
/*
** Read String Points
*/
 if( bcdtmRead_binaryFileP3D(stringFileNameP,&stringPtsP,&numstringPtsP)) goto errexit ;
/*
** Write String
*/
 if( dbg )
   {
    for( p3dP = stringPtsP ; p3dP < stringPtsP + numstringPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Edit String[%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Insert String
*/
 if( drapeOption == 1 ) if( bcdtmExtEdit_insertDtmFeatureIntoDtmObject(dtmP,DTMFeatureType::DrapeLine,dtmP->nullUserTag,stringPtsP,numstringPtsP,startPntP) )  goto errexit ;
 if( drapeOption == 2 ) if( bcdtmExtEdit_insertDtmFeatureIntoDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,stringPtsP,numstringPtsP,startPntP) )  goto errexit ;
/*
** Check Tin  -  Note The Check Tin Nulls The Tptr List
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Tin Valid") ;
   }
/*
** Write Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*startPntP) ;
/*
** Clean Up
*/
 cleanup :
 if( stringPtsP != NULL ) { free(stringPtsP) ; stringPtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin Error") ;
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
int bcdtmExtEdit_insertDtmFeatureIntoDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTMUserTag userTag,DPoint3d *stringPtsP,long numStringPts,long *startPntP)
/*
** This Function Inserts A DTM Feature Into A Tin Object
*/
{
 int ret=DTM_SUCCESS,dbg=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting DTM Edit Feature") ;
/*
** Initialise
*/
*startPntP = dtmP->nullPnt ;
/*
**  Insert Feature
*/
  switch ( dtmFeatureType )
    {
         case DTMFeatureType::Void :
       if( bcdtmExtEdit_insertVoidIntoDtmObject(dtmP,userTag,stringPtsP,numStringPts) ) goto errexit ;
         break ;

     case DTMFeatureType::DrapeLine :
     case DTMFeatureType::Breakline :
       if( bcdtmExtEdit_insertLineIntoDtmObject(dtmP,dtmFeatureType,userTag,stringPtsP,numStringPts,startPntP)) goto errexit ;
     break ;

         default :
         break   ;
        }
/*
** Write Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*startPntP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting DTM Edit Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting DTM Edit Feature Completed") ;
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
int bcdtmExtEdit_insertVoidIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *voidPtsP,long numVoidPts)
/*
** This Function Inserts A Void Into A Tin Object
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   Insert,startPnt,error ;
 DTMDirection direction;
 double area ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Void Into Tin Object") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"void.tin") ;
    bcdtmWrite_dataFileFromP3DArray(L"void.dat",voidPtsP,numVoidPts,DTMFeatureType::Void) ;
   }
/*
** Validate Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Void") ;
 if( bcdtmExtEdit_validateVoidDtmObject(dtmP,voidPtsP,&numVoidPts)) goto errexit ;
/*
** Store Feature In Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Void Into Tin") ;
 Insert = bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,voidPtsP,numVoidPts,&startPnt)  ;
 if( Insert == 1 ) goto errexit ;
 if( Insert == 2 )
   {
    bcdtmWrite_message(1,0,0,"Error Inserting Drape Void Hull") ;
    goto errexit ;
   }
/*
** Check Connectivity Of Inserted Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Void Connectivity") ;
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
/*
**  Get Void direction And If Clockwise Reverse direction
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Void direction") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt) ) goto errexit ;
/*
**  Check Void Does Not Intersect Other Voids Or Islands
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Void With Tin") ;
 if( ( error = bcdtmExtEdit_validateInsertVoidOrIslandDtmObject(dtmP,startPnt)) == 1 ) goto errexit ;
 if( error )
   {
    if( error == 2 ) { bcdtmWrite_message(1,0,0,"Void Intersects Existing Voids")   ;  }
    if( error == 3 ) { bcdtmWrite_message(1,0,0,"Void Intersects Existing Islands") ;  }
    goto errexit ;
   }
/*
** Add Feature To Tin
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Void To Tin") ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,userTag,dtmP->dtmFeatureIndex,startPnt,1)) goto errexit ;
    ++dtmP->dtmFeatureIndex ;
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
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
int bcdtmExtEdit_validateVoidDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *voidPtsP,long *numVoidPtsP)
/*
** This Function Validates A Void Polygon
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   ofs,numHullPts=0,intersectResult ;
 double Xi,Yi ;
 DPoint3d    *p3d1P,*p3d2P,*hullPtsP=NULL  ;
/*
** Write Status Message ** Developemnt Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Void") ;
/*
** Check For Closure
*/
 if( voidPtsP->x != (voidPtsP+*numVoidPtsP-1)->x  || voidPtsP->y != (voidPtsP+*numVoidPtsP-1)->y )
   {
    bcdtmWrite_message(1,0,0,"Void Does Not Close") ;
    goto errexit ;
   }
/*
** Eliminate Void Points Within The Point To Point Tolerance
*/
 for( p3d1P = voidPtsP , p3d2P = voidPtsP + 1 ; p3d2P < voidPtsP + *numVoidPtsP  ; ++p3d2P )
   {
    if( bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) > dtmP->ppTol )
      {
       ++p3d1P ;
       *p3d1P = *p3d2P ;
      }
   }
/*
** Set Number of Void Points
*/
 *numVoidPtsP = (long)(p3d1P-voidPtsP+1) ;
/*
** Check For More Than Three Points
*/
 if( *numVoidPtsP <= 3 ) { bcdtmWrite_message(1,0,0,"Three Or Less Points In Void Hull") ; goto errexit ; }
/*
** Check For Coincident Points On Void
*/
 for( p3d1P = voidPtsP ; p3d1P < voidPtsP + *numVoidPtsP - 1 ; ++p3d1P )
   {
    for( p3d2P = p3d1P + 1 ; p3d2P < voidPtsP + *numVoidPtsP - 1 ; ++p3d2P )
      {
       if( p3d1P->x == p3d2P->x && p3d1P->y == p3d2P->y )
         {
          bcdtmWrite_message(1,0,0,"Knot In Void") ;
          goto errexit ;
         }
      }
   }
/*
** Check For Crossing Void Lines
*/
 ofs = 2 ;
 for( p3d1P = voidPtsP ; p3d1P < voidPtsP + *numVoidPtsP - 2 ; ++p3d1P )
   {
    for( p3d2P = p3d1P + 2 ; p3d2P < voidPtsP + *numVoidPtsP - ofs ; ++p3d2P )
      {
           if(bcdtmMath_intersectCordLines(p3d1P->x,p3d1P->y,(p3d1P+1)->x,(p3d1P+1)->y,p3d2P->x,p3d2P->y,(p3d2P+1)->x,(p3d2P+1)->y,&Xi,&Yi))
         {
          bcdtmWrite_message(1,0,0,"Knot In Void") ;
          goto errexit ;
         }
      }
    ofs = 1 ;
   }
/*
** Extract Tin Hull From Tin Object
*/
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
** Check Void Is Totally Witn Tin Hull
*/
 bcdtmClip_checkPolygonsIntersect(hullPtsP,numHullPts,voidPtsP,*numVoidPtsP,&intersectResult) ;
 if( intersectResult != 3 )
   {
    bcdtmWrite_message(1,0,0,"Void External To Tin Hull") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
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
int bcdtmExtEdit_insertIslandIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *islandPtsP,long numIslandPts)
/*
** This Function Inserts A Island Into A Tin Object
*/
{
 int    ret=DTM_SUCCESS,dbg=0;
 long   Insert,startPnt,error ;
 DTMDirection direction;
 double area ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Island Into Tin Object") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"island.tin") ;
    bcdtmWrite_dataFileFromP3DArray(L"island.dat",islandPtsP,numIslandPts,DTMFeatureType::Island) ;
   }
/*
** Validate Island
*/
 if( bcdtmExtEdit_validateVoidDtmObject(dtmP,islandPtsP,&numIslandPts)) goto errexit ;
/*
** Store Feature In Tin
*/
 Insert = bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,islandPtsP,numIslandPts,&startPnt)  ;
 if( Insert == 1 ) goto errexit ;
 if( Insert == 2 )
   {
    bcdtmWrite_message(1,0,0,"Error Inserting Island Hull") ;
    goto errexit ;
   }
/*
** Check Connectivity Of Inserted Island
*/
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
/*
**  Get Island direction And If Clockwise Reverse direction
*/
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt) ) goto errexit ;
/*
**  Check Island Does Not Intersect Other Islands Or Islands
*/
 if( ( error = bcdtmExtEdit_validateInsertVoidOrIslandDtmObject(dtmP,startPnt)) == 1 ) goto errexit ;
 if( error )
   {
    if( error == 2 ) { bcdtmWrite_message(1,0,0,"Island Intersects Existing Voids") ;  }
    if( error == 3 ) { bcdtmWrite_message(1,0,0,"Island Intersects Existing Islands") ;  }
    goto errexit ;
   }
/*
** Add Feature To Tin
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Void To Tin") ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,userTag,dtmP->dtmFeatureIndex,startPnt,1)) goto errexit ;
    ++dtmP->dtmFeatureIndex ;
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
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_insertLineIntoDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTMUserTag userTag,DPoint3d *stringPtsP,long numStringPts,long *startPntP)
/*
** This Function Inserts A String Into A Tin Object
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   insert,drapeOption=0,insertOption=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line Into Tin ** dtmFeatureType = %2ld",dtmFeatureType) ;
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
 if( dtmFeatureType == DTMFeatureType::DrapeLine ) { drapeOption = 1 ; insertOption = 2 ; }
 if( dtmFeatureType == DTMFeatureType::Breakline ) { drapeOption = 2 ; insertOption = 2 ; }
/*
** Validate String
*/
 if( bcdtmExtEdit_validateStringDtmObject(dtmP,stringPtsP,&numStringPts)) goto errexit ;
/*
** Store Feature In Tin
*/
 insert = bcdtmInsert_internalStringIntoDtmObject(dtmP,drapeOption,insertOption,stringPtsP,numStringPts,startPntP)  ;
 if( insert == 1 ) goto errexit ;
 if( insert == 2 )
   {
    bcdtmWrite_message(1,0,0,"Error Inserting Line" ) ;
    goto errexit ;
   }
/*
** Check Connectivity Of Inserted String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Inserted String") ;
 if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,*startPntP,0)) goto errexit ;
/*
** Write Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*startPntP) ;
/*
** Retriangulate Along Tptr List
*/
 if( bcdtmExtEdit_retriangualteAlongTptrListDtmObject(dtmP,1,1,*startPntP) ) goto errexit ;
/*
** Add Feature To Tin
*/
 if( dtmFeatureType == DTMFeatureType::Breakline && *startPntP != dtmP->nullPnt )
   {
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Breakline,userTag,dtmP->dtmFeatureIndex,*startPntP,0)) goto errexit ;
    ++dtmP->dtmFeatureIndex ;
   }
/*
** Write Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*startPntP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Into Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Into Tin Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *startPntP = dtmP->nullPnt ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_validateStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long *numStringPtsP)
/*
** This Function Validates A String Polygon
*/
{
 int    ret=DTM_SUCCESS ;
 long   numHullPts=0,closeFlag ;
 double Xi,Yi ;
 DPoint3d    *p3d1P,*p3d2P,*hullPtsP=NULL  ;
/*
** Test For Closure
*/
 closeFlag = 0 ;
 p3d1P = stringPtsP ;
 p3d2P = stringPtsP + *numStringPtsP - 1 ;
 if( p3d1P->x == p3d2P->x && p3d1P->y == p3d2P->y ) closeFlag = 1 ;
/*
** Eliminate String Points Within The Point To Point Tolerance
*/
 for( p3d1P = stringPtsP , p3d2P = stringPtsP + 1 ; p3d2P < stringPtsP + *numStringPtsP  ; ++p3d2P )
   {
    if( bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) > dtmP->ppTol )
      {
       ++p3d1P ;
       *p3d1P = *p3d2P ;
      }
   }
/*
** Set Number of String Points
*/
 *numStringPtsP = (long)(p3d1P-stringPtsP+1) ;
/*
** Check For Coincident Points On String
*/
 for( p3d1P = stringPtsP ; p3d1P < stringPtsP + *numStringPtsP - 1 ; ++p3d1P )
   {
    for( p3d2P = p3d1P + 1 ; p3d2P < stringPtsP + *numStringPtsP - 1 ; ++p3d2P )
      {
       if( p3d1P->x == p3d2P->x && p3d1P->y == p3d2P->y  )
         {
          bcdtmWrite_message(1,0,0,"Knot In String") ;
          goto errexit ;
         }
      }
   }
/*
** Check For Crossing String Lines
*/
 for( p3d1P = stringPtsP ; p3d1P < stringPtsP + *numStringPtsP - 1 - closeFlag ; ++p3d1P )
   {
    for( p3d2P = p3d1P + 2 ; p3d2P < stringPtsP + *numStringPtsP - 1 - closeFlag ; ++p3d2P )
      {
           if(bcdtmMath_intersectCordLines(p3d1P->x,p3d1P->y,(p3d1P+1)->x,(p3d1P+1)->y,p3d2P->x,p3d2P->y,(p3d2P+1)->x,(p3d2P+1)->y,&Xi,&Yi))
         {
          bcdtmWrite_message(1,0,0,"Knot In String") ;
          goto errexit ;
         }
      }
   }
/*
** Extract Tin Hull From Tin Objects
*/
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
** Check For Coincident Points On String And Tin Hull
*/
 for( p3d1P = stringPtsP ; p3d1P < stringPtsP + *numStringPtsP - 1 ; ++p3d1P )
   {
    for( p3d2P = hullPtsP + 1 ; p3d2P < hullPtsP + numHullPts ; ++p3d2P )
      {
       if( p3d1P->x == p3d2P->x && p3d1P->y == p3d2P->y )
         {
          bcdtmWrite_message(1,0,0,"Coincident String And Hull Points") ;
          goto errexit ;
         }
      }
   }
/*
** Check For Intersecting String And Hull Lines
*/
 for( p3d1P = stringPtsP ; p3d1P < stringPtsP + *numStringPtsP - 2 ; ++p3d1P )
   {
    for( p3d2P = hullPtsP ; p3d2P < hullPtsP + numHullPts - 1 ; ++p3d2P )
      {
           if(bcdtmMath_intersectCordLines(p3d1P->x,p3d1P->y,(p3d1P+1)->x,(p3d1P+1)->y,p3d2P->x,p3d2P->y,(p3d2P+1)->x,(p3d2P+1)->y,&Xi,&Yi))
         {
          bcdtmWrite_message(1,0,0,"String Cuts Tin Hull") ;
          goto errexit ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
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
int bcdtmExtEdit_validateInsertVoidOrIslandDtmObject(BC_DTM_OBJ *dtmP,long startPoint)
/*
** This Test If The Void Or Island To Be Inserted Intersects With Existing Voids , Island Or Holes
**
** Return Values  ==  0  No Intersections
**                ==  1  System Error Terminate
**                ==  2  Intersects With Existing Voids
**                ==  3  Intersects With Existing Islands
**                ==  4  Void Internal To Existing Void
*/
{
 int   ret=DTM_SUCCESS ;
 long  sp, listEntry;
 DTMFeatureType dtmFeatureType;
/*
** Initialise
*/
 sp = startPoint ;
 if( nodeAddrP(dtmP,sp)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Tptr List And Check For Intersections
*/
 do
   {
    if( ( listEntry = nodeAddrP(dtmP,sp)->fPtr) != dtmP->nullPtr )
      {
       while ( listEntry != dtmP->nullPtr )
         {
          dtmFeatureType = ftableAddrP(dtmP,flistAddrP(dtmP,listEntry)->dtmFeature)->dtmFeatureType ;
          if( dtmFeatureType == DTMFeatureType::Void   ) return(2) ;
          else if( dtmFeatureType == DTMFeatureType::Hole   ) return(2) ;
          else if( dtmFeatureType == DTMFeatureType::Island ) return(3) ;
          listEntry = flistAddrP(dtmP,listEntry)->nextPtr ;
         }
      }
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint ) ;
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
int bcdtmExtEdit_retriangualteAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long leftSide,long rightSide,long firstPoint)
/*
** This Function Retriangulates Along The Temporary Pointer List
*/
{
 int   ret=DTM_SUCCESS ;
 long  sp,np,ap,ap1,cp,cp1 ;
/*
** Check For Valid Start Point
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints ) goto errexit ;
/*
** Only Process If TPtr List Exists For First Point
*/
 if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt )
   {
/*
**  Check Left Side
*/
    if( leftSide )
      {
       sp = firstPoint ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
/*
**     Scan Along Tptr List And Check MAX_MIN Criteria
*/
        while ( np != dtmP->nullPnt && np != firstPoint )
         {
/*
**        Check Line Anti Clockwise
*/
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
            {
             if( bcdtmList_testLineDtmObject(dtmP,sp,ap) )
               {
                if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,ap) )
                  {
                   if(( ap1 = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                   if( bcdtmList_testLineDtmObject(dtmP,ap,ap1) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,np,ap1,ap) < 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,np,ap1,sp) > 0      )
                        {
                         if( bcdtmTin_maxMinTestDtmObject(dtmP,np,ap1,sp,ap) )
                           {
                            if( bcdtmList_deleteLineDtmObject(dtmP,sp,ap)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,ap1,sp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap1,np,ap)) goto errexit ;
                           }
                        }
                     }
                  }
               }
            }
          sp = np ;
          np = nodeAddrP(dtmP,np)->tPtr ;
         }
      }
/*
**  Check Right Side
*/
    if( rightSide )
      {
       sp = firstPoint ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
/*
**     Scan Along Tptr List And Check MAX_MIN Criteria
*/
        while ( np != dtmP->nullPnt && np != firstPoint )
         {
/*
**        Check Line Clockwise
*/
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
            {
             if( bcdtmList_testLineDtmObject(dtmP,sp,cp) )
               {
                if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,cp) )
                  {
                   if(( cp1 = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
                   if( bcdtmList_testLineDtmObject(dtmP,cp,cp1) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,np,cp1,cp) > 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,np,cp1,sp) < 0      )
                        {
                         if( bcdtmTin_maxMinTestDtmObject(dtmP,np,cp1,sp,cp) )
                           {
                            if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,cp1,cp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp1,np,sp)) goto errexit ;
                           }
                        }
                     }
                  }
               }
            }
          sp = np ;
          np = nodeAddrP(dtmP,np)->tPtr ;
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
int bcdtmExtEdit_tptrMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation)
/*
** This Function Moves The z Values On A Tptr List
*/
{
 long   sp ;
/*
** Scan Tptr List And Move z Value
*/
 sp = startPoint ;
 do
   {
        if( moveOption == 1 ) pointAddrP(dtmP,sp)->z = pointAddrP(dtmP,sp)->z + elevation ;
        if( moveOption == 2 ) pointAddrP(dtmP,sp)->z = elevation ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint && sp != dtmP->nullPnt ) ;
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
int bcdtmExtEdit_tptrPolygonMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation)
/*
** This Function Moves The z Values within A Tptr Polygon
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   sp,clc,cp,np,point,process,nullValue1=-98989898,nullValue2 =-87878787 ;
 DTMDirection direction;
 double area ;
 DTM_TIN_POINT  *pointP ;
 DTM_TIN_NODE   *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Moving Internal Tptr Polygon Points") ;
/*
** Check Direction Of Tptr Polygon And If Clockwise
** Set Direction Anti Clockwise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction tPtr Polygon") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ) goto errexit ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPoint)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Clip Polygon") ;
 sp = startPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt ) nodeAddrP(dtmP,cp)->tPtr = nullValue1 ;
       if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
      }
    sp = np ;
   } while ( sp != startPoint ) ;
/*
** Mark All Points Connecting to Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Mark Points") ;
 process = 1 ;
 while( process )
   {
    process = 0 ;
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == nullValue1 )
             {
              clc = nodeAddrP(dtmP,sp)->cPtr ;
              while ( clc != dtmP->nullPtr )
                {
                 cp  = clistAddrP(dtmP,clc)->pntNum ;
                 clc = clistAddrP(dtmP,clc)->nextPtr ;
                 if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt )
                   {
                        nodeAddrP(dtmP,cp)->tPtr = nullValue1 ;
                        process = 1 ;
                   }
                }
              nodeAddrP(dtmP,sp)->tPtr = nullValue2 ;
             }
      }
   }
/*
** Scan Points And Move z Value
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
    if( nodeP->cPtr != dtmP->nullPtr && nodeP->tPtr == nullValue2  )
      {
       pointP = pointAddrP(dtmP,point) ;
       if( moveOption == 1 ) pointP->z = pointP->z + elevation ;
       if( moveOption == 2 ) pointP->z = elevation ;
       nodeP->tPtr = dtmP->nullPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Internal Tptr Polygon Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Internal Tptr Polygon Points Error") ;
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
int bcdtmExtEdit_nullTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Nulls Out The Tptr List
*/
{
 long sp,np ;
/*
** Initilaise
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) return(DTM_SUCCESS) ;
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
    sp = np ;
   } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
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
int bcdtmExtEdit_removeInternalTptrPointsAndRetriangulateDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long fillOption)
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   sp,np,cp,lp,process,clc,nullVal1=-98989898,nullVal2 =-87878787 ;
 DTMDirection direction;
 double area ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin Completed") ;
/*
** Check Direction Of Tptr Polygon And If Clockwise
** Set Direction Anti Clockwise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction tPtr Polygon") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Clip Polygon") ;
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt ) nodeAddrP(dtmP,cp)->tPtr = nullVal1 ;
       if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
      }
    sp = np ;
   } while ( sp != startPnt ) ;
/*
** Mark All Points Connecting to Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Mark Points") ;
 process = 1 ;
 while( process )
   {
    process = 0 ;
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == nullVal1 )
             {
              clc = nodeAddrP(dtmP,sp)->cPtr ;
              while ( clc != dtmP->nullPtr )
                {
                 cp  = clistAddrP(dtmP,clc)->pntNum ;
                 clc = clistAddrP(dtmP,clc)->nextPtr ;
                 if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt )
                   {
                        nodeAddrP(dtmP,cp)->tPtr = nullVal1 ;
                        process = 1 ;
                   }
                }
              nodeAddrP(dtmP,sp)->tPtr = nullVal2 ;
             }
      }
   }
/*
** Clip Dtm Features Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Features Internal To Clip Polygon")  ;
 if( bcdtmClip_dtmFeaturesInternalToTptrPolygonDtmObject(dtmP,startPnt,nullVal2)) goto errexit ;
/*
** Delete All Lines Connecting To Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Connecting To Internal Points") ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->tPtr == nullVal2 )
      {
       clc = nodeAddrP(dtmP,sp)->cPtr ;
       while ( clc != dtmP->nullPtr )
             {
              cp  = clistAddrP(dtmP,clc)->pntNum ;
              clc = clistAddrP(dtmP,clc)->nextPtr ;
              if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp) ) goto errexit ;
             }
       nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,sp)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,sp)->fPtr = dtmP->nullPtr ;
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
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
       if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit ;
       cp = lp ;
      }
    sp = np ;
   } while( sp != startPnt ) ;
/*
** Fill Tptr Polygon With Triangles Insert As Void And
** Resolve Adjoining Ajoining Polygonal Features
*/
 if( fillOption )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Filling Tptr Polygon With Triangles")  ;
    if( bcdtmClip_fillTptrPolygonWithTrianglesDtmObject(dtmP,startPnt)) goto errexit ;
    if( fillOption == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Clip Void")  ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,0,dtmP->numFeatures,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,startPnt,0)) goto errexit ;
       ++dtmP->dtmFeatureIndex ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Adjoining Polygonal Features")  ;
       if( bcdtmClip_findAndMergeAdjoiningVoidsDtmObject(dtmP,dtmP->numFeatures-1)) goto errexit ;
      }
   }
/*
** Set Convex Hull
*/
 bcdtmList_setConvexHullDtmObject(dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Edit String Into Tin Error") ;
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
int bcdtmExtEdit_drapeDeleteLineOnEditDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts,DPoint3d **drapePointsPP,long *numDrapePointsP)
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  ofs1,ofs2,numDrapePts=0 ;
 DPoint3d   *p3dP ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping Delete Line On Edit Tin") ;
/*
** Drape Delete Line On Tin
*/
 if( bcdtmDrape_stringDtmObject(dtmP,deleteLinePtsP,numDeleteLinePts,false,&drapePtsP,&numDrapePts)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points = %4ld",numDrapePts) ;
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )
      {
       bcdtmWrite_message(0,0,0,"Drape Point [%4ld] ** Line = %2ld Type = %2ld ** %10.4lf %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeLine,drapeP->drapeType,drapeP->drapeX,drapeP->drapeY,drapeP->drapeZ) ;
      }
   }
/*
** Alloacte Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Drape Points") ;
 *numDrapePointsP = numDrapePts ;
 *drapePointsPP = ( DPoint3d  * ) malloc(*numDrapePointsP * sizeof(DPoint3d)) ;
 if( *drapePointsPP == NULL )
  {
   bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
   goto errexit ;
  }
/*
** Copy The Drape Point Coordinates
*/
 for( p3dP = *drapePointsPP , drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++p3dP , ++drapeP )
   {
    p3dP->x = drapeP->drapeX ;
    p3dP->y = drapeP->drapeY ;
    p3dP->z = drapeP->drapeZ ;
   }
/*
** Remove Duplicate Drape Points
*/
 for( ofs1 = 0 , ofs2 = 1 ; ofs2 <  *numDrapePointsP ; ++ofs2 )
   {
    if( (*drapePointsPP+ofs1)->x != (*drapePointsPP+ofs2)->x || (*drapePointsPP+ofs1)->y != (*drapePointsPP+ofs2)->y )
      {
       ++ofs1 ;
       if( ofs1 != ofs2 )
         {
          (*drapePointsPP+ofs1)->x = (*drapePointsPP+ofs2)->x  ;
          (*drapePointsPP+ofs1)->y = (*drapePointsPP+ofs2)->y  ;
          (*drapePointsPP+ofs1)->z = (*drapePointsPP+ofs2)->z  ;
         }
      }
   }
/*
** Reset Number Of Coords
*/
 *numDrapePointsP = ofs1 + 1 ;
/*
** Reassign Memory
*/
 if( *numDrapePointsP != numDrapePts ) *drapePointsPP = ( DPoint3d * ) realloc ( *drapePointsPP , *numDrapePointsP * sizeof(DPoint3d)) ;
/*
** Write Out Drape Coordinates ** Development Only
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points After Duplicate Removal = %4ld",*numDrapePointsP) ;
    for( p3dP = *drapePointsPP ; p3dP < *drapePointsPP + *numDrapePointsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Drape Point [%4ld] ** %10.4lf %10.4lf %10.4lf",(long)(p3dP-*drapePointsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Draping Delete Line On Edit Tin Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Draping Delete Line On Edit Tin Error") ;
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
int bcdtmExtEdit_checkForVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *voidsFoundP )
/*
** This Function Scans Along A Tptr List Looking For Voids
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,np,voidLine ;
/*
** Initialise
*/
 *voidsFoundP = 0 ;
 if( startPnt >= 0 && startPnt < dtmP->numPoints )
   {
    if( nodeAddrP(dtmP,startPnt)->tPtr >= 0 && nodeAddrP(dtmP,startPnt)->tPtr < dtmP->numPoints )
      {
/*
**     Scan List Looking For Void Lines
*/
       sp = startPnt ;
       do
         {
          np = nodeAddrP(dtmP,sp)->tPtr ;
          if( np != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine)) goto errexit ;
             if( voidLine ) *voidsFoundP = 1 ;
            }
          sp = np ;
        } while ( sp != dtmP->nullPnt && sp != startPnt && ! *voidsFoundP ) ;
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
int bcdtmExtEdit_removeVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPnt,double contourInterval,void *userP )
/*
** This Function Scans Along A Tptr List Looking For Voids
*/
{
 int   ret=DTM_SUCCESS ;
 long  n,sp,np,*pl,spnt,voidLine,voidFeature,*islandsP=NULL,numIslands=0,*pointListP=NULL ;
/*
** Initialise
*/
 if( startPnt >= 0 && startPnt < dtmP->numPoints )
   {
    if( nodeAddrP(dtmP,startPnt)->tPtr >= 0 && nodeAddrP(dtmP,startPnt)->tPtr < dtmP->numPoints )
      {
/*
**     Copy Tptr List To Pointer List
*/
       if( bcdtmList_copyTptrValuesToPointListDtmObject(dtmP,&pointListP) ) goto errexit ;
/*
**     Scan List Looking For Void Lines
*/
       sp = startPnt ;
       do
         {
          np = *(pointListP+sp) ;
          if( np != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,&voidLine)) goto errexit ;
             if( voidLine )
               {
/*
**              Find Enclosing Void
*/
                if( bcdtmExtEdit_findDtmFeatureTypeEnclosingPointDtmObject(dtmP,sp,DTMFeatureType::Void,&voidFeature)) {  free(pointListP) ; goto errexit ; }
/*
**              Get Island Features Internal To Void
*/
                if( bcdtmExtEdit_getIslandsInternalToVoidDtmObject(dtmP,voidFeature,&islandsP,&numIslands)) goto errexit ;
/*
**              Clear Void Bits
*/
                if( bcdtmExtEdit_clearVoidBitOfInternalVoidPointsDtmObject(dtmP,voidFeature,islandsP,numIslands))  goto errexit ;
/*
**              Erase Void And Island Hulls
*/
                if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
                if( bcdtmExtEdit_drawTptrLineFeaturesDtmObject(dtmP,loadFunctionP,2,spnt,userP)) goto errexit ;
                for( n = 0 ; n < numIslands ; ++n )
                  {
                   if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,*(islandsP+n),&spnt)) goto errexit ;
                   if( bcdtmExtEdit_drawTptrLineFeaturesDtmObject(dtmP,loadFunctionP,2,spnt,userP)) goto errexit ;
                  }
/*
**              Delete Void Features
*/
                if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&spnt)) goto errexit ;
                if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,voidFeature)) goto errexit ;
                for( n = 0 ; n < numIslands ; ++n ) if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,*(islandsP+n)) ) goto errexit ;
/*
**              Draw All features Internal To Tptr Hull
*/
                bcdtmExtEdit_drawInternalTptrFeaturesDtmObject(dtmP,loadFunctionP,3,spnt,contourInterval,userP) ;
/*
**              Free islandsP memory
*/
                if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
               }
            }
          sp = np ;
         } while ( sp != dtmP->nullPnt && sp != startPnt ) ;
      }
   }
/*
** Restore Tptr List
*/
 for( spnt = 0 , pl = pointListP ; spnt < dtmP->numPoints ; ++spnt , ++pl ) nodeAddrP(dtmP,spnt)->tPtr = *pl ;
/*
** Clean Up
*/
 cleanup :
 if( islandsP   != NULL ) free(islandsP) ;
 if( pointListP != NULL ) free(pointListP) ;
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
int bcdtmExtEdit_clearVoidBitOfInternalVoidPointsDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandsP, long numIslands)
/*
** This Function Removes Internal Void Points And Lines
*/
{
 int   ret=DTM_SUCCESS ;
 long  n,sp,np,pp,lp,fpnt,clc,nvp,process,islandFeature  ;
/*
** Null Out Tptr Values
*/
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Scan Around Void And Mark Internal Void Points
*/
 process = nvp = 0 ;
 fpnt = sp = ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ;
 bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&pp) ;
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&np) ;
 do
   {
    if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( lp != pp  )
      {
       if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
         {
          if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD))
            {
             nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
             ++nvp ;
             process = 1 ;
            }
         }
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
      }
    pp = sp ; sp = np ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sp,&np) ;
   } while ( sp != fpnt ) ;
/*
** Scan Around Islands And Mark External Void Points
*/
 for( np = 0 ; np < numIslands ; ++ np )
   {
    islandFeature = *(islandsP+np) ;
    fpnt = sp = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;  ;
    if( bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&pp)) goto errexit ;
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np))  goto errexit ;
    do
      {
       if( ( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
       while ( lp != pp  )
         {
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD))
               {
                nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
                ++nvp ;
                process = 1 ;
               }
            }
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
         }
       pp = sp ; sp = np ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
      } while ( sp != fpnt ) ;
   }
/*
** Sequentially Scan Points
*/
 while( process )
   {
    process = 0 ;
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == -dtmP->nullPnt )
         {
          clc = nodeAddrP(dtmP,sp)->cPtr ;
          while ( clc != dtmP->nullPtr  )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD)) nodeAddrP(dtmP,lp)->tPtr = -dtmP->nullPnt ;
                process = 1 ;
                ++nvp ;
               }
            }
          nodeAddrP(dtmP,sp)->tPtr = -dtmP->nullPnt + 10 ;
         }
      }
   }
/*
** Clear Void Bit
*/
 if( nvp )
   {
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr < 0 )
         {
          bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ;
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
         }
      }
   }
/*
** Scan Around islandsP And Mark External Void Points
*/
 for( n = 0 ; n < numIslands ; ++n )
   {
    islandFeature = *(islandsP+n) ;
    fpnt = sp = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
    bcdtmExtList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&pp) ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
    do
      {
       if( ( lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
       while ( lp != pp  )
         {
          bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,lp)->PCWD) ;
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
         }
       pp = sp ; sp = np ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np) ;
      } while ( sp != fpnt ) ;
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
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
int bcdtmExtEdit_deleteTrianglesOnDeleteLineDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts)
/*
** This Is The Controlling Function For Deleting Triangles On A Delete Line
**
** Rob Cormack June 2003
*/
{
 int         ret=DTM_SUCCESS,dbg=0 ;
 long        numDrapePts,numDrapeTinLines,numDtmFeatures ;
 DPoint3d         *p3dP,*drapePtsP=NULL ;
 BC_DTM_OBJ  *voidPolygonsP=NULL ;
 DTM_TRG_INDEX_TABLE *drapeTinLinesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Triangles On Delete Line") ;
    bcdtmWrite_message(0,0,0,"Tin              =  %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"deleteLinePtsP   =  %p",deleteLinePtsP) ;
    bcdtmWrite_message(0,0,0,"numDeleteLinePts =  %4ld",numDeleteLinePts) ;
    for( p3dP = deleteLinePtsP ; p3dP < deleteLinePtsP + numDeleteLinePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld]  = %10.4lf %10.4lf",(long)(p3dP-deleteLinePtsP),p3dP->x,p3dP->y) ;
      }
   }
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Initialise
*/
 numDtmFeatures = dtmP->numFeatures ;
/*
** Drape Delete line On Edit Tin And Get Drape Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping Delete Line On Edit Tin") ;
 if( bcdtmExtEdit_drapeDeleteLineOnEditDtmObject(dtmP,deleteLinePtsP,numDeleteLinePts,&drapePtsP,&numDrapePts) ) goto errexit ;
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Assign Tin Lines To Drape Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Tin Lines To Drape Points") ;
 if( bcdtmExtEdit_assignTinLinesToDrapePointsDtmObject(dtmP,drapePtsP,numDrapePts,&drapeTinLinesP,&numDrapeTinLines) ) goto errexit ;
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Create Void And Island Polygons For Deleted Triangles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Void And Island Polygons For Deleted Triangles") ;
 if( bcdtmExtEdit_createVoidAndIslandPolygonsForDeletedTrianglesDtmObject(dtmP,drapeTinLinesP,numDrapePts,&voidPolygonsP) ) goto errexit ;
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Insert Void And Island Polygons Into Edit Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Edit Tin") ;
 if( bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject(dtmP,voidPolygonsP) ) goto errexit ;
 if( dbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Remove Inserted Voids If On Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
 if( bcdtmExtEdit_removeInsertedVoidsOnTinHullDtmObject(dtmP,numDtmFeatures)) goto errexit ;
 if( dbg ) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,dbg) ;
/*
** Check Tin Topology And Feature Topology
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Tin Topolgy Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg)) { bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
    else                                                     bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP      != NULL ) { free(drapePtsP)      ; drapePtsP      = NULL ; }
 if( drapeTinLinesP != NULL ) { free(drapeTinLinesP) ; drapeTinLinesP = NULL ; }
 if( voidPolygonsP  != NULL ) bcdtmObject_destroyDtmObject(&voidPolygonsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Triangles On Delete Line Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Triangles On Delete Line Error") ;
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
int bcdtmExtEdit_assignTinLinesToDrapePointsDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *drapePtsP,long numDrapePts,DTM_TRG_INDEX_TABLE **drapeTinLinesP,long *numDrapeTinLines)
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   P1,P2,P3,Ptype ;
 double n1,n2,n3,z ;
 DPoint3d    *p3dP ;
 DTM_TRG_INDEX_TABLE *polyPtsP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Tin Lines To Intersect Points") ;
/*
** Allocate Memory To Drape Tin Lines
*/
 *numDrapeTinLines = numDrapePts ;
 if( *drapeTinLinesP != NULL ) { free(*drapeTinLinesP) ; *drapeTinLinesP = NULL ; }
 *drapeTinLinesP = ( DTM_TRG_INDEX_TABLE * ) malloc ( *numDrapeTinLines * sizeof(DTM_TRG_INDEX_TABLE)) ;
 if( *drapeTinLinesP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Drape Points
*/
 for( p3dP = drapePtsP , polyPtsP = *drapeTinLinesP ; p3dP < drapePtsP + numDrapePts ; ++p3dP , ++polyPtsP )
   {
/*
**  Find Triangle
*/
    if( bcdtmFind_triangleForPointDtmObject(dtmP,p3dP->x,p3dP->y,&z,&Ptype,&P1,&P2,&P3 ) ) goto errexit ;
/*
**  If Point In Triangle Check Proximity To Tin Points
*/
    if( Ptype == 4 )
      {
       if     ( bcdtmMath_distance(p3dP->x,p3dP->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y ) <= dtmP->mppTol ) { Ptype = 1 ; P2 = P3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(p3dP->x,p3dP->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y ) <= dtmP->mppTol ) { Ptype = 1 ; P1 = P2 ; P2 = P3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(p3dP->x,p3dP->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y ) <= dtmP->mppTol ) { Ptype = 1 ; P1 = P3 ; P2 = P3 = dtmP->nullPnt ; }
      }
/*
**  If Point In Triangle Check Proximity To Tin Lines
*/
    if( Ptype == 4 )
      {
       n1 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,p3dP->x,p3dP->y) ;
       n2 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,p3dP->x,p3dP->y) ;
       n3 = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,p3dP->x,p3dP->y) ;
       if     ( n1 <= n2 && n1 <= n3 && n1 <= dtmP->mppTol ) { Ptype = 2 ; P3 = dtmP->nullPnt ; }
       else if( n2 <= n3 && n2 <= n1 && n2 <= dtmP->mppTol ) { Ptype = 2 ; P1 = P2 ; P2 = P3 ; P3 = dtmP->nullPnt ; }
       else if( n3 <= n1 && n3 <= n2 && n3 <= dtmP->mppTol ) { Ptype = 2 ; P2 = P1 ; P1 = P3 ; P3 = dtmP->nullPnt ; }
       if( Ptype == 2 )
         {
          if( nodeAddrP(dtmP,P1)->hPtr == P2 )   Ptype = 3 ;
          if( nodeAddrP(dtmP,P2)->hPtr == P1 ) { Ptype = 3 ; P3 = P1 ; P1 = P2 ; P2 = P3 ; P3 = dtmP->nullPnt ; }
         }
      }
/*
** Store Points
*/
    polyPtsP->index  = Ptype  ;
    polyPtsP->trgPnt1 = P1    ;
    polyPtsP->trgPnt2 = P2    ;
    polyPtsP->trgPnt3 = P3    ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Drape Point[%4ld] ** Type = %2ld  %9ld %9ld %9ld",(long)(polyPtsP-*drapeTinLinesP),polyPtsP->index,polyPtsP->trgPnt1,polyPtsP->trgPnt2,polyPtsP->trgPnt3) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Tin Lines To Intersect Points Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Assigning Tin Lines To Intersect Points Error") ;
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
int bcdtmExtEdit_createVoidAndIslandPolygonsForDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,DTM_TRG_INDEX_TABLE *delPointsP,long numDelPoints,BC_DTM_OBJ **voidPolygonsPP)
{
 int  ret=DTM_SUCCESS,dbg=0    ;
 long p1=0,p2,p3,clc,numVoidPts,msgLevel ;
 DPoint3d  *voidPtsP=NULL ;
 BC_DTM_OBJ *voidDtmP=NULL ;
 DTM_TRG_INDEX_TABLE *polyPtsP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Void And Island Polygons For Deleted Triangles") ;
/*
** Initialise
*/
 msgLevel = DTM_MSG_LEVEL ;
 if( *voidPolygonsPP != NULL ) bcdtmObject_destroyDtmObject(voidPolygonsPP) ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&voidDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidDtmP,numDelPoints*3,numDelPoints*3) ;
/*
** Write Deleted Triangles To Data Object
*/
 for( polyPtsP = delPointsP ; polyPtsP < delPointsP + numDelPoints ; ++polyPtsP )
   {
    switch ( polyPtsP->index )
      {
       case   1  :   /* Delete Line Passes Through Triangle Vertex */
/*
**     Scan Point And Write Triangles
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Triangles For Point = %6ld",polyPtsP->trgPnt1) ;
       clc = nodeAddrP(dtmP,polyPtsP->trgPnt1)->cPtr ;
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,polyPtsP->index,p2)) < 0 ) goto errexit ;
       while ( clc != dtmP->nullPtr )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,p3)->hPtr != p1 )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
             if( bcdtmLoad_getCachePoints(&voidPtsP,&numVoidPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&voidDtmP->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;

//             if( bcdtmObject_storePointInDtmObject(voidDtmP,2,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
            }
          p2 = p3 ;
         }
       break  ;

       case  2  :     /* Delete Line Passes Through Triangle Edge  */
       case  3  :
/*
**     Write Triangles On Either Side Of Line
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Triangles For Line = %6ld %6ld",polyPtsP->trgPnt1,polyPtsP->trgPnt2) ;
          if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,polyPtsP->trgPnt1,polyPtsP->trgPnt2))   < 0 ) goto errexit ;
          if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,polyPtsP->trgPnt1,polyPtsP->trgPnt2)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,p1,polyPtsP->trgPnt2)) p1 = dtmP->nullPnt ;
          if( ! bcdtmList_testLineDtmObject(dtmP,p2,polyPtsP->trgPnt2)) p2 = dtmP->nullPnt ;
          if( p1 != dtmP->nullPnt )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z )) goto errexit ;
             if( bcdtmLoad_getCachePoints(&voidPtsP,&numVoidPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&voidDtmP->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;

//             if( bcdtmObject_storePointInDtmObject(voidDtmP,2,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z )) goto errexit ;
            }
          if( p2 != dtmP->nullPnt )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z )) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
             if( bcdtmLoad_getCachePoints(&voidPtsP,&numVoidPts)) goto errexit ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&voidDtmP->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;

//             if( bcdtmObject_storePointInDtmObject(voidDtmP,2,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z )) goto errexit ;
//             if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
            }
       break ;

       case  4 :    /* Start Or End Point Of Line Internal To Triangle */
          if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Triangle = %6ld %6ld %6ld",polyPtsP->trgPnt1,polyPtsP->trgPnt2,polyPtsP->trgPnt3) ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt3)->x,pointAddrP(dtmP,polyPtsP->trgPnt3)->y,pointAddrP(dtmP,polyPtsP->trgPnt3)->z )) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
          if( bcdtmLoad_getCachePoints(&voidPtsP,&numVoidPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&voidDtmP->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;

//          if( bcdtmObject_storePointInDtmObject(voidDtmP,2,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
//          if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt2)->x,pointAddrP(dtmP,polyPtsP->trgPnt2)->y,pointAddrP(dtmP,polyPtsP->trgPnt2)->z )) goto errexit ;
//          if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt3)->x,pointAddrP(dtmP,polyPtsP->trgPnt3)->y,pointAddrP(dtmP,polyPtsP->trgPnt3)->z )) goto errexit ;
//          if( bcdtmObject_storePointInDtmObject(voidDtmP,3,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,polyPtsP->trgPnt1)->x,pointAddrP(dtmP,polyPtsP->trgPnt1)->y,pointAddrP(dtmP,polyPtsP->trgPnt1)->z )) goto errexit ;
       break   ;
      } ;
   }
/*
** Write Data Object ** Development Only
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(voidDtmP,L"deletedTriangles.dat") ;
/*
** Triangulate Data Object
*/
 DTM_MSG_LEVEL         = - 1 ;
 DTM_NORMALISE_OPTION  = false ;             // To Inhibit Normalisation Of Coordinates
 DTM_DUPLICATE_OPTION = false ;             // To Inhibit Removal Of None Identical Points
 voidDtmP->ppTol = voidDtmP->plTol = 0.0 ;
 if( bcdtmObject_createTinDtmObject(voidDtmP,1,0.0)) goto errexit ;
 DTM_MSG_LEVEL         = msgLevel ;
 DTM_NORMALISE_OPTION  = true ;
 DTM_DUPLICATE_OPTION = true ;
/*
** Write Tin Object ** Development Only
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(voidDtmP,L"deletedTriangles.tin") ;
/*
** Extract Void Polygons From Tin
*/
 if( bcdtmExtEdit_extractVoidAndIslandPolygonsFromDeletedTrianglesDtmObject(voidDtmP,voidPolygonsPP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 DTM_MSG_LEVEL = msgLevel ;
 if( voidPtsP != NULL ) { free(voidPtsP) ; voidPtsP = NULL ; }
 if( voidDtmP != NULL ) bcdtmObject_destroyDtmObject(&voidDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Void And Island Polygons For Deleted Triangles Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Creating Void And Island Polygons For Deleted Triangles Error") ;
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
int bcdtmExtEdit_extractVoidAndIslandPolygonsFromDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **voidPolygonsPP)
{
 int     ret=DTM_SUCCESS,dbg=0 ;
 long    sp,np,p1,p2,p3,clc,numStartFeatures,numIslands,numVoids ;
 long    feature,numVoidPts ;
 DTMFeatureType dtmFeatureType;
 DPoint3d     *voidPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Void Polygons From Deleted Triangles Tin") ;
/*
** Initialise
*/
 numStartFeatures = dtmP->numFeatures ;
 if( bcdtmObject_createDtmObject(voidPolygonsPP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(*voidPolygonsPP,dtmP->numTriangles,dtmP->numTriangles) ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
/*
** Scan Tin Hull And Look For Start Of Void Polygon
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Scanning Tin Hull For Void Polygons") ;
 numVoids = 0 ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
/*
**  Test For Hull Break Line
*/
    if( bcdtmList_testForBreakLineDtmObject(dtmP,sp,np) )
      {
/*
**     Test For Start Of New Void Polygon
*/
       if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,np)->sPtr == dtmP->nullPnt )
         {
/*
**        Extract Void Polygon
*/
          nodeAddrP(dtmP,sp)->tPtr = np ;
          nodeAddrP(dtmP,sp)->sPtr = np ;
          p1 = sp ;
          p2 = np ;
/*
**       Scan Around Exterior Of Break Lines
*/
          do
            {
             p3 = p1 ;
             do
               {
                if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
               }  while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,p2,p3) ) ;
             nodeAddrP(dtmP,p2)->tPtr = p3 ;
             nodeAddrP(dtmP,p2)->sPtr = p3 ;
             p1 = p2 ;
             p2 = p3 ;
            } while ( p3 != sp  ) ;
/*
**        Store Void In Tin Object
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,sp,1)) goto errexit ;
          ++dtmP->dtmFeatureIndex ;
          ++numVoids ;
         }
      }
/*
** Reset For Next Hull Line
*/
    sp = np ;
   } while ( sp != dtmP->hullPoint ) ;
/*
** Scan Internal Lines And Extract islandsP
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Scanning Internal Tin Lines For Island Polygons") ;
 numIslands = 0 ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    while( clc != dtmP->nullPtr )
      {
       np  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( sp < np  )
         {
/*
**        Test For Internal Line
*/
          if( nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp )
            {
/*
**           Test For Not A Break Line
*/
             if( ! bcdtmList_testForBreakLineDtmObject(dtmP,sp,np) )
               {
/*
**              Scan To Start Line Of Island
*/
                p1 = np ;
                do
                  {
                   if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,sp,p1)) < 0 ) goto errexit ;
                  } while ( p1 != np && ! bcdtmList_testForBreakLineDtmObject(dtmP,sp,p1) ) ;
                np = p1 ;
/*
**              Test For Start Of New Island Polygon
*/
                if( bcdtmList_testForBreakLineDtmObject(dtmP,sp,np) )
                  {
                   if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,np)->sPtr == dtmP->nullPnt )
                     {
/*
**                    Extract Island Polygon
*/
                      nodeAddrP(dtmP,sp)->sPtr = np ;
                      nodeAddrP(dtmP,sp)->tPtr = np ;
/*
**                    Scan Around Interior Of Break Lines
*/
                      p1 = sp ;
                      p2 = np ;
                      do
                        {
                         p3 = p1 ;
                         do
                           {
                            if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
                           }  while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,p2,p3) ) ;
                         nodeAddrP(dtmP,p2)->sPtr = p3 ;
                         nodeAddrP(dtmP,p2)->tPtr = p3 ;
                         p1 = p2 ;
                         p2 = p3 ;
                        } while ( p3 != sp  ) ;
/*
**                    Store Feature As Polygon
*/
                      if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,DTM_NULL_USER_TAG,dtmP->dtmFeatureIndex,sp,1)) goto errexit ;
                      ++dtmP->dtmFeatureIndex ;
                      ++numIslands ;
                     }
                  }
               }
            }
         }
      }
   }
/*
** Write Number Of Detected Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   Detected = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands Detected = %6ld",numIslands) ;
   }
/*
** Write Resolved Void And Island Features To Dtm Object
*/
 for( feature = numStartFeatures ; feature < dtmP->numFeatures ; ++feature )
   {
    if( ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Void   ) dtmFeatureType = DTMFeatureType::Void ;
       if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Island ) dtmFeatureType = DTMFeatureType::Island ;
/*
**     Write Feature To Data Object
*/
       sp =  ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ;
//       if( bcdtmObject_storePointInDtmObject(*voidPolygonsPP,fscode,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z ) ) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z )) goto errexit ;
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,sp,&np) ) goto errexit ;
          sp = np ;
//          if( bcdtmObject_storePointInDtmObject(*voidPolygonsPP,fncode,DTM_NULL_USER_TAG,nullGuid,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z ) ) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z )) goto errexit ;
         } while ( sp != ftableAddrP(dtmP,feature)->dtmFeaturePts.firstPoint ) ;

       if( bcdtmLoad_getCachePoints(&voidPtsP,&numVoidPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(*voidPolygonsPP,dtmFeatureType,DTM_NULL_USER_TAG,1,&(*voidPolygonsPP)->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;
      }
   }
/*
** Write Void Polygons ** Development Only
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(*voidPolygonsPP,L"voidPolygons.dat") ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Extracting Void Polygons From Deleted Triangles Tin Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Extracting Void Polygons From Deleted Triangles Tin Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *voidPolygonsPP != NULL ) bcdtmObject_destroyDtmObject(voidPolygonsPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_clipLastDtmFeatureToVoidDtmObject(BC_DTM_OBJ *dtmP )
/*
** This Clips The Last DTM Feature Inserted
*/
{
 int ret=DTM_SUCCESS ;
/*
** Clip Last Inserted Feature
*/
 if( bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(dtmP,dtmP->numFeatures-1)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** REutnt
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
int bcdtmExtEdit_getDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long **deletedDtmFeaturesPP,long *numDeletedDtmFeaturesP)
{
 int   ret=DTM_SUCCESS ;
 long  n,*deletedFeaturesP=NULL,numDeletedFeatures=0,*islandsP=NULL,numIslands=0,voidFeature ;
/*
** Initialise
*/
 *deletedDtmFeaturesPP = NULL ;
 *numDeletedDtmFeaturesP = 0 ;
/*
** Get Void feature External To Island
*/
 if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) bcdtmExtEdit_getVoidExternalToIslandDtmObject(dtmP,dtmFeature,&voidFeature) ;
 else                                                              voidFeature = dtmFeature ;
/*
** Get Island dtmFeatures Internal To Void
*/
 if( bcdtmExtEdit_getIslandsInternalToVoidDtmObject(dtmP,voidFeature,&islandsP,&numIslands)) goto errexit ;
/*
** Set Delete dtmFeatures
*/
 numDeletedFeatures = numIslands + 1 ;
 deletedFeaturesP   = ( long *) malloc( numDeletedFeatures * sizeof(long)) ;
 if( deletedFeaturesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
   }
 *deletedFeaturesP = voidFeature ;
 for( n = 0 ; n < numIslands ; ++n ) *(deletedFeaturesP+n+1) = *(islandsP+n) ;
/*
** Set variables for Return
*/
 *deletedDtmFeaturesPP    = deletedFeaturesP ;
 *numDeletedDtmFeaturesP = numDeletedFeatures ;
/*
** Clean Up
*/
 cleanup :
 if( islandsP != NULL ) free(islandsP) ;
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
int bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject
(
 BC_DTM_OBJ *dtmP,                          /* ==> Pointer To DTM Object          */
 double      x,                             /* ==> x Coordinate Of Cursor Point   */
 double      y,                             /* ==> y Coordinate Of Cursor Point   */
 long        point,                         /* ==> Tin Point                      */
 SelectedLinearFeatures& linearFeatures     /* <== Linear Features Found          */
)
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long onLine,numFeaturePts=0,numPntFeatures=0;
 bool accumulateFeature=false ;
 DPoint3d  *p3dP,*featurePtsP=NULL ;
 double nx,ny,distance,cursorDistance=0.0 ;
 LinearFeature  linearFeature ;
 DTM_TIN_POINT_FEATURES *featP,*pntFeaturesP=NULL ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting DTM Linear Features For Point") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x       = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y       = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"point   = %8ld",point) ;
   }

//  Get Features For Tin Point 1

  if( point != dtmP->nullPnt )
    {
     if( nodeAddrP(dtmP,point)->fPtr != dtmP->nullPtr )
       {
        if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,point,&pntFeaturesP,&numPntFeatures) ) goto errexit ;
       }
    }

//  Scan Point Features And Accumulate

  if( pntFeaturesP != NULL )
    {
     for( featP = pntFeaturesP ; featP < pntFeaturesP + numPntFeatures ; ++featP )
       {

       if (featP->dtmFeatureType == DTMFeatureType::GroupSpots || featP->dtmFeatureType == DTMFeatureType::FeatureSpot)
           continue;
//      Check Feature Has Not Already Been Accumulated

        accumulateFeature = true ;
        SelectedLinearFeatures::iterator it  ;
        for( it = linearFeatures.begin() ; it != linearFeatures.end() && accumulateFeature == true ; ++it )
          {
           if( it->featureNumber == featP->dtmFeature ) accumulateFeature = false ;
          }

//      Get Feature Points

        if( accumulateFeature )
          {

//         Get Feature Points

           if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,featP->dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;

//         Scan Feature And Find Closest Distance To Cursor Point

           p3dP = featurePtsP ;
           cursorDistance = bcdtmMath_distance(x,y,p3dP->x,p3dP->y) ;
           for( p3dP = featurePtsP + 1 ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
             {
              distance = bcdtmMath_distance(x,y,p3dP->x,p3dP->y) ;
              if( distance < cursorDistance ) cursorDistance = distance ;
              distance = bcdtmMath_distanceOfPointFromLine(&onLine,x,y,(p3dP-1)->x,(p3dP-1)->y,p3dP->x,p3dP->y,&nx,&ny) ;
              if( onLine && distance < cursorDistance ) cursorDistance = distance ;
             }

//         Accumulate Feature

           linearFeature.featureType     = featP->dtmFeatureType ;
           linearFeature.featureNumber   = featP->dtmFeature ;
           linearFeature.featureDistance = cursorDistance ;
           linearFeature.featurePtsP     = (DPoint3d*)featurePtsP ;
           linearFeature.numFeaturePts   = numFeaturePts ;
           linearFeatures.push_back (linearFeature) ;
           featurePtsP = NULL ;
           numFeaturePts = 0 ;
          }
       }
    }

// Clean Up

 cleanup :
 if( pntFeaturesP != NULL ) free(pntFeaturesP) ;
 if( featurePtsP  != NULL ) free(featurePtsP) ;

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting DTM Linear Features For Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting DTM Linear Features For Point Error") ;
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
int bcdtmExtEdit_selectDtmEditLinearFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,                          /* ==> Pointer To DTM Object               */
 double  x,                                 /* ==> Data Point x Coordinate Value       */
 double  y,                                 /* ==> Data Point y Coordinate Value       */
 double  snapTolerance,                     /* ==> Snap Tolerance                      */
 SelectedLinearFeatures& linearFeatures     /* <== Linear Features Found               */
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long pnt,cPnt,tPnt1,tPnt2,tPnt3,fndType,startPnt,lastPnt ;
 SelectedLinearFeatures::iterator feature ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Snapping to DTM Linear Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x       = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y       = %12.5lf",y) ;
   }
/*
** Initialise
*/
// if( *linearFeaturesPP != NULL) { free(*linearFeaturesPP) ; *linearFeaturesPP = NULL ; }
// *numLinearFeaturesP = 0 ;
/*
** Test For Valid Dtm
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Closest Point And Triangle
*/
 bcdtmFind_closestPointDtmObject(dtmP,x,y,&cPnt) ;
 bcdtmFind_triangleDtmObject(dtmP,x,y,&fndType,&tPnt1,&tPnt2,&tPnt3) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"cPnt = %8ld ** tPnt1 = %8ld tPnt2 = %8ld tPnt3 = %8ld",cPnt,tPnt1,tPnt2,tPnt3) ;
/*
** Get Features For Closest Point And Triangle Points
*/
 if( bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(dtmP,x,y,cPnt,linearFeatures)) goto errexit ;
 if( bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(dtmP,x,y,tPnt1,linearFeatures)) goto errexit ;
 if( bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(dtmP,x,y,tPnt2,linearFeatures)) goto errexit ;
 if( bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(dtmP,x,y,tPnt3,linearFeatures)) goto errexit ;
/*
** Set Start And Last Points For Scan
*/
 bcdtmFind_binaryScanDtmObject(dtmP,x-snapTolerance,&startPnt) ;
 while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= x-snapTolerance ) --startPnt ;
 bcdtmFind_binaryScanDtmObject(dtmP,x+snapTolerance,&lastPnt) ;
 while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= x+snapTolerance ) ++lastPnt ;
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
** Scan Points To Find Closest Feature
*/
 for( pnt = startPnt ; pnt <= lastPnt ; ++pnt )
   {
    if( bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->x) <= snapTolerance )
      {
       if( bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(dtmP,x,y,pnt,linearFeatures)) goto errexit ;
      }
   }
/*
** Sort Accumulated Features On Distance From Cursor Point
*/
 if( linearFeatures.size() > 0 )
   {
    linearFeatures.sortOnAscendingDistance() ;
   }
/*
** Report Selected Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Selected Features = %8ld",linearFeatures.size()) ;
    if( linearFeatures.size() > 0 )
      {
       for( pnt = 0 , feature = linearFeatures.begin() ; feature != linearFeatures.end() ; ++feature , ++pnt )
         {
          bcdtmWrite_message(0,0,0,"Linear Feature[%8ld] ** Type = %5ld Distance = %12.5lf",pnt,feature->featureType,feature->featureDistance) ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to DTM Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping to DTM Feature Error") ;
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
int bcdtmExtEdit_getVoidFeaturesInternalToIslandDtmObject
(
 BC_DTM_OBJ             *dtmP,
 long                   islandFeature,
 DTM_TIN_POINT_FEATURES **voidsPP,
 long                   *numVoidsP
)
/*
** This Function Gets The List Of All Void Features Internal To An Island
**
** Rob Cormack - June 2003
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   memVoids=0,memVoidsInc=10,voidFeature ;
 long   ofs,lp,clc,pnt,ppnt,npnt,lpnt,fPnt,lPnt,sPnt,numLineFeatures ;
 DTM_TIN_POINT_FEATURES *lineFeaturesP=NULL,*featP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void Features Internal To Island") ;
/*
** Initialise
*/
 *numVoidsP = 0 ;
 if( *voidsPP != NULL ) { free(*voidsPP) ; *voidsPP = NULL ; }
/*
** Copy Void Feature To Tptr List
*/
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,islandFeature,&sPnt)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 fPnt = lPnt = dtmP->nullPnt ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**     Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp))
            {
             if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
      }
/*
** Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
/*
**        Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp) )
               {
                if( lPnt == dtmP->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { nodeAddrP(dtmP,lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                nodeAddrP(dtmP,lPnt)->tPtr = -(lPnt+1) ;
               }
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
            }
         }
      }
/*
**  Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;
    pnt  = npnt ;
   } while ( ppnt!= sPnt ) ;
/*
** Scan Internal Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
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
          if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt && ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,lp) )
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
** Scan Marked Points For Connection To An Island Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Marked Points For Connection To Island Feature") ;
 if( fPnt != dtmP->nullPnt )
   {
    pnt = fPnt ;
    do
      {
/*
**     Scan Marked Point For Island Features
*/
       lpnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Void,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
         }
/*
**     Get Next Marked Point
*/
       npnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Scan Void Hull For Direct Connection To Island
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Void Hull For Direct Connection To Islands") ;
 ppnt= sPnt ;
 pnt = nodeAddrP(dtmP,sPnt)->tPtr ;
 do
   {
    lp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
/*
**  Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
    if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,lp) )
      {
       if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
       while ( lp != ppnt && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,lp))
         {
          if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
            {
             if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Void,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
             for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
               {
                if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
               }
            }
          if(( lp = bcdtmList_nextAntDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
         }
       if( lp != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Void,pnt,lp,&lineFeaturesP,&numLineFeatures)) goto errexit ;
          for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
         }
      }
/*
**  Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
/*
**     Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,pnt,lp) )
         {
          if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
          while ( lp != npnt && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,lp,pnt))
            {
             if( nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Void,lp,&lineFeaturesP,&numLineFeatures) ) goto errexit ;
                for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
                  {
                   if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
                  }
               }
            if(( lp = bcdtmList_nextClkDtmObject(dtmP,pnt,lp)) < 0 ) goto errexit ;
           }
         if( lp != npnt )
           {
            if( bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(dtmP,DTMFeatureType::Void,lp,pnt,&lineFeaturesP,&numLineFeatures)) goto errexit ;
            for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
              {
               if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
              }
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
** Null Tptr List
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,sPnt)) goto errexit ;
/*
** Scan Detected Voids For Connection To Other Voids
*/
 for( ofs = 0 ; ofs < *numVoidsP ; ++ofs )
   {
    voidFeature = (*voidsPP+ofs)->dtmFeature ;
    sPnt = ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ;
    if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,voidFeature,sPnt,&ppnt)) goto errexit ;
    do
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,sPnt,&npnt)) goto errexit ;
       if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,npnt)) < 0 ) goto errexit ;
       while ( pnt != ppnt )
         {
          if( bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(dtmP,DTMFeatureType::Void,pnt,&lineFeaturesP,&numLineFeatures)) goto errexit ;
          for( featP = lineFeaturesP ; featP < lineFeaturesP + numLineFeatures ; ++featP )
            {
             if( bcdtmExtEdit_storeDtmFeatureInDtmFeatureList(voidsPP,numVoidsP,&memVoids,memVoidsInc,featP->dtmFeature,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->dtmFeatureIndex,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
            }
          if(( pnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pnt)) < 0 ) goto errexit ;
         }
       ppnt = sPnt ;
       sPnt = npnt ;
      } while ( sPnt != ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint ) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( lineFeaturesP != NULL ) { free(lineFeaturesP) ; lineFeaturesP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void Features Internal To Island Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Getting Void Features Internal To Island Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_getVoidsInternalToIslandDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       islandFeature,
 long       **voidsPP,
 long       *numVoidsP
)
/*
** This Function Gets The Set Of Voids Internal To A Island
*/
{
 int   ret=DTM_SUCCESS ;
 long  numVoidFeatures ;
 DTM_TIN_POINT_FEATURES *ivp,*voidFeaturesP=NULL ;
/*
** Initialise
*/
 *numVoidsP = 0 ;
 if( *voidsPP != NULL ) { free(*voidsPP) ; *voidsPP = NULL ; }
/*
** Check For A Void Feature
*/
 if( islandFeature >= 0 && islandFeature < dtmP->numFeatures )
   {
    if( ftableAddrP(dtmP,islandFeature)->dtmFeatureType  == DTMFeatureType::Island && ftableAddrP(dtmP,islandFeature)->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Get Void Features Internal To Island Feature
*/
       if( bcdtmExtEdit_getVoidFeaturesInternalToIslandDtmObject(dtmP,islandFeature,&voidFeaturesP,&numVoidFeatures) ) goto errexit ;
/*
**      Allocate Memory For Voids
*/
       *voidsPP = ( long * ) malloc ( numVoidFeatures * sizeof(long)) ;
       if( *voidsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**     Copy Void Features
*/
       for( ivp = voidFeaturesP ; ivp < voidFeaturesP + numVoidFeatures ; ++ivp )
         {
          *(*voidsPP+*numVoidsP) = ivp->dtmFeature ;
          ++*numVoidsP ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( voidFeaturesP != NULL ) { free(voidFeaturesP) ; voidFeaturesP = NULL ; }
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numVoidsP = 0 ;
 if( *voidsPP != NULL ) { free(*voidsPP) ; *voidsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_deleteLinearFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 DTMFeatureType dtmFeatureType
)
/*
** This Function Deletes A Linear Feature
*/
{
 int   ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long  feature ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Linear Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeature     = %8ld",dtmFeature) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Before Deleting Linear Feature") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Tin Valid") ;
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
   }
/*
** Check For Feature Range Error
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Feature Range Error") ;
    goto errexit ;
   }
/*
** Check For Correct Feature Type
*/
  dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
  if( dtmFeatureType != dtmFeatureP->dtmFeatureType )
    {
     bcdtmWrite_message(2,0,0,"Incorrect Feature Type") ;
     goto errexit ;
    }
/*
** Remove Tin Error And Roll Back Features With The Same Feature Id
*/
  dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
  dtmFeatureId = dtmFeatureP->dtmFeatureId ;
  if( dtmFeatureId != dtmP->nullFeatureId )
    {
     for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
       {
        dtmFeatureP = ftableAddrP(dtmP,feature) ;
        if( dtmFeatureP->dtmFeatureId == dtmFeatureId && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback ))
          {
           if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,feature,true)) goto errexit ;
          }
       }
    }
/*
**  Switch On Feature Type
*/
  switch( dtmFeatureType )
    {
     case  DTMFeatureType::Void :
     case  DTMFeatureType::Hole :
     case  DTMFeatureType::Island :
       if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Polygonal Feature Type") ;
       if( bcdtmExtEdit_deletePolygonalFeatureDtmObject(dtmP,dtmFeature,dtmFeatureType)) goto errexit ;
     break ;

     default :
       if( dbg ) bcdtmWrite_message(0,0,0,"Deleting None Polygonal Feature Type") ;
       if( bcdtmExtEdit_deleteNonePolygonalFeatureDtmObject(dtmP,dtmFeature,dtmFeatureType)) goto errexit ;
     break ;
    }

/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin After  Deleting Linear Feature") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Tin Invalid After Deleting Linear Feature") ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Tin Valid After Deleting Linear Feature") ;
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
   }
bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Linear Feature Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Linear Feature Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_deletePolygonalFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 DTMFeatureType dtmFeatureType
)

//  This Is A Specific Edit Function That Is Only Called
//  From "bcdtmExtEdit_deleteLinearFeatureDtmObject"
//  Do Not Call It

{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long feat,feature,numIslands=0,*islandListP=NULL,numVoids=0,*voidListP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Polygonal Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeature     = %8ld",dtmFeature) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
   }

// Delete Void Feature

 if( dtmFeatureType == DTMFeatureType::Void )
   {

    // Get Islands Internal To Void

    if( bcdtmEdit_getIslandsInternalToVoidDtmObject(dtmP,dtmFeature,&islandListP,&numIslands)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Internal Islands = %8ld",numIslands) ;

    // Remove Internal Island Tin Error And Roll Back Features

    if( numIslands > 0 )
      {
       for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
         {
          dtmFeatureP = ftableAddrP(dtmP,feature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback ))
            {
             for( feat = 0 ; feat < numIslands ; ++feat )
               {
                if( ftableAddrP(dtmP,*(islandListP+feat))->dtmFeatureId ==  dtmFeatureP->dtmFeatureId )
                  {
                   if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,feature,true)) goto errexit ;
                  }
               }
            }
         }
      }

    // Remove Void Feature

    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature,true)) goto errexit ;

    // Remove Internal Island Features

    for( feat = 0 ; feat < numIslands ; ++feat )
      {
       if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,*(islandListP+feat),true)) goto errexit ;
      }
   }

// Delete Island Feature

 if( dtmFeatureType == DTMFeatureType::Island )
   {

    // Get Islands Internal To Void

    if( bcdtmExtEdit_getVoidsInternalToIslandDtmObject(dtmP,dtmFeature,&voidListP,&numVoids)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Internal Voids = %8ld",numVoids) ;

    // Remove Internal Void Tin Error And Roll Back Features

    if( numVoids > 0 )
      {
       for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
         {
          dtmFeatureP = ftableAddrP(dtmP,feature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback ))
            {
             for( feat = 0 ; feat < numVoids ; ++feat )
               {
                if( ftableAddrP(dtmP,*(voidListP+feat))->dtmFeatureId ==  dtmFeatureP->dtmFeatureId )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Removing Roll Back Or Tin Error Void Feature %8ld",feature) ;
                   if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,feature,true)) goto errexit ;
                  }
               }
            }
         }
      }

    // Remove Island Feature

    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Island Feature %8ld",feature) ;
    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature,true)) goto errexit ;

    // Remove Internal Void Features

    for( feat = 0 ; feat < numVoids ; ++feat )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Internal Void Feature %8ld",*(voidListP+feat)) ;
       if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,*(voidListP+feat),true)) goto errexit ;
      }
   }

// Delete Hole Feature

 if( dtmFeatureType == DTMFeatureType::Hole )
   {
    if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature, true)) goto errexit ;
   }

// Reset Void Bits

 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Void Bits") ;
 if( bcdtmExtMark_voidPointsDtmObject(dtmP)) goto errexit ;

// Clean Up

 cleanup :
 if( voidListP   != NULL ) free(voidListP) ;
 if( islandListP != NULL ) free(islandListP) ;

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Polygonal Feature Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting Polygonal Feature Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmExtEdit_deleteNonePolygonalFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 DTMFeatureType dtmFeatureType
)

//  This Is A Specific Edit Function That Is Only Called
//  From "bcdtmExtEdit_deleteLinearFeatureDtmObject"
//  Do Not Call It

{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long firstPnt,numPntList,*lP,*pntListP=NULL ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting None Polygonal Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeature     = %8ld",dtmFeature) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
   }

// Copy Feature Points To A Point List

 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&firstPnt)) goto errexit ;
 if( bcdtmExtList_copyTptrListToPointListDtmObject(dtmP,firstPnt,&pntListP,&numPntList)) goto errexit ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,firstPnt)) goto errexit ;

// Delete Feature

 if( bcdtmExtInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature, true)) goto errexit ;

// Check For Closed Feature

 if( *pntListP == *(pntListP+numPntList-1) ) --numPntList ;

// Scan Point List And Delete Points And Retriangulate

 for( lP = pntListP ; lP < pntListP + numPntList ; ++lP )
   {
    if( nodeAddrP(dtmP,*lP)->hPtr == dtmP->nullPnt && nodeAddrP(dtmP,*lP)->fPtr == dtmP->nullPtr )
      {
       if( bcdtmExtEdit_insertTptrPolygonAroundPointDtmObject(dtmP,*lP,&firstPnt)) goto errexit ;
       if( bcdtmExtEdit_removePointDtmObject(dtmP,*lP,0,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt) ) goto errexit ;
       if( bcdtmClip_fillTptrPolygonWithTrianglesDtmObject(dtmP,firstPnt)) goto errexit ;
       if( bcdtmList_nullTptrListDtmObject(dtmP,firstPnt)) goto errexit ;
      }
   }

// Clean Up

 cleanup :
 if( pntListP != NULL ) free(pntListP) ;

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting None Polygonal Feature Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Deleting None Polygonal Feature Error") ;
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
