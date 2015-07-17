/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLatticeVolume.cpp $
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
BENTLEYDTM_EXPORT int bcdtmLatticeVolume_surfaceToElevationDtmFile
(
 WCharCP dtmFileP,                  /* ==> Pointer To Dtm File                           */
 long        numLatticePts,              /* ==> Number Of Lattice Points To Use               */ 
 VOLRANGETAB *rangeTableP,               /* ==> Pointer To Volume Range Table                 */
 long        numRanges,                  /* ==> Number Of Volume Ranges                       */
 DPoint3d         *polygonPtsP,               /* ==> Pointer To Volume Polygon Points              */
 long        numPolygonPts,              /* ==> Number Of Volume Polygon Points               */
 double      elevation,                  /* ==> Plane Elevation For Calculations              */
 DTMFeatureCallback loadFunctionP,          /* ==> Pointer To Load Function For Volume Polygons  */
 void        *userP,                     /* ==> User Pointer Passed Back To Caller            */
 double      *cutP,                      /* <== Volume Cut Between Dtm File And Elevation     */
 double      *fillP,                     /* <== Volume Fill Between Dtm File And Elevation    */
 double      *balanceP,                  /* <== Volume Balance Between Dtm File And Elevation */
 double      *areaP,                     /* <== Area Of Volume Calculation                    */
 long        *numCellsP,                 /* <== Actual Number Of Lattic Cells Used            */
 double      *cellAreaP                  /* <== Area Of A Lattice Cell                        */
)
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *dtmP=NULL ;
 double cutArea = 0, fillArea = 0;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes File") ; 
    bcdtmWrite_message(0,0,0,"dtmFileP      = %s",dtmFileP) ;
   }
/*
** Test If Requested Tin Is Current Tin
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit  ;
/*
** Calculate Surface To Plane Volume
*/
 if( bcdtmLatticeVolume_surfaceToElevationDtmObject (dtmP,numLatticePts,rangeTableP,numRanges,polygonPtsP,numPolygonPts,elevation,loadFunctionP,userP,*cutP,*fillP,*balanceP,cutArea, fillArea ,*numCellsP,*cellAreaP) ) goto errexit ;
 *areaP = cutArea + fillArea;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes File Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes File Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLatticeVolume_surfaceToElevationDtmObject
(
 BC_DTM_OBJ  *dtmP,                      /* ==> Pointer To Dtm Object                         */
 long        numLatticePts,              /* ==> Number Of Lattice Points To Use               */
 VOLRANGETAB *rangeTableP,               /* ==> Pointer To Volume Range Table                 */
 long        numRanges,                  /* ==> Number Of Volume Ranges                       */
 DPoint3d         *polygonPtsP,               /* ==> Pointer To Volume Polygon Points              */
 long        numPolygonPts,              /* ==> Number Of Volume Polygon Points               */
 double      elevation,                  /* ==> Plane Elevation For Calculations              */
 DTMFeatureCallback loadFunctionP,          /* ==> Pointer To Load Function For Volume Polygons  */
 void        *userP,                     /* ==> User Pointer Passed Back To Caller            */
 double      &cutP,                      /* <== Volume Cut Between Dtm File And Elevation     */
 double      &fillP,                     /* <== Volume Fill Between Dtm File And Elevation    */
 double      &balanceP,                  /* <== Volume Balance Between Dtm File And Elevation */
 double      &cutAreaP,                  /* <== Area Of Cut Volume Calculation                    */
 double      &fillAreaP,                 /* <== Area Of Fill Volume Calculation                    */
 long        &numCellsP,                 /* <== Actual Number Of Lattic Cells Used            */
 double      &cellAreaP                  /* <== Area Of A Lattice Cell                        */
)
{
/*
** This Function Calculates The Lattice Surface To Elevation Volumes
*/
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   intersectFlag ;  
 DPoint3d    *p3dP ;     
 VOLRANGETAB *vrtP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"numLatticePts  = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
    bcdtmWrite_message(0,0,0,"elevation      = %8.2lf",elevation) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP          = %p",userP) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Volume Ranges") ;
       for( vrtP = rangeTableP ; vrtP < rangeTableP + numRanges ; ++vrtP )
         {
          bcdtmWrite_message(0,0,0,"%12.4lf %12.4lf",vrtP->Low,vrtP->High) ;
         } 
       bcdtmWrite_message(0,0,0,"Polygon Points") ;
       for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Polygon Point[%6ld] ** %12.4lf %12.4lf %12.4lf",(long)(p3dP-polygonPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Check For Old Tin Files
*/
 if( dtmP->ppTol == 0.0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Convert Old Dtm (dtmP) File(s)") ;
    ret = 20 ; 
    goto errexit ;
   }
/*
** Initialise
*/
 cutP = fillP = balanceP = cutAreaP = fillAreaP = 0.0 ;
 if( numRanges > 0 )
   {
    for( vrtP = rangeTableP ; vrtP < rangeTableP + numRanges ; ++vrtP )
      { 
       vrtP->Cut  = 0.0 ;
       vrtP->Fill = 0.0 ;
      }  
   }
/*
** Get Intersection Polygons With Tin Hull And Volume Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygon And Tin Hull To Get Volume Polygon") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullDtmObject(dtmP,polygonPtsP,numPolygonPts,&polyP,&intersectFlag )) goto errexit ;
/*
** Calculate Lattice Volume
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Lattice Volumes") ;
 if( bcdtmLatticeVolume_calculateVolumeSurfaceToElevationDtmObject(dtmP,numLatticePts,polyP,intersectFlag,rangeTableP,numRanges,elevation,loadFunctionP,userP,cutP,fillP,balanceP,cutAreaP,fillAreaP, numCellsP,cellAreaP)) goto errexit ;
/*
** Write Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"cutAreaP = %10.4lf fillAreaP = %10.4lf cutP = %10.4lf fillP = %10.4lf balanceP = %10.4lf",cutAreaP,fillAreaP, cutP,fillP,balanceP) ;
/*
** Clean Up
*/
 cleanup :
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes Dtm Object Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Elevation Volumes Dtm Object Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLatticeVolume_calculateVolumeSurfaceToElevationDtmObject
(
 BC_DTM_OBJ      *dtmP,
 long            numLatticePts,
 DTM_POLYGON_OBJ *polyP,
 long            intersectFlag,
 VOLRANGETAB     *rangeTableP,
 long            numRanges,
 double          elevation,
 DTMFeatureCallback loadFunctionP,           /* ==> Pointer To Load Function For Volume Polygons */
 void            *userP,
 double          &cutP,
 double          &fillP,
 double          &balanceP,
 double          &cutAreaP,
 double          &fillAreaP,
 long            &numCellsP,
 double          &cellAreaP
) 
{
 int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long       i,j,nr,numPolygonPts,numLatPts ;
 float      z1,z2,z3,z4,*mp1,*mp2,*mp3,*mp4 ;
 double     ar,vol,height,hmin,hmax,hn,hm,zh,totalPolyArea ;
 BC_DTM_OBJ *clipDtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
 DPoint3d         *polygonPtsP=NULL ;
 DTM_POLYGON_LIST *plistPistP ;
/*
** Initialise
*/
 numCellsP = 0 ; 
 fillP = cutP = balanceP = cutAreaP = fillAreaP = 0.0 ; 
/*
** Scan Intersected Polygons And Accumulate Volumes
*/
  totalPolyArea = 0.0 ;
  for( plistPistP = polyP->polyListP ; plistPistP < polyP->polyListP + polyP->numPolygons ; ++plistPistP ) totalPolyArea = totalPolyArea + plistPistP->area ;
/*
** Scan Intersected Polygons And Accumulate Volume Calculations
*/
  for( plistPistP = polyP->polyListP ; plistPistP < polyP->polyListP + polyP->numPolygons ; ++plistPistP )
    {
     bcdtmWrite_message(10,0,0,"Processing Volume Polygon %3ld of %3ld",(long)(plistPistP-polyP->polyListP)+1,polyP->numPolygons) ;
/*
**   Clip Tin Object If Necessary
*/ 
     if( intersectFlag == 1 ) 
       {
        clipDtmP = dtmP ;
        bcdtmList_extractHullDtmObject(clipDtmP,&polygonPtsP,&numPolygonPts) ;
        if( loadFunctionP != NULL )
          {
           if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ; 
           if( loadFunctionP(DTMFeatureType::Polygon,dtmP->nullUserTag,dtmP->nullFeatureId,polygonPtsP,numPolygonPts,userP) != DTM_SUCCESS ) goto errexit ;
          }
       } 
     else 
       {
        if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plistPistP-polyP->polyListP),&polygonPtsP,&numPolygonPts)) goto errexit ;
        if( loadFunctionP != NULL )
          {
           if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ; 
           if( loadFunctionP(DTMFeatureType::Polygon,dtmP->nullUserTag,dtmP->nullFeatureId,polygonPtsP,numPolygonPts,userP) != DTM_SUCCESS ) goto errexit ;
          }
        if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ; 
        if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtmP, &clipDtmP, polygonPtsP, numPolygonPts, DTMClipOption::External)) goto errexit;
       }
/*
**  Build Lattice
*/
    numLatPts = (long)( (double)numLatticePts * ( plistPistP->area / totalPolyArea )) ; 
    if( numLatPts < 1000 ) numLatPts = 1000 ;
    if( bcdtmLattice_createLatticeFromDtmObject(clipDtmP,&latticeP,0,1,numLatPts,0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0) ) goto errexit ;
/*
** Calculate Volumes
*/
    ar = cellAreaP = latticeP->DX * latticeP->DY ;
/*
** Scan Lattice and Calculate Volumes
*/
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
         {
          mp1 = latticeP->LAT + latticeP->NYL *  j + i ;
          mp2 = latticeP->LAT + latticeP->NYL *  j + i + 1 ;
          mp3 = latticeP->LAT + latticeP->NYL * (j + 1) + i ;
          mp4 = latticeP->LAT + latticeP->NYL * (j + 1) + i + 1 ;
          z1 = *mp1 ; z2 = *mp2 ; z3 = *mp3 ; z4 = *mp4 ;
          if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
            {
             ++(numCellsP) ; 
             zh = (double) ( ( z1 + z2 + z3 + z4 ) / 4.0 ) ;
             height = zh - elevation ;
             vol    = fabs( ar * height) ;
             if (height >= 0.0)
                 {
                 cutP = cutP + vol; hmax = zh; hmin = elevation;
                 cutAreaP = cutAreaP + ar;
                 }
             else
                 {
                 fillP = fillP + vol; hmax = elevation; hmin = zh;
                 fillAreaP = fillAreaP + ar;
                 }
             for( nr = 0 ; nr < numRanges ; ++nr )
               {
                if( hmax > rangeTableP[nr].Low && hmin <  rangeTableP[nr].High )
                  {
                   hn = hmin ; hm = hmax ;
                   if( hn < rangeTableP[nr].Low  )  hn = rangeTableP[nr].Low ;
                   if( hm > rangeTableP[nr].High )  hm = rangeTableP[nr].High ;
                   vol = ar * ( hm - hn ) ;
                   if( height >= 0.0 ) rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + vol ;
                   else                rangeTableP[nr].Fill = rangeTableP[nr].Fill + vol ;
                  }
               }
            }
         }
      }
/*
** Free Memory
*/
    if( clipDtmP != dtmP && clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
    if( latticeP    != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
    if( polygonPtsP != NULL ) { free(polygonPtsP) ; polygonPtsP = NULL ; numPolygonPts = 0 ; }
   }
 bcdtmWrite_message(10,0,0,"") ;
/*
** Calculate balanceP
*/
 balanceP = cutP - fillP ;
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP    != dtmP && clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( latticeP    != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 if( polygonPtsP != NULL ) free(polygonPtsP) ; 
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLatticeVolume_surfaceToSurfaceDtmFiles
(
 WCharCP fromDtmFileP,              /* ==> Pointer To From Dtm File                      */
 WCharCP toDtmFileP,                /* ==> Pointer To To Dtm File                        */          
 long        numLatticePts,              /* ==> Number Of Lattice Points To Use               */ 
 VOLRANGETAB *rangeTableP,               /* ==> Pointer To Volume Range Table                 */
 long        numRanges,                  /* ==> Number Of Volume Ranges                       */
 DPoint3d         *polygonPtsP,               /* ==> Pointer To Volume Polygon Points              */
 long        numPolygonPts,              /* ==> Number Of Volume Polygon Points               */
 DTMFeatureCallback loadFunctionP,          /* ==> Pointer To Load Function For Volume Polygons  */
 void        *userP,                     /* ==> User Pointer Passed Back To Caller            */
 double      *cutP,                      /* <== Volume Cut Between Dtm Files                  */
 double      *fillP,                     /* <== Volume Fill Between Dtm Files                 */
 double      *balanceP,                  /* <== Volume Balance Between Dtm Files              */
 double      *areaP,                     /* <== Area Of Volume Calculation                    */
 long        *numCellsP,                 /* <== Actual Number Of Lattic Cells Used            */
 double      *cellAreaP                  /* <== Area Of A Lattice Cell                        */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *fromDtmP=NULL,*toDtmP=NULL ;
 double cutArea = 0, fillArea = 0;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Files") ; 
    bcdtmWrite_message(0,0,0,"fromDtmFileP = %s",fromDtmFileP) ;
    bcdtmWrite_message(0,0,0,"toDtmFileP   = %s",toDtmFileP) ;
   }
/*
** Check For Different Files
*/
 if( wcscmp(fromDtmFileP,toDtmFileP) == 0 )
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ;
    goto errexit  ;
   } 
/*
** Read Dtm Files Into Memory
*/
 if( bcdtmRead_fromFileDtmObject(&fromDtmP,fromDtmFileP)) goto errexit  ;
 if( bcdtmRead_fromFileDtmObject(&toDtmP,toDtmFileP))  goto errexit  ; 
/*
** Calculate Surface To Surface Tin Objects
*/
 if( bcdtmLatticeVolume_surfaceToSurfaceDtmObjects(fromDtmP,toDtmP,numLatticePts,rangeTableP,numRanges,polygonPtsP,numPolygonPts,loadFunctionP,userP,*cutP,*fillP,*balanceP,cutArea, fillArea, *numCellsP,*cellAreaP) ) goto errexit ;
 *areaP = cutArea + fillArea;
/*
** Clean Up
*/
 cleanup :
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ;
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Files Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Files Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLatticeVolume_surfaceToSurfaceDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,                  /* ==> Pointer To From Dtm Object                    */
 BC_DTM_OBJ  *toDtmP,                    /* ==> Pointer To To Dtm Object                      */
 long        numLatticePts,              /* ==> Number Of Lattice Points To Use               */ 
 VOLRANGETAB *rangeTableP,               /* ==> Pointer To Volume Range Table                 */
 long        numRanges,                  /* ==> Number Of Volume Ranges                       */
 DPoint3d         *polygonPtsP,               /* ==> Pointer To Volume Polygon Points              */
 long        numPolygonPts,              /* ==> Number Of Volume Polygon Points               */
 DTMFeatureCallback loadFunctionP,          /* ==> Pointer To Load Function For Volume Polygons  */
 void        *userP,                     /* ==> User Pointer Passed Back To Caller            */
 double      &cutP,                      /* <== Volume Cut Between Dtm Files                  */
 double      &fillP,                     /* <== Volume Fill Between Dtm Files                 */
 double      &balanceP,                  /* <== Volume Balance Between Dtm Files              */
 double      &cutAreaP,                     /* <== Area Of Volume Calculation                    */
 double      &fillAreaP,                     /* <== Area Of Volume Calculation                    */
 long        &numCellsP,                 /* <== Actual Number Of Lattic Cells Used            */
 double      &cellAreaP                  /* <== Area Of A Lattice Cell                        */
)
/*
** This Function Calculates The Lattice Volumes Between Two DTM Surfaces
*/
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long        intersectFlag ; 
 DPoint3d         *p3dP ;
 VOLRANGETAB *vrtP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"fromDtmP       = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP         = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP          = %p",userP) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Volume Ranges") ;
       for( vrtP = rangeTableP ; vrtP < rangeTableP + numRanges ; ++vrtP )
         {
          bcdtmWrite_message(0,0,0,"%12.4lf %12.4lf",vrtP->Low,vrtP->High) ;
         } 
       bcdtmWrite_message(0,0,0,"Polygon Points") ;
       for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Polygon Point[%6ld] ** %12.4lf %12.4lf %12.4lf",(long)(p3dP-polygonPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Initialise
*/
 cutP = fillP = balanceP = cutAreaP = fillAreaP = 0.0 ;
 if( numRanges > 0 )
   {
    for( vrtP = rangeTableP ; vrtP < rangeTableP + numRanges ; ++vrtP )
      { 
       vrtP->Cut  = 0.0 ;
       vrtP->Fill = 0.0 ;
      }  
   }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(toDtmP))   goto errexit  ;
/*
** Check Both DTM Objects Are In TIN_STATE
*/
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin  )
   {
    if( fromDtmP->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",fromDtmP) ;
    if( toDtmP->dtmState   != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",toDtmP) ;
    goto errexit ;
   }
/*
** Check For Old Dtm (Tin) Files
*/
 if( fromDtmP->ppTol == 0.0 || toDtmP->ppTol == 0.0 )
   {
    if( fromDtmP->ppTol == 0.0 )bcdtmWrite_message(1,0,0,"Convert From Dtm File/Object") ;
    if( toDtmP->ppTol   == 0.0 )bcdtmWrite_message(1,0,0,"Convert To Dtm File/Object")   ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Get Intersect Volume Polygons Between Tin Hulls And Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygon And Tin Hulls To Get Volume Polygons") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(fromDtmP,toDtmP,polygonPtsP,numPolygonPts,&polyP,&intersectFlag) ) goto errexit ;
/*
**  Calculate Surface To Surface Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Surface To Surface Volume") ;
 if( bcdtmLatticeVolume_calculateVolumeSurfaceToSurfaceDtmObjects(fromDtmP,toDtmP,polyP,0,numLatticePts,rangeTableP,numRanges,loadFunctionP,userP,cutP,fillP,balanceP,cutAreaP,fillAreaP, numCellsP,cellAreaP) ) goto errexit ;
/*
** Write Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"cutP = %10.4lf fillP = %10.4lf balanceP = %10.4lf cutAreaP = %10.4lf fillAreaP = %10.4lf ",cutP,fillP,balanceP,cutAreaP, fillAreaP) ;
/*
** Clean Up
*/
 cleanup :
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Objects Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Lattice Surface To Surface Volumes Dtm Objects Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLatticeVolume_calculateVolumeSurfaceToSurfaceDtmObjects
(
 BC_DTM_OBJ      *fromDtmP,
 BC_DTM_OBJ      *toDtmP,
 DTM_POLYGON_OBJ *polyP,
 long            intersectFlag,
 long            numLatticePts,
 VOLRANGETAB     *rangeTableP,
 long            numRanges,
 DTMFeatureCallback loadFunctionP,           /* ==> Pointer To Load Function For Volume Polygons */
 void            *userP,
 double          &cutP,
 double          &fillP,
 double          &balanceP,
 double          &cutAreaP,
 double          &fillAreaP,
 long            &numCellsP,
 double          &cellAreaP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   i,j,nr,lv,numVolLatticePts,numPolygonPts ;
 float  z1,z2,z3,z4    ;
 double ar,vol,height,hmin,hmax,hn,hm,zh,zs,totalPolygonArea ;
 DPoint3d    *polygonPtsP=NULL ;
 BC_DTM_OBJ  *dtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL,*rangeLatticeP=NULL ;
 DTM_POLYGON_LIST *plistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Lattice Volumes Surface To Surface") ;
/*
** Initialise
*/
 numCellsP = 0 ; 
 fillP = cutP = balanceP = cutAreaP = fillAreaP = cellAreaP = 0.0 ; 
/*
** Scan Intersected Polygons And Accumulate areaPs
*/
  totalPolygonArea = 0.0 ;
  for( plistP = polyP->polyListP ; plistP < polyP->polyListP + polyP->numPolygons ; ++plistP ) totalPolygonArea = totalPolygonArea + plistP->area ;
/*
** Scan Intersected Polygons And Accumulate Volume Calculations
*/
  for( plistP = polyP->polyListP ; plistP < polyP->polyListP + polyP->numPolygons ; ++plistP )
    {
     bcdtmWrite_message(10,0,0,"Processing Volume Polygon %3ld of %3ld",(long)(plistP-polyP->polyListP)+1,polyP->numPolygons) ;
/*
**   Clip Tin Object If Necessary
*/ 
     if( intersectFlag == 1 ) 
       {
        dtmP = toDtmP ;
        bcdtmList_extractHullDtmObject(toDtmP,&polygonPtsP,&numPolygonPts) ;
        if( loadFunctionP != NULL )
          {
           if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ; 
           if( loadFunctionP(DTMFeatureType::Polygon,toDtmP->nullUserTag,toDtmP->nullFeatureId,polygonPtsP,numPolygonPts,userP) != DTM_SUCCESS ) goto errexit ;
          }
       } 
     else 
       {
        if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plistP-polyP->polyListP),&polygonPtsP,&numPolygonPts))  goto errexit ;
        if( loadFunctionP != NULL )
          {
           if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ; 
           if( loadFunctionP(DTMFeatureType::Polygon,toDtmP->nullUserTag,toDtmP->nullFeatureId,polygonPtsP,numPolygonPts,userP) != DTM_SUCCESS ) goto errexit ;
          }
        if( dbg ) bcdtmWrite_message(0,0,0,"Cloning And Clipping Dtm Object") ;
        if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
        if (bcdtmClip_cloneAndClipToPolygonDtmObject (toDtmP, &dtmP, polygonPtsP, numPolygonPts, DTMClipOption::External)) goto errexit;
       }
/*
**  Create Isopach Lattice 
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Isopach Lattice") ;
    numVolLatticePts = (long)( (double)numLatticePts * ( plistP->area / totalPolygonArea )) ; 
    if( numVolLatticePts < 1000 ) numVolLatticePts = 1000 ;
    if( bcdtmLattice_createIsopachLatticeFromDtmObjects(fromDtmP,dtmP,&latticeP,1,numVolLatticePts,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0) ) goto errexit  ; 
/*
**  Write Lattice Stats
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Isopach Lattice ** NXL = %8ld NYL = %8ld",latticeP->NXL,latticeP->NYL) ;
/*
**  If Volume Ranges Set Creat Lattice For Ranges From Primary DTM
*/
    if( numRanges > 0 )
      {
       if( rangeLatticeP != NULL ) bcdtmObject_deleteLatticeObject(&rangeLatticeP) ;
       if( bcdtmObject_createLatticeObject(&rangeLatticeP)) goto errexit ;
       *rangeLatticeP = *latticeP ;
       rangeLatticeP->LAT = NULL ; 
       if( bcdtmLattice_populateLatticeDtmObject(fromDtmP,rangeLatticeP,0,NULL)) goto errexit ;
    
//       if( bcdtmLattice_createLatticeFromDtmObject(fromDtmP,&rangeLatticeP,0,1,numVolLatticePts,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0) ) goto errexit ;
/*
**  Write Lattice Stats
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Range Lattice   ** NXL = %8ld NYL = %8ld",rangeLatticeP->NXL,rangeLatticeP->NYL) ;
      }
/*
**  Calculate Volumes
*/
    ar = cellAreaP = latticeP->DX * latticeP->DY ;
/*
** Scan LAT Array And Calculate Volumes
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Lattice And Calculating Volumes") ;
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
         {
          z1 = *(latticeP->LAT + latticeP->NYL *  j + i);
          z2 = *(latticeP->LAT + latticeP->NYL *  j + i + 1);
          z3 = *(latticeP->LAT + latticeP->NYL * (j + 1) + i) ;
          z4 = *(latticeP->LAT + latticeP->NYL * (j + 1) + i + 1) ;
          if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
            {
             ++(numCellsP) ; 
             zh  = (double) ( ( z1 + z2 + z3 + z4 ) / 4.0 ) ;
/*
**           Round z Value
*/
             lv  = (long)(zh*100000.0) + 5 ;
             lv  = lv / 10 ;
             zh  = (float)(lv) / 10000.0 ;  
             vol = fabs( ar * zh ) ;
             if (zh >= 0.0)
                 {
                 cutP = cutP + vol;
                 cutAreaP = cutAreaP + ar;
                 }
             else
                 {
                 fillP = fillP + vol;
                 fillAreaP = fillAreaP + ar;
                 }
/*
**           Determine Range Volumes
*/
             if( numRanges > 0 )
               {
                height = fabs((double)zh) ;
                z1 = *( rangeLatticeP->LAT + rangeLatticeP->NYL * j + i ) ;
                z2 = *( rangeLatticeP->LAT + rangeLatticeP->NYL * j + i + 1 ) ;
                z3 = *( rangeLatticeP->LAT + rangeLatticeP->NYL * (j + 1) + i ) ;
                z4 = *( rangeLatticeP->LAT + rangeLatticeP->NYL * (j + 1) + i + 1 ) ;
                zs =  ( double ) (( z1+z2+z3+z4) / 4.0 ) ;
                if( zh >= 0.0 ) { hmax = zs ; hmin = zs - height ; }
                else            { hmin = zs ; hmax = zs + height ; }
                for( nr = 0 ; nr < numRanges ; ++nr )
                  {
                   if( hmax > rangeTableP[nr].Low && hmin <  rangeTableP[nr].High )
                     {
                      hn = hmin ; hm = hmax ;
                      if( hn < rangeTableP[nr].Low  )  hn = rangeTableP[nr].Low ;
                      if( hm > rangeTableP[nr].High )  hm = rangeTableP[nr].High ;
                      vol = ar * ( hm - hn ) ;
                      if( zh >= 0.0 ) rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + vol ;
                      else            rangeTableP[nr].Fill = rangeTableP[nr].Fill + vol ;
                     }
                  }
               }
            }
         }
      }
   } 
/*
** Calculate Balance
*/
 balanceP = cutP - fillP ;
/*
** Clean Up
*/
 cleanup :
 if( polygonPtsP   != NULL ) { free(polygonPtsP) ; polygonPtsP = NULL ; }
 if( latticeP      != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 if( rangeLatticeP != NULL ) bcdtmObject_deleteLatticeObject(&rangeLatticeP) ;
 if( dtmP != NULL && dtmP != toDtmP) bcdtmObject_destroyDtmObject(&dtmP) ; 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Lattice Volumes Surface To Surface Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Lattice Volumes Surface To Surface Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
