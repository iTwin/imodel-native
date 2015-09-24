/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmConnect.cpp $
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
BENTLEYDTM_EXPORT int bcdtmConnect_reportValidationErrorsInConnectStrings
(
 DTM_POINT_ARRAY  **lineStringsPP,              /* ==> Pointer To Line Strings            */ 
 DTM_CONNECT_INPUT_STRING_ERROR *stringErrorsP, /* ==> Pointer To Connect String Errors   */
 long    numStringErrors,                       /* ==> Number Of Connect String Errors    */
 WCharCP reportFileP                           /* ==> Point To Report File Name          */
)
{
 int  ret=DTM_SUCCESS ;
 __time32_t localTime ;
 DPoint3d  *p3d1P,*p3d2P ;
 char dateStrP[100] ;
 FILE *errorFP=NULL ;
 DTM_CONNECT_INPUT_STRING_ERROR *strErrP ;
/*
** Only Report For Valid Arguments
*/
 if( stringErrorsP != NULL && numStringErrors > 0 && wcslen(reportFileP) > 0 )
   {
/*
**  Open Report File
*/
    errorFP = bcdtmFile_open(reportFileP,L"w") ;
    if( errorFP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Cannot Open Report File") ;
       goto errexit ; 
      }  
/*
**  Get Date
*/
    _time32(&localTime) ;
    strcpy(dateStrP,_ctime32(&localTime)) ;
    fprintf(errorFP,"%s \n\n",dateStrP) ;
/*
**  Write Errors
*/
    if( numStringErrors > 0 )
      {
       fprintf(errorFP,"Number Of Connect Strings Validation Errors = %6ld\n",numStringErrors) ;  
       for( strErrP = stringErrorsP ; strErrP < stringErrorsP + numStringErrors ; ++strErrP )
         {
          switch( strErrP->errorType )
            {
             case 1  :  // Zero Length String
             fprintf(errorFP,"\nError[%6ld] ** Zero Length String - String Offset = %6ld\n",((long)(strErrP-stringErrorsP)+1),strErrP->string1Offset) ;
             break   ;
          
             case 2  :  // Closed  String
             fprintf(errorFP,"\nError[%6ld] ** Closed String - String Offset = %6ld ** Location = %15.5lf %15.5lf\n",((long)(strErrP-stringErrorsP)+1),strErrP->string1Offset,strErrP->x,strErrP->y) ;
             break   ;

             case 3  :  // Knotted  String
             fprintf(errorFP,"\nError[%6ld] ** Knot In String - String Offset = %6ld Segment Offset = %6ld ** Location = %15.5lf %15.5lf\n",((long)(strErrP-stringErrorsP)+1),strErrP->string1Offset,strErrP->segment1Offset,strErrP->x,strErrP->y) ;
             break   ;

             case 4  :  // Intersected  String
             fprintf(errorFP,"\nError[%6ld] ** Intersected String - String 1 Offset = %6ld Segment 1 Offset = %6ld ** Location = %15.5lf %15.5lf\n",((long)(strErrP-stringErrorsP)+1),strErrP->string1Offset,strErrP->segment1Offset,strErrP->x,strErrP->y) ;
             fprintf(errorFP,"                                      String 2 Offset = %6ld Segment 2 Offset = %6ld\n",strErrP->string2Offset,strErrP->segment2Offset) ;
             p3d1P = (*(lineStringsPP+strErrP->string1Offset))->pointsP+strErrP->segment1Offset ;
             p3d2P = (*(lineStringsPP+strErrP->string2Offset))->pointsP+strErrP->segment2Offset ;
             fprintf(errorFP,"String 1 Segment = %15.5lf %15.5lf ** %15.5lf %15.5lf\n",p3d1P->x,p3d1P->y,(p3d1P+1)->x,(p3d1P+1)->y) ;
             fprintf(errorFP,"String 2 Segment = %15.5lf %15.5lf ** %15.5lf %15.5lf\n",p3d2P->x,p3d2P->y,(p3d2P+1)->x,(p3d2P+1)->y) ;
             break   ;
          
             case 5  :  // More Than Two Coincident String End Points
             fprintf(errorFP,"\nError[%6ld] ** More Than Two Concident String End Points ** Location = %15.5lf %15.5lf\n",((long)(strErrP-stringErrorsP)+1),strErrP->x,strErrP->y) ;
             break   ;

             default :
             break   ;
          
            } ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( errorFP != NULL ) { fclose(errorFP) ; errorFP = NULL ; }
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
BENTLEYDTM_Private int bcdtmConnect_releaseConnectPointsMemory
(
 DTM_CONNECT_POINT  **connectPtsPP,           /* Pointer To Connection Line Table      */
 long               numConnectPts             /* Number Of Lines                       */  
)
{
 DTM_CONNECT_POINT   *conPntP ;
 DTM_CONNECTION_LINE *linePntP ;
/*
** Check For Valid Data
*/
 if( numConnectPts > 0 && *connectPtsPP != NULL )
   {
/*
**  Scan Arrays And Free memory
*/
    for( conPntP = *connectPtsPP ; conPntP < *connectPtsPP + numConnectPts ; ++conPntP )
      {
       if( conPntP->conLineP != NULL )
         { 
          for( linePntP = conPntP->conLineP ; linePntP < conPntP->conLineP + conPntP->numConLine ; ++linePntP )
            {
             if( linePntP->intConLineP != NULL ) free(linePntP->intConLineP) ;
            } 
          free(conPntP->conLineP) ;
         }
      } 
    free(*connectPtsPP) ;
    *connectPtsPP = NULL ;
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
BENTLEYDTM_EXPORT int bcdtmConnect_strings
(
 DTM_POINT_ARRAY **lineStringsPP,                 /* ==> Pointer To Array Of Line Strings       */
 long    numLineStrings,                          /* ==> Number Of Line Strings                 */
 DTM_CONNECT_INPUT_STRING_ERROR **stringErrorsPP, /* <== Pointer To Line String Intersections   */
 long    *numStringErrorsP,                       /* <== Number Of Intersect Points             */
 DTM_CONNECTED_STRING **connectedStringPP,        /* <== Pointer To Sring Connection Offsets    */
 long    *numConnectedStringsP,                   /* <== Number Of String Connection Offsets    */         
 int     *connectedStringResultP,                 /* <== Connected String Result                */
 int     *connectedStringCloseP,                  /* <== Indicates If Connected String Closes   */
 double  *connectedStringLengthP                  /* <== Length Of Connected String             */
)
/*
** This Function Connects Line Strings 
**
** Notes 
**
** 1. *connectedStringResultP    ==  0   Lines Connected
**                               ==  1   Failed To Connect Lines
**
** 2. If Knots Or Intersecting Strings Are Detected Then Examine "intersectPointsPP" to get points of intersection. 
**    If string1Offset and string2Offset are the same value then it is a knot.
**
** 3. *connectedStringCloseP indicates that the connected line can be closed. 
**    The returned connected line is not closed. 
**    It is the responsibility of calling function to close the connected line
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   startTime,numConnectLines=0,numConnectPoints=0,numLineConnections=0,numNotConnected=0 ; 
 long   useTinMethod=TRUE,reportErrors=FALSE ;
 DTM_CONNECT_LINE  *connectLinesP=NULL ;
 DTM_CONNECT_POINT *conPntP,*connectPointsP=NULL ; 
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Connecting Line Strings") ;
    bcdtmWrite_message(0,0,0,"lineStringsPP   = %p",lineStringsPP) ;
    bcdtmWrite_message(0,0,0,"numLineStrings  = %8ld",numLineStrings) ;
   }
/*
** Initialise
*/
 startTime = bcdtmClock() ;
 *numConnectedStringsP   = 0   ;
 *connectedStringResultP = 0   ;
 *connectedStringCloseP  = 0   ;
 *connectedStringLengthP = 0.0 ;
 *numStringErrorsP       = 0   ; 
 if( *stringErrorsPP != NULL ) { free(*stringErrorsPP) ; *stringErrorsPP = NULL ; }
/*
** Validate Arguments
*/
 if( lineStringsPP == NULL || numLineStrings < 1 )
   { 
    bcdtmWrite_message(2,0,0,"No Line Strings") ;
    goto errexit ;
   }
/*
** Validate Line Strings
*/
 if( dbg )   bcdtmWrite_message(0,0,0,"Validating Line Strings") ;
 if( bcdtmConnect_validateConnectLineStrings(lineStringsPP,numLineStrings,stringErrorsPP,numStringErrorsP)) goto errexit ;
 if( *numStringErrorsP > 0 ) *connectedStringResultP = 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"*numStringErrorsP = %2ld",*numStringErrorsP) ;
/*
** Check For One line String
*/
 if( numLineStrings == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Single String") ;
/*
**  Remove Closing String Error
*/
    if( *numStringErrorsP == 1 && (*stringErrorsPP)->errorType == 2 )
      {
       *numStringErrorsP = 0  ;
       free(*stringErrorsPP)  ;
       *stringErrorsPP = NULL ; 
       *connectedStringResultP = 0 ;
      }
/*
**  Calculate String Length And Test For Closure
*/
    if( *numStringErrorsP == 0 ) 
      {
       if( bcdtmString_getStringLength2D((*lineStringsPP)->pointsP,(*lineStringsPP)->numPoints,connectedStringLengthP)) goto errexit ;
       if( bcdtmString_testIfStringCanBeClosed2D((*lineStringsPP)->pointsP,(*lineStringsPP)->numPoints,connectedStringCloseP)) goto errexit ;
       *numConnectedStringsP = 1 ;
       *connectedStringPP = ( DTM_CONNECTED_STRING * ) malloc ( *numConnectedStringsP * sizeof( DTM_CONNECTED_STRING )) ;
       if( *connectedStringPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       (*connectedStringPP)->stringOffset = 0 ;
       (*connectedStringPP)->isReversed   = 0 ;
       goto cleanup ;
      }
   }
/*
**  Report Errors
*/
 if( reportErrors == TRUE && *numStringErrorsP > 0 )
   {
    if( bcdtmConnect_reportValidationErrorsInConnectStrings(lineStringsPP,*stringErrorsPP,*numStringErrorsP,L"connectErrors.txt")) goto errexit ;
   }
/*
** Continue If No Validation Errors Detected
*/
 if( ! *numStringErrorsP ) 
   {
/*
**  Use Tin Method To Determine Connection Lines
*/
    if( useTinMethod == TRUE )
      { 
/*
**     Build Connect Tin
*/
       if( dbg )   bcdtmWrite_message(0,0,0,"Building Connect Tin") ;
       if( bcdtmConnect_buildConnectDtmObject(&dtmP,lineStringsPP,numLineStrings)) goto errexit ;
       if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"connect.tin") ;
/*
**     Build Connect Tables
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tables From Tin") ;
       if( bcdtmConnect_buildConnectTablesFromTinLinesDtmObject(dtmP,lineStringsPP,numLineStrings,&connectLinesP,&numConnectLines,&connectPointsP,&numConnectPoints)) goto errexit ;
      }
/*
**  Use String Intersection Method To Determine Connection Lines
*/
    else
      {
 /*
**     Build Connect Tables
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tables From Strings") ;
       if( bcdtmConnect_buildConnectLineTablesFromStrings(lineStringsPP,numLineStrings,&connectLinesP,&numConnectLines,&connectPointsP,&numConnectPoints)) goto errexit ;
      } 
/*
**  Count Number Of Line Connections
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Counting Line Connections") ;
       numNotConnected    = 0 ;
       numLineConnections = 0 ;
       for( conPntP = connectPointsP ; conPntP < connectPointsP + numConnectPoints ; ++conPntP )
         {
          numLineConnections =  numLineConnections + conPntP->numConLine ;
          bcdtmWrite_message(0,0,0,"Point[%6ld] ** Number Of Connect Points = %6ld",(long)(conPntP-connectPointsP),conPntP->numConLine) ;
         if( conPntP->numConLine == 0 ) ++numNotConnected ;
        }
       bcdtmWrite_message(0,0,0,"Number Of Connect Lines      = %6ld",numConnectLines) ;
       bcdtmWrite_message(0,0,0,"Number Of Unconnected Points = %6ld",numNotConnected) ;
      }
/*
**  Connect Line Strings
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Connecting Strings") ; 
    if( bcdtmConnect_lineStringsUsingConnectionTables(connectLinesP,numConnectLines,connectPointsP,numConnectPoints,connectedStringPP,numConnectedStringsP,connectedStringResultP,connectedStringCloseP,connectedStringLengthP) ) goto errexit ;
/*
**  Check If Connected Line String Can Close
*/
    if( useTinMethod == TRUE && ! *connectedStringResultP && ! *connectedStringCloseP )
      {
       if( bcdtmConnect_testForConnectedStringClosureDtmObject(dtmP,lineStringsPP,*connectedStringPP,*numConnectedStringsP,connectedStringCloseP)) goto errexit ; 
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( connectLinesP  != NULL  ) { free(connectLinesP)  ; connectLinesP  = NULL ; }
 if( connectPointsP != NULL  )  bcdtmConnect_releaseConnectPointsMemory(&connectPointsP,numConnectPoints) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Connect Strings = %8.4lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Line Strings Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Line Strings Error") ;
 return( ret) ;
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
BENTLEYDTM_Private int bcdtmConnect_testForConnectedStringClosureDtmObject
(
 BC_DTM_OBJ               *dtmP,
 DTM_POINT_ARRAY      **lineStringsPP,
 DTM_CONNECTED_STRING *connectedStringP,
 long                 numConnectedStrings,
 int                  *stringCloseP
 )
{ 
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,closePnt1,closePnt2,numBreaks ;
 double closeLineX1,closeLineY1,closeLineX2,closeLineY2,conLineX1,conLineY1,conLineX2,conLineY2 ;
 DTM_CONNECTED_STRING *conStrP ;
/*
**  Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Connected String Closure Completed") ;
/*
** Initialise
*/
 *stringCloseP = 0 ;
/*
**  Get The Coordinates For The Closing Connection line
*/
 ofs = connectedStringP->stringOffset ;
 if( connectedStringP->isReversed == 0 )
   { 
    closeLineX1 = ((*(lineStringsPP+ofs))->pointsP)->x ;
    closeLineY1 = ((*(lineStringsPP+ofs))->pointsP)->y ;
   }
 else
   {
    closeLineX1 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->x ;
    closeLineY1 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->y ;
   } 
/*
**  Get The Last Point Of The Connected String
*/
 ofs = (connectedStringP+numConnectedStrings-1)->stringOffset ;
 if( (connectedStringP+numConnectedStrings-1)->isReversed == 0 )
   {
    closeLineX2 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->x ;
    closeLineY2 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->y ;
   }
 else 
   { 
    closeLineX2 = ((*(lineStringsPP+ofs))->pointsP)->x ;
    closeLineY2 = ((*(lineStringsPP+ofs))->pointsP)->y ;
   }
/*
**  Get Tin Point Numbers For Closing Connection Line
*/
 bcdtmFind_closestPointDtmObject(dtmP,closeLineX1,closeLineY1,&closePnt1) ;
 bcdtmFind_closestPointDtmObject(dtmP,closeLineX2,closeLineY2,&closePnt2) ;
/*
** Get Number Of Break Points
*/
 if( bcdtmConnect_getNumberOfDrapeBreakBetweenPointsDtmObject(dtmP,closePnt1,closePnt2,&numBreaks)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numBreaks = %2ld",numBreaks) ;
/*
** Test For Intersection With Connection Lines
*/
 if( numBreaks == 2 ) 
   {
    *stringCloseP = 1 ;
/*
**  Scan Connection Lines
*/
    for( conStrP = connectedStringP ; conStrP < connectedStringP + numConnectedStrings - 1 && *stringCloseP ; ++conStrP )
      {
/*
**     Get Start Coordinates For Connection Line
*/
       ofs = conStrP->stringOffset ;
       if( conStrP->isReversed == 0 )
         { 
          conLineX1 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->x ;
          conLineY1 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->y ;
         }
       else
         {
          conLineX1 = ((*(lineStringsPP+ofs))->pointsP)->x ;
          conLineY1 = ((*(lineStringsPP+ofs))->pointsP)->y ;
         } 
/*
**     Get Start Coordinates For Connection Line
*/
       ofs = (conStrP+1)->stringOffset ;
       if( (conStrP+1)->isReversed == 0 )
         { 
          conLineX2 = ((*(lineStringsPP+ofs))->pointsP)->x ;
          conLineY2 = ((*(lineStringsPP+ofs))->pointsP)->y ;
         }
       else
         {
          conLineX2 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->x ;
          conLineY2 = ((*(lineStringsPP+ofs))->pointsP+(*(lineStringsPP+ofs))->numPoints-1)->y ;
         } 
/*
**     Test For Intesection With Closing Connection
*/
       if( bcdtmMath_checkIfLinesIntersect(closeLineX1,closeLineY1,closeLineX2,closeLineY2,conLineX1,conLineY1,conLineX2,conLineY2))
         {
          *stringCloseP = 0 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Closing Connection Intesects Connection Line ** %12.5lf %12.5lf ** %12.5lf %12.5lf %12.5lf",conLineX1,conLineY1,conLineX2,conLineY2) ;
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Connected String Closure Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Connected String Closure Error") ;
 return( ret) ;
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
BENTLEYDTM_EXPORT int bcdtmConnect_validateConnectLineStrings
(
 DTM_POINT_ARRAY **lineStringsPP,                 /* ==> Pointer To Array Of Line Strings       */
 long    numLineStrings,                          /* ==> Number Of Line Strings                 */
 DTM_CONNECT_INPUT_STRING_ERROR **stringErrorsPP, /* <== Pointer To String Errors               */
 long    *numStringErrorsP                        /* <== Number Of String Errors                */
)
/*
** This Function Validates Line Strings Prior To Processing By The Connection Function
**
** Error Codes                 ==  1   Zero Length Line String
**                             ==  2   Closed Line String 
**                             ==  3   Knot(s) Detected In String
**                             ==  4   Intersecting String
**                             ==  5   More Than Two Coincident Line String End Points
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   closeFlag=0,intersectType=1,numIntPts=0,numEndPts=0,memErrors=0,memErrorsInc=1000 /*,numDecPlaces=5*/ ;
 double length=0,pptol=0.00001 ;
 DPoint3d    *p3d1P,*p3d2P,*endPtsP=NULL ;
 DTM_POINT_ARRAY **pointArrayPP ;
 DTM_INTERSECT_POINT *intPntP,*intPnt1P,*intPnt2P,*intPtsP=NULL ;

/*
** Write Entry Message
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Validating Line Strings") ;
/*
** Initialise
*/
 *numStringErrorsP = 0     ;
 if( *stringErrorsPP != NULL ) { free(*stringErrorsPP) ; *stringErrorsPP = NULL ; }
/*
**  Round Coordinates - Leave To powerCivil
*/
// if( dbg ) bcdtmWrite_message(0,0,0,"Rounding Line String Coordiantes") ;
// if( bcdtmString_roundArrayOfLineStrings3D(lineStringsPP,numLineStrings,numDecPlaces)) goto errexit ;
   if( bcdtmString_makeCloseEndPointsOfArrayOfLineStringsCoincident2D(lineStringsPP,numLineStrings,pptol)) goto errexit ;
/*
**  Remove Duplicate XY Points From Line Strings 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate XY Points From Line Strings") ;
 for( pointArrayPP = lineStringsPP ; pointArrayPP < lineStringsPP + numLineStrings ; ++pointArrayPP )
   {
    if( bcdtmString_removeDuplicatePoints2D((*pointArrayPP)->pointsP,&(*pointArrayPP)->numPoints,0.0)) goto errexit ;
   } 
/*
**  Check For Zero Length Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Zero Length Strings") ;
 for( pointArrayPP = lineStringsPP ; pointArrayPP < lineStringsPP + numLineStrings ; ++pointArrayPP )
   {
    if( bcdtmString_getStringLength2D((*pointArrayPP)->pointsP,(*pointArrayPP)->numPoints,&length)) goto errexit ;
    if( length == 0.0 ) 
      {  
/*
**     Check Memory
*/
       if( *numStringErrorsP == memErrors )
         {
          memErrors = memErrors + memErrorsInc ;
          if( *stringErrorsPP == NULL ) *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) malloc ( memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          else                          *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) realloc ( *stringErrorsPP , memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          if( *stringErrorsPP == NULL )
            {
             bcdtmWrite_message(2,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Error
*/
       (*stringErrorsPP+*numStringErrorsP)->errorType = 1 ;
       (*stringErrorsPP+*numStringErrorsP)->string1Offset  = (long)(pointArrayPP-lineStringsPP) ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->string2Offset  = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->x = 0.0  ;
       (*stringErrorsPP+*numStringErrorsP)->y = 0.0  ;
       (*stringErrorsPP+*numStringErrorsP)->z = 0.0  ;
       ++*numStringErrorsP ;
      }
   } 
/*
**  Check For Closed Line Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Closed Line Strings") ;
 for( pointArrayPP = lineStringsPP ; pointArrayPP < lineStringsPP + numLineStrings ; ++pointArrayPP )
   {
    if( bcdtmString_testForClosedString2D((*pointArrayPP)->pointsP,(*pointArrayPP)->numPoints,&closeFlag)) goto errexit ;
    if( closeFlag ) 
      {  
/*
**     Check Memory
*/
       if( *numStringErrorsP == memErrors )
         {
          memErrors = memErrors + memErrorsInc ;
          if( *stringErrorsPP == NULL ) *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) malloc ( memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          else                          *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) realloc ( *stringErrorsPP , memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          if( *stringErrorsPP == NULL )
            {
             bcdtmWrite_message(2,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Error
*/
       (*stringErrorsPP+*numStringErrorsP)->errorType = 2 ;
       (*stringErrorsPP+*numStringErrorsP)->string1Offset  = (long)(pointArrayPP-lineStringsPP) ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->string2Offset  = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->x = (*pointArrayPP)->pointsP->x  ;
       (*stringErrorsPP+*numStringErrorsP)->y = (*pointArrayPP)->pointsP->y  ;
       (*stringErrorsPP+*numStringErrorsP)->z = (*pointArrayPP)->pointsP->y  ;
       ++*numStringErrorsP ;
      }
   } 
/*
** Check For Intersecting Line Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Line Strings") ;
 if( bcdtmString_detectStringIntersections2D(lineStringsPP,numLineStrings,intersectType,&intPtsP,&numIntPts) ) goto errexit ;
 if( numIntPts > 0 ) 
   {  
/*
**  Qsort Intersection Points
*/
    qsortCPP(intPtsP,numIntPts,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCoordinateCompareFunction) ;
/*
**  Write Intersection Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"intPnt[%6ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.4lf %12.4lf",(long)(intPntP-intPtsP),intPntP->string1Offset,intPntP->segment1Offset,intPntP->string2Offset,intPntP->segment2Offset,intPntP->x,intPntP->y) ;
         } 
      }
/*
**  Remove Duplicate Intersection Points
*/
    for( intPnt1P = intPtsP , intPnt2P = intPtsP + 1  ; intPnt2P < intPtsP + numIntPts ; ++intPnt2P )
      {
       if( intPnt2P->x != intPnt1P->x || intPnt2P->y != intPnt1P->y ) 
         {
          ++intPnt1P ;
          *intPnt1P = *intPnt2P ;
         } 
      }
    numIntPts = (long)(intPnt1P-intPtsP) + 1  ;
/*
**  Write Intersection Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"intPnt[%6ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.4lf %12.4lf",(long)(intPntP-intPtsP),intPntP->string1Offset,intPntP->segment1Offset,intPntP->string2Offset,intPntP->segment2Offset,intPntP->x,intPntP->y) ;
         } 
      }
/*
**  Store Intersection Points In String Errors
*/   
    for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
/*
**     Check Memory
*/
       if( *numStringErrorsP == memErrors )
         {
          memErrors = memErrors + memErrorsInc ;
          if( *stringErrorsPP == NULL ) *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) malloc ( memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          else                          *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) realloc ( *stringErrorsPP , memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          if( *stringErrorsPP == NULL )
            {
             bcdtmWrite_message(2,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Error
*/
       if( intPntP->string1Offset == intPntP->string2Offset ) (*stringErrorsPP+*numStringErrorsP)->errorType = 3 ;
       else                                                   (*stringErrorsPP+*numStringErrorsP)->errorType = 4 ; 
       (*stringErrorsPP+*numStringErrorsP)->string1Offset  = intPntP->string1Offset  ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = intPntP->segment1Offset ;
       (*stringErrorsPP+*numStringErrorsP)->string2Offset  = intPntP->string2Offset  ;
       (*stringErrorsPP+*numStringErrorsP)->segment2Offset = intPntP->segment2Offset ;
       (*stringErrorsPP+*numStringErrorsP)->x = intPntP->x  ;
       (*stringErrorsPP+*numStringErrorsP)->y = intPntP->y  ;
       (*stringErrorsPP+*numStringErrorsP)->z = intPntP->zSegment1  ;
       ++*numStringErrorsP ;
      }
   }
/*
** Check For More Than Two Coincident String End Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For More Than Two Coincident String End Points") ;
/*
** Allocate Memory For End Points
*/
 endPtsP = ( DPoint3d * ) malloc(numLineStrings * 2 * sizeof(DPoint3d)) ;
 if( endPtsP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Polulate End Points
*/
 numEndPts = 0 ;
 for( pointArrayPP = lineStringsPP ; pointArrayPP < lineStringsPP + numLineStrings ; ++pointArrayPP )
   {
    (endPtsP+numEndPts)->x = (*pointArrayPP)->pointsP->x ;
    (endPtsP+numEndPts)->y = (*pointArrayPP)->pointsP->y ;
    (endPtsP+numEndPts)->z = (*pointArrayPP)->pointsP->z ;
    ++numEndPts ; 
    (endPtsP+numEndPts)->x = ((*pointArrayPP)->pointsP+(*pointArrayPP)->numPoints-1)->x ;
    (endPtsP+numEndPts)->y = ((*pointArrayPP)->pointsP+(*pointArrayPP)->numPoints-1)->y ;
    (endPtsP+numEndPts)->z = ((*pointArrayPP)->pointsP+(*pointArrayPP)->numPoints-1)->z ;
    ++numEndPts ; 
   }
/*
** Qsort End Points
*/
 qsortCPP(endPtsP,numEndPts,sizeof(DPoint3d),bcdtmConnect_lineEndPointsCompareFunction) ;
/*
** Scan For More Than Two Coincident End Points
*/
 for( p3d1P = endPtsP ; p3d1P < endPtsP + numEndPts ; ++p3d1P )
   {
    p3d2P = p3d1P + 1 ;
    while( p3d2P < endPtsP + numEndPts && p3d2P->x == p3d1P->x && p3d2P->y == p3d1P->y ) ++p3d2P ;
    --p3d2P ;
/*
**  Check For More Than Two Coincident Points
*/
    if( (long)(p3d2P-p3d1P)+1 > 2 )
      {
/*
**     Check Memory
*/
       if( *numStringErrorsP == memErrors )
         {
          memErrors = memErrors + memErrorsInc ;
          if( *stringErrorsPP == NULL ) *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) malloc ( memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          else                          *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) realloc ( *stringErrorsPP , memErrors * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
          if( *stringErrorsPP == NULL )
            {
             bcdtmWrite_message(2,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Error
*/
       (*stringErrorsPP+*numStringErrorsP)->errorType = 5 ;
       (*stringErrorsPP+*numStringErrorsP)->string1Offset  = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->string2Offset  = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->segment1Offset = -1 ;
       (*stringErrorsPP+*numStringErrorsP)->x = p3d1P->x  ;
       (*stringErrorsPP+*numStringErrorsP)->y = p3d1P->y  ;
       (*stringErrorsPP+*numStringErrorsP)->z = p3d1P->z  ;
       ++*numStringErrorsP ;
/*
**     Reset Pointer
*/
       p3d1P = p3d2P  ;
      }
   }
/*
** Reallocate Memory
*/
 if( *numStringErrorsP > 0 && *numStringErrorsP < memErrors ) *stringErrorsPP = (DTM_CONNECT_INPUT_STRING_ERROR * ) realloc ( *stringErrorsPP , *numStringErrorsP * sizeof(DTM_CONNECT_INPUT_STRING_ERROR)) ;
/*
** Cleanup
*/
 cleanup :
 if( endPtsP != NULL ) free(endPtsP) ; 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Line Strings Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Line Strings Error") ;
 return( ret) ;
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
BENTLEYDTM_Private int bcdtmConnect_lineEndPointsCompareFunction(const DPoint3d *p3d1P,const DPoint3d *p3d2P)
/*
** Compare Function For Qsort Of Increasing Coordinate Value
*/
{
 if      (  p3d1P->x   <  p3d2P->x  ) return( -1) ;
 else if (  p3d1P->x   >  p3d2P->x  ) return(  1) ;
 if      (  p3d1P->y   <  p3d2P->y  ) return( -1) ;
 else if (  p3d1P->y   >  p3d2P->y  ) return(  1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmConnect_dtmFeatureTypeOccurrencesInDtmObject
(
 BC_DTM_OBJ *dataP,                  /* ==> Pointer To Data Object     */
 DTMFeatureType dtmFeatureType,                /* ==> Dtm Feature Type           */
 DTM_CONNECT_LINE  **connectLinesPP,  /* <== Pointer To Connect Lines   */
 long  *numConnectLinesP,             /* <== Number Of Connect Lines    */
 DTM_CONNECT_POINT **connectPointsPP, /* <== Pointer To Connect Points  */
 long  *numConnectPointsP,            /* <== Number Of Connect Points   */
 long  *numLineConnectionsP           /* <== Number Of Line Connection  */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d     *p3dP ;
 long    numFeatureArray=0,numNotConnected=0 ;
 BC_DTM_OBJ  *dtmP=NULL ;
 DTM_POINT_ARRAY **pointArrayPP,**featureArrayPP=NULL ;
 DTM_CONNECT_POINT *conPntP ;
 static long seqConnect=0 ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    ++seqConnect ;
    bcdtmWrite_message(0,0,0,"Connecting Feature Strings ** Sequence %4ld",seqConnect) ;
    bcdtmWrite_message(0,0,0,"dataP               = %p",dataP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType      = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"connectLinesPP      = %p",*connectLinesPP) ;
    bcdtmWrite_message(0,0,0,"numConnectLinesP    = %8ld",*numConnectLinesP) ;
    bcdtmWrite_message(0,0,0,"connectPointsPP     = %p",*connectPointsPP) ;
    bcdtmWrite_message(0,0,0,"numConnectPointsP   = %8ld",*numConnectPointsP) ;
    bcdtmWrite_message(0,0,0,"numLineConnectionsP = %8ld",*numLineConnectionsP) ;
   }
/*
** Initialise
*/
 *numLineConnectionsP = 0 ;
/*
** Extract Dtm Feature Type Strings From Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Dtm Feature Type Strings From Data Object") ;
 if( bcdtmList_copyAllDtmFeatureTypePointsToPointArraysDtmObject(dataP,dtmFeatureType,&featureArrayPP,&numFeatureArray)) goto errexit ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Feature Type Strings Extracted From Data Object = %6ld",numFeatureArray) ;
    for( pointArrayPP = featureArrayPP ; pointArrayPP < featureArrayPP + numFeatureArray ; ++pointArrayPP )
      {
       bcdtmWrite_message(0,0,0,"Feature[%6ld] ** pointsP = %p numPoints = %6ld",(long)(pointArrayPP-featureArrayPP),(*pointArrayPP)->pointsP,(*pointArrayPP)->numPoints) ;
       for( p3dP = (*pointArrayPP)->pointsP ; p3dP < (*pointArrayPP)->pointsP + (*pointArrayPP)->numPoints ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"**** Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-(*pointArrayPP)->pointsP),p3dP->x,p3dP->y,p3dP->y) ;
         } 
      }
   }
/*
** Build Connect Tin From Features
*/  
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tin") ;
 if( bcdtmConnect_buildConnectDtmObject(&dtmP,featureArrayPP,numFeatureArray)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"connect.tin") ;
/*
** Build Connect Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tables") ;
 if( bcdtmConnect_buildConnectTablesFromTinLinesDtmObject(dtmP,featureArrayPP,numFeatureArray,connectLinesPP,numConnectLinesP,connectPointsPP,numConnectPointsP)) goto errexit ;
/*
** Count Number Of Line Connections
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Line Connections") ;
 numNotConnected = 0 ;
 *numLineConnectionsP = 0 ;
 for( conPntP = *connectPointsPP ; conPntP < *connectPointsPP + *numConnectPointsP ; ++conPntP )
   {
    *numLineConnectionsP =  *numLineConnectionsP + conPntP->numConLine ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Point[%6ld] ** Number Of Connect Points = %6ld",(long)(conPntP-*connectPointsPP),conPntP->numConLine) ;
    if( conPntP->numConLine == 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"No Connections To Point = %6ld",(long)(conPntP-*connectPointsPP)) ;
       ++numNotConnected ;
      }
   }
/*
** Write Statistics
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numConnectLines              = %6ld",*numConnectLinesP) ; 
    bcdtmWrite_message(0,0,0,"numConnectPoints             = %6ld",*numConnectPointsP) ;
    bcdtmWrite_message(0,0,0,"Number Of Line Connections   = %6ld",*numLineConnectionsP) ;
    bcdtmWrite_message(0,0,0,"Number Of Unconnected Points = %6ld",numNotConnected) ;
   } 
/*
** Cleanup
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( featureArrayPP != NULL )  bcdtmMem_freePointerArrayToPointArrayMemory(&featureArrayPP,numFeatureArray) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Feature Strings Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Feature Strings Error") ;
 return( ret) ;
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
BENTLEYDTM_Public int bcdtmConnect_buildConnectDtmObject
(
 BC_DTM_OBJ **dtmPP,                 /* <== Pointer To Tin Object                        */
 DTM_POINT_ARRAY **pointArrayPP,     /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long  numPointArray                 /* ==> Number Of Point Arrays                       */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    lineNum=0,numInc=10 ;
 double  dx,dy,extInc,xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d     randomSpot[4] ;
 DTM_POINT_ARRAY **pntArrayPP ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 

/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tin") ;
/*
** Get Bounding Cube For Point Arrays
*/
 if( bcdtmString_getBoundingCubeForArrayOfStrings(pointArrayPP,numPointArray,&xMin,&yMin,&zMin,&xMax,&yMax,&zMax)) goto errexit ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Populate Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Data Object With Connect Strings") ;
 for( lineNum = 0 , pntArrayPP = pointArrayPP ; pntArrayPP < pointArrayPP + numPointArray ; ++ lineNum , ++pntArrayPP )
   {
    if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::Breakline,lineNum,1,&nullFeatureId,(*pntArrayPP)->pointsP,(*pntArrayPP)->numPoints)) goto errexit ;
   }
/*
** Extend Bounding Rectangle
*/
 dx = xMax - xMin ;
 dy = yMax - yMin ;
 if( dx >= dy ) extInc = dx / (double)(numInc ) ;
 else           extInc = dy / (double)(numInc ) ;         
 xMin -= extInc ;
 xMax += extInc ;
 yMin -= extInc ;
 yMax += extInc ;
/*
** Store Bounding Rectangle In Tin
*/
 randomSpot[0].x = xMin ; randomSpot[0].y = yMin ; randomSpot[0].z = 0.0 ;
 randomSpot[1].x = xMax ; randomSpot[1].y = yMin ; randomSpot[1].z = 0.0 ;
 randomSpot[2].x = xMax ; randomSpot[2].y = yMax ; randomSpot[2].z = 0.0 ;
 randomSpot[3].x = xMin ; randomSpot[3].y = yMax ; randomSpot[3].z = 0.0 ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,randomSpot,4)) goto errexit ;
/*
** Write Data Object
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(*dtmPP,L"connectTin.dat") ;
/*
** Triangulate Dtm Object
*/
 if( bcdtmObject_createTinDtmObject(*dtmPP,1,0.0, false)) goto errexit ;
/*
** Cleanup
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(*dtmPP) ;
 bcdtmList_nullSptrValuesDtmObject(*dtmPP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Connect Tin Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Building Connect Tin Error") ;
 return( ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP)  ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmConnect_buildConnectTablesFromTinLinesDtmObject
(
 BC_DTM_OBJ *dtmP,                    /* ==> Pointer To Tin Object                        */
 DTM_POINT_ARRAY **lineStringsPP,      /* ==> Pointer To Pointer Array Of Dtm Point Arrays */
 long  numLineStrings,                 /* ==> Number Of Point Arrays                       */
 DTM_CONNECT_LINE **connectLinesPP,    /* <== Pointer To Connect Lines                     */
 long  *numConnectLinesP,              /* <== Number Of Connect Lines                      */
 DTM_CONNECT_POINT **connectPointsPP,  /* <== Pointer To Connect Points                    */
 long  *numConnectPointsP              /* <== Number Of Connect Points                     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1,pnt2,pnt3,line,cPtr,node,lineNum=0,pntNum,intConnections=FALSE,numMarked=0 ;
 long   numConLines=0,memConLines=0,memConLinesInc=100,numPointFeatures=0,numConnectionLines=0,ofsIntConLine ;
 long   firstPnt,lastPnt,numBreaks,pointOneFeature,numIntPts,intersectType=1,numConOfs,numIntConLines,numPntConLines=0 ;
 double dx,dy,lineLength=0.0 ;
 DPoint3d    *p3dP,*conLinePtsP=NULL ;
 
 DTM_POINT_ARRAY        *pntArrayP=NULL,**pntArrayPP,**connectionLinesPP=NULL ;
 DTM_CONNECT_LINE       *lineP ;
 DTM_CONNECT_POINT      *pointP ;
 DTM_CONNECTION_LINE    *conLineP,*conLine1P,*conLine2P,*conLinesP=NULL,*pntConLinesP=NULL ;
 DTM_INTERSECT_POINT    *intP,*int1P,*int2P,*intPtsP=NULL ;
 DTM_CONNECTION_LINE_INTERSECT *intConLineP,*intConLinesP=NULL ;
 DTM_TIN_POINT_FEATURES *pointFeaturesP=NULL ;
 struct Connect_Offsets { long pnt1,pnt2 ; } *conOffsetsP=NULL ;
 DTM_TIN_NODE  *nodesP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tables") ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"connect.tin") ;
/*
** Allocate Memory For Connect Lines Table
*/
 *numConnectLinesP = numLineStrings ;
 *connectLinesPP = ( DTM_CONNECT_LINE * ) malloc( *numConnectLinesP * sizeof(DTM_CONNECT_LINE)) ;
 if( *connectLinesPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Allocate Memory For Connect Points Table
*/
 *numConnectPointsP = numLineStrings * 2 ;
 *connectPointsPP = ( DTM_CONNECT_POINT * ) malloc( *numConnectPointsP * sizeof(DTM_CONNECT_POINT)) ;
 if( *connectPointsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Populate Connect Lines And Connect Points Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Connect Lines And Connect Points Table") ;
 for( lineNum = 0 , pntArrayPP = lineStringsPP ; pntArrayPP < lineStringsPP + numLineStrings ; ++lineNum , ++pntArrayPP )
   {
/*
**  Get Tin Point Numbers For Line End Points
*/
    p3dP = (*pntArrayPP)->pointsP ;
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&pnt1) ;
    p3dP = (*pntArrayPP)->pointsP + (*pntArrayPP)->numPoints - 1;
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&pnt2) ;
/*
**  Calulate Line Length
*/
    lineLength = 0 ;
    for( p3dP = (*pntArrayPP)->pointsP + 1 ; p3dP <  (*pntArrayPP)->pointsP + (*pntArrayPP)->numPoints ; ++p3dP )
      {
       dx = p3dP->x - (p3dP-1)->x ; 
       dy = p3dP->y - (p3dP-1)->y ; 
       lineLength = lineLength + sqrt( dx*dx + dy*dy) ;
      }   
/*
**  Populate Connect Line Table
*/
    (*connectLinesPP+lineNum)->line      = lineNum ;
    (*connectLinesPP+lineNum)->point1    = pnt1 ;
    (*connectLinesPP+lineNum)->point2    = pnt2 ;
    (*connectLinesPP+lineNum)->isReversed = 0 ;
    (*connectLinesPP+lineNum)->length    = lineLength ;
/*
**  Populate Connect Points Table
*/
    (*connectPointsPP+lineNum*2)->line   = lineNum ;
    (*connectPointsPP+lineNum*2)->point1 = pnt1 ;
    (*connectPointsPP+lineNum*2)->point2 = pnt2 ;
    (*connectPointsPP+lineNum*2)->isReversed = 0 ;
    (*connectPointsPP+lineNum*2)->numConLine = 0 ;
    (*connectPointsPP+lineNum*2)->conLineP = NULL ;

    (*connectPointsPP+lineNum*2+1)->line   = lineNum ;
    (*connectPointsPP+lineNum*2+1)->point1 = pnt2 ;
    (*connectPointsPP+lineNum*2+1)->point2 = pnt1 ;
    (*connectPointsPP+lineNum*2+1)->isReversed = 1 ;
    (*connectPointsPP+lineNum*2+1)->numConLine = 0 ;
    (*connectPointsPP+lineNum*2+1)->conLineP = NULL ;
   }
/*
** Mark Tin Points Corresponding To Connect Points
*/
 for( pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pointP )
   {
    pnt1 = pointP->point1 ;
/*
**  Test For Coincident Connect Points
*/
    if      ( nodeAddrP(dtmP,pnt1)->sPtr == dtmP->nullPnt ) nodeAddrP(dtmP,pnt1)->sPtr = (long)(pointP-*connectPointsPP) ;
    else if ( nodeAddrP(dtmP,pnt1)->tPtr == dtmP->nullPnt ) nodeAddrP(dtmP,pnt1)->tPtr = (long)(pointP-*connectPointsPP) ;
   }
/*
** Write Out Marked Tin Points
*/
 if( dbg )
   {
    numMarked = 0 ;
    bcdtmWrite_message(0,0,0,"Marked Tin Points") ;
    for( node = 0 ; node < dtmP->numPoints ; ++node )
      {
       nodesP = nodeAddrP(dtmP,node) ;
       if( nodesP->sPtr != dtmP->nullPnt || nodesP->tPtr != dtmP->nullPnt )
         {
          pnt1 = node ;
          bcdtmWrite_message(0,0,0,"dtmPoint[%6ld] ** sPtr = %9ld tPtr = %9ld ** %12.5lf %12.5lf",pnt1,nodesP->sPtr,nodesP->tPtr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
          ++numMarked ;
         }
      }
    bcdtmWrite_message(0,0,0,"Number Of Marked Tin Points = %6ld",numMarked) ;
   }
/*
** Scan Tin And Store Zero Length Connection Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Zero Length Connection Lines") ;
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodesP = nodeAddrP(dtmP,node) ;
    if( nodesP->sPtr != dtmP->nullPnt && nodesP->tPtr != dtmP->nullPnt )
      {
       pnt1 = node ;
       if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,pnt1,pnt1,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
       nodesP->sPtr = nodesP->tPtr = dtmP->nullPnt ;
      }   
   }
/*
**  Write Number Of Connection Lines
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Zero Length Connection Lines = %6ld",numConLines) ;
    for( conLineP = conLinesP ; conLineP < conLinesP + numConLines ; ++conLineP)
      {
       bcdtmWrite_message(0,0,0,"Connection Line[%4ld] ** fromPoint = %6ld toPoint = %6ld line = %6ld distance = %10.4lf",(long)(conLineP-conLinesP),conLineP->fromPoint,conLineP->toPoint,conLineP->line,conLineP->distance) ;
      }
   } 
/*
** Scan Tin And Store None Zero Length Connection Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing None Zero Length Connection Lines") ;
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodesP = nodeAddrP(dtmP,node) ;
    if( nodesP->sPtr != dtmP->nullPnt )
      {
/*
**     Set Tin Point
*/
       pnt1 = node ;
/*
**     Get Tin Point For Other End Of Connect Line
*/
       line = nodesP->sPtr / 2 ;
       if( nodesP->sPtr % 2 == 1 )  bcdtmFind_closestPointDtmObject(dtmP,(*(lineStringsPP+line))->pointsP->x,(*(lineStringsPP+line))->pointsP->y,&pnt2) ;
       else                         bcdtmFind_closestPointDtmObject(dtmP,((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->x,((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->y,&pnt2) ; 
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"tin-pnt1 = %8ld ** %12.5lf %12.5lf ** Connect Point = %6ld",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,nodeAddrP(dtmP,pnt1)->sPtr) ; 
          bcdtmWrite_message(0,0,0,"tin-pnt2 = %8ld ** %12.5lf %12.5lf ** Connect Point = %6ld",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,nodeAddrP(dtmP,pnt2)->sPtr) ; 
         }
/*
**     Get Dtm Feature For Point One
*/ 
       if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,pnt1,&pointFeaturesP,&numPointFeatures) ) goto errexit ;
       pointOneFeature = pointFeaturesP->dtmFeature ;
       if( dbg ) bcdtmWrite_message(0,0,0,"pointOneFeature = %6ld",pointOneFeature) ;
/*
**     Scan Point One
*/ 
       cPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
       while( cPtr != dtmP->nullPtr )
         {
          pnt3 = clistAddrP(dtmP,cPtr)->pntNum ;
          cPtr = clistAddrP(dtmP,cPtr)->nextPtr ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Tin Line ** pnt1 = %6ld pnt3 = %6ld ** pnt2 = %6ld",pnt1,pnt3,pnt2) ;
/*
**        Check For Connection To Another Line End Point
*/
          if( pnt3 != pnt2 && nodeAddrP(dtmP,pnt3)->sPtr != dtmP->nullPnt && ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,pnt1,pnt3) ) 
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Connection Line = %6ld %6ld ** %6ld %6ld",pnt1,pnt3,nodeAddrP(dtmP,pnt1)->sPtr,nodeAddrP(dtmP,pnt3)->sPtr) ;
             if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,pnt1,pnt3,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
            }
/*
**        Check For Connection To Another Line Segment Point
*/
          else
            {
             if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,pnt3,&pointFeaturesP,&numPointFeatures) ) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Point Features = %6ld",numPointFeatures) ;
             if( numPointFeatures > 0 && pointOneFeature != pointFeaturesP->dtmFeature  )
               { 
                if( bcdtmList_getFirstAndLastPointForDtmFeatureDtmObject(dtmP,pointFeaturesP->dtmFeature,&firstPnt,&lastPnt)) goto errexit ;
/*
**              Check For Connection To Feature First Point
*/
                if( ! bcdtmList_testLineDtmObject(dtmP,pnt1,firstPnt) && nodeAddrP(dtmP,firstPnt)->sPtr != dtmP->nullPnt )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Getting Number Of Drape Break Points") ;
                   if( bcdtmConnect_getNumberOfDrapeBreakBetweenPointsDtmObject(dtmP,pnt1,firstPnt,&numBreaks)) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Drape Break Points = %6ld",numBreaks) ;
                   if( numBreaks == 2 )
                     {
                      intConnections = TRUE ;
                      if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,pnt1,firstPnt,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
                      if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,firstPnt,pnt1,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
                     }
                  } 
/*
**              Check For Connection To Feature Last Point
*/
                if( ! bcdtmList_testLineDtmObject(dtmP,pnt1,lastPnt) && nodeAddrP(dtmP,lastPnt)->sPtr != dtmP->nullPnt  )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Getting Number Of Drape Break Points") ;
                   if( bcdtmConnect_getNumberOfDrapeBreakBetweenPointsDtmObject(dtmP,pnt1,lastPnt,&numBreaks)) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Drape Break Points = %6ld",numBreaks) ;
                   if( numBreaks == 2 )
                     {
                      intConnections = TRUE ;
                      if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,pnt1,lastPnt,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
                      if( bcdtmConnect_storeConnectionLineDtmObject(dtmP,&conLinesP,lastPnt,pnt1,&numConLines,&memConLines,memConLinesInc)) goto errexit ;
                     }
                  } 
               }
            }
         } 
      }
   } 
/*
**  Write Number Of Connection Lines
*/
 if( dbg == 2 ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Connection Lines = %6ld",numConLines) ;
    for( conLineP = conLinesP ; conLineP < conLinesP + numConLines ; ++conLineP)
      {
       bcdtmWrite_message(0,0,0,"Connection Line[%4ld] ** fromPoint = %6ld toPoint = %6ld distance = %10.4lf",(long)(conLineP-conLinesP),conLineP->fromPoint,conLineP->toPoint,conLineP->distance) ;
      }
   } 
/*
**  Add Connection Lines To Connect Points Table
*/
 if( numConLines > 0 )
   {
/*
**  Sort Connection Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Connection Lines") ;
    qsortCPP(conLinesP,numConLines,sizeof(DTM_CONNECTION_LINE),bcdtmConnect_connectionLinePointsCompareFunction) ;
/*
**  Write Connection Lines
*/
    if( dbg == 2 )
      {
       for( conLineP = conLinesP ; conLineP < conLinesP + numConLines ; ++conLineP)
         {
          bcdtmWrite_message(0,0,0,"Connection Line[%4ld] ** fromPoint = %6ld toPoint = %6ld distance = %10.4lf",(long)(conLineP-conLinesP),conLineP->fromPoint,conLineP->toPoint,conLineP->distance) ;
         }
      }
/*
**  Remove Duplicate Connection Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Connection Lines") ;
    for( conLine1P = conLinesP , conLine2P = conLinesP + 1 ; conLine2P < conLinesP + numConLines ; ++conLine2P )
      {
       if( conLine2P->fromPoint != conLine1P->fromPoint || conLine2P->toPoint != conLine1P->toPoint )
         {
          ++conLine1P ;
          if( conLine1P != conLine2P ) *conLine1P = *conLine2P ;
         } 
      }
    numConLines = (long)(conLine1P-conLinesP) + 1 ;
/*
**  Write Connection Lines
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Connection Lines = %6ld",numConLines) ;
       for( conLineP = conLinesP ; conLineP < conLinesP + numConLines ; ++conLineP)
         {
          bcdtmWrite_message(0,0,0,"Connection Line[%4ld] ** fromPoint = %6ld toPoint = %6ld distance = %10.4lf",(long)(conLineP-conLinesP),conLineP->fromPoint,conLineP->toPoint,conLineP->distance) ;
         }
      } 
/*
**  Scan Connection Lines And Populate Connect Points Table With Connection Lines
*/
    for( conLine1P = conLinesP ; conLine1P < conLinesP + numConLines ; ++conLine1P )
      {
/*
**     Get All Connection Lines For Connect Point
*/
       conLine2P = conLine1P ;
       while( conLine2P < conLinesP + numConLines && conLine2P->fromPoint == conLine1P->fromPoint ) ++conLine2P ;
       --conLine2P ;
/*
**     Allocate memory
*/
       numPntConLines = (long)(conLine2P-conLine1P) + 1 ;
       pntConLinesP   = ( DTM_CONNECTION_LINE * ) malloc( numPntConLines * sizeof(DTM_CONNECTION_LINE)) ;
       if( pntConLinesP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }  
/*
**     Populate Connection Lines
*/
       memcpy(pntConLinesP,conLine1P,numPntConLines*sizeof(DTM_CONNECTION_LINE));
/*
**     Sort Connection Lines On Distance
*/
       qsortCPP(pntConLinesP,numPntConLines,sizeof(DTM_CONNECTION_LINE),bcdtmConnect_connectionLineDistanceCompareFunction) ;
/*
**     Write Connection Lines For Point
*/
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Number Of Connection Lines For Point %6ld = %6ld",conLine1P->fromPoint,numPntConLines) ;
          for( conLineP = pntConLinesP ; conLineP < pntConLinesP + numPntConLines ; ++conLineP)
            {
             bcdtmWrite_message(0,0,0,"Connection Line[%4ld] ** fromPoint = %6ld toPoint = %6ld distance = %10.4lf",(long)(conLineP-pntConLinesP),conLineP->fromPoint,conLineP->toPoint,conLineP->distance) ;
            }
         } 
/*
**     Populate Connection Lines
*/
       (*connectPointsPP+conLine1P->fromPoint)->conLineP   = pntConLinesP ;
       (*connectPointsPP+conLine1P->fromPoint)->numConLine = numPntConLines ;
       pntConLinesP = NULL ;
/*
**     Reset Connection Line Pointer
*/
       conLine1P = conLine2P ;  
      }
   }
/*
** Convert Tin Point Numbers To Point Offsets
*/
 pntNum = 0 ;
 for( lineP = *connectLinesPP ; lineP < *connectLinesPP + *numConnectLinesP ; ++lineP )
   {
    lineP->point1 = pntNum ;
    (*connectPointsPP+pntNum)->point1 = pntNum ;
    (*connectPointsPP+pntNum)->point2 = pntNum + 1 ;
    ++pntNum ;
    lineP->point2 = pntNum ;
    (*connectPointsPP+pntNum)->point1 = pntNum ;
    (*connectPointsPP+pntNum)->point2 = pntNum - 1 ;
    ++pntNum ;
   }
/*
**  Write Out Connect Lines And Connect Points List
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Connect Lines = %4ld",*numConnectLinesP ) ;
    for( lineP = *connectLinesPP ; lineP < *connectLinesPP + *numConnectLinesP ; ++lineP )
      {
       bcdtmWrite_message(0,0,0,"Line[%4ld] = line = %4ld ** pnt1 = %4ld pnt2 = %4ld isRev = %2ld ** len = %10.4lf",(long)(lineP-*connectLinesPP),lineP->line,lineP->point1,lineP->point2,lineP->isReversed,lineP->length) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Connect Points = %4ld",*numConnectPointsP ) ;
    for( pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pointP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] ** line = %4ld ** pnt1 = %4ld pnt2 = %4ld isRev = %2ld ** numConLines = %4ld",(long)(pointP-*connectPointsPP),pointP->line,pointP->point1,pointP->point2,pointP->isReversed,pointP->numConLine) ;
       if( pointP->numConLine > 0 )
         {
          for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine ; ++conLineP ) 
            {
             bcdtmWrite_message(0,0,0,"**** Connected Point[%4ld] ** pnt = %8ld line = %4ld distance = %10.4lf",(long)(conLineP-pointP->conLineP),conLineP->toPoint,conLineP->line,conLineP->distance) ;
            }  
         } 
      }
   }
/*
** Determine Intersecting Connection Lines
*/
 if( intConnections == TRUE )
   {
/*
**  Count Number Of Connections Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Counting Number Of Connection Lines") ;
    numConnectionLines = 0 ;
    for( pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pointP )
      {
       numConnectionLines = numConnectionLines + pointP->numConLine ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Connection Lines = %6ld",numConnectionLines) ;
/*
**  Allocate memory To Hold Connection Point Offsets
*/
    numConOfs = numConnectionLines ;
    conOffsetsP = ( struct Connect_Offsets * ) malloc ( numConOfs * sizeof(struct Connect_Offsets)) ;
    if( conOffsetsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Allocate memory For Connection Lines
*/
    connectionLinesPP = ( DTM_POINT_ARRAY ** ) malloc( numConnectionLines * sizeof(DTM_POINT_ARRAY *)) ;
    if( connectionLinesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }  
    for( pntArrayPP = connectionLinesPP ; pntArrayPP < connectionLinesPP + numConnectionLines ; ++pntArrayPP ) *pntArrayPP = NULL ;
/*
**  Polulate Connection Lines Array
*/
    lineNum = 0 ;
    for( pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pointP )
      {
/*
**     Scan Connection Lines For Point
*/
       for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine ; ++conLineP )
         {
/*
**        Populate Connection Offsets
*/
          (conOffsetsP+lineNum)->pnt1 = pointP->point1  ;
          (conOffsetsP+lineNum)->pnt2 = conLineP->toPoint ;
/*
**        Allocate Memory For Connection Line Coordinates
*/
          conLinePtsP = ( DPoint3d * ) malloc( 2 * sizeof(DPoint3d)) ;
          if( conLinePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }  
/*
**        Set Connection Line Start Coordinates
*/
          line = pointP->point1 / 2 ;
          if( pointP->point1 % 2 == 0 )
            {
             conLinePtsP->x = (*(lineStringsPP+line))->pointsP->x ;
             conLinePtsP->y = (*(lineStringsPP+line))->pointsP->y ;
             conLinePtsP->z = (*(lineStringsPP+line))->pointsP->z ;
            }
          else
            {
             conLinePtsP->x = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->x ;
             conLinePtsP->y = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->y ;
             conLinePtsP->z = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->z ;
            }
/*
**        Set Connection Line End Coordinates
*/
          line = conLineP->toPoint / 2 ;
          if( conLineP->toPoint % 2 == 0 )
            {
             (conLinePtsP+1)->x = (*(lineStringsPP+line))->pointsP->x ;
             (conLinePtsP+1)->y = (*(lineStringsPP+line))->pointsP->y ;
             (conLinePtsP+1)->z = (*(lineStringsPP+line))->pointsP->z ;
            }
          else
            {
             (conLinePtsP+1)->x = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->x ;
             (conLinePtsP+1)->y = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->y ;
             (conLinePtsP+1)->z = ((*(lineStringsPP+line))->pointsP+(*(lineStringsPP+line))->numPoints-1)->z ;
            }
/*
**        Allocate Memory For Point Array
*/
          pntArrayP = ( DTM_POINT_ARRAY * ) malloc(sizeof(DTM_POINT_ARRAY)) ;
          if( pntArrayP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
/*
**        Populate Point Array
*/
          pntArrayP->pointsP   = conLinePtsP ;
          pntArrayP->numPoints = 2 ;
          conLinePtsP = NULL ;
/*
**        Populate Pointer array To Connection Lines
*/
          *(connectionLinesPP+lineNum) = pntArrayP ;
          ++lineNum ;
          pntArrayP = NULL ;
         }
      }
/*
**  Get Connection Line Intersections
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Connection Lines") ;
    if( bcdtmString_detectStringIntersections2D(connectionLinesPP,numConnectionLines,intersectType,&intPtsP,&numIntPts) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Connection Line Intersections = %6ld",numIntPts) ;
/*
**  Write Intersection Points
*/ 
    if( dbg == 1 ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersection Points With Other Connect Lines = %6ld",numIntPts) ;
       for( int1P = intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
         {
          bcdtmWrite_message(0,0,0,"intPoint[%4ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.5lf %12.5lf %10.4lf",(long)(int1P-intPtsP),int1P->string1Offset,int1P->segment1Offset,int1P->string2Offset,int1P->segment2Offset,int1P->x,int1P->y,int1P->zSegment1) ; 
         }
      }
/*
**  Add Intersections To Connection Line Tables Of Connect Points
*/ 
    if( numIntPts > 0 )
      {
/*
**     Convert Line And Segment Offsets To Connect Point Offsets
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Coverting Line Segment Offsets To Connect Point Offsets") ;
       for( intP = intPtsP ; intP < intPtsP + numIntPts ; ++intP) 
         {
          line = intP->string1Offset ;
          intP->string1Offset  = (conOffsetsP+line)->pnt1 ;
          intP->segment1Offset = (conOffsetsP+line)->pnt2 ;
          line = intP->string2Offset ;
          intP->string2Offset  = (conOffsetsP+line)->pnt1 ;
          intP->segment2Offset = (conOffsetsP+line)->pnt2 ;
         }
/*
**     Write Intersection Points
*/ 
       if( dbg == 1 ) 
         {
          bcdtmWrite_message(0,0,0,"Number Of Intersection Points With Other Connect Lines = %6ld",numIntPts) ;
          for( int1P = intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
            {
             bcdtmWrite_message(0,0,0,"intPoint[%4ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.5lf %12.5lf %10.4lf",(long)(int1P-intPtsP),int1P->string1Offset,int1P->segment1Offset,int1P->string2Offset,int1P->segment2Offset,int1P->x,int1P->y,int1P->zSegment1) ; 
            }
         }
/*
**     Sort Connection Line Intersections On Connection Line Point Offsets
*/
       qsortCPP(intPtsP,numIntPts,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCompareFunction) ;
/*
**     Add Intersected Connected Lines To Connection Line Table
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Updating Connection Lines With Intersecting Connection Lines") ;
       for( int1P = intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
         {
/*
**        Scan To End Of Intersections For Connection Line
*/
          int2P = int1P ;
          while( int2P < intPtsP + numIntPts && int2P->string1Offset == int1P->string1Offset &&  int2P->segment1Offset == int1P->segment1Offset ) ++int2P ;
          --int2P ;
/*
**        Write Out Intersected Connection Lines
*/
          if( dbg )
            {
             for( intP = int1P ; intP <= int2P ; ++intP) 
               {
                bcdtmWrite_message(0,0,0,"Connection Line %6ld %6ld Intersects Connection Line %6ld %6ld",intP->string1Offset,intP->segment1Offset,intP->string2Offset,intP->segment2Offset) ;  
               } 
            }
/*
**        Count Number Of Intersected Connection Lines
*/   
          numIntConLines = (long)(int2P-int1P) + 1 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersections For Connect Line %6ld %6ld = %6ld",int1P->string1Offset,int1P->segment1Offset,numIntConLines) ;
          intConLinesP = ( DTM_CONNECTION_LINE_INTERSECT * ) malloc ( numIntConLines * sizeof(DTM_CONNECTION_LINE_INTERSECT)) ;
          if( intConLinesP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ; 
            }
/*
**        Store Connection Line Intersections
*/
          intConLineP = intConLinesP ;
          for( intP = int1P ; intP <= int2P ; ++intP) 
            {
/*
**           Find Offset For Intersected Connection Lines
*/
             ofsIntConLine = -1 ;
             pointP  = *connectPointsPP + intP->string2Offset  ;      
             for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine && ofsIntConLine == -1 ; ++conLineP )
               {
               if( conLineP->toPoint == intP->segment2Offset ) ofsIntConLine = (long)(conLineP-pointP->conLineP) ;
               }
/*
**           Check Offset Found
*/
             if( ofsIntConLine < 0 )
               {
                bcdtmWrite_message(2,0,0,"Cannot Find Offset For Intersected Connection line %6ld %6ld",intP->string2Offset,intP->segment2Offset) ;
                goto errexit ;
               }
/*
**           Store Intersected Connection Line
*/
             intConLineP->point1 = intP->string2Offset  ;
             intConLineP->point2 = intP->segment2Offset ;
             intConLineP->index  = ofsIntConLine ;
             ++intConLineP ;
            }
/*
**        Find Offset For Intersecting Connection Line
*/
          pointP = *connectPointsPP + int1P->string1Offset  ; 
          for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine && intConLinesP != NULL ; ++conLineP )
            {
             if( conLineP->toPoint == int1P->segment1Offset ) 
               {
                conLineP->intConLineP   = intConLinesP ;
                conLineP->numIntConLine = numIntConLines ;
                intConLinesP = NULL ;      
               }
            }
/*
**        Check Connection Line Found  
*/
          if( intConLinesP != NULL )
            {
             bcdtmWrite_message(2,0,0,"Cannot Find Entry For Connection Line %6ld %6ld",intP->string1Offset,intP->segment1Offset) ;
             goto errexit ;
            }
/*
**        Reset Scan pointer
*/
          int1P = int2P ;
         }
      }
   }  
/*
** Cleanup
*/
 cleanup :
 if( conLinePtsP  != NULL ) { free(conLinePtsP)  ; conLinePtsP = NULL ; }
 if( intPtsP      != NULL ) { free(intPtsP)      ; intPtsP     = NULL ; }
 if( intConLinesP != NULL ) { free(conLinesP)    ; conLinesP   = NULL ; }
 if( conOffsetsP  != NULL ) { free(conOffsetsP)  ; conOffsetsP = NULL ; }
 if( pntConLinesP != NULL ) { free(pntConLinesP) ; pntConLinesP = NULL ; }
 if( pntArrayP    != NULL )
   {
    if( pntArrayP->pointsP != NULL ) free(pntArrayP->pointsP ) ;
    free( pntArrayP) ;
    pntArrayP = NULL ; 
   } 
 if( connectionLinesPP != NULL ) bcdtmMem_freePointerArrayToPointArrayMemory(&connectionLinesPP,numConnectionLines) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Connect Tables Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Building Connect Tables Error") ;
 return( ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numConnectLinesP  = 0 ;
 *numConnectPointsP = 0 ;
 if( *connectLinesPP  != NULL ) { free(*connectLinesPP)  ; *connectLinesPP  = NULL ; }
 if( *connectPointsPP != NULL ) { free(*connectPointsPP) ; *connectPointsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmConnect_getNumberOfDrapeBreakBetweenPointsDtmObject
(
 BC_DTM_OBJ *dtmP,              /* ==> Pointer To Tin Object          */
 long  startPnt,                 /* ==> Start Drape Point              */
 long  endPnt,                   /* ==> End Drape Point                */
 long  *numBreaksP               /* <== Number Of Drape Break Points   */
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long              numDrapePts=0,numStringPts=2,dtmFeatureOption=FALSE ;
 DPoint3d               stringPts[2] ;          
 DTM_DRAPE_POINT   *drapeP,*drapePtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Number Of Drape Breaks Between Tin Points Completed") ;
/*
** Initialise
*/
 *numBreaksP = 0 ;
/*
** Set Coordinates For Drape String
*/
 stringPts[0].x = pointAddrP(dtmP,startPnt)->x ;
 stringPts[0].y = pointAddrP(dtmP,startPnt)->y ;
 stringPts[1].x = pointAddrP(dtmP,endPnt)->x ;
 stringPts[1].y = pointAddrP(dtmP,endPnt)->y ;
/*
** Write String Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drape Line = %12.5lf %12.5lf ** %12.5lf %12.5lf",stringPts[0].x,stringPts[0].y,stringPts[1].x,stringPts[1].y) ;
/*
** Drape String
*/
 if( bcdtmDrape_stringDtmObject(dtmP,stringPts,numStringPts,dtmFeatureOption,&drapePtsP,&numDrapePts)) goto errexit ;
/*
** Count Number Of Break Points
*/
 for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP)
   {
   if (drapeP->drapeType == DTMDrapedLineCode::Breakline) ++*numBreaksP;
   }
/*
** Write Out Drape Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts) ;
   }
/*
** Cleanup
*/
 cleanup :
 if( drapePtsP != NULL ) free(drapePtsP) ;  // As dtmFeatureOption is FALSE
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Number Of Drape Breaks Between Tin Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Number Of Drape Breaks Between Tin Points Error") ;
 return( ret) ;
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
BENTLEYDTM_Private int bcdtmConnect_storeConnectionLineDtmObject
(
 BC_DTM_OBJ  *dtmP ,                  /*  ==> Pointer To Tin Object                */
 DTM_CONNECTION_LINE **conLinesPP,     /* <==> Pointer To Connection Lines          */
 long  fromPoint,                      /*  ==> Connection From Point                */
 long  toPoint,                        /*  ==> Connection To Point                  */                        
 long  *numConLinesP,                  /* <==> Number Of Connection Lines           */
 long  *memConLinesP,                  /* <==> Memory Connection Lines              */
 long  memConLinesInc                  /*  ==> Memory Increment Amount              */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long isStored=FALSE,zeroLength=0 ;
 DTM_CONNECTION_LINE *conLineP ;
/*
**  Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Connection Line ** fromPnt = %6ld toPnt = %6ld",fromPoint,toPoint) ;
/*
** Check For Zero Length Connection Line
*/
 if( fromPoint == toPoint ) zeroLength = 1 ;
/*
** Check Connection Line Does Not Exist
*/
 isStored = FALSE ;
 if( *numConLinesP > 0 )
   {
    for( conLineP = *conLinesPP ; conLineP < *conLinesPP + *numConLinesP && isStored == FALSE ; ++conLineP )
      {
       if( conLineP->fromPoint  == nodeAddrP(dtmP,fromPoint)->sPtr &&
           conLineP->toPoint    == nodeAddrP(dtmP,toPoint)->sPtr      ) isStored = TRUE ;
      }
   }
/*
** Store Connection Line
*/
 if( isStored == FALSE )
   {
/*
**  Check Memory
*/
    if( *numConLinesP + zeroLength >= *memConLinesP  )
      {
       *memConLinesP = *memConLinesP + memConLinesInc ;
       if( *conLinesPP == NULL ) *conLinesPP = ( DTM_CONNECTION_LINE * ) malloc( *memConLinesP * sizeof(DTM_CONNECTION_LINE)) ;
       else                      *conLinesPP = ( DTM_CONNECTION_LINE * ) realloc( *conLinesPP , *memConLinesP * sizeof(DTM_CONNECTION_LINE)) ;  
       if( *conLinesPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store Connected Line
*/
    if( ! zeroLength )
      { 
       (*conLinesPP+*numConLinesP)->fromPoint     = nodeAddrP(dtmP,fromPoint)->sPtr ;
       (*conLinesPP+*numConLinesP)->toPoint       = nodeAddrP(dtmP,toPoint)->sPtr ;
       (*conLinesPP+*numConLinesP)->line          = nodeAddrP(dtmP,toPoint)->sPtr / 2 ; 
       (*conLinesPP+*numConLinesP)->distance      = bcdtmMath_pointDistanceDtmObject(dtmP,fromPoint,toPoint) ;
       (*conLinesPP+*numConLinesP)->intConLineP   = NULL  ; 
       (*conLinesPP+*numConLinesP)->numIntConLine = 0     ; 
       ++*numConLinesP ; 
      }
    else
      {
       (*conLinesPP+*numConLinesP)->fromPoint     = nodeAddrP(dtmP,fromPoint)->tPtr ;
       (*conLinesPP+*numConLinesP)->toPoint       = nodeAddrP(dtmP,toPoint)->sPtr ;
       (*conLinesPP+*numConLinesP)->line          = nodeAddrP(dtmP,toPoint)->sPtr / 2 ; 
       (*conLinesPP+*numConLinesP)->distance      = 0.0 ;
       (*conLinesPP+*numConLinesP)->intConLineP   = NULL  ; 
       (*conLinesPP+*numConLinesP)->numIntConLine = 0     ; 
       ++*numConLinesP ; 
       (*conLinesPP+*numConLinesP)->fromPoint     = nodeAddrP(dtmP,fromPoint)->sPtr ;
       (*conLinesPP+*numConLinesP)->toPoint       = nodeAddrP(dtmP,toPoint)->tPtr ;
       (*conLinesPP+*numConLinesP)->line          = nodeAddrP(dtmP,toPoint)->tPtr / 2 ; 
       (*conLinesPP+*numConLinesP)->distance      = 0.0 ;
       (*conLinesPP+*numConLinesP)->intConLineP   = NULL  ; 
       (*conLinesPP+*numConLinesP)->numIntConLine = 0     ; 
       ++*numConLinesP ; 
      }
   } 
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Connection Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Connection Line Error") ;
 return( ret) ;
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
BENTLEYDTM_Private int bcdtmConnect_connectionLineDistanceCompareFunction(const DTM_CONNECTION_LINE *conLine1P,const DTM_CONNECTION_LINE *conLine2P)
/*
** Compare Function For Qsort Of Increasing Distance Connect Points
*/
{
 if     (  conLine1P->distance   >  conLine2P->distance  ) return( 1) ;
 else if(  conLine1P->distance   <  conLine2P->distance  ) return(-1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmConnect_connectionLinePointsCompareFunction(const DTM_CONNECTION_LINE *conLine1P,const DTM_CONNECTION_LINE *conLine2P)
/*
** Compare Function For Qsort Of Increasing Distance Connect Points
*/
{
 if     (  conLine1P->fromPoint  <  conLine2P->fromPoint  ) return(-1) ;
 else if(  conLine1P->fromPoint  >  conLine2P->fromPoint  ) return( 1) ;
 if     (  conLine1P->toPoint    <  conLine2P->toPoint    ) return(-1) ;
 else if(  conLine1P->toPoint    >  conLine2P->toPoint    ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmConnect_buildConnectLineTablesFromStrings
(
 DTM_POINT_ARRAY **lineStringsPP,      /* ==> Pointer To Pointer Array Of Line Strings     */
 long  numLineStrings,                 /* ==> Number Of Line Strings                       */
 DTM_CONNECT_LINE **connectLinesPP,    /* <== Pointer To Connect Lines                     */
 long  *numConnectLinesP,              /* <== Number Of Connect Lines                      */
 DTM_CONNECT_POINT **connectPointsPP,  /* <== Pointer To Connect Points                    */
 long  *numConnectPointsP              /* <== Number Of Connect Points                     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1,pnt2,line1,line2,numLine,numPts,numIntPts,lineNum,numValidConOfs,*conLineIndexP=NULL,numConnections=0 ;
 long   numLocalStrings=0,memLocalStrings=0,memLocalStringsInc=1000,intersectType=1,ofsConLine,ofsIntConLine ;
 long   numConnectPts,numConOfs=0,numConIntPts=0,numConIntPtsFnd=0 ;
 double dx,dy,lineLength=0.0,pntX,pntY,conPntX,conPntY ;
 DPoint3d    *p3dP,*ptsP ;
 
 DTM_POINT_ARRAY   **pntArrayPP,**pntArray1PP,**pntArray2PP,**localStringsPP=NULL,*newLineP=NULL ;
 DTM_CONNECT_LINE  *lineP ;
 DTM_CONNECT_POINT *pointP ;
 DTM_CONNECTION_LINE *conLineP,*conLinesP=NULL ;
 DTM_INTERSECT_POINT *intP,*int1P,*int2P,*intPtsP=NULL ;
 DTM_CONNECTION_LINE_INTERSECT *intConLineP,*intConLinesP=NULL ;
 struct Connect_Offsets { long pnt1,pnt2,valid ; } *conOfsP,*con1P,*con2P,*conOffsetsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Connect Tables From Strings") ;
/*
** Write Out String End Points
*/
 if( dbg == 2 )
   {
    for( pntArrayPP = lineStringsPP ; pntArrayPP < lineStringsPP + numLineStrings ; ++pntArrayPP )
      {
       pnt1   = (long)(pntArrayPP-lineStringsPP) * 2 ;
       pnt2   = pnt1 + 1 ;
       p3dP   = (*pntArrayPP)->pointsP ;
       numPts = (*pntArrayPP)->numPoints ;
       bcdtmWrite_message(0,0,0,"EndPoint[%6ld] = %12.5lf %12.5lf %10.4lf",pnt1,p3dP->x,p3dP->y,p3dP->z) ;
       p3dP   = p3dP + numPts - 1 ; 
       bcdtmWrite_message(0,0,0,"EndPoint[%6ld] = %12.5lf %12.5lf %10.4lf",pnt2,p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Make A Local Copy Of The Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Making A Local Copy of The Line Strings") ;
 numLocalStrings = numLineStrings ;
 memLocalStrings = numLineStrings + memLocalStringsInc ;
 localStringsPP = ( DTM_POINT_ARRAY ** ) malloc ( memLocalStrings * sizeof(DTM_POINT_ARRAY *)) ;
 if( localStringsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Local Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Local Copy") ;
 for( pntArray1PP = localStringsPP  ; pntArray1PP < localStringsPP + memLocalStrings ; ++pntArray1PP )
   {
    *pntArray1PP = NULL ;
   }
/*
** Copy From Line Strings To Local Line Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copy line Strings To Local Strings") ;
 for( pntArray1PP = localStringsPP , pntArray2PP = lineStringsPP ; pntArray2PP < lineStringsPP + numLineStrings ; ++pntArray1PP , ++pntArray2PP )
   {
    ptsP = (DPoint3d * ) malloc( (*pntArray2PP)->numPoints  * sizeof(DPoint3d)) ;
    if( ptsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
    memcpy(ptsP,(*pntArray2PP)->pointsP,(*pntArray2PP)->numPoints*sizeof(DPoint3d)) ;
    newLineP = ( DTM_POINT_ARRAY * ) malloc(sizeof(DTM_POINT_ARRAY)) ;
    if( newLineP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
    newLineP->numPoints = (*pntArray2PP)->numPoints ;
    newLineP->pointsP   = ptsP ;
    *pntArray1PP = newLineP ;
    ptsP     = NULL ;
    newLineP = NULL ;
   } 
/*
** Allocate memory To Hold Connection Point Offsets
*/
 numConOfs = numLineStrings * ( numLineStrings - 1 ) * 4 ;
 conOffsetsP = ( struct Connect_Offsets * ) malloc ( numConOfs * sizeof(struct Connect_Offsets)) ;
 if( conOffsetsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Populate Connection Points Offset
*/
 numConOfs = 0 ;
 for( pntArray1PP = lineStringsPP  ; pntArray1PP < lineStringsPP + numLineStrings ; ++pntArray1PP ) 
   {
    line1 = (long)(pntArray1PP-lineStringsPP) ;
/*
**  Create Connection Offsets From Both Ends Of The Line
*/
    for( numPts = 0 ; numPts < 2 ; ++numPts )
      {
/*
**     Set Line 1 Point Numbers
*/
       if( numPts == 0 ) pnt1 = line1 * 2 ;
       else              pnt1 = line1 * 2 + 1 ;
       for( pntArray2PP = lineStringsPP ; pntArray2PP < lineStringsPP + numLineStrings ; ++pntArray2PP ) 
         {
          if( pntArray2PP != pntArray1PP )
            {
             line2 = (long)(pntArray2PP-lineStringsPP) ; 
/*
**           Populate Connect Offsets
*/         
             pnt2  = line2 * 2 ;
             (conOffsetsP+numConOfs)->pnt1  = pnt1 ;
             (conOffsetsP+numConOfs)->pnt2  = pnt2 ;
             (conOffsetsP+numConOfs)->valid = 1    ;
             ++numConOfs ;
             pnt2  = line2 * 2 + 1 ;
             (conOffsetsP+numConOfs)->pnt1  = pnt1 ;
             (conOffsetsP+numConOfs)->pnt2  = pnt2 ;
             (conOffsetsP+numConOfs)->valid = 1    ;
             ++numConOfs ;
            }
         } 
      } 
   }
/*
**  Write Connection Offsets
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Connect Offsets = %6ld",numConOfs) ;
    for( conOfsP = conOffsetsP ; conOfsP < conOffsetsP + numConOfs ; ++conOfsP )
      {
       bcdtmWrite_message(0,0,0,"connectionOffset[%6ld] = pnt1 = %6ld pnt2 %6ld valid = %2ld",(long)(conOfsP-conOffsetsP),conOfsP->pnt1,conOfsP->pnt2,conOfsP->valid) ;
      }
   }
/*
** Allocate Memory For Connection Line Indexes
*/
 conLineIndexP = ( long * ) malloc(numConOfs/2*sizeof(long)) ;
 if( conLineIndexP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Append Local Strings With All Connections
*/
 numLine = 0 ;
 for( conOfsP = conOffsetsP ; conOfsP < conOffsetsP + numConOfs ; ++conOfsP )
   {
    if( conOfsP->pnt1 < conOfsP->pnt2 )
      {
       line1 = conOfsP->pnt1 / 2 ;
       line2 = conOfsP->pnt2 / 2 ; 
       pnt1  = conOfsP->pnt1 - line1 * 2 ;
       pnt2  = conOfsP->pnt2 - line2 * 2 ;
       *(conLineIndexP+numLine) = (long)(conOfsP-conOffsetsP) ;
       ++numLine ;
/*
**     Allocate Memory For Connection Line Points
*/ 
       numPts = 2 ;
       ptsP = (DPoint3d * ) malloc( numPts * sizeof(DPoint3d)) ;
       if( ptsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Set Coordinates For Pnt1
*/
       if( pnt1 == 0 )
         { 
          ptsP[0].x = (*(lineStringsPP+line1))->pointsP->x ;
          ptsP[0].y = (*(lineStringsPP+line1))->pointsP->y ;
          ptsP[0].z = (*(lineStringsPP+line1))->pointsP->z ;
         }
       else
         {
          ptsP[0].x = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->x ;
          ptsP[0].y = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->y ;
          ptsP[0].z = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->z ;
         } 
/*
**     Set Coordinates For Pnt2
*/
       if( pnt2 == 0 )
         { 
          ptsP[1].x = (*(lineStringsPP+line2))->pointsP->x ;
          ptsP[1].y = (*(lineStringsPP+line2))->pointsP->y ;
          ptsP[1].z = (*(lineStringsPP+line2))->pointsP->z ;
         }
       else
        {
         ptsP[1].x = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->x ;
         ptsP[1].y = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->y ;
         ptsP[1].z = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->z ;
        } 
/*
**    Allocate Point Array Memory For Connection Line
*/
      newLineP = ( DTM_POINT_ARRAY * ) malloc(sizeof(DTM_POINT_ARRAY)) ;
      if( newLineP == NULL )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        } 
/*
**    Store Point Array
*/
      newLineP->pointsP   = ptsP ;
      newLineP->numPoints = numPts ;
      ptsP = NULL ; 
/*
**     Check Local Lines Memory
*/
       if( numLocalStrings == memLocalStrings )
         {
          memLocalStrings = memLocalStrings + memLocalStringsInc ;
          localStringsPP  = ( DTM_POINT_ARRAY ** ) realloc ( localStringsPP , memLocalStrings * sizeof(DTM_POINT_ARRAY *)) ;
          if( localStringsPP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          for( pntArrayPP = localStringsPP + numLocalStrings ; pntArrayPP < localStringsPP + memLocalStrings ; ++pntArrayPP ) *pntArrayPP = NULL ;
         }
/*
**     Store Line Connection In Local Line Strings
*/
       *(localStringsPP+numLocalStrings) = newLineP ;
       ++numLocalStrings ;
       newLineP = NULL ;
      } 
   } 
/*
** Write Stats
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Local Line Strings  = %6ld",numLocalStrings) ;
    bcdtmWrite_message(0,0,0,"Number Of Connection Strings  = %6ld",numLocalStrings - numLineStrings ) ;
   }
/*
** Check For Intersecting Line Strings
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Line Strings") ;
 if( bcdtmString_detectStringIntersections2D(localStringsPP,numLocalStrings,intersectType,&intPtsP,&numIntPts) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %6ld",numIntPts) ;
/*
**  Write Intersection Points
*/ 
 if( dbg == 2 ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersection Points With Other Connect Lines = %6ld",numIntPts) ;
    numLine = numIntPts ;
    if( numLine > 100 ) numLine = 100 ;
    for( int1P = intPtsP ; int1P < intPtsP + numLine ; ++int1P )
      {
       bcdtmWrite_message(0,0,0,"intPoint[%4ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.5lf %12.5lf %10.4lf",(long)(int1P-intPtsP),int1P->string1Offset,int1P->segment1Offset,int1P->string2Offset,int1P->segment2Offset,int1P->x,int1P->y,int1P->zSegment1) ; 
      }
   }
/*
** Process Intersection Points
*/
 if( numIntPts > 0 )
   {
/*
** Mark Connection Lines That Intersect Existing Strings
*/
    for( int1P =  intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
      {
       if( int1P->string1Offset >= numLineStrings && int1P->string2Offset < numLineStrings ) 
         {
          lineNum = int1P->string1Offset - numLineStrings ;
          conOfsP = conOffsetsP + *(conLineIndexP+lineNum ) ;
          conOfsP->valid = 0 ;
          pnt1 = conOfsP->pnt1 ;
          pnt2 = conOfsP->pnt2 ;
          conOfsP = conOffsetsP + pnt2 * ( numLineStrings - 1 ) * 2 + pnt1 ;
          conOfsP->valid = 0 ;
          int1P->string2Offset = - 1 ;
         }
       if( int1P->string2Offset >= numLineStrings && int1P->string1Offset < numLineStrings ) 
         {
          lineNum = int1P->string2Offset - numLineStrings ;
          conOfsP = conOffsetsP + *(conLineIndexP+lineNum ) ;
          conOfsP->valid = 0 ;
          pnt1 = conOfsP->pnt1 ;
          pnt2 = conOfsP->pnt2 ;
          conOfsP = conOffsetsP + pnt2 * ( numLineStrings - 1 ) * 2 + pnt1 ;
          conOfsP->valid = 0 ;
          int1P->string1Offset = - 1 ;
         }
      }
/*
**  Write Connection Offsets
*/
    if( dbg == 2 )
      {
       numValidConOfs = 0 ;
       bcdtmWrite_message(0,0,0,"Number Of Connect Offsets = %6ld",numConOfs) ;
       for( conOfsP = conOffsetsP ; conOfsP < conOffsetsP + numConOfs ; ++conOfsP )
         {
          bcdtmWrite_message(0,0,0,"connectionOffset[%6ld] = pnt1 = %6ld pnt2 %6ld valid = %2ld",(long)(conOfsP-conOffsetsP),conOfsP->pnt1,conOfsP->pnt2,conOfsP->valid) ;
          if( conOfsP->valid) ++ numValidConOfs ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Valid Connect Offsets = %6ld",numValidConOfs) ;
      }
/*
**  Remove Connections That Intersect Connect Strings
*/
    for( con1P = con2P = conOffsetsP ; con2P < conOffsetsP + numConOfs ; ++con2P )
      {
       if( con2P->valid )
         {
          if( con1P != con2P ) *con1P = *con2P ;
          ++con1P ;
         } 
      }
    numConOfs = (long)(con1P-conOffsetsP) ;
/*
**  Write Valid Connection Lines
*/
    if( dbg == 2 )
      {
       numValidConOfs = 0 ;
       bcdtmWrite_message(0,0,0,"Number Of valid Connection Lines = %6ld",numConOfs) ;
       for( conOfsP = conOffsetsP ; conOfsP < conOffsetsP + numConOfs ; ++conOfsP )
         {
          bcdtmWrite_message(0,0,0,"connectionOffset[%6ld] = pnt1 = %6ld pnt2 %6ld valid = %2ld",(long)(conOfsP-conOffsetsP),conOfsP->pnt1,conOfsP->pnt2,conOfsP->valid) ;
          if( conOfsP->valid) ++ numValidConOfs ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Valid Connect Offsets = %6ld",numValidConOfs) ;
      }
/*
** Free Intersect Points memory
*/
   numIntPts = 0 ;
   if( intPtsP != NULL ) { free(intPtsP) ; intPtsP = NULL ; }
/*
** Free Local Line Strings memory
*/
   if( localStringsPP != NULL )  bcdtmMem_freePointerArrayToPointArrayMemory(&localStringsPP,numLocalStrings) ;
   numLocalStrings = 0 ;
/*
** Allocate Local Strings Memory To Store Connection Lines
*/
   numLocalStrings = numConOfs ;
   localStringsPP = ( DTM_POINT_ARRAY ** ) malloc ( numLocalStrings * sizeof(DTM_POINT_ARRAY)) ;
   if( localStringsPP == NULL )
     {
      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
      goto errexit ;
     }
/*
** Populate Local Line Strings With Connection Lines
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Populating Local Strings With Connection Lines") ;
   numLine = 0 ;
   for( conOfsP = conOffsetsP ; conOfsP < conOffsetsP + numConOfs ; ++conOfsP )
     {
      line1 = conOfsP->pnt1 / 2 ;
      line2 = conOfsP->pnt2 / 2 ; 
      pnt1  = conOfsP->pnt1 - line1 * 2 ;
      pnt2  = conOfsP->pnt2 - line2 * 2 ;
/*
**    Allocate Memory For Connection Line Points
*/ 
      numPts = 2 ;
      ptsP = (DPoint3d * ) malloc( numPts * sizeof(DPoint3d)) ;
      if( ptsP == NULL )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
/*
**    Set Coordinates For Pnt1
*/
      if( pnt1 == 0 )
        { 
         ptsP[0].x = (*(lineStringsPP+line1))->pointsP->x ;
         ptsP[0].y = (*(lineStringsPP+line1))->pointsP->y ;
         ptsP[0].z = (*(lineStringsPP+line1))->pointsP->z ;
        }
      else
        {
         ptsP[0].x = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->x ;
         ptsP[0].y = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->y ;
         ptsP[0].z = ((*(lineStringsPP+line1))->pointsP+(*(lineStringsPP+line1))->numPoints-1)->z ;
        } 
/*
**    Set Coordinates For Pnt2
*/
      if( pnt2 == 0 )
        { 
         ptsP[1].x = (*(lineStringsPP+line2))->pointsP->x ;
         ptsP[1].y = (*(lineStringsPP+line2))->pointsP->y ;
         ptsP[1].z = (*(lineStringsPP+line2))->pointsP->z ;
        }
      else
        {
         ptsP[1].x = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->x ;
         ptsP[1].y = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->y ;
         ptsP[1].z = ((*(lineStringsPP+line2))->pointsP+(*(lineStringsPP+line2))->numPoints-1)->z ;
        } 
/*
**   Allocate Point Array Memory For Connection Line
*/
     newLineP = ( DTM_POINT_ARRAY * ) malloc(sizeof(DTM_POINT_ARRAY)) ;
     if( newLineP == NULL )
       {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
       } 
/*
**   Store Point Array
*/
     newLineP->pointsP   = ptsP ;
     newLineP->numPoints = numPts ;
     ptsP = NULL ; 
/*
**   Store Line Connection In Local Line Strings
*/
     *(localStringsPP+numLine) = newLineP ;
     ++numLine ;
     newLineP = NULL ;
     }
/*
**  Determine Connection Line Intersections
*/ 
   if( dbg ) bcdtmWrite_message(0,0,0,"Determine Connection Line Intersections") ;
   if( bcdtmString_detectStringIntersections2D(localStringsPP,numLocalStrings,intersectType,&intPtsP,&numIntPts) ) goto errexit ;
   if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Connection Line Intersections = %6ld",numIntPts) ;
/*
**  Change Connection Line Offsets To Connection Line Point Offsets
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Determine Connection Line Offsets To Connection Line Point Offsets") ;
    for( int1P =  intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
      {
       conOfsP = conOffsetsP + int1P->string1Offset ;
       int1P->string1Offset  = conOfsP->pnt1 ;
       int1P->segment1Offset = conOfsP->pnt2 ;
       conOfsP = conOffsetsP + int1P->string2Offset ;
       int1P->string2Offset  = conOfsP->pnt1 ;
       int1P->segment2Offset = conOfsP->pnt2 ;
      }
/*
**  Sort Connection Line Intersections On Connection Line Point Offsets
*/
    qsortCPP(intPtsP,numIntPts,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCompareFunction) ;
/*
**  Write Intersection Points
*/ 
    if( dbg == 2 ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersection Points With Other Connect Lines = %6ld",numIntPts) ;
       numLine = numIntPts ;
       if( numLine > 100 ) numLine = 100 ;
       for( int1P = intPtsP ; int1P < intPtsP + numLine ; ++int1P )
         {
          bcdtmWrite_message(0,0,0,"intPoint[%4ld] ** str1 = %6ld seg1 = %6ld ** str2 = %6ld seg2 = %6ld ** %12.5lf %12.5lf %10.4lf",(long)(int1P-intPtsP),int1P->string1Offset,int1P->segment1Offset,int1P->string2Offset,int1P->segment2Offset,int1P->x,int1P->y,int1P->zSegment1) ; 
         }
      }
   }
/*
** Allocate Memory For Connect Lines Table
*/
 *numConnectLinesP = numLineStrings ;
 *connectLinesPP = ( DTM_CONNECT_LINE * ) malloc( *numConnectLinesP * sizeof(DTM_CONNECT_LINE)) ;
 if( *connectLinesPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Allocate Memory For Connect Points Table
*/
 *numConnectPointsP = numLineStrings * 2 ;
 *connectPointsPP = ( DTM_CONNECT_POINT * ) malloc( *numConnectPointsP * sizeof(DTM_CONNECT_POINT)) ;
 if( *connectPointsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Populate Connect Lines And Connect Points Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Connect Lines And Connect Points Table") ;
 for( lineNum = 0 , pntArrayPP = lineStringsPP ; pntArrayPP < lineStringsPP + numLineStrings ; ++lineNum , ++pntArrayPP )
   {
/*
**  Set Point Numbers
*/
    pnt1 = lineNum * 2 ;
    pnt2 = lineNum * 2 + 1 ;
/*
**  Calulate Line Length
*/
    lineLength = 0 ;
    for( p3dP = (*pntArrayPP)->pointsP + 1 ; p3dP <  (*pntArrayPP)->pointsP + (*pntArrayPP)->numPoints ; ++p3dP )
      {
       dx = p3dP->x - (p3dP-1)->x ; 
       dy = p3dP->y - (p3dP-1)->y ; 
       lineLength = lineLength + sqrt( dx*dx + dy*dy) ;
      }   
/*
**  Populate Connect Line Table
*/
    (*connectLinesPP+lineNum)->line      = lineNum ;
    (*connectLinesPP+lineNum)->point1    = pnt1 ;
    (*connectLinesPP+lineNum)->point2    = pnt2 ;
    (*connectLinesPP+lineNum)->isReversed = 0 ;
    (*connectLinesPP+lineNum)->length    = lineLength ;
/*
**  Populate Connect Points Table
*/
    (*connectPointsPP+lineNum*2)->line   = lineNum ;
    (*connectPointsPP+lineNum*2)->point1 = pnt1 ;
    (*connectPointsPP+lineNum*2)->point2 = pnt2 ;
    (*connectPointsPP+lineNum*2)->isReversed = 0 ; 
    (*connectPointsPP+lineNum*2)->numConLine = 0 ;
    (*connectPointsPP+lineNum*2)->conLineP   = NULL ;

    (*connectPointsPP+lineNum*2+1)->line   = lineNum ;
    (*connectPointsPP+lineNum*2+1)->point1 = pnt2 ;
    (*connectPointsPP+lineNum*2+1)->point2 = pnt1 ;
    (*connectPointsPP+lineNum*2+1)->isReversed = 1 ;
    (*connectPointsPP+lineNum*2+1)->numConLine = 0 ;
    (*connectPointsPP+lineNum*2+1)->conLineP   = NULL ;
   }
/*
** Add Connection Points To Points Table
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Adding Connected Points To Connect Point Table") ;
  con1P = conOffsetsP ;
  for( pnt1 = 0 , pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pnt1 , ++pointP )
    {
/*
**   Set Connect Point Coordinates
*/
      if( pnt1 % 2 == 0 ) 
        {
         pntX = (*(lineStringsPP+pnt1/2))->pointsP->x ;
         pntY = (*(lineStringsPP+pnt1/2))->pointsP->y ;
        }
      else
        {
         pntX = ((*(lineStringsPP+pnt1/2))->pointsP+(*(lineStringsPP+pnt1/2))->numPoints-1)->x ;
         pntY = ((*(lineStringsPP+pnt1/2))->pointsP+(*(lineStringsPP+pnt1/2))->numPoints-1)->y ;
        }
     if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Point[%4ld] ** pntX = %12.5lf pntY = %12.5lf ** con1P->pnt1 = %6ld ",pnt1,pntX,pntY,con1P->pnt1) ;
/*
**   Get Set Of Connecting Points
*/ 
     if( con1P->pnt1 == pnt1 )
       {
        con2P = con1P + 1 ;
        while( con2P < conOffsetsP + numConOfs && con2P->pnt1 == pnt1 ) ++con2P ; 
        --con2P ;
        numConnectPts = (long) ( con2P - con1P ) + 1 ;
/*
**      Allocate Memory For Connecting Points
*/
        conLinesP = (DTM_CONNECTION_LINE * ) malloc ( numConnectPts * sizeof(DTM_CONNECTION_LINE)) ;
        if( conLinesP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
/*
**      Polulate Connecting Points
*/
        conLineP = conLinesP ;
        for( conOfsP = con1P ; conOfsP <= con2P ; ++conOfsP )
          {
           conLineP->fromPoint = pnt1 ;
           conLineP->toPoint   = conOfsP->pnt2 ;
           conLineP->line      = conOfsP->pnt2 / 2 ;
           conLineP->isMarked  = 0 ;
           conLineP->numIntConLine = 0 ;
           conLineP->intConLineP = NULL ;
/*
**         Set Connected Point Coordinates
*/
           if( conOfsP->pnt2 % 2 == 0 ) 
             {
              conPntX = (*(lineStringsPP+conOfsP->pnt2/2))->pointsP->x ;
              conPntY = (*(lineStringsPP+conOfsP->pnt2/2))->pointsP->y ;
             }
           else
             {
              conPntX = ((*(lineStringsPP+conOfsP->pnt2/2))->pointsP+(*(lineStringsPP+conOfsP->pnt2/2))->numPoints-1)->x ;
              conPntY = ((*(lineStringsPP+conOfsP->pnt2/2))->pointsP+(*(lineStringsPP+conOfsP->pnt2/2))->numPoints-1)->y ;
             }
/*
**         Set Distance To Connected Point
*/
           conLineP->distance = bcdtmMath_distance(pntX,pntY,conPntX,conPntY) ;
           ++conLineP ;
          } 
/*
**      Sort Connection Points
*/
        qsortCPP(conLinesP,numConnectPts,sizeof(DTM_CONNECTION_LINE),bcdtmConnect_connectionLineDistanceCompareFunction) ;
/*
**      Update Connection Point
*/
        pointP->conLineP   = conLinesP ;
        pointP->numConLine = numConnectPts ;
        conLinesP = NULL ;
/*
**      Set Pointer To Connection Offsets For Next Iteration
*/
        con1P = con2P + 1 ;
       }
    }
/*
** Write Total Intersections For each Connection Line
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Connection Line Intersects For Each Connection Line") ;
    numConnections = 0 ;
    intP = intPtsP ;
    for( int1P = intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
      {
       int2P = int1P + 1 ;
       while( int2P < intPtsP + numIntPts && int2P->string1Offset == int1P->string1Offset && int2P->segment1Offset == int1P->segment1Offset ) ++int2P ;
       --int2P ;
       if( intP->string1Offset == int1P->string1Offset ) numConnections = numConnections + (long)(int2P-int1P)+1 ;
       else
         {
          bcdtmWrite_message(0,0,0,"Total Connection Line Intersect From Connection Point %6ld          = %6ld",intP->string1Offset,numConnections) ;
          numConnections = (long)(int2P-int1P)+1 ;
          intP = int1P ;
         }
       bcdtmWrite_message(0,0,0,"Connection Line %6ld %6ld ** Number Of Connection Line Intersects = %6ld",int1P->string1Offset,int1P->segment1Offset,(long)(int2P-int1P)+1) ;
       int1P = int2P ;
      }
    bcdtmWrite_message(0,0,0,"Total Connection Line Intersect For Connection Line %6ld  = %6ld",intP->string1Offset,intP->segment1Offset,numConnections) ;
   }
/*
**  Add Intersected Connected Point Connections To Connected Points Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Updating Connection Lines With Intersecting Connection Lines") ;
 for( int1P = intPtsP ; int1P < intPtsP + numIntPts ; ++int1P )
   {
/*
**  Scan To End Of Intersections For Connection Line
*/
    int2P = int1P ;
    while( int2P < intPtsP + numIntPts && int2P->string1Offset == int1P->string1Offset &&  int2P->segment1Offset == int1P->segment1Offset ) ++int2P ;
    --int2P ;
/*
**  Write Connection Line
*/
   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Connection Line = %6ld %6ld Number Of Connection Line Intersects = %6ld",int1P->string1Offset,int1P->segment1Offset,(long)(int2P-int1P)+1) ;
/*
**  Find Connection Line Offset To Check If It Is valid
*/
    ofsConLine = -1 ;
    pointP  = *connectPointsPP + int1P->string1Offset ;      
    for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine &&  ofsConLine == -1 ; ++conLineP )
      {
       if( conLineP->toPoint == int1P->segment1Offset ) ofsConLine = (long)(conLineP-pointP->conLineP) ;
      }
/*
**  Find Intersects With Other Line Connection
*/
    if( ofsConLine >= 0 )
      {
/*
**     Count Number Of Intersected Connection Lines
*/   
       numConIntPts = (long)(int2P-int1P) + 1 ;
       intConLinesP = ( DTM_CONNECTION_LINE_INTERSECT * ) malloc ( numConIntPts * sizeof(DTM_CONNECTION_LINE_INTERSECT)) ;
       if( intConLinesP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
/*
**     Scan Connection Line Intersects For Valid Connection Lines
*/
       numConIntPtsFnd = 0 ;
       intConLineP = intConLinesP ;
       for( intP = int1P ; intP <= int2P ; ++intP) 
         {
/*
**        Find Offset For Intersected Connected Line
*/
          ofsIntConLine = -1 ;
          pointP  = *connectPointsPP + intP->string2Offset  ;      
          for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine && ofsIntConLine == -1 ; ++conLineP )
            {
             if( conLineP->toPoint == intP->segment2Offset ) ofsIntConLine = (long)(conLineP-pointP->conLineP) ;
            }
/*
**        If Offset Found Increment Counter And Pointer
*/
          if( ofsIntConLine >= 0 )
            {
             intConLineP->point1 = intP->string2Offset  ;
             intConLineP->point2 = intP->segment2Offset ;
             intConLineP->index  = ofsIntConLine ;
             ++numConIntPtsFnd ; 
             ++intConLineP ;
            }
         }
/*
**     Write Connection Line
*/
      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Connection Line = %6ld %6ld Number Of Intersects = %6ld Number Of Intersects Found = %6ld",int1P->string1Offset,int1P->segment1Offset,(long)(int2P-int1P)+1,numConIntPtsFnd) ;
/*
**     Check Correct Number Of Intersects Found
*/
       if( numConIntPtsFnd != (long)(int2P-int1P)+1 )
         {
          bcdtmWrite_message(2,0,0,"Correct Number Of Intersects Not Found For Connection Line") ;
          goto errexit ;
         }
/*
**     If No Intersects Found Free Memory
*/
       if( ! numConIntPtsFnd  )
         {
          free(intConLinesP) ;
          intConLinesP = NULL ;
          numConIntPts = 0 ;
         }  
/*
**     Reallocate Memory And Update Connected Lines
*/
       else
         {
          if( numConIntPtsFnd < numConIntPts ) 
            {
             numConIntPts = numConIntPtsFnd ;
             intConLinesP = ( DTM_CONNECTION_LINE_INTERSECT * ) realloc (intConLinesP,numConIntPts * sizeof(DTM_CONNECTION_LINE_INTERSECT)) ; 
            }
/*
**        Update Connected Lines
*/
          conLineP = (*connectPointsPP + int1P->string1Offset)->conLineP + ofsConLine ;  
          conLineP->intConLineP   = intConLinesP ;
          conLineP->numIntConLine = numConIntPts ;
          intConLinesP = NULL ;
          numConIntPts = 0 ;
         }
      }
/*
**  Set For Next Connect Line
*/
    int1P = int2P ;
   }
/*
**  Write Out Connect Lines And Connect Points List
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Connect Lines = %4ld",*numConnectLinesP ) ;
    for( lineP = *connectLinesPP ; lineP < *connectLinesPP + *numConnectLinesP ; ++lineP )
      {
       bcdtmWrite_message(0,0,0,"Line[%4ld] = line = %4ld ** pnt1 = %4ld pnt2 = %4ld isRev = %2ld ** len = %10.4lf",(long)(lineP-*connectLinesPP),lineP->line,lineP->point1,lineP->point2,lineP->isReversed,lineP->length) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Connect Points = %4ld",*numConnectPointsP ) ;
    for( pointP = *connectPointsPP ; pointP < *connectPointsPP + *numConnectPointsP ; ++pointP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] ** line = %4ld ** pnt1 = %4ld pnt2 = %4ld isRev = %2ld ** numConnectPts = %4ld",(long)(pointP-*connectPointsPP),pointP->line,pointP->point1,pointP->point2,pointP->isReversed,pointP->numConLine) ;
//       if( pointP->numConLine > 0 )
//         {
//          for( conLineP = pointP->conLineP ; conLineP < pointP->conLineP + pointP->numConLine ; ++conLineP ) 
//            {
//             bcdtmWrite_message(0,0,0,"**** Connected Point[%4ld] ** pnt = %8ld line = %4ld distance = %10.4lf",(long)(conLineP-pointP->conLineP),conLineP->toPoint,conLineP->line,conLineP->distance) ;
//            }  
//         } 
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( localStringsPP != NULL )  bcdtmMem_freePointerArrayToPointArrayMemory(&localStringsPP,numLocalStrings) ;
 if( intPtsP        != NULL )  free(intPtsP) ; 
 if( newLineP != NULL )
   {
    if( newLineP->pointsP != NULL ) free(newLineP->pointsP) ;
    free(newLineP) ;
   }
 if( conLinesP     != NULL ) free(conLinesP) ;
 if( intConLinesP  != NULL ) free(intConLinesP)    ;
 if( conOffsetsP   != NULL ) free(conOffsetsP)   ;
 if( conLineIndexP != NULL ) free(conLineIndexP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Connect Tables From Strings Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Building Connect Tables From Strings Error") ;
 return( ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numConnectLinesP  = 0 ;
 *numConnectPointsP = 0 ;
 if( *connectLinesPP  != NULL ) { free(*connectLinesPP)  ; *connectLinesPP  = NULL ; }
 if( *connectPointsPP != NULL ) { free(*connectPointsPP) ; *connectPointsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmConnect_lineStringsUsingConnectionTables
(
 DTM_CONNECT_LINE     *connectLinesP,            /* ==> Pointer To Connect Lines    */
 long                 numConnectLines,           /* ==> Number Of Connect Lines     */
 DTM_CONNECT_POINT    *connectPointsP,           /* ==> Pointer To Connect Points   */
 long                 numConnectPoints,          /* ==> Number Of Connect Points    */
 DTM_CONNECTED_STRING **connectStringPP,         /* <== Pointer To Connected String */
 long                 *numConnectStringP,        /* <== Number Of Connected Strings */
 int                  *connectResultP,           /* <== Connection Result           */
 int                  *connectCloseP,            /* <== Connection Can Close        */
 double               *connectLengthP            /* <== Connection Lenghth          */
)
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      line,result,close,startPnt,numConString,numSolutions=0,totalNumSolutions=0 ;
 double    length ;
 DTM_CONNECT_LINE     *lineP  ;
 DTM_CONNECT_POINT    *conPntP ;
 DTM_CONNECTION_LINE  *conLineP ;
 DTM_CONNECTED_STRING *conStringP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Connecting Lines Using Connection Tables") ;
    bcdtmWrite_message(0,0,0,"connectLinesP    = %p",*connectLinesP) ;
    bcdtmWrite_message(0,0,0,"numConnectLines  = %8ld",numConnectLines) ;
    bcdtmWrite_message(0,0,0,"connectPointsP   = %p",*connectPointsP) ;
    bcdtmWrite_message(0,0,0,"numConnectPoints = %8ld",numConnectPoints) ;
   } 
/*
**  Write Out Connect Lines And Connect Points List
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Connect Lines = %4ld",numConnectLines ) ;
    for( lineP = connectLinesP ; lineP < connectLinesP + numConnectLines ; ++lineP )
      {
       bcdtmWrite_message(0,0,0,"Line[%4ld] = line = %4ld  pnt1 = %4ld pnt2 = %4ld isRev = %2ld  len = %10.4lf",(long)(lineP-connectLinesP),lineP->line,lineP->point1,lineP->point2,lineP->isReversed,lineP->length) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Connect Points = %4ld",numConnectPoints ) ;
    for( conPntP = connectPointsP ; conPntP < connectPointsP + numConnectPoints ; ++conPntP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld]  line = %4ld  pnt1 = %4ld pnt2 = %4ld isRev = %2ld  connectPtsP = %p numConnectPts = %4ld",(long)(conPntP-connectPointsP),conPntP->line,conPntP->point1,conPntP->point2,conPntP->isReversed,conPntP->conLineP,conPntP->numConLine) ;
       if( conPntP->numConLine > 0 )
         {
          for( conLineP = conPntP->conLineP ; conLineP < conPntP->conLineP + conPntP->numConLine ; ++conLineP ) 
            {
             bcdtmWrite_message(0,0,0," Connected Point[%4ld]  pnt = %8ld line = %4ld distance = %10.4lf numIntConPts = %6ld",(long)(conLineP-conPntP->conLineP),conLineP->toPoint,conLineP->line,conLineP->distance,conLineP->numIntConLine) ;
            }  
         } 
      }
   }
/*
** Initialise Return Arguments
*/
 *connectResultP    = 1   ; 
 *connectCloseP     = 0   ;
 *connectLengthP    = 0.0 ;
 *numConnectStringP = 0   ;
 if( *connectStringPP != NULL ) { free(*connectStringPP) ; *connectStringPP = NULL ; }
/*
** Check For Zero Length Connection Lines
*/
 for( conPntP = connectPointsP ; conPntP < connectPointsP + numConnectPoints ; ++conPntP )
   {
    if( conPntP->numConLine > 1 && conPntP->conLineP->distance == 0.0 ) 
      {
       if( (conPntP->conLineP+1)->distance == 0.0 ) 
         {
          *connectResultP = 1 ;
          bcdtmWrite_message(1,0,0,"More Than Two Coincident Connect Line End Points") ;
          goto errexit ;  
         }
       conPntP->numConLine = 1 ;
      }
   }
/*
** Scan Each Connect Line And Try And Connect Lines
*/
 for( lineP = connectLinesP  ; lineP < connectLinesP +  numConnectLines  ; ++lineP  )
   {
    for( line = 0 ; line < 2 ; ++line )
      {
       if( line == 0 ) startPnt = lineP->point1 ;
       else            startPnt = lineP->point2 ;
       if( bcdtmConnect_lineStringsFromStartPoint(connectLinesP,numConnectLines,connectPointsP,numConnectPoints,startPnt,&numSolutions,&conStringP,&numConString,&result,&close,&length)) goto errexit ;
       if( ! result )
         {
          ++totalNumSolutions ;  
          if( *connectResultP != 0  || length < *connectLengthP )
            {
             *connectResultP    = 0 ; 
             *connectCloseP     = close ;
             *connectStringPP   = conStringP ;
             *numConnectStringP = numConString ;
             *connectLengthP    = length ;
             conStringP         = NULL   ;
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Total Number Of Solutions = %6ld",totalNumSolutions) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Lines Using Connection Tables Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Lines Using Connection Tables Error") ;
 return( ret) ;
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
BENTLEYDTM_Private int bcdtmConnect_lineStringsFromStartPoint
(
 DTM_CONNECT_LINE     *connectLinesP,            /* ==> Pointer To Connect Lines               */
 long                 numConnectLines,           /* ==> Number Of Connect Lines                */
 DTM_CONNECT_POINT    *connectPointsP,           /* ==> Pointer To Connect Points              */
 long                 numConnectPoints,          /* ==> Number Of Connect Points               */
 long                 startPoint,                /* ==> Start Point Offset To Begin Connection */ 
 long                 *numSolutionsP,            /* <== Number Of Solutions Found              */
 DTM_CONNECTED_STRING **connectStringPP,         /* <== Pointer To Connected String            */
 long                 *numConnectStringP,        /* <== Number Of Connected Strings            */
 long                 *connectResultP,           /* <== Connection Result                      */
 long                 *connectCloseP,            /* <== Connection Can Close                   */
 double               *connectLengthP            /* <== Connection Lenghth                     */
)
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      startPnt,lastPnt,prevPnt,nodePnt,nextPnt,solutionFound,allScanned,numConString ;
 long      line,pnt1,pnt2,nodeFound,*longP,*lineMarkP=NULL,startScanFound,minClose=0 ;
 double    length,distance,minLength=0.0 ;
 DTM_CONNECT_POINT    *conPntP ;
 DTM_CONNECTION_LINE  *conLineP ;
 DTM_CONNECTED_STRING *conStrP,*conStringP=NULL,*minConStringP=NULL ;
 DTM_CONNECTION_LINE_INTERSECT *intLineP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Connecting Line Strings From Start Points") ;
    bcdtmWrite_message(0,0,0,"connectLinesP    = %p",*connectLinesP) ;
    bcdtmWrite_message(0,0,0,"numConnectLines  = %8ld",numConnectLines) ;
    bcdtmWrite_message(0,0,0,"connectPointsP   = %p",*connectPointsP) ;
    bcdtmWrite_message(0,0,0,"numConnectPoints = %8ld",numConnectPoints) ;
    bcdtmWrite_message(0,0,0,"startPoint       = %8ld",startPoint) ;
   } 
/*
** Initialise Return Arguments
*/
 *numSolutionsP     = 0   ;
 *connectResultP    = 1   ; 
 *connectCloseP     = 0   ;
 *connectLengthP    = 0.0 ;
 *numConnectStringP = 0   ;
 if( *connectStringPP != NULL ) { free(*connectStringPP) ; *connectStringPP = NULL ; }
/*
** Unmark All Connected Points
*/
 for( conPntP = connectPointsP ; conPntP < connectPointsP + numConnectPoints ; ++conPntP )
   {
    if( conPntP->numConLine > 0 )
      {
       for( conLineP = conPntP->conLineP ; conLineP < conPntP->conLineP + conPntP->numConLine ; ++conLineP ) conLineP->isMarked = 0 ;
      }
   }
/*
** Allocate Memory To Hold Connected String Offsets
*/
 conStringP = (DTM_CONNECTED_STRING * ) malloc ( numConnectLines * sizeof(DTM_CONNECTED_STRING)) ;
 if( conStringP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }     
/*
** Allocate Memory To Mark Processed Lines
*/
 lineMarkP = (long * ) malloc ( numConnectLines  * sizeof(long)) ;
 if( lineMarkP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }  
 for( longP = lineMarkP ; longP < lineMarkP + numConnectLines ; ++longP ) *longP = 0 ;
/*
** Initialise For Scanning
*/
 numConString = 0 ;
 distance     = 0.0 ;
 length       = 0.0 ;
 startPnt = startPoint ;
/*
**  Scan All Possible Connections From Start Line
*/
 allScanned = FALSE ;
 while ( allScanned == FALSE )
   { 
    allScanned = TRUE ; 
/*
**  Connect Strings
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"******* Forward Scanning ** numConString = %6ld startPnt = %6ld",numConString,startPnt) ;
    do
      {
       length = length + distance + (connectLinesP + (connectPointsP+startPnt)->line)->length ;
       (conStringP+numConString)->stringOffset = (connectPointsP+startPnt)->line ;
       if( startPnt % 2 == 0 ) (conStringP+numConString)->isReversed = 0 ;
       else                    (conStringP+numConString)->isReversed = 1 ;
       *(lineMarkP+(connectPointsP+startPnt)->line) = 1 ;
       ++numConString ;
       nextPnt = (connectPointsP+startPnt)->point2 ; 
       if( dbg ) bcdtmWrite_message(0,0,0,"numConString = %2ld length = %12.3lf ** startPnt  = %6ld line = %6ld nextPnt = %6ld",numConString,length,startPnt,(connectPointsP+startPnt)->line,nextPnt) ;
      }   while ( numConString < numConnectLines && bcdtmConnect_lineStringsGetNextConnectPoint(connectPointsP,lineMarkP,nextPnt,&startPnt,&distance))  ;
/*
**  Write Out Connect String
*/
    if( dbg  )
      {
       bcdtmWrite_message(0,0,0,"Number Of Connected Lines = %6ld **  Connected Length = %12.4lf",numConString,length) ; 
       for( conStrP = conStringP ; conStrP < conStringP + numConString ; ++conStrP )
         {
          if( conStrP->isReversed == 0 ) { pnt1 = conStrP->stringOffset * 2     ; pnt2 = pnt1 + 1 ; }
          else                          { pnt1 = conStrP->stringOffset * 2 + 1 ; pnt2 = pnt1 - 1 ; }
          bcdtmWrite_message(0,0,0,"Line[%4ld] ** pnt1 = %6ld line = %6ld pnt2  = %6ld mark = %2ld",(long)(conStrP-conStringP),pnt1,conStrP->stringOffset,pnt2,*(lineMarkP+conStrP->stringOffset)) ;
         }
      } 
/*
**  Test For Solution
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Solution numConString = %6ld numConnectLines = %6ld",numConString,numConnectLines) ;
    solutionFound = FALSE ;
    if( numConString == numConnectLines ) solutionFound = TRUE ;
/*
**  Set Minimum Length Solution
*/
    if( solutionFound == TRUE )
      {
       ++numSolutionsP ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Solutions Found = %6ld ** length = %12.5lf",*numSolutionsP,length) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Solution Found ** length %12.4lf",length) ;
       if( minConStringP == NULL || length < minLength )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Minimum Solution Found") ;
/*
**        Allocate Memory To Hold Minumum Length Solution
*/
          if( minConStringP == NULL )
            {
             minConStringP = (DTM_CONNECTED_STRING * ) malloc ( numConnectLines * sizeof(DTM_CONNECTED_STRING)) ;
             if( minConStringP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
            }
/*
**        Check For Closing String
*/
          minClose = 0 ;
          line = (conStringP+numConString-1)->stringOffset ;
          if( (conStringP+numConString-1)->isReversed == 0 ) lastPnt = line * 2 + 1 ; // Set To Opposite End Of Line
          else                                               lastPnt = line * 2     ; // Set To Opposite End Of Line
          for( conLineP = (connectPointsP+startPoint)->conLineP ; conLineP < (connectPointsP+startPoint)->conLineP + (connectPointsP+startPoint)->numConLine && minClose == 0 ; ++conLineP )
            {
             if( conLineP->toPoint == lastPnt && ! conLineP->isMarked ) minClose = 1 ;
            } 
/*
**        Copy Solution To Minimum Solution
*/
          memcpy(minConStringP,conStringP,numConnectLines*sizeof(DTM_CONNECTED_STRING)) ;
          minLength = length ;
         }   
      }
/*
**  Post Order Traverse Connection Tree Looking For Alternative Connection Paths
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Post Order Traversing Connect Tree") ;
    startScanFound = FALSE ;
    while ( numConString > 1 && startScanFound == FALSE )
      { 
/*
**     Set Node Point
*/
       --numConString ;
       line = (conStringP+numConString)->stringOffset ;
       if( (conStringP+numConString)->isReversed == 0 ) nodePnt = line * 2     ; 
       else                                             nodePnt = line * 2 + 1 ; 
       if( dbg ) bcdtmWrite_message(0,0,0,"Current Line  = %6ld Node = %6ld",line,nodePnt) ;
       *(lineMarkP+line) = 0 ;
       length = length - (connectLinesP+line)->length ;
/*
**     Set Previous Node
*/ 
       line = (conStringP+numConString-1)->stringOffset ;
       if( (conStringP+numConString-1)->isReversed == 0 ) prevPnt = line * 2 + 1 ; // Set To Opposite End Of Line
       else                                               prevPnt = line * 2     ; // Set To Opposite End Of Line
       if( dbg ) bcdtmWrite_message(0,0,0,"Previous Line = %6ld Node = %6ld",line,prevPnt) ;
/*
**     Check If Scan Can Be Restarted Fron Previous Point
*/
       nodeFound = 0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point %6ld For Connection Lines",prevPnt) ;
       for( conLineP = (connectPointsP+prevPnt)->conLineP ; conLineP <  (connectPointsP+prevPnt)->conLineP + (connectPointsP+prevPnt)->numConLine && startScanFound == FALSE ; ++conLineP )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"point = %6ld line = %6ld isMarked = %6ld lineMark = %2ld",conLineP->toPoint,conLineP->line,conLineP->isMarked,*(lineMarkP+conLineP->line)) ;
          if( nodeFound && ! conLineP->isMarked && ! *(lineMarkP+conLineP->line) ) 
            {
             startScanFound = TRUE ;
             startPnt = conLineP->toPoint ; 
             distance = conLineP->distance ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Scan Start From startPnt = %6ld numConString = %6ld length = %12.4lf",startPnt,numConString,length) ;
/*
**           Mark Connection Lines Intersected By This Connection Line
*/ 
             if( dbg ) bcdtmWrite_message(0,0,0,"Marking Connection Line Intersections With Connect Line %6ld %6ld",prevPnt,startPnt) ;
             for( intLineP = conLineP->intConLineP ; intLineP < conLineP->intConLineP + conLineP->numIntConLine ; ++intLineP )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Marking Connection Line %6ld %6ld ** %3ld",intLineP->point1,intLineP->point2,((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked) ;
                ++((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked ;
               }
            }
/*
**        Check For node point
*/
          if( conLineP->toPoint == nodePnt ) 
            {
             nodeFound = 1 ;
             length = length - conLineP->distance ;
            }
         } 
/*
**     Un Mark Connection Lines Intersected By Removed Connection
*/
       if( startScanFound == FALSE ) 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Unmarking Connection Line Intersections With Connection Line %6ld %6ld",prevPnt,nodePnt) ;
          for( conLineP = (connectPointsP+prevPnt)->conLineP ; conLineP <  (connectPointsP+prevPnt)->conLineP + (connectPointsP+prevPnt)->numConLine ; ++conLineP )
            {
             if( conLineP->toPoint == nodePnt )
               {
                for( intLineP = conLineP->intConLineP ; intLineP < conLineP->intConLineP + conLineP->numIntConLine ; ++intLineP )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Unmarking Connection Line %6ld %6ld ** %3ld",intLineP->point1,intLineP->point2,((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked) ;
                   if( ((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked == 0 ) 
                     {
                      bcdtmWrite_message(0,0,0,"Connect Line %6ld %6ld isMark = %6ld",intLineP->point1,intLineP->point2,((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked) ;
                      goto errexit ;
                     }
                   --((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked ;
                  }
               }
            }
         }
      }
/*
**  Check For Continuing Post Order Scan
*/
    if( startScanFound == TRUE ) 
      {
       allScanned = FALSE ;  
      }
   }
/*
** Set Return Values
*/
 if( minConStringP != NULL )
   {
    *connectResultP    = 0   ; 
    *connectCloseP     = minClose  ;
    *connectLengthP    = minLength ;
    *numConnectStringP = numConnectLines  ;
    *connectStringPP   = minConStringP ;
    minConStringP      = NULL ;
   }
/*
** Cleanup
*/
 cleanup :
 if( lineMarkP     != NULL ) { free(lineMarkP)     ; lineMarkP     = NULL ; }
 if( conStringP    != NULL ) { free(conStringP)    ; conStringP    = NULL ; }
 if( minConStringP != NULL ) { free(minConStringP) ; minConStringP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Line Strings From Start Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Connecting Line Strings From Start Point Error") ;
 return( ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numConnectStringP = 0   ;
 if( *connectStringPP != NULL ) { free(*connectStringPP) ; *connectStringPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmConnect_lineStringsGetNextConnectPoint
(
 DTM_CONNECT_POINT  *connectPointsP,
 long   *lineMarkP,
 long   point,
 long   *nextPointP,
 double *lengthP
)
{
 int  nextPntFnd=FALSE,dbg=DTM_TRACE_VALUE(0) ;
 long numConLines ;
 DTM_CONNECTION_LINE *conLineP,*conLinesP ;
 DTM_CONNECTION_LINE_INTERSECT *intLineP ;
/*
** Scan Connected Points
*/
 conLinesP   = (connectPointsP+point)->conLineP ;
 numConLines = (connectPointsP+point)->numConLine ;
 for( conLineP = conLinesP ; conLineP < conLinesP + numConLines &&  nextPntFnd == FALSE  ; ++conLineP )
   {
    if( ! conLineP->isMarked && ! *(lineMarkP+conLineP->line) )
      {
/*
**     Set Connection Line
*/
       *nextPointP = conLineP->toPoint    ;
       *lengthP    = conLineP->distance ;
       nextPntFnd  = TRUE ;
/*
**     Mark Connection Lines Intersected By This Connection Line
*/
       if( conLineP->numIntConLine > 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking Connection Line Intersections With Connect Line %6ld %6ld",point,*nextPointP) ;
          for( intLineP = conLineP->intConLineP ; intLineP < conLineP->intConLineP + conLineP->numIntConLine ; ++intLineP )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Marking Connection Line %6ld %6ld ** %3ld",intLineP->point1,intLineP->point2,((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked) ;
             ++((connectPointsP+intLineP->point1)->conLineP+intLineP->index)->isMarked ;
            }
         } 
      }
   }
/*
** Job Completed
*/
 return( nextPntFnd ) ;
}
