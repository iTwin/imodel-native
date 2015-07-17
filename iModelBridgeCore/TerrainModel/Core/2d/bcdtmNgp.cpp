/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmNgp.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
//#pragma optimize( "p", on )
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLoad_markPointsInternalToPondsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Determines Ponds About
** Low Points , Zero Slope Sump Lines And Zero Slope Triangles
** And Marks All Points Internal To The Ponds
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long lowPoint,zeroSlopeSumpLine,zeroSlopeTriangle ;
 long ap,cp,p1,p2,p3,node,clptr,lowPnt,sumpPnt1,sumpPnt2,trgPnt1,trgPnt2,trgPnt3,voidLine,voidTriangle ;
 DTM_TIN_NODE *nodeP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Points InternalTo Ponds") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
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
** Scan Data Points And Determine Ponds
*/
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( ( clptr = nodeP->cPtr) != dtmP->nullPtr && nodeP->hPtr == dtmP->nullPnt )
      {
       p1 = node;
/*
**     Test For Void Point
*/
       if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) )
         {
/*
**        Ignore Void Points
*/
          lowPnt   = p1 ;
          lowPoint = TRUE ; 
          while ( clptr != dtmP->nullPtr && lowPoint )
            {
             p2  = clistAddrP(dtmP,clptr)->pntNum ;
             clptr = clistAddrP(dtmP,clptr)->nextPtr ;
             if( pointAddrP(dtmP,p2)->z <= pointAddrP(dtmP,p1)->z ) lowPoint = FALSE  ;
            }
/*
**        Test For Zero Slope Sumpline
*/
          if( ! lowPoint  ) 
            {
             clptr = nodeP->cPtr ;
             zeroSlopeSumpLine = FALSE ;
             while ( clptr != dtmP->nullPtr && ! zeroSlopeSumpLine )
               {
                p2  = clistAddrP(dtmP,clptr)->pntNum ;
                clptr = clistAddrP(dtmP,clptr)->nextPtr ;
                if( p2 > p1 ) 
                  {
/*
**                 Ignore Void Lines
*/
                   if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit ;
                   if( ! voidLine )
                     { 
                      if( nodeAddrP(dtmP,p1)->hPtr == dtmP->nullPnt )
                        {
                         if( pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p2)->z )
                           {
                            if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ; 
                            if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ; 
/*
**                          Check For Zero Slope Sump Line 
*/
                            if( pointAddrP(dtmP,ap)->z > pointAddrP(dtmP,p1)->z && pointAddrP(dtmP,cp)->z > pointAddrP(dtmP,p1)->z )
                              {
                               zeroSlopeSumpLine = TRUE ;
                               sumpPnt1 = p1 ;
                               sumpPnt2 = p2 ; 
                              }
                           } 
                        }
                     }
                  }
               } 
            }
/*
**        Test For Zero Slope Triangle Pond
*/
          if( ! lowPoint && ! zeroSlopeSumpLine  ) 
            {
             clptr = nodeP->cPtr ;
             zeroSlopeTriangle = FALSE ;
             if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clptr)->pntNum)) < 0 ) goto errexit ;
             while ( clptr != dtmP->nullPtr && ! zeroSlopeTriangle )
               {
                p3  = clistAddrP(dtmP,clptr)->pntNum ;
                clptr = clistAddrP(dtmP,clptr)->nextPtr ;
                if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,node)->hPtr != p2 )
                  {
/*
**                 Ignore Void Triangles
*/
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( ! voidTriangle )
                     {
                      if( pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p2)->z &&
                          pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p3)->z        )
                        {
                         zeroSlopeTriangle = TRUE ;
                         trgPnt1 = p1 ;  
                         trgPnt2 = p2 ;  
                         trgPnt3 = p3 ;  
                        }
                     } 
                  }
                p2 = p3 ;
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
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Ponds Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Ponds Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit : 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
} 
