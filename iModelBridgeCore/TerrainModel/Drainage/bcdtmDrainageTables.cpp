/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageTables.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>

#pragma warning (disable: 4101)
#pragma warning (disable: 4102)
// Global Variables
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmTables_createAndCheckDrainageTablesDtmObject(BC_DTM_OBJ *dtmP)
{
 int  ret=DTM_SUCCESS,dbg=1 ;
 int  exitPoint,priorPoint,nextPoint ;
 int  numFound=0,numNotFound=0,startTime=bcdtmClock() ;
 bool trgFound=false ;
 DPoint3d  dtmPoints[2] ;
 BC_DTM_OBJ *dataP=nullptr ;

 
 // Calculate Drainage Tables
 
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Drainage Tables") ;
 DTMDrainageTables drainageTables = DTMDrainageTables(dtmP) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Create Tables = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

 // Scan And Find All Triangles In Index
 
 int  p1,p2,p3,clc,antPnt,clkPnt,numTriangles=0,numTrianglesFound=0 ;
 int  flowPnt,flowDir1,flowDir2,flowDir3 ;
 double ascentAngle,descentAngle,slope ;
 bool   lowPoint ;
 DPoint3d *pnt1P,*pnt2P,*antPntP,*clkPntP ;

 startTime = bcdtmClock() ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )	       
   {
    if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
      {
       clc = nodeAddrP(dtmP,p1)->cPtr;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ; 
       while( clc != dtmP->nullPtr )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p3)->hPtr != p1 )
            {
             ++numTriangles ;
             drainageTables.GetTriangleHydrologyParameters(p1,p2,p3,trgFound,flowPnt,flowDir1,flowDir2,flowDir3,ascentAngle,descentAngle,slope) ;
             if( trgFound )
               {
                ++numTrianglesFound ;
               }
             else
               {
                bcdtmWrite_message(0,0,0,"Triangle Not Found ** %8ld %8ld %8ld",p1,p2,p3) ;
                goto errexit ;
               }
            }
          p2 = p3 ;
         }
      }          
  }
 
 //  Report Number Of Triangles Found
  
  if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles = %8ld ** Number Of Triangles Found = %8ld",numTriangles,numTrianglesFound) ;
 
  // Log Time To Find All Triangles
  
  if( dbg ) bcdtmWrite_message(0,0,0,"Time To Find %8ld Triangles In Triangle Index Table = %8.3lf Seconds",numTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 
  //  Check All Triangles Found
  
  if( numTrianglesFound != dtmP->numTriangles )
    {
     bcdtmWrite_message(0,0,0,"DTM Alignment Problems With Triangle Index") ;
     goto errexit ;
    }
 
  // Add Pond Tables
  
  if( dbg ) bcdtmWrite_message(0,0,0,"Determing Ponds And Creating Pond Tables") ;
  if( bcdtmDrainage_determinePondsDtmObject(dtmP,&drainageTables, nullptr,false,true,nullptr)) goto errexit ;
  if( dbg )
    {
     bcdtmWrite_message(0,0,0,"Size Of Low Point  Pond Table = %8ld",drainageTables.SizeOfLowPointPondTable()) ;
     bcdtmWrite_message(0,0,0,"Size Of Zero Slope Pond Table = %8ld",drainageTables.SizeOfZeroSlopeLinePondTable()) ;
    }
    
 // Find Low Point Table Entries

 numFound = numNotFound = 0 ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )	       
   {
/*
**  Check For None Void Point
*/
    if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) )
      {
       if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr && nodeAddrP(dtmP,p1)->hPtr == dtmP->nullPnt )
         {

//        Test For Low Point

          lowPoint = true ; 
          while ( clc != dtmP->nullPtr && lowPoint )
            {
             p2  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( pointAddrP(dtmP,p2)->z <= pointAddrP(dtmP,p1)->z ) lowPoint = false ;
            }
            
//        Find Low Point In Tables

          if( lowPoint )
            {                
             drainageTables.FindLowPointPond(p1,exitPoint,priorPoint,nextPoint) ;
             if( exitPoint >= 0 ) ++numFound ;
             else                 ++numNotFound ;  
            } 
         } 
      }
   } 

 bcdtmWrite_message(0,0,0,"Number Of Low Point Ponds Found     = %8ld",numFound) ;    
 bcdtmWrite_message(0,0,0,"Number Of Low Point Ponds Not Found = %8ld",numNotFound) ;    
  
// Find All Zero Slope Line Pond Table Entries

 numFound = numNotFound = 0 ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )	       
   {
    if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       while( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p2 > p1 && nodeAddrP(dtmP,p2)->hPtr != p1 && nodeAddrP(dtmP,p1)->hPtr != p2)
            { 
             pnt2P = pointAddrP(dtmP,p2) ;
             if( pnt1P->z == pnt2P->z )
               {
                if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                antPntP = pointAddrP(dtmP,antPnt) ;
                clkPntP = pointAddrP(dtmP,clkPnt) ;
                if( ( antPntP->z == pnt1P->z && clkPntP->z != pnt1P->z ) ||
                    ( antPntP->z != pnt1P->z && clkPntP->z == pnt1P->z ) ||   
                    ( antPntP->z >  pnt1P->z && clkPntP->z >  pnt1P->z )     )
                  {  
                   drainageTables.FindZeroSlopeLinePond(p1,p2,exitPoint,priorPoint,nextPoint) ;
                   if( exitPoint >= 0 ) ++numFound ;
                   else                 
                     {
                      if( dataP == nullptr )if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
                      dtmPoints[0].x = pnt1P->x ; dtmPoints[0].y = pnt1P->y ; dtmPoints[0].z = pnt1P->z ;
                      dtmPoints[1].x = pnt2P->x ; dtmPoints[1].y = pnt2P->y ; dtmPoints[1].z = pnt2P->z ;
                      if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,dtmPoints,2)) goto errexit ;
                      ++numNotFound ;  
                      if( numNotFound < 100 )
                        {
                         bcdtmWrite_message(0,0,0,"Line[%8ld,%8ld] ** pnt1P->z = %10.4lf pnt2P->z = %10.4lf ** antPntP->z = %10.4lf clkPntP->z = %10.4lf",p1,p2,pnt1P->z,pnt2P->z,antPntP->z,clkPntP->z) ;
                        }
                     }   
                  } 
               } 
            }
         }  
      }
   } 
 bcdtmWrite_message(0,0,0,"Number Of Zero Slope Line Ponds Found     = %8ld",numFound) ;    
 bcdtmWrite_message(0,0,0,"Number Of Zero Slope Line Ponds Not Found = %8ld",numNotFound) ;   
 if( dataP != nullptr ) bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"missingSumps.dat") ; 

 // Clean Up  
       
  cleanup :
  if( dataP != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
      
  // Return

  bcdtmWrite_message(0,0,0,"Time To Complete Tests = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
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
int DTMTriangleIndex::CreateTriangleIndex(void)
  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
   int  p1,p2,p3,clc,numTriangles=0 ;
   DTM_TRG_INDEX_TABLE triangleIndex ;
      
  // Scan All Triangles And Create Index
  
   if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Index m-dtm = %p",m_dtm) ;
    
   for( p1 = 0 ; p1 < m_dtm->numPoints ; ++p1 )	       
     {
      if( nodeAddrP(m_dtm,p1)->cPtr != m_dtm->nullPtr )
        {
         clc = nodeAddrP(m_dtm,p1)->cPtr;
         if( ( p2 = bcdtmList_nextAntDtmObject(m_dtm,p1,clistAddrP(m_dtm,clc)->pntNum)) < 0 ) goto errexit ; 
         while( clc != m_dtm->nullPtr )
           {
            p3  = clistAddrP(m_dtm,clc)->pntNum ;
            clc = clistAddrP(m_dtm,clc)->nextPtr ;
            if( p2 > p1 && p3 > p1 && nodeAddrP(m_dtm,p3)->hPtr != p1 )
              {
               triangleIndex.index = numTriangles ;
               if( bcdtmList_testForVoidTriangleDtmObject(m_dtm,p1,p2,p3,triangleIndex.voidTriangle)) goto errexit ;
               triangleIndex.flatTriangle = false ;
               if( pointAddrP(m_dtm,p1)->z == pointAddrP(m_dtm,p2)->z && pointAddrP(m_dtm,p1)->z == pointAddrP(m_dtm,p3)->z ) triangleIndex.flatTriangle = true ;
               triangleIndex.trgPnt1 = p1 ;
               triangleIndex.trgPnt2 = p2 ;
               triangleIndex.trgPnt3 = p3 ;
               m_triangleIndex.push_back(triangleIndex);
               ++numTriangles ;
              }
            p2 = p3 ;
           }
        }          
    }
   
   // Clean Up  
       
   cleanup :
      
  // Return

  if( dbg ) bcdtmWrite_message(0,0,0,"Creating %8ld Triangle Index Completed",numTriangles) ;

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
int DTMTriangleIndex::TriangleIndexSize(void)
  {
   return ( int ) m_triangleIndex.size() ;
  } 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DtmTriangleIndex::iterator DTMTriangleIndex::FirstTriangle(void)
  {
   return m_triangleIndex.begin() ;
  } 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DtmTriangleIndex::iterator DTMTriangleIndex::LastTriangle(void)
  {
   return m_triangleIndex.end() - 1 ;
  } 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BC_DTM_OBJ* DTMTriangleIndex::TriangleIndexedDtm(void)
  {
   return m_dtm ;
  }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMTriangleIndex::FindIndexForTriangle
( 
 int trgPnt1, 
 int trgPnt2 , 
 int trgPnt3 , 
 int &trgIndex 
 )
   {
    
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),trgPnt ;
    
    // Log Parameters

    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Finding Index For Triangle") ;
       bcdtmWrite_message(0,0,0,"trgpnt1  = %8ld",trgPnt1) ;
       bcdtmWrite_message(0,0,0,"trgpnt2  = %8ld",trgPnt2) ;
       bcdtmWrite_message(0,0,0,"trgpnt3  = %8ld",trgPnt3) ;
      }
       
    //  Set Index To Less Than Zero Value
     
    trgIndex = -1  ;
       
    //  Range Check Triangle Points
       
    if( dbg ) bcdtmWrite_message(0,0,0,"Range Checking Triangle Points ** m_dtm->numPoints = %8ld",m_dtm->numPoints) ;
    if( ( trgPnt1 < 0 || trgPnt1 >= m_numPoints ) || 
        ( trgPnt2 < 0 || trgPnt2 >= m_numPoints ) || 
        ( trgPnt3 < 0 || trgPnt3 >= m_numPoints )     )
      {
       bcdtmWrite_message(1,0,0,"Triangle Index Points Range Error") ;
       goto errexit ;          
      }
    
    //  Set Triangle Points In Ascending Order
       
    if( dbg ) bcdtmWrite_message(0,0,0,"Ordering Triangle Points") ;
    while( trgPnt1 > trgPnt2 || trgPnt1 > trgPnt3 ) 
      { 
       trgPnt  = trgPnt1 ; 
       trgPnt1 = trgPnt2 ; 
       trgPnt2 = trgPnt3 ; 
       trgPnt3 = trgPnt  ; 
      }
          
    //  Binary Search Triangle Index
       
    if( dbg ) bcdtmWrite_message(0,0,0,"Binary Searching Triangle Index") ;
    if( m_triangleIndex.size() != 0)
      {
       bool indexFound=false ;
       DtmTriangleIndex::iterator indexP ;
       DtmTriangleIndex::iterator index1P = indexP = m_triangleIndex.begin ();
       DtmTriangleIndex::iterator index2P = m_triangleIndex.end() - 1 ;
        
       // Check First And Last Triangle Index Entries
       
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking First And Last Entries In Index Table") ;   
       if( trgPnt1 == index1P->trgPnt1 && trgPnt2 == index1P->trgPnt2 && trgPnt3 == index1P->trgPnt3 )
         {
          indexFound = true ;
          trgIndex = index1P->index ;
         }
       else if( trgPnt1 == index2P->trgPnt1 && trgPnt2 == index2P->trgPnt2 && trgPnt3 == index2P->trgPnt3 )
         {
          indexFound = true ;
          trgIndex = index2P->index ;
         }
           
       // Scan Triangle Index To Find Entry  
         
       if( indexFound == false )
         {
           
          // Binary Scan To Find Entry For trgPnt1 ;
          
          if( dbg ) bcdtmWrite_message(0,0,0,"Binary Searching To Find Entry For trgPnt1") ;  
          while ( indexFound == false && ( index2P - index1P ) > 0 )
            {
             indexP = index1P + ( index2P - index1P ) / 2 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"**** IndexP ** %8ld %8ld %8ld",indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3) ;                
             if( trgPnt1 == indexP->trgPnt1 ) indexFound = true ;
             else if( trgPnt1 > indexP->trgPnt1 ) index1P = indexP ;
             else                                 index2P = indexP ;
            } 
          if( dbg )
            {
             if( ! indexFound ) bcdtmWrite_message(0,0,0,"Entry For trgPnt1 Not Found") ;
             else               bcdtmWrite_message(0,0,0,"Entry For trgPnt1 == %8ld %8ld %8ld",indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3) ;
            }
            
          //  Scan All Entries For trgPnt1
              
          if( indexFound )
            {
                
             // Scan BackWards
            
             indexFound = false ;
             index1P = indexP ;
             index2P = m_triangleIndex.begin() ;
             while( index1P >= index2P && index1P->trgPnt1 == trgPnt1 && indexFound == false )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Backwards ** %8ld %8ld %8ld",index1P->trgPnt1,index1P->trgPnt2,index1P->trgPnt3) ;                
                if( index1P->trgPnt2 == trgPnt2 && index1P->trgPnt3 == trgPnt3 )
                  {
                   indexFound = true ;
                   trgIndex = index1P->index ;
                 }
                else
                  {
                   --index1P ;  
                  } 
               }
                 
              // Scan Forwards
               
               index1P = indexP + 1 ;
               index2P = m_triangleIndex.end() - 1 ;
               while( index1P <= index2P && index1P->trgPnt1 == trgPnt1 && indexFound == false )
                 {
                  if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Forewards ** %8ld %8ld %8ld",index1P->trgPnt1,index1P->trgPnt2,index1P->trgPnt3) ;                
                  if( index1P->trgPnt2 == trgPnt2 && index1P->trgPnt3 == trgPnt3 )
                    {
                     indexFound = true ;
                     trgIndex = index1P->index ;
                   }
                  else
                    {
                     ++index1P ;  
                    } 
                 }
              }
          }
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
|  Method To Log A Triangle Index                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMTriangleIndex::LogTriangleIndex(void)
   {
    int ret = DTM_SUCCESS ;
    
    // Check Table Has Been Created
    
    if( m_dtm != nullptr )
      {
       bcdtmWrite_message(0,0,0,"Size Of Triangle Index Table = %8ld",m_triangleIndex.size()) ;
       for( DtmTriangleIndex::iterator indexP = m_triangleIndex.begin() ; indexP != m_triangleIndex.end() ; ++indexP )
         {
          bcdtmWrite_message(0,0,0,"Index[%8ld] = %8ld %8ld %8ld",indexP->index,indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3) ;
         }
      }     
    else
      {
       bcdtmWrite_message(1,0,0,"Triangle Index Table Has Not Been Created") ;
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
|  Method To Log The Triangle Counts                                 |
|                                                                    |
+-------------------------------------------------------------------*/
void DTMTriangleIndex::LogTriangleCounts(void)
   {
    int numVoidTriangles=0 ;
    int numFlatTriangles=0 ;
    int numSlopeTriangles=0 ;
    int numTriangles=0 ; 
    
    // Check Table Has Been Created
    
    if( m_dtm != nullptr )
        {
        bcdtmWrite_message(0,0,0,"Size Of Triangle Index Table = %8ld",m_triangleIndex.size()) ;
        for( DtmTriangleIndex::iterator triangle = m_triangleIndex.begin() ; triangle != m_triangleIndex.end() ; ++triangle )
            {
            if     ( triangle->voidTriangle ) ++numVoidTriangles ;
            else if( triangle->flatTriangle ) ++numFlatTriangles ;
            else                              ++numSlopeTriangles ;
            ++numTriangles ;
            }
        }  

    //  Write Triangle Counts

    bcdtmWrite_message(0,0,0,"Number Of Void    Triangles  = %8ld",numVoidTriangles) ;   
    bcdtmWrite_message(0,0,0,"Number Of Flat    Triangles  = %8ld",numFlatTriangles) ;   
    bcdtmWrite_message(0,0,0,"Number Of Sloping Triangles  = %8ld",numSlopeTriangles) ; 
    bcdtmWrite_message(0,0,0,"Total Number Of Triangles    = %8ld",numTriangles) ; 
   }     

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

int DTMHydrologyTable::CreateHydrologyTable
( 
 DTMTriangleIndex& triangleIndex 
)
   {
      DTM_TRG_HYD_TABLE hydrologyTable ;
      BC_DTM_OBJ *dtmP=triangleIndex.TriangleIndexedDtm() ;
      for( DtmTriangleIndex::iterator triangle = triangleIndex.FirstTriangle() ; triangle <= triangleIndex.LastTriangle() ; triangle++)
         {
          hydrologyTable.flowPnt =  dtmP->nullPnt ;
          hydrologyTable.flowDir1 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,triangle->trgPnt1,triangle->trgPnt2,triangle->trgPnt3) ;
          hydrologyTable.flowDir2 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,triangle->trgPnt2,triangle->trgPnt3,triangle->trgPnt1) ;
          hydrologyTable.flowDir3 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,triangle->trgPnt3,triangle->trgPnt1,triangle->trgPnt2) ;
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,triangle->trgPnt1,triangle->trgPnt2,triangle->trgPnt3,&hydrologyTable.descentAngle,&hydrologyTable.ascentAngle,&hydrologyTable.slope) ;
          m_hydrologyTable.push_back(hydrologyTable);
         } 
      m_tablesCreated = true ;        
      return(DTM_SUCCESS) ;   
    }  
    
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    
int DTMHydrologyTable::GetTriangleHydrologyParameters
( 
 DTMTriangleIndex& triangleIndex,    // ==> Triangle Index
 int trgPnt1,                        // ==> Triangle Point 1
 int trgPnt2,                        // ==> Triangle Point 2
 int trgPnt3,                        // ==> Triangle Point 3
 int& flowPnt,                       // <== Not Used. Intended For Latter Development
 int& flowDir1,                      // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
 int& flowDir2,                      // <== Flow Direction Triangle Edge ** trgPnt2 - trgPnt3  
 int& flowDir3,                      // <== Flow Direction Triangle Edge ** trgPnt3 - trgPnt1 
 double& ascentAngle,                // <== Maximum Ascent Angle 
 double& descentAngle,               // <== Maximum Descent Angle
 double& slope                       // <== Triangle Slope
)
    {
       int ret=DTM_SUCCESS ;
       int trgIndex ;
       
       // Check Tables Created
       
       if( m_tablesCreated == false )
         {
          bcdtmWrite_message(1,0,0,"Hydrology Tables Not Created") ;
          goto errexit ;
         } 
          
       // Get Triangle Index   
       
       if( triangleIndex.FindIndexForTriangle(trgPnt1,trgPnt2,trgPnt3,trgIndex) != DTM_SUCCESS )
         {
          flowPnt      = 0 ;
          flowDir1     = 0 ;
          flowDir2     = 0 ;
          flowDir3     = 0 ;
          ascentAngle  = 0.0 ;
          descentAngle = 0.0 ;
          slope        = 0.0 ;
          goto errexit ;
         }
       
       // Return Hydrology Parameters
       
       else
         {
          bvector<DTM_TRG_HYD_TABLE>::iterator triangle = m_hydrologyTable.begin() + trgIndex ;
          flowPnt      = triangle->flowPnt  ;
          flowDir1     = triangle->flowDir1 ;
          flowDir2     = triangle->flowDir2 ;
          flowDir3     = triangle->flowDir3 ;
          ascentAngle  = triangle->ascentAngle ;
          descentAngle = triangle->descentAngle ;
          slope        = triangle->slope ;
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
int DTMHydrologyTable::HydrologyTableSize(void)
   {
    return (int) m_hydrologyTable.size() ;
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DtmHydrologyTable::iterator DTMHydrologyTable::FirstHydrology(void)
   {
    return m_hydrologyTable.begin() ;
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::GetTriangleHydrologyParameters
( 
 int trgPnt1,                        // ==> Triangle Point 1
 int trgPnt2,                        // ==> Triangle Point 2
 int trgPnt3,                        // ==> Triangle Point 3
 bool& trgFound,                     // <== True = Triangle Found , False = Triangle Not Found 
 int& flowPnt,                       // <== Not Used. Intended For Latter Development
 int& flowDir1,                      // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
 int& flowDir2,                      // <== Flow Direction Triangle Edge ** trgPnt2 - trgPnt3  
 int& flowDir3,                      // <== Flow Direction Triangle Edge ** trgPnt3 - trgPnt1 
 double& ascentAngle,                // <== Maximum Ascent Angle 
 double& descentAngle,               // <== Maximum Descent Angle
 double& slope                       // <== Triangle Slope
)
   {
    int trgIndex=0 ;
    m_drainageTriangleIndex.FindIndexForTriangle(trgPnt1,trgPnt2,trgPnt3,trgIndex) ;
    if( trgIndex != m_drainageTriangleIndex.TriangleIndexedDtm()->nullPnt )
      {
       trgFound = true ; 
       DtmHydrologyTable::iterator triangle = m_drainageHydrologyTable.FirstHydrology() + trgIndex ;
       flowPnt      = triangle->flowPnt  ;
       flowDir1     = triangle->flowDir1 ;
       flowDir2     = triangle->flowDir2 ;
       flowDir3     = triangle->flowDir3 ;
       ascentAngle  = triangle->ascentAngle ;
       descentAngle = triangle->descentAngle ;
       slope        = triangle->slope ;
      }  
    else
      {
       trgFound = false ; 
      }    
      
    return(DTM_SUCCESS) ;  
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::GetTriangleEdgeFlowDirection
( 
 // 
 //   Gets The Flow Direction For Triangle Edge trgPnt1 - trgPnt2
 //   Note : Triangle Points trgPnt1 - trgPnt2 - trgPnt3 Must Have A Clockwise Direction
 //          As The Drainage Utilities Trace In a CCW Direction The Flow Direction Is mapped To A CCW Value
 //
 //   Flow Direction Values
 //     >=  1  Direction Flows To The Edge
 //     ==  0  Direction Flows Parallel To The Edge
 //     <= -1  Direction Flows Away From The Edge

 int   trgPnt1,                    // ==> Triangle Point 1
 int   trgPnt2,                    // ==> Triangle Point 2
 int   trgPnt3,                    // ==> Triangle Point 3
 bool& trgFound,                   // <== True = Triangle Found , False = Triangle Not Found 
 bool& voidTriangle,               // <== True = Void Triangle, False = Not A Void Triangle 
 int&  flowDirection               // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
)
   {
    int dbg=DTM_TRACE_VALUE(0),trgOffset=0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Triangle %8ld %8ld %8ld",trgPnt1,trgPnt2,trgPnt3) ;    
    m_drainageTriangleIndex.FindIndexForTriangle(trgPnt1,trgPnt2,trgPnt3,trgOffset) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Offset = %8ld",trgOffset) ;    
    if( trgOffset != m_drainageTriangleIndex.TriangleIndexedDtm()->nullPnt )
      {
       trgFound = true ; 
       voidTriangle = false ;
       DtmTriangleIndex::iterator  trgIndex = m_drainageTriangleIndex.FirstTriangle() + trgOffset ; 
       DtmHydrologyTable::iterator hydIndex = m_drainageHydrologyTable.FirstHydrology() + trgOffset ;
       if( trgIndex->voidTriangle ) voidTriangle = true ;
       
       //  Map To CCW Direction
       
       if     ( trgPnt1 == trgIndex->trgPnt1 ) flowDirection = hydIndex->flowDir3 ;
       else if( trgPnt1 == trgIndex->trgPnt2 ) flowDirection = hydIndex->flowDir1 ;
       else if( trgPnt1 == trgIndex->trgPnt3 ) flowDirection = hydIndex->flowDir2 ;
       else    trgFound = false ;
       if( dbg ) bcdtmWrite_message(0,0,0,"flowDirection = %2d",flowDirection) ;       
      }  
    else
      {
       trgFound = false ; 
      }    
    return(DTM_SUCCESS) ;  
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::GetTriangleSlopeAndSlopeAngles
( 
 // 
 //   Gets The Triangle Ascent Angles And Slope
 //   Note : Triangle Points trgPnt1 - trgPnt2 - trgPnt3 Must Have A Clockwise Direction
 //

 int     trgPnt1,                  // ==> Triangle Point 1
 int     trgPnt2,                  // ==> Triangle Point 2
 int     trgPnt3,                  // ==> Triangle Point 3
 bool&   trgFound,                 // <== True = Triangle Found , False = Triangle Not Found 
 bool&   voidTriangle,             // <== True = Void Triangle ,False = Not A Void Triangle
 double& ascentAngle,              // <== Triangle Ascent  
 double& descentAngle,             // <== Triangle Descent Angle 
 double& slope                     // <== Triangle Slope
)
   {
    int dbg=DTM_TRACE_VALUE(0),trgOffset=0 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Triangle %8ld %8ld %8ld",trgPnt1,trgPnt2,trgPnt3) ;    
    m_drainageTriangleIndex.FindIndexForTriangle(trgPnt1,trgPnt2,trgPnt3,trgOffset) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Offset = %8ld",trgOffset) ;    
    if( trgOffset != m_drainageTriangleIndex.TriangleIndexedDtm()->nullPnt )
      {
       trgFound = true ; 
       voidTriangle = false ;
       DtmTriangleIndex::iterator trgIndex = m_drainageTriangleIndex.FirstTriangle() + trgOffset ;
       if( trgIndex->voidTriangle) voidTriangle = true ;
       DtmHydrologyTable::iterator hydIndex = m_drainageHydrologyTable.FirstHydrology() + trgOffset ;
       slope        = hydIndex->slope ;
       ascentAngle  = hydIndex->ascentAngle ;
       descentAngle = hydIndex->descentAngle ;
      }  
    else
      {
       trgFound = false ; 
      }    
    return(DTM_SUCCESS) ;  
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::StoreLowPointPond
( 
 int lowPoint,                 // ==> Low Point
 int exitPoint,                // ==> Pond Exit Point
 int priorPoint,               // ==> Prior Point To  Exit Point On Tptr Polygon
 int nextPoint                 // ==> Next Point From Exit Point On Tptr Polygon 
 ) 
    {
     return(m_drainageLowPointPondTable.StoreLowPointPond(lowPoint,exitPoint,priorPoint,nextPoint)) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::SortLowPointPondTable(void) 
    {
     return( m_drainageLowPointPondTable.SortLowPointPondTable()) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::SizeOfLowPointPondTable(void) 
    {
     return( m_drainageLowPointPondTable.SizeOfLowPointPondTable()) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::FindLowPointPond    //   Finds A Low Point Pond Table Entry
( 
 int lowPoint,                 // ==> Low Point
 int& exitPoint,               // <== Pond Exit Point
 int& priorPoint,              // <== Prior Point To  Exit Point On Tptr Polygon
 int& nextPoint                // <== Next Point From Exit Point On Tptr Polygon 
) 
   {
    return( m_drainageLowPointPondTable.FindLowPointPond(lowPoint,exitPoint,priorPoint,nextPoint)) ;
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::StoreZeroSlopeLinePond
( 
 int point1,                   // ==> Point One End Of Zero Slope Line
 int point2,                   // ==> Point Other End Of Zero Slope Line
 int exitPoint,                // ==> Pond Exit Point
 int priorPoint,               // ==> Prior Point To  Exit Point On Tptr Polygon
 int nextPoint                 // ==> Next Point From Exit Point On Tptr Polygon 
 ) 
    {
     return(m_drainageSumpLinePondTable.StoreZeroSlopeLinePond(point1,point2,exitPoint,priorPoint,nextPoint)) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::SizeOfTriangleIndex(void) 
    {
     return( m_drainageTriangleIndex.TriangleIndexSize()) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMTriangleIndex* DTMDrainageTables::GetTriangleIndex(void) 
    {
     return( &m_drainageTriangleIndex ) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::SortZeroSlopeLinePondTable(void) 
    {
     return( m_drainageSumpLinePondTable.SortZeroSlopeLinePondTable()) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::SizeOfZeroSlopeLinePondTable(void) 
    {
     return( m_drainageSumpLinePondTable.SizeOfZeroSlopeLinePondTable()) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::FindZeroSlopeLinePond    //   Finds A Zero Slope Line Pond Table Entry
( 
 int point1,                   // ==> Point On One End Of Zero Slope Line
 int point2,                   // ==> Point On Other End Of Zero Slope Line
 int& exitPoint,               // <== Pond Exit Point
 int& priorPoint,              // <== Prior Point To  Exit Point On Tptr Polygon
 int& nextPoint                // <== Next Point From Exit Point On Tptr Polygon 
) 
   {
    return( m_drainageSumpLinePondTable.FindZeroSlopeLinePond(point1,point2,exitPoint,priorPoint,nextPoint)) ;
   }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMDrainageTables::ClearPondTables(void) 
    {
     m_drainageLowPointPondTable.ClearLowPointPondTable() ;
     m_drainageSumpLinePondTable.ClearZeroSlopeLinePondTable() ;
     return( DTM_SUCCESS ) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
bool DTMDrainageTables::CheckForDtmChange
(
 int numPoints,                 // ==> Number Of DTM Points
 int numLines,                  // ==> Number Of DTM Lines
 int numTriangles,              // ==> Number Of DTM Triangles
 int numFeatures,               // ==> Number Of DTM Features 
 __time32_t modifiedTime        // ==> Modified Time
 ) 
 {
   if     ( m_numPoints    != numPoints    ) return(true) ;
   else if( m_numLines     != numLines     ) return(true) ;
   else if( m_numTriangles != numTriangles ) return(true) ;
   else if( m_numFeatures  != numFeatures  ) return(true) ;
   else if( m_modifiedTime != modifiedTime ) return(true) ;
   return(false) ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMLowPointPondTable::StoreLowPointPond
( 
 int lowPoint,                   // ==> Pond Low Point
 int exitPoint,                  // ==> Exit Point From Pond
 int priorPoint,                 // ==> Prior Point To Exit Point On Tptr Polygon 
 int nextPoint                   // ==> Next Point After Exit Point On Tptr Polygon
)

//  Note - Prior And Next Point Are Only Required To Trace Pond Boundary

  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
   DTM_LOW_POINT_POND_TABLE lowPointTable ;
   
   lowPointTable.lowPoint   = lowPoint   ;
   lowPointTable.exitPoint  = exitPoint  ;
   lowPointTable.priorPoint = priorPoint ;
   lowPointTable.nextPoint  = nextPoint  ;
   
   m_lowPointPondTable.push_back(lowPointTable);
   
   return(ret) ;

  }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMLowPointPondTable::SortLowPointPondTable(void)
  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   int  numDuplicates=0 ;
   DtmLowPointPondTable::iterator pnt,pnt1,nextPnt ;   
   
   // Only Sort If There Is More Than One Occurrence
   
   if( m_lowPointPondTable.size() > 1 )
     {
   
      // Sort Pond Table   

      std::sort ( m_lowPointPondTable.begin(), m_lowPointPondTable.end(), &LowPointCompare );
   
      // Remove Duplicates
   
      pnt = m_lowPointPondTable.begin() ;
      for( pnt1 = m_lowPointPondTable.begin() + 1 ; pnt1 < m_lowPointPondTable.end() ; ++pnt1 )
        {
         if( pnt1->lowPoint != pnt->lowPoint  )
           {
            ++pnt ;
            if( pnt != pnt1 ) *pnt = *pnt1 ;
           }
         else
           {
            ++numDuplicates ;
           }  
        }  
      if( numDuplicates > 0 ) m_lowPointPondTable.resize((int)(pnt-m_lowPointPondTable.begin()) + 1 ) ;
      if( dbg ) bcdtmWrite_message(0,0,0,"numDuplicates = %8ld",numDuplicates) ;
      
      // Log Low Point Table
   
      if( dbg )
        {
         for( pnt = m_lowPointPondTable.begin() ; pnt < m_lowPointPondTable.end() ; ++pnt )
           {
            bcdtmWrite_message(0,0,0,"LowPoint[%8ld] = %8ld ** %8ld %8ld %8ld",(int)(pnt-m_lowPointPondTable.begin()),pnt->lowPoint,pnt->exitPoint,pnt->priorPoint,pnt->nextPoint) ;
           }  
        }
   
      // Check Pond Table Sort Order And Log Errors
   
      if( cdbg )
        {
         for( pnt = m_lowPointPondTable.begin() ; pnt < m_lowPointPondTable.end() - 1 && ret == DTM_SUCCESS ; ++pnt )
           {
            nextPnt = pnt + 1 ;
            if( nextPnt->lowPoint <= pnt->lowPoint )
              {
               bcdtmWrite_message(0,0,0,"Invalid Low Point Table Sort Order ** pnt->lowPoint = %8ld nextPnt->lowPoint = %8ld",pnt->lowPoint,nextPnt->lowPoint) ;
               ret = DTM_ERROR ;
              }
           }  
        }   
     }   

   // Return
      
   return(ret) ;
  }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMLowPointPondTable::SizeOfLowPointPondTable(void) 
{ 
 return (int) m_lowPointPondTable.size() ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMLowPointPondTable::ClearLowPointPondTable(void) 
{ 
 m_lowPointPondTable.clear() ;
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMLowPointPondTable::FindLowPointPond
( 
 int lowPoint,                   // ==> Pond Low Point
 int& exitPoint,                 // <== Exit Point From Pond
 int& priorPoint,                // <== Prior Point To Exit Point On Tptr Polygon 
 int& nextPoint                  // <== Next Point After Exit Point On Tptr Polygon
)
  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
   int  sizeOfTable,midOffset ;
   DtmLowPointPondTable::iterator basePnt,firstPnt,lastPnt,midPnt ;
   
   // Initialise
   
   exitPoint  = -1 ;
   priorPoint = -1 ;
   nextPoint  = -1 ;
   
   if( dbg ) bcdtmWrite_message(0,0,0,"Find Low Point Pond For Point %8ld",lowPoint) ;
   
   // Only Find If Table Exists
   
   if( ( sizeOfTable = ( int) m_lowPointPondTable.size()) > 0 )
     {
      firstPnt = m_lowPointPondTable.begin() ;
      lastPnt  = m_lowPointPondTable.end() - 1 ;
      basePnt  = firstPnt ;
      if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %8ld lastPnt = %8ld",firstPnt->lowPoint,lastPnt->lowPoint) ;

//    Check For First Point

      if( lowPoint == firstPnt->lowPoint )   
        {
         exitPoint  = firstPnt->exitPoint ;
         priorPoint = firstPnt->priorPoint ;
         nextPoint  = firstPnt->nextPoint ;
        } 
           
//    Check For Last Point

      else if( lowPoint == lastPnt->lowPoint )   
        {
         exitPoint  = lastPnt->exitPoint ;
         priorPoint = lastPnt->priorPoint ;
         nextPoint  = lastPnt->nextPoint ;
        }  
        
//    Binary Seach To Find Low Point

      else if (sizeOfTable > 2)
          {
          midOffset = ((int)(lastPnt - basePnt) + (int)(firstPnt - basePnt)) / 2;
          midPnt = basePnt + midOffset;
          while ((int)(lastPnt - basePnt) - (int)(firstPnt - basePnt) > 1)
              {
              midPnt = basePnt + midOffset;
              if (dbg) bcdtmWrite_message (0, 0, 0, "lowPoint = %8ld ** firstPnt = %8ld midPnt = %8ld lastPnt = %8ld", lowPoint, firstPnt->lowPoint, midPnt->lowPoint, lastPnt->lowPoint);
              if (midPnt->lowPoint == lowPoint)
                  {
                  lastPnt = firstPnt = midPnt;
                  }
              else if (midPnt->lowPoint < lowPoint)
                  {
                  firstPnt = midPnt;
                  }
              else if (midPnt->lowPoint > lowPoint)
                  {
                  lastPnt = midPnt;
                  }
              midOffset = ((int)(lastPnt - basePnt) + (int)(firstPnt - basePnt)) / 2;
              }

          //       Check If Entry Is Found   
          if (midPnt->lowPoint == lowPoint)
              {
              exitPoint = midPnt->exitPoint;
              priorPoint = midPnt->priorPoint;
              nextPoint = midPnt->nextPoint;
              }
          }
     }

// Return
      
   return(ret) ;
  }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMSumpLinePondTable::StoreZeroSlopeLinePond
( 
 int point1,                     // ==> Point At One End Of Sump Line
 int point2,                     // ==> Point At Other End Of Sump Line
 int exitPoint,                  // ==> Exit Point From Pond
 int priorPoint,                 // ==> Prior Point To Exit Point On Tptr Polygon 
 int nextPoint                   // ==> Next Point After Exit Point On Tptr Polygon
)

//  Note - Prior And Next Point Are Only Required To Trace Pond Boundary

  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
   int  point ;
   DTM_SUMP_LINE_POND_TABLE zeroSumpLineTable ;
   
   // Check Lowest Point Is First
   
   if( point1 > point2 )
     {
      point  = point1 ;
      point1 = point2 ;
      point2 = point  ;   
     }
     
   // Store
      
   zeroSumpLineTable.sP1        = point1   ;
   zeroSumpLineTable.sP2        = point2   ;
   zeroSumpLineTable.exitPoint  = exitPoint  ;
   zeroSumpLineTable.priorPoint = priorPoint ;
   zeroSumpLineTable.nextPoint  = nextPoint  ;
   
   m_sumpLinePondTable.push_back(zeroSumpLineTable);
   
   return(ret) ;

  }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMSumpLinePondTable::SortZeroSlopeLinePondTable(void)
  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   int  numDuplicates=0 ;
   DtmSumpLinePondTable::iterator pnt,pnt1,nextPnt ;   
   
   // Only Sort If There Is More Than One Occurrence
   
   if( m_sumpLinePondTable.size() > 1 )
   
     {  
   
      // Sort Pond Table   

      std::sort ( m_sumpLinePondTable.begin(), m_sumpLinePondTable.end(), &ZeroSlopeLineCompare );
   
      // Remove Duplicates
   
      pnt = m_sumpLinePondTable.begin() ;
      for( pnt1 = m_sumpLinePondTable.begin() + 1 ; pnt1 < m_sumpLinePondTable.end() ; ++pnt1 )
        {
         if( pnt1->sP1 != pnt->sP1 || pnt1->sP2 != pnt->sP2 )
           {
            ++pnt ;
            if( pnt != pnt1 ) *pnt = *pnt1 ;
           }
         else
           {
            ++numDuplicates ;
           }  
        }  
      if( numDuplicates > 0 ) m_sumpLinePondTable.resize((int)(pnt-m_sumpLinePondTable.begin()) + 1 ) ;
      if( dbg ) bcdtmWrite_message(0,0,0,"numDuplicates = %8ld",numDuplicates) ;
     
      // Log Low Point Table
   
      if( dbg )
        {
         for( pnt = m_sumpLinePondTable.begin() ; pnt < m_sumpLinePondTable.end() ; ++pnt )
           {
            bcdtmWrite_message(0,0,0,"SumpLine[%8ld] = %8ld %8ld ** %8ld %8ld %8ld",(int)(pnt-m_sumpLinePondTable.begin()),pnt->sP1,pnt->sP2,pnt->exitPoint,pnt->priorPoint,pnt->nextPoint) ;
           }  
        }
   
      // Check Pond Table Sort Order And Log Errors
   
      if( cdbg )
        {
         for( pnt = m_sumpLinePondTable.begin() ; pnt < m_sumpLinePondTable.end() - 1 && ret == DTM_SUCCESS ; ++pnt )
           {
            nextPnt = pnt + 1 ;
            if( nextPnt->sP1 < pnt->sP1 || ( nextPnt->sP1 == pnt->sP1 && nextPnt->sP2 < pnt->sP2 ) || ( nextPnt->sP1 == pnt->sP1 && nextPnt->sP2 == pnt->sP2 ) )
              {
               bcdtmWrite_message(0,0,0,"Invalid Zero Sump Line Table Sort Order ** pnt->sP1 = %8ld  pnt->sP2 = %8ld ** nextPnt->sP1 = %8ld nextPnt->sP2 = %8ld",pnt->sP1,pnt->sP2,nextPnt->sP1,nextPnt->sP2) ;
               ret = DTM_ERROR ;
              }
           }  
        }   
     }   
   

   // Return
      
   return(ret) ;
  }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMSumpLinePondTable::SizeOfZeroSlopeLinePondTable(void) 
{ 
 return (int) m_sumpLinePondTable.size() ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMSumpLinePondTable::ClearZeroSlopeLinePondTable(void) 
{ 
  m_sumpLinePondTable.clear() ;
  return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int DTMSumpLinePondTable::FindZeroSlopeLinePond
( 
 int  point1,                    // ==> Point One End Of Sump Line
 int  point2,                    // ==> Point Other One End Of Sump Line
 int& exitPoint,                 // <== Exit Point From Pond
 int& priorPoint,                // <== Prior Point To Exit Point On Tptr Polygon 
 int& nextPoint                  // <== Next Point After Exit Point On Tptr Polygon
)
  {
   int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   int  point,sizeOfTable,midOffset ;
   DtmSumpLinePondTable::iterator basePnt,firstPnt,lastPnt,midPnt ;
   
   // Initialise
   
   exitPoint  = -1 ;
   priorPoint = -1 ;
   nextPoint  = -1 ;
   
   if( dbg ) bcdtmWrite_message(0,0,0,"Find Sump Line Pond For Line %8ld %8ld",point1,point2) ;
   
   // Check Point1 Is Lowest Point Number
   
   if( point1 > point2 )
     {
      point  = point1 ;
      point1 = point2 ;
      point2 = point  ;
     }
   
   // Only Find If Table Exists
   
   if( ( sizeOfTable = ( int) m_sumpLinePondTable.size()) > 0 )
     {
      firstPnt = m_sumpLinePondTable.begin() ;
      lastPnt  = m_sumpLinePondTable.end() - 1 ;
      basePnt  = firstPnt ;
      if( dbg ) bcdtmWrite_message(0,0,0,"firstPnt = %8ld %8ld ** lastPnt = %8ld %8ls",firstPnt->sP1,firstPnt->sP2,lastPnt->sP1,lastPnt->sP2) ;

//    Check For First Point

      if( point1 == firstPnt->sP1 && point2 == firstPnt->sP2 )   
        {
         exitPoint  = firstPnt->exitPoint ;
         priorPoint = firstPnt->priorPoint ;
         nextPoint  = firstPnt->nextPoint ;
        } 
           
//    Check For Last Point

      else if(  point1 == lastPnt->sP1 && point2 == lastPnt->sP2 )   
        {
         exitPoint  = lastPnt->exitPoint ;
         priorPoint = lastPnt->priorPoint ;
         nextPoint  = lastPnt->nextPoint ;
        }  
        
//    Binary Seach To Find Low Point

      else if (sizeOfTable > 2)
          {
          midOffset = ((int)(lastPnt - basePnt) + (int)(firstPnt - basePnt)) / 2;
          midPnt = basePnt + midOffset;
          while ((int)(lastPnt - basePnt) - (int)(firstPnt - basePnt) > 1)
              {
              midPnt = basePnt + midOffset;
              //          if( dbg ) bcdtmWrite_message(0,0,0,"lowPoint = %8ld ** firstPnt = %8ld midPnt = %8ld lastPnt = %8ld",lowPoint,firstPnt->lowPoint,midPnt->lowPoint,lastPnt->lowPoint) ;
              if (midPnt->sP1 == point1 && midPnt->sP2 == point2)
                  {
                  lastPnt = firstPnt = midPnt;
                  }
              else if (midPnt->sP1 < point1 || (midPnt->sP1 == point1 && midPnt->sP2 < point2))
                  {
                  firstPnt = midPnt;
                  }
              else if (midPnt->sP1 > point1 || (midPnt->sP1 == point1 && midPnt->sP2 > point2))
                  {
                  lastPnt = midPnt;
                  }
              midOffset = ((int)(lastPnt - basePnt) + (int)(firstPnt - basePnt)) / 2;
              }

          //       Check If Entry Is Found   

          if (midPnt->sP1 == point1 && midPnt->sP2 == point2)
              {
              exitPoint = midPnt->exitPoint;
              priorPoint = midPnt->priorPoint;
              nextPoint = midPnt->nextPoint;
              }
          }
     }
     
//   If Entry Not Found Perform Linear Search - Development Only

   if( cdbg && exitPoint == -1 )     
     {
      for( midPnt = m_sumpLinePondTable.begin() ; midPnt <= m_sumpLinePondTable.end() - 1  ; ++midPnt )
        {
         if( midPnt->sP1 == point1 && midPnt->sP2 == point2 && exitPoint == -1 )
           {
            exitPoint  = midPnt->exitPoint ;
            priorPoint = midPnt->priorPoint ;
            nextPoint  = midPnt->nextPoint ;
            bcdtmWrite_message(0,0,0,"Sump Line Pond Entry Found In Linear Search") ;
            ret = DTM_ERROR ;
           } 
        }
     } 

// Return
      
   return(ret) ;
  }
