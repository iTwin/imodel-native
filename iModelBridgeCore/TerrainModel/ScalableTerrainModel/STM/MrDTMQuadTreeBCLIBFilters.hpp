//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/MrDTMQuadTreeBCLIBFilters.hpp $
//:>    $RCSfile: MrDTMQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

                                                                                                                
#include <ImagePP/all/h/HFCException.h>

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeFilterNonRandom<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

{
    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {
            totalNumberOfPoints += subNodes[indexNodes]->size();
        }
    }

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    parentNode->push_back(subNodes[indexNodes]);
                }
            }
        }
    }
    else
    {

            

        POINT* pointArray[8];
        POINT* pointArrayPromoted = new POINT[totalNumberOfPoints / 4 + 10];
        size_t pointArrayPromotedNumber = 0;
        size_t pointArrayNumber[8];
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            pointArray[indexNodes] = NULL;
            if (subNodes[indexNodes] != NULL)
            {
                pointArray[indexNodes] = new POINT[subNodes[indexNodes]->size()];
                pointArrayNumber[indexNodes] = 0;
            }
        }


        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                POINT* INPointArray = new POINT[numPoints];
                subNodes[indexNodes]->get(INPointArray, numPoints);

                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    if ((indexPoint % 4) == 0)
                    {
                        pointArrayPromoted[pointArrayPromotedNumber] = INPointArray[indexPoint];
                        pointArrayPromotedNumber++;
                    }
                    else
                    {
                        pointArray[indexNodes][pointArrayNumber[indexNodes]] = INPointArray[indexPoint];
                        (pointArrayNumber[indexNodes])++;
                    }
                }
                delete [] INPointArray;
            }
        }

    	// Although doubtful, if there are already points in the parent node then we should assume 
    	// they were previously promoted somehow and should be retained.

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                subNodes[indexNodes]->clear();
                subNodes[indexNodes]->reserve(pointArrayNumber[indexNodes] + 1);
                subNodes[indexNodes]->push_back(pointArray[indexNodes], pointArrayNumber[indexNodes]);

            }
        }

        parentNode->push_back(pointArrayPromoted, pointArrayPromotedNumber);

        delete [] pointArrayPromoted;

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                delete [] pointArray[indexNodes];
            }
        }
    }

    return true;

}

/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeFilterNonRandom<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeFilterRandom<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

{
    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {
            totalNumberOfPoints += subNodes[indexNodes]->size();
        }
    }

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    parentNode->push_back(subNodes[indexNodes]);
                }
            }
        }
    }
    else
    {

    
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 3 /4) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                if (subNodes[indexNodes]->size() <= 5)
                {
                    // Too few content in node ... promote them all
                    parentNode->push_back (subNodes[indexNodes]);
                    subNodes[indexNodes]->clear();
                }
                else
                {
                    // Randomize the node content
                    subNodes[indexNodes]->random_shuffle();
                                                                   
                    size_t indexStart = (subNodes[indexNodes]->size() * 3 / 4) + 1;
                    HASSERT ((indexStart > 0) && (indexStart < (subNodes[indexNodes]->size() - 1)));

                    parentNode->push_back(subNodes[indexNodes], indexStart, subNodes[indexNodes]->size() - 1);
                    subNodes[indexNodes]->clearFrom (indexStart);
                }
            }
        }
    }

    return true;

}


/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeFilterRandom<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}





/**----------------------------------------------------------------------------
 Add the tile boundary to a bcLib DTM object as breaklines.
-----------------------------------------------------------------------------*/
template<class EXTENT> void AddExtentAsBreaklines(BC_DTM_OBJ* po_pDTMObj, EXTENT extent)
{
    HPRECONDITION(po_pDTMObj != 0);
    double ZValue = (po_pDTMObj->zMax + po_pDTMObj->zMin) / 2.0;
    DPoint3d     TileHullPts[5];   
        
    TileHullPts[0].x = ExtentOp<EXTENT>::GetXMin(extent); 
    TileHullPts[0].y = ExtentOp<EXTENT>::GetYMin(extent); 
    TileHullPts[0].z = ZValue;
    TileHullPts[1].x = ExtentOp<EXTENT>::GetXMax(extent); 
    TileHullPts[1].y = ExtentOp<EXTENT>::GetYMin(extent); 
    TileHullPts[1].z = ZValue;
    TileHullPts[2].x = ExtentOp<EXTENT>::GetXMax(extent); 
    TileHullPts[2].y = ExtentOp<EXTENT>::GetYMax(extent); 
    TileHullPts[2].z = ZValue;
    TileHullPts[3].x = ExtentOp<EXTENT>::GetXMin(extent); 
    TileHullPts[3].y = ExtentOp<EXTENT>::GetYMax(extent); 
    TileHullPts[3].z = ZValue;
    TileHullPts[4].x = ExtentOp<EXTENT>::GetXMin(extent); 
    TileHullPts[4].y = ExtentOp<EXTENT>::GetYMin(extent); 
    TileHullPts[4].z = ZValue;
    
    //Add the tile boundaries to the DTM before computing the metrics.
    int status = bcdtmObject_storeDtmFeatureInDtmObject(po_pDTMObj, DTMF_HARD_BREAK, 0, 1, &po_pDTMObj->nullFeatureId, TileHullPts, 5);                                        

    assert(status == 0);
}


template<class POINT, class EXTENT> void BuildCombinedDTM (BC_DTM_OBJ* pDtmObject, 
                                             HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  sourceVectors[],
                                             size_t numSubNodes)
{
    int         status;


    // Copyu into DTM structure
    for (size_t nodeInd = 0; nodeInd < numSubNodes; nodeInd++)
    {                  
        if (sourceVectors[nodeInd] != NULL)
        {
            if (sourceVectors[nodeInd]->size() > 0)
            {
                // Allocate required buffer
                DPoint3d* pPt = new DPoint3d[sourceVectors[nodeInd]->size()];

                for (size_t ptInd = 0; ptInd < sourceVectors[nodeInd]->size(); ptInd++)
                {                                                                           
                    pPt[ptInd].x = PointOp<POINT>::GetX(sourceVectors[nodeInd]->operator[](ptInd));
                    pPt[ptInd].y = PointOp<POINT>::GetY(sourceVectors[nodeInd]->operator[](ptInd));
                    pPt[ptInd].z = PointOp<POINT>::GetZ(sourceVectors[nodeInd]->operator[](ptInd));
                }

                if (bcdtmObject_storeDtmFeatureInDtmObject(pDtmObject, DTMFeatureType::RandomSpots, pDtmObject->nullUserTag, 1, &pDtmObject->nullFeatureId, pPt, (long)sourceVectors[nodeInd]->size())) 
                {
                    //Problem with the insertion of a point.
                    status = 1;
                    break;
                }

                delete [] pPt;
            }
        }
    }

}


template<class POINT> void InsertDTMIntoVector (BC_DTM_OBJ* pDtm, HPMPooledVector<POINT>* outputVector)
{
    DTM_TIN_POINT* pBcPt;
    POINT          pt;                 
    
    //Add the filtered points to the parent node
    outputVector->reserve(pDtm->numPoints);

    for (int ptInd = 0; ptInd < pDtm->numPoints; ++ptInd)
        {                
        pBcPt = pointAddrP(pDtm, ptInd);                    
        
        PointOp<POINT>::SetX(pt, pBcPt->x);
        PointOp<POINT>::SetY(pt, pBcPt->y);
        PointOp<POINT>::SetZ(pt, pBcPt->z);

        outputVector->push_back(pt);                
        }
}

template <class POINT, class EXTENT> void SpreadDTMIntoSubNodes (BC_DTM_OBJ* pDtmObject, 
                                             HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                             size_t numSubNodes)
{
    DTM_TIN_POINT* pBcPt;

    // Clear subnodes and reserve points

    for (size_t nodeIndex = 0; nodeIndex < numSubNodes; nodeIndex++)
    {
        size_t nodeSize;            
        if (subNodes[nodeIndex] != NULL)
        {
            nodeSize = subNodes[nodeIndex]->size();            
            subNodes[nodeIndex]->clear();
            subNodes[nodeIndex]->m_nodeHeader.m_totalCount -= nodeSize;
            subNodes[nodeIndex]->reserve (1 + nodeSize *3 / 4 );           
        }
    }

    int ptInd;
    for (ptInd = 0; ptInd < pDtmObject->numPoints; ptInd += 1)
    {                
        pBcPt = pointAddrP(pDtmObject, ptInd);                    

        POINT tyty = PointOp<POINT>::Create(pBcPt->x, pBcPt->y, pBcPt->z);

        for (size_t nodeIndex = 0; nodeIndex < numSubNodes ; nodeIndex++)
        {
            if (subNodes[nodeIndex] != NULL)
            {
                if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(subNodes[nodeIndex]->GetNodeExtent(), tyty))
                {
                    subNodes[nodeIndex]->push_back(tyty);
                    subNodes[nodeIndex]->m_nodeHeader.m_totalCount++;
                    break;
                }
            }
        }
            
    }
}


template<class EXTENT> void ComputeViewDependentMetrics (BC_DTM_OBJ* pDtmObject, 
                                                         EXTENT nodeExtent,
                                                         double viewDependentMetrics[])
{/*
    int         status;
    for (int ViewInd = 0; ViewInd < NB_SAMPLE_CAMERA_ORIENTATIONS; ViewInd++)
        {
        viewDependentMetrics[ViewInd] = DEFAULT_VIEW_DEPENDENT_METRIC;
        }                

    if (pDtmObject->numPoints > 20)
        {

        if (((pDtmObject->xMax - pDtmObject->xMin) != 0) &&
            ((pDtmObject->yMax - pDtmObject->yMin) != 0))
            {
            AddExtentAsBreaklines(pDtmObject, nodeExtent);

            status = bcdtmMultiResolution_getMinNativeToScreenFactorM03(pDtmObject, viewDependentMetrics);          
            }

        }*/
}


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// MrDTMQuadTreeBCLIBFilterViewDependent Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::FilterLeaf(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  outputNode, 
                                    double viewDependentMetrics[]) const

    {

    // Nothing to be done    

    return false;
    }


/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::IsProgressiveFilter() const
{
    return false;
}

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// MrDTMQuadTreeBCLIBFilter1 Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================


//If a metric cannot be computed, set the metric to a very large number. This 
//will ensure that points in such leaf node are used (i.e. : always greater than the 
//minimum metric threshold) and that points in such parent node are not used. 
//Note that HDOUBLE_MAX is not used to avoid overflow when computing the metric depending 
//on the current root to view transformation matrix.
#ifndef DEFAULT_VIEW_DEPENDENT_METRIC 
    #define DEFAULT_VIEW_DEPENDENT_METRIC HFLOAT_MAX
#endif

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {

                                        
    int         status;
    BC_DTM_OBJ* pDtmObject = 0;
    //DPoint3d         pt; 
          
    status = bcdtmObject_createDtmObject(&pDtmObject);   

    assert(status == 0);

    BuildCombinedDTM (pDtmObject, subNodes, numSubNodes);

    BC_DTM_OBJ* pFilteredDtm = 0;

// bcdtmWrite_message(0,0,0,"Filtering A01 Tile") ;

    if ((pDtmObject->numPoints >= 1) && (pDtmObject->numPoints <= 4))
    {
        // Very few points ... this would cause the filtering to crash (at least for 1 point) ... we consider these points to have a great spatial value and
        // send them all to the parent node
        InsertDTMIntoVector (pDtmObject, &*parentNode);

        FilterLeaf (parentNode, viewDependentMetrics);

    }
    // If there are any point ...
    else if (pDtmObject->numPoints > 0)
        {
        //Filtering
        if (status == 0)
            {
           //DPoint3d*   pPt;
            DTM_TIN_POINT* pBcPt;
            DPoint3d    pt; 
            //POINT  bcPt;
            //long   numPointsRemove = pDtmObject->numPoints * 3/4;             
            //long   numFilteredPoints;            
            
            status = bcdtmObject_createDtmObject(&pFilteredDtm);               

            //Private  int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;

            //long filterOption = 1;                   /* ==> < 1-Least Squares Plane , 2 - Average >                      */
            //long boundaryOption = 1;                 /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */            

            for (int ptInd = 0; ptInd < pDtmObject->numPoints; ptInd += 4)
                {                
                pBcPt = pointAddrP(pDtmObject, ptInd);                    
                
                pt.x = pBcPt->x;
                pt.y = pBcPt->y;
                pt.z = pBcPt->z;
                
                if (bcdtmObject_storeDtmFeatureInDtmObject(pFilteredDtm, DTMFeatureType::RandomSpots, pFilteredDtm->nullUserTag, 1, &pFilteredDtm->nullFeatureId, &pt, 1)) 
                    {
                    //Problem with the insertion of a point.
                    status = 1;
                    break;
                    }                                                       
                }


            }

        //Inserting the resulting filtering points in the parent node

        InsertDTMIntoVector (pFilteredDtm, &*parentNode);
        
        ComputeViewDependentMetrics (pFilteredDtm, parentNode->GetNodeExtent(), viewDependentMetrics);


        if (pFilteredDtm != 0)
            {
            status = bcdtmObject_destroyDtmObject(&pFilteredDtm);
            assert(status == 0);
            }


        }
     else
        {
        // There are in fact no point in leafes ... current should be a leaf
        FilterLeaf (parentNode, viewDependentMetrics);
        }

    if (pDtmObject != 0)
        {
        status = bcdtmObject_destroyDtmObject(&pDtmObject);
        assert(status == 0);
        }
    return true;
    }




//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// MrDTMQuadTreeBCLIBFilter2 Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBFilter2<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {

                                        
    int         status;
    BC_DTM_OBJ* pDtmObject = 0;
    //DPoint3d         pt; 
          
    status = bcdtmObject_createDtmObject(&pDtmObject);   

    assert(status == 0);

    BuildCombinedDTM (pDtmObject, subNodes, numSubNodes);

    BC_DTM_OBJ* pFilteredDtm = 0;

    if ((pDtmObject->numPoints >= 1) && (pDtmObject->numPoints <= 4))
    {
        // Very few points ... this would cause the filtering to crash (at least for 1 point) ... we consider these points to have a great spatial value and
        // send them all to the parent node
        InsertDTMIntoVector (pDtmObject, &*parentNode);

        FilterLeaf (parentNode, viewDependentMetrics);

    }
    // If there are any point ...
    else if (pDtmObject->numPoints > 0)
        {
        //Filtering
        if (status == 0)
            {
           //DPoint3d*   pPt;
            //DTM_TIN_POINT* pBcPt;
            //DPoint3d    pt; 
            //POINT  bcPt;
            long   numPointsRemove = pDtmObject->numPoints * 3/4;             
            long   numFilteredPoints;            
            
            status = bcdtmObject_createDtmObject(&pFilteredDtm);               

            //Private  int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;

            //long filterOption = 1;                   /* ==> < 1-Least Squares Plane , 2 - Average >                      */
            //long boundaryOption = 1;                 /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */            



            status = bcdtmMultiResolution_tileDecimateRandomSpotsDtmObject(pDtmObject,            
                                                                          numPointsRemove,
                                                                          &numFilteredPoints,
                                                                          pFilteredDtm);             


            assert(status == 0);

            }

        //Inserting the resulting filtering points in the parent node
        if (status == 0)
            {            
            bcdtmMath_setBoundingCubeDtmObject(pDtmObject) ;
            InsertDTMIntoVector (pDtmObject, &*parentNode);
        
            ComputeViewDependentMetrics (pDtmObject, parentNode->GetNodeExtent(), viewDependentMetrics);
            }
        else
            {
            // Something went wrong ... use alternate filtering method 
            MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
            alternateFilter.Filter (parentNode, subNodes, numSubNodes, viewDependentMetrics);  
            }

        if (pFilteredDtm != 0)
            {
            status = bcdtmObject_destroyDtmObject(&pFilteredDtm);
            assert(status == 0);
            }


        }
     else
        {
        // There are in fact no point in leafes ... current should be a leaf
        FilterLeaf (parentNode, viewDependentMetrics);
        }
    if (pDtmObject != 0)
        {
        status = bcdtmObject_destroyDtmObject(&pDtmObject);
        assert(status == 0);
        }
    return true;
    }




//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// MrDTMQuadTreeBCLIBFilter3 Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBFilter3<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {

                                        
    int         status;
    BC_DTM_OBJ* pDtmObject = 0;
    //DPoint3d         pt; 
          
    status = bcdtmObject_createDtmObject(&pDtmObject);   

    assert(status == 0);

    BuildCombinedDTM (pDtmObject, subNodes, numSubNodes);

    BC_DTM_OBJ* pFilteredDtm = 0;

    if ((pDtmObject->numPoints >= 1) && (pDtmObject->numPoints <= 4))
    {
        // Very few points ... this would cause the filtering to crash (at least for 1 point) ... we consider these points to have a great spatial value and
        // send them all to the parent node
        InsertDTMIntoVector (pDtmObject, &*parentNode);
        FilterLeaf (parentNode, viewDependentMetrics);

    }
    // If there are any point ...
    else if (pDtmObject->numPoints > 0)
        {
        //Filtering
        if (status == 0)
            {
           //DPoint3d*   pPt;
            //DTM_TIN_POINT* pBcPt;
            //DPoint3d    pt; 
            //POINT  bcPt;
            long   numPointsRemove = pDtmObject->numPoints * 3/4;             
            long   numFilteredPoints;            
            
            status = bcdtmObject_createDtmObject(&pFilteredDtm);               

            //Private  int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;

            long filterOption = 1;                   /* ==> < 1-Least Squares Plane , 2 - Average >                      */
            long boundaryOption = 1;                 /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */            




            status = bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(pDtmObject,
                                                                          filterOption,
                                                                          boundaryOption,
                                                                          numPointsRemove,
                                                                          &numFilteredPoints,
                                                                          pFilteredDtm);             
                                                                           
            }

        //Inserting the resulting filtering points in the parent node
        if (status == 0)
            {            
            InsertDTMIntoVector (pDtmObject, &*parentNode);
        
            ComputeViewDependentMetrics (pDtmObject, parentNode->GetNodeExtent(), viewDependentMetrics);
            }
        else
            {
            // Something went wrong ... use alternate filtering method 
            MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
            alternateFilter.Filter (parentNode, subNodes, numSubNodes, viewDependentMetrics);  
            }

        if (pFilteredDtm != 0)
            {
            status = bcdtmObject_destroyDtmObject(&pFilteredDtm);
            assert(status == 0);
            }


        }
     else
        {
        // There are in fact no point in leafes ... current should be a leaf
        FilterLeaf (parentNode, viewDependentMetrics);
        }

    if (pDtmObject != 0)
        {
        status = bcdtmObject_destroyDtmObject(&pDtmObject);
        assert(status == 0);
        }
    return true;
    }



/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

{
    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {
            totalNumberOfPoints += subNodes[indexNodes]->size();
        }
    }

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    size_t initialNodeCount = subNodes[indexNodes]->size();
                    parentNode->push_back(subNodes[indexNodes]);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= initialNodeCount;
                    subNodes[indexNodes]->clear();
                }
            }
        }
    }
    else
    {

    
#if (1)
        size_t pointArrayInitialNumber[8];
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 3 /4) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodes[indexNodes]->size() <= 10)
                {
                    // Too few content in node ... promote them all
                    size_t initialNodeCount = subNodes[indexNodes]->size();
                    parentNode->push_back (subNodes[indexNodes]);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= initialNodeCount;
                    subNodes[indexNodes]->clear();
                }
                else
                {
                    pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();

                    // Randomize the node content
                    subNodes[indexNodes]->random_shuffle();
                                                                   
                    size_t indexStart = (subNodes[indexNodes]->size() * 3 / 4) + 1;
                    HASSERT ((indexStart > 0) && (indexStart <= (subNodes[indexNodes]->size() - 1)));

                    parentNode->push_back(subNodes[indexNodes], indexStart, subNodes[indexNodes]->size() - 1);
                    subNodes[indexNodes]->clearFrom (indexStart);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();
                }
            }
        }

#else

        POINT* pointArray[8];
        POINT* pointArrayPromoted = new POINT[totalNumberOfPoints / 4 + 10];
        size_t pointArrayPromotedNumber = 0;
        size_t pointArrayNumber[8];
        size_t pointArrayInitialNumber[8];

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            pointArray[indexNodes] = NULL;
            if (subNodes[indexNodes] != NULL)
            {
                pointArray[indexNodes] = new POINT[subNodes[indexNodes]->size()];
                pointArrayNumber[indexNodes] = 0;
                pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();
            }
        }


        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                POINT* INPointArray = new POINT[numPoints];
                subNodes[indexNodes]->get(INPointArray, numPoints);

                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    if ((indexPoint % 4) == 0)
                    {
                        pointArrayPromoted[pointArrayPromotedNumber] = INPointArray[indexPoint];
                        pointArrayPromotedNumber++;
                    }
                    else
                    {
                        pointArray[indexNodes][pointArrayNumber[indexNodes]] = INPointArray[indexPoint];
                        (pointArrayNumber[indexNodes])++;
                    }
                }
                delete [] INPointArray;
            }
        }

    	// Although doubtful, if there are already points in the parent node then we should assume 
    	// they were previously promoted somehow and should be retained.

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                subNodes[indexNodes]->clear();
                subNodes[indexNodes]->reserve(pointArrayNumber[indexNodes] + 1);
                subNodes[indexNodes]->push_back(pointArray[indexNodes], pointArrayNumber[indexNodes]);
                subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();

            }
        }

        parentNode->push_back(pointArrayPromoted, pointArrayPromotedNumber);
        

        delete [] pointArrayPromoted;

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                delete [] pointArray[indexNodes];
            }
        }
#endif
    }

    return true;

    }




/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {

                                        
    int         status;
    BC_DTM_OBJ* pDtmObject = 0;
    //DPoint3d         pt; 
          
    status = bcdtmObject_createDtmObject(&pDtmObject);   

    assert(status == 0);

    BuildCombinedDTM (pDtmObject, subNodes, numSubNodes);

    BC_DTM_OBJ* pFilteredDtm = 0;

    if ((pDtmObject->numPoints >= 1) && (pDtmObject->numPoints <= 30))
    {
        // Very few points ... this would cause the filtering to crash (at least for 1 point) ... we consider these points to have a great spatial value and
        // send them all to the parent node
        InsertDTMIntoVector (pDtmObject, &*parentNode);


        for (size_t nodeIndex = 0; nodeIndex < numSubNodes; nodeIndex++)
        {
            if (subNodes[nodeIndex] != NULL)
            {
                subNodes[nodeIndex]->m_nodeHeader.m_totalCount -= subNodes[nodeIndex]->size();
                subNodes[nodeIndex]->clear();
            }
        }

        FilterLeaf (parentNode, viewDependentMetrics);

    }
    // If there are any point ...
    else if (pDtmObject->numPoints > 0)
    {
        long totalNumPoints = pDtmObject->numPoints;
        long   numFilteredPoints;            
        long   numPointsRemove = pDtmObject->numPoints * 3/4;   
        //Filtering
        if (status == 0)
            {
           //DPoint3d*   pPt;
            //DTM_TIN_POINT* pBcPt;
            //DPoint3d    pt; 
            //POINT  bcPt;
          
            
            status = bcdtmObject_createDtmObject(&pFilteredDtm);               

            //Private  int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;

            //long filterOption = 1;                   /* ==> < 1-Least Squares Plane , 2 - Average >                      */
            //long boundaryOption = 1;                 /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */            



            status = bcdtmMultiResolution_tileDecimateRandomSpotsDtmObject(pDtmObject,            
                                                                          numPointsRemove,
                                                                          &numFilteredPoints,
                                                                          pFilteredDtm);             



            }

        //Inserting the resulting filtering points in the parent node
        // We tolerate that the algortihm cannot remove as many points as desired up to a
        // limit . If this minimal limit is not attaiend we use an alternate algorithm.
        if ((status == 0) && ((pDtmObject->numPoints + pFilteredDtm->numPoints == totalNumPoints) && 
                              (numFilteredPoints >= (0.75 * numPointsRemove)))) 
            {            
            bcdtmMath_setBoundingCubeDtmObject(pDtmObject) ;
            InsertDTMIntoVector (pDtmObject, &*parentNode);
            SpreadDTMIntoSubNodes (pFilteredDtm, subNodes, numSubNodes);

        
            ComputeViewDependentMetrics (pDtmObject, parentNode->GetNodeExtent(), viewDependentMetrics);
            }
        else
            {
            // Something went wrong ... use alternate filtering method 
            MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
            alternateFilter.Filter (parentNode, subNodes, numSubNodes, viewDependentMetrics);  
            }

        if (pFilteredDtm != 0)
            {
            status = bcdtmObject_destroyDtmObject(&pFilteredDtm);
            assert(status == 0);
            }


        }
    else
        {
        // There are in fact no point in leafes ... current should be a leaf
        FilterLeaf (parentNode, viewDependentMetrics);
        }
    if (pDtmObject != 0)
        {
        status = bcdtmObject_destroyDtmObject(&pDtmObject);
        assert(status == 0);
       }
    return true;
    }



/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool MrDTMQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>::Filter(
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parentNode, 
                                    HFCPtr<HGFPointIndexNode<POINT, EXTENT> >  subNodes[],
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {

                                        
    int         status;
    BC_DTM_OBJ* pDtmObject = 0;
    //DPoint3d         pt; 
          
    status = bcdtmObject_createDtmObject(&pDtmObject);   

    assert(status == 0);

    BuildCombinedDTM (pDtmObject, subNodes, numSubNodes);

    BC_DTM_OBJ* pFilteredDtm = 0;

    if ((pDtmObject->numPoints >= 1) && (pDtmObject->numPoints <= 35))
    {
        // Very few points ... this would cause the filtering to crash (at least for 1 point) ... we consider these points to have a great spatial value and
        // send them all to the parent node
        InsertDTMIntoVector (pDtmObject, &*parentNode);


        for (size_t nodeIndex = 0; nodeIndex < numSubNodes; nodeIndex++)
        {
            if (subNodes[nodeIndex] != NULL)
                subNodes[nodeIndex]->clear();
        }

        FilterLeaf (parentNode, viewDependentMetrics);

    }
#if (0)
    else if ((pDtmObject->numPoints >= 35) && (pDtmObject->numPoints <= 1000))
    {
        MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
        alternateFilter.Filter (parentNode, firstSubNode, secondSubNode, thirdSubNode, fourthSubNode, viewDependentMetrics);
    }
#endif
    // If there are any point ...
    else if (pDtmObject->numPoints > 0)
        {
        long totalNumPoints = pDtmObject->numPoints;
        //Filtering
        if (status == 0)
            {
           //DPoint3d*   pPt;
            //DTM_TIN_POINT* pBcPt;
            //DPoint3d    pt; 
            //POINT  bcPt;
            long   numPointsRemove = pDtmObject->numPoints * 3/4;             
            long   numFilteredPoints;            
            status = bcdtmObject_createDtmObject(&pFilteredDtm);               

            //Private  int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(BC_DTM_OBJ *dtmP,long filterOption,long boundaryOption,long numPointsRemove,long *numFilteredSpotsP,BC_DTM_OBJ *filteredPtsP ) ;

            long filterOption = 2;                   /* ==> < 1-Least Squares Plane , 2 - Average >                      */
            long boundaryOption = 2;                 /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */            




            status = bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(pDtmObject,
                                                                          filterOption,
                                                                          boundaryOption,
                                                                          numPointsRemove,
                                                                          &numFilteredPoints,
                                                                          pFilteredDtm);             
                                                                           
            }
        // HASSERT (pDtmObject->numPoints + pFilteredDtm->numPoints == totalNumPoints);

        //Inserting the resulting filtering points in the parent node
        if ((status == 0) && (pDtmObject->numPoints + pFilteredDtm->numPoints == totalNumPoints)) 
            {        

            InsertDTMIntoVector (pDtmObject, &*parentNode);
            SpreadDTMIntoSubNodes (pFilteredDtm, subNodes, numSubNodes);
        
            ComputeViewDependentMetrics (pDtmObject, parentNode->GetNodeExtent(), viewDependentMetrics);
            }
        else
            {
            // Something went wrong ... use alternate filtering method 
            MrDTMQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
            alternateFilter.Filter (parentNode, subNodes, numSubNodes, viewDependentMetrics);  
            }

        if (pFilteredDtm != 0)
            {
            status = bcdtmObject_destroyDtmObject(&pFilteredDtm);
            assert(status == 0);
            }


        }
    else
        {
        // There are in fact no point in leafes ... current should be a leaf
        FilterLeaf (parentNode, viewDependentMetrics);
        }
    if (pDtmObject != 0)
        {
        status = bcdtmObject_destroyDtmObject(&pDtmObject);
        assert(status == 0);
        }
    return true;
    }

