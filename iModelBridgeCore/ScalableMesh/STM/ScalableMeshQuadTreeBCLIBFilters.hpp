//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.hpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <windows.h> //for showing info.
#include <ImagePP/all/h/HFCException.h>
#include "ScalableMesh/Garland/GarlandMeshFilter.h"
#include "ScalableMesh\ScalableMeshGraph.h"

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeFilterNonRandom<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
                parentNode->push_back(subNodes[indexNodes]);                
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeFilterNonRandom<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeFilterRandom<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
                parentNode->push_back(subNodes[indexNodes]);                
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeFilterRandom<POINT, EXTENT>::IsProgressiveFilter() const
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
                                             vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  sourceVectors,
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
    DPoint3d* pBcPt;
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
                                                                 vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
                                             size_t numSubNodes)
{
    DPoint3d* pBcPt;

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

// ScalableMeshQuadTreeBCLIBFilterViewDependent Class

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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::FilterLeaf(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> >  outputNode, 
                                    double viewDependentMetrics[]) const

    {

    // Nothing to be done    

    return false;
    }


/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilterViewDependent<POINT, EXTENT>::IsProgressiveFilter() const
{
    return false;
}

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// ScalableMeshQuadTreeBCLIBFilter1 Class

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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
                parentNode->push_back(subNodes[indexNodes]);                                    
            }
        }
    }
    else
    {    
        size_t pointArrayInitialNumber[8];
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 1 /8) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodes[indexNodes]->size() <= 10)
                {
                    // Too few content in node ... promote them all                    
                    parentNode->push_back (subNodes[indexNodes]);                                        
                }
                else
                {
                    pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();

                    // Randomize the node content
                    subNodes[indexNodes]->random_shuffle();
                                                                   
                    size_t indexStart = (subNodes[indexNodes]->size() * 7 / 8) + 1;
                    HASSERT ((indexStart > 0) && (indexStart <= (subNodes[indexNodes]->size() - 1)));

                    parentNode->push_back(subNodes[indexNodes], indexStart, subNodes[indexNodes]->size() - 1);
                    /*
                    subNodes[indexNodes]->clearFrom (indexStart);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();
                    */
                }
            }
        }
    }

    return true;
    }




//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// ScalableMeshQuadTreeBCLIBFilter2 Class

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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilter2<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
            ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
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

// ScalableMeshQuadTreeBCLIBFilter3 Class

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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilter3<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
            ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
                size_t initialNodeCount = subNodes[indexNodes]->size();
                parentNode->push_back(subNodes[indexNodes]);
                subNodes[indexNodes]->m_nodeHeader.m_totalCount -= initialNodeCount;
                subNodes[indexNodes]->clear();                
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter2<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
            ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
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
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter3<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
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
        ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
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
            ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT> alternateFilter;
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
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const

    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>,SMPointIndexNode<POINT, EXTENT>>(parentNode);
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
                parentNode->push_back(subNodes[indexNodes]);                                    
            }
        }
    }
    else
    {    
        size_t pointArrayInitialNumber[8];
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 1 /8) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodes[indexNodes]->size() <= 10)
                    {
                    // Too few content in node ... promote them all                    
                    parentNode->push_back(subNodes[indexNodes]);
                    }
                else
                    {
                    pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();

                    // Randomize the node content
                    //NEEDS_WORK_SM - Cannot random_shuffle once meshed. 
                    //subNodes[indexNodes]->random_shuffle();

                    vector<POINT> points(subNodes[indexNodes]->size());

                    subNodes[indexNodes]->get(&points[0], points.size());

                    std::random_shuffle(points.begin(), points.end());

                    size_t count = (points.size() / 8) + 1;

                    parentNode->push_back(&points[0], count);

                    /*
                    subNodes[indexNodes]->clearFrom (indexStart);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();
                    */
                }
            }
        }
        
    if (pParentMeshNode->m_nodeHeader.m_arePoints3d)
        {
        pParentMeshNode->GetMesher3d()->Mesh(pParentMeshNode);
        }
    else
        {
        pParentMeshNode->GetMesher2_5d()->Mesh(pParentMeshNode);
        }    
    }

    return true;
    }

/**----------------------------------------------------------------------------
 Filtering process based on Garland and Heckbert (1997) "Surface
 Simplification Using Quadric Error Metrics". Reduces the number of vertices 
 in collapsing certain edges that minimize the error generated compared to 
 the original surface. The targeted number of points to reach corresponds 
 here to 1/8 of the original number of points contained in the subnodes.
-----------------------------------------------------------------------------*/
//static int iterationCounter = 0;
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIB_GarlandMeshFilter<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
                                    size_t numSubNodes,
                                    double viewDependentMetrics[]) const
{

#ifdef NO_GARLAND_FILTER
    return true;
#else
    // Create a single input mesh object in order to avoid 
    // having problems with the respective indexes of the points 
    // and faces in each subnode.
    //-----------------------------------------------------------
    //iterationCounter++;
HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);
if (NULL == pParentMeshNode->GetGraphPtr()) pParentMeshNode->LoadGraph();
    ScalableMeshMeshPtr inputMesh = ScalableMeshMesh::Create();

    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {                        
            size_t numFaceIndexes = subNodes[indexNodes]->m_nodeHeader.m_nbFaceIndexes;

            if (numFaceIndexes > 0)
            {
                size_t numVertices = subNodes[indexNodes]->size(); //Returns the nb of vertices only.

                // Get the vertices and convert them from POINT to DPoint3d:
                DPoint3d* vertices = new DPoint3d[numVertices];
                ToBcPtConverter converter;

                for (size_t pointInd = 0; pointInd < subNodes[indexNodes]->size(); pointInd++)
                { 
                    vertices[pointInd] = converter.operator()(subNodes[indexNodes]->operator[](pointInd));
                }
                
                // Get the face indexes (stored after the vertices):
                int32_t* faceIndexes = (int32_t*)&subNodes[indexNodes]->operator[](subNodes[indexNodes]->size());

                inputMesh->AppendMesh(numVertices, vertices, numFaceIndexes, faceIndexes, 0, 0, 0);
                delete[] vertices;
            }   
        }
    }

    size_t numPoints = inputMesh->GetNbPoints();
    size_t numFaceIndexes = inputMesh->GetNbFaceIndexes();
    const int32_t* faceIndexes = inputMesh->GetFaceIndexes();
    const DPoint3d* points = inputMesh->GetPoints();

    if (numPoints > 0)
    {
        if (numPoints > 10) 
        {                 
            // Initialize the decimation process.
            //-----------------------------------------------------------  
            GarlandMeshFilter* simplifier = new GarlandMeshFilter();
            int targetNumberOfVertices = floor((int)numPoints / 8);
            simplifier->EdgeCollapseInit(targetNumberOfVertices);
        
            // Add the points and faces of the new mesh to an 
            // "adjacency model" on which will be applied the 
            // "edge collapse" process (simplification).
            //-----------------------------------------------------------
            for (size_t indexPoint = 0; indexPoint < numPoints; indexPoint++)
            {
                simplifier->AddPointToAdjacencyModel(points[indexPoint].x, points[indexPoint].y, points[indexPoint].z);
            }
            for (size_t index = 0; index < numFaceIndexes; index += 3)
            {
                if ((index + 2) <  numFaceIndexes)
                {
                    simplifier->AddFaceToAdjacencyModel(faceIndexes[index], faceIndexes[index + 1], faceIndexes[index + 2]);
                }
            }
            simplifier->UpdateFaceAndVerticeCount();

            // For debug.
            //simplifier->AnalyseCurrentFacesAndVertices();

            // Get rid of degenerate faces and unused vertices.
            // Initializes the count of valid vertices, faces and edges.
            //-----------------------------------------------------------
            simplifier->CleanUpAdjacencyModel(); 

            // For debug.
            /*stringstream ss; ss << "TestAfterCleanUp\\CLEANED_MODEL_" << iterationCounter << ".obj";
            simplifier->OutputModelToFile(ss.str());*/
  
            if (simplifier->GetCurrentVertexCount() > 10 && simplifier->GetCurrentVertexCount() > targetNumberOfVertices)
            {
                // Run the "edge collapse" process.
                //-----------------------------------------------------------
                simplifier->EdgeCollapseRun();

                // Return the preserved points and faces.
                //-----------------------------------------------------------
                vector<DPoint3d> resultingVerticesAsDPoint3d;
                simplifier->GetRemainingPoints(resultingVerticesAsDPoint3d);  
                size_t newNbPoints = resultingVerticesAsDPoint3d.size();
                size_t newNbFaceIndexes = 0;

                // Allows for a gap of 100 points under the target.
                assert((newNbPoints <= (size_t)targetNumberOfVertices) && ((float)newNbPoints >= (float)targetNumberOfVertices - 100));
          
                if (newNbPoints > 0)
                {
                    vector<POINT> resultingVertices;
                    POINT pt;

                    for (size_t i = 0; i < resultingVerticesAsDPoint3d.size(); i++)
                    {
                        PointOp<POINT>::SetX(pt, resultingVerticesAsDPoint3d[i].x);
                        PointOp<POINT>::SetY(pt, resultingVerticesAsDPoint3d[i].y);
                        PointOp<POINT>::SetZ(pt, resultingVerticesAsDPoint3d[i].z);
                        resultingVertices.push_back(pt);
                    }

                    vector<int32_t> resultingFaceIndexes;
                    simplifier->GetRemainingFaceIndexes(resultingFaceIndexes);
                    newNbFaceIndexes = resultingFaceIndexes.size();

                    parentNode->m_nodeHeader.m_nbFaceIndexes = resultingFaceIndexes.size();

                    // Add the remaining points and faces to the parent node.
                    //-----------------------------------------------------------
                    parentNode->push_back(&resultingVertices[0], resultingVerticesAsDPoint3d.size());

                    bvector<int> componentPointsId;
                    if (NULL == pParentMeshNode->GetGraphPtr()) pParentMeshNode->CreateGraph();
                    else *(pParentMeshNode->GetGraphPtr()) = MTGGraph();
                    CreateGraphFromIndexBuffer(pParentMeshNode->GetGraphPtr(), (const long*)&resultingFaceIndexes[0], (int)newNbFaceIndexes, (int)resultingVerticesAsDPoint3d.size(), componentPointsId, &resultingVerticesAsDPoint3d[0]);
                    pParentMeshNode->SetGraphDirty();
                    if (componentPointsId.size() > 0)
                        {
                        if (parentNode->m_nodeHeader.m_meshComponents == nullptr) parentNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                        else if (parentNode->m_nodeHeader.m_numberOfMeshComponents != componentPointsId.size())
                            {
                            delete[] parentNode->m_nodeHeader.m_meshComponents;
                            parentNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                            }
                        parentNode->m_nodeHeader.m_numberOfMeshComponents = componentPointsId.size();
                        memcpy(parentNode->m_nodeHeader.m_meshComponents, componentPointsId.data(), componentPointsId.size()*sizeof(int));
                        }
                    size_t nbPointsForFaceInd = (size_t)ceil((newNbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));// +countGraph;
                    POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];
                    memcpy(pPiggyBackMeshIndexes, &resultingFaceIndexes[0], newNbFaceIndexes * sizeof(int32_t));
                    //memcpy(pPiggyBackMeshIndexes + nbPointsForFaceInd - countGraph, serializedGraph, ct);
                    //free(serializedGraph);
                    parentNode->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);
                    parentNode->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);
                    delete[] pPiggyBackMeshIndexes;
                }

                // For debug.
                /*stringstream inputMeshName;  inputMeshName << "INPUT_MESH_" << iterationCounter << ".obj";
                stringstream reportName; reportName << "TestReport\\Report_" << iterationCounter << ".txt";
                simplifier->SaveCurrentFaceAndVertexAnalysis(inputMeshName.str(), reportName.str());

                stringstream outputName; outputName << "TestOutput\\DECIMATED_MODEL_" << iterationCounter << ".obj";
                simplifier->OutputModelToFile(outputName.str());

                stringstream inputName; inputName << "TestInput\\INPUT_MESH_" << iterationCounter << ".obj";
                simplifier->OutputMeshToFile(inputName.str(), numPoints, points, numFaceIndexes, faceIndexes);

                stringstream modelName; modelName << "DECIMATED_MODEL_" << iterationCounter << ".obj";
                stringstream fileName; fileName << "TestTermination\\TerminationReport_" << iterationCounter << ".txt";
                simplifier->SaveReasonForProcessTermination(modelName.str(), fileName.str());*/

                // For debug.
                /*stringstream msg;
                msg << "Initial nb points: " << numPoints << endl;
                msg << "Expected nb points: " << targetNumberOfVertices << endl;
                msg << "Simplified nb points: " << newNbPoints << endl;
                msg << "Percentage reduction: " << (float)newNbPoints * 100 / (float)numPoints << " %" << endl;
                msg << "\nInitial nb faces: " << numFaceIndexes / 3 << endl;
                msg << "Simplified nb faces: " << newNbFaceIndexes / 3 << endl;
                msg << "Percentage reduction: " << ((float)newNbFaceIndexes / 3) * 100 / ((float)numFaceIndexes / 3) << " %" << endl;
                MessageBoxA(NULL, msg.str().c_str(), "Information", MB_ICONINFORMATION | MB_OK);*/

                delete simplifier;
                return true;
            }
            delete simplifier;
        }
    
        // Push back in the parent node the few points that can't be simplified:
        for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                parentNode->push_back(subNodes[indexNodes]);
            }
        }

        // Push back face indexes:
        size_t nbPointsForFaceInd = (size_t)ceil((numFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));
        POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];
        memcpy(pPiggyBackMeshIndexes, &faceIndexes[0], numFaceIndexes * sizeof(int32_t));

        parentNode->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);
        parentNode->m_nodeHeader.m_nbFaceIndexes = numFaceIndexes;
        parentNode->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);

        delete[] pPiggyBackMeshIndexes;

        // For debug
        /*string m = ""; m += "Count of points that have not been simplified: "; m += to_string(numPoints);
        MessageBoxA(NULL, m.c_str(), "Information", MB_ICONINFORMATION | MB_OK);*/
    }   
    return true;
#endif
}