/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmString.cpp $
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
BENTLEYDTM_EXPORT  int bcdtmString_printArrayOfLineStringsToFile
(
 DTM_POINT_ARRAY **lineStringsPP,    /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long     numLineStrings,            /* ==> Number Of Line Strings                       */
 WCharCP fileNameP                 /* ==> File Name To Write To                        */               
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 DPoint3d  *p3dP ;
 DTM_POINT_ARRAY **lineStringPP ;
 FILE *fileFP=NULL ;
/*
** Check For Presence Of String Arrays
*/
 if( numLineStrings <= 0 || lineStringsPP == NULL ) goto errexit ;
/*
** Open File If File Name Present
*/
 if( wcslen(fileNameP) > 0 ) 
   {
    fileFP = bcdtmFile_open(fileNameP,L"w") ;
    if( fileFP == NULL )
      {
       bcdtmWrite_message(2,0,0,"Cannot Open Line String Print File %s",fileNameP) ;
       goto errexit ;
      }
   }
/*
** Write Strings
*/
 if( fileFP != NULL || dbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Line Strings = %6ld",numLineStrings) ;
    if( fileFP != NULL ) fprintf(fileFP,"Number Of Line Strings = %6ld\n",numLineStrings) ;
/*
**  Scan Array Of Strings
*/
    for( lineStringPP = lineStringsPP ; lineStringPP < lineStringsPP + numLineStrings ; ++lineStringPP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Line String[%6ld] ** Number Of Points = %6ld",(long)(lineStringPP-lineStringsPP)+1,(*lineStringPP)->numPoints) ;
       if( fileFP != NULL ) fprintf(fileFP,"Line String[%6ld] ** Number Of Points = %6ld\n",(long)(lineStringPP-lineStringsPP)+1,(*lineStringPP)->numPoints) ;
       if( (*lineStringPP)->numPoints > 0 )
         {
          for( p3dP = (*lineStringPP)->pointsP ; p3dP < (*lineStringPP)->pointsP + (*lineStringPP)->numPoints ; ++p3dP )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Point[%6ld] = %15.5lf %15.5lf %10.4lf",(long)(p3dP-(*lineStringPP)->pointsP)+1,p3dP->x,p3dP->y,p3dP->z) ;
             if( fileFP != NULL ) fprintf(fileFP,"**** Point[%6ld] = %15.5lf %15.5lf %10.4lf\n",(long)(p3dP-(*lineStringPP)->pointsP)+1,p3dP->x,p3dP->y,p3dP->z) ;
            }
         } 
      }
   }
/*
**  Clean Up
*/
 cleanup :
 if( fileFP != NULL ) { fclose(fileFP) ; fileFP = NULL ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
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
BENTLEYDTM_EXPORT  int bcdtmString_roundArrayOfLineStringsToPointTolerance3D
(
 DTM_POINT_ARRAY **lineStringsPP,    /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long     numLineStrings,            /* ==> Number Of Line Strings                       */
 double   pptol                      /* ==> Point To Point Tolerance                     */
)
{
 int  ret=DTM_SUCCESS ;
 long point,numDecPlaces=0 ;
/*
** Check For Presence Of String Arrays
*/
 if( numLineStrings <= 0 || lineStringsPP == NULL ) goto errexit ;
/*
** Validate
*/
 if( pptol <= 0.0 )        pptol = 0.0001 ;
 if( pptol <  0.00000001 ) pptol = 0.00000001 ;
/*
** Determine Number Of Decimal Points
*/
 numDecPlaces = 0 ;
 point = (long) pptol ;
 while( ! point  )
   {
    ++numDecPlaces ;
    pptol = pptol * 10.0 ;  
    point = (long)(pptol) ;
   } 
/*
** Round Strings To Decimal Places
*/
 if( bcdtmString_roundArrayOfLineStrings3D(lineStringsPP,numLineStrings,numDecPlaces)) goto errexit ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT  int bcdtmString_roundArrayOfLineStrings3D
(
 DTM_POINT_ARRAY **lineStringsPP,    /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long     numLineStrings,            /* ==> Number Of Line Strings                       */
 long     numDecPlaces               /* ==> Number Of Decimal Places To Round < 1,,8>    */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_POINT_ARRAY **lineStringPP ;
/*
** Check For Presence Of String Arrays
*/
 if( numLineStrings <= 0 || lineStringsPP == NULL ) goto errexit ;
/*
** Scan Array Of Strings
*/
 for( lineStringPP = lineStringsPP ; lineStringPP < lineStringsPP + numLineStrings ; ++lineStringPP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Rounding Line String %4ld of %4ld",(long)(lineStringPP-lineStringsPP)+1,numLineStrings) ;
    if( bcdtmString_roundLineStringCoordinates3D((*lineStringPP)->pointsP,(*lineStringPP)->numPoints,numDecPlaces)) goto errexit ;
   }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_roundLineStringCoordinates3D
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points                  */
 long     numStringPoints ,               /* ==> Number Of String Points                   */
 long     numDecPlaces                    /* ==> Number Of Decimal Places To Round < 1,,8> */
)
{
 int      ret=DTM_SUCCESS ;
 DPoint3d      *p3dP       ;
/*
** Check For Presence Of Line String
*/
 if( numStringPoints <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Check Number Of Decimal Places
*/
 if( numDecPlaces < 1 || numDecPlaces > 8 ) goto errexit ; 
/*
** Scan Points And Remove Points Within Point To Point Tolerance
*/
 for( p3dP = stringPointsP ; p3dP < stringPointsP + numStringPoints ; ++p3dP )
   {
/*
**  Round Coordinates
*/
    p3dP->x = bcdtmMath_roundToDecimalPoints(p3dP->x,numDecPlaces) ;
    p3dP->y = bcdtmMath_roundToDecimalPoints(p3dP->y,numDecPlaces) ;
    p3dP->z = bcdtmMath_roundToDecimalPoints(p3dP->z,numDecPlaces) ;
   }
/*
** Reset String Size
*/
 numStringPoints = (long)(p3dP - stringPointsP ) + 1 ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_removeDuplicatePoints2D
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points    */
 long     *numStringPointsP,              /* ==> Number Of String Points     */
 double   pptol                           /* ==> Point To Point Tolerance    */
)
{
 int      ret=DTM_SUCCESS ;
 long     removeFlag  ;
 DPoint3d      *p3d1P,*p3d2P ;
/*
** Check For Presence Of Line String
*/
 if( numStringPointsP <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Check Point To Point Tolerance Is Greater Than Or Equal To Zero
*/
 if( pptol < 0.0 ) pptol = 0.0 ; 
/*
** Scan Points And Remove Points Within Point To Point Tolerance
*/
 for( p3d1P = stringPointsP , p3d2P = stringPointsP + 1 ; p3d2P < stringPointsP + *numStringPointsP ; ++p3d2P )
   {
/*
** Mark Points To Be Removed
*/
    removeFlag = 0 ;
    if( fabs(p3d1P->x - p3d2P->x) <= pptol &&  fabs(p3d1P->y - p3d2P->y) <= pptol )
      {
       if( bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) <= pptol ) removeFlag = 1 ;
      }
/*
** Copy Over Removed Points
*/
    if( ! removeFlag )
      {
       ++p3d1P ;  
       if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
      }
   }
/*
** Reset String Size
*/
 *numStringPointsP = (long)(p3d1P - stringPointsP ) + 1 ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  double bcdtmString_getStringLength2D
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points    */
 long     numStringPoints,                /* ==> Number Of String Points     */
 double   *stringLengthP                  /* <== String Length               */
)
{
 int      ret=DTM_SUCCESS ;
 DPoint3d      *p3dP      ;
/*
** Initialise
*/
 *stringLengthP = 0.0 ;
/*
** Check For Presence Of Line String
*/
 if( numStringPoints <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Scan Points 
*/
 for( p3dP = stringPointsP  ; p3dP < stringPointsP + numStringPoints - 1 ; ++p3dP )
   {
    *stringLengthP = *stringLengthP + bcdtmMath_distance(p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y) ;
   }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  int bcdtmString_testForClosedString2D
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points    */
 long     numStringPoints,                /* ==> Number Of String Points     */
 long     *closeFlagP                     /* <== Pointer To Close Flag       */
)
{
 int      ret=DTM_SUCCESS ;
/*
** Initialise
*/
 *closeFlagP = 0 ;
/*
** Check For Presence Of Line String
*/
 if( numStringPoints <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Scan Points 
*/
 if( stringPointsP->x == (stringPointsP+numStringPoints-1)->x  &&
     stringPointsP->y == (stringPointsP+numStringPoints-1)->y      ) *closeFlagP = 1 ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  int bcdtmString_testIfStringCanBeClosed2D
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points          */
 long     numStringPoints,                /* ==> Number Of String Points           */
 int      *stringCloseP                   /* <== String Can Be Closed <0,1>        */
)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d      *p3dP      ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking If String Can Be Closed") ;
    bcdtmWrite_message(0,0,0,"stringPointsP = %p",stringPointsP) ;
    bcdtmWrite_message(0,0,0,"numStringPoints = %8ld",numStringPoints) ;
   }
/*
** Initialise
*/
 *stringCloseP = 1 ;
/*
** Check For Presence Of Line String
*/
 if( numStringPoints <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Scan For Intesections Of Closing Segment With Other Segments
*/    
 if( stringPointsP->x != (stringPointsP+numStringPoints-1)->x || stringPointsP->y != (stringPointsP+numStringPoints-1)->y ) 
   {
    for( p3dP = stringPointsP + 1  ; p3dP < stringPointsP + numStringPoints - 2 && *stringCloseP == 1 ; ++p3dP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking String Segment %6ld",(long)(p3dP-stringPointsP)) ;
       if( bcdtmMath_checkIfLinesIntersect(stringPointsP->x,stringPointsP->y,(stringPointsP+numStringPoints-1)->x,(stringPointsP+numStringPoints-1)->y,p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y)) *stringCloseP = 0 ;
      }  
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"stringCloseP = %2ld",*stringCloseP) ;
/*
**  Clean Up
*/
 cleanup :
/*
**  Return
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking If String Can Be Closed Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking If String Can Be Closed Error") ;
return(ret) ;
/*
**  Error Exit
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
BENTLEYDTM_Public  int bcdtmString_getBoundingCubeForString
(
 DPoint3d      *stringPointsP,                 /* ==> Pointer To String Points        */
 long     numStringPoints,                /* ==> Number Of String Points         */
 double   *xMinP,                         /* <== Pointer To x Minimum Coordinate */
 double   *yMinP,                         /* <== Pointer To y Minimum Coordinate */
 double   *zMinP,                         /* <== Pointer To z Minimum Coordinate */
 double   *xMaxP,                         /* <== Pointer To x Maximum Coordinate */
 double   *yMaxP,                         /* <== Pointer To y Maximum Coordinate */
 double   *zMaxP                          /* <== Pointer To y Maximum Coordinate */
)
{
 int  ret=DTM_SUCCESS ;
 DPoint3d  *p3dP ;
/*
** Initialise
*/
 *xMinP = 0.0 ;
 *yMinP = 0.0 ;
 *xMaxP = 0.0 ;
 *yMaxP = 0.0 ;
/*
** Check For Presence Of Line String
*/
 if( numStringPoints <= 0 || stringPointsP == NULL ) goto errexit ;
/*
** Scan String Points 
*/
 for( p3dP = stringPointsP ; p3dP < stringPointsP + numStringPoints ; ++p3dP )
   {
    if( p3dP == stringPointsP )
      {
       *xMinP = *xMaxP = p3dP->x ;
       *yMinP = *yMaxP = p3dP->y ;
       *zMinP = *zMaxP = p3dP->z ;
      }
    else
      {
       if( p3dP->x < *xMinP ) *xMinP = p3dP->x ;
       if( p3dP->x > *xMaxP ) *xMaxP = p3dP->x ;
       if( p3dP->y < *yMinP ) *yMinP = p3dP->y ;
       if( p3dP->y > *yMaxP ) *yMaxP = p3dP->y ;
       if( p3dP->z < *zMinP ) *zMinP = p3dP->z ;
       if( p3dP->z > *zMaxP ) *zMaxP = p3dP->z ;
      }
   }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  int bcdtmString_getBoundingCubeForArrayOfStrings
(
 DTM_POINT_ARRAY **pointArrayPP,          /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long     numPointArray,                  /* ==> Number Of Point Arrays                       */
 double   *xMinP,                         /* <== Pointer To x Minimum Coordinate              */
 double   *yMinP,                         /* <== Pointer To y Minimum Coordinate              */
 double   *zMinP,                         /* <== Pointer To z Minimum Coordinate              */
 double   *xMaxP,                         /* <== Pointer To x Maximum Coordinate              */
 double   *yMaxP,                         /* <== Pointer To y Maximum Coordinate              */
 double   *zMaxP                          /* <== Pointer To y Maximum Coordinate              */
)
{
 int  ret=DTM_SUCCESS ;
 DPoint3d  *p3dP ;
 DTM_POINT_ARRAY **pntArrayPP ;
/*
** Initialise
*/
 *xMinP = 0.0 ;
 *yMinP = 0.0 ;
 *xMaxP = 0.0 ;
 *yMaxP = 0.0 ;
/*
** Check For Presence Of String Arrays
*/
 if( numPointArray <= 0 || pointArrayPP == NULL ) goto errexit ;
/*
** Scan Array Of Strings
*/
 for( pntArrayPP = pointArrayPP ; pntArrayPP < pointArrayPP + numPointArray ; ++pntArrayPP )
   {
/*
**  Scan String Points 
*/
    for( p3dP = (*pntArrayPP)->pointsP ; p3dP < (*pntArrayPP)->pointsP + (*pntArrayPP)->numPoints ; ++p3dP )
      {
       if( pntArrayPP == pointArrayPP && p3dP == (*pntArrayPP)->pointsP )
         {
          *xMinP = *xMaxP = p3dP->x ;
          *yMinP = *yMaxP = p3dP->y ;
          *zMinP = *zMaxP = p3dP->z ;
         }
       else
         {
          if( p3dP->x < *xMinP ) *xMinP = p3dP->x ;
          if( p3dP->x > *xMaxP ) *xMaxP = p3dP->x ;
          if( p3dP->y < *yMinP ) *yMinP = p3dP->y ;
          if( p3dP->y > *yMaxP ) *yMaxP = p3dP->y ;
          if( p3dP->z < *zMinP ) *zMinP = p3dP->z ;
          if( p3dP->z > *zMaxP ) *zMaxP = p3dP->z ;
         }
      }
   }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo Generic Line String Intersection Function
* @doc  Generic Line String Intersection Function 
* @notes 1. This function determines the intersection points of a set of line strings
* @notes 2. The number of intersection points returned is twice the actual number
* @notes 3. The same intersection point is returned for each intersecting line strings
* @author Rob Cormack 24 February 2003 rob@geopak.com
* @param  *lineStrings      ==> Pointer To An Array Of Line String Pointers stored in structure DTM_POINT_ARRAY
* @param  numLineStrings    ==> Number Of Line Strings Pointers
* @param  **intPoints       <== Pointer To An Array Of Intersection Points stored in structure DTM_INTERSECT_POINT
* @param  *numIntPoints     <== Number Of Intersection Points 
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmString_detectStringIntersections2D
(
 DTM_POINT_ARRAY          **lineStringsPP,              /* ==> Pointer To An Array Of Line Strings   */
 long                     numLineStrings,               /* ==> Number Of Line Strings                */
 long                     intersectType,                /* ==> Type Of Intersect To Detect           */  
 DTM_INTERSECT_POINT      **intPointsPP,                /* <== Pointer To Intersection Points        */
 long                     *numIntPointsP                /* <== Number Of Intersection Points         */
)
{
 int                ret=DTM_SUCCESS ;
 long               numIntTable=0,memIntPoints=0,memIntPointsInc=5000 ;
 DTM_STRING_INTERSECT_TABLE  *intTableP=NULL ;
/*
** Initialise
*/
 *numIntPointsP = 0 ;
 if( *intPointsPP != NULL ) { free(*intPointsPP) ; *intPointsPP = NULL ; }
/*
** Check For Presence Of Line Strings
*/
 if( numLineStrings <= 0 || lineStringsPP == NULL ) goto errexit ;
/*
** Build Line String Table
*/
 if( bcdtmString_buildLineStringIntersectionTable2D(lineStringsPP,numLineStrings,&intTableP,&numIntTable) ) goto errexit ;
/*
** Scan For Line String Intersections
*/
 if( bcdtmString_scanForStringLineIntersections2D(intersectType,intTableP,numIntTable,intPointsPP,numIntPointsP,&memIntPoints,memIntPointsInc)) goto errexit ;
/*
** Reallocate Memory For Intersections Points
*/
 if( *numIntPointsP > 0 && *numIntPointsP != memIntPoints ) *intPointsPP = ( DTM_INTERSECT_POINT * ) realloc( *intPointsPP , *numIntPointsP * sizeof( DTM_INTERSECT_POINT )) ;
/*
** Sort Intersection Points On String Number
*/
 if( *numIntPointsP > 0 )
   {
    qsortCPP(*intPointsPP,*numIntPointsP,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCompareFunction) ;
   }
/*
**  Clean Up
*/
 cleanup :
 if( intTableP != NULL ) free(intTableP) ;
 if( *numIntPointsP == 0 && *intPointsPP != NULL ) { free(*intPointsPP) ; *intPointsPP = NULL ; }
/*
**  Return
*/
 return(ret) ;
/*
**  Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numIntPointsP = 0 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int  bcdtmString_buildLineStringIntersectionTable2D
(
 DTM_POINT_ARRAY **lineStringsPP,
 long numLineStrings,
 DTM_STRING_INTERSECT_TABLE **intTablePP,
 long *numIntTableP
) 
{
 int    ret=DTM_SUCCESS ;
 long   numString,numPoints,numSegments,memIntTable=0,memIntTableInc=0  ;
 double cord ; 
 DPoint3d    *p3dP,*pointsP ;
 DTM_POINT_ARRAY            **plinePP ;
 DTM_STRING_INTERSECT_TABLE *intP  ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
/*
** Determine Size Of Line String Intersection Table
*/
 memIntTableInc = 0 ;
 for( plinePP = lineStringsPP ; plinePP < lineStringsPP + numLineStrings ; ++plinePP )
   {
    memIntTableInc = memIntTableInc + (*plinePP)->numPoints - 1 ;
   } 
/*
** Scan Line Strings And Build Intersection Table
*/
 for( plinePP = lineStringsPP ; plinePP < lineStringsPP + numLineStrings ; ++plinePP )
   {
/*
** Set String Parameters
*/
    numString = (long)(plinePP-lineStringsPP) ;
    pointsP   = (*plinePP)->pointsP ;
    numPoints = (*plinePP)->numPoints ;
    numSegments = numPoints - 1 ;
/*
** Store String Segments In Intersection Table
*/
    for( p3dP = pointsP ; p3dP < pointsP + numPoints - 1 ; ++p3dP )
      {
/*
**  Check For Memory Allocation
*/
       if( *numIntTableP == memIntTable )
         {
          memIntTable = memIntTable + memIntTableInc ;
          if( *intTablePP == NULL ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) malloc ( memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
          else                      *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
          if( *intTablePP == NULL ) goto errexit ; 
         }
/*
**  Store String Line
*/
       (*intTablePP+*numIntTableP)->string      = numString ; 
       (*intTablePP+*numIntTableP)->segment     = (long)(p3dP-pointsP) ;
       (*intTablePP+*numIntTableP)->numSegments = numSegments ;
       (*intTablePP+*numIntTableP)->isReversed  = 0 ;
       (*intTablePP+*numIntTableP)->X1 = p3dP->x ;
       (*intTablePP+*numIntTableP)->Y1 = p3dP->y ;
       (*intTablePP+*numIntTableP)->Z1 = p3dP->z ;
       (*intTablePP+*numIntTableP)->X2 = (p3dP+1)->x ;
       (*intTablePP+*numIntTableP)->Y2 = (p3dP+1)->y ;
       (*intTablePP+*numIntTableP)->Z2 = (p3dP+1)->z ;
       ++*numIntTableP ;
      }
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
/*
** Order Line Coordinates In Increasing x and y Coordinate Values
*/
 for( intP = *intTablePP ; intP < *intTablePP + *numIntTableP ; ++intP )
   {
    if( intP->X1 > intP->X2 || ( intP->X1 == intP->X2 && intP->Y1 > intP->Y2 ) )
      {
       intP->isReversed = 1 ;
       cord = intP->X1 ; intP->X1 = intP->X2 ; intP->X2 = cord ;       
       cord = intP->Y1 ; intP->Y1 = intP->Y2 ; intP->Y2 = cord ;       
       cord = intP->Z1 ; intP->Z1 = intP->Z2 ; intP->Z2 = cord ;       
      }
   }
/*
** Sort Intersection Table
*/
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STRING_INTERSECT_TABLE),bcdtmString_intersectionTableCompareFunction) ;
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
 *numIntTableP = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmString_intersectionTableCompareFunction(const DTM_STRING_INTERSECT_TABLE *int1P,const DTM_STRING_INTERSECT_TABLE *int2P)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  int1P->X1  < int2P->X1 ) return(-1) ;
 else if(  int1P->X1  > int2P->X1 ) return( 1) ;
 else if(  int1P->Y1  < int2P->Y2 ) return(-1) ;
 else if(  int1P->Y1  > int2P->Y2 ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_scanForStringLineIntersections2D
(
 long  intersectType,
 DTM_STRING_INTERSECT_TABLE *intTableP,
 long numIntTable,
 DTM_INTERSECT_POINT **intPtsPP,
 long *numIntPtsP,
 long *memIntPtsP,
 long memIntPtsInc
)
/*
** This Function Scans Line Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 long    numActiveintTableP=0,memActiveintTableP=0 ;
 DTM_STRING_INTERSECT_TABLE *intP,*activeintTablePP=NULL ;
/*
** Scan Sorted Line String Table and Look For Intersections
*/
 for( intP = intTableP ; intP < intTableP + numIntTable  ; ++intP)
   {
    if( bcdtmString_deleteActiveStringLines(activeintTablePP,&numActiveintTableP,intP)) goto errexit ;
    if( bcdtmString_addActiveStringLine(&activeintTablePP,&numActiveintTableP,&memActiveintTableP,intP))  goto errexit ;
    if( bcdtmString_determineActiveStringLineIntersections(intersectType,activeintTablePP,numActiveintTableP,intPtsPP,numIntPtsP,memIntPtsP,memIntPtsInc)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( activeintTablePP != NULL ) { free(activeintTablePP) ; activeintTablePP = NULL ; }
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
BENTLEYDTM_Private int bcdtmString_deleteActiveStringLines
(
 DTM_STRING_INTERSECT_TABLE *activeIntTableP,
 long                       *numActiveIntTable,
 DTM_STRING_INTERSECT_TABLE *intTableP
)
/*
** This Functions Deletes Entries From The Active Line Intersection List
*/
{
 long   count=0 ;
 DTM_STRING_INTERSECT_TABLE *int1P,*int2P ;
/*
** Scan Active Line List And Mark Entries For Deletion
*/
 for ( int1P = activeIntTableP ; int1P < activeIntTableP + *numActiveIntTable ; ++int1P )
   {
    if( int1P->X2 < intTableP->X1 ) { int1P->string = -9999 ; ++count ; }
   }
 if( count == 0 ) return(0) ; 
/*
** Delete Marked Entries
*/
 if( count > 0 )
   {
    for( int1P = int2P = activeIntTableP ; int2P < activeIntTableP + *numActiveIntTable ; ++int2P )
      {
       if( int2P->string != -9999 )
         {
          if( int1P != int2P ) *int1P = *int2P ;
          ++int1P ;
         }
      }
   }
/*
** Reset Number Of Active Entries
*/
 *numActiveIntTable = *numActiveIntTable - count ;
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
BENTLEYDTM_Private int bcdtmString_addActiveStringLine(DTM_STRING_INTERSECT_TABLE **activeIntTableP,long *numActiveIntTable,long *memActiveIntTable,DTM_STRING_INTERSECT_TABLE *intP)
/*
** This Functions Adds An Entry To The Active Line List
*/
{
 long memInc=1000 ;
/*
** Test For Memory
*/
 if( *numActiveIntTable == *memActiveIntTable )
   {
    *memActiveIntTable = *memActiveIntTable + memInc ;
    if( *activeIntTableP == NULL ) *activeIntTableP = (DTM_STRING_INTERSECT_TABLE*)malloc ( *memActiveIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
    else                           *activeIntTableP = (DTM_STRING_INTERSECT_TABLE*)realloc( *activeIntTableP, *memActiveIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
    if( *activeIntTableP == NULL )  return(1) ; 
   }
/*
** Store Entry
*/
 *(*activeIntTableP+*numActiveIntTable) = *intP ;
 ++*numActiveIntTable ;
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
BENTLEYDTM_Private int bcdtmString_determineActiveStringLineIntersections
(
 long  intersectType,
 DTM_STRING_INTERSECT_TABLE *activeIntTableP,
 long numActiveIntTable,
 DTM_INTERSECT_POINT **intPtsP,
 long *numIntPtsP,
 long *memIntPtsP,
 long memIntPtsInc 
) 
/*
** Determine Line Intersections
** 
** Notes :-
**
** 1. If intersectType == 0     Process All Intersection
** 2. If intersectType == 1     Ignore Intersections Coincident With Line End Points     
*/
{
 int                ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long               discreetPoint=0 ;
 double             di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x=0.0,y=0.0 ;
 DTM_STRING_INTERSECT_TABLE  *activeLineP,*scanLineP ;
/*
** Initialise
*/
 activeLineP = activeIntTableP + numActiveIntTable - 1 ;
/*
** Scan Active Line List
*/
 for( scanLineP = activeIntTableP ; scanLineP < activeIntTableP + numActiveIntTable - 1 ; ++scanLineP )
   {
/*
**  Check Lines Are Not Consecutive Segments Of The Same Line String
*/
    if( activeLineP->string != scanLineP->string || ( activeLineP->string == scanLineP->string && labs(activeLineP->segment-scanLineP->segment) > 1 ) )
      {
/*
**     Check Lines Do Not Close Line String
*/
       if( bcdtmString_checkForClosingIntersectLines(activeLineP,scanLineP) == FALSE  )
         {
/*
**        Check For Coincident Line End Points
*/ 
          if( ! intersectType || bcdtmString_checkForConnectingIntersectLineEndPoints(activeLineP,scanLineP) == FALSE  )
            {
/*
**           Check Lines Intersect
*/
             if( bcdtmMath_checkIfLinesIntersect(scanLineP->X1,scanLineP->Y1,scanLineP->X2,scanLineP->Y2,activeLineP->X1,activeLineP->Y1,activeLineP->X2,activeLineP->Y2))
               {
/*
**              Check Lines Don't Intersect At Discreet Points
*/
                discreetPoint = FALSE ;
                if     ( ( scanLineP->X1 == activeLineP->X1 &&  scanLineP->Y1 == activeLineP->Y1 ) && ( scanLineP->Z1 == activeLineP->Z1 )) discreetPoint = 1 ;
                else if( ( scanLineP->X1 == activeLineP->X2 &&  scanLineP->Y1 == activeLineP->Y2 ) && ( scanLineP->Z1 == activeLineP->Z2 )) discreetPoint = 2 ;
                else if( ( scanLineP->X2 == activeLineP->X1 &&  scanLineP->Y2 == activeLineP->Y1 ) && ( scanLineP->Z2 == activeLineP->Z1 )) discreetPoint = 3 ;
                else if( ( scanLineP->X2 == activeLineP->X2 &&  scanLineP->Y1 == activeLineP->Y2 ) && ( scanLineP->Z2 == activeLineP->Z2 )) discreetPoint = 4 ;
                if( dbg && discreetPoint )
                  {
                   bcdtmWrite_message(0,0,0,"Discreet Intersect Point = %6ld",discreetPoint) ;
                   bcdtmWrite_message(0,0,0,"Scan   Line = %6ld ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",scanLineP->string,scanLineP->X1,scanLineP->Y1,scanLineP->Z1,scanLineP->X2,scanLineP->Y2,scanLineP->Z2) ;
                   bcdtmWrite_message(0,0,0,"Active Line = %6ld ** %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf",activeLineP->string,activeLineP->X1,activeLineP->Y1,activeLineP->Z1,activeLineP->X2,activeLineP->Y2,activeLineP->Z2) ;
                  }
/*
**              Check The Same Line Only Intersects At End Points
*/
                if( discreetPoint && scanLineP->string == activeLineP->string )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"scanLine   Segment = %4ld of %6ld",scanLineP->segment,scanLineP->numSegments-1) ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"activeLine Segment = %4ld of %6ld",activeLineP->segment,activeLineP->numSegments-1) ;
                   discreetPoint = 0 ;
                   if     ( scanLineP->segment   == 0 && activeLineP->segment == activeLineP->numSegments - 1 ) discreetPoint = 1 ;
                   else if( activeLineP->segment == 0 && scanLineP->segment   == scanLineP->numSegments   - 1 ) discreetPoint = 1 ;
                   if( dbg && discreetPoint == 0  ) bcdtmWrite_message(0,0,0,"False Discreet Point") ;
                  }
/*
**              Intersect Lines
*/          
                if( ! discreetPoint  )
                  {
                   bcdtmMath_normalIntersectCordLines(scanLineP->X1,scanLineP->Y1,scanLineP->X2,scanLineP->Y2,activeLineP->X1,activeLineP->Y1,activeLineP->X2,activeLineP->Y2,&x,&y) ;
             
/*
**                 Check Memory
*/
                   if( *numIntPtsP + 1 >= *memIntPtsP )
                     {
                      *memIntPtsP = *memIntPtsP + memIntPtsInc ;
                      if( *intPtsP == NULL ) *intPtsP = ( DTM_INTERSECT_POINT * ) malloc ( *memIntPtsP * sizeof(DTM_INTERSECT_POINT)) ;
                      else                   *intPtsP = ( DTM_INTERSECT_POINT * ) realloc( *intPtsP,*memIntPtsP * sizeof(DTM_INTERSECT_POINT)) ;
                      if( *intPtsP == NULL ) goto errexit ; 
                     }
/*
**                 Calculate Distances For Active Line String
*/
                   if( activeLineP->isReversed == 0 ) { Xs = activeLineP->X1 ; Ys = activeLineP->Y1 ; Zs = activeLineP->Z1 ; Xe = activeLineP->X2 ; Ye = activeLineP->Y2 ; Ze = activeLineP->Z2 ; }
                   if( activeLineP->isReversed == 1 ) { Xs = activeLineP->X2 ; Ys = activeLineP->Y2 ; Zs = activeLineP->Z2 ; Xe = activeLineP->X1 ; Ye = activeLineP->Y1 ; Ze = activeLineP->Z1 ; }
                   dz = Ze - Zs ;
                   di = bcdtmMath_distance(Xs,Ys,x,y) ;
                   dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
**                 Store Intersection Point Active Line String
*/
                   (*intPtsP+*numIntPtsP)->string1Offset  = activeLineP->string  ; 
                   (*intPtsP+*numIntPtsP)->segment1Offset = activeLineP->segment ; 
                   (*intPtsP+*numIntPtsP)->string2Offset  = scanLineP->string  ; 
                   (*intPtsP+*numIntPtsP)->segment2Offset = scanLineP->segment ; 
                   (*intPtsP+*numIntPtsP)->distance = di ; 
                   (*intPtsP+*numIntPtsP)->x = x ; 
                   (*intPtsP+*numIntPtsP)->y = y ; 
                   (*intPtsP+*numIntPtsP)->zSegment1 = Zs + dz * di / dl ; 
                   ++*numIntPtsP ; 
/*
**                 Calculate Distances For Scan Line String
*/
                   if( scanLineP->isReversed == 0 ) { Xs = scanLineP->X1 ; Ys = scanLineP->Y1 ; Zs = scanLineP->Z1 ; Xe = scanLineP->X2 ; Ye = scanLineP->Y2 ; Ze = scanLineP->Z2 ; }
                   if( scanLineP->isReversed == 1 ) { Xs = scanLineP->X2 ; Ys = scanLineP->Y2 ; Zs = scanLineP->Z2 ; Xe = scanLineP->X1 ; Ye = scanLineP->Y1 ; Ze = scanLineP->Z1 ; }
                   dz = Ze - Zs ;
                   di = bcdtmMath_distance(Xs,Ys,x,y) ;
                   dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
**                 Store Intersection Point For Scan Line String
*/
                   (*intPtsP+*numIntPtsP)->string1Offset  = scanLineP->string  ; 
                   (*intPtsP+*numIntPtsP)->segment1Offset = scanLineP->segment ; 
                   (*intPtsP+*numIntPtsP)->string2Offset  = activeLineP->string  ; 
                   (*intPtsP+*numIntPtsP)->segment2Offset = activeLineP->segment ; 
                   (*intPtsP+*numIntPtsP)->distance = di ; 
                   (*intPtsP+*numIntPtsP)->x = x ; 
                   (*intPtsP+*numIntPtsP)->y = y ; 
                   (*intPtsP+*numIntPtsP)->zSegment1 = Zs + dz * di / dl ; 
                   ++*numIntPtsP ; 
/*
**                 Store Other Line z Values
*/
                   (*intPtsP+*numIntPtsP-2)->zSegment2 = (*intPtsP+*numIntPtsP-1)->zSegment1 ; 
                   (*intPtsP+*numIntPtsP-1)->zSegment2 = (*intPtsP+*numIntPtsP-2)->zSegment1 ; 
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
BENTLEYDTM_Private int bcdtmString_checkForClosingIntersectLines
(
 DTM_STRING_INTERSECT_TABLE *activeLineP ,
 DTM_STRING_INTERSECT_TABLE *scanLineP 
)
{
 int    stringClose=FALSE ;
 double scanX,scanY,activeX,activeY ;
/*
** Check For Same String
*/ 
 if( activeLineP->string == scanLineP->string )
   {
/*
**  Check Intersect Lines Area The First And Last Segment Of A String
*/
    if(  ( activeLineP->segment == 0 && scanLineP->segment   == scanLineP->numSegments - 1   ) ||
         ( scanLineP->segment   == 0 && activeLineP->segment == activeLineP->numSegments - 1 )      )
      {
/*
**     Get End Points Coordinates Of Active Line
*/
       if( activeLineP->segment == 0 )
         {
          if( activeLineP->isReversed == 0 ) { activeX = activeLineP->X1 ; activeY = activeLineP->Y1 ;  }
          else                               { activeX = activeLineP->X2 ; activeY = activeLineP->Y2 ;  }
         }
       else
         { 
          if( activeLineP->isReversed == 1 ) { activeX = activeLineP->X2 ; activeY = activeLineP->Y2 ;  }
          else                               { activeX = activeLineP->X1 ; activeY = activeLineP->Y1 ;  }
         }             
/*
**     Get End Point Coordinates Of Scan Line
*/
       if( scanLineP->segment == 0 )
         {
          if( scanLineP->isReversed == 0 ) { scanX = scanLineP->X1 ; scanY = scanLineP->Y1 ;  }
          else                             { scanX = scanLineP->X2 ; scanY = scanLineP->Y2 ;  }
         }
       else
         { 
          if( scanLineP->isReversed == 1 ) { scanX = scanLineP->X2 ; scanY = scanLineP->Y2 ;  }
          else                             { scanX = scanLineP->X1 ; scanY = scanLineP->Y1 ;  }
         }             
/*
**     Check For Closing
*/
       if( activeX == scanX && activeY == scanY ) stringClose = TRUE ;
      }
   }
/*
** Return
*/
 return(stringClose) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmString_checkForConnectingIntersectLineEndPoints
(
 DTM_STRING_INTERSECT_TABLE *activeLineP ,
 DTM_STRING_INTERSECT_TABLE *scanLineP 
)
{
 int    dbg=DTM_TRACE_VALUE(0),coincidentEndPts=FALSE ;
 long   activeStart=FALSE,activeEnd=FALSE,scanStart=FALSE,scanEnd=FALSE ;
 double scanStartX=0.0,scanStartY=0.0,activeStartX=0.0,activeStartY=0.0,scanEndX=0.0,scanEndY=0.0,activeEndX=0.0,activeEndY=0.0 ;
/*
** Check For End Points
*/
 if( activeLineP->segment == 0 )                            activeStart = TRUE ; 
 if( activeLineP->segment == activeLineP->numSegments - 1 ) activeEnd   = TRUE ; 
 if( scanLineP->segment   == 0 )                            scanStart   = TRUE ; 
 if( scanLineP->segment   == scanLineP->numSegments - 1   ) scanEnd     = TRUE ; 
/*
** Check Both Lines Are End Lines
*/
 if(( activeStart == TRUE || activeEnd == TRUE ) && ( scanStart == TRUE || scanEnd == TRUE ) )
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"activeStart = %2ld activeEnd = %2ld ** scanStart = %2ld scanEnd = %2ld",activeStart,activeEnd,scanStart,scanEnd) ;
       bcdtmWrite_message(0,0,0,"active ** lineString = %4ld segment = %4ld",activeLineP->string,activeLineP->segment) ;
       bcdtmWrite_message(0,0,0,"scan   ** lineString = %4ld segment = %4ld",scanLineP->string,scanLineP->segment) ;
      }
/*
**  Set Active Line End Point Coordinates
*/
    if( activeStart == TRUE )
      {
       if( activeLineP->isReversed == 0 ) { activeStartX = activeLineP->X1 ; activeStartY = activeLineP->Y1 ;  }
       else                               { activeStartX = activeLineP->X2 ; activeStartY = activeLineP->Y2 ;  }
       if( dbg ) bcdtmWrite_message(0,0,0,"activeStartX = %20.10lf activeStartY = %20.10lf",activeStartX,activeStartY) ;
      }
    if( activeEnd == TRUE )
      {
       if( activeLineP->isReversed == 0 ) { activeEndX   = activeLineP->X2 ; activeEndY   = activeLineP->Y2 ;  }
       else                               { activeEndX   = activeLineP->X1 ; activeEndY   = activeLineP->Y1 ;  }
       if( dbg ) bcdtmWrite_message(0,0,0,"activeEndX   = %20.10lf activeEndY   = %20.10lf",activeEndX,activeEndY) ;
      }
/*
**  Set Scan Line End Point Coordinates
*/
    if( scanStart == TRUE )
      {
       if( scanLineP->isReversed == 0 ) { scanStartX = scanLineP->X1 ; scanStartY = scanLineP->Y1 ;  }
       else                             { scanStartX = scanLineP->X2 ; scanStartY = scanLineP->Y2 ;  }
       if( dbg ) bcdtmWrite_message(0,0,0,"scanStartX   = %20.10lf scanStartY   = %20.10lf",scanStartX,scanStartY) ;
      }
    if( scanEnd == TRUE )
      {
       if( scanLineP->isReversed == 0 ) { scanEndX   = scanLineP->X2 ; scanEndY   = scanLineP->Y2 ;  }
       else                             { scanEndX   = scanLineP->X1 ; scanEndY   = scanLineP->Y1 ;  }
       if( dbg ) bcdtmWrite_message(0,0,0,"scanEndX     = %20.10lf scanEndY     = %20.10lf",scanEndX,scanEndY) ;
      }
/*
**   Get End Points Coordinates Of Active Line
*/
     if     ( activeStart == TRUE && scanStart == TRUE  && activeStartX == scanStartX && activeStartY == scanStartY ) coincidentEndPts = TRUE ;
     else if( activeStart == TRUE && scanEnd   == TRUE  && activeStartX == scanEndX   && activeStartY == scanEndY   ) coincidentEndPts = TRUE ;
     else if( activeEnd   == TRUE && scanStart == TRUE  && activeEndX   == scanStartX && activeEndY   == scanStartY ) coincidentEndPts = TRUE ;
     else if( activeEnd   == TRUE && scanEnd   == TRUE  && activeEndX   == scanEndX   && activeEndY   == scanEndY   ) coincidentEndPts = TRUE ;
   }
/*
** Write result
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"coincidentEndPts = %2ld",coincidentEndPts) ;
// bcdtmWrite_message(0,0,0,"coincidentEndPts = %2ld",coincidentEndPts) ;
/*
** Return
*/
 return(coincidentEndPts) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_intersectPointsCompareFunction(const DTM_INTERSECT_POINT *intPnt1P,const DTM_INTERSECT_POINT  *intPnt2P)
/*
** Compare Function For Qsort Of String Line Intersection Points
*/
{
 if     ( intPnt1P->string1Offset  < intPnt2P->string1Offset  ) return(-1) ;
 else if( intPnt1P->string1Offset  > intPnt2P->string1Offset  ) return( 1) ;
 else if( intPnt1P->segment1Offset < intPnt2P->segment1Offset ) return(-1) ;
 else if( intPnt1P->segment1Offset > intPnt2P->segment1Offset ) return( 1) ;
 else if( intPnt1P->string2Offset  < intPnt2P->string2Offset  ) return(-1) ;
 else if( intPnt1P->string2Offset  > intPnt2P->string2Offset  ) return( 1) ;
 else if( intPnt1P->segment2Offset < intPnt2P->segment2Offset ) return(-1) ;
 else if( intPnt1P->segment2Offset > intPnt2P->segment2Offset ) return( 1) ;
 else if( intPnt1P->distance       < intPnt2P->distance       ) return(-1) ;
 else if( intPnt1P->distance       > intPnt2P->distance       ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_intersectPointsCoordinateCompareFunction(const DTM_INTERSECT_POINT *intPnt1P,const DTM_INTERSECT_POINT  *intPnt2P)
/*
** Compare Function For Qsort Of String Line Intersection Points Coordinates
*/
{
 if     ( intPnt1P->x              < intPnt2P->x              ) return(-1) ;
 else if( intPnt1P->x              > intPnt2P->x              ) return( 1) ;
 else if( intPnt1P->y              < intPnt2P->y              ) return(-1) ;
 else if( intPnt1P->y              > intPnt2P->y              ) return( 1) ;
 else if( intPnt1P->string1Offset  < intPnt2P->string1Offset  ) return(-1) ;
 else if( intPnt1P->string1Offset  > intPnt2P->string1Offset  ) return( 1) ;
 else if( intPnt1P->segment1Offset < intPnt2P->segment1Offset ) return(-1) ;
 else if( intPnt1P->segment1Offset > intPnt2P->segment1Offset ) return( 1) ;
 else if( intPnt1P->string2Offset  < intPnt2P->string2Offset  ) return(-1) ;
 else if( intPnt1P->string2Offset  > intPnt2P->string2Offset  ) return( 1) ;
 else if( intPnt1P->segment2Offset < intPnt2P->segment2Offset ) return(-1) ;
 else if( intPnt1P->segment2Offset > intPnt2P->segment2Offset ) return( 1) ;
 else if( intPnt1P->distance       < intPnt2P->distance       ) return(-1) ;
 else if( intPnt1P->distance       > intPnt2P->distance       ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmString_extractDtmFeatureTypeStringsFromDataObject
(
 DTM_DAT_OBJ *dataP,                   /* ==> Pointer To Data Object                           */
 DTMFeatureType dtmFeatureType,                 /* ==> Dtm Feature Type                                 */
 DTM_POINT_ARRAY ***featureArrayPPP,   /* <== Pointer To Pointer Array Of Feature Point Arrays */
 long  *numFeatureArrayP               /* <== Number Of Feature Arrays                         */  
)      
/*
** This Function Extracts The Points For All Occurrences Of A Dtm Feature Type In A Data Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   fsCode,fnCode,nextCode,numFeaturePts ;
 long   memPointArray=0,memPointArrayInc=1000 ;
 DPoint3d    *p3dP,*featurePtsP=NULL ;
 DTM_DATA_POINT   *startPntP,*endPntP,*dataPntP ;
 DTM_FEATURE_CODE *fcodeP,*featureStartP,*featureEndP ;
 DTM_POINT_ARRAY  *pointArrayP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Feature Strings") ;
/*
** Initialise
*/
 *numFeatureArrayP = 0 ;
 if( *featureArrayPPP != NULL ) 
   {
    bcdtmWrite_message(2,0,0,"Pointer To Pointer Array Of Dtm Point Arrays Not Null") ;
    goto errexit ;
   }
/*
** Get Start And Next Codes For Dtm Feature Type
*/
 if( bcdtmData_getFeatureCodesForDtmFeatureType(dtmFeatureType,&fsCode,&fnCode) ) goto errexit ;
 if( fsCode < 0 ) 
   { 
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type %4ld",dtmFeatureType) ;
    goto errexit ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature Type %4ld **startCode = %2ld nextCode = %2ld ",dtmFeatureType,fsCode,fnCode) ;
/*
** Count Number Of Features In Data Object
*/
 memPointArrayInc = 0 ;
 for( fcodeP = dataP->featureCodeP ; fcodeP < dataP->featureCodeP + dataP->numPts ; ++fcodeP )
   {
    if( *fcodeP == fsCode   )  ++memPointArrayInc ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dtm Feature Type Occurrence %4ld In Data Object = %6ld",dtmFeatureType,memPointArrayInc) ;
/*
** Scan Feature Codes And Extract The Dtm Feature Type Strings
*/ 
 for( fcodeP = dataP->featureCodeP ; fcodeP < dataP->featureCodeP + dataP->numPts ; ++fcodeP )
   {
    nextCode = DTM_NULL_PNT ;
    if( *fcodeP == fsCode   )  nextCode = fnCode  ;   
    if( nextCode != DTM_NULL_PNT ) 
      {
/*
**     Get Start And End Of Dtm Feature Type Occurrence In Data Object
*/
       featureStartP = fcodeP ; 
       ++fcodeP ; 
       while ( fcodeP < dataP->featureCodeP + dataP->numPts && *fcodeP == fnCode ) ++fcodeP ;
       --fcodeP ;
       featureEndP = fcodeP ;
/*
**     If More Than One Point In Feature Store Feature
*/
       if( featureEndP > featureStartP )
         {
          startPntP = dataP->pointsP + (long)(featureStartP - dataP->featureCodeP) ; 
          endPntP   = dataP->pointsP + (long)(featureEndP   - dataP->featureCodeP) ; 
       
/*
**        Allocate memory For Feature Points
*/
          numFeaturePts = (long)(endPntP-startPntP) + 1 ;
          featurePtsP   = ( DPoint3d * ) malloc(numFeaturePts * sizeof(DPoint3d)) ;
          if( featurePtsP == NULL ) 
            { 
             bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
             goto errexit ; 
            }
/*
**        Copy Data Object Points To Feature Points 
*/
          for( p3dP = featurePtsP , dataPntP = startPntP ; dataPntP <= endPntP ; ++p3dP , ++dataPntP )
            {
             p3dP->x = dataPntP->x ;
             p3dP->y = dataPntP->y ;
             p3dP->z = dataPntP->z ;
            } 
/*
**        Allocate Memory For Dtm Point Array
*/
          pointArrayP = ( DTM_POINT_ARRAY * ) malloc ( sizeof(DTM_POINT_ARRAY )) ;
          if( pointArrayP == NULL ) 
            { 
             bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
             goto errexit ; 
            }
/*
**        Populate Dtm Point Array
*/
          pointArrayP->pointsP   = featurePtsP   ;
          pointArrayP->numPoints = numFeaturePts ;
          featurePtsP = NULL ;
/*
**        Check Memory For Pointer Array To Dtm Point Arrays
*/
          if( *numFeatureArrayP == memPointArray )
            {
             if( bcdtmMem_allocatePointerArrayToPointArrayMemory(featureArrayPPP,*numFeatureArrayP,&memPointArray,memPointArrayInc)) goto errexit ;
            }
/*
**        Populate Pointer To  Point Arrays
*/      
          *(*featureArrayPPP+*numFeatureArrayP) = pointArrayP ;
          ++*numFeatureArrayP ;
          pointArrayP = NULL ;
         }
      }
   }
/*
** Realloc Memory
*/
 if( *featureArrayPPP != NULL && *numFeatureArrayP < memPointArray )
   {
    *featureArrayPPP = ( DTM_POINT_ARRAY ** ) realloc ( *featureArrayPPP , *numFeatureArrayP * sizeof(DTM_POINT_ARRAY)) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Extracting Feature Strings Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Extracting Feature Strings Error") ;
 return( ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( featurePtsP != NULL ) free(featurePtsP) ;
 if( pointArrayP != NULL )
   {
    if( pointArrayP->pointsP != NULL ) free(pointArrayP->pointsP) ;
    free(pointArrayP) ;
   }
 if( *featureArrayPPP != NULL )  bcdtmMem_freePointerArrayToPointArrayMemory(featureArrayPPP,memPointArray) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmString_lineEndPointsCompareFunction(const void *void1P,const void *void2P)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 struct Line_End_Points { double x,y,z ; long lineOffset,pointOffset ; } *lineEndPnt1P,*lineEndPnt2P ;

 lineEndPnt1P = ( struct Line_End_Points *) void1P ;
 lineEndPnt2P = ( struct Line_End_Points *) void2P ;

 if     (  lineEndPnt1P->x  < lineEndPnt2P->x ) return(-1) ;
 else if(  lineEndPnt1P->x  > lineEndPnt2P->x ) return( 1) ;
 else if(  lineEndPnt1P->y  < lineEndPnt2P->y ) return(-1) ;
 else if(  lineEndPnt1P->y  > lineEndPnt2P->y ) return( 1) ;
 else if(  lineEndPnt1P->z  < lineEndPnt2P->z ) return(-1) ;
 else if(  lineEndPnt1P->z  > lineEndPnt2P->z ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT  int bcdtmString_makeCloseEndPointsOfArrayOfLineStringsCoincident2D
(
 DTM_POINT_ARRAY **lineStringsPP,    /* ==> Pointer To Array Of Line Strings    */
 long     numLineStrings,            /* ==> Number Of Line Strings              */
 double   pptol                      /* ==> point to point tolerance            */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    numLineEndPts=0 ;
 DTM_POINT_ARRAY **lineStringPP ;
 struct Line_End_Points { double x,y,z ; long lineOffset,pointOffset ; } *lineEndPntP,*lineEndPnt1P,*lineEndPnt2P,*lineEndPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Making Close End Points Coincident") ;
/*
** Validate 
*/
 if( pptol < 0.0 ) pptol = 0.0001 ;
/*
** Check For Presence Of String Arrays
*/
 if( numLineStrings > 0 && lineStringsPP != NULL ) 
   {
/*
**  Remove Points Within Point To Point Tolerance
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Strings") ;
    for( lineStringPP = lineStringsPP ; lineStringPP < lineStringsPP + numLineStrings ; ++lineStringPP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates Line From String %4ld of %4ld",(long)(lineStringPP-lineStringsPP)+1,numLineStrings) ;
       if( bcdtmString_removeDuplicatePoints2D((*lineStringPP)->pointsP,&(*lineStringPP)->numPoints,pptol)) goto errexit ;
      } 
/*
**  Allocate Memory For Line End Points
*/
    numLineEndPts = numLineStrings * 2 ;
    lineEndPtsP = ( struct Line_End_Points * ) malloc( numLineEndPts * sizeof(struct Line_End_Points)) ;
    if( lineEndPtsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Populate Line End Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Line End Points Array") ;
    lineEndPntP = lineEndPtsP ;
    for( lineStringPP = lineStringsPP ; lineStringPP < lineStringsPP + numLineStrings ; ++lineStringPP )
      {
       lineEndPntP->lineOffset  = (long)(lineStringPP-lineStringsPP) ;
       lineEndPntP->pointOffset = 0 ;
       lineEndPntP->x = (*lineStringPP)->pointsP->x ;  
       lineEndPntP->y = (*lineStringPP)->pointsP->y ;  
       lineEndPntP->z = (*lineStringPP)->pointsP->z ;  
       ++lineEndPntP ;
       lineEndPntP->lineOffset  = (long)(lineStringPP-lineStringsPP) ;
       lineEndPntP->pointOffset = (*lineStringPP)->numPoints-1 ;
       lineEndPntP->x = ((*lineStringPP)->pointsP+(*lineStringPP)->numPoints-1)->x ;  
       lineEndPntP->y = ((*lineStringPP)->pointsP+(*lineStringPP)->numPoints-1)->y ;
       lineEndPntP->z = ((*lineStringPP)->pointsP+(*lineStringPP)->numPoints-1)->z ;
       ++lineEndPntP ;
      }
/*
**  Sort Line End Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting End Points Array") ;
    qsortCPP(lineEndPtsP,numLineEndPts,sizeof(struct Line_End_Points),bcdtmString_lineEndPointsCompareFunction) ;
/*
**  Write Sorted End Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Line End Points = %6ld",numLineEndPts) ;
       for( lineEndPntP = lineEndPtsP ; lineEndPntP < lineEndPtsP + numLineEndPts ; ++lineEndPntP  )
         {
          bcdtmWrite_message(0,0,0,"End Point[%6ld] ** x = %12.5lf y = %12.5lf z = %10.4lf ** lineOffset = %6ld pointOffset = %6ld",(long)(lineEndPntP-lineEndPtsP)+1,lineEndPntP->x,lineEndPntP->y,lineEndPntP->z,lineEndPntP->lineOffset,lineEndPntP->pointOffset) ;
         }
      } 
/*
**  Scan End Points Array And Make Near Coincident End Points Coincident
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning End Points For Near Coincident End Points") ;
    for( lineEndPnt1P = lineEndPtsP ; lineEndPnt1P < lineEndPtsP + numLineEndPts ; ++lineEndPnt1P )
      {
/*
**     Scan To x Difference Less Than pptol
*/
       lineEndPnt2P = lineEndPnt1P ;
       while( lineEndPnt2P < lineEndPtsP + numLineEndPts && fabs(lineEndPnt2P->x-lineEndPnt1P->x) < pptol ) ++lineEndPnt2P ;
       --lineEndPnt2P ;
/*
**     Scan Range And Set End Points Within Tolerance
*/
       if( lineEndPnt2P > lineEndPnt1P )
         {
          for( lineEndPntP = lineEndPnt1P + 1 ; lineEndPntP <= lineEndPnt2P ; ++lineEndPntP )
            {
             if( fabs(lineEndPntP->x - lineEndPnt1P->x) < pptol && fabs(lineEndPntP->y - lineEndPnt1P->y) < pptol )
               {
                if( bcdtmMath_distance(lineEndPntP->x,lineEndPntP->y,lineEndPnt1P->x,lineEndPnt1P->y) < pptol )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Setting End Point ** line = %6ld point = %6ld To line = %6ld point = %6ld",lineEndPntP->lineOffset,lineEndPntP->pointOffset,lineEndPnt1P->lineOffset,lineEndPnt1P->pointOffset) ;
                   lineEndPntP->x = lineEndPnt1P->x ;
                   lineEndPntP->y = lineEndPnt1P->y ;
                   lineEndPntP->z = lineEndPnt1P->z ;
                  }
               }
            }
         }
      }
/*
**  Write Coincident End Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Line End Points = %6ld",numLineEndPts) ;
       for( lineEndPntP = lineEndPtsP ; lineEndPntP < lineEndPtsP + numLineEndPts ; ++lineEndPntP  )
         {
          bcdtmWrite_message(0,0,0,"End Point[%6ld] ** x = %12.5lf y = %12.5lf z = %10.4lf ** lineOffset = %6ld pointOffset = %6ld",(long)(lineEndPntP-lineEndPtsP)+1,lineEndPntP->x,lineEndPntP->y,lineEndPntP->z,lineEndPntP->lineOffset,lineEndPntP->pointOffset) ;
         }
      } 
/*
**  Scan End Points Array And Reset Line End Point Coordinates
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning End Points Array And Resetting String End Points") ;
    for( lineEndPntP = lineEndPtsP ; lineEndPntP < lineEndPtsP + numLineEndPts ; ++lineEndPntP )
      {
       ((*(lineStringsPP+lineEndPntP->lineOffset))->pointsP+lineEndPntP->pointOffset)->x = lineEndPntP->x ;
       ((*(lineStringsPP+lineEndPntP->lineOffset))->pointsP+lineEndPntP->pointOffset)->y = lineEndPntP->y ;
       ((*(lineStringsPP+lineEndPntP->lineOffset))->pointsP+lineEndPntP->pointOffset)->z = lineEndPntP->z ;
      }
   }
/*
**  Clean Up
*/
 cleanup :
 if( lineEndPtsP != NULL ) { free(lineEndPtsP) ; lineEndPtsP = NULL ; }
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Making Close End Points Coincident Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Making Close End Points Coincident Error") ;
 return(ret) ;
/*
**  Error Exit
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
BENTLEYDTM_Public int bcdtmString_intersectDtmFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To DTM Object                          */
 DTMFeatureType  *featureListP,        /* ==> Features To Be Included For Intersection       */
 long  numFeatureList        /* ==> Number Of Features In List                     */
)
/*
** This Function Intersects Crossing Features In An Untriangulated DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,intersectType=1,numIntTable=0,numIntPoints=0,memIntPoints=0,memIntPointsInc=10000 ;
 char  dtmFeatureTypeName[50] ;
 DTM_STRING_INTERSECT_TABLE  *intTableP=NULL ;
 DTM_INTERSECT_POINT  *intP,*intPointsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Intersecting Crossing Features") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureListP    = %p",featureListP) ;
    bcdtmWrite_message(0,0,0,"numFeatureList  = %8ld",numFeatureList) ;
    if( dbg == 2 )
      {
       for( n = 0 ; n < numFeatureList ; ++n )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(*(featureListP+n),dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Feature[%2ld] ** Type = %4ld Name = %30s",n+1,*(featureListP+n),dtmFeatureTypeName) ;
         }
      } 
   }
/*
** Check For Valid DTM Object
*/   
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Data State DTM
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Inavlid DTM State For Method") ;
    goto errexit ;
   } 
/*
** Create Intersection Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Feature Intersection Tables") ; 
 if( bcdtmString_buildFeatureIntersectionTableDtmObject(dtmP,featureListP,numFeatureList,&intTableP,&numIntTable) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numIntTable = %6ld",numIntTable) ;
/*
** Scan For Line String Intersections
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ; 
 if( bcdtmString_scanForStringLineIntersections2D(intersectType,intTableP,numIntTable,&intPointsP,&numIntPoints,&memIntPoints,memIntPointsInc)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numIntPoints = %6ld",numIntPoints) ; 
/*
** Reallocate Memory For Intersections Points
*/
 if( numIntPoints > 0 && numIntPoints != memIntPoints ) intPointsP = ( DTM_INTERSECT_POINT * ) realloc( intPointsP , numIntPoints * sizeof( DTM_INTERSECT_POINT )) ;
/*
** Write Out Intersection Points
*/
 if( dbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %6ld",numIntPoints) ; 
    for( intP = intPointsP ; intP < intPointsP + numIntPoints ; ++intP )
      {
      }
   }
/*
** Sort Intersection Points On String Number
*/
 if( numIntPoints > 0  )
   {
    qsortCPP(intPointsP,numIntPoints,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCompareFunction) ;
   }
/*
** Insert Intersection Points Into Features
*/

//  TODO - For Removal Of Old Code
/*
** Cleanup
*/
 cleanup :
 if( intTableP  != NULL ) free(intTableP) ;
 if( intPointsP != NULL ) free(intPointsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Crossing Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Crossing Features Error") ;
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
BENTLEYDTM_Public int  bcdtmString_buildFeatureIntersectionTableDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType  *featureListP,
 long  numFeatureList,
 DTM_STRING_INTERSECT_TABLE **intTablePP,
 long *numIntTableP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   point, dtmFeature;
 DTMFeatureType *featureP;
 long   memIntTable=0,memIntTableInc=10000  ;
 double cord ; 
 DTM_TIN_POINT *pnt1P ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_STRING_INTERSECT_TABLE *intP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table") ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
/*
** Scan DTM Features 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For DTM Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if(dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
       {
/*
**      Scan Feature List For Features Equal To This Feature
*/
        for( featureP = featureListP ; featureP < featureListP + numFeatureList ; ++featureP )
          {
           if( *featureP == dtmFeatureP->dtmFeatureType )
             { 
              pnt1P = pointAddrP(dtmP,dtmFeatureP->dtmFeaturePts.firstPoint) ;
              for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ; ++point )
                {
/*
**               Check For Memory Allocation
*/
                 if( *numIntTableP == memIntTable )
                   {
                    memIntTable = memIntTable + memIntTableInc ;
                    if( *intTablePP == NULL ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) malloc ( memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
                    else                      *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
                    if( *intTablePP == NULL ) goto errexit ; 
                   }
/*
**               Store Dtm Feature
*/
                 (*intTablePP+*numIntTableP)->string      = dtmFeature ; 
                 (*intTablePP+*numIntTableP)->segment     = point - dtmFeatureP->dtmFeaturePts.firstPoint ;
                 (*intTablePP+*numIntTableP)->numSegments = dtmFeatureP->numDtmFeaturePts - 1 ;
                 (*intTablePP+*numIntTableP)->isReversed  = 0 ;
                 (*intTablePP+*numIntTableP)->X1 = pnt1P->x ;
                 (*intTablePP+*numIntTableP)->Y1 = pnt1P->y ;
                 (*intTablePP+*numIntTableP)->Z1 = pnt1P->z ;
                 pnt1P = pointAddrP(dtmP,point+1) ;
                 (*intTablePP+*numIntTableP)->X2 = pnt1P->x ;
                 (*intTablePP+*numIntTableP)->Y2 = pnt1P->y ;
                 (*intTablePP+*numIntTableP)->Z2 = pnt1P->z ;
                 ++*numIntTableP ;
                } 
             }
          }
        }
   } 
/*
** Reallocate Intersection Table Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Intersection Table ** numIntTable = %8ld",*numIntTableP) ;
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
/*
** Order Line Coordinates In Increasing x and y Coordinate Values
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Ordering Line Coordinates") ;
 for( intP = *intTablePP ; intP < *intTablePP + *numIntTableP ; ++intP )
   {
    if( intP->X1 > intP->X2 || ( intP->X1 == intP->X2 && intP->Y1 > intP->Y2 ) )
      {
       intP->isReversed = 1 ;
       cord = intP->X1 ; intP->X1 = intP->X2 ; intP->X2 = cord ;       
       cord = intP->Y1 ; intP->Y1 = intP->Y2 ; intP->Y2 = cord ;       
       cord = intP->Z1 ; intP->Z1 = intP->Z2 ; intP->Z2 = cord ;       
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STRING_INTERSECT_TABLE),bcdtmString_intersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit 
*/
 errexit :
 *numIntTableP = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
