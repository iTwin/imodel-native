//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/MrDTMQuadTreeQueries.hpp $
//:>    $RCSfile: MrDTMQuadTreeQueries.hpp,v $
//:>   $Revision: 1.20 $
//:>       $Date: 2012/11/29 17:30:34 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "InternalUtilityFunctions.h"
/**----------------------------------------------------------------------------
 MrDTMQuadTreeViewDependentPointQuery
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::GlobalPreQuery(HGFPointIndex<POINT, EXTENT>& index,
                                                                                                             list<POINT>&                  points) 
    {            
    bool queryResult;

    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    EXTENT indexExtent(index.GetIndexExtent());




    //Not a projective transformation
    IHGFPointIndexFilter<POINT, EXTENT>* filter = index.GetFilter();

    MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>* pQuadTreeViewDependentFilter = 
                                                                        dynamic_cast<MrDTMQuadTreeBCLIBFilterViewDependent<POINT, 
                                                                                                                           EXTENT>*>(filter);

    if ((m_useSameResolutionWhenCameraIsOff == true) &&
        (pQuadTreeViewDependentFilter != 0) &&
        (m_rootToViewMatrix[3][0] == 0.0) &&
        (m_rootToViewMatrix[3][1] == 0.0) && 
        (m_rootToViewMatrix[3][2] == 0.0) && 
        (m_rootToViewMatrix[3][3] == 1.0) && 
        (m_sourceGCSPtr == 0) &&
        (m_targetGCSPtr == 0))
        {      
        HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, points);
        double             rootToViewScale;

      
        DPoint3d tileBorder[4];

        tileBorder[0].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        tileBorder[0].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        tileBorder[0].z = 0;

        tileBorder[1].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        tileBorder[1].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        tileBorder[1].z = 0;

        tileBorder[2].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        tileBorder[2].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        tileBorder[2].z = 0;

        tileBorder[3].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        tileBorder[3].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        tileBorder[3].z = 0;
        
        bcdtmMultiResolution_getTileAreaInCurrentView(m_rootToViewMatrix, 
                                                      tileBorder, 
                                                      4,
                                                      &rootToViewScale);  
      
        //The splithreshold is a good heuristic but could be replace by something more precise 
        //like the maximum number of points in a leaf (e.g. : a threshold of 5000 could 
        //lead to node having on average between 1250 and 5000 points)
        size_t splitThreshold = index.GetSplitTreshold();	
                    
        double levelVal = 0;
        
        if (rootToViewScale > 0)
            {
            levelVal = log((double)rootToViewScale / (splitThreshold * m_meanScreenPixelsPerPoint)) / log(4.0);
            }

        size_t level;

        //Negative level means that the a 
        //lower resolution should be taken but no such resolution exists.
        if (levelVal < 0)
            {
            level = 0;
            }
        else
            {
            level = (size_t)floor(levelVal);
            }

        if (level > index.GetDepth())
            {
            level = index.GetDepth();
            }
        
        m_pQueryByLevel = new MrDTMQuadTreeLevelPointIndexQuery<POINT, EXTENT>(m_extent, 
                                                                               level,                                                                                
                                                                               m_viewBox); 
        
        //MST - Query the points here for now
        index.Query(m_pQueryByLevel.get(), points);                        

        //MST - Stop the index to query each node since all the points are obtained 
        //      during the global query. Returning false is probably not the best 
        //      mechanism though...
        queryResult = false;
        }
    else
        {
        m_pQueryByLevel = 0;
        queryResult = HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, points); 
        }            

    return queryResult;
    }

template<class POINT, class EXTENT> bool MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::GlobalPreQuery(HGFPointIndex<POINT, EXTENT>& index,
                                                                                                             HPMMemoryManagedVector<POINT>&                  points) 
    {            
    bool queryResult;

    EXTENT indexExtent(index.GetIndexExtent());



    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    //Not a projective transformation
    IHGFPointIndexFilter<POINT, EXTENT>* filter = index.GetFilter();

    MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>* pQuadTreeViewDependentFilter = 
                                                                        dynamic_cast<MrDTMQuadTreeBCLIBFilterViewDependent<POINT, 
                                                                                                                           EXTENT>*>(filter);
    
    if ((m_useSameResolutionWhenCameraIsOff == true) &&
        (pQuadTreeViewDependentFilter != 0) &&
        (m_rootToViewMatrix[3][0] == 0.0) &&
        (m_rootToViewMatrix[3][1] == 0.0) && 
        (m_rootToViewMatrix[3][2] == 0.0) && 
        (m_rootToViewMatrix[3][3] == 1.0) && 
        (m_sourceGCSPtr == 0) &&
        (m_targetGCSPtr == 0))
        {      
        HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, points);
        double             rootToViewScale;
                       
        DPoint3d tileBorder[4];

        tileBorder[0].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        tileBorder[0].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        tileBorder[0].z = 0;

        tileBorder[1].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        tileBorder[1].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        tileBorder[1].z = 0;

        tileBorder[2].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        tileBorder[2].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        tileBorder[2].z = 0;

        tileBorder[3].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        tileBorder[3].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        tileBorder[3].z = 0;
        
        bcdtmMultiResolution_getTileAreaInCurrentView(m_rootToViewMatrix, 
                                                      tileBorder, 
                                                      4,
                                                      &rootToViewScale);  
              
        unsigned __int64 numberOfPointsAtRoot;

        if (m_useSplitThresholdForLevelSelection == true)
            {
            //The splithreshold is a good heuristic but could be replace by something more precise 
            //like the maximum number of points in a leaf (e.g. : a threshold of 5000 could 
            //lead to node having on average between 1250 and 5000 points)        
            numberOfPointsAtRoot = index.GetSplitTreshold();
            }
        else
            {                
            //For dataset with highly varying densities (e.g. : point cloud of a mines taken from 
            //various long range terrestrial LIDAR measurements) using the split threshold leads to
            //a much lower resolution then expected being used. The number of objects at the root is 
            //a better, if not prefect, value in this case.
            numberOfPointsAtRoot = index.GetNbObjectsAtLevel(0);  

            if (numberOfPointsAtRoot == 0)
                {
                numberOfPointsAtRoot = index.GetSplitTreshold();	
                }
            }

        double levelVal;

        if (rootToViewScale > 0)
        {
			levelVal = log((double)rootToViewScale / (numberOfPointsAtRoot * m_meanScreenPixelsPerPoint)) / log(4.0);            
        }
        else
        {
			levelVal = 0;
        }

        size_t level;

        //Negative level means that the a 
        //lower resolution should be taken but no such resolution exists.
        if (levelVal < 0)
            {
            level = 0;
            }
        else
            {
            level = (size_t)floor(levelVal);
            }

        if (level > index.GetDepth())
            {
            level = index.GetDepth();
            }
        
        m_pQueryByLevel = new MrDTMQuadTreeLevelPointIndexQuery<POINT, EXTENT>(m_extent, 
                                                                               level,                                                                                
                                                                               m_viewBox);  
        

        //MST - Query the points here for now
        index.Query(m_pQueryByLevel.get(), points);                        

        //MST - Stop the index to query each node since all the points are obtained 
        //      during the global query. Returning false is probably not the best 
        //      mechanism though...
        queryResult = false;
        }
    else
        {
        m_pQueryByLevel = 0;
        queryResult = HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, points); 
        }            

    return queryResult;
    }


// Specific Query implementation
template<class POINT, class EXTENT> bool MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::Query(HFCPtr<HGFPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    list<POINT>&                             resultPoints)
    {            
    bool queryResult;

    if (m_pQueryByLevel == 0)
        {   
        // Check if extent overlap         
        EXTENT visibleExtent;
        EXTENT nodeExtent;

        if (node->IsEmpty())
            {
            nodeExtent = node->GetNodeExtent();
            ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
            ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                        
            }
        else
            {            
            nodeExtent = node->GetContentExtent();                     
            }
                             
        if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox) == FALSE)
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        finalNode = !IsCorrectForCurrentView(node, visibleExtent, m_nearestPredefinedCameraOri, m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)                               
        if (finalNode == false)
            {            
            if (node->GetFilter()->IsProgressiveFilter())
                {
                 
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                    
                    if ((node->GetFilter()->IsProgressiveFilter() && (node->GetLevel() == 0)) ||                        
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, (node->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }                
                }               
            }
        else
            {                                       
            if ((node->GetParentNode() != 0) && 
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                
                for (size_t currentIndex = 0 ; currentIndex < parentNode->size(); currentIndex++)
                    {                   
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, parentNode->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(parentNode->operator[](currentIndex));
                        }
                    }    
                }
            }        
     
        if (finalNode && m_gatherTileBreaklines && node->size() > 0)
            {            
            AddBreaklinesForExtent(node->GetNodeExtent());
            }
        
            queryResult = !finalNode;
        }
    else
        {
        queryResult = true;
        }        

    return queryResult; 
    }
template<class POINT, class EXTENT> bool MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::Query(HFCPtr<HGFPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    HPMMemoryManagedVector<POINT>&                             resultPoints)
    {            
    bool queryResult;

    if (m_pQueryByLevel == 0)
        {   
        // Check if extent overlap         
        EXTENT visibleExtent;
        EXTENT nodeExtent;

        if (node->IsEmpty())
            {
            nodeExtent = node->GetNodeExtent();
            ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
            ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                        
            }
        else
            {            
            nodeExtent = node->GetContentExtent();                     
            }
                             
        if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox) == FALSE)
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        finalNode = !IsCorrectForCurrentView(node, visibleExtent, m_nearestPredefinedCameraOri, m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)                               
        if (finalNode == false)
            {            
            if (node->GetFilter()->IsProgressiveFilter())
                {
                 
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                    
                    if ((node->GetFilter()->IsProgressiveFilter() && (node->GetLevel() == 0)) ||                        
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, (node->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        if (resultPoints.size() == resultPoints.capacity())
                            resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }                
                }               
            }
        else
            {                                       
            if ((node->GetParentNode() != 0) && 
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                
                for (size_t currentIndex = 0 ; currentIndex < parentNode->size(); currentIndex++)
                    {                   
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, parentNode->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        if (resultPoints.size() == resultPoints.capacity())
                            resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                        resultPoints.push_back(parentNode->operator[](currentIndex));
                        }
                    }    
                }
            }        
     
        if (finalNode && m_gatherTileBreaklines && node->size() > 0)
            {            
            AddBreaklinesForExtent(node->GetNodeExtent());
            }
        
            queryResult = !finalNode;
        }
    else
        {
        queryResult = true;
        }        

    return queryResult; 
    }

/**----------------------------------------------------------------------------
 Indicates if the provided node is adequate for obtaining result.
 The visible extent is of course provided in the STM GCS and units.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::IsCorrectForCurrentView(HFCPtr<HGFPointIndexNode<POINT, EXTENT>> node,
                                                                                                                      const EXTENT&                            pi_visibleExtent,
                                                                                                                      int                                      pi_NearestPredefinedCameraOri,
                                                                                                                      double                                   pi_RootToViewMatrix[][4]) const
    {    
    bool IsCorrect = false;
    if (!node->IsLoaded())
        node->Load();
 
    //Return always true for the root node so that something is displayed at the screen.
    if (node->GetParentNode() == 0) 
        {
        IsCorrect = true;
        }
    else
        {        
        double                  rootToViewScale;
        int                     nbPoints = 4;
        HArrayAutoPtr<DPoint3d> tileBorderPts(new DPoint3d[nbPoints]);        
        double                  nodeExtentArea = ExtentOp<EXTENT>::GetWidth(node->GetNodeExtent()) * ExtentOp<EXTENT>::GetHeight(node->GetNodeExtent());
        double                  visibleNodeExtentArea = ExtentOp<EXTENT>::GetWidth(pi_visibleExtent) * ExtentOp<EXTENT>::GetHeight(pi_visibleExtent);
        double                  visibleExtentToNodeExtentScale = visibleNodeExtentArea / nodeExtentArea;

        // Usually the following value should be between 0 and 1 but for reason of floating-point mathematical imprecision
        // sometimes the value is slightly greater than 1 by a minuscule amount. A 1 percent tolerance is quite acceptable
        // for our purpose and will not result in any ill effect.
        assert((visibleExtentToNodeExtentScale > 0) && (visibleExtentToNodeExtentScale <= 1.01));
                        

        // We convert the visible extent into targetGCS units (or meters if no GCS)
        // because the localToView matrix was also converted this way.
        tileBorderPts[0].x = ExtentOp<EXTENT>::GetXMin(pi_visibleExtent);
        tileBorderPts[0].y = ExtentOp<EXTENT>::GetYMin(pi_visibleExtent);
        tileBorderPts[0].z = 0;

        tileBorderPts[1].x = ExtentOp<EXTENT>::GetXMin(pi_visibleExtent);
        tileBorderPts[1].y = ExtentOp<EXTENT>::GetYMax(pi_visibleExtent);
        tileBorderPts[1].z = 0;

        tileBorderPts[2].x = ExtentOp<EXTENT>::GetXMax(pi_visibleExtent);
        tileBorderPts[2].y = ExtentOp<EXTENT>::GetYMax(pi_visibleExtent);
        tileBorderPts[2].z = 0;

        tileBorderPts[3].x = ExtentOp<EXTENT>::GetXMax(pi_visibleExtent);
        tileBorderPts[3].y = ExtentOp<EXTENT>::GetYMin(pi_visibleExtent);
        tileBorderPts[3].z = 0;

        if ((m_sourceGCSPtr != 0) && (m_targetGCSPtr != 0))
            {       
            GeoPoint  sourceLatLong;
            GeoPoint  targetLatLong;        
            StatusInt stat1;
            StatusInt stat2;
            StatusInt stat3;
              
            for (int boxPtInd = 0; boxPtInd < 4; boxPtInd++)
                {                                             
                stat1 = m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, tileBorderPts[boxPtInd]);
                assert(stat1 == 0);
                stat2 = m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *m_targetGCSPtr);
                assert(stat2 == 0);
                stat3 = m_targetGCSPtr->CartesianFromLatLong(tileBorderPts[boxPtInd], targetLatLong);                        
                assert(stat3 == 0);
                }    
            }
#if 0 //Reprojection Plane
        double shapeWithVanishingLine[8];
#endif
        double vanishingLineCutCorrectionFactor = 1.0;
        
        if ((m_rootToViewMatrix[3][0] != 0.0) ||
            (m_rootToViewMatrix[3][1] != 0.0)) 
//            (m_rootToViewMatrix[3][2] != 0.0) ||            
            {                           
            
            vector<DPoint3d> tileborderPoints;
            vector<DPoint3d> shapeInFrontOfProjectivePlane;

            for (int ptInd = 0; ptInd < nbPoints; ptInd++)
                {
                DPoint3d pt3D = {tileBorderPts[ptInd].x, tileBorderPts[ptInd].y, tileBorderPts[ptInd].z};
                tileborderPoints.push_back(pt3D);                                        
                }                        

            int status = GetShapeInFrontOfProjectivePlane(shapeInFrontOfProjectivePlane, 
                                                          vanishingLineCutCorrectionFactor,
                                                          tileborderPoints, 
                                                          m_rootToViewMatrix);

            assert(status == SUCCESS);

            //The tile was cut by the projective plane.
            if (vanishingLineCutCorrectionFactor != 1.0)
                {
                nbPoints = (int)shapeInFrontOfProjectivePlane.size();
                tileBorderPts = new DPoint3d[nbPoints];

                vector<DPoint3d>::const_iterator pointIter(shapeInFrontOfProjectivePlane.begin());                
                
                for (int ptInd = 0; ptInd < nbPoints; ptInd++)
                    {
                    tileBorderPts[ptInd] = *pointIter;
                    pointIter++;
                    }               

                assert(pointIter == shapeInFrontOfProjectivePlane.end());
                }
                                  
            }
                                              
//Old code
#if 0        
        if ((m_sourceGCSPtr != 0) && (m_targetGCSPtr != 0))
            {                
            DRange2d range;
            DPoint2d extent[4];
            DPoint2d reprojectedExtent[4];

            range.low.x = tileLimit.minX;
            range.low.y = tileLimit.minY;            
            range.high.x = tileLimit.maxX;
            range.high.y = tileLimit.maxY;

            bsiDRange2d_box2Points(&range, extent);
            
            GeoPoint  sourceLatLong;
            GeoPoint  targetLatLong;
            DPoint3d  sourcePoint;
            DPoint3d  targetPoint;
            StatusInt stat1;
            StatusInt stat2;
            StatusInt stat3;
              
            for (int boxPtInd = 0; boxPtInd < 4; boxPtInd++)
                {                                   
                sourcePoint.x = extent[boxPtInd].x;
                sourcePoint.y = extent[boxPtInd].y;
                sourcePoint.z = 0;

                stat1 = m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, sourcePoint);
                stat2 = m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *m_targetGCSPtr);
                stat3 = m_targetGCSPtr->CartesianFromLatLong(targetPoint, targetLatLong);            

                reprojectedExtent[boxPtInd].x = targetPoint.x;
                reprojectedExtent[boxPtInd].y = targetPoint.y;                
                }                          

            bsiDRange2d_initFromArray(&range, reprojectedExtent, 4);

            tileLimit.minX = range.low.x;
            tileLimit.minY = range.low.y;
            tileLimit.maxX = range.high.x;
            tileLimit.maxY = range.high.y;
            }        
#endif              
 
    bcdtmMultiResolution_getTileAreaInCurrentView(pi_RootToViewMatrix, tileBorderPts, nbPoints, &rootToViewScale);    
            
    #ifdef ACTIVATE_NODE_QUERY_TRACING

        if (m_pTracingXMLFile != 0)
        {
            char   TempBuffer[3000]; 
            int    NbChars;
            size_t NbWrittenChars;    
            
            //Extent
            NbChars = sprintf(TempBuffer, 
                              "<CheckedNode><NbOfPoints>%i</NbOfPoints><Level>%i</Level><DTMCoverAreaFactor>%.8f</DTMCoverAreaFactor>", 
                              node->GetNbObjects(), 
                              node->GetLevel(), 
                              node->GetViewDependentMetrics()[0]);      

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, m_pTracingXMLFile);
                
            assert(NbWrittenChars == NbChars);

            //Extent
            NbChars = sprintf(TempBuffer, 
                              "<Extent><MinX>%.3f</MinX><MaxX>%.3f</MaxX><MinY>%.3f</MinY><MaxY>%.3f</MaxY><DeltaX>%.3f</DeltaX><DeltaY>%.3f</DeltaY></Extent>",                       
                              ExtentOp<EXTENT>::GetXMin(node->GetNodeExtent()), 
                              ExtentOp<EXTENT>::GetXMax(node->GetNodeExtent()), 
                              ExtentOp<EXTENT>::GetYMin(node->GetNodeExtent()), 
                              ExtentOp<EXTENT>::GetYMax(node->GetNodeExtent()), 
                              ExtentOp<EXTENT>::GetWidth(node->GetNodeExtent()), 
                              ExtentOp<EXTENT>::GetHeight(node->GetNodeExtent()));        

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, m_pTracingXMLFile);
                
            assert(NbWrittenChars == NbChars);

            double NbScreenPixelsPerPoints = 0;
         
            NbScreenPixelsPerPoints = rootToViewScale / (node->GetSplitTreshold() * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor);

            NbChars = sprintf(TempBuffer,                                                     
                              "<RootToViewScale>%.3f</RootToViewScale><ScreenPixelPerPoints>%.3f</ScreenPixelPerPoints></CheckedNode>",
                              rootToViewScale, 
                              NbScreenPixelsPerPoints);        

            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, m_pTracingXMLFile);
                
            assert(NbWrittenChars == NbChars);                    
        }
    #endif

        
        if ((node->GetNbObjects() > 0) || (m_useSplitThresholdForTileSelection == true))
            {        
            size_t nbOfPointsInTile; 

            if (m_useSplitThresholdForTileSelection == true)
                {
                nbOfPointsInTile = node->GetSplitTreshold();	
                }
            else
                {
                nbOfPointsInTile = node->GetNbObjects();                
                }
            
            //Note that the DTM area to tile area ratio is not valid in the case of a projective.
            //IsCorrect = HDOUBLE_GREATER_OR_EQUAL(rootToViewScale * node->GetViewDependentMetrics()[0] / node->GetNbObjects(), m_meanAreaFactor, m_meanAreaFactorTolerance);
            //IsCorrect = rootToViewScale * node->GetViewDependentMetrics()[0] / node->GetNbObjects() > m_meanScreenPixelsPerPoint;
            IsCorrect = rootToViewScale / (nbOfPointsInTile * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor) > m_meanScreenPixelsPerPoint;                           
            }
        else
            {
            IsCorrect = true;
            }
        }             

    return IsCorrect;

    //return (node->GetViewDependentMetrics()[pi_NearestPredefinedCameraOri] * rootToViewScale) > m_meanAreaFactor;
}


/**----------------------------------------------------------------------------
 Tracing related functions are only compiled in debug.
-----------------------------------------------------------------------------*/

template<class POINT, class EXTENT> bool MrDTMQuadTreeLevelPointIndexQuery<POINT, EXTENT>::Query(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                                                                                 HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                 size_t numSubNodes,
                                                                                                 list<POINT>& resultPoints)
    {    
    //MST : This function does not currently return all the points at level 0 if the progressive filtering is false, 
    //      which could lead to missing data at the border. 
    assert(node->GetFilter()->IsProgressiveFilter() == true);

    // Before we make sure requested level is appropriate
    if (m_requestedLevel < 0)
        m_requestedLevel = 0;
           
    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
        {
        nodeExtent = node->GetNodeExtent();
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                        
        }
    else
        {            
        nodeExtent = node->GetContentExtent();                         
        }
        
    bool isVisible = GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox);
                                                                      
    if ((isVisible == true) && (node->GetLevel() <= m_requestedLevel))
        {            
        // If this is the appropriate level or it is a higher level and progressive is set.
        if (node->IsLeaf() || 
            m_requestedLevel == node->GetLevel() || 
            (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel()))
            {
            // Copy content
			
			POINT nodeExtentOrigin = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMin(nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeExtent));
    		POINT nodeExtentCorner = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMax(nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeExtent));

            // If the whole node is located within extent ... copy it all
            if ((ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentOrigin) && 
                 ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentCorner)) ||                 
                ((node->GetLevel() == 0) && //Always return all the points in the lowest level. 
                 (node->GetFilter()->IsProgressiveFilter() == true)))  
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    resultPoints.push_back(node->operator[](currentIndex));
                    }
                }
            else
                {
                // Search in present list of objects for current node
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // Check if point is in extent of object                            
                    //if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, node->operator[](currentIndex)))
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, node->operator[](currentIndex)))      
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                }  
            }
        return true;
        }
    else
        {
        // No need to dig deeper in subnodes ... either extents do not overlap or 
        // level reached
        return false;
        }
    }

template<class POINT, class EXTENT> bool MrDTMQuadTreeLevelPointIndexQuery<POINT, EXTENT>::Query(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > node, 
                                                                                                 HFCPtr<HGFPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                 size_t numSubNodes,
                                                                                                 HPMMemoryManagedVector<POINT>& resultPoints)
    {      
    //MST : This function does not currently return all the points at level 0 if the progressive filtering is false, 
    //      which could lead to missing data at the border. 
    assert(node->GetFilter()->IsProgressiveFilter() == true);

    // Before we make sure requested level is appropriate
    if (m_requestedLevel < 0)
        m_requestedLevel = 0;
           
    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
        {
        nodeExtent = node->GetNodeExtent();
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                        
        }
    else
        {            
        nodeExtent = node->GetContentExtent();                         
        }
    

    bool isVisible = GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox);
                                                                      
    if ((isVisible == true) && (node->GetLevel() <= m_requestedLevel))
        {            
        // If this is the appropriate level or it is a higher level and progressive is set.
        if (node->IsLeaf() || 
            m_requestedLevel == node->GetLevel() || 
            (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel()))
            {
            // Copy content
			
			POINT nodeExtentOrigin = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMin(nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeExtent));
    		POINT nodeExtentCorner = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMax(nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeExtent));
            
            // If the whole node is located within extent ... copy it all
            if ((ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentOrigin) && 
                 ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, nodeExtentCorner)) || 
                ((node->GetLevel() == 0) && //Always return all the points in the lowest level. 
                 (node->GetFilter()->IsProgressiveFilter() == true)))  
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    if (resultPoints.size() == resultPoints.capacity())
                        resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                    resultPoints.push_back(node->operator[](currentIndex));
                    }
                }
            else
                {
                // Search in present list of objects for current node
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {
                    if ((g_checkIndexingStopCallbackFP != 0) && (currentIndex % 100 == 0) && (g_checkIndexingStopCallbackFP(0) != 0))
                        {
                        return false;
                        }

                    // Check if point is in extent of object                            
                    //if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_extent, node->operator[](currentIndex)))
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, node->operator[](currentIndex)))      
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        if (resultPoints.size() == resultPoints.capacity())
                            resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }
                }  
            }
        return true;
        }
    else
        {
        // No need to dig deeper in subnodes ... either extents do not overlap or 
        // level reached
        return false;
        }
    }


#ifdef ACTIVATE_NODE_QUERY_TRACING

/**----------------------------------------------------------------------------
 Set the tracing XML file name which outputs a trace of the nodes checked 
 during the last query. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::SetTracingXMLFileName(AString& pi_rTracingXMLFileName)
{
    m_pTracingXMLFileName = pi_rTracingXMLFileName;
}

/**----------------------------------------------------------------------------
 Set the tracing XML file name which outputs a trace of the nodes checked 
 during the last query. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::OpenTracingXMLFile()
{    
    if ((m_pTracingXMLFileName.empty() == false) && (m_pTracingXMLFile == 0))
    {
        char   TempBuffer[300]; 
        int    NbChars;
        size_t NbWrittenChars;    

        m_pTracingXMLFile = fopen(m_pTracingXMLFileName.c_str(), "w+");

        assert(m_pTracingXMLFile != 0);

        //Header and starting root node        
        NbChars = sprintf(TempBuffer, 
                          "<?xml version=\"1.0\" encoding=\"utf-8\"?><TracedNodes>");        

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, m_pTracingXMLFile);
            
        assert(NbWrittenChars == NbChars);
    }
}

template<class POINT, class EXTENT> void MrDTMQuadTreeViewDependentPointQuery<POINT, EXTENT>::CloseTracingXMLFile()
{
    if (m_pTracingXMLFile != 0)
    {
        char   TempBuffer[50]; 
        int    NbChars;
        size_t NbWrittenChars;    

        //Closing root node
        NbChars = sprintf(TempBuffer, 
                          "</TracedNodes>");        

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, m_pTracingXMLFile);
            
        assert(NbWrittenChars == NbChars);

        fclose(m_pTracingXMLFile);        

        m_pTracingXMLFile = 0;
    }
}
       
#endif