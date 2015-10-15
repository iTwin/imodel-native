/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmTinVolume.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
thread_local static long numVolTriangles=0 ; 
thread_local static double totTrgArea=0.0 ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToSurfaceBalanceDtmFiles
(
 WCharCP fromDtmFileP,    /* ==> Pointer To From Dtm File                     */
 WCharCP toDtmFileP,      /* ==> Pointer To To Dtm File                       */
 DPoint3d    *polygonPtsP,        /* ==> Pointer To Volume Polygon Points             */
 long   numPolygonPts,       /* ==> Number Of Volume Polygon Points              */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
 void   *userP,              /* ==> User Pointer Passed Back To Caller            */
 double *fromAreaP,          /* <== Area Of From Dtm File == toAreaP             */
 double *toAreaP,            /* <== Area Of To Dtm File   == fromPreaP           */
 double *balanceP            /* <== Volume Balance Between Dtm Files             */
)
/*
** This Function Calculates the Volume Balance Between Two Dtm Files
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *fromDtmP=NULL,*toDtmP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Files") ; 
    bcdtmWrite_message(0,0,0,"fromDtmFileP = %s",fromDtmFileP) ;
    bcdtmWrite_message(0,0,0,"toDtmFileP   = %s",toDtmFileP) ;
   }
/*
** Initialise
*/
 *fromAreaP = 0.0 ;
 *toAreaP   = 0.0 ;
 *balanceP  = 0.0 ;
/*
** Check For Different Files
*/
 if( wcscmp(fromDtmFileP,toDtmFileP) == 0 )
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ;
    goto errexit ;
   } 
/*
** Read From Dtm File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading From Dtm File %s",fromDtmFileP) ; 
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&fromDtmP,fromDtmFileP)) goto errexit ;
/*
** Read To Dtm File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading To Dtm File %s",toDtmFileP) ; 
 if( bcdtmRead_fromFileDtmObject(&toDtmP,toDtmFileP)) goto errexit ;
/*
** Calculate Surface Balance Dtm Objects
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Surface To Surface Balance") ; 
 if( bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects(fromDtmP,toDtmP,polygonPtsP,numPolygonPts,loadFunctionP,userP,fromAreaP,toAreaP,balanceP)) goto errexit ;
/*
** Delete To Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting To Dtm Object") ; 
 if( toDtmP != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Files Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Files Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToSurfaceBalanceDtmObjects
(
 BC_DTM_OBJ *fromDtmP,             /* ==> Pointer To From Dtm Object                   */ 
 BC_DTM_OBJ *toDtmP,               /* ==> Pointer To To Dtm Object                     */
 DPoint3d        *polygonPtsP,          /* ==> Pointer To Volume Polygon Points             */
 long       numPolygonPts,         /* ==> Number Of Volume Polygon Points              */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function For Volume Polygons */
 void       *userP,                /* ==> User Pointer Passed Back To Caller            */
 double     *fromAreaP,            /* <== Area Of From Dtm Object == toAreaP           */
 double     *toAreaP,              /* <== Area Of To Dtm Object   == fromAreaP         */
 double     *balanceP              /* <== Volume Balance Between Dtm Objects           */
)
/*
** This Function Calculates the Volume Balance Between Two Dtm Objects
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long   intersectFlag=0,numRanges=0 ;
 double cut,fill,balance1,balance2,elevation ;
 double xMin1,yMin1,zMin1,xMax1,yMax1,zMax1 ;
 double xMin2,yMin2,zMin2,xMax2,yMax2,zMax2 ;
 double xDif,yDif,zDif ; 
 VOLRANGETAB *rangeTableP=NULL ;
 DTM_POLYGON_OBJ *polyP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Objects") ; 
    bcdtmWrite_message(0,0,0,"fromDtmP      = %p",fromDtmP) ; 
    bcdtmWrite_message(0,0,0,"toDtmP        = %p",toDtmP) ; 
    bcdtmWrite_message(0,0,0,"polygonPtsP   = %p",polygonPtsP) ; 
    bcdtmWrite_message(0,0,0,"numPolygonPts = %8ld",numPolygonPts) ; 
   }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin  )
   {
    if( fromDtmP->dtmState != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",fromDtmP) ;
    if( toDtmP->dtmState   != DTMState::Tin ) bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",toDtmP) ;
    goto errexit ;
   }
/*
** Check For Old Tin Files
*/
 if( fromDtmP->ppTol == 0.0 || toDtmP->ppTol == 0.0) 
   { 
    bcdtmWrite_message(1,0,0,"Convert Old Dtm (dtmP) File(s)") ;
    return(20) ; 
   }
/*
** Initialise variables
*/
 *fromAreaP = *toAreaP = *balanceP = 0.0 ;
/*
** Get Elevation Value For Determing Balance
*/
 bcdtmUtility_getBoundingCubeDtmObject(fromDtmP,&xMin1,&yMin1,&zMin1,&xMax1,&yMax1,&zMax1,&xDif,&yDif,&zDif) ;
 bcdtmUtility_getBoundingCubeDtmObject(toDtmP,&xMin2,&yMin2,&zMin2,&xMax2,&yMax2,&zMax2,&xDif,&yDif,&zDif) ;
 if( zMin1 <= zMin2 ) elevation = zMin1 ;
 else                 elevation = zMin2 ;
/*
** Intersect Polygon And Tin Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Volume Polygon And Dtm Hull") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(fromDtmP,toDtmP,polygonPtsP,numPolygonPts,&polyP,&intersectFlag) ) goto errexit ;
 intersectFlag = 2 ;
/*
**  Calculate Volume For From DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Volume For From Dtm") ;
 if( bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject(fromDtmP,polyP,intersectFlag,rangeTableP,numRanges,elevation,loadFunctionP,userP,cut,fill,balance1,*fromAreaP)) goto errexit ;
/*
**  Calculate Volume For To Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Volume For To Dtm") ;
 if( bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject(toDtmP,polyP,intersectFlag,rangeTableP,numRanges,elevation,loadFunctionP,userP,cut,fill,balance2,*toAreaP)) goto errexit ;
/*
** Set Areas Balance Between Tins
*/
 *balanceP = balance1-balance2 ;
/*
** Clean Up
*/
 cleanup :
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Objects Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Balance Dtm Objects Error") ; 
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
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToElevationDtmFile
(
 WCharCP dtmFileP,           /* ==> Pointer To Dtm File                          */
 VOLRANGETAB *rangeTableP,        /* ==> Pointer To Volume Range Table                */
 long        numRanges,           /* ==> Number Of Volume Ranges                      */
 DPoint3d         *polygonPtsP,        /* ==> Pointer To Volume Polygon Points             */
 long        numPolygonPts,       /* ==> Number Of Volume Polygon Points              */
 double      elevation,           /* ==> Plane Elevation For Calculations             */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
 void        *userP,              /* ==> User Pointer Passed Back To Caller            */
 double      *cutP,               /* <== Volume Cut Between Dtm File And Plane        */
 double      *fillP,              /* <== Volume Fill Between Dtm File And Plane       */
 double      *balanceP,           /* <== Volume Balance Between Dtm File And Plane    */
 double      *areaP               /* <== Area Of Volume Calculation                   */
)
{
/*
** This Function Calculates The Surface To Plane Volumes For A Dtm File
*/
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Surface To Plane Dtm File") ;
    bcdtmWrite_message(0,0,0,"dtmFileP       = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
    bcdtmWrite_message(0,0,0,"elevation      = %8.2lf",elevation) ;
   }
/*
** Initialise
*/
 *cutP = *fillP = *balanceP = *areaP = 0.0 ;
/*
** Read From Dtm File
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Calculate Surface To Plane Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Surface To Plane Volumes") ;
 if( bcdtmTinVolume_surfaceToElevationDtmObject (dtmP,rangeTableP,numRanges,polygonPtsP,numPolygonPts,elevation,loadFunctionP,userP,*cutP,*fillP,*balanceP,*areaP)) goto errexit ;
/*
** Write Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cut = %12.4lf Fill = %12.4lf Balance = %12.4lf Area = %12.4lf",*cutP,*fillP,*balanceP,*areaP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Plane Dtm File Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Plane Dtm File Error") ; 
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
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToElevationDtmObject
(
BC_DTM_OBJ         *dtmP,             /* ==> Pointer To Dtm Object                        */
VOLRANGETAB        *rangeTableP,      /* ==> Pointer To Volume Range Table                */
long               numRanges,         /* ==> Number Of Volume Ranges                      */
DPoint3d           *polygonPtsP,      /* ==> Pointer To Volume Polygon Points             */
long               numPolygonPts,     /* ==> Number Of Volume Polygon Points              */
double             elevation,         /* ==> Plane Elevation For Calculations             */
DTMFeatureCallback loadFunctionP,     /* ==> Pointer To Load Function For Volume Polygons */
void               *userP,            /* ==> User Pointer Passed Back To Caller           */
double             &cutP,             /* <== Volume Cut Between Dtm Object And Plane      */
double             &fillP,            /* <== Volume Fill Between Dtm Object And Plane     */
double             &balanceP,         /* <== Volume Balance Between Dtm Object And Plane  */
double             &areaP             /* <== Area Of Volume Calculation                   */
)
    {
    double cutArea = 0;
    double fillArea = 0;
    int status = bcdtmTinVolume_surfaceToElevationDtmObject (dtmP, rangeTableP, numRanges, polygonPtsP, numPolygonPts, elevation, loadFunctionP, userP, cutP, fillP, balanceP, cutArea, fillArea);
    areaP = cutArea + fillArea;
    return status;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToElevationDtmObject
(
 BC_DTM_OBJ         *dtmP,             /* ==> Pointer To Dtm Object                        */
 VOLRANGETAB        *rangeTableP,      /* ==> Pointer To Volume Range Table                */
 long               numRanges,         /* ==> Number Of Volume Ranges                      */
 DPoint3d           *polygonPtsP,      /* ==> Pointer To Volume Polygon Points             */
 long               numPolygonPts,     /* ==> Number Of Volume Polygon Points              */
 double             elevation,         /* ==> Plane Elevation For Calculations             */
 DTMFeatureCallback loadFunctionP,     /* ==> Pointer To Load Function For Volume Polygons */
 void               *userP,            /* ==> User Pointer Passed Back To Caller           */
 double             &cutP,             /* <== Volume Cut Between Dtm Object And Plane      */
 double             &fillP,            /* <== Volume Fill Between Dtm Object And Plane     */
 double             &balanceP,         /* <== Volume Balance Between Dtm Object And Plane  */
 double             &cutAreaP,         /* <== Area Of Cut Volume Calculation               */
 double             &fillAreaP         /* <== Area Of Fill Volume Calculation              */
 )
{
/*
** This Function Calculates The Surface To Plane Volumes For A Dtm Object
*/
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long  intersectFlag=0 ;
 DPoint3d   *p3dP ;
 VOLRANGETAB *vrtP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Surface To Plane Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
    bcdtmWrite_message(0,0,0,"elevation      = %8.2lf",elevation) ;
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
    return(20) ; 
   }
/*
** Initialise
*/
 cutP = fillP = balanceP = cutAreaP = fillAreaP = 0.0;
 if( numRanges > 0 )
   {
    for( vrtP = rangeTableP ; vrtP < rangeTableP + numRanges ; ++vrtP )
      { 
       vrtP->Cut  = 0.0 ;
       vrtP->Fill = 0.0 ;
      }  
   }
/*
** Clip Dtm To Polygon
*/
 if( polygonPtsP == NULL ) numPolygonPts = 0 ;
 if( bcdtmPolygon_intersectPolygonAndTinHullDtmObject(dtmP,polygonPtsP,numPolygonPts,&polyP,&intersectFlag)) goto errexit ;
/*
** Calculate Dtm To Plane Volumes
*/
 if( bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject(dtmP,polyP,intersectFlag,rangeTableP,numRanges,elevation,loadFunctionP,userP,cutP,fillP,balanceP,cutAreaP, fillAreaP) ) goto errexit ;
/*
** Write Volumes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cut = %12.4lf Fill = %12.4lf Balance = %12.4lf CutArea = %12.4lf FillArea = %12.4lf",cutP,fillP,balanceP,cutAreaP, fillAreaP) ;
/*
** Clean Up
*/
 cleanup :
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Plane Dtm Object Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Plane Dtm Object Error") ; 
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
BENTLEYDTM_Private int bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject
(
BC_DTM_OBJ      *dtmP,
DTM_POLYGON_OBJ *polyP,
long            intersectFlag,
VOLRANGETAB     *rangeTableP,
long            numRanges,
double          elevation,
DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons  */
void            *userP,              /* ==> User Pointer Passed Back To Caller            */
double          &cutP,
double          &fillP,
double          &balanceP,
double          &areaP
)
    {
    double cutArea = 0;
    double fillArea = 0;
    int status = bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject (dtmP, polyP, intersectFlag, rangeTableP, numRanges, elevation, loadFunctionP, userP, cutP, fillP, balanceP, cutArea, fillArea);
    areaP = cutArea + fillArea;
    return status;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTinVolume_calculateVolumeSurfaceToPlaneDtmObject
(
BC_DTM_OBJ      *dtmP,
DTM_POLYGON_OBJ *polyP,
long            intersectFlag,
VOLRANGETAB     *rangeTableP,
long            numRanges,
double          elevation,
DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons  */
void            *userP,              /* ==> User Pointer Passed Back To Caller            */
double          &cutP,
double          &fillP,
double          &balanceP,
double          &cutAreaP,
double          &fillAreaP
)
/*
** This Function Calculates The Dtm Volumes Above and Below The Elevation
*/
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long   p1, p2, p3, clPtr, voidTriangle, numHullPts, dtmFeature, voidsInDtm = FALSE;
    double cutTriangle, fillTriangle, cutAreaTriangle, fillAreaTriangle;
    DPoint3d    *hullPtsP = NULL;
    BC_DTM_OBJ *clipDtmP = NULL;
    DTM_POLYGON_LIST *pListP;
    BC_DTM_FEATURE *dtmFeatureP;
    /*
    ** Initialise Variables
    */
    cutP = fillP = balanceP = cutAreaP = fillAreaP = 0.0;
    /*
    ** Check For Voids In DTM
    */
    voidsInDtm = FALSE;
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures && voidsInDtm == FALSE; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island)) voidsInDtm = TRUE;
        }
    /*
    ** Scan Intersected Polygons And Accumulate Areas
    */
    for (pListP = polyP->polyListP; pListP < polyP->polyListP + polyP->numPolygons; ++pListP)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Processing Volume Polygon %3ld of %3ld", (long)(pListP - polyP->polyListP) + 1, polyP->numPolygons);
        /*
        **   Clip Dtm Object If Necessary
        */
        if (intersectFlag == 1)
            {
            clipDtmP = dtmP;
            bcdtmList_extractHullDtmObject (clipDtmP, &hullPtsP, &numHullPts);
            if (loadFunctionP != NULL)
                {
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Calling Load Function = %p", loadFunctionP);
                if (loadFunctionP (DTMFeatureType::Polygon, dtmP->nullUserTag, dtmP->nullFeatureId, hullPtsP, numHullPts, userP) != DTM_SUCCESS) goto errexit;
                }
            }
        else
            {
            if (bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon (polyP, (long)(pListP - polyP->polyListP), &hullPtsP, &numHullPts)) goto errexit;
            if (loadFunctionP != NULL)
                {
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Calling Load Function = %p", loadFunctionP);
                if (loadFunctionP (DTMFeatureType::Polygon, dtmP->nullUserTag, dtmP->nullFeatureId, hullPtsP, numHullPts, userP) != DTM_SUCCESS) goto errexit;
                }
            if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtmP, &clipDtmP, hullPtsP, numHullPts, DTMClipOption::External)) goto errexit;
            }
        /*
        ** Scan Circular List and Extract each Traingle
        */
        for (p1 = 0; p1 < clipDtmP->numPoints; ++p1)
            {
            if ((clPtr = nodeAddrP (clipDtmP, p1)->cPtr) != clipDtmP->nullPtr)
                {
                if ((p2 = bcdtmList_nextAntDtmObject (clipDtmP, p1, clistAddrP (clipDtmP, nodeAddrP (clipDtmP, p1)->cPtr)->pntNum)) < 0) goto errexit;
                while (clPtr != clipDtmP->nullPtr)
                    {
                    p3 = clistAddrP (clipDtmP, clPtr)->pntNum;
                    if (p2 > p1 && p3 > p1 && nodeAddrP (clipDtmP, p3)->hPtr != p1)
                        {
                        voidTriangle = FALSE;
                        if (voidsInDtm == TRUE) if (bcdtmList_testForVoidTriangleDtmObject (clipDtmP, p1, p2, p3, &voidTriangle)) goto errexit;
                        if (!voidTriangle)
                            {
                            bcdtmTinVolume_prismToFlatPlaneDtmObject (clipDtmP, p1, p2, p3, elevation, cutTriangle, fillTriangle, cutAreaTriangle, fillAreaTriangle);
                            cutP += cutTriangle;
                            fillP += fillTriangle;
                            cutAreaP += cutAreaTriangle;
                            fillAreaP += fillAreaTriangle;
                            if (numRanges > 0)
                                {
                                if (bcdtmTinVolume_calculateRangeVolumes (cutTriangle, fillTriangle, rangeTableP, numRanges, pointAddrP (clipDtmP, p1)->x, pointAddrP (clipDtmP, p1)->y, elevation, pointAddrP (clipDtmP, p2)->x, pointAddrP (clipDtmP, p2)->y, elevation, pointAddrP (clipDtmP, p3)->x, pointAddrP (clipDtmP, p3)->y, elevation, pointAddrP (clipDtmP, p1)->z, pointAddrP (clipDtmP, p2)->z, pointAddrP (clipDtmP, p3)->z)) goto errexit;
                                }
                            }
                        }
                    clPtr = clistAddrP (clipDtmP, clPtr)->nextPtr;
                    p2 = p3;
                    }
                }
            }
        /*
        **  Delete Dtm Object
        */
        if (clipDtmP != NULL && clipDtmP != dtmP) bcdtmObject_destroyDtmObject (&clipDtmP);
        if (hullPtsP != NULL)
            {
            free (hullPtsP); hullPtsP = NULL;
            }
        }
    /*
    ** Calculate Balance
    */
    balanceP = cutP - fillP;
    /*
    ** Clean Up
    */
cleanup:
    if (clipDtmP != NULL && clipDtmP != dtmP) bcdtmObject_destroyDtmObject (&clipDtmP);
    if (hullPtsP != NULL)
        {
        free (hullPtsP); hullPtsP = NULL;
        }
    /*
    ** Job Completed
    */
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS)  ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int bcdtmTinVolume_prismoidalVolumeArea (double elevation, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double X3, double Y3, double Z3, double &cutP, double &fillP, double &cutAreaP, double &fillAreaP)
    {
    double height, Zmin, Zmax;
    /*
    ** Initialise Variables
    */
    cutP = fillP = cutAreaP = fillAreaP = 0.0;
    /*
    ** Get Max & Min Z Values
    */
    Zmin = Zmax = Z1;
    if (Z2 < Zmin) Zmin = Z2; if (Z2 > Zmax) Zmax = Z2;
    if (Z3 < Zmin) Zmin = Z3; if (Z3 > Zmax) Zmax = Z3;
    /*
    ** Calculate Volume
    */
    if (elevation <= Zmin)
        {
        height = (Z1 + Z2 + Z3) / 3.0 - elevation;
        cutAreaP = bcdtmMath_coordinateTriangleArea (X1, Y1, X2, Y2, X3, Y3);
        cutP = cutAreaP * height;
        return(DTM_SUCCESS);
        }
    if (elevation >= Zmax)
        {
        height = elevation - (Z1 + Z2 + Z3) / 3.0;
        fillAreaP = bcdtmMath_coordinateTriangleArea (X1, Y1, X2, Y2, X3, Y3);
        fillP = fillAreaP * height;
        return(DTM_SUCCESS);
        }
    /*
    ** Job Completed
    */
    return(DTM_SUCCESS);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_prismToFlatPlaneDtmObject (BC_DTM_OBJ *dtmP, long P1, long P2, long P3, double elevation, double &cutP, double &fillP, double &cutAreaP, double &fillAreaP)
/*
** This Function Calculates the Volume Of A Prism Othogonal With A Flat Plane
*/
    {
    double X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3;
    /*
    ** Initialise varaibles
    */
    X1 = pointAddrP (dtmP, P1)->x; Y1 = pointAddrP (dtmP, P1)->y; Z1 = pointAddrP (dtmP, P1)->z;
    X2 = pointAddrP (dtmP, P2)->x; Y2 = pointAddrP (dtmP, P2)->y; Z2 = pointAddrP (dtmP, P2)->z;
    X3 = pointAddrP (dtmP, P3)->x; Y3 = pointAddrP (dtmP, P3)->y; Z3 = pointAddrP (dtmP, P3)->z;

    return bcdtmTinVolume_prismToFlatPlane (elevation, X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, cutP, fillP, cutAreaP, fillAreaP);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTinVolume_prismToFlatPlane (double elevation, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double X3, double Y3, double Z3, double *cutP, double *fillP, double *areaP)
    {
    double cutArea = 0, fillArea = 0;
    int status = bcdtmTinVolume_prismToFlatPlane (elevation, X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, *cutP, *fillP, cutArea, fillArea);
    *areaP = cutArea + fillArea;
    return status;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTinVolume_prismToFlatPlane (double elevation, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double X3, double Y3, double Z3, double &cutP, double &fillP, double &cutAreaP, double &fillAreaP)
/*
** This Function Calculates the Volume Of A Prism Othogonal With A Flat Plane
*/
    {
    double height, planeCut, planeFill, planeCutArea, planeFillArea;
    double X4, Y4, Z4, X5, Y5, Z5, Zmin, Zmax;
    /*
    ** Initialise varaibles
    */
    cutP = fillP = cutAreaP = fillAreaP = 0.0;
    Zmin = Zmax = Z1;
    if (Z2 < Zmin) Zmin = Z2; if (Z2 > Zmax) Zmax = Z2;
    if (Z3 < Zmin) Zmin = Z3; if (Z3 > Zmax) Zmax = Z3;
    Z4 = Z5 = elevation;

    /*
    ** Calculate Volume
    */
    if (elevation <= Zmin)
        {
        height = (Z1 + Z2 + Z3) / 3.0 - elevation;
        cutP = bcdtmMath_coordinateTriangleArea (X1, Y1, X2, Y2, X3, Y3) * height;
        if (height == 0.0)
            cutAreaP = 0;
        else
            cutAreaP = 0.5 * (X1*Y2 + Y1*X3 + Y3*X2 - Y2*X3 - Y1*X2 - X1*Y3);
        if (cutAreaP < 0.0) cutAreaP = -cutAreaP;
        fillAreaP = 0;
        return(DTM_SUCCESS);
        }

    if (elevation >= Zmax)
        {
        height = elevation - (Z1 + Z2 + Z3) / 3.0;
        fillP = bcdtmMath_coordinateTriangleArea (X1, Y1, X2, Y2, X3, Y3)  * height;
        if (height == 0.0)
            fillAreaP = 0;
        else
            fillAreaP = 0.5 * (X1*Y2 + Y1*X3 + Y3*X2 - Y2*X3 - Y1*X2 - X1*Y3);
        if (fillAreaP < 0.0) fillAreaP = -fillAreaP;
        cutAreaP = 0;
        return(DTM_SUCCESS);
        }
    if ((elevation >= Z1 && elevation < Z2 && elevation < Z3) ||
        (elevation <= Z1 && elevation > Z2 && elevation > Z3))
        {
        if (elevation > Z1)
            {
            X4 = X1 + (X2 - X1) * (elevation - Z1) / (Z2 - Z1);
            Y4 = Y1 + (Y2 - Y1) * (elevation - Z1) / (Z2 - Z1);
            X5 = X1 + (X3 - X1) * (elevation - Z1) / (Z3 - Z1);
            Y5 = Y1 + (Y3 - Y1) * (elevation - Z1) / (Z3 - Z1);
            }
        else
            {
            X4 = X1 + (X2 - X1) * (Z1 - elevation) / (Z1 - Z2);
            Y4 = Y1 + (Y2 - Y1) * (Z1 - elevation) / (Z1 - Z2);
            X5 = X1 + (X3 - X1) * (Z1 - elevation) / (Z1 - Z3);
            Y5 = Y1 + (Y3 - Y1) * (Z1 - elevation) / (Z1 - Z3);
            }
        cutAreaP = fillAreaP = 0;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X1, Y1, Z1, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP += planeCut;
        fillP += planeFill;
        cutAreaP += planeCutArea;
        fillAreaP += planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X2, Y2, Z2, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP += planeCut;
        fillP += planeFill;
        cutAreaP += planeCutArea;
        fillAreaP += planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X3, Y3, Z3, X5, Y5, Z5, X2, Y2, Z2, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP += planeCut;
        fillP += planeFill;
        cutAreaP += planeCutArea;
        fillAreaP += planeFillArea;
        return(DTM_SUCCESS);
        }

    if ((elevation >= Z2 && elevation < Z1 && elevation < Z3) ||
        (elevation <= Z2 && elevation > Z1 && elevation > Z3))
        {
        if (elevation > Z2)
            {
            X4 = X2 + (X1 - X2) * (elevation - Z2) / (Z1 - Z2);
            Y4 = Y2 + (Y1 - Y2) * (elevation - Z2) / (Z1 - Z2);
            X5 = X2 + (X3 - X2) * (elevation - Z2) / (Z3 - Z2);
            Y5 = Y2 + (Y3 - Y2) * (elevation - Z2) / (Z3 - Z2);
            }
        else
            {
            X4 = X2 + (X1 - X2) * (Z2 - elevation) / (Z2 - Z1);
            Y4 = Y2 + (Y1 - Y2) * (Z2 - elevation) / (Z2 - Z1);
            X5 = X2 + (X3 - X2) * (Z2 - elevation) / (Z2 - Z3);
            Y5 = Y2 + (Y3 - Y2) * (Z2 - elevation) / (Z2 - Z3);
            }
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X2, Y2, Z2, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X1, Y1, Z1, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X3, Y3, Z3, X5, Y5, Z5, X1, Y1, Z1, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        return(DTM_SUCCESS);
        }

    if ((elevation >= Z3 && elevation < Z1 && elevation < Z2) ||
        (elevation <= Z3 && elevation > Z1 && elevation > Z2))
        {
        if (Z3 < elevation)
            {
            X4 = X3 + (X1 - X3) * (elevation - Z3) / (Z1 - Z3);
            Y4 = Y3 + (Y1 - Y3) * (elevation - Z3) / (Z1 - Z3);
            X5 = X3 + (X2 - X3) * (elevation - Z3) / (Z2 - Z3);
            Y5 = Y3 + (Y2 - Y3) * (elevation - Z3) / (Z2 - Z3);
            }
        else
            {
            X4 = X3 + (X1 - X3) * (Z3 - elevation) / (Z3 - Z1);
            Y4 = Y3 + (Y1 - Y3) * (Z3 - elevation) / (Z3 - Z1);
            X5 = X3 + (X2 - X3) * (Z3 - elevation) / (Z3 - Z2);
            Y5 = Y3 + (Y2 - Y3) * (Z3 - elevation) / (Z3 - Z2);
            }
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X3, Y3, Z3, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X1, Y1, Z1, X4, Y4, Z4, X5, Y5, Z5, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X2, Y2, Z2, X5, Y5, Z5, X1, Y1, Z1, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        return(DTM_SUCCESS);
        }


    if ((elevation == Z1 && elevation > Z2 && elevation < Z3) ||
        (elevation == Z1 && elevation < Z2 && elevation > Z3))
        {
        if (elevation > Z2)
            {
            X4 = X2 + (X3 - X2) * (elevation - Z2) / (Z3 - Z2);
            Y4 = Y2 + (Y3 - Y2) * (elevation - Z2) / (Z3 - Z2);
            }
        else
            {
            X4 = X2 + (X3 - X2) * (Z2 - elevation) / (Z2 - Z3);
            Y4 = Y2 + (Y3 - Y2) * (Z2 - elevation) / (Z2 - Z3);
            }
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X1, Y1, Z1, X2, Y2, Z2, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X1, Y1, Z1, X3, Y3, Z3, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        return(DTM_SUCCESS);
        }

    if ((elevation == Z2 && elevation > Z1 && elevation < Z3) ||
        (elevation == Z2 && elevation < Z1 && elevation > Z3))
        {
        if (elevation > Z1)
            {
            X4 = X1 + (X3 - X1) * (elevation - Z1) / (Z3 - Z1);
            Y4 = Y1 + (Y3 - Y1) * (elevation - Z1) / (Z3 - Z1);
            }
        else
            {
            X4 = X1 + (X3 - X1) * (Z1 - elevation) / (Z1 - Z3);
            Y4 = Y1 + (Y3 - Y1) * (Z1 - elevation) / (Z1 - Z3);
            }
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X2, Y2, Z2, X1, Y1, Z1, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        return(DTM_SUCCESS);
        }

    if ((elevation == Z3 && elevation > Z1 && elevation < Z2) ||
        (elevation == Z3 && elevation < Z1 && elevation > Z2))
        {
        if (elevation > Z1)
            {
            X4 = X1 + (X2 - X1) * (elevation - Z1) / (Z2 - Z1);
            Y4 = Y1 + (Y2 - Y1) * (elevation - Z1) / (Z2 - Z1);
            }
        else
            {
            X4 = X1 + (X2 - X1) * (Z1 - elevation) / (Z1 - Z2);
            Y4 = Y1 + (Y2 - Y1) * (Z1 - elevation) / (Z1 - Z2);
            }
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X3, Y3, Z3, X1, Y1, Z1, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        bcdtmTinVolume_prismoidalVolumeArea (elevation, X3, Y3, Z3, X2, Y2, Z2, X4, Y4, Z4, planeCut, planeFill, planeCutArea, planeFillArea);
        cutP = cutP + planeCut;
        fillP = fillP + planeFill;
        cutAreaP = cutAreaP + planeCutArea;
        fillAreaP = fillAreaP + planeFillArea;
        return(DTM_SUCCESS);
        }
    /*
    ** Job Completed
    */
    return(DTM_SUCCESS);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTinVolume_prismoidalVolume(double elevation,double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double *cutP, double *fillP )
{
 double height,Zmin,Zmax ;
 /*
** Initialise Variables
*/
 *cutP = *fillP = 0.0 ;
/*
** Get Max & Min z Values
*/
 Zmin = Zmax = Z1 ;
 if( Z2 < Zmin ) Zmin = Z2 ; if( Z2 > Zmax ) Zmax = Z2 ;
 if( Z3 < Zmin ) Zmin = Z3 ; if( Z3 > Zmax ) Zmax = Z3 ;
/*
** Calculate Volume
*/
 if( elevation <= Zmin )
   {
    height = (Z1+Z2+Z3)/3.0 - elevation ;
    *cutP = bcdtmMath_coordinateTriangleArea(X1,Y1,X2,Y2,X3,Y3) * height  ;
    return(DTM_SUCCESS) ;
   }
 if( elevation >= Zmax )
   {
    height = elevation - (Z1+Z2+Z3) / 3.0 ;
    *fillP = bcdtmMath_coordinateTriangleArea(X1,Y1,X2,Y2,X3,Y3)  * height ;
    return(DTM_SUCCESS) ;
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
BENTLEYDTM_Private int bcdtmTinVolume_calculateRangeVolumes(double cutVolume,double fillVolume,VOLRANGETAB *rangeTableP,long numRanges,double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double SZ1,double SZ2,double SZ3 )
/*
** This Function Calculates The Range Volumes
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   nr,Surface=0 ;  
 double T1=0.0,T2=0.0,T3=0.0,B1=0.0,B2=0.0,B3=0.0     ;
 double cut,fill,area,zPrismMin=0.0,zPrismMax=0.0,zBotMin=0.0,zBotMax=0.0,zTopMin=0.0,zTopMax=0.0,prismVolume=0.0,rangeVolume=0.0 ;
 double rangeLow,rangeHigh,Zh,Zs,trgArea=0.0,percentDiff ;
 double P1,P2,P3,S1,S2,S3 ;
 double X4=0.0,Y4=0.0,Z4=0.0,X5=0.0,Y5=0.0,Z5=0.0 ;
 double Zp,Zsmin,Zsmax,Zsm1,Zsm2 ;
 double Qvol,Pvol,Rqvol,Rpvol,Zsmn,Zsmm,Zs2l,Zs2h,Zs3l,Zs3h ; 
 double sumRangeVolume,sumRangeCut,sumRangeFill ;
/*
** Only Process If Ranges Set And Cut And Fill Volumes Present
*/
 if( numRanges > 0 && ( cutVolume > 0.0 || fillVolume > 0.0 ))
   {
/*
**  Get Max & Min Values For Surface
*/
    Zh = ( Z1 + Z2  + Z3  ) / 3.0 ;
    Zs = (SZ1 + SZ2 + SZ3 ) / 3.0 ; 
    trgArea = bcdtmMath_coordinateTriangleArea(X1,Y1,X2,Y2,X3,Y3) ;
    prismVolume = trgArea * (Zs-Zh) ;
    if( prismVolume < 0.0 ) prismVolume = -prismVolume ;
/*
**  Determine Surface Type
** 
**  Surface = 1 ** cutVolume  Only        -  No Vertical Overlap Of Triangles
**  Surface = 2 ** fillVolume Only        -  No Vertical Overlap Of Triangles
**  Surface = 3 ** cutVolume And fillVolume
**  Surface = 4 ** cutVolume or fillVolume Only - Vertical Overlap Of Triangles 
*/
    Surface = 3 ;                                                  //  cutVolume And fillVolume
    if( cutVolume != 0.0 && fillVolume == 0.0 ) Surface = 1 ;      //  cutVolume Only
    if( cutVolume == 0.0 && fillVolume != 0.0 ) Surface = 2 ;      //  fillVolume Only
/*
**  Set Max Min z Values For Surface Type 1 and 2
*/
    if( Surface == 1 ) { T1 = SZ1 ; T2 = SZ2 ; T3 = SZ3  ; B1 = Z1  ; B2 = Z2  ; B3 = Z3  ; }
    if( Surface == 2 ) { T1 = Z1  ; T2 = Z2  ; T3 = Z3   ; B1 = SZ1 ; B2 = SZ2 ; B3 = SZ3 ; }
    if( Surface <= 2 )
      {
       zTopMin = zTopMax = T1 ;
       if( zTopMin > T2 ) zTopMin = T2 ; 
       if( zTopMin > T3 ) zTopMin = T3 ; 
       if( zTopMax < T2 ) zTopMax = T2 ;
       if( zTopMax < T3 ) zTopMax = T3 ; 
       zBotMin = zBotMax = B1 ;
       if( zBotMin > B2 ) zBotMin = B2 ;
       if( zBotMin > B3 ) zBotMin = B3 ; 
       if( zBotMax < B2 ) zBotMax = B2 ;
       if( zBotMax < B3 ) zBotMax = B3 ; 
       zPrismMin = zBotMin ;
       zPrismMax = zTopMax ;
       if( zTopMin < zBotMax ) Surface = 4 ;     // cutVolume or fillVolume Only - Vertical Overlap Of Triangles        
      }
/*
**  Determine Range Volumes
*/                        
    sumRangeCut  = 0.0 ;
    sumRangeFill = 0.0 ;
    sumRangeVolume  = 0.0 ;
/*
**  Surface Type 3  ** cutVolume And fillVolume
*/
    if( Surface == 3 )
      {
/*
**     Write Triangle Points
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"X1 = %12.5lf Y1 = %12.5lf Z1 = %12.5lf SZ1 = %12.5lf",X1,Y1,Z1,SZ1) ;
          bcdtmWrite_message(0,0,0,"X2 = %12.5lf Y2 = %12.5lf Z2 = %12.5lf SZ2 = %12.5lf",X2,Y2,Z2,SZ2) ;
          bcdtmWrite_message(0,0,0,"X3 = %12.5lf Y3 = %12.5lf Z3 = %12.5lf SZ3 = %12.5lf",X3,Y3,Z3,SZ3) ;
         }  
/*
**     Set Surfaces
*/
       P1 = SZ1 ; P2 = SZ2 ; P3 = SZ3 ;
       S1 = Z1  ; S2 =  Z2 ; S3 = Z3  ;
       Zp = ( P1 + P2 + P3 ) / 3.0 ;
       Zs = ( S1 + S2 + S3 ) / 3.0 ;
       Zsmin = Zsmax = P1 ;
       if( P2 < Zsmin ) Zsmin = P2 ;
       if( P2 > Zsmax ) Zsmax = P2 ;
       if( P3 < Zsmin ) Zsmin = P3 ;
       if( P3 > Zsmax ) Zsmax = P3 ;
       if( S1 < Zsmin ) Zsmin = S1 ;
       if( S1 > Zsmax ) Zsmax = S1 ;
       if( S2 < Zsmin ) Zsmin = S2 ;
       if( S2 > Zsmax ) Zsmax = S2 ;
       if( S3 < Zsmin ) Zsmin = S3 ;
       if( S3 > Zsmax ) Zsmax = S3 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Zsmin = %12.5lf Zsmax = %12.5lf",Zsmin,Zsmax) ;
/*
**     Write Triangle Elevations 
*/
       if( dbg )
         {
          if( P1 >  S1 ) bcdtmWrite_message(0,0,0,"P1 >  S1") ;
          if( P1 == S1 ) bcdtmWrite_message(0,0,0,"P1 == S1") ;
          if( P1 <  S1 ) bcdtmWrite_message(0,0,0,"P1 <  S1") ;
          if( P2 >  S2 ) bcdtmWrite_message(0,0,0,"P2 >  S2") ;
          if( P2 == S2 ) bcdtmWrite_message(0,0,0,"P2 == S2") ;
          if( P2 <  S2 ) bcdtmWrite_message(0,0,0,"P2 <  S2") ;
          if( P3 >  S3 ) bcdtmWrite_message(0,0,0,"P3 >  S3") ;
          if( P3 == S3 ) bcdtmWrite_message(0,0,0,"P3 == S3") ;
          if( P3 <  S3 ) bcdtmWrite_message(0,0,0,"P3 <  S3") ;
         }
/*
**     Set Surface So That Point 1 is the only Point Above or Below The Other Surface
*/
       if     ( ( P2 > S2 && P1 <= S1 && P3 <= S3 )  || ( P2 < S2 && P1 >= S1 && P3 >= S3 )  )
         {
          X4 = X2 ; Y4 = Y2 ; Z4 = P2 ; Z5 = S2 ;
          X2 = X1 ; Y2 = Y1 ; P2 = P1 ; S2 = S1 ;
          X1 = X4 ; Y1 = Y4 ; P1 = Z4 ; S1 = Z5 ; 
         }
   
       else if( ( P3 > S3 && P1 <= S1 && P2 <= S2 )  || ( P3 < S3 && P1 >= S1 && P2 >= S2 )  )
         {
          X4 = X3 ; Y4 = Y3 ; Z4 = P3 ; Z5 = S3 ; 
          X3 = X1 ; Y3 = Y1 ; P3 = P1 ; S3 = S1 ; 
          X1 = X4 ; Y1 = Y4 ; P1 = Z4 ; S1 = Z5 ; 
         }
/*
**     Check P1 Is the Only Point Above Or Below The Other Surface
*/
       if(  ( P1 < S1 && P2 < S2 ) || ( P1 > S1 && P2 > S2 ) ) return(0) ;
       if(  ( P1 < S1 && P3 < S3 ) || ( P1 > S1 && P3 > S3 ) ) return(0) ;
/*
**     Write Adjusted Triangle Points
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"X1 = %12.5lf Y1 = %12.5lf S1 = %12.5lf P1 = %12.5lf",X1,Y1,S1,P1) ;
          bcdtmWrite_message(0,0,0,"X2 = %12.5lf Y2 = %12.5lf S2 = %12.5lf P2 = %12.5lf",X2,Y2,S2,P2) ;
          bcdtmWrite_message(0,0,0,"X3 = %12.5lf Y3 = %12.5lf S3 = %12.5lf P3 = %12.5lf",X3,Y3,S3,P3) ;
          if( P1 >  S1 ) bcdtmWrite_message(0,0,0,"P1 >  S1") ;
          if( P1 == S1 ) bcdtmWrite_message(0,0,0,"P1 == S1") ;
          if( P1 <  S1 ) bcdtmWrite_message(0,0,0,"P1 <  S1") ;
          if( P2 >  S2 ) bcdtmWrite_message(0,0,0,"P2 >  S2") ;
          if( P2 == S2 ) bcdtmWrite_message(0,0,0,"P2 == S2") ;
          if( P2 <  S2 ) bcdtmWrite_message(0,0,0,"P2 <  S2") ;
          if( P3 >  S3 ) bcdtmWrite_message(0,0,0,"P3 >  S3") ;
          if( P3 == S3 ) bcdtmWrite_message(0,0,0,"P3 == S3") ;
          if( P3 <  S3 ) bcdtmWrite_message(0,0,0,"P3 <  S3") ;
         }
/*
**     Calculate Elevation Of Surface Intersection Point Of 1-2 
*/
       if( P1 == P2 || S1 == S2 )
         {
          if( P1 == P2 ) Z4 = P2 ;
          if( S1 == S2 ) Z4 = S2 ;
         }
       else  bcdtmMath_normalIntersectCordLines(X1,P1,X2,P2,X1,S1,X2,S2,&X4,&Z4)  ;
/*
**     Calculate XY Coordinates Of Surface Intersection Point Of 1-2 
*/
       if( P1 != P2 && fabs(P1-P2) > fabs(S1-S2)  )
         {
          X4 = X1 +  ( X2 - X1 ) * fabs(Z4-P1) / fabs(P1-P2) ;
          Y4 = Y1 +  ( Y2 - Y1 ) * fabs(Z4-P1) / fabs(P1-P2) ;
         }
       else
         {
          if( S1 == S2 )  return(0) ;
          X4 = X1 +  ( X2 - X1 ) * fabs(Z4-S1) / fabs(S1-S2) ;
          Y4 = Y1 +  ( Y2 - Y1 ) * fabs(Z4-S1) / fabs(S1-S2) ;
         }
/*
**     Calculate Elevation Of Surface Intersection Point Of 1-3 
*/
       if( P1 == P3 || S1== S3 ) 
         {
          if( P1 == P3 ) Z5 = P3 ;
          if( S1 == S3 ) Z5 = S3 ;
         }
       else bcdtmMath_normalIntersectCordLines(X1,P1,X3,P3,X1,S1,X3,S3,&X5,&Z5)  ;
/*
**     Calculate XY Coordinates Of Surface Intersection Point Of 1-3 
*/
       if( P1 != P3 && fabs(P1-P3) > fabs(S1-S3) )
         {
          X5 = X1 +  ( X3 - X1 ) * fabs(Z5-P1) / fabs(P1-P3) ;
          Y5 = Y1 +  ( Y3 - Y1 ) * fabs(Z5-P1) / fabs(P1-P3) ;
         }
       else
         {
          if( S1 == S3 ) return(0) ;
          X5 = X1 +  ( X3 - X1 ) * fabs(Z5-S1) / fabs(S1-S3) ;
          Y5 = Y1 +  ( Y3 - Y1 ) * fabs(Z5-S1) / fabs(S1-S3) ;
         }
/*
**
*/
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"X4 = %12.5lf Y4 = %12.5lf Z4 = %12.5lf",X4,Y4,Z4) ;
          bcdtmWrite_message(0,0,0,"X5 = %12.5lf Y5 = %12.5lf Z5 = %12.5lf",X5,Y5,Z5) ;
         }
/*
**     Set Intermediate z Ranges
*/
       if( Z4 <= Z5 ) { Zsm1 = Z4 ; Zsm2 = Z5 ; }
       else           { Zsm1 = Z5 ; Zsm2 = Z4 ; }
/*
**     Determine Range Volumes
*/           
       if( P1 < S1 ) { Pvol = fillVolume ; Qvol = cutVolume  ; }
       else          { Pvol = cutVolume  ; Qvol = fillVolume ; }
       for( nr = 0 ; nr < numRanges ; ++nr )
         {
          rangeLow  = rangeTableP[nr].Low  ;
          rangeHigh = rangeTableP[nr].High  ;
          if( Zsmin < rangeHigh && Zsmax > rangeLow )
            {
             Rpvol = Rqvol = 0.0 ;             
             if( Zsmin >= rangeLow && Zsmax <= rangeHigh )
               {
                Rpvol = Pvol ; Rqvol = Qvol ;
               }
             else
               {
/*
**              Process Range Low For P1
*/
                if( P1 <  S1 ) { Zsmn = P1 ; Zsmm = S1 ; }
                else           { Zsmn = S1 ; Zsmm = P1 ; }  
                if( Zsmn >= rangeLow &&  Zsmm <= rangeHigh ) Rpvol = Pvol ;
                else
                  {
                   if( rangeLow < Zsmm && rangeHigh > Zsmn ) Rpvol = Pvol ;           
                   else                              Rpvol = 0.0  ;
                   if( rangeLow > Zsmn && rangeLow <  Zsmm )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,Zsmn,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rpvol = Rpvol - fill  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,Zsmm,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rpvol = Rpvol + fill ;
                     }
/*
**                  Process Range High For P1
*/
                   if( rangeHigh > Zsmn && rangeHigh < Zsmm )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,Zsmm,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rpvol = Rpvol - cut  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,Zsmn,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rpvol = Rpvol + cut ;
                     }
                  }
/*
**              Initialise Variables For P2-P3
*/
                if( P2 <= S2 ) { Zs2l = P2 ; Zs2h = S2 ; }
                else           { Zs2l = S2 ; Zs2h = P2 ; } 
                if( P3 <= S3 ) { Zs3l = P3 ; Zs3h = S3 ; }
                else           { Zs3l = S3 ; Zs3h = P3 ; } 
                if( Zs2l <= Zs3l ) Zsmn = Zs2l ; 
                else               Zsmn = Zs3l ;  
                if( Zs2h >= Zs3h ) Zsmm = Zs2h ;
                else               Zsmm = Zs3h ;
                if( Zsmn >= rangeLow && Zsmm <= rangeHigh ) 
                  { 
                   Rqvol = Qvol ;
                  }
                else
                  {
/*
**                 Process Range Low For P2-P3
*/
                   if( rangeLow < Zsmm && rangeHigh > Zsmn ) Rqvol = Qvol ; 
                   else                                      Rqvol = 0.0 ;          
                   if( rangeLow > Zsmn && rangeLow < Zsmm )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X2,Y2,Zs2l,X3,Y3,Zs3l,X4,Y4,Z4,&cut,&fill,&area) ;
                      Rqvol = Rqvol - fill  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X3,Y3,Zs3l,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rqvol = Rqvol - fill  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X2,Y2,Zs2h,X3,Y3,Zs3h,X4,Y4,Z4,&cut,&fill,&area) ;
                      Rqvol = Rqvol + fill ;
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X3,Y3,Zs3h,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rqvol = Rqvol + fill ;
                     }
/*
**                 Process Range High For P2-P3
*/
                   if( rangeHigh > Zsmn && rangeHigh < Zsmm )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X2,Y2,Zs2h,X3,Y3,Zs3h,X4,Y4,Z4,&cut,&fill,&area) ;
                      Rqvol = Rqvol - cut  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X3,Y3,Zs3h,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rqvol = Rqvol - cut  ;
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X2,Y2,Zs2l,X3,Y3,Zs3l,X4,Y4,Z4,&cut,&fill,&area) ;
                      Rqvol = Rqvol + cut ;
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X3,Y3,Zs3l,X4,Y4,Z4,X5,Y5,Z5,&cut,&fill,&area) ;
                      Rqvol = Rqvol + cut ;
                     }
                  }
               } 
/*
**           Adjust For Rounding Errors
*/
             if( Rpvol < 0.0 ) Rpvol = 0.0 ;
             if( Rqvol < 0.0 ) Rqvol = 0.0 ; 
/*
**           Accumulate Range Volumes
*/
             if( P1 >  S1 )       /*  cutVolume On P1 */
               { 
                rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + Rpvol ;
                sumRangeCut = sumRangeCut + Rpvol ;
               }
             else                  /* fillVolume On P1 */  
               { 
                rangeTableP[nr].Fill = rangeTableP[nr].Fill + Rpvol ;
                sumRangeFill = sumRangeFill + Rpvol ;
               }
             if( P2 >= S2 && P3 >= S3 )    /*  cutVolume On Q */
               { 
                rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + Rqvol ;
                sumRangeCut = sumRangeCut + Rqvol ;
               }
             else                  /* fillVolume On Q */  
               { 
                rangeTableP[nr].Fill = rangeTableP[nr].Fill + Rqvol ;
                sumRangeFill = sumRangeFill + Rqvol ;
               }
            }
         }
/*
**     Check Range Cut Volumes For Surface Type 3
*/
       if( cdbg && cutVolume > 0.000001 )
         {
          if( sumRangeCut != cutVolume )
            {
             percentDiff = fabs(cutVolume-sumRangeCut)/cutVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 3 ** cutVolume  = %20.15lf rangeCut  = %20.15lf Difference = %20.15lf%%",cutVolume,sumRangeCut,percentDiff) ;
                goto errexit ;
               }  
            } 
         }
/*
**     Check Range Fill Volumes For Surface Type 3
*/
       if( cdbg && fillVolume > 0.000001 )
         {
          if( sumRangeFill != fillVolume )
            {
             percentDiff = fabs(fillVolume-sumRangeFill)/fillVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 3 ** fillVolume = %20.15lf rangeFill = %20.15lf Difference = %20.15lf%%",fillVolume,sumRangeFill,percentDiff) ;
                goto errexit ;
               }  
            } 
         }
      }
/*
**  Surface Types 1 And 2
*/
    if( Surface == 1 || Surface == 2 )
      {
       sumRangeVolume = 0.0 ;
       for( nr = 0 ; nr < numRanges ; ++nr )
         {
          rangeVolume = 0.0 ;
          rangeLow  = rangeTableP[nr].Low   ;
          rangeHigh = rangeTableP[nr].High  ;
/*
**        Volume Calulation Elevations Must Fall Within Volume Range Elevations
*/
          if( zPrismMin  < rangeHigh && zPrismMax > rangeLow )
            {
             if( zBotMin >= rangeLow && zTopMax <= rangeHigh ) rangeVolume = prismVolume ;
             else
               {
                rangeVolume = (rangeHigh-rangeLow) * trgArea ;
                if(  rangeLow >= zTopMin  )
                  {
                   bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,T1,X2,Y2,T2,X3,Y3,T3,&cut,&fill,&area) ;
                   rangeVolume = rangeVolume + fill  ;
                   bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,T1,X2,Y2,T2,X3,Y3,T3,&cut,&fill,&area) ;
                   rangeVolume = rangeVolume - fill ;  
                  }
                else if( rangeHigh <= zBotMax )
                  {
                   bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,B1,X2,Y2,B2,X3,Y3,B3,&cut,&fill,&area) ;
                   rangeVolume = rangeVolume + cut ;
                   bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,B1,X2,Y2,B2,X3,Y3,B3,&cut,&fill,&area) ;
                   rangeVolume = rangeVolume - cut ;
                  }
                else
                  {
/*
**                 cutVolume Out Volumes Above High Range
*/
                   if( rangeHigh >= zTopMin )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,T1,X2,Y2,T2,X3,Y3,T3,&cut,&fill,&area) ;
                      rangeVolume = rangeVolume - fill ;
                     }   
/*
**                 cutVolume Out Volumes Below Low Range
*/
                   if( rangeLow  <= zBotMax  )
                     {
                      bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,B1,X2,Y2,B2,X3,Y3,B3,&cut,&fill,&area) ;
                      rangeVolume = rangeVolume - cut ;
                     }
                  }
               }
/*
**           Accumulate Volumes
*/
             if( rangeVolume < 0.0 ) rangeVolume = 0.0 ; 
             if( Surface == 1) rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + rangeVolume ;
             if( Surface == 2) rangeTableP[nr].Fill = rangeTableP[nr].Fill + rangeVolume ;    
             sumRangeVolume = sumRangeVolume + rangeVolume ;
            }
         }
/*
**     Check Range Volumes For Surface Type 1
*/
       if( cdbg && cutVolume > 0.000001 )
         {
          if( sumRangeVolume != cutVolume )
            {
             percentDiff = fabs(cutVolume-sumRangeVolume)/cutVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 1 ** cutVolume = %20.15lf rangeVolume = %20.15lf Difference = %20.15lf%%",cutVolume,sumRangeVolume,percentDiff) ;
                goto errexit ;
               }  
            } 
         }
/*
**     Check Range Volumes For Surface Type 2
*/
       if( cdbg && fillVolume > 0.000001 )
         {
          if( sumRangeVolume != fillVolume )
            {
             percentDiff = fabs(fillVolume-sumRangeVolume)/fillVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 2 ** fillVolume = %20.15lf rangeVolume = %20.15lf Difference = %20.15lf%%",fillVolume,sumRangeVolume,percentDiff) ;
                goto errexit ;
               }  
            } 
         }
      }
/*
**  Surface Type 4
*/
    if( Surface == 4 )
      {
       sumRangeVolume = 0.0 ;
       for( nr = 0 ; nr < numRanges ; ++nr )
         {
          rangeVolume = 0.0 ;
          rangeLow  = rangeTableP[nr].Low   ;
          rangeHigh = rangeTableP[nr].High  ;
/*
**        Volume Calulation Elevations Must Fall Within Volume Range Elevations
*/
          if( zPrismMin < rangeHigh && zPrismMax > rangeLow )
            {
             if( zBotMin >= rangeLow && zTopMax <= rangeHigh ) rangeVolume = prismVolume ;
             else
               {
                rangeVolume = (rangeHigh-rangeLow) * trgArea ;
                bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,B1,X2,Y2,B2,X3,Y3,B3,&cut,&fill,&area) ;
                rangeVolume = rangeVolume - cut ;
                bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,B1,X2,Y2,B2,X3,Y3,B3,&cut,&fill,&area) ;
                rangeVolume = rangeVolume + cut ;
                bcdtmTinVolume_prismToFlatPlane(rangeHigh,X1,Y1,T1,X2,Y2,T2,X3,Y3,T3,&cut,&fill,&area) ;
                rangeVolume = rangeVolume - fill ;
                bcdtmTinVolume_prismToFlatPlane(rangeLow,X1,Y1,T1,X2,Y2,T2,X3,Y3,T3,&cut,&fill,&area) ;
                rangeVolume = rangeVolume + fill ;
               }
/*
**           Accumulate Volumes
*/
             if( rangeVolume < 0.0 ) rangeVolume = 0.0 ; 
             if( cutVolume  != 0.0 ) rangeTableP[nr].Cut  = rangeTableP[nr].Cut  + rangeVolume ;
             if( fillVolume != 0.0 ) rangeTableP[nr].Fill = rangeTableP[nr].Fill + rangeVolume ;    
             sumRangeVolume = sumRangeVolume + rangeVolume ;
            }
         }
/*
**     Check Range Volumes For Surface Type Four
*/
       if( cdbg )
         {
          if( cutVolume > 0.000001 && sumRangeVolume != cutVolume )
            {
             percentDiff = fabs(cutVolume-sumRangeVolume)/cutVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 4 ** cutVolume = %20.15lf rangeVolume = %20.15lf Difference = %20.15lf%%",cutVolume,sumRangeVolume,percentDiff) ;
                goto errexit ;
               }  
            } 
          if( fillVolume > 0.000001 && sumRangeVolume != fillVolume )
            {
             percentDiff = fabs(fillVolume-sumRangeVolume)/fillVolume * 100.0 ;
             if( percentDiff > 0.000001 )
               {
                bcdtmWrite_message(0,0,0,"Surface Type 4 ** fillVolume = %20.15lf rangeVolume = %20.15lf Difference = %20.15lf%%",fillVolume,sumRangeVolume,percentDiff) ;
                goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToSurfaceDtmFiles
(
 WCharCP fromDtmFileP,       /* ==> Pointer To From Dtm File                     */
 WCharCP toDtmFileP,         /* ==> Pointer To To Dtm File                       */
 VOLRANGETAB *rangeTableP,        /* ==> Pointer To Volume Range Table                */
 long        numRanges,           /* ==> Number Of Volume Ranges                      */
 DPoint3d         *polygonPtsP,        /* ==> Pointer To Volume Polygon Points             */
 long        numPolygonPts,       /* ==> Number Of Volume Polygon Points              */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
 void        *userP,              /* ==> User Pointer Passed Back To User             */
 double      *cutP,               /* <== Volume Cut Between Dtm Files                 */
 double      *fillP,              /* <== Volume Fill Between Dtm Files                */
 double      *balanceP,           /* <== Volume Balance Between Dtm Files             */
 double      *areaP               /* <== Area Of Volume Calculation                   */
)
/*
** This Function Calculates The Surface To Surface Volumes Between Two Dtm Files
*/
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *fromDtmP=NULL,*toDtmP=NULL ;
 /*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Surface To Surface Dtm Files") ;
    bcdtmWrite_message(0,0,0,"fromDtmFileP   = %ws",fromDtmFileP) ;
    bcdtmWrite_message(0,0,0,"toDtmFileP     = %ws",toDtmFileP) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
   }
/*
** Initialise
*/
 *cutP = *fillP = *balanceP = *areaP = 0.0 ;
/*
** Check For Different Files
*/
 if( wcscmp(fromDtmFileP,toDtmFileP) == 0 )
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ;
    goto errexit ;
   } 
/*
** Read From Dtm File
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&fromDtmP,fromDtmFileP)) goto errexit ;
/*
** Read To Dtm File
*/
 if( bcdtmRead_fromFileDtmObject(&toDtmP,toDtmFileP)) goto errexit ;
/*
** Calculate Surface To Surface Dtm Objects
*/
 if( bcdtmTinVolume_surfaceToSurfaceDtmObjects(fromDtmP,toDtmP,rangeTableP,numRanges,polygonPtsP,numPolygonPts,loadFunctionP,userP,*cutP,*fillP,*balanceP,*areaP)) goto errexit ;
/*
** Delete To Dtm Objects
*/
 if( toDtmP != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Dtm Files Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Surface To Surface Dtm Files Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToSurfaceDtmObjects
(
BC_DTM_OBJ  *fromDtmP,           /* ==> Pointer To From Dtm Object                   */
BC_DTM_OBJ  *toDtmP,             /* ==> Pointer To To Dtm Object                     */
VOLRANGETAB *rangeTableP,        /* ==> Pointer To Volume Range Table                */
long        numRanges,           /* ==> Number Of Volume Ranges                      */
DPoint3d    *polygonPtsP,        /* ==> Pointer To Volume Polygon Points             */
long        numPolygonPts,       /* ==> Number Of Volume Polygon Points              */
DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
void        *userP,              /* ==> User Pointer Pass Back To User               */
double      &cutP,               /* <== Volume Cut Between Dtm Objects               */
double      &fillP,              /* <== Volume Fill Between Dtm Objects              */
double      &balanceP,           /* <== Volume Balance Between Dtm Objects           */
double      &areaP               /* <== Area Of Volume Calculation                   */
)
    {
    double cutArea = 0, fillArea = 0;
    int status = bcdtmTinVolume_surfaceToSurfaceDtmObjects (fromDtmP, toDtmP, rangeTableP, numRanges, polygonPtsP, numPolygonPts, loadFunctionP, userP, cutP, fillP, balanceP, cutArea, fillArea);
    areaP = cutArea + fillArea;
    return status;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmTinVolume_surfaceToSurfaceDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,           /* ==> Pointer To From Dtm Object                   */
 BC_DTM_OBJ  *toDtmP,             /* ==> Pointer To To Dtm Object                     */
 VOLRANGETAB *rangeTableP,        /* ==> Pointer To Volume Range Table                */
 long        numRanges,           /* ==> Number Of Volume Ranges                      */
 DPoint3d    *polygonPtsP,        /* ==> Pointer To Volume Polygon Points             */
 long        numPolygonPts,       /* ==> Number Of Volume Polygon Points              */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
 void        *userP,              /* ==> User Pointer Pass Back To User               */
 double      &cutP,               /* <== Volume Cut Between Dtm Objects               */
 double      &fillP,              /* <== Volume Fill Between Dtm Objects              */
 double      &balanceP,           /* <== Volume Balance Between Dtm Objects           */
 double      &cutAreaP,           /* <== Area Of Cut Volume Calculation                   */
 double      &fillAreaP           /* <== Area Of Cut Volume Calculation                   */
 )
/*
** This Function Calculates The Surface To Surface Volumes Between Two Dtm Objects
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ; 
 long            intersectFlag,saveTol=FALSE ;
 double          sppTol = 0.0,splTol = 0.0;
 DPoint3d        *p3dP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
 VOLRANGETAB     *vrtP ;
 long            startTime ;
 BC_DTM_OBJ      *cloneFromDtmP=NULL,*saveFromDtmP=NULL ;
/*
** Write Entry Message
*/
 startTime = bcdtmClock() ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Surface To Surface Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"fromDtmP       = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP         = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"rangeTableP    = %p",rangeTableP) ;
    bcdtmWrite_message(0,0,0,"numRanges      = %8ld",numRanges) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP    = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts  = %8ld",numPolygonPts) ;
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
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit  ;
/*
** Check If Both DTM Objects Are In TIN_STATE
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
    return(20) ;
   }
/*
** Check And Write Dtm Variables
*/
 if( cdbg )
   { 
    bcdtmWrite_message(0,0,0,"From DTM      = %p",fromDtmP ) ;
    bcdtmWrite_message(0,0,0,"ppTol         = %15.12lf",fromDtmP->ppTol ) ;
    bcdtmWrite_message(0,0,0,"plTol         = %15.12lf",fromDtmP->plTol ) ;
    bcdtmWrite_message(0,0,0,"mppTol        = %15.12lf",fromDtmP->mppTol ) ;
    bcdtmWrite_message(0,0,0,"numPoints     = %10ld",fromDtmP->numPoints ) ;
    bcdtmWrite_message(0,0,0,"numSortedPts  = %10ld",fromDtmP->numSortedPoints ) ;
    bcdtmWrite_message(0,0,0,"memPoints     = %10ld",fromDtmP->memPoints ) ;
    bcdtmWrite_message(0,0,0,"numTriangles  = %10ld",fromDtmP->numTriangles ) ;
    bcdtmWrite_message(0,0,0,"numFeatures   = %10ld",fromDtmP->numFeatures ) ;
    bcdtmWrite_message(0,0,0,"Checking fromDtm Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(fromDtmP))
      {
       bcdtmWrite_message(0,0,0,"fromDtm Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"To DTM        = %p",toDtmP ) ;
    bcdtmWrite_message(0,0,0,"ppTol         = %15.12lf",toDtmP->ppTol ) ;
    bcdtmWrite_message(0,0,0,"plTol         = %15.12lf",toDtmP->plTol ) ;
    bcdtmWrite_message(0,0,0,"mppTol        = %15.12lf",toDtmP->mppTol ) ;
    bcdtmWrite_message(0,0,0,"numPoints     = %10ld",toDtmP->numPoints ) ;
    bcdtmWrite_message(0,0,0,"numSortedPts  = %10ld",toDtmP->numSortedPoints ) ;
    bcdtmWrite_message(0,0,0,"memPoints     = %10ld",toDtmP->memPoints ) ;
    bcdtmWrite_message(0,0,0,"numTriangles  = %10ld",toDtmP->numTriangles ) ;
    bcdtmWrite_message(0,0,0,"numFeatures   = %10ld",toDtmP->numFeatures ) ;
    bcdtmWrite_message(0,0,0,"Checking toDtm Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(toDtmP))
      {
       bcdtmWrite_message(0,0,0,"toDtm Triangulation Invalid") ;
       goto errexit ;
      }
   }
/*
** Clone From DTM If It Is An Element
*/
 if( fromDtmP->dtmObjType == BC_DTM_ELM_TYPE )
   {
    if( bcdtmObject_cloneDtmObject(fromDtmP,&cloneFromDtmP)) goto errexit ;
    saveFromDtmP = fromDtmP ;
    fromDtmP = cloneFromDtmP ;
   }
/*
** Save Dtm Tolerances
*/
 sppTol  = fromDtmP->ppTol ;
 splTol  = fromDtmP->plTol ;
 saveTol = TRUE ;
/*
** Set Precision Tolerances For Volume Calculations
*/
// fromDtmP->ppTol = fromDtmP->plTol = fromDtmP->mppTol * 100.0 ;  
// toDtmP->ppTol   = toDtmP->plTol   = fromDtmP->mppTol * 100.0 ;  
/*
** Get Intersect Polygons Between Dtm Hulls And Volume Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Volume Polygon And Dtm Hulls") ;
 if( bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects(fromDtmP,toDtmP,polygonPtsP,numPolygonPts,&polyP,&intersectFlag) ) goto errexit ;
/*
** Set Precision Tolerances For Volume Calculations
*/
 fromDtmP->ppTol = fromDtmP->plTol = fromDtmP->mppTol * 10000.0 ;  
// toDtmP->ppTol   = toDtmP->plTol   = fromDtmP->mppTol * 100.0 ;  
/*
**  Calculate Surface To Surface Volumes
*/
 numVolTriangles = 0 ;
 totTrgArea      = 0.0 ; 
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Surface To Surface Dtm Volumes") ;
 if( bcdtmTinVolume_calculateVolumeSurfaceToSurfaceDtmObjects(fromDtmP,toDtmP,polyP,rangeTableP,numRanges,loadFunctionP,userP,cutP,fillP,balanceP,cutAreaP, fillAreaP) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numVolTriangles = %9ld Cut = %10.4lf Fill = %10.4lf Balance = %10.4lf CutArea = %10.4lf FillArea = %10.4lf",numVolTriangles,cutP,fillP,balanceP,cutAreaP, fillAreaP) ;
/*
** Clean Up
*/
 cleanup :
 if( saveTol == TRUE )
   {
    fromDtmP->ppTol = sppTol ;
    fromDtmP->plTol = splTol ;
   }
 if( saveFromDtmP  != NULL ) { fromDtmP = saveFromDtmP ; saveFromDtmP = NULL ; }
 if( cloneFromDtmP != NULL ) bcdtmObject_destroyDtmObject(&cloneFromDtmP) ;
 if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ; 
/*
** Return
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Calculate Volume = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
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
BENTLEYDTM_Public int bcdtmTinVolume_calculateSurfaceToSurfaceVolumeForTriangleDtmObject(BC_DTM_OBJ *dtmP,long Ndp,VOLRANGETAB *rangeTableP,long numRanges,long Spnt,DPoint3d *TrgPts,double *cutP,double *fillP,double *areaP)
/*
** Calculates The Volumes Of All to Dtm Triangles Internal To a from Dtm Triangle
*/
{
 int      ret=DTM_SUCCESS;
 long     p1,p2=0,p3,sp,np,lp,hp,clc,process,VoidFlag,NoMarked=0 ;
 double   area=0.0,cut=0.0,fill=0.0 ;
 double   X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3 ;
 double   SZ1,SZ2,SZ3 ;
 double   Xmin=0.0,Ymin=0.0,Zmin=0.0  ;
 VOLRANGETAB *nr ;
/*
** Initialise variables
*/
 *cutP = *fillP = *areaP = 0.0 ;
/*
** Normalise Triangle
*/
 Xmin = TrgPts->x ; Ymin = TrgPts->y ; Zmin = TrgPts->z ;
 if( (TrgPts+1)->x < Xmin ) Xmin = (TrgPts+1)->x ;
 if( (TrgPts+2)->x < Xmin ) Xmin = (TrgPts+2)->x ;
 if( (TrgPts+1)->y < Ymin ) Ymin = (TrgPts+1)->y ;
 if( (TrgPts+2)->y < Ymin ) Ymin = (TrgPts+2)->y ;
 if( (TrgPts+1)->z < Zmin ) Zmin = (TrgPts+1)->z ;
 if( (TrgPts+2)->z < Zmin ) Zmin = (TrgPts+2)->z ;
 TrgPts->x = TrgPts->x - Xmin ; (TrgPts+1)->x = (TrgPts+1)->x - Xmin ; (TrgPts+2)->x = (TrgPts+2)->x - Xmin ;
 TrgPts->y = TrgPts->y - Ymin ; (TrgPts+1)->y = (TrgPts+1)->y - Ymin ; (TrgPts+2)->y = (TrgPts+2)->y - Ymin ;
 TrgPts->z = TrgPts->z - Zmin ; (TrgPts+1)->z = (TrgPts+1)->z - Zmin ; (TrgPts+2)->z = (TrgPts+2)->z - Zmin ;
/*
** Calculate Plane Coefficients
*/
 bcdtmTinVolume_interpolateZOnPlaneOfTriangle(1,TrgPts,0.0,0.0,&SZ1) ;
/*
** Mark All Points Internal To Inserted Boundary
*/
 lp = Ndp ; hp = 0 ; sp = Spnt ;
 do
   {
    if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->tPtr)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,np)->tPtr != sp )
      {
       if( np < lp ) lp = np ;
       if( np > hp && np < Ndp ) hp = np ;
       if(nodeAddrP(dtmP,np)->tPtr == dtmP->nullPnt ) { nodeAddrP(dtmP,np)->tPtr = -dtmP->nullPnt ; ++NoMarked ; }
       if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
      }
    sp = nodeAddrP(dtmP,sp)->tPtr ;
    if( sp < lp ) lp = sp ;
    if( sp > hp && sp < Ndp ) hp = sp ;
   } while ( sp != Spnt ) ;
/*
** Mark All Points Connected To Internal Points
*/
 process = 1 ;
 while ( process && NoMarked > 0 )
   {
    process = 0 ;
    for ( sp = lp ; sp <= hp ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == -dtmP->nullPnt )
         {
          clc = nodeAddrP(dtmP,sp)->cPtr ;
          while ( clc != dtmP->nullPtr )
            {
             np  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( nodeAddrP(dtmP,np)->tPtr == dtmP->nullPnt )
               {
                nodeAddrP(dtmP,np)->tPtr = -dtmP->nullPnt ;
                process = 1 ;
                ++NoMarked ;
               }
            }
         }
      }
   }
/*
** Mark Insert Boundary
*/
 sp = Spnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = - (nodeAddrP(dtmP,sp)->tPtr + 1 ) ;
    sp = np ;
   } while ( sp != Spnt ) ; 
/*
** Calculate Volumes For All Triangles
*/
 if( lp > hp ) { lp = Ndp ; hp = dtmP->numPoints - 1 ; }
 for ( p1 = lp ; p1 <= hp ; ++p1 )
   {
    if( nodeAddrP(dtmP,p1)->tPtr < 0 )
      {
       if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ) clc = dtmP->nullPtr ; 
       else
         {
          clc = nodeAddrP(dtmP,p1)->cPtr ;
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
         } 
       while ( clc != dtmP->nullPtr )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,p2)->tPtr < 0 && nodeAddrP(dtmP,p3)->tPtr < 0 )
            {
             if( nodeAddrP(dtmP,p1)->hPtr != p2 )
               {
                VoidFlag = 0 ;
                bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&VoidFlag) ;
                if( ! VoidFlag )
                  {
                   ++numVolTriangles ;
                   X1 = pointAddrP(dtmP,p1)->x - Xmin ; Y1 = pointAddrP(dtmP,p1)->y - Ymin ; Z1 = pointAddrP(dtmP,p1)->z - Zmin ;
                   X2 = pointAddrP(dtmP,p2)->x - Xmin ; Y2 = pointAddrP(dtmP,p2)->y - Ymin ; Z2 = pointAddrP(dtmP,p2)->z - Zmin ;
                   X3 = pointAddrP(dtmP,p3)->x - Xmin ; Y3 = pointAddrP(dtmP,p3)->y - Ymin ; Z3 = pointAddrP(dtmP,p3)->z - Zmin ;
                   bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X1,Y1,&SZ1) ;
                   bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X2,Y2,&SZ2) ;
                   bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X3,Y3,&SZ3) ;
                   bcdtmTinVolume_prismToFlatPlane(0.0,X1,Y1,(Z1-SZ1),X2,Y2,(Z2-SZ2),X3,Y3,(Z3-SZ3),&cut,&fill,&area) ;
                   *cutP  = *cutP  + cut ;  *fillP = *fillP + fill ; *areaP = *areaP + area ;
                   if( numRanges > 0 ) 
                     {
                      for( nr = rangeTableP ; nr < rangeTableP + numRanges ; ++nr ) { nr->Low -= Zmin ; nr->High -= Zmin ; }
                      if( bcdtmTinVolume_calculateRangeVolumes(cut,fill,rangeTableP,numRanges,X1,Y1,SZ1,X2,Y2,SZ2,X3,Y3,SZ3,Z1,Z2,Z3) ) goto errexit ;
                      for( nr = rangeTableP ; nr < rangeTableP + numRanges ; ++nr ) { nr->Low += Zmin ; nr->High += Zmin ; }
                     }
                  }
               } 
            }
          p2 = p3 ;
         }
       if      ( nodeAddrP(dtmP,p1)->tPtr == -dtmP->nullPnt ) nodeAddrP(dtmP,p1)->tPtr = dtmP->nullPnt ;
       else if ( nodeAddrP(dtmP,p1)->tPtr <  0            ) nodeAddrP(dtmP,p1)->tPtr = -nodeAddrP(dtmP,p1)->tPtr - 1 ;
      }
    if( p1 == hp && p1 != dtmP->numPoints - 1 ) { p1 = Ndp - 1 ; hp = dtmP->numPoints - 1 ; }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_calculateVolumeBalance(double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double SZ1,double SZ2,double SZ3,double *balanceP)
/*
** Calculate Volume balance For Triangle
*/
{
 double Zh,Zs,Zmin,Area ;
 Zmin = Z1 ;
 if( Zmin < Z2  ) Zmin = Z2  ;
 if( Zmin < Z3  ) Zmin = Z3  ;
 if( Zmin < SZ1 ) Zmin = SZ1 ;
 if( Zmin < SZ2 ) Zmin = SZ2 ;
 if( Zmin < SZ3 ) Zmin = SZ3 ;
 Zh = ( Z1 + Z2  + Z3  ) / 3.0 ;
 Zs = (SZ1 + SZ2 + SZ3 ) / 3.0 ; 
 Area = bcdtmMath_coordinateTriangleArea(X1,Y1,X2,Y2,X3,Y3) ;
 if( Area < 0.0 ) Area = -Area ;
 *balanceP = (Zh-Zmin) * Area - (Zs-Zmin) * Area ;
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
BENTLEYDTM_Public int bcdtmTinVolume_calculateSurfaceToSurfaceVolumeForTriangleTinObjectNew(BC_DTM_OBJ *dtmP,VOLRANGETAB *rangeTableP,long numRanges,long Spnt,DPoint3d *TrgPts,double *cutP,double *fillP,double *areaP)
/*
** Calculates the Volumes of all secondary Dtm Triangles internal
** to a primary Dtm Triangle
*/
{
 int      ret=DTM_SUCCESS ;
 long     sp,np,NumMarked=0,NumUnMarked=0 ;
 double   Zs,Xmin=0.0,Ymin=0.0,Zmin=0.0  ;
/*
** Initialise variables
*/
 *cutP = *fillP = *areaP = 0.0 ;
/*
** Normalise Triangle
*/
 Xmin = TrgPts->x ; Ymin = TrgPts->y ; Zmin = TrgPts->z ;
 if( (TrgPts+1)->x < Xmin ) Xmin = (TrgPts+1)->x ;
 if( (TrgPts+2)->x < Xmin ) Xmin = (TrgPts+2)->x ;
 if( (TrgPts+1)->y < Ymin ) Ymin = (TrgPts+1)->y ;
 if( (TrgPts+2)->y < Ymin ) Ymin = (TrgPts+2)->y ;
 if( (TrgPts+1)->z < Zmin ) Zmin = (TrgPts+1)->z ;
 if( (TrgPts+2)->z < Zmin ) Zmin = (TrgPts+2)->z ;
 TrgPts->x = TrgPts->x - Xmin ; (TrgPts+1)->x = (TrgPts+1)->x - Xmin ; (TrgPts+2)->x = (TrgPts+2)->x - Xmin ;
 TrgPts->y = TrgPts->y - Ymin ; (TrgPts+1)->y = (TrgPts+1)->y - Ymin ; (TrgPts+2)->y = (TrgPts+2)->y - Ymin ;
 TrgPts->z = TrgPts->z - Zmin ; (TrgPts+1)->z = (TrgPts+1)->z - Zmin ; (TrgPts+2)->z = (TrgPts+2)->z - Zmin ;
/*
** Calculate Plane Coefficients
*/
 bcdtmTinVolume_interpolateZOnPlaneOfTriangle(1,TrgPts,0.0,0.0,&Zs) ;
/*
**  Mark Internal Tptr Polygon Points
*/
 if( bcdtmTinVolume_markInternalTptrPolygonPointsDtmObject(dtmP,Spnt,-dtmP->nullPnt,&NumMarked) ) goto errexit ;
/*
**  Calculate Volumes For Internal Tptr Polygon Points
*/
 if( NumMarked ) if( bcdtmTinVolume_calculateVolumesForMarkedInternalTptrPolygonPointsDtmObject(dtmP,Spnt,-dtmP->nullPnt,&NumUnMarked,rangeTableP,numRanges,Xmin,Ymin,Zmin,cutP,fillP,areaP)) goto errexit ;
/*
** Check For Marking Errors
*/
 if( NumMarked != NumUnMarked ) 
   {
    bcdtmWrite_message(0,0,0,"Number Marked = %6ld Number Un Marked = %6ld",NumMarked,NumUnMarked) ; ; 
    bcdtmWrite_message(1,0,0,"Prismoidal Mark Error ") ; goto errexit ; 
   }
/*
** Mark Tptr Polygon
*/
 sp = Spnt ;
 do {  np = nodeAddrP(dtmP,sp)->tPtr ;  nodeAddrP(dtmP,sp)->tPtr = - (nodeAddrP(dtmP,sp)->tPtr + 1 ) ; sp = np ; } while ( sp != Spnt ) ; 
/*
**  Calculate Volumes For Tptr Polygon Points
*/
 sp = Spnt ;
 do 
   {
    np = -nodeAddrP(dtmP,sp)->tPtr - 1 ;  
    if( bcdtmTinVolume_calculateVolumesForPointDtmObject(dtmP,sp,rangeTableP,numRanges,Xmin,Ymin,Zmin,cutP,fillP,areaP)) goto errexit ;
    nodeAddrP(dtmP,sp)->tPtr = np ; 
    sp = np ; 
   } while ( sp != Spnt ) ; 
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_markInternalTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long Spnt,long Mark,long *NumMarked ) 
/*
** This Function Marks All Points Internal To a tPtr Polygon
*/
{
 int ret=DTM_SUCCESS ;
 long pp,sp,np,ap,lp,clc,Fpnt,Lpnt ;
/*
** Initialise
*/
 Fpnt = Lpnt = dtmP->nullPnt ;
 *NumMarked = 0 ;
/*
** Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List
*/
 pp = Spnt ;
 sp = nodeAddrP(dtmP,Spnt)->tPtr ; 
 do
   {
    ap = np = nodeAddrP(dtmP,sp)->tPtr ;
    if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
    while ( ap != pp )
      {
       if( nodeAddrP(dtmP,ap)->tPtr == dtmP->nullPnt )
         {
          nodeAddrP(dtmP,ap)->tPtr = Mark ;
          ++(*NumMarked) ;
          clc = nodeAddrP(dtmP,ap)->cPtr ;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )
               {
                if( Lpnt == dtmP->nullPnt ) { Fpnt = Lpnt = lp ;  }
                else                      { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; }
                nodeAddrP(dtmP,lp)->tPtr = lp ;
               }
            }
         } 
       if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
      }
    pp = sp ;  
    sp = np ; 
   } while ( pp != Spnt ) ;
/*
** Scan Tptr List And Mark Points
*/
 while ( Fpnt != Lpnt )
   {
    np = nodeAddrP(dtmP,Fpnt)->tPtr ;
    nodeAddrP(dtmP,Fpnt)->tPtr = Mark ;
    ++(*NumMarked) ;
    clc = nodeAddrP(dtmP,Fpnt)->cPtr ;
    while( clc != dtmP->nullPtr )
      {
       lp  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if(nodeAddrP(dtmP,lp)->tPtr == dtmP->nullPnt )  
         { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; nodeAddrP(dtmP,lp)->tPtr = lp ; }
      }
    Fpnt = np ;
   }
/*
** Mark Last Point
*/
 if( Lpnt != dtmP->nullPnt )
   {
    nodeAddrP(dtmP,Lpnt)->tPtr = Mark ;
    ++(*NumMarked) ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_unmarkInternalTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long Spnt,long Mark,long *NumUnMarked ) 
/*
** This Function Marks All Points Internal To a tPtr Polygon
*/
{
 int  ret=DTM_SUCCESS ;
 long pp,sp,np,ap,lp,clc,Fpnt,Lpnt ;
/*
** Initialise
*/
 Fpnt = Lpnt = dtmP->nullPnt ;
 *NumUnMarked = 0 ;
/*
** Scan Around Tptr Polygon And Un Mark Internal Points And Create Internal Tptr List
*/
 pp = Spnt ;
 sp = nodeAddrP(dtmP,Spnt)->tPtr ; 
 do
   {
    ap = np = nodeAddrP(dtmP,sp)->tPtr ;
    if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
    while ( ap != pp )
      {
       if( nodeAddrP(dtmP,ap)->tPtr == Mark )
         {
          nodeAddrP(dtmP,ap)->tPtr = dtmP->nullPnt ;
          ++(*NumUnMarked) ;
          clc = nodeAddrP(dtmP,ap)->cPtr ;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if(nodeAddrP(dtmP,lp)->tPtr == Mark )
               {
                if( Lpnt == dtmP->nullPnt ) { Fpnt = Lpnt = lp ; }
                else                        { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; }
                nodeAddrP(dtmP,lp)->tPtr = lp ;
               }
            }
         } 
       if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
      }
    pp = sp ;  
    sp = np ; 
   } while ( pp != Spnt ) ;
/*
** Scan Tptr List And Mark Points
*/
 while ( Fpnt != Lpnt )
   {
    np = nodeAddrP(dtmP,Fpnt)->tPtr ;
    nodeAddrP(dtmP,Fpnt)->tPtr = dtmP->nullPnt ;
    ++(*NumUnMarked) ;
    clc = nodeAddrP(dtmP,Fpnt)->cPtr ;
    while( clc != dtmP->nullPtr )
      {
       lp  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if(nodeAddrP(dtmP,lp)->tPtr == Mark ) 
         { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; nodeAddrP(dtmP,lp)->tPtr = lp ; }
      }
    Fpnt = np ;
   }
/*
** Mark Last Point
*/
 if( Lpnt != dtmP->nullPnt )
   {
    nodeAddrP(dtmP,Lpnt)->tPtr = Mark ;
    ++(*NumUnMarked) ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_calculateVolumesForMarkedInternalTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long Spnt,long Mark,long *NumUnMarked,VOLRANGETAB *rangeTableP,long numRanges,double Xmin,double Ymin,double Zmin,double *cutP,double *fillP,double *areaP ) 
/*
** This Function Marks All Points Internal To a tPtr Polygon
*/
{
 int  ret=DTM_SUCCESS ;
 long pp,sp,np,ap,lp,clc,Fpnt,Lpnt ;
/*
** Initialise
*/
 Fpnt = Lpnt = dtmP->nullPnt ;
 *NumUnMarked = 0 ;
/*
** Scan Around Tptr Polygon And Un Mark Internal Points And Create Internal Tptr List
*/
 pp = Spnt ;
 sp = nodeAddrP(dtmP,Spnt)->tPtr ; 
 do
   {
    ap = np = nodeAddrP(dtmP,sp)->tPtr ;
    if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
    while ( ap != pp )
      {
       if( nodeAddrP(dtmP,ap)->tPtr == Mark )
         {
          if( bcdtmTinVolume_calculateVolumesForPointDtmObject(dtmP,ap,rangeTableP,numRanges,Xmin,Ymin,Zmin,cutP,fillP,areaP)) goto errexit ;
          nodeAddrP(dtmP,ap)->tPtr = dtmP->nullPnt ;
          ++(*NumUnMarked) ;
          clc = nodeAddrP(dtmP,ap)->cPtr ;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if(nodeAddrP(dtmP,lp)->tPtr == Mark )
               {
                if( Lpnt == dtmP->nullPnt ) { Fpnt = Lpnt = lp ; }
                else                      { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; }
                nodeAddrP(dtmP,lp)->tPtr = lp ;
               }
            }
         } 
       if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
      }
    pp = sp ;  
    sp = np ; 
   } while ( pp != Spnt ) ;
/*
** Scan Tptr List And Calculate Volumes Points
*/
 while ( Fpnt != Lpnt )
   {
    np = nodeAddrP(dtmP,Fpnt)->tPtr ;
    if( bcdtmTinVolume_calculateVolumesForPointDtmObject(dtmP,Fpnt,rangeTableP,numRanges,Xmin,Ymin,Zmin,cutP,fillP,areaP)) goto errexit ;
    nodeAddrP(dtmP,Fpnt)->tPtr = dtmP->nullPnt ;
    ++(*NumUnMarked) ;
    clc = nodeAddrP(dtmP,Fpnt)->cPtr ;
    while( clc != dtmP->nullPtr )
      {
       lp  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if(nodeAddrP(dtmP,lp)->tPtr == Mark ) 
         { nodeAddrP(dtmP,Lpnt)->tPtr = lp ; Lpnt = lp ; nodeAddrP(dtmP,lp)->tPtr = lp ; }
      }
    Fpnt = np ;
   }
/*
** Calculate Volume For Last Point
*/
 if( Lpnt != dtmP->nullPnt )
   {
    if( bcdtmTinVolume_calculateVolumesForPointDtmObject(dtmP,Lpnt,rangeTableP,numRanges,Xmin,Ymin,Zmin,cutP,fillP,areaP)) goto errexit ;
    nodeAddrP(dtmP,Lpnt)->tPtr = dtmP->nullPnt ;
    ++(*NumUnMarked) ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_calculateVolumesForPointDtmObject(BC_DTM_OBJ *dtmP,long P1,VOLRANGETAB *rangeTableP,long numRanges,double Xmin,double Ymin,double Zmin,double *cutP,double *fillP,double *areaP)
/*
** This Function Calculates The Volumes For All Triangles About A Point
*/
{
 int    ret=DTM_SUCCESS ;
 long   clc,P2,P3,VoidFlag,TrgFlag ;
 double cut,fill,area,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,SZ1,SZ2,SZ3 ;
 DPoint3d    *TrgPts=NULL ;
 VOLRANGETAB *nr ;
/*
** If Void Point Dont Calculate Volume
*/
 if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) ) 
   {
    clc = nodeAddrP(dtmP,P1)->cPtr ;
    if( ( P2 = bcdtmList_nextAntDtmObject(dtmP,P1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       P3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       TrgFlag = 0 ;
       if     ( nodeAddrP(dtmP,P1)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P2)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P3)->tPtr == -dtmP->nullPnt ) TrgFlag = 1 ;
       else if( nodeAddrP(dtmP,P1)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P3)->tPtr == P2 ) TrgFlag = 1;
       else if( nodeAddrP(dtmP,P1)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P2)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P3)->tPtr !=  dtmP->nullPnt ) TrgFlag = 1 ;
       else if( nodeAddrP(dtmP,P1)->tPtr == -dtmP->nullPnt && nodeAddrP(dtmP,P2)->tPtr !=  dtmP->nullPnt && nodeAddrP(dtmP,P3)->tPtr == -dtmP->nullPnt ) TrgFlag = 1 ;
       else if((nodeAddrP(dtmP,P1)->tPtr != -dtmP->nullPnt && nodeAddrP(dtmP,P1)->tPtr < 0  && nodeAddrP(dtmP,P2)->tPtr < 0 && nodeAddrP(dtmP,P3)->tPtr < 0 ) ) TrgFlag = 1 ;
       if( TrgFlag )
         {
          if( nodeAddrP(dtmP,P1)->hPtr != P2 )
            {
             ++numVolTriangles ;
             bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,&VoidFlag) ;
             if( ! VoidFlag )
               {
                X1 = pointAddrP(dtmP,P1)->x - Xmin ; Y1 = pointAddrP(dtmP,P1)->y - Ymin ; Z1 = pointAddrP(dtmP,P1)->z - Zmin ;
                X2 = pointAddrP(dtmP,P2)->x - Xmin ; Y2 = pointAddrP(dtmP,P2)->y - Ymin ; Z2 = pointAddrP(dtmP,P2)->z - Zmin ;
                X3 = pointAddrP(dtmP,P3)->x - Xmin ; Y3 = pointAddrP(dtmP,P3)->y - Ymin ; Z3 = pointAddrP(dtmP,P3)->z - Zmin ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X1,Y1,&SZ1) ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X2,Y2,&SZ2) ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,TrgPts,X3,Y3,&SZ3) ;
                bcdtmTinVolume_prismToFlatPlane(0.0,X1,Y1,(Z1-SZ1),X2,Y2,(Z2-SZ2),X3,Y3,(Z3-SZ3),&cut,&fill,&area) ;
                *cutP  = *cutP  + cut ;  *fillP = *fillP + fill ; *areaP = *areaP + area ;
                if( numRanges > 0 ) 
                  {
                   for( nr = rangeTableP ; nr < rangeTableP + numRanges ; ++nr ) { nr->Low -= Zmin ; nr->High -= Zmin ; }
                   if( bcdtmTinVolume_calculateRangeVolumes(cut,fill,rangeTableP,numRanges,X1,Y1,SZ1,X2,Y2,SZ2,X3,Y3,SZ3,Z1,Z2,Z3) ) goto errexit ;
                   for( nr = rangeTableP ; nr < rangeTableP + numRanges ; ++nr ) { nr->Low += Zmin ; nr->High += Zmin ; }
                  }
               }
            } 
         }
       P2 = P3 ;
      } 
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_interpolateZOnPlaneOfTriangle(long CoefficientFlag,DPoint3d *TrgPts,double x,double y,double *z )
/*
** This routine finds the z value for a point on the triangle plane
*/
{
 static double ca=0.0,cb=0.0,cc=0.0,cd=0.0 ;
/*
** Calculate Coefficients of Plane
*/
 if( CoefficientFlag ) bcdtmMath_calculatePlaneCoefficients(TrgPts->x,TrgPts->y,TrgPts->z,(TrgPts+1)->x,(TrgPts+1)->y,(TrgPts+1)->z,(TrgPts+2)->x,(TrgPts+2)->y,(TrgPts+2)->z,&ca,&cb,&cc,&cd ) ;
/*
** Calculate z value
*/
 *z = - ( ca*x + cb*y + cd ) / cc   ;
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
BENTLEYDTM_Public int bcdtmTinVolume_intersectZLines(double P1,double P2,double S1,double S2,double *z)
/*
** This Function Intersects z Lines
*/
{
 double d1,d2 ;
 d1 = P2-P1 ;
 d2 = S2-S1 ;
 if     ( d1 == 0.0 ) *z = P1 ; 
 else if( d2 == 0.0 ) *z = S1 ;
 else if( d1 == d2  ) *z = (P1+P2) / 2.0 ;
 else                 *z = (P1*d2 - S1*d1) / ( d2-d1) ;
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTinVolume_calculateVolumeSurfaceToSurfaceDtmObjects
(
 BC_DTM_OBJ      *fromDtmP,
 BC_DTM_OBJ      *toDtmP,
 DTM_POLYGON_OBJ *polygonP,
 VOLRANGETAB     *rangeTableP,
 long            numRanges,
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function For Volume Polygons */
 void            *userP,
 double          &cutVolumeP,
 double          &fillVolumeP,
 double          &balanceVolumeP,
 double          &cutAreaP,
 double          &fillAreaP
 )
/*
** This Function Calculates The Volume Between Two Tins
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,nr,clc,numTriangles,voidTriangle,numPolyPts,dtmFeature,voidsInDtm ;
 DTMDirection hullDirection;
 double diff, cutTrg, fillTrg, cutAreaTrg, fillAreaTrg, totalPolyArea, polyArea, hullArea, voidArea, polyVoidArea = 0.0, totalPolyVoidArea = 0.0, rangeCut, rangeFill;
 DPoint3d    trgPts[4],*polyPtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
 DTM_POLYGON_LIST *pl ;
 VOLRANGETAB *vrTabP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Calculating Surface To Surface Tin Volumes") ;
    bcdtmWrite_message(0,0,0,"fromDtmP               = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"fromDtmP->numPoints    = %8ld",fromDtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"fromDtmP->numSortedPts = %8ld",fromDtmP->numSortedPoints) ;
    bcdtmWrite_message(0,0,0,"fromDtmP->ppTol        = %20.15lf",fromDtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"fromDtmP->plTol        = %20.15lf",fromDtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"fromDtmP->mppTol       = %20.15lf",fromDtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"toDtmP                 = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP-numPts          = %8ld",toDtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"toDtmP-numSortedPts    = %8ld",toDtmP->numSortedPoints) ;
    bcdtmWrite_message(0,0,0,"toDtmP-ppTol           = %20.15lf",toDtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"toDtmP-plTol           = %20.15lf",toDtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"toDtmP-mppTol          = %20.15lf",toDtmP->mppTol) ;
   }
/*
** Initialise Variables
*/
 cutVolumeP     = 0.0 ;
 fillVolumeP    = 0.0 ;
 cutAreaP = 0.0;
 fillAreaP = 0.0;
 balanceVolumeP = 0.0;
 if( numRanges > 0 && rangeTableP != NULL )
   {
    for ( vrTabP = rangeTableP ; vrTabP < rangeTableP + numRanges ; ++vrTabP )
      {
       vrTabP->Cut  = 0.0 ;
       vrTabP->Fill = 0.0 ;
      }
   }
/*
** Consectutively Process Volume Polygons
*/
 totalPolyArea = 0.0 ; 
 totalPolyVoidArea = 0.0 ;
 for( pl = polygonP->polyListP ; pl < polygonP->polyListP + polygonP->numPolygons ; ++pl )
   {
    polyArea = 0.0 ; 
    voidArea = 0.0 ;
    numTriangles = 0 ;
    totalPolyArea = totalPolyArea + pl->area ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Volume Polygon %3ld of %3ld",(long)(pl-polygonP->polyListP)+1,polygonP->numPolygons) ;
    if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polygonP,(long)(pl-polygonP->polyListP),&polyPtsP,&numPolyPts)) goto errexit ;
/*
**  Load Volume Polygon
*/
   if( loadFunctionP != NULL )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ; 
       if( loadFunctionP(DTMFeatureType::Polygon,toDtmP->nullUserTag,toDtmP->nullFeatureId,polyPtsP,numPolyPts,userP) != DTM_SUCCESS ) goto errexit ;
      }
/*
**  Clip Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin") ;
    if (bcdtmClip_cloneAndClipToPolygonDtmObject (toDtmP, &dtmP, polyPtsP, numPolyPts, DTMClipOption::External)) goto errexit;
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin Completed") ;
/*
**  Free DPoint3d Polygon Memory
*/
    if( polyPtsP != NULL ) 
      { 
       free(polyPtsP) ;
       polyPtsP = NULL ;
      }
/*
** Check Clipped Tin Area Against Polygon Area 
*/
    if( cdbg )
      {
       bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,dtmP->hullPoint,&hullArea,&hullDirection) ;
       diff = fabs(fabs(hullArea-pl->area)/pl->area) ;
       bcdtmWrite_message(0,0,0,"Polygon Area  = %20.10lf",pl->area) ;
       bcdtmWrite_message(0,0,0,"Hull Area     = %20.10lf",hullArea)  ;
       bcdtmWrite_message(0,0,0,"Difference    = %20.10lf",hullArea-pl->area)  ;
       bcdtmWrite_message(0,0,0,"Difference(%%) = %20.10lf",diff*100.0) ;
       if( diff > 0.001 ) bcdtmWrite_message(0,0,0,"Significant Area Differences") ; 
      }
/*
** Check For Voids In DTM
*/
 voidsInDtm = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && voidsInDtm == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType ==DTMFeatureType::Island ) ) voidsInDtm = TRUE ;
   }
/*
** Scan Cyclic List And Calculate Volumes For Each Triangle
*/
    for( p1 = 0 ;  p1 < dtmP->numPoints  ; ++p1  )
      {
       if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
         {
          if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,nodeAddrP(dtmP,p1)->cPtr)->pntNum)) < 0 ) goto errexit ;
          while( clc != dtmP->nullPtr )
            {
             p3  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p3)->hPtr != p1 )
               {
                if( dbg && numTriangles % 10000 == 0 ) bcdtmWrite_message(0,0,0,"Triangles Processed = %8ld of %8ld",numTriangles,dtmP->numTriangles) ;
/*
**              Test For Void Triangle
*/
                voidTriangle = FALSE ;
                if( voidsInDtm == TRUE ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
/*
**              Calculate Volumes For Triangle
*/
                if( ! voidTriangle )
                  {
                   trgPts[0].x = pointAddrP(dtmP,p1)->x ; trgPts[0].y = pointAddrP(dtmP,p1)->y ; trgPts[0].z = pointAddrP(dtmP,p1)->z ;
                   trgPts[1].x = pointAddrP(dtmP,p3)->x ; trgPts[1].y = pointAddrP(dtmP,p3)->y ; trgPts[1].z = pointAddrP(dtmP,p3)->z ;
                   trgPts[2].x = pointAddrP(dtmP,p2)->x ; trgPts[2].y = pointAddrP(dtmP,p2)->y ; trgPts[2].z = pointAddrP(dtmP,p2)->z ;
                   trgPts[3].x = pointAddrP(dtmP,p1)->x ; trgPts[3].y = pointAddrP(dtmP,p1)->y ; trgPts[3].z = pointAddrP(dtmP,p1)->z ;
                   if( bcdtmTinVolume_surfaceToSurfaceVolumeForTriangleDtmObject(fromDtmP,numTriangles,rangeTableP,numRanges,trgPts,4,cutTrg,fillTrg,cutAreaTrg,fillAreaTrg, voidArea)) goto errexit ;
                   cutVolumeP  = cutVolumeP  + cutTrg  ;
                   fillVolumeP = fillVolumeP + fillTrg ;
                   cutAreaP = cutAreaP + cutAreaTrg;
                   fillAreaP = fillAreaP + fillAreaTrg;
                   polyArea = polyArea + cutAreaTrg + fillAreaTrg;
                   polyVoidArea = polyVoidArea + voidArea ;
                  }
/*
**              Calculate Area Of Void Triangle
*/
                else if ( cdbg )
                  {
                   trgPts[0].x = pointAddrP(dtmP,p1)->x ; trgPts[0].y = pointAddrP(dtmP,p1)->y ;
                   trgPts[1].x = pointAddrP(dtmP,p3)->x ; trgPts[1].y = pointAddrP(dtmP,p3)->y ;
                   trgPts[2].x = pointAddrP(dtmP,p2)->x ; trgPts[2].y = pointAddrP(dtmP,p2)->y ;
                   polyVoidArea = polyVoidArea + bcdtmMath_coordinateTriangleArea(trgPts[0].x,trgPts[0].y,trgPts[1].x,trgPts[1].y,trgPts[2].x,trgPts[2].y) ;
                  }
                ++numTriangles ; 
               }
             p2  = p3 ;
            }
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangles Processed = %8ld of %8ld",numTriangles,dtmP->numTriangles) ;
/*
**  Destroy DTM
*/
    if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Check Areas
*/
    if( cdbg )
      {
       diff = fabs(fabs(polyArea+polyVoidArea-pl->area)/pl->area) ;
       bcdtmWrite_message(0,0,0,"Polygon Area  = %20.10lf",pl->area) ;
       bcdtmWrite_message(0,0,0,"Volume Area   = %20.10lf",polyArea)  ;
       bcdtmWrite_message(0,0,0,"Void  Area    = %20.10lf",polyVoidArea)  ;
       bcdtmWrite_message(0,0,0,"Difference    = %20.10lf",polyArea+polyVoidArea-pl->area) ;
       bcdtmWrite_message(0,0,0,"Difference(%%) = %20.10lf",diff*100.0) ;
       if( diff > 0.001 ) bcdtmWrite_message(0,0,0,"Significant Area Differences") ; 
       totalPolyVoidArea = totalPolyVoidArea + polyVoidArea ;
      }
   }
/*
** Calculate Balance
*/
 balanceVolumeP = cutVolumeP - fillVolumeP ;
/*
** Write Out Volumes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numVolTriangles = %9ld Cut = %12.4lf Fill = %12.4lf Balance = %12.4lf CutArea = %12.4lf FillArea = %12.4lf",numVolTriangles,cutVolumeP,fillVolumeP,balanceVolumeP,cutAreaP, fillAreaP) ;
    if( numRanges > 0 )
      {
       rangeCut = rangeFill = 0.0 ; 
       bcdtmWrite_message(0,0,0,"Volume Ranges") ;
       for( nr = 0 ; nr < numRanges ; ++nr )
         {
          bcdtmWrite_message(0,0,0,"Range %10.4lf to %10.4lf ** Cut = %12.4lf Fill = %12.4lf",rangeTableP[nr].Low,rangeTableP[nr].High,rangeTableP[nr].Cut,rangeTableP[nr].Fill) ;
          rangeCut  = rangeCut  + rangeTableP[nr].Cut  ;
          rangeFill = rangeFill + rangeTableP[nr].Fill ;
         }
       bcdtmWrite_message(0,0,0,"rangeCut = %12.4lf rangeFill   = %12.4lf rangeBalance = %12.4lf",rangeCut,rangeFill,rangeCut-rangeFill) ;
       bcdtmWrite_message(0,0,0,"diff(*cutVolumeP-rangeCut)     = %12.4lf",(cutVolumeP-rangeCut)) ;
       bcdtmWrite_message(0,0,0,"diff(*fillVolumeP-rangeFill)   = %12.4lf",(fillVolumeP-rangeFill)) ;
       bcdtmWrite_message(0,0,0,"diff(*balanceVolumeP-rangeBal) = %12.4lf",(balanceVolumeP-(rangeCut-rangeFill))) ;
      }
   }
/*
** Clean Tin Object
*/
 bcdtmList_cleanDtmObject(fromDtmP) ;
/*
** Check Tin Structure
*/
 if( cdbg ) 
   {
    if( bcdtmCheck_tinComponentDtmObject(fromDtmP)) goto errexit ;
   }
/*
** Check Areas  
*/
 if( cdbg )
   {
    totalPolyArea = totTrgArea ;
    diff = fabs(fabs((cutAreaP + fillAreaP)+totalPolyVoidArea-totalPolyArea)/totalPolyArea) ;
    bcdtmWrite_message(0,0,0,"Triangle Area = %20.10lf",totalPolyArea) ;
    bcdtmWrite_message(0,0,0,"Volume Area   = %20.10lf",cutAreaP + fillAreaP)  ;
    bcdtmWrite_message(0,0,0,"Void  Area    = %20.10lf",totalPolyVoidArea)  ;
    bcdtmWrite_message(0,0,0,"Difference    = %20.10lf",(cutAreaP + fillAreaP)+totalPolyVoidArea-totalPolyArea) ;
    bcdtmWrite_message(0,0,0,"Difference(%%) = %20.10lf",diff*100.0) ;
    if( diff > 0.001 ) bcdtmWrite_message(0,0,0,"Significant Area Differences") ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmTinVolume_surfaceToSurfaceVolumeForTriangleDtmObject(BC_DTM_OBJ *dtmP,long trgNum,VOLRANGETAB *volRangeTableP,long numRanges,DPoint3d *trgPtsP,long numTrgPts,double &cutVolumeP,double &fillVolumeP,double &cutAreaP, double &fillAreaP,double &voidAreaP) 
/*
** This Is The Controlling Routine For :-
**
** 1. Inserting A Triangle Into The Tin
** 2  Calculating The Triangle Volumes
** 3. Removing The Triangle From The Tin
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,sp,np,clc,startPnt,numPts,numFeatures,lastPnt,priorLastPnt,calcVolumes,insertError; 
 DTMDirection direction;
 double trgArea,tptrArea=0.0,diff ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing Triangle %8ld",trgNum) ;
/*
** Initialise
*/
 numPts       = dtmP->numPoints ;
 numFeatures  = dtmP->numFeatures ;
 cutVolumeP  = 0.0 ; 
 fillVolumeP = 0.0 ;
 cutAreaP = 0.0;
 fillAreaP = 0.0;
 voidAreaP = 0.0;
/*
**  Calculate Triangle Area
*/
 trgArea = bcdtmMath_coordinateTriangleArea(trgPtsP->x,trgPtsP->y,(trgPtsP+1)->x,(trgPtsP+1)->y,(trgPtsP+2)->x,(trgPtsP+2)->y) ;
 totTrgArea = totTrgArea + trgArea ;
 if( dbg ) bcdtmWrite_message(0,0,0,"trgNum = %8ld Area = %20.10lf",trgNum,trgArea) ;
/*
**  Only Calculate Volumes For Triangles With Areas Greater Than 0.0001
*/
 if( trgArea > 0.0001 )  
   { 
/*
** Insert Triangle Into Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Secondary Triangle") ;
    calcVolumes = TRUE ;
    insertError = 0 ;
    lastPnt = startPnt = priorLastPnt = dtmP->nullPnt ;
    if( ( insertError = bcdtmTinVolume_insertInternalTriangleIntoDtmObject(dtmP,1,trgPtsP,numTrgPts,&startPnt)) != DTM_SUCCESS )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Insert Error ** trgNum = %6ld trgArea = %12.7lf error = %2d startPnt = %9ld",trgNum,trgArea,insertError,startPnt) ;
       if( insertError == 11 || insertError == 12 )
         {
          if( startPnt == dtmP->nullPnt ) calcVolumes = FALSE ; 
          else
            {
             if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
             if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) calcVolumes = FALSE ;
             else
               {
                if( bcdtmList_getLastPointInTptrListDtmObject(dtmP,startPnt,&lastPnt,&priorLastPnt)) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"lastPnt = %9ld priorLastPnt = %9ld",lastPnt,priorLastPnt) ;
                if     ( lastPnt == dtmP->nullPnt || priorLastPnt == dtmP->nullPnt ) calcVolumes = FALSE ;
                else if(nodeAddrP(dtmP,priorLastPnt)->tPtr == dtmP->nullPnt ) calcVolumes = FALSE ;
                else if(nodeAddrP(dtmP,lastPnt)->tPtr  == priorLastPnt ) calcVolumes = FALSE ;
                else startPnt = lastPnt ;
               }
            }
         }
       else calcVolumes = FALSE ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Secondary Triangle Completed") ;
/*
**  Check Connectivity Of Inserted Traingle
*/
    if( calcVolumes == TRUE )
      {
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Connectivity Error In Triangle Polygon ") ;
          calcVolumes = FALSE ;
          insertError = 12 ;
         }
      } 
/*
**   Check For Only Two Points In Inserted Triangle
*/
    if( calcVolumes == TRUE )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Two Inserted Points") ;
       if(nodeAddrP(dtmP,nodeAddrP(dtmP,startPnt)->tPtr)->tPtr == startPnt ) calcVolumes = FALSE ;
      } 
/*
**  Check Direction And Area Of Inserted Tptr Triangle
*/
    if( calcVolumes == TRUE )
      {
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&tptrArea,&direction) ;
       if (direction == DTMDirection::Clockwise)
         {
          calcVolumes = FALSE ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Triangle Is Reversed ** Area = %20.10lf",tptrArea) ;
         }
       else if( TRUE )
         {
          diff = fabs(fabs(tptrArea-trgArea)/trgArea) ;
          if( trgArea > 0.01 && diff > 0.05 ) bcdtmWrite_message(0,0,0,"Insert Area Difference     ** trgNum = %6ld Triangle Area = %12.7lf Tptr Area = %12.7lf Percentage Difference = %10.4lf",trgNum,trgArea,tptrArea,diff*100.0) ;
         }
      }
/*
**  Calculate Prismoidal Triangle Volumes
*/
    if( calcVolumes == TRUE )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Volumes") ;
       if( bcdtmTinVolume_calculatePrismoidalVolumesForTriangleDtmObject(dtmP,volRangeTableP,numRanges,startPnt,trgPtsP,cutVolumeP,fillVolumeP,cutAreaP, fillAreaP,voidAreaP)) goto errexit ;
/*
**     Check Prismoidal Area Against Triangle Area 
*/
       if( TRUE )
         {
          diff = fabs(fabs((cutAreaP + fillAreaP)+voidAreaP-tptrArea)/tptrArea) ;
          if( trgArea > 0.01 && diff > 0.01 ) bcdtmWrite_message(0,0,0,"Prismoidal Area Difference ** trgNum = %6ld Tptr Triangle Area = %12.4lf Calculated Volume Area = %10.4lf Percentage Difference = %10.4lf",trgNum,tptrArea,cutAreaP + fillAreaP,diff*100.0) ;
         } 
      }
    else
      {
       bcdtmWrite_message(0,0,0,"Triangle Insert Error ** trgNum = %6ld trgArea = %12.7lf error = %2d startPnt = %9ld",trgNum,trgArea,insertError,startPnt) ;
      }  
/*
**  Null TPTR List
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Tptr List") ;
    if( ! insertError ) { sp = startPnt ; do { np = nodeAddrP(dtmP,sp)->tPtr ; nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ; sp = np ; } while ( sp != startPnt ) ;  }
    else                 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
**  Remove Inserted Points and Insert Deleted Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Inserted Points") ;
    for ( p1 = dtmP->numPoints - 1 ; p1 >= numPts ; --p1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Point %9ld",p1) ; 
/*
**     Remove Point From All Feature Lists
*/
       if( bcdtmInsert_removePointFromAllDtmFeaturesDtmObject(dtmP,p1)) goto errexit ;
/* 
**     Insert Deleted Lines
*/  
       if( dtmP->numFeatures > numFeatures )
         {
          if( p1 == ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmFeaturePts.firstPoint )
            {
            p2 = (long)ftableAddrP (dtmP, dtmP->numFeatures - 1)->dtmFeatureType;// Dont know what this is doing? but below is storing point nums in featureType.
             p3 = (long) ftableAddrP(dtmP,dtmP->numFeatures-1)->dtmUserTag ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Inserted Deleted Line %8ld ** %9ld %9ld",p1,p2,p3) ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p3,p1))  goto errexit ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p3,p2,p1)) goto errexit ;
             if( nodeAddrP(dtmP,p1)->hPtr == p3 ) nodeAddrP(dtmP,p2)->hPtr = p3 ;
             if( nodeAddrP(dtmP,p1)->hPtr == p2 ) nodeAddrP(dtmP,p3)->hPtr = p2 ;
             --dtmP->numFeatures ;
            } 
         }
/*
**     Remove Inserted Point
*/
       clc = nodeAddrP(dtmP,p1)->cPtr ;
       while( clc != dtmP->nullPtr  )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) goto errexit ;
         }  
       nodeAddrP(dtmP,p1)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,p1)->tPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,p1)->sPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,p1)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,p1)->fPtr = dtmP->nullPtr ;
      }
/*
**  Reset Tin Parameters
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Tin Parameters") ;
    dtmP->numPoints   = numPts ;
    dtmP->numFeatures = numFeatures ;
/*
**  Check Topology To Ensure Dtm Features Have Not Been
**  Corrupted By The Insertion And Removal Of Points
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Checking Topology After Restoring Primary Tin") ;
       if( bcdtmCheck_topologyDtmObject(dtmP,1))
         {
          bcdtmWrite_message(0,0,0,"Topology Error After Restoring Primary Tin ") ;
          goto errexit ;
         } 
       else bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
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
BENTLEYDTM_Private int  bcdtmTinVolume_insertInternalTriangleIntoDtmObject(BC_DTM_OBJ *dtmP,long drapeFlag,DPoint3d *trgPtsP,long numTrgPts,long *startPntP)
/*
**
** This Function Inserts A Triangle Into A Tin Object
** Assumes String Is Internal To Tin And Has Been Validated
**
** drapeFlag    = 1   Drape Intersect Vertices On Tin Surface
**              = 2   Break Intersect Vertices On Tin Surface 
** insertFlag   = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  pntNum,*ptsP,*pointsP=NULL ;
 DPoint3d   *p3d ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Triangle Into Tin ** dtmP->ppTol = %20.16lf",dtmP->ppTol) ;
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
/*
** Allocate Memory To Hold Point Numbers
*/ 
 pointsP = ( long * ) malloc ( numTrgPts * sizeof(long)) ;
 if( pointsP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store Points In Tin Object
*/
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Points In Tin") ;
 for( p3d = trgPtsP , ptsP = pointsP ; p3d < trgPtsP + numTrgPts - 1 ; ++p3d , ++ptsP)
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Point In Tin %12.5lf %12.5lf %10.4lf",p3d->x,p3d->y,p3d->z) ;
    if( bcdtmTinVolume_storePointInDtmObject(dtmP,drapeFlag,p3d->x,p3d->y,p3d->z,&pntNum)) goto errexit ;
    *ptsP = pntNum ;
/*
**  Check Precision After Inserting Point
*/
    if( cdbg == 2 )
      {
       if( bcdtmCheck_precisionDtmObject(dtmP,0)) 
         { 
          bcdtmWrite_message(0,0,0,"Tin Precision Errors After Storing Point") ;
          goto errexit ;
         }
      }
   }
 *ptsP = *pointsP ;
/*
** Check Direction Of Inserted Points
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,*pointsP,*(pointsP+1),*(pointsP+2) ) <= 0 )
   {
    bcdtmWrite_message(0,0,0,"Triangle Direction Reversed") ;
    ret = 10 ;
    goto errexit ;
   }
/*
** Check Precision After Inserting Points
*/
 if( cdbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Precision After Storing Points") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0)) 
      { 
       bcdtmWrite_message(0,0,0,"Tin Precision Errors After Storing Points") ;
       goto errexit ;
      }
   }
/*
** Write Area For Debug Purposes
*/
 if( dbg == 2 ) 
   {
    ptsP = pointsP ;
    bcdtmWrite_message(0,0,0,"Points Area = %15.7lf",bcdtmMath_coordinateTriangleArea(pointAddrP(dtmP,*ptsP)->x,pointAddrP(dtmP,*ptsP)->y,pointAddrP(dtmP,*(ptsP+1))->x,pointAddrP(dtmP,*(ptsP+1))->y,pointAddrP(dtmP,*(ptsP+2))->x,pointAddrP(dtmP,*(ptsP+2))->y)) ;
   } 
/*
** Store Lines In Tin Object
*/
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Lines In Tin") ;
 for( ptsP = pointsP + 1 ; ptsP < pointsP + numTrgPts ; ++ptsP )
   {
    if( *(ptsP-1) != *ptsP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line %6ld %6ld",*(ptsP-1),*ptsP) ;
       if( ( ret = bcdtmTinVolume_insertLineBetweenPointsDtmObject(dtmP,*(ptsP-1),*ptsP,drapeFlag)) != DTM_SUCCESS ) 
         {
          *startPntP = *pointsP ;
          goto errexit ;
         }
/*
**     Check Precision After Inserting Line
*/
       if( cdbg == 1 )
         {
          if( bcdtmCheck_precisionDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Tin Precision Errors After Inserting Line %6ld %6ld",*(ptsP-1),*ptsP) ; goto errexit ; }
          else                                         bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
         }
      }
   } 
/*
** Check Precision After Inserting Triangle
*/
 if( cdbg == 1 )
   {
    if( bcdtmCheck_precisionDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Tin Precision Errors After Inserting Triangle") ; ; goto errexit ; }
    else                                         bcdtmWrite_message(0,0,0,"Tin Precision OK After Inserting Triangle") ;
   }
/*
** Set Start Point
*/
 *startPntP = *pointsP ;
/*
** Clean Up
*/
 cleanup :
 if( pointsP != NULL ) free(pointsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal Triangle Into Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal Triangle Into Tin Error") ;
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
BENTLEYDTM_Private int bcdtmTinVolume_storePointInDtmObject(BC_DTM_OBJ *dtmP,long drapeFlag,double x,double y,double z,long *pntNumP)
/*
** This Function Stores A Point In The Tin Object
**
** drapeFlag    = 1   Drape Intersect Vertices On Tin Surface
**              = 2   Break Intersect Vertices On Tin Surface 
** insertFlag   = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
** internalFlag = 1   Point Is Internal To Tin
** 
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,pntType,trgPnt1,trgPnt2,trgPnt3,newPnt,precisionError=0 ;
 long   antPnt,clkPnt,voidFlag,onLine1,onLine2,onLine3,tableEntry,fixType=0 ;
 double Zs=0,d1,d2,d3,Xi,Yi ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Point %20.15lf %20.15lf %20.15lf In Tin",x,y,z) ;
/*
** Initialise
*/
 *pntNumP = dtmP->nullPnt ;
/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&Zs,&pntType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Point Type = %2ld",pntType) ;
/*
** If Point External Find Closest Hull Point Or Line  
*/
 if( pntType == 0 ) 
   {
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,x,y,&Zs,&pntType,&trgPnt1,&trgPnt2)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Closest Hull Line = %9ld %9ld",trgPnt1,trgPnt2) ;
       if( trgPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Point[%8ld] = %20.15lf %20.15lf %10.4lf",trgPnt1,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt1)->z) ;
       if( trgPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Point[%8ld] = %20.15lf %20.15lf %10.4lf",trgPnt2,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt2)->z) ;
      }
    if( ! pntType ) 
      {
       bcdtmWrite_message(2,0,0,"Cannot Not Find Closest Hull Line Or Point") ;
       goto errexit ;
      } 
    if( pntType == 2 ) pntType = 3 ;
   }  
/*
** Write Out Point Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Point Type = %2ld",pntType) ;
/*
** Check Point To Point Tolerances
*/
 if( pntType == 4 && dtmP->ppTol > 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point To Point Tolerances") ;
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    d3 = bcdtmMath_distance(x,y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"d1 = %20.15lf d2 = %20.15lf d3 = %20.15lf",d1,d2,d3) ;
    if      ( d1 <= d2 && d1 <= d3 && d1 < dtmP->ppTol ) { pntType = 1 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
    else if ( d2 <= d3 && d2 <= d1 && d2 < dtmP->ppTol ) { pntType = 1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
    else if ( d3 <= d1 && d3 <= d2 && d3 < dtmP->ppTol ) { pntType = 1 ; trgPnt1 = trgPnt3 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
   }
/*
** Check Point To Line Tolerances
*/
 if( pntType == 4 && dtmP->plTol > 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point To Line Tolerances") ;
    d1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,x,y,&Xi,&Yi) ;
    d2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,x,y,&Xi,&Yi) ;
    d3 = bcdtmMath_distanceOfPointFromLine(&onLine3,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,x,y,&Xi,&Yi) ;
    if     ( onLine1 && d1 <= d2 && d1 <= d3 && d1 < dtmP->plTol ) { pntType = 2 ; trgPnt3 = dtmP->nullPnt ; }
    else if( onLine2 && d2 <= d3 && d2 <= d1 && d2 < dtmP->plTol ) { pntType = 2 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = dtmP->nullPnt ; }
    else if( onLine3 && d3 <= d1 && d3 <= d2 && d3 < dtmP->plTol ) { pntType = 2 ; trgPnt2 = trgPnt1 ; trgPnt1 = trgPnt3 ; trgPnt3 = dtmP->nullPnt ; }
    if( pntType == 2 )
      {
       if      ( nodeAddrP(dtmP,trgPnt1)->hPtr == trgPnt2 )   pntType = 3 ; 
       else if ( nodeAddrP(dtmP,trgPnt2)->hPtr == trgPnt1 ) { pntType = 3 ; trgPnt3 = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = dtmP->nullPnt ; }
      }
   }
/*
** Move Points Within Point To Line Tolerance On To Line
*/
 if( ( pntType == 2 || pntType == 3 ) && dtmP->plTol > 0.0 )
   {
    d1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,x,y,&Xi,&Yi) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine1 = %2ld ** d1 = %20.15lf ** x = %20.15lf y = %20.15lf ** Xi = %20.15lf Yi = %20.15lf",onLine1,d1,x,y,Xi,Yi) ;
    x = Xi ;
    y = Yi ; 
   }
/*
** Check Point To Point Tolerance Line
*/
 if(  pntType == 2 || pntType == 3 && dtmP->ppTol > 0.0 )
   {
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    if      ( d1 <= d2 && d1 < dtmP->ppTol ) { pntType = 1 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
    else if ( d2 <= d1 && d2 < dtmP->ppTol ) { pntType = 1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
   }  
/*
** Check For Precision Problem Occurring If Point Is Inserted Into Internal Line
*/
 if( pntType == 2 )
   {
    if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,trgPnt1,trgPnt2,x,y,&precisionError,&antPnt,&clkPnt)) goto errexit ;
    if( precisionError ) 
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Detected While Inserting Point Onto Line") ;
       if( bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject(dtmP,antPnt,trgPnt2,clkPnt,trgPnt1,x,y,&x,&y,&fixType))  goto errexit ;
       if( fixType )
         {
          if( fixType == 1 ) { pntType = 1 ; trgPnt1 = trgPnt2 ; }
          if( fixType == 2 )   pntType = 1 ;
         }
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Not Fixed") ;
          goto errexit ;
         }  
      }
   }
/*
** Check For Precision Problem Occurring If Point Is Inserted Into Hull Line
*/
 if( pntType == 3 )
   {
    if( ( trgPnt3 = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
    if( bcdtmInsert_checkPointHullTrianglePrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,x,y,&precisionError) ) goto errexit ;
    if( precisionError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Potential Precision Error Detected For Inserting Point On Hull Line") ;
       if( bcdtmInsert_fixPointHullTrianglePrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,x,y,&x,&y,&fixType )) goto errexit ;
       if( ! fixType ) goto errexit ;
      } 
   }
/*
** Set z value For Point
*/
 if( pntType && drapeFlag == 1 ) z = Zs ;
/*
** Add Point To Tin
*/
 if( pntType > 1 ) 
   { 
    if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&newPnt)) goto errexit ; 
   }
 else newPnt = trgPnt1 ;
/*
** Write Out Point Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"==== Point Type = %2ld newPnt = %9ld",pntType,newPnt) ;
/*
**  Set VoidFlag
*/
 voidFlag = 0 ;   
/*
**  Insert Point Into Tin
*/
 switch ( pntType )
   {
    case  0 :      /* Point External To Tin             */
       bcdtmWrite_message(1,0,0,"Point %12.4lf %12.4lf %10.4lf External To Tin",x,y,z) ;
       goto errexit ;
    break   ;
      
    case  1 :      /* Coincident With Existing Point     */
      newPnt = trgPnt1 ;
       pointAddrP(dtmP,newPnt)->z = z ;
    break   ;

    case  2 :      /* Coincident With Internal Tin Line  */

      bcdtmList_testForVoidLineDtmObject(dtmP,trgPnt1,trgPnt2,&voidFlag) ;
      if( (p1 = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2))   < 0 ) goto errexit ; 
      if( (p2 = bcdtmList_nextClkDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ; 
      if(bcdtmList_deleteLineDtmObject(dtmP,trgPnt1,trgPnt2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt1,newPnt,p1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt1,dtmP->nullPnt)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt2,newPnt,p2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt2,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,newPnt,trgPnt2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p1,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,newPnt,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p2,trgPnt2)) goto errexit ; 
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,trgPnt1,trgPnt2) )
        {
         if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,trgPnt1,trgPnt2,newPnt)) goto errexit ; 
        }
/*
** Store Entry In feature Table For Rebuilding Tin After Removing Secondary Triangle
*/
      if (bcdtmInsert_addToFeatureTableDtmObject (dtmP, NULL, 0, (DTMFeatureType)trgPnt1, trgPnt2, DTM_NULL_FEATURE_ID, newPnt, &tableEntry)) goto errexit;// DH storing pntsNums in the featureType and FeatureId.
      
    break ;

    case  3 :      /* Coincident With External Tin Line  */
      bcdtmList_testForVoidLineDtmObject(dtmP,trgPnt1,trgPnt2,&voidFlag) ;
      if( (p1 = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2))   < 0 ) goto errexit ; 
      if(bcdtmList_deleteLineDtmObject(dtmP,trgPnt1,trgPnt2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt1,newPnt,p1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt1,dtmP->nullPnt)) goto errexit ; 
      if(bcdtmList_insertLineBeforePointDtmObject(dtmP,trgPnt2,newPnt,p1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt2,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,newPnt,trgPnt2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p1,trgPnt1)) goto errexit ; 
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,trgPnt1,trgPnt2) )
        {
         if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,trgPnt1,trgPnt2,newPnt)) goto errexit ;
        }  
      nodeAddrP(dtmP,trgPnt1)->hPtr = newPnt ;
      nodeAddrP(dtmP,newPnt)->hPtr = trgPnt2 ;
/*
** Store Entry In feature Table For Rebuilding Tin After Removing Secondary Triangle
*/
      if( bcdtmInsert_addToFeatureTableDtmObject(dtmP,NULL,0,(DTMFeatureType)trgPnt1,trgPnt2,DTM_NULL_FEATURE_ID,newPnt,&tableEntry)) goto errexit ; // DH storing pntsNums in the featureType and FeatureId.
      
    break ;

    case  4 :   /* In Triangle                      */
     bcdtmList_testForVoidTriangleDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,&voidFlag) ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt1,newPnt,trgPnt2)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt1,dtmP->nullPnt)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt2,newPnt,trgPnt3)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt2,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,trgPnt3,newPnt,trgPnt1)) goto errexit ; 
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,trgPnt3,trgPnt2)) goto errexit ; 
    break ;

    default :
      bcdtmWrite_message(1,0,0,"Illegal Point Find Code %6ld ",pntType) ;
      goto errexit ;
    break   ; 
   } ;
/*
** If Point In Void Set Void Bit
*/
 if( voidFlag ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,newPnt)->PCWD) ;
/*
** Set Point Value For Return
*/
 *pntNumP = newPnt ;
/*
** Write Cyclic List For Point
*/
 if( dbg ) bcdtmList_writeCircularListForPointDtmObject(dtmP,newPnt) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Point %12.5lf %12.5lf %12.5lf In Tin Completed",x,y,z) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Point %12.5lf %12.5lf %12.5lf In Tin Error",x,y,z) ;
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
BENTLEYDTM_Private int bcdtmTinVolume_insertLineBetweenPointsDtmObject(BC_DTM_OBJ *dtmP,long firstPnt,long lastPnt,long drapeFlag )
/*
** This Function Inserts A Line Between Two Points In a Tin Object
**
** dtmP        =     Pointer To Tin Object
** firstPnt    =     Start Point Of Line
** lastPnt     =     End Point Of Line
** drapeFlag   = 1   Drape Intersect Vertices On Tin Surface
**             = 2   Break Intersect Vertices On Tin Surface 
**  
*/
{
 int    ret=DTM_SUCCESS,bkp,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,p4,insertPnt,dtmFeatureLine,voidLine,tableEntry,precisionError,fixType ;
 double x,y,z=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Inserting Line Between %6ld and %6ld",firstPnt,lastPnt) ;
/*
** Initialise
*/
 p1 = dtmP->nullPnt ;
 p2 = dtmP->nullPnt ; 
 p3 = dtmP->nullPnt ;
 insertPnt = firstPnt ; 
/*
** Continue Insertion Until Insert Points Is Equal To Last Point
*/
 while ( insertPnt != lastPnt )
   {
/*
**   Check For Knot If So Return
*/
    if( nodeAddrP(dtmP,insertPnt)->tPtr != dtmP->nullPnt ) 
      {
       if( nodeAddrP(dtmP,nodeAddrP(dtmP,insertPnt)->tPtr)->tPtr == insertPnt )
         {
          nodeAddrP(dtmP,nodeAddrP(dtmP,insertPnt)->tPtr)->tPtr = dtmP->nullPnt ; 
         }
       else
         {
          ret = 11 ;    // Start Point Knot
          goto errexit ;
         }
      }
/*
**  Write Insertion Line
*/ 
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line Between %6ld and %6ld Angle = %12.10lf",insertPnt,lastPnt,bcdtmMath_getPointAngleDtmObject(dtmP,insertPnt,lastPnt)) ;
/*
**  Get Next Point
*/
    bkp = bcdtmTinVolume_getIntersectPointDtmObject(dtmP,insertPnt,lastPnt,p3,&dtmFeatureLine,&p1,&p2,&p3,&x,&y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"bkp = %2d ** x = %10.4lf y = %10.4lf ** p1 = %9ld p2 = %9ld p3 = %9ld",bkp,x,y,p1,p2,p3) ;
    if( bkp == 0 ) goto errexit ;
/*
**  A Knot Will Result In The Insert Line
*/
    if( bkp == 8 ) 
      {
       ret = 12 ;  //  Intersect Knot
       if( nodeAddrP(dtmP,p1)->tPtr == p2 ) nodeAddrP(dtmP,insertPnt)->tPtr = p1 ;
       if( nodeAddrP(dtmP,p2)->tPtr == p1 ) nodeAddrP(dtmP,insertPnt)->tPtr = p2 ;
       goto errexit ;
      }
/*
**  Check For Potential Precision Problem
*/
    if( bkp == 2 )
      { 
       if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,insertPnt,p1,p3,p2,x,y,&precisionError)) goto errexit ;
       if( precisionError ) 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Detected While Inserting Line") ;
          if( bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject(dtmP,insertPnt,p1,p3,p2,x,y,&x,&y,&fixType)) goto errexit ;
          if( fixType == 0 ) goto errexit ;
          else
            {
             if( fixType == 1 )   bkp = 1 ; 
             if( fixType == 2 ) { bkp = 1 ; p1 = p2 ; }
            }
         }
      }
/*
**  Calculate z Value Of Intercept Point
*/
    if( bkp == 1 || bkp == 2 || bkp == 3 )
      {
       if( bkp == 1 )      
         { 
          x = pointAddrP(dtmP,p1)->x ; 
          y = pointAddrP(dtmP,p1)->y ; 
         } 
       if( drapeFlag == 1 )  bcdtmInsert_getZvalueDtmObject(dtmP,p1,p2,x,y,&z) ;
       else                  bcdtmInsert_getZvalueDtmObject(dtmP,firstPnt,lastPnt,x,y,&z) ;
      }
/*
**  Passes Through Tin Point
*/ 
    if( bkp == 1 ) 
      { 
       nodeAddrP(dtmP,insertPnt)->tPtr = p1 ; 
       pointAddrP(dtmP,p1)->z = z ; 
       insertPnt = p1 ; 
      }
/*
**  Intersects Internal Line
*/
    if( bkp == 2 || bkp == 3 )
      {
/*
**     Check For Void Line
*/
       if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine) ) goto errexit ;
/*
**     Add P4 To Tin
*/
       if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&p4) ) goto errexit ;
/*
**     Set Void Bit In P4
*/
       if( voidLine ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,p4)->PCWD) ; 
/*
**     Delete p1-p2 And Add New Lines To Tin
*/
       if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2) ) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,insertPnt,p4,p1)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,insertPnt,dtmP->nullPnt)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,p4,insertPnt)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p1,insertPnt) ) goto errexit;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p4,insertPnt) ) goto errexit;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p4,p2,insertPnt)) goto errexit ;
       if( p3 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,p4,p2)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1)) goto errexit ;
         }  
/*
**     Store Entry In feature Table For Rebuilding Tin After Removing Secondary Triangle
*/
       if( bcdtmInsert_addToFeatureTableDtmObject(dtmP,NULL,0,(DTMFeatureType)p1,p2,DTM_NULL_FEATURE_ID,p4,&tableEntry)) goto errexit ; // DH storing pntsNums in the featureType and FeatureId.
/*
**     Intersects Tin Hull Line
*/   
       if( bkp == 3 )
         {
          if(nodeAddrP(dtmP,p1)->hPtr == p2 ) { nodeAddrP(dtmP,p1)->hPtr = p4 ;nodeAddrP(dtmP,p4)->hPtr = p2 ; }
          if(nodeAddrP(dtmP,p2)->hPtr == p1 ) { nodeAddrP(dtmP,p2)->hPtr = p4 ;nodeAddrP(dtmP,p4)->hPtr = p1 ; }
         } 
/*
**     Add Point To All Dtm Features
*/
       if( dtmFeatureLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p1,p2,p4)) goto errexit ; 
/*
**     Set Flag Byte For Intersect Point
*/
       bcdtmFlag_setFlag((unsigned char *)&nodeAddrP(dtmP,p4)->PCWD,2) ; /* PCWD Bit 3 */
/*
**     Update TPTR List
*/
       nodeAddrP(dtmP,insertPnt)->tPtr = p4 ; 
       insertPnt = p4 ; 
/*
**     Check Precision After Inserting Triangle
*/
       if( cdbg == 1 )
         {
          if( bcdtmCheck_precisionDtmObject(dtmP,0)) 
            { 
             bcdtmWrite_message(0,0,0,"Tin Precision Errors After Inserting Line Point") ;
             goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"**** Inserting Line Between %6ld and %6ld Completed",firstPnt,lastPnt) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"**** Inserting Line Between %6ld and %6ld Error",firstPnt,lastPnt) ;
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
BENTLEYDTM_Private int bcdtmTinVolume_getIntersectPointDtmObject(BC_DTM_OBJ *dtmP,long firstPnt,long lastPnt,long indexPnt,long *dtmFeatureLineP,long *pnt1P,long *pnt2P,long *pnt3P,double *xP,double *yP)
/*
**
** This Function Gets The Next Tin Line Intercept with firstPnt-lastPnt
**
** Return Values  == 0 Error Terminate
**                == 1 firstPnt-lastPnt Intecepts Point pnt1P
**                == 2 firstPnt-lastPnt Intercepts Internal Line pnt1P-pnt2P
**                == 3 firstPnt-lastPnt Intercepts Hull Line pnt1P-pnt2P
**                == 6 Point Merged Continue Processing
**                == 8 Knot Will Be Inserted In Tptr Array
**
*/
{
 int    dbg=DTM_TRACE_VALUE(0) ;
 long   p1=0,p2=0,p3,clc,sdof1,sdof2,sp1,sp2,scan,internalLine,onLine1,onLine2 ;
 long   hullPnt,hullPnt1,hullPnt2,firstPtr;
 DTMDirection direction;
 double d1,d2,n1,n2 ;
 double firstPntArea,hullPntArea ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Intersect Point ** firstPnt = %8ld lastPoint = %8ld",firstPnt,lastPnt) ;
/*
** Initialise Variables
*/
 *pnt1P = dtmP->nullPnt ;
 *pnt2P = dtmP->nullPnt ;
 *pnt3P = dtmP->nullPnt ;
 *xP = 0.0 ; 
 *yP = 0.0 ;
 sp1 = dtmP->nullPnt ; 
 sp2 = dtmP->nullPnt ;
/*
** Write Cyclic List For Point
*/
 if( dbg ) bcdtmList_writeCircularListForPointDtmObject(dtmP,firstPnt) ;
/*
**  Test If firstPnt and lastPnt Connected
*/
 if( bcdtmList_testLineDtmObject(dtmP,firstPnt,lastPnt)) 
   { 
    *pnt1P = lastPnt ;
    return(1) ;
   } 
/*
** Set Internal Line Flag
*/
 internalLine = 0 ;
/*
** If Index Point Is Null Scan Cyclic List
*/
 if( indexPnt == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Cyclic List") ;
    clc = nodeAddrP(dtmP,firstPnt)->cPtr  ;
    p2  = clistAddrP(dtmP,clc)->pntNum ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,p2)) < 0 ) return(0) ;
    scan = 1 ;
    while ( clc  != dtmP->nullPtr && scan )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( dbg ) bcdtmWrite_message(0,0,0,"p1 = %6ld p1->hPtr = %9ld p2 = %6ld p2->hPtr = %9ld",p1,nodeAddrP(dtmP,p1)->hPtr,p2,nodeAddrP(dtmP,p2)->hPtr) ;
       if( nodeAddrP(dtmP,firstPnt)->hPtr == p1) 
         { 
          sp1 = p1 ; 
          sp2 = p2 ; 
         }
       else
         {
          sdof1 = bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,firstPnt) ;
          sdof2 = bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,lastPnt) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"** sdof1 = %2d sdof2 = %2d",sdof1,sdof2) ;
          if( sdof1 != sdof2 && sdof1 != 0 && sdof2 != 0 )
            {
             sdof1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p1) ;
             sdof2 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p2) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"== sdof1 = %2d sdof2 = %2d",sdof1,sdof2) ;
             if( sdof1 != sdof2 )
               {
                if( sdof1 == 0 ) { *pnt1P = p1 ; return(1) ; }
                if( sdof2 == 0 ) { *pnt1P = p2 ; return(1) ; }
                if( sdof1 == 1 && sdof2 == -1 ) { internalLine = 1 ; scan = 0 ; }
               }
            }
         }
       if( scan ) p1  = p2 ;
      }
/*
**  If Line Not Found Then Line Must Go External To Tin
*/
    if( scan )
      {
       p1 = sp1 ;
       p2 = sp2 ;
      } 
   }  
/*
** Look At Points Either Side Of Index Point
*/
 if( indexPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Either Side Of Index Point") ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,indexPnt))   < 0 ) return(0) ;
    if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,firstPnt,indexPnt)) < 0 ) return(0) ;
    sdof1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,indexPnt) ;
    if( sdof1 ==  0 ) { *pnt1P = indexPnt ; return(1) ; }
    if( sdof1 == -1 )  p2 = indexPnt ;
    if( sdof1 ==  1 )  p1 = indexPnt ;
    internalLine = 1 ;
   }
/*
**  If Internal Get Intecept Point
*/
 if( internalLine )
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Line") ;
/*
** Write Internal Line
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"internalLine = %1ld ** p1 = %9ld p2 = %9ld",internalLine,p1,p2) ;
/*
** Test If p1-p2 Is In Tptr List
*/
    if( nodeAddrP(dtmP,p1)->tPtr == p2 || nodeAddrP(dtmP,p2)->tPtr == p1 )
      {
       *pnt1P = p1 ;
       *pnt2P = p2 ;
       return(8) ;
      } 
/*
**  Test If p1-p2 is A Dtm Feature Line
*/
    *dtmFeatureLineP = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ;
/*
**  Test Point To Line Tolerance Of firstPntLP with pnt1P && pnt2P
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Testing Point To Line") ;
    n1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xP,yP) ;
    n2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xP,yP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine1 = %2ld n1 = %20.16lf",onLine1,n1) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine2 = %2ld n2 = %20.16lf",onLine1,n2) ;
    if( n1 > dtmP->plTol || nodeAddrP(dtmP,p1)->tPtr != dtmP->nullPnt ) onLine1 = 0 ;
    if( n2 > dtmP->plTol || nodeAddrP(dtmP,p2)->tPtr != dtmP->nullPnt ) onLine2 = 0 ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->plTol = %12.10lf onLine1 = %2ld onLine2 =%2ld",dtmP->ppTol,onLine1,onLine2) ;
onLine1 = 0 ;
    if     ( onLine1 && onLine2 )
      {
       if( n1 <= n2  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
       else            { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
      }
    else if( onLine1  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
    else if( onLine2  ) { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
/*
**  Get Point On Opposite to firstPnt of pnt1Ppnt2P
*/
    if( (p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) return(0) ;
    if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3)) p3 = dtmP->nullPnt ;
/*
**  Intersect Insert Line And p1-p2
*/
    bcdtmInsert_normalIntersectInsertLineDtmObject(dtmP,firstPnt,lastPnt,p1,p2,xP,yP) ;
/*
**  Test For xP,yP ==  p1 or == p2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Testing Equality") ;
    if( pointAddrP(dtmP,p1)->x == *xP && pointAddrP(dtmP,p1)->y == *yP ) { *pnt1P = p1 ; return(1) ; }
    if( pointAddrP(dtmP,p2)->x == *xP && pointAddrP(dtmP,p2)->y == *yP ) { *pnt1P = p2 ; return(1) ; }
/*
** Test Point To Point Tolerance Of xPyP with pnt1P && pnt2P
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Testing Point To Point") ;
    onLine1 = onLine2 = 0 ;
    d1  = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
    d2  = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
    if( d1 < dtmP->ppTol ) onLine1 = 1 ;
    if( d2 < dtmP->ppTol ) onLine2 = 2 ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"onLine1 = %2ld d1 = %20.16lf",onLine1,d1) ;
       bcdtmWrite_message(0,0,0,"onLine2 = %2ld d2 = %20.16lf",onLine1,d2) ;
      }
 onLine1 = 0 ;
    if     ( onLine1 && onLine2 )
      {
       if( n1 <= n2  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
       else            { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
      }
    else if( onLine1  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
    else if( onLine2  ) { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
/*
** Set Values For Return
*/
    *pnt1P = p1 ;
    *pnt2P = p2 ;
    *pnt3P = p3 ;
    if( p3 == dtmP->nullPnt ) return(3) ;
    else                      return(2) ;
   }
/*
**  Insert Line Goes External
*/
 if( ! internalLine )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Insert Line Goes External") ;
       bcdtmWrite_message(0,0,0,"firstPnt = %6ld firstPnt->hPtr = %9ld ** lastPnt = %6ld lastPnt->hPtr = %9ld",firstPnt,nodeAddrP(dtmP,firstPnt)->hPtr,lastPnt,nodeAddrP(dtmP,lastPnt)->hPtr) ;
       bcdtmWrite_message(0,0,0,"p1 = %6ld p2 = %6ld",p1,p2) ;
      }
    if( nodeAddrP(dtmP,lastPnt)->hPtr != dtmP->nullPnt ) hullPnt = lastPnt ;
    else
      {
       bcdtmInsert_findClosestLineInterceptWithHullDtmObject(dtmP,firstPnt,lastPnt,&hullPnt1,&hullPnt2) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt1 = %9ld hullPnt2 = %9ld",hullPnt1,hullPnt2) ;
       if( hullPnt1 == dtmP->nullPnt && hullPnt2 == dtmP->nullPnt ) return(0) ;
       if( hullPnt1 == p1 ) { *pnt1P = hullPnt1 ; return(1) ; }
       if( hullPnt2 == p2 ) { *pnt1P = hullPnt2 ; return(1) ; }
       hullPnt = hullPnt1 ;
      } 
    firstPtr = nodeAddrP(dtmP,firstPnt)->hPtr ;
    nodeAddrP(dtmP,firstPnt)->hPtr = hullPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,firstPnt,&firstPntArea,&direction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt ** Area = %20.15lf direction = %1ld",firstPntArea,direction) ;
    nodeAddrP(dtmP,firstPnt)->hPtr = firstPtr ;
    firstPtr = nodeAddrP(dtmP,hullPnt)->hPtr ;
    nodeAddrP(dtmP,hullPnt)->hPtr = firstPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,hullPnt,&hullPntArea,&direction) ;
    nodeAddrP(dtmP,hullPnt)->hPtr = firstPtr ;
    if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt ** Area = %20.15lf direction = %1ld",hullPntArea,direction) ;
    if( firstPntArea <= hullPntArea ) *pnt1P = p2 ;
    else                              *pnt1P = p1 ;  
    return(1) ;
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
BENTLEYDTM_Private int bcdtmTinVolume_calculatePrismoidalVolumesForTriangleDtmObject(BC_DTM_OBJ *dtmP,VOLRANGETAB *volRangeTabP,long numRanges,long startPnt,DPoint3d *trgPtsP,double &cutP,double &fillP,double &cutAreaP, double &fillAreaP,double &voidAreaP)
/*
** Calculates the Volumes of all secondary Tin Triangles internal
** to a primary Tin Triangle
*/
{
 int      ret=DTM_SUCCESS,cdbg=DTM_CHECK_VALUE(0) ;
 long     p2,p3,firstPnt,nextPnt,clc,voidTriangle;
 DTMDirection tptrDirection;
 double   cutArea=0.0,fillArea = 0,area = 0, cut=0.0,fill=0.0,trgArea=0.0,tptrArea=0.0,volArea=0.0 ;
 double   X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3 ;
 double   sZ1,sZ2,sZ3,xMin,yMin,zMin ;
 VOLRANGETAB *vrTabP ;
/*
** Calculate Triangle And Tptr Polygon Areas For Comparison Reasons
*/
 if( cdbg )
   {
    bcdtmList_cleanTptrPolygonDtmObject(dtmP,startPnt) ;
    trgArea = bcdtmMath_coordinateTriangleArea(trgPtsP->x,trgPtsP->y,(trgPtsP+1)->x,(trgPtsP+1)->y,(trgPtsP+2)->x,(trgPtsP+2)->y) ;
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&tptrArea,&tptrDirection) ;
   }
/*
** Initialise 
*/
 cutP  = 0.0 ;
 fillP = 0.0 ;
 cutAreaP = 0.0;
 fillAreaP = 0.0;
 voidAreaP = 0.0;
/*
** Normalise Triangle Coordinates
*/
 xMin = trgPtsP->x ;
 yMin = trgPtsP->y ;
 zMin = trgPtsP->z ;
 if( (trgPtsP+1)->x < xMin ) xMin = (trgPtsP+1)->x ;
 if( (trgPtsP+2)->x < xMin ) xMin = (trgPtsP+2)->x ;
 if( (trgPtsP+1)->y < yMin ) yMin = (trgPtsP+1)->y ;
 if( (trgPtsP+2)->y < yMin ) yMin = (trgPtsP+2)->y ;
 if( (trgPtsP+1)->z < zMin ) zMin = (trgPtsP+1)->z ;
 if( (trgPtsP+2)->z < zMin ) zMin = (trgPtsP+2)->z ;
 trgPtsP->x = trgPtsP->x - xMin ;
 (trgPtsP+1)->x = (trgPtsP+1)->x - xMin ;
 (trgPtsP+2)->x = (trgPtsP+2)->x - xMin ;
 trgPtsP->y = trgPtsP->y - yMin ;
 (trgPtsP+1)->y = (trgPtsP+1)->y - yMin ;
 (trgPtsP+2)->y = (trgPtsP+2)->y - yMin ;
 trgPtsP->z = trgPtsP->z - zMin ;
 (trgPtsP+1)->z = (trgPtsP+1)->z - zMin ; 
 (trgPtsP+2)->z = (trgPtsP+2)->z - zMin ;
/*
** Calculate Plane Coefficients For Triangle
*/
 bcdtmTinVolume_interpolateZOnPlaneOfTriangle(1,trgPtsP,0.0,0.0,&sZ1) ;
/*
** Create Sptr List Of All Internal And Tptr Polygon Points
*/
 if( bcdtmTinVolume_createSptrListOfInternalAndTptrPolygonPointsDtmObject(dtmP,startPnt,&firstPnt) ) goto errexit ;
/*
** Scan Sptr List And Calculate Volumes For All Triangles
*/
 while ( firstPnt != dtmP->nullPnt )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->sPtr ;
/*
**  Scan Cyclic List For firstPnt And Accumulate Volumes
*/
    clc = nodeAddrP(dtmP,firstPnt)->cPtr ;
    if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( nodeAddrP(dtmP,p2)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,p3)->sPtr != dtmP->nullPnt )
         {
          if( nodeAddrP(dtmP,firstPnt)->hPtr != p2 )
            {
             ++numVolTriangles ;    
             bcdtmList_testForVoidTriangleDtmObject(dtmP,firstPnt,p2,p3,&voidTriangle) ;
             if( ! voidTriangle )
               {
                X1 = pointAddrP(dtmP,firstPnt)->x - xMin ; Y1 = pointAddrP(dtmP,firstPnt)->y - yMin ; Z1 = pointAddrP(dtmP,firstPnt)->z - zMin ;
                X2 = pointAddrP(dtmP,p2)->x - xMin ; Y2 = pointAddrP(dtmP,p2)->y - yMin ; Z2 = pointAddrP(dtmP,p2)->z - zMin ;
                X3 = pointAddrP(dtmP,p3)->x - xMin ; Y3 = pointAddrP(dtmP,p3)->y - yMin ; Z3 = pointAddrP(dtmP,p3)->z - zMin ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,trgPtsP,X1,Y1,&sZ1) ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,trgPtsP,X2,Y2,&sZ2) ;
                bcdtmTinVolume_interpolateZOnPlaneOfTriangle(0,trgPtsP,X3,Y3,&sZ3) ;
                bcdtmTinVolume_prismToFlatPlane(0.0,X1,Y1,(Z1-sZ1),X2,Y2,(Z2-sZ2),X3,Y3,(Z3-sZ3),cut,fill,cutArea, fillArea) ;
                cutP  = cutP  + cut  ;  
                fillP = fillP + fill ;
                cutAreaP = cutAreaP + cutArea;
                fillAreaP = fillAreaP + fillArea;
                area = cutArea + fillArea;
                if (numRanges > 0)
                  {
                   for( vrTabP = volRangeTabP ; vrTabP < volRangeTabP + numRanges ; ++vrTabP ) { vrTabP->Low -= zMin ; vrTabP->High -= zMin ; }
                   if( bcdtmTinVolume_calculateRangeVolumes(cut,fill,volRangeTabP,numRanges,X1,Y1,sZ1,X2,Y2,sZ2,X3,Y3,sZ3,Z1,Z2,Z3) ) goto errexit ;
                   for( vrTabP = volRangeTabP ; vrTabP < volRangeTabP + numRanges ; ++vrTabP ) { vrTabP->Low += zMin ; vrTabP->High += zMin ; }
                  }
               }
             else 
               {
                X1 = pointAddrP(dtmP,firstPnt)->x - xMin ; Y1 = pointAddrP(dtmP,firstPnt)->y - yMin ;
                X2 = pointAddrP(dtmP,p2)->x - xMin ; Y2 = pointAddrP(dtmP,p2)->y - yMin ;
                X3 = pointAddrP(dtmP,p3)->x - xMin ; Y3 = pointAddrP(dtmP,p3)->y - yMin ;
                area = bcdtmMath_coordinateTriangleArea(X1,Y1,X2,Y2,X3,Y3) ;
                voidAreaP = voidAreaP + area ;
               }
             volArea = volArea + area ;
            } 
         }
       p2 = p3 ;
      }
/*
**  Null Sptr For firstPnt
*/
    nodeAddrP(dtmP,firstPnt)->sPtr = dtmP->nullPnt ;
/*
**  Set firstPnt To nextPnt
*/
    if( nextPnt == firstPnt ) firstPnt = dtmP->nullPnt ;
    else                      firstPnt = nextPnt ; 
   }
/*
** Compare Areas For Validation Purposes
*/
 if( cdbg ) 
   {
    if( (trgArea-volArea)/trgArea > 0.001 )
      {
       bcdtmWrite_message(0,0,0,"*** trgArea = %12.7lf tptrArea = %12.7lf volArea = %12.7lf",trgArea,tptrArea,volArea) ;
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
BENTLEYDTM_Public int bcdtmTinVolume_createSptrListOfInternalAndTptrPolygonPointsDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *firstPntP ) 
/*
** This Function Creates An Sptr List Of The Internal And Tptr Polygon Points
** The TPTR Polygon Must Be AntiClockwise
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long priorPnt,scanPnt,nextPnt,antPnt,clPtr,firstPnt,lastPnt,numSptrPts ;
/*
** Initialise
*/
 *firstPntP = dtmP->nullPnt ;
 lastPnt    = dtmP->nullPnt ;
 numSptrPts = 0 ;
/*
** Add Points Immediately Internal To Tptr Polygon To Sptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Points Immediately Internal To Tptr Polygon") ;
 priorPnt = startPnt ;
 scanPnt  = nodeAddrP(dtmP,startPnt)->tPtr ; 
 do
   {
    antPnt = nextPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
    while ( antPnt != priorPnt )
      {
       if( nodeAddrP(dtmP,antPnt)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,antPnt)->tPtr == dtmP->nullPnt )
         {
          if( lastPnt == dtmP->nullPnt ) 
            { 
             *firstPntP = antPnt ;  
             lastPnt = antPnt ;  
            }
          else                            
            { 
             nodeAddrP(dtmP,lastPnt)->sPtr = antPnt ;
             lastPnt = antPnt ; 
            }
          nodeAddrP(dtmP,antPnt)->sPtr = antPnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point %9ld To Sptr List",antPnt) ;
          ++numSptrPts ;
         } 
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,antPnt)) < 0 ) goto errexit ;
      }
    priorPnt = scanPnt ;  
    scanPnt  = nextPnt ; 
   } while ( priorPnt != startPnt ) ;
/*
**  Write Number Of Points In Sptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points In Sptr List = %6ld",numSptrPts) ;
/*
** Scan Sptr List And Add Internal Connecting Points To Sptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Connecting Points") ;
 firstPnt = *firstPntP ;
 while ( firstPnt != dtmP->nullPnt  )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->sPtr ;
    clPtr   = nodeAddrP(dtmP,firstPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       scanPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr    = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(nodeAddrP(dtmP,scanPnt)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,scanPnt)->tPtr == dtmP->nullPnt )  
         { 
          nodeAddrP(dtmP,lastPnt)->sPtr = scanPnt ; 
          lastPnt = scanPnt ; 
          nodeAddrP(dtmP,scanPnt)->sPtr = scanPnt ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point %9ld To Sptr List",scanPnt) ;
          ++numSptrPts ;
         }
      }
    if( nextPnt == nodeAddrP(dtmP,nextPnt)->sPtr ) firstPnt = dtmP->nullPnt ;
    else                                        firstPnt = nextPnt ;
   }
/*
**  Write Number Of Points In Sptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points In Sptr List = %6ld",numSptrPts) ;
/*
** Add Tptr Polygon Points To Sptr List
*/
 scanPnt = startPnt ; 
 do
   {
    if(nodeAddrP(dtmP,scanPnt)->sPtr != dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"Sptr List Point Already Set") ;
       goto errexit ;
      }
/*
**   Add Tptr Polygon Point To Sptr List
*/
    if( lastPnt == dtmP->nullPnt ) 
      { 
       *firstPntP = scanPnt ;
       lastPnt  = scanPnt ;
      }
    else                            
      { 
       nodeAddrP(dtmP,lastPnt)->sPtr = scanPnt ; 
       lastPnt = scanPnt ; 
      }
    nodeAddrP(dtmP,scanPnt)->sPtr = scanPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point %9ld To Sptr List",scanPnt) ;
    ++numSptrPts ;
/*
**  Get Next Tptr Polygon Point
*/
    scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
   } while ( scanPnt != startPnt ) ;
/*
** Write Number Of Points In Sptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points In Sptr List = %6ld",numSptrPts) ;
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
