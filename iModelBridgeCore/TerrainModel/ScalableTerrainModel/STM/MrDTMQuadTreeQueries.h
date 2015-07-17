//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/MrDTMQuadTreeQueries.h $
//:>    $RCSfile: MrDTMQuadTreeQueries.h,v $
//:>   $Revision: 1.12 $
//:>       $Date: 2012/11/29 17:30:30 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HGFPointIndex.h>

#include <ImagePP/all/h/IDTMTypes.h>

#include <ScalableTerrainModel/MrDTMUtilityFunctions.h>

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/** -----------------------------------------------------------------------------
    The MrDTMQuadTreeViewDependentPointQuery class is used for the query of
    a point quadtree index content based on a view dependent fashion. The query
    takes into account the view box and point of view of the display to be 
    realised to query individual point nodes according to required density.
    ----------------------------------------------------------------------------- 
*/

template<class POINT, class EXTENT> class MrDTMQuadTreeViewDependentPointQuery : public HGFViewDependentPointIndexQuery<POINT, EXTENT>
{

    private: 
         
        HAutoPtr<HGFLevelPointIndexQuery<POINT, EXTENT>> m_pQueryByLevel;

        //Parameters controlling the tile selection 
        double m_meanScreenPixelsPerPoint;                
        bool   m_useSameResolutionWhenCameraIsOff;         //Determine if the same or different resolution method is used when the camera is off        
        bool   m_useSplitThresholdForLevelSelection;        
        bool   m_useSplitThresholdForTileSelection;             

        // The viewbox to query in. This viewbox is expressed in the coordinate system and units
        // of the STM to be queiried upon.
        // If reprojection is used (when both m_sourceGCSPtr and m_targetGCSPtr are set) then
        // this member will contain the viewbox as reprojected in the STM GCS.
        DPoint3d                                         m_viewBox[8];
        // The GCS of the STM
        BaseGCSPtr                                       m_sourceGCSPtr;

        // The GCS of the target
        BaseGCSPtr                                       m_targetGCSPtr;

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
            return myExtent;
            }
                                            
    public:

        // Primary methods

        MrDTMQuadTreeViewDependentPointQuery(const EXTENT&      extent, 
                                             const double       rootToViewMatrix[][4],
                                             const double       viewportRotMatrix[][3],                                                                 
                                             bool               gatherTileBreaklines)
                            : HGFViewDependentPointIndexQuery(extent, rootToViewMatrix, viewportRotMatrix, gatherTileBreaklines)
                            {  
                            m_meanScreenPixelsPerPoint = MEAN_SCREEN_PIXELS_PER_POINT;

                            m_useSameResolutionWhenCameraIsOff = false;                
                            m_useSplitThresholdForLevelSelection = true;        
                            m_useSplitThresholdForTileSelection = false;    

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
                            MrDTMQuadTreeViewDependentPointQuery(const EXTENT&  extent, 
                                                                 const double   rootToViewMatrix[][4],
                                                                 const double   viewportRotMatrix[][3],                                                                 
                                                                 const DPoint3d viewBox[],                                                                 
                                                                 bool           gatherTileBreaklines)
                            : HGFViewDependentPointIndexQuery(extent, rootToViewMatrix, viewportRotMatrix, gatherTileBreaklines)
                            {  
                            m_meanScreenPixelsPerPoint = MEAN_SCREEN_PIXELS_PER_POINT;

#ifdef ACTIVATE_NODE_QUERY_TRACING
                            m_pTracingXMLFileName = "";
                            m_pTracingXMLFile     = 0;
#endif                                                
                            memcpy(m_viewBox, viewBox, sizeof(DPoint3d) * 8);                                
                            }
                                                        
        virtual             ~MrDTMQuadTreeViewDependentPointQuery() {};
       
        virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                            list<POINT>& points);        
        virtual bool        GlobalPreQuery (HGFPointIndex<POINT, EXTENT>& index,
                                            HPMMemoryManagedVector<POINT>& points);        

        // Specific Query implementation
        virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   list<POINT>& resultPoints); 
        virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HPMMemoryManagedVector<POINT>& resultPoints); 

        virtual bool        IsCorrectForCurrentView(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node,
                                                    const EXTENT& pi_visibleExtent,
                                                    int           pi_NearestPredefinedCameraOri,
                                                    double        pi_RootToViewMatrix[][4]) const;

        virtual void        SetMeanScreenPixelsPerPoint (double meanScreenPixelsPerPoint) {m_meanScreenPixelsPerPoint = meanScreenPixelsPerPoint;}
        virtual double      GetMeanScreenPixelsPerPoint () {return m_meanScreenPixelsPerPoint;}        

        void                SetUseSameResolutionWhenCameraIsOff(bool useSameResolution) {m_useSameResolutionWhenCameraIsOff = useSameResolution;}                    
        void                SetUseSplitThresholdForLevelSelection(bool useSplitThreshold) {m_useSplitThresholdForLevelSelection = useSplitThreshold;}                        
        void                SetUseSplitThresholdForTileSelection(bool useSplitThreshold) {m_useSplitThresholdForTileSelection = useSplitThreshold;}        
                        
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
        void SetReprojectionInfo(BaseGCSPtr& pi_sourceGCSPtr,
                                 BaseGCSPtr& pi_targetGCSPtr)
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
};


/*======================================================================================================
** MrDTMQuadTreeLevelPointIndexQuery 
**
** This query fetches points within a specified area at a specified depth level. If the level
** requested is deeper than the maximum depth of the index then it is automatically limited
** to this level.
**====================================================================================================*/

template<class POINT, class EXTENT> class MrDTMQuadTreeLevelPointIndexQuery : public  HGFLevelPointIndexQuery<POINT, EXTENT>
{

    private:
        
        DPoint3d   m_viewBox[8];         
   
    public:

                            MrDTMQuadTreeLevelPointIndexQuery(const EXTENT   extent, 
                                                              size_t         level,                                                     
                                                              const DPoint3d viewBox[]) 
                            : HGFLevelPointIndexQuery(extent, level)                              
                                {                                                             
                                memcpy(m_viewBox, viewBox, sizeof(DPoint3d) * 8);                                
                                }                            

                            virtual ~MrDTMQuadTreeLevelPointIndexQuery() {}

        
        // The Query process gathers points up to level depth
        virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   list<POINT>& resultPoints);
        virtual bool        Query (HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                   HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                   size_t numSubNodes,
                                   HPMMemoryManagedVector<POINT>& resultPoints);

        

};        

#include "MrDTMQuadTreeQueries.hpp"