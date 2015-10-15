/*--------------------------------------------------------------------------------------+
|
** Module Code  bcdtmInsert.c
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"

//#pragma optimize( "p", on )
thread_local long numPrecisionError = 0, numSnapFix = 0; // These are only used in Debug code.

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_addPointToDtmObject(BC_DTM_OBJ *dtmP,double Xp,double Yp,double Zp,long *pntNumP)
/*
** This Function Adds a Point To A DTM Object In Tin State
*/
{
 DTM_TIN_POINT *pntP ;
 DTM_TIN_NODE  *nodeP ;
/*
** Initialise
*/
 *pntNumP = dtmP->nullPnt;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Test For Memory Allocation
*/
    if( dtmP->numPoints == dtmP->memPoints )
      {
       if( bcdtmObject_incrementTinMemoryDtmObject(dtmP) )  goto errexit ;
      }
/*
**  Set Memory Reference
*/
    pntP  = pointAddrP(dtmP,dtmP->numPoints) ;
    nodeP = nodeAddrP(dtmP,dtmP->numPoints)  ;
/*
**  Add Point
*/
    pntP->x = Xp  ;
    pntP->y = Yp  ;
    pntP->z = Zp  ;
    nodeP->PRGN = 0  ;
    nodeP->PCWD = 0  ;
    nodeP->hPtr = dtmP->nullPnt ;
    nodeP->tPtr = dtmP->nullPnt ;
    nodeP->sPtr = dtmP->nullPnt ;
    nodeP->fPtr = dtmP->nullPtr ;
    nodeP->cPtr = dtmP->nullPtr ;
    *pntNumP = dtmP->numPoints  ;
    bcdtmFlag_setInsertPoint(dtmP,*pntNumP) ;
    ++dtmP->numPoints ;
/*
** Set Bounding Cube
*/
    if( Xp < dtmP->xMin ) dtmP->xMin = Xp ;
    if( Xp > dtmP->xMax ) dtmP->xMax = Xp ;
    if( Yp < dtmP->yMin ) dtmP->yMin = Yp ;
    if( Yp > dtmP->yMax ) dtmP->yMax = Yp ;
    if( Zp < dtmP->zMin ) dtmP->zMin = Zp ;
    if( Zp > dtmP->zMax ) dtmP->zMax = Zp ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}

int bcdtmInsert_addPointAndFixFeaturesToDtmObject (BC_DTM_OBJ* dtmP, long firstPnt, long p1, long p2, long p3, int bkp, double intPntX, double intPntY, double intPntZ, long insertLine, long* p4)
    {
    int    ret=DTM_SUCCESS,dbg=0 ;
    long   voidLine=0;

    if( bcdtmInsert_addPointToDtmObject(dtmP,intPntX,intPntY,intPntZ,p4) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"p4 = %8ld ** %12.5lf %12.5lf %10.4lf",p4,intPntX,intPntY,intPntZ) ;
    /*
    **    Check For Void Line
    */
    bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"voidLine = %2ld",voidLine) ;
    if( voidLine ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,*p4)->PCWD) ;
    /*
    **     Update Clist Structure
    */
    if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2) )
        goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,firstPnt,*p4,p1))
        goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*p4,firstPnt,dtmP->nullPnt))
        goto errexit ;
    if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,*p4,firstPnt))
        goto errexit ;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*p4,p1,firstPnt) )
        goto errexit;
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,*p4,firstPnt) )
        goto errexit;
    if( bcdtmList_insertLineBeforePointDtmObject(dtmP,*p4,p2,firstPnt))
        goto errexit ;
    if( p3 != dtmP->nullPnt )
        {
        if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,*p4,p2))
            goto errexit ;
        if( bcdtmList_insertLineAfterPointDtmObject(dtmP,*p4,p3,p1))
            goto errexit ;
        }
    /*
    **    If Intersecting Tin Hull Update Hull Pointers
    */
    if( bkp == 3 )
        {
        if(nodeAddrP(dtmP,p1)->hPtr == p2 ) { nodeAddrP(dtmP,p1)->hPtr = *p4 ;nodeAddrP(dtmP,*p4)->hPtr = p2 ; }
        if(nodeAddrP(dtmP,p2)->hPtr == p1 ) { nodeAddrP(dtmP,p2)->hPtr = *p4 ;nodeAddrP(dtmP,*p4)->hPtr = p1 ; }
        }
    /*
    **     Set Flag Byte For Intersect Point
    */
    bcdtmFlag_setFlag((unsigned char *)&nodeAddrP(dtmP,*p4)->PCWD,2) ; /* PCWD Bit 3 */
    /*
    **     If Intersecting Inserted Line Feature Update Feature List Structure
    */
    if( insertLine )
        if( bcdtmInsert_pointIntoAllDtmFeaturesWithPntTypeDtmObject(dtmP,p1,p2,*p4, 2))
        goto errexit ;
        /*
        ** Clean Up
        */
cleanup :
        /*
        ** Job Completed
        */
        //if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points %8ld %8ld Completed",startPnt,lastPnt) ;
        //if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points %8ld %8ld Completed",startPnt,lastPnt) ;
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
BENTLEYDTM_Public int bcdtmInsert_lineBetweenPointsDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object        */
 long       firstPnt,            /* ==> First Tin Point              */
 long       lastPnt,             /* ==> Last  Tin Point              */
 long       drapeOption,         /* ==> Drape Option                 */
 long       insertOption         /* ==> InsertOption                 */
 )
 /*
 ** This Function Inserts A Line Between Two Points In Dtm Object
 **
 ** drapeOption  = 1   Insert As Drape Line
**              = 2   Insert As Break Line
 ** insertOption = 1   Move Tin Lines That Are Not Linear Features
 **              = 2   Intersect Tin Lines
**
 ** RobC  -  Rob Cormack -  rob.cormack@bentley.com
 **
 ** Modified 03/05/2007  To Take Account Of Latest Precision Fixing Algorithm
 **
 ** If insertOption == 1  Will Firstly Try To Swap Tin Lines That Are Not Linear Features
 */
    {
 int    ret=DTM_SUCCESS,bkp,dbg=DTM_TRACE_VALUE(0) ;
    long   p1=0,p2,p3,p4,startPnt,endPnt,insertLine,voidLine=0,fixType,precisionError ;
    double intPntX,intPntY,intPntZ=0.0 ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Inserting Line Between Points") ;
        bcdtmWrite_message(0,0,0,"firstPnt     = %8ld",firstPnt) ;
        bcdtmWrite_message(0,0,0,"lastPnt      = %8ld",lastPnt) ;
        bcdtmWrite_message(0,0,0,"drapeOption  = %8ld",drapeOption) ;
        bcdtmWrite_message(0,0,0,"insertOption = %8ld",insertOption) ;
        }
    /*
    ** Initialise
    */
 p1 = dtmP->nullPnt ;
    p2 = dtmP->nullPnt ;
    p3 = dtmP->nullPnt ;
    startPnt = firstPnt ;
    endPnt   = lastPnt ;
    /*
    ** Write additional diagnostics
    */
    if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"startPnt = %6ld tPtr = %9ld ** %12.5lf %12.5lf",startPnt,nodeAddrP(dtmP,startPnt)->tPtr,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y) ;
    bcdtmWrite_message(0,0,0,"endPnt   = %6ld tPtr = %9ld ** %12.5lf %12.5lf",endPnt,nodeAddrP(dtmP,endPnt)->tPtr,pointAddrP(dtmP,endPnt)->x,pointAddrP(dtmP,endPnt)->y) ;
        if(   bcdtmList_testLineDtmObject(dtmP,firstPnt,lastPnt)) bcdtmWrite_message(0,0,0,"Points %6ld %6ld Connected",firstPnt,lastPnt) ;
        else                                                      bcdtmWrite_message(0,0,0,"Points %6ld %6ld Not Connected",firstPnt,lastPnt) ;
        bcdtmList_writeCircularListForPointDtmObject(dtmP,firstPnt) ;
        bcdtmList_writeCircularListForPointDtmObject(dtmP,lastPnt) ;
        if(nodeAddrP(dtmP,firstPnt)->tPtr != dtmP->nullPnt ) bcdtmList_writeCircularListForPointDtmObject(dtmP,nodeAddrP(dtmP,firstPnt)->tPtr) ;
        if(nodeAddrP(dtmP,endPnt)->tPtr   != dtmP->nullPnt ) bcdtmList_writeCircularListForPointDtmObject(dtmP,nodeAddrP(dtmP,endPnt)->tPtr) ;
        }
    /*
** Insert And Swap Tin Lines
    */
    if( insertOption == 1 )
        {
    if( dbg ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines") ;
        if( bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject(dtmP,firstPnt,lastPnt)) // Was bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject
      {
            bcdtmWrite_message(1,0,0,"Error Swapping Lines firstPnt = %6ld lastPnt = %6ld",firstPnt,lastPnt) ;
       return(0) ;
            }
   }
    /*
    ** Insert Line Into Tin
    */
    while ( firstPnt != lastPnt )
        {
        /*
        **   Check For Start Knot If So Return
        */
    if( nodeAddrP(dtmP,firstPnt)->tPtr != dtmP->nullPnt )
            {
            if( dbg == 1 )
         {
                bcdtmWrite_message(0,0,0,"Start Knot Detected") ;
          bcdtmWrite_message(0,0,0,"firstPnt = %6ld hPtr = %9ld tPtr = %9ld ** %10.4lf %10.4lf %10.4lf",firstPnt,nodeAddrP(dtmP,firstPnt)->hPtr,nodeAddrP(dtmP,firstPnt)->tPtr,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,firstPnt)->z) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %6ld hPtr = %9ld tPtr = %9ld ** %10.4lf %10.4lf %10.4lf",lastPnt,nodeAddrP(dtmP,endPnt)->hPtr,nodeAddrP(dtmP,endPnt)->tPtr,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,lastPnt)->z) ;
          bcdtmWrite_message(0,0,0,"Insert Line startPnt = %6ld tPtr = %9ld ** %12.5lf %12.5lf",startPnt,nodeAddrP(dtmP,startPnt)->tPtr,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y) ;
         }
            return(10) ;
            }
        /*
        **  Get Next Intersect Point
        */
    if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %8ld lastPnt = %8ld angle = %18.16lf distance = %12.5lf",firstPnt,lastPnt,bcdtmMath_getPointAngleDtmObject(dtmP,firstPnt,lastPnt),bcdtmMath_pointDistanceDtmObject(dtmP,firstPnt,lastPnt));
        bkp = bcdtmInsert_getIntersectPointDtmObject(dtmP,firstPnt,lastPnt,p3,&insertLine,&p1,&p2,&p3,&intPntX,&intPntY) ;

        if( dbg ) bcdtmWrite_message(0,0,0,"bkp = %6ld P1 = %6ld P2 = %9ld P3 = %9ld",bkp,p1,p2,p3)  ;
        /*
        **  Check For System Error
        */
        if ( bkp == 0 ) goto errexit ;
        /*
        **  Check For Intersect Knot
        */
    if( bkp == 8 )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Knot Detected") ;
            return(12) ;
            }
        /*
        **  Check For Intersect Precision Problem With Internal Line
        */
        if( bkp == 2 )
      {
            if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,firstPnt,p1,p3,p2,intPntX,intPntY,&precisionError)) goto errexit ;
       if( precisionError )
                {
                ++numPrecisionError ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Detected While Inserting Line") ;
                if( bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject(dtmP,firstPnt,p1,p3,p2,intPntX,intPntY,&intPntX,&intPntY,&fixType)) goto errexit ;
                if( fixType == 0 ) goto errexit ;
                else
                    {
             if( fixType == 1 )   bkp = 1 ;
                    if( fixType == 2 ) { bkp = 1 ; p1 = p2 ; }
                    }
                }
            }
        /*
**  Get Z Value Of Intersect Point
        */
        if( bkp == 1 || bkp == 2 || bkp == 3 )
            {
       if( bkp == 1 ) { intPntX = pointAddrP(dtmP,p1)->x ; intPntY = pointAddrP(dtmP,p1)->y ; }
            if( drapeOption == 1 ) bcdtmInsert_getZvalueDtmObject(dtmP,p1,p2,intPntX,intPntY,&intPntZ) ;
            else                   bcdtmInsert_getZvalueDtmObject(dtmP,startPnt,endPnt,intPntX,intPntY,&intPntZ) ;
            }
        /*
        **  Passes Through Tin Point
*/
    if( bkp == 1 )
      {
       nodeAddrP(dtmP,firstPnt)->tPtr = p1 ;
       pointAddrP(dtmP,p1)->z = intPntZ ;
       firstPnt = p1 ;
            }
        /*
        **  Intersects Internal Line
        */
        else if( bkp == 2 || bkp == 3 )
            {
            /*
            **     Check For Knot If So Return
            */
       if( nodeAddrP(dtmP,p1)->tPtr == p2 || nodeAddrP(dtmP,p2)->tPtr == p1  )
         {
                bcdtmWrite_message(0,0,0,"Knot Detected p1 = %6ld p2 = %6ld",p1,p2) ;
          bcdtmWrite_message(0,0,0,"p1 = %6ld tPtr = %6ld ** %10.4lf %10.4lf %10.4lf",p1,nodeAddrP(dtmP,p1)->tPtr,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          bcdtmWrite_message(0,0,0,"p2 = %6ld tPtr = %6ld ** %10.4lf %10.4lf %10.4lf",p2,nodeAddrP(dtmP,p2)->tPtr,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
                return(2) ;
         }
            /*
            **    Add Point To Tin
            */
            if (bcdtmInsert_addPointAndFixFeaturesToDtmObject(dtmP, firstPnt, p1, p2, p3, bkp, intPntX, intPntY, intPntZ, insertLine, &p4) ) goto errexit;
            /*
            **     Update Temporary Pointer  Array
            */
       nodeAddrP(dtmP,firstPnt)->tPtr = p4 ;
       firstPnt = p4 ;
            }
        }
        /*
        ** Clean Up
        */
cleanup :
        /*
        ** Job Completed
        */
        if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points %8ld %8ld Completed",startPnt,lastPnt) ;
        if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points %8ld %8ld Completed",startPnt,lastPnt) ;
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
BENTLEYDTM_Public int bcdtmInsert_getIntersectPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   firstPnt,
 long   lastPnt,
 long   indexPnt,
 long   *insertLineP,
 long   *pnt1P,
 long   *pnt2P,
 long   *pnt3P,
 double *intPntXP,
 double *intPntYP
)
/*
** This Function Gets The Next Line Intercept with firstPnt-lastPnt
**
** Return Values  == 0 Error Terminate
**                == 1 Insert Line Intecepts Point pnt1P
**                == 2 Insert Line Intercepts Internal Line pnt1P-pnt2P
**                == 3 Insert Line Intercepts Hull Line pnt1P-pnt2P
**                == 6 Point Merged Continue Processing
**                == 8 Knot Will Be Inserted In Tptr Array
**
*/
{
 int    dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1=0,p2=0,p3=0,sp1,sp2,clPtr,sd1,sd2,cont=1,internalLine=0,onLine1,onLine2,onPoint1,onPoint2,status ;
 long   hullPnt,hullPnt1,hullPnt2,firstPtr,createKnot ;
 DTMDirection direction;
 double d1,d2,df,n1,n2,firstPntArea,hullPntArea ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Intersect Point") ;
    bcdtmWrite_message(0,0,0,"firstPnt = %8ld firstPnt->hPtr = %9ld",firstPnt,nodeAddrP(dtmP,firstPnt)->hPtr) ;
    bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->hPtr  = %9ld",lastPnt,nodeAddrP(dtmP,lastPnt)->hPtr) ;
    bcdtmWrite_message(0,0,0,"indexPnt = %8ld",indexPnt) ;
    bcdtmWrite_message(0,0,0,"angle firstPnt-lastPnt = %16.14lf",bcdtmMath_getPointAngleDtmObject(dtmP,firstPnt,lastPnt)) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,firstPnt) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,lastPnt) ;
   }
/*
** Initialise Variables
*/
 *intPntXP = *intPntYP = 0.0 ;
 sp1 = sp2 = dtmP->nullPnt ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
/*
** Check Tin Precision
*/
 if( cdbg )
   {
    if( bcdtmCheck_precisionDtmObject(dtmP,0) )
      {
       bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
       return(0) ;
      }
   }
/*
**  Test If firstPnt and lastPnt Connected
*/
 if( bcdtmList_testLineDtmObject(dtmP,firstPnt,lastPnt)) { *pnt1P = lastPnt ; return(1) ; }
/*
** If Index Point Is Null Scan Cyclic List
*/
 if( indexPnt == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning List For Points") ;
    clPtr = nodeAddrP(dtmP,firstPnt)->cPtr  ;
    p2  = clistAddrP(dtmP,clPtr)->pntNum ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,p2)) < 0 ) return(0) ;
    while ( clPtr  != dtmP->nullPtr && cont )
      {
       p2  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if( nodeAddrP(dtmP,firstPnt)->hPtr == p1) { sp1 = p1 ; sp2 = p2 ; }
       else
         {
          if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,firstPnt) < 0 )
            {
             if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,lastPnt) > 0 )
               {
                sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p1) ;
                if( sd1 == 0 ) { *pnt1P = p1 ; return(1) ; }
                sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p2) ;
                if( sd2 == 0 ) { *pnt1P = p2 ; return(1) ; }
                if( sd1 == 1 && sd2 == -1 ) { internalLine = 1 ; cont = 0 ; }
               }
            }
         }
       if( cont ) { p1  = p2 ; p2 = dtmP->nullPnt ; }
      }
   }
/*
** Look At Points Either Side Of Index Point
*/
 if( indexPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Points Either Side Of Index Points") ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,indexPnt))   < 0 ) return(0) ;
    if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,firstPnt,indexPnt)) < 0 ) return(0) ;
    sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,indexPnt) ;
    if( sd1 ==  0 ) { *pnt1P = indexPnt ; return(1) ; }
    if( sd1 == -1 )  p2 = indexPnt ;
    if( sd1 ==  1 )  p1 = indexPnt ;
    internalLine = 1 ;
   }
/*
** Insert Line Internal To Tin Hull
*/
 if( internalLine )
   {
/*
** Write Insert Line Ine Internal Or External to Tin Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Insert Line Is Internal To Tin Hull") ;
/*
**  Test If pnt1P-pnt2P In Tptr List
*/
    if( nodeAddrP(dtmP,p1)->tPtr == p2 || nodeAddrP(dtmP,p2)->tPtr == p1 ) return(8) ;
/*
**  Test If pnt1P-pnt2P is An Inserted Line
*/
    *insertLineP = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ;
    if( dbg &&   *insertLineP ) bcdtmWrite_message(0,0,0,"pnt1P pnt2P Insert Line") ;
    if( dbg && ! *insertLineP ) bcdtmWrite_message(0,0,0,"pnt1P pnt2P Not Insert Line") ;
/*
** Test Point To Line Tolerance Of Insert Line with pnt1P && pnt2P
*/
    n1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,intPntXP,intPntYP) ;
    n2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,intPntXP,intPntYP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"l1 = %2ld n1 = %15.10lf l2 = %2ld n2 = %15.10lf dtmP->ppTol = %15.10lf dtmP->plTol = %15.10lf",onLine1,n1,onLine2,n2,dtmP->ppTol,dtmP->plTol) ;
    if( n1 >= dtmP->plTol || nodeAddrP(dtmP,p1)->tPtr != dtmP->nullPnt ) onLine1 = 0 ;
    if( n2 >= dtmP->plTol || nodeAddrP(dtmP,p2)->tPtr != dtmP->nullPnt ) onLine2 = 0 ;
    if     ( onLine1 && onLine2 )
      {
       if( n1 <= n2  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
       else            { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
      }
    else if( onLine1  )    { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
    else if( onLine2  )    { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
/*
**  Get Point On Opposite to firstPnt of pnt1Ppnt2P
*/
    if( (p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) return(0) ;
    if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3)) p3 = dtmP->nullPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"p3 = %6ld p3->hPtr = %9ld p3->tPtr = %9ld",p3,nodeAddrP(dtmP,p3)->hPtr,nodeAddrP(dtmP,p3)->tPtr) ;
/*
**  Intersect Insert Line And p1-p2
*/
    bcdtmInsert_normalIntersectInsertLineDtmObject(dtmP,firstPnt,lastPnt,p1,p2,intPntXP,intPntYP) ;
/*
**  Test For intPntXP,intPntYP ==  p1 or == p2
*/
    if( pointAddrP(dtmP,p1)->x == *intPntXP && pointAddrP(dtmP,p1)->y == *intPntYP ) { *pnt1P = p1 ; return(1) ; }
    if( pointAddrP(dtmP,p2)->x == *intPntXP && pointAddrP(dtmP,p2)->y == *intPntYP ) { *pnt1P = p2 ; return(1) ; }
/*
** Test Point To Point Tolerance Of intPntXP-intPntYP with pnt1P && pnt2P
*/
    onPoint1 = 0 ;
    onPoint2 = 0 ;
    d1  = bcdtmMath_distance(*intPntXP,*intPntYP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
    d2  = bcdtmMath_distance(*intPntXP,*intPntYP,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
    if( d1 < dtmP->ppTol ) onPoint1 = 1 ;
    if( d2 < dtmP->ppTol ) onPoint2 = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onPoint1 = %2ld d1 = %15.10lf ** onPoint2 = %2ld d2 = %15.10lf ** dtmP->ppTol = %20.15lf",onPoint1,d1,onPoint2,d2,dtmP->ppTol) ;
    if( onPoint1 && onPoint2 )
      {
       if( d1 <= d2  )   { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
       else              { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
      }
    else if( onPoint1  ) { *pnt1P = p1 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
    else if( onPoint2  ) { *pnt1P = p2 ; *pnt2P = *pnt3P = dtmP->nullPnt ; return(1) ; }
/*
**  Test Point To Point Tolerance Of XiYi with Fp
*/
    df  = bcdtmMath_distance(*intPntXP,*intPntYP,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"df = %15.10lf dtmP->ppTol = %15.10lf",df,dtmP->ppTol) ;
    if( df < dtmP->ppTol )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"df = %15.10lf dtmP->ppTol = %15.10lf",df,dtmP->ppTol) ;
       if( ( status = bcdtmInsert_checkPointCanBeMovedOnToLineDtmObject(dtmP,firstPnt,p2,p1,*intPntXP,*intPntYP)) == 1 ) return(0) ;
       if( ! status )
         {
/*
**        Check If Moving Point Will Cause Knot In Current Feature
*/
          if( nodeAddrP(dtmP,p1)->tPtr == p2 ) return(8) ;
          if( nodeAddrP(dtmP,p2)->tPtr == p1 ) return(8) ;
/*
**        Check If Moving Point Will Cause A Knot On An Existing Stored Feature
*/
          if(  bcdtmInsert_checkForCreationOfKnotInExistingFeatureDtmObject(dtmP,p1,p2,firstPnt,&createKnot)) return(0) ;
          if( createKnot )
            {
             *pnt1P = p1 ; *pnt2P = p2 ; *pnt3P = p3 ; return(2) ;
            }
/*
**        Move Point Onto Line
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Moving First Point OnTo P1 P2") ;
          if( *insertLineP )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Checking And Fixing Insert Lines") ;
             if( bcdtmInsert_checkAndFixInsertLinesDtmObject(dtmP,firstPnt,p1,p2)) return(0) ;
             *insertLineP = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ;
             if( *insertLineP ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p1,p2,firstPnt)) return(0) ;
            }
          pointAddrP(dtmP,firstPnt)->x = *intPntXP ;
          pointAddrP(dtmP,firstPnt)->y = *intPntYP ;
          if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) return(0) ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,firstPnt,p3,p1)) return(0) ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,firstPnt,p2)) return(0) ;
          *pnt3P = p3 ;
          return(6) ;
         }
       else
         {
          if( nodeAddrP(dtmP,firstPnt)->fPtr != dtmP->nullPtr || bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,p3,p1) <= 0 || bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,p3,p2) >= 0 )
            {
             if( d1 <= d2 ) { *pnt1P = p1 ; return(1) ; }
             else           { *pnt1P = p2 ; return(1) ; }
            }
          else
            {
             if( *insertLineP )
               {
                if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p1,p2,firstPnt)) return(0) ;
               }
             if( nodeAddrP(dtmP,p1)->tPtr == p2 ) { nodeAddrP(dtmP,p1)->tPtr = firstPnt ; nodeAddrP(dtmP,firstPnt)->tPtr = p2 ; }
             if( nodeAddrP(dtmP,p2)->tPtr == p1 ) { nodeAddrP(dtmP,p2)->tPtr = firstPnt ; nodeAddrP(dtmP,firstPnt)->tPtr = p1 ; }
             if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) return(0) ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,firstPnt,p3,p1)) return(0) ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,firstPnt,p2)) return(0) ;
             *pnt3P = p3 ;
             return(6) ;
            }
         }
     }
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
    p1 = nodeAddrP(dtmP,firstPnt)->hPtr ;
    if(p1 == dtmP->nullPnt)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Point isn't a hull point");
        return 0;
        }
    if( (p2 = bcdtmList_nextClkDtmObject(dtmP,firstPnt,p1)) < 0 ) return(0) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Insert Line Is External To Tin Hull") ;
       bcdtmWrite_message(0,0,0,"firstPnt = %8ld firstPnt->hPtr = %9ld",firstPnt,nodeAddrP(dtmP,firstPnt)->hPtr) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->hPtr  = %9ld",lastPnt,nodeAddrP(dtmP,lastPnt)->hPtr) ;
       bcdtmWrite_message(0,0,0,"p1 = %8ld p2 = %8ld",p1,p2) ;
      }
    if( nodeAddrP(dtmP,lastPnt)->hPtr != dtmP->nullPnt ) hullPnt = lastPnt ;
    else
      {
       bcdtmInsert_findClosestLineInterceptWithHullDtmObject(dtmP,firstPnt,lastPnt,&hullPnt1,&hullPnt2) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt1 = %9ld hullPnt2 = %9ld",hullPnt1,hullPnt2) ;
       if( hullPnt1 == p1 ) { *pnt1P = hullPnt1 ; return(1) ; }
       if( hullPnt2 == p2 ) { *pnt1P = hullPnt2 ; return(1) ; }
       hullPnt = hullPnt1 ;
      }
    if (hullPnt == dtmP->nullPnt) return (0);
    firstPtr = nodeAddrP(dtmP,firstPnt)->hPtr ;
    nodeAddrP(dtmP,firstPnt)->hPtr = hullPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,firstPnt,&firstPntArea,&direction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt ** Area = %20.15lf direction = %1ld",firstPntArea,direction) ;
    nodeAddrP(dtmP,firstPnt)->hPtr = firstPtr ;

    firstPtr = nodeAddrP(dtmP,hullPnt)->hPtr ;
    nodeAddrP(dtmP,hullPnt)->hPtr = firstPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,hullPnt,&hullPntArea,&direction) ;
    nodeAddrP(dtmP,hullPnt)->hPtr = firstPtr ;

    if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt  ** Area = %20.15lf direction = %1ld",hullPntArea,direction) ;
    if( firstPntArea <= hullPntArea ) *pnt1P = p2 ;
    else                              *pnt1P = p1 ;
    return(1) ;
   }
/*
** Job Completed
*/
 return(0) ;
}

struct SwapLines
    {
    long P2;
    long P3;
    enum
        {
        Unknown,
        CantSwap,
        Done
        } state;
    SwapLines(long p2, long p3) : P2(p2), P3(p3)
        {
        state = Unknown;
        }
    };


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       firstPnt,
 long       lastPnt,
 bvector<SwapLines>& crossingLines
 )
/*
** This Function Scan From firstPnt To lastPnt And Swaps Lines That Intersect line firstPntlastPnt
*/
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long   L1,L2;
    double d1,d2,X1,X2,Y1,Y2;
    DTM_TIN_POINT* sPt;
    DTM_TIN_POINT* ePt;
    enum {
        NotRecrossing,
        All,
        OnlyCantSwap,
        Failed
        } mode = NotRecrossing;

    int numLeftToSwap = (int)crossingLines.size();
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Swapping tin lines that intersects %6ld %6ld",firstPnt,lastPnt) ;
        bcdtmWrite_message(0,0,0,"Number of crossing lines %d",numLeftToSwap) ;
        // Add More.
        }

    if (numLeftToSwap == 0)
        return ret;
    sPt = pointAddrP(dtmP,firstPnt);
    ePt = pointAddrP(dtmP,lastPnt);
    /*
    ** Write Entry Message
    */
    while (numLeftToSwap && mode != Failed)
        {
        bool hasSwapped = false;
        bool hasSwappedACantSwap = false;

        if (dbg) bcdtmWrite_message(0,0,0,"Starting loop mode = %d",mode) ;

        // First Pass.
        for (SwapLines& it : crossingLines)
            {
            if (it.state != SwapLines::Done && (mode != OnlyCantSwap || it.state == SwapLines::CantSwap))
                {
                long P2 = it.P2;
                long P3 = it.P3;
                long P1 =  bcdtmList_nextAntDtmObject(dtmP,P3,P2);
                long P4 =  bcdtmList_nextClkDtmObject(dtmP,P3,P2);
                int sd1 = bcdtmMath_pointSideOfDtmObject (dtmP, P1, P4, P2);
                int sd2 = bcdtmMath_pointSideOfDtmObject (dtmP, P1, P4, P3);

                //        if( dbg ) bcdtmWrite_message(0,0,0,"00 sd1 = %2ld sd2 = %2ld",sd1,sd2) ;
                /*
                **     Check Line Is Not Closer Than plTol
                */
                d1  = bcdtmMath_distanceOfPointFromLine (&L1,sPt->x,sPt->y,ePt->x,ePt->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,&X1,&Y1) ;
                d2  = bcdtmMath_distanceOfPointFromLine (&L2,sPt->x,sPt->y,ePt->x,ePt->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,&X2,&Y2) ;

                if     ( pointAddrP(dtmP,P2)->x == X1 && pointAddrP(dtmP,P2)->y == Y1 ) sd1 = 0 ;
                else if( pointAddrP(dtmP,P3)->x == X2 && pointAddrP(dtmP,P3)->y == Y2 ) sd2 = 0 ;
                else if( d1 < d2 && d1 < dtmP->plTol ) sd1 = 0 ;
                else if( d2 < d1 && d2 < dtmP->plTol ) sd2 = 0 ;

                /*
                **     Swap Line
                */
                if( dbg ) bcdtmWrite_message(0,0,0,"01 sd1 = %2ld sd2 = %2ld",sd1,sd2) ;

                // For the first two passes done swap links that would still cross the line.
                if( sd1 >  0 && sd2 < 0)
                    {
                    int sd3 = (P1 == firstPnt) || mode == All ? 0 : bcdtmMath_pointSideOfDtmObject (dtmP, firstPnt, lastPnt, P1);
                    int sd4 = (P4 == lastPnt) || mode == All ? 0 : bcdtmMath_pointSideOfDtmObject (dtmP, firstPnt, lastPnt, P4);

                    if (sd3 == 0 || sd4 == 0 || sd3 == sd4 || mode != All)
                        {
                        if( dbg ) bcdtmWrite_message(0,0,0,"Swapping %10ld %10ld With %10ld %10ld",P2,P3,P1,P4) ;
                        if( bcdtmList_deleteLineDtmObject(dtmP,P2,P3) ) goto errexit ;
                        if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,P4,P2) ) goto errexit ;
                        if( bcdtmList_insertLineAfterPointDtmObject(dtmP,P4,P1,P3) ) goto errexit ;

                        if (it.state == SwapLines::CantSwap)
                            {
                            hasSwappedACantSwap = true;
                            it.state = SwapLines::Unknown;
                            }
                        if (sd3 == 0 || sd4 == 0 || sd3 == sd4)
                            {
                            hasSwapped = true;
                            numLeftToSwap--;
                            it.state = SwapLines::Done;
                            }
                        else
                            {
                            // If this still crosses the line update the Points Numbers.
                            it.P2 = P1;
                            it.P3 = P4;
                            }
                        }
                    }
                else
                    {
                    it.state = SwapLines::CantSwap;
                    if (sd1 == 0 || sd2 == 0)
                        {
                        if (dbg) bcdtmWrite_message(0,0,0,"Point found on this so ignore this link.") ;
                        numLeftToSwap--;
                        it.state = SwapLines::Done;
                        }
                    }
                }
            }
        if (dbg) bcdtmWrite_message(0,0,0,"Finished loop mode = %d hasSwapped %d hasSwappedACantSwap",mode, hasSwapped, hasSwappedACantSwap) ;

        switch (mode)
            {
            case NotRecrossing:
                if (!hasSwapped)
                    mode = All;
                break;
            case All:
                if (!hasSwappedACantSwap)
                    mode = OnlyCantSwap;
                break;
            case OnlyCantSwap:
                if (!hasSwappedACantSwap)
                    {
                    if (numLeftToSwap)
                        mode = Failed;
                    }
                else
                    mode = All;
                break;
            }
        }
    if (dbg && mode == Failed)
        bcdtmWrite_message(0,0,0,"Failed to move out all crossing features") ;


    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    crossingLines.clear();
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines Between Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines Between Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject
(
BC_DTM_OBJ *dtmP,
long       firstPnt,
long       lastPnt,
bool allowAdd
)
/*
** This Function Scan From firstPnt To lastPnt And Swaps Lines That Intersect line firstPntlastPnt
*/
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    int    sd1, sd2, sd3;
    long   startPnt, P1, P2, P3, P4;
    //long loop=0 ;
    double X, Y;
    DTM_TIN_POINT *firstPt, *lastPt, *P2Pt, *P3Pt;
    double pTolSq = dtmP->ppTol * dtmP->ppTol;
    long prevInsErrorPt = -1, prevInsErrorPt2 = -1, prevInsErrorPt3 = -1;
    bvector <SwapLines> crossingLines;

    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Adding Intersection points between Tin Lines Between %6ld %6ld", firstPnt, lastPnt);
        bcdtmWrite_message (0, 0, 0, "firstPnt = %6ld ** %10.4lf %10.4lf %10.4lf", firstPnt, pointAddrP (dtmP, firstPnt)->x, pointAddrP (dtmP, firstPnt)->y, pointAddrP (dtmP, firstPnt)->z);
        bcdtmWrite_message (0, 0, 0, "lastPnt = %6ld ** %10.4lf %10.4lf %10.4lf", lastPnt, pointAddrP (dtmP, lastPnt)->x, pointAddrP (dtmP, lastPnt)->y, pointAddrP (dtmP, lastPnt)->z);
        bcdtmWrite_message (0, 0, 0, "Angle firstPntlastPnt = %12.10lf", bcdtmMath_getAngle (pointAddrP (dtmP, firstPnt)->x, pointAddrP (dtmP, firstPnt)->y, pointAddrP (dtmP, lastPnt)->x, pointAddrP (dtmP, lastPnt)->y));
        bcdtmList_writeCircularListForPointDtmObject (dtmP, firstPnt);
        bcdtmList_writeCircularListForPointDtmObject (dtmP, lastPnt);
        }
    /*
    ** Get Start Triangle Or Point
    */
    P1 = startPnt = firstPnt;
    if (bcdtmTin_getSwapTriangleDtmObject (dtmP, P1, lastPnt, &P2, &P3, &P4))
        goto errexit;
    if (P2 == dtmP->nullPnt) goto cleanup;
    while (P1 != lastPnt)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "startPnt = %10ld P1 = %10ld P2 = %10ld P3 = %10ld P4 = %10ld", startPnt, P1, P2, P3, P4);
        //++loop ;
        //if( loop > 10000 )
        //    goto cleanup ;
        if (P1 == P4) goto cleanup;
        /*
        **  Line Passes Through A Point
        */
        if (P3 == dtmP->nullPnt)
            {
            bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject (dtmP, startPnt, P1, crossingLines);
            if (dbg) bcdtmWrite_message (0, 0, 0, "startPntlastPnt Passes Through P1");
            startPnt = P1 = P2;
            if (P1 != lastPnt) if (bcdtmTin_getSwapTriangleDtmObject (dtmP, P1, lastPnt, &P2, &P3, &P4))
                goto errexit;
            if (P2 == dtmP->nullPnt) goto cleanup;
            }
        /*
        **  Line Passes Through A Line
        **  Check If Line Can Be Swapped
        */
        else
            {
            /*
            **     Test if edge is a feature Line
            */
            if (bcdtmList_testForDtmFeatureLineDtmObject (dtmP, P2, P3) || nodeAddrP (dtmP, P2)->tPtr == P3 && nodeAddrP (dtmP, P3)->tPtr == P2)
                {
                double Z = 0;
                long newP;
                long bkp = 3;

                if (dbg)
                    bcdtmWrite_message (0, 0, 0, "Adding Intersect for  %10ld %10ld", P2, P3);

                firstPt = pointAddrP (dtmP, firstPnt);
                lastPt = pointAddrP (dtmP, lastPnt);
                P2Pt = pointAddrP (dtmP, P2);
                P3Pt = pointAddrP (dtmP, P3);
                if (bcdtmMath_intersectCordLines (firstPt->x, firstPt->y, lastPt->x, lastPt->y,
                    P2Pt->x, P2Pt->y, P3Pt->x, P3Pt->y, &X, &Y) == 1)
                    {

                    if (dbg)
                        bcdtmWrite_message (0, 0, 0, "Intersection Point %12.4lf %12.4lf %10.4lf", X, Y, Z);

                    bool swapLines = false;
                    bool insError = !allowAdd;

                    if (bcdtmMath_distanceSquared (P2Pt->x, P2Pt->y, X, Y) <= pTolSq)
                        {
                        swapLines = insError = true;
                        if (dbg)
                            bcdtmWrite_message (0, 0, 0, "Point is too close to the P2");
                        }
                    else if (bcdtmMath_distanceSquared (P3Pt->x, P3Pt->y, X, Y) <= pTolSq)
                        {
                        swapLines = insError = true;
                        if (dbg)
                            bcdtmWrite_message (0, 0, 0, "Point is too close to the P3");
                        }
                    else
                        {
                        long onLine;
                        double X, Y;
                        double d1 = bcdtmMath_distanceOfPointFromLine (&onLine, firstPt->x, firstPt->y, lastPt->x, lastPt->y, pointAddrP (dtmP, P2)->x, pointAddrP (dtmP, P2)->y, &X, &Y);
                        if (onLine && d1 < dtmP->plTol)
                            {
                            swapLines = insError = true;
                            if (dbg)
                                bcdtmWrite_message (0, 0, 0, "P1 is close to line.");
                            }
                        else
                            {
                            double d2 = bcdtmMath_distanceOfPointFromLine (&onLine, firstPt->x, firstPt->y, lastPt->x, lastPt->y, pointAddrP (dtmP, P3)->x, pointAddrP (dtmP, P3)->y, &X, &Y);

                            if (onLine && d2 < dtmP->plTol)
                                {
                                swapLines = insError = true;
                                if (dbg)
                                    bcdtmWrite_message (0, 0, 0, "P1 ot P2 is close to line.");
                                }
                            }
                        }

                    if (!insError)
                        {
                        long precisionError;
                        if (bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject (dtmP, P1, P2, P4, P3, X, Y, &precisionError)) goto errexit;

                        if (precisionError)
                            {
                            long fixType;
                            if (dbg) bcdtmWrite_message (0, 0, 0, "Precision Error Detected While Inserting Line");
                            if (bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject (dtmP, P1, P2, P4, P3, X, Y, &X, &Y, &fixType)) goto errexit;
                            insError = true;
                            }
                        }
                    if (!insError)
                        {
                        if (dbg)
                            bcdtmWrite_message (0, 0, 0, "Adding Point %12.4lf %12.4lf %10.4lf", X, Y, Z);

                        bcdtmInsert_getZvalueDtmObject (dtmP, P2, P3, X, Y, &Z);
                        if (bcdtmInsert_addPointAndFixFeaturesToDtmObject (dtmP, P1, P2, P3, P4, bkp, X, Y, Z, TRUE, &newP))
                            goto errexit;

                        bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject (dtmP, startPnt, newP, crossingLines);
                        P1 = startPnt = newP;

                        sd1 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P4);
                        if (sd1 == 0)
                            {
                            startPnt = P1;
                            if (startPnt != lastPnt) if (bcdtmTin_getSwapTriangleDtmObject (dtmP, P1, lastPnt, &P2, &P3, &P4)) goto errexit;
                            if (P2 == dtmP->nullPnt)  goto cleanup;
                            }
                        else
                            {
                            if (sd1 > 0) { P2 = P4; }
                            else if (sd1 < 0) { P3 = P4; }
                            if (!bcdtmList_testLineDtmObject (dtmP, P2, P3)) goto cleanup;
                            if ((P4 = bcdtmList_nextAntDtmObject (dtmP, P2, P3)) < 0)
                                goto errexit;
                            /*
                            **           Check For Precion Problem - P2 And P3 Are On Opposite Side Of startPnt-lastPnt
                            */
                            sd2 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P2);
                            sd3 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P3);
                            if (sd2 == sd3) goto cleanup;
                            }
                        continue;
                        }
                    if (insError && crossingLines.empty())
                        {
                        if (prevInsErrorPt == P1)
                            {
                            if (dbg)
                                bcdtmWrite_message(0, 0, 0, "Failed on the same point.");
                            goto cleanup;
                            }
                        if (prevInsErrorPt2 == P1 && prevInsErrorPt3 == prevInsErrorPt)
                            {
                            if (dbg)
                                bcdtmWrite_message(0, 0, 0, "Failed on the same swapping points.");
                            goto cleanup;
                            }
                        prevInsErrorPt3 = prevInsErrorPt2;
                        prevInsErrorPt2 = prevInsErrorPt;
                        prevInsErrorPt = P1;
                        }
                    if (swapLines)
                        bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject (dtmP, startPnt, P2, crossingLines);
                    }
                else
                    {
                    if (dbg)
                        bcdtmWrite_message (0, 0, 0, "No intersection found");
                    }
                }
            else
                crossingLines.push_back (SwapLines (P2, P3));

            sd1 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P4);
            if (sd1 == 0)
                {
                if (startPnt == P4)
                    {
                    if (dbg)
                        bcdtmWrite_message(0, 0, 0, "Failed same startPnt and lastPnt.");
                    goto cleanup;
                    }
                bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject (dtmP, startPnt, P4, crossingLines);
                startPnt = P1 = P4;
                if (startPnt != lastPnt) if (bcdtmTin_getSwapTriangleDtmObject (dtmP, P1, lastPnt, &P2, &P3, &P4)) goto errexit;
                if (P2 == dtmP->nullPnt) goto cleanup;
                }
            else
                {
                if (sd1 > 0) { P1 = P2; P2 = P4; }
                else if (sd1 < 0) { P1 = P3; P3 = P4; }
                if (!bcdtmList_testLineDtmObject (dtmP, P2, P3)) goto cleanup;
                if ((P4 = bcdtmList_nextAntDtmObject (dtmP, P2, P3)) < 0)
                    goto errexit;
                /*
                **           Check For Precion Problem - P2 And P3 Are On Opposite Side Of startPnt-lastPnt
                */
                sd2 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P2);
                sd3 = bcdtmMath_pointSideOfDtmObject (dtmP, startPnt, lastPnt, P3);
                if (sd2 == sd3) goto cleanup;
                }
            }
        }
    bcdtmInsert_swapTinLinesThatIntersectInsertLineHelperDtmObject (dtmP, startPnt, lastPnt, crossingLines);
    /*
    ** Job Completed
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Checking Tin Structure");
        if (bcdtmCheck_tinComponentDtmObject (dtmP)) bcdtmWrite_message (0, 0, 0, "Tin Structure Corrupted");
        else                                        bcdtmWrite_message (0, 0, 0, "Tin Structure Valid");
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Swapping Tin Lines Between Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Swapping Tin Lines Between Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    ret = DTM_ERROR;
    goto cleanup;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject
    (
    BC_DTM_OBJ *dtmP,
    long       firstPnt,
    long       lastPnt
    )
    {
    return bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject (dtmP, firstPnt, lastPnt, true);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int  bcdtmInsert_normalIntersectInsertLineDtmObject(BC_DTM_OBJ *dtmP,long Fp,long Lp,long P1,long P2,double *Xi,double *Yi)
/*
** This Function Calculates The Insert Line Intersect Point Between Two Lines
*/
{
 double  x1,y1,x2,y2,x3,y3,x4,y4 ;
 double  n1,n2,xmin,ymin,xmax,ymax ;
 DTM_TIN_POINT  *p1P,*p2P,*fpP,*lpP ;
/*
** Get Point Addresses
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 fpP = pointAddrP(dtmP,Fp) ;
 lpP = pointAddrP(dtmP,Lp) ;
/*
** Get Minimum Coordinates
*/
 xmin = xmax = p1P->x ;
 ymin = ymax = p1P->y ;
 if( p2P->x < xmin ) xmin = p2P->x ;
 if( p2P->x > xmax ) xmax = p2P->x ;
 if( p2P->y < ymin ) ymin = p2P->y ;
 if( p2P->y > ymax ) ymax = p2P->y ;
 if( fpP->x < xmin ) xmin = fpP->x ;
 if( fpP->x > xmax ) xmax = fpP->x ;
 if( fpP->y < ymin ) ymin = fpP->y ;
 if( fpP->y > ymax ) ymax = fpP->y ;
 if( lpP->x < xmin ) xmin = lpP->x ;
 if( lpP->x > xmax ) xmax = lpP->x ;
 if( lpP->y < ymin ) ymin = lpP->y ;
 if( lpP->y > ymax ) ymax = lpP->y ;
/*
** Subtract Minimum From Coordinates
*/
 x1 = fpP->x - xmin ;
 y1 = fpP->y - ymin ;
 x2 = lpP->x - xmin ;
 y2 = lpP->y - ymin ;
 x3 = p1P->x - xmin ;
 y3 = p1P->y - ymin ;
 x4 = p2P->x - xmin ;
 y4 = p2P->y - ymin ;
 n1  = bcdtmMath_normalDistanceToCordLine(x1,y1,x2,y2,x3,y3) ;
 n2  = bcdtmMath_normalDistanceToCordLine(x1,y1,x2,y2,x4,y4) ;
 if( n1+n2 != 0.0 )
   {
    *Xi = x3 + ( x4 - x3 ) * n1 / (n1+n2) + xmin ;
    *Yi = y3 + ( y4 - y3 ) * n1 / (n1+n2) + ymin ;
   }
 else
   {
    n1 = bcdtmMath_pointDistanceDtmObject(dtmP,Fp,P1) ;
    n2 = bcdtmMath_pointDistanceDtmObject(dtmP,Fp,P2) ;
    if( n1 <= n2 ) { *Xi = pointAddrP(dtmP,P1)->x ; *Yi = pointAddrP(dtmP,P1)->y ; }
    else           { *Xi = pointAddrP(dtmP,P2)->x ; *Yi = pointAddrP(dtmP,P2)->y ; }
   }
/*
** Adjust Line
*/
 if( *Xi < xmin ) *Xi = xmin ;
 if( *Xi > xmax ) *Xi = xmax ;
 if( *Yi < ymin ) *Yi = ymin ;
 if( *Yi > ymax ) *Yi = ymax ;
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
BENTLEYDTM_Public int bcdtmInsert_findClosestLineInterceptWithHullDtmObject(BC_DTM_OBJ *dtmP,long firstPnt,long lastPnt,long *p1P,long *p2P)
/*
** This Routine Find the Closeset Hull Line Intercept to
**
** Return Values == 0  Success
**               == 1  Error Terminate
**
** Ptype == 0  No Intercept
**       == 1  Intesects With Hull Point p1P
**       == 2  Intesects With Hull Line  p1P-p2P
**       == 3  Pulled Onto Next Hull Point
**
*/
{
 long   p1,p2,sd1,sd2,isw=0 ;
 double Xs,Ys,Xe,Ye,xc,yc,dist,mindist=0.0,xmin,ymin,xmax,ymax,xlmin,ylmin,xlmax,ylmax ;
/*
** Initialiase
*/
 *p1P = *p2P = dtmP->nullPnt ;
 Xs  = pointAddrP(dtmP,firstPnt)->x ;
 Ys  = pointAddrP(dtmP,firstPnt)->y ;
 Xe  = pointAddrP(dtmP,lastPnt)->x ;
 Ye  = pointAddrP(dtmP,lastPnt)->y ;
/*
** Set Scan Parameters
*/
 if( Xs <= Xe ) { xlmin = Xs ; xlmax = Xe ; }
 else           { xlmin = Xe ; xlmax = Xs ; }
 if( Ys <= Ye ) { ylmin = Ys ; ylmax = Ye ; }
 else           { ylmin = Ye ; ylmax = Ys ; }
 xlmin = xlmin - dtmP->ppTol ;
 xlmax = xlmax + dtmP->ppTol ;
 ylmin = ylmin - dtmP->ppTol ;
 ylmax = ylmax + dtmP->ppTol ;
/*
** Scan Hull Looking For Closest Line Intercept
*/
 isw = 1 ;
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    if( p1 != firstPnt && p2 != firstPnt )
      {
       if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p2)->x ) { xmin = pointAddrP(dtmP,p1)->x ; xmax = pointAddrP(dtmP,p2)->x ; }
       else                                                   { xmin = pointAddrP(dtmP,p2)->x ; xmax = pointAddrP(dtmP,p1)->x ; }
       if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p2)->y ) { ymin = pointAddrP(dtmP,p1)->y ; ymax = pointAddrP(dtmP,p2)->y ; }
       else                                                   { ymin = pointAddrP(dtmP,p2)->y ; ymax = pointAddrP(dtmP,p1)->y ; }
       if( xmin <= xlmax && xmax >= xlmin && ymin <= ylmax && ymax >= ylmin )
         {
          sd1 = bcdtmMath_sideOf(Xs,Ys,Xe,Ye,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
          sd2 = bcdtmMath_sideOf(Xs,Ys,Xe,Ye,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
          if( sd1 != sd2 )
            {
             sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,Xs,Ys) ;
             sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,Xe,Ye) ;
             if( sd1 != sd2 )
               {
                bcdtmMath_normalIntersectCordLines(Xs,Ys,Xe,Ye,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&xc,&yc) ;
                dist = bcdtmMath_distance(Xs,Ys,xc,yc) ;
                if( isw || dist < mindist  )
                  {
                   isw = 0 ;
                   mindist = dist ;
                   *p1P = p1 ; *p2P = p2 ;
                  }
               }
            }
         }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
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
BENTLEYDTM_Public int bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,long P4,double x,double y,long *precisionError)
/*
** Check For A Precision Error Occurring If The Point XY Is
** Inserted As A Common Vertex In The Clockwise Quadrilateral P1P2P3P4
*/
{
 DTM_TIN_POINT  *p1P,*p2P,*p3P,*p4P ;
/*
** Get Point Addresses
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p3P = pointAddrP(dtmP,P3) ;
 p4P = pointAddrP(dtmP,P4) ;
/*
** Initialise
*/
 *precisionError = 0 ;
/*
** Check Side Of Conditions
*/
 if     ( bcdtmMath_allSideOf(p1P->x,p1P->y,p2P->x,p2P->y,x,y) >= 0 ) *precisionError = 1 ;
 else if( bcdtmMath_allSideOf(p2P->x,p2P->y,p3P->x,p3P->y,x,y) >= 0 ) *precisionError = 1 ;
 else if( bcdtmMath_allSideOf(p3P->x,p3P->y,p4P->x,p4P->y,x,y) >= 0 ) *precisionError = 1 ;
 else if( bcdtmMath_allSideOf(p4P->x,p4P->y,p1P->x,p1P->y,x,y) >= 0 ) *precisionError = 1 ;
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
BENTLEYDTM_Public int bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject(BC_DTM_OBJ *dtmP,long trgPnt1,long trgPnt2,long trgPnt3,long trgPnt4,double pointX,double pointY,double *fixedXP,double *fixedYP,long *fixTypeP )
/*
** Assumes:-
**
** 1. Quadliteral trgPnt1-trgPnt2-trgPnt3-trgPnt4 Is In A ClockWise Direction
** 2. pointX,pointY is on Line trgPnt2-trgPnt4
**
** This is heuristic function and should be only modified when a new condition arises
** That can not be fixed.
**
** fixTypeP    <==  0   Precision Not Fixed
**             <==  1   Snap To Point trgPnt2
**             <==  2   Snap To Point trgPnt4
**             <==  3   Precision Fixed By Moving Point On line trgPnt2-trgPnt4
**
**
*/
{
 int    ret=DTM_SUCCESS,sdof12,sdof14,sdof32,sdof34,sdof132,sdof134,dbg=DTM_TRACE_VALUE(0);
 long   n,precisionError,pointOnLine,dtmFeatureLine,fixType,loop,numLoop ;
 double dx,dy,d1,d2,d3,d4,d5,xMin,xMax,yMin,yMax,ratio ;
 double angle1,angle12,angle14,angle3,angle32,angle34,x,y ;
 double angle,angleInc,saveX,saveY,factor = 0.0;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Fixing Point Quadrilateral Precision") ;
    bcdtmWrite_message(0,0,0,"pointX  = %20.15lf pointY = %20.15lf",pointX,pointY) ;
    bcdtmWrite_message(0,0,0,"trgPnt1 = %8ld Hptr = %9ld Tptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt1,nodeAddrP(dtmP,trgPnt1)->hPtr,nodeAddrP(dtmP,trgPnt1)->tPtr,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"trgPnt2 = %8ld Hptr = %9ld Tptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt2,nodeAddrP(dtmP,trgPnt2)->hPtr,nodeAddrP(dtmP,trgPnt2)->tPtr,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"trgPnt3 = %8ld Hptr = %9ld Tptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt3,nodeAddrP(dtmP,trgPnt3)->hPtr,nodeAddrP(dtmP,trgPnt3)->tPtr,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt3)->z) ;
    bcdtmWrite_message(0,0,0,"trgPnt4 = %8ld Hptr = %9ld Tptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt4,nodeAddrP(dtmP,trgPnt4)->hPtr,nodeAddrP(dtmP,trgPnt4)->tPtr,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y,pointAddrP(dtmP,trgPnt4)->z) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol  = %20.15lf",dtmP->ppTol)  ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol  = %20.15lf",dtmP->plTol)  ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol = %20.15lf",dtmP->mppTol)  ;
   }
/*
** Initialise
*/
 *fixTypeP = 0 ;
 *fixedXP  = 0.0 ;
 *fixedYP  = 0.0 ;
 saveX     = pointX ;
 saveY     = pointY ;
/*
** Check MPP Tolerance Is Not Zero
*/
 if( dtmP->mppTol == 0.0 )
   {
    bcdtmWrite_message(2,0,0,"Machine Point To Point Tolerance Is Zero For Precision Fix") ;
    goto errexit ;
   }
/*
**  Check Point Is On Line trgPnt2-trgPnt4
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Point Is On Line") ;
 if( pointAddrP(dtmP,trgPnt2)->x <= pointAddrP(dtmP,trgPnt4)->x ) { xMin = pointAddrP(dtmP,trgPnt2)->x ; xMax = pointAddrP(dtmP,trgPnt4)->x ; }
 else                                                             { xMin = pointAddrP(dtmP,trgPnt4)->x ; xMax = pointAddrP(dtmP,trgPnt2)->x ; }
 if( pointAddrP(dtmP,trgPnt2)->y <= pointAddrP(dtmP,trgPnt4)->y ) { yMin = pointAddrP(dtmP,trgPnt2)->y ; yMax = pointAddrP(dtmP,trgPnt4)->y ; }
 else                                                             { yMin = pointAddrP(dtmP,trgPnt4)->y ; yMax = pointAddrP(dtmP,trgPnt2)->y ; }
 pointOnLine = 1 ;
 if( pointX < xMin || pointX > xMax && pointY < yMin || pointY > yMax ) pointOnLine = 0 ;
/*
** Write Point On Line Diagnostics
*/
 if( dbg )
   {
    if( pointOnLine ) bcdtmWrite_message(0,0,0,"Point On Line") ;
    else              bcdtmWrite_message(0,0,0,"Point Not On Line") ;
   }
/*
**  Test If trgPnt2-trgPnt4 Is A Feature Line
*/
 dtmFeatureLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,trgPnt2,trgPnt4) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureLine trgPnt2-trgPnt4 = %2ld",dtmFeatureLine) ;
/*
** Recalculate Point Coordinates On Line P2 P4
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Recalculating Intersect Point Coordinates") ;
    bcdtmWrite_message(0,0,0,"pointX  = %20.15lf pointY = %20.15lf",pointX,pointY) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt1-point-trgPoint2 = %2d[ 1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt2-point-trgPoint4 = %2d[-1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPoint2 = %2d[-1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPoint4 = %2d[ 1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
   }
 d2 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
 d4 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
 ratio = d2 / ( d2 + d4 ) ;
 dx = pointAddrP(dtmP,trgPnt4)->x - pointAddrP(dtmP,trgPnt2)->x ;
 dy = pointAddrP(dtmP,trgPnt4)->y - pointAddrP(dtmP,trgPnt2)->y ;
 pointX = pointAddrP(dtmP,trgPnt2)->x + ratio * dx ;
 pointY = pointAddrP(dtmP,trgPnt2)->y + ratio * dy ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Recalculated Intersect Point Coordinates") ;
    bcdtmWrite_message(0,0,0,"pointX  = %20.15lf pointY = %20.15lf",pointX,pointY) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt1-point-trgPoint2 = %2d[ 1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt1-point-trgPoint4 = %2d[-1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPoint2 = %2d[-1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPoint4 = %2d[ 1]",bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
   }
 if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,pointX,pointY,&precisionError)) goto errexit ;
 if( ! precisionError  )
   {
    *fixTypeP = 3 ;
    *fixedXP = pointX ;
    *fixedYP = pointY ;
   }
/*
** Scan Around Point
*/
 if( ! *fixTypeP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Around Point") ;
    precisionError = TRUE ;
    angleInc = DTM_2PYE / 1000.0 ;
    for( n = 0 ; n < 3 && precisionError == TRUE ; ++n )
      {
       angle  = 0.0 ;
       if     ( n == 0 ) factor =   1.0 ;
       else if( n == 1 ) factor =  10.0 ;
       else if( n == 2 ) factor =  10.5 ;
       while ( angle < DTM_2PYE && precisionError )
         {
          x = pointX + dtmP->mppTol * factor * cos(angle) ;
          y = pointY + dtmP->mppTol * factor * sin(angle) ;
          if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,x,y,&precisionError)) goto errexit ;
          if( precisionError == FALSE )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Precision Fixed During Point Scan ** factor = %8.2lf angle = %18.16lf",factor,angle) ;
             *fixTypeP = 3 ;
             *fixedXP = x ;
             *fixedYP = y ;
            }
          angle = angle + angleInc ;
         }
      }
   }
/*
**  Move Point Towards A Quadrilateral Vertice
*/
 if( ! *fixTypeP )
   {
/*
**  Calculate Precision Varaibles
*/
    d1 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
    d2 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    d3 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) ;
    d4 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
/*
**  Report Precision Varaibles
*/
    if( dbg == 1 )
      {
       d5 = bcdtmMath_distance(pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
/*
**     Calculate Precision Variables
*/
       sdof12  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
       sdof14  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
       sdof32  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
       sdof34  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
       sdof132 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
       sdof134 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
/*
**     Calculate Precision Angles
*/
       angle12 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt1,trgPnt2) ;
       angle1  = bcdtmMath_getAngle(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY) ;
       angle14 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt1,trgPnt4) ;
       angle32 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt3,trgPnt2) ;
       angle3  = bcdtmMath_getAngle(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY) ;
       angle34 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt3,trgPnt4) ;
       if( angle12 < angle14 ) angle12 += DTM_2PYE ;
       if( angle1  < angle14 ) angle1  += DTM_2PYE ;
       if( angle34 < angle32 ) angle34 += DTM_2PYE ;
       if( angle3  < angle32 ) angle3  += DTM_2PYE ;
/*
**     Write Precision Varaibles
*/
       bcdtmWrite_message(0,0,0,"distance  trgPnt1-point   = %20.15lf",d1) ;
       if( d1 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt1-point   == 0.0") ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt2-point   = %20.15lf",d2) ;
       if( d2 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt2-point   == 0.0") ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt3-point   = %20.15lf",d3) ;
       if( d3 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt3-point   == 0.0") ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt4-point   = %20.15lf",d4) ;
       if( d4 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt4-point   == 0.0") ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt2-trgPnt4 = %20.15lf",d5) ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt1-trgPnt2 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt1-trgPnt4 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt3-trgPnt2 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
       bcdtmWrite_message(0,0,0,"distance  trgPnt3-trgPnt4 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
       bcdtmWrite_message(0,0,0,"sideOf12  trgPnt1-point-trgPnt2   = %2d[ 1]",sdof12) ;
       bcdtmWrite_message(0,0,0,"sideOf14  trgPnt1-point-trgPnt4   = %2d[-1]",sdof14) ;
       bcdtmWrite_message(0,0,0,"sideOf32  trgPnt3-point-trgPnt2   = %2d[-1]",sdof32) ;
       bcdtmWrite_message(0,0,0,"sideOf34  trgPnt3-point-trgPnt4   = %2d[ 1]",sdof34) ;
       bcdtmWrite_message(0,0,0,"sideOf132 trgPnt1-trgpnt3-trgPnt2 = %2d ",sdof132) ;
       bcdtmWrite_message(0,0,0,"sideOf134 trgPnt1-trgpnt3-trgPnt4 = %2d ",sdof134) ;
       bcdtmWrite_message(0,0,0,"angle12                           = %17.15lf",angle12) ;
       bcdtmWrite_message(0,0,0,"angle1                            = %17.15lf",angle1) ;
       bcdtmWrite_message(0,0,0,"angle14                           = %17.15lf",angle14) ;
       bcdtmWrite_message(0,0,0,"angle12-angle14                   = %17.15lf",angle12-angle14) ;
       bcdtmWrite_message(0,0,0,"angle32                           = %17.15lf",angle32) ;
       bcdtmWrite_message(0,0,0,"angle3                            = %17.15lf",angle3) ;
       bcdtmWrite_message(0,0,0,"angle34                           = %17.15lf",angle34) ;
       bcdtmWrite_message(0,0,0,"angle34-angle32                   = %17.15lf",angle34-angle32) ;
      }
/*
**  Determine Fix Type
*/
    loop    =    0 ;
    numLoop = 1000 ;
    precisionError = TRUE ;
    if     ( d1 == 0.0 ) fixType = 3 ;
    else if( d3 == 0.0 ) fixType = 1 ;
    else if( d4 <  d2  ) fixType = 2 ;
    else if( d2 <  d4  ) fixType = 4 ;
    else                 fixType = 0 ;
/*
**  Make Precision Fix
*/
    switch( fixType )
      {
/*
**     Move Point Towards trgPnt1
*/
       case  1 :
         if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Moving Point Towards trgpnt1 ") ;
         d5 = d1 ;
         d1 = d1 - dtmP->mppTol  ;
         dx = pointX - pointAddrP(dtmP,trgPnt1)->x ;
         dy = pointY - pointAddrP(dtmP,trgPnt1)->y ;
         while ( d1 > dtmP->mppTol && precisionError  && loop < numLoop )
           {
            ++loop ;
            d1 = d1 - dtmP->mppTol ;
            pointX = pointAddrP(dtmP,trgPnt1)->x + dx * d1 / d5 ;
            pointY = pointAddrP(dtmP,trgPnt1)->y + dy * d1 / d5 ;
            if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,pointX,pointY,&precisionError)) goto errexit ;
           }
       break ;
/*
**     Move Point Towards trgPnt2
*/
       case  2 :
         if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Moving Point Towards trgpnt2 ") ;
         d5 = d2 ;
         d2 = d2 - dtmP->mppTol  ;
         dx = pointX - pointAddrP(dtmP,trgPnt2)->x ;
         dy = pointY - pointAddrP(dtmP,trgPnt2)->y ;
         while ( d2 > dtmP->mppTol && precisionError  && loop < numLoop )
           {
            ++loop ;
            d2 = d2 - dtmP->mppTol ;
            pointX = pointAddrP(dtmP,trgPnt2)->x + dx * d2 / d5 ;
            pointY = pointAddrP(dtmP,trgPnt2)->y + dy * d2 / d5 ;
            if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,pointX,pointY,&precisionError)) goto errexit ;
           }
      break ;
/*
**    Move Point Towards trgPnt3
*/
       case  3 :
         if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Moving Point Towards trgpnt3 ") ;
         d5 = d3 ;
         d3 = d3 - dtmP->mppTol  ;
         dx = pointX - pointAddrP(dtmP,trgPnt3)->x ;
         dy = pointY - pointAddrP(dtmP,trgPnt3)->y ;
         while ( d3 > dtmP->mppTol && precisionError  && loop < numLoop )
           {
            ++loop ;
            d3 = d3 - dtmP->mppTol ;
            pointX = pointAddrP(dtmP,trgPnt3)->x + dx * d3 / d5 ;
            pointY = pointAddrP(dtmP,trgPnt3)->y + dy * d3 / d5 ;
            if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,pointX,pointY,&precisionError)) goto errexit ;
           }
       break ;
/*
**     Move Point Towards trgPnt4
*/
       case  4 :
         if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Moving Point Towards trgpnt4 ") ;
         d5 = d4 ;
         d4 = d4 - dtmP->mppTol ;
         dx = pointX - pointAddrP(dtmP,trgPnt4)->x ;
         dy = pointY - pointAddrP(dtmP,trgPnt4)->y ;
         while ( d4 > dtmP->mppTol && precisionError  && loop < numLoop )
           {
            ++loop ;
            d4 = d4 - dtmP->mppTol ;
            pointX = pointAddrP(dtmP,trgPnt4)->x + dx * d4 / d5 ;
            pointY = pointAddrP(dtmP,trgPnt4)->y + dy * d4 / d5 ;
            if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,trgPnt4,pointX,pointY,&precisionError)) goto errexit ;
           }
       break ;

/*
**     No Fix
*/
       default :
       break   ;
      } ;
/*
**  Check If Precision Fixed
*/
    if( precisionError == FALSE )
      {
       *fixTypeP = 3 ;
       *fixedXP = pointX ;
       *fixedYP = pointY ;
      }
   }
/*
** Precision Not Fixed So Snap To Closest Of trgPnt2 or trgPnt4
*/
 if( ! *fixTypeP )
   {
    ++numSnapFix ;
    d2 = bcdtmMath_distance(saveX,saveY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    d4 = bcdtmMath_distance(saveX,saveY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
    if( d2 <= d4 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Snapping To trgpnt2") ;
       *fixTypeP = 1 ;
       *fixedXP  = pointAddrP(dtmP,trgPnt2)->x ;
       *fixedYP  = pointAddrP(dtmP,trgPnt2)->y ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Precision By Snapping To trgpnt4") ;
       *fixTypeP = 2 ;
       *fixedXP  = pointAddrP(dtmP,trgPnt4)->x ;
       *fixedYP  = pointAddrP(dtmP,trgPnt4)->y ;
      }
   }

/*
**  Write Precision Fix Results
*/
 if( dbg )
   {
    pointX = *fixedXP ;
    pointY = *fixedYP ;
/*
**  Calculate Precision Varaibles
*/
    d1 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
    d2 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    d3 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) ;
    d4 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
    d5 = bcdtmMath_distance(pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
/*
**  Calculate Precision Variables
*/
    sdof12  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    sdof14  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
    sdof32  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    sdof34  = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
    sdof132 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    sdof134 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y) ;
/*
**  Calculate Precision Angles
*/
    angle12 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt1,trgPnt2) ;
    angle1  = bcdtmMath_getAngle(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointX,pointY) ;
    angle14 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt1,trgPnt4) ;
    angle32 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt3,trgPnt2) ;
    angle3  = bcdtmMath_getAngle(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY) ;
    angle34 = bcdtmMath_getPointAngleDtmObject(dtmP,trgPnt3,trgPnt4) ;
    if( angle12 < angle14 ) angle12 += DTM_2PYE ;
    if( angle1  < angle14 ) angle1  += DTM_2PYE ;
    if( angle34 < angle32 ) angle34 += DTM_2PYE ;
    if( angle3  < angle32 ) angle3  += DTM_2PYE ;
/*
**  Write Precision Varaibles
*/
    bcdtmWrite_message(0,0,0,"Results Of Precision Fix") ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt1-point   = %20.15lf",d1) ;
    if( d1 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt1-point   == 0.0") ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt2-point   = %20.15lf",d2) ;
    if( d2 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt2-point   == 0.0") ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt3-point   = %20.15lf",d3) ;
    if( d3 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt3-point   == 0.0") ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt4-point   = %20.15lf",d4) ;
    if( d4 == 0.0 ) bcdtmWrite_message(0,0,0,"distance  trgPnt4-point   == 0.0") ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt2-trgPnt4 = %20.15lf",d5) ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt1-trgPnt2 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt1-trgPnt4 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt3-trgPnt2 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y)) ;
    bcdtmWrite_message(0,0,0,"distance  trgPnt3-trgPnt4 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt4)->x,pointAddrP(dtmP,trgPnt4)->y)) ;
    bcdtmWrite_message(0,0,0,"sideOf12  trgPnt1-point-trgPnt2   = %2d[ 1]",sdof12) ;
    bcdtmWrite_message(0,0,0,"sideOf14  trgPnt1-point-trgPnt4   = %2d[-1]",sdof14) ;
    bcdtmWrite_message(0,0,0,"sideOf32  trgPnt3-point-trgPnt2   = %2d[-1]",sdof32) ;
    bcdtmWrite_message(0,0,0,"sideOf34  trgPnt3-point-trgPnt4   = %2d[ 1]",sdof34) ;
    bcdtmWrite_message(0,0,0,"sideOf132 trgPnt1-trgpnt3-trgPnt2 = %2d ",sdof132) ;
    bcdtmWrite_message(0,0,0,"sideOf134 trgPnt1-trgpnt3-trgPnt4 = %2d ",sdof134) ;
    bcdtmWrite_message(0,0,0,"angle12                           = %17.15lf",angle12) ;
    bcdtmWrite_message(0,0,0,"angle1                            = %17.15lf",angle1) ;
    bcdtmWrite_message(0,0,0,"angle14                           = %17.15lf",angle14) ;
    bcdtmWrite_message(0,0,0,"angle12-angle14                   = %17.15lf",angle12-angle14) ;
    bcdtmWrite_message(0,0,0,"angle32                           = %17.15lf",angle32) ;
    bcdtmWrite_message(0,0,0,"angle3                            = %17.15lf",angle3) ;
    bcdtmWrite_message(0,0,0,"angle34                           = %17.15lf",angle34) ;
    bcdtmWrite_message(0,0,0,"angle34-angle32                   = %17.15lf",angle34-angle32) ;
   }
/*
** Write Results Of Precision Fix
*/
 if( dbg )
   {
    if( *fixTypeP ) bcdtmWrite_message(0,0,0,"**** Precision Fixed ** fixType = %2ld",*fixTypeP) ;
    else            bcdtmWrite_message(0,0,0,"**** Precision Not Fixed") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Point Quadrilateral Precision Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Point Quadrilateral Precision Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkPointHullTrianglePrecisionDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double x,double y,long *precisionErrorP)
/*
** Check For A Precision Error Occurring If The Point XY Is
** Inserted On Line P1-P2 Of Triangle P1P2P3
** P1P2P3 Must Be In An Anticlockwise Direction
** This Function Should Be Called Prior To Inserting A Point
** Into A Hull Line
*/
{
 DTM_TIN_POINT  *p1P,*p2P,*p3P ;
/*
** Initialise
*/
 *precisionErrorP = 0 ;
/*
** Get Point Addresses
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p3P = pointAddrP(dtmP,P3) ;
/*
** Check Side Of Conditions
*/
 if     ( bcdtmMath_allSideOf(p1P->x,p1P->y,p3P->x,p3P->y,x,y) >= 0 ) *precisionErrorP = 1 ;
 else if( bcdtmMath_allSideOf(p2P->x,p2P->y,p3P->x,p3P->y,x,y) <= 0 ) *precisionErrorP = 1 ;
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
BENTLEYDTM_Public int bcdtmInsert_fixPointHullTrianglePrecisionDtmObject(BC_DTM_OBJ *dtmP,long trgPnt1,long trgPnt2,long trgPnt3,double pointX,double pointY,double *fixedXP,double *fixedYP,long *fixTypeP )
/*
** Assumes:-
**
** 1. triangle trgPnt1-trgPnt2-trgPnt3 Is In An Anti ClockWise Direction
** 2. pointX,pointY is on Line trgPnt1-trgPnt2
**
*/
{
 int    ret=DTM_SUCCESS,sdof1,sdof2,sdof3,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   pointOnLine;
 double dx,dy,d1,d2,d3,d4,xMin,xMax,yMin,yMax ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Fixing Hull Triangle Precision") ;
    bcdtmWrite_message(0,0,0,"pointX  = %20.15lf pointY = %20.15lf",pointX,pointY) ;
    bcdtmWrite_message(0,0,0,"trgPnt1 = %8ld Fptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt1,nodeAddrP(dtmP,trgPnt1)->hPtr,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"trgPnt2 = %8ld Fptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt2,nodeAddrP(dtmP,trgPnt2)->hPtr,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointAddrP(dtmP,trgPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"trgPnt3 = %8ld Fptr = %9ld ** %20.15lf %20.15lf %10.4lf",trgPnt3,nodeAddrP(dtmP,trgPnt3)->hPtr,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointAddrP(dtmP,trgPnt3)->z) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol = %20.15lf",dtmP->ppTol)  ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol = %20.15lf",dtmP->plTol)  ;
   }
/*
** Initialise
*/
 *fixTypeP =  0 ;
 *fixedXP = 0.0 ;
 *fixedYP = 0.0 ;
/*
** Check Point Is On Line trgPnt1-trgPnt2
*/
 if( pointAddrP(dtmP,trgPnt1)->x <= pointAddrP(dtmP,trgPnt2)->x ) { xMin = pointAddrP(dtmP,trgPnt1)->x ; xMax = pointAddrP(dtmP,trgPnt2)->x ; }
 else                                                     { xMin = pointAddrP(dtmP,trgPnt2)->x ; xMax = pointAddrP(dtmP,trgPnt1)->x ; }
 if( pointAddrP(dtmP,trgPnt1)->y <= pointAddrP(dtmP,trgPnt2)->y ) { yMin = pointAddrP(dtmP,trgPnt1)->y ; yMax = pointAddrP(dtmP,trgPnt2)->y ; }
 else                                                     { yMin = pointAddrP(dtmP,trgPnt2)->y ; yMax = pointAddrP(dtmP,trgPnt1)->y ; }
 pointOnLine = 1 ;
 if( pointX < xMin || pointX > xMax || pointY < yMin || pointY > yMax ) pointOnLine = 0 ;
 if( dbg &&   pointOnLine ) bcdtmWrite_message(0,0,0,"Point On Line") ;
 if( ! pointOnLine )
   {
    bcdtmWrite_message(2,0,0,"Point Not On Insert Line") ;
    goto errexit ;
   }
/*
** Calculate Distances
*/
 d1 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
 d2 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
 d3 = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) ;
 d4 = bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
/*
** Ratio Re-Calculate Point Coordinates
*/
 dx = pointAddrP(dtmP,trgPnt2)->x - pointAddrP(dtmP,trgPnt1)->x ;
 dy = pointAddrP(dtmP,trgPnt2)->y - pointAddrP(dtmP,trgPnt1)->y ;
 pointX = pointAddrP(dtmP,trgPnt1)->x + dx * d1 / d4 ;
 pointY = pointAddrP(dtmP,trgPnt1)->y + dy * d1 / d4 ;
/*
** Calculate Side Ofs
*/
 sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
 sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
 sdof3 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointX,pointY) ;
/*
**  Write Precision Varaibles
*/
 if( dbg )
   {
    bcdtmList_writeCircularListForPointDtmObject(dtmP,trgPnt3) ;
    bcdtmWrite_message(0,0,0,"distance trgPnt1-point       = %20.15lf",d1) ;
    bcdtmWrite_message(0,0,0,"distance trgPnt2-point       = %20.15lf",d2) ;
    bcdtmWrite_message(0,0,0,"distance trgPnt3-point       = %20.15lf",d3) ;
    bcdtmWrite_message(0,0,0,"distance trgPnt1-trgPnt2     = %20.15lf",d4) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPnt1 = %2d",sdof1) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt3-point-trgPnt2 = %2d",sdof2) ;
    bcdtmWrite_message(0,0,0,"sideOf trgPnt1-trgPnt2-point = %2d",sdof3) ;
   }
/*
** Check If ReCalculation Fixed Precision Problen
*/
 if( sdof1 < 0 && sdof2 > 0 )
   {
    *fixTypeP = 1 ;
    *fixedXP = pointX ;
    *fixedYP = pointY ;
   }
/*
** Move Point Towards trgPnt2
*/
 else if ( sdof1 >= 0 && sdof2 > 0 )
   {
    while( bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) >= 0 )
      {
       d1 = d1 + dtmP->ppTol ;
       pointX = pointAddrP(dtmP,trgPnt1)->x + dx * d1 / d4 ;
       pointY = pointAddrP(dtmP,trgPnt1)->y + dy * d1 / d4 ;
      }
    *fixTypeP = 2 ;
    *fixedXP = pointX ;
    *fixedYP = pointY ;
   }
/*
** Move Point Towards trgPnt1
*/
 else if ( sdof1 < 0 && sdof2 <= 0 )
   {
    while( bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) <= 0 )
      {
       d1 = d1 - dtmP->ppTol ;
       pointX = pointAddrP(dtmP,trgPnt1)->x + dx * d1 / d4 ;
       pointY = pointAddrP(dtmP,trgPnt1)->y + dy * d1 / d4 ;
      }
    *fixTypeP = 3 ;
    *fixedXP = pointX ;
    *fixedYP = pointY ;
   }
/*
** Write Results Of Precision Fix
*/
 if( cdbg || dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Point Triangle Hull Precision Fix Results") ;
    sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
    sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y,pointX,pointY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
    sdof3 = bcdtmMath_sideOf(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,pointX,pointY) ;
    bcdtmWrite_message(0,0,0,"**** sideOf trgPnt3-point-trgPnt1 = %2d",sdof1) ;
    bcdtmWrite_message(0,0,0,"**** sideOf trgPnt3-point-trgPnt2 = %2d",sdof2) ;
    bcdtmWrite_message(0,0,0,"**** sideOf trgPnt1-trgPnt2-point = %2d",sdof3) ;
    if      ( *fixTypeP == 1 ) bcdtmWrite_message(0,0,0,"**** Precision Fixed By Recalculation") ;
    else if ( *fixTypeP == 2 ) bcdtmWrite_message(0,0,0,"**** Precision Fixed By Moving Point Towards trgPnt2") ;
    else if ( *fixTypeP == 3 ) bcdtmWrite_message(0,0,0,"**** Precision Fixed By Moving Point Towards trgPnt1") ;
    else                       bcdtmWrite_message(0,0,0,"**** Precision Not Fixed") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Hull Triangle Precision Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Hull Triangle Precision Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkPointXYCanBeMovedDtmObject(BC_DTM_OBJ *dtmP,long Point,long *moveFlagP,double x,double y)
/*
** This Function Checks If Point XY can be moved to x & y
*/
{
 long   p2,p3,clPtr ;
 double sx,sy    ;
/*
** Initialise Variables
*/
 *moveFlagP = 0 ;
 if(( clPtr = nodeAddrP(dtmP,Point)->cPtr ) == dtmP->nullPtr ) return(0) ;
 *moveFlagP = 1 ;
 sx = x ; sy = y ;
 pointAddrP(dtmP,Point)->x = x ;
 pointAddrP(dtmP,Point)->y = y ;
/*
** Check Point Precision
*/
 if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,Point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) return(1) ;
 while ( clPtr != dtmP->nullPtr && *moveFlagP )
   {
    p3  = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    if( nodeAddrP(dtmP,Point)->hPtr != p2 )
      {
       if( bcdtmMath_allPointSideOfDtmObject(dtmP,Point,p2,p3) >= 0 ) *moveFlagP = 0 ;
      }
    p2 = p3 ;
   }
/*
** Reset Point XY Coordinates
*/
 pointAddrP(dtmP,Point)->x = sx ;
 pointAddrP(dtmP,Point)->y = sy ;
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
BENTLEYDTM_Public int bcdtmInsert_checkPointCanBeMovedOnToLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double x,double y)
/*
** P1,P2,P3 Form An Anti Clockwise triangle . P2 P3 Is an Internal triangle Edge
** This Function Tests If P1 can be moved to x,y on Line P2,P3
**
** Scan Point And Check If It can Be Moved To x,y
** Return values == 0 Point Can Be Moved
**               == 1 System Error
**               == 2 Point Can Not Be Moved
**
*/
{
 long   p1,p2,clPtr,sclPtr,process ;
 double sx,sy ;
/*
**  Initialise
*/
 sclPtr = clPtr = nodeAddrP(dtmP,P1)->cPtr ;
 if( clPtr == dtmP->nullPtr ) return(2) ;
 process = 1 ;
 while ( clPtr != dtmP->nullPtr  && process )
   {
    if( clistAddrP(dtmP,clPtr)->pntNum == P2 ) process = 0 ;
    else                                       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
   }
 if( process ) return(2) ;
 sx = pointAddrP(dtmP,P1)->x ;
 sy = pointAddrP(dtmP,P1)->y ;
/*
** Scan About P1 From P2 To P3 And Check Precision
*/
 pointAddrP(dtmP,P1)->x = x ;
 pointAddrP(dtmP,P1)->y = y ;
 p1 = P2 ;
 clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
 if( clPtr == dtmP->nullPtr )  clPtr = sclPtr ;
 p2 = clistAddrP(dtmP,clPtr)->pntNum ;
 while ( p1 != P3 || p2 != P2 )
   {
    if( bcdtmMath_allPointSideOfDtmObject(dtmP,P1,p1,p2) >= 0 )
      {
       pointAddrP(dtmP,P1)->x = sx ;
       pointAddrP(dtmP,P1)->y = sy ;
       return(2) ;
      }
    p1  = p2 ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    if( clPtr == dtmP->nullPtr ) clPtr = sclPtr ;
    p2  = clistAddrP(dtmP,clPtr)->pntNum ;
   }
/*
** Check P1 can connect to Point On other Side Of P2P3
*/
 if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,P2,P3)) < 0 ) return(1) ;
 if( bcdtmMath_pointSideOfDtmObject(dtmP,P1,p1,P2) >= 0 || bcdtmMath_pointSideOfDtmObject(dtmP,P1,p1,P3) <= 0 )
   { pointAddrP(dtmP,P1)->x = sx ; pointAddrP(dtmP,P1)->y = sy ; return(2) ; }
/*
** Restore Values
*/
 pointAddrP(dtmP,P1)->x = sx ;
 pointAddrP(dtmP,P1)->y = sy ;
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
BENTLEYDTM_Public int bcdtmInsert_getZvalueDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,double x,double y,double *z)
/*
** This Function Interpolates The z value At XY on Line P1P2
*/
{
 double  dx,dy,dz ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Test For Single Point
*/
 *z = 0.0 ;
 if( P2 == dtmP->nullPnt || P1 == P2 ) { *z = pointAddrP(dtmP,P1)->z ; return(DTM_SUCCESS) ; }
/*
** Interpolate On Line
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 dx = p2P->x - p1P->x ;
 dy = p2P->y - p1P->y ;
 dz = p2P->z - p1P->z ;
 if( fabs(dx) >= fabs(dy) ) *z = p1P->z + ( x - p1P->x ) / dx  * dz ;
 else                       *z = p1P->z + ( y - p1P->y ) / dy  * dz ;
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
BENTLEYDTM_Public int bcdtmInsert_checkForCreationOfKnotInExistingFeatureDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,long *createKnotP)
/*
** This Function Checks For The Creation Of A Knot In an Existing feature
** If point P3 Is Moved Onto Line P1,P2
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long cl1Ptr,cl2Ptr,cl3Ptr,p4,point,feature ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Creation Of Knot In Existing feature ** P1 = %6ld P2 = %6ld P3 = %6ld",P1,P2,P3) ;
/*
** Initialise
*/
 *createKnotP = 0 ;
/*
** Check If P3 P1 And P2 Are On Same feature
*/
 cl1Ptr = nodeAddrP(dtmP,P3)->fPtr  ;
 while( cl1Ptr != dtmP->nullPtr && ! *createKnotP )
   {
    point   = flistAddrP(dtmP,cl1Ptr)->nextPnt ;
    feature = flistAddrP(dtmP,cl1Ptr)->dtmFeature ;
    cl1Ptr     = flistAddrP(dtmP,cl1Ptr)->nextPtr  ;
/*
**  Check If  P1 P2 Are On The Same feature As P3
*/
    cl2Ptr = nodeAddrP(dtmP,P1)->fPtr ;
    while ( cl2Ptr != dtmP->nullPtr && ! * createKnotP )
      {
       if( flistAddrP(dtmP,cl2Ptr)->nextPnt == P2 && flistAddrP(dtmP,cl2Ptr)->dtmFeature == feature ) *createKnotP = 1 ;
       cl2Ptr = flistAddrP(dtmP,cl2Ptr)->nextPtr ;
      }
/*
**  Check If P2 P1 Are On The Same feature As P3
*/
    cl2Ptr = nodeAddrP(dtmP,P2)->fPtr ;
    while ( cl2Ptr != dtmP->nullPtr && ! * createKnotP )
      {
       if( flistAddrP(dtmP,cl2Ptr)->nextPnt == P1 && flistAddrP(dtmP,cl2Ptr)->dtmFeature == feature ) *createKnotP = 1 ;
       cl2Ptr = flistAddrP(dtmP,cl2Ptr)->nextPtr ;
      }
   }
/*
** Scan All features For Which P3 Is Last point
*/
 if( ! * createKnotP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning P3 For Existing features On P1 P2") ;
    cl1Ptr = nodeAddrP(dtmP,P3)->cPtr ;
    while( cl1Ptr != dtmP->nullPtr && ! *createKnotP )
      {
       p4  = clistAddrP(dtmP,cl1Ptr)->pntNum ;
       cl1Ptr = clistAddrP(dtmP,cl1Ptr)->nextPtr ;
/*
**     Scan feature List For p4
*/
       cl2Ptr = nodeAddrP(dtmP,p4)->fPtr  ;
       while( cl2Ptr != dtmP->nullPtr && ! *createKnotP )
         {
          point   = flistAddrP(dtmP,cl2Ptr)->nextPnt ;
          feature = flistAddrP(dtmP,cl2Ptr)->dtmFeature ;
          cl2Ptr     = flistAddrP(dtmP,cl2Ptr)->nextPtr  ;
          if( point == P3 )
            {
/*
**           Scan For P1 P2 On Same feature
*/
             cl3Ptr = nodeAddrP(dtmP,P1)->fPtr ;
             while ( cl3Ptr != dtmP->nullPtr && ! * createKnotP )
               {
                if( flistAddrP(dtmP,cl3Ptr)->nextPnt == P2 && flistAddrP(dtmP,cl3Ptr)->dtmFeature == feature ) *createKnotP = 1 ;
                cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ;
               }
/*
**           Scan For P2 P1 On Same feature
*/
             cl3Ptr = nodeAddrP(dtmP,P2)->fPtr ;
             while ( cl3Ptr != dtmP->nullPtr && ! * createKnotP )
               {
                if( flistAddrP(dtmP,cl3Ptr)->nextPnt == P1 && flistAddrP(dtmP,cl3Ptr)->dtmFeature == feature ) *createKnotP = 1 ;
                cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ;
               }
            }
         }
      }
   }
/*
** Write Departing Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Creation Of Knot In Existing feature Completed ** creatKnotP = %1ld",*createKnotP) ;
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
BENTLEYDTM_Public int bcdtmInsert_checkAndFixInsertLinesDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3)
/*
** This Function Checks And Fixes Insert Lines
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  clPtr,nextPnt,priorPnt ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Fixing Feature Lines For P1 P2 P3",P1,P2,P3) ;
//    bcdtmList_writeDtmFeaturesForPointDtmObject(dtmP,P1) ;
//    bcdtmList_writeDtmFeaturesForPointDtmObject(dtmP,P2) ;
//    bcdtmList_writeDtmFeaturesForPointDtmObject(dtmP,P3) ;
   }
/*
** If P1,P2 or P3 Not On Insert Line Return
*/
 if( nodeAddrP(dtmP,P1)->fPtr == dtmP->nullPtr ) goto cleanup ;
 if( nodeAddrP(dtmP,P2)->fPtr == dtmP->nullPtr ) goto cleanup ;
 if( nodeAddrP(dtmP,P3)->fPtr == dtmP->nullPtr ) goto cleanup ;
/*
** Check If P1,P2,P3 Are On The Same Insert Line About P2 ;
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking If P1 P2 P3 Are On Same Insert Line About P2") ;
 clPtr = nodeAddrP(dtmP,P2)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature,P2,&priorPnt) ) goto errexit ;
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature,P2,&nextPnt)  ) goto errexit ;
    if(( priorPnt == P1 && nextPnt == P3 ) || ( priorPnt == P3 && nextPnt == P1 ))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Point %6ld From Feature %6ld",P2,flistAddrP(dtmP,clPtr)->dtmFeature) ;
       if( bcdtmInsert_removePointFromDtmFeatureDtmObject(dtmP,P2,flistAddrP(dtmP,clPtr)->dtmFeature)) goto errexit ;
       clPtr = nodeAddrP(dtmP,P2)->fPtr ;
      }
    else clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Check If P1,P2,P3 Are On The Same Insert Line About P3 ;
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking If P1 P2 P3 Are On Same Insert Line About P3") ;
 clPtr = nodeAddrP(dtmP,P3)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature,P3,&priorPnt) ) goto errexit ;
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clPtr)->dtmFeature,P3,&nextPnt)  ) goto errexit ;
    if(( priorPnt == P1 && nextPnt == P2 ) || ( priorPnt == P2 && nextPnt == P1 ))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Point %6ld From Feature %6ld",P3,flistAddrP(dtmP,clPtr)->dtmFeature) ;
       if( bcdtmInsert_removePointFromDtmFeatureDtmObject(dtmP,P3,flistAddrP(dtmP,clPtr)->dtmFeature)) goto errexit ;
       clPtr = nodeAddrP(dtmP,P3)->fPtr ;
      }
    else clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmInsert_pointIntoDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long P1,long P2,long P3)
/*
** This Function Inserts A Point Into A DTM Feature
*/
{
 int  ret=DTM_SUCCESS ;
 long listPtr,listOfs,pl3Ptr,cl3Ptr ;
/*
** Scan P1 and See If it Connects To P2. If so Insert P1P3 and P3P2
*/
 listPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while( listPtr != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,listPtr)->dtmFeature == dtmFeature )
      {
       if( flistAddrP(dtmP,listPtr)->nextPnt == P2 )
         {
          flistAddrP(dtmP,listPtr)->nextPnt = P3 ;
          if( bcdtmInsert_getNextFeatureListOffsetDtmObject(dtmP,&listOfs) ) goto errexit  ;
          flistAddrP(dtmP,listOfs)->nextPnt = P2  ;
          flistAddrP(dtmP,listOfs)->nextPtr = dtmP->nullPtr ;
          flistAddrP(dtmP,listOfs)->dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ;
/*
**        Store New Feature List Entry For P3
*/
          pl3Ptr = cl3Ptr = nodeAddrP(dtmP,P3)->fPtr ;
          while( cl3Ptr != dtmP->nullPtr ) { pl3Ptr = cl3Ptr ; cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ; }
          if( pl3Ptr == cl3Ptr ) nodeAddrP(dtmP,P3 )->fPtr = listOfs  ;
          else                   flistAddrP(dtmP,pl3Ptr)->nextPtr = listOfs  ;
         }
      }
    listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
   }
/*
** Scan P2 and See If it Connects To P1. If so Insert P2P3 and P3P1
*/
 listPtr = nodeAddrP(dtmP,P2)->fPtr ;
 while( listPtr != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,listPtr)->dtmFeature == dtmFeature )
      {
       if( flistAddrP(dtmP,listPtr)->nextPnt == P1 )
         {
          flistAddrP(dtmP,listPtr)->nextPnt = P3 ;
          if( bcdtmInsert_getNextFeatureListOffsetDtmObject(dtmP,&listOfs) ) goto errexit  ;
          flistAddrP(dtmP,listOfs)->nextPnt = P1  ;
          flistAddrP(dtmP,listOfs)->nextPtr = dtmP->nullPtr ;
          flistAddrP(dtmP,listOfs)->dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ;
/*
**        Store New Feature List Entry For P3
*/
          pl3Ptr = cl3Ptr = nodeAddrP(dtmP,P3)->fPtr ;
          while( cl3Ptr != dtmP->nullPtr ) { pl3Ptr = cl3Ptr ; cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ; }
          if( pl3Ptr == cl3Ptr ) nodeAddrP(dtmP,P3 )->fPtr = listOfs  ;
          else                   flistAddrP(dtmP,pl3Ptr)->nextPtr = listOfs  ;
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
BENTLEYDTM_Public int bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3)
    {
    return bcdtmInsert_pointIntoAllDtmFeaturesWithPntTypeDtmObject(dtmP,P1,P2,P3, 1) ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public        int bcdtmInsert_pointIntoAllDtmFeaturesWithPntTypeDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3, int pntType)
/*
** This Function Inserts A Point Into The Feature Linked Lists
*/
{
 int  ret=DTM_SUCCESS ;
 long clPtr,plPtr,cl3Ptr,pl3Ptr,fListOfs ;
/*
** Scan P1 and See If it Connects To P2. If so Insert P1P3 and P3P2
*/
 plPtr = clPtr = nodeAddrP(dtmP,P1)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
   const DTM_FEATURE_LIST* fListP = flistAddrP (dtmP, clPtr);
   if (fListP->nextPnt == P2 && ftableAddrP (dtmP, fListP->dtmFeature)->dtmFeatureType != DTMFeatureType::GroupSpots)
      {
/*
**     Change P2 Point Value To P3
*/
       flistAddrP(dtmP,clPtr)->nextPnt  = P3 ;
/*
**     Allocate New Feature List Entry For P3
*/
       if( bcdtmInsert_getNextFeatureListOffsetDtmObject(dtmP,&fListOfs) ) goto errexit ;
       flistAddrP(dtmP,fListOfs)->nextPnt = P2   ;
       flistAddrP(dtmP,fListOfs)->dtmFeature = flistAddrP(dtmP,clPtr)->dtmFeature ;
       flistAddrP(dtmP,fListOfs)->nextPtr = dtmP->nullPtr ;
       flistAddrP(dtmP,fListOfs)->pntType = pntType;

/*
**     Store New Feature List Entry For P3
*/
       pl3Ptr = cl3Ptr = nodeAddrP(dtmP,P3)->fPtr ;
       while( cl3Ptr != dtmP->nullPtr ) { pl3Ptr = cl3Ptr ; cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ; }
       if( pl3Ptr == cl3Ptr ) nodeAddrP(dtmP,P3 )->fPtr = fListOfs  ;
       else                   flistAddrP(dtmP,pl3Ptr)->nextPtr = fListOfs  ;
      }
    plPtr = clPtr ;
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Scan P2 and See If it Connects To P1. If so Insert P2P3 and P3P1
*/
 plPtr = clPtr = nodeAddrP(dtmP,P2)->fPtr ;
 while( clPtr != dtmP->nullPtr )
   {
   const DTM_FEATURE_LIST* fListP = flistAddrP (dtmP, clPtr);
   if (fListP->nextPnt == P1 && ftableAddrP (dtmP, fListP->dtmFeature)->dtmFeatureType != DTMFeatureType::GroupSpots)
       {
/*
**     Change P1 Point Value To P3
*/
       flistAddrP(dtmP,clPtr)->nextPnt  = P3 ;
/*
**     Allocate New Feature List Entry For P3
*/
       if( bcdtmInsert_getNextFeatureListOffsetDtmObject(dtmP,&fListOfs) ) goto errexit ;
       flistAddrP(dtmP,fListOfs)->nextPnt    = P1   ;
       flistAddrP(dtmP,fListOfs)->dtmFeature = flistAddrP(dtmP,clPtr)->dtmFeature ;
       flistAddrP(dtmP,fListOfs)->nextPtr    = dtmP->nullPtr ;
       flistAddrP(dtmP,fListOfs)->pntType = pntType;
/*
**     Store New Feature List Entry For P3
*/
       pl3Ptr = cl3Ptr = nodeAddrP(dtmP,P3)->fPtr ;
       while( cl3Ptr != dtmP->nullPtr ) { pl3Ptr = cl3Ptr ; cl3Ptr = flistAddrP(dtmP,cl3Ptr)->nextPtr ; }
       if( pl3Ptr == cl3Ptr ) nodeAddrP(dtmP,P3 )->fPtr = fListOfs  ;
       else                   flistAddrP(dtmP,pl3Ptr)->nextPtr = fListOfs  ;
      }
    plPtr = clPtr ;
    clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmInsert_getNextFeatureListOffsetDtmObject(BC_DTM_OBJ *dtmP,long *listOffsetP)
/*
** This Function Gets The Next Entry Offset In The Feature List
*/
{
 int ret=DTM_SUCCESS ;
/*
**  Initialise
*/
 *listOffsetP = dtmP->nullPnt ;
/*
** Get Next Entry In Feature List
*/
 if( dtmP->fListDelPtr != dtmP->nullPtr )
   {
    *listOffsetP = dtmP->fListDelPtr ;
    dtmP->fListDelPtr = flistAddrP(dtmP,*listOffsetP)->nextPtr ;
   }
 else
   {
    if( dtmP->numFlist >= dtmP->memFlist )
      {
       if(bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit ;
      }
    *listOffsetP = dtmP->numFlist ;
    ++dtmP->numFlist ;
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
BENTLEYDTM_Public int bcdtmInsert_removePointFromDtmFeatureDtmObject (BC_DTM_OBJ *dtmP, long point, long dtmFeature)
    {
    int  ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long nextPnt, priorPnt, listEntry, priorListEntry, clp, pointRemoved, pointOnDtmFeature;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Removing Point From Dtm Feature");
        bcdtmWrite_message (0, 0, 0, "dtmP       = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "point      = %8ld", point);
        bcdtmWrite_message (0, 0, 0, "dtmFeature = %8ld", dtmFeature);
        }
    /*
    ** Check Validity Of Dtm Feature
    */
    if (dtmFeature >= 0 && dtmFeature < dtmP->numFeatures)
        {
        if (ftableAddrP (dtmP, dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt)
            {
            /*
            **     Check Point Is On Dtm Feature
            */
            if (dbg) bcdtmWrite_message (0, 0, 0, "Checking Point Is On Dtm Feature");
            pointOnDtmFeature = 0;
            nextPnt = priorPnt = ftableAddrP (dtmP, dtmFeature)->dtmFeaturePts.firstPoint;
            do
                {
                if (nextPnt == point) pointOnDtmFeature = 1;
                if (bcdtmList_getNextPointForDtmFeatureDtmObject (dtmP, dtmFeature, nextPnt, &nextPnt))  goto errexit;
                } while (nextPnt != priorPnt && nextPnt != dtmP->nullPnt && !pointOnDtmFeature);
                /*
                **     Only Process If Point Is On Dtm Feature
                */
                if (pointOnDtmFeature)
                    {
                    /*
                    **        Check Feature List Exists For Point
                    */
                    if ((listEntry = priorListEntry = nodeAddrP (dtmP, point)->fPtr) == dtmP->nullPtr)
                        {
                        bcdtmWrite_message (2, 0, 0, "No Feature List For Point");
                        goto errexit;
                        }
                    /*
                    **        Get Feature List Entry For DTM Feature
                    */
                    while (listEntry != dtmP->nullPtr && flistAddrP (dtmP, listEntry)->dtmFeature != dtmFeature)
                        {
                        priorListEntry = listEntry;
                        listEntry = flistAddrP (dtmP, listEntry)->nextPtr;
                        }
                    /*
                    **        Check A Feature List Entry For Dtm Feature Exists For Point
                    */
                    if (listEntry == dtmP->nullPtr)
                        {
                        bcdtmWrite_message (2, 0, 0, "Feature List For Point Does Not Exist");
                        goto errexit;
                        }
                    /*
                    **        Remove Point From Feature List
                    */
                    if (dbg) bcdtmWrite_message (0, 0, 0, "Removing Point From Feature List");
                    pointRemoved = FALSE;
                    nextPnt = flistAddrP (dtmP, listEntry)->nextPnt;
                    if (bcdtmList_getPriorPointForDtmFeatureDtmObject (dtmP, dtmFeature, point, &priorPnt)) goto errexit;
                    /*
                    **        Set Next Point Dtm Feature List Entry Of Prior Point To Next Point
                    */
                    if (priorPnt != dtmP->nullPnt)
                        {
                        if ((clp = nodeAddrP (dtmP, priorPnt)->fPtr) == dtmP->nullPtr)
                            {
                            bcdtmWrite_message (2, 0, 0, "Prior Point Has No Feature List");
                            goto errexit;
                            }
                        /*
                        **           Scan Prior Point To For Dtm Feature List Entry
                        */
                        while (clp != dtmP->nullPtr && pointRemoved == FALSE)
                            {
                            if (dbg) bcdtmWrite_message (0, 0, 0, "nextPoint = %9ld dtmFeature = %9ld", flistAddrP (dtmP, clp)->nextPnt, flistAddrP (dtmP, clp)->dtmFeature);
                            if (flistAddrP (dtmP, clp)->nextPnt == point && flistAddrP (dtmP, clp)->dtmFeature == dtmFeature)
                                {
                                flistAddrP (dtmP, clp)->nextPnt = nextPnt;
                                pointRemoved = TRUE;
                                }
                            else   clp = flistAddrP (dtmP, clp)->nextPtr;
                            }
                        /*
                        **           Check Point Has Been Removed From Dtm Feature
                        */
                        if (!pointRemoved)
                            {
                            bcdtmWrite_message (2, 0, 0, "Error Removing Feature From Point Feature List");
                            goto errexit;
                            }
                        }
                    /*
                    **        If Point Is First Point In Dtm Feature List For Point Set First pointer
                    */
                    if (ftableAddrP (dtmP, dtmFeature)->dtmFeaturePts.firstPoint == point)
                        {
                        ftableAddrP (dtmP, dtmFeature)->dtmFeaturePts.firstPoint = flistAddrP (dtmP, listEntry)->nextPnt;
                        }
                    if (ftableAddrP (dtmP, dtmFeature)->dtmFeaturePts.firstPoint == dtmP->nullPnt)
                        {
                        ftableAddrP (dtmP, dtmFeature)->dtmFeatureState = DTMFeatureState::Deleted;
                        }
                    /*
                    **        Update Point Dtm Feature List Entry
                    */
                    if (listEntry == nodeAddrP (dtmP, point)->fPtr) nodeAddrP (dtmP, point)->fPtr = flistAddrP (dtmP, listEntry)->nextPtr;
                    else                                          flistAddrP (dtmP, priorListEntry)->nextPtr = flistAddrP (dtmP, listEntry)->nextPtr;
                    /*
                    **        Update Delete Dtm Feature List pointer
                    */
                    if (dtmP->fListDelPtr == dtmP->nullPtr)
                        {
                        flistAddrP (dtmP, listEntry)->nextPtr = dtmP->nullPtr; dtmP->fListDelPtr = listEntry;
                        }
                    else
                        {
                        flistAddrP (dtmP, listEntry)->nextPtr = dtmP->fListDelPtr; dtmP->fListDelPtr = listEntry;
                        }
                    }
            }
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS)  bcdtmWrite_message (0, 0, 0, "Removing Point From Dtm Feature Completed");
    if (dbg && ret != DTM_SUCCESS)  bcdtmWrite_message (0, 0, 0, "Removing Point From Dtm Feature Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_removePointFromAllDtmFeaturesDtmObject (BC_DTM_OBJ *dtmP, long point)
    {
    int  ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long pp, np, flPtr, feat, fpnt, cln, clp, process;
    /*
    ** Check point Has Associated Feature List
    */
    if ((flPtr = nodeAddrP (dtmP, point)->fPtr) != dtmP->nullPtr)
        {
        /*
        **  Scan Feature List And Delete point
        */
        while (flPtr != dtmP->nullPtr)
            {
            np = flistAddrP (dtmP, flPtr)->nextPnt;
            feat = flistAddrP (dtmP, flPtr)->dtmFeature;
            fpnt = ftableAddrP (dtmP, feat)->dtmFeaturePts.firstPoint;
            cln = flistAddrP (dtmP, flPtr)->nextPtr;
            if (dbg) bcdtmWrite_message (0, 0, 0, "Removing Point %8ld From DTM Feature %8ld", point, flistAddrP (dtmP, flPtr)->dtmFeature);
            /*
            **     Remove point
            */
            if (bcdtmList_getPriorPointForDtmFeatureDtmObject (dtmP, feat, point, &pp)) goto errexit;
            if (dbg) bcdtmWrite_message (0, 0, 0, "pp = %8ld np = %8ld", pp, np);
            if (pp != dtmP->nullPnt)
                {
                if ((clp = nodeAddrP (dtmP, pp)->fPtr) == dtmP->nullPtr) goto errexit;
                process = 1;
                while (clp != dtmP->nullPtr && process)
                    {
                    if (flistAddrP (dtmP, clp)->nextPnt == point && flistAddrP (dtmP, clp)->dtmFeature == feat)
                        {
                        flistAddrP (dtmP, clp)->nextPnt = np;
                        process = 0;
                        }
                    clp = flistAddrP (dtmP, clp)->nextPtr;
                    }
                if (process)
                    {
                    bcdtmWrite_message (1, 0, 0, "Error Removing point From Feature List");
                    goto errexit;
                    }
                }
            /*
            **     If point First point In Feature List Set First pointer
            */
            if (fpnt == point)  ftableAddrP (dtmP, feat)->dtmFeaturePts.firstPoint = flistAddrP (dtmP, flPtr)->nextPnt;
            if (ftableAddrP (dtmP, feat)->dtmFeaturePts.firstPoint == dtmP->nullPnt)
                {
                ftableAddrP (dtmP, feat)->dtmFeatureState = DTMFeatureState::Deleted;
                }
            /*
            **     Update Delete Feature List pointer
            */
            if (dtmP->fListDelPtr == dtmP->nullPtr)
                {
                flistAddrP (dtmP, flPtr)->nextPtr = dtmP->nullPtr;
                dtmP->fListDelPtr = flPtr;
                }
            else
                {
                flistAddrP (dtmP, flPtr)->nextPtr = dtmP->fListDelPtr;
                dtmP->fListDelPtr = flPtr;
                }
            /*
            **     Write Feature Points
            */
            if (dbg)
                {
                bcdtmList_writePointsForDtmFeatureDtmObject (dtmP, feat);
                }
            /*
            **     Get Next Feature In List
            */
            flPtr = cln;
            }
        /*
        **   Update List pointer For point
        */
        nodeAddrP (dtmP, point)->fPtr = dtmP->nullPtr;
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_addDtmFeatureToDtmObject
(
 BC_DTM_OBJ            *dtmP,
 BC_DTM_FEATURE        *dtmFeaureP,
 long                  dtmFeatureNum,
 DTMFeatureType dtmFeatureType,
 DTMUserTag            userTag,
 DTMFeatureId          userFeatureId,
 long                  firstPoint,
 long                  clearOption
)
/*
** This Function Adds A DTM Feature To The Tin Component Of A Dtm Object
** Assumes Feature Point List Is Stored In The tPtr Array
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  np,sp,newDtmFeatureNum=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tin") ;
    bcdtmWrite_message(0,0,0,"dtmFeaureP         = %p",dtmFeaureP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureNum      = %8ld",dtmFeatureNum) ;
    bcdtmWrite_message(0,0,0,"userTag            = %8I64d",userTag) ;
    bcdtmWrite_message(0,0,0,"userFeatureId      = %8I64d",userFeatureId) ;
    bcdtmWrite_message(0,0,0,"firstPoint         = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"clearOption        = %8ld",clearOption) ;
   }
 if( dbg == 2 )  bcdtmList_writeTptrListDtmObject(dtmP,firstPoint) ;
/*
** Initialise
*/
 sp = firstPoint   ;
 np = dtmP->nullPnt ;
/*
** Check For More Than One Point In Feature
*/
  if( dtmFeatureType != DTMFeatureType::GroupSpots && nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt )  goto cleanup ;
/*
** Add Entry To Feature Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature Table") ;
 if( bcdtmInsert_addToFeatureTableDtmObject(dtmP,dtmFeaureP,dtmFeatureNum,dtmFeatureType,userTag,userFeatureId,firstPoint,&newDtmFeatureNum)) goto errexit  ;
/*
** Add Entry To Feature List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature List") ;
 if( bcdtmInsert_addToFeatureListDtmObject(dtmP,dtmFeatureType,newDtmFeatureNum,firstPoint,clearOption)) goto errexit  ;
/*
** Write Points For Feature
*/
 if( dbg == 2 ) bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,newDtmFeatureNum) ;
/*
** Check Dtm Feature End Points To Ensure Modified List Structure Is Correct
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points After Adding Dtm Feature %6ld",newDtmFeatureNum) ;
    if( bcdtmCheck_dtmFeatureEndPointsDtmObject(dtmP,cdbg))
      {
       bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
       goto errexit  ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tin Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tin Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_addToFeatureTableDtmObject
(
 BC_DTM_OBJ *dtmP,
 BC_DTM_FEATURE *dtmFeatureP,
 long dtmFeatureNum,
 DTMFeatureType dtmFeatureType,
 DTMUserTag userTag,
 DTMFeatureId userFeatureId,
 long firstPoint,
 long *newDtmFeatureNumP
 )
/*
** This Function Adds A Dtm Feature To The Feature Table
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature Table") ;
/*
** Initialise
*/
 *newDtmFeatureNumP = dtmP->nullPnt ;
/*
** Check For Existing Feature
*/
 if( dtmFeatureP != NULL )
   {
    *newDtmFeatureNumP = dtmFeatureNum ;
    bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
    dtmFeatureP->dtmFeaturePts.offsetPI = 0 ;
   }
/*
** New Feature
*/
 else
   {
/*
** Test For Memory Allocation
*/
    if( dtmP->numFeatures >= dtmP->memFeatures )
      {
       if(bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
     }
/*
**  Set Feature Pointer And Number
*/
    dtmFeatureP        = ftableAddrP(dtmP,dtmP->numFeatures) ;
    *newDtmFeatureNumP = dtmP->numFeatures ;
    ++dtmP->numFeatures ;
   }

/*
** Add Entry To Feature Table
*/
 dtmFeatureP->dtmFeatureState          = DTMFeatureState::Tin ;
 dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint ;
 dtmFeatureP->dtmFeatureType           = (DTMFeatureType)dtmFeatureType ;
 dtmFeatureP->dtmUserTag               = userTag ;
 dtmFeatureP->dtmFeatureId             = userFeatureId ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Feature Table Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_addToFeatureListDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 long dtmFeatureNum,
 long firstPoint,
 long clearOption
)
/*
** This Function Adds A New Feature Entry To The Feature List
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,np,flPtr,nodeFptr,lastFlPtr ;
 DTM_TIN_NODE *spP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Adding Dtm Tptr Feature Points To Feature List") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureNum  = %8ld",dtmFeatureNum) ;
    bcdtmWrite_message(0,0,0,"firstPoint     = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"clearOption    = %8ld",clearOption) ;
   }
/*
** Initialise
*/
 sp  = firstPoint ;
 spP = nodeAddrP(dtmP,sp) ;
/*
** Only Insert If Tptr List Is Not Empty
*/
 if( spP->tPtr != dtmP->nullPnt || dtmFeatureType == DTMFeatureType::GroupSpots )
   {
/*
**  Scan TPtr List and Add Feature List Records
*/
    do
      {
/*
**     Get Deleted Entry From Feature List Table
*/
       if( dtmP->fListDelPtr != dtmP->nullPtr )
         {
          flPtr = dtmP->fListDelPtr ;
          dtmP->fListDelPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
         }
/*
**     Get New Entry From Feature List Table
*/
       else
         {
          if( dtmP->numFlist >= dtmP->memFlist )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Feature List Memory") ;
             if(bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit  ;
            }
          flPtr = dtmP->numFlist ;
         }
/*
**    Add Entry To Feature List
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Adding Entry To Feature List") ;
      flistAddrP(dtmP,flPtr)->nextPnt    = spP->tPtr     ;
      flistAddrP(dtmP,flPtr)->nextPtr    = dtmP->nullPtr ;
      flistAddrP(dtmP,flPtr)->dtmFeature = dtmFeatureNum ;
      flistAddrP(dtmP,flPtr)->pntType = 1 ;
      if( flPtr == dtmP->numFlist ) ++dtmP->numFlist ;
/*
**    Set Node Pointers To Feature List Entry
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Setting Node Pointers To Feature List Entry") ;
      if( ( nodeFptr = nodeAddrP(dtmP,sp)->fPtr) == dtmP->nullPtr )
        {
         nodeAddrP(dtmP,sp)->fPtr = flPtr ;
        }
      else
        {
         lastFlPtr = nodeFptr ;
         while ( lastFlPtr != dtmP->nullPtr )
           {
            nodeFptr = lastFlPtr ;
            lastFlPtr = flistAddrP(dtmP,nodeFptr)->nextPtr ;
           }
         flistAddrP(dtmP,nodeFptr)->nextPtr = flPtr ;
        }
/*
**    Get Next Point
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Feature Point") ;
      np = spP->tPtr ;
      if( clearOption ) spP->tPtr = dtmP->nullPnt ;
      sp = np ;
      if( sp != dtmP->nullPnt ) spP = nodeAddrP(dtmP,sp) ;
     } while( sp != dtmP->nullPnt && sp != firstPoint ) ;
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Tptr Feature Points To Feature List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Tptr Feature Points To Feature List Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_retriangualteAlongBreakLinesDtmObject(BC_DTM_OBJ *dtmP)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,startPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Scan Feature List For Break Lines
*/
 for( dtmFeature = 0  ; dtmFeature <  dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating Break Line %6ld",dtmFeature) ;
       if( dbg && bcdtmCheck_forKnotInDtmFeatureDtmObject(dtmP,dtmFeature)) goto errexit ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt) ) goto errexit  ;
       if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
       if( bcdtmInsert_retriangualteAlongBreakLineDtmObject(dtmP,startPnt) ) goto errexit  ;
       if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit  ;
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
BENTLEYDTM_Public int bcdtmInsert_retriangualteAlongBreakLineDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Retriangulates Along The Tptr List
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,np,ap,ap1,cp,cp1,numswapped,lp,process,loop ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating Break Line ** startPnt = %10ld",startPnt) ;
/*
** Initialise
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Scan Along Tptr List And Check MAX_MIN Criteria
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr != dtmP->nullPnt )
   {
    loop = 5 ;
    numswapped = 1 ;
    while ( numswapped && loop )
      {
       --loop ;
       numswapped = 0 ;
       sp = startPnt ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
       while ( np != dtmP->nullPnt && np != startPnt )
         {
/*
**        Write Break Line Coordinates
*/
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"sp = %6ld  ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z ) ;
             bcdtmWrite_message(0,0,0,"np = %6ld  ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z ) ;
            }
/*
**        Following Code Added 24/3/03 by RobC To Scan Out Past
**        First Triangle Immediately Adjacent To Break Line
*/
          lp = np ;
          if( ( ap  = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
          process = 1 ;
          while ( process )
            {
             process = 0 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"ap = %6ld  ** %10.4lf %10.4lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z ) ;
             if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,ap) )
               {
                if( bcdtmList_testLineDtmObject(dtmP,ap,lp) )
                  {
                   if( ( ap1 = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"ap1 = %6ld  ** %10.4lf %10.4lf %10.4lf",ap1,pointAddrP(dtmP,ap1)->x,pointAddrP(dtmP,ap1)->y,pointAddrP(dtmP,ap1)->z ) ;
                   if( bcdtmList_testLineDtmObject(dtmP,ap1,ap) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,lp,ap1,ap) < 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,lp,ap1,sp) > 0      )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Doing Max Min Test") ;
                         if( bcdtmTin_maxMinTestDtmObject(dtmP,lp,ap1,sp,ap) )
                           {
                            if( bcdtmList_deleteLineDtmObject(dtmP,sp,ap)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,lp,ap1,sp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap1,lp,ap)) goto errexit ;
                            ++numswapped ;
                           }
                         else
                           {
                            process = 1 ;
                            lp = ap ;
                            ap = ap1 ;
                           }
                        }
                     }
                  }
               }
            }

/*
**        Following Code Added 24/3/03 by RobC To Scan Out Past
**        First Triangle Immediately Adjacent To Break Line
*/
          lp = np ;
          if( ( cp  = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
          process = 1 ;
          while ( process )
            {
             process = 0 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"cp = %6ld  ** %10.4lf %10.4lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z ) ;
             if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,cp) )
               {
                if( bcdtmList_testLineDtmObject(dtmP,cp,lp) )
                  {
                   if( ( cp1 = bcdtmList_nextClkDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"cp1 = %6ld  ** %10.4lf %10.4lf %10.4lf",cp1,pointAddrP(dtmP,cp1)->x,pointAddrP(dtmP,cp1)->y,pointAddrP(dtmP,cp1)->z ) ;
                   if( bcdtmList_testLineDtmObject(dtmP,cp1,cp) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,lp,cp1,cp) > 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,lp,cp1,sp) < 0      )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Doing Max Min Test") ;
                         if( bcdtmTin_maxMinTestDtmObject(dtmP,lp,cp1,sp,cp) )
                           {
                            if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,lp,cp1,cp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp1,lp,sp)) goto errexit ;
                            ++numswapped ;
                           }
                         else
                           {
                            process = 1 ;
                            lp = cp ;
                            cp = cp1 ;
                           }
                        }
                     }
                  }
               }
            }
/*
**        Set For Next Break Line Segment
*/
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
/*-------------------------------------------------------------+
|                                                              |
|                                                              |
|                                                              |
+-------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_removeZeroSlopeTrianglesAlongContourLinesDtmObject(BC_DTM_OBJ *dtmP)
{
 int  ret=DTM_SUCCESS ;
 long dtmFeature,startPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Scan Feature List For Break Lines
*/
 for( dtmFeature = 0  ; dtmFeature <  dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine )
      {
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt) ) goto errexit ;
       if( bcdtmInsert_removeZeroSlopeTrianglesAlongTptrListDtmObject(dtmP,startPnt)) goto errexit ;
       if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
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
/*-------------------------------------------------------------+
|                                                              |
|                                                              |
|                                                              |
+-------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_removeZeroSlopeTrianglesAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt )
/*
** This Routine Removes Zero Slope Triangles On The Same Contour Line
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),sideof ;
 long   sp,np,rp,pp,cp,rpn,sfp,efp,processRight,processLeft,loop ;
 long   closeFlag=0,processPoint,numLeft,numRight ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Zero Slope Triangles ** startPnt = %6ld z = %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->z) ;
/*
** Write Contour Line
*/
 if( dbg == 2 )
   {
    sp = startPnt ;
    do
      {
       bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while( sp != startPnt && sp != dtmP->nullPnt ) ;
    if( sp != dtmP->nullPnt )  bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
   }
/*
** Test for Valid Start Point
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto cleanup ;
/*
**  Scan Along Contour Line
**  Note. Code Is Set Up To Do More Than One Pass Along Contour
**  However it appears one pass is sufficient
*/
 loop = 0 ;
 processRight = processLeft = 1 ;
 while ( ( processRight || processLeft ) && loop < 2 )
   {
/*
** Initialise Variables
*/
    sfp = startPnt ;
    efp = nodeAddrP(dtmP,sfp)->tPtr ;
    while( nodeAddrP(dtmP,efp)->tPtr != dtmP->nullPnt && efp != sfp ) efp = nodeAddrP(dtmP,efp)->tPtr ;
/*
** Test For Closure
*/
    if( efp == sfp ) closeFlag = 1 ;
    else             closeFlag = 0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"sfp = %6ld efp = %6ld Closeflag = %1ld",sfp,efp,closeFlag) ;
    sp = sfp ;
    pp = dtmP->nullPnt ;
    if( closeFlag )
      {
       pp = nodeAddrP(dtmP,sp)->tPtr ;
       while( nodeAddrP(dtmP,pp)->tPtr != sp ) if(( pp = bcdtmList_nextAntDtmObject(dtmP,sp,pp)) < 0 ) goto errexit ;
      }
    numRight = numLeft = 0 ;
/*
**  Process All Points On Along Tptr List
*/
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** np = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
/*
**     Look At Lines To The Right Of Sp-Np
*/
       if( processRight )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Processing Right") ;
          processPoint = 1 ;
          while ( processPoint )
            {
             processPoint = 0 ;
             cp = dtmP->nullPnt ;
             rp = np ;
             if( (rpn = bcdtmList_nextClkDtmObject(dtmP,sp,rp)) < 0 )  goto errexit ;
             sideof =  bcdtmMath_linePointSideOfDtmObject(dtmP,sp,np,pointAddrP(dtmP,rpn)->x,pointAddrP(dtmP,rpn)->y) ;
             while ( rpn != pp && sideof < 0 && ( nodeAddrP(dtmP,rpn)->tPtr != dtmP->nullPnt || pointAddrP(dtmP,rpn)->z == pointAddrP(dtmP,startPnt)->z ) )
               {
                cp = rp  ;
                rp = rpn ;
                if( (rpn = bcdtmList_nextClkDtmObject(dtmP,sp,rpn)) < 0 )  goto errexit ;
                sideof =  bcdtmMath_linePointSideOfDtmObject(dtmP,sp,np,pointAddrP(dtmP,rpn)->x,pointAddrP(dtmP,rpn)->y) ;
               }
             if( rpn != pp && sideof < 0 )
               {
                if( bcdtmList_testLineDtmObject(dtmP,rpn,rp) && bcdtmList_testLineDtmObject(dtmP,rp,cp) )
                  {
                   if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,rp) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,rpn,cp,rp) < 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,rpn,cp,sp) > 0   )
                        {
                         ++numRight ;
                         processPoint = 1 ;
                         if( dbg ) bcdtmWrite_message(0,0,0,"Deleting %6ld %6ld ** Inserting %6ld %6ld",sp,rp,cp,rpn) ;
                         if( bcdtmList_deleteLineDtmObject(dtmP,sp,rp)) goto errexit ;
                         if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,rpn,rp)) goto errexit ;
                         if( bcdtmList_insertLineAfterPointDtmObject(dtmP,rpn,cp,sp)) goto errexit ;
                        }
                     }
                  }
               }
            }
         }
/*
**     Look At Lines To The Left Of Sp-Np
*/
       if( processLeft )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Processing Left") ;
          processPoint = 1 ;
          while ( processPoint )
            {
             processPoint = 0 ;
             cp = dtmP->nullPnt ;
             rp = np ;
             if( (rpn = bcdtmList_nextAntDtmObject(dtmP,sp,rp)) < 0 )  goto errexit ;
             if( pp == dtmP->nullPnt ) sideof = bcdtmMath_linePointSideOfDtmObject(dtmP,sp,np,pointAddrP(dtmP,rpn)->x,pointAddrP(dtmP,rpn)->y) ;
             else        sideof = 1 ;
             while ( rpn != pp && sideof > 0 && (nodeAddrP(dtmP,rpn)->tPtr != dtmP->nullPnt || pointAddrP(dtmP,rpn)->z == pointAddrP(dtmP,startPnt)->z ))
               {
                cp = rp  ;
                rp = rpn ;
                if( (rpn = bcdtmList_nextAntDtmObject(dtmP,sp,rpn)) < 0 )  goto errexit ;
                if( pp == dtmP->nullPnt ) sideof = bcdtmMath_linePointSideOfDtmObject(dtmP,sp,np,pointAddrP(dtmP,rpn)->x,pointAddrP(dtmP,rpn)->y) ;
                else        sideof = 1 ;
               }
             if( rpn != pp && sideof > 0 )
               {
                if( bcdtmList_testLineDtmObject(dtmP,rpn,rp) && bcdtmList_testLineDtmObject(dtmP,rp,cp) )
                  {
                   if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,sp,rp) )
                     {
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,rpn,cp,rp) > 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,rpn,cp,sp) < 0   )
                        {
                         ++numLeft ;
                         processPoint = 1 ;
                         if( dbg ) bcdtmWrite_message(0,0,0,"Deleting %6ld %6ld ** Inserting %6ld %6ld",sp,rp,cp,rpn) ;
                         if( bcdtmList_deleteLineDtmObject(dtmP,sp,rp)) goto errexit ;
                         if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,rpn,sp)) goto errexit ;
                         if( bcdtmList_insertLineAfterPointDtmObject(dtmP,rpn,cp,rp)) goto errexit ;
                        }
                     }
                  }
               }
            }
         }

/*
**     Get Next Point On Contour
*/
       pp = sp ;
       sp = np ;
      } while ( sp != sfp && sp != efp ) ;
/*
**   Set Up For Repeat Scan
*/
    processRight = numRight ;
    processLeft  = numLeft ;
/*
**  Looks as If It Only Needs One Pass Is Needed - Code Left In But negated
**  For Future Testing If More Than One Pass is required
**
*/
    processRight = processLeft = 0 ;
/*
**  Reverse Tptr List And Scan Again
*/
    ++loop ;
 //   bcdtmList_reverseTptrListDtmObject(dtmP,&startPnt) ;
   }
/*
** Write Status Message
*/
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Zero Slope Triangles Removed ** startPnt = %6ld z = %10.4lf Completed",startPnt,pointAddrP(dtmP,startPnt)->z) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Zero Slope Triangles Removed ** startPnt = %6ld z = %10.4lf Error",startPnt,pointAddrP(dtmP,startPnt)->z) ;
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

BENTLEYDTM_Public int bcdtmInsert_removeDtmFeatureFromDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Removes A DTM Feature From A Dtm Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  point,firstPoint,numFeatures,numFeaturePts ;
 DPoint3d   *p3dP,*featurePtsP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %6ld From Dtm Object %p",dtmFeature,dtmP) ;
/*
** Check Feature Range
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Remove Feature Depending On Its State
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 switch ( dtmFeatureP->dtmFeatureState )
   {
    case DTMFeatureState::Unused         :
    break ;

    case DTMFeatureState::Data    :
    if( bcdtmInsert_removeFirstPointDtmDataFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
    break ;

    case DTMFeatureState::PointsArray   :
    if( bcdtmInsert_removePointsArrayDtmDataFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
    break ;

    case DTMFeatureState::OffsetsArray  :
    if( bcdtmInsert_removeOffsetsArrayDtmDataFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
    break ;

    case DTMFeatureState::Tin            :
    if( bcdtmList_copyDtmFeatureToSptrListDtmObject(dtmP,dtmFeature,&firstPoint)) goto errexit ;
    point = firstPoint ;
    do
      {
        if( bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,point,&numFeatures)) goto errexit ;
        if( numFeatures == 1 ) bcdtmFlag_setInsertPoint(dtmP,point) ;
        point = nodeAddrP(dtmP,point)->sPtr ;
      } while( point != firstPoint && point != dtmP->nullPnt ) ;
    if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
    if( bcdtmList_nullSptrListDtmObject(dtmP,firstPoint)) goto errexit ;


    break ;

    case DTMFeatureState::TinError    :
      if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
      for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
        {
         bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&point) ;
         if( bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,point,&numFeatures)) goto errexit ;
         if( numFeatures == 0 ) bcdtmFlag_setInsertPoint(dtmP,point) ;
        }

      if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
        {
         bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
         dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
        }
      dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
    break ;

    case DTMFeatureState::Rollback     :
      if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
      for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
        {
         bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&point) ;
         if( bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,point,&numFeatures)) goto errexit ;
         if( numFeatures == 0 ) bcdtmFlag_setInsertPoint(dtmP,point) ;
        }

      if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
        {
         bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
         dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
        }
      dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
    break ;

    case DTMFeatureState::Deleted        :
    break ;

  } ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_removeFirstPointDtmDataFeatureFromDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature)
/*
** This Function Removes A DTM Feature From A DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  feature,numPoints,firstPoint,lastPoint ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *firstPointP,*lastPointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing First Point Dtm Feature %6ld",dtmFeature) ;
/*
** Check Feature Exists
*/
 if( dtmP->fTablePP == NULL || dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Feature Does Not Exist In Tin Object") ;
    goto errexit ;
   }
/*
** Remove Dtm Feature Only If It Exists
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
   {
    firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
    lastPoint  = firstPoint + dtmFeatureP->numDtmFeaturePts ;
    while ( lastPoint < dtmP->numPoints )
      {
       firstPointP = pointAddrP(dtmP,firstPoint) ;
       lastPointP  = pointAddrP(dtmP,lastPoint) ;
       *firstPointP = *lastPointP ;
       ++firstPoint ;
       ++lastPoint  ;
      }
/*
**  Reset Header values
*/
    numPoints = dtmFeatureP->numDtmFeaturePts ;
    dtmP->numPoints = dtmP->numPoints - numPoints ;
/*
**  Mark Feature As Deleted
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Dtm Feature As Deleted") ;
    dtmFeatureP->dtmFeaturePts.firstPoint = dtmP->nullPnt ;
    dtmFeatureP->numDtmFeaturePts = 0 ;
    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
/*
**  Reset Lower Feature First Points
*/
    for( feature = dtmFeature + 1 ; feature < dtmP->numFeatures ; ++feature )
      {
       dtmFeatureP = ftableAddrP(dtmP,feature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          dtmFeatureP->dtmFeaturePts.firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint - numPoints ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing First Point Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing First Point Dtm Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Public int bcdtmInsert_removePointsArrayDtmDataFeatureFromDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Removes A DTM Feature From A DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Points Array Dtm Feature %6ld",dtmFeature) ;
/*
** Check Feature Exists
*/
 if( dtmP->fTablePP == NULL || dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Feature Does Not Exist In Tin Object") ;
    goto errexit ;
   }
/*
** Remove Dtm Feature Only If It Exists
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
   {
    if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
      {
       bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
       dtmFeatureP->dtmFeaturePts.pointsPI = 0;
      }
    dtmFeatureP->numDtmFeaturePts = 0 ;
    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Points Array Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Points Array Dtm Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Public int bcdtmInsert_removeOffsetsArrayDtmDataFeatureFromDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Removes A DTM Feature From A DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Offsets Array Dtm Feature %6ld",dtmFeature) ;
/*
** Check Feature Exists
*/
 if( dtmP->fTablePP == NULL || dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Feature Does Not Exist In Tin Object") ;
    goto errexit ;
   }
/*
** Remove Dtm Feature Only If It Exists
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
   {
    if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
      {
       bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
       dtmFeatureP->dtmFeaturePts.pointsPI = 0;
      }
    dtmFeatureP->numDtmFeaturePts = 0 ;
    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Points Array Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Points Array Dtm Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Public int bcdtmInsert_removeDtmTinFeatureFromDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Removes A DTM Feature From A Tin Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  firstPnt,nextPnt,flPtr,lastFlPtr,numFeatures ;
 BC_DTM_FEATURE *dtmFeatureP ;
 long  numPtsMarked=0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %6ld",dtmFeature) ;
/*
** Report Feature List States
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing dtmFeature %8ld From Feature List Table",dtmFeature) ;
/*
** Check Feature Exists
*/
 if( dtmP->fTablePP == NULL || dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Feature Does Not Exist In Tin Object") ;
    goto errexit ;
   }
/*
** Remove Dtm Hull Feature Only If It Exists
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
 if     ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Hull Feature ** FeatureId = %10I64d",dtmFeatureP->dtmFeatureId) ;
    firstPnt = dtmP->hullPoint ;
    do
      {
/*
**     Count Number Of Point Features
*/
       numFeatures = 0 ;
       flPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
       while( flPtr != dtmP->nullPtr )
         {
          ++numFeatures ;
          flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
         }
/*
**     Mark Point For Latter Deletion
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"numFeatures = %6ld",numFeatures) ;
       if( numFeatures == 0 ) bcdtmFlag_setDeletePointBitPCWD(&nodeAddrP(dtmP,firstPnt)->PCWD) ;
/*
**     Get Next Point On Tin Hull
*/
       firstPnt = nodeAddrP(dtmP,firstPnt)->hPtr ;
      } while ( firstPnt != dtmP->hullPoint ) ;
/*
**  Mark Hull Feature As Deleted
*/
    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
   }
/*
** Remove None Dtm Hull Features
*/
else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
   {
    numPtsMarked = 0 ;
/*
**  Mark Feature Only Points
*/
    nextPnt = firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Feature Point = %8ld ** %12.5lf %12.5lf %10.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z) ;
/*
**     Count Number Of Point Features
*/
       numFeatures = 0 ;
       flPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
       while( flPtr != dtmP->nullPtr )
         {
          ++numFeatures ;
          flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"numFeatures At Point = %8ld",numFeatures) ;
/*
**     Mark Point For Latter Deletion
*/
       if( numFeatures == 1 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking For Delete ** Feature Point[%8ld] ** %12.5lf %12.5lf %10.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z) ;
          bcdtmFlag_setDeletePointBitPCWD(&nodeAddrP(dtmP,nextPnt)->PCWD) ;
          ++numPtsMarked ;
        }
/*
**     Get Next Point For Feature
*/
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,nextPnt,&nextPnt)) goto errexit ;
      } while( nextPnt != firstPnt && nextPnt != dtmP->nullPnt ) ;

/*
** Initialise
*/
    nextPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
    flPtr   = nodeAddrP(dtmP,nextPnt)->fPtr ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"00 nextPnt = %8ld ** flPtr = %8ld",nextPnt,flPtr) ;
/*
** Scan Feature Points And Remove From Point Feature Table
*/
    while ( flPtr != dtmP->nullPtr )
      {
       lastFlPtr = flPtr ;
       while ( flPtr != dtmP->nullPtr  && flistAddrP(dtmP,flPtr)->dtmFeature != dtmFeature )
         {
          lastFlPtr = flPtr ;
          flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
         }
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"flPtr = %8ld",flPtr) ;
       if( flPtr != dtmP->nullPtr )
         {
          flistAddrP(dtmP,flPtr)->dtmFeature = dtmP->nullPnt ;
          if( flPtr == nodeAddrP(dtmP,nextPnt)->fPtr ) nodeAddrP(dtmP,nextPnt)->fPtr  = flistAddrP(dtmP,flPtr)->nextPtr ;
          else                                         flistAddrP(dtmP,lastFlPtr)->nextPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
          if( dtmP->fListDelPtr == dtmP->nullPtr ) { dtmP->fListDelPtr = flPtr ; flistAddrP(dtmP,flPtr)->nextPtr = dtmP->nullPtr ; }
          else                                     { flistAddrP(dtmP,flPtr)->nextPtr = dtmP->fListDelPtr ; dtmP->fListDelPtr = flPtr ; }
/*
**        Get Next Feature Point
*/
          nextPnt  = flistAddrP(dtmP,flPtr)->nextPnt ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"01 nextPnt = %8ld",nextPnt) ;
          if( nextPnt != dtmP->nullPnt ) flPtr = nodeAddrP(dtmP,nextPnt)->fPtr  ;
          else                           flPtr = dtmP->nullPtr  ;
         }
      }


/*
** Mark Feature As Deleted
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Dtm Feature As Deleted") ;
    dtmFeatureP->dtmFeaturePts.firstPoint = dtmP->nullPnt ;
    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Feature Points Marked For Delete = %8ld of %8ld",numPtsMarked,dtmP->numPoints) ;
   }
/*
** Check Dtm Feature End Points To Ensure Modified List Structure Is Correct
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points After Removing Dtm Feature %6ld",dtmFeature) ;
    if( bcdtmCheck_dtmFeatureEndPointsDtmObject(dtmP,0))
      {
       bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
       goto errexit  ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_EXPORT int bcdtmInsert_internalStringIntoDtmObject
(
 BC_DTM_OBJ *dtmP,        /* ==> Pointer To DTM Object     */
 long drapeOption,        /* ==> Drape Option              */
 long insertOption,       /* ==> Insert Option             */
 DPoint3d *stringPtsP,         /* ==> Pointer To String Points  */
 long numStringPts,       /* ==> Number Of String Points   */
 long *startPntP          /* <== Start Point Of Tptr List  */
)
/*
**
** This Function Inserts A String Into A Tptr List Of A Dtm Object
** Assumes String Is Internal To Tin And Has Been Validated
**
** drapeOption    = 1   Drape Intersect Vertices On Tin Surface
**                = 2   Break Intersect Vertices On Tin Surface
** insertOption   = 1   Move Tin Lines That Are Not Linear Features
**                = 2   Intersect Tin Lines
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  dtmPntNum,*pntP,*pointNumP=NULL,insert,saveIncPoints ;
 DPoint3d   *p3d ;
 long  startTime=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Internal String Into Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drapeOption           = %8ld",drapeOption) ;
    bcdtmWrite_message(0,0,0,"insertOption          = %8ld",insertOption) ;
    bcdtmWrite_message(0,0,0,"stringPtsP            = %p",stringPtsP) ;
    bcdtmWrite_message(0,0,0,"numStringPts          = %8ld",numStringPts) ;
    bcdtmWrite_message(0,0,0,"startPntP             = %8ld",*startPntP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->incPoints       = %8ld",dtmP->incPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol           = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol           = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol          = %20.15lf",dtmP->mppTol) ;
    if( dbg == 2 )
      {
       for( p3d = stringPtsP ; p3d < stringPtsP + numStringPts ; ++p3d )
         {
          bcdtmWrite_message(0,0,0,"Point[%4ld] = %10.4lf %10.4lf %8.4lf",(long)(p3d-stringPtsP),p3d->x,p3d->y,p3d->z) ;
         }
      }
   }
/*
** Set Dtm Point Increment Memory To Four Times The Number Of String Points
*/
 saveIncPoints = dtmP->incPoints ;
 if( dtmP->incPoints < numStringPts * 4 ) dtmP->incPoints = numStringPts * 4 ;
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Store String Points In Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting String Points") ;
 startTime = bcdtmClock() ;
 for( p3d = stringPtsP ; p3d < stringPtsP + numStringPts ; ++p3d )
   {
    if( dbg == 1 )  bcdtmWrite_message(0,0,0,"Inserting Point[%4ld] ** %10.4lf %10.4lf %8.4lf",(long)(p3d-stringPtsP),p3d->x,p3d->y,p3d->z) ;
    if( bcdtmInsert_storePointInDtmObject(dtmP,drapeOption,insertOption,p3d->x,p3d->y,p3d->z,&dtmPntNum)) goto errexit ;
    if( dtmP->numPoints - dtmP->numSortedPoints > 1500 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Resorting Points") ;
       if( bcdtmTin_resortTinStructureDtmObject(dtmP) ) goto errexit  ;
      }
/*
**  Check Tin Precision - After Inserting Point
*/
    if( cdbg == 2 && dtmPntNum >= dtmP->numSortedPoints )
      {
       bcdtmWrite_message(0,0,0,"Checking Tin After Inserting String Point") ;
       if( bcdtmCheck_precisionDtmObject(dtmP,0))
         {
          bcdtmWrite_message(0,0,0,"Tin Invalid After Inserting Point %8ld",(long)(p3d-stringPtsP)) ;
          goto errexit ;
         }
       bcdtmWrite_message(0,0,0,"Tin OK") ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"String Points Inserted") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Time To Insert String Points = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Check Tin Precision - After Inserting Point
*/
 if( cdbg && dtmP->numPoints > dtmP->numSortedPoints )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Precision After Inserting String Points") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0))
      {
       bcdtmWrite_message(0,0,0,"Tin Precision Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
   }
/*
** Allocate Memory To Hold Point Numbers
*/
 pointNumP = ( long * ) malloc ( numStringPts * sizeof(long)) ;
 if( pointNumP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Get Point Numbers
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Point Numbers") ;
 startTime = bcdtmClock() ;
 for( p3d = stringPtsP , pntP = pointNumP ; p3d < stringPtsP + numStringPts ; ++p3d , ++pntP)
   {
    bcdtmFind_closestPointDtmObject(dtmP,p3d->x,p3d->y,&dtmPntNum) ;
    *pntP = dtmPntNum ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Time To Get Point Numbers = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Tin - Development Only
*/
 if( cdbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin After Inserting String Points") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin OK") ;
   }
/*
** Check For Memory Reallocation
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit  ;
/*
** Store Lines In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting String Segments") ;
 startTime = bcdtmClock() ;
 for( pntP = pointNumP + 1 ; pntP < pointNumP + numStringPts ; ++pntP )
   {
    if( *(pntP-1) != *pntP )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Segment %6ld of %6ld ** From %8ld %8ld",(long)(pntP-pointNumP),numStringPts-1,*(pntP-1),*pntP) ;
       if( ( insert = bcdtmInsert_lineBetweenPointsDtmObject(dtmP,*(pntP-1),*pntP,drapeOption,insertOption)) != DTM_SUCCESS )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Insert Line = %8ld %8ld Insert Error = %6ld",*(pntP-1),*pntP,insert) ;
          ret = 2 ;
          goto errexit ;
         }
/*
**     Check Tin - Development Only
*/
       if( cdbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Checking Tin While Inserting String Segments") ;
          if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ;
          bcdtmWrite_message(0,0,0,"Tin OK") ;
         }
      }
   }

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Lines Inserted") ;
    bcdtmWrite_message(0,0,0,"**** Time To Insert Lines = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"**** dtmP->numSortedPoints = %6ld dtmP->numPoints = %6ld",dtmP->numSortedPoints,dtmP->numPoints) ;
   }
/*
** Check Tin - Development Only
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin After Inserting String") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin OK") ;
   }
/*
** Set Start Point
*/
 *startPntP = *pointNumP ;
/*
** Free memory
*/
 cleanup :
 dtmP->incPoints = saveIncPoints ;
 if( pointNumP != NULL ) free(pointNumP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Internal String Into Dtm Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Internal String Into Dtm Object Error") ;
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
BENTLEYDTM_Public int  bcdtmInsert_storePointInDtmObject
(
 BC_DTM_OBJ *dtmP,
 long drapeOption,
 long internalPoint,
 double x,
 double y,
 double z,
 long *dtmPointNumP
)
/*
** This Function Stores A Point In The Tin Object
**
** drapeOption   = 1   Drape Intersect Vertices On Tin Surface
**               = 2   Break Intersect Vertices On Tin Surface
** internalPoint = 1   Insert Point Is Internal To Tin
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   findType,antPnt,clkPnt,pnt1,pnt2,pnt3,dtmPoint,voidPoint ;
 long   onLine1,onLine2,onLine3,fixType,precisionError ;
 double surfaceZ=0,d1,d2,d3,Xi,Yi ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Point Into Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drapeOption   = %8ld",drapeOption) ;
    bcdtmWrite_message(0,0,0,"internalPoint = %8ld",internalPoint) ;
    bcdtmWrite_message(0,0,0,"x          = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y          = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"z          = %12.5lf",z) ;
   }
/*
** Initialise
*/
 *dtmPointNumP = dtmP->nullPnt ;
/*
** Find Triangle For Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Point x = %10.4lf y = %10.4lf",x,y) ;
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&surfaceZ,&findType,&pnt1,&pnt2,&pnt3)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"x = %10.4lf y = %10.4lf ** findType = %2ld pnt1 = %6ld pnt2 = %9ld pnt3 = %9ld",x,y,findType,pnt1,pnt2,pnt3) ;
/*
** If Insert Point Is External And It Is Not Internal Point Check Point is Beyond ppTol and plTol ;
*/
 if( findType == 0  && ! internalPoint)
   {
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,x,y,&surfaceZ,&findType,&pnt1,&pnt2)) goto errexit ;
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    if   ( d1 <= d2 && d1 < dtmP->ppTol ) { findType = 1 ; pnt2 = pnt3 = dtmP->nullPnt ; }
    if   ( d2 <= d1 && d2 < dtmP->ppTol ) { findType = 1 ; pnt1 = pnt2 ; pnt2 = pnt3 = dtmP->nullPnt ; }
    if( findType == 2 )
      {
       d1 = bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt1,pnt2,x,y) ;
       if( d1 <= dtmP->plTol ) findType = 3 ;
       else                    findType = 0 ;
      }
/*
**  If Not Found With Tolerance To Tin Hull Then Insert Point Can Be Considered An External Point
*/
    if( ! findType )
      {
       ret = 2 ;
       goto errexit ;
      } ;
   }
/*
** If Insert Point Is Internal And Its Find Type Is External Then Find The Closest Hull Line
*/
 if( findType == 0 && internalPoint )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Line To Internal Point") ;
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,x,y,&surfaceZ,&findType,&pnt1,&pnt2)) goto errexit ;
    if( findType == 2 ) findType = 3 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"00 findType = %6ld",findType) ;
/*
** Check Point To Point Tolerances Triangle
*/
 if( findType == 4 && dtmP->ppTol > 0.0 )
   {
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    d3 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
    if      ( d1 <= d2 && d1 <= d3 && d1 < dtmP->ppTol ) { findType = 1 ; pnt2 = pnt3 = dtmP->nullPnt ; }
    else if ( d2 <= d3 && d2 <= d1 && d2 < dtmP->ppTol ) { findType = 1 ; pnt1 = pnt2 ; pnt2 = pnt3 = dtmP->nullPnt ; }
    else if ( d3 <= d1 && d3 <= d2 && d3 < dtmP->ppTol ) { findType = 1 ; pnt1 = pnt3 ; pnt2 = pnt3 = dtmP->nullPnt ; }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"01 findType = %6ld",findType) ;
/*
** Check Point To Line Tolerances Triangle
*/
 if( findType == 4 && dtmP->plTol > 0.0 )
   {
    d1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,x,y,&Xi,&Yi) ;
    d2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,x,y,&Xi,&Yi) ;
    d3 = bcdtmMath_distanceOfPointFromLine(&onLine3,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,x,y,&Xi,&Yi) ;
    if     ( onLine1 && d1 <= d2 && d1 <= d3 && d1 < dtmP->plTol ) { findType = 2 ; pnt3 = dtmP->nullPnt ; }
    else if( onLine2 && d2 <= d3 && d2 <= d1 && d2 < dtmP->plTol ) { findType = 2 ; pnt1 = pnt2 ; pnt2 = pnt3 ; pnt3 = dtmP->nullPnt ; }
    else if( onLine3 && d3 <= d1 && d3 <= d2 && d3 < dtmP->plTol ) { findType = 2 ; pnt2 = pnt1 ; pnt1 = pnt3 ; pnt3 = dtmP->nullPnt ; }
    if( findType == 2 )
      {
       bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,x,y,&Xi,&Yi) ;
       x = Xi ; y = Yi ;
       if      ( nodeAddrP(dtmP,pnt1)->hPtr == pnt2 )   findType = 3 ;
       else if ( nodeAddrP(dtmP,pnt2)->hPtr == pnt1 ) { findType = 3 ; pnt3 = pnt1 ; pnt1 = pnt2 ; pnt2 = pnt3 ; pnt3 = dtmP->nullPnt ; }
      }
    if( findType != 4 )
      {
       bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&surfaceZ,pnt1,pnt2) ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"02 findType = %6ld",findType) ;
/*
** Move Points Within Point To Line Tolerance On To Line
*/
 if( findType == 2 || findType == 3 )
   {
    d1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,x,y,&Xi,&Yi) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine1 = %2ld ** d1 = %20.15lf ** x = %20.15lf y = %20.15lf ** Xi = %20.15lf Yi = %20.15lf",onLine1,d1,x,y,Xi,Yi) ;
    x = Xi ;
    y = Yi ;
   }
/*
** Check Point To Point Tolerance With Line End Points
*/
 if( findType == 2 || findType == 3 )
   {
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    if      ( d1 <= d2 && d1 < dtmP->ppTol ) { findType = 1 ; pnt2 = pnt3 = dtmP->nullPnt ; }
    else if ( d2 <= d1 && d2 < dtmP->ppTol ) { findType = 1 ; pnt1 = pnt2 ; pnt2 = pnt3 = dtmP->nullPnt ; }
   }
/*
** Check For Precision Problem Occurring If Point Is Inserted Into Internal Line
*/
 if( findType == 2 )
   {
    if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,pnt1,pnt2,x,y,&precisionError,&antPnt,&clkPnt)) goto errexit ;
    if( precisionError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Detected While Inserting Point Onto Line") ;
       if( bcdtmInsert_fixPointQuadrilateralPrecisionDtmObject(dtmP,antPnt,pnt2,clkPnt,pnt1,x,y,&x,&y,&fixType))  goto errexit ;
       if( fixType )
         {
          if( fixType == 1 ) { findType = 1 ; pnt1 = pnt2 ; }
          if( fixType == 2 )   findType = 1 ;
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
 if( findType == 3 )
   {
    if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
    if( bcdtmInsert_checkPointHullTrianglePrecisionDtmObject(dtmP,pnt1,pnt2,pnt3,x,y,&precisionError) ) goto errexit ;
    if( precisionError )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Potential Precision Error Detected For Inserting Point On Hull Line") ;
       if( bcdtmInsert_fixPointHullTrianglePrecisionDtmObject(dtmP,pnt1,pnt2,pnt3,x,y,&x,&y,&fixType )) goto errexit ;
       if( ! fixType ) goto errexit ;
      }
   }
/*
** Set z value For Point
*/
 if( findType && drapeOption == 1 ) z = surfaceZ ;
/*
** Add Point To Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Insert Point findType = %6ld",findType) ;
 if( findType > 1 ) { if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&dtmPoint)) goto errexit ; }
 else               dtmPoint = pnt1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Insert Point = %8ld",dtmPoint) ;
/*
** Null Void Point
*/
 voidPoint = 0 ;
/*
**  Insert Point Into Tin
*/
 switch ( findType )
   {
    case  0 :      /* Point External To Tin             */
      if( internalPoint)
        {
         bcdtmWrite_message(1,0,0,"Point %12.4lf %12.4lf %10.4lf External To Tin",x,y,z) ;
         goto errexit ;
        }
    break   ;

    case  1 :      /* Coincident With Existing Point     */
      dtmPoint = pnt1 ;
      pointAddrP(dtmP,dtmPoint)->z = z ;
    break   ;

    case  2 :      /* Coincident With Internal Tin Line  */

      bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,&voidPoint) ;
      if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
      if( (clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
      if(bcdtmList_deleteLineDtmObject(dtmP,pnt1,pnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,dtmPoint,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt2,dtmPoint,clkPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt2,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPoint,pnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,antPnt,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,clkPnt,dtmPoint,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,clkPnt,pnt2)) goto errexit ;
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,pnt1,pnt2) )
         {
          if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,pnt1,pnt2,dtmPoint)) goto errexit ;
         }
    break ;

    case  3 :      /* Coincident With External Tin Line  */
      bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,&voidPoint) ;
      if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2))   < 0 ) goto errexit ;
      if(bcdtmList_deleteLineDtmObject(dtmP,pnt1,pnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,dtmPoint,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineBeforePointDtmObject(dtmP,pnt2,dtmPoint,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt2,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPoint,pnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,antPnt,pnt1)) goto errexit ;
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,pnt1,pnt2) )
        {
         if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,pnt1,pnt2,dtmPoint)) goto errexit ;
        }
      nodeAddrP(dtmP,pnt1)->hPtr = dtmPoint ;
      nodeAddrP(dtmP,dtmPoint)->hPtr = pnt2 ;
    break ;

    case  4 :   /* In Triangle                      */
      if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,&voidPoint)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,dtmPoint,pnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt2,dtmPoint,pnt3)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt2,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt3,dtmPoint,pnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPoint,pnt3,pnt2)) goto errexit ;
    break ;

    default :
      bcdtmWrite_message(2,0,0,"Illegal Point Find Code %6ld ",findType) ;
      goto errexit ;
    break   ;
   } ;
/*
** If Point In Void Set Void Bit
*/
 if( voidPoint ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,dtmPoint)->PCWD) ;
/*
** Set Point Value For Return
*/
 *dtmPointNumP = dtmPoint ;
/*
** Free memory
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Point Into Dtm Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Point Into Dtm Object Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkPointCanBeMovedDtmObject(BC_DTM_OBJ *dtmP,long point,double x,double y,long *moveFlagP)
{
 long p1,p2,sp,clPtr ;
/*
** Initialise
*/
 *moveFlagP = 1 ;
 if( (clPtr = nodeAddrP(dtmP,point)->cPtr) == dtmP->nullPtr ) return(DTM_SUCCESS) ;
 sp = p1  = clistAddrP(dtmP,clPtr)->pntNum ;
 clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    p2    = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    if( nodeAddrP(dtmP,p2)->hPtr != point )
      {
       if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) >= 0 )
         {
          *moveFlagP = 0  ;
          return(DTM_SUCCESS) ;
         }
      }
    p1 = p2 ;
   }
 if( nodeAddrP(dtmP,sp)->hPtr != point )
   {
    if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,x,y) >= 0 )
      {
       *moveFlagP = 0  ;
       return(DTM_SUCCESS) ;
      }
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
BENTLEYDTM_Public int bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,double x,double y,long *precisionError,long *antPt,long *clockPt)
/*
** Check For A Precision Error Occurring If The Point XY Is
** Inserted Into The Tin Line  P1P2
*/
{
 int  ret=DTM_SUCCESS,s1,s2,dbg=DTM_TRACE_VALUE(0) ;
 long ap,cp ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Point Line For Precision Error") ;
    bcdtmWrite_message(0,0,0,"==== ** x = %12.5lf y = %12.5lf",x,y) ;
    bcdtmWrite_message(0,0,0,"==== P1 = %8ld ** %12.5lf %12.5lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"==== P2 = %8ld ** %12.5lf %12.5lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
   }
/*
** Initialise
*/
 *precisionError = 0 ;
 ap = cp = dtmP->nullPnt ;
 if(( ap = bcdtmList_nextAntDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
 if(( cp = bcdtmList_nextClkDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,ap,P2)) ap = dtmP->nullPnt ;
 if( ! bcdtmList_testLineDtmObject(dtmP,cp,P2)) cp = dtmP->nullPnt ;
 if( dbg )
   {
    if( ap != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"==== ap = %8ld ** %12.5lf %12.5lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z) ;
    else                      bcdtmWrite_message(0,0,0,"==== ap = %8ld",ap) ;
    if( cp != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"==== cp = %8ld ** %12.5lf %12.5lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z) ;
    else                      bcdtmWrite_message(0,0,0,"==== cp = %8ld",cp) ;
   }
 if( ap != cp )
   {
    if( ap != dtmP->nullPnt ) if( bcdtmMath_sideOf(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y) >= 0 ) *precisionError = 1 ;
    if( ap != dtmP->nullPnt ) if( bcdtmMath_sideOf(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y) <= 0 ) *precisionError = 2 ;
    if( cp != dtmP->nullPnt ) if( bcdtmMath_sideOf(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y) <= 0 ) *precisionError = 3 ;
    if( cp != dtmP->nullPnt ) if( bcdtmMath_sideOf(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y) >= 0 ) *precisionError = 4 ;
    *antPt = ap ; *clockPt = cp ;
   }
 else
   {
    s1 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
    s2 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
    if( s1 == 0 || s2 == 0 || s1 == s2 ) *precisionError = 1 ;
    *antPt = ap ; *clockPt = cp ;
   }
/*
** Write Result Of Precision Check
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Precision Check = %2ld",*precisionError) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Line For Precision Error Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Point Line For Precision Error Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_checkPointTriangleLinePrecisionDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double x,double y,long *precisionError)
/*
** Check For A Precision Error Occurring If The Point XY Is
** Inserted On Line P1-P2 Of Triangle P1P2P3
** P1P2P3 Must Be In An Anticlockwise Direction
** This Function Should Be Called Prior To Inserting A Point
** Into A Hull Line
*/
{
/*
** Initialise
*/
 *precisionError = 0 ;
/*
** Check Side Of Conditions
*/
 if     ( bcdtmMath_allSideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,x,y) >= 0 ) *precisionError = 1 ;
 else if( bcdtmMath_allSideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,x,y) <= 0 ) *precisionError = 1 ;
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
BENTLEYDTM_Public int bcdtmInsert_triangulateAboutPointDtmObject(BC_DTM_OBJ *dtmP,long point)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p1,p2,p3,listEntry,process ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating About Point %10ld",point) ;
/*
** Retriangulate About Point
*/
 process = TRUE ;
 while( process == TRUE )
   {
    process = FALSE ;
    if( ( listEntry = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
      {
       p2 = clistAddrP(dtmP,listEntry)->pntNum ;
       if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,point,p2)) < 0 ) goto errexit ;
       listEntry = clistAddrP(dtmP,listEntry)->nextPtr ;
       while ( listEntry != dtmP->nullPtr  )
         {
          p3  = clistAddrP(dtmP,listEntry)->pntNum ;
          listEntry = clistAddrP(dtmP,listEntry)->nextPtr ;
          if( nodeAddrP(dtmP,p2)->hPtr != point && nodeAddrP(dtmP,point)->hPtr != p2 &&
              nodeAddrP(dtmP,p2)->tPtr != point && nodeAddrP(dtmP,point)->tPtr != p2     )
            {
             if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,point,p2))
               {
                if( bcdtmMath_pointSideOfDtmObject(dtmP,p3,p1,point) > 0 && bcdtmMath_pointSideOfDtmObject(dtmP,p3,p1,p2) < 0 )
                  {
                   if( bcdtmTin_maxMinTestDtmObject(dtmP,p1,p3,p2,point))
                     {
                      if( bcdtmList_deleteLineDtmObject(dtmP,point,p2)) goto errexit ;
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,p1,point)) goto errexit ;
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,p3,p2)) goto errexit ;
                      process = TRUE ;
                      p2 = p3 ;
                      if( listEntry != dtmP->nullPtr )
                        {
                         p3  = clistAddrP(dtmP,listEntry)->pntNum ;
                         listEntry = clistAddrP(dtmP,listEntry)->nextPtr ;
                        }
                     }
                  }
               }
            }
          p1 = p2 ;
          p2 = p3 ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating About Point %10ld Completed",point) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating About Point %10ld Error",point) ;
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
BENTLEYDTM_Private int bcdtmInsert_scanPointForPointAngleIntersectionWithTriangleBaseDtmObject(BC_DTM_OBJ *dtmP,long pnt,long anglePnt,long *basePnt1P,long *basePnt2P )
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,clPtr ;
 double angle,angP1,angP2,angP3 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Angle Intersection With Triangle Base") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pnt         = %8ld",pnt) ;
    bcdtmWrite_message(0,0,0,"anglePnt    = %8ld",anglePnt) ;
   }
/*
** Initialise
*/
 *basePnt1P = dtmP->nullPnt ;
 *basePnt2P = dtmP->nullPnt ;
/*
** Get Angle
*/
 angle = bcdtmMath_getPointAngleDtmObject(dtmP,pnt,anglePnt) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Point Angle = %12.10lf",angle) ;
/*
** Scan Circular List For Point
*/
 clPtr = nodeAddrP(dtmP,pnt)->cPtr ;
 if(( p2 = bcdtmList_nextAntDtmObject(dtmP,pnt,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
 angP2   = bcdtmMath_getPointAngleDtmObject(dtmP,pnt,p2) ;
 while( clPtr != dtmP->nullPtr && *basePnt1P == dtmP->nullPnt )
   {
    p1    = clistAddrP(dtmP,clPtr)->pntNum  ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    angP1 = bcdtmMath_getPointAngleDtmObject(dtmP,pnt,p1) ;
    if( nodeAddrP(dtmP,p1)->hPtr != pnt )
      {
       if( angP2 < angP1 ) angP2 += DTM_2PYE ;
       angP3 = angle ;
       if( angP3 < angP1 ) angP3 += DTM_2PYE ;
       if( angP3 >= angP1 && angP3 <= angP2 )
         {
          if     ( angP3 == angP1 ) *basePnt1P = p1 ;
          else if( angP3 == angP2 ) *basePnt1P = p2 ;
          else
            {
             *basePnt1P = p2 ;
             *basePnt2P = p1 ;
            }
         }
      }
/*
** Set For Next Triangle Base
*/
    p2    = p1 ;
    angP2 = angP1 ;
   }
/*
** Write Triangle Base points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"basePnt1 = %9ld ** basePnt2 = %9ld",*basePnt1P,*basePnt2P) ;
    if( *basePnt1P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"basePnt1 = %9ld ** %12.5lf %12.5lf %10.4lf",*basePnt1P,pointAddrP(dtmP,*basePnt1P)->x,pointAddrP(dtmP,*basePnt1P)->y,pointAddrP(dtmP,*basePnt1P)->z) ;
    if( *basePnt2P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"basePnt2 = %9ld ** %12.5lf %12.5lf %10.4lf",*basePnt2P,pointAddrP(dtmP,*basePnt2P)->x,pointAddrP(dtmP,*basePnt2P)->y,pointAddrP(dtmP,*basePnt2P)->z) ;
    if( *basePnt1P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"angle(Pnt-BasePnt1) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,pnt,*basePnt1P));
                                      bcdtmWrite_message(0,0,0,"angle(Pnt-anglePnt) = %12.10lf",angle) ;
    if( *basePnt2P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"angle(Pnt-BasePnt2) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,pnt,*basePnt2P));
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Angle Intersection With Triangle Base Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Angle Intersection With Triangle Base Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_removeSliverTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3 )
/*
** This Function Removes A Sliver Triangle
** Assumes P1->P2->P3 Clockwise
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,insertLine,lineDeleted=FALSE ;
 double d1,d2,d3 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Sliver Triangle") ;
    bcdtmWrite_message(0,0,0,"P1 = %8ld ** %12.5lf %12.5lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %8ld ** %12.5lf %12.5lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    bcdtmWrite_message(0,0,0,"P3 = %8ld ** %12.5lf %12.5lf %10.4lf",P3,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z) ;
   }
/*
** Initialise
*/
 d1 = bcdtmMath_pointDistanceDtmObject(dtmP,P1,P2) ;
 d2 = bcdtmMath_pointDistanceDtmObject(dtmP,P2,P3) ;
 d3 = bcdtmMath_pointDistanceDtmObject(dtmP,P3,P1) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Length P1-P2 = %20.15lf",d1) ;
    bcdtmWrite_message(0,0,0,"Length P2-P3 = %20.15lf",d2) ;
    bcdtmWrite_message(0,0,0,"Length P3-P1 = %20.15lf",d3) ;
    if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P1,P2)) bcdtmWrite_message(0,0,0,"Feature Line P1-P2") ;
    if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P2,P3)) bcdtmWrite_message(0,0,0,"Feature Line P2-P3") ;
    if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P3,P1)) bcdtmWrite_message(0,0,0,"Feature Line P3-P1") ;
   }
/*
** Try And Remove P1-P2 If P1-P2 Is The Longest Line
*/
 if( d1 >= d2 && d1 >= d3 )
   {
/*
**  Check Line Can Be Deleted
*/
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
    if( bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P3,P1) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P3,P2) > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing P1-P2") ;
/*
**     Delete Line
*/
       insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P1,P2) ;
       if( insertLine )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Insert Line P1-P2") ;
          if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,P1,P2,P3)) goto errexit ;
         }
       if( nodeAddrP(dtmP,P2)->hPtr == P1 )
         {
          bcdtmList_deleteLineDtmObject(dtmP,P1,P2) ;
          nodeAddrP(dtmP,P2)->hPtr = P3 ;
          nodeAddrP(dtmP,P3)->hPtr = P1 ;
         }
       else
         {
          bcdtmList_deleteLineDtmObject(dtmP,P1,P2) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,P3,antPnt,P1) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,P3,P2) ;
         }
       lineDeleted = TRUE ;
      }
   }
/*
**  Try And Remove P2-P3 If P2-P3 Is The Longest Line
*/
 else  if( d2 >= d1 && d2 >= d3 )
   {
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,P2,P3)) < 0 ) goto errexit ;
/*
**  Check Line Can Be Deleted
*/
    if( bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P1,P2) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P1,P3) > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing P2-P3") ;
       insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P2,P3) ;
       if( insertLine )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Insert Line P2-P3") ;
          if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,P2,P3,P1)) goto errexit ;
         }
       if( nodeAddrP(dtmP,P3)->hPtr == P2 )
         {
          bcdtmList_deleteLineDtmObject(dtmP,P2,P3) ;
          nodeAddrP(dtmP,P3)->hPtr = P1 ;
          nodeAddrP(dtmP,P1)->hPtr = P2 ;
         }
       else
         {
          bcdtmList_deleteLineDtmObject(dtmP,P2,P3) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,P1,antPnt,P2) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,P1,P3) ;
         }
       lineDeleted = TRUE ;
      }
   }
/*
** Try And Remove P3-P1 If P3-P1 Is The Longest Line
*/
 else  if( d3 >= d1 && d3 >= d3 )
   {
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,P3,P1)) < 0 ) goto errexit ;
/*
** Check Line Can Be Deleted
*/
    if( bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P2,P3) < 0 && bcdtmMath_pointSideOfDtmObject(dtmP,antPnt,P2,P1) > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing P3-P1") ;
       insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,P3,P1) ;
       if( insertLine )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Insert Line P3-P1") ;
          if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,P3,P1,P2)) goto errexit ;
         }
       if( nodeAddrP(dtmP,P1)->hPtr == P3 )
         {
          bcdtmList_deleteLineDtmObject(dtmP,P3,P1) ;
          nodeAddrP(dtmP,P3)->hPtr = P2 ;
          nodeAddrP(dtmP,P2)->hPtr = P1 ;
         }
       else
         {
          bcdtmList_deleteLineDtmObject(dtmP,P3,P1) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,P2,antPnt,P3) ;
          bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,P2,P1) ;
         }
       lineDeleted = TRUE ;
      }
   }
/*
** If Line Has Not Been Removed Merge Closest Points
*/
 if( lineDeleted == FALSE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Merging Closest Points") ;
    if( d1 <= d2 && d1 <= d3 ) { if( bcdtmMerge_connectedPointsDtmObject(dtmP,P2,P1) ) goto errexit ; }
    if( d2 <= d1 && d2 <= d3 ) { if( bcdtmMerge_connectedPointsDtmObject(dtmP,P3,P2) ) goto errexit ; }
    if( d3 <= d1 && d3 <= d2 ) { if( bcdtmMerge_connectedPointsDtmObject(dtmP,P1,P3) ) goto errexit ; }
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
BENTLEYDTM_Public int bcdtmInsert_rubberBandTinPointsOnToInsertStringPointsDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *StringPts,long NumStringPts,long *numMovedP)
/*
** This Function Rubber Bands Tin Points Onto The Insert String
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   node,cPnt,moveFlag ;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Points") ;
/*
** Initialise
*/
 *numMovedP = 0 ;
 for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->sPtr = 0 ;
/*
** Scan String And Find Closest Tin Point To String Point
*/
 for( p3dP = StringPts  ; p3dP < StringPts + NumStringPts ; ++p3dP )
   {
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&cPnt) ;
    if( ! nodeAddrP(dtmP,cPnt)->sPtr )
      {
       if( fabs(p3dP->x-pointAddrP(dtmP,cPnt)->x) < dtmP->ppTol && fabs(p3dP->y-pointAddrP(dtmP,cPnt)->y) < dtmP->ppTol )
         {
          if( bcdtmMath_distance(p3dP->x,p3dP->y,pointAddrP(dtmP,cPnt)->x,pointAddrP(dtmP,cPnt)->y) <= dtmP->ppTol )
            {
             if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,cPnt,p3dP->x,p3dP->y,&moveFlag)) goto errexit ;
             if( moveFlag )
               {
                pointAddrP(dtmP,cPnt)->x = p3dP->x ;
                pointAddrP(dtmP,cPnt)->y = p3dP->y ;
                pointAddrP(dtmP,cPnt)->z = p3dP->z ;
                nodeAddrP(dtmP,cPnt)->sPtr = 1 ;
                ++*numMovedP ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Points Error") ;
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
BENTLEYDTM_Private int  bcdtmInsert_rubberBandTinPointsOnToInsertStringLinesDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,long *numMovedP)
/*
** This Function Rubber Bands Tin Points Onto The Insert Lines
*/
{
 int    ret=DTM_SUCCESS,bkp,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,np1,np2,np3,node,fndType,moveFlag ;
 double x1,y1,x2,y2,xi,yi,zi ;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Lines") ;
/*
** Initialise
*/
 *numMovedP = 0 ;
 for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->sPtr = 0 ;
/*
** Scan String And Find Closest Tin Point To String Point
*/
 for( p3dP = stringPtsP  ; p3dP < stringPtsP + numStringPts - 1 ; ++p3dP )
   {
    x1 = p3dP->x ;
    y1 = p3dP->y ;
    x2 = (p3dP+1)->x ;
    y2 = (p3dP+1)->y ;
/*
**  If Start Point Is External To Tin Then Drape Line Is Invalid
*/
    if( bcdtmFind_triangleDtmObject(dtmP,x1,y1,&fndType,&p1,&p2,&p3)) goto errexit ;
    if( fndType != 0 )
/*
**  Scan Across Tin Surface Looking For Tin Points Within P2P Tolerance Of Insert Lines
*/
      {
       bkp = 0 ;
       if( fndType == 2 ) fndType = 3 ;
       while ( ! bkp )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"fndType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",fndType,p1,p2,p3) ;
          bkp = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,x1,y1,x2,y2,&fndType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"**** bkp = %2d fndType = %2ld ** np1 = %9ld np2 = %9ld np3 = %9ld",bkp,fndType,np1,np2,np3) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"**** xi  = %12.4lf yi = %12.4lf",xi,yi) ;
          if( bkp == 0 )
            {
             if( fndType == 2 )
               {
                if( bcdtmMath_distance(xi,yi,pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y) <= dtmP->ppTol )
                  {
                   if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,np1,xi,yi,&moveFlag)) goto errexit ;
                   if( moveFlag )
                     {
                      pointAddrP(dtmP,np1)->x = xi ;
                      pointAddrP(dtmP,np1)->y = yi ;
                      ++*numMovedP ;
                      fndType = 1 ;
                      np2 = np3 = dtmP->nullPnt ;
                     }
                  }
                else if( bcdtmMath_distance(xi,yi,pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y) <= dtmP->ppTol )
                  {
                   if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,np2,xi,yi,&moveFlag)) goto errexit ;
                   if( moveFlag )
                     {
                      pointAddrP(dtmP,np2)->x = xi ;
                      pointAddrP(dtmP,np2)->y = yi ;
                      ++*numMovedP ;
                      fndType = 1 ;
                      np1 = np2 ;
                      np2 = np3 = dtmP->nullPnt ;
                     }
                  }
               }
             p1 = np1 ;
             p2 = np2 ;
             p3 = np3 ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Lines Error") ;
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
BENTLEYDTM_Private int  bcdtmInsert_rubberBandHullPointsOnToInsertStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d **stringPtsPP,long *numStringPtsP,long *numMovedP)
/*
** This Function Rubber Bands Tin Hull Points Onto The Insert String
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,node,numIntTable,numIntPts,memIntPts,incIntPts,moveFlag,numIntersectPts ;
 DPoint3d    *p3d1P,*p3d2P,*intersectPtsP=NULL ;
 DTM_STR_INT_TAB *intTabP,*intTableP=NULL ;
 DTM_STR_INT_PTS *intPntP,*intPnt1P,*intPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Points Onto Insert String") ;
/*
** Initialise
*/
 *numMovedP = 0 ;
/*
** Build Intersection Table For String And Hull Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table") ;
 if( bcdtmInsert_buildStringHullPointsIntersectionTableDtmObject(dtmP,*stringPtsPP,*numStringPtsP,&intTableP,&numIntTable)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Intersection Table Entries = %6ld",numIntTable) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of String Hull Points Intersection Table Entries = %6ld",numIntTable ) ;
    for( intTabP = intTableP ; intTabP < intTableP + numIntTable ; ++intTabP )
      {
       bcdtmWrite_message(0,0,0,"Entry[%5ld] ** String = %6ld Segment = %6ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(intTabP-intTableP),intTabP->String,intTabP->Segment,intTabP->Type,intTabP->Direction,intTabP->X1,intTabP->Y1,intTabP->Z1,intTabP->X2,intTabP->Y2,intTabP->Z2) ;
      }
   }
/*
** Scan Intersection Table And For Intersections
*/
 incIntPts = numIntTable / 10 ;
 if( incIntPts == 0 ) incIntPts = 100 ;
 numIntPts = memIntPts = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For String Hull Point Intersections") ;
 if( bcdtmInsert_scanForStringHullPointIntersections(intTableP,numIntTable,&intPtsP,&numIntPts,&memIntPts,incIntPts) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Point Intersections = %4ld",numIntPts) ;
/*
** Sort Intersection Points
*/
 if( numIntPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting String Hull Point Intersection Points") ;
    qsortCPP(intPtsP,numIntPts,sizeof(DTM_STR_INT_PTS),bcdtmInsert_intersectionPointsExternalInsertLineCompareFunction) ;
/*
** Write Intersection Points
*/
    if( dbg == 2  )
      {
       bcdtmWrite_message(0,0,0,"Number Of String Hull Point Intersections = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(intPntP-intPtsP),intPntP->String1,intPntP->Segment1,intPntP->String2,intPntP->Segment2,intPntP->Distance,intPntP->x,intPntP->y,intPntP->z,intPntP->Z2) ;
         }
      }
/*
**  Test And See If Points Can Be Moved And Move Them
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Moving Hull Points Onto Insert String") ;
     for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->sPtr = 0 ;
     for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
       {
        if( ! nodeAddrP(dtmP,intPntP->String1)->sPtr )
          {
           if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,intPntP->String1,intPntP->x,intPntP->y,&moveFlag)) goto errexit ;
           if( moveFlag )
             {
              pointAddrP(dtmP,intPntP->String1)->x = intPntP->x  ;
              pointAddrP(dtmP,intPntP->String1)->y = intPntP->y  ;
              nodeAddrP(dtmP,intPntP->String1)->sPtr = 1 ;
              intPntP->Segment1 = 4 ;
              ++*numMovedP ;
             }
          }
       }
    for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->sPtr = 0 ;
/*
**  Remove Intersection Points That Werent Moved
*/
    for( intPntP = intPnt1P = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       if( intPntP->Segment1 == 4 )
         {
          if( intPnt1P != intPntP ) *intPnt1P = *intPntP ;
          ++intPnt1P ;
         }
      }
    numIntPts = (long)(intPnt1P-intPtsP) ;
/*
**  Write Intersection Points
*/
    if( dbg == 1  )
      {
       bcdtmWrite_message(0,0,0,"Number Of String Hull Intersections = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %12.6lf x = %12.6lf y = %12.6lf z = %12.6lf Z2 = %12.6lf",(long)(intPntP-intPtsP),intPntP->String1,intPntP->Segment1,intPntP->String2,intPntP->Segment2,intPntP->Distance,intPntP->x,intPntP->y,intPntP->z,intPntP->Z2) ;
         }
      }
/*
**  Add Intersection Points To String
*/
    if( numIntPts > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding Moved Points Onto Insert String") ;
       numIntersectPts = *numStringPtsP + numIntPts ;
       intersectPtsP = (DPoint3d * ) malloc( numIntersectPts * sizeof(DPoint3d)) ;
       if( intersectPtsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
       for( p3d1P = *stringPtsPP, ofs = 0, p3d2P = intersectPtsP, intPntP = intPtsP ; p3d1P < *stringPtsPP + *numStringPtsP ; ++p3d1P , ++ofs )
         {
          *p3d2P = *p3d1P ;
          ++p3d2P ;
          if( ofs == intPntP->String2 )
            {
             while ( intPntP < intPtsP + numIntPts && ofs == intPntP->String2 )
               {
                if( ( intPntP->x != p3d1P->x     && intPntP->y != p3d1P->y     ) &&
                    ( intPntP->x != (p3d1P+1)->x && intPntP->y != (p3d1P+1)->y )     )
                  {
                   p3d2P->x = intPntP->x ;
                   p3d2P->y = intPntP->y ;
                   p3d2P->z = intPntP->Z2 ;
                   ++p3d2P ;
                  }
                ++intPntP ;
               }
            }
         }
       numIntersectPts = (long)(p3d2P-intersectPtsP) ;
/*
**     Set String Pointers
*/
       free(*stringPtsPP) ;
       *stringPtsPP = intersectPtsP ;
       *numStringPtsP = numIntersectPts ;
       intersectPtsP = NULL ;
/*
**     Write String Points
*/
       if( dbg == 1 )
         {
          bcdtmWrite_message(0,0,0,"Number Of String Points = %6ld",*numStringPtsP) ;
          for( p3d1P = *stringPtsPP ; p3d1P < *stringPtsPP + *numStringPtsP ; ++p3d1P )
            {
             bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d1P-*stringPtsPP),p3d1P->x,p3d1P->y,p3d1P->z ) ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( intTableP     != NULL ) free(intTableP) ;
 if( intPtsP       != NULL ) free(intPtsP)   ;
 if( intersectPtsP != NULL ) free(intersectPtsP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Points Onto Insert String Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Points Onto Insert String Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_intersectionPointsExternalInsertLineCompareFunction(const DTM_STR_INT_PTS *intPnt1P,const DTM_STR_INT_PTS  *intPnt2P)
/*
** Compare Function For Qsort Of String Line And Tin Hull Intersection Points
*/
{
 if     ( intPnt1P->String2  < intPnt2P->String2  ) return(-1) ;
 else if( intPnt1P->String2  > intPnt2P->String2  ) return( 1) ;
 else if( intPnt1P->Distance < intPnt2P->Distance ) return(-1) ;
 else if( intPnt1P->Distance > intPnt2P->Distance ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int  bcdtmInsert_buildStringHullPointsIntersectionTableDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,DTM_STR_INT_TAB **intTablePP,long *numIntTableP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,pp,memIntTable,incIntTable  ;
 double cord,priorAng,nextAng,radAng ;
 DPoint3d    *p3dP ;
 DTM_STR_INT_TAB *intTabP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Point Intersection Table") ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 incIntTable = numStringPts * 2 ;
/*
** Store Hull Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing String Lines In Intersection Table") ;
 sp = dtmP->hullPoint ;
 np = nodeAddrP(dtmP,sp)->hPtr ;
 if( ( pp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
 do
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                      *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Get Angles
*/
    np = nodeAddrP(dtmP,sp)->hPtr ;
    nextAng  = bcdtmMath_getPointAngleDtmObject(dtmP,sp,np) ;
    priorAng = bcdtmMath_getPointAngleDtmObject(dtmP,sp,pp) ;
    if( priorAng > nextAng ) nextAng += DTM_2PYE ;
    radAng = ( nextAng + priorAng ) / 2.0 ;
    if( radAng >= DTM_2PYE ) radAng -= DTM_2PYE ;
/*
**  Store Hull Line
*/
    (*intTablePP+*numIntTableP)->String  = sp  ;
    (*intTablePP+*numIntTableP)->Segment = sp  ;
    (*intTablePP+*numIntTableP)->Type    = 1   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = pointAddrP(dtmP,sp)->x + dtmP->ppTol*cos(radAng) ;
    (*intTablePP+*numIntTableP)->Y1 = pointAddrP(dtmP,sp)->y + dtmP->ppTol*sin(radAng) ;
    (*intTablePP+*numIntTableP)->Z1 = pointAddrP(dtmP,sp)->z  ;
    (*intTablePP+*numIntTableP)->X2 = pointAddrP(dtmP,sp)->x - dtmP->ppTol*cos(radAng) ;
    (*intTablePP+*numIntTableP)->Y2 = pointAddrP(dtmP,sp)->y - dtmP->ppTol*sin(radAng) ;
    (*intTablePP+*numIntTableP)->Z2 = pointAddrP(dtmP,sp)->z ;
    ++*numIntTableP ;
/*
** Set For Next Hull Point
*/
    pp = sp ;
    sp = np ;
   } while( sp != dtmP->hullPoint ) ;
/*
** Store String Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing String Lines In Intersection Table") ;
 for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts - 1 ; ++p3dP )
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                    *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store String Line
*/
    (*intTablePP+*numIntTableP)->String  = (long)(p3dP-stringPtsP) ;
    (*intTablePP+*numIntTableP)->Segment = (long)(p3dP+1-stringPtsP)  ;
    (*intTablePP+*numIntTableP)->Type    = 2   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = p3dP->x ;
    (*intTablePP+*numIntTableP)->Y1 = p3dP->y ;
    (*intTablePP+*numIntTableP)->Z1 = p3dP->z ;
    (*intTablePP+*numIntTableP)->X2 = (p3dP+1)->x ;
    (*intTablePP+*numIntTableP)->Y2 = (p3dP+1)->y ;
    (*intTablePP+*numIntTableP)->Z2 = (p3dP+1)->z ;
    ++*numIntTableP ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( intTabP = *intTablePP ; intTabP < *intTablePP + *numIntTableP ; ++intTabP )
   {
    if( intTabP->X1 > intTabP->X2 || ( intTabP->X1 == intTabP->X2 && intTabP->Y1 > intTabP->Y2 ) )
      {
       intTabP->Direction = 2 ;
       cord = intTabP->X1 ; intTabP->X1 = intTabP->X2 ; intTabP->X2 = cord ;
       cord = intTabP->Y1 ; intTabP->Y1 = intTabP->Y2 ; intTabP->Y2 = cord ;
       cord = intTabP->Z1 ; intTabP->Z1 = intTabP->Z2 ; intTabP->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STR_INT_TAB),bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building String Hull Points Intersection Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building String Hull Points Intersection Table Error") ;
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInsert_scanForStringHullPointIntersections(DTM_STR_INT_TAB *intTableP,long numIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts)
/*
** This Function Scans for StringHull Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 long    numActiveIntTable=0,memActiveIntTable=0 ;
 DTM_STR_INT_TAB *intTabP,*activeIntTableP=NULL ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( intTabP = intTableP ; intTabP < intTableP + numIntTable  ; ++intTabP)
   {
    if( bcdtmClean_deleteActiveStringLines(activeIntTableP,&numActiveIntTable,intTabP)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&activeIntTableP,&numActiveIntTable,&memActiveIntTable,intTabP))  goto errexit ;
    if( bcdtmInsert_determineStringHullPointIntersections(activeIntTableP,numActiveIntTable,intPtsPP,numIntPtsP,memIntPtsP,incIntPts)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( activeIntTableP != NULL ) free(activeIntTableP) ;
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
BENTLEYDTM_Private int bcdtmInsert_determineStringHullPointIntersections(DTM_STR_INT_TAB *activeIntTableP,long numActiveIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts )
/*
** Determine Line Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 double  dl,z2,dz,dist,x,y ;
 DTM_STR_INT_TAB  *activeP,*scanP,*hullP,*strP ;
/*
** Initialise
*/
 activeP = activeIntTableP + numActiveIntTable - 1 ;
/*
** Scan Active Line List
*/
 for( scanP = activeIntTableP ; scanP < activeIntTableP + numActiveIntTable - 1 ; ++scanP )
   {
/*
**  Only Compare String Lines To Hull Points
*/
    if( ( activeP->Type == 1 && scanP->Type == 2 ) || ( activeP->Type == 2 && scanP->Type == 1 ) )
      {
/*
**     Set Line Types
*/
       if( activeP->Type == 1 ) { hullP = activeP ; strP = scanP ; }
       else                 { hullP = scanP ; strP = activeP ; }
/*
**     Check Lines Intersect
*/
       if( bcdtmMath_checkIfLinesIntersect(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2))
         {
/*
**        Intersect Lines
*/
          bcdtmMath_normalIntersectCordLines(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2,&x,&y) ;

/*
**        Check Memory
*/
          if( *numIntPtsP + 1 >= *memIntPtsP )
            {
             *memIntPtsP = *memIntPtsP + incIntPts ;
             if( *intPtsPP == NULL ) *intPtsPP = ( DTM_STR_INT_PTS * ) malloc ( *memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             else                  *intPtsPP = ( DTM_STR_INT_PTS * ) realloc( *intPtsPP,*memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             if( *intPtsPP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
**       Check And Modify Intersection Point If It Is Almost Coincident With String End Points
*/
         if( strP->X1 == strP->X2 ) x = strP->X1 ;
         if( strP->Y1 == strP->Y2 ) y = strP->Y1 ;
         if     ( bcdtmMath_distance(x,y,strP->X1,strP->Y1) < 0.0000001 ) { x = strP->X1 ; y = strP->Y1 ; }
         else if( bcdtmMath_distance(x,y,strP->X2,strP->Y2) < 0.0000001 ) { x = strP->X2 ; y = strP->Y2 ; }
/*
**       Calculate Z2 Value
*/
          dl = bcdtmMath_distance(strP->X1,strP->Y1,strP->X2,strP->Y2) ;
          if( strP->Direction == 1 )
            {
             dist = bcdtmMath_distance(x,y,strP->X1,strP->Y1) ;
             dz   = strP->Z2 - strP->Z1 ;
             z2   = strP->Z1 +  dz * dist / dl ;
            }
          else
            {
             dist = bcdtmMath_distance(x,y,strP->X2,strP->Y2) ;
             dz   = strP->Z1 - strP->Z2 ;
             z2   = strP->Z2 +  dz * dist / dl ;
            }
/*
**        Store Intersection Point Alp
*/
          (*intPtsPP+*numIntPtsP)->String1  = hullP->String  ;
          (*intPtsPP+*numIntPtsP)->Segment1 = hullP->Segment ;
          (*intPtsPP+*numIntPtsP)->String2  = strP->String   ;
          (*intPtsPP+*numIntPtsP)->Segment2 = strP->Segment  ;
          (*intPtsPP+*numIntPtsP)->Distance = dist ;
          (*intPtsPP+*numIntPtsP)->x  = x ;
          (*intPtsPP+*numIntPtsP)->y  = y ;
          (*intPtsPP+*numIntPtsP)->z  = 0.0 ;
          (*intPtsPP+*numIntPtsP)->Z2 = z2 ;
          ++*numIntPtsP ;
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
BENTLEYDTM_Private int  bcdtmInsert_rubberBandHullLinesOnToInsertStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,long *numMovedP)
/*
** This Function Rubber Bands Hull Lines Onto The Insert String
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ap,np,lp,hp1,hp2,node,dtmFeatureLine,precisionError ;
 long   numIntTable,numIntPts,memIntPts,incIntPts ;
 DTM_STR_INT_TAB *intTabP,*intTableP=NULL ;
 DTM_STR_INT_PTS *intPntP,*intPnt1P,*intPnt2P,*intPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Lines Onto Insert String") ;
/*
** Initialise
*/
 *numMovedP = 0 ;
/*
** Build Intersection Table For String And Hull Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table") ;
 if( bcdtmInsert_buildStringHullLinesIntersectionTableDtmObject(dtmP,stringPtsP,numStringPts,&intTableP,&numIntTable)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Intersection Table Entries = %6ld",numIntTable) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of String Hull Points Intersection Table Entries = %6ld",numIntTable ) ;
    for( intTabP = intTableP ; intTabP < intTableP + numIntTable ; ++intTabP )
      {
       bcdtmWrite_message(0,0,0,"Entry[%5ld] ** String = %6ld Segment = %6ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(intTabP-intTableP),intTabP->String,intTabP->Segment,intTabP->Type,intTabP->Direction,intTabP->X1,intTabP->Y1,intTabP->Z1,intTabP->X2,intTabP->Y2,intTabP->Z2) ;
      }
   }
/*
** Scan Intersection Table And For Intersections
*/
 incIntPts = numIntTable / 10 ;
 if( incIntPts == 0 ) incIntPts = 100 ;
 numIntPts = memIntPts = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For String Hull Point Intersections") ;
 if( bcdtmInsert_scanForStringHullLineIntersectionsDtmObject(intTableP,numIntTable,&intPtsP,&numIntPts,&memIntPts,incIntPts) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Point Intersections = %4ld",numIntPts) ;
/*
** Sort Intersection Points
*/
 if( numIntPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting String Hull Point Intersection Points") ;
    qsortCPP(intPtsP,numIntPts,sizeof(DTM_STR_INT_PTS),bcdtmInsert_intersectionLinesHullExternalInsertLineCompareFunction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Point Intersections = %6ld",numIntPts) ;
/*
**  Write Intersection Points
*/
    if( dbg == 1  )
      {
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld DH = %8.4lf DS = %20.18lf ** %12.4lf %12.4lf",(long)(intPntP-intPtsP),intPntP->String1,intPntP->Segment1,intPntP->String2,intPntP->Segment2,intPntP->Distance,intPntP->Z2,intPntP->x,intPntP->y) ;
         }
      }
/*
**   Test And Insert Intersection Points Into Hull
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Moving Hull Points Onto Insert String") ;
     for( node = 0 ; node  < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->sPtr = 0 ;
     for( intPnt1P = intPtsP ; intPnt1P < intPtsP + numIntPts ; ++intPnt1P )
       {
        intPnt2P = intPnt1P ;
        while ( intPnt2P < intPtsP + numIntPts && intPnt2P->String1 == intPnt1P->String1 ) ++intPnt2P ;
        --intPnt2P ;
        hp1 = intPnt1P->String1  ;
        hp2 = intPnt1P->Segment1 ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Points Into Hull Line %9ld %9ld",hp1,hp2) ;
        if( ( ap = bcdtmList_nextAntDtmObject(dtmP,hp1,hp2)) < 0 ) goto errexit ;
        lp = hp1 ;
        dtmFeatureLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,hp1,hp2) ;
        for( intPntP = intPnt1P ; intPntP <= intPnt2P ; ++intPntP )
          {
           bcdtmInsert_checkPointTriangleLinePrecisionDtmObject(dtmP,lp,hp2,ap,(stringPtsP+intPntP->String2)->x,(stringPtsP+intPntP->String2)->y,&precisionError) ;
           if( ! precisionError )
             {
              if( bcdtmList_deleteLineDtmObject(dtmP,lp,hp2) ) goto errexit ;
              if( bcdtmInsert_addPointToDtmObject(dtmP,(stringPtsP+intPntP->String2)->x,(stringPtsP+intPntP->String2)->y,(stringPtsP+intPntP->String2)->z,&np)) goto errexit ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,lp,np,ap)) goto errexit ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,lp,dtmP->nullPnt)) goto errexit ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,np,hp2)) goto errexit ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,ap,lp))  goto errexit ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,hp2,ap)) goto errexit ;
              if( bcdtmList_insertLineBeforePointDtmObject(dtmP,hp2,np,ap))goto errexit ;
              if( dtmFeatureLine )if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,lp,hp2,np)) goto errexit ;
/*
**            Set Hull Pointers
*/
              nodeAddrP(dtmP,lp)->hPtr = np  ;
              nodeAddrP(dtmP,np)->hPtr = hp2 ;
              lp = np ;
             }
          }
        intPnt1P = intPnt2P ;
       }
   }
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology After Rubber Banding Hull Lines") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Topology Corrupted") ; goto errexit ; }
    else                                        bcdtmWrite_message(0,0,0,"Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Precision After Rubber Banding Hull Lines") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0)){ bcdtmWrite_message(0,0,0,"Precision Corrupted") ; goto errexit ; }
    else                                        bcdtmWrite_message(0,0,0,"Precision OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( intTableP != NULL ) free(intTableP) ;
 if( intPtsP   != NULL ) free(intPtsP)   ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Lines Onto Insert String Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Lines Onto Insert String Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInsert_intersectionLinesHullExternalInsertLineCompareFunction(const DTM_STR_INT_PTS *intPnt1P,const DTM_STR_INT_PTS  *intPnt2P)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     ( intPnt1P->String1  < intPnt2P->String1  ) return(-1) ;
 else if( intPnt1P->String1  > intPnt2P->String1  ) return( 1) ;
 else if( intPnt1P->Distance < intPnt2P->Distance ) return(-1) ;
 else if( intPnt1P->Distance > intPnt2P->Distance ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int  bcdtmInsert_buildStringHullLinesIntersectionTableDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,DTM_STR_INT_TAB **intTablePP,long *numIntTableP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,pofs,ppofs,npofs,closeFlag,memIntTable,incIntTable  ;
 double cord,priorAng=0.0,nextAng=0.0,radAng=0.0 ;
 DPoint3d    *p3dP ;
 DTM_STR_INT_TAB *intTabP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Lines Intersection Table") ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 incIntTable = numStringPts * 2 ;
/*
** Check For Closing String
*/
 closeFlag = 0 ;
 if( stringPtsP->x == (stringPtsP+numStringPts-1)->x && stringPtsP->y == (stringPtsP+numStringPts-1)->y ) closeFlag = 1 ;
/*
** Store Hull Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Hull Lines In Intersection Table") ;
 sp = dtmP->hullPoint ;
 do
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                      *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
** Set Next Hull Point
*/
    np = nodeAddrP(dtmP,sp)->hPtr ;
/*
**  Store Hull Line
*/
    (*intTablePP+*numIntTableP)->String  = sp  ;
    (*intTablePP+*numIntTableP)->Segment = np  ;
    (*intTablePP+*numIntTableP)->Type    = 1   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = pointAddrP(dtmP,sp)->x ;
    (*intTablePP+*numIntTableP)->Y1 = pointAddrP(dtmP,sp)->y ;
    (*intTablePP+*numIntTableP)->Z1 = pointAddrP(dtmP,sp)->z ;
    (*intTablePP+*numIntTableP)->X2 = pointAddrP(dtmP,np)->x ;
    (*intTablePP+*numIntTableP)->Y2 = pointAddrP(dtmP,np)->y ;
    (*intTablePP+*numIntTableP)->Z2 = pointAddrP(dtmP,np)->z ;
    ++*numIntTableP ;
/*
** Set For Next Hull Point
*/
    sp = np ;
   } while( sp != dtmP->hullPoint ) ;
/*
** Store String Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing String Lines In Intersection Table") ;
 ppofs = DTM_NULL_PNT ;
 if( closeFlag == 1 ) ppofs = numStringPts - 2 ;
 for( p3dP = stringPtsP , pofs = 0 ; p3dP < stringPtsP + numStringPts - closeFlag ; ++p3dP , ++pofs )
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                    *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Set Next Point Offset
*/
    npofs = pofs + 1 ;
    if( npofs >= numStringPts ) npofs = DTM_NULL_PNT ;
/*
**  Get Angles
*/
    if( npofs != DTM_NULL_PNT ) nextAng = bcdtmMath_getAngle(p3dP->x,p3dP->y,(stringPtsP+npofs)->x,(stringPtsP+npofs)->y) ;
    if( ppofs != DTM_NULL_PNT ) priorAng = bcdtmMath_getAngle(p3dP->x,p3dP->y,(stringPtsP+ppofs)->x,(stringPtsP+ppofs)->y) ;
    if( ppofs != DTM_NULL_PNT && npofs != DTM_NULL_PNT )
      {
       if( priorAng > nextAng ) nextAng += DTM_2PYE ;
       radAng = ( nextAng + priorAng ) / 2.0 ;
       if( radAng >= DTM_2PYE ) radAng -= DTM_2PYE ;
      }
    else if( ppofs == DTM_NULL_PNT && npofs != DTM_NULL_PNT )
      {
       radAng = nextAng - DTM_PYE / 2.0 ;
       if( radAng < 0.0 ) radAng += DTM_2PYE ;
      }
    else if( ppofs != DTM_NULL_PNT && npofs == DTM_NULL_PNT )
      {
       radAng = priorAng + DTM_PYE / 2.0 ;
       if( radAng > DTM_2PYE ) radAng -= DTM_2PYE ;
      }
/*
**  Store String Line
*/
    (*intTablePP+*numIntTableP)->String  = (long)(p3dP-stringPtsP) ;
    (*intTablePP+*numIntTableP)->Segment = (long)(p3dP-stringPtsP)  ;
    (*intTablePP+*numIntTableP)->Type    = 2   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = p3dP->x + dtmP->ppTol * cos(radAng) ;
    (*intTablePP+*numIntTableP)->Y1 = p3dP->y + dtmP->ppTol * sin(radAng) ;
    (*intTablePP+*numIntTableP)->Z1 = p3dP->z ;
    (*intTablePP+*numIntTableP)->X2 = p3dP->x - dtmP->ppTol * cos(radAng) ;
    (*intTablePP+*numIntTableP)->Y2 = p3dP->y - dtmP->ppTol * sin(radAng) ;
    (*intTablePP+*numIntTableP)->Z2 = p3dP->z ;
    ++*numIntTableP ;
/*
**  Set Prior Offset
*/
    ppofs = pofs ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( intTabP = *intTablePP ; intTabP < *intTablePP + *numIntTableP ; ++intTabP )
   {
    if( intTabP->X1 > intTabP->X2 || ( intTabP->X1 == intTabP->X2 && intTabP->Y1 > intTabP->Y2 ) )
      {
       intTabP->Direction = 2 ;
       cord = intTabP->X1 ; intTabP->X1 = intTabP->X2 ; intTabP->X2 = cord ;
       cord = intTabP->Y1 ; intTabP->Y1 = intTabP->Y2 ; intTabP->Y2 = cord ;
       cord = intTabP->Z1 ; intTabP->Z1 = intTabP->Z2 ; intTabP->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STR_INT_TAB),bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building String Hull Lines Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building String Hull Lines Intersection Table Error") ;
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInsert_scanForStringHullLineIntersectionsDtmObject(DTM_STR_INT_TAB *intTableP,long numIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts)
/*
** This Function Scans for StringHull Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 long    numActiveIntTable=0,memActiveTable=0 ;
 DTM_STR_INT_TAB *intTabP,*activeIntTableP=NULL ;
/*
** Scan Sorted Intersection Table and Look For Intersections
*/
 for( intTabP = intTableP ; intTabP < intTableP + numIntTable  ; ++intTabP)
   {
    if( bcdtmClean_deleteActiveStringLines(activeIntTableP,&numActiveIntTable,intTabP)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&activeIntTableP,&numActiveIntTable,&memActiveTable,intTabP))  goto errexit ;
    if( bcdtmInsert_determineStringHullLineIntersectionsDtmObject(activeIntTableP,numActiveIntTable,intPtsPP,numIntPtsP,memIntPtsP,incIntPts)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( activeIntTableP != NULL ) free(activeIntTableP) ;
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
BENTLEYDTM_Private int bcdtmInsert_determineStringHullLineIntersectionsDtmObject(DTM_STR_INT_TAB *activeIntTableP,long numActiveIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts )
/*
** Determine Line Intersections
*/
{
 int    ret=DTM_SUCCESS ;
 double ds,dh,x,y ;
 DTM_STR_INT_TAB  *activeP,*scanP,*hullP,*strP ;
/*
** Initialise
*/
 activeP = activeIntTableP + numActiveIntTable - 1 ;
/*
** Scan Active Line List
*/
 for( scanP = activeIntTableP ; scanP < activeIntTableP + numActiveIntTable - 1 ; ++scanP )
   {
/*
**  Only Compare String Lines To Hull Points
*/
    if( ( activeP->Type == 1 && scanP->Type == 2 ) || ( activeP->Type == 2 && scanP->Type == 1 ) )
      {
/*
**     Set Line Types
*/
       if( activeP->Type == 1 ) { hullP = activeP ; strP = scanP ; }
       else                 { hullP = scanP ; strP = activeP ; }
/*
**     Check Lines Intersect
*/
       if( bcdtmMath_checkIfLinesIntersect(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2))
         {
/*
**        Intersect Lines
*/
          bcdtmMath_normalIntersectCordLines(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2,&x,&y) ;

/*
**        Check Memory
*/
          if( *numIntPtsP + 1 >= *memIntPtsP )
            {
             *memIntPtsP = *memIntPtsP + incIntPts ;
             if( *intPtsPP == NULL ) *intPtsPP = ( DTM_STR_INT_PTS * ) malloc ( *memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             else                  *intPtsPP = ( DTM_STR_INT_PTS * ) realloc( *intPtsPP,*memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             if( *intPtsPP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
**        Calculate Distance Value
*/
          ds = bcdtmMath_distance(x,y,(strP->X1+strP->X2)/2.0,(strP->Y1+strP->Y2)/2.0) ;
          if( hullP->Direction == 1 ) dh = bcdtmMath_distance(x,y,hullP->X1,hullP->Y1) ;
          else                        dh = bcdtmMath_distance(x,y,hullP->X2,hullP->Y2) ;
/*
**        Store Intersection Point Alp
*/
          (*intPtsPP+*numIntPtsP)->String1  = hullP->String  ;
          (*intPtsPP+*numIntPtsP)->Segment1 = hullP->Segment ;
          (*intPtsPP+*numIntPtsP)->String2  = strP->String   ;
          (*intPtsPP+*numIntPtsP)->Segment2 = strP->Segment  ;
          (*intPtsPP+*numIntPtsP)->Distance = dh ;
          (*intPtsPP+*numIntPtsP)->x  = x   ;
          (*intPtsPP+*numIntPtsP)->y  = y   ;
          (*intPtsPP+*numIntPtsP)->z  = 0.0 ;
          (*intPtsPP+*numIntPtsP)->Z2 = ds  ;
          ++*numIntPtsP ;
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
BENTLEYDTM_Public int bcdtmInsert_colinearStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,long *startPntP)
/*
** This Function Inserts
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,fndType,pnt1,pnt2,pnt3,newPnt,lastPnt=0 ;
 long   hullPnt1 = 0,hullPnt2 = 0,onLine,lineFound ;
 double n1,n2 = 0.0,n3,x,y ;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Colinear String") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"stringPtsP   = %p",stringPtsP) ;
    bcdtmWrite_message(0,0,0,"numStringPts = %8ld",numStringPts) ;
    bcdtmWrite_message(0,0,0,"startPntP    = %8ld",*startPntP) ;
   }
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
/*
** Insert String Points Into DTMFeatureState::Tin
*/
 for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point %4ld of %4ld ** %12.5lf %12.5lf %10.4lf",(long)(p3dP-stringPtsP)+1,numStringPts,p3dP->x,p3dP->y,p3dP->z) ;
    if( bcdtmFind_triangleDtmObject(dtmP,p3dP->x,p3dP->y,&fndType,&pnt1,&pnt2,&pnt3) ) goto errexit ;
/*
**  Point External To Tin
*/
    if( fndType == 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Point External To Tin") ;
/*
**     Find Closest Hull Line
*/
       pnt1 = dtmP->hullPoint ;
       lineFound = FALSE ;
       do
         {
          pnt2 = nodeAddrP(dtmP,pnt1)->hPtr ;
          n1 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,p3dP->x,p3dP->y,&x,&y) ;
          if( onLine && ( n1 < n2 || lineFound == FALSE ))
            {
             n2 = n1 ;
             hullPnt1 = pnt1 ;
             hullPnt2 = pnt2 ;
             lineFound = TRUE ;
            }
           pnt1 = pnt2 ;
         } while ( pnt1 != dtmP->hullPoint ) ;
/*
**     Check Hull Line Found
*/
       if( lineFound == FALSE )
         {
          bcdtmWrite_message(1,0,0,"Cannot Find Closest Tin Hull Line") ;
          goto errexit ;
         }
       else
         {
          pnt1 = hullPnt1 ;
          pnt2 = hullPnt2 ;
          pnt3 = dtmP->nullPnt ;
          fndType = 3 ;
         }
      }
/*
**   Point On Tin Line
*/
    if( fndType == 2 )
      {
       n1  = bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt1,pnt2,p3dP->x,p3dP->y) ;
       n2  = bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt2,pnt3,p3dP->x,p3dP->y) ;
       n3  = bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt3,pnt1,p3dP->x,p3dP->y) ;
       if     ( n2 <= n1 && n2 <= n3 ) { pnt1 = pnt2 ; pnt2 = pnt3 ; }
       else if( n3 <= n1 && n3 <= n2 ) { pnt2 = pnt1 ; pnt1 = pnt3 ; }
       pnt3 = dtmP->nullPnt ;
      }
/*
**  Test If Point On Tin Hull Line
*/
    if( fndType == 2 )
      {
       if      ( nodeAddrP(dtmP,pnt1)->hPtr == pnt2 )    fndType = 3 ;
       else if ( nodeAddrP(dtmP,pnt2)->hPtr == pnt1 )  { fndType = 3 ; p1 = pnt1 ; pnt1 = pnt2 ; pnt2 = p1 ; }
      }
/*
**  Add Point To Tin
*/
    if( fndType > 1 ) { if( bcdtmInsert_addPointToDtmObject(dtmP,p3dP->x,p3dP->y,p3dP->z,&newPnt)) goto errexit ; }
    else              newPnt = pnt1 ;
/*
**  Set Tptr List
*/
    if( *startPntP == dtmP->nullPnt ) { *startPntP = lastPnt = newPnt ; }
    else                              { nodeAddrP(dtmP,lastPnt)->tPtr = newPnt ; lastPnt = newPnt ; }
/*
** Insert Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
    switch( fndType )
      {
       case  0 :      /* Point External To Tin              */
         bcdtmWrite_message(1,0,0,"Point External To Tin") ;
         goto errexit ;
       break   ;

       case  1 :      /* Coincident With Existing Point     */
         pointAddrP(dtmP,pnt1)->z = p3dP->z ;
       break   ;

       case  2 :      /* Coincident With Internal Tin Line  */
         if( (p1 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2))   < 0 ) goto errexit ;
         if( (p2 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
         if(bcdtmList_deleteLineDtmObject(dtmP,pnt1,pnt2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,newPnt,p1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,pnt1,dtmP->nullPnt)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt2,newPnt,p2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,pnt2,pnt1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,newPnt,pnt2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p1,pnt1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,newPnt,pnt1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p2,pnt2)) goto errexit ;
         if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,pnt1,pnt2) )
           {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,pnt1,pnt2,newPnt)) goto errexit ;
           }
       break ;

       case  3 :      /* Coincident With External Tin Line  */
         if( (p1 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2))   < 0 ) goto errexit ;
         if(bcdtmList_deleteLineDtmObject(dtmP,pnt1,pnt2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,newPnt,p1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,pnt1,dtmP->nullPnt)) goto errexit ;
         if(bcdtmList_insertLineBeforePointDtmObject(dtmP,pnt2,newPnt,p1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,pnt2,pnt1)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,newPnt,pnt2)) goto errexit ;
         if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newPnt,p1,pnt1)) goto errexit ;
         if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,pnt1,pnt2) )
           {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,pnt1,pnt2,newPnt)) goto errexit ;
           }
         nodeAddrP(dtmP,pnt1)->hPtr   = newPnt ;
         nodeAddrP(dtmP,newPnt)->hPtr = pnt2 ;
       break ;
      } ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Colinear String Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Colinear String Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_rigidInternalStringIntoDtmObject(BC_DTM_OBJ *dtmP,long drapeFlag,long insertFlag,DPoint3d *stringPtsP,long numStringPts,long *startPntP)
/*
**
** This Function Inserts A String Into A Tin Object
** Tin pointsP Are Pulled Onto The Insert String Where Possible
** Assumes String Is Internal To Tin And Has Been Validated
**
** drapeFlag    = 1   Drape Intersect Vertices On Tin Surface
**              = 2   Break Intersect Vertices On Tin Surface
** insertFlag   = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    pntNum,*pntP,*pnt1P,*pointsP=NULL,onLine,pointsMoved1=0,pointsMoved2=0 ;
 DPoint3d     *p3dP,*p3d1P ;
 double  dn,xn,yn    ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Rigid Internal String") ;
/*
** Write Parameters
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drapeFlag             = %1ld",drapeFlag) ;
    bcdtmWrite_message(0,0,0,"insertFlag            = %1ld",insertFlag) ;
    bcdtmWrite_message(0,0,0,"stringPtsP            = %p",stringPtsP) ;
    bcdtmWrite_message(0,0,0,"numStringPts          = %8ld",numStringPts) ;
    bcdtmWrite_message(0,0,0,"startPntP             = %8ld",*startPntP) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol           = %20.18lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol           = %20.18lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol          = %20.18lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints       = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->incPoints       = %8ld",dtmP->incPoints) ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of String pointsP = %6ld",numStringPts) ;
       for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP )
         {
          p3d1P = p3dP + 1 ;
          if( p3d1P >= stringPtsP + numStringPts ) p3d1P = stringPtsP + 1 ;
          bcdtmWrite_message(0,0,0,"Point[%4ld] ** %10.4lf %10.4lf %10.4lf Angle = %12.10lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z,bcdtmMath_getAngle(p3dP->x,p3dP->y,p3d1P->x,p3d1P->y)) ;
         }
      }
   }
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
/*
** Null Out Sptr Array For Marking Inserted String Points In Tin To Inhibit Snapping To A String Point
** It has to Be Done This Way As The Tin Is Cleaned After Inserting 1500 points
*/
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Rubber Band Tin Points Onto Insert String Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Points") ;
 if( bcdtmInsert_rubberBandTinPointsOnToInsertStringPointsDtmObject(dtmP,stringPtsP,numStringPts,&pointsMoved1)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Points Moved = %6ld",pointsMoved1) ;
/*
** Rubber Band Tin Points Onto Insert String Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Lines") ;
 if( bcdtmInsert_rubberBandTinPointsOnToInsertStringLinesDtmObject(dtmP,stringPtsP,numStringPts,&pointsMoved2)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Points Moved = %6ld",pointsMoved2) ;
/*
** Check Tin Precision
*/
 if( cdbg && ( pointsMoved1 || pointsMoved2 ) )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,1)) { bcdtmWrite_message(0,0,0,"Tin Precision Errors") ; goto errexit ; }
    else                                         bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Allocate Memory To Hold Point Numbers
*/
 pointsP = ( long * ) malloc ( numStringPts * sizeof(long)) ;
 if( pointsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Store Points In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Points") ;
 for( p3dP = stringPtsP , pntP = pointsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++pntP)
   {
    if( bcdtmInsert_storeRigidPointInDtmObject(dtmP,drapeFlag,insertFlag,p3dP->x,p3dP->y,p3dP->z,&pntNum))
      {
       bcdtmWrite_message(0,0,0,"Error Detected While Inserting Point") ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Point = %6ld ** %12.5lf %12.5lf %10.4lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
       goto errexit ;
      }
    *pntP = pntNum ;
/*
**  Mark Inserted String Point In Tin
*/
    nodeAddrP(dtmP,pntNum)->sPtr = 1 ;
   }
/*
** Check String Points And Tin Points Are Identical
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking String Points And Tin Points Are Identical") ;
    for( p3dP = stringPtsP , pntP = pointsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++pntP)
      {
       if( p3dP->x != pointAddrP(dtmP,*pntP)->x || p3dP->y != pointAddrP(dtmP,*pntP)->y )
         {
          bcdtmWrite_message(2,0,0,"String Point And Tin Point Not Identical") ;
          bcdtmWrite_message(0,0,0,"String Point[%8ld] = %15.8lf %15.8lf %10.4lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
          bcdtmWrite_message(0,0,0,"Tin    Point[%8ld] = %15.8lf %15.8lf %10.4lf",*pntP,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pntP)->z) ;
          ret = DTM_ERROR ;
         }
      }
    if( ret == DTM_ERROR ) goto errexit ;
   }
/*
** Write String Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of String pointsP = %6ld",numStringPts) ;
    for(  pntP = pointsP ; pntP < pointsP + numStringPts ; ++pntP)
      {
       pnt1P = pntP + 1 ;
       if( pnt1P >= pointsP + numStringPts ) pnt1P = pointsP + 1 ;
       bcdtmWrite_message(0,0,0,"Point[%4ld]  Np = %6ld  ** %10.4lf %10.4lf %10.4lf Angle = %12.10lf",(long)(pntP-pointsP),*pntP,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pntP)->z,bcdtmMath_getAngle(pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y)) ;
      }
   }
/*
** Check Topology And Precision - Development Only
*/
 if( cdbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,1)) { bcdtmWrite_message(0,0,0,"Tin Topology Errors") ; goto errexit ; }
    else                                       bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,1))  { bcdtmWrite_message(0,0,0,"Tin Precision Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Check For Memory Reallocation
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
** Store Lines In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines") ;
 for( pntP = pointsP + 1 ; pntP < pointsP + numStringPts ; ++pntP )
   {
    if( *(pntP-1) != *pntP )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Line %6ld of %6ld Between points %9ld %9ld",(long)(pntP-pointsP),numStringPts,*(pntP-1),*pntP) ;
       if( bcdtmInsert_rigidLineBetweenPointsDtmObject(dtmP,*(pntP-1),*pntP,drapeFlag,insertFlag) )
         {
          bcdtmWrite_message(0,0,0,"Errors Detected While Inserting Line %6ld of %6ld",(long)(pntP-pointsP),numStringPts) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Sp = %6ld ** %12.5lf %12.5lf %10.4lf",*(pntP-1),pointAddrP(dtmP,*(pntP-1))->x,pointAddrP(dtmP,*(pntP-1))->y,pointAddrP(dtmP,*(pntP-1))->z) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Ep = %6ld ** %12.5lf %12.5lf %10.4lf",*pntP,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pntP)->z) ;
          goto errexit ;
         }
       if( cdbg == 2  )
         {
          if( bcdtmCheck_precisionDtmObject(dtmP,1))
            {
             bcdtmWrite_message(0,0,0,"Tin Precision Errors After Inserting Line") ;
             bcdtmWrite_message(0,0,0,"Sp = %6ld ** %12.5lf %12.5lf %10.4lf",*(pntP-1),pointAddrP(dtmP,*(pntP-1))->x,pointAddrP(dtmP,*(pntP-1))->y,pointAddrP(dtmP,*(pntP-1))->z) ;
             bcdtmWrite_message(0,0,0,"Ep = %6ld ** %12.5lf %12.5lf %10.4lf",*pntP,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pntP)->z) ;
             goto errexit ;
            }
         }
      }
   }
/*
** Check Connectivity Of Tptr List
*/
 if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,*pointsP,1))
   {
    bcdtmWrite_message(2,0,0,"Connectivity Error In Rigid Internal String") ;
    goto errexit ;
   }
/*
** Check String Points And Tin Points Are Identical
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking String Points And Tin Points Are Identical") ;
    for( p3dP = stringPtsP , pntP = pointsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++pntP)
      {
       if( p3dP->x != pointAddrP(dtmP,*pntP)->x || p3dP->y != pointAddrP(dtmP,*pntP)->y )
         {
          bcdtmWrite_message(2,0,0,"String Point And Tin Point Not Identical") ;
          bcdtmWrite_message(0,0,0,"String Point[%8ld] = %15.8lf %15.8lf %10.4lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
          bcdtmWrite_message(0,0,0,"Tin    Point[%8ld] = %15.8lf %15.8lf %10.4lf",*pntP,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,*pntP)->z) ;
          ret = DTM_ERROR ;
         }
      }
    if( ret == DTM_ERROR ) goto errexit ;
/*
**  Checking Inserted Points Are On String
*/
    bcdtmWrite_message(0,0,0,"Checking Inserted Points Are On String") ;
    for( pntP = pointsP + 1 ; pntP < pointsP + numStringPts ; ++pntP )
      {
       if( *(pntP-1) != *pntP )
         {
          pntNum = nodeAddrP(dtmP,*(pntP-1))->tPtr ;
          while( pntNum != *pntP )
            {
             dn = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,*(pntP-1))->x,pointAddrP(dtmP,*(pntP-1))->y,pointAddrP(dtmP,*pntP)->x,pointAddrP(dtmP,*pntP)->y,pointAddrP(dtmP,pntNum)->x,pointAddrP(dtmP,pntNum)->y,&xn,&yn) ;
             if( dn > 0.00000001 )
               {
                bcdtmWrite_message(0,0,0,"Inserted Point %9ld Is Off Line By %20.15lf ** startPnt = %9ld endPnt = %9ld",pntNum,dn,*(pntP-1),*pntP) ;
                ret = DTM_ERROR ;
               }
             pntNum = nodeAddrP(dtmP,pntNum)->tPtr ;
            }
         }
      }
    if( ret != DTM_SUCCESS ) goto errexit ;
   }
/*
** Check Topology And Precision - Development Only
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,1)) { bcdtmWrite_message(0,0,0,"Tin Topology Errors") ; goto errexit ; }
    else                                        bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,1))  { bcdtmWrite_message(0,0,0,"Tin Precision Errors") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Set Start Point
*/
 *startPntP = *pointsP  ;
/*
** Write Some Tin Statistics After Insertion
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints       = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->incPoints       = %8ld",dtmP->incPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Points Inserted = %6ld",dtmP->numPoints-dtmP->numSortedPoints) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pointsP != NULL ) free(pointsP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Rigid Internal String Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Rigid Internal String Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_storeRigidPointInDtmObject(BC_DTM_OBJ *dtmP,long drapeFlag,long insertFlag,double x,double y,double z,long *dtmPntNumP)
/*
** This Function Stores A Rigid Point In The Tin Object
**
** drapeFlag    = 1   Drape Intersect Vertices On Tin Surface
**              = 2   Break Intersect Vertices On Tin Surface
** insertFlag   = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pntType,dtmPnt1,dtmPnt2,dtmPnt3,newTinPnt,antPnt=0,clkPnt=0,voidFlag=0 ;
 long   onLine1,onLine2,onLine3,sp1=0,sp2=0,sp3=0,spt,perr,moveFlag ;
 double surfaceZ=0.0,d1,d2,d3,Xi,Yi ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Rigid Point In Tin ** drapeFlag = %2ld insertFlag = %2ld ** %15.8lf %15.8lf %10.4lf",drapeFlag,insertFlag,x,y,z ) ;
/*
** Initialise
*/
 *dtmPntNumP = dtmP->nullPnt ;
/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&surfaceZ,&pntType,&dtmPnt1,&dtmPnt2,&dtmPnt3)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"pntType = %2ld ** dtmPnt1 = %9ld dtmPnt2 = %9ld dtmPnt3 = %9ld",pntType,dtmPnt1,dtmPnt2,dtmPnt3) ;
/*
** If Point External - Find Closest Hull Line
*/
 if( pntType == 0 )
   {
bcdtmWrite_message(0,0,0,"Internal Point External To Tin") ;
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,x,y,&surfaceZ,&pntType,&dtmPnt1,&dtmPnt2)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"pntType = %2ld dtmPnt1 = %9ld dtmPnt2 = %9ld",pntType,dtmPnt1,dtmPnt2) ;
    if( pntType == 2 )
      {
       d1 = bcdtmMath_distance(pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,x,y) ;
       d2 = bcdtmMath_distance(pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,x,y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Pptol = %20.15lf D1 = %15.12lf D2 = %15.12lf",dtmP->ppTol,d1,d2) ;
       if( d1 <= d2 && d1 <= dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt1)->sPtr == dtmP->nullPnt ) { pntType = 1 ; dtmPnt2 = dtmPnt3 = DTM_NULL_PNT ; }}
       if( d2 <= d1 && d2 <= dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt2)->sPtr == dtmP->nullPnt ) { pntType = 1 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = dtmPnt3 = DTM_NULL_PNT ;}}
       if( pntType == 2 ) pntType = 3 ;
      }
/*
**  Write Hull Point Type
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Point Type = %2ld",pntType) ;
       if( dtmPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"dtmPnt1 = %6ld dtmPnt1->fTableP = %9ld ** %10.4lf %10.4lf %10.4lf",dtmPnt1,nodeAddrP(dtmP,dtmPnt1)->hPtr,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt1)->z) ;
       if( dtmPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"dtmPnt2 = %6ld dtmPnt2->fTableP = %9ld ** %10.4lf %10.4lf %10.4lf",dtmPnt2,nodeAddrP(dtmP,dtmPnt2)->hPtr,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,pointAddrP(dtmP,dtmPnt2)->z) ;
      }
/*
** Check For A Precision Problem
*/
    perr = 0 ;
    if( pntType == 3 ) if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;

    if( perr ) bcdtmWrite_message(0,0,0,"perr = %2ld",perr) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"perr = %2ld",perr) ;
       bcdtmList_writeCircularListForPointDtmObject(dtmP,antPnt) ;
      }
/*
** If A Precision Problem Retriangulate About Triangle Point
** Following Code Added 4/8/03  Rob C
** Code Removes Slivers Triangles
*/
    if( perr )
      {
       if( bcdtmInsert_triangulateAboutPointDtmObject(dtmP,antPnt)) goto errexit ;
       if( bcdtmInsert_triangulateAboutPointDtmObject(dtmP,dtmPnt1)) goto errexit ;
       if( bcdtmInsert_triangulateAboutPointDtmObject(dtmP,dtmPnt2)) goto errexit ;
       if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"perr = %2ld",perr) ;
      }
/*
**  Following Code Added 11/12/03  Rob C
**  Code Removes Slivers On Internal Side Of Hull
*/
    if( perr == 1 )
      {
       if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,antPnt,dtmPnt2))
         {
          if( ( newTinPnt = bcdtmList_nextAntDtmObject(dtmP,antPnt,dtmPnt2)) < 0 ) goto errexit ;
          if( bcdtmMath_pointSideOfDtmObject(dtmP,dtmPnt1,newTinPnt,antPnt) > 0 &&
              bcdtmMath_pointSideOfDtmObject(dtmP,dtmPnt1,newTinPnt,dtmPnt2) < 0    )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Sliver") ;
             if( bcdtmList_deleteLineDtmObject(dtmP,antPnt,dtmPnt2)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt1,newTinPnt,antPnt)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt1,dtmPnt2)) goto errexit ;
             antPnt = newTinPnt ;
             if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"perr = %2ld",perr) ;
            }
         }
      }
/*
** If Precision Error Check Preceding Hull Line
*/
    if( perr )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error ** Checking Preceding Hull Line") ;
       sp1 = dtmPnt1 ; sp2 = dtmPnt2 ;
       if(( dtmPnt2 = bcdtmList_nextClkDtmObject(dtmP,dtmPnt1,dtmPnt2)) < 0 ) goto errexit ;
       sp3 = dtmPnt1 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = sp3 ;
       if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;
      }
/*
** If Precision Error Check Following Hull Line
*/
    if( perr )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error ** Checking Following Hull Line") ;
       dtmPnt1 = sp1  ; dtmPnt2 = sp2 ;
       if(( dtmPnt2 = bcdtmList_nextAntDtmObject(dtmP,dtmPnt2,dtmPnt1)) < 0 ) goto errexit ;
       dtmPnt1 = sp2 ;
       if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;
      }
    if( perr ) pntType = 0 ;
    if( perr && dbg ) bcdtmWrite_message(0,0,0,"Precision Error With Rigid Hull Point") ;
   }
/*
** Save Current Types
*/
 sp1 = dtmPnt1 ;
 sp2 = dtmPnt2 ;
 sp3 = dtmPnt3 ;
 spt = pntType ;
/*
** Check Point To Line Tolerances
*/
 if( pntType == 4  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point To Line Tolerance Triangle") ;
    d1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,x,y,&Xi,&Yi) ;
    d2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y,x,y,&Xi,&Yi) ;
    d3 = bcdtmMath_distanceOfPointFromLine(&onLine3,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,x,y,&Xi,&Yi) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"plTolol = %20.15lf D1 = %15.12lf D2 = %15.12lf D3 = %15.12lf",dtmP->plTol,d1,d2,d3) ;
    if     ( onLine1 && d1 <= d2 && d1 <= d3 && d1 < dtmP->plTol ) { pntType = 2 ; dtmPnt3 = dtmP->nullPnt ; }
    else if( onLine2 && d2 <= d3 && d2 <= d1 && d2 < dtmP->plTol ) { pntType = 2 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = dtmPnt3 ; dtmPnt3 = dtmP->nullPnt ; }
    else if( onLine3 && d3 <= d1 && d3 <= d2 && d3 < dtmP->plTol ) { pntType = 2 ; dtmPnt2 = dtmPnt1 ; dtmPnt1 = dtmPnt3 ; dtmPnt3 = dtmP->nullPnt ; }
    if( pntType == 2 )
      {
       if      ( nodeAddrP(dtmP,dtmPnt1)->hPtr == dtmPnt2 )   pntType = 3 ;
       else if ( nodeAddrP(dtmP,dtmPnt2)->hPtr == dtmPnt1 ) { pntType = 3 ; dtmPnt3 = dtmPnt1 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = dtmPnt3 ; dtmPnt3 = dtmP->nullPnt ; }
      }
   }
/*
** Check Point To Point Tolerances For Triangle
*/
 if( pntType == 4 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point To Line Tolerance Triangle") ;
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) ;
    d3 = bcdtmMath_distance(x,y,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y) ;
    if      ( d1 <= d2 && d1 <= d3 && d1 < dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt1)->sPtr == dtmP->nullPnt ) { pntType = 1 ; dtmPnt2 = dtmPnt3 = dtmP->nullPnt ; }}
    else if ( d2 <= d3 && d2 <= d1 && d2 < dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt2)->sPtr == dtmP->nullPnt ) { pntType = 1 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = dtmPnt3 = dtmP->nullPnt ; }}
    else if ( d3 <= d1 && d3 <= d2 && d3 < dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt3)->sPtr == dtmP->nullPnt ) { pntType = 1 ; dtmPnt1 = dtmPnt3 ; dtmPnt2 = dtmPnt3 = dtmP->nullPnt ; }}
   }
/*
** Check Point To Point Tolerance For Line
*/
 if( pntType == 2 || pntType == 3 && dtmP->ppTol > 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point To Line Tolerance Line") ;
    d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
    d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) ;
    if      ( d1 <= d2 && d1 < dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt1)->sPtr == dtmP->nullPnt ) {pntType = 1 ; dtmPnt2 = dtmPnt3 = dtmP->nullPnt ; }}
    else if ( d2 <= d1 && d2 < dtmP->ppTol ) { if( nodeAddrP(dtmP,dtmPnt2)->sPtr == dtmP->nullPnt ) {pntType = 1 ; dtmPnt1 = dtmPnt2 ; dtmPnt2 = dtmPnt3 = dtmP->nullPnt ; }}
   }
/*
** Check Precision For pntType == 1  Points
*/
 if( pntType == 1 && ( pointAddrP(dtmP,dtmPnt1)->x != x || pointAddrP(dtmP,dtmPnt1)->y != y ) )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point Precision") ;
    if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,dtmPnt1,x,y,&moveFlag)) goto errexit ;
    if( moveFlag )
      {
       pointAddrP(dtmP,dtmPnt1)->x = x ;
       pointAddrP(dtmP,dtmPnt1)->y = y ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Point Move Precision Error ** %10.4lf %10.4lf %10.4lf",x,y,z) ;
       dtmPnt1 = sp1 ; dtmPnt2 = sp2 ; dtmPnt3 = sp3 ; pntType = spt ;
      }
   }
/*
** Check Precision For pntType == 2 && pntType == 3  Lines
*/
 if( pntType == 2 || pntType == 3 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point Line Precision") ;
    if( bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,dtmPnt1,dtmPnt2,x,y,&perr,&antPnt,&clkPnt)) goto errexit ;
    if( perr )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Line Insert Precision Error pntType = %2ld ** %10.4lf %10.4lf %10.4lf",pntType,x,y,z) ;
       if( pntType == 2 ) { dtmPnt1 = sp1 ; dtmPnt2 = sp2 ; dtmPnt3 = sp3 ; pntType = spt ; }
      }
   }
/*
** Set z value For Point
*/
 if( pntType && drapeFlag == 1 ) z = surfaceZ ;
/*
** Add Point To Tin
*/
 if( pntType == 1 ) newTinPnt = dtmPnt1 ;
 else              { if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&newTinPnt)) goto errexit ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point[%6ld] ** Type = %2ld ** dtmPnt1 = %6ld dtmPnt2 = %9ld dtmPnt3 = %9ld ** %15.8lf %15.8lf %10.4lf",newTinPnt,pntType,dtmPnt1,dtmPnt2,dtmPnt3,pointAddrP(dtmP,newTinPnt)->x,pointAddrP(dtmP,newTinPnt)->y,pointAddrP(dtmP,newTinPnt)->z) ;
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
      newTinPnt = dtmPnt1 ;
      pointAddrP(dtmP,newTinPnt)->z = z ;
    break   ;

    case  2 :      /* Coincident With Internal Tin Line  */

      bcdtmList_testForVoidLineDtmObject(dtmP,dtmPnt1,dtmPnt2,&voidFlag) ;
      if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,dtmPnt1,dtmPnt2))   < 0 ) goto errexit ;
      if( (clkPnt = bcdtmList_nextClkDtmObject(dtmP,dtmPnt1,dtmPnt2)) < 0 ) goto errexit ;
      if(bcdtmList_deleteLineDtmObject(dtmP,dtmPnt1,dtmPnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt1,newTinPnt,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt2,newTinPnt,clkPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt2,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,newTinPnt,dtmPnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,antPnt,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,clkPnt,newTinPnt,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,clkPnt,dtmPnt2)) goto errexit ;
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,dtmPnt1,dtmPnt2) )
         {
          if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,dtmPnt1,dtmPnt2,newTinPnt)) goto errexit ;
         }
    break ;

    case  3 :      /* Coincident With External Tin Line  */
      bcdtmList_testForVoidLineDtmObject(dtmP,dtmPnt1,dtmPnt2,&voidFlag) ;
      if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,dtmPnt1,dtmPnt2))   < 0 ) goto errexit ;
      if(bcdtmList_deleteLineDtmObject(dtmP,dtmPnt1,dtmPnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt1,newTinPnt,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineBeforePointDtmObject(dtmP,dtmPnt2,newTinPnt,antPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt2,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,newTinPnt,dtmPnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,antPnt,dtmPnt1)) goto errexit ;
      if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,dtmPnt1,dtmPnt2) )
        {
         if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,dtmPnt1,dtmPnt2,newTinPnt)) goto errexit ;
        }
      nodeAddrP(dtmP,dtmPnt1)->hPtr = newTinPnt ;
      nodeAddrP(dtmP,newTinPnt)->hPtr = dtmPnt2 ;
    break ;

    case  4 :   /* In Triangle                      */
      bcdtmList_testForVoidTriangleDtmObject(dtmP,dtmPnt1,dtmPnt2,dtmPnt3,&voidFlag) ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt1,newTinPnt,dtmPnt2)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt1,dtmP->nullPnt)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt2,newTinPnt,dtmPnt3)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt2,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt3,newTinPnt,dtmPnt1)) goto errexit ;
      if(bcdtmList_insertLineAfterPointDtmObject(dtmP,newTinPnt,dtmPnt3,dtmPnt2)) goto errexit ;
    break ;

    default :
      bcdtmWrite_message(2,0,0,"Illegal Point Find Code %6ld",pntType) ;
      goto errexit ;
    break   ;
   } ;
/*
** If Point In Void Set Void Bit
*/
 if( voidFlag ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,newTinPnt)->PCWD) ;
/*
** Set Point Value For Return
*/
 *dtmPntNumP = newTinPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Rigid Point In Tin Completed" ) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Rigid Point In Tin Error" ) ;
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
BENTLEYDTM_Private int bcdtmInsert_rigidLineBetweenPointsDtmObject(BC_DTM_OBJ *dtmP,long firstPnt,long lastPnt,long drapeFlag,long insertFlag)
/*
** This Function Inserts A Line Between Two Points In a Tin Object
**
** dtmP        =     Pointer To Tin Object
** firstPnt    =     First Point Of Line
** lastPnt     =     Last Point Of Line
** drapeFlag   = 1   Drape On Surface
**             = 2   Break Over Surface
** insertFlag  = 1   Move Tin Lines That Intersect Insert Line
**             = 2   Intersect Tin Lines
**
*/
{
 int    ret=DTM_SUCCESS,bkp,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   dtmPnt1,dtmPnt2,dtmPnt3,dtmPnt4,startPnt,endPnt,dtmFeatureLine,voidLine,numPreErrors=0 ;
 double intX,intY,intZ=0.0,ppTol=0.0,plTol=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Rigid Line From firstPnt = %6ld To lastPnt = %6ld",firstPnt,lastPnt) ;
/*
** Initialise
*/
 dtmPnt1  = dtmP->nullPnt ;
 dtmPnt2  = dtmP->nullPnt ;
 dtmPnt3  = dtmP->nullPnt ;
 startPnt = firstPnt ;
 endPnt   = lastPnt ;
 ppTol    = dtmP->ppTol ;
 plTol    = dtmP->plTol ;
 dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
/*
** Swap Tin Lines That Intersect Insert Line
*/
 if( insertFlag == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Swapping Tin Lines") ;
    if( bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject(dtmP,firstPnt,lastPnt)) // Was bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject
      {
       bcdtmWrite_message(1,0,0,"Error Swapping Lines firstPnt = %6ld lastPnt = %6ld",firstPnt,lastPnt) ;
       goto errexit ;
      }
   }
/*
** Insert Line
*/
 while ( firstPnt != lastPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** firstPnt = %6ld lastPnt = %6ld Angle = %15.12lf",firstPnt,lastPnt,bcdtmMath_getPointAngleDtmObject(dtmP,firstPnt,lastPnt)) ;
/*
**   Check For Knot If So Return
*/
    if( nodeAddrP(dtmP,firstPnt)->tPtr != dtmP->nullPnt )
      {
       bcdtmWrite_message(1,0,0,"Knot Detected 01 ** startPnt = %6ld endPnt = %6ld",startPnt,endPnt) ;
       ret = 2 ;
       goto errexit  ;
      }
/*
**  Get Next Point
*/
    if( dbg )bcdtmWrite_message(0,0,0,"Line = %8ld 68998->tPtr = %10ld",firstPnt,nodeAddrP(dtmP,68998)->tPtr) ;
    bkp = bcdtmInsert_getRigidIntersectPointDtmObject(dtmP,firstPnt,lastPnt,dtmPnt3,&dtmFeatureLine,&dtmPnt1,&dtmPnt2,&dtmPnt3,&intX,&intY) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"bkp = %2ld ** %7ld %9ld %9ld ** %10.4lf %10.4lf",bkp,dtmPnt1,dtmPnt2,dtmPnt3,intX,intY) ;
/*
**  Check For Precision Error
*/
    if( bkp == 10 )
      {
       dtmPnt3 = dtmP->nullPnt ;
       ++numPreErrors ;
       if( numPreErrors > 4 )
         {
          bcdtmWrite_message(2,0,0,"Cannot Fix Precision Error") ;
          goto errexit ;
         }
      }
    else numPreErrors = 0 ;
/*
**  Check For DTM System Error
*/
    if( bkp == 0  ) { bcdtmWrite_message(2,0,0,"No Intersect Point Found") ; goto errexit ; }
/*
**  Check For Knot
*/
    if( bkp == 8  ) { bcdtmWrite_message(1,0,0,"Knot Detected 02") ; ret = 2 ; goto errexit  ; }
/*
**  Get z Value Of Intecept Point
*/
    if( bkp == 1 || bkp == 2 || bkp == 3 )
      {
       if( bkp == 1 )      { intX = pointAddrP(dtmP,dtmPnt1)->x ; intY = pointAddrP(dtmP,dtmPnt1)->y ; }
       if( drapeFlag == 1 )  bcdtmInsert_getZvalueDtmObject(dtmP,dtmPnt1,dtmPnt2,intX,intY,&intZ) ;
       else                  bcdtmInsert_getZvalueDtmObject(dtmP,startPnt,endPnt,intX,intY,&intZ) ;
      }
/*
**  Passes Through Tin Point
*/
    if( bkp == 1 )
      {
       nodeAddrP(dtmP,firstPnt)->tPtr = dtmPnt1 ;
       pointAddrP(dtmP,dtmPnt1)->z = intZ ;
       firstPnt = dtmPnt1 ;
      }
/*
**  Intersects Internal Line
*/
    if( bkp == 2 || bkp == 3 )
      {
/*
**     Check For Knot If So Return
*/
       if( nodeAddrP(dtmP,dtmPnt1)->tPtr == dtmPnt2 || nodeAddrP(dtmP,dtmPnt2)->tPtr == dtmPnt1  ) { ret = 2 ; goto errexit ;}
/*
**     Check For Void Line
*/
       bcdtmList_testForVoidLineDtmObject(dtmP,dtmPnt1,dtmPnt2,&voidLine) ;
       if( bcdtmList_deleteLineDtmObject(dtmP,dtmPnt1,dtmPnt2) ) goto errexit ;
       if( bcdtmInsert_addPointToDtmObject(dtmP,intX,intY,intZ,&dtmPnt4) ) goto errexit ;
       if( voidLine ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,dtmPnt4)->PCWD) ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,firstPnt,dtmPnt4,dtmPnt1)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt4,firstPnt,dtmP->nullPnt)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,dtmPnt1,dtmPnt4,firstPnt)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt4,dtmPnt1,firstPnt) ) goto errexit;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt2,dtmPnt4,firstPnt) ) goto errexit;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,dtmPnt4,dtmPnt2,firstPnt)) goto errexit ;
       if( dtmPnt3 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt3,dtmPnt4,dtmPnt2)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt4,dtmPnt3,dtmPnt1)) goto errexit ;
         }
/*
**     If Intersecting Tin Hull Update Hull Pointers
*/
       if( bkp == 3 )
         {
          if(nodeAddrP(dtmP,dtmPnt1)->hPtr == dtmPnt2 ) { nodeAddrP(dtmP,dtmPnt1)->hPtr = dtmPnt4 ;nodeAddrP(dtmP,dtmPnt4)->hPtr = dtmPnt2 ; }
          if(nodeAddrP(dtmP,dtmPnt2)->hPtr == dtmPnt1 ) { nodeAddrP(dtmP,dtmPnt2)->hPtr = dtmPnt4 ;nodeAddrP(dtmP,dtmPnt4)->hPtr = dtmPnt1 ; }
         }
/*
**     If Intersecting A DTM Feature Line Update Feature List Structure
*/
       if( dtmFeatureLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,dtmPnt1,dtmPnt2,dtmPnt4)) goto errexit ;
/*
**     Perform Checks
*/
       if( cdbg )
         {
          if( bcdtmCheck_precisionDtmObject(dtmP,1))
            {
             bcdtmWrite_message(0,0,0,"Tin Precision Errors After Inserting Line") ;
             bcdtmWrite_message(0,0,0,"firstPnt = %6ld ** %12.5lf %12.5lf %10.4lf",firstPnt,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,firstPnt)->z) ;
             bcdtmWrite_message(0,0,0,"P4 = %6ld ** %12.5lf %12.5lf %10.4lf",dtmPnt4,pointAddrP(dtmP,dtmPnt4)->x,pointAddrP(dtmP,dtmPnt4)->y,pointAddrP(dtmP,dtmPnt4)->z) ;
             goto errexit ;
            }
         }
/*
**     Update Temporary Pointer  Array
*/
       nodeAddrP(dtmP,firstPnt)->tPtr = dtmPnt4 ;
       firstPnt = dtmPnt4 ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 dtmP->ppTol = ppTol ;
 dtmP->plTol = plTol ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal Rigid Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal Rigid Line Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_getRigidIntersectPointDtmObject(BC_DTM_OBJ *dtmP,long firstPnt,long lastPnt,long indexPnt,long *dtmFeatureLineP,long *dtmPnt1P,long *dtmPnt2P,long *dtmPnt3P,double *intXP,double *intYP)
/*
** This Function Gets The Next Tin Line Intercept with firstPnt-lastPnt
**
** Return Values  ==  0 Error Terminate
**                ==  1 firstPnt-lastPnt Intecepts Point dtmPnt1P
**                ==  2 firstPnt-lastPnt Intercepts Internal Line dtmPnt1P-dtmPnt2P
**                ==  3 firstPnt-lastPnt Intercepts Hull Line dtmPnt1P-dtmPnt2P
**                ==  6 Point Merged Continue Processing
**                ==  8 Knot Will Be Inserted In Tptr Array
**                == 10 Precison Error Detected And Fixed
**
*/
{
 int    sdp1,sdp2,dbg=DTM_TRACE_VALUE(0) ;
 long   p1=0,p2=0,p3=0,clPtr,sp1,sp2,cont=1,internalFlag=0,onLine1,onLine2,antPnt,clkPnt,preError,moveFlag ;
 long   hullPnt,hullPnt1,hullPnt2,fpPtr,movePoint ;
 DTMDirection direction;
 double dp1,dp2,dfp,dp3,nd1,nd2,n1X,n1Y,n2X,n2Y,fpArea,hullArea ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Rigid Intersect Point") ;
    bcdtmWrite_message(0,0,0,"firstPnt = %9ld ** %12.5lf %12.5lf,%10.4lf",firstPnt,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,firstPnt)->z) ;
    bcdtmWrite_message(0,0,0,"lastPnt  = %9ld ** %12.5lf %12.5lf,%10.4lf",lastPnt,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,lastPnt)->z) ;
    bcdtmWrite_message(0,0,0,"indexPnt = %9ld",indexPnt ) ;
   }
/*
** Initialise Variables
*/
 *dtmPnt1P = dtmP->nullPnt ;
 *dtmPnt2P = dtmP->nullPnt ;
 *dtmPnt3P = dtmP->nullPnt ;
 *intXP    = 0.0 ;
 *intYP    = 0.0 ;
 sp1       = dtmP->nullPnt ;
 sp2       = dtmP->nullPnt ;
/*
**  Test If firstPnt and lastPnt Connected
*/
 if( bcdtmList_testLineDtmObject(dtmP,firstPnt,lastPnt)) { *dtmPnt1P = lastPnt ; return(1) ; }
/*
** If Index Point Is Null Scan Cyclic List
*/
 if( indexPnt == dtmP->nullPnt )
   {
    clPtr = nodeAddrP(dtmP,firstPnt)->cPtr  ;
    p2  = clistAddrP(dtmP,clPtr)->pntNum ;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,p2)) < 0 ) return(0) ;
    while ( clPtr  != dtmP->nullPtr && cont )
      {
       p2  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
       if( nodeAddrP(dtmP,firstPnt)->hPtr == p1) { sp1 = p1 ; sp2 = p2 ; }
       else
         {
          if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,firstPnt) < 0 )
            {
             if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,lastPnt) > 0 )
               {
                sdp1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p1) ;
                if( sdp1 == 0 ) { *dtmPnt1P = p1 ; return(1) ; }
                sdp2 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,p2) ;
                if( sdp2 == 0 ) { *dtmPnt1P = p2 ; return(1) ; }
                if( sdp1 == 1 && sdp2 == -1 ) { internalFlag = 1 ; cont = 0 ; }
               }
            }
         }
       if( cont ) p1  = p2 ;
      }
    if( cont ) { p1 = sp1 ; p2 = sp2 ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"00 p1 = %6ld p2 = %6ld Internal Flag = %2ld",p1,p2,internalFlag) ;
/*
** Note - Required For Development Purposes In case Of Precision Fuck Up
*/
    if( p1 == dtmP->nullPnt )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Unable Fo Find Intersection With Triangle Edge") ;
          bcdtmWrite_message(0,0,0,"firstPnt = %6ld lastPnt = %6ld indexPnt = %9ld",firstPnt,lastPnt,indexPnt) ;
          bcdtmList_writeCircularListForPointDtmObject(dtmP,firstPnt) ;
          bcdtmWrite_message(0,0,0,"FPLP Angle = %20.15lf",bcdtmMath_getPointAngleDtmObject(dtmP,firstPnt,lastPnt)) ;
         }
/*
**     Scan Point For Angle Intersection With Triangle Base
*/
       if( bcdtmInsert_scanPointForPointAngleIntersectionWithTriangleBaseDtmObject(dtmP,firstPnt,lastPnt,&p1,&p2)) return(0) ;
       if( p2 == dtmP->nullPnt ) { *dtmPnt1P = p1 ; return(1) ; }
/*
**     Check For Line Going External
*/
       if( nodeAddrP(dtmP,p2)->hPtr == firstPnt ) internalFlag = 0 ;
      }
   }
/*
** Look At Points Either Side Of Index Point
*/
 if( indexPnt != dtmP->nullPnt )
   {
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,firstPnt,indexPnt))   < 0 ) return(0) ;
    if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,firstPnt,indexPnt)) < 0 ) return(0) ;
    sdp1 = bcdtmMath_pointSideOfDtmObject(dtmP,firstPnt,lastPnt,indexPnt) ;
    if( sdp1 ==  0 ) { *dtmPnt1P = indexPnt ; return(1) ; }
    if( sdp1 == -1 )  p2 = indexPnt ;
    if( sdp1 ==  1 )  p1 = indexPnt ;
    internalFlag = 1 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"01 p1 = %6ld p2 = %6ld",p1,p2) ;
/*
**  If Internal Get Intecept Point
*/
 if( internalFlag )
   {
/*
**  Test If An Intersection Will A Cause Knot In The Insert String
*/
    if( nodeAddrP(dtmP,p1)->tPtr == p2 ) return(8) ;
    if( nodeAddrP(dtmP,p2)->tPtr == p1 ) return(8) ;
/*
**  Test If dtmPnt1P-dtmPnt2P is A Dtm Feature Line
*/
    *dtmFeatureLineP = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ;
/*
**  Calculate Normal Distance To Insert Line From p1 and p2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Point To Line") ;
    nd1 = bcdtmMath_distanceOfPointFromLine(&onLine1,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&n1X,&n1Y) ;
    nd2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&n2X,&n2Y) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"onLine1 = %2ld ** nd1 = %20.15lf ** xi = %20.15lf yi = %20.15lf",onLine1,nd1,n1X,n1Y) ;
       bcdtmWrite_message(0,0,0,"onLine2 = %2ld ** nd2 = %20.15lf ** xi = %20.15lf yi = %20.15lf",onLine2,nd2,n2X,n2Y) ;
      }
/*
**  Determine If normal Distance To Insert Line Is Within Point To Line Tolerance
*/
    if( nd1 >= dtmP->plTol ) onLine1 = 0 ;
    if( nd2 >= dtmP->plTol ) onLine2 = 0 ;
/*
**  Can Not Snap To A Previously Inserted String Point
*/
    if( nodeAddrP(dtmP,p1)->tPtr != dtmP->nullPnt ) onLine1 = 0 ;
    if( nodeAddrP(dtmP,p2)->tPtr != dtmP->nullPnt ) onLine2 = 0 ;
/*
**  Snap To Closest Point
*/
    if ( onLine1 && onLine2 )
      {
       if( nd1 <= nd2  )  onLine2 = 0 ;
       else               onLine1 = 0 ;
      }
/*
**  Only Snap If Point Can Be Moved Onto Insert Line
*/
    if( onLine1  )
      {
       if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p1,n1X,n1Y,&moveFlag)) return(0) ;
       if( moveFlag )
         {
          pointAddrP(dtmP,p1)->x = n1X ;
          pointAddrP(dtmP,p1)->y = n1Y ;
          *dtmPnt1P = p1 ;
          *dtmPnt2P = dtmP->nullPnt ;
          *dtmPnt3P = dtmP->nullPnt ;
          return(1) ;
         }
      }
    else if( onLine2  )
      {
       if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p2,n2X,n2Y,&moveFlag)) return(0) ;
       if( moveFlag )
         {
          pointAddrP(dtmP,p2)->x = n2X ;
          pointAddrP(dtmP,p2)->y = n2Y ;
          *dtmPnt1P = p2 ;
          *dtmPnt2P = dtmP->nullPnt ;
          *dtmPnt3P = dtmP->nullPnt ;
           return(1) ;
         }
      }
/*
**  Get Point On Opposite of Line p1-p2 to firstPnt
*/
    if( (p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) return(0) ;
    if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3)) p3 = dtmP->nullPnt ;
/*
**  Intersect Insert Line And p1-p2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Intersection") ;
    bcdtmInsert_normalIntersectInsertLineDtmObject(dtmP,firstPnt,lastPnt,p1,p2,intXP,intYP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"intXP = %20.15lf intYP = %20.15lf",*intXP,*intYP) ;
/*
**  Test For Intersection Point Equal To p1 or p2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Equality") ;
    if( pointAddrP(dtmP,p1)->x == *intXP && pointAddrP(dtmP,p1)->y == *intYP ) { *dtmPnt1P = p1 ; return(1) ; }
    if( pointAddrP(dtmP,p2)->x == *intXP && pointAddrP(dtmP,p2)->y == *intYP ) { *dtmPnt1P = p2 ; return(1) ; }
/*
**  Test Point To Point Tolerance Of Intersection Point with dtmPnt1P && dtmPnt2P
**  Note Points Marked By sPtr Cannot Be Snapped To
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Point To Point") ;
    dp1  = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
    dp2  = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
    dfp  = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dp1 = %20.15lf dp2 = %20.15lf ** ppTol = %20.15lf",dp1,dp2,dtmP->ppTol) ;
    if( nodeAddrP(dtmP,p1)->tPtr != dtmP->nullPnt  ) dp1 = dtmP->ppTol ;
    if( nodeAddrP(dtmP,p2)->tPtr != dtmP->nullPnt  ) dp2 = dtmP->ppTol ;
    if( dp1 <= dp2 && dp1 < dtmP->ppTol  )
      {
       if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p1,*intXP,*intYP,&moveFlag)) return(0) ;
       if( moveFlag )
         {
          pointAddrP(dtmP,p1)->x = *intXP ;
          pointAddrP(dtmP,p1)->y = *intYP ;
          *dtmPnt1P = p1 ;
          return(1) ;
         }
      }
    else if( dp2 < dtmP->ppTol  )
      {
       if( bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p2,*intXP,*intYP,&moveFlag)) return(0) ;
       if( moveFlag )
         {
          pointAddrP(dtmP,p2)->x = *intXP ;
          pointAddrP(dtmP,p2)->y = *intYP ;
          *dtmPnt1P = p2 ;
          return(1) ;
         }
      }
/*
** Check For Potential Precision Error
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Precision") ;
    bcdtmInsert_checkForPointLinePrecisionErrorDtmObject(dtmP,p1,p2,*intXP,*intYP,&preError,&antPnt,&clkPnt) ;
/*
**  Potential Precision Error Detected If Intersect Point Inserted
*/
    if( preError )
      {
/*
**     Write Precision Error Details For Debugging Purposes
*/
       if( dbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Precision Error %3ld Detected ** antPnt = %9ld clkPnt = %9ld",preError,antPnt,clkPnt) ;
          bcdtmWrite_message(0,0,0,"intXP = %20.15lf intYP = %20.15lf",*intXP,*intYP) ;
          bcdtmWrite_message(0,0,0,"firstPnt = %9ld ** %12.5lf %12.5lf %10.4lf",firstPnt,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y,pointAddrP(dtmP,firstPnt)->z) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %9ld ** %12.5lf %12.5lf %10.4lf",lastPnt,pointAddrP(dtmP,lastPnt)->x,pointAddrP(dtmP,lastPnt)->y,pointAddrP(dtmP,lastPnt)->z) ;
          bcdtmWrite_message(0,0,0,"p1 = %9ld ** %12.5lf %12.5lf %10.4lf ** tPtr = %10ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->tPtr) ;
          bcdtmWrite_message(0,0,0,"p2 = %9ld ** %12.5lf %12.5lf %10.4lf ** tPtr = %10ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->tPtr) ;
          bcdtmWrite_message(0,0,0,"p3 = %9ld ** %12.5lf %12.5lf %10.4lf ** tPtr = %10ld",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z,nodeAddrP(dtmP,p3)->tPtr) ;
          dp1 = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
          dp2 = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
          dfp = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,firstPnt)->x,pointAddrP(dtmP,firstPnt)->y) ;
          if( p3 != dtmP->nullPnt ) dp3 = bcdtmMath_distance(*intXP,*intYP,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
          else                      dp3 = 0.0 ;
          bcdtmWrite_message(0,0,0,"dp1 = %20.15lf",dp1) ;
          bcdtmWrite_message(0,0,0,"dp2 = %20.15lf",dp2) ;
          bcdtmWrite_message(0,0,0,"dfp = %20.15lf",dfp) ;
          if( p3 != dtmP->nullPnt )
            {
             bcdtmWrite_message(0,0,0,"dp3 = %20.15lf",dp3) ;
             bcdtmWrite_message(0,0,0,"dp23 = %20.15lf",bcdtmMath_pointDistanceDtmObject(dtmP,p2,p3)) ;
            }

          bcdtmWrite_message(0,0,0,"Included Angle fp-p1-p2 = %12.10lf",bcdtmMath_calculateIncludedPointAngleDtmObject(dtmP,firstPnt,p1,p2)) ;
          bcdtmWrite_message(0,0,0,"Included Angle p3-p2-p1 = %12.10lf",bcdtmMath_calculateIncludedPointAngleDtmObject(dtmP,p3,p2,p1)) ;
          bcdtmWrite_message(0,0,0,"sideOf(p1,p2,p3) = %2ld",bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,p3)) ;
          bcdtmWrite_message(0,0,0,"sideOf(p2,p3,p1) = %2ld",bcdtmMath_pointSideOfDtmObject(dtmP,p2,p3,p1)) ;
          bcdtmWrite_message(0,0,0,"sideOf(p3,p1,p2) = %2ld",bcdtmMath_pointSideOfDtmObject(dtmP,p3,p1,p2)) ;
         }
/*
**     Fix Precision Error
*/
       if( preError == 1 && nodeAddrP(dtmP,p1)->tPtr == dtmP->nullPnt )
         {
          bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p1,*intXP,*intYP,&movePoint) ;
          if( movePoint )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Moving Point 1") ;
             pointAddrP(dtmP,p1)->x = *intXP ;
             pointAddrP(dtmP,p1)->y = *intYP ;
             *dtmPnt1P = p1 ;
             *dtmPnt2P = dtmP->nullPnt ;
             *dtmPnt3P = dtmP->nullPnt ;
             return(1) ;
            }
         }
         if( preError == 2 && nodeAddrP(dtmP,p2)->tPtr == dtmP->nullPnt )
           {
            bcdtmInsert_checkPointCanBeMovedDtmObject(dtmP,p2,*intXP,*intYP,&movePoint) ;
            if( movePoint )
              {
               if( dbg ) bcdtmWrite_message(0,0,0,"Moving Point 2") ;
               pointAddrP(dtmP,p2)->x = *intXP ;
               pointAddrP(dtmP,p2)->y = *intYP ;
               *dtmPnt1P = p2 ;
               *dtmPnt2P = dtmP->nullPnt ;
               *dtmPnt3P = dtmP->nullPnt ;
               return(1) ;
              }
           }

       if( preError == 1 || preError == 2  )
         {
          *dtmPnt1P = p1 ;
          *dtmPnt2P = p2 ;
          *dtmPnt3P = p3 ;
          if( p3 == dtmP->nullPnt ) return(3) ;
          else                      return(2) ;
         }


       if( preError == 1 || preError == 2  )
         {
          if( bcdtmInsert_removeSliverTriangleDtmObject(dtmP,p1,p3,p2)) return(1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Sliver %6ld %6ld %6ld Removed",p1,p3,p2) ;
         }
       if( preError == 3 || preError == 4  )
         {
          if( bcdtmInsert_removeSliverTriangleDtmObject(dtmP,firstPnt,p1,p2)) return(1) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error Sliver %6ld %6ld %6ld Removed",firstPnt,p1,p2) ;
         }
       return(10) ;
      }
/*
**  Set Values For Return
*/
    *dtmPnt1P = p1 ;
    *dtmPnt2P = p2 ;
    *dtmPnt3P = p3 ;
    if( p3 == dtmP->nullPnt ) return(3) ;
    else                      return(2) ;
   }
/*
**  Insert Line Goes External
*/
 if( ! internalFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Line Goes External p1 = %6ld p2 = %6ld",p1,p2) ;
    if( p1 == lastPnt ) { *dtmPnt1P = p1 ; return(1) ; }
    if( p2 == lastPnt ) { *dtmPnt1P = p2 ; return(1) ; }
    if( nodeAddrP(dtmP,lastPnt)->hPtr != dtmP->nullPnt ) hullPnt = lastPnt ;
    else
      {
       bcdtmInsert_findClosestLineInterceptWithHullDtmObject(dtmP,firstPnt,lastPnt,&hullPnt1,&hullPnt2) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt1 = %9ld hullPnt2 = %9ld",hullPnt1,hullPnt2) ;
       if( hullPnt1 == p1 ) { *dtmPnt1P = hullPnt1 ; return(1) ; }
       if( hullPnt2 == p2 ) { *dtmPnt1P = hullPnt2 ; return(1) ; }
       hullPnt = hullPnt1 ;
      }
    fpPtr = nodeAddrP(dtmP,firstPnt)->hPtr ;
    nodeAddrP(dtmP,firstPnt)->hPtr = hullPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,firstPnt,&fpArea,&direction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt ** Area = %20.15lf direction = %1ld",fpArea,direction) ;
    nodeAddrP(dtmP,firstPnt)->hPtr = fpPtr ;
    fpPtr = nodeAddrP(dtmP,hullPnt)->hPtr ;
    nodeAddrP(dtmP,hullPnt)->hPtr = firstPnt ;
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,hullPnt,&hullArea,&direction) ;
    nodeAddrP(dtmP,hullPnt)->hPtr = fpPtr ;
    if( dbg ) bcdtmWrite_message(0,0,0,"hullPnt ** Area = %20.15lf direction = %1ld",hullArea,direction) ;
    if( fpArea <= hullArea ) *dtmPnt1P = p2 ;
    else                     *dtmPnt1P = p1 ;
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
BENTLEYDTM_Public int bcdtmInsert_rigidExternalStringIntoDtmObject
(
 BC_DTM_OBJ *dtmP,
 long  drapeOption,
 long  insertOption,
 DPoint3d   *userStringPtsP,
 long  numUserStringPts,
 long  *startPntP
)
/*
**
** This Function Inserts A Rigid String That Is Both Internal And External To The Tin Hull
**
** Tin Points That Are Within The Point To Point Tolerance Of The String Points Are Moved Onto The String Points
** Tin Points That Are Within The Point To Line  Tolerance Of The String Lines Are Moved Onto The String Lines
**
**
** drapeOption  = 1   Drape Intersect Vertices On Tin Surface
**              = 2   Break Intersect Vertices On Tin Surface
** insertOption = 1   Move Tin Lines That Are Not Linear Features
**              = 2   Intersect Tin Lines
**
** Author : Rob Cormack
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    P1,P2,P3,closeFlag,numStringPts,numInsertPts,pointsMoved ;
 long    fndType,numInternalPts,numExternalPts;
 DPoint3d     *p3dP,*p3dnP,*stringPtsP=NULL ;
 double  xMidPnt,yMidPnt,sppTol=0.0,splTol=0.0 ;
 double  ang,nextAng,priorAng ;
 DTM_INSERT_POINT  *insertPtsP=NULL,*intPntP ;
 long    useNewExternalTest=TRUE ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Rigid External String") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drapeOption      = %8ld",drapeOption) ;
    bcdtmWrite_message(0,0,0,"insertOption     = %8ld",insertOption) ;
    bcdtmWrite_message(0,0,0,"numUserStringPts = %8ld",numUserStringPts) ;
   }
/*
** Print Out String Points For Debugging Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of String Points = %6ld ",numUserStringPts) ;
    if( dbg == 2 )
      {
       for( p3dP = userStringPtsP ; p3dP < userStringPtsP + numUserStringPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.6lf %12.6lf %12.6lf",(long)(p3dP-userStringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
    if( userStringPtsP->x != (userStringPtsP+numUserStringPts-1)->x || userStringPtsP->y != (userStringPtsP+numUserStringPts-1)->y ) bcdtmWrite_message(0,0,0,"Open String") ;
    else                                                                                                                         bcdtmWrite_message(0,0,0,"Closed String") ;
   }
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
 sppTol = dtmP->ppTol ;
 splTol = dtmP->plTol ;

//  Log Tolerances

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"ppTol   = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"plTol   = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"mppTol  = %20.15lf",dtmP->mppTol) ;
   }
/*
** Set Tin Tolerances At Machine Tolerance
*/
 dtmP->ppTol = dtmP->mppTol * 10000.0 ;
 dtmP->plTol = dtmP->mppTol * 10000.0 ;
/*
** Test For String Closure
*/
 closeFlag = 0 ;
 if( userStringPtsP->x == (userStringPtsP+numUserStringPts-1)->x && userStringPtsP->y == (userStringPtsP+numUserStringPts-1)->y ) closeFlag = 1 ;
/*
** Copy String Points
*/
 numStringPts = numUserStringPts ;
 stringPtsP = (DPoint3d *) malloc( numStringPts * sizeof(DPoint3d) ) ;
 if( stringPtsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( p3dP = stringPtsP , p3dnP = userStringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++p3dnP )
   {
    *p3dP = *p3dnP ;
   }
/*
** Rubber Band Tin Points Onto Insert String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Tin Points Onto Insert String Points") ;
 if( bcdtmInsert_rubberBandTinPointsOnToInsertStringPointsDtmObject(dtmP,stringPtsP,numStringPts,&pointsMoved)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Points Moved = %6ld",pointsMoved) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External String Points = %6ld ",numStringPts) ;
/*
** Check Tin For Precision Errors
*/
 if( cdbg  )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,1))
      {
       bcdtmWrite_message(1,0,0,"Tin Precision Errors") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Rubber Band Hull Lines Onto Insert String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Lines Onto Insert String") ;
 if( bcdtmInsert_rubberBandHullLinesOnToInsertStringDtmObject(dtmP,stringPtsP,numStringPts,&pointsMoved) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Moved = %6ld",pointsMoved) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External String Points = %6ld ",numStringPts) ;
/*
** Rubber Band Hull Points Onto Insert String
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rubber Banding Hull Points Onto Insert String") ;
 if( bcdtmInsert_rubberBandHullPointsOnToInsertStringDtmObject(dtmP,&stringPtsP,&numStringPts,&pointsMoved) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Moved = %6ld",pointsMoved) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External String Points = %6ld ",numStringPts) ;
/*
** Print Out String Points For Debugging Purposes
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of External String Points = %6ld ",numStringPts) ;
    for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.6lf %12.6lf %12.6lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Check Tin For Precision Errors
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,1))
      {
       bcdtmWrite_message(1,0,0,"Tin Precision Errors") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Intersect String And With Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting String With Tin Hull") ;
 if( bcdtmInsert_intersectStringWithTinHullDtmObject(dtmP,&stringPtsP,&numStringPts,dtmP->ppTol)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External String Points = %6ld ",numStringPts) ;
/*
** Print Out String Points For Debugging Purposes
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"After Hull Intersection ** Number Of External String Point = %6ld ",numStringPts) ;
    for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %15.4lf %15.4lf %15.4lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Create Insert Points Array And Copy Insert String To This Array
*/
 numInsertPts = numStringPts ;
 insertPtsP = ( DTM_INSERT_POINT * ) malloc ( numInsertPts * sizeof(DTM_INSERT_POINT)) ;
 if( insertPtsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( intPntP = insertPtsP , p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++intPntP )
   {
    intPntP->x = p3dP->x ;
    intPntP->y = p3dP->y ;
    intPntP->z = p3dP->z ;
    intPntP->PointNumber = DTM_NULL_PNT ;
    intPntP->PointType   = DTM_NULL_PNT ;
    intPntP->LineType    = DTM_NULL_PNT ;
   }
/*
** Scan Insert Points And Determine Which Points Are Internal And External To Tin
*/
 numInternalPts = numExternalPts = 0 ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts ; ++intPntP )
   {
    bcdtmFind_triangleDtmObject(dtmP,intPntP->x,intPntP->y,&fndType,&P1,&P2,&P3) ;
    if( fndType ) intPntP->PointType = 1 ;
/*
** Check For Point Just OutSide Hull ** Math Precision Fudge
*/
    else
      {
       intPntP->PointType = 2 ;
       bcdtmInsert_findClosestHullLineDtmObject(dtmP,intPntP->x,intPntP->y,&P1,&P2) ;
       if( P1 != dtmP->nullPnt && P2 != dtmP->nullPnt )
         {
// RobC - 3-Sep-2012 - Fix For D108596
//          if( bcdtmMath_normalDistanceToLineDtmObject(dtmP,P1,P2,intPntP->x,intPntP->y) <= sppTol  ) intPntP->PointType = 1 ;
         }
      }
/*
**  Increment Point Counts
*/
    if( intPntP->PointType == 1 ) ++numInternalPts ;
    else                          ++numExternalPts ;
   }
/*
** Write Out Point Types
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numInternalPts = %6ld numExternalPts = %6ld",numInternalPts,numExternalPts) ;

 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts ; ++intPntP )
   {
    if( bcdtmFind_triangleDtmObject(dtmP,intPntP->x,intPntP->y,&fndType,&P1,&P2,&P3)) goto errexit ;
    if( ! fndType && intPntP->PointType == 1 )
      {
       bcdtmWrite_message(0,0,0,"Point External And Not Internal") ;
      }
    if( fndType  && intPntP->PointType == 2 )
      {
       bcdtmWrite_message(0,0,0,"Point Internal And Not External") ;
      }
   }



 if( dbg == 2 )
   {
    for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts ; ++intPntP )
      {
       bcdtmWrite_message(0,0,0,"Insert Point[%6ld] ** Type = %2ld  %12.4lf %12.4lf %10.4lf",(long)(intPntP-insertPtsP),intPntP->PointType,intPntP->x,intPntP->y,intPntP->z) ;
      }
   }
/*
** Check For Memory Reallocation
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
** Insert Internal Points In Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Points Into Tin") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts ; ++intPntP )
   {
    if(  intPntP->PointType == 1 )
      {
       if( intPntP == insertPtsP + numInsertPts - 1 && closeFlag ) intPntP->PointNumber = insertPtsP->PointNumber ;
       else if( bcdtmInsert_storeRigidPointInDtmObject(dtmP,drapeOption,insertOption,intPntP->x,intPntP->y,intPntP->z,&intPntP->PointNumber)) goto errexit ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Point[%6ld] As Internal Point %8ld ** %12.6lf %12.6lf %12.6lf",(long)(intPntP-insertPtsP),intPntP->PointNumber,intPntP->x,intPntP->y,intPntP->z) ;
      }
   }
/*
**  Check Tin
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Features Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1)) goto errexit ;
    bcdtmWrite_message(0,0,0,"DTM Feature Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,1)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,dbg))  goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Insert External Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting External Points Into Tin") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts ; ++intPntP )
   {
    if(  intPntP->PointType == 2 )
      {
       if( intPntP == insertPtsP + numInsertPts - 1 && closeFlag ) intPntP->PointNumber = insertPtsP->PointNumber ;
       else if( bcdtmInsert_addPointToDtmObject(dtmP,intPntP->x,intPntP->y,intPntP->z,&intPntP->PointNumber) ) goto errexit ;
       if( dbg == 2) bcdtmWrite_message(0,0,0,"Inserted External Point %8ld ** %12.6lf %12.6lf %12.6lf",intPntP->PointNumber,intPntP->x,intPntP->y,intPntP->z) ;
      }
   }
/*
** Determine Whether Lines Are Internal Or External
*/
 if( useNewExternalTest == TRUE ) goto newExternalTest ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Internal And External Lines") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts -  1 ; ++intPntP )
   {
    if( intPntP->PointNumber != (intPntP+1)->PointNumber )
      {
       intPntP->LineType = 1 ;
       if      (   intPntP->PointType == 2  ||  (intPntP+1)->PointType == 2 ) intPntP->LineType = 2 ;
/*
**     Check If Mid Point Of Hull Points Is Internal Or External To Tin Hull
*/
       else if (  nodeAddrP(dtmP,intPntP->PointNumber)->hPtr != dtmP->nullPnt && nodeAddrP(dtmP,(intPntP+1)->PointNumber)->hPtr != dtmP->nullPnt )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Testing For External Line %9ld %9ld",intPntP->PointNumber,(intPntP+1)->PointNumber) ;
          if ( nodeAddrP(dtmP,intPntP->PointNumber)->hPtr     == nodeAddrP(dtmP,(intPntP+1)->PointNumber)->hPtr ||
               nodeAddrP(dtmP,(intPntP+1)->PointNumber)->hPtr == nodeAddrP(dtmP,intPntP->PointNumber)->hPtr         )
            {
             xMidPnt = ( pointAddrP(dtmP,intPntP->PointNumber)->x + pointAddrP(dtmP,(intPntP+1)->PointNumber)->x ) / 2.0 ;
             yMidPnt = ( pointAddrP(dtmP,intPntP->PointNumber)->y + pointAddrP(dtmP,(intPntP+1)->PointNumber)->y ) / 2.0 ;
             if ( bcdtmFind_triangleDtmObject(dtmP,xMidPnt,yMidPnt,&fndType,&P1,&P2,&P3)) goto errexit ;
             if( fndType ) intPntP->LineType = 2 ;

/*
**           Check For Point Just OutSide Hull ** Math Precision Fudge
*/
             else
               {
                intPntP->LineType = 2 ;
                bcdtmInsert_findClosestHullLineDtmObject(dtmP,intPntP->x,intPntP->y,&P1,&P2) ;
                if( P1 != dtmP->nullPnt && P2 != dtmP->nullPnt )
                  {
                   if( bcdtmMath_normalDistanceToLineDtmObject(dtmP,P1,P2,intPntP->x,intPntP->y) <= splTol  ) intPntP->LineType = 1 ;
                  }
               }
            }
         }
      }
   }
/*
** Determine Whether Lines Are Internal Or External
*/
 newExternalTest :
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Internal And External Lines") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts -  1 ; ++intPntP )
   {
    if( intPntP->PointNumber != (intPntP+1)->PointNumber )
      {
       intPntP->LineType = 1 ;
       if      (   intPntP->PointType == 2  ||  (intPntP+1)->PointType == 2 ) intPntP->LineType = 2 ;
/*
**     Check If Line Goes External To Hull
*/
       else if (  nodeAddrP(dtmP,intPntP->PointNumber)->hPtr != dtmP->nullPnt && nodeAddrP(dtmP,(intPntP+1)->PointNumber)->hPtr != dtmP->nullPnt )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Testing For External Line %9ld %9ld",intPntP->PointNumber,(intPntP+1)->PointNumber) ;
          P1 = nodeAddrP(dtmP,intPntP->PointNumber)->hPtr ;
          if( ( P2 = bcdtmList_nextClkDtmObject(dtmP,intPntP->PointNumber,P1)) < 0 ) goto errexit ;
          ang  = bcdtmMath_getPointAngleDtmObject(dtmP,intPntP->PointNumber,(intPntP+1)->PointNumber) ;
          nextAng = bcdtmMath_getPointAngleDtmObject(dtmP,intPntP->PointNumber,P1) ;
          priorAng = bcdtmMath_getPointAngleDtmObject(dtmP,intPntP->PointNumber,P2) ;
          if( ang  < priorAng ) ang  += DTM_2PYE ;
          if( nextAng < priorAng ) nextAng += DTM_2PYE ;
          if( ang > priorAng && ang < nextAng ) intPntP->LineType = 2 ;
         }
      }
   }
/*
** Write Insert Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Insert Points = %6ld",numInsertPts) ;
    for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts  ; ++intPntP )
      {
       bcdtmWrite_message(0,0,0,"Point[%5ld] Pn = %6ld Pt = %2ld Lt = %2ld ** %12.6lf %12.6lf %10.4lf",(long)(intPntP-insertPtsP),intPntP->PointNumber,intPntP->PointType,intPntP->LineType,intPntP->x,intPntP->y,intPntP->z) ;
      }
   }
/*
** Check For Memory Reallocation
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
** Insert Internal Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Lines Into Tin") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts -  1 ; ++intPntP )
   {
    if( intPntP->PointNumber != (intPntP+1)->PointNumber && intPntP->LineType == 1 )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Inserting Internal Line %8ld %8ld",intPntP->PointNumber,(intPntP+1)->PointNumber) ;
       if( bcdtmInsert_rigidLineBetweenPointsDtmObject(dtmP,intPntP->PointNumber,(intPntP+1)->PointNumber,drapeOption,insertOption))
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Error Detected Inserting Internal Line %8ld %8ld",intPntP->PointNumber,(intPntP+1)->PointNumber) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Line %8ld Of %8ld",(long)(intPntP-insertPtsP),numInsertPts -  1) ;
          goto errexit ;
         }
      }
   }
/*
**  Check Tin
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Features Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1)) goto errexit ;
    bcdtmWrite_message(0,0,0,"DTM Feature Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,1)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0))  goto errexit ;
    bcdtmWrite_message(0,0,0,"Tin Precision OK") ;
   }
/*
** Insert External Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting External Lines Into Tin") ;
 for( intPntP = insertPtsP ; intPntP < insertPtsP + numInsertPts -  1 ; ++intPntP )
   {
    if( intPntP->PointNumber != (intPntP+1)->PointNumber && intPntP->LineType == 2 )
      {
       if( ! bcdtmList_testLineDtmObject(dtmP,intPntP->PointNumber,(intPntP+1)->PointNumber) )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Inserting External Line %8ld %8ld",intPntP->PointNumber,(intPntP+1)->PointNumber) ;
          if( bcdtmList_insertLineDtmObject(dtmP,intPntP->PointNumber,(intPntP+1)->PointNumber) ) goto errexit ;
         }
       nodeAddrP(dtmP,intPntP->PointNumber)->tPtr = (intPntP+1)->PointNumber ;
      }
   }
/*
** Set Start Point
*/
 *startPntP = insertPtsP->PointNumber ;
/*
** Write Inserted External String
*/
 if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,*startPntP) ;
/*
** Check Connectivity Of Tptr Array
*/
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*startPntP,1))
   {
    bcdtmWrite_message(1,0,0,"Connectivity Error In Inserted Rigid External String") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( insertPtsP != NULL ) free(insertPtsP) ;
 if( stringPtsP != NULL ) free(stringPtsP) ;
/*
** Reset Tolerances
*/
 dtmP->ppTol = sppTol ;
 dtmP->plTol = splTol ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Rigid External String Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Rigid External String Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_intersectStringWithTinHullDtmObject(BC_DTM_OBJ *dtmP,DPoint3d **stringPtsPP,long *numStringPtsP,double mppTol)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p1,p2,p3,p4,ofs,numIntTable,numIntPts,memIntPts,incIntPts,numIntersectPts ;
 DTM_STR_INT_TAB *intTabP,*intTableP=NULL ;
 DTM_STR_INT_PTS *intPntP,*intPnt1P,*intPtsP=NULL ;
 double d1,d2,d3,d4,xMin,yMin,xMax,yMax ;
 DPoint3d   *p3d1P,*p3d2P,*intersectPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Intersecting String With Tin Hull") ;
    bcdtmWrite_message(0,0,0,"Mathematical Tolerance = %20.15lf",mppTol) ;
   }
/*
** Build Intersection Table For String And Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table") ;
 if( bcdtmInsert_buildStringHullIntersectionTableDtmObject(dtmP,*stringPtsPP,*numStringPtsP,&intTableP,&numIntTable)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Intersection Table Entries = %6ld",numIntTable) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of String Hull Edge Intersection Table Entries = %6ld",numIntTable ) ;
    for( intTabP = intTableP ; intTabP < intTableP + numIntTable ; ++intTabP )
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** String = %6ld Segment = %6ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(intTabP-intTableP),intTabP->String,intTabP->Segment,intTabP->Type,intTabP->Direction,intTabP->X1,intTabP->Y1,intTabP->Z1,intTabP->X2,intTabP->Y2,intTabP->Z2) ;
      }
   }
/*
** Scan Intersection Table And For Intersections
*/
 incIntPts = numIntTable / 10 ;
 if( incIntPts == 0 ) incIntPts = 100 ;
 numIntPts = memIntPts = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For String Hull Intersections") ;
 if( bcdtmInsert_scanForStringHullIntersectionsDtmObject(intTableP,numIntTable,&intPtsP,&numIntPts,&memIntPts,incIntPts,mppTol) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of String Hull Intersections = %4ld",numIntPts) ;
/*
** Check For Incorrectly Calculated Intersection Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking For Incorrectly Calculated Intersection Points") ;
/*
**  Check Hull Points
*/
    for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       p1 = intPntP->String2  ;
       p2 = intPntP->Segment2 ;
       if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p2)->x ) { xMin = pointAddrP(dtmP,p1)->x ; xMax = pointAddrP(dtmP,p2)->x ; }
       else                                                   { xMin = pointAddrP(dtmP,p2)->x ; xMax = pointAddrP(dtmP,p1)->x ; }
       if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p2)->y ) { yMin = pointAddrP(dtmP,p1)->y ; yMax = pointAddrP(dtmP,p2)->y ; }
       else                                                   { yMin = pointAddrP(dtmP,p2)->y ; yMax = pointAddrP(dtmP,p1)->y ; }
       if( intPntP->x < xMin || intPntP->x > xMax || intPntP->y < yMin || intPntP->y > yMax )
         {
          bcdtmWrite_message(0,0,0,"Hull Line Intersection Error") ;
          bcdtmWrite_message(0,0,0,"**** Hull Point 1 = %6ld ** %12.6lf %12.6lf %12.6lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          bcdtmWrite_message(0,0,0,"**** Hull Point 2 = %6ld ** %12.6lf %12.6lf %12.6lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
          bcdtmWrite_message(0,0,0,"**** Intersection Point[%6ld] = %12.6lf %12.6lf %12.6lf",(long)(intPntP-intPtsP),intPntP->x,intPntP->y,intPntP->z) ;
          ret = 1 ;
         }
      }
/*
** Check String Points
*/
    for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       p3d1P = *stringPtsPP + intPntP->Segment1 ;
       p3d2P = p3d1P + 1 ;
       if( p3d1P->x <= p3d2P->x ) { xMin = p3d1P->x ; xMax = p3d2P->x ; }
       else                       { xMin = p3d2P->x ; xMax = p3d1P->x ; }
       if( p3d1P->y <= p3d2P->y ) { yMin = p3d1P->y ; yMax = p3d2P->y ; }
       else                       { yMin = p3d2P->y ; yMax = p3d1P->y ; }
       if( intPntP->x < xMin || intPntP->x > xMax || intPntP->y < yMin || intPntP->y > yMax )
         {
          bcdtmWrite_message(0,0,0,"String Line Intersection Error") ;
          bcdtmWrite_message(0,0,0,"**** String Point 1 = %6ld ** %12.6lf %12.6lf %12.6lf",(long)(p3d1P-*stringPtsPP),p3d1P->x,p3d1P->y,p3d1P->z) ;
          bcdtmWrite_message(0,0,0,"**** String Point 2 = %6ld ** %12.6lf %12.6lf %12.6lf",(long)(p3d2P-*stringPtsPP),p3d2P->x,p3d2P->y,p3d2P->z) ;
          bcdtmWrite_message(0,0,0,"**** Intersection Point[%6ld] = %12.6lf %12.6lf %12.6lf",(long)(intPntP-intPtsP),intPntP->x,intPntP->y,intPntP->z) ;
          ret = 1 ;
         }
      }
    if( ret == 1 ) goto errexit ;
   }
/*
** Sort Intersection Points
*/
 if( numIntPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting String Hull Intersection Points") ;
    qsortCPP(intPtsP,numIntPts,sizeof(DTM_STR_INT_PTS),bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
/*
** Write Intersection Points
*/
    if( dbg == 2  )
      {
       bcdtmWrite_message(0,0,0,"Number Of String Hull Intersections = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %20.15lf x = %12.6lf y = %12.6lf z = %12.6lf Z2 = %12.6lf",(long)(intPntP-intPtsP),intPntP->String1,intPntP->Segment1,intPntP->String2,intPntP->Segment2,intPntP->Distance,intPntP->x,intPntP->y,intPntP->z,intPntP->Z2) ;
         }
      }
/*
**
**  Mark Intersection Points That Are Coincident With String Point Ends
**  Do This Because Of Math Precision Problems When Calculating Intersection Points
**
*/
    for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       p3d1P = *stringPtsPP + intPntP->Segment1 ;
       p3d2P = p3d1P + 1 ;
       d1 = bcdtmMath_distance(p3d1P->x,p3d1P->y,intPntP->x,intPntP->y) ;
       d2 = bcdtmMath_distance(p3d2P->x,p3d2P->y,intPntP->x,intPntP->y) ;
       if( dbg == 2  )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** d1 = %20.15lf  d2 = %20.15lf",(long)(intPntP-intPtsP),d1,d2) ;
          if( d1 == 0.0 ) bcdtmWrite_message(0,0,0,"d1 == 0.0") ;
          if( d2 == 0.0 ) bcdtmWrite_message(0,0,0,"d2 == 0.0") ;
          d3 = bcdtmMath_distance(pointAddrP(dtmP,intPntP->String2)->x,pointAddrP(dtmP,intPntP->String2)->y,intPntP->x,intPntP->y) ;
          d4 = bcdtmMath_distance(pointAddrP(dtmP,intPntP->Segment2)->x,pointAddrP(dtmP,intPntP->Segment2)->y,intPntP->x,intPntP->y) ;
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** d3 = %20.15lf  d4 = %20.15lf",(long)(intPntP-intPtsP),d3,d4) ;
          if( d3 == 0.0 ) bcdtmWrite_message(0,0,0,"d3 == 0.0") ;
          if( d4 == 0.0 ) bcdtmWrite_message(0,0,0,"d4 == 0.0") ;
          if( p3d1P->x == pointAddrP(dtmP,intPntP->String2)->x  && p3d1P->y == pointAddrP(dtmP,intPntP->String2)->y  ) bcdtmWrite_message(0,0,0,"Str1 == Hull1") ;
          if( p3d1P->x == pointAddrP(dtmP,intPntP->Segment2)->x && p3d1P->y == pointAddrP(dtmP,intPntP->Segment2)->y ) bcdtmWrite_message(0,0,0,"Str1 == Hull2") ;
          if( p3d2P->x == pointAddrP(dtmP,intPntP->String2)->x  && p3d2P->y == pointAddrP(dtmP,intPntP->String2)->y  ) bcdtmWrite_message(0,0,0,"Str2 == Hull1") ;
          if( p3d2P->x == pointAddrP(dtmP,intPntP->Segment2)->x && p3d2P->y == pointAddrP(dtmP,intPntP->Segment2)->y ) bcdtmWrite_message(0,0,0,"Str2 == Hull2") ;
         }
       if( d1 <= mppTol || d2 <= mppTol )  intPntP->String1 = DTM_NULL_PNT ;
      }
/*
**  Remove Marked Intersection Points
*/
    for( intPntP = intPnt1P = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       if( intPntP->String1 != DTM_NULL_PNT )
         {
          if( intPnt1P != intPntP ) *intPnt1P = *intPntP ;
          ++intPnt1P ;
         }
      }
    numIntPts = (long)(intPnt1P-intPtsP) ;
/*
** Write Intersection Points
*/
    if( dbg == 2  )
      {
       bcdtmWrite_message(0,0,0,"Number Of String Hull Intersections = %6ld",numIntPts) ;
       for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
         {
          bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(intPntP-intPtsP),intPntP->String1,intPntP->Segment1,intPntP->String2,intPntP->Segment2,intPntP->Distance,intPntP->x,intPntP->y,intPntP->z,intPntP->Z2) ;
         }
      }
/*
**  Add Intersection Points To Tin
*/
    for( intPntP = intPtsP ; intPntP < intPtsP + numIntPts ; ++intPntP )
      {
       p1 = intPntP->String2  ;
       p2 = intPntP->Segment2 ;
       d1 = bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,intPntP->x,intPntP->y) ;
       d2 = bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,intPntP->x,intPntP->y) ;
       if( dbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Hull Point 1 = %6ld ** %12.6lf %12.6lf %12.6lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          bcdtmWrite_message(0,0,0,"Hull Point 2 = %6ld ** %12.6lf %12.6lf %12.6lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
          bcdtmWrite_message(0,0,0,"Str  Point 1 = %6ld ** %12.6lf %12.6lf %12.6lf",intPntP->Segment1,(*stringPtsPP+intPntP->Segment1)->x,(*stringPtsPP+intPntP->Segment1)->y,(*stringPtsPP+intPntP->Segment1)->z) ;
          bcdtmWrite_message(0,0,0,"Str  Point 2 = %6ld ** %12.6lf %12.6lf %12.6lf",intPntP->Segment1+1,(*stringPtsPP+intPntP->Segment1+1)->x,(*stringPtsPP+intPntP->Segment1+1)->y,(*stringPtsPP+intPntP->Segment1+1)->z) ;
          bcdtmWrite_message(0,0,0,"Distance P1 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,intPntP->x,intPntP->y)) ;
          bcdtmWrite_message(0,0,0,"Distance P2 = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,intPntP->x,intPntP->y)) ;
         }
/*
**     Dont Insert Coincident Intersection And Hull Points
*/
       if( d1 >= mppTol && d2 >= mppTol )
         {
/*
**        Check And Adjust For Previous Intersection Points On Hull Line P1P2
*/
          while ( nodeAddrP(dtmP,p1)->hPtr != p2 )
            {
             p3 = nodeAddrP(dtmP,p1)->hPtr ;
             if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p3)->x ) { d1 = pointAddrP(dtmP,p1)->x ; d2 = pointAddrP(dtmP,p3)->x ; }
             else                                                   { d1 = pointAddrP(dtmP,p3)->x ; d2 = pointAddrP(dtmP,p1)->x ; }
             if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p3)->y ) { d3 = pointAddrP(dtmP,p1)->y ; d4 = pointAddrP(dtmP,p3)->y ; }
             else                                                   { d3 = pointAddrP(dtmP,p3)->y ; d4 = pointAddrP(dtmP,p1)->y ; }
             if( intPntP->x >= d1 && intPntP->x <= d2 && intPntP->y >= d3 && intPntP->y <= d4 ) p2 = p3 ;
             else                                                                   p1 = p3 ;
            }
/*
**        Insert Intersection Point Into Hull Line
*/
          if(( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
          if( bcdtmInsert_addPointToDtmObject(dtmP,intPntP->x,intPntP->y,intPntP->z,&p4) ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Point In Hull ** Np = %8ld  %12.6lf %12.6lf %12.6lf",p4,intPntP->x,intPntP->y,intPntP->z ) ;
          if( bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p1,p2,p4)) goto errexit ;
          if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2) ) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,p4,p3)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p1,dtmP->nullPnt)) goto errexit ;
          if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p2,p4,p3)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p2,p1) ) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,p4,p2) ) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1)) goto errexit ;
          nodeAddrP(dtmP,p1)->hPtr = p4 ;
          nodeAddrP(dtmP,p4)->hPtr = p2 ;
         }
      }
/*
**  Add Intersection Points To String
*/
    numIntersectPts = *numStringPtsP + numIntPts ;
    intersectPtsP = (DPoint3d * ) malloc( numIntersectPts * sizeof(DPoint3d)) ;
    if( intersectPtsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
    for( p3d1P = *stringPtsPP, ofs = 0, p3d2P = intersectPtsP, intPntP = intPtsP ; p3d1P < *stringPtsPP + *numStringPtsP ; ++p3d1P , ++ofs )
      {
       *p3d2P = *p3d1P ;
       ++p3d2P ;
       if( ofs == intPntP->Segment1 )
         {
          while ( intPntP < intPtsP + numIntPts && ofs == intPntP->Segment1 )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Intersection Point In String Segment %6ld ** %12.6lf %12.6lf %12.6lf",intPntP->Segment1,intPntP->x,intPntP->y,intPntP->z ) ;
             p3d2P->x = intPntP->x ;
             p3d2P->y = intPntP->y ;
             p3d2P->z = intPntP->z ;
             ++p3d2P ;
             ++intPntP ;
            }
         }
      }
/*
**  Set String Pointers
*/
    free(*stringPtsPP) ;
    *stringPtsPP = intersectPtsP ;
    *numStringPtsP = numIntersectPts ;
    intersectPtsP = NULL ;
/*
**   Write String Points
*/
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of String Points = %6ld",*numStringPtsP) ;
       for( p3d1P = *stringPtsPP ; p3d1P < *stringPtsPP + *numStringPtsP ; ++p3d1P )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d1P-*stringPtsPP),p3d1P->x,p3d1P->y,p3d1P->z ) ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( intTableP != NULL ) free(intTableP) ;
 if( intPtsP   != NULL ) free(intPtsP)   ;
 if( intersectPtsP != NULL ) free(intersectPtsP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Intersecting String With Tin Hull Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Intersecting String With Tin Hull Error") ;
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
BENTLEYDTM_Private int bcdtmInsert_buildStringHullIntersectionTableDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long numStringPts,DTM_STR_INT_TAB **intTablePP,long *numIntTableP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,memIntTable,incIntTable  ;
 double cord ;
 DPoint3d    *p3dP ;
 DTM_STR_INT_TAB *intTabP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table") ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 incIntTable = numStringPts * 2 ;
/*
** Store Hull Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing String Lines In Intersection Table") ;
 sp = dtmP->hullPoint ;
 do
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                      *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store Hull Line
*/
    np = nodeAddrP(dtmP,sp)->hPtr ;
    (*intTablePP+*numIntTableP)->String  = sp  ;
    (*intTablePP+*numIntTableP)->Segment = np  ;
    (*intTablePP+*numIntTableP)->Type    = 1   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = pointAddrP(dtmP,sp)->x ;
    (*intTablePP+*numIntTableP)->Y1 = pointAddrP(dtmP,sp)->y ;
    (*intTablePP+*numIntTableP)->Z1 = pointAddrP(dtmP,sp)->z ;
    (*intTablePP+*numIntTableP)->X2 = pointAddrP(dtmP,np)->x ;
    (*intTablePP+*numIntTableP)->Y2 = pointAddrP(dtmP,np)->y ;
    (*intTablePP+*numIntTableP)->Z2 = pointAddrP(dtmP,np)->z ;
    ++*numIntTableP ;
/*
** Reset
*/
    sp = np ;
   } while( sp != dtmP->hullPoint ) ;
/*
** Store String Lines In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing String Lines In Intersection Table") ;
 for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts - 1 ; ++p3dP )
   {
/*
**  Check For Memory Allocation
*/
    if( *numIntTableP == memIntTable )
      {
       memIntTable = memIntTable + incIntTable ;
       if( *intTablePP == NULL ) *intTablePP = ( DTM_STR_INT_TAB * ) malloc ( memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                      *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *intTablePP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store String Line
*/
    (*intTablePP+*numIntTableP)->String  = (long)(p3dP-stringPtsP) ;
    (*intTablePP+*numIntTableP)->Segment = (long)(p3dP+1-stringPtsP)  ;
    (*intTablePP+*numIntTableP)->Type    = 2   ;
    (*intTablePP+*numIntTableP)->Direction = 1 ;
    (*intTablePP+*numIntTableP)->X1 = p3dP->x ;
    (*intTablePP+*numIntTableP)->Y1 = p3dP->y ;
    (*intTablePP+*numIntTableP)->Z1 = p3dP->z ;
    (*intTablePP+*numIntTableP)->X2 = (p3dP+1)->x ;
    (*intTablePP+*numIntTableP)->Y2 = (p3dP+1)->y ;
    (*intTablePP+*numIntTableP)->Z2 = (p3dP+1)->z ;
    ++*numIntTableP ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STR_INT_TAB * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( intTabP = *intTablePP ; intTabP < *intTablePP + *numIntTableP ; ++intTabP )
   {
    if( intTabP->X1 > intTabP->X2 || ( intTabP->X1 == intTabP->X2 && intTabP->Y1 > intTabP->Y2 ) )
      {
       intTabP->Direction = 2 ;
       cord = intTabP->X1 ; intTabP->X1 = intTabP->X2 ; intTabP->X2 = cord ;
       cord = intTabP->Y1 ; intTabP->Y1 = intTabP->Y2 ; intTabP->Y2 = cord ;
       cord = intTabP->Z1 ; intTabP->Z1 = intTabP->Z2 ; intTabP->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STR_INT_TAB),bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building String Hull Intersection Table Error") ;
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInsert_scanForStringHullIntersectionsDtmObject(DTM_STR_INT_TAB *intTableP,long numIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts,double mppTol)
/*
** This Function Scans for StringHull Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 long    numActiveIntTable=0,memActiveIntTable=0 ;
 DTM_STR_INT_TAB *pint,*activeIntTableP=NULL ;
/*
** Scan Sorted Intersection Table and Look For Intersections
*/
 for( pint = intTableP ; pint < intTableP + numIntTable  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(activeIntTableP,&numActiveIntTable,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&activeIntTableP,&numActiveIntTable,&memActiveIntTable,pint))  goto errexit ;
    if( bcdtmInsert_determineStringHullIntersectionsDtmObject(activeIntTableP,numActiveIntTable,intPtsPP,numIntPtsP,memIntPtsP,incIntPts,mppTol)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( activeIntTableP != NULL ) free(activeIntTableP) ;
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
BENTLEYDTM_Private int bcdtmInsert_determineStringHullIntersectionsDtmObject(DTM_STR_INT_TAB *activeIntTableP,long activeIntTablePNe,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long incIntPts,double mppTol)
/*
** Determine Line Intersections
*/
{
 int              ret=DTM_SUCCESS ;
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y ;
 double           axMin,ayMin,axMax,ayMax,sxMin,sxMax,syMin,syMax ;
 DTM_STR_INT_TAB  *activeP,*scanP,*saveP,*hullP ;
/*
** Initialise
*/
 activeP = activeIntTableP + activeIntTablePNe - 1 ;
/*
** Set Active Line Limits
*/
 if( activeP->X1 <= activeP->X2 ) { axMin = activeP->X1 ; axMax = activeP->X2 ; }
 else                             { axMin = activeP->X2 ; axMax = activeP->X1 ; }
 if( activeP->Y1 <= activeP->Y2 ) { ayMin = activeP->Y1 ; ayMax = activeP->Y2 ; }
 else                             { ayMin = activeP->Y2 ; ayMax = activeP->Y1 ; }
/*
** Scan Active Line List
*/
 for( scanP = activeIntTableP ; scanP < activeIntTableP + activeIntTablePNe - 1 ; ++scanP )
   {
/*
**  Only Compare String Lines To Hull Lines
*/
    if( ( activeP->Type == 1 && scanP->Type == 2 ) || ( activeP->Type == 2 && scanP->Type == 1 ) )
      {
/*
**      Only Compare For Non Coincident End Points
*/
        if( ( activeP->X1 != scanP->X1 ||  activeP->Y1 != scanP->Y1 ) && ( activeP->X1 != scanP->X2 || activeP->Y1 != scanP->Y2 ) &&
            ( activeP->X2 != scanP->X1 ||  activeP->Y2 != scanP->Y1 ) && ( activeP->X2 != scanP->X2 || activeP->Y2 != scanP->Y2 )     )
          {
/*
**        Check Lines Intersect
*/
          if( bcdtmMath_checkIfLinesIntersect(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2))
            {
/*
**           Check For Colinear Lines
*/
             if( ! bcdtmInsert_checkForColinearLines(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2,mppTol) )
               {
/*
**              Set Scan Line Limits
*/
                if( scanP->X1 <= scanP->X2 ) { sxMin = scanP->X1 ; sxMax = scanP->X2 ; }
                else                         { sxMin = scanP->X2 ; sxMax = scanP->X1 ; }
                if( scanP->Y1 <= scanP->Y2 ) { syMin = scanP->Y1 ; syMax = scanP->Y2 ; }
                else                         { syMin = scanP->Y2 ; syMax = scanP->Y1 ; }
/*
**              Intersect Lines
*/
                bcdtmMath_normalIntersectCordLines(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2,&x,&y) ;
/*
**              Check Intersection Point Is On Active Line ** Fudge For Math Precision
*/
                if( x < axMin || x > axMax || y < ayMin || y > ayMax )
                  {
                   di = bcdtmMath_distance(activeP->X1,activeP->Y1,x,y) ;
                   dl = bcdtmMath_distance(activeP->X2,activeP->Y2,x,y) ;
                   if( di <= dl ) { x = activeP->X1 ; y = activeP->Y1 ; }
                   else           { x = activeP->X2 ; y = activeP->Y2 ; }
                  }

/*
**              Check Intersection Point Is On Scan Line ** Fudge For Math Precision
*/
                if( x < sxMin || x > sxMax || y < syMin || y > syMax )
                  {
                   di = bcdtmMath_distance(scanP->X1,scanP->Y1,x,y) ;
                   dl = bcdtmMath_distance(scanP->X2,scanP->Y2,x,y) ;
                   if( di <= dl ) { x = scanP->X1 ; y = scanP->Y1 ; }
                   else           { x = scanP->X2 ; y = scanP->Y2 ; }
                  }
/*
**              Check Memory
*/
                if( *numIntPtsP + 1 >= *memIntPtsP )
                  {
                   *memIntPtsP = *memIntPtsP + incIntPts ;
                   if( *intPtsPP == NULL ) *intPtsPP = ( DTM_STR_INT_PTS * ) malloc ( *memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
                   else                  *intPtsPP = ( DTM_STR_INT_PTS * ) realloc( *intPtsPP,*memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
                   if( *intPtsPP == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
                  }
/*
**              Assign Pointers
*/
                if( scanP->Type == 2 ) { saveP = scanP   ; hullP = activeP ; }
                else                   { saveP = activeP ; hullP = scanP   ; }
/*
**              Calculate Distances For Sptr
*/
                if( saveP->Direction == 1 ) { Xs = saveP->X1 ; Ys = saveP->Y1 ; Zs = saveP->Z1 ; Xe = saveP->X2 ; Ye = saveP->Y2 ; Ze = saveP->Z2 ; }
                if( saveP->Direction == 2 ) { Xs = saveP->X2 ; Ys = saveP->Y2 ; Zs = saveP->Z2 ; Xe = saveP->X1 ; Ye = saveP->Y1 ; Ze = saveP->Z1 ; }
                dz = Ze - Zs ;
                di = bcdtmMath_distance(Xs,Ys,x,y) ;
                dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
**              Store Intersection Point Alp
*/
                (*intPtsPP+*numIntPtsP)->String1  = 0  ;
                (*intPtsPP+*numIntPtsP)->Segment1 = saveP->String ;
                (*intPtsPP+*numIntPtsP)->String2  = hullP->String  ;
                (*intPtsPP+*numIntPtsP)->Segment2 = hullP->Segment ;
                (*intPtsPP+*numIntPtsP)->Distance = di ;
                (*intPtsPP+*numIntPtsP)->x  = x ;
                (*intPtsPP+*numIntPtsP)->y  = y ;
                (*intPtsPP+*numIntPtsP)->z  = Zs + dz * di / dl ;
                (*intPtsPP+*numIntPtsP)->Z2 = 0.0 ;
                ++*numIntPtsP ;
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
BENTLEYDTM_Private int bcdtmInsert_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *hullPnt1P,long *hullPnt2P)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 long   p1,p2,isw,onLine ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn  ;
/*
** Initialiase
*/
 *hullPnt1P = *hullPnt2P = dtmP->nullPnt ;
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
       d4 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y,&Xn,&Yn) ;
       if( isw )
         {
          *hullPnt1P = p1 ;
          *hullPnt2P = p2 ;
          dn = d1 ;
          if( d2 < dn ) dn = d2 ;
          if( d3 < dn ) dn = d3 ;
          if( onLine && d4 < dn ) dn = d4 ;
          isw = 0 ;
         }
       else
         {
          if( d1 < dn || d2 < dn || d3 < dn || ( onLine && d4 < dn ) )
            {
             *hullPnt1P = p1 ;
             *hullPnt2P = p2 ;
             if( d1 < dn ) dn = d1 ;
             if( d2 < dn ) dn = d2 ;
             if( d3 < dn ) dn = d3 ;
             if( onLine && d4 < dn ) dn = d4 ;
            }
         }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
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
BENTLEYDTM_Public int bcdtmInsert_retriangualteAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long firstPoint)
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
    sp = firstPoint ;
    np = nodeAddrP(dtmP,sp)->tPtr ;
/*
**  Scan Along Tptr List And Check MAX_MIN Criteria
*/
    while ( np != dtmP->nullPnt && np != firstPoint )
      {
/*
**     Check Line Anti Clockwise
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
/*
**     Check Line Clockwise
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
BENTLEYDTM_EXPORT int bcdtmInsert_internalDtmFeatureMrDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Dtm Object
 DTMFeatureType dtmFeatureType,              // ==> Dtm Feature Type
 long drapeOption,                 // ==> < 1 Drape,2 Break >
 long insertOption,                // ==> < 1 Move Tin Lines That Are Not Linear Features,2 Intersect Tin Lines >
 DTMUserTag userTag,             // ==> User Tag
 DTMFeatureId **featureIdPP,     // <== Assigned Feature Id's
 long *numFeatureIdsP,             // <== Number Of Assigned Feature Ids
 DPoint3d *featurePtsP,                 // ==> Feature Points
 long numFeaturePts                // ==> Number Of Feature Points
)
/*
** This Function Inserts A DTM Feature Into A Triangulated DTM
** The Feature Must Be Totally Internal To The Tin Hull
** This Is A Special Function With No Validation For The Mr DTM and should not
** Be Used For Civil or bcLIB DTM.
**
** If The Feature Is External to the Tin Hull The returned FeatureID
** will be set to value of the null feature Id ( dtmP->nullFeatureId )
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   insert,startPnt,numPolygons = 0,isInternal=FALSE,checkForInternal=TRUE;
 DTMDirection direction;
 DPoint3d    *p3dP,*regionPtsP=NULL ;
 double area ;
 DTM_POINT_ARRAY *pArrayP=NULL,**pArrayPP,**polygonsPP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Internal DTM Feature") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType  = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"userTag         = %10I64d",userTag) ;
    bcdtmWrite_message(0,0,0,"*featureIdPP    = %p",*featureIdPP) ;
    bcdtmWrite_message(0,0,0,"*numFeatureIdsP = %8ld",*numFeatureIdsP) ;
    bcdtmWrite_message(0,0,0,"featurePtsP     = %p",featurePtsP) ;
    bcdtmWrite_message(0,0,0,"numFeaturePts   = %8ld",numFeaturePts) ;
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Initialise
*/
 *numFeatureIdsP = 0 ;
 if( *featureIdPP != NULL ) { free(*featureIdPP) ; *featureIdPP = NULL ; }
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For Valid DTM Store Feature Type
*/
 if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(2,0,0,"Invalid Import DTM Feature Type") ;
    goto errexit ;
   }
/*
**  Check Code Implemented For Dtm Feature Type
*/
 switch( dtmFeatureType )
   {
    case  DTMFeatureType::RandomSpots     :
    case  DTMFeatureType::GroupSpots      :
    case  DTMFeatureType::Breakline      :
    case  DTMFeatureType::GraphicBreak   :
    case  DTMFeatureType::SoftBreakline      :
    case  DTMFeatureType::ContourLine    :
    case  DTMFeatureType::Void            :
    case  DTMFeatureType::VoidLine       :
    case  DTMFeatureType::Island          :
    case  DTMFeatureType::Hole            :
    case  DTMFeatureType::HoleLine       :
    case  DTMFeatureType::BreakVoid      :
    case  DTMFeatureType::DrapeVoid      :
    case  DTMFeatureType::Hull            :
    case  DTMFeatureType::DrapeHull      :
    case  DTMFeatureType::HullLine       :
    case  DTMFeatureType::SlopeToe       :
    case  DTMFeatureType::Polygon         :
       bcdtmWrite_message(1,0,0,"Code Not Implemented For Dtm Feature Type") ;
       goto errexit ;
    break ;

    default :
    break ;
  }
/*
** Check If Feature Is Internal To Tin Hull
*/
 if( checkForInternal )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Region Is Internal To Tin Hull") ;
    if( bcdtmInsert_checkFeatureIsInternalToTinHullMrDtmObject(dtmP,featurePtsP,numFeaturePts,&isInternal)) goto errexit ;
    if( dbg )
      {
       if(  isInternal ) bcdtmWrite_message(0,0,0,"Region Is Internal To Tin Hull") ;
       if( !isInternal ) bcdtmWrite_message(0,0,0,"Region Is Not Internal To Tin Hull") ;
      }
   }
/*
** Feature Internal To Tin Hull
*/
 if( isInternal )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Region Is Internal To Tin Hull") ;
    polygonsPP = ( DTM_POINT_ARRAY **) malloc(sizeof(DTM_POINT_ARRAY *)) ;
    if( polygonsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    *polygonsPP = NULL ;
    numPolygons = 1 ;

    pArrayP = ( DTM_POINT_ARRAY *) malloc(sizeof(DTM_POINT_ARRAY)) ;
    if( pArrayP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    pArrayP->pointsP = NULL ;
    pArrayP->numPoints = 0 ;
    *polygonsPP = pArrayP ;
    pArrayP = NULL ;

    regionPtsP = ( DPoint3d *) malloc( numFeaturePts * sizeof(DPoint3d)) ;
    if( regionPtsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    memcpy(regionPtsP,featurePtsP,numFeaturePts*sizeof(DPoint3d)) ;
    (*polygonsPP)->pointsP   = regionPtsP ;
    (*polygonsPP)->numPoints = numFeaturePts ;
    regionPtsP = NULL ;
   }
/*
** Clip Feature To Tin Hull
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Region To Tin Hull") ;
    if( bcdtmClip_polygonToTinHullDtmObject(dtmP,featurePtsP,numFeaturePts,&polygonsPP,&numPolygons)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"numPolygons = %8ld",numPolygons) ;
       for( pArrayPP = polygonsPP ; pArrayPP < polygonsPP + numPolygons ; ++pArrayPP )
         {
          bcdtmWrite_message(0,0,0,"Polygon[%4ld] ** pointsP = %p numPoints = %8ld",(long)(pArrayPP-polygonsPP),(*pArrayPP)->pointsP,(*pArrayPP)->numPoints) ;
         }
      }
   }
/*
** Insert Feature
*/
 if( numPolygons > 0 )
   {
/*
**  Allocate Memory For Feature Id
*/
    *featureIdPP = ( DTMFeatureId * ) malloc( numPolygons * sizeof(DTMFeatureId)) ;
    if( *featureIdPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Scan Polygons And Insert Into Tin
*/
    for( pArrayPP = polygonsPP ; pArrayPP < polygonsPP + numPolygons  ; ++pArrayPP )
      {
/*
**     Insert Feature Into Tin As A Tptr Polygon
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Dtm Feature") ;
       insert = bcdtmInsert_internalStringIntoDtmObject(dtmP,drapeOption,insertOption,(*pArrayPP)->pointsP,(*pArrayPP)->numPoints,&startPnt)  ;
       if( insert == 1 ) goto errexit ;
       if( insert == 2 )
         {
          bcdtmWrite_message(1,0,0,"Error Inserting Dtm Feature") ;
          goto errexit ;
         }
/*
**     Check Connectivity Of Inserted Dtm Feature
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking DTM Feature Connectivity") ;
       if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,startPnt,0)) goto errexit ;
/*
**     Set Direction Of Polygonal DTM Feature Anti Clockwise
*/
       if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
       if (direction == DTMDirection::Clockwise) if (bcdtmList_reverseTptrPolygonDtmObject (dtmP, startPnt)) goto errexit;
/*
**     Add Feature To Triangulated DTM
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Adding DTM Feature To Tin") ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,userTag,dtmP->dtmFeatureIndex,startPnt,1)) goto errexit ;
       *(*featureIdPP+*numFeatureIdsP) = dtmP->dtmFeatureIndex ;
       ++*numFeatureIdsP ;
       ++dtmP->dtmFeatureIndex ;
      }
/*
** Clean Dtm Object
*/
//    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;  // Not Required For MrDTM
   }
/*
** Clean Up
*/
 cleanup :
 if( pArrayP    != NULL ) free(pArrayP) ;
 if( regionPtsP != NULL ) free(regionPtsP) ;
 if( polygonsPP != NULL ) bcdtmMem_freePointerArrayToPointArrayMemory(&polygonsPP,numPolygons) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal DTM Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Internal DTM Feature Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkFeatureIsInternalToTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Dtm Object
 DPoint3d  *featurePtsP,                // ==> Feature Points
 long numFeaturePts,               // ==> Number Of Feature Points
 long *isInternalP                 // <== Set To 1 if feature is internal or 0 if feature is not internal
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numDrapePts ;
 DPoint3d *p3dP ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;

 double dd = 0.0,xi,yi,z ;
 long p1,p2,fndType,drapeFlag,onLine ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featurePtsP     = %p",featurePtsP) ;
    bcdtmWrite_message(0,0,0,"numFeaturePts   = %8ld",numFeaturePts) ;
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Check If Points Are Internal To DTM
*/
 if( dbg == 2 )
   {
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       if( bcdtmDrape_pointDtmObject(dtmP,p3dP->x,p3dP->y,&z,&drapeFlag)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** %12.5lf %12.5lf %10.4lf ** drapeFlag = %2ld",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z,drapeFlag) ;
       if( ! drapeFlag )
         {
          if( bcdtmFind_findClosestHullLineDtmObject(dtmP,p3dP->x,p3dP->y,&z,&fndType,&p1,&p2)) goto errexit ;
          bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** p1 = %8ld p2 = %8ld",fndType,p1,p2) ;
          if( fndType )
            {
             if( fndType == 1 )     // Hull Point
               {
                pnt1P = pointAddrP(dtmP,p1) ;
                dd = bcdtmMath_distance(p3dP->x,p3dP->y,pnt1P->x,pnt1P->y) ;
               }
             if( fndType == 2 )     // Hull Line
               {
                fndType = 0 ;
                pnt1P = pointAddrP(dtmP,p1) ;
                pnt2P = pointAddrP(dtmP,p2) ;
                dd = bcdtmMath_distanceOfPointFromLine(&onLine,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,p3dP->x,p3dP->y,&xi,&yi) ;
               }
             bcdtmWrite_message(0,0,0,"Distance To Tin Hull = %20.15lf ** dtmP->ppTol = %20.15lf",dd,dtmP->ppTol) ;
            }
         }
      }
   }
/*
** Initialise
*/
 *isInternalP = 1 ;
/*
** Drape Feature On DTM
*/
 if( bcdtmDrape_stringDtmObject(dtmP,featurePtsP,numFeaturePts,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
** Write Out Drape Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points = %8ld",numDrapePts) ;
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )
      {
       bcdtmWrite_message(0,0,0,"Drape Point[%4ld] = %12.5lf %12.5lf %12.5lf ** Line = %4ld Type = %2ld",(long)(drapeP-drapePtsP),drapeP->drapeX,drapeP->drapeY,drapeP->drapeZ,drapeP->drapeLine,drapeP->drapeType) ;
      }
   }
/*
** Check If Feature Goes External To Tin Hull
*/
 for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts && *isInternalP ; ++drapeP )
   {
   if (drapeP->drapeType == DTMDrapedLineCode::External) *isInternalP = 0;
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *isInternalP = 0 ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInsert_checkFeatureIsInternalToTinHullMrDtmObject
(
 BC_DTM_OBJ *dtmP,                 // ==> Dtm Object
 DPoint3d  *featurePtsP,                // ==> Feature Points
 long numFeaturePts,               // ==> Number Of Feature Points
 long *isInternalP                 // <== Set To 1 if feature is internal or 0 if feature is not internal
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numDrapePts ;
 DPoint3d *p3dP ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;

 double dd = 0.0,xi,yi,z ;
 long p1,p2,fndType,drapeFlag,onLine ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featurePtsP     = %p",featurePtsP) ;
    bcdtmWrite_message(0,0,0,"numFeaturePts   = %8ld",numFeaturePts) ;
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Check If Points Are Internal To DTM
*/
 if( dbg == 2 )
   {
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       if( bcdtmDrape_pointDtmObject(dtmP,p3dP->x,p3dP->y,&z,&drapeFlag)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** %12.5lf %12.5lf %10.4lf ** drapeFlag = %2ld",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z,drapeFlag) ;
       if( ! drapeFlag )
         {
          if( bcdtmFind_findClosestHullLineDtmObject(dtmP,p3dP->x,p3dP->y,&z,&fndType,&p1,&p2)) goto errexit ;
          bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** p1 = %8ld p2 = %8ld",fndType,p1,p2) ;
          if( fndType )
            {
             if( fndType == 1 )     // Hull Point
               {
                pnt1P = pointAddrP(dtmP,p1) ;
                dd = bcdtmMath_distance(p3dP->x,p3dP->y,pnt1P->x,pnt1P->y) ;
               }
             if( fndType == 2 )     // Hull Line
               {
                fndType = 0 ;
                pnt1P = pointAddrP(dtmP,p1) ;
                pnt2P = pointAddrP(dtmP,p2) ;
                dd = bcdtmMath_distanceOfPointFromLine(&onLine,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,p3dP->x,p3dP->y,&xi,&yi) ;
               }
             bcdtmWrite_message(0,0,0,"Distance To Tin Hull = %20.15lf ** dtmP->ppTol = %20.15lf",dd,dtmP->ppTol) ;
            }
         }
      }
   }
/*
** Initialise
*/
 *isInternalP = 1 ;
/*
** Drape Feature On DTM
*/
 if( bcdtmDrape_stringDtmObject(dtmP,featurePtsP,numFeaturePts,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
** Write Out Drape Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points = %8ld",numDrapePts) ;
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )
      {
       bcdtmWrite_message(0,0,0,"Drape Point[%4ld] = %12.5lf %12.5lf %12.5lf ** Line = %4ld Type = %2ld",(long)(drapeP-drapePtsP),drapeP->drapeX,drapeP->drapeY,drapeP->drapeZ,drapeP->drapeLine,drapeP->drapeType) ;
      }
   }
/*
** Check If Feature Goes External To Tin Hull
*/
 for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts && *isInternalP ; ++drapeP )
   {
   if (drapeP->drapeType == DTMDrapedLineCode::External) *isInternalP = 0;
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Feature Is Internal To Tin Hull Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *isInternalP = 0 ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmInsert_removeFeaturesWithUsertagMrDtmDtmObject
(
  BC_DTM_OBJ *dtmP,
  DTMUserTag userTag
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,dtmFeature,pointsRemoved,firstPoint,numPointsFlag ;
 char dtmFeatureTypeName[256];
 unsigned char *flagP,*pointsFlagP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
**  This Is A Special Purpose MrDTM Funtion Only
**  And Should Only Be Used By MrDTM Applications
**
**
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Features With Usertag MrDTM") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"userTag = %8I64d",userTag) ;
   }
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
** Log Features With User Tag
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Features = %8ld",dtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
       bcdtmWrite_message(0,0,0,"Dtm Feature[%4ld] ** Type = %-20s UserTag = %10I64d",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag) ;
      }
   }
/*
** Create Flag Array To Mark Points To Be Deleted
*/
 numPointsFlag = ( dtmP->numPoints / 8 + 1 ) ;
 pointsFlagP = ( unsigned char * ) malloc( numPointsFlag * sizeof( char )) ;
 if( pointsFlagP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( flagP = pointsFlagP ; flagP < pointsFlagP + numPointsFlag ; ++flagP ) *flagP = ( char ) 0 ;

/*
** Remove Feature And Mark Feature Points
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmUserTag == userTag )
      {
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&firstPoint)) goto errexit ;
       point = firstPoint ;
       do
         {
          bcdtmFlag_setFlag(pointsFlagP,point) ;
          point = nodeAddrP(dtmP,point)->tPtr ;
         } while( point != dtmP->nullPnt && point != firstPoint ) ;
       if( bcdtmList_nullTptrListDtmObject(dtmP,firstPoint)) goto errexit ;
       if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
      }
   }
/*
** Remove Marked Internal Points
*/
 pointsRemoved = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( bcdtmFlag_testFlag(pointsFlagP,point) )
      {
       if( nodeAddrP(dtmP,point)->fPtr == dtmP->nullPtr )
         {
          if( nodeAddrP(dtmP,point)->hPtr == dtmP->nullPnt )
            {
             ++pointsRemoved ;
             if( bcdtmEdit_deletePointDtmObject(dtmP,point,1)) goto errexit ;
            }
         }
      }
   }
/*
** Remove Marked External Points
*/
/*
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( bcdtmFlag_testFlag(pointsFlagP,point) )
      {
       if( nodeAddrP(dtmP,point)->fPtr == dtmP->nullPtr )
         {
          if( nodeAddrP(dtmP,point)->hPtr != dtmP->nullPnt )
            {
             ++pointsRemoved ;
             if( bcdtmEdit_deletePointDtmObject(dtmP,point,2)) goto errexit ;
            }
         }
      }
   }
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Points Removed = %8ld",pointsRemoved) ;
/*
** Clean DTM Object
*/
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(1,0,0,"Tin Valid") ;
   }
/*
** Log Features With User Tag
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Features = %8ld",dtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
       bcdtmWrite_message(0,0,0,"Dtm Feature[%4ld] ** Type = %-20s UserTag = %10I64d",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( pointsFlagP != NULL ) { free(pointsFlagP) ; pointsFlagP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Features With Usertag MrDTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Features With Usertag MrDTM Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkForColinearLines(double X1,double Y1,double X2,double Y2,double X3,double Y3,double X4,double Y4,double Mpptol)
/*
** This An Experimental Function That Checks For Two Lines Being Colinear
** Assumes That The Lines Mathematically Intersect
** This Is A Test To Determine If The Lines Actually Intersect Or Are Colinear
** Only To Be Used By Specific Insert Functions
**
*/
{
 int     colinear1,colinear2 ;
 long    dbg=DTM_TRACE_VALUE(0) ;
 double  n1=0.0,n2=0.0 ;
if( dbg )  bcdtmWrite_message(0,0,0,"Mpptol = %20.15lf N1 = %20.15lf N2 = %20.15lf",Mpptol,n1,n2) ;
/*
** Calculate Variables
*/
/*
 n1  = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X3,Y3) ;
 n2  = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X4,Y4) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Mpptol = %20.15lf N1 = %20.15lf N2 = %20.15lf",Mpptol,n1,n2) ;
    if( n1 == 0.0 ) bcdtmWrite_message(0,0,0,"N1 == 0.0") ;
    if( n2 == 0.0 ) bcdtmWrite_message(0,0,0,"N2 == 0.0") ;
   }
 if( n1 < Mpptol ) n1 = 0.0 ;
 if( n2 < Mpptol ) n2 = 0.0 ;
 if( n1 == 0.0 && n2 == 0.0 ) Colinear = 1 ;
 if( dbg )
   {
    if( Colinear  ) bcdtmWrite_message(0,0,0,"Lines Are Colinear") ;
    else            bcdtmWrite_message(0,0,0,"Lines Are Not Colinear") ;
   }
*/
 colinear1 = bcdtmMath_testForColinearPoints2d(X1,Y1,X2,Y2,X3,Y3) ;
 colinear2 = bcdtmMath_testForColinearPoints2d(X1,Y1,X2,Y2,X4,Y4) ;
/*
** Job Completed
*/
 if( colinear1 && colinear2 ) return(1) ;
 else                         return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmMath_testForColinearPoints2d(double x1,double y1,double x2,double y2,double x3,double y3)
/*
**
** This Function Tests If Three Points Are Colinear
** Uses The Vector Dot Product To Test For Colinear Points
**
** Author : Rob Cormack  July 2003   rob.cormack@bentley.com
**
*/
{
 DPoint3d      v1,v2 ;
 double   lengthV1,lengthV2,dotProduct,cosAngle ;
/*
** Determine Vectors
*/
 v1.x = x2 - x1 ;
 v1.y = y2 - y1 ;
 v2.x = x3 - x1 ;
 v2.y = y3 - y1 ;
/*
** Calculate Vector Lengths
*/
 lengthV1   = sqrt(v1.x*v1.x + v1.y*v1.y ) ;
 lengthV2   = sqrt(v2.x*v2.x + v2.y*v2.y ) ;
/*
** Calculate Dot Product For The Two Vextors
*/
 dotProduct = v1.x * v2.x + v1.y * v2.y ;
/*
** Calculate Cos Of Angle Between Vectors
*/
 cosAngle = dotProduct / ( lengthV1*lengthV2 )      ;
/*
** Job Completed
*/
 if( cosAngle == 1.0 || cosAngle == -1.0 ) return(1) ;
 else                                      return(0) ;
}



