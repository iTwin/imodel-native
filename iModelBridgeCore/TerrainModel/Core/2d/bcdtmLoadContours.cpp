/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLoadContours.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"
#include "TerrainModel\Drainage\drainage.h"
#include "..\Drainage\bcdtmDrainagePond.h"
//#pragma optimize( "p", on )

thread_local static DPoint3d  *conPtsP = NULL;               /* DTM Contour Points                                      */
thread_local static long *conWghtP=NULL ;                   /* Weighting Of Each Contour Point                         */
thread_local static long numConPts=0  ;                     /* Number Of DTM Contour Points                            */
thread_local static long memConPts=0  ;                     /* Amount Of Memory Currently Allocated For Contour Points */
thread_local static long memConPtsInc=10000 ;               /* Memory Allocation Amounts For Contour Points            */
typedef int (*DtmCallBack) ();
thread_local DTMFeatureCallback ContourLoadFunctionP = NULL;
thread_local void* ContourLoadFunctionUserArgP = NULL;

BcDTMAppData::Key const DTMPondAppData::AppDataID;
// Prototype Functions

class DtmContourIndexArraySort : public ArraySort<DtmContourIndex, DtmContourIndexArray>
    {
     private:
#ifdef _WIN32_WCE
        class CompareClass : public ICompareClass<DtmContourIndex>
            {
            private:
                BC_DTM_OBJ* m_dtmP;
            public:
                CompareClass(BC_DTM_OBJ *dtmP)
                    {
                    m_dtmP = dtmP;
                    }
                virtual int Compare(DtmContourIndex* index1P, DtmContourIndex* index2P)
                    {
                     // access to dtp via m_dtmP
                     double p1Z,p2Z,zMinLine1,zMaxLine1,zMinLine2,zMaxLine2 ;

                     p1Z = pointAddrP(m_dtmP,index1P->p1)->z ;
                     p2Z = pointAddrP(m_dtmP,index1P->p2)->z ;
                     if( p1Z <= p2Z ) { zMinLine1 = p1Z ; zMaxLine1 = p2Z ; }
                     else             { zMinLine1 = p2Z ; zMaxLine1 = p1Z ; }

                     p1Z = pointAddrP(m_dtmP,index2P->p1)->z ;
                     p2Z = pointAddrP(m_dtmP,index2P->p2)->z ;
                     if( p1Z <= p2Z ) { zMinLine2 = p1Z ; zMaxLine2 = p2Z ; }
                     else             { zMinLine2 = p2Z ; zMaxLine2 = p1Z ; }

                     if     (  zMinLine1 < zMinLine2 ) return(-1) ;
                     else if(  zMinLine1 > zMinLine2 ) return( 1) ;
                     else if(  zMaxLine1 < zMaxLine2 ) return(-1) ;
                     else if(  zMaxLine1 > zMaxLine2 ) return( 1) ;
                     return(0) ;
                    }

//JGA Changed this to below ... was not compiling this may be wrong ... need to debug
//JGA Changed this to below ... was not compiling this may be wrong ... need to debug
//JGA Changed this to below ... was not compiling this may be wrong ... need to debug
//JGA Changed this to below ... was not compiling this may be wrong ... need to debug
 /*
                virtual bool LessThan(TYPE* p1P, TYPE* p2P)
                    {
                     return Compare(p1P, p2P) < 0;
                    }
                virtual bool GreaterThan(TYPE* p1P, TYPE* p2P)
                    {
                     return Compare(p1P, p2P) > 0;
                    }
*/

                virtual bool LessThan(DtmContourIndex* index1P, DtmContourIndex* index2P)
                    {
                     return Compare(index1P, index2P) < 0;
                    }
                virtual bool GreaterThan(DtmContourIndex* index1P, DtmContourIndex* index2P)
                    {
                     return Compare(index1P, index2P) > 0;
                    }

            };

#else
        class CompareClass
            {
            private:
                BC_DTM_OBJ* m_dtmP;
            public:
                CompareClass(BC_DTM_OBJ *dtmP)
                    {
                    m_dtmP = dtmP;
                    }
                 __forceinline int Compare(DtmContourIndex* index1P, DtmContourIndex* index2P)
                    {
                     // access to dtp via m_dtmP
                     double p1Z,p2Z,zMinLine1,zMaxLine1,zMinLine2,zMaxLine2 ;

                     p1Z = pointAddrP(m_dtmP,index1P->p1)->z ;
                     p2Z = pointAddrP(m_dtmP,index1P->p2)->z ;
                     if( p1Z <= p2Z ) { zMinLine1 = p1Z ; zMaxLine1 = p2Z ; }
                     else             { zMinLine1 = p2Z ; zMaxLine1 = p1Z ; }

                     p1Z = pointAddrP(m_dtmP,index2P->p1)->z ;
                     p2Z = pointAddrP(m_dtmP,index2P->p2)->z ;
                     if( p1Z <= p2Z ) { zMinLine2 = p1Z ; zMaxLine2 = p2Z ; }
                     else             { zMinLine2 = p2Z ; zMaxLine2 = p1Z ; }

                     if     (  zMinLine1 < zMinLine2 ) return(-1) ;
                     else if(  zMinLine1 > zMinLine2 ) return( 1) ;
                     else if(  zMaxLine1 < zMaxLine2 ) return(-1) ;
                     else if(  zMaxLine1 > zMaxLine2 ) return( 1) ;
                     return(0) ;
                    }
                __forceinline bool LessThan(DtmContourIndex* p1P, DtmContourIndex* p2P)
                    {
                     return(Compare(p1P, p2P) < 0) ;
                    }

                __forceinline bool GreaterThan(DtmContourIndex* p1P, DtmContourIndex* p2P)
                    {
                     return(Compare(p1P, p2P) > 0) ;
                    }
            };
#endif
    public:
        int doSort(DtmContourIndexArray& theArray, BC_DTM_OBJ *dtmP)
            {
            CompareClass compare(dtmP);
#ifdef _WIN32_WCE
            return ArraySort<DtmContourIndex, DtmContourIndexArray>::DoSort(theArray, &compare, 0, theArray.getSize());
#else
            return ArraySort<DtmContourIndex, DtmContourIndexArray>::DoSort<CompareClass>(theArray, &compare, 0, theArray.getSize());
#endif
            }
        int doResort(DtmContourIndexArray& theArray, int start, int length, BC_DTM_OBJ *dtmP)
            {
            CompareClass compare(dtmP);
#ifdef _WIN32_WCE
            return ArraySort<DtmContourIndex, DtmContourIndexArray>::DoSort(theArray, &compare, start, length);
#else
            return ArraySort<DtmContourIndex, DtmContourIndexArray>::DoSort<CompareClass>(theArray, &compare, start, length);
#endif
            }
    };


/*==============================================================================*//**
* @memo   Frees The Contour Point Memory
* @doc    Frees The Contour Point Memory
* @notes  This function frees the Contour Point memory
* @author Rob Cormack 29 October 2004 rob.cormack@bentley.com
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_freeContourMemory(void)
{
 int ret=DTM_SUCCESS ;
 numConPts = memConPts = 0 ;
 if( conPtsP  != NULL ) { free(conPtsP)  ; conPtsP  = NULL ; }
 if( conWghtP != NULL ) { free(conWghtP) ; conWghtP = NULL ; }
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_removeDuplicateContourPoints(void)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long *wght1P,*wght2P ;
 DPoint3d  *p3d1P=NULL,*p3d2P=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Removing Duplicate Contour Points") ;
    bcdtmWrite_message(0,0,0,"Number Of Contour Points Before Removing Duplicates = %8ld",numConPts) ;
   }
/*
** Scan Load Points Array And Remove Duplicates
*/
 if( numConPts > 1 )
   {
    for( p3d1P = conPtsP , p3d2P = conPtsP + 1 , wght1P = conWghtP ,wght2P = conWghtP + 1 ; p3d2P < conPtsP + numConPts ; ++p3d2P , ++wght2P )
      {
       if( p3d1P->x != p3d2P->x || p3d1P->y != p3d2P->y || p3d1P->z != p3d2P->z )
         {
          ++p3d1P ;
          ++wght1P ;
          if( p3d1P != p3d2P )
            {
             *p3d1P  = *p3d2P  ;
             *wght1P = *wght2P ;
            }
         }
      }
   }
/*
** Reset Number Of Load Points
*/
 numConPts = (long)(p3d1P-conPtsP) + 1 ;
 if( dbg )  bcdtmWrite_message(0,0,0,"Number Of Contour Points After  Removing Duplicates = %8ld",numConPts) ;
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
BENTLEYDTM_Public int bcdtmLoad_setLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,unsigned char *maskLineP)
{
 long  clPtr ;
 DTM_CIR_LIST *clistP ;
/*
** Get Line Offset
*/
 if( pnt1 > pnt2 ) { clPtr = pnt1 ; pnt1 = pnt2 ; pnt2 = clPtr ; }
 clPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clPtr != dtmP->nullPtr  )
   {
    clistP = clistAddrP(dtmP,clPtr) ;
    if( clistP->pntNum == pnt2 )
      {
       bcdtmFlag_setFlag(maskLineP,clPtr) ;
       return(0) ;
      }
    clPtr = clistP->nextPtr ;
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
BENTLEYDTM_Public int bcdtmLoad_clearLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,unsigned char *maskLineP)
{
 long  clPtr ;
 DTM_CIR_LIST *clistP ;
/*
** Get Line Offset
*/
 if( pnt1 > pnt2 ) { clPtr = pnt1 ; pnt1 = pnt2 ; pnt2 = clPtr ; }
 clPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clPtr != dtmP->nullPtr  )
   {
    clistP = clistAddrP(dtmP,clPtr) ;
    if( clistP->pntNum == pnt2 )
      {
       bcdtmFlag_clearFlag(maskLineP,clPtr) ;
       return(0) ;
      }
    clPtr = clistP->nextPtr ;
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
BENTLEYDTM_Public int bcdtmLoad_testLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,unsigned char *maskLineP)
{
 long  clPtr ;
 DTM_CIR_LIST *clistP ;
/*
** Get Line Offset
*/
 if( pnt1 > pnt2 ) { clPtr = pnt1 ; pnt1 = pnt2 ; pnt2 = clPtr ; }
 clPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    clistP = clistAddrP(dtmP,clPtr) ;
    if( clistP->pntNum == pnt2 ) return (bcdtmFlag_testFlag(maskLineP,clPtr)) ;
    clPtr = clistP->nextPtr ;
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
BENTLEYDTM_Public int bcdtmLoad_testForOverlapWithTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 DPoint3d  *featPtsP,
 long numFeatPts,
 DTMFenceOption *featureExtentP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numDrapePts,numPtsOnDrape ;
 double xMin,yMin,xMax,yMax ;
 DPoint3d    *ptsP ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Overlap With Tin Hull") ;
/*
** Initialise
*/
 *featureExtentP = DTMFenceOption::Outside ;
/*
** Check Clipping Tin
*/
 if( dtmP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Clipping Tin") ;
    goto errexit ;
   }
/*
** Get Bounding Rectangle For Feature
*/
 xMin = xMax = featPtsP->x ;
 yMin = yMax = featPtsP->y ;
 for( ptsP = featPtsP + 1 ; ptsP < featPtsP + numFeatPts ; ++ptsP )
   {
    if( ptsP->x < xMin ) xMin = ptsP->x ;
    if( ptsP->x > xMax ) xMax = ptsP->x ;
    if( ptsP->y < yMin ) yMin = ptsP->y ;
    if( ptsP->y > yMax ) yMax = ptsP->y ;
   }
/*
** Determine Feature Extent In Relation To Tin Hull
*/
 if( xMax >= dtmP->xMin && xMin <= dtmP->xMax && yMax >= dtmP->yMin && yMin <= dtmP->yMax )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Draping Feature On Clip Tin") ;
/*
**  Drape Feature Points On Clipping Tin
*/
    if( bcdtmDrape_stringDtmObject(dtmP,featPtsP,numFeatPts,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
**  Write Drape Points
*/
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts) ;
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )
         {
          bcdtmWrite_message(0,0,0,"Drape Point[%6ld]  L = %4ld T = %2ld ** %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeLine,drapeP->drapeType,drapeP->drapeX,drapeP->drapeY ) ;
         }
      }
/*
**  Determine If Feature Overlaps Tin Hull
*/
    numPtsOnDrape = 0 ;
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )
      {
      if (drapeP->drapeType != DTMDrapedLineCode::External) ++numPtsOnDrape;
      }
    if     ( numPtsOnDrape == numDrapePts ) *featureExtentP = DTMFenceOption::Inside  ;
    else if( numPtsOnDrape == 0           ) *featureExtentP = DTMFenceOption::Outside ;
    else                                    *featureExtentP = DTMFenceOption::Overlap ;
    if( dbg ) bcdtmWrite_message(0,0,0,"featureExtentP = %2ld",*featureExtentP) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Overlap With Tin Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Overlap With Tin Hull Error") ;
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
BENTLEYDTM_Public int bcdtmLoad_buildContourIndexDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   startPnt,
 long   lastPnt,
 double cMinZ,
 double cMaxZ,
 double cInt,
 double cReg,
 unsigned char   **tinLinePP,
 DtmContourIndexArray& contourIndexPP,
 long *numContourIndexP
)
/*
** This Function Builds An Index For Tracing Contours
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   p1,p2,cl,level,startTime=0,startIndex=0,indexLine,numContours;
 long   memContourIndex=0,memContourIndexInc=32768 ;

 double zMin,zMax,firstContour,lastContour;
 unsigned char   *cp ;
 DtmContourIndexArraySort sort;
 DtmContourIndexArray::iterator index;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Building Contour Index")     ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP)       ;
    bcdtmWrite_message(0,0,0,"startPnt   = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"lastPnt    = %8ld",lastPnt)  ;
    bcdtmWrite_message(0,0,0,"cMinZ      = %8.2lf",cMinZ)  ;
    bcdtmWrite_message(0,0,0,"cMaxZ      = %8.2lf",cMaxZ)  ;
    bcdtmWrite_message(0,0,0,"cReg       = %8.2lf",cReg)  ;
   }
    startIndex = bcdtmClock() ;
/*
** Allocate Memory For Tin Line Flag If Not Already Allocated
*/
 if( *tinLinePP == NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Void Window Flag") ;
    *tinLinePP = ( unsigned char * ) malloc(dtmP->numPoints*sizeof(char)) ;
    if( *tinLinePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
    for( cp = *tinLinePP ; cp < *tinLinePP + dtmP->numPoints ; ++cp ) *cp = 0 ;
   }
/*
** Determine First And Last Contours For Contour Range
*/
 level = (long) ((cMinZ - cReg) / cInt ) ;
 firstContour = cReg + ((double)level) * cInt ;
 level = (long) ((cMaxZ - cReg) / cInt ) + 1 ;
 lastContour  = cReg + ((double)level) * cInt ;
 firstContour = bcdtmMath_roundToDecimalPoints(firstContour,8) ;
 lastContour  = bcdtmMath_roundToDecimalPoints(lastContour,8) ;
 numContours  = (long)((lastContour-firstContour) / cInt) + 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numContours = %4ld firstContour = %10.6lf lastContour = %10.6lf",numContours,firstContour,lastContour) ;
/*
** Validate Contour Range
*/
 if( numContours < 0 )
   {
    bcdtmWrite_message(1,0,0,"Error In Contour Range") ;
    goto errexit ;
   }
/*
** Allocate Memory For Contour Index
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Contour Index") ;
 *numContourIndexP = 0 ;
// contourIndexPP.resize(dtmP->numLines);
// RobC 24Apr2008 - Allocate memory For One Partition Only
 memContourIndex = memContourIndexInc ;
 contourIndexPP.resize(memContourIndex);
 index = contourIndexPP.start();
/*
** Populate Contour Index
*/
 for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
   {
    cl = nodeAddrP(dtmP,p1)->cPtr ;
    while ( cl != dtmP->nullPtr )
      {
       p2 = clistAddrP(dtmP,+cl)->pntNum ;
       cl = clistAddrP(dtmP,+cl)->nextPtr ;
/*
**     Process Tin Line
*/
       if( p2 > p1 && ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,*tinLinePP) )
         {
/*
**       Ignore Boundary Lines
*/
          if( ! bcdtmList_testForHullLineDtmObject(dtmP,p1,p2) &&
              ! bcdtmList_testForHullLineDtmObject(dtmP,p2,p1)    )
            {
/*
**           Ignore Zero Slope Lines
*/
             double p1Z = pointAddrP(dtmP,p1)->z;
             double p2Z = pointAddrP(dtmP,p2)->z;
             if( p1Z != p2Z )
               {
                if( p1Z < p2Z ) { zMin = p1Z ; zMax = p2Z ; }
                else            { zMin = p2Z ; zMax = p1Z ; }
/*
**              Ignore Lines That Contours Can Not Start On
*/
                indexLine = 0 ;

                double h = zMin - firstContour;
                h = firstContour + (ceil(h / cInt) * cInt);

                if( h <= zMax) indexLine = 1;

                if( indexLine && *numContourIndexP < dtmP->numLines)
                  {
/*
**                 Check Memory
*/
                   if( *numContourIndexP == memContourIndex)
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"00 *numContourIndexP = %8ld ** memContourIndex = %8ld",*numContourIndexP,memContourIndex) ;
                      memContourIndex = memContourIndex + memContourIndexInc ;
                      if( contourIndexPP.resize(memContourIndex))
                        {
                         bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                         goto errexit ;
                        }
                      if( dbg ) bcdtmWrite_message(0,0,0,"01 *numContourIndexP = %8ld ** memContourIndex = %8ld",*numContourIndexP,memContourIndex) ;
                      index = contourIndexPP.start() + *numContourIndexP ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Index Reset") ;
                     }
/*
**                 Store Index
*/
                   index->p1 = p1 ;
                   index->p2 = p2 ;
                   ++index;
                   ++*numContourIndexP ;
                  }
               }
            }
         }
      }
   }
/*
** Reallocate memory
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numIndexes = %8ld ** indexSize = %8ld",*numContourIndexP,contourIndexPP.getSize()) ;
    bcdtmWrite_message(0,0,0,"Number Of Tin Lines = %8ld",dtmP->numLines) ;
   }
 if( *numContourIndexP < memContourIndex )  contourIndexPP.resize(*numContourIndexP);
/*
** Write Statistics On Population Time
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Contour Index Population Time = %7.4lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Sort Contour Index
*/
 if( *numContourIndexP > 1 )
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Contour Index") ;
    sort.doSort(contourIndexPP,dtmP);
   }
/*
** Write Statistics On Sort And Build Times
*/
 if( tdbg )
   {
    bcdtmWrite_message(0,0,0,"Contour Index Sort Time  = %7.4lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"Contour Index Build Time = %7.4lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startIndex)) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Contour Index Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Contour Index Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto  cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_getFirstTinLineForContourFromIndexDtmObject(BC_DTM_OBJ *dtmP,DtmContourIndexArray& contourIndexP,long numContourIndex,double contourValue,long *contourLineP)
/*
** This Function Finds The First Entry In The Contour Index For The Contour Value
*/
{
 int  ret=DTM_SUCCESS ;
 long ofs,process=1 ;
 double zMin,zMax ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Initialise
*/
 ofs = *contourLineP ;
/*
** Check Index Line Range
*/
 if( ofs < numContourIndex )
   {
/*
**  Set Start Index Position
*/
    DtmContourIndexArray::iterator index = contourIndexP.start() + ofs;
/*
**  Scan Index To First Entry For Contour Value
*/
    p1P = pointAddrP(dtmP,index->p1) ;
    p2P = pointAddrP(dtmP,index->p2) ;
    if( p1P->z <= p2P->z ) { zMin = p1P->z ; zMax = p2P->z ; }
    else                   { zMax = p1P->z ; zMin = p2P->z ; }

    while( ofs < numContourIndex && contourValue >= zMin && process )
      {
       if( contourValue <= zMax )
         {
          process = 0 ;
          *contourLineP = ofs ;
         }
       else
         {
          ++ofs ;
          if( ofs < numContourIndex )
            {
             ++index;
             p1P = pointAddrP(dtmP,index->p1) ;
             p2P = pointAddrP(dtmP,index->p2) ;
             if( p1P->z <= p2P->z ) { zMin = p1P->z ; zMax = p2P->z ; }
             else                   { zMax = p1P->z ; zMin = p2P->z ; }
            }
         }
      }
   }
/*
** Check First Contour Line Is In Index Range
*/
 if( *contourLineP < 0 || *contourLineP >= numContourIndex )
   {
    bcdtmWrite_message(2,0,0,"Contour Index Range Error") ;
    goto errexit ;
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
 goto  cleanup ;
}
/*==============================================================================*//**
* @memo   Load Contours From A DTM File
* @doc    Load Contours From A DTM File
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @notes  Smoothing Option 2 Spline Smooths The Contour With Contour Overlap Detection
* @notes  Smoothing Option 3 Spline Smooths The Contour Without Contour Overlap Detection
* @param  dtmFile               ==> Dtm File Name
* @param  conInt                ==> Contour Interval
* @param  conReg                ==> Contour Registration
* @param  loadRange             ==> Load Contour Range <TRUE,FALSE>
* @param  conMin                ==> Contour Range Minimum Value
* @param  conMax                ==> Contour Range Maximum Value
* @param  loadValues            ==> Load Contour Values
* @param  *conValuesP           ==> Contour Values
* @param  numConValues          ==> Number Of Contour Values
* @param  smoothOption          ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)>
* @param  smoothFactor          ==> Smoothing Factor
* @param  smoothDensity         ==> Point Densification For Spline Smoothing
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence <TRUE,FALSE>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> P3D Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_contoursFromDtmFile
(
 WCharCP dtmFile,            /* ==> DTM File Name                         */
 double  conInt,              /* ==> Contour Interval                      */
 double  conReg,              /* ==> Contour Registration                  */
 long    loadRange,           /* ==> Load Contour Range <TRUE,FALSE>       */
 double  conMin,              /* ==> Contour Range Minimum Value           */
 double  conMax,              /* ==> Contour Range Maximum Value           */
 long    loadValues,          /* ==> Load Contour Values                   */
 double  *conValuesP,         /* ==> Contour Values                        */
 long    numConValues,        /* ==> Number Of Contour Values              */
 DTMContourSmoothing smoothOption,        /* ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)> */
 double  smoothFactor,        /* ==> Contour Smoothing Factor              */
 long    smoothDensity,       /* ==> Point Densification For Spline Smoothing            */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function              */
 long    useFence,            /* ==> Load Feature Within Fence<TRUE,FALSE> */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>   */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>        */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points             */
 long    numFencePts,         /* ==> Number Of Fence Points                */
 void    *userP               /* ==> User Pointer Passed Back To User      */
)
/*
** This Function Loads Contours From A Dtm Object
*/
{
 int     ret=DTM_SUCCESS ;
 long   depressionOption=0,maxSlopeOption=0 ;
 double smoothLength=0.0,maxSlopeValue=0.0 ;
 BC_DTM_OBJ  *dtmP=NULL ;
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFile)) goto errexit ;
/*
** Load Dtm Feature From Dtm Object
*/
 if( bcdtmLoad_contoursFromDtmObject(dtmP,conInt,conReg,loadRange,conMin,conMax,loadValues,conValuesP,numConValues,smoothOption,smoothFactor,smoothDensity,smoothLength,useFence,fenceOption,fenceType,fencePtsP,numFencePts,depressionOption,maxSlopeOption,maxSlopeValue,loadFunctionP,userP)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmLoad_contoursCreateDepressionDtmObject
(
    BC_DTM_OBJ* dtmP,
     DTMFeatureCallback loadFunctionP,    /* ==> Pointer To Load Function                            */
     void    *userP                  /* ==> User Pointer Passed Back To User                    */
    )
    {
    int ret = DTM_SUCCESS;
    int dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0);
    long testTime ;

//  Create Extended Component If It Does Not Exist

    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Depression Contours") ;
    DTMPondAppData* pondAppData = reinterpret_cast<DTMPondAppData*>(dtmP->FindAppData (DTMPondAppData::AppDataID));
    if ( !pondAppData)
        {
        pondAppData = DTMPondAppData::Create ();
        dtmP->AddAppData (DTMPondAppData::AppDataID, pondAppData);
         }

//  Check If New Depression DTM Needs To Be Calculated

    testTime = bcdtmClock() ;
    if (!pondAppData->hasPonds || pondAppData->m_dtmCreationTime != dtmP->lastModifiedTime)
      {

//     This needs Temporary access to the arrays.

       bcdtmMemory_setMemoryAccess(dtmP, DTMAccessMode::Temporary);
       if (pondAppData->pondDtmP) bcdtmObject_destroyDtmObject (&pondAppData->pondDtmP);
       ContourLoadFunctionP = loadFunctionP ;
       ContourLoadFunctionUserArgP = pondAppData;

//     Do A Check Stop Call

       if( bcdtmLoad_callUserLoadFunction(ContourLoadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,NULL,0,userP)) goto errexit ;

//     Create Depression DTM

       if( dbg ) bcdtmWrite_message(0,0,0,"Creating Depression DTM") ;
       if( bcdtmLoad_createDepressionDtmObject(dtmP, pondAppData->pondDtmP))
         {
          if( pondAppData->pondDtmP != NULL ) bcdtmObject_destroyDtmObject( &pondAppData->pondDtmP) ;
//          goto errexit ;
         }
       pondAppData->hasPonds = 1 ;
       pondAppData->m_dtmCreationTime = dtmP->lastModifiedTime;
      }
    else if( dbg ) bcdtmWrite_message(0,0,0,"Depression DTM Prior Created") ;

//  Log Timing Stats

    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"Time To Create Depression  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),testTime) ) ;
       bcdtmWrite_message(0,0,0,"Number Of Depression Ponds = %8ld",pondAppData->pondDtmP->numFeatures) ;
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
     BENTLEYDTM_EXPORT int bcdtmLoad_contoursFromDtmObject
         (
         BC_DTM_OBJ *dtmP,                  /* ==> Pointer to Dtm object                 */
         double  conInt,                    /* ==> Contour Interval                      */
         double  conReg,                    /* ==> Contour Registration                  */
         long    loadRange,              /* ==> Load Contour Range <TRUE,FALSE>       */
         double  conMin,                    /* ==> Contour Range Minimum Value           */
         double  conMax,                    /* ==> Contour Range Maximum Value           */
         long    loadValues,             /* ==> Load Contour Values                   */
         double  *conValuesP,               /* ==> Contour Values                        */
         long    numConValues,              /* ==> Number Of Contour Values              */
         DTMContourSmoothing smoothOption,  /* ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)> */
         double  smoothFactor,           /* ==> Contour Smoothing Factor                            */
         long    smoothDensity,          /* ==> Point Densification For Spline Smoothing            */
         double  smoothLength,           /* ==> Distance Between Spline Vertices                    */
         long    useFence,                  /* ==> Load Contours Within Fence<TRUE,FALSE>              */
         DTMFenceOption fenceOption,               /* ==> Fence Option <DTMFenceOption::Inside,DTMFenceOption::Overlap,DTMFenceOption::Outside>   */
         DTMFenceType fenceType,                 /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>        */
         DPoint3d     *fencePtsP,                /* ==> DPoint3d Array Of Fence Points                           */
         long    numFencePts,               /* ==> Number Of Fence Points                              */
         long    depressionOption,          /* ==> Mark Depression Contours                            */
         long    maxSlopeOption,            /* ==> Max Slope Option                                    */
         double  maxSlopeValue,          /* ==> Max Slope Value                                     */
         DTMFeatureCallback loadFunctionP,  /*==> Pointer To Load Function                             */
         void    *userP                  /* ==> User Pointer Passed Back To User                    */
         )
         {
         BENTLEY_NAMESPACE_NAME::TerrainModel::DTMContourParams contourParams;
         /*
         ** Create Contour Params
         */
         contourParams.interval = conInt;
         contourParams.conReg = conReg;
         contourParams.loadRange = loadRange != 0;
         contourParams.conMin = conMin;
         contourParams.conMax = conMax;
         contourParams.loadValues = loadValues;
         contourParams.conValuesP = conValuesP;
         contourParams.numConValues = numConValues;
         contourParams.smoothOption = smoothOption;
         contourParams.smoothFactor = smoothFactor;
         contourParams.smoothDensity = smoothDensity;
         contourParams.smoothLength = smoothLength;
         contourParams.depressionOption = depressionOption != 0;
         contourParams.maxSlopeOption = maxSlopeOption;
         contourParams.maxSlopeValue = maxSlopeValue;
         return bcdtmLoad_contoursFromDtmObject (dtmP, contourParams, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMFenceParams (useFence ? DTMFenceType::None : fenceType, fenceOption, fencePtsP, numFencePts), loadFunctionP, userP);
         }

BENTLEYDTM_EXPORT int bcdtmLoad_contoursFromDtmObject
(
BC_DTM_OBJ *dtmP,                  /* ==> Pointer to Dtm object                 */
TerrainModel::DTMContourParamsCR contourParamsC,
TerrainModel::DTMFenceParamsCR fenceParams,
DTMFeatureCallback loadFunctionP,  /*==> Pointer To Load Function                             */
void    *userP                  /* ==> User Pointer Passed Back To User                    */
)
/*
** This Function Loads Contours From A Dtm Object
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long    p1,p2,cPtr,node,listPtr,intOffset ;
 long    findType,minPnt,maxPnt,startPnt=0,lastPnt=0,trgPnt1,trgPnt2,trgPnt3 ;
 long    voidsInDtm,voidLine,numMarked=0,count ;
 long    numContourIndex=0,contourStartIndex=0,startTime,testTime ;
 double  *dblP,firstContour,lastContour,contourValue,zClipMin,zClipMax ;
 unsigned char    *cP,*tinLine1P=NULL,*tinLine2P=NULL ;
 bool useFence = (DTMFenceType::None != fenceParams.fenceType);
 DPoint3dCP     p3dP ;
 DTM_TIN_POINT *pntP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ  *clipDtmP=NULL;
 DtmContourIndexArray  contourIndex;
 BENTLEY_NAMESPACE_NAME::TerrainModel::DTMContourParams contourParams (contourParamsC);
/*
** Write Entry Message
*/
startTime = bcdtmClock() ;
if( dbg )
   {
   bcdtmWrite_message (0, 0, 0, "Loading Contours From Dtm Object");
    bcdtmWrite_message (0, 0, 0, "dtmP             = %p", dtmP);
    bcdtmWrite_message (0, 0, 0, "conInt           = %8.3lf", contourParams.interval);
    bcdtmWrite_message (0, 0, 0, "conReg           = %8.3lf", contourParams.conReg);
    bcdtmWrite_message (0, 0, 0, "loadRange        = %8ld", contourParams.loadRange);
    bcdtmWrite_message (0, 0, 0, "conMin           = %8.3lf", contourParams.conMin);
    bcdtmWrite_message (0, 0, 0, "conMax           = %8.3lf", contourParams.conMax);
    bcdtmWrite_message (0, 0, 0, "loadValues       = %8ld", contourParams.loadValues);
    bcdtmWrite_message (0, 0, 0, "conValuesP       = %p", contourParams.conValuesP);
    bcdtmWrite_message (0, 0, 0, "numConValues     = %8ld", contourParams.numConValues);
    bcdtmWrite_message (0, 0, 0, "smoothOption     = %8ld", contourParams.smoothOption);
    bcdtmWrite_message (0, 0, 0, "smoothFactor     = %8.2lf", contourParams.smoothFactor);
    bcdtmWrite_message (0, 0, 0, "smoothDensity    = %8ld", contourParams.smoothDensity);
    bcdtmWrite_message (0, 0, 0, "smoothLength     = %8.3lf", contourParams.smoothLength);
    bcdtmWrite_message (0, 0, 0, "useFence         = %8ld", useFence);
    bcdtmWrite_message (0, 0, 0, "fenceOption      = %8ld", fenceParams.fenceOption);
    bcdtmWrite_message (0, 0, 0, "fenceType        = %8ld", fenceParams.fenceType);
    bcdtmWrite_message (0, 0, 0, "fencePtsP        = %p", fenceParams.points);
    bcdtmWrite_message (0, 0, 0, "numFencePts      = %8ld", fenceParams.numPoints);
    bcdtmWrite_message (0, 0, 0, "depressionOption = %8ld", contourParams.depressionOption);
    bcdtmWrite_message (0, 0, 0, "maxSlopeOption   = %8ld", contourParams.maxSlopeOption);
    bcdtmWrite_message (0, 0, 0, "maxSlopeValue    = %8ld", contourParams.maxSlopeValue);
    bcdtmWrite_message (0, 0, 0, "Load Function    = %p", loadFunctionP);
    bcdtmWrite_message (0, 0, 0, "User Pointer     = %p", userP);
    if (useFence && fenceParams.numPoints > 2 && fenceParams.points != NULL)
      {
      for (p3dP = fenceParams.points; p3dP < fenceParams.points + fenceParams.numPoints; ++p3dP)
         {
         bcdtmWrite_message (0, 0, 0, "Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf", (long)(p3dP - fenceParams.points), p3dP->x, p3dP->y, p3dP->z);
         }
       bcdtmWrite_message(0,0,0,"DTM Coordinate Ranges") ;
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",dtmP->xMin,dtmP->xMax,dtmP->xRange) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",dtmP->yMin,dtmP->yMax,dtmP->yRange) ;
       bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",dtmP->zMin,dtmP->zMax,dtmP->zRange) ;
      }
   }
/*
** Write Fence Stats
*/
/*
 if( fencePtsP != NULL && numFencePts > 0 )
   {
    DTMDirection direction ;
    double area,xFenceMin,xFenceMax,yFenceMin,yFenceMax;
    xFenceMin = xFenceMax = fencePtsP->x ;
    yFenceMin = yFenceMax = fencePtsP->y ;
    for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
       if( p3dP->x < xFenceMin ) xFenceMin = p3dP->x ;
       if( p3dP->x > xFenceMax ) xFenceMax = p3dP->x ;
       if( p3dP->y < yFenceMin ) yFenceMin = p3dP->y ;
       if( p3dP->y > yFenceMax ) yFenceMax = p3dP->y ;
      }
    bcdtmWrite_message(0,0,0,"Triangles ** fenceType = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"xFenceMin = %12.3lf xFenceMax = %12.3lf xFenceRange = %12.3lf",xFenceMin,xFenceMax,xFenceMax-xFenceMin) ;
    bcdtmWrite_message(0,0,0,"yFenceMin = %12.3lf yFenceMax = %12.3lf yFenceRange = %12.3lf",yFenceMin,yFenceMax,yFenceMax-yFenceMin) ;
    bcdtmMath_getPolygonDirectionP3D(fencePtsP,numFencePts,&direction,&area) ;
    if( direction == 1 ) bcdtmWrite_message(0,0,0,"Fence Area = %12.5lf Fence Direction = Clockwise",area) ;
    else                 bcdtmWrite_message(0,0,0,"Fence Area = %12.5lf Fence Direction = Anti Clockwise",area) ;
   }
*/
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit;
   }
 if (dtmP->numLines == 0)
   {
    if (dbg) bcdtmWrite_message(0,0,0,"DTM has no lines so they can't be any contours.") ;
    goto cleanup;
   }

 if (contourParams.loadValues)
     contourParams.interval = 0.0;
 /*
 **     Turn Fence Off If Fence Option Has Been Set To Overlap As The Dtm
 **     Lines Were Selected With The Overlap Option
 */
 if (fenceParams.fenceType != DTMFenceType::None && fenceParams.fenceOption == DTMFenceOption::Overlap) useFence = false;

/*
** Validate Contour Value Range
*/
 if (contourParams.conMin != contourParams.conMax)
   {
   if (contourParams.conMin < dtmP->zMin) contourParams.conMin = dtmP->zMin;
   if (contourParams.conMax > dtmP->zMax) contourParams.conMax = dtmP->zMax;
   }
 if (contourParams.conMin >= contourParams.conMax)
   {
   contourParams.conMin = dtmP->zMin;
   contourParams.conMax = dtmP->zMax;
   }
/*
** Validate Smoothing Options
*/
 if (contourParams.smoothOption < DTMContourSmoothing::None || contourParams.smoothOption > DTMContourSmoothing::SplineWithoutOverLapDetection) contourParams.smoothOption = DTMContourSmoothing::None;
/*
** Validate Smoothing Factor
*/
 if (contourParams.smoothOption == DTMContourSmoothing::Vertex)
   {
   if (contourParams.smoothFactor < 0.1 || contourParams.smoothFactor > 0.5) contourParams.smoothFactor = 0.3;
   }
 else if (contourParams.smoothOption == DTMContourSmoothing::Spline || contourParams.smoothOption == DTMContourSmoothing::SplineWithoutOverLapDetection)
   {
   if (contourParams.smoothFactor < 0.0 || contourParams.smoothFactor  > 5.0) contourParams.smoothFactor = 2.5;
   if (contourParams.smoothDensity < 3 || contourParams.smoothDensity > 10) contourParams.smoothDensity = 5;
   }
/*
** Validate Fence Options
*/
 if (useFence)
   {
   if (fenceParams.fenceType != DTMFenceType::Block && fenceParams.fenceType != DTMFenceType::Shape) useFence = false;
   if (fenceParams.fenceOption != DTMFenceOption::Inside && fenceParams.fenceOption != DTMFenceOption::Overlap && fenceParams.fenceOption != DTMFenceOption::Outside) useFence = false;
   else if (fenceParams.points == NULL || fenceParams.numPoints <= 4) useFence = false;
   else if (fenceParams.points->x != (fenceParams.points + fenceParams.numPoints - 1)->x || fenceParams.points->y != (fenceParams.points + fenceParams.numPoints - 1)->y) useFence = false;
   }
/*
** Validate Depression Option
*/
// if( depressionOption && loadRange == FALSE ) depressionOption = FALSE ;
/*
**  Create Depression DTM
*/
 if (contourParams.depressionOption)
   bcdtmLoad_contoursCreateDepressionDtmObject (dtmP, loadFunctionP, userP);

 DTMPondAppData* pondAppData = contourParams.depressionOption ? reinterpret_cast <DTMPondAppData*>(dtmP->FindAppData (DTMPondAppData::AppDataID)) : nullptr;
/*
** Validate Max Slope Option
*/
 if (contourParams.maxSlopeOption && contourParams.maxSlopeValue <= 0.0) contourParams.maxSlopeOption = 0;
/*
** Scan Dtm Features And Check For The Presence Of Voids
*/
 voidsInDtm = FALSE ;
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
/*
** Build Clipping Dtm For Fence Operations
*/
 if (useFence)
   {
   if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fenceParams.points, fenceParams.numPoints)) useFence = false;
    else
      {
      if (fenceParams.fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax) useFence = false;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld",useFence) ;
   }
/*
** Allocate Memory For Marking Scanned Dtm Lines
*/
 tinLine1P = ( unsigned char * ) malloc (dtmP->numLines*sizeof(char)) ;
 if( tinLine1P == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( cP = tinLine1P ; cP < tinLine1P + dtmP->numLines ; ++cP ) *cP = 0 ;
/*
** Initialise Point Range
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
/*
** Mark Dtm Lines Outside Of Fence And Internal To Voids As Scanned
*/
 if(  useFence || voidsInDtm == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Dtm Lines External To Fence Or Internal To Voids") ;
    testTime = bcdtmClock() ;
    tinLine2P = ( unsigned char * ) malloc( dtmP->numLines*sizeof(char)) ;
    if( tinLine2P == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( cP = tinLine2P ; cP < tinLine2P + dtmP->numLines ; ++cP ) *cP = (char)-1/*255*/ ;
/*
**  Initialise Point And z Ranges For Clipping
*/
    zClipMin = dtmP->zMin ;
    zClipMax = dtmP->zMax ;
    startPnt = 0 ;
    lastPnt  = dtmP->numPoints - 1 ;
/*
**  Set First And Last Points For Fence
*/
    if( useFence)
      {
       testTime = bcdtmClock() ;
/*
**     Initialise
*/
       minPnt = lastPnt ;
       maxPnt = startPnt  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Fence Point Extent") ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints  - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Initial ** startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**     Mark Points Within Fence Block
*/
       testTime = bcdtmClock() ;
       if (fenceParams.fenceType == DTMFenceType::Block)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
          for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
            {
             pntP = pointAddrP(dtmP,p1) ;
             if( pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
               {
                nodeAddrP(dtmP,p1)->sPtr = 1 ;
                cPtr = nodeAddrP(dtmP,p1)->cPtr ;
                while( cPtr != dtmP->nullPtr )
                  {
                   ++numMarked ;
                   clistP = clistAddrP(dtmP,cPtr) ;
                   nodeAddrP(dtmP,clistP->pntNum)->sPtr = 1 ;
                   cPtr = clistP->nextPtr ;
                   if( clistP->pntNum < minPnt ) minPnt = clistP->pntNum ;
                   if( clistP->pntNum > maxPnt ) maxPnt = clistP->pntNum ;
                  }
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"Num Marked Within Fence Block = %8ld",numMarked) ;
         }
/*
**     Mark Points Within Shape Fence
*/
       else if (fenceParams.fenceType == DTMFenceType::Shape)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Shape") ;
          for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
            {
             pntP = pointAddrP(dtmP,p1) ;
             findType = 0 ;
             if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
               {
                if( bcdtmFind_triangleDtmObject(clipDtmP,pntP->x,pntP->y,&findType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                if( findType  )
                  {
                   nodeAddrP(dtmP,p1)->sPtr = 1 ;
                   cPtr = nodeAddrP(dtmP,p1)->cPtr ;
                   while( cPtr != dtmP->nullPtr )
                     {
                      ++numMarked ;
                      clistP = clistAddrP(dtmP,cPtr) ;
                      nodeAddrP(dtmP,clistP->pntNum)->sPtr = 1 ;
                      cPtr = clistP->nextPtr ;
                      if( clistP->pntNum < minPnt ) minPnt = clistP->pntNum ;
                      if( clistP->pntNum > maxPnt ) maxPnt = clistP->pntNum ;
                     }
                  }
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"Num Marked Within Fence Shape = %8ld",numMarked) ;
         }
/*
**    Mark Triangle Edges That Span The Fence
*/
      if( bcdtmLoad_markTriangleEdgesThatSpanTheFenceDtmObject(dtmP,clipDtmP,&minPnt,&maxPnt,&numMarked)) goto errexit ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Num Marked After Intersection With Fence Edge = %8ld",numMarked) ;
/*
**     Reset First And Last Point
*/
      startPnt = minPnt ;
      lastPnt  = maxPnt ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Reset ** startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
      if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Mark Fence Points  = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),testTime)) ;
      }
/*
** Mark Dtm Lines
*/
    testTime = bcdtmClock() ;
    zClipMin = zClipMax = pointAddrP(dtmP,startPnt)->z ;
    for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
      {
       listPtr = nodeAddrP(dtmP,p1)->cPtr ;
       while ( listPtr != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,listPtr)->pntNum ;
          listPtr = clistAddrP(dtmP,listPtr)->nextPtr ;
          if( p2 > p1 )
            {
             bool clipFlag = false ;
/*
**           Determine Tin Line Extents In Relation To Fence Option And Fence Extent
*/
             if( useFence && ( nodeAddrP(dtmP,p1)->sPtr != 1 || nodeAddrP(dtmP,p2)->sPtr != 1 )) clipFlag = true ;
/*
**           Clip Lines Internal To Voids
*/
             if( voidsInDtm == TRUE && clipFlag == false )
               {
                bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine) ;
                if( voidLine ) clipFlag = true ;
               }
/*
**           Mark Clip Line And Set Contour Ranges
*/
             if( clipFlag == false )
               {
                bcdtmLoad_clearLineDtmObject(dtmP,p1,p2,tinLine2P) ;
                if(pointAddrP(dtmP,p1)->z < zClipMin ) zClipMin = pointAddrP(dtmP,p1)->z ;
                if(pointAddrP(dtmP,p1)->z > zClipMax ) zClipMax = pointAddrP(dtmP,p1)->z ;
                if(pointAddrP(dtmP,p2)->z < zClipMin ) zClipMin = pointAddrP(dtmP,p2)->z ;
                if(pointAddrP(dtmP,p2)->z > zClipMax ) zClipMax = pointAddrP(dtmP,p2)->z ;
               }
            }
         }
      }
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Mark Fence Lines   = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),testTime)) ;
/*
**  Reset SPTR Values
*/
    if( useFence ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
**  Reset Min Max Contour Range
*/
    if (zClipMin > contourParams.conMin) contourParams.conMin = zClipMin;
    if (zClipMax < contourParams.conMax) contourParams.conMax = zClipMax;
    if (dbg) bcdtmWrite_message (0, 0, 0, "conMin = %10.4lf conMax = %10.4lf", contourParams.conMin, contourParams.conMax);
/*
**  Write Mark Time
*/
    if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Mark Lines = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),testTime) ) ;
   }
/*
** Build Contour Index For Loading Range Of Contours
*/
 if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,NULL,0,userP)) goto errexit ;
 if (contourParams.loadRange)
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Contour Index") ;
    testTime = bcdtmClock() ;
    if( tinLine2P != NULL ) memcpy(tinLine1P,tinLine2P,dtmP->numLines*sizeof(char)) ;
    if (bcdtmLoad_buildContourIndexDtmObject (dtmP, startPnt, lastPnt, contourParams.conMin, contourParams.conMax, contourParams.interval, contourParams.conReg, &tinLine1P, contourIndex, &numContourIndex)) goto errexit;
    if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Build Index = %7.3lf seconds ** Index = %p Index Size = %8ld",bcdtmClock_elapsedTime(bcdtmClock(),testTime),contourIndex,numContourIndex)  ;
   }

     {
     TerrainModel::DTMFenceParams noFence;
     TerrainModel::DTMFenceParamsCP fenceParamsP = useFence ? &fenceParams : &noFence;
     /*
     ** Load Contour Range
     */
     if (bcdtmLoad_callUserLoadFunction (loadFunctionP, DTMFeatureType::CheckStop, dtmP->nullUserTag, dtmP->nullFeatureId, NULL, 0, userP)) goto errexit;
     if (contourParams.loadRange)
         {
         if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Range Contours");
         testTime = bcdtmClock ();
         intOffset = (long)((contourParams.conMin - contourParams.conReg) / contourParams.interval);
         firstContour = contourParams.conReg + ((double)intOffset) * contourParams.interval;
         intOffset = (long)((contourParams.conMax - contourParams.conReg) / contourParams.interval) + 1;
         lastContour = contourParams.conReg + ((double)intOffset) * contourParams.interval;
         for (contourValue = firstContour, count = 0; contourValue <= lastContour; contourValue = contourValue + contourParams.interval, count++)
             {
             contourValue = bcdtmMath_roundToDecimalPoints (contourValue, 8);
             if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Contour %15.4lf", contourValue);
             if (tinLine2P != NULL) memcpy (tinLine1P, tinLine2P, dtmP->numLines*sizeof(char));
             else                    for (cP = tinLine1P; cP < tinLine1P + dtmP->numLines; ++cP) *cP = 0;
             if (bcdtmLoad_plotContourDtmObject (dtmP, contourParams, *fenceParamsP, pondAppData, clipDtmP, contourValue, tinLine1P, voidsInDtm, contourIndex, numContourIndex, &contourStartIndex, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
             if ((count & 0xff) == 0xff && bcdtmLoad_callUserLoadFunction (loadFunctionP, DTMFeatureType::CheckStop, dtmP->nullUserTag, dtmP->nullFeatureId, NULL, 0, userP)) goto errexit;
             }
         if (tdbg) bcdtmWrite_message (0, 0, 0, "Time To Range Contours = %7.3lf seconds", bcdtmClock_elapsedTime (bcdtmClock (), testTime));
         }
     /*
     ** Load Contours Values
     */
     if (contourParams.loadValues)
         {
         contourParams.interval = 0.0;
         if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Contour Values");
         for (dblP = contourParams.conValuesP, count = 0; dblP < contourParams.conValuesP + contourParams.numConValues; ++dblP, count++)
             {
             contourValue = bcdtmMath_roundToDecimalPoints (*dblP, 8);
             if (contourValue >= dtmP->zMin && contourValue <= dtmP->zMax)
                 {
                 if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Contour Value %10.5lf", contourValue);
                 if (tinLine2P != NULL) memcpy (tinLine1P, tinLine2P, dtmP->numLines*sizeof(char));
                 else                    for (cP = tinLine1P; cP < tinLine1P + dtmP->numLines; ++cP) *cP = 0;
                 if (bcdtmLoad_plotContourDtmObject (dtmP, contourParams, *fenceParamsP, pondAppData, clipDtmP, contourValue, tinLine1P, voidsInDtm, contourIndex, numContourIndex, &contourStartIndex, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
                 if ((count & 0xff) == 0xff && bcdtmLoad_callUserLoadFunction (loadFunctionP, DTMFeatureType::CheckStop, dtmP->nullUserTag, dtmP->nullFeatureId, NULL, 0, userP)) goto errexit;
                 }
             }
         }
      }
/*
** Clean Up
*/
 cleanup :
 if( useFence ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
 contourIndex.empty() ;
 if( tinLine1P      != NULL ) { free(tinLine1P) ; tinLine1P = NULL ; }
 if( tinLine2P      != NULL ) { free(tinLine2P) ; tinLine2P = NULL ; }
 if( clipDtmP       != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 numConPts = 0 ;
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Dtm Object Error") ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Contours = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;

// bcdtmWrite_message(0,0,0,"**** contour Interval      = %8.3lf",conInt) ;
// bcdtmWrite_message(0,0,0,"**** Time To Load Contours = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
// if( ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Dtm Object Completed") ;
// if( ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Dtm Object Error") ;

/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_plotContourDtmObject
(
 BC_DTM_OBJ *dtmP,                /* ==> Pointer to Dtm object              */
 TerrainModel::DTMContourParamsCR contourParams,  /* ==> Contour Params */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDtmP,
 double contourValue,             /* ==> Contour Value To Be Plotted        */
 unsigned char   *tinLineP,                /* ==> Marker Array For Marking Scanned Dtm Lines  */
 long   voidsInDtm,               /* ==> Voids Present<TRUE,FALSE>                   */
 DtmContourIndexArray& contourIndexP,  /* ==> Pointer To Contour Index                    */
 long   numContourIndex,          /* ==> Number Of Size Of Contour Index             */
 long   *contourStartLineP,       /* ==> Contour Index Start Scan                    */
 DTMFeatureCallback loadFunctionP,       /* ==> Pointer To Load Function                    */
 void   *userP                    /* ==> User Pointer Passed Back To User            */
)
/*
** This Routine Plots A Contour Value
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,ap,cl,listPtr,spnt,feature,contourStartOffset  ;
 long   startTime=0 ;
 double zMin,zMax ;
 BC_DTM_FEATURE  *fP ;
 DTM_TIN_POINT *p1P,*p2P ;
 long zsp1 = DTM_NULL_PNT, zsp2 = 0;
/*
** Initialise
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Plotting Contour %10.4lf",contourValue) ;
    startTime = bcdtmClock() ;
   }
/*
** Scan Dtm Hull for Non Closing Contours
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Dtm Hull For Contours") ;
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    if(  ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
      {
       if( ! bcdtmList_testForVoidOrHoleHullLineDtmObject(dtmP,p1,p2) )
         {
/*
**        Check For Zero Slope Triangle On Hull
*/
          if( contourValue == pointAddrP(dtmP,p1)->z && contourValue  == pointAddrP(dtmP,p2)->z )
            {
             if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             if( contourValue == pointAddrP(dtmP,ap)->z )
               {
                if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
                  {
                  zsp1 = p1;
                  zsp2 = p2;
                  }
               }
            }
/*
**        Check For Contour Start On Hull Line
*/
          else if (zsp1 != DTM_NULL_PNT)
              {
              if (bcdtmLoad_traceZeroSlopeContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, zsp1, zsp2, DTMDirection::AntiClockwise, 1, contourValue, tinLineP, loadFunctionP, userP)) goto errexit;
               zsp1 = DTM_NULL_PNT;
              }
          if( contourValue >= pointAddrP(dtmP,p1)->z && contourValue < pointAddrP(dtmP,p2)->z ||
              contourValue <= pointAddrP(dtmP,p1)->z && contourValue > pointAddrP(dtmP,p2)->z    )
            {
             if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
               {
                if( bcdtmLoad_traceContourDtmObject(dtmP,contourParams,fenceParams,pondExtendedAppData,clipDtmP,contourValue,p1,p2,DTMDirection::AntiClockwise,tinLineP,loadFunctionP, userP) != DTM_SUCCESS ) goto errexit  ;
               }
            }
         }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
   if (zsp1 != DTM_NULL_PNT)
      {
      if (bcdtmLoad_traceZeroSlopeContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, zsp1, zsp2, DTMDirection::AntiClockwise, 1, contourValue, tinLineP, loadFunctionP, userP)) goto errexit;
      }
/*
** Scan Void Hulls For Non Closing Contours
*/
 if( voidsInDtm == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Voids For Contours") ;
    for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
      {
       fP = ftableAddrP(dtmP,feature) ;
       if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
          if( ( fP->dtmFeatureType == DTMFeatureType::Void || fP->dtmFeatureType == DTMFeatureType::Hole || fP->dtmFeatureType == DTMFeatureType::Island ) )
            {
             spnt = p1 = p2 = ( long )fP->dtmFeaturePts.firstPoint ;
             listPtr = nodeAddrP(dtmP,p1)->fPtr ;
             while ( listPtr != dtmP->nullPtr )
               {
                while ( listPtr != dtmP->nullPtr  && flistAddrP(dtmP,listPtr)->dtmFeature != feature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
                if( listPtr != dtmP->nullPtr )
                  {
                   p2 = flistAddrP(dtmP,listPtr)->nextPnt ;
                   if( p2 != dtmP->nullPnt )
                     {
                      if(nodeAddrP(dtmP,p1)->hPtr != p2 )
                        {
                         if( contourValue >= pointAddrP(dtmP,p1)->z && contourValue < pointAddrP(dtmP,p2)->z ||
                             contourValue <= pointAddrP(dtmP,p1)->z && contourValue > pointAddrP(dtmP,p2)->z    )
                           {
                            if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
                              {
                              if (fP->dtmFeatureType == DTMFeatureType::Island) { if (bcdtmLoad_traceContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourValue, p1, p2, DTMDirection::AntiClockwise, tinLineP, loadFunctionP, userP)) goto errexit; }
                              else { if (bcdtmLoad_traceContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourValue, p1, p2, DTMDirection::Clockwise, tinLineP, loadFunctionP, userP)) goto errexit; }
                              }
                           }
                        }
                     }
                   listPtr   = nodeAddrP(dtmP,p1)->fPtr ;
                   p1 = p2 ;
                   if( p1 == dtmP->nullPnt || p1 == spnt ) listPtr = dtmP->nullPtr ;
                  }
               }
            }
         }
      }
   }
/*
** Scan Contour Index For Contours
*/
// contourPicker :
 if( numContourIndex > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Index For Contours") ;
    if( bcdtmLoad_getFirstTinLineForContourFromIndexDtmObject(dtmP,contourIndexP,numContourIndex,contourValue,contourStartLineP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"*contourStartLineP = %8ld",*contourStartLineP) ;
/*
**  Check Index Range
*/
    if( *contourStartLineP < numContourIndex )
      {
/*
**     Set Contour Index Start
*/
       contourStartOffset = *contourStartLineP ;
       DtmContourIndexArray::iterator index = contourIndexP.start() + *contourStartLineP ;
/*
**     Set Zmin And Zmax For Start Contour Index
*/
       p1P = pointAddrP(dtmP,index->p1) ;
       p2P = pointAddrP(dtmP,index->p2) ;
       if( p1P->z <= p2P->z ) { zMin = p1P->z ; zMax = p2P->z ; }
       else                   { zMax = p1P->z ; zMin = p2P->z ; }
/*
**     Write Debug Information
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Start Line Index For Contour %10.4lf ContourStartIndex = %6ld ** %10.4lf %10.4lf",contourValue,*contourStartLineP,zMin,zMax) ;
/*
**     Scan Contour Index
*/
       while ( contourStartOffset < numContourIndex && contourValue >= zMin )
         {
          if( contourValue <= zMax )
            {
             p2 = index->p2 ;
             double p2Z = pointAddrP(dtmP,p2)->z;
             if( contourValue != p2Z )
               {
                p1 = index->p1 ;
                if( ! bcdtmList_testForHullLineDtmObject(dtmP,p1,p2) &&
                    ! bcdtmList_testForHullLineDtmObject(dtmP,p2,p1)    )
                  {
                   if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Contour %10.4lf",contourValue) ;
                      if( nodeAddrP(dtmP,p1)->hPtr == p2 )
                        {
                        if (bcdtmLoad_traceContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourValue, p1, p2, DTMDirection::AntiClockwise, tinLineP, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
                        }
                      else
                        {
                        if (bcdtmLoad_traceContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourValue, p1, p2, DTMDirection::Clockwise, tinLineP, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
                        }
                     }
                  }
               }
            }
          ++contourStartOffset ;
/*
**        Check Index Range
*/
          if( contourStartOffset < numContourIndex )
            {
             ++index ;
/*
**           Set Zmin And Zmax For Next Contour Index
*/
             p1P = pointAddrP(dtmP,index->p1) ;
             p2P = pointAddrP(dtmP,index->p2) ;
             if( p1P->z <= p2P->z ) { zMin = p1P->z ; zMax = p2P->z ; }
             else                   { zMax = p1P->z ; zMin = p2P->z ; }
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Next Line Offset For %10.4lf ** %10.4lf %10.4lf",contourValue,zMin,zMax) ;
            }
         }
      }
   }
/*
** If No Contour Index Scan Dtm For Contours
*/
  else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Dtm For Contours") ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       cl = nodeAddrP(dtmP,p1)->cPtr ;
       while ( cl != dtmP->nullPtr )
         {
          p2 = clistAddrP(dtmP,cl)->pntNum ;
          if( ! bcdtmList_testForHullLineDtmObject(dtmP,p1,p2) &&
              ! bcdtmList_testForHullLineDtmObject(dtmP,p2,p1)    )
            {
             if( p2 > p1 )
               {
                if( contourValue >= pointAddrP(dtmP,p1)->z && contourValue < pointAddrP(dtmP,p2)->z ||
                    contourValue <= pointAddrP(dtmP,p1)->z && contourValue > pointAddrP(dtmP,p2)->z    )
                  {
                   if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
                     {
                     if (bcdtmLoad_traceContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourValue, p1, p2, DTMDirection::Clockwise, tinLineP, loadFunctionP, userP)) goto errexit;
                     }
                  }
               }
            }
          cl = clistAddrP(dtmP,cl)->nextPtr ;
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
 if( dbg && ret == DTM_SUCCESS)
   {
    bcdtmWrite_message(0,0,0,"Time To Plot %10.4lf Contour = %7.3lf seconds",contourValue,bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
    bcdtmWrite_message(0,0,0,"Plotting Contour %10.4lf Completed",contourValue) ;
   }
 if( dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Plotting Contour %10.4lf Error",contourValue) ;
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
BENTLEYDTM_Private int bcdtmLoad_traceContourDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer to Dtm object              */
 TerrainModel::DTMContourParamsCR contourParams,   /* ==> Contour Params */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData, /* ==> pond Data */
 BC_DTM_OBJ* clipDTMP,
 double contourValue,              /* ==> Contour Value To Be Plotted        */
 long   p1,                       /* ==> First Dtm Line Point               */
 long   p2,                        /* ==> Second Dtm Line Point              */
 DTMDirection   direction,                 /* ==> Trace Direction                    */
 unsigned char   *tinLineP,                /* ==> Marker Array For Marking Scanned Dtm Lines         */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function           */
 void   *userP                    /* ==> User Pointer Passed Back To User   */
)
/*
** This Routine Traces A Contour Across The Dtm Surface
**
** Contour Tracing Algorithm Notes :-
**
** Note 1 : test = 1 if P1->Z <= Cv < P2->Z
**          test = 2 if P1->Z >= Cv > P2->Z
** Note 2 : if direction == DTMDirection::AntiClockwise - Scan Anticlockwise About P1
**          if direction == DTMDirection::Clockwise     - Scan Clockwise About P1
** Note 3 : Contour Tracing Does Two Scans Through Dtm
**          First Scan Traces To End Of Contour Without Drawing Contour
**          Second Scan Traces From Contour End Backwards Drawing Contour
** Note 4 : During Scanning Only P1->z Can Be Equal To The Contour Value
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   test = 1, scan, contourScanned = FALSE, zeroSlopeLine, zeroSlopeTriangle;
 long contourDirection = 0;
 long   weight=0,p3,sp1,sp2,lp2,lp1,llp1 ;
 double ra,zp1 = 0.0,zp2 = 0.0,lzp1,xc,yc,xlc=0.0,ylc=0.0 ;
 static long conSeq=0 ;
/*
** Write Entry Message
*/
 ++conSeq ;
 if( conSeq == 0 ) dbg=DTM_TRACE_VALUE(0) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour %10.4lf ** Direction = %1ld",contourValue,direction) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
   }
/*
** Set Contour Test Setting - See Above
*/
 test = 0 ;
 if     ( contourValue >= pointAddrP(dtmP,p1)->z && contourValue < pointAddrP(dtmP,p2)->z ) test= 1 ;
 else if( contourValue <= pointAddrP(dtmP,p1)->z && contourValue > pointAddrP(dtmP,p2)->z ) test= 2 ;
 if( test == 0 )
   {
    bcdtmWrite_message(2,0,0,"Contour Test Error") ;
    goto errexit ;
   }
/*
** Set Contour Direction To Be Passed As Last Vertice Of Contour
** Use Contour Direction For Placing Contour Labels In Increasing
** Or Decreasing Elevation Direction
** Rob. Cormack   23/4/93
*/
 contourDirection = 0;
 if      ( test == 1 && direction == DTMDirection::AntiClockwise ) contourDirection = 0 ;
 else  if( test == 1 && direction == DTMDirection::Clockwise     ) contourDirection = 1 ;
 else  if( test == 2 && direction == DTMDirection::AntiClockwise ) contourDirection = 1 ;
 else  if( test == 2 && direction == DTMDirection::Clockwise     ) contourDirection = 0 ;
/*
** Scan Through Dtm Two Times
** Firstly Scan To End Of Contour
** Secondly Scan Back Along Contour While Drawing It
*/
 for( scan = 0 ; scan < 2 ; ++scan )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"scan = %ld ** p1 = %6ld p2 = %6ld",scan,p1,p2) ;
    sp1 = p1 ;
    sp2 = p2 ;
    contourScanned = FALSE ;
    llp1 = lp1 = dtmP->nullPnt ;
    if( direction == DTMDirection::Clockwise && nodeAddrP(dtmP,p1)->hPtr == p2 ) contourScanned = TRUE ;
/*
** Draw First Contour Point
*/
    if( scan && contourScanned == FALSE )
      {
       ra = (contourValue-pointAddrP(dtmP,p1)->z)/(pointAddrP(dtmP,p2)->z - pointAddrP(dtmP,p1)->z) ;
       xc = pointAddrP(dtmP,p1)->x + ( pointAddrP(dtmP,p2)->x - pointAddrP(dtmP,p1)->x ) * ra ;
       yc = pointAddrP(dtmP,p1)->y + ( pointAddrP(dtmP,p2)->y - pointAddrP(dtmP,p1)->y ) * ra ;
       weight = 0 ;
       if      ( ra == 0.0 ) weight = 1 ;
       else if ( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) weight = 2 ;
       if( bcdtmLoad_storeContourPoint(1,xc,yc,contourValue,weight)) goto errexit  ;
       xlc = xc ; ylc = yc  ;
       bcdtmLoad_setLineDtmObject(dtmP,p1,p2,tinLineP) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Start Contour Point = %10.4lf %10.4lf %10.4lf",xc,yc,contourValue) ;
      }
/*
**  Get Next Dtm Point Around P1
*/
    while ( contourScanned == FALSE )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Scan = %1ld Direction = %2ld",scan,direction) ;
          bcdtmWrite_message(0,0,0,"*LP1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
          bcdtmWrite_message(0,0,0,"*LP2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
         }
       if( lp1 != p1 ) llp1 = lp1 ;
       lp1  = p1 ;
       lp2  = p2 ;
       lzp1 = pointAddrP(dtmP,p1)->z ;
       if( direction == DTMDirection::Clockwise) { if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
       else                            { if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"**P1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
          bcdtmWrite_message(0,0,0,"**P2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
         }
/*
**     Check For Termination On Steep Triangle
*/
       double ascentAngle,descentAngle,triangleSlope=0.0 ;
       if (contourParams.maxSlopeOption)
         {
          if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,lp1,lp2,p2,&descentAngle,&ascentAngle,&triangleSlope)) goto errexit ;
         }
       if (contourParams.maxSlopeOption && triangleSlope >= contourParams.maxSlopeValue)
         {
          contourScanned = TRUE ;
          p2 = lp2 ;
         }
       else
         {
/*
**     Check For Switching P1 and P2
*/
       zp1 = pointAddrP(dtmP,p1)->z ;
       zp2 = pointAddrP(dtmP,p2)->z ;
       if( ( test == 1 && contourValue >= zp2 ) || ( test == 2 && contourValue <= zp2 ) )
         {
/*
**        Check For Contour Ridge Or Sump
*/
          if( p2 == llp1 && pointAddrP(dtmP,p2)->z == contourValue )
            {
             p2 = lp2 ;
             sp1 = p1 ;
             sp2 = p2 ;
             contourScanned = TRUE ;
             if( dbg && contourScanned ) bcdtmWrite_message(0,0,0,"Contour Terminated At Loop Back") ;
            }
/*
**        Switch P1 And P2 To Maintain Scan Criteria Of P1->z == Contour Value
*/
          else
            {
             p1 = p2  ;
             p2 = lp2 ;
             zp1 = pointAddrP(dtmP,p1)->z ;
             zp2 = pointAddrP(dtmP,p2)->z ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Switched P1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Switched P2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
             if( zp2 == contourValue )
               {
                bcdtmWrite_message(2,0,0,"Invalid Value For Contour P2 ** P2 = %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
                goto errexit ;
               }
            }
         }
/*
**    Check For Termination On An Already Drawn Line
*/
       if( p1 != sp1 || p2 != sp2 )
         {
          if( bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP)) contourScanned = TRUE ;
          else if( lzp1 == contourValue && zp1 == contourValue && lp1 != p1 && bcdtmLoad_testLineDtmObject(dtmP,p1,lp1,tinLineP)) contourScanned = TRUE ;
          if( contourScanned == TRUE && ! scan )
            {
             p1 = lp1 ;
             p2 = lp2 ;
            }
         }
         }
/*
**    Continue Contour Scan
*/
       if( contourScanned == FALSE )
         {

/*
**        Check For A Zero Slope Line
*/
          zeroSlopeLine = 0 ;
          zeroSlopeTriangle = 0 ;
          if( lp1 != p1 && lzp1 == contourValue && zp1 == contourValue ) zeroSlopeLine = 1 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Line = %2ld",zeroSlopeLine) ;
/*
**        Test For For Zero Slope Triangle
*/
          if( zeroSlopeLine )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Zero Slope Triangle") ;
             if(( p3 = bcdtmList_nextClkDtmObject(dtmP,lp1,p1)) < 0 ) goto errexit  ;
             if( bcdtmList_testLineDtmObject(dtmP,p1,p3) && pointAddrP(dtmP,p3)->z == contourValue ) zeroSlopeTriangle = 1 ;
             else
               {
                if(( p3 = bcdtmList_nextAntDtmObject(dtmP,lp1,p1)) < 0 ) goto errexit  ;
                if( bcdtmList_testLineDtmObject(dtmP,p1,p3) && pointAddrP(dtmP,p3)->z == contourValue ) zeroSlopeTriangle = 1 ;
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle = %2ld",zeroSlopeTriangle) ;
            }
/*
**       If Zero Slope Triangle Trace Contour Around Triangle Edges
*/
         if( zeroSlopeTriangle )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle = %2ld Scan = %1ld test = %2ld",zeroSlopeTriangle,scan,test) ;
            if (scan)  if (bcdtmLoad_contourFeature (dtmP, contourParams, fenceParams, pondExtendedAppData,clipDTMP, contourDirection, loadFunctionP, userP)) goto errexit;
            if (bcdtmLoad_traceZeroSlopeContourDtmObject (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDTMP, lp1, p1, direction, contourDirection, contourValue, tinLineP, loadFunctionP, userP)) goto errexit;
/*
**          Set For Reverse Scan Along Contour
*/
            contourScanned = TRUE ;
            p1 = lp1 ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated On Zero Slope Triangle") ;
            if( bcdtmList_testForTinHullLineDtmObject(dtmP,p1,p2) || bcdtmList_testForTinHullLineDtmObject(dtmP,p2,p1) ) scan = 2 ;
           }
/*
**       Calculate Intersection Point
*/
         else
           {
            if( scan )
              {
               if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Intersection Point") ;
               ra = ( contourValue - zp1 ) / ( zp2 - zp1 ) ;
               xc = pointAddrP(dtmP,p1)->x + ( pointAddrP(dtmP,p2)->x - pointAddrP(dtmP,p1)->x ) * ra ;
               yc = pointAddrP(dtmP,p1)->y + ( pointAddrP(dtmP,p2)->y - pointAddrP(dtmP,p1)->y ) * ra ;
               if( dbg ) bcdtmWrite_message(0,0,0,"Next  Contour Point = %10.4lf %10.4lf %10.4lf",xc,yc,contourValue) ;
               if( xc != xlc || yc != ylc )
                 {
                  weight = 0 ;
                  if      ( ra == 0.0 ) weight = 1 ;
                  else if ( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) weight = 2 ;
                  if( bcdtmLoad_storeContourPoint(1,xc,yc,contourValue,weight)) goto errexit  ;
                  xlc = xc ; ylc = yc ;
                 }
               bcdtmLoad_setLineDtmObject(dtmP,p1,p2,tinLineP) ;
               if( zeroSlopeLine ) bcdtmLoad_setLineDtmObject(dtmP,lp1,p1,tinLineP) ;
              }
/*
**           Check for Termination of closed contour
*/
             if( sp1 == p1 && sp2 == p2 )
               {
                contourScanned = TRUE ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated At Start Point") ;
               }
/*
**           Check For Temination On Hull Line
*/
             else if( bcdtmList_testForHullLineDtmObject(dtmP,p1,p2) || bcdtmList_testForHullLineDtmObject(dtmP,p2,p1) )
               {
                contourScanned = TRUE ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated On Hull Line") ;
               }
            }
         }
      }
/*
**  Reverse Contour Scan Direction
*/
    if( direction == DTMDirection::Clockwise ) direction = DTMDirection::AntiClockwise ;
    else                             direction = DTMDirection::Clockwise ;
   }
/*
**  Load Contour Feature
*/
   if (bcdtmLoad_contourFeature (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDTMP, contourDirection, loadFunctionP, userP)) goto errexit;
/*
** Clean Up
*/
 cleanup :
 if( bcdtmLoad_storeContourPoint(0,0.0,0.0,0.0,0)) goto errexit  ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour %10.4lf Completed",contourValue) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour %10.4lf Error",contourValue) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto  cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_traceZeroSlopeContourDtmObject
(
 BC_DTM_OBJ *dtmP,                /* ==> Pointer To Dtm Object              */
 TerrainModel::DTMContourParamsCR contourParams,   /* ==> Contour Params */
 TerrainModel::DTMFenceParamsCR fenceParams,   /* ==> Fence Params */
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDtmP,
 long   p1,                        /* ==> First Point Of Dtm Line            */
 long   p2,                        /* ==> Second Point Of Dtm Line           */
 DTMDirection direction,                /* ==> Contour Scan Direction             */
 long   contourDirection,          /* ==> Contour Direction Used For Increasing Or Decreasing Slope Labelling Of Contours */
 double contourValue,              /* ==> Contour Value Being Traced         */
 unsigned char   *tinLineP,                /* ==> Marker Array For Marking Scanned Dtm Lines         */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function           */
 void   *userP                    /* ==> User Pointer Passed Back To User   */
)
/*
** This routine places contours around Zero Slope Triangles
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lp,np,p3,voidLine ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Zero Slope Contour ** P1 = %6ld P2 = %6ld Cv = %10.4lf",p1,p2,contourValue) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
    bcdtmWrite_message(0,0,0,"direction         = %8ld",direction) ;
    bcdtmWrite_message(0,0,0,"contourDirection  = %8ld",contourDirection) ;
    bcdtmWrite_message(0,0,0,"Test Line P1-P2   = %8ld",bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP)) ;
   }
/*
** Only Process If Contour Line Has Not Been Previously Drawn
*/
 if( ! bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
   {
/*
** Only Process Non Void Lines
*/
    if( bcdtmList_testForVoidLineDtmObject (dtmP,p1,p2,&voidLine)) goto errexit  ;
    if( ! voidLine )
      {
/*
** Write First Two Points
*/
       if( bcdtmLoad_storeContourPoint(1,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,1)) goto errexit  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** P2 = %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
       if( bcdtmLoad_storeContourPoint(1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,1)) goto errexit  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** P1 = %6ld ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
       bcdtmLoad_setLineDtmObject(dtmP,p1,p2,tinLineP) ;
/*
**     Get Next Point on Zero Slope Triangle
*/
       lp = p2 ;
       while ( p1 != lp && ! voidLine )
         {
          np = p2 ;
          if( direction == DTMDirection::Clockwise ) { if((p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
          else                             { if((p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2))   < 0 ) goto errexit  ; }

          while( pointAddrP(dtmP,p3)->z == contourValue && bcdtmList_testForValidTriangleDtmObject(dtmP,p1,p2,p3) && p3 != np )
            {
             p2 = p3 ;
             if( direction == DTMDirection::Clockwise ) { if((p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
             else                             { if((p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2))   < 0 ) goto errexit  ; }

             // If the triangle is a void line then exit the loop.
             if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p3,&voidLine)) goto errexit  ;
             if (voidLine != 0)
                 break;
            }
          if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit  ;
          if( bcdtmLoad_testLineDtmObject(dtmP,p1,p2,tinLineP) )
              break;
          if( ! voidLine )
            {
             if( bcdtmLoad_storeContourPoint(1,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,1)) goto errexit  ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** P2 = %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
             bcdtmLoad_setLineDtmObject(dtmP,p1,p2,tinLineP) ;
             p3 = p2 ; p2 = p1 ; p1 = p3 ;
            }
         }
/*
**     Load Contour Feature
*/
       if (bcdtmLoad_contourFeature (dtmP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourDirection, loadFunctionP, userP)) goto errexit;
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
 numConPts = 0 ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_storeContourPoint
(
 long   storeOption,             /* ==> Store Option                    */
 double x,                       /* ==> x Coordinate Contour Point      */
 double y,                       /* ==> y Coordinate Contour Point      */
 double z,                       /* ==> z Coordinate Contour Point      */
 long   weight                   /* ==> Contour Point Weight            */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** This Function Stores A Contour Point On The Heap
**
** storeFlag  ==  0   Clear Load Points
**            ==  1   Store Load Point
*/
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Contour Point[%8ld] ** S = %2ld W = %2ld ** %12.5lf %12.5lf %10.4lf",numConPts,storeOption,weight,x,y,z) ;
/*
** Store Points If Not Clearing Memory
*/
 if( ! storeOption  ) numConPts = 0 ;
 else
   {
/*
** Check For Sufficient Heap Space
*/
    if( numConPts == memConPts )
      {
       memConPts = memConPts + memConPtsInc ;
       if( conPtsP == NULL )  conPtsP  = ( DPoint3d  * ) malloc  ( memConPts * sizeof(DPoint3d)) ;
       else                   conPtsP  = ( DPoint3d  * ) realloc ( conPtsP , memConPts * sizeof(DPoint3d)) ;
       if( conWghtP == NULL ) conWghtP = ( long * ) malloc  ( memConPts * sizeof(long)) ;
       else                   conWghtP = ( long * ) realloc ( conWghtP , memConPts * sizeof(long)) ;
       if( conPtsP == NULL || conWghtP == NULL )
         {
          bcdtmLoad_freeContourMemory() ;
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit  ;
         }
      }
/*
** Store Point
*/
    (conPtsP+numConPts)->x = x ;
    (conPtsP+numConPts)->y = y ;
    (conPtsP+numConPts)->z = z ;
    *(conWghtP+numConPts) = weight ;
    ++numConPts ;
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
 numConPts = 0 ;
 goto cleanup   ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_contourFeature
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object                                          */
 TerrainModel::DTMContourParamsCR contourParams,   /* ==> Contour Params                                                 */
 TerrainModel::DTMFenceParamsCR fenceParams,   /* ==> Fence Params                                                 */
 DTMPondAppData* pondExtendedAppData, /* ==> PondData                                              */
 BC_DTM_OBJ *clipDtmP, /* ==> ClipDTM*/
 long   contourDirection,          /* ==> Contour Direction                                              */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function                                       */
 void   *userP                   /* ==> User Pointer Passed Back To User                               */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,*lP,clipResult,numClipArrays,conType ;
 DPoint3d  *p3dP ;
 DTMFeatureId depressionId=DTM_NULL_FEATURE_ID ;
 DTM_POINT_ARRAY **clipArraysPP=NULL ;
/*
** Write Contour Points - Development Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading Contour %10.4lf ** Number Of Contour Points = %8ld",conPtsP->z,numConPts) ;
    if( dbg == 2 )
      {
       for( p3dP = conPtsP , lP = conWghtP ; p3dP < conPtsP + numConPts ; ++p3dP , ++lP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** W = %2ld  %12.5lf %12.5lf %10.4lf",(long)(p3dP-conPtsP),*lP,p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
**  Process Contour Only If More Than Two Points ( 2 vertices + direction )
*/
 if( numConPts >= 2 )
   {
/*
**  Remove Duplicate Contour Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Contour Points") ;
    bcdtmLoad_removeDuplicateContourPoints() ;
/*
**  Process Contour Only If More Than Two Points ( 2 vertices + direction )
*/
    if( numConPts > 1 )
      {
/*
**     Check For Contours Within A Depression
*/
       if( pondExtendedAppData != NULL )
         {
          if(pondExtendedAppData->pondDtmP != NULL )
            {
            if( bcdtmLoad_checkForDepressionContourDtmObject(pondExtendedAppData->pondDtmP,conPtsP,numConPts,1.0,&conType)) goto errexit ;
            depressionId = (DTMFeatureId) conType ;
            }
         }
/*
**     Smooth Contours
*/
       if( contourParams.smoothOption != DTMContourSmoothing::None && numConPts > 2 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Smoothing Contour") ;
          if (bcdtmLoad_smoothContour (dtmP, contourParams.realInterval != 0 ? contourParams.realInterval : contourParams.interval, contourParams.smoothOption, contourParams.smoothFactor, contourParams.smoothDensity)) goto errexit;
         }
/*
**     Load Contour
*/
       if (fenceParams.fenceType == DTMFenceType::None)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Calling User Load Function %p",loadFunctionP) ;
          if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Contour,(DTMUserTag)contourDirection,depressionId,conPtsP,numConPts,userP)) goto errexit ;
         }
/*
**     Check If Contour Lies In Fence
*/
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Cliping Contour To Fence") ;
          if (bcdtmClip_featurePointArrayToTinHullDtmObject (clipDtmP, fenceParams.fenceOption, conPtsP, numConPts, &clipResult, &clipArraysPP, &numClipArrays)) goto errexit;
          if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Contour,(DTMUserTag)contourDirection,depressionId,conPtsP,numConPts,userP)) goto errexit ;
          if( clipResult == 2 )
            {
             for( n = 0 ; n < numClipArrays ; ++n )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Calling User Load Function") ;
                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Contour,(DTMUserTag)contourDirection,depressionId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
               }
             bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 numConPts = 0 ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Loading Contour %10.4lf Completed",conPtsP->z) ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Loading Contour %10.4lf Error",conPtsP->z) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup   ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLoad_smoothContour
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object Needed For Bspline Smmoting     */
 double conInt,                  /* ==> Contour Interval. Neede for Bspline Smoothing         */
 DTMContourSmoothing smoothOption, /* ==> Smoothing Option <1,2,3>                              */
 double smoothFactor,            /* ==> Smoothing Factor - Depends On Smoothing Option        */
 long  smoothDensity             /* ==> Bspline Smoothing Densification                       */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numTmpConPts ;
 double filterTolerance=0.001 ;
 BC_DTM_OBJ *smoothDtmP=NULL ;
 DPoint3d    *p3dP,*tmpConPtsP=NULL ;
 long   *w1P,*w2P,*tmpWghtP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Smoothing Contour") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"conInt        = %8.4lf",conInt) ;
    bcdtmWrite_message(0,0,0,"smoothOption  = %8ld",smoothOption) ;
    bcdtmWrite_message(0,0,0,"smoothFactor  = %8.3lf",smoothFactor) ;
    bcdtmWrite_message(0,0,0,"smoothDensity = %8ld",smoothDensity) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Contour Points = %6ld",numConPts) ;
       for( p3dP = conPtsP, w1P = conWghtP ; p3dP < conPtsP + numConPts ; ++p3dP , ++w1P )
         {
          bcdtmWrite_message(0,0,0,"contourPoint[%5ld] = %12.4lf %12.4lf %10.4lf ** %2ld",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,p3dP->z,*w1P) ;
         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Smoothing Contour ** smoothOption = %2ld ** smoothFactor = %8.2lf",smoothOption,smoothFactor) ;
/*
** Set Filter Tolerance
*/
 if( dtmP != NULL ) filterTolerance = dtmP->ppTol * 10.0 ;
 if( filterTolerance >= 0.1 ) filterTolerance = 0.01 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Filter Tolerance = %12.8lf",filterTolerance) ;
/*
** Only Process For More Than Two Contour Points
*/
 if (numConPts > 3 && smoothOption >= DTMContourSmoothing::Vertex && smoothOption <= DTMContourSmoothing::SplineWithoutOverLapDetection)
   {
/*
**  Copy Contour Points And Weights
*/
    numTmpConPts = numConPts ;
    if( bcdtmUtl_copy3DTo3D(conPtsP,numConPts,&tmpConPtsP)) goto errexit ;
    tmpWghtP = ( long * ) malloc(numConPts*sizeof(long)) ;
    if( tmpWghtP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( w1P = tmpWghtP , w2P = conWghtP ; w2P < conWghtP + numConPts ; ++w1P,++w2P) *w1P = *w2P ;
/*
**  Filter Contour Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points Before Filter = %8ld",numConPts) ;
    if( bcdtmLoad_filterWeightedPointArray(conPtsP,&numConPts,conWghtP,filterTolerance)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points After  Filter = %8ld",numConPts) ;
/*
** Vertex And Five Point Smooth Contour Line
*/
    if (smoothOption == DTMContourSmoothing::Vertex)
      {
       if( smoothFactor < 0.1 || smoothFactor > 0.5 ) smoothFactor = 0.3 ;// 0.1 to 0.5 For Linear Smoothing
//       if( bcdtmLoad_vertexSmoothContourLine(&conPtsP,&numConPts,&conWghtP,smoothFactor)) goto errexit ;
//       if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points After Vertex Smooth  = %8ld",numConPts) ;
/*
**     Five Point Smooth Contour Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points Before Five Point Smooth = %8ld",numTmpConPts)   ;
       if( bcdtmLoad_fivePointSmoothContour(&tmpConPtsP,&numTmpConPts,&tmpWghtP,smoothFactor)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points After 5 Point Smooth = %8ld",numConPts) ;
      }
/*
**  Spline Smooth Contour Line
*/
    else if (smoothOption == DTMContourSmoothing::Spline || smoothOption == DTMContourSmoothing::SplineWithoutOverLapDetection)
      {
       smoothDtmP   = dtmP ;
       if( smoothDensity < 3  || smoothDensity > 10 ) smoothDensity = 5  ;
       if( smoothFactor < 0.0 || smoothFactor > 5.0 ) smoothFactor = 2.5 ; // 0.0 to 5.0 For Bspline Smoothing
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points Before Spline Smooth = %8ld",numTmpConPts)   ;
       if( bcdtmLoad_splineSmoothContour(dtmP,conInt,&tmpConPtsP,&numTmpConPts,&tmpWghtP,smoothOption,smoothFactor,smoothDensity)) goto errexit      ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points After  Spline Smooth = %8ld",numTmpConPts)   ;
      }
/*
**  Filter Contour Points
*/
    if( bcdtmLoad_filterWeightedPointArray(tmpConPtsP,&numTmpConPts,tmpWghtP,filterTolerance)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Contour Points After Filter = %8ld",numTmpConPts)    ;
/*
**  Store Smoothed Contour In Contour Points Buffer
*/
    numConPts = 0 ;
    for( p3dP = tmpConPtsP , w1P = tmpWghtP ; p3dP < tmpConPtsP + numTmpConPts ; ++p3dP , ++w1P )
      {
       if( numConPts == memConPts )
         {
          memConPts = memConPts + memConPtsInc ;
          conPtsP   = ( DPoint3d  * ) realloc ( conPtsP  , memConPts * sizeof(DPoint3d)) ;
          conWghtP  = ( long * ) realloc ( conWghtP , memConPts * sizeof(long)) ;
          if( conPtsP == NULL || conWghtP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit  ;
            }
         }
       (conPtsP+numConPts)->x = p3dP->x ;
       (conPtsP+numConPts)->y = p3dP->y ;
       (conPtsP+numConPts)->z = p3dP->z ;
       *(conWghtP+numConPts)  = *w1P ;
       ++numConPts ;
      }
   }
 if( dbg && numConPts > 2000 ) bcdtmWrite_message(0,0,0,"DTM_ERROR Number Contour Points = %8ld",numConPts)    ;
/*
** Clean Up
*/
 cleanup :
 if( tmpConPtsP != NULL ) { free(tmpConPtsP) ; tmpConPtsP = NULL ; }
 if( tmpWghtP   != NULL ) { free(tmpWghtP)   ; tmpWghtP   = NULL ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Smoothing Contour Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Smoothing Contour Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmLoad_freeContourMemory() ;
 goto cleanup   ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_densifyWeightedPointArray
(
 BC_DTM_OBJ *dtmP,             /* ==> Pointer To Dtm Object            */
 DPoint3d        **pointsPP,        /* ==> Pointer To Point Array           */
 long       *numPtsP,          /* ==> Number Of Points In Point Array  */
 long       **wghtPtsPP,       /* ==> Pointer To Point Weight Array    */
 double     tolerance          /* ==> Densify Tolerance                */
)
/*
** This function filters a point array. Each point has an associated
** weight. Only Points with a weighting of zero can be filtered
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    *weightP,*denWeightP=NULL ;
 long    n,numPoints=0,memPoints=0,memPointsInc=10000,numIncrements ;
 DPoint3d     *denPointsP=NULL ;
 double  delta,distance;
 DPoint3d     *p3d1P,*p3d2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Densifying Weighted Points") ;
    bcdtmWrite_message(0,0,0,"*pointsPP  = %p",*pointsPP) ;
    bcdtmWrite_message(0,0,0,"*numPtsP   = %8ld",*numPtsP) ;
    bcdtmWrite_message(0,0,0,"*wghtPtsPP = %p",*wghtPtsPP) ;
    bcdtmWrite_message(0,0,0,"tolerance  = %8.4lf",tolerance) ;
   }
/*
** Calculate Tolerance
*/
tolerance = sqrt( dtmP->xRange * dtmP->xRange + dtmP->yRange * dtmP->yRange ) /1000.0  ;
/*
** Create Memory Arrays For The Densified Points
*/
 memPoints = memPointsInc ;
 denWeightP = ( long * ) malloc( memPoints * sizeof(long)) ;
 denPointsP = ( DPoint3d  * ) malloc( memPoints * sizeof(DPoint3d)) ;
 if( denWeightP == NULL || denPointsP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Variables
*/
 for( p3d1P = *pointsPP , weightP = *wghtPtsPP ; p3d1P < *pointsPP + *numPtsP ; ++p3d1P , ++weightP )
   {
/*
**  Check Memory
*/
    if( numPoints >= memPoints )
      {
       memPoints = memPoints + memPointsInc ;
       denWeightP = ( long * ) realloc( denWeightP , memPoints * sizeof(long)) ;
       denPointsP = ( DPoint3d  * ) realloc( denPointsP , memPoints * sizeof(DPoint3d)) ;
       if( denWeightP == NULL || denPointsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store Point
*/
    *(denWeightP+numPoints) = *weightP ;
    *(denPointsP+numPoints) = *p3d1P ;
    ++numPoints ;
/*
**  Densify Segment
*/
    p3d2P = p3d1P + 1 ;
    if( p3d2P < *pointsPP + *numPtsP )
      {
       distance = sqrt( ( p3d2P->x-p3d1P->x) * ( p3d2P->x-p3d1P->x) + ( p3d2P->y-p3d1P->y) * ( p3d2P->y-p3d1P->y) ) ;
       if( tolerance < distance )
         {
          numIncrements = (long) ( distance / tolerance ) + 1 ;
          delta = distance / ( double ) ( numIncrements ) ;
// bcdtmWrite_message(0,0,0,"numIncrements = %4ld distance = %10.4lf delta = %10.4lf ** tolerance = %10.4lf",numIncrements,distance,delta,tolerance) ;
          for( n = 1 ; n < numIncrements ; ++n )
            {
/*
**           Check Memory
*/
             if( numPoints >= memPoints )
               {
                memPoints = memPoints + memPointsInc ;
                denWeightP = ( long * ) realloc( denWeightP , memPoints * sizeof(long)) ;
                denPointsP = ( DPoint3d  * ) realloc( denPointsP , memPoints * sizeof(DPoint3d)) ;
                if( denWeightP == NULL || denPointsP == NULL )
                  {
                   bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                   goto errexit ;
                  }
               }
/*
**           Store Densified Point
*/
             *(denWeightP+numPoints) = 0 ;
             (denPointsP+numPoints)->x  = p3d1P->x + ( p3d2P->x - p3d1P->x ) * ((double)(n) * delta ) / distance ;
             (denPointsP+numPoints)->y  = p3d1P->y + ( p3d2P->y - p3d1P->y ) * ((double)(n) * delta ) / distance ;
             (denPointsP+numPoints)->z  = p3d1P->z + ( p3d2P->z - p3d1P->z ) * ((double)(n) * delta ) / distance ;
             ++numPoints ;
            }
         }
      }
   }
/*
** Realloc If Necessary
*/
 if( numPoints < memPoints )
   {
    denWeightP = ( long * ) realloc( denWeightP , numPoints * sizeof(long)) ;
    denPointsP = ( DPoint3d  * ) realloc( denPointsP , numPoints * sizeof(DPoint3d)) ;
   }
/*
** Set Return Arrays
*/
 *numPtsP = numPoints ;
 free( *pointsPP ) ;
 *pointsPP  = denPointsP ;
 denPointsP = NULL ;
 free( *wghtPtsPP ) ;
 *wghtPtsPP = denWeightP ;
 denWeightP = NULL ;
/*
** Clean Up
*/
 cleanup :
 if( denWeightP != NULL ) free(denWeightP) ;
 if( denPointsP != NULL ) free(denPointsP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Densifying Weighted Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Densifying Weighted Points Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_filterWeightedPointArray
(
 DPoint3d    *pointsP,        /* ==> Pointer To Point Array           */
 long   *numPtsP,        /* ==> Number Of Points In Point Array  */
 long   *wghtPtsP,       /* ==> Pointer To Point Weight Array    */
 double tolerance        /* ==> Normal To Chord Filter Tolerance */
)
/*
** This function filters a point array. Each point has an associated
** weight. Only Points with a weighting of zero can be filtered
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    *filterPtsP=NULL,*lP,*w1P,*w2P ;
 double  delta,d,dx,dy,x,y,x1,y1,x2,y2,a1,a2,a3,r;
 DPoint3d     *p3d1P,*p3d2P,*p3d3P,*p3dtP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Weighted Points") ;
/*
** Initialise Variables
*/
 delta = fabs(tolerance) ;
 p3d1P = pointsP  ;
 p3d2P = pointsP + 2 ;
 p3dtP = pointsP + *numPtsP - 1 ;
/*
** Allocate Memory For Filter Point Marker
*/
 filterPtsP = (long *) malloc( *numPtsP * sizeof(long)) ;
 if( filterPtsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Mark All Points As Not Filtered
*/
 for( lP = filterPtsP ; lP < filterPtsP + *numPtsP ; ++lP ) *lP = 1 ;
/*
** Filter linear feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Points") ;
 while ( p3d2P <= p3dtP )
   {
    x1 = p3d1P->x ; y1 = p3d1P->y ;
    x2 = p3d2P->x ; y2 = p3d2P->y ;
    dx = x2 - x1 ; dy = y2 - y1 ;
    r = sqrt( dx*dx + dy*dy)    ;
    if( r > 0.0 )
      {
       a1 = dy / r ; a2 = -dx / r ; a3 = -a1 * x1 - a2 * y1 ;
       for( p3d3P = p3d1P + 1 ; p3d3P < p3d2P ; ++p3d3P )
         {
          x = p3d3P->x ; y = p3d3P->y ;
          d = a1 * x + a2 * y + a3  ;
          if( fabs(d) > delta )
            {
             for( lP = filterPtsP +(long)(p3d1P-pointsP) + 1 ; lP < filterPtsP +(long)(p3d2P-pointsP) - 1  ; ++lP ) *lP = 0 ;
             p3d1P = p3d2P - 1 ;
             p3d3P = p3d2P ;
            }
         }
      }
    ++p3d2P ;
   }
/*
** Null Out To Last Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out To Last Point") ;
 for( lP = filterPtsP + (long)(p3d1P-pointsP) + 1 ; lP < filterPtsP +(long)(p3d2P-pointsP) - 1 ; ++lP ) *lP = 0 ;
/*
** Ensure Weighted Points Have Not Been Filtered
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Ensuring Weighted Points Have Not Been Filtered") ;
 for( w1P = wghtPtsP , lP = filterPtsP ; w1P < wghtPtsP + *numPtsP ; ++w1P , ++lP ) if ( *w1P ) *lP = 1 ;
/*
** Remove Filtered Vertices
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Filtered Points") ;
 lP   = filterPtsP ;
 w1P  = w2P  = wghtPtsP ;
 p3d1P = p3d2P = pointsP ;
 while ( p3d2P <= p3dtP )
   {
    if( *lP )
      {
       if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
       ++p3d1P ;
       if( w1P  != w2P  ) *w1P  = *w2P  ;
       ++w1P  ;
      }
    ++p3d2P ;
    ++w2P  ;
    ++lP ;
   }
 *numPtsP = (long)(p3d1P-pointsP) ;
/*
** Clean Up
*/
 cleanup :
 if( filterPtsP != NULL ) free(filterPtsP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Weighted Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Weighted Points Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_vertexSmoothContour(DPoint3d **conPtsPP,long *numConPtsP,long **weightPP)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long closeFlag=0,*pointTypeP=NULL,*typeP,*wghtP ;
 double Xm,Ym,ratio=0.3333  ;
 DPoint3d    *p3d1P,*p3d2P,*p3d3P,*p3d4P,*smoothPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Vertex Smoothing Contour") ;
/*
** Allocate Memory For Smoothed Points
*/
 smoothPtsP  = ( DPoint3d *  ) malloc (*numConPtsP * sizeof(DPoint3d)) ;
 pointTypeP  = ( long * ) malloc (*numConPtsP * sizeof(long)) ;
 if( smoothPtsP == NULL || pointTypeP == NULL)
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Vertex Points Array
*/
 if( (*conPtsPP)->x == (*conPtsPP+*numConPtsP-1)->x &&  (*conPtsPP)->y == (*conPtsPP+*numConPtsP-1)->y ) closeFlag = 1 ;
 smoothPtsP->x = (*conPtsPP)->x ;
 smoothPtsP->y = (*conPtsPP)->y ;
 smoothPtsP->z = (*conPtsPP)->z ;
 (smoothPtsP+*numConPtsP-1)->x = (*conPtsPP+*numConPtsP-1)->x ;
 (smoothPtsP+*numConPtsP-1)->y = (*conPtsPP+*numConPtsP-1)->y ;
 (smoothPtsP+*numConPtsP-1)->z = (*conPtsPP+*numConPtsP-1)->z ;
/*
** Initialise Point Types Array
*/
 *pointTypeP = *(pointTypeP+*numConPtsP-1) = 0 ;
 p3d1P = *conPtsPP ;
 for( p3d2P = *conPtsPP + 1 , typeP = pointTypeP + 1 ; p3d2P < *conPtsPP + *numConPtsP - 1 ; ++p3d2P , ++typeP )
   {
    p3d3P = p3d2P + 1 ;
    *typeP  = bcdtmMath_sideOf(p3d1P->x,p3d1P->y,p3d3P->x,p3d3P->y,p3d2P->x,p3d2P->y) ;
    p3d1P = p3d2P ;
   }
 if( closeFlag )
   {
    p3d2P  = *conPtsPP ;
    p3d1P = *conPtsPP + *numConPtsP-2 ;
    p3d3P = *conPtsPP + 1 ;
    *pointTypeP = *(pointTypeP+*numConPtsP-1) = bcdtmMath_sideOf(p3d1P->x,p3d1P->y,p3d3P->x,p3d3P->y,p3d2P->x,p3d2P->y) ;
   }
/*
** Write Out Point Types
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Vertex Move Point Assignments") ;
    for( p3d2P = *conPtsPP , typeP = pointTypeP  ; p3d2P < *conPtsPP + *numConPtsP ; ++p3d2P , ++typeP )
      {
       bcdtmWrite_message(0,0,0,"Type = %2ld ** %10.4lf %10.4lf %10.4lf",*typeP,p3d2P->x,p3d2P->y,p3d2P->z) ;
      }
   }
/*
** Scan And Shift verticesP
*/
 p3d1P = *conPtsPP ;
 for( p3d2P = *conPtsPP + 1 , wghtP = *weightPP , p3d4P = smoothPtsP + 1 , typeP = pointTypeP ; p3d2P < *conPtsPP + *numConPtsP - 1 ; ++p3d2P , ++p3d4P , ++typeP, ++wghtP )
   {
    p3d3P = p3d2P + 1 ;
    if( ! *wghtP && ( *typeP == -*(typeP-1) && *typeP == -*(typeP+1)) )
      {
       Xm = (p3d1P->x + p3d3P->x) / 2.0 ;
       Ym = (p3d1P->y + p3d3P->y) / 2.0 ;
       p3d4P->x = p3d2P->x + ( Xm - p3d2P->x ) * ratio ;
       p3d4P->y = p3d2P->y + ( Ym - p3d2P->y ) * ratio ;
       p3d4P->z = p3d2P->z ;
      }
    else
      {
       p3d4P->x = p3d2P->x ;
       p3d4P->y = p3d2P->y ;
       p3d4P->z = p3d2P->z ;
      }
    p3d1P = p3d2P ;
   }
/*
** Move First Point For Closure
*/
 if( closeFlag && *numConPtsP > 2 && ! **weightPP )
   {
    if( *pointTypeP == -*(pointTypeP+*numConPtsP-2) && *pointTypeP == -*(pointTypeP+1))
      {
       p3d1P = *conPtsPP + *numConPtsP - 2 ;
       p3d2P  = *conPtsPP ;
       p3d3P = p3d2P + 1 ;
       p3d4P = smoothPtsP  ;
       Xm = (p3d1P->x + p3d3P->x) / 2.0 ;
       Ym = (p3d1P->y + p3d3P->y) / 2.0 ;
       p3d4P->x = p3d2P->x + ( Xm - p3d2P->x ) * ratio ;
       p3d4P->y = p3d2P->y + ( Ym - p3d2P->y ) * ratio ;
       p3d4P->z = p3d2P->z ;
       (smoothPtsP + *numConPtsP - 1)->x = smoothPtsP->x ;
       (smoothPtsP + *numConPtsP - 1)->y = smoothPtsP->y ;
      }
   }
/*
** Set Smoothed Contour Points
*/
 free(*conPtsPP) ;
 *conPtsPP  = smoothPtsP ;
 smoothPtsP = NULL ;
/*
** Clean Up
*/
 cleanup :
 if( pointTypeP != NULL ) free(pointTypeP) ;
 if( smoothPtsP != NULL ) free(smoothPtsP)  ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Vertex Smoothing Contour Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Vertex Smoothing Contour Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_fivePointSmoothContour
(
 DPoint3d    **conPtsPP,
 long   *numConPtsP,
 long   **weightP,
 double smoothFactor
)
{
 long    numTmpPts,closeFlag=0,*wP;
 double  X1,Y1,X2,Y2,X3,Y3,X4,Y4,X5,Y5 ;
 DPoint3d     *tmpPtsP=NULL,*pP,*pS ;
/*
** Check There Are More Than Two Points In Contour Line
*/
 if( *numConPtsP <= 2 ) return(0) ;
/*
** Check If Contour Line Closes
*/
 closeFlag = 0 ;
 pP = *conPtsPP ; pS = *conPtsPP + *numConPtsP - 1 ;
 if( pP->x == pS->x && pP->y == pS->y ) closeFlag = 1 ;
/*
** Allocate Memory To Hold Smoothed Contour Line Points
*/
 numTmpPts = *numConPtsP + ( *numConPtsP - 2 ) * 4 ;
 if( closeFlag ) numTmpPts = numTmpPts + 4 ;
 numTmpPts = *numConPtsP * 7 ;
 tmpPtsP   = ( DPoint3d * ) malloc( numTmpPts * sizeof( DPoint3d )) ;
 if( tmpPtsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    return(1) ;
   }
/*
** Smooth Contour Line
*/
 wP = *weightP ;
 pS = tmpPtsP ;
 if( closeFlag )
   {
    if( *wP ) { *pS = **conPtsPP ; ++pS ; }
    else
      {
       bcdtmLoad_fivePointSmooth((*conPtsPP+*numConPtsP-2)->x,(*conPtsPP+*numConPtsP-2)->y,(*conPtsPP)->x,(*conPtsPP)->y,(*conPtsPP+1)->x,(*conPtsPP+1)->y,smoothFactor,&X1,&Y1,&X2,&Y2,&X3,&Y3,&X4,&Y4,&X5,&Y5 ) ;
       pS->x = X1 ; pS->y = Y1 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X2 ; pS->y = Y2 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X3 ; pS->y = Y3 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X4 ; pS->y = Y4 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X5 ; pS->y = Y5 ; pS->z = (*conPtsPP)->z ; ++pS ;
      }
   }
 else { *pS = **conPtsPP ; ++pS ; }

 wP = *weightP + 1 ;
 pP = *conPtsPP + 1 ;
 while ( pP < *conPtsPP + *numConPtsP - 1 )
   {
    if( *wP )
      { pS->x = pP->x ; pS->y = pP->y ; pS->z = pP->z ; ++pS ; }
    else
      {
       bcdtmLoad_fivePointSmooth((pP-1)->x,(pP-1)->y,pP->x,pP->y,(pP+1)->x,(pP+1)->y,smoothFactor,&X1,&Y1,&X2,&Y2,&X3,&Y3,&X4,&Y4,&X5,&Y5 ) ;
       pS->x = X1 ; pS->y = Y1 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X2 ; pS->y = Y2 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X3 ; pS->y = Y3 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X4 ; pS->y = Y4 ; pS->z = (*conPtsPP)->z ; ++pS ;
       pS->x = X5 ; pS->y = Y5 ; pS->z = (*conPtsPP)->z ; ++pS ;
      }
    ++pP ;
    ++wP ;
   }
 if( closeFlag ) *pS = *tmpPtsP ;
 else            { pS->x = pP->x ; pS->y = pP->y ; pS->z = pP->z ; }
/*
** Free Memory For Contour Points
*/
 free(*conPtsPP) ;
 *conPtsPP   = tmpPtsP ;
 *numConPtsP = (long ) ( pS - tmpPtsP ) + 1 ;
 *weightP    = ( long * ) realloc(*weightP,*numConPtsP*sizeof(long)) ;
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
BENTLEYDTM_Private int bcdtmLoad_fivePointSmooth(double X1,double Y1,double X2,double Y2,double X3,double Y3,double smoothFactor, double *Sx1, double *Sy1,double *Sx2, double *Sy2,double *Sx3, double *Sy3,double *Sx4, double *Sy4,double *Sx5, double *Sy5 )
{
 double x15,y15,xlo,ylo,xhi,yhi ;
/*
** Calculate Points 1 and 5
*/
 *Sx1 = X2 + (X1-X2) * smoothFactor ;
 *Sy1 = Y2 + (Y1-Y2) * smoothFactor ;
 *Sx5 = X2 + (X3-X2) * smoothFactor ;
 *Sy5 = Y2 + (Y3-Y2) * smoothFactor ;
/*
** Calculate Smoothing For Points 2,3,4
*/
 x15 = (*Sx1 + *Sx5 ) / 2.0 ;
 y15 = (*Sy1 + *Sy5 ) / 2.0 ;

 *Sx3 = ( x15 + X2 ) / 2.0 ;
 *Sy3 = ( y15 + Y2 ) / 2.0 ;

 xlo = ( x15 + *Sx1 ) / 2.0 ;
 ylo = ( y15 + *Sy1 ) / 2.0 ;
 xhi = ( X2  + *Sx1 ) / 2.0 ;
 yhi = ( Y2  + *Sy1 ) / 2.0 ;

 *Sx2 = ( xlo + xhi ) / 2.0 ;
 *Sy2 = ( ylo + yhi ) / 2.0 ;
 *Sx2 = ( *Sx2 + xhi ) / 2.0 ;
 *Sy2 = ( *Sy2 + yhi ) / 2.0 ;

 xlo = ( x15 + *Sx5 ) / 2.0 ;
 ylo = ( y15 + *Sy5 ) / 2.0 ;
 xhi = ( X2  + *Sx5 ) / 2.0 ;
 yhi = ( Y2  + *Sy5 ) / 2.0 ;

 *Sx4 = ( xlo + xhi ) / 2.0 ;
 *Sy4 = ( ylo + yhi ) / 2.0 ;
 *Sx4 = ( *Sx4 + xhi ) / 2.0 ;
 *Sy4 = ( *Sy4 + yhi ) / 2.0 ;
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
BENTLEYDTM_Private int bcdtmLoad_splineSmoothContour
(
 BC_DTM_OBJ *dtmP,       /* ==> Pointer To Dtm Object For Pull Back Of Smoothed Points  */
 double conInt,          /* ==> Contour Interval                                        */
 DPoint3d    **conPtsPP,      /* ==> Pointer To Contour Points                               */
 long   *numconPtsP,     /* ==> Number Of Contour Points                                */
 long   **conWghtPP,     /* ==> Pointer To Contour Point Weightings                     */
 DTMContourSmoothing smoothOption, /* ==> Smoothing Option <2,3>                                  */
 double smoothFactor,    /* ==> Smoothing Factor                                        */
 long   pointDensity     /* ==> Point Densification Between Succesive Contour Points    */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   i,n,ii=0,splineSize=0,bndyCondition=0,*wP,numSplinePts=0,memSplinePts=0,memSplinePtsInc=1000,closeFlag=0,chordOption=0 ;
 double dl,ll,ax=0.0,bx=0.0,xx=0.0,sp,dsp,ddsp,*sp1P,*sp2P ;
 double contourValue,*xSplineP=NULL,*ySplineP=NULL,*wSplineP=NULL,*lSplineP=NULL ;
 DPoint3d    *p3dP,*splinePtsP=NULL ;
 long   numBefore=0 ;
/*
** Write Entry Message
*/
// bcdtmWrite_message(0,0,0,"Spline Smoothing Contour ** contourValue = %10.4lf conInt = %10.4lf smoothOption = %2ld smoothFactor = %5.2lf pointDensity = %2ld",conPtsP->z,conInt,smoothOption,smoothFactor,pointDensity) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Spline Smoothing Contour") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Contour Interval = %8.4lf",conInt) ;
    bcdtmWrite_message(0,0,0,"Contour Points   = %p",*conPtsPP) ;
    bcdtmWrite_message(0,0,0,"Number Points    = %p",*numconPtsP) ;
    bcdtmWrite_message(0,0,0,"Contour Weights  = %p",*conWghtPP) ;
    bcdtmWrite_message(0,0,0,"Smooth Factor    = %8.4lf",smoothFactor) ;
    bcdtmWrite_message(0,0,0,"Point  Density   = %8ld",pointDensity) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Contour Points And Weights") ;
       for( p3dP = *conPtsPP , wP = *conWghtPP ; p3dP < *conPtsPP + *numconPtsP ; ++p3dP , ++wP )
         {
          bcdtmWrite_message(0,0,0,"Contour Point[%8ld] ** %10.4lf %10.4lf %10.4lf ** %2ld",(long)(p3dP-*conPtsPP),p3dP->x,p3dP->y,p3dP->z,*wP) ;
         }
      }
   }
/*
** Densify Contour Line For Spline Control
*/
 numBefore = *numconPtsP ;
// if( bcdtmLoad_densifyWeightedPointArray(conPtsPP,numconPtsP,conWghtPP,25.0)) goto errexit ;
// if( numBefore != *numconPtsP ) bcdtmWrite_message(0,0,0,"Densifying Contour %8.3lf ** Before = %8ld After  = %8ld",(*conPtsPP)->z,numBefore,*numconPtsP) ;
// pointDensity = 2 ;
/*
** Only Process If More Than Two Contour Points
*/
 if( *numconPtsP > 2 )
   {
    contourValue = (*conPtsPP)->z ;
    splineSize   = *numconPtsP ;
    if( bcdtmMath_distance((*conPtsPP)->x,(*conPtsPP)->y,(*conPtsPP+*numconPtsP-1)->x,(*conPtsPP+*numconPtsP-1)->y) < 0.0001 ) closeFlag=1 ;
    if( dbg &&   closeFlag ) bcdtmWrite_message(0,0,0,"Closed Contour") ;
    if( dbg && ! closeFlag ) bcdtmWrite_message(0,0,0,"Open Contour") ;
    if( closeFlag )
      {
       (*conPtsPP+*numconPtsP-1)->x = (*conPtsPP)->x ;
       (*conPtsPP+*numconPtsP-1)->y = (*conPtsPP)->y ;
      }
/*
** Allocate Memory For Spline Arrays
*/
    xSplineP = ( double * ) malloc (splineSize*sizeof(double)) ;
    ySplineP = ( double * ) malloc (splineSize*sizeof(double)) ;
    lSplineP = ( double * ) malloc (splineSize*sizeof(double)) ;
    wSplineP = ( double * ) malloc (splineSize*sizeof(double)) ;
    if( xSplineP == NULL || ySplineP == NULL || wSplineP == NULL || lSplineP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
** Copy Contour Point Coordinates To Spline x & y Arrays
*/
    for( p3dP = *conPtsPP , sp1P = xSplineP , sp2P = ySplineP ; p3dP < *conPtsPP + *numconPtsP ; ++p3dP, ++sp1P, ++sp2P)
      {
       *sp1P = p3dP->x ;
       *sp2P = p3dP->y ;
      }
/*
** Copy Contour Point Weights To Spline Weight Array
*/
    for( sp1P = wSplineP , wP = *conWghtPP ; sp1P < wSplineP + splineSize; ++sp1P , ++wP )
      {
       if( *wP ) *sp1P = 0.0 ;
       else      *sp1P = smoothFactor ;
      }
//    if( ! closeFlag ) *wSplineP = *(wSplineP+splineSize-1) = 0.0 ;
/*
** Set Weighting Of End Points To Force Spline through end Points
*/
    *wSplineP = *(wSplineP+splineSize-1) = 0.0 ;
    if( closeFlag ) *(wSplineP+1) = *(wSplineP+splineSize-2) = 0.0 ;
/*
** Calculate Knots ( Chord Lengths )
*/
    *lSplineP = 0.0 ;
    for( n = 1 ; n < splineSize; ++n )
      {
       if( chordOption )  *(lSplineP+n) = *(lSplineP+n-1) + sqrt((*(xSplineP+n)-*(xSplineP+n-1))*(*(xSplineP+n)-*(xSplineP+n-1)) + (*(ySplineP+n)-*(ySplineP+n-1))*(*(ySplineP+n)-*(ySplineP+n-1))) ;
       else               *(lSplineP+n) = *(lSplineP+n-1) + sqrt (sqrt((*(xSplineP+n)-*(xSplineP+n-1))*(*(xSplineP+n)-*(xSplineP+n-1)) + (*(ySplineP+n)-*(ySplineP+n-1))*(*(ySplineP+n)-*(ySplineP+n-1)))) ;
      }
/*
** Calculate Spline Parameters
*/
    bndyCondition = 2 ;
    if( closeFlag )  bndyCondition = 3 ;
    xx = ax = bx = 0.0 ;
/*
** Calculate x Spline Parameters
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating x Spline Parameters") ;
    if( bcdtmLoad_cubicInterpolateSpline(0,splineSize,lSplineP,xSplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp)) goto errexit ;
/*
** Calculate x Spline Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating x Spline Points") ;
    for( sp1P = lSplineP+1 , wP = *conWghtPP ; sp1P < lSplineP + *numconPtsP ; ++sp1P , ++wP )
      {
       n  = pointDensity ;
       ll =  *sp1P - *(sp1P-1) ;
       ii = (long)(sp1P-lSplineP-1)  ;
       dl = ll / (double)(n+1)  ;
       if( sp1P == lSplineP + *numconPtsP - 1 ) n = n + 1 ;
       for( i = 0 ; i <= n ; ++i )
         {
          xx = *(sp1P-1) + dl * (double)(i) ;
          bcdtmLoad_cubicInterpolateSpline(1,splineSize,lSplineP,xSplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp) ;
          if( numSplinePts == memSplinePts )
            {
             memSplinePts = memSplinePts + memSplinePtsInc ;
             if( splinePtsP == NULL ) splinePtsP = ( DPoint3d * ) malloc (memSplinePts * sizeof(DPoint3d)) ;
             else                     splinePtsP = ( DPoint3d * ) realloc(splinePtsP,memSplinePts * sizeof(DPoint3d)) ;
             if( splinePtsP == NULL )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure")  ;
                goto errexit ;
               }
            }
          (splinePtsP+numSplinePts)->x = sp ;
          (splinePtsP+numSplinePts)->z = contourValue ;
          ++numSplinePts ;
         }
      }
/*
**  Free Spline Memory
*/
    bcdtmLoad_cubicInterpolateSpline(2,splineSize,xSplineP,ySplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp) ;
/*
**  Calculate y Spline Parameters
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating y Spline Parameters") ;
    if( bcdtmLoad_cubicInterpolateSpline(0,splineSize,lSplineP,ySplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp)) goto errexit ;
/*
**  Calculate Smoothed y Spline Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating y Spline Points") ;
    numSplinePts = 0 ;
    for( sp1P = lSplineP+1 , wP = *conWghtPP ; sp1P < lSplineP + *numconPtsP ; ++sp1P , ++wP )
      {
       n = pointDensity ;
       ll =  *sp1P -*(sp1P-1) ;
       ii = (long)(sp1P-lSplineP-1)  ;
       dl = ll / (double)(n+1)  ;
       if( sp1P == lSplineP + *numconPtsP - 1 ) n = n + 1 ;
       for( i = 0 ; i <= n ; ++i )
         {
          xx = *(sp1P-1) + dl * (double)(i) ;
          bcdtmLoad_cubicInterpolateSpline(1,splineSize,lSplineP,ySplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp) ;
          (splinePtsP+numSplinePts)->y = sp ;
          (splinePtsP+numSplinePts)->z = contourValue ;
          ++numSplinePts ;
         }
      }
/*
**   Pull Spline Points Back Onto Break Lines
*/
    if( dtmP != NULL )
      {
       if( bcdtmLoad_pullSmoothPointsBackOnToBreakLines(xSplineP,ySplineP,*conWghtPP,splineSize,pointDensity,splinePtsP,numSplinePts)) goto errexit ;
/*
**      Pull Spline Points Back To Within A  Ratio Of The Contour Interval
*/
       if (smoothOption == DTMContourSmoothing::Spline) if (bcdtmLoad_pullSmoothPointsBack (dtmP, conInt, xSplineP, ySplineP, *conWghtPP, splineSize, pointDensity, contourValue, splinePtsP, numSplinePts)) goto errexit;
      }
/*
**  Pull Spline Points Back Where Knots Have Ocuured
*/
 //   if( smoothOption == 2 ) if( bcdtmLoad_pullSmoothPointsBackAtKnots(xSplineP,ySplineP,*conWghtPP,splineSize,pointDensity,contourValue,splinePtsP,numSplinePts)) goto errexit ;
/*
**    Assign Spline Points To Contour Points
*/
     if( numSplinePts < memSplinePts ) splinePtsP = ( DPoint3d * ) realloc(splinePtsP,numSplinePts*sizeof(DPoint3d)) ;
     free(*conPtsPP) ;
     *conPtsPP   = splinePtsP ;
     *numconPtsP = numSplinePts ;
     splinePtsP  = NULL ;
/*
**   Reassign Point Weights
*/
     *conWghtPP  = ( long *) realloc(*conWghtPP,numSplinePts*sizeof(long)) ;
     if( *conWghtPP == NULL )
       {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
       }
     for( wP = *conWghtPP ; wP < *conWghtPP + splineSize; ++wP ) *wP = 0 ;
     for( sp1P = wSplineP , wP = *conWghtPP ; sp1P < wSplineP + splineSize; ++sp1P , wP = wP + pointDensity )
      {
       *wP = (long)(*sp1P) ;
      }
    }
/*
** Clean Up
*/
 cleanup :
 if( xSplineP != NULL ) free(xSplineP)  ;
 if( ySplineP != NULL ) free(ySplineP)  ;
 if( wSplineP != NULL ) free(wSplineP)  ;
 if( lSplineP != NULL ) free(lSplineP)  ;
 if( splinePtsP != NULL ) free(splinePtsP) ;
 bcdtmLoad_cubicInterpolateSpline(2,splineSize,xSplineP,ySplineP,wSplineP,bndyCondition,ax,bx,ii,xx,&sp,&dsp,&ddsp) ;
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
BENTLEYDTM_Private int bcdtmLoad_cubicInterpolateSpline(long Ind,long N,double x[],double y[],double Rho[],long Ib,double Ax,double Bx,long I,double Xx,double *Sp,double *Dsp,double *D2sp)
/*
**
** This Function Constructs the Smoothing Cubic B spline
** for an array of points. The smaller the point weight values the closer the
** spline to the points. The smoothing spline will pass through points with
** a point weightvalue of zero.
**
** ==> Ind  - mode type:
**          = 0 - Calculate Spline Parameters
**          = 1 - Calculate Point On Spline
** ==> N    - Number of points in array
** ==> X    - Array (of size n) containing x-coordinates
** ==> Y    - Array (of size n) containing y-coordinates
** ==> Rho  - Array (of size n) containing pointweightPing coefficients
** ==> Ib   - Type of boundary conditions
**          = i (i=1-3) - conditions of ith type
** ==> Ax   - Start parameter value for boundary condition of the first type
** ==> Bx   - End parameter value for boundary condition of the first type
** ==> I    - Start Of Spline Interval On which Point will be evaluated.
** ==> Xx   - Point On Spline To be Evaluated
** <== Sp   - Value of spline at point Xx
** <== Dsp  - Value of its first derivative at point Xx
** <== D2sp - Value of its second derivative at point xx
**
**  a,b,c,d,e,g,zm          - work arrays (of size n)
**  u,v,w,p,aa,dd,q,r,st,t  - work arrays (of size n+2)
**
*/
{
 long   i,nn ;
 double h,h1,h2,h3=0,h4,di,tt ;
 static double *A=NULL,*B=NULL,*C=NULL,*D=NULL,*E=NULL,*G=NULL,*Zm=NULL,*Aa=NULL,*Dd=NULL ;
 static double *P=NULL,*Q=NULL,*R=NULL,*S=NULL,*T=NULL,*U=NULL,*V=NULL,*W=NULL ;
/*
** If First Call Calculate Spline Parameters
*/
 nn = N ;
 if( Ind == 0 )
   {
/*
** Free Memory
*/
    if( A  != NULL ) { free(A)  ; A  = NULL ; }
    if( B  != NULL ) { free(B)  ; B  = NULL ; }
    if( C  != NULL ) { free(C)  ; C  = NULL ; }
    if( D  != NULL ) { free(D)  ; D  = NULL ; }
    if( G  != NULL ) { free(G)  ; G  = NULL ; }
    if( E  != NULL ) { free(E)  ; E  = NULL ; }
    if( Zm != NULL ) { free(Zm) ; Zm = NULL ; }

    if( Aa != NULL ) { free(Aa) ; Aa = NULL ; }
    if( Dd != NULL ) { free(Dd) ; Dd = NULL ; }
    if( P  != NULL ) { free(P)  ; P  = NULL ; }
    if( Q  != NULL ) { free(Q)  ; Q  = NULL ; }
    if( R  != NULL ) { free(R)  ; R  = NULL ; }
    if( S  != NULL ) { free(S)  ; S  = NULL ; }
    if( T  != NULL ) { free(T)  ; T  = NULL ; }
    if( U  != NULL ) { free(U)  ; U  = NULL ; }
    if( V  != NULL ) { free(V)  ; V  = NULL ; }
    if( W  != NULL ) { free(W)  ; W  = NULL ; }
/*
** Allocate Memory
*/
    A  = (double *) malloc( N * sizeof(double)) ;
    B  = (double *) malloc( N * sizeof(double)) ;
    C  = (double *) malloc( N * sizeof(double)) ;
    D  = (double *) malloc( N * sizeof(double)) ;
    G  = (double *) malloc( N * sizeof(double)) ;
    E  = (double *) malloc( N * sizeof(double)) ;
    Zm = (double *) malloc( N * sizeof(double)) ;

    Aa = (double *) malloc( (N+2) * sizeof(double)) ;
    Dd = (double *) malloc( (N+2) * sizeof(double)) ;
    P  = (double *) malloc( (N+2) * sizeof(double)) ;
    Q  = (double *) malloc( (N+2) * sizeof(double)) ;
    R  = (double *) malloc( (N+2) * sizeof(double)) ;
    S  = (double *) malloc( (N+2) * sizeof(double)) ;
    T  = (double *) malloc( (N+2) * sizeof(double)) ;
    U  = (double *) malloc( (N+2) * sizeof(double)) ;
    V  = (double *) malloc( (N+2) * sizeof(double)) ;
    W  = (double *) malloc( (N+2) * sizeof(double)) ;
/*
** Calculate Spline Parameters For Boundary Condition
*/
    switch ( Ib )
      {
       case   1 :
         h1       = *(x+1)-*x ;
         *A       = h1/3.0 + (*Rho+*(Rho+1))/(h1*h1) ;
         h2       = *(x+2) - *(x+1) ;
         *B       = h1/6.0-(1.0/h1 +1.0/h2)**(Rho+1)/h1 - *Rho/(h1*h1) ;
         *C       = *(Rho+1)/(h1*h2) ;
         *G       = (*(y+1) - *y)/h1 - Ax ;
         *(E+2)   = *C ;
         h1       = *(x+N-1) - *(x+N-2) ;
         h2       = *(x+N-2) - *(x+N-3) ;
         *(A+N-1) = h1/3.0 +(*(Rho+N-2)+*(Rho+N-1))/(h1*h1) ;
         *(B+N-2) = h1/6.0-(1.0/h1 +1.0/h2)**(Rho+N-2)/h1 -*(Rho+N-1)/(h1*h1) ;
         *(C+N-3) = *(Rho+N-2)/(h1*h2) ;
         *E       = 0.0 ;
         *(E+1)   = 0.0 ;
         *D       = 0.0 ;
         *(D+1)   = *B ;
         *(C+N-2) = 0.0 ;
         *(C+N-1) = 0.0 ;
         *(B+N-1) = 0.0 ;
         *(D+N-1) = *(B+N-2) ;
         *(G+N-1) = Bx - (*(y+N-1) - *(y+N-2))/h1 ;
         *(E+N-1) = *(C+N-3) ;
       break    ;

       case   2 :
         *A       = 1.0  ;
         *(A+N-1) = 1.0  ;
         *B       = 0.0  ;
         *(B+N-1) = 0.0  ;
         *(B+N-2) = 0.0  ;
         *C       = 0.0  ;
         *(C+N-1) = 0.0  ;
         *(C+N-2) = 0.0  ;
         *(C+N-3) = 0.0  ;
         *D       = 0.0  ;
         *(D+1)   = 0.0  ;
         *(D+N-1) = 0.0  ;
         *E       = 0.0  ;
         *(E+1)   = 0.0  ;
         *(E+2)   = 0.0  ;
         *(E+N-1) = 0.0  ;
         *G       = 0.0  ;
         *(G+N-1) = 0.0  ;
       break    ;

       case   3 :
         h1       = *(x+1) - *x ;
         h2       = *(x+N-1) - *(x+N-2) ;
         h3       = *(x+2) - *(x+1) ;
         h4       = *(x+N-2) - *(x+N-3) ;
         *A       = (h1 + h2)/3.0 + *(Rho+N-2)/(h2*h2) + *(Rho+1)/(h1*h1) + (1.0/h1 + 1.0/h2)*(1.0/h1 + 1.0/h2)* *Rho ;
         *B       = h1/6.0   -((1.0  /h2 + 1.0  /h1)* *Rho + (1.0  /h2 + 1.0  /h3)* *(Rho+1))/h2 ;
         *C       = *Rho/(h1*h3) ;
         *G       = (*(y+1) - *y)/h1 - (*y - *(y+N-2))/h2 ;
         *(D+1)   = *B ;
         *(E+2)   = *C ;
         *(C+N-3) = *(Rho+N-2)/(h2*h4) ;
         *(C+N-2) = *(Rho+N-1)/(h2*h1) ;
         *(B+N-2) = h2/3.0   - ((1.0  /h4 + 1.0  /h2)* *(Rho+N-2) + (1.0  /h2 + 1.0  /h1)* *(Rho+N-1))/h2 ;
         *D       = *(B+N-2) ;
         *E       = *(C+N-3) ;
         *(E+1)   = *(C+N-2) ;
         *(G+N-2) = (*(y+N-1) - *(y+N-2))/h2 - (*(y+N-2) - *(y+N-3))/h4 ;
         nn       = N - 1 ;
       break    ;
      } ;
/*
** Calculate Intermediate Values
*/
    for( i = 1 ; i < N - 1 ; ++i )
      {
       h1   = *(x+i)   - *(x+i-1) ;
       h2   = *(x+i+1) - *(x+i) ;
       *(A+i) = (h1+h2)/3.0 + *(Rho+i-1)/(h1*h1) + *(Rho+i+1)/(h2*h2) + (1.0/h1 + 1.0/h2)*(1.0/h1 + 1.0/h2)* *(Rho+i) ;
       *(G+i) = (*(y+i+1) - *(y+i))/h2 - (*(y+i)- *(y+i-1))/h1 ;
       if( i < N-2 )
         {
          h3 = (*(x+i+2)-*(x+i+1)) ;
          *(B+i) = h2/6.0 - ((1.0/h1 + 1.0/h2) * *(Rho+i) + (1.0/h2 + 1.0/h3)* *(Rho+i+1))/h2 ;
          *(D+i+1) = *(B+i) ;
         }
       if( i < N-3 )
         {
          *(C+i) = *(Rho+i+1)/(h2*h3) ;
          *(E+i+2) = *(C+i) ;
         }
      }
/*
**  Solve Equations
*/
     bcdtmLoad_solvePentaDiagonalMatrix(nn,A,B,C,D,E,G,U,V,W,P,Q,R,S,T,Aa,Dd,Zm) ;
     if( Ib == 2 ) *(Zm+N-1) = *Zm ;
     *Aa = *y - *Rho*(*(Zm+1)-*Zm)/(*(x+1)-*x) ;
     *(Aa+N-1) = *(y+N-1) + *(Rho+N-1)*(*(Zm+N-1)-*(Zm+N-2))/(*(x+N-1)-*(x+N-2)) ;
     if( Ib == 3 )
       {
        *(Zm+N-1) = *Zm ;
        di = (*(Zm+1)-*Zm)/(*(x+1)-*x) ;
        di = di - (*(Zm+N-1)-*(Zm+N-2))/(*(x+N-1)-*(x+N-2)) ;
        *Aa = *y - *Rho*di ;
        *(Aa+N-1) = *Aa ;
       }
     for( i = 1 ; i < N - 1 ; ++i )
       {
        di = (*(Zm+i+1)-*(Zm+i))/(*(x+i+1)-*(x+i)) ;
        di = di - (*(Zm+i)-*(Zm+i-1))/(*(x+i)-*(x+i-1)) ;
        *(Aa+i) = *(y+i) - *(Rho+i)*di ;
       }
/*
** Free Unwanted WorkSpace Memory
*/
    if( A  != NULL ) { free(A)  ; A  = NULL ; }
    if( B  != NULL ) { free(B)  ; B  = NULL ; }
    if( C  != NULL ) { free(C)  ; C  = NULL ; }
    if( D  != NULL ) { free(D)  ; D  = NULL ; }
    if( G  != NULL ) { free(G)  ; G  = NULL ; }
    if( E  != NULL ) { free(E)  ; E  = NULL ; }
    if( P  != NULL ) { free(P)  ; P  = NULL ; }
    if( Q  != NULL ) { free(Q)  ; Q  = NULL ; }
    if( R  != NULL ) { free(R)  ; R  = NULL ; }
    if( S  != NULL ) { free(S)  ; S  = NULL ; }
    if( T  != NULL ) { free(T)  ; T  = NULL ; }
    if( U  != NULL ) { free(U)  ; U  = NULL ; }
    if( V  != NULL ) { free(V)  ; V  = NULL ; }
    if( W  != NULL ) { free(W)  ; W  = NULL ; }
   }
/*
** Calculate Interpolated Values
*/
 if( Ind == 1 )
   {
/*
    for( i = 0 ; i < N - 1 && *(x+i) <= Xx ; ++i )  ;
    --i ;
*/
    i     = I ;
    h     = *(x+i+1) - *(x+i) ;
    tt    = (Xx - *(x+i))/h ;
    *Sp   = *(Aa+i)*(1.0-tt) + *(Aa+i+1)*tt - h*h/6.0*tt*(1.0-tt) * ((2.0-tt)**(Zm+i) + (1.0+tt)**(Zm+i+1)) ;
    *Dsp  = (*(Aa+i+1) - *(Aa+i))/h - h/6.0*((2.0 - 6.0*tt + 3.0*tt*tt)**(Zm+i) + (1.0 - 3.0*tt*tt)**(Zm+i+1)) ;
    *D2sp = (1.0 - tt)**(Zm+i) + tt**(Zm+i+1) ;
  }
/*
** Free Remaining Workspace Memory
*/
 if( Ind == 2 )
   {
    if( Zm != NULL ) { free(Zm) ; Zm = NULL ; }
    if( Aa != NULL ) { free(Aa) ; Aa = NULL ; }
    if( Dd != NULL ) { free(Dd) ; Dd = NULL ; }
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
BENTLEYDTM_Private int bcdtmLoad_solvePentaDiagonalMatrix(long n,double a[],double b[],double c[],double d[],double e[],double g[],double u[],double v[],double w[],double p[],double q[],double r[],double s[],double t[],double aa[],double dd[],double z[])
/*
** This Function solves linear systems of the form
**
** Ú                                          ¿ Ú  ¿ Ú  ¿
** ³a1   b1   c1    0   ..........0   e1   d1 ³ ³z1³ ³g1³
** ³d2   a2   b2   c2    0 ........    0   e2 ³ ³z2³ ³g2³
** ³e3   d3   a3   b3   c3    0 ...    0    0 ³ ³z3³ ³g3³
** ³O    e4   d4   a4   b4   c4   .         0 ³ ³z4³=³g4³
** ³..........................................³ ³..³ ³  ³
** ³                                          ³ ³  ³ ³  ³
** ³bn   cn   0     0  .....  0   en  dn   an ³ ³zn³ ³gn³
** À                                          Ù À  Ù À  Ù
**
**  with pentadiagonal matrix by sweep method
**
** ==> n - number of equations
** ==> a,b,c,d,e - arrays (of size n) for elements of the matrix's diagonal
** ==> g - array (of size n) for elements of the right parts
**
** ==> u,v,w,p,q,r,s,t,aa,dd - work arrays (of size n+2)
**
** <== z - array (of size n) containing solution of the system
**
*/
{
 long   i ;
 double a11,a12,a21,a22,b1,b2 ;
/*
** Initialise
*/
 v[0] = 0.0 ;
 v[1] = 0.0 ;
 w[0] = 0.0 ;
 w[1] = 0.0 ;
 u[0] = 0.0 ;
 u[1] = 0.0 ;

 for( i = 0 ; i < n ; ++ i )
   {
    dd[i]  = d[i] + e[i]*v[i] ;
    aa[i]  = a[i] + dd[i]*v[i+1] +e[i]*w[i] ;
    if( aa[i] == 0.0 ) aa[i] = 0.00000001 ;
    u[i+2] = (g[i] - dd[i]*u[i+1] - e[i]*u[i]) / aa[i] ;
    v[i+2] = -(b[i]+dd[i]*w[i+1])/aa[i] ;
    w[i+2] = -c[i]/aa[i] ;
   }

 p[0] = 0.0 ;
 q[1] = 0.0 ;
 p[1] = 1.0 ;
 q[0] = 1.0 ;


 for( i = 0 ; i < n ; ++ i )
   {
    p[i+2] = -(dd[i]*p[i+1] + e[i]*p[i]) / aa[i] ;
    q[i+2] = -(dd[i]*q[i+1] + e[i]*q[i]) / aa[i] ;
   }

 t[n-2] = 0.0 ;
 s[n-2] = 0.0 ;
 t[n-1] = 0.0 ;
 r[n-1] = 0.0 ;
 r[n-2] = 1.0 ;
 s[n-1] = 1.0 ;

 for( i = n-3 ; i >= 0 ; --i )
   {
    t[i] = v[i+2]*t[i+1] +w[i+2]*t[i+2] + u[i+2] ;
    s[i] = v[i+2]*s[i+1] +w[i+2]*s[i+2] + p[i+2] ;
    r[i] = v[i+2]*r[i+1] +w[i+2]*r[i+2] + q[i+2] ;
   }


 a11    =  1.0 - q[n] - w[n]*r[0] ;
 a12    = -(p[n] + v[n] + w[n]*s[0]) ;
 a21    = -(v[n+1]*r[0] + w[n+1]*r[1] + q[n+1]) ;
 a22    =  1.0 - p[n+1] - v[n+1]*s[0] - w[n+1]*s[1] ;
 b1     = w[n]*t[0] + u[n] ;
 b2     = v[n+1]*t[0] + w[n+1]*t[1] + u[n+1] ;
 z[n-2] = ( b1*a22 - b2*a12)/(a11*a22-a12*a21) ;
 z[n-1] = (-b1*a21 + b2*a11)/(a11*a22-a12*a21) ;
/*
** Calculate Solution
*/
 for( i = 0 ; i < n - 2 ; ++i )
   {
    z[i] = t[i] + s[i]*z[n-1] + r[i]*z[n-2] ;
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
BENTLEYDTM_Private int bcdtmLoad_pullSmoothPointsBackOnToBreakLines(double x[],double y[],long conWght[],long numPts,long pointDensity,DPoint3d *conPtsP,long numConPts)
/*
** This Function Pulls Smoothed Points Back Onto Break Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,lofs,mofs,density ;
 double dx,dy,Xp,Yp ;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg) bcdtmWrite_message(0,0,0,"Pulling Points Back Onto Break Line") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Pre Smoothed Contour Points = %6ld",numPts) ;
    for( ofs = 0 ; ofs < numPts ; ++ofs )
      {
       bcdtmWrite_message(0,0,0,"BS Point[%6ld] ** %12.4lf %12.4lf %10.4lf",ofs,x[ofs],y[ofs],conPtsP->z) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Smoothed Contour Points = %6ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"AS Point[%6ld] ** %12.4lf %12.4lf %12.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,conPtsP->z) ;
      }
   }
/*
** Initialise
*/
 density = pointDensity + 1 ;
/*
** Scan Unsmoothed Contour Points And Look For Consecutive Break Points
*/
 for( lofs = 0 ; lofs < numPts - 1 ; ++lofs )
   {
/*
** Test For Consecutive Break Points
*/
    if( conWght[lofs] == 2 && conWght[lofs+1] == 2 )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Consecutive Break Points Detected ** Contour Value = %10.4lf",conPtsP->z) ;
          bcdtmWrite_message(0,0,0,"Break Point 1 = %4ld ** %10.4lf %10.4lf",lofs,x[lofs],y[lofs] ) ;
          bcdtmWrite_message(0,0,0,"Break Point 2 = %4ld ** %10.4lf %10.4lf",lofs+1,x[lofs+1],y[lofs+1] ) ;
         }
/*
** Project Smoothed Points Back Onto Un Smoothed Break Lines
*/
       for( mofs = 0 ; mofs < density ; ++mofs )
         {
/*
** Project Smooth Point Back To Un Smoothed Break Line
*/
          if( ! mofs ) { Xp = x[lofs] ; Yp = y[lofs] ; }
          else
            {
             dx = x[lofs+1] - x[lofs] ;
             if( dx == 0.0 )  Xp = x[lofs] ;
             else             Xp = x[lofs] + dx * (double)mofs / (double)density ;
             dy = y[lofs+1] - y[lofs] ;
             if( dy == 0.0 )  Yp = y[lofs] ;
             else             Yp = y[lofs] + dy * (double)mofs / (double)density ;
            }
/*
** Calculate Smooth Point Offset
*/
          ofs = lofs * density + mofs ;
/*
** Set Smooth Point Back Onto Break Line
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Smooth Point[%4ld] %10.4lf %10.4lf Projected on Break %10.4lf %10.4lf",ofs,(conPtsP+ofs)->x,(conPtsP+ofs)->y,Xp,Yp) ;
          (conPtsP+ofs)->x = Xp ;
          (conPtsP+ofs)->y = Yp ;
         }
      }
   }
/*
** Write Points After Pull Back Onto Breaks
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Smoothed Contour Points After Pull Back Onto Breaks = %6ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] ** %12.4lf %12.4lf %10.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,p3dP->z) ;
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
BENTLEYDTM_Private int bcdtmLoad_pullSmoothPointsBack
(
 BC_DTM_OBJ *dtmP,          /* ==> Pointer To Dtm Object Used For Pull Back  */
 double conInt,             /* ==> Contour Interval                          */
 double *xKnotP,            /* ==> Pointer To Knot x Coordinates             */
 double *yKnotP,            /* ==> Pointer To Knot y Coordinates             */
 long   *knotWghtP,         /* ==> Pointer To Knot Weightings                */
 long   numKnots,           /* ==> Number Of Knots                           */
 long   pointDensity,       /* ==> Density Of Splined Points                 */
 double contourValue,       /* ==> Contour Value                             */
 DPoint3d    *conPtsP,           /* ==> Pointer To Splined Contour Points         */
 long   numConPts           /* ==> Number Of Splined Contour Points          */
)
/*
** This Function Pulls Smoothed Contour Points Back To Within A Range Of The Contour Interval
** This Is Done To Stop Contours Overlapping
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ofs,lofs,mofs,density,process,drapeFlag ;
 double dx,dy,z,x,y,Xp,Yp,Xs,Ys,normalDist,intervalRatio=0.75 ;
 double ppTolSquared = dtmP->ppTol * dtmP->ppTol;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg) bcdtmWrite_message(0,0,0,"Pulling Smooth Points Back") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Contour Points = %6ld",numKnots) ;
    for( ofs = 0 ; ofs < numKnots ; ++ofs )
      {
       bcdtmWrite_message(0,0,0,"BS Point[%6ld] ** %12.4lf %12.4lf %10.4lf",ofs,*(xKnotP+ofs),*(yKnotP+ofs),conPtsP->z) ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Smoothed Contour Points = %6ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"AS Point[%6ld] ** %12.4lf %12.4lf %12.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,conPtsP->z) ;
      }
   }
/*
** Only Process If Contour Interval Is Greater Than Zero
*/
 if( conInt > 0.0 )
   {
/*
**  Initialise
*/
    conInt  = conInt * intervalRatio ;
    density = pointDensity + 1 ;
/*
**  Scan Unsmoothed Contour Points And Look For Non Consecutive Break Points
*/
    for( lofs = 0 ; lofs < numKnots - 1 ; ++lofs )
      {
/*
**     Test For Consecutive Non Break Points As Smooth Points Between Break
**     Points Have Already Been Pulled Back Onto Unsmoothed Line
*/
       if( *(knotWghtP+lofs) != 2 || *(knotWghtP+lofs+1) != 2 )
         {
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Consecutive Non Break Contour Points Detected ** Contour Value = %10.4lf",conPtsP->z) ;
             bcdtmWrite_message(0,0,0,"Point 1 = %4ld ** %10.4lf %10.4lf",lofs,*(xKnotP+lofs),*(yKnotP+lofs) ) ;
             bcdtmWrite_message(0,0,0,"Point 2 = %4ld ** %10.4lf %10.4lf",lofs+1,xKnotP[lofs+1],yKnotP[lofs+1] ) ;
            }
/*
**        Check Smooth Points For Pull Back Onto Un Smooth Contour Line
*/
          for( mofs = 0 ; mofs < density ; ++mofs )
            {
/*
**           Calculate Smooth Point Offset
*/
             ofs = lofs * density + mofs ;
             p3dP = conPtsP + ofs ;
/*
**           Project Smooth Point Back Onto Un Smoothed Contour Line
*/
             if( ! mofs ) { Xp = *(xKnotP+lofs) ; Yp = *(yKnotP+lofs) ; }
             else
               {
                dx = xKnotP[lofs+1] - *(xKnotP+lofs) ;
                if( dx == 0.0 )  Xp = *(xKnotP+lofs) ;
                else             Xp = *(xKnotP+lofs) + dx * (double)mofs / (double)density ;
                dy = yKnotP[lofs+1] - *(yKnotP+lofs) ;
                if( dy == 0.0 )  Yp = *(yKnotP+lofs) ;
                else             Yp = *(yKnotP+lofs) + dy * (double)mofs / (double)density ;
               }
/*
**           Write Points For Development Purposes Only
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Point[%8ld] ** %10.4lf %10.4lf ** Smooth = %10.4lf %10.4lf",ofs,Xp,Yp,p3dP->x,p3dP->y) ;
/*
**           Check Projected Point Is On Un Smoothed Contour Segment ** Development Only
*/
             if( cdbg )
               {
                if( ( normalDist = bcdtmMath_normalDistanceToCordLine(*(xKnotP+lofs),*(yKnotP+lofs),xKnotP[lofs+1],yKnotP[lofs+1],Xp,Yp)) > 0.00001 )
                  {
                   bcdtmWrite_message(0,0,0,"Projected Point Error %10.4lf %10.4lf ** contourValue = %10.4lf",Xp,Yp,contourValue) ;
                   bcdtmWrite_message(0,0,0,"xKnotP0 = %10.4lf yKnotP0 = %10.4lf",*(xKnotP+lofs),*(yKnotP+lofs)) ;
                   bcdtmWrite_message(0,0,0,"xKnotP1 = %10.4lf yKnotP1 = %10.4lf",*(xKnotP+lofs+1),*(yKnotP+lofs+1)) ;
                   bcdtmWrite_message(0,0,0,"lofs = %4ld mofs = %2ld xKnotP = %10.4lf yKnotP = %10.4lf normalDist = %10.4lf ** contourValue = %10.4lf",lofs,mofs,p3dP->x,p3dP->y,normalDist,contourValue) ;
                   bcdtmWrite_message(0,0,0,"Development Check Error") ;
                   goto errexit ;
                  }
               }
/*
**           If Smooth Point And Project Point Are Not Coincident Then Check For Pull Back
*/
             if( bcdtmMath_distanceSquared(Xp,Yp,p3dP->x,p3dP->y) > ppTolSquared )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Pull Back Of Smoothed Point") ;
/*
**              Binary Scan Along Line Until  fabs(contourValue-z) == ratio * conInterval
*/
                process = 1 ;
                x = Xs = p3dP->x ;
                y = Ys = p3dP->y ;
                if( bcdtmDrape_pointDtmObject(dtmP,x,y,&z,&drapeFlag)) goto errexit ;
                while ( process && (( drapeFlag == 1 && fabs( fabs(contourValue-z) - conInt ) > 0.001 ) || drapeFlag == 2 ))
                  {
                   if( drapeFlag == 2 || fabs(contourValue-z) - conInt > 0 )
                     {
                      Xs = x ;
                      Ys = y ;
                      x = ( Xp + Xs ) / 2.0 ;
                      y = ( Yp + Ys ) / 2.0 ;
                     }
                   else
                     {
                      Xp = x ;
                      Yp = y ;
                      x = ( Xp + Xs ) / 2.0 ;
                      y = ( Yp + Ys ) / 2.0 ;
                     }
                   if( bcdtmDrape_pointDtmObject(dtmP,x,y,&z,&drapeFlag)) goto errexit ;
                   if( bcdtmMath_distanceSquared (x,y,Xs,Ys) < ppTolSquared ) process = 0 ;
                   if( bcdtmMath_distanceSquared (x,y,Xp,Yp) < ppTolSquared ) process = 0 ;
                  }
                p3dP->x = x ;
                p3dP->y = y ;
               }
            }
         }
      }
/*
** Write Points After Pull Back
*/
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"After Pullback Number Of Smoothed Contour Points = %6ld",numConPts) ;
       for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] ** %12.4lf %12.4lf %12.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,conPtsP->z) ;
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
BENTLEYDTM_Private int bcdtmLoad_pullSmoothPointsBackAtKnots
(
 double x[],
 double y[],
 long weightP[],
 long numSplinePts,
 long smoothDensity,
 double contourValue,
 DPoint3d *conPtsP,
 long numConPts
)
/*
** This Function Detects And Fixes Knots In A Smoothed Contour
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,lofs,mofs,density,closeFlag=0 ;
 long   numConIntTable,numConIntPts,memConIntPts,memConIntPtsInc ;
 double dx,dy,px,py ;
 DPoint3d    *p3dP ;
 DTM_STR_INT_TAB *intTableP,*conIntTableP=NULL ;
 DTM_STR_INT_PTS *intPtsP,*conIntPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Detecting Knots In Contour ** Contour Value = %10.4lf ** Number Of Points = %6ld",contourValue,numConPts) ;
 if( dbg == 2 )
   {
   bcdtmWrite_message(0,0,0,"Number Of Contours = %6ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
      bcdtmWrite_message(0,0,0,"Contour Point[%6ld] ** %10.4lf %10.4lf %10.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Initialise
*/
 density = smoothDensity + 1 ;
 if( conPtsP->x == (conPtsP+numConPts-1)->x && conPtsP->y == (conPtsP+numConPts-1)->y ) closeFlag = 1 ;
/*
** Create And Build Contour Intersection Table
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Building Contour Intersection Table") ;
 if( bcdtmLoad_buildContourIntersectionTable(conPtsP,numConPts,&conIntTableP,&numConIntTable)) goto errexit ;
 if( dbg )bcdtmWrite_message(0,0,0,"Number Of Contour Intersection Table Entries = %6ld",numConIntTable) ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
   bcdtmWrite_message(0,0,0,"Number Of Contour Intersection Table Entries = %6ld",numConIntTable ) ;
    for( intTableP = conIntTableP ; intTableP < conIntTableP + numConIntTable ; ++intTableP )
      {
      bcdtmWrite_message(0,0,0,"Entry[%4ld] ** Contour = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(intTableP-conIntTableP),intTableP->String,intTableP->Segment,intTableP->Type,intTableP->Direction,intTableP->X1,intTableP->Y1,intTableP->Z1,intTableP->X2,intTableP->Y2,intTableP->Z2) ;
      }
   }
/*
** Scan For Intersections
*/
 memConIntPtsInc = numConIntTable * 2 ;
 if( memConIntPtsInc < 1000 ) memConIntPtsInc = 1000 ;
 numConIntPts = memConIntPts = 0 ;
 if( dbg )bcdtmWrite_message(0,0,0,"Scanning For Contour Intersections") ;
 if( bcdtmLoad_scanForContourIntersections(conIntTableP,numConIntTable,&conIntPtsP,&numConIntPts,&memConIntPts,memConIntPtsInc) ) goto errexit ;
 if( dbg )bcdtmWrite_message(0,0,0,"Number Of Contour Knots = %4ld",numConIntPts) ;
/*
** Process Intersection Points
*/
 if( numConIntPts > 0 )
   {
/*
** Sort Intersection Points
*/
    if( dbg )bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
    qsort(conIntPtsP,numConIntPts,sizeof(DTM_STR_INT_PTS),(int(*)(const void*, const void*))bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
    if( dbg )bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",numConIntPts) ;
/*
**  Write Intersection Points
*/
    if( dbg == 2  )
      {
      bcdtmWrite_message(0,0,0,"Knots Detected In Contour ** Contour Value = %10.4lf ** Number Of Points = %6ld",contourValue,numConPts) ;
      bcdtmWrite_message(0,0,0,"Number Of Knots = %6ld",numConIntPts) ;
       for( intPtsP = conIntPtsP ; intPtsP < conIntPtsP + numConIntPts ; ++intPtsP )
         {
         bcdtmWrite_message(0,0,0,"Int Point[%5ld] ** Str1 = %4ld Seg1 = %7ld Str2 = %4ld Seg2 = %7ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf Z2 = %10.4lf",(long)(intPtsP-conIntPtsP),intPtsP->String1,intPtsP->Segment1,intPtsP->String2,intPtsP->Segment2,intPtsP->Distance,intPtsP->x,intPtsP->y,intPtsP->z,intPtsP->Z2) ;
           }
      }
/*
**  Pull Back All Points On Smoothed Contour Segments
*/
    for( intPtsP = conIntPtsP ; intPtsP < conIntPtsP + numConIntPts ; intPtsP = intPtsP + 2 )
      {
       if( ! closeFlag || intPtsP->Segment1 != 0 || intPtsP->Distance != 0.0 )
         {
/*
**        Pull Back Smooth Points On Segment 1
*/
          lofs = intPtsP->Segment1 / density ;
          for( mofs = 0 ; mofs < density ; ++mofs )
            {
/*
**           Calculate Smooth Point Offset
*/
             ofs = lofs * density + mofs ;
/*
**           Project Smooth Point Back
*/
             if( ! mofs ) { px = x[lofs] ; py = y[lofs] ; }
             else
               {
                dx = x[lofs+1] - x[lofs] ;
                if( dx == 0.0 )  px = x[lofs] ;
                else             px = x[lofs] + dx * (double)mofs / (double)density ;
                dy = y[lofs+1] - y[lofs] ;
                if( dy == 0.0 )  py = y[lofs] ;
                else             py = y[lofs] + dy * (double)mofs / (double)density ;
               }
             (conPtsP+ofs)->x = px ;
             (conPtsP+ofs)->y = py ;
            }
/*
**        Pull Back Smooth Points On Segment 2
*/
          lofs = intPtsP->Segment2 / density ;
          for( mofs = 0 ; mofs <= density ; ++mofs )
            {
/*
**           Calculate Smooth Point Offset
*/
             ofs = lofs * density + mofs ;
/*
**           Project Smooth Point Back
*/
             if( ! mofs ) { px = x[lofs] ; py = y[lofs] ; }
             else
               {
                dx = x[lofs+1] - x[lofs] ;
                if( dx == 0.0 )  px = x[lofs] ;
                else             px = x[lofs] + dx * (double)mofs / (double)density ;
                dy = y[lofs+1] - y[lofs] ;
                if( dy == 0.0 )  py = y[lofs] ;
                else             py = y[lofs] + dy * (double)mofs / (double)density ;
               }
             (conPtsP+ofs)->x = px ;
             (conPtsP+ofs)->y = py ;
            }
         }
      }
   }
/*
** Write Pull Backed Contour Points
*/
 if( dbg == 2 )
   {
   bcdtmWrite_message(0,0,0,"Number Contours = %6ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
      bcdtmWrite_message(0,0,0,"Contour Point[%6ld] ** %10.4lf %10.4lf %10.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** CleanUp
*/
 cleanup :
 if( conIntTableP != NULL ) free(conIntTableP) ;
 if( conIntPtsP   != NULL ) free(conIntPtsP) ;
/*
** Normal Exit
*/
 if( dbg && ! ret )bcdtmWrite_message(0,0,0,"Intersecting Contours Completed") ;
 if( dbg &&   ret )bcdtmWrite_message(0,0,0,"Intersecting Contours Error") ;
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
BENTLEYDTM_Private int  bcdtmLoad_buildContourIntersectionTable(DPoint3d *conPtsP,long numConPts,DTM_STR_INT_TAB **conIntTablePP,long *numConIntTableP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   memConIntTable,memConIntTableInc=1000  ;
 double cord ;
 DPoint3d   *p3d1P,*p3d2P ;
 DTM_STR_INT_TAB *intP ;
/*
** Write Entry Message
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Building Contour Intersection Table") ;
/*
** Initialise
*/
 *numConIntTableP = memConIntTable = 0 ;
 if( *conIntTablePP != NULL ) { free(*conIntTablePP) ; *conIntTablePP = NULL ; }
 memConIntTableInc = numConPts - 1  ;
/*
** Store Contour Segments In Intersection Table
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( p3d1P = conPtsP ; p3d1P < conPtsP + numConPts - 1 ; ++p3d1P )
   {
    p3d2P = p3d1P + 1 ;
/*
**  Check For Memory Allocation
*/
    if( *numConIntTableP == memConIntTable )
      {
       memConIntTable = memConIntTable + memConIntTableInc ;
       if( *conIntTablePP == NULL ) *conIntTablePP = ( DTM_STR_INT_TAB * ) malloc ( memConIntTable * sizeof(DTM_STR_INT_TAB)) ;
       else                         *conIntTablePP = ( DTM_STR_INT_TAB * ) realloc ( *conIntTablePP,memConIntTable * sizeof(DTM_STR_INT_TAB)) ;
       if( *conIntTablePP == NULL ) {bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store
*/
    (*conIntTablePP+*numConIntTableP)->String  = 1 ;
    (*conIntTablePP+*numConIntTableP)->Segment = (long)(p3d1P-conPtsP) ;
    (*conIntTablePP+*numConIntTableP)->Type    = 1   ;
    (*conIntTablePP+*numConIntTableP)->Direction = 1 ;
    (*conIntTablePP+*numConIntTableP)->X1 = p3d1P->x ;
    (*conIntTablePP+*numConIntTableP)->Y1 = p3d1P->y ;
    (*conIntTablePP+*numConIntTableP)->Z1 = p3d1P->z ;
    (*conIntTablePP+*numConIntTableP)->X2 = p3d2P->x ;
    (*conIntTablePP+*numConIntTableP)->Y2 = p3d2P->y ;
    (*conIntTablePP+*numConIntTableP)->Z2 = p3d2P->z ;
    ++*numConIntTableP ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *numConIntTableP < memConIntTable ) *conIntTablePP = ( DTM_STR_INT_TAB * ) realloc ( *conIntTablePP, *numConIntTableP * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( intP = *conIntTablePP ; intP < *conIntTablePP + *numConIntTableP ; ++intP )
   {
    if( intP->X1 > intP->X2 || ( intP->X1 == intP->X2 && intP->Y1 > intP->Y2 ) )
      {
       intP->Direction = 2 ;
       cord = intP->X1 ; intP->X1 = intP->X2 ; intP->X2 = cord ;
       cord = intP->Y1 ; intP->Y1 = intP->Y2 ; intP->Y2 = cord ;
       cord = intP->Z1 ; intP->Z1 = intP->Z2 ; intP->Z2 = cord ;
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsort(*conIntTablePP,*numConIntTableP,sizeof(DTM_STR_INT_TAB),(int(*)(const void*, const void*))bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )bcdtmWrite_message(0,0,0,"Building Contour Intersection Table Completed") ;
 if( dbg && ret != DTM_SUCCESS )bcdtmWrite_message(0,0,0,"Building Contour Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *numConIntTableP = 0 ;
 if( *conIntTablePP != NULL ) { free(*conIntTablePP) ; *conIntTablePP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_scanForContourIntersections(DTM_STR_INT_TAB *conIntTableP,long numConIntTable,DTM_STR_INT_PTS **conIntPtsPP,long *nunConIntPtsP,long *memConIntPtsP,long memConIntPtsInc)
/*
** This Function Scans for Radial Intersections
*/
{
 int     ret=DTM_SUCCESS ;
 long    numActConIntTable=0,memActConIntTable=0 ;
 DTM_STR_INT_TAB *intTabP,*actConIntTableP=NULL ;
/*
** Scan Sorted Contour Intersection Table and Look For Intersections
*/
 for( intTabP = conIntTableP ; intTabP < conIntTableP + numConIntTable  ; ++intTabP)
   {
    if( bcdtmClean_deleteActiveStringLines(actConIntTableP,&numActConIntTable,intTabP)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&actConIntTableP,&numActConIntTable,&memActConIntTable,intTabP))  goto errexit ;
    if( bcdtmLoad_determineContourIntersections(actConIntTableP,numActConIntTable,conIntPtsPP,nunConIntPtsP,memConIntPtsP,memConIntPtsInc)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( actConIntTableP != NULL ) { free(actConIntTableP) ; actConIntTableP = NULL ; }
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
BENTLEYDTM_Private int bcdtmLoad_determineContourIntersections(DTM_STR_INT_TAB *actIntTableP,long numActIntTable,DTM_STR_INT_PTS **intPtsPP,long *numIntPtsP,long *memIntPtsP,long memIntPtsInc )
/*
** Determine Line Intersections
*/
{
 int              ret=DTM_SUCCESS ;
 double           di,dl,dz,xs=0.0,ys=0.0,zs=0.0,xe=0.0,ye=0.0,ze=0.0,x,y ;
 DTM_STR_INT_TAB  *activeP,*scanP ;
/*
** Initialise
*/
 activeP = actIntTableP + numActIntTable - 1 ;
/*
** Scan Active Line List
*/
 for( scanP = actIntTableP ; scanP < actIntTableP + numActIntTable - 1 ; ++scanP )
   {
/*
**  Do Not Test Consecutive Segments
*/
    if( labs(scanP->Segment-activeP->Segment) > 1 )
      {
/*
**     Check Lines Intersect
*/
       if(  bcdtmMath_checkIfLinesIntersect(scanP->X1,scanP->Y1,scanP->X2,scanP->Y2,activeP->X1,activeP->Y1,activeP->X2,activeP->Y2))
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
             *memIntPtsP = *memIntPtsP + memIntPtsInc ;
             if( *intPtsPP == NULL ) *intPtsPP = ( DTM_STR_INT_PTS * ) malloc ( *memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             else                  *intPtsPP = ( DTM_STR_INT_PTS * ) realloc( *intPtsPP,*memIntPtsP * sizeof(DTM_STR_INT_PTS)) ;
             if( *intPtsPP == NULL ) {bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
**        Calculate Distances For Active Line
*/
          if( activeP->Direction == 1 ) { xs = activeP->X1 ; ys = activeP->Y1 ; zs = activeP->Z1 ; xe = activeP->X2 ; ye = activeP->Y2 ; ze = activeP->Z2 ; }
          if( activeP->Direction == 2 ) { xs = activeP->X2 ; ys = activeP->Y2 ; zs = activeP->Z2 ; xe = activeP->X1 ; ye = activeP->Y1 ; ze = activeP->Z1 ; }
          dz = ze - zs ;
          di = bcdtmMath_distance(xs,ys,x,y) ;
          dl = bcdtmMath_distance(xs,ys,xe,ye) ;
/*
**        Store Intersection Point Active Line
*/
          (*intPtsPP+*numIntPtsP)->String1  = activeP->String  ;
          (*intPtsPP+*numIntPtsP)->Segment1 = activeP->Segment ;
          (*intPtsPP+*numIntPtsP)->String2  = scanP->String  ;
          (*intPtsPP+*numIntPtsP)->Segment2 = scanP->Segment ;
          (*intPtsPP+*numIntPtsP)->Distance = di ;
          (*intPtsPP+*numIntPtsP)->x = x ;
          (*intPtsPP+*numIntPtsP)->y = y ;
          (*intPtsPP+*numIntPtsP)->z = zs + dz * di / dl ;
          ++*numIntPtsP ;
/*
**        Calculate Distances For Scan Line
*/
          if( scanP->Direction == 1 ) { xs = scanP->X1 ; ys = scanP->Y1 ; zs = scanP->Z1 ; xe = scanP->X2 ; ye = scanP->Y2 ; ze = scanP->Z2 ; }
          if( scanP->Direction == 2 ) { xs = scanP->X2 ; ys = scanP->Y2 ; zs = scanP->Z2 ; xe = scanP->X1 ; ye = scanP->Y1 ; ze = scanP->Z1 ; }
          dz = ze - zs ;
          di = bcdtmMath_distance(xs,ys,x,y) ;
          dl = bcdtmMath_distance(xs,ys,xe,ye) ;
/*
**        Store Intersection Point For Scan Line
*/
          (*intPtsPP+*numIntPtsP)->String1  = scanP->String    ;
          (*intPtsPP+*numIntPtsP)->Segment1 = scanP->Segment   ;
          (*intPtsPP+*numIntPtsP)->String2  = activeP->String  ;
          (*intPtsPP+*numIntPtsP)->Segment2 = activeP->Segment ;
          (*intPtsPP+*numIntPtsP)->Distance = di ;
          (*intPtsPP+*numIntPtsP)->x = x ;
          (*intPtsPP+*numIntPtsP)->y = y ;
          (*intPtsPP+*numIntPtsP)->z = zs + dz * di / dl ;
          ++*numIntPtsP ;
/*
**        Store z2 Values
*/
          (*intPtsPP+*numIntPtsP-2)->Z2 = (*intPtsPP+*numIntPtsP-1)->z ;
          (*intPtsPP+*numIntPtsP-1)->Z2 = (*intPtsPP+*numIntPtsP-2)->z ;
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
 if( ret == DTM_SUCCESS )  ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Load Contours From A DTM Lattice File
* @doc    Load Contours From A DTM Lattice File
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @param  latticeFile           ==> Lattice File Name
* @param  conInt                ==> Contour Interval
* @param  conReg                ==> Contour Registration
* @param  loadRange             ==> Load Contour Range <TRUE,FALSE>
* @param  conMin                ==> Contour Range Minimum Value
* @param  conMax                ==> Contour Range Maximum Value
* @param  loadValues            ==> Load Contour Values
* @param  conValuesP            ==> Contour Values
* @param  numConValues          ==> Number Of Contour Values
* @param  smoothOption          ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2)>
* @param  smoothFactor          ==> Smoothing Factor
* @param  smoothDensity         ==> Point Densification For Spline Smoothing
* @param  loadFunctionP         ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence <TRUE,FALSE>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> P3D Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack 23 November 2004  rob.cormack@bentley.con
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_contoursFromLatticeFile
(
 WCharCP latticeFile,        /* ==> Lattice File Name                     */
 double  conInt,              /* ==> Contour Interval                      */
 double  conReg,              /* ==> Contour Registration                  */
 long    loadRange,           /* ==> Load Contour Range <TRUE,FALSE>       */
 double  conMin,              /* ==> Contour Range Minimum Value           */
 double  conMax,              /* ==> Contour Range Maximum Value           */
 long    loadValues,          /* ==> Load Contour Values                   */
 double  *conValuesP,         /* ==> Contour Values                        */
 long    numConValues,        /* ==> Number Of Contour Values              */
 DTMContourSmoothing smoothOption, /* ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2)> */
 double  smoothFactor,        /* ==> Contour Smoothing Factor              */
 long    smoothDensity,       /* ==> Point Densification For Spline Smoothing            */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function              */
 long    useFence,            /* ==> Load Feature Within Fence<TRUE,FALSE> */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>   */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points             */
 long    numFencePts,         /* ==> Number Of Fence Points                */
 void    *userP               /* ==> User Pointer Passed Back To User      */
)
/*
** This Function Loads Contours From A Tin Object
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_LAT_OBJ  *latticeP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Contours From Lattice File %s",latticeFile) ;
/*
** Test If Requested Tin Is Current Tin
*/
 if( bcdtmUtl_testForAndSetCurrentLatticeObject(&latticeP,latticeFile)) goto errexit ;
/*
** Load Dtm Feature From Tin Object
*/
 if( bcdtmLoad_contoursFromLatticeObject(latticeP,conInt,conReg,loadRange,conMin,conMax,loadValues,conValuesP,numConValues,smoothOption,smoothFactor,smoothDensity,loadFunctionP,useFence,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Lattice File %s Completed",latticeFile) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contours From Lattice File %s Error",latticeFile) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Load Contours From A DTM Lattice Object
* @doc    Load Contours From A DTM Lattice Object
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @author Rob Cormack 23 November 2004  rob.cormack@bentley.con
* @param  *latticeP             ==> Pointer To Lattice Object
* @param  conInt                ==> Contour Interval
* @param  conReg                ==> Contour Registration
* @param  loadRange             ==> Load Contour Range <TRUE,FALSE>
* @param  conMin                ==> Contour Range Minimum Value
* @param  conMax                ==> Contour Range Maximum Value
* @param  loadValues            ==> Load Contour Values
* @param  *conValuesP           ==> Contour Values
* @param  numConValues          ==> Number Of Contour Values
* @param  smoothOption          ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2)>
* @param  smoothFactor          ==> Smoothing Factor
* @param  smoothDensity         ==> Point Densification For Spline Smoothing
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence <TRUE,FALSE>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> P3D Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_contoursFromLatticeObject
(
DTM_LAT_OBJ  *latticeP,      /* ==> Pointer to Lattice object             */
double  conInt,              /* ==> Contour Interval                      */
double  conReg,              /* ==> Contour Registration                  */
long    loadRange,           /* ==> Load Contour Range <TRUE,FALSE>       */
double  conMin,              /* ==> Contour Range Minimum Value           */
double  conMax,              /* ==> Contour Range Maximum Value           */
long    loadValues,          /* ==> Load Contour Values <TRUE,FALSE>      */
double  *conValuesP,         /* ==> Contour Values                        */
long    numConValues,        /* ==> Number Of Contour Values              */
DTMContourSmoothing smoothOption, /* ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2)> */
double  smoothFactor,        /* ==> Contour Smoothing Factor              */
long    smoothDensity,       /* ==> Point Densification For Spline Smoothing            */
DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function              */
long    useFence,            /* ==> Load Feature Within Fence<TRUE,FALSE> */
DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>   */
DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points             */
long    numFencePts,         /* ==> Number Of Fence Points                */
void    *userP               /* ==> User Pointer Passed Back To User      */
)
/*
** This Function Loads Contours From A Lattice Object
*/
    {
    int     ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long    i, j, p1, p2, p3, latOfs, intOffset, insideFence;
    long    startTime = 0, finishTime = 0, fndType, numInside = 0;
    double  x, y, *dblP, firstContour, lastContour, contourValue, zClipMin, zClipMax;
    unsigned char    *cP, *maskLat1P = NULL, *maskLat2P = NULL;
    BC_DTM_OBJ  *clipDtmP = NULL;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMContourParams contourParams;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        startTime = bcdtmClock ();
        bcdtmWrite_message (0, 0, 0, "Loading Contours From Lattice Object");
        bcdtmWrite_message (0, 0, 0, "Lattice Object         = %p", latticeP);
        bcdtmWrite_message (0, 0, 0, "Contour Interval       = %8.3lf", conInt);
        bcdtmWrite_message (0, 0, 0, "Contour Registration   = %8.3lf", conReg);
        bcdtmWrite_message (0, 0, 0, "Load Contour Range     = %8ld", loadRange);
        bcdtmWrite_message (0, 0, 0, "Contour Min            = %8.3lf", conMin);
        bcdtmWrite_message (0, 0, 0, "Contour Max            = %8.3lf", conMax);
        bcdtmWrite_message (0, 0, 0, "Load Contour Values    = %8ld", loadValues);
        bcdtmWrite_message (0, 0, 0, "Contour Values Pointer = %p", conValuesP);
        bcdtmWrite_message (0, 0, 0, "Number Contour Values  = %8ld", numConValues);
        bcdtmWrite_message (0, 0, 0, "Smooth Option          = %8ld", smoothOption);
        bcdtmWrite_message (0, 0, 0, "Smooth Factor          = %8.2lf", smoothFactor);
        bcdtmWrite_message (0, 0, 0, "Smooth Density         = %8ld", smoothDensity);
        bcdtmWrite_message (0, 0, 0, "Load Function          = %p", loadFunctionP);
        bcdtmWrite_message (0, 0, 0, "Use Fence              = %8ld", useFence);
        bcdtmWrite_message (0, 0, 0, "Fence Option           = %8ld", fenceOption);
        bcdtmWrite_message (0, 0, 0, "Fence Points           = %p", fencePtsP);
        bcdtmWrite_message (0, 0, 0, "Num Fence Points       = %8ld", numFencePts);
        bcdtmWrite_message (0, 0, 0, "User Pointer           = %p", userP);
        }
    /*
    ** Test For Valid Tin Object
    */
    if (bcdtmObject_testForValidLatticeObject (latticeP)) goto errexit;
    /*
    ** Validate Contour Value Range
    */
    if (conMin != conMax)
        {
        if (conMin < latticeP->LZMIN) conMin = latticeP->LZMIN;
        if (conMax > latticeP->LZMAX) conMax = latticeP->LZMAX;
        }
    if (conMin >= conMax)
        {
        conMin = latticeP->LZMIN;
        conMax = latticeP->LZMAX;
        }
    /*
    ** Allocate Memory For Marking Null Lattice Points
    */
    maskLat1P = (unsigned char *)malloc (latticeP->NOLATPTS / 8 + 1);
    if (maskLat1P == NULL)
        {
        bcdtmWrite_message (1, 0, 0, "Memory Allocation Failure");
        goto errexit;
        }
    for (cP = maskLat1P; cP < maskLat1P + latticeP->NOLATPTS / 8 + 1; ++cP) *cP = 0;
    /*
    ** Mask Null Lattice Points
    */
    for (i = 0; i < latticeP->NYL - 1; ++i)
        {
        for (j = 0; j < latticeP->NXL - 1; ++j)
            {
            latOfs = j*latticeP->NYL + i;
            if (*(latticeP->LAT + latOfs) == latticeP->NULLVAL)
                {
                bcdtmFlag_setFlag (maskLat1P, latOfs);
                }
            }
        }
    /*
    ** Allocate Memory For Copy Of Marked Null Lattice Points
    */
    maskLat2P = (unsigned char *)malloc (latticeP->NOLATPTS / 8 + 1);
    if (maskLat2P == NULL)
        {
        bcdtmWrite_message (1, 0, 0, "Memory Allocation Failure");
        goto errexit;
        }
    memcpy (maskLat2P, maskLat1P, latticeP->NOLATPTS / 8 + 1);
    /*
    ** Build Clipping Tin For Fence Operations
    */
    if (useFence == TRUE)
        {
        /*
        ** Validate Fence
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Validating Fence");
        if (useFence == TRUE && (fencePtsP == NULL || numFencePts <= 2)) useFence = FALSE;
        if (useFence == TRUE && (fencePtsP->x != (fencePtsP + numFencePts - 1)->x || fencePtsP->y != (fencePtsP + numFencePts - 1)->y)) useFence = FALSE;
        /*
        **  Build Clipping Tin For Fence
        */
        if (useFence == TRUE)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Building Clipping Tin");
            if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fencePtsP, numFencePts)) goto errexit;
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "Fence Range ** xMin = %12.5lf xMax = %12.5lf", clipDtmP->xMin, clipDtmP->xMax);
                bcdtmWrite_message (0, 0, 0, "            ** yMin = %12.5lf yMax = %12.5lf", clipDtmP->yMin, clipDtmP->yMax);
                }
            /*
            **     Initialise z Ranges For Clipping
            */
            zClipMin = latticeP->LZMAX;
            zClipMax = latticeP->LZMIN;
            /*
            **     Mask Lattice Points External To Clipping Tin
            */
            for (i = 0; i < latticeP->NYL - 1; ++i)
                {
                for (j = 0; j < latticeP->NXL - 1; ++j)
                    {
                    latOfs = j*latticeP->NYL + i;
                    if (*(latticeP->LAT + latOfs) != latticeP->NULLVAL)
                        {
                        x = latticeP->DX * i + latticeP->LXMIN;
                        y = latticeP->DY * j + latticeP->LYMIN;
                        insideFence = TRUE;
                        if (x < clipDtmP->xMin || x > clipDtmP->xMax) insideFence = FALSE;
                        else if (y < clipDtmP->yMin || y > clipDtmP->yMax) insideFence = FALSE;
                        else
                            {
                            if (bcdtmFind_triangleDtmObject (clipDtmP, x, y, &fndType, &p1, &p2, &p3)) goto errexit;
                            if (!fndType) insideFence = FALSE;
                            }
                        if (insideFence == FALSE)
                            {
                            ++numInside;
                            bcdtmFlag_setFlag (maskLat2P, latOfs);
                            }
                        else
                            {
                            if (*(latticeP->LAT + latOfs) < zClipMin) zClipMin = *(latticeP->LAT + latOfs);
                            if (*(latticeP->LAT + latOfs) > zClipMax) zClipMax = *(latticeP->LAT + latOfs);
                            }
                        }
                    }
                }
            /*
            **      Reset Min Max Contour Range
            */
            if (zClipMin > conMin) conMin = zClipMin;
            if (zClipMax < conMax) conMax = zClipMax;
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "numInside = %8ld", numInside);
                bcdtmWrite_message (0, 0, 0, "conMin = %10.4lf conMax = %10.4lf", conMin, conMax);
                }
            }
        }
    /*
    ** Setup contour Params
    */
    contourParams.interval = conInt;
    contourParams.conReg = conReg;
    contourParams.loadRange = loadRange != 0;
    contourParams.conMin = conMin;
    contourParams.conMax = conMax;
    contourParams.loadValues = loadValues;
    contourParams.conValuesP = conValuesP;
    contourParams.numConValues = numConValues;
    contourParams.smoothOption = smoothOption;
    contourParams.smoothFactor = smoothFactor;
    contourParams.smoothDensity = smoothDensity;
        {
        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMFenceParams fenceParams (useFence ? DTMFenceType::Block : DTMFenceType::None, fenceOption, fencePtsP, numFencePts);
        /*
       ** Load Contour Range
       */
        if (loadRange == TRUE)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Range Contours");
            intOffset = (long)((conMin - conReg) / conInt);
            firstContour = conReg + ((double)intOffset) * conInt;
            intOffset = (long)((conMax - conReg) / conInt) + 1;
            lastContour = conReg + ((double)intOffset) * conInt;
            for (contourValue = firstContour; contourValue <= lastContour; contourValue = contourValue + conInt)
                {
                contourValue = bcdtmMath_roundToDecimalPoints (contourValue, 8);
                if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Contour %10.4lf", contourValue);
                memcpy (maskLat1P, maskLat2P, latticeP->NOLATPTS / 8 + 1);
                if (bcdtmLoad_plotContourLatticeObject (latticeP, contourParams, fenceParams, nullptr, clipDtmP, maskLat1P, (float)contourValue, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
                }
            }
        /*
        ** Load Contours Values
        */
        if (loadValues == TRUE)
            {
            conInt = 0.0;
            if (dbg) bcdtmWrite_message (0, 0, 0, "Loading Contour Values");
            for (dblP = conValuesP; dblP < conValuesP + numConValues; ++dblP)
                {
                contourValue = bcdtmMath_roundToDecimalPoints (*dblP, 8);
                memcpy (maskLat1P, maskLat2P, latticeP->NOLATPTS / 8 + 1);
                if (bcdtmLoad_plotContourLatticeObject (latticeP, contourParams, fenceParams, nullptr, clipDtmP, maskLat1P, (float)contourValue, loadFunctionP, userP) != DTM_SUCCESS) goto errexit;
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup:
    if (maskLat1P != NULL) { free (maskLat1P); maskLat1P = NULL; }
    if (maskLat2P != NULL) { free (maskLat2P); maskLat2P = NULL; }
    if (clipDtmP != NULL) bcdtmObject_destroyDtmObject (&clipDtmP);
    /*
    ** Write Departing Message
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Loading Contours From Lattice Object Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Loading Contours From Lattice Object Error");
    if (dbg)
        {
        finishTime = bcdtmClock ();
        bcdtmWrite_message (0, 0, 0, "Time To Load Contours = %7.3lf seconds", bcdtmClock_elapsedTime (finishTime, startTime));
        }
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
BENTLEYDTM_Private int bcdtmLoad_plotContourLatticeObject
(
 DTM_LAT_OBJ *latticeP,     /* ==> Pointer to Lattice object                              */
 TerrainModel::DTMContourParamsCR contourParams, /* ==> contour Params                                    */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDtmP,
 unsigned char   *markLatP,          /* ==> Pointer To Marked Lattice Points                       */
 float  contourValue,       /* ==> Contour Value                                          */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function                               */
 void   *userP              /* ==> User Pointer Passed Back To User                       */
)
/*
** This Function Determines The Starting Point For Tracing Contours
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  direction ;
 long  i,j,ofs1,ofs2,ofs ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting %10.4f Contour",contourValue) ;
/*
** Scan Lines Parallel To x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Lattice x Lines") ;
 for( j = 0 ; j < latticeP->NXL ; ++j )
   {
    for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
      {
       direction = 4 ;
       ofs1 = j * latticeP->NYL + i ;
       ofs2 = ofs1 + 1 ;
       if( ! bcdtmFlag_testFlag(markLatP,ofs1) && ! bcdtmFlag_testFlag(markLatP,ofs2) )
         {
          if     ( contourValue > *(latticeP->LAT+ofs1) && contourValue <= *(latticeP->LAT+ofs2)) direction = 1 ;
          else if( contourValue > *(latticeP->LAT+ofs2) && contourValue <= *(latticeP->LAT+ofs1)) direction = 2 ;
          if( direction != 4 )
            {
             if( direction == 2 ) { ofs = ofs2 ; ofs2 = ofs1 ; ofs1 = ofs ;}
               {
               if (direction == 1)
                   {
                   if (bcdtmLoad_traceContourLatticeObject (latticeP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, markLatP, contourValue, i, j, i + 1, j, loadFunctionP, userP)) goto errexit;
                   }
               else if (direction == 2)
                   {
                   if (bcdtmLoad_traceContourLatticeObject (latticeP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, markLatP, contourValue, i + 1, j, i, j, loadFunctionP, userP)) goto errexit;
                   }
               }
            }
         }
      }
   }
/*
** Scan Lines Parallel To y Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Lattice y Lines") ;
 for( i = 0 ; i < latticeP->NYL  ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       direction  = 4 ;
       ofs1 = j * latticeP->NYL + i ;
       ofs2 = ofs1 + latticeP->NYL  ;
       if( ! bcdtmFlag_testFlag(markLatP,ofs1) && ! bcdtmFlag_testFlag(markLatP,ofs2) )
         {
          if     ( contourValue > *(latticeP->LAT+ofs1) && contourValue <= *(latticeP->LAT+ofs2)) direction = 1 ;
          else if( contourValue > *(latticeP->LAT+ofs2) && contourValue <= *(latticeP->LAT+ofs1)) direction = 2 ;
          if( direction != 4 )
            {
             if( direction == 2 ) { ofs = ofs2 ; ofs2 = ofs1 ; ofs1 = ofs ; }
               {
               if (direction == 1)
                   {
                   if (bcdtmLoad_traceContourLatticeObject (latticeP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, markLatP, contourValue, i, j, i, j + 1, loadFunctionP, userP)) goto errexit;
                   }
               else if (direction == 2)
                   {
                   if (bcdtmLoad_traceContourLatticeObject (latticeP, contourParams, fenceParams, pondExtendedAppData, clipDtmP, markLatP, contourValue, i, j + 1, i, j, loadFunctionP, userP)) goto errexit;
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
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting %10.4f Contour Completed",contourValue) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting %10.4f Contour Error",contourValue) ;
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
BENTLEYDTM_Private int bcdtmLoad_traceContourLatticeObject
(
 DTM_LAT_OBJ *latticeP,     /* ==> Pointer To Lattice Object        */
 TerrainModel::DTMContourParamsCR contourParams, /* ==> Contour Params */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDtmP,
 unsigned char *markLatP,            /* ==> Pointer To Marked Lattice Points */
 float contourValue,        /* ==> Contour Value To Be Traced       */
 long i1,                   /* ==> Start Row                        */
 long j1,                   /* ==> Start Column                     */
 long i2,                   /* ==> Next Row                         */
 long j2,                   /* ==> Next Column                      */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                   */
 void *userP                /* ==> User Pointer Passed Back To User           */
)
/*
** This Function Traces Contours
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    scan, traced, weight = 0, i3 = 0, j3 = 0, li1, li2, lj1, lj2, contourDirection = 1,direction;
 long    ofs1,ofs2,ofs3,isp1,isp2 ;
 double  ra,xs=0.0,ys=0.0,xc,yc,xlc=0.0,ylc=0.0   ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Tracing %10.4f Contour",contourValue) ;
/*
** Set Starting Conditions
*/
 li1 = i1 ;
 lj1 = j1 ;
 li2 = i2 ;
 lj2 = j2 ;
 direction = 0 ;
/*
** Scan Twice Through Lattice.
** Scan To lattice Edge And Then Reverse Scan Drawing Contour
*/
 for ( scan = 0 ; scan < 2 ; ++scan )
   {
    ofs1 = j1 * latticeP->NYL + i1 ;
    ofs2 = j2 * latticeP->NYL + i2 ;
    isp1 = ofs1 ;
    isp2 = ofs2 ;
    if( scan )
      {
       ra = ( contourValue - *(latticeP->LAT+ofs1) ) / ( *(latticeP->LAT+ofs2) - *(latticeP->LAT+ofs1) ) ;
       xs = latticeP->DX * i1 + ( latticeP->DX * i2 -  latticeP->DX * i1 ) * ra ;
       ys = latticeP->DY * j1 + ( latticeP->DY * j2 -  latticeP->DY * j1 ) * ra ;
       if( bcdtmLoad_storeContourPoint(1,xs+latticeP->LXMIN,ys+latticeP->LYMIN,contourValue,weight)) goto errexit  ;
       bcdtmFlag_setFlag(markLatP,ofs1) ;
       bcdtmFlag_setFlag(markLatP,ofs2) ;
       xlc = xs ; ylc = ys ;
      }
    traced = 0 ;
/*
** Get Next Point Around P(i1,j1)
*/
    while ( ! traced  )
      {
       li1 = i1 ;
       lj1 = j1 ;
       li2 = i2 ;
       lj2 = j2    ;
       if( direction == 0 ) bcdtmLoad_getNextClock(i1,j1,i2,j2,&i3,&j3) ;
       if( direction == 1 ) bcdtmLoad_getNextAnt(i1,j1,i2,j2,&i3,&j3) ;
       ofs3 = j3 * latticeP->NYL + i3 ;
       if ( i3 < 0 || i3 >= latticeP->NYL ||  j3 < 0 || j3 >= latticeP->NXL ) traced = 1 ;
       else if( *(latticeP->LAT + ofs3) == latticeP->NULLVAL ) traced = 1 ;
       while ( ! traced && ( contourValue <= *(latticeP->LAT+ofs1) || contourValue > *(latticeP->LAT+ofs3)) )
         {
          i2 = i1 ; j2 = j1 ; i1 = i3 ; j1 = j3 ;
          ofs2 = ofs1 ; ofs1 = ofs3 ;
          if( direction == 0 ) bcdtmLoad_getNextClock(i1,j1,i2,j2,&i3,&j3) ;
          if( direction == 1 ) bcdtmLoad_getNextAnt(i1,j1,i2,j2,&i3,&j3) ;
          ofs3 = j3 * latticeP->NYL + i3 ;
          if( i3 < 0 || i3 >= latticeP->NYL || j3 < 0 || j3 >= latticeP->NXL ) traced = 1 ;
          else if( *(latticeP->LAT + ofs3) == latticeP->NULLVAL   )            traced = 1 ;
         }
       if( ! traced )
         {
          i2 = i3 ;
          j2 = j3 ;
          ofs2 = ofs3 ;
         }
       else
         {
          i1 = li1 ;
          j1 = lj1 ;
          i2 = li2 ;
          j2 = lj2 ;
         }
/*
**     Test for end of closed contour
*/
       if( ! traced && isp1 == ofs1 && isp2 == ofs2 )
         {
          traced = 1 ;
          if(scan)
            {
             if( bcdtmLoad_storeContourPoint(1,xs+latticeP->LXMIN,ys+latticeP->LYMIN,contourValue,weight)) goto errexit  ;
            }
         }
/*
**     Set Flag Pointer
*/
       if( scan )
         {
          bcdtmFlag_setFlag(markLatP,ofs1) ;
          bcdtmFlag_setFlag(markLatP,ofs2) ;
         }
/*
**     Calculate Intersection Point
*/
       if( scan && ! traced )
         {
          ra = ( contourValue - *(latticeP->LAT+ofs1) ) / ( *(latticeP->LAT+ofs2) - *(latticeP->LAT+ofs1) ) ;
          xc = latticeP->DX * i1 + ( latticeP->DX * i2 -  latticeP->DX * i1 ) * ra ;
          yc = latticeP->DY * j1 + ( latticeP->DY * j2 -  latticeP->DY * j1 ) * ra ;
          if( xc != xlc || yc != ylc )
            {
             if( bcdtmLoad_storeContourPoint(1,xc+latticeP->LXMIN,yc+latticeP->LYMIN,contourValue,weight)) goto errexit  ;
             xlc = xc ; ylc = yc ;
            }
         }
      }
/*
**  Set Direction For Reverse Trace
*/
    if( traced )
      {
       ++direction ;
       direction %= 2 ;
      }
   }
/*
** Store Contour Direction Coordinate
*/
 contourDirection = 1;
 if (bcdtmLoad_contourFeature (NULL, contourParams, fenceParams, pondExtendedAppData, clipDtmP, contourDirection, loadFunctionP, userP)) goto errexit;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"**** Tracing %10.4f Contour Completed",contourValue) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"**** Tracing %10.4f Contour Error",contourValue) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Contour At A Point From A DTM Object
* @doc    Loads The Contour At A Point From A DTM Object
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @notes  Smoothing Option 2 Spline Smooths The Contour With Contour Overlap Detection
* @notes  Smoothing Option 3 Spline Smooths The Contour Without Contour Overlap Detection
* @author Rob Cormack July 2010  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To Dtm Object
* @param  x                     ==> x Coordinate Of The Point
* @param  Y                     ==> Y Coordinate Of The Point
* @param  smoothOption          ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)>
* @param  smoothFactor          ==> Smoothing Factor
* @param  smoothDensity         ==> Point Densification For Spline Smoothing
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence <TRUE,FALSE>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> P3D Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_contourForPointDtmObject
(
 BC_DTM_OBJ *dtmP,            /* ==> Pointer to Dtm object                 */
 double  x,                   /* ==> x Coordinate Of Point                 */
 double  y,                   /* ==> y Coordinate Of Point                 */
 double  contourInterval,     /* ==> Contour Interval                      */
 DTMContourSmoothing smoothOption, /* ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)> */
 double  smoothFactor,        /* ==> Contour Smoothing Factor                            */
 long    smoothDensity,       /* ==> Point Densification For Spline Smoothing            */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function                            */
 long    useFence,            /* ==> Load Contour Within Fence<TRUE,FALSE>               */
 DTMFenceOption fenceOption,         /* ==> Fence Option <DTMFenceOption::Inside,DTMFenceOption::Overlap,DTMFenceOption::Outside>   */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>        */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                           */
 long    numFencePts,         /* ==> Number Of Fence Points                              */
 void    *userP               /* ==> User Pointer Passed Back To User                    */
)
/*
** This Function Loads A Contour At A Point From A Dtm Object
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long      pnt1,pnt2,pnt3,findType,voidFlag,pointInVoid,startTime=0 ;
 DPoint3dP p3dP ;
 double    z ;
 BENTLEY_NAMESPACE_NAME::TerrainModel::DTMContourParams contourParams;
/*
** Write Entry Message
*/
 startTime = bcdtmClock() ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading Contour For Point Dtm Object") ;
    bcdtmWrite_message(0,0,0,"Dtm Object             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x                      = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y                      = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"Contour Interval       = %12.5lf",contourInterval) ;
    bcdtmWrite_message(0,0,0,"Smooth Option          = %8ld",smoothOption) ;
    bcdtmWrite_message(0,0,0,"Smooth Factor          = %8.2lf",smoothFactor) ;
    bcdtmWrite_message(0,0,0,"Smooth Density         = %8ld",smoothDensity) ;
    bcdtmWrite_message(0,0,0,"Load Function          = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Use Fence              = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"Fence Option           = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"Fence Type             = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"Fence Points           = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"Num Fence Points       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"User Pointer           = %p",userP) ;
    if( useFence && numFencePts > 2 && fencePtsP != NULL )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",dtmP->xMin,dtmP->xMax,dtmP->xRange) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",dtmP->yMin,dtmP->yMax,dtmP->yRange) ;
       bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",dtmP->zMin,dtmP->zMax,dtmP->zRange) ;
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
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Validate Smoothing Options
*/
 if (smoothOption < DTMContourSmoothing::None || smoothOption > DTMContourSmoothing::SplineWithoutOverLapDetection) smoothOption = DTMContourSmoothing::None;
/*
** Validate Smoothing Factor
*/
 if (smoothOption == DTMContourSmoothing::Vertex)
   {
    if( smoothFactor < 0.1 || smoothFactor > 0.5 ) smoothFactor = 0.3 ;
   }
 else if (smoothOption == DTMContourSmoothing::Spline || smoothOption == DTMContourSmoothing::SplineWithoutOverLapDetection)
   {
    if( smoothFactor < 0.0 || smoothFactor  > 5.0 ) smoothFactor = 2.5 ;
    if( smoothDensity < 3  || smoothDensity > 10  ) smoothDensity = 5  ;
   }
/*
** Validate Fence Options
*/
 if( useFence == TRUE )
   {
    if( fenceType != DTMFenceType::Block && fenceType != DTMFenceType::Shape ) useFence = FALSE ;
    if( fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Overlap && fenceOption != DTMFenceOption::Outside) useFence = FALSE ;
    else if( fencePtsP == NULL || numFencePts <= 4 ) useFence = FALSE ;
    else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
  }
/*
** Create Contour Params
*/
 contourParams.interval = contourInterval;
 contourParams.smoothOption = smoothOption;
 contourParams.smoothFactor = smoothFactor;
 contourParams.smoothDensity = smoothDensity;

/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&findType,&pnt1,&pnt2,&pnt3)) goto errexit ;
/*
** Test For Point In Void
*/
 pointInVoid = FALSE ;
 if( findType )
   {
    if( findType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) pointInVoid = TRUE ;
    if( findType == 2 || findType == 3 )
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,&voidFlag)) goto errexit ;
       if( voidFlag ) pointInVoid = TRUE ;
      }
    if( findType == 4 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,&voidFlag)) goto errexit ;
       if( voidFlag ) pointInVoid = TRUE ; ;
      }
   }
/*
** Plot The Contour
*/
 if( findType && pointInVoid == FALSE )
   {
   BENTLEY_NAMESPACE_NAME::TerrainModel::DTMFenceParams fenceParams (useFence ? fenceType : DTMFenceType::None, fenceOption, fencePtsP, numFencePts);
   if (bcdtmLoad_plotContourAtPointDtmObject (dtmP, contourParams, fenceParams, nullptr, nullptr, x, y, z, findType, pnt1, pnt2, pnt3, loadFunctionP, userP)) goto errexit;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contour For Point Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Contour For Point Dtm Object Error") ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Contours = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
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
BENTLEYDTM_Private int bcdtmLoad_plotContourAtPointDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer to Dtm object                 */
 TerrainModel::DTMContourParamsCR contourParams,   /* ==> Contour Params                        */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDtmP,
 double  x,                        /* ==> x Coordinate Of Point                 */
 double  y,                        /* ==> y Coordinate Of Point                 */
 double  z,                        /* ==> z Coordinate Of Point                 */
 long    findType,                 /* ==> Point Type In Relation To Triangle    */
 long    trgPnt1,                  /* ==> Triangle Point 1                      */
 long    trgPnt2,                  /* ==> Triangle Point 2                      */
 long    trgPnt3,                  /* ==> Triangle Point 3                      */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function                            */
 void    *userP               /* ==> User Pointer Passed Back To User                    */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long clPtr,startEdge1,startEdge2,antPnt,clkPnt,zeroSlopeTriangle ;
/*
** Determine Starting Conditions For Contour
*/
 startEdge1 = dtmP->nullPnt ;
 startEdge2 = dtmP->nullPnt ;
 zeroSlopeTriangle = FALSE ;
/*
** Point Coincident With Triangle Point
*/
 if( findType == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point Coincident With Triangle Point") ;
    clPtr = nodeAddrP(dtmP,trgPnt1)->cPtr ;
    while( clPtr != dtmP->nullPtr && startEdge1 == dtmP->nullPnt )
      {
       trgPnt2 = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
       if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,trgPnt2)->z )
         {
          startEdge1 = trgPnt1 ;
          startEdge2 = trgPnt2 ;
         }
      }
   }
/*
** Point Colinear With An Internal Triangle Edge
*/
 if( findType == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point Colinear With An Internal Triangle Edge") ;
    if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,trgPnt2)->z )
      {
       startEdge1 = trgPnt1 ;
       startEdge2 = trgPnt2 ;
      }
    else            // Zero Slope Line
      {
       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
       if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,antPnt)->z )
         {
          startEdge1 = trgPnt1 ;
          startEdge2 = antPnt  ;
         }
       else if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,clkPnt)->z )
         {
          startEdge1 = trgPnt1 ;
          startEdge2 = clkPnt  ;
         }
      }
   }
/*
** Point Colinear With An External Triangle Edge
*/
 if( findType == 3 )
   {
    if( nodeAddrP(dtmP,trgPnt2)->hPtr == trgPnt1 )
      {
       trgPnt3 = trgPnt1 ;
       trgPnt1 = trgPnt2 ;
       trgPnt2 = trgPnt3 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Point Colinear With An External Triangle Edge") ;
    if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,trgPnt2)->z )
      {
       startEdge1 = trgPnt1 ;
       startEdge2 = trgPnt2 ;
      }
    else            // Zero Slope Line
      {
       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
       if( pointAddrP(dtmP,trgPnt1)->z != pointAddrP(dtmP,antPnt)->z )
         {
          startEdge1 = trgPnt1 ;
          startEdge2 = antPnt  ;
         }
      }
   }
/*
** Point Internal To Triangle
*/
 if( findType == 4 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point Internal To Triangle") ;
    if( pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt2)->z && pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt3)->z ) zeroSlopeTriangle = TRUE ;
    else
      {
       if(      ( z >= pointAddrP(dtmP,trgPnt1)->z && z < pointAddrP(dtmP,trgPnt2)->z ) ||  ( z >= pointAddrP(dtmP,trgPnt2)->z && z < pointAddrP(dtmP,trgPnt1)->z )) { startEdge1 = trgPnt1 ; startEdge2 = trgPnt2 ; }
       else if( ( z >= pointAddrP(dtmP,trgPnt2)->z && z < pointAddrP(dtmP,trgPnt3)->z ) ||  ( z >= pointAddrP(dtmP,trgPnt3)->z && z < pointAddrP(dtmP,trgPnt2)->z )) { startEdge1 = trgPnt2 ; startEdge2 = trgPnt3 ; }
       else if( ( z >= pointAddrP(dtmP,trgPnt3)->z && z < pointAddrP(dtmP,trgPnt1)->z ) ||  ( z >= pointAddrP(dtmP,trgPnt1)->z && z < pointAddrP(dtmP,trgPnt3)->z )) { startEdge1 = trgPnt3 ; startEdge2 = trgPnt1 ; }
      }
   }
/*
** Only Plot Contour If Starting Conditions Correct
*/
 if( startEdge1 != dtmP->nullPnt )
   {
/*
**  Trace Contour Across Tin Surface
*/
   if (bcdtmLoad_traceContourFromTriangleEdgeDtmObject (dtmP, contourParams, fenceParams, nullptr, clipDtmP, z, startEdge1, startEdge2, loadFunctionP, userP)) goto errexit;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Contour For Point Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Contour For Point Dtm Object Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_traceContourFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP,            /* ==> Pointer to Dtm object                 */
 TerrainModel::DTMContourParamsCR contourParams, /* ==> Contour Params */
 TerrainModel::DTMFenceParamsCR fenceParams,
 DTMPondAppData* pondExtendedAppData,
 BC_DTM_OBJ* clipDTMP,
 double  contourValue,        /* ==> Contour Value To Be Plotted           */
 long    p1,                  /* ==> Triangle Edge Point                   */
 long    p2,                  /* ==> Triangle Edge Point                   */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function                            */
 void    *userP               /* ==> User Pointer Passed Back To User                    */
)
/*
** This Routine Traces A Contour From A Triangle Edge Across The Tin Surface
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   test = 1, scan, contourScanned = FALSE, zeroSlopeLine, zeroSlopeTriangle;
 DTMDirection direction = DTMDirection::AntiClockwise;
 long contourDirection = 0;
 long   weight=0,p3,sp1,sp2,lp2,lp1,llp1 ;
 double ra,zp1,zp2,lzp1,xc,yc,xlc=0.0,ylc=0.0 ;
 BC_DTM_OBJ  *clipDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour From Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"p1         = %8ld",p1) ;
    bcdtmWrite_message(0,0,0,"p2         = %8ld",p2) ;
    bcdtmWrite_message(0,0,0,"p1->hPtr   = %8ld",nodeAddrP(dtmP,p1)->hPtr) ;
    bcdtmWrite_message(0,0,0,"p2->hPtr   = %8ld",nodeAddrP(dtmP,p2)->hPtr) ;
   }
/*
**  Build Clipping Dtm For Fence Operations
*/
 if (fenceParams.fenceType != DTMFenceType::None) if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fenceParams.points, fenceParams.numPoints)) goto errexit;
/*
** Contour Tracing Algorithm Notes :-
**
** Note 1 : test = 1 if P1->Z <= Cv < P2->Z
**          test = 2 if P1->Z >= Cv > P2->Z
** Note 2 : if direction == DTMDirection::AntiClockwise - Scan Anticlockwise About P1
**          if direction == DTMDirection::Clockwise     - Scan Clockwise About P1
** Note 3 : Contour Tracing Does Two Scans Through Dtm
**          First Scan Traces To End Of Contour Without Drawing Contour
**          Second Scan Traces From Contour End Backwards Drawing Contour
** Note 4 : During Scanning Only P1->z Can Be Equal To The Contour Value
*/
 if( nodeAddrP(dtmP,p2)->hPtr == p1 )
   {
    p3 = p1 ;
    p1 = p2 ;
    p2 = p3 ;
   }
/*
** Set Contour Test Setting - See Above
*/
 test = 0 ;
 if     ( contourValue >= pointAddrP(dtmP,p1)->z && contourValue < pointAddrP(dtmP,p2)->z ) test= 1 ;
 else if( contourValue <= pointAddrP(dtmP,p1)->z && contourValue > pointAddrP(dtmP,p2)->z ) test= 2 ;
 if( test == 0 )
   {
    bcdtmWrite_message(2,0,0,"Contour Test Error") ;
    goto errexit ;
   }
/*
** Scan Through Dtm Two Times
** Firstly Scan To End Of Contour
** Secondly Scan Back Along Contour While Drawing It
*/
 for( scan = 0 ; scan < 2 ; ++scan )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"scan = %ld ** p1 = %6ld p2 = %6ld",scan,p1,p2) ;
    sp1 = p1 ;
    sp2 = p2 ;
    contourScanned = FALSE ;
    llp1 = lp1 = dtmP->nullPnt ;
    if( direction == DTMDirection::Clockwise && nodeAddrP(dtmP,p1)->hPtr == p2 ) contourScanned = TRUE ;
/*
**  Draw First Contour Point
*/
    if( scan && contourScanned == FALSE )
      {
       ra = (contourValue-pointAddrP(dtmP,p1)->z)/(pointAddrP(dtmP,p2)->z - pointAddrP(dtmP,p1)->z) ;
       xc = pointAddrP(dtmP,p1)->x + ( pointAddrP(dtmP,p2)->x - pointAddrP(dtmP,p1)->x ) * ra ;
       yc = pointAddrP(dtmP,p1)->y + ( pointAddrP(dtmP,p2)->y - pointAddrP(dtmP,p1)->y ) * ra ;
       weight = 0 ;
       if      ( ra == 0.0 ) weight = 1 ;
       else if ( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) weight = 2 ;
       if( bcdtmLoad_storeContourPoint(1,xc,yc,contourValue,weight)) goto errexit  ;
       xlc = xc ; ylc = yc  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Start Contour Point = %10.4lf %10.4lf %10.4lf",xc,yc,contourValue) ;
      }
/*
**  Get Next Dtm Point Around P1
*/
    while ( contourScanned == FALSE )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Scan = %1ld Direction = %2ld",scan,direction) ;
          bcdtmWrite_message(0,0,0,"*LP1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
          bcdtmWrite_message(0,0,0,"*LP2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
         }
       if( lp1 != p1 ) llp1 = lp1 ;
       lp1  = p1 ;
       lp2  = p2 ;
       lzp1 = pointAddrP(dtmP,p1)->z ;
       if( direction == DTMDirection::Clockwise) { if( ( p2 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
       else                            { if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit  ; }
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"**P1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
          bcdtmWrite_message(0,0,0,"**P2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
         }
/*
**     Check For Switching P1 and P2
*/
       zp1 = pointAddrP(dtmP,p1)->z ;
       zp2 = pointAddrP(dtmP,p2)->z ;
       if( ( test == 1 && contourValue >= zp2 ) || ( test == 2 && contourValue <= zp2 ) )
         {
/*
**        Check For Contour Ridge Or Sump
*/
          if( p2 == llp1 && pointAddrP(dtmP,p2)->z == contourValue )
            {
             p2 = lp2 ;
             sp1 = p1 ;
             sp2 = p2 ;
             contourScanned = TRUE ;
             if( dbg && contourScanned ) bcdtmWrite_message(0,0,0,"Contour Terminated At Loop Back") ;
            }
/*
**        Switch P1 And P2 To Maintain Scan Criteria Of P1->z == Contour Value
*/
          else
            {
             p1 = p2  ;
             p2 = lp2 ;
             zp1 = pointAddrP(dtmP,p1)->z ;
             zp2 = pointAddrP(dtmP,p2)->z ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Switched P1 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,nodeAddrP(dtmP,p1)->hPtr) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Switched P2 = %6ld ** %10.4lf %10.4lf %8.2lf ** Fptr = %9ld",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,nodeAddrP(dtmP,p2)->hPtr) ;
             if( zp2 == contourValue )
               {
                bcdtmWrite_message(2,0,0,"Invalid Value For Contour P2 ** P2 = %6ld ** %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
                goto errexit ;
               }
            }
         }
/*
**    Check For Termination On An Already Drawn Line
*/
       if( p1 != sp1 || p2 != sp2 )
         {
          if( lzp1 == contourValue && zp1 == contourValue && lp1 != p1 ) contourScanned = TRUE ;
          if( contourScanned == TRUE && ! scan )
            {
             p1 = lp1 ;
             p2 = lp2 ;
            }
         }
/*
**    Continue Contour Scan
*/
       if( contourScanned == FALSE )
         {

/*
**        Check For A Zero Slope Line
*/
          zeroSlopeLine = 0 ;
          zeroSlopeTriangle = 0 ;
          if( lp1 != p1 && lzp1 == contourValue && zp1 == contourValue ) zeroSlopeLine = 1 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Line = %2ld",zeroSlopeLine) ;
/*
**        Test For For Zero Slope Triangle
*/
          if( zeroSlopeLine )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Zero Slope Triangle") ;
             if(( p3 = bcdtmList_nextClkDtmObject(dtmP,lp1,p1)) < 0 ) goto errexit  ;
             if( bcdtmList_testLineDtmObject(dtmP,p1,p3) && pointAddrP(dtmP,p3)->z == contourValue ) zeroSlopeTriangle = 1 ;
             else
               {
                if(( p3 = bcdtmList_nextAntDtmObject(dtmP,lp1,p1)) < 0 ) goto errexit  ;
                if( bcdtmList_testLineDtmObject(dtmP,p1,p3) && pointAddrP(dtmP,p3)->z == contourValue ) zeroSlopeTriangle = 1 ;
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle = %2ld",zeroSlopeTriangle) ;
            }
/*
**       If Zero Slope Triangle Trace Contour Around Triangle Edges
*/
         if( zeroSlopeTriangle )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle = %2ld Scan = %1ld test = %2ld",zeroSlopeTriangle,scan,test) ;//
//            if( scan )  if( bcdtmLoad_contourFeature(dtmP,contourValue,smoothOption,smoothFactor,smoothDensity,contourDirection,clipDtmP,useFence,fenceOption,loadFunctionP,userP) ) goto errexit ;
//            if( bcdtmLoad_traceZeroSlopeContourDtmObject(dtmP,contourValue,lp1,p1,direction,contourDirection,contourValue,smoothOption,smoothFactor,smoothDensity,tinLineP,clipDtmP,useFence,fenceOption,loadFunctionP,userP)) goto errexit ;
/*
**          Set For Reverse Scan Along Contour
*/
            contourScanned = TRUE ;
            p1 = lp1 ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated On Zero Slope Triangle") ;
            if( bcdtmList_testForTinHullLineDtmObject(dtmP,p1,p2) || bcdtmList_testForTinHullLineDtmObject(dtmP,p2,p1) ) scan = 2 ;
           }
/*
**       Calculate Intersection Point
*/
         else
           {
            if( scan )
              {
               if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Intersection Point") ;
               ra = ( contourValue - zp1 ) / ( zp2 - zp1 ) ;
               xc = pointAddrP(dtmP,p1)->x + ( pointAddrP(dtmP,p2)->x - pointAddrP(dtmP,p1)->x ) * ra ;
               yc = pointAddrP(dtmP,p1)->y + ( pointAddrP(dtmP,p2)->y - pointAddrP(dtmP,p1)->y ) * ra ;
               if( dbg ) bcdtmWrite_message(0,0,0,"Next  Contour Point = %10.4lf %10.4lf %10.4lf",xc,yc,contourValue) ;
               if( xc != xlc || yc != ylc )
                 {
                  weight = 0 ;
                  if      ( ra == 0.0 ) weight = 1 ;
                  else if ( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) weight = 2 ;
                  if( bcdtmLoad_storeContourPoint(1,xc,yc,contourValue,weight)) goto errexit  ;
                  xlc = xc ; ylc = yc ;
                 }
              }
/*
**           Check for Termination of closed contour
*/
             if( sp1 == p1 && sp2 == p2 )
               {
                contourScanned = TRUE ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated At Start Point") ;
               }
/*
**           Check For Temination On Hull Line
*/
             else if( bcdtmList_testForHullLineDtmObject(dtmP,p1,p2) || bcdtmList_testForHullLineDtmObject(dtmP,p2,p1) )
               {
                contourScanned = TRUE ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Contour Terminated On Hull Line") ;
               }
            }
         }
      }
/*
**  Reverse Contour Scan Direction
*/
    if( direction == DTMDirection::Clockwise ) direction = DTMDirection::AntiClockwise ;
    else                             direction = DTMDirection::Clockwise ;
   }
/*
**  Load Contour Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Contour Feature") ;
 if( bcdtmLoad_contourFeature(dtmP,contourParams,fenceParams,pondExtendedAppData, clipDtmP, contourDirection,loadFunctionP,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( bcdtmLoad_storeContourPoint(0,0.0,0.0,0.0,0)) goto errexit  ;
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour From Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour From Triangle Edge Error") ;
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
BENTLEYDTM_Private int  bcdtmContour_pondCallBackFunction
(
 DTMFeatureType dtmFeatureType,
 DTMUserTag   userTag,
 DTMFeatureId featureId,
 DPoint3d            *featurePtsP,
 long           numFeaturePts,
 void           *userP
)
/*
** Sample DTM Interrupt Load Function
**
** This Function Receives The Load Features From The DTM
** As The DTM Reuses The Feature Points Memory Do Not Free It
** If You Require The Feature Points Then Make A Copy
** If You wish To Free The Feature Points Memory. Call  bcdtmLoadNgp_freeMemory() ;
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char  dtmFeatureTypeName[100] ;
 long  numPondPts;
 DTMDirection direction;
 double area ;
 DPoint3d   *p3dP,*pondPtsP=NULL ;
 DTMFeatureId dtmFeatureId ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Record
*/
 if( dbg )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"DTM Feature = %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
    if( dbg == 2 )
      {
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Pond Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Store Ponds In DTM
*/
 if( userP != NULL && dtmFeatureType == DTMFeatureType::LowPointPond )
   {
    dtmP = ( BC_DTM_OBJ *) userP ;
/*
**  Check For Closed Pond
*/
    if( numFeaturePts >= 4 && featurePtsP->x == (featurePtsP+numFeaturePts-1)->x && featurePtsP->y == (featurePtsP+numFeaturePts-1)->y )
      {
       numPondPts = numFeaturePts ;
       bcdtmUtl_copy3DTo3D(featurePtsP,numFeaturePts,&pondPtsP) ;
       bcdtmMath_getPolygonDirectionP3D(pondPtsP,numPondPts,&direction,&area) ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"CallBack ** direction = %2ld area = %12.8lf",direction,area) ;
       if (direction != DTMDirection::Unknown)
         {
         if (direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (pondPtsP, numPondPts);
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GraphicBreak,dtmP->nullUserTag,3,&dtmFeatureId,pondPtsP,numPondPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Region,dtmP->nullUserTag,3,&dtmFeatureId,pondPtsP,numPondPts)) goto errexit ;
          if( dtmP->numFeatures % 250 == 0 )
            {
             if( ContourLoadFunctionP != NULL )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Firing Off Check Stop") ;
                if( bcdtmLoad_callUserLoadFunction(ContourLoadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,NULL,0,ContourLoadFunctionUserArgP)) goto errexit ;
               }
            }
         }
      }
    else if( dbg ) bcdtmWrite_message(0,0,0,"Pond Does Not Close") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pondPtsP != NULL ) { free(pondPtsP) ; pondPtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pond Call Back Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pond Call Back Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
static int (*bcdtmLoad_createDepressionDtmObjectOverrideP) (BC_DTM_OBJ *dtmP, BC_DTM_OBJ*& depressionDtmP,DTMFeatureCallback loadFunctionP,void *userP) = NULL ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_overrideCreateDepressionDtmObject (int (*overrideP) (BC_DTM_OBJ *dtmP, BC_DTM_OBJ*& depressionDtmP,DTMFeatureCallback loadFunctionP,void *userP))
    {
    bcdtmLoad_createDepressionDtmObjectOverrideP = overrideP;
    return DTM_SUCCESS;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLoad_createDepressionDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer to Dtm object            */
 BC_DTM_OBJ*& depressionDtmP    /* <== Created High Low DTM             */
 )
/*
** This Method Creates A DTM Of The Pond ( Depression ) Boundaries
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   dtmFeature,numHullPts ;
 DPoint3d    *hullPtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;

 if (bcdtmLoad_createDepressionDtmObjectOverrideP)
     return bcdtmLoad_createDepressionDtmObjectOverrideP (dtmP, depressionDtmP,ContourLoadFunctionP,ContourLoadFunctionUserArgP);
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Depression DTM") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"depressionDtmPP = %p",*depressionDtmP) ;
   }
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;

/*
** Destroy Low High DTM If It Exists
*/
 if( depressionDtmP != NULL ) if( bcdtmObject_destroyDtmObject(&depressionDtmP)) goto errexit ;
/*
** Create Low High DTM
*/
 if( bcdtmObject_createDtmObject(&depressionDtmP)) goto errexit ;
/*
** Populate With Tin Hull
*/
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(depressionDtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
/*
** Scan For Ponds And Store In Depression DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Ponds") ;
// ToDo :  Draiange
 if( bcdtmDrainage_determinePondsDtmObject(dtmP,nullptr, (DTMFeatureCallback)bcdtmContour_pondCallBackFunction,1,0,depressionDtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Ponds Loaded = %8ld",(depressionDtmP)->numFeatures) ;
 if( dbg == 1 ) bcdtmWrite_toFileDtmObject(depressionDtmP,L"depressions.bcdtm") ;
/*
** Triangulate DTM
*/
 if( (depressionDtmP)->numFeatures )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Depression DTM") ;
    if( bcdtmObject_triangulateDtmObject(depressionDtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Hard Breaks") ;
    if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(depressionDtmP,DTMFeatureType::Breakline)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM") ;
    if( bcdtmList_cleanDtmObject(depressionDtmP)) goto errexit ;
    if( dbg )
      {
       for( dtmFeature = 0 ; dtmFeature < (depressionDtmP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(depressionDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region )
            {
             dtmFeatureP->dtmFeatureType = DTMFeatureType::Breakline ;
            }
         }
       bcdtmWrite_toFileDtmObject(depressionDtmP,L"depressions.bcdtm") ;
       for( dtmFeature = 0 ; dtmFeature < (depressionDtmP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(depressionDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )
            {
             dtmFeatureP->dtmFeatureType = DTMFeatureType::Region ;
            }
         }
      }
    if( dbg ) bcdtmObject_reportStatisticsDtmObject(depressionDtmP) ;
/*
**  Check All Pond Regions Close And Are Counter Clockwise
*/
    if( cdbg )
      {
       long pondError=0 ;
       DTMDirection direction;
       double area=0.0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Ponds") ;
       for( dtmFeature = 0 ; dtmFeature < (depressionDtmP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(depressionDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Region )
            {
/*
**           Check Region Feature Connectivity And Closure
*/
             if( bcdtmList_checkConnectivityOfDtmFeatureDtmObject(depressionDtmP,dtmFeature,1))
               {
                pondError = 1 ;
               }
/*
**           Check Region Feature Direction
*/
             else
               {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(depressionDtmP,dtmFeature,&hullPtsP,&numHullPts)) goto errexit ;
                if( bcdtmMath_getPolygonDirectionP3D(hullPtsP,numHullPts,&direction,&area)) goto errexit ;
                if (direction != DTMDirection::AntiClockwise) pondError = 1;
                if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
                if( dbg ) bcdtmWrite_message(0,0,0,"Pond[%8ld] ** numPts = %8ld area = %15.5lf direction = %2ld",dtmFeature,numHullPts,area,direction) ;
               }
            }
         }
/*
**     Check For Pond Errors
*/
       if( pondError )
         {
          bcdtmWrite_message(2,0,0,"Error In Depression Ponds") ;
          goto errexit ;
         }
      }
   }
/*
** No Pond Features So Destroy DTM
*/
 else bcdtmObject_destroyDtmObject(&depressionDtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Depression DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Depression DTM Error") ;
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
BENTLEYDTM_Public int bcdtmLoad_checkForDepressionContourDtmObject
(
 BC_DTM_OBJ *depressionDtmP, /* ==> Low High DTM             */
 DPoint3d        *conPtsP,        /* ==> Contour Points           */
 long       numConPts,       /* ==> Number Of Contour Points */
 double     conInterval,     /* ==> Contour Interval         */
 long       *conTypeP        /* <== Contour Type             */
 )
/*
** This Method Checks For A Depression Contour
*/
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long              process,fndType,nextPnt,tempPnt,trgPnts[3] ;
 long              dtmFeature1,dtmFeature2,dtmFeature3 ;
 long              fl1Ptr,fl2Ptr,fl3Ptr ;
 DPoint3d               *p3dP ;
 DTM_TIN_NODE      *node1P,*node2P,*node3P ;
 DTM_FEATURE_LIST  *flist1P,*flist2P,*flist3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For Depression Contour") ;
    bcdtmWrite_message(0,0,0,"depressionDtmP = %p",depressionDtmP) ;
    bcdtmWrite_message(0,0,0,"conPtsP        = %p",conPtsP) ;
    bcdtmWrite_message(0,0,0,"numConPts      = %8ld",numConPts) ;
   }
/*
** Log Contour Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Contour Points = %8ld",numConPts) ;
    for( p3dP = conPtsP ; p3dP < conPtsP + numConPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Contour Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-conPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Initialise
*/
 *conTypeP = 0 ;
/*
** Check If DTM Is In Tin State
*/
 if( depressionDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**  Find Triangle For Contour Point In Depression DTM
*/
 fndType = 0 ;
 long midPt = numConPts / 2;
 for( p3dP = conPtsP + midPt; p3dP < conPtsP + numConPts && fndType != 2 ; ++p3dP )
   {
    if( bcdtmFind_triangleDtmObject(depressionDtmP,p3dP->x,p3dP->y,&fndType,&trgPnts[0],&trgPnts[1],&trgPnts[2]) ) goto errexit ;
    if( dbg == 1 )bcdtmWrite_message(0,0,0,"01 fndType = %2ld",fndType) ;
    if( dbg == 1 && fndType == 2  )
      {
       bcdtmWrite_message(0,0,0,"00 fndType  = %2ld",fndType) ;
       bcdtmWrite_message(0,0,0,"Index Point = %12.5lf %12.5lf %10.4lf",p3dP->x,p3dP->y,p3dP->z) ;
       bcdtmWrite_message(0,0,0,"trgPnt1 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf",trgPnts[0],nodeAddrP(depressionDtmP,trgPnts[0])->fPtr,pointAddrP(depressionDtmP,trgPnts[0])->x,pointAddrP(depressionDtmP,trgPnts[0])->y,pointAddrP(depressionDtmP,trgPnts[0])->z ) ;
       bcdtmWrite_message(0,0,0,"trgPnt2 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf",trgPnts[1],nodeAddrP(depressionDtmP,trgPnts[1])->fPtr,pointAddrP(depressionDtmP,trgPnts[1])->x,pointAddrP(depressionDtmP,trgPnts[1])->y,pointAddrP(depressionDtmP,trgPnts[1])->z ) ;
       bcdtmWrite_message(0,0,0,"trgPnt3 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf",trgPnts[2],nodeAddrP(depressionDtmP,trgPnts[2])->fPtr,pointAddrP(depressionDtmP,trgPnts[2])->x,pointAddrP(depressionDtmP,trgPnts[2])->y,pointAddrP(depressionDtmP,trgPnts[2])->z ) ;
      }
   }
 if (fndType != 2 && midPt != 0)
     {
     for (p3dP = conPtsP + (midPt - 1); p3dP >= conPtsP && fndType != 2; --p3dP)
         {
         if (bcdtmFind_triangleDtmObject (depressionDtmP, p3dP->x, p3dP->y, &fndType, &trgPnts[0], &trgPnts[1], &trgPnts[2])) goto errexit;
         if (dbg == 1)bcdtmWrite_message (0, 0, 0, "01 fndType = %2ld", fndType);
         if (dbg == 1 && fndType == 2)
             {
             bcdtmWrite_message (0, 0, 0, "00 fndType  = %2ld", fndType);
             bcdtmWrite_message (0, 0, 0, "Index Point = %12.5lf %12.5lf %10.4lf", p3dP->x, p3dP->y, p3dP->z);
             bcdtmWrite_message (0, 0, 0, "trgPnt1 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf", trgPnts[0], nodeAddrP (depressionDtmP, trgPnts[0])->fPtr, pointAddrP (depressionDtmP, trgPnts[0])->x, pointAddrP (depressionDtmP, trgPnts[0])->y, pointAddrP (depressionDtmP, trgPnts[0])->z);
             bcdtmWrite_message (0, 0, 0, "trgPnt2 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf", trgPnts[1], nodeAddrP (depressionDtmP, trgPnts[1])->fPtr, pointAddrP (depressionDtmP, trgPnts[1])->x, pointAddrP (depressionDtmP, trgPnts[1])->y, pointAddrP (depressionDtmP, trgPnts[1])->z);
             bcdtmWrite_message (0, 0, 0, "trgPnt3 = %8ld ** fPtr = %10ld ** % 12.5lf %12.5lf %10.4lf", trgPnts[2], nodeAddrP (depressionDtmP, trgPnts[2])->fPtr, pointAddrP (depressionDtmP, trgPnts[2])->x, pointAddrP (depressionDtmP, trgPnts[2])->y, pointAddrP (depressionDtmP, trgPnts[2])->z);
             }
         }
     }
     /*
** Check If Contour Point Is In A Depression
*/
 if( fndType == 2 )
   {
    node1P = nodeAddrP(depressionDtmP,trgPnts[0]) ;
    node2P = nodeAddrP(depressionDtmP,trgPnts[1]) ;
    node3P = nodeAddrP(depressionDtmP,trgPnts[2]) ;
    if( node1P->fPtr != depressionDtmP->nullPtr && node2P->fPtr != depressionDtmP->nullPtr && node3P->fPtr != depressionDtmP->nullPtr )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Dep Contour") ;
       fl1Ptr = node1P->fPtr ;
       while( fl1Ptr != depressionDtmP->nullPtr && ! *conTypeP )
         {
          flist1P     = flistAddrP(depressionDtmP,fl1Ptr) ;
          dtmFeature1 = flist1P->dtmFeature ;
          fl1Ptr      = flist1P->nextPtr ;
          fl2Ptr      = node2P->fPtr ;
          while( fl2Ptr != depressionDtmP->nullPtr && ! *conTypeP )
            {
             flist2P = flistAddrP(depressionDtmP,fl2Ptr) ;
             dtmFeature2 = flist2P->dtmFeature ;
             fl2Ptr      = flist2P->nextPtr ;
             if( dtmFeature2 == dtmFeature1 )
               {
                fl3Ptr = node3P->fPtr ;
                while( fl3Ptr != depressionDtmP->nullPtr && ! *conTypeP )
                  {
                   flist3P = flistAddrP(depressionDtmP,fl3Ptr) ;
                   dtmFeature3 = flist3P->dtmFeature ;
                   fl3Ptr      = flist3P->nextPtr ;
                   if( dtmFeature3 == dtmFeature2 )
                     {
                      process = 1 ;
                      tempPnt = trgPnts[0] ;
                      if( bcdtmList_getNextPointForDtmFeatureDtmObject(depressionDtmP,dtmFeature1,tempPnt,&nextPnt) ) goto errexit ;
                      while( nextPnt != trgPnts[0] && ! *conTypeP && process )
                        {
                         if( nextPnt == trgPnts[1] ) process = 0 ;
                         if( nextPnt == trgPnts[2] ) *conTypeP = 1 ;
                         tempPnt = nextPnt ;
                         if( tempPnt != depressionDtmP->nullPnt ) if( bcdtmList_getNextPointForDtmFeatureDtmObject(depressionDtmP,dtmFeature1,tempPnt,&nextPnt) ) goto errexit ;
                         if( nextPnt == depressionDtmP->nullPnt ) process = 0 ;
                        }
                     }
                  }
               }
            }
         }
      }
   }
/*
** Log Depression Result
*/
 if( dbg )
   {
    if( *conTypeP ) bcdtmWrite_message(0,0,0,"Depression Contour") ;
    else            bcdtmWrite_message(0,0,0,"Not A Depression Contour") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Depression Contour Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Depression Contour Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_markTriangleEdgesThatSpanTheFenceDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To DTM                         */
 BC_DTM_OBJ *clipDtmP,       /* ==> Pointer To Clipping DTM                */
 long       *minPntP,        /* <=> Pointer To First Marked Point          */
 long       *maxPntP,        /* <=> Pointer To First Marked Point          */
 long       *numMarkedP      /* <=> Pointer To Sum Number Of Points Marked */
)
/*
** This Marks Triangle Edges That Intersect The Fence
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ap,cp,p1,p2,p3,np1,np2,np3,fndType,drapeType = 0;
 long   onLine,processDrape,hullPnt1,hullPnt2 ;
 double nd,xi,yi,zi,xls,yls,zls,xle,yle ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Triangle Edges that Span The Fence") ;
    bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"clipDtmP  = %p",clipDtmP) ;
    bcdtmWrite_message(0,0,0,"*minPntP  = %8ld",*minPntP) ;
    bcdtmWrite_message(0,0,0,"*maxPntP  = %8ld",*maxPntP) ;
   }
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
 if( clipDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Scan The Clip Dtm Hull And Look For Intersections With Triangle Edges
*/
  hullPnt1 = clipDtmP->hullPoint ;
  do
    {
     hullPnt2 = nodeAddrP(clipDtmP,hullPnt1)->hPtr ;
     if( dbg == 1 ) bcdtmWrite_message(0,0,0,"hullPnt1 = %8ld hullPnt2 = %8ld",hullPnt1,hullPnt2) ;
/*
**  Intersect The Tin
*/
    pnt1P = pointAddrP(clipDtmP,hullPnt1) ;
    pnt2P = pointAddrP(clipDtmP,hullPnt2) ;
    xls = pnt1P->x ; yls = pnt1P->y   ;
    xle = pnt2P->x ; yle = pnt2P->y   ;
/*
**  Write Out Line To Be Drapped
*/
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Drape Line = %10.4lf %10.4lf ** %10.4lf %10.4lf",xls,yls,xle,yle) ;
/*
**  Find Triangle Containing Drape Start Point
*/
    processDrape = 1 ;
    if( bcdtmFind_triangleDtmObject(dtmP,xls,yls,&fndType,&p1,&p2,&p3)) goto errexit ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
/*
**  Triangle Not Found
*/
    if( fndType == 0 )
      {
/*
**     Find Closest Intersection With Tin Hull
*/
       if( fndType == 0 )
         {
          p1 = p2 = dtmP->nullPnt ;
          if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&p1,&p2,&xls,&yls,&zls) ) goto errexit ;
          if( fndType == 3 ) fndType = 1 ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Hull Intercept ** fndType = %2ld ** p1 = %8ld p2 = %8ld p3 = %8ld",fndType,p1,p2,p3) ;
         }
       if( fndType == 0 ) processDrape = 0 ;
       if( fndType == 2 )
         {
          p3 = p1 ;
          p1 = p2 ;
          p2 = p3 ;
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         }
      }
/*
**  Point In Triangle - Test Snap To Triangle Verices Or Triangle Edges
*/
    else if( fndType == 2 )
      {
       fndType = 3 ;
       if     ( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) < dtmP->ppTol ) { fndType = 1 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) < dtmP->ppTol ) { fndType = 1 ; p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) < dtmP->ppTol ) { fndType = 1 ; p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p1 = p2 ; p2 = p3 ; p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p2 = p1 ; p1 = p3 ; p3 = dtmP->nullPnt ; }
       if( fndType == 2 )
         {
          if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xle,yle) > 0 )
            {
             p3 = p1 ;
             p1 = p2 ;
             p2 = p3 ;
            }
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         }
      }
/*
**  Plot Start of Profile Line on Surface
*/
    if( processDrape )
      {
       drapeType = fndType ;
/*
**     Intersects Triangle Edge On Hull
*/
       if( drapeType == 2 )
         {
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,ap,p2)) ap = dtmP->nullPnt ;
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,cp,p2)) cp = dtmP->nullPnt ;
          if( nodeAddrP(dtmP,p1)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,p1)->sPtr = 1  ;
             ++*numMarkedP ;
             if( p1 < *minPntP ) *minPntP = p1 ;
             if( p1 > *maxPntP ) *maxPntP = p1 ;
            }
          if( nodeAddrP(dtmP,p2)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,p2)->sPtr = 1  ;
             ++*numMarkedP ;
             if( p2 < *minPntP ) *minPntP = p2 ;
             if( p2 > *maxPntP ) *maxPntP = p2 ;
            }
          if( ap != dtmP->nullPnt && nodeAddrP(dtmP,ap)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,ap)->sPtr = 1  ;
             ++*numMarkedP ;
             if( ap < *minPntP ) *minPntP = ap ;
             if( ap > *maxPntP ) *maxPntP = ap ;
            }
          if( cp != dtmP->nullPnt && nodeAddrP(dtmP,cp)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,cp)->sPtr = 1  ;
             ++*numMarkedP ;
             if( cp < *minPntP ) *minPntP = cp ;
             if( cp > *maxPntP ) *maxPntP = cp ;
            }
         }
/*
**     Starts In Triangle
*/
       if( drapeType == 3 )
         {
         if( nodeAddrP(dtmP,p1)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,p1)->sPtr = 1  ;
             ++*numMarkedP ;
             if( p1 < *minPntP ) *minPntP = p1 ;
             if( p1 > *maxPntP ) *maxPntP = p1 ;
            }
          if( nodeAddrP(dtmP,p2)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,p2)->sPtr = 1  ;
             ++*numMarkedP ;
             if( p2 < *minPntP ) *minPntP = p2 ;
             if( p2 > *maxPntP ) *maxPntP = p2 ;
            }
          if( nodeAddrP(dtmP,p3)->sPtr == dtmP->nullPnt )
            {
             nodeAddrP(dtmP,p3)->sPtr = 1  ;
             ++*numMarkedP ;
             if( p3 < *minPntP ) *minPntP = p3 ;
             if( p3 > *maxPntP ) *maxPntP = p3 ;
            }
         }
      }
/*
**  Scan To Drape Line End
*/
    while ( processDrape )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
       fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi)  ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,xi,yi,zi) ;
/*
**     Error Detected
*/
       if( fndType == 4 ) goto errexit ;
/*
**     Next Drape Point Not Found
*/
       if( fndType == 3 ) processDrape = 0 ;
/*
**     Next Drape Point Found
*/
       if( fndType == 0 || fndType == 1 )
          {
           xls = xi ; yls = yi ; zls = zi ;
           p1 = np1 ; p2 = np2 ; p3 = np3 ;
/*
**         Check For Intersection With Triangle Edge
*/
           if( drapeType == 2 )
             {
              if( ( ap = bcdtmList_nextAntDtmObject(dtmP,np1,np2)) < 0 ) goto errexit ;
              if( ! bcdtmList_testLineDtmObject(dtmP,ap,np2)) ap = dtmP->nullPnt ;
              if( ( cp = bcdtmList_nextClkDtmObject(dtmP,np1,np2)) < 0 ) goto errexit ;
              if( ! bcdtmList_testLineDtmObject(dtmP,cp,np2)) cp = dtmP->nullPnt ;
              if( nodeAddrP(dtmP,p1)->sPtr == dtmP->nullPnt )
                {
                 nodeAddrP(dtmP,np1)->sPtr = 1  ;
                 ++*numMarkedP ;
                 if( np1 < *minPntP ) *minPntP = np1 ;
                 if( np1 > *maxPntP ) *maxPntP = np1 ;
                }
              if( nodeAddrP(dtmP,np2)->sPtr == dtmP->nullPnt )
                {
                 nodeAddrP(dtmP,np2)->sPtr = 1  ;
                 ++*numMarkedP ;
                 if( np2 < *minPntP ) *minPntP = np2 ;
                 if( np2 > *maxPntP ) *maxPntP = np2 ;
                }
              if( ap != dtmP->nullPnt && nodeAddrP(dtmP,ap)->sPtr == dtmP->nullPnt )
                {
                 nodeAddrP(dtmP,ap)->sPtr = 1  ;
                 ++*numMarkedP ;
                 if( ap < *minPntP ) *minPntP = ap ;
                 if( ap > *maxPntP ) *maxPntP = ap ;
                }
              if( cp != dtmP->nullPnt && nodeAddrP(dtmP,cp)->sPtr == dtmP->nullPnt )
                {
                 nodeAddrP(dtmP,cp)->sPtr = 1  ;
                 ++*numMarkedP ;
                 if( cp < *minPntP ) *minPntP = cp ;
                 if( cp > *maxPntP ) *maxPntP = cp ;
                }
             }
/*
**         If Drape Terminates In Triangle Check Snap Tolerances
*/
           if( drapeType == 3 )
             {
              if( nodeAddrP(dtmP,p1)->sPtr == dtmP->nullPnt )
               {
                nodeAddrP(dtmP,p1)->sPtr = 1  ;
                ++*numMarkedP ;
                if( p1 < *minPntP ) *minPntP = p1 ;
                if( p1 > *maxPntP ) *maxPntP = p1 ;
               }
             if( nodeAddrP(dtmP,p2)->sPtr == dtmP->nullPnt )
               {
                nodeAddrP(dtmP,p2)->sPtr = 1  ;
                ++*numMarkedP ;
                if( p2 < *minPntP ) *minPntP = p2 ;
                if( p2 > *maxPntP ) *maxPntP = p2 ;
               }
             if( nodeAddrP(dtmP,p3)->sPtr == dtmP->nullPnt )
               {
                nodeAddrP(dtmP,p3)->sPtr = 1  ;
                ++*numMarkedP ;
                if( p3 < *minPntP ) *minPntP = p3 ;
                if( p3 > *maxPntP ) *maxPntP = p3 ;
               }
             }
          if( fndType == 1 ) processDrape = 0 ;
         }
/*
**     Drape Line Goes External To Tin Hull
*/
       if( fndType == 2 )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Line Goes External To Tin Hull") ;
/*
**        Check If End Point Is In Point To Point Tolerance Of Tin Hull
*/
          if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xle,yle,&zls,&fndType,&np1,&np2)) goto errexit ;
          if( dbg == 1 )
            {
             bcdtmWrite_message(0,0,0,"p1 = %10ld p2 = %10ld p3 = %10ld",p1,p2,p3) ;
             bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** np1 = %8ld np2 = %8ld ** zls = %10.4lf",fndType,np1,np2,zls) ;
            }
          if( fndType )
            {
             if( fndType == 1 )     // Hull Point
               {
                pnt1P = pointAddrP(dtmP,np1) ;
                if( bcdtmMath_distance(xle,yle,pnt1P->x,pnt1P->y) > dtmP->ppTol ) fndType = 0 ;
                if( fndType && p2 == dtmP->nullPnt && np1 != p1 && ! bcdtmList_testLineDtmObject(dtmP,p1,np1)) fndType = 0 ;
                if( fndType && p2 != dtmP->nullPnt && np1 != p1 && np2 != p1 ) fndType = 0 ;
                if( fndType )
                  {
                   drapeType = 1 ;
                   xls = pnt1P->x ;
                   yls = pnt1P->y ;
                   p1 = np1 ;
                   p2 = dtmP->nullPnt ; p3 = dtmP->nullPnt ;
                   processDrape = 0 ;
                  }
               }
             else if( fndType == 2 )     // Hull Line
               {
                fndType = 0 ;
                if( p2 == dtmP->nullPnt && ( np1 == p1 || np2 == p1 ) ) fndType = 2 ;
                if( p2 != dtmP->nullPnt && ( np1 != p1 || np2 != p2 ) && ( np1 != p2 || np2 != p1 ) ) fndType = 0 ;
                if( fndType )
                  {
                   pnt1P = pointAddrP(dtmP,np1) ;
                   pnt2P = pointAddrP(dtmP,np2) ;
                   nd = bcdtmMath_distanceOfPointFromLine(&onLine,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,xle,yle,&xi,&yi) ;
                   if( ! onLine || nd > dtmP->ppTol ) fndType = 0 ;
                   if( fndType )
                     {
                      drapeType = 2 ;
                      xls = xi ;
                      yls = yi ;
                      p1 = np1 ;
                      p2 = np2 ;
                      p3 = dtmP->nullPnt ;
                      processDrape = 0 ;
                     }
                  }
               }
            }
/*
**        Look For Intercept With Tin Hull
*/
          if( fndType == 0 )
            {
             if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&drapeType,&p1,&p2,&xi,&yi,&zls) ) goto errexit ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld ** p1 = %9ld p2 = %9ld",drapeType,p1,p2) ;
/*
**           No Further Intersections Of Drape Line With Tin Hull
*/
             if( drapeType == 0 ) processDrape = 0 ;
/*
**           Drape Line Coincident With Tin Hull
*/
             if( drapeType == 3 )
               {
                drapeType = 1 ;
                xls = xi ; yls = yi ;
                p2 = dtmP->nullPnt ; p3 = dtmP->nullPnt ;
               }
/*
**           Drape Line Crosses Gulf In Tin Hull
*/
             else if( drapeType != 0 )
               {
/*
**              Store Dummy Drape Point At Mid Point In Gulf
*/
                xls = ( xls + xi ) / 2.0 ;
                yls = ( yls + yi ) / 2.0 ;
/*
**              Store Drape Point At Hull Intersection
*/
                xls =  xi ; yls = yi  ;
                if( drapeType == 2 )
                  {
                   p3 = p1 ; p1 = p2 ; p2 = p3 ;
                   if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                  }
               }
            }
         }
      }
/*
**   Reset For Next Hull Segment
*/
     hullPnt1 = hullPnt2 ;
    } while( hullPnt1 != clipDtmP->hullPoint ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Write Departing Message
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Triangle Edges that Span The Fence Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Triangle Edges that Span The Fence Error") ;
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
BENTLEYDTM_Public int bcdtmLoad_getNextClock(long i1,long j1,long i2,long j2,long *i3,long *j3)
{
 long di,dj ;
 di = i2 - i1 ;
 dj = j2 - j1 ;
 if( di ==  0 && dj ==  1 ) { *i3 = i1 + 1 ; *j3 = j1     ; }
 if( di ==  1 && dj ==  0 ) { *i3 = i1     ; *j3 = j1 - 1 ; }
 if( di ==  0 && dj == -1 ) { *i3 = i1 - 1 ; *j3 = j1     ; }
 if( di == -1 && dj ==  0 ) { *i3 = i1     ; *j3 = j1 + 1 ; }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_getNextAnt(long i1,long j1,long i2,long j2,long *i3,long *j3)
{
 long di,dj ;
 di = i2 - i1 ;
 dj = j2 - j1 ;
 if( di ==  0 && dj ==  1 ) { *i3 = i1 - 1 ; *j3 = j1     ; }
 if( di ==  1 && dj ==  0 ) { *i3 = i1     ; *j3 = j1 + 1 ; }
 if( di ==  0 && dj == -1 ) { *i3 = i1 + 1 ; *j3 = j1     ; }
 if( di == -1 && dj ==  0 ) { *i3 = i1     ; *j3 = j1 - 1 ; }
 return(0) ;
}