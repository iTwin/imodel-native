/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmDelta.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

extern long numPrecisionError,numSnapFix ;  //These are only used in Debug Code.
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDelta_createDeltaSurfaceDtmFile
(
 WCharCP fromFileP,             /* ==> From Dtm File                                 */
 WCharCP toFileP,               /* ==> To Dtm File For Delta Option = 2              */
 WCharCP deltaFileP,            /* ==> Delta File To Be Created                      */
 long deltaOption,               /* ==> Delta Option <1=Elevation,2=Surface>          */
 long modelOption,               /* ==> Model Option <1=Tin,2=Lattice>                */ 
 DPoint3d *polyPtsP,                  /* ==> Pointer To Fence Points                       */
 long numPolyPts,                /* ==> Number Of Fence Points                        */
 long numLatticePts,             /* ==> Number Of Lattice Points For Model Option = 2 */
 double elevation                /* ==> Elevation For Delta Option = 1                */ 
)
/*
** This Is the Controlling Function For Creating A Delta Surface
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)  ;
 long   modelType=0 ;
 BC_DTM_OBJ *dtm1P=NULL,*dtm2P=NULL,*dtm3P=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Delta Surface") ;
    bcdtmWrite_message(0,0,0,"fromFileP        = %s",fromFileP) ;
    bcdtmWrite_message(0,0,0,"toFileP          = %s",toFileP) ;
    bcdtmWrite_message(0,0,0,"deltaFileP       = %s",deltaFileP) ;
    bcdtmWrite_message(0,0,0,"deltaOption      = %8ld",deltaOption) ;
    bcdtmWrite_message(0,0,0,"modelOption      = %8ld",modelOption) ;
    bcdtmWrite_message(0,0,0,"polyPtsP         = %p",polyPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolyPts       = %8ld",numPolyPts) ;
    bcdtmWrite_message(0,0,0,"numLatticePts    = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"elevation        = %8.3lf",elevation) ;
   }
/*
** Initialse Variables
*/
 if( deltaOption == 1 && modelOption == 1 ) modelType = 1 ;
 if( deltaOption == 2 && modelOption == 1 ) modelType = 2 ;
 if( deltaOption == 1 && modelOption == 2 ) modelType = 3 ;
 if( deltaOption == 2 && modelOption == 2 ) modelType = 4 ;
/*
** Test If Requested Tin File Is Current Tin Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtm1P,fromFileP)) goto errexit ;
/*
** Create Model
*/
 switch ( modelType )
   {
    case  1 :    /* Delta Tin To Elevation     */
      if( bcdtmDelta_createDeltaTinToElevationDtmObject(dtm1P,&dtm3P,polyPtsP,numPolyPts,elevation)) goto errexit ;
    break   ;

    case  2 :    /* Delta Tin To Surface       */
      if( bcdtmRead_fromFileDtmObject(&dtm2P,toFileP)) goto errexit ; 
      if( bcdtmDelta_createDeltaTinToSurfaceDtmObject(&dtm1P,&dtm2P,&dtm3P,polyPtsP,numPolyPts)) goto errexit ;
    break   ;

    case  3 :    /* Delta Lattice To Elevation */
      if( bcdtmDelta_createDeltaLatticeToElevationDtmObject(dtm1P,&latticeP,polyPtsP,numPolyPts,elevation,numLatticePts)) goto errexit ;
    break   ;

    case  4 :    /* Delta Lattice To Surface   */
      if( bcdtmRead_fromFileDtmObject(&dtm2P,toFileP)) goto errexit ; 
      if( bcdtmDelta_createDeltaLatticeToSurfaceDtmObject(dtm1P,dtm2P,&latticeP,polyPtsP,numPolyPts,numLatticePts) ) goto errexit ;
    break   ;

    default :
    break ;
   } ;
/*
** Destroy Dtm Objects
*/
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Set Current Objects
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Current Objects") ;
 bcdtmUtility_setCurrentDtmObject(dtm3P,deltaFileP) ;
 if( latticeP != NULL ) bcdtmUtl_setCurrentLatticeObject(latticeP,deltaFileP) ;
/*
** Write Delta Object To File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Delta Surface To File") ;
 if( wcslen(deltaFileP) )
   { 
    if( modelOption == 1 ) if( bcdtmWrite_toFileDtmObject(dtm3P,deltaFileP) ) goto errexit ;
    if( modelOption == 2 ) if( bcdtmWrite_toFileLatticeObject(latticeP,deltaFileP)) goto errexit ;
   } 
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Surface Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Surface Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
 if( dtm3P != NULL ) bcdtmObject_destroyDtmObject(&dtm3P) ;
 if( latticeP != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDelta_createGeopakPortDeltaSurfaceDtmFile
(
 WCharCP fromFileP,             /* ==> From Dtm File                                 */
 WCharCP toFileP,               /* ==> To Dtm File For Delta Option = 2              */
 void **deltaObjPP,              /* <== Pointer To Created Delta Object               */
 long deltaOption,               /* ==> Delta Option <1=Elevation,2=Surface>          */
 long modelOption,               /* ==> Model Option <1=Tin,2=Lattice>                */ 
 DPoint3d *polyPtsP,                  /* ==> Pointer To Fence Points                       */
 long numPolyPts,                /* ==> Number Of Fence Points                        */
 long numLatticePts,             /* ==> Number Of Lattice Points For Model Option = 2 */
 double elevation                /* ==> Elevation For Delta Option = 1                */ 
)
/*
** This Is the Controlling Function For Creating A Delta Surface
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)  ;
 long   modelType=0 ;
 BC_DTM_OBJ *dtm1P=NULL,*dtm2P=NULL,*dtm3P=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Delta Surface") ;
    bcdtmWrite_message(0,0,0,"fromFileP        = %s",fromFileP) ;
    bcdtmWrite_message(0,0,0,"toFileP          = %s",toFileP) ;
    bcdtmWrite_message(0,0,0,"deltaOption      = %8ld",deltaOption) ;
    bcdtmWrite_message(0,0,0,"modelOption      = %8ld",modelOption) ;
    bcdtmWrite_message(0,0,0,"polyPtsP         = %p",polyPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolyPts       = %8ld",numPolyPts) ;
    bcdtmWrite_message(0,0,0,"numLatticePts    = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"elevation        = %8.3lf",elevation) ;
   }
/*
** Initialse Variables
*/
 *deltaObjPP = NULL ;
 if( deltaOption == 1 && modelOption == 1 ) modelType = 1 ;
 if( deltaOption == 2 && modelOption == 1 ) modelType = 2 ;
 if( deltaOption == 1 && modelOption == 2 ) modelType = 3 ;
 if( deltaOption == 2 && modelOption == 2 ) modelType = 4 ;
/*
** Test If Requested Tin File Is Current Tin Object
*/
 if( bcdtmRead_fromFileDtmObject(&dtm1P,fromFileP)) goto errexit ;
/*
** Create Model
*/
 switch ( modelType )
   {
    case  1 :    /* Delta Tin To Elevation     */
      if( bcdtmDelta_createDeltaTinToElevationDtmObject(dtm1P,&dtm3P,polyPtsP,numPolyPts,elevation)) goto errexit ;
      *deltaObjPP = ( void *) dtm3P ;
    break   ;

    case  2 :    /* Delta Tin To Surface       */
      if( bcdtmRead_fromFileDtmObject(&dtm2P,toFileP)) goto errexit ; 
      if( bcdtmDelta_createDeltaTinToSurfaceDtmObject(&dtm1P,&dtm2P,&dtm3P,polyPtsP,numPolyPts)) goto errexit ;
      *deltaObjPP = ( void *)dtm3P ;
    break   ;

    case  3 :    /* Delta Lattice To Elevation */
      if( bcdtmDelta_createDeltaLatticeToElevationDtmObject(dtm1P,&latticeP,polyPtsP,numPolyPts,elevation,numLatticePts)) goto errexit ;
       *deltaObjPP = ( void *)latticeP ;
   break   ;

    case  4 :    /* Delta Lattice To Surface   */
      if( bcdtmRead_fromFileDtmObject(&dtm2P,toFileP)) goto errexit ; 
      if( bcdtmDelta_createDeltaLatticeToSurfaceDtmObject(dtm1P,dtm2P,&latticeP,polyPtsP,numPolyPts,numLatticePts) ) goto errexit ;
      *deltaObjPP = ( void *)latticeP ;
    break   ;

    default :
    break ;
   } ;
/*
** Cleanup
*/
 cleanup :
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Surface Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Surface Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *deltaObjPP   = NULL ;
 if( dtm3P    != NULL ) bcdtmObject_destroyDtmObject(&dtm3P) ;
 if( latticeP != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDelta_createDeltaTinToElevationDtmObject
(
 BC_DTM_OBJ *dtm1P,           /* ==> Pointer To Dtm Object               */
 BC_DTM_OBJ **dtm2PP,         /* <== Pointer To Created Delta Dtm Object */
 DPoint3d *deltaPtsP,              /* ==> Pointer To Fence Points             */
 long numDeltaPts,            /* ==> Number Of Fence Points              */
 double elevation             /* ==> Elevation For Delta Calculations    */
)
/*
** This Function Creates a Delta Tin To An Elevation
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pnt,numPolyPts,intersectResult ;
 DPoint3d     *polyPtsP=NULL ; 
 double  largestArea=0.0  ;
 DTM_POLYGON_OBJ  *polyP=NULL ;
 DTM_POLYGON_LIST *plistP,*pp ;
 DPoint3d    *pntP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Delta Tin Surface To Elevation") ;
    bcdtmWrite_message(0,0,0,"dtm1P            = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2PP           = %p",*dtm2PP) ;
    bcdtmWrite_message(0,0,0,"deltaPtsP        = %p",deltaPtsP) ;
    bcdtmWrite_message(0,0,0,"numDeltaPts      = %8ld",numDeltaPts) ;
    bcdtmWrite_message(0,0,0,"elevation        = %8.3lf",elevation) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtm1P->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtm1P) ;
    goto errexit ;
   }
/*
** Check For Old Tin Files
*/
 if( dtm1P->ppTol == 0.0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Convert Old Dtm (dtmP) File(s)") ;
    ret = 20 ;
    goto errexit ;
   }
/*
**  Intersect Delta Polygon And Tin Hull
*/
 if( bcdtmPolygon_intersectPolygonAndTinHullDtmObject(dtm1P,deltaPtsP,numDeltaPts,&polyP,&intersectResult ) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld",intersectResult) ;
/*
** If No Intersection Return
*/
   if( ! intersectResult ) { if( polyP != NULL )  bcdtmPolygon_deletePolygonObject(&polyP) ; return(0) ; } 
/*
** If Tin Hull Totally Within Polygon Clone Dtm Object
*/
 if( intersectResult == 1 ) { if( bcdtmObject_cloneDtmObject(dtm1P,dtm2PP)) goto errexit ; }
/*
** Else Clip dtm1P
*/  
 else
   {
    plistP = polyP->polyListP ;
/*
**  If More Than One Intersect Polygon, Get Polygon With Largest Area
*/
    if( polyP->numPolygons > 1 ) 
      {
       largestArea = (polyP->polyListP)->area ;
       for( pp = polyP->polyListP ; pp < polyP->polyListP + polyP->numPolygons ; ++pp) 
         { 
          if( pp->area > largestArea ) { largestArea = pp->area ; plistP = pp ; }
         }
      }
/*
**  Copy Polygon Object Polygon To DPoint3d Polygon
*/ 
    if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plistP-polyP->polyListP),&polyPtsP,&numPolyPts))  goto errexit ;
/*
**  Clip Tin Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Cloning And Clipping Dtm Object") ; 
    if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtm1P,dtm2PP,polyPtsP,numPolyPts,DTMClipOption::External)) goto errexit ;
   }
/*
** Calculate Delta Tin Values
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Delta Tin") ; 
 for( pnt = 0  ; pnt < (*dtm2PP)->numPoints ; ++pnt ) 
   {
    pntP  = pointAddrP(*dtm2PP,pnt) ;
    pntP->z = elevation - pntP->z  ;
   } 
 bcdtmMath_setBoundingCubeDtmObject(*dtm2PP) ;
/*
** Cleanup
*/
 cleanup :
 if( polyPtsP != NULL ) free(polyPtsP) ; 
 if( polyP    != NULL ) bcdtmPolygon_deletePolygonObject(&polyP)  ; 
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtm2PP != NULL ) bcdtmObject_destroyDtmObject(dtm2PP) ; goto errexit ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDelta_createDeltaTinToSurfaceDtmObject(BC_DTM_OBJ **dtm1PP,BC_DTM_OBJ **dtm2PP,BC_DTM_OBJ **deltaDtmPP,DPoint3d *deltaPtsP,long numDeltaPts)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pnt,intersectResult,numClipPts,tinReverse,startTime  ;
 DTMDirection direction;
 double  largestArea,area ;
 DPoint3d     *clipPtsP=NULL ;
 BC_DTM_OBJ       *dtm3P=NULL,*dtm4P=NULL,*saveTin ;
 DPoint3d    *pntP ;
 DTM_POLYGON_OBJ  *polyP=NULL ;
 DTM_POLYGON_LIST *plist1P,*plist2P ; 
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Delta Tin Surface To Surface") ;
    bcdtmWrite_message(0,0,0,"dtm1PP      = %p",*dtm1PP) ;
    bcdtmWrite_message(0,0,0,"dtm2PP      = %p",*dtm2PP) ;
    bcdtmWrite_message(0,0,0,"deltaDtmPP  = %p",*deltaDtmPP) ;
    bcdtmWrite_message(0,0,0,"deltaPtsP   = %p",deltaPtsP) ;
    bcdtmWrite_message(0,0,0,"numDeltaPts = %4ld",numDeltaPts) ;
   }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(*dtm1PP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(*dtm2PP)) goto errexit  ;
/*
** Check If Both DTM Objects Are In TIN_STATE
*/
 if( (*dtm1PP)->dtmState != DTMState::Tin || (*dtm2PP)->dtmState != DTMState::Tin  )
   {
    if( (*dtm1PP)->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",*dtm1PP) ;
    if( (*dtm2PP)->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",*dtm2PP) ;
    goto errexit ;
   }
/*
** Check For Old Dtm (Tin) Files
*/
 if( (*dtm1PP)->ppTol == 0.0 || (*dtm2PP)->ppTol == 0.0 )
   {
    if( (*dtm1PP)->ppTol == 0.0 )bcdtmWrite_message(1,0,0,"Convert From Dtm File/Object") ;
    if( (*dtm2PP)->ppTol == 0.0 )bcdtmWrite_message(1,0,0,"Convert To Dtm File/Object")   ;
    ret = 20  ;
    goto errexit ;
   }
/*
** Get Intersect Polygons Between Tin Hulls And Delta Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Tin Hulls And Delta Polygon") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(*dtm1PP,*dtm2PP,deltaPtsP,numDeltaPts,&polyP,&intersectResult) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld ** Number Intersection Polygons = %2ld",intersectResult,polyP->numPolygons) ;
/*
** If No Intersection Found Return
*/
 if( ! intersectResult ) 
  {  
   bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
   goto errexit ;
  }
/*
**  If More Than One Intersect Polygon, Get Polygon With Largest Area
*/
 plist1P = polyP->polyListP ;
 if( polyP->numPolygons > 1 ) 
   {
    largestArea = (polyP->polyListP)->area ;
    for( plist2P = polyP->polyListP ; plist2P < polyP->polyListP + polyP->numPolygons ; ++plist2P) 
      { 
       if( plist2P->area > largestArea ) { largestArea = plist2P->area ; plist1P = plist2P ;} 
      }
   }
/*
** Copy Polygon Object Polygon To DPoint3d Polygon
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Intersection Polygon") ;
 if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plist1P-polyP->polyListP),&clipPtsP,&numClipPts)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Polygon = %p Number Of Points = %6ld",clipPtsP,numClipPts) ;
/*
**  Clip dtm1P Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cliping dtm1P => dtm3P") ;
 startTime = bcdtmClock() ;
 if( bcdtmClip_cloneAndClipToPolygonDtmObject(*dtm1PP,&dtm3P,clipPtsP,numClipPts,DTMClipOption::External)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"**** Time To Clip dtm1P     = %12.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtm3P,dtm3P->hullPoint,&area,&direction) ;
    bcdtmWrite_message(0,0,0,"**** Area Of Clip dtm1P Hull = %12.3lf",area) ;
    bcdtmWrite_toFileDtmObject(dtm3P,L"tin3.dtm") ;
    bcdtmWrite_message(0,0,0,"dtm3P->ppTol = %20.15lf dtm3P->plTol = %20.15lf dtm3P->mppTol = %20.15lf",dtm3P->ppTol,dtm3P->plTol,dtm3P->mppTol) ;
   }
 bcdtmObject_destroyDtmObject(dtm1PP) ;
/*
**  Clip dtm2P Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cliping dtm2P => Tin4") ;
 startTime = bcdtmClock() ;
 if (bcdtmClip_cloneAndClipToPolygonDtmObject (*dtm2PP, &dtm4P, clipPtsP, numClipPts, DTMClipOption::External)) goto errexit;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"**** Time To Clip dtm2P      = %12.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtm4P,dtm4P->hullPoint,&area,&direction);
    bcdtmWrite_message(0,0,0,"**** Area Of Clip dtm2P Hull = %12.3lf",area) ;
    bcdtmWrite_toFileDtmObject(dtm4P,L"tin4.dtm") ;
    bcdtmWrite_message(0,0,0,"dtm4P->ppTol = %20.15lf dtm4P->plTol = %20.15lf dtm4P->mppTol = %20.15lf",dtm4P->ppTol,dtm4P->plTol,dtm4P->mppTol) ;
   }
 bcdtmObject_destroyDtmObject(dtm2PP) ;
/*
** Insert Tin With Smallest Number Of Points
*/
 tinReverse = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtm3P->numPoints = %7ld dtm4P->numPoints = %7ld",dtm3P->numPoints,dtm4P->numPoints) ;
 if( dtm4P->numPoints > dtm3P->numPoints )
   {
    tinReverse = 1 ;
    saveTin  = dtm3P ; 
    dtm3P = dtm4P ; 
    dtm4P = saveTin ;
   }
/*
**  Intersect dtm3P And dtm4P
*/ 
 if( bcdtmDelta_calculateDeltaTinToTinDtmObject(dtm3P,dtm4P)) goto errexit ; 
/*
** Negate z Values If Tins Reversed
*/
 if( tinReverse )
   {
    for( pnt = 0 ; pnt < dtm3P->numPoints ; ++pnt )
      {
       pntP = pointAddrP(dtm3P,pnt) ;
       pntP->z = - pntP->z ;
      }
   }
/*
** Set Delta Tin
*/
 *deltaDtmPP = dtm3P ;
 dtm3P = NULL ;
/*
** Cleanup
*/
 cleanup :
 if( clipPtsP != NULL ) { free(clipPtsP) ; clipPtsP = NULL ; }
 if( polyP     != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;  
 if( dtm3P     != NULL ) bcdtmObject_destroyDtmObject(&dtm3P) ;
 if( dtm4P     != NULL ) bcdtmObject_destroyDtmObject(&dtm4P) ;
/*
** Job Complist1Peted
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
BENTLEYDTM_EXPORT int bcdtmDelta_cloneAndCreateDeltaTinToElevationDtmObject
(
 BC_DTM_OBJ *dtmP,                  //  DTM To Create Delta For
 BC_DTM_OBJ **deltaDtmPP,           //  Delta DTM To Create
 double     elevation,              //  Elevation For Delta Calculation 
 DPoint3d        *deltaPtsP,             //  Clip Boundary For Delta Calculation
 long       numDeltaPts             //  Number Of Points In Clipping Boundary
 )
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long    pnt  ;
 DPoint3d    *pntP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Elevation") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"deltaDtmPP  = %p",*deltaDtmPP) ;
    bcdtmWrite_message(0,0,0,"elevation   = %8.3lf",elevation) ;
    bcdtmWrite_message(0,0,0,"deltaPtsP   = %p",deltaPtsP) ;
    bcdtmWrite_message(0,0,0,"numDeltaPts = %8ld",numDeltaPts) ;
   }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check DTM Object Is In TIN_STATE
*/
 if( (dtmP)->dtmState != DTMState::Tin  )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**  Check Delta DTM Is Null
*/
 if( *deltaDtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"Method Requires NULL Delta DTM") ;
    goto errexit ;
   }  
/*
** Clone And Clip 
*/
 if( deltaPtsP != NULL )
   {
   if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtmP, deltaDtmPP, deltaPtsP, numDeltaPts, DTMClipOption::Internal)) goto errexit;
   }
 else
   {    
    if( bcdtmObject_cloneDtmObject(dtmP,deltaDtmPP)) goto errexit ; 
   } 
/*
** Subtract Elevation From DTM
*/
 for( pnt = 0 ; pnt < (*deltaDtmPP)->numPoints ; ++pnt )
   {
    pntP = pointAddrP(*deltaDtmPP,pnt) ;
    pntP->z = pntP->z - elevation ;
   }
/*
** Recalculate Bounding Cube
*/
 (*deltaDtmPP)->zMin -= elevation;
 (*deltaDtmPP)->zMax -= elevation;
/*
** Clean Up
*/
 cleanup :   
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Elevation Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Elevation Error") ;
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
BENTLEYDTM_EXPORT int bcdtmDelta_cloneAndCreateDeltaTinToSurfaceDtmObject
(
 BC_DTM_OBJ *dtm1P,
 BC_DTM_OBJ *dtm2P,
 BC_DTM_OBJ **deltaDtmPP,
 DPoint3d        *deltaPtsP,
 long       numDeltaPts
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    pnt,intersectResult,numClipPts,tinReverse,startTime  ;
 DTMDirection direction;
 double  largestArea,area ;
 DPoint3d     *clipPtsP=NULL ;
 BC_DTM_OBJ       *dtm3P=NULL,*dtm4P=NULL,*saveTin ;
 DPoint3d    *pntP ;
 DTM_POLYGON_OBJ  *polyP=NULL ;
 DTM_POLYGON_LIST *plist1P,*plist2P ; 
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Surface") ;
    bcdtmWrite_message(0,0,0,"dtm1P       = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P       = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"deltaDtmPP  = %p",*deltaDtmPP) ;
    bcdtmWrite_message(0,0,0,"deltaPtsP   = %p",deltaPtsP) ;
    bcdtmWrite_message(0,0,0,"numDeltaPts = %4ld",numDeltaPts) ;
   }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit  ;
/*
** Write DTM's To File
*/
 if( dbg == 2 )
   {
    bcdtmWrite_toFileDtmObject(dtm1P,L"delta1.bcdtm") ;
    bcdtmWrite_toFileDtmObject(dtm2P,L"delta2.bcdtm") ;
   } 
/*
** Check If Both DTM Objects Are In TIN_STATE
*/
 if( (dtm1P)->dtmState != DTMState::Tin || (dtm2P)->dtmState != DTMState::Tin  )
   {
    if( (dtm1P)->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtm1P) ;
    if( (dtm2P)->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtm2P) ;
    goto errexit ;
   }
/*
** Check For Old Dtm (Tin) Files
*/
 if( (dtm1P)->ppTol == 0.0 || (dtm2P)->ppTol == 0.0 )
   {
    if( (dtm1P)->ppTol == 0.0 )bcdtmWrite_message(1,0,0,"Convert From Dtm File/Object") ;
    if( (dtm2P)->ppTol == 0.0 )bcdtmWrite_message(1,0,0,"Convert To Dtm File/Object")   ;
    ret = 20  ;
    goto errexit ;
   }
/*
** Get Intersect Polygons Between Tin Hulls And Delta Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Tin Hulls And Delta Polygon") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(dtm1P,dtm2P,deltaPtsP,numDeltaPts,&polyP,&intersectResult) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld ** Number Intersection Polygons = %2ld",intersectResult,polyP->numPolygons) ;
/*
** If No Intersection Found Return
*/
 if( ! intersectResult ) 
  {  
   bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
   goto errexit ;
  }
/*
**  If More Than One Intersect Polygon, Get Polygon With Largest Area
*/
 plist1P = polyP->polyListP ;
 if( polyP->numPolygons > 1 ) 
   {
    largestArea = (polyP->polyListP)->area ;
    for( plist2P = polyP->polyListP ; plist2P < polyP->polyListP + polyP->numPolygons ; ++plist2P) 
      { 
       if( plist2P->area > largestArea ) { largestArea = plist2P->area ; plist1P = plist2P ;} 
      }
   }
/*
** Copy Polygon Object Polygon To DPoint3d Polygon
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Intersection Polygon") ;
 if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plist1P-polyP->polyListP),&clipPtsP,&numClipPts)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Polygon = %p Number Of Points = %6ld",clipPtsP,numClipPts) ;
/*
**  Clip dtm1P Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cliping dtm1P => dtm3P") ;
 startTime = bcdtmClock() ;
 if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtm1P, &dtm3P, clipPtsP, numClipPts, DTMClipOption::External)) goto errexit;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"**** Time To Clip dtm1P     = %12.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtm3P,dtm3P->hullPoint,&area,&direction) ;
    bcdtmWrite_message(0,0,0,"**** Area Of Clip dtm1P Hull = %12.3lf",area) ;
    bcdtmWrite_toFileDtmObject(dtm3P,L"tin3.dtm") ;
    bcdtmWrite_message(0,0,0,"dtm3P->ppTol = %20.15lf dtm3P->plTol = %20.15lf dtm3P->mppTol = %20.15lf",dtm3P->ppTol,dtm3P->plTol,dtm3P->mppTol) ;
   }
/*
**  Clip dtm2P Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cliping dtm2P => Tin4") ;
 startTime = bcdtmClock() ;
 if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtm2P, &dtm4P, clipPtsP, numClipPts, DTMClipOption::External)) goto errexit;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"**** Time To Clip dtm2P      = %12.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtm4P,dtm4P->hullPoint,&area,&direction);
    bcdtmWrite_message(0,0,0,"**** Area Of Clip dtm2P Hull = %12.3lf",area) ;
    bcdtmWrite_toFileDtmObject(dtm4P,L"tin4.dtm") ;
    bcdtmWrite_message(0,0,0,"dtm4P->ppTol = %20.15lf dtm4P->plTol = %20.15lf dtm4P->mppTol = %20.15lf",dtm4P->ppTol,dtm4P->plTol,dtm4P->mppTol) ;
   }
/*
** Insert Tin With Smallest Number Of Points
*/
 tinReverse = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtm3P->numPoints = %7ld dtm4P->numPoints = %7ld",dtm3P->numPoints,dtm4P->numPoints) ;
 if( dtm4P->numPoints > dtm3P->numPoints )
   {
    tinReverse = 1 ;
    saveTin  = dtm3P ; 
    dtm3P = dtm4P ; 
    dtm4P = saveTin ;
   }
/*
**  Intersect dtm3P And dtm4P
*/ 
 if( bcdtmDelta_calculateDeltaTinToTinDtmObject(dtm3P,dtm4P)) goto errexit ; 
/*
** Negate z Values If Tins Reversed
*/
 if( tinReverse )
   {
    for( pnt = 0 ; pnt < dtm3P->numPoints ; ++pnt )
      {
       pntP = pointAddrP(dtm3P,pnt) ;
       pntP->z = - pntP->z ;
      }
/*
** Recalculate Bounding Cube
*/
    bcdtmMath_setBoundingCubeDtmObject(dtm3P) ;
   }
/*
** Set Delta Tin
*/
 *deltaDtmPP = dtm3P ;
  dtm3P = NULL ;
/*
** Check Tin
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Delta Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(*deltaDtmPP) )
      {
       bcdtmWrite_message(2,0,0,"Delta Dtm Invalid") ;
       goto errexit ;
      } 
    else bcdtmWrite_message(2,0,0,"Delta Dtm Valid") ;
   }
/*
** Cleanup
*/
 cleanup :
 if( clipPtsP  != NULL ) { free(clipPtsP) ; clipPtsP = NULL ; }
 if( polyP     != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;  
 if( dtm3P     != NULL ) bcdtmObject_destroyDtmObject(&dtm3P) ;
 if( dtm4P     != NULL ) bcdtmObject_destroyDtmObject(&dtm4P) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Surface Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Creating Delta Tin Surface To Surface Error") ;
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
BENTLEYDTM_EXPORT int bcdtmDelta_createDeltaLatticeToElevationDtmObject(BC_DTM_OBJ *dtm1P,DTM_LAT_OBJ **latticePP,DPoint3d *deltaPtsP,long numDeltaPts,double elevation,long numLatticePts )
/*
** This Function Creates a Delta Lattice To An Elevation
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ  *dtm2P=NULL ;
/*
** Write Status Message
*/
 bcdtmWrite_message(0,1,0,"Creating Delta Lattice Surface To Elevation") ;
/*
** Create Delta Tin
*/
 if( bcdtmDelta_createDeltaTinToElevationDtmObject(dtm1P,&dtm2P,deltaPtsP,numDeltaPts,elevation) != DTM_SUCCESS )  goto errexit ;
/*
** Create Lattice
*/
 if( bcdtmLattice_createLatticeFromDtmObject(dtm2P,latticePP,0,1,numLatticePts,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0) ) goto errexit ;
/*
** Cleanup
*/
 cleanup :
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDelta_createDeltaLatticeToSurfaceDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,DTM_LAT_OBJ **latticePP,DPoint3d *deltaPtsP,long numDeltaPts,long numLatticePts)
{
 int     ret=DTM_SUCCESS ;
 long    intersectResult,numPolyPts ;
 double  largestArea ;
 DPoint3d     *polyPtsP=NULL ;
 BC_DTM_OBJ       *dtm3P=NULL ;
 DTM_POLYGON_OBJ  *polyP=NULL ;
 DTM_POLYGON_LIST *plist1P,*plist2P ; 
/*
** Write Status Message
*/
 bcdtmWrite_message(0,1,0,"Creating Delta Surface To Surface") ;
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit  ;
/*
** Check If Both DTM Objects Are In TIN_STATE
*/
 if( dtm1P->dtmState != DTMState::Tin || dtm2P->dtmState != DTMState::Tin  )
   {
    if( dtm1P->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtm1P) ;
    if( dtm2P->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtm2P) ;
    goto errexit ;
   }
/*
** Check For Old Dtm (Tin) Files
*/
 if( dtm1P->ppTol == 0.0 || dtm2P->ppTol == 0.0 )
   {
    if( dtm1P->ppTol == 0.0 ) bcdtmWrite_message(1,0,0,"Convert From Dtm File/Object") ;
    if( dtm2P->ppTol == 0.0 ) bcdtmWrite_message(1,0,0,"Convert To Dtm File/Object")   ;
    ret = 20  ;
    goto errexit ;
   }
/*
** Get Intersect Polygons Between Tin Hulls And Delta Polygon
*/
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(dtm1P,dtm2P,deltaPtsP,numDeltaPts,&polyP,&intersectResult) ) goto errexit ;
/*
** If No Intersection Found Return
*/
 if( ! intersectResult ) 
  {  
   bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
   goto errexit ;
  }
/*
**  If More Than One Intersect Polygon Get Polygon With Largest Area
*/
 plist1P = polyP->polyListP ;
 if( polyP->numPolygons > 1 ) 
   {
    largestArea = (polyP->polyListP)->area ;
    for( plist2P = polyP->polyListP ; plist2P < polyP->polyListP + polyP->numPolygons ; ++plist2P ) 
      { 
       if( plist2P->area > largestArea ) 
         {
          largestArea = plist2P->area ;
          plist1P = plist2P ;
         }
      }
   }
/*
** Copy Polygon Object Polygon To DPoint3d Polygon
*/ 
 if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plist1P-polyP->polyListP),&polyPtsP,&numPolyPts)) goto errexit ;
/*
**  Clip dtm2P Object
*/
 if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtm2P, &dtm3P, polyPtsP, numPolyPts, DTMClipOption::External)) goto errexit;
/*
** Build Delta Lattice
*/
 if( bcdtmLattice_createIsopachLatticeFromDtmObjects(dtm1P,dtm3P,latticePP,1,numLatticePts,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( polyPtsP != NULL ) free(polyPtsP) ; 
 if( polyP    != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;  
 if( dtm3P    != NULL && dtm3P != dtm2P ) bcdtmObject_destroyDtmObject(&dtm3P) ; 
/*
** Job Complist1Peted
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDelta_calculateDeltaTinToTinDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Creates a Delta Tin To A Surface
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    node,insert,sp1,sp2,numDtm2Pts=0,numDtm2Lines=0,NumFeatPts=0,incPoints ;
 DPoint3d     *dtm2PtsP=NULL,*FeatPts=NULL ;
 PNTLINE *dtm2LinesP=NULL ;
 DTM_POLYGON_OBJ  *polyP=NULL,*intPolyP=NULL ;
 DTM_POLYGON_LIST *plistP ;
 DTMDirection direction;
 double   area ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Delta Tin To Tin") ;
/*
** Initialise
*/
 incPoints = dtm1P->incPoints ;
/*
** Copy Void And Islands To Polygon Object
*/
 if( bcdtmDelta_copyVoidsAndIslandsToPolygonObjectDtmObject(dtm1P,dtm2P,&polyP)) goto errexit ;
/*
** Remove DTM Features
*/
 if( bcdtmData_deleteAllDtmFeaturesDtmObject(dtm1P)) goto errexit ;
 for( node = 0 ; node < dtm1P->numPoints ; ++node ) nodeAddrP(dtm1P,node)->PCWD = 0 ;
 bcdtmList_cleanDtmObject(dtm1P) ;
 if( dbg )  bcdtmUtility_writeStatisticsDtmObject(dtm1P) ;
 if( bcdtmData_deleteAllDtmFeaturesDtmObject(dtm2P)) goto errexit ;
 for( node = 0 ; node < dtm2P->numPoints ; ++node ) nodeAddrP(dtm2P,node)->PCWD = 0 ;
 bcdtmList_cleanDtmObject(dtm2P) ;
 if( dbg )  bcdtmUtility_writeStatisticsDtmObject(dtm2P) ;
/*
** Intersect Voids And Islands In Polygon Object
*/
 if( polyP != NULL ) 
   { 
    if( bcdtmDelta_intersectVoidsAndIslands(polyP,&intPolyP)) goto errexit ;
   }
/*
** Insert Voids And Island Hulls Into Both TINs
*/
 if( intPolyP != NULL ) 
   {
    for( plistP = intPolyP->polyListP ; plistP < intPolyP->polyListP + intPolyP->numPolygons ; ++plistP ) 
      { 
       if( plistP->userTag == 1 || plistP->userTag == 2 )
         { 
          bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(intPolyP,(long)(plistP-intPolyP->polyListP),&FeatPts,&NumFeatPts) ;
          insert = bcdtmInsert_internalStringIntoDtmObject(dtm1P,1,2,FeatPts,NumFeatPts,&sp1) ;
          insert = bcdtmInsert_internalStringIntoDtmObject(dtm2P,1,2,FeatPts,NumFeatPts,&sp2) ;
          bcdtmList_nullTptrListDtmObject(dtm1P,sp1) ;
          bcdtmList_nullTptrListDtmObject(dtm2P,sp2) ;
          if( FeatPts != NULL ) { free(FeatPts) ; FeatPts = NULL ; }
         } 
      }
    bcdtmList_cleanDtmObject(dtm1P) ;
    bcdtmList_cleanDtmObject(dtm2P) ;
   } 
/*
** Save dtm2P Points
*/
 if( bcdtmDelta_copyTinPtsToP3DArrayDtmObject(dtm2P,&dtm2PtsP,&numDtm2Pts)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin 2 Points = %8ld",numDtm2Pts) ;
/*
** Save Primary Tin Lines To File
*/
 if( bcdtmDelta_copyTinLinesToPointLinesDtmObject(dtm2P,&dtm2LinesP,&numDtm2Lines) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin 2 Lines  = %8ld",numDtm2Lines) ;
/*
** Insert dtm2P Into Secondary dtm1P
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping dtm2P On dtm1P") ;
 dtm1P->incPoints = dtm2P->numPoints ;
 if ( bcdtmDelta_insertTinIntoTinDtmObject(dtm1P,&dtm2PtsP,numDtm2Pts,&dtm2LinesP,numDtm2Lines) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtm2P Draped On dtm1P") ;
/*
** Free Memory
*/
 if( dtm2PtsP   != NULL ) { free(dtm2PtsP)   ; dtm2PtsP   = NULL ; }
 if( dtm2LinesP != NULL ) { free(dtm2LinesP) ; dtm2LinesP = NULL ; }
/*
** Insert Voids And Islands Into TIN1
*/
 if( intPolyP != NULL ) 
   {
    if( dbg ) bcdtmWrite_message(1,0,0,"Inserting Voids And Islands In Delta Tin") ;
    for( plistP = intPolyP->polyListP ; plistP < intPolyP->polyListP + intPolyP->numPolygons ; ++plistP ) 
      { 
       if( plistP->userTag == 1 || plistP->userTag == 2 )
         {
          bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(intPolyP,(long)(plistP-intPolyP->polyListP),&FeatPts,&NumFeatPts) ;
          insert = bcdtmInsert_internalStringIntoDtmObject(dtm1P,1,2,FeatPts,NumFeatPts,&sp1) ;
          if( plistP->userTag == 1 )
            {
             bcdtmMath_getPolygonDirectionP3D(FeatPts,NumFeatPts,&direction,&area) ; 
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Adding Void   ** direction = %2ld area = %15.5lf",direction,area) ;
             if( area > 0.001 ) if( bcdtmInsert_addDtmFeatureToDtmObject(dtm1P,NULL,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,dtm1P->nullFeatureId,sp1,1)) goto errexit ; 
            }
          else
            {
             bcdtmMath_getPolygonDirectionP3D(FeatPts,NumFeatPts,&direction,&area) ; 
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Adding Island ** direction = %2ld area = %15.5lf",direction,area) ;
             if( area > 0.001 ) if( bcdtmInsert_addDtmFeatureToDtmObject(dtm1P,NULL,0,DTMFeatureType::Island,DTM_NULL_USER_TAG,dtm1P->nullFeatureId,sp1,1)) goto errexit ; 
            }
          if( FeatPts != NULL ) { free(FeatPts) ; FeatPts = NULL ; }
         } 
      }
    bcdtmMark_voidPointsDtmObject(dtm1P) ;
    bcdtmPolygon_deletePolygonObject(&intPolyP) ;
   } 
/*
** Clean Tin 1
*/
 if( bcdtmList_cleanDtmObject(dtm1P)) goto errexit ; ;
/*
** Drape Secondary Points Onto Primary Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Subtracting To-Tin From From-Tin") ;
 if( bcdtmDelta_drapeTin1PointsOnTin2DtmObject(dtm1P,dtm2P) ) goto errexit ;
/*
** Recalculate Bounding Cube
*/
 bcdtmMath_setBoundingCubeDtmObject(dtm1P) ;
/*
** Check Tin - Development Only
*/
 if( cdbg == 1 ) 
   { 
    bcdtmWrite_message(0,0,0,"Checking Delta Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtm1P)) goto errexit ; 
    bcdtmWrite_message(0,0,0,"Tin OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 dtm1P->incPoints = incPoints ;
 if( polyP      != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ; 
 if( intPolyP   != NULL ) bcdtmPolygon_deletePolygonObject(&intPolyP) ;
 if( dtm2PtsP   != NULL ) { free(dtm2PtsP)   ; dtm2PtsP   = NULL ; }
 if( dtm2LinesP != NULL ) { free(dtm2LinesP) ; dtm2LinesP = NULL ; }
 if( FeatPts    != NULL ) { free(FeatPts)    ; FeatPts = NULL ;   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Tin To Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Delta Tin To Tin Error") ;
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
BENTLEYDTM_Private int bcdtmDelta_drapeTin1PointsOnTin2DtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Drapes dtm1P Points On dtm2P
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long pnt,drapeResult  ; 
 DPoint3d  *pntP ;
 double z   ;
 long errCnt = 0 ;
/*
** Scan All Points And Determine If they Fall On Tin
*/
 for( pnt = 0 ; pnt < dtm1P->numPoints ; ++pnt )
   {
    pntP = pointAddrP(dtm1P,pnt) ;
    if( bcdtmDrape_pointDtmObject(dtm2P,pntP->x,pntP->y,&z,&drapeResult)) goto errexit ;
    if( ! drapeResult ) 
      {
       bcdtmWrite_message(1,0,0,"dtm1P Point[%8ld] ** %12.5lf %12.5lf %10.4lf ** External To dtm2P Hull",pnt,pntP->x,pntP->y,pntP->z) ; 
       ++errCnt ;
       if( errCnt > 100 ) goto errexit ;
      }
    pntP->z  = pntP->z - z ;
    if( dbg && pnt % 50000 == 0 ) bcdtmWrite_message(10,0,0,"Points Subtracted = %8ld of %8ld",pnt,dtm1P->numPoints)  ;
   }
 if( dbg ) 
   {
    bcdtmWrite_message(10,0,0,"Points Subtracted = %6ld of %6ld",pnt,dtm1P->numPoints) ;
    bcdtmWrite_message(10,0,0,"") ;
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
BENTLEYDTM_Private int bcdtmDelta_copyTinPtsToP3DArrayDtmObject(BC_DTM_OBJ *dtmP,DPoint3d **tinPtsPP,long *numTinPtsP)
/*
** This Function Copies The Tin Points To A Point Array
*/
{
 int   ret=DTM_SUCCESS ;
 long  pnt ;
 DPoint3d   *p3dP=NULL ;
 DPoint3d *pntP ; 
/*
** Allocate Memory
*/
 *numTinPtsP = dtmP->numPoints ;
 *tinPtsPP   = ( DPoint3d * ) malloc( *numTinPtsP * sizeof(DPoint3d) ) ;
 if( *tinPtsPP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ;
   }
/*
** Copy The Points
*/
 for( pnt = 0 , p3dP = *tinPtsPP ; pnt < dtmP->numPoints ; ++pnt , ++p3dP )
   { 
    pntP    = pointAddrP(dtmP,pnt) ;
    p3dP->x = pntP->x ; 
    p3dP->y = pntP->y ; 
    p3dP->z = pntP->z ; 
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
BENTLEYDTM_Private int bcdtmDelta_copyTinLinesToPointLinesDtmObject(BC_DTM_OBJ *dtmP,PNTLINE **tinLinesPP,long *numTinLinesP)
/*
** This Function Saves The Tin Line In Point Form
*/
{
 int     ret=DTM_SUCCESS ;
 long    p1,p2,clcPtr;
 PNTLINE *lineP ;
 DTM_CIR_LIST *clistP ;
/*
** Initialise Variables
*/
 *numTinLinesP = dtmP->numLines ;
/*
** Allocate Memory
*/
 *tinLinesPP = ( PNTLINE * ) malloc( *numTinLinesP * sizeof(PNTLINE) ) ;
 if( *tinLinesPP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Save The Tin Lines
*/
 lineP = *tinLinesPP ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clcPtr = nodeAddrP(dtmP,p1)->cPtr ;
    while ( clcPtr != DTM_NULL_PTR )
      {
       clistP = clistAddrP(dtmP,clcPtr) ;
       p2     = clistP->pntNum ;
       clcPtr = clistP->nextPtr ;
       if( p2 > p1 ) 
         { 
          lineP->P1 = p1 ; 
          lineP->P2 = p2 ; 
          ++lineP ;
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
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDelta_insertTinIntoTinDtmObject
(
 BC_DTM_OBJ *dtmP,
 DPoint3d        **tinPtsPP,
 long       numTinPts,
 PNTLINE    **tinLinesPP,
 long       numTinLines
)
/*
** This is the controlling Function for Inserting A Tin Into A Tin
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long      sp,np,p1,p2,cnt,*lpP,*pointsP=NULL,pntNum,incPoints,numMoved=0 ;
 DPoint3d       *p3dP  ;
 PNTLINE   *lineP   ;
 long      startTime ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP ;
/*
** Initialise
*/
 incPoints = dtmP->incPoints ;
/*
** Moving pointsP Within Tolerance To Tin pointsP
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Moving Points Within Tolerance")  ;
 for( p3dP = *tinPtsPP ; p3dP < *tinPtsPP + numTinPts ; ++p3dP )
   {
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&p1) ;
    pntP = pointAddrP(dtmP,p1) ;
    if( fabs(p3dP->x-pntP->x) <= dtmP->ppTol && fabs(p3dP->y-pntP->y) <= dtmP->ppTol )
      {
       if( bcdtmMath_distance(p3dP->x,p3dP->y,pntP->x,pntP->y) < dtmP->ppTol )
         { 
          p3dP->x = pntP->x ; 
          p3dP->y = pntP->y ; 
          ++numMoved ;
         }
      }   
   }
/*
** Write Move Times
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"**** Time To Move Points    = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Points Moved = %8ld",numMoved) ;
   }
/*
** Write Stats - Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->memPoints = %8ld dtmP->incPoints   = %8ld",dtmP->numPoints,dtmP->memPoints,dtmP->incPoints) ;
    bcdtmWrite_message(0,0,0,"numTinPts = %8ld",numTinPts) ;
   }
/*
** Set Memory Increment Parameters
*/
 if( dtmP->incPoints < numTinPts * 10 ) dtmP->incPoints = numTinPts * 10 ; 
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->incPoints   = %8ld",dtmP->incPoints) ;
/*
** Set Tolerances
*/
 dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 1000 ;
/*
** Store Points In Tin Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting To-Tin Points Into From-Tin") ;
 for( cnt = 0 , p3dP = *tinPtsPP ; p3dP < *tinPtsPP + numTinPts ; ++p3dP, ++cnt )
   {
    if( dbg && cnt % 1000 == 0 ) bcdtmWrite_message(0,0,0,"Points Inserted = %8ld of %8ld",cnt,numTinPts) ;
    if( bcdtmInsert_storePointInDtmObject(dtmP,1,1,p3dP->x,p3dP->y,p3dP->z,&pntNum) ) goto errexit ;
    if( dtmP->numPoints - dtmP->numSortedPoints > 1500 ) 
      { 
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Resorting Tin Structure") ;
       if( bcdtmTin_resortTinStructureDtmObject(dtmP) ) 
       goto errexit ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Points Inserted = %8ld of %8ld",cnt,numTinPts) ;
 if( bcdtmTin_resortTinStructureDtmObject(dtmP) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Time To Insert Points = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation After Inserting Points") ;
    if( bcdtmCheck_trianglesDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   } 
/*
** Allocate Memory To Hold To-Tin Point Numbers
*/
 pointsP = ( long * ) malloc( numTinPts * sizeof(long)) ;
 if( pointsP == NULL )  { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Get Point Numbers
*/
 startTime = bcdtmClock() ;
 for( p3dP = *tinPtsPP , lpP = pointsP ; p3dP < *tinPtsPP + numTinPts ; ++p3dP, ++lpP )
   {
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&pntNum) ;
    *lpP = pntNum ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Time To Get Point Numbers = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Free Unwanted Memory
*/
 free(*tinPtsPP) ; *tinPtsPP = NULL ;
/*
** Store Lines In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting To-Tin Lines Into From-Tin") ;
 dtmP->plTol = dtmP->ppTol ;
 for( cnt = 0 , lineP = *tinLinesPP ; lineP < *tinLinesPP + numTinLines ; ++cnt , ++lineP )
   {
    p1 = *(pointsP + lineP->P1) ;
    p2 = *(pointsP + lineP->P2) ;
    if( dbg && cnt % 1000 == 0 ) 
      {
       bcdtmWrite_message(10,0,0,"Lines Inserted = %6ld of %6ld ** dtmP->numPoints = %8ld dtmP->memPoints = %8ld",cnt,numTinLines,dtmP->numPoints,dtmP->memPoints) ;
       bcdtmWrite_message(0,0,0,"numPrecisionError = %8ld ** numSnapFix = %8ld",numPrecisionError,numSnapFix) ;
       if( cdbg )
         {
          if( bcdtmCheck_trianglesDtmObject(dtmP))
          goto errexit ;
         } 
      }
    if( p1 != p2 ) 
      {
       if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,p1,p2,1,2) == 1 ) goto errexit ;
       sp = p1 ;
       do
         {
          nodeP = nodeAddrP(dtmP,sp) ;
          np    = nodeP->tPtr ;
          nodeP->tPtr = dtmP->nullPnt ;
          sp = np ;
         } while( sp != dtmP->nullPnt ) ;  
      } 
   } 

 if( dbg )
   {
    bcdtmWrite_message(10,0,0,"Lines Inserted = %6ld of %6ld ** Tin pointsP = %9ld",cnt,numTinLines,dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"numPrecisionError = %8ld",numPrecisionError) ;
   }
/*
** Clean Up
*/
 cleanup :
 dtmP->incPoints = incPoints ;
 if( pointsP      != NULL ) { free(pointsP)     ; pointsP     = NULL ; }
 if( *tinLinesPP  != NULL ) { free(*tinLinesPP) ; *tinLinesPP = NULL ; }
 if( *tinPtsPP    != NULL ) { free(*tinPtsPP)   ; *tinPtsPP   = NULL ; }
 bcdtmWrite_message(10,0,0,"") ;
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
BENTLEYDTM_Private int bcdtmDelta_copyVoidsAndIslandsToPolygonObjectDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,DTM_POLYGON_OBJ **polyPP) 
/*
** This Function Copies The Voids And Islands To A Polygon Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Create Polygon Object
*/
 if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ;
/*
** Copy TIN1 Features
*/
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm1P,*polyPP,DTMFeatureType::Void,11)) goto errexit ;
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm1P,*polyPP,DTMFeatureType::Hole,11)) goto errexit ;
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm1P,*polyPP,DTMFeatureType::Island,12)) goto errexit ;
/*
** Copy TIN2 Features
*/
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm2P,*polyPP,DTMFeatureType::Void,21)) goto errexit ;
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm2P,*polyPP,DTMFeatureType::Hole,21)) goto errexit ;
if( bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(dtm2P,*polyPP,DTMFeatureType::Island,22)) goto errexit ;
/*
** If No Features Delete Polygon Object
*/
 if( (*polyPP)->numPolygons == 0 )  if( bcdtmPolygon_deletePolygonObject(polyPP)) goto errexit ;
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
BENTLEYDTM_Public int bcdtmDelta_intersectVoidsAndIslands(DTM_POLYGON_OBJ *Poly,DTM_POLYGON_OBJ **IntPoly ) 
/*
** This Function Intersects The Voids And Islands From Two Tins
** In Polygon Object IntPoly
*/
{
 long    Tin1Voids,Tin2Voids,ofs,npoly=0,tag  ;
 long    *TagList=NULL,NumTags ;
 long    Utag1,Utag2,Utag3,Utag4;
 DTM_POLYGON_LIST  *pl ;
 TAGOBJ  *IntTag=NULL ;
/*
** Check For Polygons In Polygon Object
*/
 if( Poly->numPolygons <= 0 ) return(0) ;
/*
** Check For Voids From Both Tins
*/
 Tin1Voids = Tin2Voids = 0 ;
 for( pl = Poly->polyListP ; pl < Poly->polyListP + Poly->numPolygons ; ++pl )
   {
    if( pl->userTag / 10 == 1 ) Tin1Voids = 1 ;
    if( pl->userTag / 10 == 2 ) Tin2Voids = 1 ;
   }
/*
** If No Voids In Either Tin Return
*/
 if( ! Tin1Voids && ! Tin2Voids ) return(0) ;
/*
** If Voids In Only One Tin Copy Polygon Object Polygons
*/
 if( ( Tin1Voids && ! Tin2Voids ) || ( ! Tin1Voids && Tin2Voids ))
   {
    if( bcdtmPolygon_createPolygonObject(IntPoly)) return(1) ; 
    if( bcdtmPolygon_copyPolygonObjectToPolygonObject(Poly,*IntPoly)) return(1) ;
    for( pl = (*IntPoly)->polyListP ; pl < (*IntPoly)->polyListP + (*IntPoly)->numPolygons ; ++pl ) pl->userTag = pl->userTag % 10 ;
    return(0) ;
   }
/*
** Intersect Polygons
*/
 if( bcdtmPolygon_intersectPolygonObjectPolygons(Poly,IntPoly,&IntTag) ) goto Error_Exit ;
/*
** Resolve Islands From Intersected Polygons
*/
 for( ofs = 0 ; ofs < (*IntPoly)->numPolygons ; ++ofs )
   {
    pl = (*IntPoly)->polyListP + ofs ;
    pl->userTag = 3 ;
    bcdtmPolygon_getTagListFromTagObject(IntTag,ofs,&TagList,&NumTags,&Utag1,&Utag2,&Utag3,&Utag4) ;
/*
** Check For Island Within Void From Same DTMFeatureState::Tin
*/
    if( NumTags == 1 && pl->s1 != DTM_NULL_PNT )
      {
       if( *TagList % 10 == 2 )
         { 
          tag = *TagList ;
          bcdtmPolygon_getTagListFromTagObject(IntTag,pl->s1,&TagList,&NumTags,&Utag1,&Utag2,&Utag3,&Utag4) ;
          if( NumTags == 1 && *TagList % 10 == 1 && tag / 10 == *TagList / 10 ) pl->userTag = 2 ;
         }
      }  
/*
** Check For Intersection Of Islands
*/      
    if( NumTags == 2 )
      {
       if( *TagList % 10 == 2 && *(TagList+1) % 10 == 2 ) pl->userTag = 2 ;
      } 
    if( NumTags == 1 )
      {
       if( *TagList % 10 == 2 ) pl->userTag = 2 ;
      } 
   }
/*
** Intersect Voids
*/
 npoly = (*IntPoly)->numPolygons ;
 if( bcdtmDelta_intersectVoidsInPolygonObject(Poly,IntPoly) ) goto Error_Exit ; 
 for( ofs = npoly ; ofs < (*IntPoly)->numPolygons ; ++ofs ) ((*IntPoly)->polyListP + ofs)->userTag = 1 ;
/*
** Error Exit 
*/
 Error_Exit :
/*
** Delete Objects
*/
 if( IntTag  != NULL ) bcdtmPolygon_deleteTagObject(&IntTag) ;
 if( TagList != NULL ) free(TagList) ;
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
BENTLEYDTM_Public int bcdtmDelta_intersectVoidsInPolygonObject
(
 DTM_POLYGON_OBJ *voidPolyP,
 DTM_POLYGON_OBJ **intVoidPolyPP
) 
/*
** This Function Intersects The Polygons In Polygon Object Poly And Stores Them
** In Polygon Object IntPoly
*/
{
 int ret=DTM_SUCCESS ;
 DTMDirection direction;
 long    numErrors  ;
 long    polyOfs, numVoidPts, dtmFeature;
 DTMFeatureType dtmFeatureType = DTMFeatureType::Void;
 double  area ;
 DPoint3d     *voidPtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
 DTM_POLYGON_LIST  *pl ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Copy Voids To DTM Object
*/
 for( pl = voidPolyP->polyListP ; pl < voidPolyP->polyListP + voidPolyP->numPolygons ; ++pl )
   {
    if( pl->userTag % 10 == 1 )
      {
       polyOfs = ( long ) ( pl - voidPolyP->polyListP ) ;
       if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(voidPolyP,polyOfs,&voidPtsP,&numVoidPts)) goto errexit ;
      }
   }
/*
** Intersect Voids
*/
 if( bcdtmString_intersectDtmFeaturesDtmObject(dtmP,&dtmFeatureType,1)) goto errexit ;
/*
** Validate Intersected Voids
*/
 if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,dtmP->ppTol,&numErrors)) goto errexit  ;
/*
** Create Polygon Object For Intersected Voids
*/
 if( *intVoidPolyPP == NULL ) 
   { 
    if( bcdtmPolygon_createPolygonObject(intVoidPolyPP)) goto errexit ;  
   }
/*
** Copy Voids To The Polygon Object
*/   
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Void )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&voidPtsP,&numVoidPts)) goto errexit ;
       bcdtmMath_getPolygonDirectionP3D(voidPtsP,numVoidPts,&direction,&area) ;
       if (direction == DTMDirection::Clockwise)
         {
          bcdtmMath_reversePolygonDirectionP3D(voidPtsP,numVoidPts) ;
         }
        if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*intVoidPolyPP,voidPtsP,numVoidPts,1)) goto errexit ;
      } 
   }
/*
** Cleanup
*/
 cleanup :
 if( voidPtsP != NULL ) free(voidPtsP) ;
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ; 
}
