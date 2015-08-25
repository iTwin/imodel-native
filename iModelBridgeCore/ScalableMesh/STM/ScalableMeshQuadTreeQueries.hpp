//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeQueries.hpp $
//:>    $RCSfile: ScalableMeshQuadTreeQueries.hpp,v $
//:>   $Revision: 1.20 $
//:>       $Date: 2012/11/29 17:30:34 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 ScalableMeshQuadTreeViewDependentPointQuery
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                             list<POINT>&                  points) 
    {            
    bool queryResult;

    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    EXTENT indexExtent(index.GetIndexExtent());




    //Not a projective transformation
    ISMPointIndexFilter<POINT, EXTENT>* filter = index.GetFilter();

    ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>* pQuadTreeViewDependentFilter = 
                                                                        dynamic_cast<ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, 
                                                                                                                           EXTENT>*>(filter);

    if ((m_useSameResolutionWhenCameraIsOff == true || true) &&
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
            levelVal = log((double)rootToViewScale / (splitThreshold * m_meanScreenPixelsPerPoint)) / log(8.0);
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
        
        m_pQueryByLevel = new ScalableMeshQuadTreeLevelPointIndexQuery<POINT, EXTENT>(m_extent, 
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

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                             HPMMemoryManagedVector<POINT>&                  points) 
    {            
    bool queryResult;

    EXTENT indexExtent(index.GetIndexExtent());



    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    //Not a projective transformation
    ISMPointIndexFilter<POINT, EXTENT>* filter = index.GetFilter();

    ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>* pQuadTreeViewDependentFilter = 
                                                                        dynamic_cast<ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, 
                                                                                                                           EXTENT>*>(filter);
    //NEEDS_WORK_SM - m_useSameResolutionWhenCameraIsOff should react to the variable STM_USE_SAME_RESOLUTION_WHEN_CAMERA_IS_OFF, but doesn't seem to work
    if ((m_useSameResolutionWhenCameraIsOff == true || false) &&
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
            
            //NEEDS_WORK_SM : With an octree a lot of split node can have very few to none points, so divide the splitThreshold by an arbritrary 
            //number of points. 
            numberOfPointsAtRoot /= 4;
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
        
        m_pQueryByLevel = new ScalableMeshQuadTreeLevelPointIndexQuery<POINT, EXTENT>(m_extent, 
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
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
            //NEEDS_WORK_SM : This code doesn't work since that could lead to the same point being queried multiple time.
            if (/*(node->GetParentNode() != 0) && */
                (node->GetFilter()->IsProgressiveFilter() == false))
                {                                
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                   
                    // Check if point is in extent of object
                    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, node->operator[](currentIndex)))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        resultPoints.push_back(node->operator[](currentIndex));
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
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
        if (finalNode == false && !node->IsLeaf())
            {                   
            if (node->GetFilter()->IsProgressiveFilter())
                {                 
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                    
                    if ((node->GetLevel() == 0) ||                        
                        ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, (node->operator[](currentIndex))))
                        {
                        // The point falls inside extent of object .. we add a reference to the list
                        if (resultPoints.size() == resultPoints.capacity())
                            resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                        resultPoints.push_back(node->operator[](currentIndex));
                        }
                    }                
                }                          
            else
            if (node->GetLevel() == 0) 
                {
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                                                            
                    // The point falls inside extent of object .. we add a reference to the list
                    if (resultPoints.size() == resultPoints.capacity())
                        resultPoints.reserve(resultPoints.size() + (resultPoints.size()/10) + 1);
                    resultPoints.push_back(node->operator[](currentIndex));
                    }                                 
                }
            }
        else
            {                                       
            if (/*(node->GetParentNode() != 0) && */
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                //NEEDS_WORK_SM : Can lead to duplicate points being returned. 
                //HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                
                for (size_t currentIndex = 0 ; currentIndex < node->size(); currentIndex++)
                    {                   
                    // Check if point is in extent of object
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
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

        //NEEDS_WORK_SM : Stitching triangles usually cross the tile boundary.
        assert((visibleExtentToNodeExtentScale > 0) /*&& (visibleExtentToNodeExtentScale <= 1.01)*/);
                        

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

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPointIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                                                                                 HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
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

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPointIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                                                                                 HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                 size_t numSubNodes,
                                                                                                 HPMMemoryManagedVector<POINT>& resultPoints)
    {      
    //MST : This function does not currently return all the points at level 0 if the progressive filtering is false, 
    //      which could lead to missing data at the border. 
    //NEEDS_WORK_SM : The comment above is not important for now.
    //assert(node->GetFilter()->IsProgressiveFilter() == true);

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



//Level mesh query
/**----------------------------------------------------------------------------
 Tracing related functions are only compiled in debug.
-----------------------------------------------------------------------------*/

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                                                                                HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                size_t numSubNodes,
                                                                                                list<POINT>& resultPoints)
    {    
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                                                                                 HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                 size_t numSubNodes,
                                                                                                 HPMMemoryManagedVector<POINT>& resultPoints)
    {      
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                   HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                   size_t                                   numSubNodes,
                                                                                                   vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {
    assert(node->GetFilter()->IsProgressiveFilter() == false);

    // Before we make sure requested level is appropriate
    if (m_requestedLevel < 0)
        m_requestedLevel = 0;

    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
        {
        nodeExtent = node->GetNodeExtent();
        //NEEDS_WORK_SM : Don't think this is necessary with an octree
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
        if (/*node->IsLeaf()
            7744xx..
            
            || */ //NEEDS_WORK_SM : Why leaf, this is the given level that we wants?
            m_requestedLevel == node->GetLevel() /*||
                                                 (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel())*/)
            {         
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {
                meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
                }
            else
                {
                //NEEDS_WORK_SM : Seems to happen with ScalableMeshQuadTreeBCLIBMeshFilter1 because of duplicate points. 
                //In the end every node with at least one points should probably have a mesh in it.
                //assert(node->size() <= 4);
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

template<class POINT, class EXTENT> 
int AddVisibleMesh(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                   EXTENT                                    visibleExtent,
                   Bentley::ScalableMesh::ScalableMeshMesh*                mesh)
    {
    vector<DPoint3d> dataPoints;
    vector<int32_t>    faceIndexes;

    ToBcPtConverter converter; 

    struct PointVisibility
        {
        PointVisibility()
            {
            m_isVisible = false;
            m_mappedIndex = -1;
            }

        bool  m_isVisible; 
        int32_t m_mappedIndex;
        };

    vector<PointVisibility> pointVisibilities(node->size() + 1); 

    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
        {
        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, node->operator[](pointInd)))      
            {
            pointVisibilities[pointInd + 1].m_isVisible = true;                            
            dataPoints.push_back(converter.operator()(node->operator[](pointInd)));
            pointVisibilities[pointInd + 1].m_mappedIndex = (int32_t)dataPoints.size();
            }                                                                                           
        }

    Int32* nodeFaceIndexes = (Int32*)&node->operator[](node->size());

    for (size_t faceVerticeInd = 0; faceVerticeInd < node->m_nodeHeader.m_nbFaceIndexes; faceVerticeInd += 3)
        {                        
        if (pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_isVisible ||
            pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_isVisible ||
            pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_isVisible) 
            {
            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex == -1)
                {                                
                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd] - 1)));                                
                pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex = (int32_t)dataPoints.size();
                }

            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex);

            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex == -1)
                {                                
                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd + 1] - 1)));                                                                
                pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex = (int32_t)dataPoints.size();
                }

            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex);

            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex == -1)
                {                                
                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd + 2] - 1)));                                                                                                
                pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex = (int32_t)dataPoints.size();
                }

            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex);
            }                                                
        }

    assert(dataPoints.size() > 0 || faceIndexes.size() == 0);

    int status; 

    if (dataPoints.size() > 0)
        {            
        assert(faceIndexes.size() % 3 == 0);
        status = mesh->AppendMesh(dataPoints.size(), &dataPoints[0], faceIndexes.size(), &faceIndexes[0], 0, 0, 0);
        }
    else
        {
        status = SUCCESS;
        }    

    return status;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node, 
                                                                                                HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                size_t numSubNodes,
                                                                                                Bentley::ScalableMesh::ScalableMeshMesh* mesh)
    {         
    assert(node->GetFilter()->IsProgressiveFilter() == false);

    // Before we make sure requested level is appropriate
    if (m_requestedLevel < 0)
        m_requestedLevel = 0;
           
    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
        {
        nodeExtent = node->GetNodeExtent();
        //NEEDS_WORK_SM : Don't think this is necessary with an octree
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                                
        }
    else
        {            
        nodeExtent = node->GetContentExtent();                         
        }
    

    bool isVisible = GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox);

    //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
    static bool s_clipMesh = false;

    if ((isVisible == true) && (node->GetLevel() <= m_requestedLevel))
        {            
        // If this is the appropriate level or it is a higher level and progressive is set.
        if (/*node->IsLeaf() || */ //NEEDS_WORK_SM : Why leaf, this is the given level that we wants?
            m_requestedLevel == node->GetLevel() /*|| 
            (node->GetFilter()->IsProgressiveFilter() && m_requestedLevel > node->GetLevel())*/)
            {
            // Copy content            
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {                               
                int status;

                if (s_clipMesh == true)
                    {   
                    status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);
                    /*
                    vector<DPoint3d> dataPoints;
                    vector<Int32>    faceIndexes;

                    ToBcPtConverter converter; 

                    struct PointVisibility
                        {
                        PointVisibility()
                            {
                            m_isVisible = false;
                            m_mappedIndex = -1;
                            }

                        bool  m_isVisible; 
                        Int32 m_mappedIndex;
                        };

                    vector<PointVisibility> pointVisibilities(node->size() + 1); 

                    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                        {
                        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(visibleExtent, node->operator[](pointInd)))      
                            {
                            pointVisibilities[pointInd].m_isVisible = true;                            
                            dataPoints.push_back(converter.operator()(node->operator[](pointInd)));
                            pointVisibilities[pointInd].m_mappedIndex = (Int32)dataPoints.size();                            
                            }                                                                                           
                        }

                    Int32* nodeFaceIndexes = (Int32*)&node->operator[](node->size());

                    for (size_t faceVerticeInd = 0; faceVerticeInd < node->m_nodeHeader.m_nbFaceIndexes; faceVerticeInd += 3)
                        {                        
                        if (pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_isVisible ||
                            pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_isVisible ||
                            pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_isVisible) 
                            {
                            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex == -1)
                                {                                
                                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd] - 1)));                                
                                pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex = (Int32)dataPoints.size();
                                }

                            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd]].m_mappedIndex);

                            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex == -1)
                                {                                
                                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd + 1] - 1)));                                                                
                                pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex = (Int32)dataPoints.size();
                                }

                            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd + 1]].m_mappedIndex);

                            if (pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex == -1)
                                {                                
                                dataPoints.push_back(converter.operator()(node->operator[](nodeFaceIndexes[faceVerticeInd + 2] - 1)));                                                                                                
                                pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex = (Int32)dataPoints.size();
                                }

                            faceIndexes.push_back(pointVisibilities[nodeFaceIndexes[faceVerticeInd + 2]].m_mappedIndex);
                            }                                                
                        }

                    assert(dataPoints.size() > 0 || faceIndexes.size() == 0);

                    if (dataPoints.size() > 0)
                        {                    
                        status = mesh->AppendMesh(dataPoints.size(), &dataPoints[0], faceIndexes.size(), &faceIndexes[0], 0, 0, 0);
                        }
                    else
                        {
                        status = SUCCESS;
                        }
                        */
                    }
                else
                    {
                    //size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(Int32)) / (double)sizeof(POINT));                  
                    vector<DPoint3d> dataPoints(node->size());

                    ToBcPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                        }

                    status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (Int32*)&node->operator[](node->size()), 0, 0, 0);
                    }

                assert(status == SUCCESS);
                }
            else
                {
                //NEEDS_WORK_SM : Seems to happen with ScalableMeshQuadTreeBCLIBMeshFilter1 because of duplicate points. 
                //In the end every node with at least one points should probably have a mesh in it.
                //assert(node->size() <= 4);
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
template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::SetTracingXMLFileName(AString& pi_rTracingXMLFileName)
{
    m_pTracingXMLFileName = pi_rTracingXMLFileName;
}

/**----------------------------------------------------------------------------
 Set the tracing XML file name which outputs a trace of the nodes checked 
 during the last query. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::OpenTracingXMLFile()
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

template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::CloseTracingXMLFile()
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

////////////Scalable Mesh


/**----------------------------------------------------------------------------
 ScalableMeshQuadTreeViewDependentMeshQuery
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                             list<POINT>&                  points) 
    {            
    assert(!"Must not be called");
   
    return false;
    }


template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                             HPMMemoryManagedVector<POINT>&                  points) 
    {            
    assert(!"Must not be called");
   
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                            Bentley::ScalableMesh::ScalableMeshMesh* mesh) 
    {                

    static bool s_useOldReprojectedTile = false;

    bool queryResult;
    

    //NEEDS_WORK_SM - Not implemented yet.
    assert(m_useSameResolutionWhenCameraIsOff == false);

    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    //NEEDS_WORK_SM : Probably better with the content extent
    //EXTENT indexExtent(index.GetIndexExtent());
    EXTENT indexExtent(index.GetContentExtent());

    //The root to view matrix is normalized
    //assert(m_rootToViewMatrix[3][3] == 1.0);

    //Not a projective transformation
    /*
    ISMPointIndexFilter<POINT, EXTENT>* filter = index.GetFilter();
    
    ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>* pQuadTreeViewDependentFilter = 
                                                                        dynamic_cast<ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, 
                                                                                                                           EXTENT>*>(filter);
    */

    //NEEDS_WORK_SM : Parameters enforce here for now.
    if ((m_useSameResolutionWhenCameraIsOff == true || true) &&
        /*(pQuadTreeViewDependentFilter != 0) &&*/ //NEEDS_WORK_SM Not sure about the filter
        (m_rootToViewMatrix[3][0] == 0.0) &&
        (m_rootToViewMatrix[3][1] == 0.0) && 
        (m_rootToViewMatrix[3][2] == 0.0) && 
        (m_rootToViewMatrix[3][3] == 1.0) && 
        (m_sourceGCSPtr == 0) &&
        (m_targetGCSPtr == 0))
        {              
        HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, mesh);
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

        double rootToViewScale2_5D;

        bcdtmMultiResolution_getTileAreaInCurrentView(m_rootToViewMatrix, 
                                                      tileBorder, 
                                                      4,
                                                      &rootToViewScale2_5D);  

        
        //NEEDS_WORK_SM : Why do we handle reprojection in IsCorrectForCurrentView and not here?
        DPoint3d facePts[8];

        facePts[0].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        facePts[0].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        facePts[0].z = ExtentOp<EXTENT>::GetZMin(indexExtent);

        facePts[1].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        facePts[1].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        facePts[1].z = ExtentOp<EXTENT>::GetZMin(indexExtent);

        facePts[2].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        facePts[2].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        facePts[2].z = ExtentOp<EXTENT>::GetZMin(indexExtent);

        facePts[3].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        facePts[3].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        facePts[3].z = ExtentOp<EXTENT>::GetZMin(indexExtent);

        facePts[4].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        facePts[4].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        facePts[4].z = ExtentOp<EXTENT>::GetZMax(indexExtent);

        facePts[5].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        facePts[5].y = ExtentOp<EXTENT>::GetYMin(indexExtent);
        facePts[5].z = ExtentOp<EXTENT>::GetZMax(indexExtent);

        facePts[6].x = ExtentOp<EXTENT>::GetXMin(indexExtent);
        facePts[6].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        facePts[6].z = ExtentOp<EXTENT>::GetZMax(indexExtent);

        facePts[7].x = ExtentOp<EXTENT>::GetXMax(indexExtent);
        facePts[7].y = ExtentOp<EXTENT>::GetYMax(indexExtent);
        facePts[7].z = ExtentOp<EXTENT>::GetZMax(indexExtent);

        bcdtmMultiResolution_getMaxFaceAreaInCurrentView(m_rootToViewMatrix, 
                                                         facePts, 
                                                         8, 
                                                         &rootToViewScale);

        if (s_useOldReprojectedTile)
            {
            rootToViewScale = rootToViewScale2_5D;
            }

              
        unsigned __int64 numberOfPointsAtRoot;

        //NEEDS_WORK_SM : Use the number of point in the root node for now
        if (m_useSplitThresholdForLevelSelection == true && false)
            {
            //The splithreshold is a good heuristic but could be replace by something more precise 
            //like the maximum number of points in a leaf (e.g. : a threshold of 5000 could 
            //lead to node having on average between 1250 and 5000 points)        
            numberOfPointsAtRoot = index.GetSplitTreshold();

            //NEEDS_WORK_SM : With an octree a lot of split node can have very few to none points, so divide the splitThreshold by an arbritrary 
            //number of points. 
            numberOfPointsAtRoot /= 4;
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
            levelVal = log((double)rootToViewScale / (numberOfPointsAtRoot * m_meanScreenPixelsPerPoint)) / log(8.0);            
        }
        else
        {
            levelVal = 0;
        }

        //NEEDS_WORK_SM : The view dependent mesh query use the first bad level, so increase the level here for now.
        levelVal += 1;

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


        static int s_fixLevel = -1;

        if (s_fixLevel > -1)
            {   
            level = (size_t)s_fixLevel;
            }

        if (level > index.GetDepth())
            {
            level = index.GetDepth();
            }
        
        m_pQueryByLevel = new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>(m_extent, 
                                                                              level,                                                                                
                                                                              m_viewBox);  
        

        //MST - Query the points here for now
        index.Query(m_pQueryByLevel.get(), mesh);                        

        //MST - Stop the index to query each node since all the points are obtained 
        //      during the global query. Returning false is probably not the best 
        //      mechanism though...
        queryResult = false;
        }
    else
        {
        m_pQueryByLevel = 0;
        queryResult = HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, mesh); 
        }            

    return queryResult;
    }


template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                            vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes) 
    {    
    m_pQueryByLevel = 0;
    return HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, meshNodes);    
    }

// Specific Query implementation
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    list<POINT>&                             resultPoints)
    {           
    assert(0);

    return false; 
    }
        
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    HPMMemoryManagedVector<POINT>&                             resultPoints)
    {
    assert(!"Must not be called");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    Bentley::ScalableMesh::ScalableMeshMesh*               mesh)
    {                        
    bool queryResult;

    assert(mesh != 0);
    assert(m_pQueryByLevel == 0);

    if (mesh->GetNbPoints() > m_maxNumberOfPoints)
        {
        return false;    
        }
    else
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
        //NEEDS_WORK_SM : The incorrect one is use, probably need to change the UI accordingly or use the parent data with limited region.
        finalNode = !IsCorrectForCurrentView(node, visibleExtent, m_nearestPredefinedCameraOri, m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)              
        if (finalNode == true || node->IsLeaf())                             
            {                                       
            assert(node->GetFilter()->IsProgressiveFilter() == false);
                        
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {         
                //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
                static bool s_clipMesh = true;
                int status;

                if (s_clipMesh == true)
                    {   
                    status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);
                    }
                else
                    {
                    //size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(Int32)) / (double)sizeof(POINT));                  
                    vector<DPoint3d> dataPoints(node->size());

                    ToBcPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                        }
                                
                    status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&node->operator[](node->size()), 0, 0, 0);
                    }

                assert(status == SUCCESS);
                }
            else
                {
                //NEEDS_WORK_SM : Seems to happen with ScalableMeshQuadTreeBCLIBMeshFilter1 because of duplicate points. 
                //In the end every node with at least one points should probably have a mesh in it.
                //assert(node->size() <= 4);
                }

#if 0 
            if (/*(node->GetParentNode() != 0) && */
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                
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
#endif
            }   
        else //Root node with no child
        if (node->GetParentNode() == 0 && node->GetSubNodeNoSplit() == 0 && node->m_apSubNodes[0] == 0)
            {
            vector<DPoint3d> dataPoints(node->size());

            ToBcPtConverter converter; 

            for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                {
                dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                }
                            
            int status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (Int32*)&node->operator[](node->size()), 0, 0, 0);

            assert(status == SUCCESS);                        
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

//NEEDS_WORK_SM Cleanup
static bool s_useNew3dLODQuery = true;
static bool s_useXrowForCamOn = true;
static bool s_useClipVectorForVisibility = true;
static bool s_progressiveDisplay = false;

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {                        
    bool queryResult;

    assert(m_pQueryByLevel == 0);
    
    auto nodeIter(meshNodes.begin()); 
    auto nodeIterEnd(meshNodes.end()); 

    size_t nbPoints = 0;

    while (nodeIter != nodeIterEnd)
        {
        nbPoints += (*nodeIter).m_indexNode->size();
        nodeIter++;
        }
    
    if (nbPoints > m_maxNumberOfPoints)
        {
        return false;    
        }
    else
    if (m_pQueryByLevel == 0)
        {           
        // Check if extent overlap         
        EXTENT visibleExtent;
        EXTENT nodeExtent;

        if (node->IsEmpty())
            {
            /*
            nodeExtent = node->GetNodeExtent();
            ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(m_extent));
            ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(m_extent));                        
            */
            return false;
            }
        
        nodeExtent = node->GetContentExtent();                     
                     
        if (s_useClipVectorForVisibility)
            {            
            assert(m_viewClipVector != 0);

            //NEEDS_WORK_SM : Tolerance (i.e. radius) and center could be precomputed during SM generation
            double maxDimension = max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
            double tolerance = maxDimension / cos(PI / 4);

            DPoint3d center;

            center.Init(ExtentOp<EXTENT>::GetXMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) / 2,
                        ExtentOp<EXTENT>::GetYMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) / 2,
                        ExtentOp<EXTENT>::GetZMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetThickness(node->GetContentExtent()) / 2); 

            if (!m_viewClipVector->PointInside(center, tolerance))                
                return false;      

            visibleExtent = node->GetContentExtent();
            }
        else
            {
            if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, m_viewBox) == FALSE)
                return false;
            }

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        //NEEDS_WORK_SM : The incorrect one is use, probably need to change the UI accordingly or use the parent data with limited region.
        if (s_useNew3dLODQuery /*&& (m_rootToViewMatrix[3][0] != 0 || m_rootToViewMatrix[3][1] != 0 || m_rootToViewMatrix[3][2] != 0)*/)
            {
            finalNode = !IsCorrectForCurrentViewSphere(node, visibleExtent, m_nearestPredefinedCameraOri, m_rootToViewMatrix);
            }
        else
            {
            //NEEDS_WORK_SM : Not needed?
            finalNode = !IsCorrectForCurrentView(node, visibleExtent, m_nearestPredefinedCameraOri, m_rootToViewMatrix);
            }

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)              
        if (finalNode == true || node->IsLeaf())                             
            {                                       
            assert(node->GetFilter()->IsProgressiveFilter() == false);
                        
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {         
                //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
                static bool s_clipMesh = true;

                if (s_progressiveDisplay)
                    {
                    if (!node->Discarded())
                        {
                        meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));                    
                        }
                    else
                        {
                        meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));                    
                        }
                    }
                else
                    {
                    meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
                    }
                
                /*
                if (s_clipMesh == true)
                    { 

                    status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);
                    }
                else
                    {
                    //size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(Int32)) / (double)sizeof(POINT));                  
                    vector<DPoint3d> dataPoints(node->size());

                    ToBcPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                        }
                                
                    status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (Int32*)&node->operator[](node->size()), 0, 0, 0);                    
                    }
                    */               
                }
            else
                {
                //NEEDS_WORK_SM : Seems to happen with ScalableMeshQuadTreeBCLIBMeshFilter1 because of duplicate points. 
                //In the end every node with at least one points should probably have a mesh in it.
                //assert(node->size() <= 4);
                }

#if 0 
            if (/*(node->GetParentNode() != 0) && */
                (node->GetFilter()->IsProgressiveFilter() == false))
                {
                HFCPtr<SMPointIndexNode<POINT, EXTENT>> parentNode = node->GetParentNode();
                
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
#endif
            }   
        else //Root node with no child
        if (node->GetParentNode() == 0 && node->GetSubNodeNoSplit() == 0 && node->m_apSubNodes[0] == 0)
            {
            vector<DPoint3d> dataPoints(node->size());

            ToBcPtConverter converter; 

            for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                {
                dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                }
                         
            meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));            
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


template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GetQueryNodeOrder(vector<size_t>&                            queryNodeOrder, 
                                                                                                               HFCPtr<SMPointIndexNode<POINT, EXTENT> >  node, //NEEDS_WORK_SM - Needed?
                                                                                                               HFCPtr<SMPointIndexNode<POINT, EXTENT> >  subNodes[],
                                                                                                               size_t                                     numSubNodes) 
    {    
    assert(queryNodeOrder.size() == 0);

    DMatrix4d rootToViewMatrix; 

    bsiDMatrix4d_initAffineRows(&rootToViewMatrix, 
                                (DPoint3d*)&m_rootToViewMatrix[0], 
                                (DPoint3d*)&m_rootToViewMatrix[1], 
                                (DPoint3d*)&m_rootToViewMatrix[2],
                                (DPoint3d*)&m_rootToViewMatrix[3]);

    struct OrderInfo
        {
        double m_zScreen;
        size_t m_nodeInd; 
        };

    vector<OrderInfo> orderInfos;
            
    for (size_t nodeInd = 0; nodeInd < numSubNodes; nodeInd++)
        {
        DPoint4d center;
        center.x = (ExtentOp<EXTENT>::GetXMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetXMin(subNodes[nodeInd]->GetNodeExtent())) / 2;
        center.y = (ExtentOp<EXTENT>::GetYMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetYMin(subNodes[nodeInd]->GetNodeExtent())) / 2;
        center.z = (ExtentOp<EXTENT>::GetZMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetZMin(subNodes[nodeInd]->GetNodeExtent())) / 2;
        center.w = 1;
                
        DPoint4d outPoint;

        bsiDMatrix4d_multiply4dPoints(&rootToViewMatrix, &outPoint, &center, 1);

        OrderInfo orderInfo;
        orderInfo.m_zScreen = outPoint.z;
        orderInfo.m_nodeInd = nodeInd;

        orderInfos.push_back(orderInfo);                           
        }
            
    std::sort(orderInfos.begin(), orderInfos.end(), [](OrderInfo& i,OrderInfo& j){return i.m_zScreen > j.m_zScreen;});            

     for (size_t nodeInd = 0; nodeInd < numSubNodes; nodeInd++)
        {
        queryNodeOrder.push_back(orderInfos[nodeInd].m_nodeInd);
        }    
    }

/**----------------------------------------------------------------------------
 Indicates if the provided node is adequate for obtaining result.
 The visible extent is of course provided in the STM GCS and units.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                                      const EXTENT&                            pi_visibleExtent,
                                                                                                                      int                                      pi_NearestPredefinedCameraOri,
                                                                                                                      double                                   pi_RootToViewMatrix[][4]) const
    {    
    bool IsCorrect = false;
    if (!node->IsLoaded())
        node->Load();
 
    size_t nbOfPointsInTile; 

    if (m_useSplitThresholdForTileSelection == true)
        {
        nbOfPointsInTile = node->GetSplitTreshold();    
        }
    else
        {
        nbOfPointsInTile = node->GetNbObjects();                
        }

    //Return always true for the root node so that something is displayed at the screen.
    /*NEEDS_WORK_SM : Not required? 
    if (node->GetParentNode() == 0) 
        {
        IsCorrect = true;
        }
    else
    */
    if (nbOfPointsInTile > 0)
        {        
        double                  rootToViewScale;
        int                     nbPoints = 8;
        HArrayAutoPtr<DPoint3d> facePts(new DPoint3d[nbPoints]);                
        double                  contentExtentArea = ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) * ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) /** ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())*/;
        double                  visibleNodeExtentArea = ExtentOp<EXTENT>::GetWidth(pi_visibleExtent) * ExtentOp<EXTENT>::GetHeight(pi_visibleExtent) /** ExtentOp<EXTENT>::GetThickness(pi_visibleExtent)*/;
        double                  visibleExtentToNodeExtentScale = visibleNodeExtentArea / contentExtentArea;

        // Usually the following value should be between 0 and 1 but for reason of floating-point mathematical imprecision
        // sometimes the value is slightly greater than 1 by a minuscule amount. A 1 percent tolerance is quite acceptable
        // for our purpose and will not result in any ill effect.

        //NEEDS_WORK_SM : Stitching triangles usually cross the tile boundary.
        assert((visibleExtentToNodeExtentScale > 0) /*&& (visibleExtentToNodeExtentScale <= 1.01)*/);

        EXTENT contentExtent = node->GetContentExtent();
                        
        // We convert the visible extent into targetGCS units (or meters if no GCS)
        // because the localToView matrix was also converted this way.        
        facePts[0].x = ExtentOp<EXTENT>::GetXMin(contentExtent);
        facePts[0].y = ExtentOp<EXTENT>::GetYMin(contentExtent);
        facePts[0].z = 0;//ExtentOp<EXTENT>::GetZMin(contentExtent);

        facePts[1].x = ExtentOp<EXTENT>::GetXMax(contentExtent);
        facePts[1].y = ExtentOp<EXTENT>::GetYMin(contentExtent);
        facePts[1].z = 0;//ExtentOp<EXTENT>::GetZMin(contentExtent);

        facePts[2].x = ExtentOp<EXTENT>::GetXMin(contentExtent);
        facePts[2].y = ExtentOp<EXTENT>::GetYMax(contentExtent);
        facePts[2].z = 0;//ExtentOp<EXTENT>::GetZMin(contentExtent);

        facePts[3].x = ExtentOp<EXTENT>::GetXMax(contentExtent);
        facePts[3].y = ExtentOp<EXTENT>::GetYMax(contentExtent);
        facePts[3].z = 0;//ExtentOp<EXTENT>::GetZMin(contentExtent);

        facePts[4].x = ExtentOp<EXTENT>::GetXMin(contentExtent);
        facePts[4].y = ExtentOp<EXTENT>::GetYMin(contentExtent);
        facePts[4].z = ExtentOp<EXTENT>::GetZMax(contentExtent);

        facePts[5].x = ExtentOp<EXTENT>::GetXMax(contentExtent);
        facePts[5].y = ExtentOp<EXTENT>::GetYMin(contentExtent);
        facePts[5].z = ExtentOp<EXTENT>::GetZMax(contentExtent);

        facePts[6].x = ExtentOp<EXTENT>::GetXMin(contentExtent);
        facePts[6].y = ExtentOp<EXTENT>::GetYMax(contentExtent);
        facePts[6].z = ExtentOp<EXTENT>::GetZMax(contentExtent);

        facePts[7].x = ExtentOp<EXTENT>::GetXMax(contentExtent);
        facePts[7].y = ExtentOp<EXTENT>::GetYMax(contentExtent);
        facePts[7].z = ExtentOp<EXTENT>::GetZMax(contentExtent);
        
        if ((m_sourceGCSPtr != 0) && (m_targetGCSPtr != 0))
            {       
            GeoPoint  sourceLatLong;
            GeoPoint  targetLatLong;        
            StatusInt stat1;
            StatusInt stat2;
            StatusInt stat3;
              
            for (int boxPtInd = 0; boxPtInd < nbPoints; boxPtInd++)
                {                                             
                stat1 = m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, facePts[boxPtInd]);
                assert(stat1 == 0);
                stat2 = m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *m_targetGCSPtr);
                assert(stat2 == 0);
                stat3 = m_targetGCSPtr->CartesianFromLatLong(facePts[boxPtInd], targetLatLong);                        
                assert(stat3 == 0);
                }    
            }
        
        double vanishingLineCutCorrectionFactor = 1.0;                                                                       

        bcdtmMultiResolution_getMaxFaceAreaInCurrentViewCamOn(rootToViewScale, vanishingLineCutCorrectionFactor, pi_RootToViewMatrix, facePts.get(), nbPoints);
            
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
               
        //Note that the DTM area to tile area ratio is not valid in the case of a projective.
        //IsCorrect = HDOUBLE_GREATER_OR_EQUAL(rootToViewScale * node->GetViewDependentMetrics()[0] / node->GetNbObjects(), m_meanAreaFactor, m_meanAreaFactorTolerance);
        //IsCorrect = rootToViewScale * node->GetViewDependentMetrics()[0] / node->GetNbObjects() > m_meanScreenPixelsPerPoint;
        //IsCorrect = rootToViewScale / (nbOfPointsInTile * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor) > m_meanScreenPixelsPerPoint;                           
        double screenPixelsPerPoint = rootToViewScale / (nbOfPointsInTile * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor); 
        IsCorrect = screenPixelsPerPoint > m_meanScreenPixelsPerPoint;                                                       
        } 
    else
    if (node->m_nodeHeader.m_totalCount > 0)
        {            
        IsCorrect = true;
        }

    return IsCorrect;

    //return (node->GetViewDependentMetrics()[pi_NearestPredefinedCameraOri] * rootToViewScale) > m_meanAreaFactor;
}

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::IsCorrectForCurrentViewSphere(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                                                  const EXTENT&                           pi_visibleExtent,
                                                                                                                                  int                                     pi_NearestPredefinedCameraOri,
                                                                                                                                  double                                  pi_RootToViewMatrix[][4]) const
    {    
    bool IsCorrect = false;
    if (!node->IsLoaded())
        node->Load();
 
    size_t nbOfPointsInTile; 

    if (m_useSplitThresholdForTileSelection == true)
        {
        nbOfPointsInTile = node->GetSplitTreshold();    
        }
    else
        {
        nbOfPointsInTile = node->GetNbObjects();                
        }

    //Return always true for the root node so that something is displayed at the screen.
    /*NEEDS_WORK_SM : Not required? 
    if (node->GetParentNode() == 0) 
        {
        IsCorrect = true;
        }
    else
    */
    if (nbOfPointsInTile > 0)
        {        
        //double                  rootToViewScale;
        int                     nbPoints = 8;
        HArrayAutoPtr<DPoint3d> facePts(new DPoint3d[nbPoints]);                
        //double                  contentExtentArea = ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) * ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) /** ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())*/;



        /*&& (m_rootToViewMatrix[3][0] != 0 || m_rootToViewMatrix[3][1] != 0 || m_rootToViewMatrix[3][2] != 0)*/

        DVec3d vecParallelToProjPlane; 

        if (!s_useXrowForCamOn && (m_rootToViewMatrix[3][0] != 0 || m_rootToViewMatrix[3][1] != 0 || m_rootToViewMatrix[3][2] != 0))
            {
            DVec3d normalToProjectionPlane(DVec3d::From(pi_RootToViewMatrix[3][0], pi_RootToViewMatrix[3][1], pi_RootToViewMatrix[3][2]));                    
            DVec3d vecParallelToProjPlane2; 
            normalToProjectionPlane.GetNormalizedTriad(vecParallelToProjPlane, vecParallelToProjPlane2, normalToProjectionPlane);                    
            }
        else
            {
            vecParallelToProjPlane = DVec3d::From(pi_RootToViewMatrix[0][0], pi_RootToViewMatrix[0][1], pi_RootToViewMatrix[0][2]);
            vecParallelToProjPlane.normalize();
            }

        double maxDimension =  max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
        vecParallelToProjPlane.Scale(maxDimension);

        DPoint3d center;

        center.Init(ExtentOp<EXTENT>::GetXMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetYMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetZMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetThickness(node->GetContentExtent()) / 2); 

        DMatrix4d rootToView;        
        memcpy(&rootToView.coff, pi_RootToViewMatrix, sizeof(double) * 16);                
        DPoint3d edge(center);
        edge.Add(vecParallelToProjPlane);
        DPoint3d centerInView;
        DPoint3d edgeInView;

        rootToView.MultiplyAndRenormalize(centerInView, center);
        rootToView.MultiplyAndRenormalize(edgeInView, edge);
        double distance = centerInView.DistanceXY(edgeInView) * 2;
        double area = distance * distance;

        double screenPixelsPerPoint = area / nbOfPointsInTile; 
        IsCorrect = screenPixelsPerPoint > m_meanScreenPixelsPerPoint;                                                       

       
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
        } 
    else
    if (node->m_nodeHeader.m_totalCount > 0)
        {            
        IsCorrect = true;
        }

    return IsCorrect;    
}




#ifdef ACTIVATE_NODE_QUERY_TRACING

/**----------------------------------------------------------------------------
 Set the tracing XML file name which outputs a trace of the nodes checked 
 during the last query. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::SetTracingXMLFileName(AString& pi_rTracingXMLFileName)
{
    m_pTracingXMLFileName = pi_rTracingXMLFileName;
}

/**----------------------------------------------------------------------------
 Set the tracing XML file name which outputs a trace of the nodes checked 
 during the last query. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::OpenTracingXMLFile()
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

template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::CloseTracingXMLFile()
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


template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                size_t numSubNodes,
                                                                                                list<POINT>& resultPoints)
    {
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                size_t numSubNodes,
                                                                                                HPMMemoryManagedVector<POINT>& resultPoints)
    {
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                     HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                     size_t numSubNodes,
                                                                                                     HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode)
    {
    EXTENT ext = node->m_nodeHeader.m_contentExtent;
    DRange3d range = DRange3d::From(DPoint3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext)),
                                    DPoint3d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext)));
    DSegment3d segment;
    DRange1d fraction;
    DRange2d range2d = DRange2d::From(DPoint2d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext)),
                                      DPoint2d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext)));
    DPoint2d origin2d = DPoint2d::From(m_target.origin.x, m_target.origin.y);
    double par,par2;
    DPoint2d intersect2d;
    DPoint2d dest2d = DPoint2d::From(m_target.direction.x, m_target.direction.y);
    if (!m_is2d ? m_target.ClipToRange(range, segment, fraction) : (bsiDRange2d_intersectRay(&range2d, &par, &par2, &intersect2d, NULL, &origin2d, &dest2d) && (par2 > 0 || par > 0))) //ray intersects the node
        {
        if (m_is2d && m_depth != -1 && (m_depth<par)) return false;
        if (node->m_nodeHeader.m_totalCount == 0) return false;
        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == m_requestedLevel) || (!node->m_nodeHeader.m_balanced && node->IsLeaf()) && (!m_is2d || par > 0))
                {
                if (isnan(m_bestHitScore) || (m_intersect == RaycastOptions::LAST_INTERSECT && m_bestHitScore < (!m_is2d ? fraction.high : par))
                    || (m_intersect == RaycastOptions::FIRST_INTERSECT && m_bestHitScore >(!m_is2d ? fraction.low : par)))
                    {
                    hitNode = node;
                    if(!m_is2d)
                        m_bestHitScore = m_intersect == RaycastOptions::LAST_INTERSECT ? fraction.high : fraction.low;
                    else 
                        m_bestHitScore = par;
                    }
                }
        else if (node->m_nodeHeader.m_balanced && node->GetLevel() > m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }


template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                            size_t numSubNodes,
                                                                                                            list<POINT>& resultPoints)
    {
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                            size_t numSubNodes,
                                                                                                            HPMMemoryManagedVector<POINT>& resultPoints)
    {
    assert(!"Incorrect call");
    return false;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                            size_t numSubNodes,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode)
    {
    EXTENT ext = node->m_nodeHeader.m_contentExtent;
    DRange3d range = DRange3d::From(DPoint3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext)),
                                    DPoint3d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext)));

    DPoint3d corners[8];
    range.Get8Corners(corners);
    int sign = 0;
    bool boxIsOut = true;
    for (size_t i = 0; i < 8 && boxIsOut; i++)
        {
        int signOfPointRelativeToPlane = m_target.Evaluate(corners[i]);
        if ((signOfPointRelativeToPlane > 0 && sign < 0) || (signOfPointRelativeToPlane < 0 && sign > 0)) boxIsOut = false;
        sign = signOfPointRelativeToPlane;
        }
    if (!boxIsOut)
        {
        if (node->m_nodeHeader.m_totalCount == 0) return false;
        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == m_requestedLevel) || (!node->m_nodeHeader.m_balanced && node->IsLeaf()))
            {
            //if (isnan(m_bestHitScore) || (m_intersect == RaycastOptions::LAST_INTERSECT && m_bestHitScore < (!m_is2d ? fraction.high : par))
            //    || (m_intersect == RaycastOptions::FIRST_INTERSECT && m_bestHitScore >(!m_is2d ? fraction.low : par)))
                {
                hitNode = node;
            //    if (!m_is2d)
            //        m_bestHitScore = m_intersect == RaycastOptions::LAST_INTERSECT ? fraction.high : fraction.low;
             //   else
            //        m_bestHitScore = par;
                }
            }
        else if (node->m_nodeHeader.m_balanced && node->GetLevel() > m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                          HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                          size_t numSubNodes,
                          vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {
    EXTENT ext = node->m_nodeHeader.m_contentExtent;
    DRange3d range = DRange3d::From(DPoint3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext)),
                                    DPoint3d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext)));
    //DSegment3d segment;
    //DRange1d fraction;
    DPoint3d corners[8];
    range.Get8Corners(corners);
    int sign = 0;
    bool boxIsOut = true;
    for (size_t i = 0; i < 8 && boxIsOut; i++)
        {
        int signOfPointRelativeToPlane = m_target.Evaluate(corners[i]) < 0 ? -1 : 1;
        if ((signOfPointRelativeToPlane > 0 && sign < 0) || (signOfPointRelativeToPlane < 0 && sign > 0)) boxIsOut = false;
        sign = signOfPointRelativeToPlane;
        }
    if (!boxIsOut)
        {
        if (node->m_nodeHeader.m_totalCount == 0) return false;
        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == m_requestedLevel) || (!node->m_nodeHeader.m_balanced && node->IsLeaf()))
            {
            //if (isnan(m_bestHitScore) || (m_intersect == RaycastOptions::LAST_INTERSECT && m_bestHitScore < (!m_is2d ? fraction.high : par))
            //    || (m_intersect == RaycastOptions::FIRST_INTERSECT && m_bestHitScore >(!m_is2d ? fraction.low : par)))
                {
                meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
                }
            }
        else if (node->m_nodeHeader.m_balanced && node->GetLevel() > m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }