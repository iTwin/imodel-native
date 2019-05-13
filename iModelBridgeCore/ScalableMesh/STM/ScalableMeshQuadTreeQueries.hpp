//:>--------------------------------------------------------------------------------------+
//:>
//:>    $RCSfile: ScalableMeshQuadTreeQueries.hpp,v $
//:>   $Revision: 1.20 $
//:>       $Date: 2012/11/29 17:30:34 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Indicates if the provided node is adequate for obtaining result.
 The visible extent is of course provided in the STM GCS and units.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentPointQuery<POINT, EXTENT>::IsCorrectForCurrentView(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                                      const EXTENT&                            pi_visibleExtent,                                                                                                                      
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

        if ((this->m_sourceGCSPtr != 0) && (this->m_targetGCSPtr != 0))
            {       
            GeoPoint  sourceLatLong;
            GeoPoint  targetLatLong;        
            StatusInt stat1;
            StatusInt stat2;
            StatusInt stat3;
              
            for (int boxPtInd = 0; boxPtInd < 4; boxPtInd++)
                {                                             
                stat1 = this->m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, tileBorderPts[boxPtInd]);
                assert(stat1 == 0);
                stat2 = this->m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *this->m_targetGCSPtr);
                assert(stat2 == 0);
                stat3 = this->m_targetGCSPtr->CartesianFromLatLong(tileBorderPts[boxPtInd], targetLatLong);                        
                assert(stat3 == 0);
                }    
            }
#if 0 //Reprojection Plane
        double shapeWithVanishingLine[8];
#endif
        double vanishingLineCutCorrectionFactor = 1.0;
        
        if ((this->m_rootToViewMatrix[3][0] != 0.0) ||
            (this->m_rootToViewMatrix[3][1] != 0.0)) 
//            (this->m_rootToViewMatrix[3][2] != 0.0) ||            
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
                                                          this->m_rootToViewMatrix);

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
        if ((this->m_sourceGCSPtr != 0) && (this->m_targetGCSPtr != 0))
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

                stat1 = this->m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, sourcePoint);
                stat2 = this->m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *this->m_targetGCSPtr);
                stat3 = this->m_targetGCSPtr->CartesianFromLatLong(targetPoint, targetLatLong);            

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
                              "<CheckedNode><NbOfPoints>%i</NbOfPoints><Level>%i</Level>", 
                              node->GetNbObjects(), 
                              node->GetLevel());      

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

        
        if (node->GetNbObjects() > 0)
            {        
            size_t nbOfPointsInTile = node->GetNbObjects();                
                            
            //Note that the DTM area to tile area ratio is not valid in the case of a projective.            
            IsCorrect = rootToViewScale / (nbOfPointsInTile * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor) > this->m_meanScreenPixelsPerPoint;                           
            }
        else
            {
            IsCorrect = true;
            }
        }             

    return IsCorrect;    
    }

 /*   template<class EXTENT> bool GetVisibleExtent(EXTENT&        po_rVisibleExtent,
        const EXTENT&  pi_rExtentInMeters,
        const DPoint3d pi_pViewBox[]);

#ifndef LINUX_SCALABLEMESH_BUILD*/
    template<class EXTENT> bool GetVisibleExtent(EXTENT&        po_rVisibleExtent,
        const EXTENT&  pi_rExtent,
        const DPoint3d pi_pViewBox[])
    {
        bool isVisible = false;

        //::DPoint3d** fencePt
        int  nbPts;

        DRange3d dtmIntersectionRange;
        DRange3d extentRange;

        extentRange.low.x = ExtentOp<EXTENT>::GetXMin(pi_rExtent);
        extentRange.low.y = ExtentOp<EXTENT>::GetYMin(pi_rExtent);
        extentRange.low.z = ExtentOp<EXTENT>::GetZMin(pi_rExtent);

        extentRange.high.x = ExtentOp<EXTENT>::GetXMax(pi_rExtent);
        extentRange.high.y = ExtentOp<EXTENT>::GetYMax(pi_rExtent);
        extentRange.high.z = ExtentOp<EXTENT>::GetZMax(pi_rExtent);

        isVisible = GetVisibleAreaForView(0,
            nbPts,
            pi_pViewBox,
            extentRange,
            dtmIntersectionRange);

        if (isVisible == true)
        {
            DPoint3d lowPt(dtmIntersectionRange.low);
            DPoint3d highPt(dtmIntersectionRange.high);

            ExtentOp<EXTENT>::SetXMin(po_rVisibleExtent, lowPt.x);
            ExtentOp<EXTENT>::SetYMin(po_rVisibleExtent, lowPt.y);
            ExtentOp<EXTENT>::SetZMin(po_rVisibleExtent, lowPt.z);

            ExtentOp<EXTENT>::SetXMax(po_rVisibleExtent, highPt.x);
            ExtentOp<EXTENT>::SetYMax(po_rVisibleExtent, highPt.y);
            ExtentOp<EXTENT>::SetZMax(po_rVisibleExtent, highPt.z);
        }

        return isVisible;
    }
//#endif

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                   HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                   size_t                                   numSubNodes,
                                                                                                   vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {
    assert(node->GetFilter() == nullptr || node->GetFilter()->IsProgressiveFilter() == false);

    // Before we make sure requested level is appropriate
    if (this->m_requestedLevel < 0)
        this->m_requestedLevel = 0;

    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
        {
        nodeExtent = node->GetNodeExtent();
        //NEEDS_WORK_SM : Don't think this is necessary with an octree
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(this->m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(this->m_extent));
        }
    else
        {
        nodeExtent = node->GetContentExtent();
        }


    bool isVisible = m_alwaysVisible || this->m_extent3d != nullptr || GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, this->m_viewBox);

    if (this->m_extent3d != nullptr)
        {
        double maxDimension = max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
        double tolerance = maxDimension / cos(PI / 4);

        DPoint3d center;

        center.Init(ExtentOp<EXTENT>::GetXMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetYMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetZMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetThickness(node->GetContentExtent()) / 2);
        isVisible = this->m_extent3d->PointInside(center, tolerance);
        }

    double resNode = node->GetMinResolution();

    if ((isVisible == true) && (node->GetLevel() <= this->m_requestedLevel) && (node->GetParentNode() == nullptr || node->GetParentNode()->GetMinResolution() >= m_pixelTolerance))
        {
        if (!m_includeUnbalancedLeafs && node->IsLeaf() && node->GetLevel() < this->m_requestedLevel)
            {
            // Could not reach requested level because the index is not balanced; do not add node in the results
            return false;
            }

        // If this is the appropriate level or it is a higher level and progressive is set.
        if (
            m_useAllRes || this->m_requestedLevel == node->GetLevel() || (!node->m_nodeHeader.m_balanced && node->IsLeaf() || (m_pixelTolerance != 0.0 && resNode != 0.0 && resNode <= m_pixelTolerance)) /*||
                                                 (node->GetFilter()->IsProgressiveFilter() && this->m_requestedLevel > node->GetLevel())*/)
            {         
            if (node->m_nodeHeader.m_nbFaceIndexes > 0 || m_ignoreFaceIndexes)
                {
                meshNodes.push_back(typename SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
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
                   BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh*                mesh)
    {
    //NEEDS_WORK_SM : Remove
    assert(!"Should not be called");
    return ERROR;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
    size_t numSubNodes,
    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh)
{
    assert((node->GetFilter() == nullptr) || (node->GetFilter()->IsProgressiveFilter() == false));

    // Before we make sure requested level is appropriate
    if (this->m_requestedLevel < 0)
        this->m_requestedLevel = 0;

    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (node->IsEmpty())
    {
        nodeExtent = node->GetNodeExtent();
        //NEEDS_WORK_SM : Don't think this is necessary with an octree
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(this->m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(this->m_extent));
    }
    else
    {
        nodeExtent = node->GetContentExtent();
    }


    bool isVisible = GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, this->m_viewBox);

    //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
    static bool s_clipMesh = false;

    double resNode = node->GetMinResolution();

    if ((isVisible == true) && (node->GetLevel() <= this->m_requestedLevel) && (node->GetParentNode() == nullptr || node->GetParentNode()->GetMinResolution() >= m_pixelTolerance))
        {            
        // If this is the appropriate level or it is a higher level and progressive is set.
        if ( this->m_requestedLevel == node->GetLevel() || (!node->m_nodeHeader.m_balanced && node->IsLeaf()) || (m_pixelTolerance != 0.0 && resNode != 0.0 && resNode <= m_pixelTolerance))
            {
            // Copy content            
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {                               
                int status;

                if (s_clipMesh == true)
                    {   
                    status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);                
                    }
                else
                    {                    
                    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(node);

                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());                    
                    vector<DPoint3d> dataPoints(pointsPtr->size());

                    PtToPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));                                            
                        }

                    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(meshNode->GetPtsIndicePtr());

                    status = mesh->AppendMesh(pointsPtr->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                            BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh) 
    {                                       
    return HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, mesh);             
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GlobalPreQuery(SMPointIndex<POINT, EXTENT>& index,
                                                                                                            vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes) 
    {        
    return HGFViewDependentPointIndexQuery<POINT, EXTENT>::GlobalPreQuery(index, meshNodes);    
    }

template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::ToggleLimitToLevel(bool shouldLimit, size_t maxLevel)
{
    m_hasLevelLimit = shouldLimit;
    m_levelLimit = maxLevel;
}

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh*               mesh)
    {                        
    bool queryResult;

    assert(mesh != 0);
    
    if (mesh->GetNbPoints() > m_maxNumberOfPoints)
        {
        return false;    
        }
    else    
        {           
        // Check if extent overlap         
        EXTENT visibleExtent;
        EXTENT nodeExtent;

        if (node->IsEmpty())
            {
            nodeExtent = node->GetNodeExtent();
            ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(this->m_extent));
            ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(this->m_extent));                        
            }
        else
            {            
            nodeExtent = node->GetContentExtent();                     
            }
                             
        if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, this->m_viewBox) == FALSE)
            return false;

        bool finalNode = false;

        // Check if coordinate falls inside node extent
        //NEEDS_WORK_SM : The incorrect one is use, probably need to change the UI accordingly or use the parent data with limited region.
        finalNode = !IsCorrectForCurrentView(node, visibleExtent, this->m_rootToViewMatrix);

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)              
        if (finalNode == true || node->IsLeaf() || (m_hasLevelLimit && node->GetLevel() >= m_levelLimit))
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
                    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(node);

                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
                    vector<DPoint3d> dataPoints(pointsPtr->size());

                    PtToPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));                                            
                        }                     

                    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(meshNode->GetPtsIndicePtr());

                    status = mesh->AppendMesh(pointsPtr->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
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
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
            vector<DPoint3d> dataPoints(pointsPtr->size());

            PtToPtConverter converter; 

            for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
                {
                dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));                                            
                }
            auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(node);

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(meshNode->GetPtsIndicePtr());

            int status = mesh->AppendMesh(pointsPtr->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);

            assert(status == SUCCESS);                        
            }
     
        if (finalNode && this->m_gatherTileBreaklines && node->GetNbPoints() > 0)
            {            
            this->AddBreaklinesForExtent(node->GetNodeExtent());
            }
        
            queryResult = !finalNode;
        }         

    return queryResult; 
    }

//NEEDS_WORK_SM Cleanup
static bool s_useNew3dLODQuery = true;
static bool s_useXrowForCamOn = false;
static bool s_useClipVectorForVisibility = true;

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                    size_t                                   numSubNodes,
                                                                                                    vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {                        
    bool queryResult;
        
    auto nodeIter(meshNodes.begin()); 
    auto nodeIterEnd(meshNodes.end()); 

    size_t nbPoints = 0;

    while (nodeIter != nodeIterEnd)
        {
        nbPoints += (*nodeIter).m_indexNode->GetNbPoints();
        nodeIter++;
        }
    
    if (nbPoints > m_maxNumberOfPoints)
        {
        return false;    
        }
    else    
        {           
        // Check if extent overlap         
        EXTENT visibleExtent;
        EXTENT nodeExtent;

        if (node->IsEmpty())
            {
            /*
            nodeExtent = node->GetNodeExtent();
            ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(this->m_extent));
            ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(this->m_extent));                        
            */
            return false;
            }
        
        nodeExtent = node->GetContentExtent();  

        if (m_invertClips && dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(node.GetPtr()) != nullptr && dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(node.GetPtr())->m_nbClips == 0)
            return false;
                     
        if (s_useClipVectorForVisibility)
            {            
            assert(this->m_viewClipVector != 0);

            //NEEDS_WORK_SM : Tolerance (i.e. radius) and center could be precomputed during SM generation
            double maxDimension = max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
            double tolerance =  sqrt(2)* maxDimension / cos(PI / 4);

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
            if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, this->m_viewBox) == FALSE)
                return false;
            }

        bool finalNode = false;
        bool shouldAddNode = true;
        // Check if coordinate falls inside node extent
        //NEEDS_WORK_SM : The incorrect one is use, probably need to change the UI accordingly or use the parent data with limited region.
        if (s_useNew3dLODQuery /*&& (this->m_rootToViewMatrix[3][0] != 0 || this->m_rootToViewMatrix[3][1] != 0 || this->m_rootToViewMatrix[3][2] != 0)*/)
            {
            finalNode = !IsCorrectForCurrentViewSphere(node, visibleExtent, this->m_rootToViewMatrix, shouldAddNode);
            }
        else
            {
            //NEEDS_WORK_SM : Not needed?
            finalNode = !IsCorrectForCurrentView(node, visibleExtent, this->m_rootToViewMatrix);
            }

        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)              
        if (finalNode == true || node->IsLeaf() || (m_hasLevelLimit && node->GetLevel() >= m_levelLimit))
            {                                       
            assert(node->GetFilter()->IsProgressiveFilter() == false);
                        
            if (node->m_nodeHeader.m_nbFaceIndexes > 0)
                {         
                //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
                static bool s_clipMesh = true;
                if (shouldAddNode)
                meshNodes.push_back(typename SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
                /*
                if (s_clipMesh == true)
                    { 

                    status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);
                    }
                else
                    {
                    //size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));                  
                    vector<DPoint3d> dataPoints(node->size());

                    PtToPtConverter converter; 

                    for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                        {
                        dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                        }
                                
                    status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&node->operator[](node->size()), 0, 0, 0);                    
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
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
            vector<DPoint3d> dataPoints(pointsPtr->size());

            PtToPtConverter converter; 

            for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
                {
                dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));                                            
                }
                         
            meshNodes.push_back(typename SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));            
            }
     
        if (finalNode && this->m_gatherTileBreaklines && node->GetNbPoints() > 0)
            {            
            this->AddBreaklinesForExtent(node->GetNodeExtent());
            }
        
            queryResult = !finalNode;
        }
       
    return queryResult; 
    }
    
static double s_toleranceFactor = 1.0;
static bool s_alwaysTrue = false;

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node, 
                                                                                                          HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                          size_t                                  numSubNodes,
                                                                                                          ProducedNodeContainer<POINT, EXTENT>&   foundNodes)
    {                        
    bool queryResult;    
                                   
    // Check if extent overlap         
    EXTENT visibleExtent;
    EXTENT nodeExtent;

    if (s_alwaysTrue)
        return true;

    if (node->IsEmpty())
        {
        /*
        nodeExtent = node->GetNodeExtent();
        ExtentOp<EXTENT>::SetZMin(nodeExtent, ExtentOp<EXTENT>::GetZMin(this->m_extent));
        ExtentOp<EXTENT>::SetZMax(nodeExtent, ExtentOp<EXTENT>::GetZMax(this->m_extent));                        
        */
        return false;
        }    
    
    nodeExtent = node->GetContentExtent();                     
                     
    if (s_useClipVectorForVisibility)
        {            
        assert(this->m_viewClipVector != 0);

        //NEEDS_WORK_SM : Tolerance (i.e. radius) and center could be precomputed during SM generation
        double maxDimension = max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
        double tolerance =  maxDimension / cos(PI / 4) * s_toleranceFactor;

        DPoint3d center;

        center.Init(ExtentOp<EXTENT>::GetXMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetYMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) / 2,
                    ExtentOp<EXTENT>::GetZMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetThickness(node->GetContentExtent()) / 2); 

        if (!m_viewClipVector->PointInside(center, tolerance))                
            {            
            return false;      
            }

        visibleExtent = node->GetContentExtent();
        }
    else
        {
        if (GetVisibleExtent<EXTENT>(visibleExtent, nodeExtent, this->m_viewBox) == FALSE)
            return false;
        }

    bool finalNode = false;
    bool shouldAddNode = true;
    // Check if coordinate falls inside node extent
    //NEEDS_WORK_SM : The incorrect one is use, probably need to change the UI accordingly or use the parent data with limited region.
    if (s_useNew3dLODQuery /*&& (this->m_rootToViewMatrix[3][0] != 0 || this->m_rootToViewMatrix[3][1] != 0 || this->m_rootToViewMatrix[3][2] != 0)*/)
        {
        finalNode = !IsCorrectForCurrentViewSphere(node, visibleExtent, this->m_rootToViewMatrix, shouldAddNode);
        }
    else
        {
        //NEEDS_WORK_SM : Not needed?
        finalNode = !IsCorrectForCurrentView(node, visibleExtent, this->m_rootToViewMatrix);
        }

    if (m_hasLevelLimit && node->GetLevel() >= m_levelLimit)
        finalNode = true;
    // The point is located inside the node ...
    // Obtain objects from subnodes (if any)              
    if (finalNode == true || node->IsLeaf())
        {                                       
        assert(node->GetFilter()==nullptr || node->GetFilter()->IsProgressiveFilter() == false);
                    
        if (!node->IsEmpty()/*node->m_nodeHeader.m_nodeCount > 0*/)
            {         
            //NEEDS_WORK_SM - In progress, can miss triangle when considering only vertices 
            static bool s_clipMesh = true;                                
            if (shouldAddNode) foundNodes.AddNode(node);
            /*
            if (s_clipMesh == true)
                { 

                status = AddVisibleMesh<POINT, EXTENT>(node, visibleExtent, mesh);
                }
            else
                {
                //size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));                  
                vector<DPoint3d> dataPoints(node->size());

                PtToPtConverter converter; 

                for (size_t pointInd = 0; pointInd < node->size(); pointInd++)
                    {
                    dataPoints[pointInd] = converter.operator()(node->operator[](pointInd));                                            
                    }
                            
                status = mesh->AppendMesh(node->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&node->operator[](node->size()), 0, 0, 0);                    
                }
                */               
            }
        else
            {
            //NEEDS_WORK_SM : Seems to happen with ScalableMeshQuadTreeBCLIBMeshFilter1 because of duplicate points. 
            //In the end every node with at least one points should probably have a mesh in it.
            //assert(node->size() <= 4);
            }
        }   
    else //Root node with no child
    if (node->GetParentNode() == 0 && node->GetSubNodeNoSplit() == 0 && node->m_apSubNodes[0] == 0)
        {
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
        vector<DPoint3d> dataPoints(pointsPtr->size());

        PtToPtConverter converter; 

        for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));                                            
            }
                                 
        foundNodes.AddNode(node);
        }
    
    if (finalNode && this->m_gatherTileBreaklines && node->GetNbPoints() > 0)
        {            
        this->AddBreaklinesForExtent(node->GetNodeExtent());
        }
    
        queryResult = !finalNode;
    
    return queryResult; 
    }


template<class POINT, class EXTENT> void ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::GetQueryNodeOrder(vector<size_t>&                            queryNodeOrder, 
                                                                                                               HFCPtr<SMPointIndexNode<POINT, EXTENT> >  node, //NEEDS_WORK_SM - Needed?
                                                                                                               HFCPtr<SMPointIndexNode<POINT, EXTENT> >  subNodes[],
                                                                                                               size_t                                     numSubNodes) 
    {    
    assert(queryNodeOrder.size() == 0);
	
    DMatrix4d rootToViewMatrix; 

	memcpy(&rootToViewMatrix.coff, this->m_rootToViewMatrix, sizeof(double) * 16);                
    
    struct OrderInfo
        {
        double m_zScreen;
        size_t m_nodeInd; 
        };

    vector<OrderInfo> orderInfos;
            
    for (size_t nodeInd = 0; nodeInd < numSubNodes; nodeInd++)
        {
        if (subNodes[nodeInd] == nullptr) continue;
        DPoint3d center;
        center.x = (ExtentOp<EXTENT>::GetXMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetXMin(subNodes[nodeInd]->GetNodeExtent())) / 2;
        center.y = (ExtentOp<EXTENT>::GetYMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetYMin(subNodes[nodeInd]->GetNodeExtent())) / 2;
        center.z = (ExtentOp<EXTENT>::GetZMax(subNodes[nodeInd]->GetNodeExtent()) + ExtentOp<EXTENT>::GetZMin(subNodes[nodeInd]->GetNodeExtent())) / 2;        
                
        DPoint3d outPoint;

		rootToViewMatrix.MultiplyAndRenormalize(outPoint, center);
        
        OrderInfo orderInfo;
        orderInfo.m_zScreen = outPoint.z;
        orderInfo.m_nodeInd = nodeInd;

        orderInfos.push_back(orderInfo);                           
        }
	DPoint4d fwd;
	rootToViewMatrix.GetColumn(fwd, 2);
	DVec3d vecFwd;
	vecFwd.XyzOf(fwd);
    std::sort(orderInfos.begin(), orderInfos.end(), [vecFwd](OrderInfo& i,OrderInfo& j){
		if (vecFwd.z != 0)
		{
			return vecFwd.z > 0 ? i.m_zScreen < j.m_zScreen : i.m_zScreen > j.m_zScreen;

		}
		return i.m_zScreen > j.m_zScreen;
	});            

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
                                                                                                                      double                                   pi_RootToViewMatrix[][4]) const
    {    
    bool IsCorrect = false;
    if (!node->IsLoaded())
        node->Load();
 
    size_t nbOfPointsInTile = node->GetNbObjects();     
		

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


        assert((visibleExtentToNodeExtentScale > 0));

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
        
        if ((this->m_sourceGCSPtr != 0) && (this->m_targetGCSPtr != 0))
            {       
            GeoPoint  sourceLatLong;
            GeoPoint  targetLatLong;        
            StatusInt stat1;
            StatusInt stat2;
            StatusInt stat3;
              
            for (int boxPtInd = 0; boxPtInd < nbPoints; boxPtInd++)
                {                                             
                stat1 = this->m_sourceGCSPtr->LatLongFromCartesian(sourceLatLong, facePts[boxPtInd]);
                assert(stat1 == 0);
                stat2 = this->m_sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *this->m_targetGCSPtr);
                assert(stat2 == 0);
                stat3 = this->m_targetGCSPtr->CartesianFromLatLong(facePts[boxPtInd], targetLatLong);                        
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
                              "<CheckedNode><NbOfPoints>%i</NbOfPoints><Level>%i</Level>", 
                              node->GetNbObjects(), 
                              node->GetLevel());      

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
        double screenPixelsPerPoint = rootToViewScale / (nbOfPointsInTile * visibleExtentToNodeExtentScale * vanishingLineCutCorrectionFactor); 
        IsCorrect = screenPixelsPerPoint > this->m_meanScreenPixelsPerPoint;                                                       
        } 
    else
    if ((node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount > 0) || !node->IsEmpty())
        {            
        IsCorrect = true;
        }

    return IsCorrect;    
    }

static bool s_useSplit = true;

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeViewDependentMeshQuery<POINT, EXTENT>::IsCorrectForCurrentViewSphere(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                                                  const EXTENT&                           pi_visibleExtent,                                                                                                                                  
                                                                                                                                  double                                  pi_RootToViewMatrix[][4],
                                                                                                                                  bool& shouldAddNode ) const
    {    
    bool IsCorrect = false;
    if (!node->IsLoaded())
        node->Load();
 
    size_t nbOfPointsInTile = node->GetNbObjects();                
    

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

		if (s_useSplit)
			{
			nbOfPointsInTile = node->GetSplitTreshold();
			}

        //double                  rootToViewScale;
        int                     nbPoints = 8;
        HArrayAutoPtr<DPoint3d> facePts(new DPoint3d[nbPoints]);                
        //double                  contentExtentArea = ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) * ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) /** ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())*/;



        /*&& (this->m_rootToViewMatrix[3][0] != 0 || this->m_rootToViewMatrix[3][1] != 0 || this->m_rootToViewMatrix[3][2] != 0)*/

        DVec3d vecParallelToProjPlane; 

        if (!s_useXrowForCamOn && (this->m_rootToViewMatrix[3][0] != 0 || this->m_rootToViewMatrix[3][1] != 0 || this->m_rootToViewMatrix[3][2] != 0))
            {
            DVec3d normalToProjectionPlane(DVec3d::From(pi_RootToViewMatrix[3][0], pi_RootToViewMatrix[3][1], pi_RootToViewMatrix[3][2]));                    
            DVec3d vecParallelToProjPlane2; 
            normalToProjectionPlane.GetNormalizedTriad(vecParallelToProjPlane, vecParallelToProjPlane2, normalToProjectionPlane);                    
            }
        else
            {
            vecParallelToProjPlane = DVec3d::From(pi_RootToViewMatrix[0][0], pi_RootToViewMatrix[0][1], pi_RootToViewMatrix[0][2]);
            vecParallelToProjPlane.Normalize();
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

		
		/*if ((this->m_rootToViewMatrix[3][0] != 0 || this->m_rootToViewMatrix[3][1] != 0 || this->m_rootToViewMatrix[3][2] != 0) && (centerInView.z > 0 || edgeInView.z > 0))
			{
			IsCorrect = true;
			}
		else*/
			{
            double res = node->GetMinResolution();
            if (res != 0.0)
                {
                double unitsPerPixel = maxDimension / distance;
                IsCorrect = unitsPerPixel * this->m_maxPixelError < res;
                }
            else
                {
                double screenPixelsPerPoint = area / nbOfPointsInTile;
                IsCorrect = screenPixelsPerPoint > this->m_meanScreenPixelsPerPoint;
                }
			}
						       
    #ifdef ACTIVATE_NODE_QUERY_TRACING

        if (m_pTracingXMLFile != 0)
        {
            char   TempBuffer[3000]; 
            int    NbChars;
            size_t NbWrittenChars;    
            
            //Extent
            NbChars = sprintf(TempBuffer, 
                              "<CheckedNode><NbOfPoints>%i</NbOfPoints><Level>%i</Level>", 
                              node->GetNbObjects(), 
                              node->GetLevel());      

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
    if ((node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount > 0) || !node->IsEmpty()) // The total count is not available in Cesium 3D tiles datasets...
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
                                                                                                     HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode)
    {
    EXTENT ext = (node->GetLevel() == this->m_requestedLevel || node->IsLeaf()) ? node->m_nodeHeader.m_contentExtent : node->m_nodeHeader.m_nodeExtent;
    DRange3d range = DRange3d::From(DPoint3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext)),
                                    DPoint3d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext)));
    DSegment3d segment;
    DRange1d fraction;
    DRange2d range2d = DRange2d::From(DPoint2d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext)),
                                      DPoint2d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext)));
    DPoint2d origin2d = DPoint2d::From(m_target.origin.x, m_target.origin.y);
    double par=0,par2=0;
    DPoint2d intersect2d, intersect2d1;
    DPoint2d dest2d = DPoint2d::From(m_target.direction.x, m_target.direction.y);

    if (!m_is2d ? m_target.ClipToRange(range, segment, fraction) : (range2d.IntersectRay (par, par2, intersect2d, intersect2d1, origin2d, dest2d) && (par2 > 0 || par > 0))) //ray intersects the node
        {
        if (m_is2d && m_depth != -1 && (m_depth<par)) return false;
        if (node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount == 0) return false;

        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == this->m_requestedLevel) || (!node->m_nodeHeader.m_balanced && (node->IsLeaf() || node->GetLevel() == this->m_requestedLevel)) && (!m_is2d || par > 0))
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
        else if (node->GetLevel() > this->m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                            size_t numSubNodes,
                                                                                                            vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {
    EXTENT ext = /*(node->GetLevel() == this->m_requestedLevel || node->IsLeaf()) ?*/ node->m_nodeHeader.m_contentExtent; /*: node->m_nodeHeader.m_nodeExtent;*/
    DRange3d range = DRange3d::From(DPoint3d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext)),
                                    DPoint3d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext)));
    DSegment3d segment;
    DRange1d fraction;
    DRange2d range2d = DRange2d::From(DPoint2d::From(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext)),
                                      DPoint2d::From(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext)));

    DPoint2d origin2d = DPoint2d::From(m_target.origin.x, m_target.origin.y);
    double par=0, par2=0;
    DPoint2d intersect2d, intersect2dA;
    DPoint2d dest2d = DPoint2d::From(m_target.direction.x, m_target.direction.y);

    if (!m_is2d ? m_target.ClipToRange(range, segment, fraction) : (range2d.IntersectRay (par, par2, intersect2d, intersect2dA, origin2d, dest2d) && (par2 > 0 || par > 0))) //ray intersects the node
        {
        if (m_is2d && m_depth != -1 && (m_depth < par)) return false;
        if (!m_is2d && m_depth != -1 && ((m_depth < fraction.low) || (fraction.high < 0 && m_depth < fabs(fraction.high)))) return false;
        if (!m_is2d && !m_useUnboundedRay && fraction.high < 1e-6) return false;
        if (node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount == 0) return false;

        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == this->m_requestedLevel) || (!node->m_nodeHeader.m_balanced && (node->GetLevel() == this->m_requestedLevel || node->IsLeaf())) && (!m_is2d || (par > 0||par2 > 0)))
            {
            double positionAlongRay = m_is2d ? par : fraction.low;
            auto it = m_fractions.insert(std::lower_bound(m_fractions.begin(), m_fractions.end(), positionAlongRay), positionAlongRay);
            meshNodes.insert(meshNodes.begin() + (it - m_fractions.begin()), node);
            if (node->GetLevel() == this->m_requestedLevel)
                return false;
            }
        else if (node->GetLevel() >= this->m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT> > node,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> > subNodes[],
                                                                                                            size_t numSubNodes,
                                                                                                            HFCPtr<SMPointIndexNode<POINT, EXTENT> >& hitNode)
    {
    EXTENT ext = (node->GetLevel() == this->m_requestedLevel || node->IsLeaf()) ? node->m_nodeHeader.m_contentExtent : node->m_nodeHeader.m_nodeExtent;
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
        if (node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount == 0) return false;
        if ((node->m_nodeHeader.m_balanced && node->GetLevel() == this->m_requestedLevel) || (!node->m_nodeHeader.m_balanced && node->IsLeaf()))
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
        else if (node->m_nodeHeader.m_balanced && node->GetLevel() > this->m_requestedLevel) return false; //too deep
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
        if (node->m_nodeHeader.m_totalCountDefined && node->m_nodeHeader.m_totalCount == 0) return false;
        if ((node->GetLevel() == this->m_requestedLevel) || (!node->m_nodeHeader.m_balanced && node->IsLeaf()))
            {
            //if (isnan(m_bestHitScore) || (m_intersect == RaycastOptions::LAST_INTERSECT && m_bestHitScore < (!m_is2d ? fraction.high : par))
            //    || (m_intersect == RaycastOptions::FIRST_INTERSECT && m_bestHitScore >(!m_is2d ? fraction.low : par)))
                {
                meshNodes.push_back(typename SMPointIndexNode<POINT, EXTENT>::QueriedNode(node));
                }
				return false;
            }
        else if (node->m_nodeHeader.m_balanced && node->GetLevel() > this->m_requestedLevel) return false; //too deep
        }
    else return false; //don't do subnodes, this is not the right extent
    return true;
    }

struct AppendClippedToMesh : public PolyfaceQuery::IClipToPlaneSetOutput
    {
    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* m_targetMesh;
    ClipVectorCP m_clipVec;

    virtual StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override;
    virtual StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override;

    AppendClippedToMesh(BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* mesh, ClipVectorCP clip) : m_targetMesh(mesh), m_clipVec(clip) {};
    };

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeContextMeshQuery<POINT,EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                          HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                          size_t                                   numSubNodes,
                                                                                                          BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh*               mesh)

    {
    bool result = true;


    if (node->IsEmpty() || node->m_nodeHeader.m_level > 3)
        {
        return false;
        }


    assert(this->m_viewClipVector != 0);

    //NEEDS_WORK_SM : Tolerance (i.e. radius) and center could be precomputed during SM generation
    double maxDimension = max(max(ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()), ExtentOp<EXTENT>::GetHeight(node->GetContentExtent())), ExtentOp<EXTENT>::GetThickness(node->GetContentExtent())) / 2.0;
    double tolerance = maxDimension / cos(PI / 4);

    DPoint3d center;

    center.Init(ExtentOp<EXTENT>::GetXMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetWidth(node->GetContentExtent()) / 2,
                ExtentOp<EXTENT>::GetYMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetHeight(node->GetContentExtent()) / 2,
                ExtentOp<EXTENT>::GetZMin(node->GetContentExtent()) + ExtentOp<EXTENT>::GetThickness(node->GetContentExtent()) / 2);
    if (node->m_nodeHeader.m_level == 3 || node->m_nodeHeader.m_IsLeaf)
        {
        auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(node);

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
        vector<DPoint3d> dataPoints(pointsPtr->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));
            }

        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(meshNode->GetPtsIndicePtr());

        if (this->m_viewClipVector->PointInside(center, tolerance))
            {
            AppendClippedToMesh meshOutput(mesh, this->m_viewClipVector.get());
            auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(static_cast<SMPointIndexNode<POINT, EXTENT>*>(dynamic_cast<SMMeshIndexNode<POINT, EXTENT>*>(node.GetPtr())));
            IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
            new ScalableMeshNode<POINT>(nodePtr)
#else
            ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
            );
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            IScalableMeshMeshPtr meshP = nodeP->GetMesh(flags);

            if (meshP.get() != nullptr)
                this->m_viewClipVector->ClipPolyface(*(meshP->GetPolyfaceQuery()), meshOutput, false);

            }
        else
            {
            bvector<int> indices(node->m_nodeHeader.m_nbFaceIndexes);
            if (!indices.empty())
                {
                memcpy(indices.data(), &(*ptIndices)[0], indices.size()*sizeof(int));
                for (auto& i : indices) i += (int)mesh->GetNbPoints();
                mesh->AppendMesh(pointsPtr->size(), &dataPoints[0], node->m_nodeHeader.m_nbFaceIndexes, indices.data(), 0, 0, 0, 0, 0, 0);
                }
            }
        }
    return result;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeContextMeshQuery<POINT, EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT>> node,
                                                                                                          HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                          size_t                                   numSubNodes,
                                                                                                          vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {
    bool result = false;

    return result;
    }

template<class POINT, class EXTENT> bool ScalableMeshQuadTreeContextMeshQuery<POINT,EXTENT>::Query(HFCPtr<SMPointIndexNode<POINT, EXTENT >> node,
                                                                                                          HFCPtr<SMPointIndexNode<POINT, EXTENT>> subNodes[],
                                                                                                          size_t                                  numSubNodes,
                                                                                                          ProducedNodeContainer<POINT, EXTENT>&   foundNodes)
    {
    bool result = false;

    return result;
    }
