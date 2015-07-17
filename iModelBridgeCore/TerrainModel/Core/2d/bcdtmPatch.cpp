/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmPatch.cpp $
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
BENTLEYDTM_EXPORT int  bcdtmPatch_surfaceToPlaneIsoPatchDtmFile
(
 WCharCP dtmFileP,
 WCharCP patchFileP,
 DPoint3d    *polygonPtsP,
 long   numPolygonPts,
 DTMFeatureCallback loadFunctionP,
 double elevation,
 long   loadOption,
 long   patchOption,
 long   numLatPts,
 double xinc,
 double yinc,
 double xreg,
 double yreg
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *dtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch DTM File") ; 
/*
** Delete Current Lattice Object
*/
/*
 if( DTM_CLOBJ != NULL ) bcdtmObject_deleteLatticeObject(&DTM_CLOBJ) ;
 DTM_CLOBJ_FILE[0] = 0 ;  
*/ 
/*
** Test If Requested DTM Is Current Tin
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Calculate Surface To Plane Patch
*/
 if( bcdtmPatch_surfaceToPlaneIsoPatchDtmObject(dtmP,&latticeP,polygonPtsP,numPolygonPts,loadFunctionP,elevation,loadOption,patchOption,numLatPts,xinc,yinc,xreg,yreg)) goto errexit ;
/*
** Set Currency
*/
/*
 DTM_CLOBJ = latticeP ;
 if( *patchFileP != 0 )
   {
    wcscpy(DTM_CLOBJ_FILE,patchFileP) ;
    if( bcdtmWrite_toFileLatticeObject(latticeP,patchFileP)) goto errexit ;
   } 
 else  wcscpy(DTM_CLOBJ_FILE,L"MEMORY.LAT") ;  
*/ 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch DTM File Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch DTM File Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( latticeP != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 goto cleanup ; 
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmPatch_surfaceToPlaneIsoPatchDtmObject
(
 BC_DTM_OBJ  *dtmP,
 DTM_LAT_OBJ **latticePP,
 DPoint3d         *polyPtsP,
 long        numPolyPts,
 DTMFeatureCallback loadFunctionP,
 double      elevation,
 long        loadOption,
 long        patchOption,
 long        numLatticePts,
 double      xinc,
 double      yinc,
 double      xreg,
 double      yreg
 ) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 double Xmin=0.0,Ymin=0.0,Ymax=0.0; 
 DPoint3d    *p3dP ;
 BC_DTM_OBJ *dtm2P=NULL ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"latticePP         = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"polyPtsP          = %p",polyPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolyPts        = %p",numPolyPts) ;
    bcdtmWrite_message(0,0,0,"elevation         = %8.2lf",elevation) ;
    bcdtmWrite_message(0,0,0,"loadOption        = %8ld",loadOption) ;
    bcdtmWrite_message(0,0,0,"patchOption       = %8ld",patchOption) ;
    bcdtmWrite_message(0,0,0,"numLatticePts     = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"xinc              = %8.2lf",xinc) ;
    bcdtmWrite_message(0,0,0,"yinc              = %8.2lf",yinc) ;
    bcdtmWrite_message(0,0,0,"xreg              = %8.2lf",xreg) ;
    bcdtmWrite_message(0,0,0,"yreg              = %8.2lf",yreg) ;
    if( dbg == 2 )
      {
       for( p3dP = polyPtsP ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"%10.3lf %10.4lf %10.4lf", p3dP->x,p3dP->y,p3dP->z) ;
         } 
      }
   }
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Null Lattice Object
*/
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
/*
** Create Lattice Object
*/
 if( bcdtmObject_createLatticeObject(latticePP)) goto errexit ;
/*
** Clip DTM To Patch Polygon
*/
 if( numPolyPts > 0 )
   {
    if( dbg )bcdtmWrite_message(0,0,0,"Clipping DTM Object To Volume Polygon") ;
    if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtmP, &dtm2P, polyPtsP, numPolyPts, DTMClipOption::External)) goto errexit;
   }
 else  dtm2P = dtmP ;
/*
** Build latticePP
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Lattice") ;
 if( bcdtmLattice_createLatticeFromDtmObject(dtm2P,latticePP,0,patchOption,numLatticePts,xinc,xreg,Xmin,Ymin,yinc,yreg,Ymin,Ymax)) goto errexit ;
/*
** Load Iso Lines
*/
 if( loadOption && patchOption == 3 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Surface To Plane Iso Lines") ;
    if( bcdtmPatch_loadBackSurfaceToPlaneIsoLines(*latticePP,elevation,loadFunctionP)) goto errexit ;
   }
/*
** Load Iso Cells
*/
 if( loadOption && patchOption == 1 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Surface To Plane Iso Cells") ;
    if( bcdtmPatch_loadBackSurfaceToPlaneIsoCells(*latticePP,elevation,loadFunctionP)) goto errexit ;
   }
/*
** Subtract elevation From Lattice
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Subtracting elevation From Lattice") ;
 if( bcdtmPatch_moveZElevationLatticeObject(*latticePP,elevation,2)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtm2P != dtmP && dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Plane Iso Patch Error") ; 
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
BENTLEYDTM_EXPORT int  bcdtmPatch_surfaceToSurfaceIsoPatchDtmFiles
(
 WCharCP dtmFile1P,
 WCharCP dtmFile2P,
 WCharCP patchFileP,
 DPoint3d    *polygonPtsP,
 long   numPolygonPts,
 DTMFeatureCallback loadFunctionP,
 long   loadOption,
 long   patchOption,
 long   numLatPts,
 double xinc,
 double yinc,
 double xreg,
 double yreg
) 
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d          *p3dP ;
 BC_DTM_OBJ   *dtm1P=NULL,*dtm2P=NULL ;
 DTM_LAT_OBJ  *latticeP=NULL,*fromLatticePP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Files") ; 
    bcdtmWrite_message(0,0,0,"dtmFile1P     = %s",dtmFile1P) ;
    bcdtmWrite_message(0,0,0,"dtmFile2P     = %s",dtmFile2P) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadOption    = %8ld",loadOption) ;
    bcdtmWrite_message(0,0,0,"patchOption   = %8ld",patchOption) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP   = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts = %8ld",numPolygonPts) ;
    for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Polygon Point[%4ld] = %10.3lf %10.4lf %10.4lf",(long)(p3dP-polygonPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Check For Different Files
*/
 if( wcscmp(dtmFile1P,dtmFile2P) == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"DTM Files Are The Same") ;
    goto errexit ;
   } 
/*
** Delete Current Lattice Object
*/
// if( DTM_CLOBJ != NULL ) bcdtmObject_deleteLatticeObject(&DTM_CLOBJ) ;
// DTM_CLOBJ_FILE[0] = 0 ;  
/*
** Read DTM Files Into Memory
*/
 if( bcdtmRead_fromFileDtmObject(&dtm1P,dtmFile1P)) goto errexit ;
 if( bcdtmRead_fromFileDtmObject(&dtm2P,dtmFile2P)) goto errexit ;
/*
** Calculate Surface To Surface IsoPach Lattice From DTM Objects
*/
 if( bcdtmPatch_surfaceToSurfaceIsoPatchDtmObjects(dtm1P,dtm2P,&latticeP,&fromLatticePP,polygonPtsP,numPolygonPts,loadFunctionP,loadOption,patchOption,numLatPts,xinc,yinc,xreg,yreg)) goto errexit ; 
/*
** Set Currency
*/
/*
 DTM_CLOBJ = latticeP ;
 if( *patchFileP != 0 )
   {
    wcscpy(DTM_CLOBJ_FILE,patchFileP) ;
    if( bcdtmWrite_toFileLatticeObject(latticeP,patchFileP)) goto errexit ;
   } 
 else  wcscpy(DTM_CLOBJ_FILE,L"MEMORY.LAT") ;  
*/ 
 /*
** Clean Up
*/
 cleanup :
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ; 
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ; 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Files Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Files Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmPatch_surfaceToSurfaceIsoPatchDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,
 BC_DTM_OBJ  *toDtmP,
 DTM_LAT_OBJ **refLatticePP,
 DTM_LAT_OBJ **isoLatticePP,
 DPoint3d         *polygonPtsP,
 long        numPolygonPts,
 DTMFeatureCallback loadFunctionP,
 long        loadOption,
 long        patchOption,
 long        numlatPts,
 double      xinc,
 double      yinc,
 double      xreg,
 double      yreg
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 double Xmin=0.0,Xmax=0.0,Ymin=0.0,Ymax=0.0 ;
 DPoint3d    *p3dP ;
 BC_DTM_OBJ *dtm3P=NULL,*dtm4P=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Objects") ; 
    bcdtmWrite_message(0,0,0,"fromDtmP      = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP        = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadOption    = %8ld",loadOption) ;
    bcdtmWrite_message(0,0,0,"patchOption   = %8ld",patchOption) ;
    bcdtmWrite_message(0,0,0,"xinc          = %8.3lf",xinc) ;
    bcdtmWrite_message(0,0,0,"yinc          = %8.3lf",yinc) ;
    bcdtmWrite_message(0,0,0,"xreg          = %8.3lf",xreg) ;
    bcdtmWrite_message(0,0,0,"yreg          = %8.3lf",yreg) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP   = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts = %8ld",numPolygonPts) ;
    for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Polygon Point[%4ld] = %10.3lf %10.4lf %10.4lf",(long)(p3dP-polygonPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Validate 
*/
 if( *refLatticePP != NULL ) bcdtmObject_deleteLatticeObject(refLatticePP) ;
 if( *isoLatticePP != NULL ) bcdtmObject_deleteLatticeObject(isoLatticePP) ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit ;
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM's") ;
    goto errexit ;
   }
/*
** Clip DTM Files To Patch Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Objects To Patch Polygon") ;
 if( bcdtmClip_toPolygonDtmObjects(fromDtmP,toDtmP,polygonPtsP,numPolygonPts,&dtm3P,&dtm4P) ) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_toFileDtmObject(dtm3P,L"dtm3.tin") ;
    bcdtmWrite_toFileDtmObject(dtm4P,L"dtm4.tin") ;
   }
/*
**  Build Lattices
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Isopach Lattice") ;
 if( bcdtmLattice_createReferenceAndIsopachLatticesFromDtmObjects(dtm3P,dtm4P,refLatticePP,isoLatticePP,patchOption,numlatPts,xinc,xreg,Xmin,Xmax,yinc,yreg,Ymin,Ymax ) ) goto errexit ;
/*
** Load Iso Lines
*/
 if( loadOption && patchOption == 3 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Isopach Lines") ; 
    if( bcdtmPatch_loadBackSurfaceToSurfaceIsoLines(*refLatticePP,*isoLatticePP,loadFunctionP)) goto errexit ;
   } 
/*
** Load Iso Cells
*/
 if( loadOption && patchOption == 1 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Isopach Cells") ; 
    if( bcdtmPatch_loadBackSurfaceToSurfaceIsoCells(*refLatticePP,*isoLatticePP,loadFunctionP)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( dtm3P != NULL && dtm3P != fromDtmP ) bcdtmObject_destroyDtmObject(&dtm3P) ; 
 if( dtm4P != NULL && dtm4P != toDtmP   ) bcdtmObject_destroyDtmObject(&dtm4P) ; 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Surface To Surface Iso Patch DTM Objects Error") ;
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
BENTLEYDTM_Public int bcdtmPatch_moveZElevationLatticeObject
(
 DTM_LAT_OBJ *latticeP,
 double      moveValue,
 long        moveOption
) 
/*
** This Function Moves The z Value Of The latticeP Object
**
** moveOption == 0  Set z to moveValue
**            == 1  Add moveValue To z
**            == 2  Subtract moveValue From z  
*/
{
 int ret=DTM_SUCCESS ;
 long i,j ;
/*
** Test For Valid latticeP Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Scan lattice array and move z values
*/
 for( j = 0 ; j < latticeP->NXL  ; ++j )
   {
    for( i = 0 ; i < latticeP->NYL  ; ++i )
      {
       if( *(latticeP->LAT + latticeP->NYL *  j + i)  != latticeP->NULLVAL )
         {
	      if( moveOption == 0 ) *(latticeP->LAT + latticeP->NYL *  j + i) = (float) moveValue ;
	      if( moveOption == 1 ) *(latticeP->LAT + latticeP->NYL *  j + i) = *(latticeP->LAT + latticeP->NYL *  j + i) + (float) moveValue ;
	      if( moveOption == 2 ) *(latticeP->LAT + latticeP->NYL *  j + i) = *(latticeP->LAT + latticeP->NYL *  j + i) - (float) moveValue ;
         }
      }
   } 
/*
** Adjust latticeP z Ranges
*/
 if( moveOption == 0 ) { latticeP->LZMIN = latticeP->LZMAX = (float) moveValue ; latticeP->LZDIF = 0.0 ; }
 if( moveOption == 1 ) { latticeP->LZMIN += (float) moveValue ; latticeP->LZMAX += (float) moveValue ; }
 if( moveOption == 2 ) { latticeP->LZMIN -= (float) moveValue ; latticeP->LZMAX -= (float) moveValue ; }
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
BENTLEYDTM_Public int bcdtmPatch_loadBackSurfaceToPlaneIsoLines
(
 DTM_LAT_OBJ *latticeP,
 double elevation,
 DTMFeatureCallback loadFunctionP
)
/*
**  This Function Loads The latticeP Points
*/
{
 int     ret=DTM_SUCCESS ;
 long    i,j ;
 float   zs ;
 double  x,y,z ;
/*
** Scan Lattice And Write Iso Lines
*/
 for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
   {
    for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
      {
       if( ( zs = *(latticeP->LAT + j*latticeP->NYL  + i)) != latticeP->NULLVAL  )  
         { 
          x = latticeP->DX * i  + latticeP->LXMIN ; 
          y = latticeP->DY * j  + latticeP->LYMIN ; 
          z = ( double ) zs ;
          if( bcdtmLoad_storePointInCache(x,y,z) ) goto errexit ;
          z = z - ( z - elevation ) ;
          if( bcdtmLoad_storePointInCache(x,y,z) ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::ISOLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,NULL) ) goto errexit ; 
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
BENTLEYDTM_Public int bcdtmPatch_loadBackSurfaceToSurfaceIsoLines
(
 DTM_LAT_OBJ *latticeP,
 DTM_LAT_OBJ *isoLatticeP,
 DTMFeatureCallback loadFunctionP
)
/*
**  This Function Loads The latticeP Points
*/
{
 int     ret=DTM_SUCCESS ;
 long    i,j ;
 float   zs ;
 double  x,y,z ;
/*
** Scan latticeP And Write Iso Lines
*/
 for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
   {
    for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
      {
       if( ( zs = *(latticeP->LAT + j*latticeP->NYL  + i)) != latticeP->NULLVAL &&
                  *(isoLatticeP->LAT + j*isoLatticeP->NYL  + i)  != isoLatticeP->NULLVAL    )  
         { 
          x = latticeP->DX * i  + latticeP->LXMIN ; 
          y = latticeP->DY * j  + latticeP->LYMIN ; 
          z = ( double ) zs ;
          if( bcdtmLoad_storePointInCache(x,y,z) ) goto errexit ;
          zs = *(isoLatticeP->LAT + j*isoLatticeP->NYL  + i ) ;
          z = z - (float) zs ;
          if( bcdtmLoad_storePointInCache(x,y,z) ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::ISOLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,NULL) ) goto errexit ; 
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
BENTLEYDTM_Public int bcdtmPatch_loadBackSurfaceToPlaneIsoCells
( 
 DTM_LAT_OBJ *latticeP,
 double      elevation,
 DTMFeatureCallback loadFunctionP
)
/*
**  This Function Loads The latticeP Points
*/
{
 int     ret=DTM_SUCCESS ;
 long    i,j ;
 float   z1,z2,z3,z4 ;
 double  x1,x2,y1,y2 ;
 DPoint3d     latPts[4] ;
/*
** Scan latticeP And Write Iso Lines
*/
 for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
      {
       z1 = *(latticeP->LAT + j*latticeP->NYL  + i)     ; z2 = *(latticeP->LAT+ j*latticeP->NYL  + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i + 1) ;
       if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
         {
          x1 = latticeP->DX * i  ; x2 = x1 + latticeP->DX ;
          y1 = latticeP->DY * j  ; y2 = y1 + latticeP->DY ;
/*
**       Get Surface Cell
*/
          latPts[0].x = x1 + latticeP->LXMIN ; latPts[0].y = y1 +  latticeP->LYMIN  ; latPts[0].z = z1 ;
          latPts[1].x = x2 + latticeP->LXMIN ; latPts[1].y = y1 +  latticeP->LYMIN  ; latPts[1].z = z2 ;
          latPts[2].x = x2 + latticeP->LXMIN ; latPts[2].y = y2 +  latticeP->LYMIN  ; latPts[2].z = z4 ;
          latPts[3].x = x1 + latticeP->LXMIN ; latPts[3].y = y2 +  latticeP->LYMIN  ; latPts[3].z = z3 ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[1].x,latPts[1].y,latPts[1].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[2].x,latPts[2].y,latPts[2].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[3].x,latPts[3].y,latPts[3].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
/*
**        Get Reference Cell
*/
          latPts[0].z = (float) z1 - ((float) z1 - elevation ) ; 
 	      latPts[1].z = (float) z2 - ((float) z2 - elevation ) ;
	      latPts[2].z = (float) z3 - ((float) z3 - elevation ) ; 
		  latPts[3].z = (float) z4 - ((float) z4 - elevation ) ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[1].x,latPts[1].y,latPts[1].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[2].x,latPts[2].y,latPts[2].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[3].x,latPts[3].y,latPts[3].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
/*
**        Write Iso Cell
*/
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::ISOCell,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,NULL) ) goto errexit ; 
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
BENTLEYDTM_Public int bcdtmPatch_loadBackSurfaceToSurfaceIsoCells
(
 DTM_LAT_OBJ *latticeP,
 DTM_LAT_OBJ *isoLatticeP,
 DTMFeatureCallback loadFunctionP
)
/*
**  This Function Loads The Iso Lattice Cells
*/
{
 int     ret=DTM_SUCCESS ;
 long    i,j ;
 float   z1,z2,z3,z4,p1,p2,p3,p4 ;
 double  x1,x2,y1,y2 ;
 DPoint3d     latPts[4] ;
/*
** Scan Lattice And Write Iso Lines
*/
 for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
      {
       z1 = *(latticeP->LAT + j*latticeP->NYL  + i)           ; z2 = *(latticeP->LAT+ j*latticeP->NYL  + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i)       ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i + 1) ;
       p1 = *(isoLatticeP->LAT + j*isoLatticeP->NYL  + i)     ; p2 = *(isoLatticeP->LAT+ j*isoLatticeP->NYL  + i + 1) ;
       p3 = *(isoLatticeP->LAT + (j+1)*isoLatticeP->NYL  + i) ; p4 = *(isoLatticeP->LAT + (j+1)*isoLatticeP->NYL  + i + 1) ;
       if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL &&
           p1 != isoLatticeP->NULLVAL && p2 != isoLatticeP->NULLVAL && p3 != isoLatticeP->NULLVAL && p4 != isoLatticeP->NULLVAL   )
         {
          x1 = latticeP->DX * i  ; x2 = x1 + latticeP->DX ;
          y1 = latticeP->DY * j  ; y2 = y1 + latticeP->DY ;
/*
**        Get Surface Cell
*/
          latPts[0].x = x1 + latticeP->LXMIN ; latPts[0].y = y1 +  latticeP->LYMIN  ; latPts[0].z = z1 ;
          latPts[1].x = x2 + latticeP->LXMIN ; latPts[1].y = y1 +  latticeP->LYMIN  ; latPts[1].z = z2 ;
          latPts[2].x = x2 + latticeP->LXMIN ; latPts[2].y = y2 +  latticeP->LYMIN  ; latPts[2].z = z4 ;
          latPts[3].x = x1 + latticeP->LXMIN ; latPts[3].y = y2 +  latticeP->LYMIN  ; latPts[3].z = z3 ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[1].x,latPts[1].y,latPts[1].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[2].x,latPts[2].y,latPts[2].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[3].x,latPts[3].y,latPts[3].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
/*
**        Get Reference Cell
*/
          latPts[0].z = (float) ( z1 - p1 ) ;
          latPts[1].z = (float) ( z2 - p2 ) ;
	      latPts[2].z = (float) ( z3 - p3 ) ; 
		  latPts[3].z = (float) ( z4 - p4 ) ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[1].x,latPts[1].y,latPts[1].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[2].x,latPts[2].y,latPts[2].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[3].x,latPts[3].y,latPts[3].z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(latPts[0].x,latPts[0].y,latPts[0].z)) goto errexit ;
/*
**        Write Iso Cell
*/
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::ISOCell,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,NULL) ) goto errexit ; 
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
