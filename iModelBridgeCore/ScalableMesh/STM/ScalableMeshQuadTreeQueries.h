//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeQueries.h $
//:>    $RCSfile: ScalableMeshQuadTreeQueries.h,v $
//:>   $Revision: 1.12 $
//:>       $Date: 2012/11/29 17:30:30 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "SMPointIndex.h"

//#include <ImagePP/all/h/IDTMTypes.h>

#include <ScalableMesh/ScalableMeshUtilityFunctions.h>

#include "ScalableMeshQuery.h"

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/** -----------------------------------------------------------------------------
    The ScalableMeshQuadTreeViewDependentPointQuery class is used for the query of
    a point quadtree index content based on a view dependent fashion. The query
    takes into account the view box and point of view of the display to be 
    realised to query individual point nodes according to required density.
    ----------------------------------------------------------------------------- 
*/


template<class POINT, class EXTENT> class ScalableMeshQuadTreeViewDependentPointQuery : public HGFViewDependentPointIndexQuery<POINT, EXTENT>
{

    protected: 
                 
        //Parameters controlling the tile selection 
        double m_meanScreenPixelsPerPoint;           
        double m_maxPixelError; //for datasets with a precomputed resolution in units
        
        // The viewbox to query in. This viewbox is expressed in the coordinate system and units
        // of the STM to be queiried upon.
        // If reprojection is used (when both m_sourceGCSPtr and m_targetGCSPtr are set) then
        // this member will contain the viewbox as reprojected in the STM GCS.
        DPoint3d                                         m_viewBox[8];
        // The GCS of the STM
        BaseGCSCPtr                                       m_sourceGCSPtr;

        // The GCS of the target
        BaseGCSCPtr                                       m_targetGCSPtr;

    protected: 

#ifdef ACTIVATE_NODE_QUERY_TRACING
        AString m_pTracingXMLFileName;
        FILE*  m_pTracingXMLFile;        
#endif

        void MoveIntersectionPointFromVanishingLine(const HGF2DLocation& intersectionPoint, 
                                                    const HGF2DLocation& pointOnIntersectedSegment, 
                                                    HGF2DLocation&       movedIntersectionPoint) const;

        EXTENT ComputeExtent(HFCPtr<HVE2DShape> shape)
            {
            HGF2DExtent shapeExtent;
            EXTENT myExtent;
            if (shape != NULL)
                { 
                shapeExtent = shape->GetExtent();
                myExtent = ExtentOp<EXTENT>::Create(shapeExtent.GetXMin(), 
                                                       shapeExtent.GetYMin(),
                                                       shapeExtent.GetXMax(),
                                                       shapeExtent.GetYMax());
                }
            else
                {
                myExtent = ExtentOp<EXTENT>::Create(0, 0, 0, 0);
                }

            return myExtent;
            }
                                            
    public:

        // Primary methods

        ScalableMeshQuadTreeViewDependentPointQuery(const EXTENT&      extent, 
                                             const double       rootToViewMatrix[][4],
                                             const double       viewportRotMatrix[][3],                                                                 
                                             bool               gatherTileBreaklines)
                            : HGFViewDependentPointIndexQuery<POINT, EXTENT>(extent, rootToViewMatrix, viewportRotMatrix, gatherTileBreaklines)
                            {  
                            m_meanScreenPixelsPerPoint = MEAN_SCREEN_PIXELS_PER_POINT;                            
                            m_maxPixelError = 1.0;
#ifdef ACTIVATE_NODE_QUERY_TRACING
                            m_pTracingXMLFileName = "";
                            m_pTracingXMLFile     = 0;
#endif                            
                            }


                            
                            /*---------------------------------------------------------------------------------**//**
                            * Creates the view dependant query. 
                            * The viewbox must be given in the target GCS if a coordinate system will be set
                            * with SetReprojectionInfo and in the units of UOR. If no coordinate system will be set
                            * then the viewbox is given in the units defined using uorsPerMeter.
                            * The uorsPerMeter parameter is used as an immediate scale factor to convert the given
                            * viewport into a unit that can be related to the units of the STM. Since usually
                            * STM units are in meters, the scale factor provided is thus sufficient to convert
                            * the viewbox provided in STM coordinates and units. This uorPerMeter value is unused 
                            * afterward.
                            * @bsimethod                                                    AlainRobert  08/2011
                            +---------------+---------------+---------------+---------------+---------------+------*/
                            ScalableMeshQuadTreeViewDependentPointQuery(const EXTENT&  extent, 
                                                                 const double   rootToViewMatrix[][4],
                                                                 const double   viewportRotMatrix[][3],                                                                 
                                                                 const DPoint3d viewBox[],                                                                 
                                                                 bool           gatherTileBreaklines)
                            : HGFViewDependentPointIndexQuery<POINT, EXTENT>(extent, rootToViewMatrix, viewportRotMatrix, gatherTileBreaklines)
                            {  
                            m_meanScreenPixelsPerPoint = MEAN_SCREEN_PIXELS_PER_POINT;
                            m_maxPixelError = 1.0;

#ifdef ACTIVATE_NODE_QUERY_TRACING
                            m_pTracingXMLFileName = "";
                            m_pTracingXMLFile     = 0;
#endif                                                
                            memcpy(m_viewBox, viewBox, sizeof(DPoint3d) * 8);                                
                            }
                                                        
        virtual             ~ScalableMeshQuadTreeViewDependentPointQuery() {};
               
     /*   virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                            HPMMemoryManagedVector<POINT>& points);   */     

        // Specific Query implementation        
        /*virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HPMMemoryManagedVector<POINT>& resultPoints); */

        virtual bool        IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                    const EXTENT& pi_visibleExtent,                                                    
                                                    double        pi_RootToViewMatrix[][4]) const;

        virtual void        SetMeanScreenPixelsPerPoint (double meanScreenPixelsPerPoint) {m_meanScreenPixelsPerPoint = meanScreenPixelsPerPoint;}
        virtual double      GetMeanScreenPixelsPerPoint () {return m_meanScreenPixelsPerPoint;}        
  

        virtual void        SetMaxPixelError(double errorinPixels) { m_maxPixelError = errorinPixels; }
        virtual double      GetMaxPixelError() { return m_maxPixelError; }
                        
#ifdef ACTIVATE_NODE_QUERY_TRACING
        void                SetTracingXMLFileName(AString& pi_rTracingXMLFileName);

        void                OpenTracingXMLFile();

        void                CloseTracingXMLFile();
#endif

         /*---------------------------------------------------------------------------------**//**
         * Sets the reprojection information.
         * This method sets the STM and target GCS. All this function does is reproject the 
         * viewbox to the STM coordinate system. My guess is that such operation should probably
         * have been performed outside the present class and the viewbox be provided already
         * properly converted to the source STM GCS.
         * The viewbox provided at the moment of the contruction was in the units defined by the
         * uorPerMeter field and have likewise been converted to meters. Noew these viewbox
         * will be converted to the units of the targerGCS then reprojected to the source GCS
         * fully ready for querying. 
         +---------------+---------------+---------------+---------------+---------------+------*/
        void SetReprojectionInfo(BaseGCSCPtr& pi_sourceGCSPtr,
                                 BaseGCSCPtr& pi_targetGCSPtr)
            {            
            m_sourceGCSPtr = pi_sourceGCSPtr;
            m_targetGCSPtr = pi_targetGCSPtr;

            // Here we have the viewbox expressed in the cartesian target GCS yet this viewbox may exceed the
            // Target GCS mathematical domain. It is also likely that it will exceeed in addition the source GCS
            // mathematical domain. For this reason, we call the appropriate reprojection function that will
            // perform the clipping of the viewbox to the domains
            // It is unsure what the result viewbox shape will be. 
            // Since the wiewbox is a 3D object but the operations for clipping to the mathematical domains is 2D
            // we will preserve the elevations separately.
            DRange3d queryExtent;

            queryExtent.low.x = ExtentOp<EXTENT>::GetXMin(m_extent);
            queryExtent.low.y = ExtentOp<EXTENT>::GetYMin(m_extent);
            queryExtent.low.z = ExtentOp<EXTENT>::GetZMin(m_extent);
            queryExtent.high.x = ExtentOp<EXTENT>::GetXMax(m_extent);
            queryExtent.high.y = ExtentOp<EXTENT>::GetYMax(m_extent);
            queryExtent.high.z = ExtentOp<EXTENT>::GetZMax(m_extent);

            GetReprojectedBoxDomainLimited(m_targetGCSPtr, m_sourceGCSPtr, m_viewBox, m_viewBox, queryExtent, NULL);    

            // Now the viewbox is in the coordinates of the STM whatever the units be.
            }

        virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
        {
            auto ret = new ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>(m_extent, m_rootToViewMatrix, m_viewportRotMatrix, m_viewBox, m_gatherTileBreaklines);
            ret->SetMaxPixelError(GetMaxPixelError());
            ret->SetMeanScreenPixelsPerPoint(GetMeanScreenPixelsPerPoint());
            return ret;
        }
};


/*======================================================================================================
** ScalableMeshQuadTreeLevelPointIndexQuery 
**
** This query fetches points within a specified area at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level.
**====================================================================================================*/

template<class POINT, class EXTENT> class ScalableMeshQuadTreeLevelPointIndexQuery : public  HGFLevelPointIndexQuery<POINT, EXTENT>
{

    private:
        
        DPoint3d   m_viewBox[8];         
   
    public:

                            ScalableMeshQuadTreeLevelPointIndexQuery(const EXTENT   extent, 
                                                              size_t         level,                                                     
                                                              const DPoint3d viewBox[]) 
                            : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level)                              
                                {                                                             
                                memcpy(m_viewBox, viewBox, sizeof(DPoint3d) * 8);                                
                                }                            

                            virtual ~ScalableMeshQuadTreeLevelPointIndexQuery() {}

        
        // The Query process gathers points up to level depth        
        /*virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HPMMemoryManagedVector<POINT>& resultPoints);*/

        
                            virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
                            {
                                auto ret = new ScalableMeshQuadTreeLevelPointIndexQuery<POINT, EXTENT>(m_extent, m_requestedLevel, m_viewBox);
                                return ret;
                            }
};        

/*======================================================================================================
** ScalableMeshQuadTreeLevelMeshIndexQuery 
**
** This query fetches points within a specified area at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level.
**====================================================================================================*/

template<class POINT, class EXTENT> class ScalableMeshQuadTreeLevelMeshIndexQuery : public  HGFLevelPointIndexQuery<POINT, EXTENT>
{

    private:
        
        DPoint3d   m_viewBox[8];         
        ClipVectorCP m_extent3d;
        bool m_useAllRes;
        bool m_alwaysVisible;
        bool m_includeUnbalancedLeafs;
        bool m_ignoreFaceIndexes;
        double m_pixelTolerance;        

    public:

        ScalableMeshQuadTreeLevelMeshIndexQuery(const EXTENT   extent,
                                                size_t         level,
                                                const DPoint3d viewBox[],
                                                double         pixelTol=0.0)
            : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level), m_useAllRes(false), m_extent3d(nullptr), m_alwaysVisible(false), m_includeUnbalancedLeafs(true), m_ignoreFaceIndexes(false), m_pixelTolerance(pixelTol)
            {
            memcpy(m_viewBox, viewBox, sizeof(DPoint3d) * 8);
            }

        ScalableMeshQuadTreeLevelMeshIndexQuery(const EXTENT   extent,
                                                size_t         level,
                                                bool           alwaysVisible,
                                                bool           includeUnbalancedLeafs,
                                                bool           ignoreIndexes)
            : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level), m_useAllRes(false), m_extent3d(nullptr), m_alwaysVisible(alwaysVisible), m_includeUnbalancedLeafs(includeUnbalancedLeafs), m_ignoreFaceIndexes(ignoreIndexes), m_pixelTolerance(0.0)
            {

            }

        ScalableMeshQuadTreeLevelMeshIndexQuery(const EXTENT   extent,
                                                size_t         level,
                                                ClipVectorCP   extent3d,
                                                bool           useAllResolutions,
                                                double         pixelTol = 0.0)
            : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level), m_useAllRes(useAllResolutions), m_extent3d(extent3d), m_alwaysVisible(false), m_includeUnbalancedLeafs(true), m_ignoreFaceIndexes(false), m_pixelTolerance(pixelTol)
            {

            }


        virtual ~ScalableMeshQuadTreeLevelMeshIndexQuery() {}


        // The Query process gathers points up to level depth                
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh);           
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);

        virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
        {
            auto ret = new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>(m_extent, m_requestedLevel,m_viewBox, m_pixelTolerance);
            ret->m_useAllRes = m_useAllRes;
            ret->m_alwaysVisible = m_alwaysVisible;
            ret->m_extent3d = m_extent3d;
            ret->m_ignoreFaceIndexes = m_ignoreFaceIndexes;
            ret->m_includeUnbalancedLeafs = m_includeUnbalancedLeafs;
            return ret;
        }

        void SetIgnoreFaceIndexes(bool ignoreIndexes)
        {
            m_ignoreFaceIndexes = ignoreIndexes;
        }
};     

/*
*   Query for retrieving nodes that intersect a given ray. Will only return nodes at chosen level that have points in them.
*   The optional parameter intersectType decides if using first or last node hit in direction of ray.
*
*   There are 2 kinds of intersect queries. The 2D queries return nodes if, when projected on the XY plane, their extent intersects the
*   ray also projected on the XY plane. The 3D queries are a full ray intersect against the node's box. Both kinds of queries can be constrained using 
*   a depth parameter corresponding to maximum length along the ray (allowing bounded queries, default is unbounded) and both can return the first, last or 
*   all intersects along the ray. "ALL_INTERSECT" queries will return nodes sorted along the ray direction.
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeLevelIntersectIndexQuery : public  HGFLevelPointIndexQuery<POINT, EXTENT>
{
public:
    enum RaycastOptions
        {
        FIRST_INTERSECT,
        LAST_INTERSECT,
        ALL_INTERSECT
        };

    private:
        
        RaycastOptions m_intersect;
        DRay3d m_target;
        double m_bestHitScore;
        double m_depth;
        bool m_is2d;
        bool m_useUnboundedRay;
        bvector<double> m_fractions;
   
    public:

        ScalableMeshQuadTreeLevelIntersectIndexQuery(const EXTENT   extent,
                                                     size_t         level,
                                                     DRay3d ray,
                                                     bool is2d = false,
                                                     double depth = -1,
                                                     bool useUnboundedRay = true,
                                                               RaycastOptions intersectType = RaycastOptions::LAST_INTERSECT)
                                                               : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level), m_intersect(intersectType), m_target(ray), m_bestHitScore(numeric_limits<double>::quiet_NaN()), m_is2d(is2d), m_depth(depth), m_useUnboundedRay(useUnboundedRay)
                                {                                                             
                                }                            

                            virtual ~ScalableMeshQuadTreeLevelIntersectIndexQuery() {}

                            void       SetLevel(size_t level)
                            {
                                m_requestedLevel = level;
                            }
        
        // The Query process gathers points up to level depth              
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode);
        virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);

        virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
        {
            auto ret = new ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, EXTENT>(m_extent, m_requestedLevel, m_target, m_is2d, m_depth, m_useUnboundedRay, m_intersect);
            return ret;
        }
};    

/*
*   Query for retrieving nodes that intersect a given plane. Will only return nodes at chosen level that have points in them.
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery : public  HGFLevelPointIndexQuery<POINT, EXTENT>
{
public:


    private:
       
        DPlane3d m_target;
        double m_bestHitScore;
        double m_depth;
   
    public:

        ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery(const EXTENT   extent,
                                                              size_t         level,                                                    
                                                               DPlane3d plane,
                                                               double depth = -1)
                                                               : HGFLevelPointIndexQuery<POINT,EXTENT>(extent, level),  m_target(plane), m_bestHitScore(numeric_limits<double>::quiet_NaN()),m_depth(depth)
                                {                                                             
                                }                            

                            virtual ~ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery() {}

        
        // The Query process gathers points up to level depth               
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode);
        virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                  HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                  size_t numSubNodes,
                                  vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);

        virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
        {
            auto ret = new ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>(m_extent, m_requestedLevel, m_target, m_depth);
            return ret;
        }

};    

/** -----------------------------------------------------------------------------
    The ScalableMeshQuadTreeViewDependentPointQuery class is used for the query of
    a point quadtree index content based on a view dependent fashion. The query
    takes into account the view box and point of view of the display to be 
    realised to query individual point nodes according to required density.
    ----------------------------------------------------------------------------- 
*/
template<class POINT, class EXTENT> class ScalableMeshQuadTreeViewDependentMeshQuery : public ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>
{

    private: 

        size_t        m_maxNumberOfPoints;

        bool m_invertClips;
                 
    protected: 
        ClipVectorPtr m_viewClipVector;
    public:

        // Primary methods

        ScalableMeshQuadTreeViewDependentMeshQuery(const EXTENT& extent, 
                                            const double         rootToViewMatrix[][4],
                                            const double         viewportRotMatrix[][3],                                                                 
                                            bool                 gatherTileBreaklines, 
                                            const ClipVectorPtr& viewClipVector,
                                            bool invertClips = false,
                                            size_t               maxNumberOfPoints = std::numeric_limits<std::size_t>::max())
        : ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>(extent,
                                              rootToViewMatrix,
                                              viewportRotMatrix,                                                                 
                                              gatherTileBreaklines)
            {              
            m_maxNumberOfPoints = maxNumberOfPoints;
            m_invertClips = invertClips;
            m_viewClipVector = viewClipVector;
            }


                            
                            /*---------------------------------------------------------------------------------**//**
                            * Creates the view dependant query. 
                            * The viewbox must be given in the target GCS if a coordinate system will be set
                            * with SetReprojectionInfo and in the units of UOR. If no coordinate system will be set
                            * then the viewbox is given in the units defined using uorsPerMeter.
                            * The uorsPerMeter parameter is used as an immediate scale factor to convert the given
                            * viewport into a unit that can be related to the units of the STM. Since usually
                            * STM units are in meters, the scale factor provided is thus sufficient to convert
                            * the viewbox provided in STM coordinates and units. This uorPerMeter value is unused 
                            * afterward.
                            * @bsimethod                                                    AlainRobert  08/2011
                            +---------------+---------------+---------------+---------------+---------------+------*/
                            ScalableMeshQuadTreeViewDependentMeshQuery(const EXTENT&  extent, 
                                                                 const double         rootToViewMatrix[][4],
                                                                 const double         viewportRotMatrix[][3],                                                                 
                                                                 const DPoint3d       viewBox[],                                                                 
                                                                 bool                 gatherTileBreaklines, 
                                                                 const ClipVectorPtr& viewClipVector,
                                                                 bool invertClips = false,
                                                                 size_t               maxNumberOfPoints = std::numeric_limits<std::size_t>::max())
                            : ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>(extent, 
                                                                  rootToViewMatrix,
                                                                  viewportRotMatrix,                                                                 
                                                                  viewBox,                                                                 
                                                                  gatherTileBreaklines)                                                                                    
                            {                                                            
                            m_maxNumberOfPoints = maxNumberOfPoints;
                            m_viewClipVector = viewClipVector;
                            m_invertClips = invertClips;
                            }
                                                        
        virtual             ~ScalableMeshQuadTreeViewDependentMeshQuery() {};
                      
        virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                            BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh);
        virtual bool        GlobalPreQuery (SMPointIndex<POINT, EXTENT>& index,
                                            vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);

        // Specific Query implementation              
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh);
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);
        virtual bool        Query (HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                   HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   ProducedNodeContainer<POINT, EXTENT>& foundNodes);

        virtual void        GetQueryNodeOrder(vector<size_t>&                           queryNodeOrder, 
                                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                              size_t                                    numSubNodes) override; 

        virtual bool        IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                    const EXTENT& pi_visibleExtent,                                                    
                                                    double        pi_RootToViewMatrix[][4]) const;

        virtual bool        IsCorrectForCurrentViewSphere(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                          const EXTENT&                           i_visibleExtent,                                                          
                                                          double                                  i_RootToViewMatrix[][4],
                                                          bool& shouldAddNode) const;

        virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
        {
            auto ret = new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>(m_extent, m_rootToViewMatrix, m_viewportRotMatrix, m_viewBox, m_gatherTileBreaklines, m_viewClipVector, m_invertClips, m_maxNumberOfPoints);
            ret->SetMaxPixelError(GetMaxPixelError());
            ret->SetMeanScreenPixelsPerPoint(GetMeanScreenPixelsPerPoint());
            return ret;
        }
        
};


template<class POINT, class EXTENT> class ScalableMeshQuadTreeContextMeshQuery : public ScalableMeshQuadTreeViewDependentMeshQuery<POINT,EXTENT>
{
public:
    ScalableMeshQuadTreeContextMeshQuery(const EXTENT& extent,
                                         const double         rootToViewMatrix[][4],
                                         const double         viewportRotMatrix[][3],
                                         const ClipVectorPtr& viewClipVector)
                                         :ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>(extent, rootToViewMatrix, viewportRotMatrix, false, viewClipVector)
        {
        }

    virtual             ~ScalableMeshQuadTreeContextMeshQuery() {};

    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                              size_t numSubNodes,
                              BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh);
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                              size_t numSubNodes,
                              vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes);
    virtual bool        Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                              HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                              size_t numSubNodes,
                              ProducedNodeContainer<POINT, EXTENT>& foundNodes);

    virtual ISMPointIndexQuery<POINT, EXTENT>* Clone()
    {
        auto ret = new ScalableMeshQuadTreeContextMeshQuery<POINT, EXTENT>(m_extent, m_rootToViewMatrix, m_viewportRotMatrix, m_viewClipVector);
        ret->SetMaxPixelError(GetMaxPixelError());
        ret->SetMeanScreenPixelsPerPoint(GetMeanScreenPixelsPerPoint());
        return ret;
    }
};

//#include "ScalableMeshQuadTreeQueries.hpp"
