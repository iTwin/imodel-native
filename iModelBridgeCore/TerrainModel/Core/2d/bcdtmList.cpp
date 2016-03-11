/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmList.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#ifdef NEW
BENTLEYDTM_Public long bcdtmList_nextClkDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 int dbg=DTM_TRACE_VALUE(0);
 long clpPtr,clhPtr ;
 DTM_CIR_LIST *cListP ;
/*
** Test Point Values Are In Correct Range
*/
 if( dbg && (p1 >= dtmP->numPoints || p2 >= dtmP->numPoints || p1 < 0 || p2 < 0 ))
   {
    bcdtmWrite_message(2,0,0,"Dtm nextClk Point Range Error ** p1 = %9ld p2 = %9ld",p1,p2) ;
    return(-99) ;
   }
/*
** Scan Circular List
*/
 clpPtr = clhPtr = nodeAddrP(dtmP,p1)->cPtr ;
 while( clpPtr != dtmP->nullPtr )
   {
    cListP = clistAddrP(dtmP,clpPtr) ;
    if( cListP->pntNum == p2 )
      {
       clpPtr = cListP->nextPtr ;
       if( clpPtr == dtmP->nullPtr ) return ( clistAddrP(dtmP,clhPtr)->pntNum ) ;
       else                    return ( clistAddrP(dtmP,clpPtr)->pntNum ) ;
      }
    clpPtr = cListP->nextPtr ;
   }
/*
** Topology Error In Circular List
*/
 bcdtmWrite_message(2,0,0,"Circular List Error Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT long bcdtmList_nextAntDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 int dbg = 0;
 long clpPtr,clhPtr ;
 DTM_CIR_LIST *cListP, *cListHP ;
/*
** Test Point Values Are In Correct Range
*/
 if( dbg && (p1 >= dtmP->numPoints || p2 >= dtmP->numPoints || p1 < 0 || p2 < 0 ))
   {
    bcdtmWrite_message(2,0,0,"Dtm nextAnt Point Range Error ** p1 = %9ld p2 = %9ld",p1,p2) ;
    return(-99) ;
   }
/*
** Scan Circular List
*/
 cListHP = NULL;
 clpPtr = nodeAddrP(dtmP,p1)->cPtr ;
 while ( clpPtr != dtmP->nullPtr )
   {
    cListP = clistAddrP(dtmP,clpPtr) ;
    if( cListP->pntNum == p2 )
      {
       if( cListHP != NULL) return( cListHP->pntNum ) ;
       clpPtr = cListP->nextPtr;
       if (clpPtr != dtmP->nullPtr)
           while( (clhPtr = (cListP = clistAddrP(dtmP,clpPtr))->nextPtr) != dtmP->nullPtr ) clpPtr = clhPtr ;
       return( cListP->pntNum ) ;
      }
    cListHP = cListP;
    clpPtr = cListP->nextPtr ;
   }
/*
** Topology Error In Circular List
*/
 bcdtmWrite_message(2,0,0,"Circular List Error Counter Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
#else
BENTLEYDTM_Public long bcdtmList_nextClkDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 long clpPtr,clhPtr ;
 DTM_CIR_LIST *cListP ;
/*
** Test Point Values Are In Correct Range
*/
 if( p1 >= dtmP->numPoints || p2 >= dtmP->numPoints || p1 < 0 || p2 < 0 )
   {
    bcdtmWrite_message(2,0,0,"Dtm nextClk Point Range Error ** p1 = %9ld p2 = %9ld",p1,p2) ;
    return(-99) ;
   }
/*
** Scan Circular List
*/
 clpPtr = clhPtr = nodeAddrP(dtmP,p1)->cPtr ;
 while( clpPtr != dtmP->nullPtr )
   {
    cListP = clistAddrP(dtmP,clpPtr) ;
    if( cListP->pntNum == p2 )
      {
       clpPtr = clistAddrP(dtmP,clpPtr)->nextPtr ;
       if( clpPtr == dtmP->nullPtr ) return ( clistAddrP(dtmP,clhPtr)->pntNum ) ;
       else                          return ( clistAddrP(dtmP,clpPtr)->pntNum ) ;
      }
    clpPtr = cListP->nextPtr ;
   }
/*
** Topology Error In Circular List
*/
 bcdtmWrite_message(2,0,0,"Circular List Error Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT long bcdtmList_nextAntDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 long clpPtr,clhPtr ;
 DTM_CIR_LIST *cListP ;
/*
** Test Point Values Are In Correct Range
*/
 if( p1 >= dtmP->numPoints || p2 >= dtmP->numPoints || p1 < 0 || p2 < 0 )
   {
    bcdtmWrite_message(2,0,0,"Dtm nextAnt Point Range Error ** p1 = %9ld p2 = %9ld",p1,p2) ;
    return(-99) ;
   }
/*
** Scan Circular List
*/
 clhPtr = DTM_NULL_PTR ;
 clpPtr = nodeAddrP(dtmP,p1)->cPtr ;
 while ( clpPtr != DTM_NULL_PTR )
   {
    cListP = clistAddrP(dtmP,clpPtr) ;
    if( cListP->pntNum == p2 )
      {
       if( clhPtr != DTM_NULL_PTR ) return( clistAddrP(dtmP,clhPtr)->pntNum ) ;
       while( clistAddrP(dtmP,clpPtr)->nextPtr != dtmP->nullPtr ) clpPtr = clistAddrP(dtmP,clpPtr)->nextPtr  ;
       return( clistAddrP(dtmP,clpPtr)->pntNum ) ;
      }
    clhPtr = clpPtr ;
    clpPtr = cListP->nextPtr ;
   }
/*
** Topology Error In Circular List
*/
 bcdtmWrite_message(2,0,0,"Circular List Error Counter Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_insertLineDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
/*
** This Function Inserts Points into the Circular List
*/
{
 int ret=DTM_SUCCESS ;
/*
** Insert Into Circular List
*/
 if( bcdtmList_addLineDtmObject(dtmP,p1,p2) ) goto errexit ;
 if( bcdtmList_addLineDtmObject(dtmP,p2,p1) ) goto errexit ;
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
BENTLEYDTM_Public int bcdtmList_addLineDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 int     ret=DTM_SUCCESS ;
 int     sd2,sd3,sd4 ;
 long    cl1Ptr,cl2Ptr,cl3Ptr ;
 DTM_CIR_LIST *cListP ;
/*
** Allocate Storage for New Entry
*/
 if( dtmP->cListDelPtr != dtmP->nullPtr )
   {
    cl1Ptr = dtmP->cListDelPtr ;
    dtmP->cListDelPtr = clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr ;
   }
 else
   {
    cl1Ptr = dtmP->cListPtr ;
    ++dtmP->cListPtr ;
    if( dtmP->cListPtr > dtmP->numPoints * 6 )
      {
       bcdtmWrite_message(1,0,0,"Not Enough Memory For Circular List") ;
       goto errexit  ;
      }
   }
/*
** Set New Entry Values
*/
 cListP = clistAddrP(dtmP,cl1Ptr)  ;
 cListP->pntNum  = p2 ;
 cListP->nextPtr = dtmP->nullPtr ;
 clistAddrP(dtmP,cl1Ptr)->nextPtr = dtmP->nullPtr  ;
 cl2Ptr = nodeAddrP(dtmP,p1)->cPtr ;
/*
** If No Entries , Insert New Entry And Return
*/
 if( cl2Ptr == dtmP->nullPtr ) nodeAddrP(dtmP,p1)->cPtr = cl1Ptr ;
 else
   {
/*
**  If Only One Entry Insert After First Entry
*/
    cl3Ptr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
    if( cl3Ptr == dtmP->nullPtr )
      {
       if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,clistAddrP(dtmP,cl1Ptr)->pntNum,clistAddrP(dtmP,cl2Ptr)->pntNum) <= 0 )
         {
          clistAddrP(dtmP,cl2Ptr)->nextPtr = cl1Ptr ;
         }
       else
         {
          nodeAddrP(dtmP,p1)->cPtr = cl1Ptr ;
          clistAddrP(dtmP,cl1Ptr)->nextPtr = cl2Ptr ;
         }
      }
    else
      {
/*
**     Scan Circular List For Insertion Point
*/
       sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,p1,clistAddrP(dtmP,cl2Ptr)->pntNum,p2) ;
       while ( cl3Ptr != dtmP->nullPtr )
         {
          sd3 = bcdtmMath_pointSideOfDtmObject(dtmP,p1,clistAddrP(dtmP,cl3Ptr)->pntNum,p2) ;
          sd4 = bcdtmMath_pointSideOfDtmObject(dtmP,clistAddrP(dtmP,cl2Ptr)->pntNum,clistAddrP(dtmP,cl3Ptr)->pntNum,p1) ;
          if( ( sd4 <= 0 && sd2 < 0 && sd3 > 0 ) ||	( sd4 >  0 && ! ( sd2 > 0 && sd3 < 0 ))   )
            {
             clistAddrP(dtmP,cl2Ptr)->nextPtr = cl1Ptr ;
             clistAddrP(dtmP,cl1Ptr)->nextPtr = cl3Ptr ;
             goto cleanup ;
            }
          sd2 = sd3 ;
          cl2Ptr = cl3Ptr ;
          cl3Ptr = clistAddrP(dtmP,cl3Ptr)->nextPtr ;
         }
       clistAddrP(dtmP,cl2Ptr)->nextPtr = cl1Ptr ;
      }
   }
/*
** Claen Up
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
BENTLEYDTM_Public int bcdtmList_deleteLineDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
/*
** This Routine Deletes A Line From the Circular List
*/
{
 int ret=DTM_SUCCESS ;
/*
** Delete From Circular List
*/
 if( bcdtmList_removeLineDtmObject(dtmP,p1,p2) ) goto errexit  ;
 if( bcdtmList_removeLineDtmObject(dtmP,p2,p1) ) goto errexit  ;
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
BENTLEYDTM_Public int bcdtmList_removeLineDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 int  ret=DTM_SUCCESS ;
 long cl1Ptr,cl2Ptr ;
/*
** Initialise Variables
*/
 cl1Ptr = dtmP->nullPtr ;
 cl2Ptr = nodeAddrP(dtmP,p1)->cPtr ;
 if( cl2Ptr == dtmP->nullPtr )
   {
    bcdtmWrite_message(1,0,0,"No Circular List For Delete Line Point") ; goto errexit  ;
   }
/*
** Scan List To Find Point to be Deleted
*/
 while( clistAddrP(dtmP,cl2Ptr)->pntNum != p2 )
   {
    cl1Ptr = cl2Ptr ;
    cl2Ptr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
   }
/*
** Delete Line
*/
 if( cl1Ptr == dtmP->nullPtr ) nodeAddrP(dtmP,p1)->cPtr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
 else                          clistAddrP(dtmP,cl1Ptr)->nextPtr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
/*
** Add Deleted Record to Delete List
*/
 clistAddrP(dtmP,cl2Ptr)->nextPtr = dtmP->nullPtr ;
 clistAddrP(dtmP,cl2Ptr)->pntNum  = dtmP->nullPnt ;
 if( dtmP->cListDelPtr == dtmP->nullPtr ) dtmP->cListDelPtr = cl2Ptr ;
 else
   {
    clistAddrP(dtmP,cl2Ptr)->nextPtr = dtmP->cListDelPtr  ;
    dtmP->cListDelPtr = cl2Ptr ;
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
BENTLEYDTM_Public int bcdtmList_insertLineBeforePointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long Bp)
{
 int  ret=DTM_SUCCESS ;
 long cl1Ptr,cl2Ptr,cl3Ptr ;
/*
** Allocate Storage for New Entry
*/
 if( dtmP->cListDelPtr != dtmP->nullPtr )
   {
    cl1Ptr = dtmP->cListDelPtr ;
    dtmP->cListDelPtr = clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr ;
   }
 else
   {
    cl1Ptr = dtmP->cListPtr ;
    ++dtmP->cListPtr ;
    if( dtmP->cListPtr > dtmP->numPoints * 6 )
      {
       bcdtmWrite_message(2,0,0,"Not Enough Memory For Circular List") ;
       goto errexit  ;
      }
   }
/*
** Set New Entry Values
*/
 clistAddrP(dtmP,cl1Ptr)->pntNum  = P2   ;
 clistAddrP(dtmP,cl1Ptr)->nextPtr  = dtmP->nullPtr  ;
 cl2Ptr = nodeAddrP(dtmP,P1)->cPtr ;
 if( cl2Ptr == dtmP->nullPtr )
   { nodeAddrP(dtmP,P1)->cPtr = cl1Ptr ; goto cleanup ; }
/*
** Scan Circular List For Insertion Point
*/
 cl3Ptr = dtmP->nullPtr ;
 while ( cl2Ptr != dtmP->nullPtr )
   {
    if( clistAddrP(dtmP,cl2Ptr)->pntNum == Bp )
      {
       if( cl3Ptr == dtmP->nullPtr ) nodeAddrP(dtmP,P1)->cPtr  = cl1Ptr ;
       else                          clistAddrP(dtmP,cl3Ptr)->nextPtr = cl1Ptr ;
       clistAddrP(dtmP,cl1Ptr)->nextPtr = cl2Ptr ;
       goto cleanup ;
      }
    cl3Ptr = cl2Ptr ;
    cl2Ptr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
   }
/*
**  Error Line Not Inserted
*/
 bcdtmWrite_message(2,0,0,"Line Before Point Insert Error") ;
 goto errexit ;
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
BENTLEYDTM_Public int bcdtmList_insertLineAfterPointDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long Ap)
{
 int   ret=DTM_SUCCESS ;
 long  cl1Ptr,cl2Ptr ;
/*
** Allocate Storage for New Entry
*/
 if( dtmP->cListDelPtr != dtmP->nullPtr )
   {
    cl1Ptr = dtmP->cListDelPtr ;
    dtmP->cListDelPtr = clistAddrP(dtmP,dtmP->cListDelPtr)->nextPtr ;
   }
 else
   {
    cl1Ptr = dtmP->cListPtr ; ++dtmP->cListPtr ;
    if( dtmP->cListPtr > dtmP->numPoints * 6 )
      {
       bcdtmWrite_message(1,0,0,"Not Enough Memory For Circular List") ;
       goto errexit  ;
      }
   }
/*
** Set New Entry Values
*/
 clistAddrP(dtmP,cl1Ptr)->pntNum  = P2   ;
 clistAddrP(dtmP,cl1Ptr)->nextPtr  = dtmP->nullPtr  ;
 cl2Ptr = nodeAddrP(dtmP,P1)->cPtr ;
 if( cl2Ptr == dtmP->nullPtr )
   { nodeAddrP(dtmP,P1)->cPtr = cl1Ptr ; goto cleanup ; }
/*
** Scan Circular List For Insertion Point
*/
 while ( cl2Ptr != dtmP->nullPtr )
   {
    if( clistAddrP(dtmP,cl2Ptr)->pntNum == Ap )
      {
       clistAddrP(dtmP,cl1Ptr)->nextPtr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
       clistAddrP(dtmP,cl2Ptr)->nextPtr = cl1Ptr ;
       goto cleanup ;
      }
    cl2Ptr = clistAddrP(dtmP,cl2Ptr)->nextPtr ;
   }
/*
**  Error Line Not Inserted
*/
 bcdtmWrite_message(2,0,0,"Line After Point Insert Error ") ;
 goto errexit ;
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
BENTLEYDTM_Public int bcdtmList_writeCircularListForPointDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Writes The Circular List For A Dtm Point
*/
{
 long   pntNum,clPtr ;
 double dx,dy,angle   ;
/*
** Header
*/
 bcdtmWrite_message(0,0,0,"**** Circular List For Point %8ld ****",point) ;
 if( point >= 0 && point < dtmP->numPoints )
   {
    bcdtmWrite_message(0,0,0,"x = %15.8lf y = %15.8lf z = %10.4lf", pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
    bcdtmWrite_message(0,0,0,"cPtr = %9ld hPtr = %9ld tPtr = %9ld sPtr = %9ld fPtr = %9ld",nodeAddrP(dtmP,point)->cPtr,nodeAddrP(dtmP,point)->hPtr,nodeAddrP(dtmP,point)->tPtr,nodeAddrP(dtmP,point)->sPtr,nodeAddrP(dtmP,point)->fPtr ) ;
   }
 else
   {
    bcdtmWrite_message(0,0,0,"No Circular List For point %8ld ** dtmP->numPoints = %8ld",point,dtmP->numPoints) ;
    return(0) ;
   }
/*
** List Points
*/
 clPtr = nodeAddrP(dtmP,point)->cPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    pntNum = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr  = clistAddrP(dtmP,clPtr)->nextPtr ;
    dx = pointAddrP(dtmP,pntNum)->x - pointAddrP(dtmP,point)->x ;
    dy = pointAddrP(dtmP,pntNum)->y - pointAddrP(dtmP,point)->y ;
    if( dx == 0.0 && dy == 0.0 ) angle = 0.0 ;
    else
      {
       angle = atan2(dy,dx) ;
       if( angle < 0.0 ) angle += DTM_2PYE ;
      }
    bcdtmWrite_message(0,0,0,"%8ld ** %15.8lf %15.8lf %10.4lf ** %16.14lf",pntNum,pointAddrP(dtmP,pntNum)->x,pointAddrP(dtmP,pntNum)->y,pointAddrP(dtmP,pntNum)->z,angle) ;
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
BENTLEYDTM_Public int bcdtmList_writeTptrListDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Wites The Tptr List For A Point
*/
{
 long np,tp ;
/*
** Initialise
*/
 bcdtmWrite_message(0,0,0,"Temporary Pointer List For Point = %6ld ** DTM Object = %p",point,dtmP) ;
 if( point >= 0 || point < dtmP->numPoints )
   {
/*
** Write List
*/
    tp = point ;
    while ( nodeAddrP(dtmP,tp)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,tp)->tPtr >= 0 )
      {
       bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->tPtr ) ;
       np = nodeAddrP(dtmP,tp)->tPtr ;
       nodeAddrP(dtmP,tp)->tPtr = -(np+1) ;
       tp = np ;
      }
    bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->tPtr ) ;
/*
**  Reset Tptr Values Positive
*/
    tp = point ;
    while( nodeAddrP(dtmP,tp)->tPtr < 0  )
      {
       np = -(nodeAddrP(dtmP,tp)->tPtr + 1 ) ;
       nodeAddrP(dtmP,tp)->tPtr = np ;
       tp = np ;
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
BENTLEYDTM_Public int bcdtmList_writeSptrListDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Wites The Sptr List For A Point
*/
{
 long np,tp ;
/*
** Initialise
*/
 bcdtmWrite_message(0,0,0,"Scratch Pointer List For Point = %6ld ** DTM Object = %p",point,dtmP) ;
 if( point >= 0 || point < dtmP->numPoints )
   {
/*
** Write List
*/
    tp = point ;
    while ( nodeAddrP(dtmP,tp)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,tp)->sPtr >= 0 )
      {
       bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->sPtr ) ;
       np = nodeAddrP(dtmP,tp)->sPtr ;
       nodeAddrP(dtmP,tp)->sPtr = -(np+1) ;
       tp = np ;
      }
    bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->sPtr ) ;
/*
**  Reset Sptr Values Positive
*/
    tp = point ;
    while( nodeAddrP(dtmP,tp)->sPtr < 0  )
      {
       np = -(nodeAddrP(dtmP,tp)->sPtr + 1 ) ;
       nodeAddrP(dtmP,tp)->sPtr = np ;
       tp = np ;
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
BENTLEYDTM_Public int bcdtmList_writeHptrListDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Wites The Hptr List For A Point
*/
{
 long np,tp ;
/*
** Initialise
*/
 bcdtmWrite_message(0,0,0,"Hull Pointer List For Point = %6ld ** DTM Object = %p",point,dtmP) ;
 if( point >= 0 || point < dtmP->numPoints )
   {
/*
** Write List
*/
    tp = point ;
    while ( nodeAddrP(dtmP,tp)->hPtr != dtmP->nullPnt && nodeAddrP(dtmP,tp)->hPtr >= 0 )
      {
       bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->hPtr ) ;
       np = nodeAddrP(dtmP,tp)->hPtr ;
       nodeAddrP(dtmP,tp)->hPtr = -(np+1) ;
       tp = np ;
      }
    bcdtmWrite_message(0,0,0,"point = %6ld ** %10.4lf %10.4lf %8.4lf ** Next = %6ld ",tp,pointAddrP(dtmP,tp)->x,pointAddrP(dtmP,tp)->y,pointAddrP(dtmP,tp)->z,nodeAddrP(dtmP,tp)->hPtr ) ;
/*
**  Reset Hptr Values Positive
*/
    tp = point ;
    while( nodeAddrP(dtmP,tp)->hPtr < 0  )
      {
       np = -(nodeAddrP(dtmP,tp)->hPtr + 1 ) ;
       nodeAddrP(dtmP,tp)->hPtr = np ;
       tp = np ;
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
BENTLEYDTM_Public int bcdtmList_writePointsForDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Writes Points For A Dtm Feature
*/
{
 long n,listPtr,nextPnt=0,firstPnt,numPts,point ;
 char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pointP ;
/*
** Initialise
*/
/*
** Only Write Points If Dtm Feature Exists
*/
 if( dtmP->numFeatures > 0 && dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
/*
**  Method To Write Points Is Dependent On The Dtm Feature State
*/
    switch( dtmFeatureP->dtmFeatureState )
      {
/*
**     Write Points From Dtm Point Array
*/
       case DTMFeatureState::Data : // Dtm Features In Dtm Points Array
       bcdtmWrite_message(0,0,0,"Dtm Feature = %5ld ** numPoints = %5ld Type = %20s ** UserTag = %9I64d",dtmFeature,dtmFeatureP->numDtmFeaturePts,dtmFeatureTypeName,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          point  = dtmFeatureP->dtmFeaturePts.firstPoint+n ;
          pointP = pointAddrP(dtmP,point) ;
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %9ld ** %12.4lf %12.4lf %10.4lf",n,point,pointP->x,pointP->y,pointP->z) ;
         }
       break ;
/*
**     Write Points From Offset Array
*/
       case DTMFeatureState::OffsetsArray : // Dtm Features In Offset Array To Dtm Points Array
       bcdtmWrite_message(0,0,0,"Dtm Feature = %5ld ** numPoints = %5ld Type = %20s ** UserTag = %9I64d",dtmFeature,dtmFeatureP->numDtmFeaturePts,dtmFeatureTypeName,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          point  = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] ;
          pointP = pointAddrP(dtmP,point) ;
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %9ld ** %12.4lf %12.4lf %10.4lf",n,point,pointP->x,pointP->y,pointP->z) ;
         }
       break ;
/*
**     Write Points From Tin
*/
       case DTMFeatureState::Tin  :       // Dtm Feature In Tin

       if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(ftableAddrP(dtmP,dtmFeature)->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Dtm Feature = %9ld ** Type = %20s ** UserTag = %9I64d",dtmFeature,dtmFeatureTypeName,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
/*
**        Initialise Scan Variables
*/
          numPts = 1 ;
          bcdtmWrite_message(0,0,0,"First Point = %9ld ** %12.4lf %12.4lf %10.4lf ** Tptr = %9ld Sptr = %9ld Hptr = %9ld",firstPnt,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,firstPnt)->z,nodeAddrP(dtmP,firstPnt)->tPtr,nodeAddrP(dtmP,firstPnt)->sPtr,nodeAddrP(dtmP,firstPnt)->hPtr) ;
/*
**        Scan dtmFeature List Pointers
*/
          listPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
          while ( listPtr != dtmP->nullPtr )
            {
             nextPnt = dtmP->nullPnt ;
             while ( listPtr != dtmP->nullPtr  && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
             if( listPtr != dtmP->nullPtr )
               {
                nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
                   bcdtmWrite_message(0,0,0,"Next  Point = %9ld ** %12.4lf %12.4lf %10.4lf ** Tptr = %9ld Sptr = %9ld Hptr = %9ld",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z,nodeAddrP(dtmP,nextPnt)->tPtr,nodeAddrP(dtmP,nextPnt)->sPtr,nodeAddrP(dtmP,nextPnt)->hPtr) ;
                   listPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
                   ++numPts ;
                  }
                else bcdtmWrite_message(0,0,0,"Next  Point = %9ld",nextPnt) ;
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr = dtmP->nullPtr ;
               }
            }
/*
**        Decrement Number Of Points For A Closed Dtm Feature
*/
          if( nextPnt == firstPnt ) --numPts ;
/*
**        Write Out Number Of Points
*/
          bcdtmWrite_message(0,0,0,"Number Of Dtm Feature Points = %6ld",numPts) ;
         }
       else bcdtmWrite_message(0,0,0,"Dtm Feature Does Not Exist") ;

       break ;
/*
**     Default
*/
       default :
       bcdtmWrite_message(0,0,0,"Write Points For State %2ld Not Yet Implemented",ftableAddrP(dtmP,dtmFeature)->dtmFeatureState) ;
       break ;
      } ;
   }
 else bcdtmWrite_message(0,0,0,"Dtm Feature Does Not Exist") ;
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
BENTLEYDTM_Public int bcdtmList_testLineDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 long clPtr ;
 DTM_CIR_LIST *clistP ;
/*
** Scan Circular List For Point
*/
 clPtr = nodeAddrP(dtmP,p1)->cPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    clistP =  clistAddrP(dtmP,clPtr) ;
    if( p2 == clistP->pntNum ) return(1) ;
    clPtr  =  clistP->nextPtr ;
   }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_nullTptrValuesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Null The Tptr Values
*/
{
 long ofs ;
/*
** Scan All Tin Points
*/
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeAddrP(dtmP,ofs)->tPtr = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmList_zeroTptrValuesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Zero The Tptr Values
*/
{
 long ofs ;
/*
** Scan All Tin Points
*/
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeAddrP(dtmP,ofs)->tPtr = 0 ;
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
BENTLEYDTM_Public int bcdtmList_rangeNullTptrValuesDtmObject(BC_DTM_OBJ *dtmP,long startNode,long endNode)
/*
** This Function Null The Tptr Values
*/
{
 long node,numPartition,remPartition ;
 DTM_TIN_NODE  *nodeP ;
/*
** Get Partition Start
*/
 numPartition = startNode / dtmP->nodePartitionSize ;
 remPartition = startNode % dtmP->nodePartitionSize ;
 nodeP = dtmP->nodesPP[numPartition] + remPartition ;
/*
** Scan All Nodes From startNode to endNode
*/
 for( node = startNode ; node <= endNode ; ++node )
   {
    nodeP->tPtr = dtmP->nullPnt ;
    ++remPartition ;
    if( remPartition == dtmP->nodePartitionSize )
      {
       remPartition = 0 ;
       ++numPartition   ;
       nodeP = dtmP->nodesPP[numPartition] + remPartition ;
      }
    else ++nodeP ;
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
BENTLEYDTM_Public int bcdtmList_nullSptrValuesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Nulls The Sptr Valyes
*/
{
 long ofs ;
/*
** Scan All Tin Points
*/
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeAddrP(dtmP,ofs)->sPtr = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmList_nullHptrValuesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Null The Hptr Values
*/
{
 long ofs ;
/*
** Scan All Tin Points
*/
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeAddrP(dtmP,ofs)->hPtr = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(BC_DTM_OBJ *dtmP,long reportFlag)
/*
** This Nulls The Tptr Values And Is Mostly Used For Development Purposes
*/
{
 long ofs ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pointP ;
/*
** Scan All Tin Points
*/
 if( reportFlag ) bcdtmWrite_message(0,0,0,"Reporting Non Null Tptr Values ** Dtm Object = %p",dtmP) ;
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeP = nodeAddrP(dtmP,ofs) ;
    if( nodeP->cPtr != dtmP->nullPtr && nodeP->tPtr != dtmP->nullPnt )
      {
       if( reportFlag )
         {
          pointP = pointAddrP(dtmP,ofs) ;
          bcdtmWrite_message(0,0,0,"Point[%6ld] ** %10.4lf %10.4lf %10.4lf ** tPtr = %9ld",ofs,pointP->x,pointP->y,pointP->z,nodeP->tPtr) ;
         }
       nodeP->tPtr = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(BC_DTM_OBJ *dtmP,long reportFlag)
/*
** This Nulls The Sptr Values And Is Mostly Used For Development Purposes
*/
{
 long ofs ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pointP ;
/*
** Scan All Tin Points
*/
 if( reportFlag ) bcdtmWrite_message(0,0,0,"Reporting Non Null Sptr Values ** Dtm Object = %p",dtmP) ;
 for( ofs = 0 ; ofs < dtmP->numPoints ; ++ofs )
   {
    nodeP = nodeAddrP(dtmP,ofs) ;
    if( nodeP->cPtr != dtmP->nullPtr && nodeP->sPtr != dtmP->nullPnt )
      {
       if( reportFlag )
         {
          pointP = pointAddrP(dtmP,ofs) ;
          bcdtmWrite_message(0,0,0,"Point[%6ld] ** %10.4lf %10.4lf %10.4lf ** sPtr = %9ld",ofs,pointP->x,pointP->y,pointP->z,nodeP->sPtr) ;
         }
       nodeP->sPtr = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmList_checkConnectivityTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long messageFlag)
/*
** This Function Checks The Connectivity Of A Tptr List
**
** Return Values == DTM_SUCCESS  No Connectivity Errors
**               == DTM_ERROR    Connectivity Errors
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,tp,lp=0,knot ;
/*
** Initialise
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Start Point %10ld Range Error",startPnt) ;
    ret = DTM_ERROR ;
   }
 else
   {
    if( nodeAddrP(dtmP,startPnt)->tPtr < 0 || nodeAddrP(dtmP,startPnt)->tPtr >= dtmP->numPoints  )
      {
       bcdtmWrite_message(2,0,0,"Next Point %10ld Range Error",nodeAddrP(dtmP,startPnt)->tPtr) ;
       ret = DTM_ERROR ;
      }
    else
      {
/*
**     Check List Connectivity
*/
       sp = startPnt ;
       while ( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,sp)->tPtr >= 0 )
         {
          if( ! bcdtmList_testLineDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->tPtr) )
            {
             if( messageFlag ) bcdtmWrite_message(0,0,0,"Unconnected Points %8ld %8ld In Tptr Polygon",sp,nodeAddrP(dtmP,sp)->tPtr) ;
             ret = DTM_ERROR ;
            }
          tp = nodeAddrP(dtmP,sp)->tPtr ;
          nodeAddrP(dtmP,sp)->tPtr = -(nodeAddrP(dtmP,sp)->tPtr + 1) ;
          sp = tp ;
         }
/*
**     Set Value Of Last Point In List
*/
       knot = 0 ;
       if( nodeAddrP(dtmP,sp)->tPtr < 0 ) { lp = sp ; knot = 1 ; }
/*
**     Reset Tptr Values Positive
*/
       sp = startPnt ;
       while( nodeAddrP(dtmP,sp)->tPtr < 0  )
         {
          tp = -(nodeAddrP(dtmP,sp)->tPtr + 1 ) ;
          nodeAddrP(dtmP,sp)->tPtr = tp ;
          sp = tp ;
         }
/*
**    Test For Knot In List
*/
      if( knot && lp != startPnt )
        {
         if( messageFlag ) bcdtmWrite_message(0,0,0,"Knot At Point %8ld ",lp) ;
         ret = DTM_ERROR ;
        }
     }
  }
/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testForDtmFeatureLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
{
/*
** This Function Tests If Line P1-P2 Is A Dtm Feature Line
*/
 long clPtr ;
/*
** Scan P1 and See If it Connects To P2
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    if( P2 == flistAddrP(dtmP,clPtr)->nextPnt ) return(1) ;
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Scan P2 and See If it Connects To P1
*/
 clPtr = nodeAddrP(dtmP,P2)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    if( P1 == flistAddrP(dtmP,clPtr)->nextPnt ) return(1) ;
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidLineDtmObjectOld(BC_DTM_OBJ *dtmP,long P1,long P2,long *voidLineP)
/*
**
** This Function Tests If Line P1-P2 is an Internal Void Line
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p,ap,np,pp,sp,lp,numPointHullFeatures ;
 DTM_TIN_POINT_FEATURES *phfP,*pointHullFeatures=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Testing For Void Line") ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
   }
/*
** Initialise
*/
 *voidLineP = 0 ;
/*
** Test if P1 or P2 are Internal Void Points
*/
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) || bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P2)->PCWD) )  *voidLineP = 1  ;
/*
** Test For Hull Line
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line") ;
    if( ! bcdtmList_testForHullLineDtmObject(dtmP,P1,P2) )
      {
       for( p = 0 ; p < 2 && ! *voidLineP ; ++p )
         {
          if( ! p  ) { sp = P1 ; lp = P2 ; }
          else       { sp = P2 ; lp = P1 ; }
/*
**        Get Hull Features For sp And Test If sp lp Is A Void Line
*/
          if( bcdtmList_getHullFeaturesForPointDtmObject(dtmP,sp,&pointHullFeatures,&numPointHullFeatures) ) goto errexit ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Number Of Point Hull Features = %6ld",numPointHullFeatures) ;
             for( phfP = pointHullFeatures ; phfP < pointHullFeatures + numPointHullFeatures  && ! *voidLineP ; ++phfP )
               {
                bcdtmWrite_message(0,0,0,"Point[%4ld] ** Feature = %6ld Type = %4ld Prior = %9ld Next = %9ld",(long)(phfP-pointHullFeatures),phfP->dtmFeature,phfP->dtmFeatureType,phfP->priorPoint,phfP->nextPoint) ;
               }
            }
          for( phfP = pointHullFeatures ; phfP < pointHullFeatures + numPointHullFeatures  && ! *voidLineP ; ++phfP )
            {
             np = phfP->nextPoint ;
             pp = phfP->priorPoint ;
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"pp = %6ld ** %10.4lf %10.4lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
                bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                bcdtmWrite_message(0,0,0,"np = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                bcdtmWrite_message(0,0,0,"lp = %6ld ** %10.4lf %10.4lf %10.4lf",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
               }
             switch ( phfP->dtmFeatureType )
               {
                case  DTMFeatureType::Void :
                case  DTMFeatureType::Hole :
/*
**              Scan Internally From Next Point
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Internally To Void Hull") ;
                ap = np  ;
                while ( ap != pp && ! bcdtmList_testIfDirectionalLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,sp,ap) && ! *voidLineP )
                  {
                   if( ap == lp ) *voidLineP = 1 ;
                   if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                  }
                if( ! *voidLineP && ap != pp )
                  {
/*
**              Scan Internally From Prior Point
*/
                   ap = pp ;
                   while ( ap != np && ! bcdtmList_testIfDirectionalLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,ap,sp) && ! *voidLineP )
                     {
                      if( ap == lp ) *voidLineP = 1 ;
                      if( ( ap = bcdtmList_nextClkDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                     }
                  }
                break ;

                case  DTMFeatureType::Island :
/*
**              Scan Externally From Next Point
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Externally To Island Hull From Next Point") ;
                ap = np  ;
                if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,ap,sp) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,sp,ap) )
                  {
                   if( ( ap = bcdtmList_nextClkDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                   while ( ap != pp && ! *voidLineP && ! bcdtmList_testForNonDirectionalIslandVoidOrHoleHullLineDtmObject(dtmP,ap,sp) )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"ap = %6ld ** %10.4lf %10.4lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z) ;
                      if( ap == lp ) *voidLineP = 1 ;
                      if( ( ap = bcdtmList_nextClkDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                     }
                  }
/*
**              Scan Externally From Prior Point
*/
                if( ! *voidLineP && ap != pp )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Externally To Island Hull From Prior Point") ;
                   ap = pp ;
                   if( ! bcdtmList_testForIslandHullLineDtmObject(dtmP,sp,ap) && ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,ap,sp) )
                     {
                      if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                      while(  ap != np && ! *voidLineP && ! bcdtmList_testForNonDirectionalIslandVoidOrHoleHullLineDtmObject(dtmP,ap,sp) )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"ap = %6ld ** %10.4lf %10.4lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z) ;
                         if( ap == lp ) *voidLineP = 1 ;
                         if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                        }
                     }
                  }
                break ;

                default :
                break   ;

               } ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( pointHullFeatures != NULL ) { free(pointHullFeatures) ; pointHullFeatures = NULL ; }
/*
** Job Completed
*/
 if( dbg &&   *voidLineP) bcdtmWrite_message(0,0,0,"Void Line") ;
 if( dbg && ! *voidLineP) bcdtmWrite_message(0,0,0,"Not A Void Line") ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,long *voidTriangleP)
/*
** This Function Tests For A Void Triangle
*/
{
 int ret=DTM_SUCCESS ;
/*
** Initialise
*/
 *voidTriangleP = 0 ;
/*
** If Any Points are Void Points Then Triangle Is A Void triangle
*/
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD)) { *voidTriangleP = 1 ; return(ret) ; }
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P2)->PCWD)) { *voidTriangleP = 1 ; return(ret) ; }
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P3)->PCWD)) { *voidTriangleP = 1 ; return(ret) ; }
/*
** If any Lines are Void Lines Then Triangle Is A Void triangle
*/
 if( bcdtmList_testForVoidLineDtmObject(dtmP,P1,P2,voidTriangleP)) goto errexit ;
 if( *voidTriangleP ) return(ret) ;
 if( bcdtmList_testForVoidLineDtmObject(dtmP,P2,P3,voidTriangleP)) goto errexit ;
 if( *voidTriangleP ) return(ret) ;
 if( bcdtmList_testForVoidLineDtmObject(dtmP,P3,P1,voidTriangleP)) goto errexit ;
/*
** Check For Single Triangle Void
*/
 if( bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,P1,P3))
   {
    if( bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,P3,P2))
      {
       if( bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,P2,P1))
         {
          *voidTriangleP = 1 ;
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
BENTLEYDTM_Public int bcdtmList_testForTinHullLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2)
/*
** This Function Tests If The Line pnt1-pnt2 is A Void Hull Line
*/
{
/*
** Test For Tin Hull Line
*/
 if( nodeAddrP(dtmP,pnt1)->hPtr == pnt2 ) return(1) ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidHullLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2)
/*
** This Function Tests If The Line pnt1-pnt2 is A Void Hull Line
*/
{
 long clc ;
/*
** Test For Void Hull Line
*/
 clc = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == pnt2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForDtmFeatureTypeLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,DTMFeatureType dtmFeatureType)
/*
** This Function Tests If The Line pnt1-pnt2 is A DtmFeature Type Line
*/
{
 long clc ;
/*
** Test For Void Hull Line
*/
 clc = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == pnt2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForIslandHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is On An Island Hull
*/
{
 long clPtr ;
/*
** Test For Island Line
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clPtr)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) return(1) ;
      }
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidOrHoleHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is A Void Or Hole Hull Line
*/
{
 long clPtr ;
/*
** Test For Void Hull Line
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clPtr)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    )  return(1) ;
      }
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForIslandVoidOrHoleHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is An Island Void Or Hole Hull Line
*/
{
 long clPtr ;
/*
** Test For Void Hull Line
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clPtr)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island    )  return(1) ;
      }
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForNonDirectionalIslandVoidOrHoleHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is An Island Void Or Hull Line
*/
{
 long clPtr ;
/*
** Scan P1
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clPtr)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  return(1) ;
      }
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Scan P2
*/
 clPtr = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clPtr)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  return(1) ;
      }
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is On A Tin ,Void, Hole Or Island Hull
*/
{
 long clc ;
/*
** Test For Tin Hull
*/
 if( nodeAddrP(dtmP,P1)->hPtr == P2 ) return(1) ;
 if( nodeAddrP(dtmP,P2)->hPtr == P1 ) return(1) ;
/*
** Scan P1 And Test For Void, Hole Or Island Hull
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Scan P2 And Test For Void, Hole Or Island Hull
*/
 clc = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidsInDtmObject(BC_DTM_OBJ *dtmP,long *voidsInDtmP)
/*
** This Function Tests If The Line P1-P2 is On A Tin ,Void, Hole Or Island Hull
*/
{
 long ofs,dtmFeature,partitionNum ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *voidsInDtmP = FALSE ;
/*
** Scan Dtm Features
*/
 if( dtmP->fTablePP != NULL )
   {
    ofs = 0 ;
    partitionNum = 0 ;
    dtmFeatureP = dtmP->fTablePP[partitionNum] ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && *voidsInDtmP == FALSE ; ++dtmFeature )
      {
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
         {
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ||
              dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole   ||
              dtmFeatureP->dtmFeatureType == DTMFeatureType::Island     ) *voidsInDtmP = TRUE  ;
         }
       ++ofs ;
       if( ofs == dtmP->featurePartitionSize )
         {
          ofs = 0 ;
          ++partitionNum ;
          dtmFeatureP = dtmP->fTablePP[partitionNum] ;
         }
       else ++dtmFeatureP ;
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
BENTLEYDTM_Public int bcdtmList_testForPointOnTinFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long point,long *onFeatureP)
/*
** This Function Tests For Point On A Dtm Tin Feature
*/
{
 long scanPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *onFeatureP = 0 ;
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures ) return(DTM_SUCCESS) ;
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin || ( scanPnt = dtmFeatureP->dtmFeaturePts.firstPoint) == dtmP->nullPnt ) return(DTM_SUCCESS) ;
/*
**  Scan Feature Looking For point
*/
 do
   {
    if( scanPnt == point ) *onFeatureP = 1 ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,scanPnt,&scanPnt) ;
   } while ( scanPnt != dtmP->nullPnt && scanPnt != dtmFeatureP->dtmFeaturePts.firstPoint && ! *onFeatureP ) ;
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
BENTLEYDTM_Public int bcdtmList_testForPointOnDtmFeatureTypeDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long point,long *featureP )
/*
** This Function Tests If a Point is on a DTM Feature Type And If So Returns
** The Feature Number For The Point
**
*/
{
 long flPtr ;
/*
** Initialise
*/
 *featureP = dtmP->nullPnt ;
/*
** Scan Feature List Points For Point
*/
 flPtr = nodeAddrP(dtmP,point)->fPtr ;
 while( flPtr != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType )
      {
       *featureP = flistAddrP(dtmP,flPtr)->dtmFeature ;
       return(0) ;
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForPointOnIslandHullDtmObject(BC_DTM_OBJ *dtmP,long P1)
/*
** This Function Tests If P1 is On An Island Void Or Hole Hull
*/
{
 long clc ;
/*
** Test If Point Is On Island Hull Line
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) return(1) ;
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForPointOnVoidHullDtmObject(BC_DTM_OBJ *dtmP,long P1)
/*
** This Function Tests If P1 is On An Island Void Or Hole Hull
*/
{
 long clc ;
/*
** Test If Point Is On Island Hull Line
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) return(1) ;
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForPointOnIslandVoidOrHoleHullDtmObject(BC_DTM_OBJ *dtmP,long P1)
/*
** This Function Tests If P1 is On An Island Void Or Hole Hull
*/
{
 long clc ;
/*
** Test If Point Is On Island Void Or Hole Hull Line
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
        ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
        ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForLineOnDtmFeatureTypeDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long pnt1,long pnt2)
/*
** This Function Tests If The Line pnt1-pnt2 Is On A DTM Feature Type
*/
{
 long flPtr ;
/*
** Initialiase
*/
 if( pnt1 < 0 || pnt1 >= dtmP->numPoints ) return(0) ;
 if( pnt2 < 0 || pnt2 >= dtmP->numPoints ) return(0) ;
 if(nodeAddrP(dtmP,pnt1)->cPtr == dtmP->nullPtr ) return(0) ;
 if(nodeAddrP(dtmP,pnt2)->cPtr == dtmP->nullPtr ) return(0) ;
/*
** Scan pnt1
*/
 if( dtmFeatureType == DTMFeatureType::Hull )
   {
    if( nodeAddrP(dtmP,pnt1)->hPtr == pnt2 ) return(1) ;
   }
 else
   {
    flPtr = nodeAddrP(dtmP,pnt1)->fPtr ;
    while( flPtr != dtmP->nullPtr )
      {
       if( flistAddrP(dtmP,flPtr)->nextPnt == pnt2 && ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1) ;
       flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
      }
   }
/*
** Scan pnt2
*/
 if( dtmFeatureType == DTMFeatureType::Hull )
   {
    if( nodeAddrP(dtmP,pnt2)->hPtr == pnt1 ) return(1) ;
   }
 else
   {
    flPtr = nodeAddrP(dtmP,pnt2)->fPtr ;
    while( flPtr != dtmP->nullPtr )
      {
       if( flistAddrP(dtmP,flPtr)->nextPnt == pnt1 && ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1) ;
       flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testIfDirectionalLineOnDtmFeatureTypeDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, long P1, long P2)
/*
** This Function Tests If Line P1P2 Is On A DTM Feature Type
** P2 Must Be Next Point From P1 On Feature Type
**
*/
{
 long clPtr ;
/*
** Scan Feature List Points For Point
*/
 clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clPtr)->nextPnt == P2 && ftableAddrP(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) return(1) ;
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_getDtmFeatureForDtmFeatureTypeOnLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 long       pnt1,
 long       pnt2,
 long       *dtmFeatureP
 )
/*
** This Function Gets The DTM Feature For The DTM Feature Type
** On Line pnt1 - pnt2 ;
**
*/
{
 int ret=DTM_SUCCESS ;
 long flPtr ;
 DTM_FEATURE_LIST *flistP ;
/*
** Initialise
*/
 *dtmFeatureP = dtmP->nullPnt ;
/*
** Scan Feature List Points For Point
*/
 flPtr = nodeAddrP(dtmP,pnt1)->fPtr ;
 while( flPtr != dtmP->nullPtr && *dtmFeatureP == dtmP->nullPnt )
   {
    flistP = flistAddrP(dtmP,flPtr) ;
    if( flistP->nextPnt == pnt2 && ftableAddrP(dtmP,flistP->dtmFeature)->dtmFeatureType == dtmFeatureType )
      {
       *dtmFeatureP = flistP->dtmFeature ;
      }
    flPtr = flistP->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_getHullFeaturesForPointDtmObject(BC_DTM_OBJ *dtmP,long Point,DTM_TIN_POINT_FEATURES **pointFeaturesPP,long *numPointFeaturesP)
/*
** This Function Gets The Hull Features For A Point
*/
{
 int  ret=DTM_SUCCESS ;
 long np,pp,lp,clPtr,flPtr,pfPtr,feature,memFeatureTable=0,memInc=10 ;
 DTM_TIN_POINT_FEATURES* lhs;
 BC_DTM_FEATURE* rhs;
/*
** Initialise
*/
 *numPointFeaturesP = 0 ;
 if( *pointFeaturesPP != NULL ) { free(*pointFeaturesPP) ; *pointFeaturesPP = NULL ; }
/*
** Scan P1 And Test For Void, Hole Or Island Hull
*/
 pfPtr = nodeAddrP(dtmP,Point)->fPtr ;
 while ( pfPtr != dtmP->nullPtr )
   {
    feature = flistAddrP(dtmP,pfPtr)->dtmFeature ;
    if( ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Void   ||
        ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Island ||
        ftableAddrP(dtmP,feature)->dtmFeatureType == DTMFeatureType::Hole      )
      {
/*
**     Get Next And Prior Points For Hull Feature
*/
       np = flistAddrP(dtmP,pfPtr)->nextPnt ;
       pp = dtmP->nullPnt ;
       clPtr = nodeAddrP(dtmP,Point)->cPtr ;
       while( clPtr != DTM_NULL_PTR )
         {
          lp  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          flPtr = nodeAddrP(dtmP,lp)->fPtr ;
          while ( flPtr != dtmP->nullPtr && pp == dtmP->nullPnt )
            {
             if( flistAddrP(dtmP,flPtr)->dtmFeature == feature && flistAddrP(dtmP,flPtr)->nextPnt == Point ) pp = lp ;
             flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
            }
         }
/*
**     Allocate memory If Necessary
*/
       if( *numPointFeaturesP == memFeatureTable )
         {
          memFeatureTable = memFeatureTable + memInc ;
          if( *pointFeaturesPP == NULL ) *pointFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          else                           *pointFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc ( *pointFeaturesPP , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          if( *pointFeaturesPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
/*
**     Store Point Feature
*/
       lhs = (*pointFeaturesPP + *numPointFeaturesP);
       rhs = ftableAddrP(dtmP,feature);
       lhs->dtmFeature     =  feature ;
       lhs->dtmFeatureType =  rhs->dtmFeatureType ;
       lhs->userTag        =  rhs->dtmUserTag ;
       lhs->userFeatureId       =  rhs->dtmFeatureId    ;
       lhs->priorPoint     =  pp ;
       lhs->nextPoint      =  np ;
       ++*numPointFeaturesP ;
      }
    pfPtr = flistAddrP(dtmP,pfPtr)->nextPtr ;
   }
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getPriorPointForDtmFeatureDtmObjectOld(BC_DTM_OBJ *dtmP,long dtmFeature,long currentPnt,long *priorPntP)
/*
** This Function Gets The Prior Point For A Dtm Feature
*/
{
 long listPtr,scanPnt,firstPnt,lastPnt ;
/*
** Initialise
*/
 *priorPntP = dtmP->nullPnt ;
/*
** Check Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       scanPnt = lastPnt = firstPnt ;
       listPtr = nodeAddrP(dtmP,scanPnt)->fPtr ;
/*
**     Scan Dtm Feature Points To Current Point
*/
       while ( listPtr != dtmP->nullPtr && *priorPntP == dtmP->nullPnt )
         {
          while ( listPtr != dtmP->nullPtr && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
          if( listPtr != dtmP->nullPtr )
            {
             scanPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
             if( scanPnt != dtmP->nullPnt )
               {
                if( scanPnt == currentPnt ) *priorPntP = lastPnt ;
                lastPnt = scanPnt ;
                listPtr = nodeAddrP(dtmP,scanPnt)->fPtr ;
               }
             if( scanPnt == dtmP->nullPnt || scanPnt == firstPnt ) listPtr = dtmP->nullPtr ;
            }
         }
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
BENTLEYDTM_EXPORT int bcdtmList_getNextPointForDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long currentPnt ,long *nextPntP)
/*
** This Function Gets The Next Point For A Dtm Feature
*/
{
 long listPtr,nextPnt,lastPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *nextPntP = dtmP->nullPnt ;
/*
** Check Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  DTM Data State
*/
//    if( dtmP->dtmState == DTMState::Data )
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
       if( dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
          nextPnt =  dtmFeatureP->dtmFeaturePts.firstPoint ;
          lastPnt = nextPnt + dtmFeatureP->numDtmFeaturePts - 1 ;
          while( nextPnt <= lastPnt && nextPnt <= currentPnt ) ++nextPnt ;
          if( nextPnt <= lastPnt ) *nextPntP = nextPnt ;
         }
      }

/*
**  DTM Tin State
*/
 //   if( dtmP->dtmState == DTMState::Tin )
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
/*
**        Scan Dtm Features For Current Point
*/
          listPtr = nodeAddrP(dtmP,currentPnt)->fPtr ;
          while ( listPtr != dtmP->nullPtr && *nextPntP == dtmP->nullPnt )
            {
             if( flistAddrP(dtmP,listPtr)->dtmFeature == dtmFeature ) *nextPntP = flistAddrP(dtmP,listPtr)->nextPnt ;
             listPtr  = flistAddrP(dtmP,listPtr)->nextPtr ;
            }
         }
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
BENTLEYDTM_Public int bcdtmList_getTptrPriorAndNextPointsDtmObject(BC_DTM_OBJ *dtmP,long thisPnt,long *priorPntP,long *nextPntP )
/*
** This Function Returns The Prior and Next Points For A tPtr String
*/
{
 long  clc ;
/*
** Initialise Variables
*/
 *priorPntP = *nextPntP = dtmP->nullPnt ;
 if( thisPnt < 0 || thisPnt > dtmP->numPoints ) return(DTM_SUCCESS) ;
/*
** Set Next Point
*/
 *nextPntP = nodeAddrP(dtmP,thisPnt)->tPtr ;
/*
** Scan thisPnt To Get Prior Pointer
*/
 clc = nodeAddrP(dtmP,thisPnt)->cPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( nodeAddrP(dtmP,clistAddrP(dtmP,clc)->pntNum)->tPtr == thisPnt )
      {
       *priorPntP = clistAddrP(dtmP,clc)->pntNum ;
       return(0) ;
      }
    clc = clistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_getLastPointInTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *lastPntP,long *lastPriorPntP)
/*
** This Function gets The last point In A Tptr List
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,tp ;
/*
** Initialise
*/
 *lastPntP = *lastPriorPntP = dtmP->nullPnt ;
 if( startPnt < 0 || startPnt >= dtmP->numPoints ) goto errexit ;
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
 sp = startPnt ;
/*
** Check List Connectivity
*/
 while ( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,sp)->tPtr >= 0 )
   {
    tp = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = -(nodeAddrP(dtmP,sp)->tPtr + 1) ;
    *lastPriorPntP = sp ;
    sp = tp ;
   }
/*
** Set Value Of Last Point
*/
 if( nodeAddrP(dtmP,sp)->tPtr < 0 )  *lastPntP = sp  ;
/*
** Reset Tptr Values Positive
*/
 sp = startPnt ;
 while( nodeAddrP(dtmP,sp)->tPtr < 0  )
   {
    tp = -(nodeAddrP(dtmP,sp)->tPtr + 1 ) ;
    nodeAddrP(dtmP,sp)->tPtr = tp ;
    sp = tp ;
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
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_nullTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Nulls Out The Tptr List
*/
{
 long sp,np ;
 DTM_TIN_NODE *nodeP ;
/*
** Check A Tptr List Exists
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr != dtmP->nullPnt )
   {
/*
**  Scan Tptr List And Null Tptr Values
*/
    sp = startPnt ;
    do
      {
       nodeP       = nodeAddrP(dtmP,sp) ;
       np          = nodeP->tPtr ;
       nodeP->tPtr = dtmP->nullPnt ;
       sp          = np ;
      } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_nullSptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Nulls Out The Tptr List
*/
{
 long sp,np ;
 DTM_TIN_NODE *nodeP ;
/*
** Check A Sptr List Exists
*/
 if( nodeAddrP(dtmP,startPnt)->sPtr != dtmP->nullPnt )
   {
/*
**  Scan Sptr List And Null Sptr Values
*/
    sp = startPnt ;
    do
      {
       nodeP       = nodeAddrP(dtmP,sp) ;
       np          = nodeP->sPtr ;
       nodeP->sPtr = dtmP->nullPnt ;
       sp          = np ;
      } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long firstCall,BC_DTM_FEATURE **dtmFeaturePP,long *dtmFeatureNumP )
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature ;
 thread_local static long lastFeature=-1 ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence") ;
/*
** Initialise
*/
 *dtmFeaturePP   = NULL ;
 *dtmFeatureNumP = 0    ;
/*
** Do First Call Processing
*/
 if( firstCall == TRUE ) lastFeature = -1 ;
/*
** Scan feature For Next Occurrence
*/
 for( feature = lastFeature + 1 ; feature < dtmP->numFeatures && *dtmFeaturePP == NULL ; ++feature )
   {
    dtmFeatureP =  ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       *dtmFeaturePP   = dtmFeatureP ;
       *dtmFeatureNumP = feature ;
       lastFeature     = feature ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Pointer To Next Feature Type Occurrence Error") ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_resortTinStructureDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Resorts the Tin Data Structures after the Insertion of Unsorted Points
**
**  2004/01/03  Rob Cormack  Rob.Cormack@Bentley.com
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     node,fTable,fList,cList ;
 long     ofs,ofs1,ofs2,*srP,*sortP=NULL,*tempP=NULL ;
 DTM_TIN_NODE      *nodeP,tempNode  ;
 DTM_TIN_POINT     tempPoint  ;
 DTM_CIR_LIST      *cListP ;
 BC_DTM_FEATURE    *fTableP ;
 DTM_FEATURE_LIST  *fListP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resorting Tin ** dtmP->numSortedPoints = %8ld dtmP->numPoints = %8ld",dtmP->numSortedPoints,dtmP->numPoints) ;
/*
** Set Number Of Sorted Points
*/
 if( bcdtmList_setNumberOfSortedPointsDtmObject(dtmP)) goto errexit ;
/*
** Write Number Of Sorted Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->numSortedPoints = %8ld",dtmP->numPoints,dtmP->numSortedPoints) ;
/*
** Only Resort If Additional Tin Points Have Been Inserted
*/
 if( dtmP->numSortedPoints <= 0 ) dtmP->numSortedPoints = 1 ;
 if( dtmP->numSortedPoints < dtmP->numPoints )
   {
/*
**  Allocate Memory For New Sort Pointers
*/
    sortP = ( long * ) malloc(dtmP->numPoints*sizeof(long)) ;
    if( sortP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
    tempP = ( long * ) malloc(dtmP->numPoints*sizeof(long)) ;
    if( tempP == NULL ) {  bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**  Quick Sort The Additional Data Points
*/
    for( srP = sortP, ofs = 0 ; srP < sortP+dtmP->numPoints ; ++srP, ++ofs) *srP = ofs ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Quick Sorting Added Points") ;
    bcdtmList_qsortDtmObject(dtmP,dtmP->numPoints-dtmP->numSortedPoints,dtmP->numSortedPoints,sortP,tempP) ;
/*
**  Merge The Sorted Additional Data Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Merging Added Points") ;
    ofs2 = dtmP->numSortedPoints ;
    if(   pointAddrP(dtmP,*(sortP+ofs2-1))->x >   pointAddrP(dtmP,*(sortP+ofs2))->x  ||
        ( pointAddrP(dtmP,*(sortP+ofs2-1))->x ==  pointAddrP(dtmP,*(sortP+ofs2))->x  &&
          pointAddrP(dtmP,*(sortP+ofs2-1))->y >   pointAddrP(dtmP,*(sortP+ofs2))->y )    )
      {
       ofs1 = 0 ;
       for( ofs = 0 ; ofs < dtmP->numPoints  ; ++ofs )
         {
          if     ( ofs1 >= dtmP->numSortedPoints ) {*(tempP+ofs)=*(sortP+ofs2);++ofs2;}
          else if( ofs2 >= dtmP->numPoints )       {*(tempP+ofs)=*(sortP+ofs1);++ofs1;}
          else
            {
             if(   pointAddrP(dtmP,*(sortP+ofs1))->x <   pointAddrP(dtmP,*(sortP+ofs2))->x  ||
                 ( pointAddrP(dtmP,*(sortP+ofs1))->x ==  pointAddrP(dtmP,*(sortP+ofs2))->x  &&
                   pointAddrP(dtmP,*(sortP+ofs1))->y <=  pointAddrP(dtmP,*(sortP+ofs2))->y)    )
               {
                *(tempP+ofs) = *(sortP+ofs1) ;
                ++ofs1 ;
               }
             else
               {
                *(tempP+ofs) = *(sortP+ofs2) ;
                ++ofs2 ;
               }
            }
         }
       for( ofs = 0 ; ofs < dtmP->numPoints  ; ++ofs ) *(sortP+ofs) = *(tempP+ofs) ;
      }
/*
**  Calculate Data Point Sort Position
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Point Sort Position") ;
    for( srP = sortP ; srP < sortP+dtmP->numPoints ; ++srP  ) *(tempP+*srP) = (long)(srP-sortP) ;
/*
**  Adjust Feature Table First Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature First Point Values") ;
    for( fTable = 0 ; fTable < dtmP->numFeatures ; ++fTable )
      {
       fTableP = ftableAddrP(dtmP,fTable) ;
       if( fTableP->dtmFeatureState == DTMFeatureState::Tin )
         {
          fTableP->dtmFeaturePts.firstPoint = *(tempP+fTableP->dtmFeaturePts.firstPoint) ;
         }
      }
/*
**  Adjust Feature List Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Feature List Next Point Values") ;
    for( fList = 0 ; fList < dtmP->numFlist ; ++fList )
      {
       fListP = flistAddrP(dtmP,fList) ;
       if( fListP->nextPnt != dtmP->nullPnt ) fListP->nextPnt = *(tempP+fListP->nextPnt) ;
      }
/*
**  Adjust Node Hull Pointers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Node Hull Pointers") ;
    for( node = 0 ; node < dtmP->numPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->hPtr != dtmP->nullPnt ) nodeP->hPtr = *(tempP+nodeP->hPtr) ;
      }
/*
**  Adjust Circular List Point Numbers
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjusting Clircular List Point Numbers") ;
    for( cList = 0 ; cList < dtmP->cListPtr ; ++cList )
      {
       cListP = clistAddrP(dtmP,cList) ;
       if( cListP->pntNum >= 0 && cListP->pntNum < dtmP->numPoints ) cListP->pntNum = *(tempP+cListP->pntNum) ;
      }
/*
**  Reset Dtm Pointer To Tin Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Resetting DTM Hull Pointers") ;
    if( dtmP->hullPoint >= 0 && dtmP->hullPoint < dtmP->numPoints )
      {
       dtmP->hullPoint    = *(tempP+dtmP->hullPoint) ;
       dtmP->nextHullPoint  = nodeAddrP(dtmP,dtmP->hullPoint)->hPtr ;
      }
    else
      {
       dtmP->hullPoint      = dtmP->nullPnt ;
       dtmP->nextHullPoint  = dtmP->nullPnt ;
      }
/*
**  Place Data In Sort Order
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Placing Points In Sorted Order") ;
    for( srP = sortP , ofs = 0 ; srP < sortP+dtmP->numPoints ; ++srP , ++ofs )
      {
       tempPoint = *pointAddrP(dtmP,ofs) ;
       *pointAddrP(dtmP,ofs)  = *pointAddrP(dtmP,*srP) ;
       *pointAddrP(dtmP,*srP) = tempPoint ;
       tempNode = *nodeAddrP(dtmP,ofs) ;
       *nodeAddrP(dtmP,ofs)  = *nodeAddrP(dtmP,*srP) ;
       *nodeAddrP(dtmP,*srP) = tempNode ;
       *(sortP+*(tempP+ofs)) = *srP ;
       *(tempP+*srP) = *(tempP+ofs) ;
      }
/*
**  Set Number Of Tin Points For Binary Searching
*/
    dtmP->numSortedPoints = dtmP->numPoints ;
   }
/*
** Check Tin Points Are Sorted
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Points Sort Order After Resorting") ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,1) )
      {
       bcdtmWrite_message(2,0,0,"Dtm Sort Order Invalid") ;
       goto errexit ;
      }
   }
/*
**  Clean Up
*/
 cleanup :
 if( sortP != NULL ) free(sortP) ;
 if( tempP != NULL ) free(tempP) ;
/*
** Job ComfListPeted
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
BENTLEYDTM_Private int bcdtmList_qsortDtmObject(BC_DTM_OBJ *dtmP,long ns,long ofs,long *sortP,long *tempP )
/*
** This Routine Quick Sorts the Tin Points
**
**  2004/01/03  Rob Cormack  Rob.Cormack@Bentley.com
*/
{
 long  i,ns1,ns2,ofs1,ofs2,temp ;
/*
** Test for Two Data Points
*/
 if( ns == 2 )
   {
    if(   pointAddrP(dtmP,*(sortP+ofs))->x >   pointAddrP(dtmP,*(sortP+ofs+1))->x  ||
        ( pointAddrP(dtmP,*(sortP+ofs))->x ==  pointAddrP(dtmP,*(sortP+ofs+1))->x  &&
          pointAddrP(dtmP,*(sortP+ofs))->y >   pointAddrP(dtmP,*(sortP+ofs+1))->y )   )
      {
       temp = *(sortP+ofs) ;
       *(sortP+ofs) = *(sortP+ofs+1) ;
       *(sortP+ofs+1) = temp ;
      }
   }
/*
** Test for more than two data Points
*/
 if( ns > 2 )
   {
    ns1 = ns / 2  ; if( ns % 2 != 0 ) ns1 = ns1+1 ; ns2 = ns - ns1 ;
    ofs1 = ofs  ; ofs2 = ofs+ns1 ;
    bcdtmList_qsortDtmObject(dtmP,ns1,ofs1,sortP,tempP) ;
    bcdtmList_qsortDtmObject(dtmP,ns2,ofs2,sortP,tempP) ;
/*
** Merge data sets
*/
    if(   pointAddrP(dtmP,*(sortP+ofs2-1))->x >   pointAddrP(dtmP,*(sortP+ofs2))->x  ||
        ( pointAddrP(dtmP,*(sortP+ofs2-1))->x ==  pointAddrP(dtmP,*(sortP+ofs2))->x  &&
          pointAddrP(dtmP,*(sortP+ofs2-1))->y >   pointAddrP(dtmP,*(sortP+ofs2))->y )    )
      {
       for( i = ofs ; i < ofs+ns ; ++i )
         {
          if     ( ofs1 >= ofs+ns1 ) {*(tempP+i)=*(sortP+ofs2);++ofs2;}
          else if( ofs2 >= ofs+ns  ) {*(tempP+i)=*(sortP+ofs1);++ofs1;}
          else
            {
             if(   pointAddrP(dtmP,*(sortP+ofs1))->x <   pointAddrP(dtmP,*(sortP+ofs2))->x  ||
                 ( pointAddrP(dtmP,*(sortP+ofs1))->x ==  pointAddrP(dtmP,*(sortP+ofs2))->x  &&
                   pointAddrP(dtmP,*(sortP+ofs1))->y <=  pointAddrP(dtmP,*(sortP+ofs2))->y)    )
                  { *(tempP+i) = *(sortP+ofs1) ; ++ofs1 ; }
             else { *(tempP+i) = *(sortP+ofs2) ; ++ofs2 ; }
            }
         }
       for( i = ofs ; i < ofs+ns ; ++i ) *(sortP+i) = *(tempP+i) ;
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
BENTLEYDTM_Public int bcdtmList_setNumberOfSortedPointsDtmObject(BC_DTM_OBJ *dtmP )
/*
** This Function Sets The Number Of Sorted Points
*/
{
 int   ret=DTM_SUCCESS ;
 long  point,process=TRUE ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check Tin Component Of Dtm Object
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Set Number Of Sorted Points
*/
    dtmP->numSortedPoints = dtmP->numPoints ;
/*
**  Scan Points And To First Point Out Of Sort Order
*/
    p1P = dtmP->pointsPP[0] ;
    for( point = 1 ; point < dtmP->numPoints && process == TRUE ; ++point )
      {
       p2P = pointAddrP(dtmP,point)  ;
       if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))
         {
          dtmP->numSortedPoints = point  ;
          process = FALSE ;
         }
       p1P = p2P ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_Public int bcdtmList_checkConnectivityOfDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long messageOption)
/*
**
** This Function Checks The Connectivity  Of a Dtm Feature
** Return Values  == 0  No Errors
**                == 1  System Error
**                == 2  Closure Error
**
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long npnt, spnt, numPnt, flPtr, lastPnt;
 DTMFeatureType dtmFeatureType;
 char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_FEATURE_LIST *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld",dtmFeature) ;
/*
** Check Feature Range
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Get Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Check For Valid Tin Feature
*/
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
   {

/*
**   Check Tin State Feature Has A First Point
*/
    if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull && dtmFeatureP->dtmFeaturePts.firstPoint == dtmP->nullPnt )
      {
       bcdtmWrite_message(1,0,0,"Tin State Feature Has A Null First Point") ;
       ret = 1 ;
      }
/*
**   Check For Valid Feature
*/
    else  if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull )
      {
       dtmFeatureType = dtmFeatureP->dtmFeatureType ;
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature Id = %10I64d ** Dtm Feature Type = %s",dtmFeatureP->dtmFeatureId,dtmFeatureTypeName) ;
/*
**    Scan Feature And Copy Tptr Values To Sptr And Null Out Tptr Values
*/
      numPnt = 0 ;
      spnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
      do
        {
         ++numPnt ;
         lastPnt = spnt ;
         nodeAddrP(dtmP,spnt)->sPtr = nodeAddrP(dtmP,spnt)->tPtr ;
         nodeAddrP(dtmP,spnt)->tPtr = dtmP->nullPnt ;
         if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,spnt,&npnt)) goto errexit ;
         if( spnt == npnt )
           {
            bcdtmWrite_message(2,0,0,"Knot In Triangulated DTM Feature") ;
            goto errexit ;
           }
         spnt = npnt ;
        } while ( spnt != dtmFeatureP->dtmFeaturePts.firstPoint && spnt != dtmP->nullPnt ) ;
       if( spnt == dtmFeatureP->dtmFeaturePts.firstPoint && numPnt > 1 ) lastPnt = dtmP->nullPnt ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",numPnt) ;
/*
**     Copy Dtm Feature To Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Feature To Tptr") ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&spnt)) goto errexit ;
/*
**     Check Points Exist
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Points Exist") ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          npnt = spnt ;
          do
            {
             if( nodeAddrP(dtmP,npnt)->cPtr == dtmP->nullPtr )
               {
                bcdtmWrite_message(1,0,0,"No Circular List For Group Spot Point %8ld",npnt) ;
                goto errexit ;
               }
             npnt = nodeAddrP(dtmP,npnt)->tPtr ;
            }  while( npnt != spnt && npnt != dtmP->nullPnt ) ;
         }
/*
**     Check Connectivity Of Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Tptr List") ;
       if( dtmFeatureP->dtmFeatureType != DTMFeatureType::GroupSpots )
         {
          if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,spnt,1))
            {
             if( messageOption ) bcdtmWrite_message(2,0,0,"** Connectivity Error In Dtm Feature %6ld Type = %20s",dtmFeature,dtmFeatureTypeName) ;
             ret = 2 ;
             goto errexit ;
            }
         }
/*
**    Check Polygonal Features Close
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Polygonal Features Close") ;
       if( dtmFeatureType == DTMFeatureType::Void ||  dtmFeatureType == DTMFeatureType::Void ||  dtmFeatureType == DTMFeatureType::Hole || dtmFeatureType == DTMFeatureType::Region )
         {
          npnt = spnt ;
          do
            {
             npnt = nodeAddrP(dtmP,npnt)->tPtr ;
            } while ( npnt != dtmP->nullPnt && npnt != spnt ) ;
          if( npnt == dtmP->nullPnt )
            {
             if( messageOption ) bcdtmWrite_message(0,0,0,"Closure Error In Feature %6ld Type = %20s",dtmFeature,dtmFeatureTypeName) ;
             ret = 2 ;
             goto errexit ;
            }
         }
/*
**     Check Last Point Of Feature Has A List Entry With Null Next Point
*/
       if( lastPnt != dtmP->nullPnt )
         {
          flPtr = nodeAddrP(dtmP,lastPnt)->fPtr ;
          while ( flPtr != dtmP->nullPtr && lastPnt != dtmP->nullPnt )
            {
             fListP = flistAddrP(dtmP,flPtr) ;
             if( fListP->dtmFeature == dtmFeature && fListP->nextPnt == dtmP->nullPnt ) lastPnt = dtmP->nullPnt ;
             flPtr = fListP->nextPtr ;
            }
          if( lastPnt != dtmP->nullPnt )
            {
             bcdtmWrite_message(2,0,0,"Dtm Feature Last Point Error") ;
             goto errexit ;
            }
         }
/*
**     Null  Tptr List
*/
       if( bcdtmList_nullTptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**     Scan Feature And Copy Sptr Values To Tptr
*/
       spnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       do
         {
          nodeAddrP(dtmP,spnt)->tPtr = nodeAddrP(dtmP,spnt)->sPtr ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,spnt,&npnt)) goto errexit ;
          spnt = npnt ;
         } while ( spnt != dtmFeatureP->dtmFeaturePts.firstPoint && spnt != dtmP->nullPnt ) ;
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Public int bcdtmList_copyDtmFeatureToTptrListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *startPnt)
/*
** This Function Copies A Dtm Feature To A Tptr List
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long flPtr,nextPnt,firstPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List",dtmFeature) ;
/*
** Initialise
*/
 *startPnt = dtmP->nullPnt ;
/*
** Check For Valid Dtm Feature
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Get Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Check For Valid Tin Feature
*/
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Is not A Valid Tin Feature") ;
    goto errexit ;
   }
/*
** Scan Dtm Feature List And Copy To Tptr List
*/
 nextPnt = firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
 flPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    while ( flPtr != dtmP->nullPtr && flistAddrP(dtmP,flPtr)->dtmFeature != dtmFeature ) flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
    if( flPtr != dtmP->nullPtr )
      {
       nodeAddrP(dtmP,nextPnt)->tPtr = flistAddrP(dtmP,flPtr)->nextPnt ;
       nextPnt = flistAddrP(dtmP,flPtr)->nextPnt ;
       if( nextPnt != dtmP->nullPnt ) flPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
       if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) flPtr = dtmP->nullPtr ;
      }
   }
/*
** Set Start Point
*/
 *startPnt = firstPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Error",dtmFeature) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret != DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_copyDtmFeatureToSptrListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *startPnt)
/*
** This Function Copies A Dtm Feature To A Tptr List
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long flPtr,nextPnt,firstPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Sptr List",dtmFeature) ;
/*
** Initialise
*/
 *startPnt = dtmP->nullPnt ;
/*
** Check For Valid Dtm Feature
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Get Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Check For Valid Tin Feature
*/
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Is not A Valid Tin Feature") ;
    goto errexit ;
   }
/*
** Scan Dtm Feature List And Copy To Tptr List
*/
 nextPnt = firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
 flPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    while ( flPtr != dtmP->nullPtr && flistAddrP(dtmP,flPtr)->dtmFeature != dtmFeature ) flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
    if( flPtr != dtmP->nullPtr )
      {
       nodeAddrP(dtmP,nextPnt)->sPtr = flistAddrP(dtmP,flPtr)->nextPnt ;
       nextPnt = flistAddrP(dtmP,flPtr)->nextPnt ;
       if( nextPnt != dtmP->nullPnt ) flPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
       if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) flPtr = dtmP->nullPtr ;
      }
   }
/*
** Set Start Point
*/
 *startPnt = firstPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Error",dtmFeature) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret != DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_copyAllDtmFeatureTypePointsToPointArraysDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTM_POINT_ARRAY ***featuresPPP,long *numFeaturesP)
/*
** This Function Copies Feature Points To A Point Array
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,numDtmFeaturePts,memDtmFeatures=0,memDtmFeaturesInc=1000 ;
 DPoint3d  *dtmFeaturePtsP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type Points To A Point Arrays") ;
/*
** Initialise
*/
 if( *featuresPPP != NULL ) bcdtmMem_freePointerArrayToPointArrayMemory(featuresPPP,*numFeaturesP) ;
 *numFeaturesP = 0 ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Scan Features And Copy Points
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
/*
**     Get Points For Feature
*/
       if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **)&dtmFeaturePtsP,&numDtmFeaturePts)) goto errexit ;
/*
**     Check Pointer Array To Point Arrays
*/
       if( *numFeaturesP == memDtmFeatures )
         {
          memDtmFeatures = memDtmFeatures + memDtmFeaturesInc ;
          if( *featuresPPP == NULL ) *featuresPPP = ( DTM_POINT_ARRAY ** ) malloc ( memDtmFeatures * sizeof( DTM_POINT_ARRAY *)) ;
          else                         *featuresPPP = ( DTM_POINT_ARRAY ** ) realloc ( *featuresPPP , memDtmFeatures * sizeof( DTM_POINT_ARRAY *)) ;
         }
/*
**     Store Feature Points In Pointer Array To Point Arrays
*/
       (*(*featuresPPP+*numFeaturesP))->pointsP   = dtmFeaturePtsP ;
       (*(*featuresPPP+*numFeaturesP))->numPoints = numDtmFeaturePts ;
       ++*numFeaturesP ;
       dtmFeaturePtsP = NULL ;
      }
   }
/*
** Realloc Memory
*/
 if( *featuresPPP != NULL && *numFeaturesP != memDtmFeatures )
   {
    if( *numFeaturesP > 0 && *numFeaturesP < memDtmFeatures ) *featuresPPP = ( DTM_POINT_ARRAY ** ) realloc ( *featuresPPP , memDtmFeatures * sizeof( DTM_POINT_ARRAY *)) ;
    else                                                      {  free(*featuresPPP) ; *featuresPPP = NULL ;  }
   }

/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type Points To A Point Arrays Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type Points To A Point Arrays Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmMem_freePointerArrayToPointArrayMemory(featuresPPP,*numFeaturesP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,DPoint3d **featPtsPP,long *numFeatPtsP)
/*
** This Function Copies Feature Points To A Point Array
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;

/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array") ;
/*
** Initialise
*/
 *numFeatPtsP = 0 ;
 if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
/*
** Test For Valid DTM Object
*/
 // if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Get Points For Dtm Feature
*/
 if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **)featPtsPP,numFeatPtsP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Error") ;
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
BENTLEYDTM_Public int bcdtmList_copyDtmFeaturePointsToPointOffsetArrayDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long **featPtsPP,long *numFeatPtsP)
/*
** This Function Copies Feature Points To A Point Array
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array") ;
/*
** Initialise
*/
 *numFeatPtsP = 0 ;
 if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Get Points For Dtm Feature
*/
 if( bcdtmObject_getPointOffsetsForDtmFeatureDtmObject(dtmP,dtmFeature,featPtsPP,numFeatPtsP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Error") ;
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
BENTLEYDTM_Public int bcdtmList_countTrianglesAndLinesDtmObject(BC_DTM_OBJ *dtmP,long *numTrianglesP, long *numLinesP)
/*
** This Function Counts The Number Of Triangles and Lines in The DTMFeatureState::Tin
*/
{
 int    ret=DTM_SUCCESS ;
 long   p1,p2,p3,clPtr ;
 DTM_CIR_LIST  *cListP ;
/*
** Initialise Variables
*/
 *numTrianglesP = *numLinesP = 0 ;
/*
** Scan All Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
         {
          cListP = clistAddrP(dtmP,clPtr) ;
          p3     = cListP->pntNum ;
          clPtr  = cListP->nextPtr ;
          if( p2 > p1 ) ++*numLinesP ;
          if( p2 > p1 && p3 > p1 ) if( nodeAddrP(dtmP,p1)->hPtr != p2 ) ++*numTrianglesP ;
          p2  = p3 ;
         }
      }
   }
/*
**  Set Tin Values
*/
 dtmP->numLines = *numLinesP ;
 dtmP->numTriangles = *numTrianglesP ;
/*
** Cleanup
*/
 cleanup :
/*
**  Job Completed
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
BENTLEYDTM_EXPORT int bcdtmList_countNonVoidTrianglesAndLinesDtmObject(BC_DTM_OBJ *dtmP,long *numTrianglesP, long *numLinesP)
/*
** This Function Counts The Number Of Triangles and Lines in The DTMFeatureState::Tin
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,clPtr,voidsInDtm=FALSE,voidLine=FALSE,voidTriangle=TRUE ;
 DTM_CIR_LIST  *cListP ;
/*
** Initialise Variables
*/
 *numLinesP = 0 ;
 *numTrianglesP = 0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**   Check For Voids In Dtm
*/
     bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
     if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**   No Voids In DTM
*/
     if( voidsInDtm == FALSE )
       {
        *numLinesP     = dtmP->numLines ;
        *numTrianglesP = dtmP->numTriangles ;
       }
/*
**  Count None Void Triangles And Lines
*/
    else
      {
       for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
         {
          clPtr = nodeAddrP(dtmP,p1)->cPtr ;
          if( clPtr != dtmP->nullPtr )
            {
             if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
             while( clPtr != dtmP->nullPtr )
               {
                cListP = clistAddrP(dtmP,clPtr) ;
                p3     = cListP->pntNum ;
                clPtr  = cListP->nextPtr ;
                if( p2 > p1 )
                  {
                   if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit ;
                   if( ! voidLine ) ++*numLinesP ;
                  }
                if( p2 > p1 && p3 > p1 ) if( nodeAddrP(dtmP,p1)->hPtr != p2 )
                  {
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( ! voidTriangle )++*numTrianglesP ;
                  }
                p2  = p3 ;
               }
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
**  Job Completed
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
BENTLEYDTM_Public int bcdtmList_checkConnectivityTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long messageFlag)
/*
** This Function Checks The Connectivity Of A Tptr Polygon
**
** Return Values == DTM_SUCCESS  No Connectivity Errors
**               == DTM_ERROR    Connectivity Errors
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,tp,lp=0,knot ;
/*
** Validate Tptr Polygon Start Point
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
 if( nodeAddrP(dtmP,startPnt)->tPtr < 0 || nodeAddrP(dtmP,startPnt)->tPtr >= dtmP->numPoints  )
   {
    bcdtmWrite_message(2,0,0,"Point->Tptr Range Error") ;
    goto errexit ;
   }
/*
** Check Tptr List Connectivity
*/
 sp = startPnt ;
 while ( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,sp)->tPtr >= 0 )
   {
    if( ! bcdtmList_testLineDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->tPtr) )
      {
       bcdtmWrite_message(2,0,0,"Unconnected Points %8ld %8ld In Tptr Polygon",sp,nodeAddrP(dtmP,sp)->tPtr) ;
       goto errexit ;
      }
    tp = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = -(nodeAddrP(dtmP,sp)->tPtr + 1) ;
    sp = tp ;
   }
/*
** Set Value Of Last Point In List
*/
 knot = 0 ;
 if( nodeAddrP(dtmP,sp)->tPtr < 0 ) { lp = sp ; knot = 1 ; }
/*
** Reset Tptr Values Positive
*/
 sp = startPnt ;
 while( nodeAddrP(dtmP,sp)->tPtr < 0  )
   {
    tp = -(nodeAddrP(dtmP,sp)->tPtr + 1 ) ;
    nodeAddrP(dtmP,sp)->tPtr = tp ;
    sp = tp ;
   }
/*
** Test For Knot In List
*/
 if( knot && lp != startPnt )
   {
    bcdtmWrite_message(2,0,0,"Tptr Polygon Knot At Point %8ld ** %12.5lf %12.5lf %10.4lf ",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
    goto errexit ;
   }
/*
** Must Detect A Knot At The Start Point For A Polygon
*/
 if( ! knot )
   {
    if( messageFlag ) bcdtmWrite_message(2,0,0,"Tptr Polygon Does Not Close") ;
    goto errexit ;
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
BENTLEYDTM_Public int bcdtmList_checkConnectivitySptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long messageFlag)
/*
** This Function Checks The Connectivity Of A Sptr Polygon
**
** Return Values == DTM_SUCCESS  No Connectivity Errors
**               == DTM_ERROR    Connectivity Errors
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,tp,lp=0,knot ;
/*
** Validate Sptr Polygon Start Point
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
 if( nodeAddrP(dtmP,startPnt)->sPtr < 0 || nodeAddrP(dtmP,startPnt)->sPtr >= dtmP->numPoints  )
   {
    bcdtmWrite_message(2,0,0,"Point->Sptr Range Error") ;
    goto errexit ;
   }
/*
** Check Sptr List Connectivity
*/
 sp = startPnt ;
 while ( nodeAddrP(dtmP,sp)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,sp)->sPtr >= 0 )
   {
    if( ! bcdtmList_testLineDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->sPtr) )
      {
       bcdtmWrite_message(2,0,0,"Unconnected Points %8ld %8ld In Sptr Polygon",sp,nodeAddrP(dtmP,sp)->sPtr) ;
       goto errexit ;
      }
    tp = nodeAddrP(dtmP,sp)->sPtr ;
    nodeAddrP(dtmP,sp)->sPtr = -(nodeAddrP(dtmP,sp)->sPtr + 1) ;
    sp = tp ;
   }
/*
** Set Value Of Last Point In List
*/
 knot = 0 ;
 if( nodeAddrP(dtmP,sp)->sPtr < 0 ) { lp = sp ; knot = 1 ; }
/*
** Reset Sptr Values Positive
*/
 sp = startPnt ;
 while( nodeAddrP(dtmP,sp)->sPtr < 0  )
   {
    tp = -(nodeAddrP(dtmP,sp)->sPtr + 1 ) ;
    nodeAddrP(dtmP,sp)->sPtr = tp ;
    sp = tp ;
   }
/*
** Test For Knot In List
*/
 if( knot && lp != startPnt )
   {
    bcdtmWrite_message(2,0,0,"Sptr Polygon Knot At Point %8ld ** %12.5lf %12.5lf %10.4lf ",lp,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
    goto errexit ;
   }
/*
** Must Detect A Knot At The Start Point For A Polygon
*/
 if( ! knot )
   {
    bcdtmWrite_message(2,0,0,"Sptr Polygon Does Not Close") ;
    goto errexit ;
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
BENTLEYDTM_Public int bcdtmList_cleanTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Cleans A Tptr Polygon
*/
{
 long  sp,np,node,partNum,partOfs ;
 DTM_TIN_NODE *nodeP   ;
/*
** Mark Tptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr  = -(np+1) ;
    sp  = np ;
   } while ( sp != startPnt ) ;
/*
** Set All Non Negative Tptr Values To Null
*/
 partNum = 0 ;
 partOfs = 0 ;
 nodeP = dtmP->nodesPP[partNum] ;
 for( node = 0 ; node < dtmP->numPoints ; ++node )
    {
    if( nodeP->tPtr >= 0 ) nodeP->tPtr = dtmP->nullPnt ;
    ++partOfs ;
    if( partOfs == dtmP->nodePartitionSize )
      {
       partOfs = 0 ;
       ++partNum   ;
       nodeP = dtmP->nodesPP[partNum] ;
      }
    else  ++nodeP ;
   }
/*
** Reset Tptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = -(nodeAddrP(dtmP,sp)->tPtr+1) ;
    nodeAddrP(dtmP,sp)->tPtr  = np ;
    sp  = np ;
   } while ( sp != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_cleanSptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Cleans A Tptr Polygon
*/
{
 long  sp,np,node,partNum,partOfs ;
 DTM_TIN_NODE *nodeP   ;
/*
** Mark Sptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->sPtr ;
    nodeAddrP(dtmP,sp)->sPtr  = -(np+1) ;
    sp  = np ;
   } while ( sp != startPnt ) ;
/*
** Set All Non Negative Sptr Values To Null
*/
 partNum = 0 ;
 partOfs = 0 ;
 nodeP = dtmP->nodesPP[partNum] ;
 for( node = 0 ; node < dtmP->numPoints ; ++node )
    {
    if( nodeP->sPtr >= 0 ) nodeP->sPtr = dtmP->nullPnt ;
    ++partOfs ;
    if( partOfs == dtmP->nodePartitionSize )
      {
       partOfs = 0 ;
       ++partNum   ;
       nodeP = dtmP->nodesPP[partNum] ;
      }
    else  ++nodeP ;
   }
/*
** Reset Sptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = -(nodeAddrP(dtmP,sp)->sPtr+1) ;
    nodeAddrP(dtmP,sp)->sPtr  = np ;
    sp  = np ;
   } while ( sp != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_countNumberOfPointsForDtmTinFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *numPtsP)
/*
** This Function Counts The Number Of Points For A Feature Reference
*/
{
 int   ret=DTM_SUCCESS ;
 long  nextPnt=0,firstPnt,fListPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *numPtsP  = 0 ;
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Set Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Scan Tin Feature
*/
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
   {
    *numPtsP = 1;
    firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
    fListPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
/*
**  Scan Dtm Feature Points
*/
    while ( fListPtr != dtmP->nullPtr )
      {
       while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
       if( fListPtr != dtmP->nullPtr )
         {
          nextPnt  = flistAddrP(dtmP,fListPtr)->nextPnt ;
          if( nextPnt != dtmP->nullPnt )
            {
             ++(*numPtsP) ;
             fListPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
            }
          if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) fListPtr = dtmP->nullPtr ;
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
BENTLEYDTM_Public int bcdtmList_setPntTypeForForDtmTinFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long pntType)
/*
** This Function Counts The Number Of Points For A Feature Reference
*/
{
 int   ret=DTM_SUCCESS ;
 long  nextPnt=0,firstPnt,fListPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Set Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Scan Tin Feature
*/
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
   {
    firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
    fListPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
/*
**  Scan Dtm Feature Points
*/
    while ( fListPtr != dtmP->nullPtr )
      {
       while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
       if( fListPtr != dtmP->nullPtr )
         {
         flistAddrP(dtmP,fListPtr)->pntType = pntType;
          nextPnt  = flistAddrP(dtmP,fListPtr)->nextPnt ;
          if( nextPnt != dtmP->nullPnt )
            {
             fListPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
            }
          if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) fListPtr = dtmP->nullPtr ;
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
BENTLEYDTM_Public int bcdtmList_reverseTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long firstPnt )
/*
** This Function Reverses The Direction Of A tPtr Polygon
*/
{
 long p1,p2,p3 ;
/*
** Get Prior Pointer To firstPnt
*/
 p1 = p2 = firstPnt ;
 while ( nodeAddrP(dtmP,p1)->tPtr != firstPnt ) p1 = nodeAddrP(dtmP,p1)->tPtr ;
 do
   {
    p3 = nodeAddrP(dtmP,p2)->tPtr ;
    nodeAddrP(dtmP,p2)->tPtr = p1 ;
    p1 = p2 ; p2 = p3   ;
   } while ( p2 != firstPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_reverseSptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long firstPnt )
/*
** This Function Reverses The Direction Of A sPtr Polygon
*/
{
 long p1,p2,p3 ;
/*
** Get Prior Pointer To firstPnt
*/
 p1 = p2 = firstPnt ;
 while ( nodeAddrP(dtmP,p1)->sPtr != firstPnt ) p1 = nodeAddrP(dtmP,p1)->sPtr ;
 do
   {
    p3 = nodeAddrP(dtmP,p2)->sPtr ;
    nodeAddrP(dtmP,p2)->sPtr = p1 ;
    p1 = p2 ; p2 = p3   ;
   } while ( p2 != firstPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_reverseDtmTinFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Reverses The Order Of Points In A Dtm Feature
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  clPtr,nextPnt,priorPnt,firstPnt,tempPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reversing Dtm Tin Feature %6ld",dtmFeature) ;
/*
** Only Reverse If Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
/*
**     Scan Dtm Feature Points And Reverse Order
*/
       priorPnt = dtmP->nullPnt ;
       nextPnt  = firstPnt ;
       clPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
          if( clPtr != dtmP->nullPtr )
            {
             tempPnt  = flistAddrP(dtmP,clPtr)->nextPnt ;
             flistAddrP(dtmP,clPtr)->nextPnt = priorPnt ;
             priorPnt = nextPnt ;
             nextPnt  = tempPnt ;
             if( nextPnt != dtmP->nullPnt ) clPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
            }
          if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) clPtr = dtmP->nullPtr  ;
         }
/*
**     If Dtm Feature Closes Connect First Point To Prior Point
*/
       if( nextPnt == firstPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature Closes") ;
          clPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
          while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
          if( clPtr != dtmP->nullPtr )  flistAddrP(dtmP,clPtr)->nextPnt = priorPnt ;
         }
/*
**     Update First Point For Feature
*/
       ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = priorPnt ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reversing Dtm Tin Feature %6ld Completed",dtmFeature) ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testForPointInTinFeatureListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long point,long *inListP)
/*
** This Function Tests For A Point Being In A Dtm Feature List
*/
{
 long flPtr ;
/*
** Initialise
*/
 *inListP = 0 ;
 if( point >= 0 || point <= dtmP->numPoints )
   {
    flPtr = nodeAddrP(dtmP,point)->fPtr ;
/*
**  Scan Feature List For Point
*/
    while ( flPtr != dtmP->nullPtr && ! *inListP )
      {
       if(flistAddrP(dtmP,flPtr)->dtmFeature == dtmFeature ) *inListP = 1 ;
       flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_getFirstAndLastPointForDtmTinFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *firstPntP,long *lastPntP)
/*
** This Function Gets The First And Last Point For A Dtm Feature
**
** On return if :-
** firstPnt == DTM_NULL_PNT and lastPnt == DTM_NULL_PNT  - Dtm Feature Does Not Exist
** firstPnt == lastPnt                         - Dtm Feature Closes
** firstPnt != DTM_NULL_PNT and lastPnt == DTM_NULL_PNT  - Dtm Feature Has Only One Point
**
*/
{
 int   ret=DTM_SUCCESS ;
 long  nextPnt,listPtr ;
/*
** Initialise
*/
 *firstPntP = *lastPntP = dtmP->nullPnt ;
/*
** Scan Dtm Feature
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures && ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
   {
    *firstPntP = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
    listPtr    = nodeAddrP(dtmP,*firstPntP)->fPtr ;
/*
** Scan Dtm Feature Points
*/
    while ( listPtr != dtmP->nullPtr )
      {
       nextPnt = dtmP->nullPnt ;
       while( listPtr != dtmP->nullPtr  && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
       if( listPtr != dtmP->nullPtr )
         {
          nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
          if( nextPnt != dtmP->nullPnt )
            {
             *lastPntP = nextPnt ;
             listPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
            }
         }
       if( nextPnt == dtmP->nullPnt || nextPnt == *firstPntP ) listPtr = dtmP->nullPtr ;
      }
   }
/*
** Job Completed
*/
 return(ret) ;
}
/*------------------------------------------------------------+
|                                                             |
|                                                             |
|                                                             |
+------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_removeNoneFeatureHullLinesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Removes None Feature Hull Lines
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,np,ap ;
 int isModified = FALSE;
/*
** Write Entry Messages
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting None Feature Hull Lines") ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Scan Around Edge And Remove Non Feature Hull Lines
*/
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
    if( (ap = bcdtmList_nextAntDtmObject(dtmP,sp,np) ) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt && ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,np) )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Deleting Hull Line ** p1 = %8ld p2 = %8ld",sp,np) ;
       if( bcdtmList_deleteLineDtmObject(dtmP,sp,np)) goto errexit ;
       isModified = TRUE;
       nodeAddrP(dtmP,sp)->hPtr = ap ;
       nodeAddrP(dtmP,ap)->hPtr = np ;
       np = ap ;
       if( (ap = bcdtmList_nextAntDtmObject(dtmP,sp,np) ) < 0 ) goto errexit ;
      }
    sp = np ;
   } while ( sp != dtmP->hullPoint ) ;
/*
** Reset Triangle Counts
*/
 bcdtmList_countTrianglesAndLinesDtmObject(dtmP,&sp,&np) ;
/*
**  Update Modified Time
 */
 if (isModified)
   bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting None Feature Hull Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting None Feature Hull Lines Error") ;
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
BENTLEYDTM_Public int bcdtmList_getDtmFeaturesForPointDtmObject(BC_DTM_OBJ *dtmP,long point,DTM_TIN_POINT_FEATURES **pointFeaturesPP,long *numPointFeaturesP)
/*
** This Function Gets The Dtm Features For A Point
** Rob Cormack June 2003
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,np,nextPnt,priorPnt,listPnt,cirListPtr,lpFeatPtr,pntFeatPtr,dtmFeature,memFeatureTable=0,memFeatureTableInc=10 ;
/*
** Initialise
*/
 *numPointFeaturesP = 0 ;
 if( *pointFeaturesPP != NULL ) { free(*pointFeaturesPP) ; *pointFeaturesPP = NULL ; }
/*
** Scan Point Feature List
*/
 pntFeatPtr = nodeAddrP(dtmP,point)->fPtr ;
 while ( pntFeatPtr != dtmP->nullPtr )
   {
    dtmFeature = flistAddrP(dtmP,pntFeatPtr)->dtmFeature ;
    nextPnt    = flistAddrP(dtmP,pntFeatPtr)->nextPnt ;
/*
** Get Prior Point For Feature
*/
    priorPnt = dtmP->nullPnt ;
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::GroupSpots )
      {
       sp = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
          if( np == point ) priorPnt = sp ;
          sp = np ;
         } while ( priorPnt == dtmP->nullPnt && sp != dtmP->nullPnt && sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
      }
    else
      {
       cirListPtr = nodeAddrP(dtmP,point)->cPtr ;
       while( cirListPtr != dtmP->nullPtr && priorPnt == dtmP->nullPnt )
         {
          listPnt    = clistAddrP(dtmP,cirListPtr)->pntNum ;
          cirListPtr = clistAddrP(dtmP,cirListPtr)->nextPtr ;
          lpFeatPtr  = nodeAddrP(dtmP,listPnt)->fPtr ;
          while ( lpFeatPtr != dtmP->nullPtr && priorPnt == dtmP->nullPnt )
            {
             if( flistAddrP(dtmP,lpFeatPtr)->dtmFeature == dtmFeature && flistAddrP(dtmP,lpFeatPtr)->nextPnt == point ) priorPnt = listPnt ;
             lpFeatPtr = flistAddrP(dtmP,lpFeatPtr)->nextPtr ;
            }
         }
      }
/*
** Allocate memory If Necessary
*/
    if( *numPointFeaturesP == memFeatureTable )
      {
       memFeatureTable = memFeatureTable + memFeatureTableInc ;
       if( *pointFeaturesPP == NULL ) *pointFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
       else                           *pointFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc ( *pointFeaturesPP , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
       if( *pointFeaturesPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
** Store Point Feature
*/
    (*pointFeaturesPP + *numPointFeaturesP)->dtmFeature     =  dtmFeature ;
    (*pointFeaturesPP + *numPointFeaturesP)->dtmFeatureType =  ftableAddrP(dtmP,dtmFeature)->dtmFeatureType ;
    (*pointFeaturesPP + *numPointFeaturesP)->userTag        =  ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
    (*pointFeaturesPP + *numPointFeaturesP)->userFeatureId       =  ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
    (*pointFeaturesPP + *numPointFeaturesP)->priorPoint     =  priorPnt ;
    (*pointFeaturesPP + *numPointFeaturesP)->nextPoint      =  nextPnt ;
    ++*numPointFeaturesP ;
/*
** Reset For Next Feature
*/
    pntFeatPtr = flistAddrP(dtmP,pntFeatPtr)->nextPtr ;
   }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_getDtmFeaturesForLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,DTM_TIN_POINT_FEATURES **lineFeaturesPP,long *numLineFeaturesP)
/*
** This Function Gets The List Of Dtm Features For Line pnt1-pnt2
**
** Rob Cormack June 2003
*/
{
 int  ret=DTM_SUCCESS ;
 long cln,feature,memFeatureTable=0,memInc=10 ;
/*
** Initialise
*/
 *numLineFeaturesP = 0 ;
 if( *lineFeaturesPP != NULL ) { free(*lineFeaturesPP) ; *lineFeaturesPP = NULL ; }
/*
** Scan pnt1 And Get Connection To pnt2
*/
 cln = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( cln != dtmP->nullPtr )
   {
    feature = flistAddrP(dtmP,cln)->dtmFeature ;
    if( flistAddrP(dtmP,cln)->nextPnt == pnt2 )
      {
/*
**     Allocate memory If Necessary
*/
       if( *numLineFeaturesP == memFeatureTable )
         {
          memFeatureTable = memFeatureTable + memInc ;
          if( *lineFeaturesPP == NULL ) *lineFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          else                          *lineFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc ( *lineFeaturesPP , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          if( *lineFeaturesPP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Point Feature
*/
       (*lineFeaturesPP + *numLineFeaturesP)->dtmFeature     =  feature ;
       (*lineFeaturesPP + *numLineFeaturesP)->dtmFeatureType =  ftableAddrP(dtmP,feature)->dtmFeatureType ;
       (*lineFeaturesPP + *numLineFeaturesP)->userTag        =  ftableAddrP(dtmP,feature)->dtmUserTag ;
       (*lineFeaturesPP + *numLineFeaturesP)->userFeatureId  =  ftableAddrP(dtmP,feature)->dtmFeatureId ;
       (*lineFeaturesPP + *numLineFeaturesP)->priorPoint     =  pnt1 ;
       (*lineFeaturesPP + *numLineFeaturesP)->nextPoint      =  pnt2 ;
       ++*numLineFeaturesP ;
      }
/*
**   Reset For Next Feature
*/
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
/*
** Scan pnt2 And Get For Connection To pnt1
*/
 cln = nodeAddrP(dtmP,pnt2)->fPtr ;
 while ( cln != dtmP->nullPtr )
   {
    feature = flistAddrP(dtmP,cln)->dtmFeature ;
    if( flistAddrP(dtmP,cln)->nextPnt == pnt1 )
      {
/*
** Allocate memory If Necessary
*/
       if( *numLineFeaturesP == memFeatureTable )
         {
          memFeatureTable = memFeatureTable + memInc ;
          if( *lineFeaturesPP == NULL ) *lineFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          else                          *lineFeaturesPP = ( DTM_TIN_POINT_FEATURES * ) realloc ( *lineFeaturesPP , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          if( *lineFeaturesPP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Point Feature
*/
       (*lineFeaturesPP + *numLineFeaturesP)->dtmFeature     =  feature ;
       (*lineFeaturesPP + *numLineFeaturesP)->dtmFeatureType =  ftableAddrP(dtmP,feature)->dtmFeatureType ;
       (*lineFeaturesPP + *numLineFeaturesP)->userTag        =  ftableAddrP(dtmP,feature)->dtmUserTag ;
       (*lineFeaturesPP + *numLineFeaturesP)->userFeatureId  =  ftableAddrP(dtmP,feature)->dtmFeatureId ;
       (*lineFeaturesPP + *numLineFeaturesP)->priorPoint     =  pnt2 ;
       (*lineFeaturesPP + *numLineFeaturesP)->nextPoint      =  pnt1 ;
       ++*numLineFeaturesP ;
      }
/*
** Reset For Next Feature
*/
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testForBreakPointDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Tests If A dtmP Point Is On A Break Line
*/
{
 long flist ;
/*
** Scan Point Features
*/
 flist = nodeAddrP(dtmP,point)->fPtr ;
 while ( flist != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,flist)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) return(1) ;
    flist = flistAddrP(dtmP,flist)->nextPtr ;
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
BENTLEYDTM_EXPORT int bcdtmList_testForBreakLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1P2 or Line P2P1 is A Break Line
*/
{
 long clc ;
/*
** Test For P1 P2 Being Break Line
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Test For P2 P1 Being Break Line
*/
 clc = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForValidTriangleDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long pnt3)
/*
** This Routine Test if the Triangle defined by
** Points p1,p2,p3 is a valid Triangle
*/
{
 int   sideof ;
 long  clc,tp,notConnect ;
/*
** Arrange Points So That pnt1,pnt2,pnt3 is Anti Clockwise
*/
 sideof = bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) ;
 if( sideof == 0 ) return(0) ;
 if( sideof <  0 ) { tp = pnt2 ; pnt2 = pnt3 ; pnt3 = tp ; }
/*
** Check That pnt3 Connects To pnt2
*/
 notConnect = 1 ;
 clc = nodeAddrP(dtmP,pnt3)->cPtr ;
 while ( clc != dtmP->nullPtr && notConnect )
   {
    if( clistAddrP(dtmP,clc)->pntNum == pnt2 ) notConnect = 0 ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
   }
 if( notConnect ) return(0) ;
/*
** Check That pnt1 Connects To pnt2
*/
 notConnect = 1 ;
 clc = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clc != dtmP->nullPtr && notConnect )
   {
    if( clistAddrP(dtmP,clc)->pntNum == pnt2 ) notConnect = 0 ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
   }
 if( notConnect ) return(0) ;
/*
** Check That pnt1 Connects To pnt3
*/
 notConnect = 1 ;  tp = dtmP->nullPnt ;
 clc = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clc != dtmP->nullPtr && notConnect )
   {
    if( tp == dtmP->nullPnt ) tp = clistAddrP(dtmP,clc)->pntNum ;
    if( clistAddrP(dtmP,clc)->pntNum == pnt3 ) notConnect = 0 ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
   }
 if( notConnect ) return(0) ;
/*
** Check That pnt2 is the Next Point Clockwise From pnt3 about pnt1
*/
 if( clc != dtmP->nullPtr && clistAddrP(dtmP,clc)->pntNum != pnt2 ) return(0) ;
 if( clc == dtmP->nullPtr && tp != pnt2 ) return(0) ;
/*
** Job Completed
*/
 return(1) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_extractHullDtmObject(BC_DTM_OBJ *dtmP, DPoint3d **hullPtsPP, long *numHullPtsP )
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long  np,dtmFeature ;
 DPoint3d   *hullP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT *pointP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Extracting Hull From DTM") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->dtmState = %8ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"*hullPtsPP     = %p",*hullPtsPP) ;
    bcdtmWrite_message(0,0,0,"*numHullPts    = %8ld",*numHullPtsP) ;
   }
/*
** Initialise
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != NULL ) { free(*hullPtsPP) ; *hullPtsPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** DTM In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg )  bcdtmWrite_message(0,0,0,"Extracting Hull From DTM Tin") ;
/*
**  Count Number of Points In Hull
*/
    *numHullPtsP = 0 ;
    np = dtmP->hullPoint  ;
    do
      {
       ++*numHullPtsP ;
       np = nodeAddrP(dtmP,np)->hPtr ;
      }
    while ( np != dtmP->hullPoint ) ;
    ++*numHullPtsP ;
/*
**  Allocate Memory To Store Hull Points
*/
    *hullPtsPP = ( DPoint3d * ) malloc(*numHullPtsP*sizeof(DPoint3d)) ;
    if( *hullPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Store Hull Points
*/
    hullP = *hullPtsPP ;
    np = dtmP->hullPoint ;
    do
      {
       pointP = pointAddrP(dtmP,np) ;
       hullP->x = pointP->x  ;
       hullP->y = pointP->y  ;
       hullP->z = pointP->z  ;
       ++hullP ;
       np = nodeAddrP(dtmP,np)->hPtr ;
      } while ( np != dtmP->hullPoint )  ;
    pointP = pointAddrP(dtmP,np) ;
    hullP->x = pointP->x ;
    hullP->y = pointP->y ;
    hullP->z = pointP->z ;
   }
/*
**  DTM Not In Tin State
*/
 else if( dtmP->dtmState == DTMState::Data )
   {
    if( dbg )  bcdtmWrite_message(0,0,0,"Extracting Hull From DTM Data") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && *numHullPtsP == 0 ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,hullPtsPP,numHullPtsP)) goto errexit ;
         }
      }
   }
 else
   {
    bcdtmWrite_message(2,0,0,"Invalid DTM State For Method") ;
    goto errexit ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Extracting Hull From DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Extracting Hull From DTM Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numHullPtsP = 0 ;
 if( *hullPtsPP != NULL ) { free(*hullPtsPP) ; *hullPtsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Tests If point is On An Island Void Or Hole Hull
*/
{
 long flPtr,onHull=0 ;
/*
** Test If A Point Is On Island Void Or Hole Hull Line
*/
 flPtr = nodeAddrP(dtmP,point)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
        ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
        ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) ++onHull ;
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
   }
/*
** Job Completed
*/
 return(onHull) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testForLineOnAnIslandOrVoidHullDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2)
/*
** This Function Tests If The Line pnt1-pnt2 is On A Void,Hole Or Island Hull
*/
{
 long flPtr ;
/*
** Scan pnt1 And Test For Void, Hole Or Island Hull
*/
 flPtr = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
   }
/*
** Scan pnt2 And Test For Void, Hole Or Island Hull
*/
 flPtr = nodeAddrP(dtmP,pnt2)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_getVoidExternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long *voidFeatureP)
/*
** This Function Gets The Void Immediately External To A Island
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    hullPnt,nextPnt,priorPnt,listPnt,lowPnt,highPnt,intersectResult;
 long    firstPnt,listPtr,dtmFeature,voidPoint,numIslandPts=0,numVoidPts=0,noVoidPoint ;
 DPoint3d     *islandPtsP=NULL,*voidPtsP=NULL ;
 DTM_DAT_OBJ *dataP=NULL ;
 wchar_t   outFile[128] ;
 static long fileNum=0 ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void External To Island") ;
/*
** Initialise
*/
 lowPnt  = dtmP->numPoints ;
 highPnt = 0 ;
 *voidFeatureP = dtmP->nullPnt  ;
/*
** Only Process If Island Feature
*/
 if( islandFeature >= 0 && islandFeature < dtmP->numFeatures )
   {
    if( ftableAddrP(dtmP,islandFeature)->dtmFeatureState == DTMFeatureState::Tin && ftableAddrP(dtmP,islandFeature)->dtmFeatureType  == DTMFeatureType::Island )
      {
/*
**     Scan Island Hull And Look For A Corresponding Void Hull Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Island Hull") ;
       noVoidPoint = TRUE ;
       priorPnt = ftableAddrP(dtmP,islandFeature)->dtmFeaturePts.firstPoint ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,priorPnt,&hullPnt) ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,hullPnt,&nextPnt) ;
       firstPnt = hullPnt ;
       do
         {
/*
**        Check If Island Point Is A Void Point
*/
          voidPoint = FALSE ;
          listPtr = nodeAddrP(dtmP,hullPnt)->fPtr ;
          while( listPtr != dtmP->nullPtr && voidPoint == FALSE )
            {
             dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ;
             listPtr    = flistAddrP(dtmP,listPtr)->nextPtr ;
             if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Void && ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt ) voidPoint = TRUE ;
            }
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Hull Point = %8ld ** Void Point = %1ld",hullPnt,voidPoint) ;
/*
**        If Void Point Test For Surrounding Void
*/
          if( voidPoint == TRUE )
            {
             noVoidPoint = FALSE ;
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Testing For Surrounding Void At Hull Point = %8ld ** %12.4lf %12.4lf %10.4lf",hullPnt,pointAddrP(dtmP,hullPnt)->x,pointAddrP(dtmP,hullPnt)->y,pointAddrP(dtmP,hullPnt)->z) ;
             listPnt = nextPnt ;
             while ( listPnt != priorPnt && *voidFeatureP == dtmP->nullPnt )
               {
                if( bcdtmList_testForSurroundingDtmFeatureTypeDtmObject(dtmP,hullPnt,nextPnt,priorPnt,DTMFeatureType::Void,voidFeatureP) != DTM_SUCCESS ) goto errexit ;
                if( *voidFeatureP == dtmP->nullPnt ) noVoidPoint = TRUE ;
                if(( listPnt = bcdtmList_nextClkDtmObject(dtmP,hullPnt,listPnt)) < 0 ) goto errexit ;
                if( dbg && *voidFeatureP != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Surrounding Void Found = %6ld",*voidFeatureP) ;
               }
            }
/*
**        Set For Next Island Hull Point
*/
          priorPnt = hullPnt ;
          hullPnt  = nextPnt ;
          bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,hullPnt,&nextPnt) ;
         } while ( hullPnt != firstPnt && *voidFeatureP == dtmP->nullPnt ) ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Hull Point = %8ld",hullPnt) ;
/*
**     Scan Out From Island Hull Looking For Surrounding Void
*/
       if( noVoidPoint == TRUE )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Out From Island Hull") ;
          if( bcdtmList_getVoidFeatureExternalToIslandFeatureDtmObject(dtmP,islandFeature,voidFeatureP) ) goto errexit ;
/*
**        Check Void Feature Surrounds Island
*/
          if( *voidFeatureP != dtmP->nullPnt )
            {
             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,islandFeature,&islandPtsP,&numIslandPts) ) goto errexit ;
             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,*voidFeatureP,&voidPtsP,&numVoidPts) ) goto errexit ;
             if( bcdtmClip_checkPolygonsIntersect(islandPtsP,numIslandPts,voidPtsP,numVoidPts,&intersectResult)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Result = %2ld",intersectResult) ;
             if( intersectResult == 0 || intersectResult == 3 ) *voidFeatureP = dtmP->nullPnt ;
            }
         }
/*
**    Write Island And Surrounding Voids To A Dat File - For Visual Checking
*/
       if( cdbg )
         {
          if( islandPtsP == NULL ) { if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,islandFeature,&islandPtsP,&numIslandPts) ) goto errexit ; }
          if( voidPtsP   == NULL && *voidFeatureP != dtmP->nullPnt ) { if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,*voidFeatureP,&voidPtsP,&numVoidPts) ) goto errexit ; }
          if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
          bcdtmObject_setMemoryAllocationParametersDataObject(dataP,numIslandPts+numVoidPts,100) ;
          if( islandPtsP != NULL ) { if( bcdtmObject_storeDtmFeatureInDataObject(dataP,DTMFeatureType::Island,DTM_NULL_USER_TAG,nullGuid,islandPtsP,numIslandPts)) goto errexit ; }
          if( voidPtsP   != NULL ) { if( bcdtmObject_storeDtmFeatureInDataObject(dataP,DTMFeatureType::Void,DTM_NULL_USER_TAG,nullGuid,voidPtsP,numVoidPts)) goto errexit ; }
          if( *voidFeatureP != dtmP->nullPnt ) swprintf(outFile,50,L"islandVoid%d.dat",fileNum) ;
          else                                 swprintf(outFile,50,L"islandVoidError%d.dat",fileNum) ;
          if( bcdtmWrite_dataFileFromDataObject(dataP,outFile) ) goto errexit ;
          bcdtmObject_deleteDataObject(&dataP) ;
          ++fileNum ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( islandPtsP != NULL ) free(islandPtsP) ;
 if( voidPtsP   != NULL ) free(voidPtsP) ;
 if( dataP      != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void External To Island Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void External To Island Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *voidFeatureP = dtmP->nullPnt ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmList_testForSurroundingDtmFeatureTypeDtmObject(BC_DTM_OBJ *dtmP,long featurePnt,long featureNextPnt,long featurePriorPnt,DTMFeatureType dtmFeatureType,long *surroundFeatureP)
/*
** This Function Tests For A Surrounding Feature Type At A Point
** On An Existing Feature
*/
{
 int  ret=DTM_SUCCESS ;
 long cnt,listPtr,nextPnt,priorPnt,dtmFeature,clkPnt ;
/*
** Initialise
*/
 *surroundFeatureP = dtmP->nullPnt ;
/*
** Scan Feature List For Point
*/
 listPtr = nodeAddrP(dtmP,featurePnt)->fPtr ;
 while( listPtr != dtmP->nullPtr && *surroundFeatureP == dtmP->nullPnt )
   {
    if( flistAddrP(dtmP,listPtr)->nextPnt == featurePnt && ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType )
      {
       dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,featurePnt,&nextPnt) ) goto errexit ;
       if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,featurePnt,&priorPnt) ) goto errexit ;
       if( priorPnt == featurePriorPnt && nextPnt == featureNextPnt )  *surroundFeatureP = dtmFeature ;
/*
**     Scan Clockwise From Prior Point To Next Point
*/
       else
         {
          cnt = 0 ;
          if( (clkPnt = bcdtmList_nextClkDtmObject(dtmP,featurePnt,priorPnt)) < 0 ) goto errexit ;
          while ( clkPnt != nextPnt )
            {
             if( clkPnt == featurePriorPnt ) ++cnt ;
             if( clkPnt == featureNextPnt  ) ++cnt ;
             if( (clkPnt = bcdtmList_nextClkDtmObject(dtmP,featurePnt,clkPnt)) < 0 ) goto errexit ;
            }
          if     ( cnt == 2 ) *surroundFeatureP = dtmFeature ;
          else if( cnt == 1 && priorPnt == featurePriorPnt ) *surroundFeatureP = dtmFeature ;
          else if( cnt == 1 && nextPnt  == featureNextPnt  ) *surroundFeatureP = dtmFeature ;
         }
      }
    listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
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
BENTLEYDTM_Private int bcdtmList_getVoidFeatureExternalToIslandFeatureDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long *voidFeatureP)
/*
** This Functions Gets The Void Feature External To An Island Feature.
** It Is Only To Be Called Directly By "bcdtmList_getVoidExternalToIslandTinObject"
** The Function Is Programmed To Ignore Implicit Voids Between Islands
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  startPnt,priorPnt,tptrPnt,nextPnt,clkPnt,listPnt,listPtr,firstPnt ;
 long  lastPnt,islandHull,voidHull,numMarked,nextIslandPnt,priorIslandPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island") ;
/*
** Initialise
*/
 numMarked = 0 ;
 startPnt = firstPnt = lastPnt = *voidFeatureP = dtmP->nullPnt ;
/*
** Only Process If Island Feature
*/
 if( islandFeature >= 0 && islandFeature < dtmP->numFeatures )
   {
    if( ftableAddrP(dtmP,islandFeature)->dtmFeatureType  == DTMFeatureType::Island )
      {
/*
**     Check All Tptr Values Are Null
*/
       if( cdbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,cdbg) ;
/*
**     Copy Island Feature To Tptr List
*/
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,islandFeature,&startPnt)) goto errexit ;
/*
**     Scan Around Tptr Polygon And Mark External Points And Create Internal Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tptr Polygon") ;
       priorPnt = startPnt ;
       tptrPnt  = nodeAddrP(dtmP,startPnt)->tPtr ;
       do
         {
          nextPnt = nodeAddrP(dtmP,tptrPnt)->tPtr ;
          if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,tptrPnt,nextPnt)) < 0 ) goto errexit ;
          while ( clkPnt != priorPnt && *voidFeatureP == dtmP->nullPnt )
            {
             if( nodeAddrP(dtmP,clkPnt)->tPtr == dtmP->nullPnt && clkPnt != lastPnt )
               {
                if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,clkPnt,&islandHull)) goto errexit ;
                if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,clkPnt,&voidHull)) goto errexit ;
                if( islandHull == dtmP->nullPnt ) *voidFeatureP = voidHull ;
/*
**              Only Process If Surrounding Void Hull Not Found
*/
                if( *voidFeatureP == dtmP->nullPnt )
                  {
/*
**                 Mark Island Boundary
*/
                   if( islandHull != dtmP->nullPnt )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Marking Island Hull = %6ld",islandHull) ;
                      if( bcdtmList_addDtmFeatureToTptrListDtmObject(dtmP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                     }
/*
**                Mark Point
*/
                   else
                     {
                      if( voidHull == dtmP->nullPnt )
                        {
                         if( firstPnt == dtmP->nullPnt ) firstPnt = clkPnt ;
                         if( lastPnt  != dtmP->nullPnt ) nodeAddrP(dtmP,lastPnt)->tPtr = clkPnt ;
                         lastPnt = clkPnt ;
                         ++numMarked ;
                        }
                     }
                  }
               }
             if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,tptrPnt,clkPnt)) < 0 ) goto errexit ;
            }
          priorPnt = tptrPnt ;
          tptrPnt  = nextPnt ;
         } while ( priorPnt != startPnt && *voidFeatureP == dtmP->nullPnt ) ;
/*
**      Scan Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tptr List") ;
       tptrPnt = firstPnt ;
       while ( tptrPnt != dtmP->nullPnt && *voidFeatureP == dtmP->nullPnt )
         {
          nextPnt = nodeAddrP(dtmP,tptrPnt)->tPtr ;
/*
**        If Point On Island Hull Scan Clockwise From Next Point To Prior Point
*/
          if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,tptrPnt,&islandHull)) goto errexit ;
          if( islandHull != dtmP->nullPnt )
            {
             bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandHull,tptrPnt,&nextIslandPnt) ;
             bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,islandHull,tptrPnt,&priorIslandPnt) ;
             if( ( listPnt = bcdtmList_nextClkDtmObject(dtmP,tptrPnt,nextIslandPnt)) < 0 ) goto errexit ;
             while ( listPnt != priorIslandPnt && *voidFeatureP == dtmP->nullPnt )
               {
                if(nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && listPnt != lastPnt )
                  {
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,listPnt,&islandHull)) goto errexit ;
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,listPnt,&voidHull)) goto errexit ;
                   if( islandHull == dtmP->nullPnt ) *voidFeatureP = voidHull ;
/*
**                 Only Process If Surrounding Void Hull Not Found
*/
                   if( *voidFeatureP == dtmP->nullPnt )
                     {
/*
**                    Mark Island Boundary
*/
                      if( islandHull != dtmP->nullPnt )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Marking Island Hull = %6ld",islandHull) ;
                         if( bcdtmList_addDtmFeatureToTptrListDtmObject(dtmP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                        }
/*
**                    Mark Point
*/
                      else
                        {
                         if( voidHull == dtmP->nullPnt )
                           {
                            nodeAddrP(dtmP,lastPnt)->tPtr = listPnt ;
                            lastPnt = listPnt ;
                            ++numMarked ;
                           }
                        }
                     }
                  }
                if((listPnt = bcdtmList_nextClkDtmObject(dtmP,tptrPnt,listPnt)) < 0 ) goto errexit ;
               }
            }
/*
**       If Point Not On Island Hull Scan Cylic List
*/
          else
            {
             listPtr = nodeAddrP(dtmP,tptrPnt)->cPtr ;
             while( listPtr != dtmP->nullPtr && *voidFeatureP == dtmP->nullPnt )
               {
                listPnt = clistAddrP(dtmP,listPtr)->pntNum ;
                listPtr = clistAddrP(dtmP,listPtr)->nextPtr ;
                if(nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && listPnt != lastPnt )
                  {
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,listPnt,&islandHull)) goto errexit ;
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,listPnt,&voidHull)) goto errexit ;
                   if( islandHull == dtmP->nullPnt ) *voidFeatureP = voidHull ;
                   if( *voidFeatureP == dtmP->nullPnt )
                     {
/*
**                    Mark Island Boundary
*/
                      if( islandHull != dtmP->nullPnt )
                        {
                         if( bcdtmList_addDtmFeatureToTptrListDtmObject(dtmP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                        }
/*
**                    Mark Point
*/
                      else
                        {
                         if( voidHull == dtmP->nullPnt )
                           {
                            nodeAddrP(dtmP,lastPnt)->tPtr = listPnt ;
                            lastPnt = listPnt ;
                            ++numMarked ;
                           }
                        }
                     }
                  }
               }
            }
/*
**        Set To Next Point In Tptr List
*/
          tptrPnt = nextPnt ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cleaning Up") ;
    bcdtmWrite_message(0,0,0,"Num Marked    = %6ld",numMarked) ;
   }
/*
** Unmark Tptr List
*/
 numMarked = 0 ;
 while ( firstPnt != dtmP->nullPnt )
   {
    ++numMarked ;
    nextPnt = nodeAddrP(dtmP,firstPnt)->tPtr ;
    nodeAddrP(dtmP,firstPnt)->tPtr = dtmP->nullPnt ;
    firstPnt = nextPnt ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Num Un Marked = %6ld",numMarked) ;
/*
** Null Out Tptr Island Polygon
*/
 if( startPnt != dtmP->nullPnt ) bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
/*
** Check For Any None Null Tptr Values
*/
 if( cdbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,cdbg) ;
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
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmList_addDtmFeatureToTptrListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *firstPntP,long *lastPntP,long *numAddedP)
/*
** This Function Adds The Hull Of A Dtm Feature To The Tptr List
** It Is A Support Function Only And Should Not Be Called For Any Other Purpose
** Assumes Feature Is Of One Of The Following Types:-
** 1.  DTMFeatureType::Void ,
** 2.  DTMFeatureType::Island
**
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long firstDtmFeaturePnt,nextDtmFeaturePnt,nextPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List") ;
/*
** Scan Dtm Feature And Add Feature Points To Tptr List
*/
 nextDtmFeaturePnt = firstDtmFeaturePnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    if( nodeAddrP(dtmP,nextDtmFeaturePnt)->tPtr == dtmP->nullPnt && nextDtmFeaturePnt != *lastPntP )
      {
       if( *firstPntP == dtmP->nullPnt ) *firstPntP = nextDtmFeaturePnt ;
       if( *lastPntP  != dtmP->nullPnt ) nodeAddrP(dtmP,*lastPntP)->tPtr = nextDtmFeaturePnt ;
       *lastPntP = nextDtmFeaturePnt ;
       ++(*numAddedP) ;
      }
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,nextDtmFeaturePnt,&nextPnt) ) goto errexit ;
    nextDtmFeaturePnt = nextPnt ;
   } while ( nextDtmFeaturePnt != firstDtmFeaturePnt ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List Error") ;
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
BENTLEYDTM_EXPORT int bcdtmList_countTinFeaturesWithErrorsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long *numFeatureErrorsP,
 long *numContourLinesP,
 long *numHardBreaksP,
 long *numVoidsP,
 long *numIslandsP,
 long *numHolesP,
 long *numBreakVoidsP,
 long *numDrapeVoidsP,
 long *numGroupSpotsP,
 long *numPolygonsP,
 long *numHullsP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Dtm Tin Features With Errors For DTM Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Initialise
*/
 *numFeatureErrorsP = 0 ;
 *numContourLinesP  = 0 ;
 *numHardBreaksP    = 0 ;
 *numVoidsP         = 0 ;
 *numIslandsP       = 0 ;
 *numHolesP         = 0 ;
 *numDrapeVoidsP    = 0 ;
 *numBreakVoidsP    = 0 ;
 *numGroupSpotsP    = 0 ;
 *numPolygonsP      = 0 ;
 *numHullsP         = 0 ;
/*
** Scan Dtm Features Array
*/
 for( feature = 0 ; feature < dtmP->numFeatures  ; ++feature )
   {
    dtmFeatureP =  ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )
      {
       ++*numFeatureErrorsP ;
       if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline    ) ++*numHardBreaksP   ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine  ) ++*numContourLinesP ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void          ) ++*numVoidsP        ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island        ) ++*numIslandsP      ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull          ) ++*numHullsP        ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole          ) ++*numHolesP        ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid    ) ++*numDrapeVoidsP   ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid    ) ++*numBreakVoidsP   ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots    ) ++*numGroupSpotsP   ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon       ) ++*numPolygonsP     ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Dtm Tin Features With Errors For DTM Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Dtm Tin Features With Errors For DTM Object %p Error",dtmP) ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrListToHptrListDtmObject(BC_DTM_OBJ *dtmP, long startPnt)
/*
** This Function Nulls Out The Fptr List
*/
{
 long scanPnt ;
/*
** Scan Tptr List And Copy To Fptr List
*/
 scanPnt = startPnt ;
 if( nodeAddrP(dtmP,scanPnt)->tPtr == dtmP->nullPnt ) return(DTM_SUCCESS) ;
 do
   {
    nodeAddrP(dtmP,scanPnt)->hPtr = nodeAddrP(dtmP,scanPnt)->tPtr ;
    scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
   } while ( nodeAddrP(dtmP,scanPnt)->tPtr != dtmP->nullPnt && scanPnt != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_copyHptrListToTptrListDtmObject(BC_DTM_OBJ *dtmP, long startPnt)
/*
** This Function Nulls Out The Fptr List
*/
{
 long scanPnt ;
/*
** Scan Tptr List And Copy To Fptr List
*/
 scanPnt = startPnt ;
 if( nodeAddrP(dtmP,scanPnt)->hPtr == dtmP->nullPnt ) return(DTM_SUCCESS) ;
 do
   {
    nodeAddrP(dtmP,scanPnt)->tPtr = nodeAddrP(dtmP,scanPnt)->hPtr ;
    scanPnt = nodeAddrP(dtmP,scanPnt)->hPtr ;
   } while ( nodeAddrP(dtmP,scanPnt)->hPtr != dtmP->nullPnt && scanPnt != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrListToSptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Copies The Tptr List To The Sptr List
*/
{
 long scanPnt ;
/*
** Scan Tptr List And Copy To Sptr List
*/
 scanPnt = startPnt ;
 if( nodeAddrP(dtmP,scanPnt)->tPtr == dtmP->nullPnt ) return(DTM_SUCCESS) ;
 do
   {
    nodeAddrP(dtmP,scanPnt)->sPtr = nodeAddrP(dtmP,scanPnt)->tPtr ;
    scanPnt = nodeAddrP(dtmP,scanPnt)->tPtr ;
   } while ( nodeAddrP(dtmP,scanPnt)->tPtr != dtmP->nullPnt && scanPnt != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_copySptrListToTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Copies The Sptr List To The Tptr List
*/
{
 long scanPnt ;
/*
** Scan Sptr List And Copy To Tptr List
*/
 scanPnt = startPnt ;
 if( nodeAddrP(dtmP,scanPnt)->sPtr == dtmP->nullPnt ) return(DTM_SUCCESS) ;
 do
   {
    nodeAddrP(dtmP,scanPnt)->tPtr = nodeAddrP(dtmP,scanPnt)->sPtr ;
    scanPnt = nodeAddrP(dtmP,scanPnt)->sPtr ;
   } while ( nodeAddrP(dtmP,scanPnt)->sPtr != dtmP->nullPnt && scanPnt != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrListToPointArrayDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       startPnt,
 DPoint3d        **tptrPtsPP,
 long       *numTptrPtsP
)
/*
** This Function And Copies A Tptr List To A DPoint3d Array
*/
{
 int  ret=DTM_SUCCESS ;
 long sp ;
 DPoint3d  *p3dP ;
/*
** Initialise
*/
 *numTptrPtsP = 0 ;
 if( *tptrPtsPP != NULL ) { free(*tptrPtsPP) ; *tptrPtsPP = NULL ; }
/*
** Validate
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Start Point Has A Null Tptr Value") ;
    goto errexit ;
   }
/*
** Scan Tptr List And Count Number Of Points
*/
 sp = startPnt ;
 do
   {
    ++*numTptrPtsP ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
 if( sp == startPnt ) ++*numTptrPtsP ;
/*
** Allocate memory For DPoint3d Array
*/
 *tptrPtsPP = ( DPoint3d * ) malloc( *numTptrPtsP * sizeof(DPoint3d)) ;
 if( *tptrPtsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Tptr List And Copy Points To DPoint3d Array
*/
 p3dP = *tptrPtsPP ;
 sp = startPnt ;
 do
   {
    p3dP->x = pointAddrP(dtmP,sp)->x  ;
    p3dP->y = pointAddrP(dtmP,sp)->y  ;
    p3dP->z = pointAddrP(dtmP,sp)->z  ;
    ++p3dP ;
    sp = nodeAddrP(dtmP,sp)->tPtr   ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
if( sp == startPnt )
  {
   p3dP->x = pointAddrP(dtmP,sp)->x  ;
   p3dP->y = pointAddrP(dtmP,sp)->y  ;
   p3dP->z = pointAddrP(dtmP,sp)->z ;
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
BENTLEYDTM_Public int bcdtmList_copySptrListToPointArrayDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       startPnt,
 DPoint3d        **tptrPtsPP,
 long       *numTptrPtsP
)
/*
** This Function And Copies A Sptr List To A DPoint3d Array
*/
{
 int  ret=DTM_SUCCESS ;
 long sp ;
 DPoint3d  *p3dP ;
/*
** Initialise
*/
 *numTptrPtsP = 0 ;
 if( *tptrPtsPP != NULL ) { free(*tptrPtsPP) ; *tptrPtsPP = NULL ; }
/*
** Validate
*/
 if( nodeAddrP(dtmP,startPnt)->sPtr == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Start Point Has A Null Tptr Value") ;
    goto errexit ;
   }
/*
** Scan Tptr List And Count Number Of Points
*/
 sp = startPnt ;
 do
   {
    ++*numTptrPtsP ;
    sp = nodeAddrP(dtmP,sp)->sPtr ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->sPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
 if( sp == startPnt ) ++*numTptrPtsP ;
/*
** Allocate memory For DPoint3d Array
*/
 *tptrPtsPP = ( DPoint3d * ) malloc( *numTptrPtsP * sizeof(DPoint3d)) ;
 if( *tptrPtsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Tptr List And Copy Points To DPoint3d Array
*/
 p3dP = *tptrPtsPP ;
 sp = startPnt ;
 do
   {
    p3dP->x = pointAddrP(dtmP,sp)->x  ;
    p3dP->y = pointAddrP(dtmP,sp)->y  ;
    p3dP->z = pointAddrP(dtmP,sp)->z  ;
    ++p3dP ;
    sp = nodeAddrP(dtmP,sp)->sPtr   ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->sPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
if( sp == startPnt )
  {
   p3dP->x = pointAddrP(dtmP,sp)->x  ;
   p3dP->y = pointAddrP(dtmP,sp)->y  ;
   p3dP->z = pointAddrP(dtmP,sp)->z ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrValuesToPointListDtmObject(BC_DTM_OBJ *dtmP,long **pntListPP)
/*
** This Function Copies The Temporary Pointer List For A Point List
*/
{
 int   ret=DTM_SUCCESS ;
 long  node,*longP ;
/*
** Initialise
*/
 if( *pntListPP != NULL ) free(*pntListPP) ;
/*
** Allocate Memory To Point List
*/
 *pntListPP = (long * ) malloc( dtmP->numPoints * sizeof(long)) ;
 if( *pntListPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Tptr List
*/
 for( node = 0 , longP = *pntListPP ; node < dtmP->numPoints ; ++node , ++longP ) *longP = nodeAddrP(dtmP,node)->tPtr ;
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
BENTLEYDTM_Public int bcdtmList_copyPointListToTptrValuesDtmObject(BC_DTM_OBJ *dtmP,long *pntListP)
/*
** This Function Copies The Temporary Pointer List For A Point List
*/
{
 long  node,*longP ;
/*
** Initialise
*/
 if( pntListP == NULL ) return(DTM_SUCCESS) ;
/*
** Copy Tptr List
*/
 for( node = 0 , longP = pntListP ; node < dtmP->numPoints ; ++node , ++longP ) nodeAddrP(dtmP,node)->tPtr = *longP ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long **pntListPP,long *numPntListP)
/*
** This Function Copies The Temporary Pointer List For A Point List
*/
{
 int   ret=DTM_SUCCESS ;
 long  scanPoint,numPoints,*longP,closeFlag=0  ;
/*
** Initialise
*/
 if( *pntListPP != NULL ) free(*pntListPP) ;
 *pntListPP = NULL ;
 *numPntListP = 0 ;
/*
** Count Number Of Tptr List Points
*/
 if( nodeAddrP(dtmP,startPoint)->tPtr != dtmP->nullPnt )
   {
    numPoints = 0 ;
    scanPoint = startPoint ;
    do
      {
       ++numPoints ;
       scanPoint = nodeAddrP(dtmP,scanPoint)->tPtr ;
      } while( scanPoint != dtmP->nullPnt &&  scanPoint != startPoint );

      if (scanPoint == startPoint)
          {
          closeFlag = 1;
          numPoints++;
          }
/*
**   Allocate Memory To Point List
*/
    *numPntListP = numPoints ;
    *pntListPP = (long * ) malloc( *numPntListP * sizeof(long)) ;
    if( *pntListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**   Copy Tptr List
*/
    longP = *pntListPP ;
    scanPoint = startPoint ;
    do
      {
       *longP = scanPoint ;
       ++longP ;
       scanPoint = nodeAddrP(dtmP,scanPoint)->tPtr ;
      } while( scanPoint != dtmP->nullPnt &&  scanPoint != startPoint );
    if( closeFlag ) *longP = startPoint ;
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
BENTLEYDTM_Public int bcdtmList_copySptrListToPointListDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long **pntListPP,long *numPntListP)
/*
** This Function Copies The Temporary Pointer List For A Point List
*/
{
 int   ret=DTM_SUCCESS ;
 long  scanPoint,numPoints,*longP,closeFlag=0  ;
/*
** Initialise
*/
 if( *pntListPP != NULL ) free(*pntListPP) ;
 *pntListPP = NULL ;
 *numPntListP = 0 ;
/*
** Count Number Of Tptr List Points
*/
 if( nodeAddrP(dtmP,startPoint)->sPtr != dtmP->nullPnt )
   {
    numPoints = 1 ;
    scanPoint = startPoint ;
    do
      {
       ++numPoints ;
       scanPoint = nodeAddrP(dtmP,scanPoint)->sPtr ;
      } while( scanPoint != dtmP->nullPnt &&  scanPoint != startPoint );

    if( scanPoint == startPoint )  closeFlag = 1 ;
/*
**   Allocate Memory To Point List
*/
    *numPntListP = numPoints ;
    *pntListPP = (long * ) malloc( *numPntListP * sizeof(long)) ;
    if( *pntListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**   Copy Tptr List
*/
    longP = *pntListPP ;
    scanPoint = startPoint ;
    do
      {
       *longP = scanPoint ;
       ++longP ;
       scanPoint = nodeAddrP(dtmP,scanPoint)->sPtr ;
      } while( scanPoint != dtmP->nullPnt &&  scanPoint != startPoint );
    if( closeFlag ) *longP = startPoint ;
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
BENTLEYDTM_Public int bcdtmList_copyPointListToTptrListDtmObject(BC_DTM_OBJ *dtmP,long *pntListP, long numPntList, long *startPointP)
/*
** This Function Copies The Temporary Pointer List For A Point List
*/
{
 long  *longP ;
/*
** Initialise
*/
 *startPointP = dtmP->nullPnt ;
/*
** Copy Point List To Tptr List
*/
  if( pntListP != NULL )
   {
    *startPointP = *pntListP ;
    for( longP = pntListP ; longP < pntListP + numPntList - 1 ; ++longP )
      {
       nodeAddrP(dtmP,*longP)->tPtr = *(longP+1) ;
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
BENTLEYDTM_EXPORT int bcdtmList_setConvexHullDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Sets The Convex Hull For An dtmP Object
*/
{
 int  ret=DTM_SUCCESS ;
 long nextHullPoint=0,np,hullPoint=0,clPtr,node,isw=1,dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_NODE  *nodeP ;
/*
**  Null Out Hull Pointers
*/
 bcdtmList_nullHptrValuesDtmObject(dtmP) ;
/*
** Scan Point Array To Detect Unconnected Points
*/
 for( node = 0 ; node < dtmP->numPoints && isw ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
      {
       hullPoint  = node ;
       if(( nextHullPoint = bcdtmList_nextAntDtmObject(dtmP,hullPoint,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr && isw )
         {
          np    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( ! bcdtmList_testLineDtmObject(dtmP,nextHullPoint,np) || bcdtmMath_pointSideOfDtmObject(dtmP,nextHullPoint,np,hullPoint) > 0 ) isw = 0 ;
          else                                                                                                                              nextHullPoint = np ;
         }
      }
   }
/*
** Check If Hull Point Found
*/
 if( isw )
   {
    bcdtmWrite_message(2,0,0,"No Hull Point Found") ;
    goto errexit ;
   }
/*
** Set Convex Hull Pointers
*/
 dtmP->hullPoint      = hullPoint ;
 dtmP->nextHullPoint  = nextHullPoint ;
 bcdtmTin_setConvexHullDtmObject(dtmP,hullPoint,nextHullPoint) ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )
      {
       dtmFeatureP->dtmFeaturePts.firstPoint = hullPoint ;
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
BENTLEYDTM_EXPORT int bcdtmList_cleanDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Cleans a Tin Object
** Removes Deleted Features, Compacts The Tin Object
** And Resets Some Tin Variables
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cleaning Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->memPoints = %8ld dtmP->numSortedPoints = %8ld",dtmP->numPoints,dtmP->memPoints,dtmP->numSortedPoints) ;
   }
/*
** Validate Tin Prior To Cleaning
*/
 if( cdbg )
   {
    bcdtmUtility_writeStatisticsDtmObject(dtmP) ;
    bcdtmWrite_message(0,0,0,"Checking Tin Prior To Cleaning") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Tin Corrupted Prior To Cleaning") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin OK Prior To Cleaning") ;
   }
/*
** Null Out tPtr sPtr and hPtr Node Values
*/
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 bcdtmList_nullHptrValuesDtmObject(dtmP) ;
/*
** Compact Tin Structure
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Compacting Circular List") ;
 if( bcdtmTin_compactCircularListDtmObject(dtmP)  ) goto errexit ;
 if( dbg )bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
 if( dbg )bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP) ) goto errexit ;
 if( bcdtmTin_compactPointAndNodeTablesDtmObject(dtmP)) goto errexit ;
/*
** Validate Tin After Compacting
*/
 if( cdbg )
   {
    if( bcdtmTin_setConvexHullDtmObject(dtmP,dtmP->hullPoint,dtmP->nextHullPoint)) goto errexit ;
    bcdtmWrite_message(0,0,0,"**** Checking Tin After Compacting") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Tin After Compacting Corrupted") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin After Compacting OK") ;
   }
/*
** Resize Tin Memory
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Resizing Tin Memory") ;
 if( bcdtmObject_resizeMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Resort Tin Data
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Sorting Tin Structure") ;
 if( bcdtmTin_resortTinStructureDtmObject(dtmP) ) goto errexit ;
/*
** Reset Convex Hull
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Resetting Tin Hull") ;
 if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
/*
** Mark Void Points
*/
 if( bcdtmTin_markInternalVoidPointsDtmObject(dtmP)) goto errexit ;
/*
** Null Out tPtr And sPtr Values
*/
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Count Number Of Triangles and Lines
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Counting Triangles And Lines") ;
 bcdtmList_countTrianglesAndLinesDtmObject(dtmP,&((dtmP)->numTriangles),&((dtmP)->numLines)) ;
/*
** Recalculate Bounding Cube
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Bounding Cube Tin Object") ;
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
/*
** Calculate Machine Point To Point Tolerance For DTMFeatureState::Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Machine Precision For Tin") ;
 bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP) ;
/*
** Check Cleaned Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Cleaned Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Cleaned Tin Corrupted") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Cleaned Tin OK") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Tin Object Error") ;
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
BENTLEYDTM_Public int bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(BC_DTM_OBJ *dtmP,long point,long *numFeaturesP)
/*
** This Function Counts The Number Of Dtm Features For A Point
*/
{
 long flPtr ;
/*
** Count Features
*/
 *numFeaturesP = 0 ;
 flPtr = nodeAddrP(dtmP,point)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    ++*numFeaturesP ;
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_countNumberOfDtmFeaturesForLineDtmObject(BC_DTM_OBJ *dtmP, long point1, long point2, long *numFeaturesP)
/*
** This Function Counts The Number Of Dtm Features For A Point
*/
{
 long flPtr ;
/*
** Count Features
*/
 *numFeaturesP = 0 ;
 flPtr = nodeAddrP(dtmP, point1)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
   if (flistAddrP(dtmP,flPtr)->nextPnt == point2)
        ++*numFeaturesP ;
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_countNumberOfHardBreakFeaturesForPointDtmObject(BC_DTM_OBJ *dtmP,long point,long *numFeaturesP)
/*
** This Function Counts The Number Of Dtm Features For A Point
*/
{
 long flPtr ;
/*
** Count Features
*/
 *numFeaturesP = 0 ;
 flPtr = nodeAddrP(dtmP,point)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) ++*numFeaturesP ;
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_cleanDtmFeatureListsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Removes The Last Point Connection In A Closing Dtm Feature List That Has Only Two Points
** This Is Necessary For Old Tins ( Pre 2000 ) Only, Where The Triangulation Engine Allowed This To Occurr
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,firstPnt,lastPnt,numPts,closeFlag ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cleaning Dtm Feature Lists") ;
    bcdtmWrite_message(0,0,0,"dtmP   =  %p") ;
   }
/*
** Scan dtmFeature Lists
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine ))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld",dtmFeature) ;
       if( bcdtmList_getFirstAndLastPointForDtmFeatureDtmObject(dtmP,dtmFeature,&firstPnt,&lastPnt) ) goto errexit  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %8ld lastPnt = %8ld",firstPnt,lastPnt) ;
/*
**     Test For Closure
*/
       if( firstPnt == lastPnt )
         {
          if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numPts,&closeFlag)) goto errexit  ;
          if( dbg ) bcdtmWrite_message(0,0,0,"numPts = %8ld closeFlag = %8ld",numPts,closeFlag) ;
          if( numPts == 3 && closeFlag )
            {
             if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline || ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::ContourLine )
               {
                if( bcdtmList_removeLastPointFromDtmFeatureListDtmObject(dtmP,dtmFeature)) goto errexit  ;
               }
             else  bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature) ;
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
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Cleaning Dtm Feature Lists Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Cleaning Dtm Feature Lists Error") ;
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
BENTLEYDTM_Public int bcdtmList_countNumberOfDtmFeaturePointsDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *numPtsP,long *closeFlagP)
/*
** This Function Counts The Number Of Feature Points
*/
{
 int   ret=DTM_SUCCESS ;
 long  nextPnt=0,firstPnt,fListPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *numPtsP    = 0 ;
 *closeFlagP = 0 ;
/*
** Scan Dtm dtmFeature
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures ) goto errexit ;
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       fListPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
       *numPtsP = 1 ;
/*
**     Scan Dtm dtmFeature Points
*/
       while ( fListPtr != dtmP->nullPtr )
         {
          while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
           if( fListPtr != dtmP->nullPtr )
             {
              nextPnt  = flistAddrP(dtmP,fListPtr)->nextPnt ;
              if( nextPnt != dtmP->nullPnt )
                {
                 ++(*numPtsP) ;
                fListPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
               }
             if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) fListPtr = dtmP->nullPtr ;
            }
         }
/*
**     If Dtm Feature Closes Decrement Number Of Points
*/
       if( nextPnt == firstPnt && *numPtsP > 1 ) *closeFlagP = 1 ;
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
BENTLEYDTM_Public int bcdtmList_getFirstAndLastPointForDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long *firstPntP,long *lastPntP)
/*
** This Function Gets The First And Last Point For A Dtm dtmFeature
**
** On return if :-
** firstPntP == DTM_NULL_PNT and lastPntP == DTM_NULL_PNT  - Dtm dtmFeature Does Not Exist
** firstPntP == lastPntP                         - Dtm dtmFeature Closes
** firstPntP != DTM_NULL_PNT and lastPntP == DTM_NULL_PNT  - Dtm dtmFeature Has Only One Point
**
*/
{
 int   ret=DTM_SUCCESS ;
 long  nextPnt,flistPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *firstPntP = dtmP->nullPnt ;
 *lastPntP  = dtmP->nullPnt ;
/*
** Scan Dtm dtmFeature
*/
 if( dtmFeature >= 0 && dtmFeature < dtmP->numFeatures )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       *firstPntP = dtmFeatureP->dtmFeaturePts.firstPoint ;
       flistPtr   = nodeAddrP(dtmP,*firstPntP)->fPtr ;
/*
**     Scan Dtm Feature Points
*/
       while ( flistPtr != dtmP->nullPtr )
         {
          nextPnt = dtmP->nullPnt ;
          while( flistPtr != dtmP->nullPtr  && flistAddrP(dtmP,flistPtr)->dtmFeature != dtmFeature ) flistPtr = flistAddrP(dtmP,flistPtr)->nextPtr ;
          if( flistPtr != dtmP->nullPtr )
            {
             nextPnt = flistAddrP(dtmP,flistPtr)->nextPnt ;
             if( nextPnt != dtmP->nullPnt )
               {
                *lastPntP = nextPnt ;
                flistPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
               }
            }
          if( nextPnt == dtmP->nullPnt || nextPnt == *firstPntP ) flistPtr = dtmP->nullPtr ;
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
BENTLEYDTM_Private int bcdtmList_removeLastPointFromDtmFeatureListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Gets The First And Last Point For A Dtm Feature
*/
{
 int ret=DTM_SUCCESS ;
 long  sp,lp,pp,spnt,fListPtr,lastFlistPtr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature > dtmP->numFeatures ) goto errexit ;
/*
** Initialise
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin || dtmFeatureP->dtmFeaturePts.firstPoint == dtmP->nullPnt ) goto errexit ;
 sp = spnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
 fListPtr = nodeAddrP(dtmP,sp)->fPtr ;
/*
** Scan Dtm Feature List Pointers To Last Point
*/
 lp = dtmP->nullPnt ;
 pp = dtmP->nullPnt ;
 while ( fListPtr != dtmP->nullPtr )
   {
    while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
    if( fListPtr != dtmP->nullPtr )
      {
       pp  = lp ;
       lp  = sp ;
       sp  = flistAddrP(dtmP,fListPtr)->nextPnt ;
       if( sp != dtmP->nullPnt ) fListPtr = nodeAddrP(dtmP,sp)->fPtr ;
      }
    if( sp == dtmP->nullPnt || sp == spnt )  fListPtr = dtmP->nullPtr ;
   }
/*
** Remove Last Point
*/
 lastFlistPtr = dtmP->nullPtr ;
 fListPtr  = nodeAddrP(dtmP,lp)->fPtr ;
 while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) { lastFlistPtr = fListPtr ; fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ; }
 if( fListPtr != dtmP->nullPtr )
   {
/*
**  Update dtmFeature List For Point
*/
    if( fListPtr == nodeAddrP(dtmP,lp)->fPtr ) nodeAddrP(dtmP,lp)->fPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
    else                                       flistAddrP(dtmP,lastFlistPtr)->nextPtr = flistAddrP(dtmP,fListPtr)->nextPtr ;
/*
**  Null Out dtmFeature List Entry
*/
    flistAddrP(dtmP,fListPtr)->nextPnt = dtmP->nullPnt ;
    flistAddrP(dtmP,fListPtr)->nextPtr = dtmP->nullPtr ;
/*
**  Update Delete Dtm Feature List Pointer
*/
    if( dtmP->fListDelPtr == dtmP->nullPtr )
      {
       dtmP->fListDelPtr = fListPtr ;
      }
    else
      {
       flistAddrP(dtmP,fListPtr)->nextPtr = dtmP->fListDelPtr ;
       dtmP->fListDelPtr = fListPtr ;
      }
   }
/*
** Null Out Next Point For Prior Point
*/
 if( pp != dtmP->nullPnt )
   {
    fListPtr  = nodeAddrP(dtmP,pp)->fPtr ;
    while ( fListPtr != dtmP->nullPtr  && flistAddrP(dtmP,fListPtr)->dtmFeature != dtmFeature ) { lastFlistPtr = fListPtr ; fListPtr = flistAddrP(dtmP,fListPtr)->nextPtr ; }
    if( fListPtr != dtmP->nullPtr ) flistAddrP(dtmP,fListPtr)->nextPnt = dtmP->nullPnt ;
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
BENTLEYDTM_Public int bcdtmInsert_scanAndInsertBrokenTptrLinksDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long drapeOption,long insertOption)
/*
** This Function Scans And Inserts Broken Links In Tptr Array
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,spnt,npnt ;
/*
** Scan Tptr List And Look For Loop
** Note This Is A Kludge To Fix Lines That Loop Back
**
*/
 spnt = startPnt ;
 do
   {
    npnt = nodeAddrP(dtmP,spnt)->tPtr ;
    if( npnt != dtmP->nullPnt )
      {
       sp = nodeAddrP(dtmP,npnt)->tPtr ;
       if( sp != dtmP->nullPnt )
         {
          if( sp == spnt )
            {
             nodeAddrP(dtmP,npnt)->tPtr = dtmP->nullPnt ;
             npnt = dtmP->nullPnt ;
            }
         }
      }
    spnt = npnt ;
   } while ( spnt != dtmP->nullPnt && spnt != startPnt ) ;

/*
** Scan Tptr List And Insert
*/
 spnt = startPnt ;
 do
   {
    npnt = nodeAddrP(dtmP,spnt)->tPtr ;
    if( npnt != dtmP->nullPnt )
      {
       if( ! bcdtmList_testLineDtmObject(dtmP,spnt,npnt) )
         {
          sp = nodeAddrP(dtmP,npnt)->tPtr ;
          nodeAddrP(dtmP,spnt)->tPtr = dtmP->nullPnt ;
          nodeAddrP(dtmP,npnt)->tPtr = dtmP->nullPnt ;
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,spnt,npnt,drapeOption,insertOption)) goto errexit ;
          nodeAddrP(dtmP,npnt)->tPtr = sp ;
         }
      }
    spnt = npnt ;
   } while ( spnt != dtmP->nullPnt && spnt != startPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_testForHullPointDtmObject(BC_DTM_OBJ *dtmP,long point,long *hullFlagP)
/*
** This Function Tests If a Point is On A Hull Boundary
** The Feature Number For The Point
**
*/
{
 long clc ;
/*
** Initialise
*/
 *hullFlagP = 0 ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Test For Point On Tin Hull
*/
    if( nodeAddrP(dtmP,point)->hPtr != dtmP->nullPnt ) *hullFlagP = 1 ;
/*
**  Scan Feature List Points For Point
*/
    else
      {
       clc = nodeAddrP(dtmP,point)->fPtr ;
       while( clc != dtmP->nullPtr && ! *hullFlagP )
         {
          if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
              ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
              ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  *hullFlagP = 1 ;
          clc = flistAddrP(dtmP,clc)->nextPtr ;
         }
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
BENTLEYDTM_Public int bcdtmList_insertTptrPolygonAroundPointDtmObject(BC_DTM_OBJ *dtmP,long point,long *startPntP)
/*
** This Function Inserts A Tptr Polygon Around A point
*/
{
 int   ret=DTM_SUCCESS ;
 long  sp,lp,np,clc,extPoint ;
/*
** Initialise
*/
 extPoint = FALSE ;
 *startPntP = dtmP->nullPnt ;
 if( point < 0 || point >= dtmP->numPoints ) goto errexit ;
 if( nodeAddrP(dtmP,point)->cPtr == dtmP->nullPtr ) goto errexit ;
 if( nodeAddrP(dtmP,point)->hPtr != dtmP->nullPnt ) extPoint = TRUE ;
/*
** Set Tptr Polygon Around Internal Point
*/
 if( extPoint == FALSE )
   {
    clc = nodeAddrP(dtmP,point)->cPtr ;
    sp = *startPntP = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    while( clc != dtmP->nullPtr )
      {
       lp  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       nodeAddrP(dtmP,lp)->tPtr = sp   ;
       sp  = lp ;
      }
    nodeAddrP (dtmP, *startPntP)->tPtr = sp;
   }
/*
** Set Tptr Polygon Around External Point
*/
 if( extPoint == TRUE )
   {
    sp = lp = nodeAddrP(dtmP,point)->hPtr ;
    nodeAddrP(dtmP,point)->tPtr = lp ;
    if( ( np = bcdtmList_nextAntDtmObject(dtmP,point,lp)) < 0 ) goto errexit ;
    while( np != sp )
      {
       nodeAddrP(dtmP,lp)->tPtr = np ;
       lp = np ;
       if( ( np = bcdtmList_nextAntDtmObject(dtmP,point,lp)) < 0 ) goto errexit ;
      }
    nodeAddrP(dtmP,lp)->tPtr = point ;
    *startPntP = point ;
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
BENTLEYDTM_Public int bcdtmList_checkForNoneNullTptrValuesDtmObject(BC_DTM_OBJ *dtmP,long *noneNullSptrP)
/*
** This Function Is Mostly Used For Development Purposes
*/
{
 long node ;
 DTM_TIN_NODE  *nP ;
/*
** Scan All Tin Points
*/
 *noneNullSptrP = 0 ;
 for( node = 0 ; node < dtmP->numPoints  && ! *noneNullSptrP  ; ++node  )
   {
    nP = nodeAddrP(dtmP,node) ;
    if( nP->cPtr != dtmP->nullPtr && nP->tPtr != dtmP->nullPnt ) *noneNullSptrP = 1 ;
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
BENTLEYDTM_Public int bcdtmList_checkForNoneNullSptrValuesDtmObject(BC_DTM_OBJ *dtmP,long *noneNullSptrP)
/*
** This Function Is Mostly Used For Development Purposes
*/
{
 long node ;
 DTM_TIN_NODE  *nP ;
/*
** Scan All Tin Points
*/
 *noneNullSptrP = 0 ;
 for( node = 0 ; node < dtmP->numPoints  && ! *noneNullSptrP  ; ++node  )
   {
    nP = nodeAddrP(dtmP,node) ;
    if( nP->cPtr != dtmP->nullPtr && nP->sPtr != dtmP->nullPnt ) *noneNullSptrP = 1 ;
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
BENTLEYDTM_Public int bcdtmList_copyTptrListFromDtmObjectToDtmObject
(
 BC_DTM_OBJ *dtm1P,
 BC_DTM_OBJ *dtm2P,
 long startPnt,
 DTMFeatureType dtmFeatureType,
 DTMUserTag userTag,
 DTMFeatureId userFeatureId
)
/*
** This Function Copies a Tptr String From A Dtm Object To A Dtm Object
*/
{
 int   ret=DTM_SUCCESS ;
 long  sp,numPts ;
 DPoint3d   *p3dP,*featurePtsP=NULL ;
 DTM_TIN_POINT *pointP ;
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit ;
/*
** Test For Points
*/
 if( startPnt < 0 || startPnt > dtm1P->numPoints ) goto errexit ;
/*
** Count Number Of Points In Tptr List
*/
 numPts = 0 ;
 sp = startPnt ;
 do
   {
    ++numPts ;
    sp = nodeAddrP(dtm1P,sp)->tPtr ;
   } while( sp != dtm1P->nullPnt && sp != startPnt ) ;
/*
** Check For Feature Closure
*/
 if( sp == startPnt ) ++numPts ;
/*
** Allocate Memory For Feature Points
*/
 featurePtsP = ( DPoint3d * ) malloc ( numPts * sizeof(DPoint3d)) ;
 if( featurePtsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Tptr String
*/
 sp = startPnt ;
 p3dP = featurePtsP ;
 do
   {
    pointP  = pointAddrP(dtm1P,sp) ;
    p3dP->x = pointP->x ;
    p3dP->y = pointP->y ;
    p3dP->z = pointP->z ;
    ++p3dP ;
    sp = nodeAddrP(dtm1P,sp)->tPtr ;
   } while ( sp != startPnt && sp != dtm1P->nullPnt ) ;
/*
** Check For Feature Closure
*/
 if( sp == startPnt )
   {
    pointP  = pointAddrP(dtm1P,sp) ;
    p3dP->x = pointP->x ;
    p3dP->y = pointP->y ;
    p3dP->z = pointP->z ;
    ++p3dP ;
   }
/*
** Store Feature Points In Dtm Object
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,dtmFeatureType,userTag,1,&userFeatureId,featurePtsP,numPts)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( featurePtsP != NULL ) free(featurePtsP) ;
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
BENTLEYDTM_Public int bcdtmList_copyCircularListPointsToPointArrayFromDtmObjectToDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMDirection direction,       /* Direction <DTMDirection::Clockwise,DTMDirection::AntiClockwise>   */
 long firstPnt,
 long point,
 long lastPnt,
 DPoint3d  **cirPtsPP,
 long *numCirPtsP
)
/*
** This Function Copies The Circular List Points About A Point To A Point Array
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  fPnt  ;
 DPoint3d   *p3dP     ;
 DTM_TIN_POINT *pointP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copying Circular List Points To Point Array") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"direction   = %8ld",direction) ;
    bcdtmWrite_message(0,0,0,"firstPnt    = %8ld",firstPnt) ;
    bcdtmWrite_message(0,0,0,"point       = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"lastPnt     = %8ld",lastPnt) ;
    bcdtmWrite_message(0,0,0,"*cirPtsPP   = %8p",*cirPtsPP) ;
    bcdtmWrite_message(0,0,0,"*numCirPtsP = %8ld",*numCirPtsP) ;
   }
/*
** Initialise
*/
 *numCirPtsP = 0 ;
 if( *cirPtsPP != NULL ) { free(*cirPtsPP) ; *cirPtsPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Check Point Range
*/
    if( point < 0 || point >= dtmP->numPoints )
      {
       bcdtmWrite_message(2,0,0,"Point Range Error") ;
       goto errexit ;
      }
/*
**  Check Point Has A Circular List
*/
    if( nodeAddrP(dtmP,point)->cPtr == dtmP->nullPtr )
      {
       bcdtmWrite_message(2,0,0,"Point Has No Circular List") ;
       goto errexit ;
      }
/*
**  Check End Points
*/
    if( firstPnt == dtmP->nullPnt || lastPnt == dtmP->nullPnt )
      {
       firstPnt = lastPnt = clistAddrP(dtmP,nodeAddrP(dtmP,point)->cPtr)->pntNum ;
      }
    else
      {
       if( ! bcdtmList_testLineDtmObject(dtmP,point,firstPnt) || ! bcdtmList_testLineDtmObject(dtmP,point,lastPnt) )
         {
          bcdtmWrite_message(2,0,0,"Lines Not Connected") ;
          goto errexit ;
         }
      }
/*
**  Count Number Of Circular List Points
*/
    fPnt = firstPnt ;
    *numCirPtsP = 1 ;
    do
      {
    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Point") ;
       if( direction == DTMDirection::Clockwise ){ if( ( fPnt = bcdtmList_nextClkDtmObject(dtmP,point,fPnt)) < 0 ) goto errexit ; }
       else                            { if( ( fPnt = bcdtmList_nextAntDtmObject(dtmP,point,fPnt)) < 0 ) goto errexit ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"Incrementing Number Of Points") ;
       ++*numCirPtsP ;
      } while( fPnt != lastPnt ) ;
/*
**  Allocate memory
*/
    *cirPtsPP = ( DPoint3d * ) malloc( *numCirPtsP * sizeof(DPoint3d)) ;
    if( *cirPtsPP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       *numCirPtsP = 0 ;
       goto errexit ;
      }
/*
**  Store Circular List Points In Point Array
*/
    p3dP = *cirPtsPP ;
    fPnt = firstPnt ;
    pointP = pointAddrP(dtmP,fPnt) ;
    p3dP->x = pointP->x ;
    p3dP->y = pointP->y ;
    p3dP->z = pointP->z ;
    ++p3dP ;
    do
      {
       if( direction == DTMDirection::Clockwise ){ if( ( fPnt = bcdtmList_nextClkDtmObject(dtmP,point,fPnt)) < 0 ) goto errexit ; }
       else                            { if( ( fPnt = bcdtmList_nextAntDtmObject(dtmP,point,fPnt)) < 0 ) goto errexit ; }
       pointP = pointAddrP(dtmP,fPnt) ;
       p3dP->x = pointP->x ;
       p3dP->y = pointP->y ;
       p3dP->z = pointP->z ;
       ++p3dP ;
      } while( fPnt != lastPnt ) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Circular List Points To Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Circular List Points To Point Array Error") ;
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
BENTLEYDTM_Public int bcdtmList_getDtmFeatureTypeOccurrencesForPointDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long Point,DTM_TIN_POINT_FEATURES **pointFeatures,long *numPointFeatures)
/*
** This Function Gets The Dtm Features For A Point
** Rob Cormack June 2003
*/
{
 int  ret=DTM_SUCCESS ;
 long np,pp,lp,clc,clm,cln,feature,memFeatureTable=0,memInc=10 ;
/*
** Initialise
*/
 *numPointFeatures = 0 ;
 if( *pointFeatures != NULL ) { free(*pointFeatures) ; *pointFeatures = NULL ; }
/*
** Scan P1 And Test For Requested Feature Type
*/
 cln = nodeAddrP(dtmP,Point)->fPtr ;
 while ( cln != dtmP->nullPtr )
   {
    feature = flistAddrP(dtmP,cln)->dtmFeature ;
    if( ftableAddrP(dtmP,feature)->dtmFeatureType == dtmFeatureType )
      {
/*
**     Get Next And Prior Points For Feature
*/
       np = flistAddrP(dtmP,cln)->nextPnt ;
       pp = dtmP->nullPnt ;
       clc = nodeAddrP(dtmP,Point)->cPtr ;
       while( clc != DTM_NULL_PTR )
         {
          lp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          clm = nodeAddrP(dtmP,lp)->fPtr ;
          while ( clm != dtmP->nullPtr && pp == dtmP->nullPnt )
            {
             if( flistAddrP(dtmP,clm)->dtmFeature == feature && flistAddrP(dtmP,clm)->nextPnt == Point ) pp = lp ;
             clm = flistAddrP(dtmP,clm)->nextPtr ;
            }
         }
/*
**     Allocate memory If Necessary
*/
       if( *numPointFeatures == memFeatureTable )
         {
          memFeatureTable = memFeatureTable + memInc ;
          if( *pointFeatures == NULL ) *pointFeatures = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          else                         *pointFeatures = ( DTM_TIN_POINT_FEATURES * ) realloc ( *pointFeatures , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          if( *pointFeatures == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
/*
**     Store Point Feature
*/
       (*pointFeatures + *numPointFeatures)->dtmFeature     =  feature ;
       (*pointFeatures + *numPointFeatures)->dtmFeatureType =  ftableAddrP(dtmP,feature)->dtmFeatureType ;
       (*pointFeatures + *numPointFeatures)->userTag        =  ftableAddrP(dtmP,feature)->dtmUserTag ;
       (*pointFeatures + *numPointFeatures)->userFeatureId  =  ftableAddrP(dtmP,feature)->dtmFeatureId ;
       (*pointFeatures + *numPointFeatures)->priorPoint     =  pp ;
       (*pointFeatures + *numPointFeatures)->nextPoint      =  np ;
       ++*numPointFeatures ;
      }
/*
**  Reset For Next Feature
*/
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getDtmFeatureTypeOccurrencesForLineDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long P1,long P2,DTM_TIN_POINT_FEATURES **lineFeatures,long *numLineFeatures)
/*
** This Function Gets The List Of Dtm Features For A Dtm Feature For Line P1-P2
**
** Rob Cormack June 2003
*/
{
 int  ret=DTM_SUCCESS ;
 long cln,feature,memFeatureTable=0,memInc=10 ;
/*
** Initialise
*/
 *numLineFeatures = 0 ;
 if( *lineFeatures != NULL ) { free(*lineFeatures) ; *lineFeatures = NULL ; }
/*
** Scan P1 And Test For Connection To P2
*/
 cln = nodeAddrP(dtmP,P1)->fPtr ;
 while ( cln != dtmP->nullPtr )
   {
    feature = flistAddrP(dtmP,cln)->dtmFeature ;
    if( flistAddrP(dtmP,cln)->nextPnt == P2 && ftableAddrP(dtmP,feature)->dtmFeatureType == dtmFeatureType )
      {
/*
**    Allocate memory If Necessary
*/
       if( *numLineFeatures == memFeatureTable )
         {
          memFeatureTable = memFeatureTable + memInc ;
          if( *lineFeatures == NULL ) *lineFeatures = ( DTM_TIN_POINT_FEATURES * ) malloc ( memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          else                        *lineFeatures = ( DTM_TIN_POINT_FEATURES * ) realloc ( *lineFeatures , memFeatureTable * sizeof(DTM_TIN_POINT_FEATURES)) ;
          if( *lineFeatures == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
/*
**     Store Point Feature
*/
       (*lineFeatures + *numLineFeatures)->dtmFeature     =  feature ;
       (*lineFeatures + *numLineFeatures)->dtmFeatureType =  ftableAddrP(dtmP,feature)->dtmFeatureType ;
       (*lineFeatures + *numLineFeatures)->userTag        =  ftableAddrP(dtmP,feature)->dtmUserTag ;
       (*lineFeatures + *numLineFeatures)->userFeatureId  =  ftableAddrP(dtmP,feature)->dtmFeatureId ;
       (*lineFeatures + *numLineFeatures)->priorPoint     =  dtmP->nullPnt ;
       (*lineFeatures + *numLineFeatures)->nextPoint      =  dtmP->nullPnt ;
       ++*numLineFeatures ;
      }
/*
**  Reset For Next Feature
*/
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_getBreakLineUsertagsAtPointDtmObject
(
 BC_DTM_OBJ   *dtmP,
 double       x,
 double       y,
 DTMUserTag **userTagsPP,
 long         *numUserTagsP
)
/*
**
** This Function Retrieves All The Break Line Usertags For A Point
**
** 3D Model Specific Function
**
** Arguements
**
** Tin              ==> Tin Object
** x                ==> x Coordinate Of Point
** y                ==> y Coordinate Of Point
** userTagsPP         <== List Of Usertags
** numUserTagsP <== Number Of userTagsPP In List
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 2. No Validity Checking On x Coordinate
** 3. No Validity Checking On y Coordinate
** 4. userTagsPP Must Be Set To Null
** 5. No Validity Checking On numUserTagsP
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  10th January 2002
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p ;
/*
** Write Entry Information
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Break Line User Tags At Point") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x            = %10.4lf",x) ;
    bcdtmWrite_message(0,0,0,"y            = %10.4lf",y) ;
    bcdtmWrite_message(0,0,0,"userTagsPP   = %p",*userTagsPP) ;
    bcdtmWrite_message(0,0,0,"numUserTagsP = %6ld",*numUserTagsP) ;
   }
/*
** Validate
*/
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { bcdtmWrite_message(1,0,0,"userTagsPP Array Not Initialised To Null") ; goto errexit ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Closest Tin Point To x,y
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Point") ;
 bcdtmFind_closestPointDtmObject(dtmP,x,y,&p) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Closest Point = %6ld ** %10.4lf %10.4lf %10.4lf",p,pointAddrP(dtmP,p)->x,pointAddrP(dtmP,p)->y,pointAddrP(dtmP,p)->z) ;
/*
** If Not A Void Point Scan Point And Get Break Line User Tags
*/
 if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p)->PCWD) )
   {
    if( bcdtmList_getBreakLineUsertagsAtTinPointDtmObject(dtmP,p,userTagsPP,numUserTagsP)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Getting Break Line User Tags Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Getting Break Line User Tags Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { free(*userTagsPP) ; *userTagsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_getBreakLineUsertagsAtTinPointDtmObject
(
 BC_DTM_OBJ   *dtmP,
 long         tinPoint,
 DTMUserTag **userTagsPP,
 long         *numUserTagsP
)
/*
**
** This Function Retrieves All The Break Line Usertags
** For A Tin Point
**
** Arguements
**
** dtmP             ===> Tin Object
** tinPoint         ===> Tin Point
** userTagsPP       <=== List Of Usertags
** numUserTagsP     <=== Number Of userTagsPP In List
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 2. tinPoint Range Is Checked
** 3. userTagsPP Must Be Set To Null
** 4. No Validity Checking On numUserTagsP
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  7th December 2001
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    nf,numBreak,numFeatures ;
 DTM_TIN_POINT_FEATURES  *featuresP=NULL ;
/*
** Write Debug Information
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Break Point User Tags") ;
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"tinPoint            = %6ld",tinPoint) ;
    bcdtmWrite_message(0,0,0,"userTagsPP          = %p",*userTagsPP) ;
    bcdtmWrite_message(0,0,0,"Number Of User Tags = %6ld",*numUserTagsP) ;
   }
/*
** Initialise
*/
 *numUserTagsP = 0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Validate
*/
 if( tinPoint < 0 || tinPoint >= dtmP->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
 if( *userTagsPP != NULL ) { bcdtmWrite_message(2,0,0,"userTagsPP Not Initialised To Null") ; goto errexit ; }
/*
** If Not A Void Point Get Dtm Feature List For Point
*/
 if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,tinPoint)->PCWD) )
   {
/*
**  Get All Features For Tin Point
*/
    if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,tinPoint,&featuresP,&numFeatures)) goto errexit ;
/*
**  Count Number Of Break Lines
*/
    numBreak = 0 ;
    for( nf = 0 ; nf < numFeatures ; ++nf )
      {
       if( (featuresP+nf)->dtmFeatureType == DTMFeatureType::Breakline ) ++numBreak ;
      }
/*
**  If Break Features Found
*/
    if( numBreak > 0 )
      {
/*
**     Allocate Memory For User tagsP
*/
       *numUserTagsP = numBreak ;
       *userTagsPP = ( DTMUserTag * ) malloc( *numUserTagsP * sizeof(DTMUserTag)) ;
       if( *userTagsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**     Copy Break Line User Tags To userTagsPP
*/
       numBreak = 0 ;
       for( nf = 0 ; nf < numFeatures ; ++nf )
         {
          if( (featuresP+nf)->dtmFeatureType == DTMFeatureType::Breakline )
            {
             *(*userTagsPP+numBreak) = (featuresP+nf)->userTag ;
             ++numBreak ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( featuresP != NULL ) free(featuresP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { free(*userTagsPP) ; *userTagsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_getConnectedBreakLineUsertagsAtPointDtmObject
(
 BC_DTM_OBJ   *dtmP,
 double       x,
 double       y,
 DTMUserTag **userTagsPP,
 long         *numUserTagsP
)
/*
**
** This Function Retrieves All The Break Line Usertags
** For Tin Points Connected To The Closest Tin Point To Point x,y
**
** 3D Model Specific Function
**
** Arguements
**
** dtmP             ==> DTM Object
** x                ==> x Coordinate Of Point
** y                ==> y Coordinate Of Point
** userTagsPP       <== List Of Usertags
** numUserTagsP     <== Number Of Usertags In List
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 2. No Validity Checking On x Coordinate
** 3. No Validity Checking On y Coordinate
** 4. userTagsPP Must Be Set To Null
** 5. No Validity Checking On numUserTagsP
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  10th January 2002
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p,clc,npt,addToList,memTags=0,memTagsInc=10,numBrkUserTags ;
 DTMUserTag *brkUserTagsP=NULL,*tagP ;
/*
** Write Debug Information
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Connected Break Line User Tags") ;
    bcdtmWrite_message(0,0,0,"dtmp         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x            = %10.4lf",x) ;
    bcdtmWrite_message(0,0,0,"y            = %10.4lf",y) ;
    bcdtmWrite_message(0,0,0,"userTagsPP   = %p",*userTagsPP) ;
    bcdtmWrite_message(0,0,0,"numUserTagsP = %6ld",*numUserTagsP) ;
   }
/*
** Validate
*/
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { bcdtmWrite_message(1,0,0,"userTagsPP Array Not Initialised To Null") ; goto errexit ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Closest Tin Point To x,y
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Point") ;
 bcdtmFind_closestPointDtmObject(dtmP,x,y,&p) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Closest Point = %6ld ** %10.4lf %10.4lf %10.4lf",p,pointAddrP(dtmP,p)->x,pointAddrP(dtmP,p)->y,pointAddrP(dtmP,p)->z) ;
/*
** If Not A Void Point Scan Point And Get Break Line User Tags For Connected Points
*/
 if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p)->PCWD) )
   {
    clc = nodeAddrP(dtmP,p)->cPtr ;
/*
**  Scan Connected Tin Points
*/
    while( clc != dtmP->nullPtr )
      {
       p   = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
/*
**     If Connected Point Not A Void Point, Get Break Line User Tags For Connected Point
*/
       if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p)->PCWD) )
         {
          if( bcdtmList_getBreakLineUsertagsAtTinPointDtmObject(dtmP,p,&brkUserTagsP,&numBrkUserTags)) goto errexit ;
          if( numBrkUserTags > 0 )
            {
             for( npt = 0 ; npt < numBrkUserTags ; ++npt)
               {
/*
**              Check If User Tag Already In List
*/
                addToList = 1 ;
                for( tagP = *userTagsPP ; tagP < *userTagsPP + *numUserTagsP && addToList ; ++tagP )
                  {
                   if( *(brkUserTagsP+npt) == *tagP ) addToList = 0 ;
                  }
/*
**              Add To List
*/
                if( addToList )
                  {
/*
**                 Check And Allocate Memory If Necessary
*/
                   if( *numUserTagsP == memTags )
                     {
                      memTags = memTags + memTagsInc ;
                      if( *userTagsPP == NULL ) *userTagsPP = (DTMUserTag*)malloc ( memTags * sizeof(DTMUserTag)) ;
                      else                      *userTagsPP = (DTMUserTag*)realloc( *userTagsPP , memTags * sizeof(DTMUserTag)) ;
                      if( *userTagsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                     }
                   *(*userTagsPP+*numUserTagsP) = *(brkUserTagsP+npt) ;
                   ++*numUserTagsP ;
                  }
               }
             free(brkUserTagsP) ;
             brkUserTagsP = NULL ;
            }
         }
      }
   }
/*
** Reallocate memory
*/
 if( *numUserTagsP > 0 ) *userTagsPP = (DTMUserTag*)realloc( *userTagsPP , *numUserTagsP * sizeof(DTMUserTag)) ;
/*
** Clean Up
*/
 cleanup :
 if( brkUserTagsP != NULL ) free(brkUserTagsP) ;
/*
** Return
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Getting Connected Break Line User Tags Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Getting Connected Break Line User Tags Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { free(*userTagsPP) ; *userTagsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_getConnectedUsertagsDtmObject
(
 BC_DTM_OBJ   *dtmP,
 DTMUserTag userTag,
 DTMUserTag **userTagsPP,
 long          *numUserTagsP
)
/*
**
** This Function Retrieves All The User Tags Connected To A Feature Specified By A User Tag
**
** Arguements
**
** dtmP         ==> Dtm Object
** UserTag      ==> UserTag Of Feature To Find Connected User tagsP
** userTagsPP   <== List Of Connected User Tags
** numUserTagsP <== Number Of User tagsP In List
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 2. No Validity Checking On User Tag
** 3. userTagsPP checked if set to NULL
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob  Cormack
** Date   :  17th January 2002
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    nt,dtmFeature,tinPoint,inList,memTags=0,memTagsInc=100,numFeatures ;
 DTMUserTag            utag ;
 BC_DTM_FEATURE          *dtmFeatureP ;
 DTM_TIN_POINT_FEATURES  *featP,*featuresP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Connected userTagsPP") ;
/*
** Initialise
*/
 *numUserTagsP = 0 ;
/*
** Validate
*/
 if( *userTagsPP != NULL ) { bcdtmWrite_message(1,0,0,"userTagsPP Not NULL") ; goto errexit ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Scan Tin Feature Table For UserTag
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmUserTag == userTag )
      {
/*
**     Scan all Feature Points
*/
       tinPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
       do
         {
/*
**        Get List Of DTM Features For Tin Point
*/
          if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,tinPoint,&featuresP,&numFeatures)) goto errexit ;
/*
**        Scan Feature List For Point And Extract User Tags
*/
          for( featP = featuresP ; featP < featuresP + numFeatures ; ++featP )
            {
             utag = featP->userTag ;
/*
**           Scan List Of User tagsP To Determine If Tag Alreay In List
*/
             inList = 0 ;
             if( utag == dtmP->nullUserTag || utag == userTag ) inList = 1 ;
             else
               {
                for( nt = 0 ; nt < *numUserTagsP && ! inList ; ++nt )
                  {
                   if( utag == *(*userTagsPP+nt)) inList = 1 ;
                  }
               }
/*
**           If Tag Not In List Add To List
*/
             if( ! inList )
               {
/*
**              Check Memory
*/
                if( *numUserTagsP == memTags )
                  {
                   memTags = memTags + memTagsInc ;
                   if( *userTagsPP == NULL ) *userTagsPP = ( DTMUserTag *) malloc( memTags * sizeof(DTMUserTag)) ;
                   else                      *userTagsPP = ( DTMUserTag *) realloc( *userTagsPP,memTags*sizeof(DTMUserTag)) ;
                   if( *userTagsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto  errexit ; }
                  }
/*
**              Store User Tag In List
*/
                *(*userTagsPP+*numUserTagsP) = utag ;
                ++*numUserTagsP ;
               }
            }
/*
**        Get Next Point For Feature
*/
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,tinPoint,&tinPoint)) goto errexit ;
/*
**        Continue Loop
*/
         } while ( tinPoint != dtmFeatureP->dtmFeaturePts.firstPoint && tinPoint != dtmP->nullPnt ) ;
      }
   }
/*
** Reallocate Memory
*/
 *userTagsPP = ( DTMUserTag *) realloc(*userTagsPP,*numUserTagsP*sizeof(DTMUserTag)) ;
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
 *numUserTagsP = 0 ;
 if( *userTagsPP != NULL ) { free(*userTagsPP) ; *userTagsPP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_extractDtmFeatureForUsertagDtmObject
(
 BC_DTM_OBJ   *dtmP,
 DTMUserTag usertag,
 long         *numFeaturesP,
 long         *featureTypeP,
 DPoint3d          **featurePtsPP,
 long         *numFeaturePtsP
)
/*
** This Function Extract Points For A DTM Feature From A User Tag
**
** dtmP             ==> Data Object
** usertag          ==> Tag For Feature
** numFeaturesP    <== Number Of Dtm Features With The Same usertag
** featureTypeP    <== Type Of First DTM Feature Found For usertag
** featurePtsP     <== DPoint3d Array For First Feature Found
** numFeaturePtsP  <== Number Of Feature Points
**
** Return Value == 0 Success
**              == 1 System Error
**              == 2 Unknown Feature Type
**
*/
{
 int    ret=DTM_SUCCESS ;
 long   dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *numFeaturesP    = 0 ;
 *numFeaturePtsP  = 0 ;
 *featureTypeP    = DTM_NULL_PNT ;
 if( *featurePtsPP != NULL ) { free(*featurePtsPP) ; *featurePtsPP = NULL ; }
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Scan DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmUserTag == usertag )
         {
          if( *numFeaturesP == 0 )
            {
             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,featurePtsPP,numFeaturePtsP)) goto errexit ;
            }
          ++*numFeaturesP ;
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
BENTLEYDTM_EXPORT int bcdtmList_getFeaturesAtPointDtmObject
(
 BC_DTM_OBJ   *dtmP,
 double       x,
 double       y,
 DTMUserTag features[],
 long         maxFeatures,
 long         *numFeaturesP
)
/*
** This Function Tests If a Point is on a DTM Feature And If So Returns
** The User Tag For The Feature
**
**  Return Values  = 0  Successfull
**                 = 1  System Error
**                 = 2  Point Not In Data Object
**
*/
{
 int    ret=DTM_SUCCESS ;
 long   point,dtmFeature ;
 DPoint3d    *p3dP,*ptsP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pointP ;

/*
** Initialise
*/
 *numFeaturesP = 0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process For Data Or Tin States
*/
 if( dtmP->dtmState == DTMState::Data || dtmP->dtmState == DTMState::Tin )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**     Tin Feature
*/
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          point = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             pointP = pointAddrP(dtmP,point) ;
             if( fabs( pointP->x - x ) <= dtmP->ppTol  && fabs(pointP->y - y ) <= dtmP->ppTol )
               {
                if( *numFeaturesP < maxFeatures )
                  {
                   features[*numFeaturesP] = dtmFeatureP->dtmUserTag ;
                   ++(*numFeaturesP) ;
                  }
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&point)) goto errexit ;
            } while ( point != dtmP->nullPnt && point != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
/*
**     Data Feature
*/
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          point = dtmFeatureP->dtmFeaturePts.firstPoint ;
          for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
            {
             pointP = pointAddrP(dtmP,point) ;
             if( fabs( pointP->x - x ) <= dtmP->ppTol  && fabs(pointP->y - y ) <= dtmP->ppTol )
               {
                if( *numFeaturesP < maxFeatures )
                  {
                   features[*numFeaturesP] = dtmFeatureP->dtmUserTag ;
                   ++(*numFeaturesP) ;
                  }
               }
            }
         }
/*
**     Point Array Feature
*/
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
         {
          ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
          for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
            {
             if( fabs( p3dP->x - x ) <= dtmP->ppTol  && fabs(p3dP->y - y ) <= dtmP->ppTol )
               {
                if( *numFeaturesP < maxFeatures )
                  {
                   features[*numFeaturesP] = dtmFeatureP->dtmUserTag ;
                   ++(*numFeaturesP) ;
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
 if( ret == DTM_SUCCESS && ! *numFeaturesP ) ret = 2 ;
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
/*------------------------------------------------------------+
|                                                             |
|                                                             |
|                                                             |
+------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_extractHullWithConnectingUserTagsDtmObject(BC_DTM_OBJ *dtmP,P3DTAG **hullPtsPP,long *numHullPtsP)
/*
** This Function Extracts The Tin Hull Coordinates Together With Their DtmFeatures And UserTags
**
** Arguements
**
** Tin          ==> Tin Object
** hullPtsPP    <== Hull Coordinates With Their Dtm Features And UserTags
** numHullPtsP  <== Size Of Hullpts Array
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 3. hullPtsPP Must Be Set To Null
** 5. No Validity Checking On NumHullpts
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  10th December 2001
**
*/
{
 int     ret=DTM_SUCCESS;
 long    sp,np,pp,nf,numPrior,numNext,numFeat,inList ;
 DTMFeatureType *priorDtmFeatureTypesP = NULL, *nextDtmFeatureTypesP = NULL;
 DTMUserTag *priorUserTagsP=NULL,*nextUserTagsP=NULL ;
 P3DTAG  *hullP ;
 FEATTAG *ftP,*ft1P,*ft2P ;
/*
** Validate Input Parameters
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != NULL ) { bcdtmWrite_message(1,0,0,"Hull Points Not Initialised To Null") ; goto errexit ; }
/*
** Count Number of Points In Hull
*/
 np = dtmP->hullPoint ;
 do
   {
    ++*numHullPtsP ;
    np = nodeAddrP(dtmP,np)->hPtr ;
   } while ( np != dtmP->hullPoint ) ;
 ++*numHullPtsP ;
/*
** Allocate Memory To Store Hull Points
*/
 *hullPtsPP = ( P3DTAG * ) malloc(*numHullPtsP*sizeof(P3DTAG)) ;
 if( *hullPtsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Store Hull Points
*/
 hullP = *hullPtsPP ;
 sp = dtmP->hullPoint ;
 np = nodeAddrP(dtmP,sp)->hPtr ;
 if(( pp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
 do
   {
    hullP->x = pointAddrP(dtmP,sp)->x  ;
    hullP->y = pointAddrP(dtmP,sp)->y  ;
    hullP->z = pointAddrP(dtmP,sp)->z  ;
    hullP->NumberOfTags = 0 ;
    hullP->Tags = NULL ;
/*
** Get Dtm Features And Tags For Hull Point
*/
    if( bcdtmList_getDtmFeaturesAndUserTagsForTinLineDtmObject(dtmP,pp,sp,&priorDtmFeatureTypesP,&priorUserTagsP,&numPrior)) goto errexit ;
    if( bcdtmList_getDtmFeaturesAndUserTagsForTinLineDtmObject(dtmP,sp,np,&nextDtmFeatureTypesP,&nextUserTagsP,&numNext)) goto errexit ;
/*
** Copy Dtm Features And Tags To hullPtsPP Array
*/
    numFeat = numPrior + numNext ;
    if( numFeat > 0 )
      {
/*
**      Allocate memory For Dtm Features And Tags
*/
       hullP->Tags = ( FEATTAG *) malloc( numFeat*sizeof(FEATTAG)) ;
       if( hullP->Tags == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**    Copy Prior Dtm Features And UserTags To hullp->Tag Array
*/
       for( nf = 0 , ftP = hullP->Tags ; nf < numPrior ; ++nf , ++ftP )
         {
         ftP->DtmFeature = *(priorDtmFeatureTypesP + nf);
          ftP->UserTag    = *(priorUserTagsP+nf) ;
         }
/*
**     Copy Next Dtm Features And UserTags To hullp->Tag Array
*/
       ft2P = ftP ;
       for( nf = 0 ; nf < numNext ; ++nf )
         {
          inList = 0 ;
          for( ft1P = hullP->Tags ; ft1P < ft2P && ! inList ; ++ft1P )
            {
            if (ft1P->DtmFeature == *(nextDtmFeatureTypesP + nf) && ft1P->UserTag == *(nextUserTagsP + nf)) inList = 1;
            }
          if( ! inList )
            {
            ftP->DtmFeature = *(nextDtmFeatureTypesP + nf);
             ftP->UserTag    = *(nextUserTagsP+nf) ;
             ++ftP ;
            }
         }
/*
**     Set Number Of Tags
*/
       hullP->NumberOfTags = (long)(ftP-hullP->Tags) ;
/*
**     Free memory
*/
       free(priorDtmFeatureTypesP) ; priorDtmFeatureTypesP = NULL ;
       free(priorUserTagsP)    ; priorUserTagsP    = NULL ;
       free (nextDtmFeatureTypesP); nextDtmFeatureTypesP = NULL;
       free(nextUserTagsP)     ; nextUserTagsP     = NULL ;
      }
    else hullP->Tags = NULL ;
/*
** Reset For Next Hull Point
*/
    ++hullP ;
    pp = sp ;
    sp = np ;
    np = nodeAddrP(dtmP,sp)->hPtr ;
   } while ( sp != dtmP->hullPoint )  ;
/*
** Set Last Point Equal To First Point To Close Hull Polygon
*/
 hullP->x = (*hullPtsPP)->x ;
 hullP->y = (*hullPtsPP)->y ;
 hullP->z = (*hullPtsPP)->z ;
 hullP->NumberOfTags = (*hullPtsPP)->NumberOfTags ;
 hullP->Tags = ( FEATTAG *) malloc(hullP->NumberOfTags*sizeof(FEATTAG)) ;
 if( hullP->Tags == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 memcpy(hullP->Tags,(*hullPtsPP)->Tags,hullP->NumberOfTags*sizeof(FEATTAG)) ;
/*
** Clean Up
*/
 cleanup :
 if( priorDtmFeatureTypesP != NULL ) free(priorDtmFeatureTypesP) ;
 if( priorUserTagsP    != NULL ) free(priorUserTagsP) ;
 if (nextDtmFeatureTypesP != NULL) free (nextDtmFeatureTypesP);
 if( nextUserTagsP     != NULL ) free(nextUserTagsP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
/*
** Free memory
*/
 if( *hullPtsPP != NULL )
   {
    for( hullP = *hullPtsPP ; hullP < *hullPtsPP + *numHullPtsP ; ++hullP)
      {
       if( hullP->Tags != NULL ) { free(hullP->Tags) ; hullP->Tags = NULL ; }
      }
    free(*hullPtsPP) ; *hullPtsPP = NULL ;
   }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getDtmFeaturesAndUserTagsForTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,DTMFeatureType **dtmFeaturesTypesPP, DTMUserTag **dtmUserTagsPP,long *numFeaturesP)
/*
** This Function Retrieves All The DTM dtmFeaturesPP And User Tags For A Tin Line
**
** Arguements
**
** Tin              ==> Tin Object
** P1               ==> Tin Point Number For Line End Point
** P2               ==> Tin Point Number For Line End Point
** dtmFeaturesPP    <== List Of dtmFeaturesPP
** dtmUserTagsPP    <== List Of Usertags
** numFeaturesP     <== Size Of Feature and UserTag Lists
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 2. Tin Point Range Is Checked
** 3. dtmFeaturesPP Must Be Set To Null
** 4. dtmUserTagsPP Must Be Set To Null
** 5. No Validity Checking On numFeaturesP
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  7th December 2001
**
*/
{
 int  ret=DTM_SUCCESS ;
 long cln,nfeat ;
/*
** Initialise
*/
 *numFeaturesP = 0 ;
/*
** Validate
*/
 if( P1 < 0 || P1 >= dtmP->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
 if( P2 < 0 || P2 >= dtmP->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
 if (*dtmFeaturesTypesPP != NULL) { bcdtmWrite_message (2, 0, 0, "dtmFeaturesPP Not Initialised To Null"); goto errexit; }
 if( *dtmUserTagsPP != NULL ) { bcdtmWrite_message(2,0,0,"dtmUserTagsPP Not Initialised To Null") ; goto errexit ; }
/*
** Count Number Of Dtm Features At Point
*/
 if(nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 ) ++*numFeaturesP ;
/*
** Scan Feature List For Point P1 And Count dtmFeaturesPP That Connect To P2
*/
 cln = nodeAddrP(dtmP,P1)->fPtr ;
 while( cln != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,cln)->nextPnt == P2 )  ++*numFeaturesP ;
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
/*
** Scan Feature List For Point P2 And Count dtmFeaturesPP That Connect To P1
*/
 cln = nodeAddrP(dtmP,P2)->fPtr ;
 while( cln != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,cln)->nextPnt == P1 )  ++*numFeaturesP ;
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
/*
** If Feature Found Found Store In User Arrays
*/
 if( *numFeaturesP > 0 )
   {
/*
**  Allocate Memory For dtmFeaturesPP And dtmUserTagsPP
*/
   *dtmFeaturesTypesPP = (DTMFeatureType *)malloc (*numFeaturesP * sizeof(DTMFeatureType));
    *dtmUserTagsPP = ( DTMUserTag *) malloc(*numFeaturesP * sizeof(DTMUserTag)) ;
    if( *dtmFeaturesTypesPP == NULL || *dtmUserTagsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**  Copy dtmFeaturesPP And dtmUserTagsPP To User Arrays
*/
    nfeat = 0 ;
/*
**  Copy Hull feature If Point On Tin Hull
*/
    if(nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 )
      {
      *(*dtmFeaturesTypesPP + nfeat) = DTMFeatureType::Hull;
       *(*dtmUserTagsPP+nfeat) = DTM_NULL_USER_TAG ;
       ++nfeat ;
      }
/*
**  Copy Feature List For P1
*/
    cln = nodeAddrP(dtmP,P1)->fPtr ;
    while( cln != dtmP->nullPtr )
      {
       if(flistAddrP(dtmP,cln)->nextPnt == P2 )
         {
         *(*dtmFeaturesTypesPP + nfeat) = ftableAddrP (dtmP, flistAddrP (dtmP, cln)->dtmFeature)->dtmFeatureType;
          *(*dtmUserTagsPP+nfeat) = ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmUserTag ;
          ++nfeat ;
         }
       cln = flistAddrP(dtmP,cln)->nextPtr ;
      }
/*
**  Copy Feature List For P2
*/
    cln = nodeAddrP(dtmP,P2)->fPtr ;
    while( cln != dtmP->nullPtr )
      {
       if(flistAddrP(dtmP,cln)->nextPnt == P1 )
         {
         *(*dtmFeaturesTypesPP + nfeat) = ftableAddrP (dtmP, flistAddrP (dtmP, cln)->dtmFeature)->dtmFeatureType;
          *(*dtmUserTagsPP+nfeat) = ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmUserTag ;
          ++nfeat ;
         }
       cln = flistAddrP(dtmP,cln)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_testForBreakLineDirectionDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1P2 is A Break Line
*/
{
 long clc ;
/*
** Test For P1 P2 Being Break Line
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmList_extractHullWithDtmFeatureTypeAndUserTagsDtmObject(BC_DTM_OBJ *dtmP,P3DTAG **hullPtsPP,long *numHullPtsP)
/*
** This Function Extracts The Tin Hull Coordinates Together With Their DtmfeatureTypesP And userTagsP
**
** Arguements
**
** Tin         ==> Tin Object
** hullPtsPP     <== Hull Coordinates With Their Dtm featureTypesP And userTagsP
** numHullPtsP  <== Size Of Hullpts Array
**
** Arguement Validation
**
** 1. No Validity Checking On The Tin Object
** 3. hullPtsPP Must Be Set To Null
** 5. No Validity Checking On NumHullpts
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  10th December 2001
**
*/
{
 int     ret=DTM_SUCCESS ;
 long    np,nf,numPointFeatures ;
 P3DTAG  *hullP ;
 FEATTAG *ftP   ;
 DTM_TIN_POINT_FEATURES *pointFeaturesP=NULL ;
/*
** Validate Input Parameters
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != NULL ) { bcdtmWrite_message(1,0,0,"Hull Points Not Initialised To Null") ; goto errexit ; }
/*
** Count Number of Points In Hull
*/
 np = dtmP->hullPoint ;
 do
   {
    ++*numHullPtsP ;
    np = nodeAddrP(dtmP,np)->hPtr ;
   } while ( np != dtmP->hullPoint ) ;
 ++*numHullPtsP ;
/*
** Allocate Memory To Store Hull Points
*/
 *hullPtsPP = ( P3DTAG * ) malloc(*numHullPtsP*sizeof(P3DTAG)) ;
 if( *hullPtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store Hull Points
*/
 hullP = *hullPtsPP ;
 np = dtmP->hullPoint ;
 do
   {
    hullP->x = pointAddrP(dtmP,np)->x  ;
    hullP->y = pointAddrP(dtmP,np)->y  ;
    hullP->z = pointAddrP(dtmP,np)->z  ;
    hullP->NumberOfTags = 0 ;
    hullP->Tags = NULL ;
/*
**  Get Dtm Feature Types And Tags For Hull Point
*/
    if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,np,&pointFeaturesP,&numPointFeatures)) goto errexit ;
/*
**  Copy To Hull Array
*/
    if( numPointFeatures > 0 )
      {
/*
**     Allocate memory For Dtm featureTypesP And Tags
*/
       hullP->Tags = ( FEATTAG *) malloc( numPointFeatures*sizeof(FEATTAG)) ;
       if( hullP->Tags == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
**     Copy Dtm Feature Types And userTagsP To hullp->Tag Array
*/
       hullP->NumberOfTags = numPointFeatures ;
       for( nf = 0 , ftP = hullP->Tags ; nf < numPointFeatures ; ++nf , ++ftP )
         {
          ftP->DtmFeature = (pointFeaturesP+nf)->dtmFeatureType ;
          ftP->UserTag    = (pointFeaturesP+nf)->userTag  ;
         }
/*
**     Free memory
*/
       if( pointFeaturesP != NULL ) { free(pointFeaturesP) ; pointFeaturesP = NULL ;}
      }
    else hullP->Tags = NULL ;
/*
** Reset For Next Hull Point
*/
    ++hullP ;
    np = nodeAddrP(dtmP,np)->hPtr ;
   } while ( np != dtmP->hullPoint )  ;
/*
** Set Last Point Equal To First Point To Close Hull Polygon
*/
 hullP->x = (*hullPtsPP)->x ;
 hullP->y = (*hullPtsPP)->y ;
 hullP->z = (*hullPtsPP)->z ;
 hullP->NumberOfTags = (*hullPtsPP)->NumberOfTags ;
 hullP->Tags = ( FEATTAG *) malloc(hullP->NumberOfTags*sizeof(FEATTAG)) ;
 if( hullP->Tags == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 memcpy(hullP->Tags,(*hullPtsPP)->Tags,hullP->NumberOfTags*sizeof(FEATTAG)) ;
/*
** Clean Up
*/
 cleanup :
 if( pointFeaturesP != NULL ) { free(pointFeaturesP) ; pointFeaturesP = NULL ;}
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
/*
** Free memory
*/
 if( *hullPtsPP != NULL )
   {
    for( hullP = *hullPtsPP ; hullP < *hullPtsPP + *numHullPtsP ; ++hullP)
      {
       if( hullP->Tags != NULL ) { free(hullP->Tags) ; hullP->Tags = NULL ; }
      }
    free(*hullPtsPP) ; *hullPtsPP = NULL ;
   }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_expandTptrPolygonAtPointDtmObject( BC_DTM_OBJ *dtmP, long *pointP)
/*
** This Function Expands A Tptr Polygon At A Tptr Polygon Point
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   expandType,pnt,antPoint,nextPnt,cleanPnt,pPoint;
 DTMDirection direction;
 long   point,priorPoint,nextPoint,startPoint,expansionNextPoint,expansionPriorPoint ;
 double beforeArea = 0.0,afterArea ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointP      = %8ld",*pointP) ;
   }
/*
** Only Expand For An Internal Point
*/
 if( nodeAddrP(dtmP,*pointP)->hPtr == dtmP->nullPnt )
   {
/*
**  Perform Area Checks
*/
    if( cdbg )
      {
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pointP,&beforeArea,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon Area      = %15.5lf",beforeArea) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon Direction = %2ld",direction) ;
      }
/*
**  Initialise
*/
    point = *pointP ;
    startPoint = dtmP->nullPnt ;
    priorPoint = nextPoint = nodeAddrP(dtmP,point)->tPtr ;
    while( nodeAddrP(dtmP,priorPoint)->tPtr != point)
      {
       if(( priorPoint = bcdtmList_nextClkDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
      }
/*
** Write Point Stats
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"priorPoint = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",priorPoint,nodeAddrP(dtmP,priorPoint)->tPtr,pointAddrP(dtmP,priorPoint)->x,pointAddrP(dtmP,priorPoint)->y,pointAddrP(dtmP,priorPoint)->z) ;
       bcdtmWrite_message(0,0,0,"point      = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",point,nodeAddrP(dtmP,point)->tPtr,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
       bcdtmWrite_message(0,0,0,"nextPoint  = %8ld Tptr = %8ld ** %12.5lf %12.5lf %10.4lf",nextPoint,nodeAddrP(dtmP,nextPoint)->tPtr,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
      }
/*
**  Write Tptr Polygon About Expand Point
*/
    if( dbg == 1 )
      {
       pnt = priorPoint ;
       bcdtmWrite_message(0,0,0,"pnt = %8ld ** pnt->tptr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr) ;
       do
         {
          if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
          bcdtmWrite_message(0,0,0,"pnt = %8ld ** pnt->tptr = %10ld pnt->z = %10.4lf",pnt,nodeAddrP(dtmP,pnt)->tPtr,pointAddrP(dtmP,pnt)->z) ;
         } while( pnt != nextPoint ) ;
      }
/*
**  Determine Expand Type ( 1 = Simple , 2 = Complex )
*/
    expandType = 1 ;
    if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
    while( pnt != priorPoint )
      {
       if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) expandType = 2 ;
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"expandType = %2ld",expandType) ;
/*
**  Simple Expansion
*/
    if( expandType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"***** Expanding Simple") ;
/*
**     Set Tptr About Expansion Point
*/
       pPoint = priorPoint ;
       do
         {
          if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,point,pPoint)) < 0 ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"pPoint = %8ld antPoint = %8ld antPnt->tPtr = %9ld",pPoint,antPoint,nodeAddrP(dtmP,antPoint)->tPtr ) ;
          nodeAddrP(dtmP,pPoint)->tPtr = antPoint ;
          pPoint = antPoint ;
         } while ( pPoint != nextPoint && nodeAddrP(dtmP,antPoint)->tPtr != pPoint  ) ;
/*
**      Removed Sections Of Tptr Polygon
*/
        nodeAddrP(dtmP,point)->tPtr = dtmP->nullPnt ;
        startPoint = nextPoint ;
      }
/*
**  Complex Expansion
*/
    if( expandType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"***** Expanding Complex") ;
/*
**     Determine Closure Directions
*/
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt )
            {
             nodeAddrP(dtmP,point)->tPtr = pnt ;
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,point,&afterArea,&direction)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"pnt = %8ld ** closure = %2ld",pnt,direction) ;
             nodeAddrP(dtmP,pnt)->sPtr = (long)direction ;
             nodeAddrP(dtmP,point)->tPtr = nextPoint ;
            }
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }
/*
**     Set Next Point For Expansion
*/
       expansionNextPoint = nextPoint ;
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          if( nodeAddrP(dtmP,pnt)->sPtr == 2 ) expansionNextPoint = pnt ;
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }
/*
**     Set Prior Point For Expansion
*/
       expansionPriorPoint = priorPoint ;
       if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
       while( pnt != expansionNextPoint )
         {
          if( nodeAddrP(dtmP,pnt)->sPtr == 1 ) expansionPriorPoint = pnt ;
          if( ( pnt = bcdtmList_nextAntDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }
/*
**     Set Point To Clean Replaced Section Of Tptr Polygon
*/
       cleanPnt = nodeAddrP(dtmP,expansionPriorPoint)->tPtr ;
       if( dbg ) bcdtmWrite_message(0,0,0,"expansionNext = %8ld expansionPrior = %8ld claenPnt = %8ld",expansionNextPoint,expansionPriorPoint,cleanPnt) ;
/*
**     Set Tptr About Expansion Point
*/
       pPoint = expansionPriorPoint ;
       do
         {
          if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,point,pPoint)) < 0 ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"pPoint = %8ld antPoint = %8ld antPnt->tPtr = %9ld",pPoint,antPoint,nodeAddrP(dtmP,antPoint)->tPtr ) ;
          nodeAddrP(dtmP,pPoint)->tPtr = antPoint ;
          pPoint = antPoint ;
         } while ( pPoint != expansionNextPoint ) ;
/*
**     Remove Old Sections Of Tptr Polygon
*/
       while( cleanPnt != expansionNextPoint )
         {
          nextPnt = nodeAddrP(dtmP,cleanPnt)->tPtr ;
          nodeAddrP(dtmP,cleanPnt)->tPtr = dtmP->nullPnt ;
          cleanPnt = nextPnt ;
         }
/*
**     Clean Sptr Settings
*/
       if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint)) < 0 ) goto errexit ;
       while( pnt != priorPoint )
         {
          nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
          if( ( pnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
         }
/*
**     Set New Index Point For Tptr Polygon
*/
       startPoint = expansionNextPoint ;
      }
/*
**   Set Return Value
*/
    *pointP = startPoint ;
/*
**  Perform Area Checks
*/
    if( cdbg )
      {
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pointP,&afterArea,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon Area      = %15.5lf",afterArea) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon Direction = %2ld",direction) ;
       if( afterArea < beforeArea )
         {
          bcdtmWrite_message(0,0,0,"Tptr Polygon Area Has Decreased") ;
          bcdtmWrite_message(0,0,0,"Expansion Point = %8ld ** beforeArea = %15.5lf afterArea = %15.5lf",point,beforeArea,afterArea) ;
          bcdtmList_writeTptrListDtmObject(dtmP,*pointP) ;
          goto errexit ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon At Point Error") ;
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
BENTLEYDTM_Public int bcdtmList_testForVoidLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       P1,
 long       P2,
 long       *voidLineP
)
/*
** This Function Tests If The Line P1-P2 is A Void Or Hole Hull Line
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long fPtr,nextPnt,priorPnt,p1OnHull=0,p2OnHull=0,process ;
/*
** Write Entry Message
*/
// return( bcdtmList_testForVoidLineDtmObjectOld(dtmP,P1,P2,voidLineP)) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Testing For Line Internal To Void") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"P1         = %8ld",P1) ;
    bcdtmWrite_message(0,0,0,"P2         = %8ld",P2) ;
    bcdtmWrite_message(0,0,0,"*voidLineP = %8ld",*voidLineP) ;
   }
/*
** Initialise
*/
 *voidLineP = FALSE ;
/*
** If Either Point Is A Void Point The Line Is Internal To A Void
*/
 if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) || bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P2)->PCWD) )
   {
    *voidLineP = TRUE ;
   }
/*
** Test For Line Spanning Void Or Island Hull
*/
 else
   {
/*
**  Determine If Points Are On Void Or Island Hulls
*/
    p1OnHull = bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,P1) ;
    if( p1OnHull ) p2OnHull = bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,P2) ;
/*
**  Only Check For Void Line If Both Points Are On Hulls
*/
    if( p1OnHull && p2OnHull )
      {
/*
**     If Points Are On More Than One Void Or Island Hull Use Old Algorithm
*/
       if( p1OnHull > 1 || p2OnHull > 1 )
         {
          return( bcdtmList_testForVoidLineDtmObjectOld(dtmP,P1,P2,voidLineP)) ;
         }
       process = 1 ;
       fPtr = nodeAddrP(dtmP,P1)->fPtr ;
       while ( fPtr != dtmP->nullPtr && *voidLineP == FALSE && process )
         {
          if( flistAddrP(dtmP,fPtr)->nextPnt == P2 ) process = 0 ;  // Consecutive Points On Hull
/*
**        P1 On An Island Hull
*/
          else if( ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"P1 On Island Hull") ;
             bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature,P1,&priorPnt) ;
             if( priorPnt == dtmP->nullPnt )
               {
                bcdtmWrite_message(1,0,0,"Invalid Prior Point") ;
                goto errexit ;
               }
             nextPnt = flistAddrP(dtmP,fPtr)->nextPnt ;
             if( bcdtmList_testForPointBetweenCircularListPointsDtmObject(dtmP,P1,nextPnt,priorPnt,P2,voidLineP)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"*voidLineP = %2ld",*voidLineP) ;
            }
/*
**        P1 On A Void Hull
*/
          else if( ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
                   ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    )
            {
             bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature,P1,&priorPnt) ;
             if( priorPnt == dtmP->nullPnt )
               {
                bcdtmWrite_message(1,0,0,"Invalid Prior Point") ;
                goto errexit ;
               }
             nextPnt = flistAddrP(dtmP,fPtr)->nextPnt ;
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"P1       = %8ld ** %12.4lf %12.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
                bcdtmWrite_message(0,0,0,"priorPnt = %8ld ** %12.4lf %12.4lf %10.4lf",priorPnt,pointAddrP(dtmP,priorPnt)->x,pointAddrP(dtmP,priorPnt)->y,pointAddrP(dtmP,priorPnt)->z) ;
                bcdtmWrite_message(0,0,0,"nextPnt  = %8ld ** %12.4lf %12.4lf %10.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z) ;
                bcdtmWrite_message(0,0,0,"P2       = %8ld ** %12.4lf %12.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
               }
             if( bcdtmList_testForPointBetweenCircularListPointsDtmObject(dtmP,P1,priorPnt,nextPnt,P2,voidLineP)) goto errexit ;
            }
/*
**        Get Next Feature At P1
*/
          fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
         }
      }
   }
/*
** Log Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"*voidLineP = %8ld",*voidLineP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Line Internal To Void Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Line Internal To Void Error") ;
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
BENTLEYDTM_Public int bcdtmList_getPriorPointForDtmFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 long       currentPnt,
 long       *priorPntP
)
/*
** This Function Gets The Prior Point For A Dtm Feature
*/
{
 int  dbg=DTM_TRACE_VALUE(0) ;
 long clPnt,clPtr,flPtr,ppPnt ;
 BC_DTM_FEATURE *dtmFeatureP = nullptr ;
/*
** Initialise
*/
 *priorPntP = dtmP->nullPnt ;
/*
** Determine Feature Type
*/
 if (dtmFeature != dtmP->nullPnt)
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Scan From Start If A Group Spot Feature
*/
 if( dtmFeatureP && dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
   {
    ppPnt = dtmP->nullPnt ;
    clPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
    if( clPnt != dtmP->nullPnt )
      {
       clPtr  = nodeAddrP(dtmP,clPnt)->fPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          while ( clPtr != dtmP->nullPtr && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
          if( clPtr != dtmP->nullPtr )
            {
             ppPnt = clPnt ;
             clPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
             if( clPnt != dtmP->nullPnt )
               {
                if( clPnt == currentPnt ) *priorPntP = ppPnt ;
                clPtr = nodeAddrP(dtmP,clPnt)->fPtr ;
               }
             if( *priorPntP != dtmP->nullPnt || clPnt == dtmP->nullPnt || clPnt == dtmFeatureP->dtmFeaturePts.firstPoint ) clPtr = dtmP->nullPtr ;
            }
         }
      }
   }
/*
** Scan About Current Point If Not A Group Spot Feature
*/
 else
   {
 clPtr = nodeAddrP(dtmP,currentPnt)->cPtr ;
 while( clPtr != dtmP->nullPtr && *priorPntP == dtmP->nullPnt )
   {
    clPnt = clistAddrP(dtmP,clPtr)->pntNum  ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Scanning Point %8ld",clPnt) ;
         }
/*
**  Scan Feature List For List Point
*/
    flPtr = nodeAddrP(dtmP,clPnt)->fPtr ;
    while( flPtr != dtmP->nullPtr && *priorPntP == dtmP->nullPnt )
      {
          if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld nextPnt = %8ld",flistAddrP(dtmP,flPtr)->dtmFeature,flistAddrP(dtmP,flPtr)->nextPnt) ;
       if( flistAddrP(dtmP,flPtr)->dtmFeature == dtmFeature && flistAddrP(dtmP,flPtr)->nextPnt == currentPnt )
         {
          *priorPntP = clPnt ;
         }
       flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
         }
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
BENTLEYDTM_Public int bcdtmList_testForPointBetweenCircularListPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       point,
 long       firstPoint,
 long       lastPoint,
 long       testPoint,
 long       *testPointFoundP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long clPtr,firstPntFnd ;
 DTM_CIR_LIST *clistP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Testing For Point Between Circular List Points") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point       = %8ld ** %12.4lf %12.4lf %10.4lf",point,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
    bcdtmWrite_message(0,0,0,"firstPoint  = %8ld ** %12.4lf %12.4lf %10.4lf",firstPoint,pointAddrP(dtmP,firstPoint)->x,pointAddrP(dtmP,firstPoint)->y,pointAddrP(dtmP,firstPoint)->z) ;
    bcdtmWrite_message(0,0,0,"lastPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",lastPoint,pointAddrP(dtmP,lastPoint)->x,pointAddrP(dtmP,lastPoint)->y,pointAddrP(dtmP,lastPoint)->z) ;
    bcdtmWrite_message(0,0,0,"testPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",testPoint,pointAddrP(dtmP,testPoint)->x,pointAddrP(dtmP,testPoint)->y,pointAddrP(dtmP,testPoint)->z) ;
   }
/*
** Initialise
*/
 *testPointFoundP = FALSE ;
/*
** Check Point Range
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints || lastPoint < 0 || lastPoint >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
**  Initialise Circular List Pointer
*/
 clPtr =  nodeAddrP(dtmP,point)->cPtr ;
 if( clPtr != dtmP->nullPtr )
   {
/*
**  Scan To First Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To First Point ** %8ld",firstPoint) ;
    firstPntFnd = FALSE ;
    while( clPtr != dtmP->nullPtr && firstPntFnd == FALSE )
      {
       clistP = clistAddrP(dtmP,clPtr) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** %8ld",clistP->pntNum) ;
       if( clistP->pntNum == firstPoint ) firstPntFnd = TRUE ;
       else                               clPtr = clistP->nextPtr ;
      }
/*
**  Scan To Second Point
*/
    if( firstPntFnd == TRUE )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Last Point ** %8ld",lastPoint) ;
       do
         {
          clPtr = clistP->nextPtr ;
          if( clPtr == dtmP->nullPtr ) clPtr = nodeAddrP(dtmP,point)->cPtr ;
          clistP = clistAddrP(dtmP,clPtr) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"==== %8ld",clistP->pntNum) ;
          if( clistP->pntNum != lastPoint && clistP->pntNum == testPoint ) *testPointFoundP = TRUE ;
         } while ( clistP->pntNum != firstPoint && clistP->pntNum != lastPoint && *testPointFoundP == FALSE ) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Point Between Circular List Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Point Between Circular List Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"Error Exiting") ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_reportNumberOfPointsMarkedForDeleteDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       reportOption,
 long       *numMarkedPointsP
)
{
 long point ;
/*
** Initialise
*/
 *numMarkedPointsP = 0 ;
 if( dtmP->dtmState == DTMState::Tin )
   {
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       if(  bcdtmFlag_testDeletePointBitPCWD(&nodeAddrP(dtmP,point)->PCWD) )
         {
          ++*numMarkedPointsP ;
          if( reportOption ) bcdtmWrite_message(0,0,0,"Point[%8ld] Marked For Delete ** %12.5lf %12.5lf %10.4lf",point,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
         }
      }
   }
 if( reportOption ) bcdtmWrite_message(0,0,0,"Number Of Points Marked For Delete = %8ld of %8ld",*numMarkedPointsP,dtmP->numPoints) ;
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
BENTLEYDTM_Public int bcdtmList_testIfPointOnDtmFeatureDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature,
 long       point
)
/*
** This Function Tests If a Point is on a DTM Feature
*/
{
 long clc ;
/*
** Scan Feature List Points For Point
*/
 clc = nodeAddrP(dtmP,point)->fPtr ;
 while( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->dtmFeature == dtmFeature )
      {
       return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_checkForValidTriangleDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 int               trgPoint1,                  // ==> Triangle Point1
 int               trgPoint2,                  // ==> Triangle Point2
 int               trgPoint3                   // ==> Triangle Point3
)

//  This Function Should Only Be Called For Development And Testing Purposes
//  The Triangle Points trgPoint1-trgPoint2-trgPoint3 Must Be Set Clockwise

{
 int ret=DTM_SUCCESS ;
 int clkPoint ;

 //  Check For Valid DTM

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

 //  Check Triangle Point Ranges

 if( trgPoint1 < 0 || trgPoint1 >= dtmP->numPoints ||
     trgPoint2 < 0 || trgPoint2 >= dtmP->numPoints ||
     trgPoint3 < 0 || trgPoint3 >= dtmP->numPoints
    )
    {
     bcdtmWrite_message(2,0,0,"Triangle Point Range Error") ;
     goto errexit ;
    }

  // Check Triangle Points Are Clockwise

 if( bcdtmMath_pointSideOfDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3) != -1 ||
     bcdtmMath_pointSideOfDtmObject(dtmP,trgPoint2,trgPoint3,trgPoint1) != -1 ||
     bcdtmMath_pointSideOfDtmObject(dtmP,trgPoint3,trgPoint1,trgPoint2) != -1
   )
   {
    bcdtmWrite_message(2,0,0,"Triangle Points Are Not Clockwise") ;
    goto errexit ;
   }

 // Check Triangle Points Connect

 if( ! bcdtmList_testLineDtmObject(dtmP,trgPoint1,trgPoint2) ||
     ! bcdtmList_testLineDtmObject(dtmP,trgPoint2,trgPoint3) ||
     ! bcdtmList_testLineDtmObject(dtmP,trgPoint3,trgPoint1)
   )
   {
    bcdtmWrite_message(2,0,0,"Triangle Points Do Not Connect") ;
    goto errexit ;
   }

 // Check Triangle Topology

 if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,trgPoint1,trgPoint2)) < 0 ) goto errexit ;
 if( clkPoint != trgPoint3)
   {
    bcdtmWrite_message(2,0,0,"Triangle Topology Error") ;
    goto errexit ;
   }

// Clean Up

 cleanup :

// Return

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
BENTLEYDTM_Public int  bcdtmList_checkForPointOnHullLineDtmObject(BC_DTM_OBJ *dtmP,long Point,long *HullLine)
/*
** This Function Tests For A Point On A DTM Hull
*/
{
 long clc,feat ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *HullLine = 0 ;
 if( nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt ) { *HullLine = 1 ; return(0) ; }
/*
** Scan Feature List For Point
*/
 clc = nodeAddrP(dtmP,Point)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    feat = flistAddrP(dtmP,clc)->dtmFeature ;
    dtmFeatureP = ftableAddrP(dtmP,feat) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Void  ||
           ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Hole  ||
           ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Island   )
         { *HullLine = 1 ; return(0) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
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
BENTLEYDTM_Public int  bcdtmList_checkForLineOnHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long *hullLineP)
/*
** This Function Tests If The Line P1P2 or P2P1 is On A dtmP,Void,Hole Or Island Hull
*/
{
 int  dbg=DTM_TRACE_VALUE(0) ;
 long clc ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line %9ld %9ld",P1,P2) ;
/*
** Initiliase
*/
 *hullLineP = 0 ;
/*
** Test For Tin Hull
*/
 if( nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 ) *hullLineP = 1 ;
/*
** Test For P1-P2 on a Void,Hole or Island Hull
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr && ! *hullLineP  )
   {
     if(flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  *hullLineP = 1 ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Test For P2-P1 on a Void,Hole or Island Hull
*/
 clc = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clc != dtmP->nullPtr  && ! *hullLineP  )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  *hullLineP = 1 ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line %9ld %9ld Completed",P1,P2) ;
 return(DTM_SUCCESS) ;
}
