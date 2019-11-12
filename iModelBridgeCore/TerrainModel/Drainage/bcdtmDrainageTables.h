/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <algorithm>
#include <stack>
#include <TerrainModel/Core/DTMIterators.h>

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

/*-------------------------------------------------------------------+
|                                                                    |
|  Class For Caching DTM Points                                      |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMPointCache : bvector<DPoint3d>
    {
    public:

        int  StorePointInCache(double X, double Y, double Z)
            {
            push_back(DPoint3d::From(X, Y, Z));
            return DTM_SUCCESS;

            }

        int  StorePointInCache(DPoint3dCR pt)
            {
            push_back(pt);
            return DTM_SUCCESS;

            }

        void  ClearCache(void)
            {
            clear();
            }

        void  FreeCache(void)
            {
            clear();
            }

        int  CallUserDelegateWithCachePoints
        (
            DTMFeatureCallback  delegateP,
            DTMFeatureType      dtmFeatureType,
            DTMUserTag          dtmUserTag,
            DTMFeatureId        dtmFeatureID,
            void *              userP
        )
            {
            int ret = DTM_SUCCESS;
            if (!empty())
                {
                ret = delegateP(dtmFeatureType, dtmUserTag, dtmFeatureID, data(), (long)size(), userP);
                clear();
                }
            return ret;
            }

        void LogCachePoints(void)
            {
            bcdtmWrite_message(0, 0, 0, "Number Of Cache Points = %8ld", size());
            for (auto&& p3dP : *this)
                {
                bcdtmWrite_message(0, 0, 0, "Cache Point[%8d] = %12.5lf %12.5lf %12.5lf", (int)(&p3dP - data()), p3dP.x, p3dP.y, p3dP.z);
                }
            }

        bool CheckIfPointInCache(double x, double y, double z)
            {
            for (auto&& p3dP : *this)
                if (p3dP.x == x && p3dP.y == y && p3dP.z == z)
                    return true;

            return false;
            }


        int CopyCachePointsToPointArray(DPoint3d **pointsPP, long *numPointsP)
            {
            *numPointsP = 0;
            if (*pointsPP != nullptr)
                {
                free(*pointsPP);
                *pointsPP = nullptr;
                }

            if (size() > 0)
                {
                *numPointsP = (long)size();
                *pointsPP = (DPoint3d *)malloc(*numPointsP * sizeof(DPoint3d));
                if (*pointsPP == nullptr)
                    {
                    bcdtmWrite_message(1, 0, 0, "Memory Allocation Failure");
                    return DTM_ERROR;
                    }

                memcpy(*pointsPP, data(), *numPointsP * sizeof(DPoint3d));
                }

            return DTM_SUCCESS;
            }

        int CopyCachePointsToPointArray(bvector<DPoint3d>& points)
            {
            points = *this;
            return DTM_SUCCESS;
            }

        int SizeOfCache(void)
            {
            return (long)size();
            }

        void RemoveDuplicatePoints(void)
            {
            if (size() > 1)
                {
                DPoint3d* p3d1P = data();
                DPoint3d* p3d2P = nullptr;
                for (p3d2P = data() + 1; p3d2P < data() + size(); ++p3d2P)
                    {
                    if (p3d2P->x != p3d1P->x || p3d2P->y != p3d1P->y)
                        {
                        ++p3d1P;
                        if (p3d1P != p3d2P) *p3d1P = *p3d2P;
                        }
                    }
                resize((p3d1P - data()) + 1);
                }
            }
    };

/*-------------------------------------------------------------------+
|                                                                    |
|  Class For Caching DTM Lines                                       |
|                                                                    |
+-------------------------------------------------------------------*/
    struct DTMLine
        {
        DPoint3d  point1 ;
        DPoint3d  point2 ;
        } ;

class DTMLineCache : bvector<DTMLine>
    {
    public:

        int  StoreLineInCache(double X1, double Y1, double Z1, double X2, double Y2, double Z2)
            {
            DTMLine newLine;
            newLine.point1 = DPoint3d::From(X1, Y1, Z1);
            newLine.point2 = DPoint3d::From(X2, Y2, Z2);
            push_back(newLine);
            return DTM_SUCCESS;
            }

        int  StoreLineInCache(DPoint3dCR pt1, DPoint3dCR pt2)
            {
            DTMLine newLine;
            newLine.point1 = pt1;
            newLine.point2 = pt2;
            push_back(newLine);
            return DTM_SUCCESS;
            }

        void  ClearCache(void)
            {
            clear();
            }

        void  FreeCache(void)
            {
            clear();
            }

        int  CallUserDelegateWithCacheLines
        (
            DTMFeatureCallback  delegateP,
            DTMFeatureType      dtmFeatureType,
            DTMUserTag          dtmUserTag,
            DTMFeatureId        dtmFeatureID,
            bool                joinFeatures,
            void *              userP
        )
            {
            int ret = DTM_SUCCESS;
            if (!empty())
                {
                if (joinFeatures)
                    {
                    BcDTMPtr dtm = BcDTM::Create();
                    for (auto&& line : *this)
                        {
                        DTMFeatureId fId;
                        dtm->AddLinearFeature(DTMFeatureType::Breakline, &line.point1, 2, &fId);
                        }
                    int numFeatures, joinedFeatures;
                    dtm->JoinFeatures(DTMFeatureType::Breakline, &numFeatures, &joinedFeatures, 0);

                    DTMFeatureEnumerator featureEnum(*dtm);
                    
                    bvector<DPoint3d> points;
                    for (auto&& feature : featureEnum)
                        {
                        feature.GetFeaturePoints(points);
                        if (delegateP(dtmFeatureType, dtmUserTag, dtmFeatureID, points.data(), points.size(), userP))
                            return DTM_ERROR;

                        }
                    }
                else
                    ret = delegateP(dtmFeatureType, dtmUserTag, dtmFeatureID, (DPoint3d *)data(), (long)size() * 2, userP);
                clear();
                }
            return ret;
            }

        void LogCacheLines(void)
            {
            bcdtmWrite_message(0, 0, 0, "Number Of Cache Lines = %8ld", size());
            for (auto&& lineP : *this)
                {
                bcdtmWrite_message(0, 0, 0, "Cache Line[%8d] = %12.5lf %12.5lf %10.4lf ** %12.5lf %12.5lf %10.4lf", (int)(&lineP - data()), lineP.point1.x, lineP.point1.y, lineP.point1.z, lineP.point2.x, lineP.point2.y, lineP.point2.z);
                }
            }

        bool CheckIfLineInCache(double x1, double y1, double z1, double x2, double y2, double z2)
            {
            for (auto&& lineP : *this)
                {
                if (lineP.point1.x == x1 && lineP.point1.y == y1 && lineP.point1.z == z1)
                    {
                    if (lineP.point2.x == x2 && lineP.point2.y == y2 && lineP.point2.z == z2)
                        return true;
                    }
                }

            return false;
            }


        int SizeOfCache(void)
            {
            return (int)size();
            }

    };

/*-------------------------------------------------------------------+
|                                                                    |
|  Class For Caching DTM Ridge Lines                                 |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTMRidgeLine
    {
    int  lowPoint    ;   // Low  Point Of Ridge Line
    int  highPoint   ;   // High Point Of Ridge line
    } ;

class DTMRidgeLineCache
   {
    
    public :

    DTMRidgeLineCache()
       {
       m_numLines    = 0 ;
       m_memLines    = 0 ;
       m_memLinesInc = 1000 ;
       m_linesP      = nullptr ;
       }  
       
    ~DTMRidgeLineCache()
        {
        if( m_linesP != nullptr )
           {
           free( m_linesP ) ;
           m_linesP = nullptr ;  
           }
       }
    
    int  StoreLineInCache( BC_DTM_OBJ* dtmP,int point1 , int point2 )
      {
      
       // Check Memory
       
       if(  m_numLines == m_memLines )
         {
          m_memLines = m_memLines + m_memLinesInc ;
          if( m_linesP == nullptr )
              m_linesP = ( DTMRidgeLine * ) malloc( m_memLines * sizeof(DTMRidgeLine)) ;
          else
              m_linesP = ( DTMRidgeLine * ) realloc( m_linesP , m_memLines * sizeof(DTMRidgeLine)) ;
          if( m_linesP == nullptr )
              return(DTM_ERROR) ;   
         }
         
       // Store Line
       
       if( mempointAddrP(dtmP,point1)->z <= mempointAddrP(dtmP,point2)->z )
           {
           (m_linesP+m_numLines)->lowPoint  = point1 ;
           (m_linesP+m_numLines)->highPoint = point2 ;
           }
       else
           {
           (m_linesP+m_numLines)->lowPoint  = point2 ;
           (m_linesP+m_numLines)->highPoint = point1 ;
           }

       ++m_numLines ;
       
       // Return
       
       return DTM_SUCCESS ;
         
      }   

    void  ClearCache( void )
       {
       m_numLines = 0 ;      
       }

    void  FreeCache( void )
       {
       if( m_linesP != nullptr )
          {
           free( m_linesP ) ;
           m_linesP = nullptr ;  
          }
       m_numLines = 0 ;      
      }

    void LogCacheLines(void)
    {
        bcdtmWrite_message(0,0,0,"Number Of Ridge Lines = %8ld",m_numLines) ;
        for( DTMRidgeLine* lineP = m_linesP ; lineP < m_linesP + m_numLines ; ++lineP )
            {
            bcdtmWrite_message(0,0,0,"Ridge Line[%8d] ** LowPoint = %8ld HighPoint = %8ld",(int)(lineP-m_linesP),lineP->lowPoint,lineP->highPoint) ;  
            }
    }

    bool CheckIfLineInCache(int point1 , int point2 )
    {
         bool ret=false ;
         for( DTMRidgeLine* lineP = m_linesP ; lineP < m_linesP + m_numLines && ret == false ; ++lineP )
         {
              if( lineP->lowPoint == point1 && lineP->highPoint == point2 || 
                  lineP->lowPoint == point2 && lineP->highPoint == point1     )

                  ret = true ;
         }

         return ret ;
    }


    int SizeOfCache(void)
    {
         return m_numLines ;
    }

    DTMRidgeLine* FirstLine(void)
    {
         return m_linesP ;
    }

    DTMRidgeLine* LastLine(void)
    {
         if( m_numLines > 0 )
             return m_linesP + m_numLines - 1 ;
         else
             return m_linesP ;
    }

    private :
    
    int        m_numLines ;
    int        m_memLines ;
    int        m_memLinesInc ;
    DTMRidgeLine*   m_linesP ;
    
    } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Class For Caching DTM Catchment Lines                             |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTMCatchmentLine
    {
    DTMFeatureType  edgeType     ;   // DTMFeatureType::RidgeLine , DTMFeatureType::SumpLine , DTMFeatureType::CCWFLOW_LINE , DTMFeatureType::ClkFlowLine
    DTMUserTag      catchmentId  ;   // Catchment Id == Tin Drain Point
    int             edgeStatus   ;   // Use Dependent On Application , Set To 1 On Store
    int             startPoint   ;   // Start Point Of Catchment Line
    int             endPoint     ;   // End   Point Of Catchment line
    int             clkPoint     ;   // Next Clockwise Point From Start To End Point
    int             ccwPoint     ;   // Next Counter Clockwise Point From Start To End Point
    double          lowElevation ;   // Lowest elevation On Line 
    } ;

int  CatchmentLineCompareFunction(const void* line1P , const void *line2P ) ;
int  CatchmentLineElevationCompareFunction(const void* line1P , const void *line2P ) ;


class DTMCatchmentLineCache
   {
    
    public :

    DTMCatchmentLineCache()
       {
       m_numLines    = 0 ;
       m_memLines    = 0 ;
       m_memLinesInc = 1000 ;
       m_linesP      = nullptr ;
       }  
       
    ~DTMCatchmentLineCache()
        {
        if( m_linesP != nullptr )
           {
           free( m_linesP ) ;
           m_linesP = nullptr ;  
           }
       }
    
    int  StoreLineInCache( BC_DTM_OBJ *dtmP,DTMFeatureType edgeType,DTMUserTag catchmentId,int startPoint , int endPoint, int ccwPoint , int clkPoint )
      {
      
       // Check Memory
       
       if(  m_numLines == m_memLines )
         {
          m_memLines = m_memLines + m_memLinesInc ;
          if( m_linesP == nullptr )
              m_linesP = ( DTMCatchmentLine * ) malloc( m_memLines * sizeof(DTMCatchmentLine)) ;
          else
              m_linesP = ( DTMCatchmentLine * ) realloc( m_linesP , m_memLines * sizeof(DTMCatchmentLine)) ;
          if( m_linesP == nullptr )
              return(DTM_ERROR) ;   
         }

       //  Set Low Eleavtion

        double lowElevation = 0.0 ;
        if( mempointAddrP(dtmP,startPoint)->z <= mempointAddrP(dtmP,endPoint)->z ) 
            lowElevation = mempointAddrP(dtmP,startPoint)->z ;
        else
            lowElevation = mempointAddrP(dtmP,startPoint)->z ; 
         
       // Store Line With Lowest Point Number First So Duplicates from Adjoining Boundaries Can Be Eliminated
/* 
       if( startPoint <= endPoint )
           {
           (m_linesP+m_numLines)->edgeType     = edgeType     ;
           (m_linesP+m_numLines)->catchmentId  = catchmentId  ;
           (m_linesP+m_numLines)->edgeStatus   = 1            ;
           (m_linesP+m_numLines)->startPoint   = startPoint   ;
           (m_linesP+m_numLines)->endPoint     = endPoint     ;
           (m_linesP+m_numLines)->ccwPoint     = ccwPoint     ;
           (m_linesP+m_numLines)->clkPoint     = clkPoint     ;
           (m_linesP+m_numLines)->lowElevation = lowElevation ;
           }
       else
           {
           (m_linesP+m_numLines)->edgeType     = edgeType     ;
           (m_linesP+m_numLines)->catchmentId  = catchmentId  ;
           (m_linesP+m_numLines)->edgeStatus   = 1            ;
           (m_linesP+m_numLines)->startPoint   = endPoint     ;
           (m_linesP+m_numLines)->endPoint     = startPoint   ;
           (m_linesP+m_numLines)->ccwPoint     = clkPoint     ;     
           (m_linesP+m_numLines)->clkPoint     = ccwPoint     ;     
           (m_linesP+m_numLines)->lowElevation = lowElevation ;
          } 
*/
           (m_linesP+m_numLines)->edgeType     = edgeType     ;
           (m_linesP+m_numLines)->catchmentId  = catchmentId  ;
           (m_linesP+m_numLines)->edgeStatus   = 1            ;
           (m_linesP+m_numLines)->startPoint   = startPoint   ;
           (m_linesP+m_numLines)->endPoint     = endPoint     ;
           (m_linesP+m_numLines)->ccwPoint     = ccwPoint     ;
           (m_linesP+m_numLines)->clkPoint     = clkPoint     ;
           (m_linesP+m_numLines)->lowElevation = lowElevation ;

       ++m_numLines ;
       
       // Return
       
       return DTM_SUCCESS ;
         
      }   

    void  ClearCache( void )
       {
       m_numLines = 0 ;      
       }

    void  FreeCache( void )
        {
        if( m_linesP != nullptr )
            {
            free( m_linesP ) ;
            m_linesP = nullptr ;  
            }
         m_numLines = m_memLines = 0 ;      
        }
    
    void  ResizeCache( void )
        {
        if( m_numLines > 0 && m_numLines  < m_memLines )
            {
            m_memLines = m_numLines ;
            m_linesP = ( DTMCatchmentLine * ) realloc( m_linesP , m_memLines * sizeof(DTMCatchmentLine)) ;
            }   
        else if( m_numLines == 0 )
            {
            this->FreeCache() ; 
            }  
        }  

    void LogCacheLines(void)
        {
        bcdtmWrite_message(0,0,0,"Number Of Catchment Lines = %8ld",m_numLines) ;
        for( DTMCatchmentLine* lineP = m_linesP ; lineP < m_linesP + m_numLines ; ++lineP )
            {
            bcdtmWrite_message(0,0,0,"Catchment Line[%8d] ** Type = %4d startPoint = %8ld endPoint = %8ld",(int)(lineP-m_linesP),lineP->edgeType,lineP->startPoint,lineP->endPoint,lineP->clkPoint,lineP->ccwPoint) ;  
            }
        }

    int SizeOfCache(void)
    {
         return m_numLines ;
    }

    DTMCatchmentLine* FirstLine(void)
    {
         return m_linesP ;
    }

    DTMCatchmentLine* LastLine(void)
    {
         if( m_numLines > 0 )
             return m_linesP + m_numLines - 1 ;
         else
             return m_linesP ;
    }


 void SortAndRemoveDuplicates(void)
    {
         if( m_numLines > 1 )
         {
         int pnt ;  
         DTMCatchmentLine* line1P ;

         // Reorder Points Prior To Sorting So Duplicates Can Be Removed
 
         for( line1P = m_linesP  ; line1P < m_linesP + m_numLines ; ++line1P )
             {
              if( line1P->edgeType != DTMFeatureType::LowPoint )
                  {
                  if( line1P->startPoint > line1P->endPoint )
                      { 
                      pnt = (m_linesP+m_numLines)->startPoint ;
                      (m_linesP+m_numLines)->startPoint   = (m_linesP+m_numLines)->endPoint ;
                      (m_linesP+m_numLines)->endPoint     = pnt ;
                      pnt = (m_linesP+m_numLines)->ccwPoint ;
                      (m_linesP+m_numLines)->ccwPoint     = (m_linesP+m_numLines)->clkPoint     ;
                      (m_linesP+m_numLines)->clkPoint     = pnt     ;
                      }
                  } 
             } 

         //  Sort

         qsort(m_linesP,m_numLines,sizeof(DTMCatchmentLine),CatchmentLineCompareFunction) ; 

         //  Remove Duplicates

         line1P = m_linesP ;
         for( DTMCatchmentLine* line2P = m_linesP + 1 ; line2P < m_linesP + m_numLines ; ++line2P )
              {
              if( line2P->startPoint != line1P->startPoint || line2P->endPoint != line1P->endPoint )
                  {
                  ++line1P ;
                  if( line1P != line2P ) *line1P = *line2P ;
                  }
              } 
          m_numLines = ( int) ( line1P - m_linesP )  + 1 ;

         //  Check For Duplicates

         line1P = m_linesP ;
         for( DTMCatchmentLine* line2P = m_linesP + 1 ; line2P < m_linesP + m_numLines ; ++line2P )
             {
             if( line2P->startPoint == line1P->startPoint && line2P->endPoint == line1P->endPoint )
                 {
                 bcdtmWrite_message(0,0,0,"Duplicate Line Types") ; 
                 }
             }
         }  
    }


 void SortOnAscendingElevation(void)
    {
         if( m_numLines > 1 )
         {

         //  Sort

         qsort(m_linesP,m_numLines,sizeof(DTMCatchmentLine),CatchmentLineElevationCompareFunction) ; 
         }

    }

private :
    
    int                 m_numLines ;
    int                 m_memLines ;
    int                 m_memLinesInc ;
    DTMCatchmentLine*   m_linesP ;

    
    } ;


/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Store Drainage Trace Points                              |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMDrainageTracePoints : bvector<DTM_STREAM_TRACE_POINTS>
{

    public:

        int  StoreTracePoint(double X, double Y, double Z, int point1, int point2)
            {
            //if (CheckForPriorExitPoint(point1, point2))
            //    X = X;
            // Check Memory
            DTM_STREAM_TRACE_POINTS newPoint;
            newPoint.x = X;
            newPoint.y = Y;
            newPoint.z = Z;
            newPoint.P1 = point1;
            newPoint.P2 = point2;
            push_back(newPoint);

            // Return

            return DTM_SUCCESS;
            }

        void  LogTracePoints(void)
            {
            bcdtmWrite_message(0, 0, 0, "Number Of Trace Points = %8ld", size());
            for (auto&& traceP : *this)
                {
                bcdtmWrite_message(0, 0, 0, "Trace Point[%8ld] = %12.5lf %12.5lf %10.4lf ** %8ld %8ld", (long)(&traceP - data()), traceP.x, traceP.y, traceP.z, traceP.P1, traceP.P2);
                }
            }

        bool CheckForPriorTracePoint(int point)
            {
            for (auto&& traceP : *this)
                {
                if (traceP.P1 == point)
                    return true;
                }
            return false;
            }

        bool CheckForPriorExitPoint(int point, int nullPnt)
            {
            for (auto&& traceP : *this)
                {
                if (traceP.P1 == point && traceP.P2 == nullPnt)
                    return true;
                }
            return false;
            }

    };

/*-------------------------------------------------------------------+
|                                                                    |
|  Structure To Store DTM Point Lists                                |
|                                                                    |
+-------------------------------------------------------------------*/

struct DTMPointList : bvector<int>
    {
           
    DTMPointList(int* pointsP, int numPoints)
        {
        resize(numPoints);
        memcpy(data(), pointsP, numPoints * sizeof(int));
        }
        
    DTMPointList(long* pointsP, long numPoints)  // For Compatiability With Core
        {
        resize(numPoints);
        memcpy(data(), pointsP, numPoints * sizeof(int));
        }
        
     DTMPointList(const DTMPointList& dtmPointList)
       {
         *this = dtmPointList;
       }
        
     DTMPointList&   operator=(const DTMPointList& dtmPointList)
       {
        assign(std::begin(dtmPointList), std::end(dtmPointList));
        return *this;
       }
      
     void Set(long* pointsP, long numPoints)  // For Compatiability With Core
         {
         resize(numPoints);
         memcpy(data(), pointsP, numPoints * sizeof(int));
         }
    };

typedef  bvector<DTMPointList>  DTMVectorPointList ;

/*-------------------------------------------------------------------+
|                                                                    |
|  Zero Slope Line Structure                                         |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_ZERO_SLOPE_SUMP_LINE { int point1 , point2 , status , flag ; }; 

/*-------------------------------------------------------------------+
|                                                                    |
|  TypeDefs                                                          |
|                                                                    |
+-------------------------------------------------------------------*/

typedef bvector<DTM_TRG_INDEX_TABLE> DtmTriangleIndex ;
typedef bvector<DTM_TRG_HYD_TABLE>   DtmHydrologyTable ;
typedef bvector<DTM_LOW_POINT_POND_TABLE> DtmLowPointPondTable ;
typedef bvector<DTM_SUMP_LINE_POND_TABLE> DtmSumpLinePondTable ;
 
/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Create And Interrogate A Triangle Index                  |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMTriangleIndex
  {

    public :

    DTMTriangleIndex(BC_DTM_OBJ* dtmP )
      {
       m_dtm = dtmP ; 
       CreateTriangleIndex() ;
       if( m_triangleIndex.size() != m_dtm->numTriangles )
         {
          m_dtm           = nullptr ; 
          m_numPoints     = 0 ;
          m_numLines      = 0 ;
          m_numTriangles  = 0 ;
          m_numFeatures   = 0 ;
          m_modifiedTime  = 0 ;
          m_triangleIndex.clear() ;
         }
       else
         {
          m_dtm           = dtmP ;                 // Needed To Create The Hydrology Tables
          m_numPoints     = (int) dtmP->numPoints;
          m_numLines      = (int) dtmP->numLines ;
          m_numTriangles  = (int) dtmP->numTriangles ;
          m_numFeatures   = (int) dtmP->numFeatures ;
          m_modifiedTime  = dtmP->modifiedTime ;
         }  
      } 
      
    ~DTMTriangleIndex()
      {
      } 
      
//  Public Methods  

    int FindIndexForTriangle( int trgPnt1, int trgPnt2 , int trgPnt3 , int &trgIndex ) ;
    int LogTriangleIndex(void) ;
    void LogTriangleCounts(void) ; 
    int TriangleIndexSize(void) ;
    DtmTriangleIndex::iterator FirstTriangle(void) ;
    DtmTriangleIndex::iterator LastTriangle(void) ;
    BC_DTM_OBJ* TriangleIndexedDtm(void) ;
    bool CheckForChangedDtm(int numPoints, int numLines, int numTriangles,int numFeatures,time_t modifiedTime) ;
    
    private:
    
//  Class Variables    

    BC_DTM_OBJ*        m_dtm ;
    int                m_numPoints ;
    int                m_numLines ;
    int                m_numTriangles ;
    int                m_numFeatures ;
    time_t         m_modifiedTime ;
    DtmTriangleIndex   m_triangleIndex;
    
//  Private Methods
   
    int CreateTriangleIndex( void ) ;
   
  } ;  

/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Create And Interrogate DTM Hydrology Tables              |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMHydrologyTable
{
    public :
     
    DTMHydrologyTable(DTMTriangleIndex& triangleIndex )
    {
       CreateHydrologyTable(triangleIndex) ;
    } ;
    
    
    ~DTMHydrologyTable()
    {
   
       m_hydrologyTable.clear();
       
    } ;

    int GetTriangleHydrologyParameters
    ( 
     DTMTriangleIndex& triangleIndex,  // Triangle Index
     int trgPnt1,             // ==> Triangle Point 1
     int trgPnt2,             // ==> Triangle Point 2
     int trgPnt3,             // ==> Triangle Point 3
     int& flowPnt,            // <== Not Used. Intended For Latter Development
     int& flowDir1,           // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
     int& flowDir2,           // <== Flow Direction Triangle Edge ** trgPnt2 - trgPnt3  
     int& flowDir3,           // <== Flow Direction Triangle Edge ** trgPnt3 - trgPnt1 
     double& ascentAngle,     // <== Maximum Ascent Angle 
     double& descentAngle,    // <== Maximum Descent Angle
     double& slope            // <== Triangle Slope
    ) ;
    int HydrologyTableSize(void) ;
    DtmHydrologyTable::iterator FirstHydrology(void) ;

    private:
    
//  Class Variables    
    
    bool m_tablesCreated ;
    BC_DTM_OBJ* m_dtm ;  
    DtmHydrologyTable   m_hydrologyTable ;
      
//  Method To Create Hydrology Table

   int CreateHydrologyTable( DTMTriangleIndex& triangleIndex ) ;
    
} ;

/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Create And Index A Low Point Pond Table                  |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMLowPointPondTable
  {

    public :

    DTMLowPointPondTable()
      {
      } 
      
    ~DTMLowPointPondTable()
      {
      } 
      
//  Public Methods  

    int StoreLowPointPond( int point, int exitPoint, int priorPoint, int nextPoint ) ;
    int SortLowPointPondTable(void) ;
    int FindLowPointPond( int point, int& exitPoint, int& priorPoint, int& nextPoint ) ;
    int SizeOfLowPointPondTable(void) ;
    int ClearLowPointPondTable(void) ;
    
    private:
    
//  Class Variables    

    DtmLowPointPondTable  m_lowPointPondTable;
    
//  Private Methods  

    static bool LowPointCompare (DTM_LOW_POINT_POND_TABLE& m1,DTM_LOW_POINT_POND_TABLE& m2)
      {
       return m1.lowPoint < m2.lowPoint ;
      }
   
  } ;  

/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Create And Index A Sump Line Pond Table                  |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMSumpLinePondTable
  {

    public :

    DTMSumpLinePondTable()
      {
      } 
      
    ~DTMSumpLinePondTable()
      {
      } 
      
//  Public Methods  

    int StoreZeroSlopeLinePond( int point1,int point2, int exitPoint, int priorPoint, int nextPoint ) ;
    int SortZeroSlopeLinePondTable(void) ;
    int FindZeroSlopeLinePond( int point1,int point2, int& exitPoint, int& priorPoint, int& nextPoint ) ;
    int SizeOfZeroSlopeLinePondTable(void) ;
    int ClearZeroSlopeLinePondTable(void) ;
    
    private:
    
//  Class Variables    

    DtmSumpLinePondTable  m_sumpLinePondTable;
    
//  Private Methods  

    static bool ZeroSlopeLineCompare (DTM_SUMP_LINE_POND_TABLE& m1,DTM_SUMP_LINE_POND_TABLE& m2)
      {
       if( m1.sP1 < m2.sP1 ) return( true ) ;
       if( m1.sP1 > m2.sP1 ) return( false ) ;
       if( m1.sP2 < m2.sP2 ) return( true) ;
       if( m1.sP2 > m2.sP2 ) return( false ) ;
       return(true) ;
      }
   
  } ;  


/*-------------------------------------------------------------------+
|                                                                    |
|  Class To Create And Query DTM Drainage Tables                     |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMDrainageTables
{
    public :
     
    DTMDrainageTables(BC_DTM_OBJ *dtmP ) : m_drainageTriangleIndex(dtmP) , m_drainageHydrologyTable(m_drainageTriangleIndex) 
    {
     m_numPoints     = dtmP->numPoints;
     m_numLines      = dtmP->numLines ;
     m_numTriangles  = dtmP->numTriangles ;
     m_numFeatures   = dtmP->numFeatures ;
     m_modifiedTime  = dtmP->modifiedTime ;
    } 
    
    ~DTMDrainageTables()
    {
    } 
 
    bool CheckForDtmChange
    (
     int numPoints,                 // ==> Number Of DTM Points
     int numLines,                  // ==> Number Of DTM Lines
     int numTriangles,              // ==> Number Of DTM Triangles
     int numFeatures,               // ==> Number Of DTM Features 
     time_t modifiedTime        // ==> Modified Time
     ) ;
     
    int GetTriangleHydrologyParameters
    ( 
     int trgPnt1,             // ==> Triangle Point 1
     int trgPnt2,             // ==> Triangle Point 2
     int trgPnt3,             // ==> Triangle Point 3
     bool& trgFound,          // <== True = Triangle Found , False = Triangle Not Found
     int& flowPnt,            // <== Not Used. Intended For Latter Development
     int& flowDir1,           // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
     int& flowDir2,           // <== Flow Direction Triangle Edge ** trgPnt2 - trgPnt3  
     int& flowDir3,           // <== Flow Direction Triangle Edge ** trgPnt3 - trgPnt1 
     double& ascentAngle,     // <== Maximum Ascent Angle 
     double& descentAngle,    // <== Maximum Descent Angle
     double& slope            // <== Triangle Slope
    ) ;
    
    int GetTriangleEdgeFlowDirection
    ( 
     // 
     //   Gets The Fow Direction For Triangle Edge trgpnt1 - trgpnt2
     //   Note : trgPnt1 - trgPnt2 - trgPnt3 Must Have A Clockwise Direction
     //          As The Drainage Utilities Trace In a CCW Direction The Flow Direction Is mapped To A CCW Value
     //
     //   Flow Direction Values
     //     >=  1  Direction Flows To The Edge
     //     ==  0  Direction Flows Parallel To The Edge
     //     <= -1  Direction Flows Away From The Edge
     //
     int   trgPnt1,           // ==> Triangle Point 1
     int   trgPnt2,           // ==> Triangle Point 2
     int   trgPnt3,           // ==> Triangle Point 3
     bool& trgFound,          // <== True = Triangle Found , False = Triangle Not Found
     bool& voidTriangle,      // <== True If Void Triangle Else False
     int&  flowDirection      // <== Flow Direction Triangle Edge ** trgPnt1 - trgPnt2 
    ) ; 

    int GetTriangleSlopeAndSlopeAngles
    ( 
     // 
     //   Gets The Triangle Slope And Slope Angles
     //   Note : Triangle Points trgPnt1 - trgPnt2 - trgPnt3 Must Have A Clockwise Direction
     //

     int     trgPnt1,                  // ==> Triangle Point 1
     int     trgPnt2,                  // ==> Triangle Point 2
     int     trgPnt3,                  // ==> Triangle Point 3
     bool&   trgFound,                 // <== True = Triangle Found , False = Triangle Not Found 
     bool&   voidTriangle,             // <== True If Void Triangle Otherwise False
     double& ascentAngle,              // <== Triangle Ascent  
     double& descentAngle,             // <== Triangle Descent Angle 
     double& slope                     // <== Triangle Slope
    ) ;
 
    int StoreLowPointPond                     //   Stores A Low Point Pond Table Entry
    ( 
     int lowPoint,                 // ==> Low Point
     int exitPoint,                // ==> Pond Exit Point
     int priorPoint,               // ==> Prior Point To  Exit Point On Tptr Polygon
     int nextPoint                 // ==> Next Point From Exit Point On Tptr Polygon 
    ) ;
 
    int SortLowPointPondTable(void) ;         // Sorts The Low Point Pond Table
    
    int SizeOfLowPointPondTable(void) ;       // Returns The Size Of The Low Point Pond Table
 
    int FindLowPointPond                      // Finds A Low Point Pond Table Entry
    ( 
     int lowPoint,                 // ==> Low Point
     int& exitPoint,               // <== Pond Exit Point
     int& priorPoint,              // <== Prior Point To  Exit Point On Tptr Polygon
     int& nextPoint                // <== Next Point From Exit Point On Tptr Polygon 
    ) ;

    int StoreZeroSlopeLinePond                //   Stores A Zero Slope Line Pond Table Entry
    ( 
     int point1,                   // ==> Point One End Of Zero Slope Line
     int point2,                   // ==> Point Other End Of Zero Slope Line
     int exitPoint,                // ==> Pond Exit Point
     int priorPoint,               // ==> Prior Point To  Exit Point On Tptr Polygon
     int nextPoint                 // ==> Next Point From Exit Point On Tptr Polygon 
    ) ;

    int SizeOfTriangleIndex(void) ;           // Returns Size Of Triangle Index

    DTMTriangleIndex* GetTriangleIndex() ;    // Returns A pointer To the Triangle Index
 
    int SortZeroSlopeLinePondTable(void) ;    // Sorts The Zero Slope Line Pond Table
    
    int SizeOfZeroSlopeLinePondTable(void) ;  // Returns The Size Of The Zero Slope Line Pond Table
 
    int FindZeroSlopeLinePond                 // Finds A Zero Slope Line Pond Table Entry
    ( 
     int point1,                   // ==> Point One End Of Zero Slope Line
     int point2,                   // ==> Point Other End Of Zero Slope Line
     int& exitPoint,               // <== Pond Exit Point
     int& priorPoint,              // <== Prior Point To  Exit Point On Tptr Polygon
     int& nextPoint                // <== Next Point From Exit Point On Tptr Polygon 
    ) ;
    
    int ClearPondTables(void)  ;              // Clears The Pond Tables 

    private :
    
    //  Class Variables    
    
    int                   m_numPoints ;
    int                   m_numLines ;
    int                   m_numTriangles ;
    int                   m_numFeatures ;
    time_t            m_modifiedTime ;
    DTMTriangleIndex      m_drainageTriangleIndex ;
    DTMHydrologyTable     m_drainageHydrologyTable ;
    DTMLowPointPondTable  m_drainageLowPointPondTable ;
    DTMSumpLinePondTable  m_drainageSumpLinePondTable ;
} ;

/*-------------------------------------------------------------------+
|                                                                    |
|  Structure To Store DTM Zero Slope Polygons                        |
|                                                                    |
+-------------------------------------------------------------------*/

struct DTMZeroSlopePolygon
    {
     int  index      ;
     DTMDirection direction  ;
     int  priorPoint ; 
     int  exitPoint  ;
     int  nextPoint  ;  
     DTMPointList pointList ;
     
     DTMZeroSlopePolygon(DTMDirection direction, const DTMPointList& pointList) : pointList(pointList)
       {
        this->index      =   0 ;
        this->priorPoint = - 1 ;
        this->exitPoint  = - 1 ;
        this->nextPoint  = - 1 ;
        this->direction  = direction ;
        this->pointList  = pointList ;
       } 

     ~DTMZeroSlopePolygon()
       {
       } 
       
    }; 
     
typedef  bvector<DTMZeroSlopePolygon>  DTMZeroSlopePolygonVector ;
     
     

BENTLEYDTMDRAINAGE_EXPORT int bcdtmTables_createAndCheckDrainageTablesDtmObject(BC_DTM_OBJ *dtmP) ;
    
    
END_BENTLEY_TERRAINMODEL_NAMESPACE
