//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.hpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <windows.h> //for showing info.
#include <ImagePP/all/h/HFCException.h>
//#include "ScalableMesh/Garland/GarlandMeshFilter.h"
#include "ScalableMesh\ScalableMeshGraph.h"
#include "CGALEdgeCollapse.h"


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
                                    size_t numSubNodes) const

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
                                    size_t numSubNodes) const

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
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const

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
        DRange3d extent = DRange3d::NullRange();
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
        extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);
            if (subNodes[indexNodes] != NULL)
            {                
                parentNode->push_back(subNodes[indexNodes]);                                    
            }
        }
        parentNode->m_nodeHeader.m_contentExtent = extent;
    }
    else
    {    
        size_t pointArrayInitialNumber[8];
        DRange3d extent = DRange3d::NullRange();
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 1 /8) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
            subNodes[indexNodes]->Pin();
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
                    extent.Extend(&points[0], (int)count);
                    /*
                    subNodes[indexNodes]->clearFrom (indexStart);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();
                    */
                }
                subNodes[indexNodes]->UnPin();
            }
            parentNode->m_nodeHeader.m_contentExtent = extent;
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

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 11/15
//=======================================================================================
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIB_CGALMeshFilter<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
    size_t numSubNodes) const
    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);
    pParentMeshNode->Pin();
    if (NULL == pParentMeshNode->GetGraphPtr()) pParentMeshNode->LoadGraph();
    MTGGraph* meshInput = nullptr;
    std::vector<DPoint3d> inputPts;
    DPoint3d extentMin, extentMax;
    extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(pParentMeshNode->m_nodeHeader.m_nodeExtent));
    extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(pParentMeshNode->m_nodeHeader.m_nodeExtent));
    Utf8String path = "E:\\output\\scmesh\\2016-05-05\\";
   //     path += (std::to_string((unsigned long long) parentNode.GetPtr())+"_").c_str();
   /* path.append("filter.log");
    std::ofstream f;
    f.open(path.c_str(), std::ios_base::app);
    f << " CREATING NODE " + std::to_string(parentNode->GetBlockID().m_integerID) + " FROM ";
    for (auto& node : subNodes)
        {
        if (node != nullptr) f << std::to_string(node->GetBlockID().m_integerID) + " ";
        }
    f << std::endl;
    f.close();*/
    bvector<bvector<DPoint3d>> polylines;
    bvector<DTMFeatureType> types;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
        if (subNodes[indexNodes] != NULL)
            {
            size_t numFaceIndexes = subNodes[indexNodes]->m_nodeHeader.m_nbFaceIndexes;

            if (numFaceIndexes > 0)
                {
                HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);
                subMeshNode->Pin();
                if (NULL == subMeshNode->GetGraphPtr()) subMeshNode->LoadGraph();
                if (meshInput == nullptr)
                    {
                    meshInput = new MTGGraph();
                    *meshInput = *subMeshNode->GetGraphPtr();
                    inputPts.resize(subMeshNode->size());
                    std::transform(subMeshNode->begin(), subMeshNode->end(), &inputPts[0], PtToPtConverter());
                    subMeshNode->ReadFeatureDefinitions(polylines, types);
                   /* Utf8String str2 = "beforeFilter_beforeMerge_sub_";
                    str2 += std::to_string(inputPts.size()).c_str();
                    PrintGraph(path, str2, subMeshNode->GetGraphPtr());
                    str2 += "_init1.g";
                    void* graphData;
                    size_t ct = meshInput->WriteToBinaryStream(graphData);
                    FILE* graphSaved = fopen((path + str2).c_str(), "wb");
                    fwrite(&ct, sizeof(size_t), 1, graphSaved);
                    fwrite(graphData, 1, ct, graphSaved);
                    size_t npts = inputPts.size();
                    fwrite(&npts, sizeof(size_t), 1, graphSaved);
                    fwrite(&inputPts[0], sizeof(DPoint3d), npts, graphSaved);
                    fclose(graphSaved);
                    delete[]graphData;*/
                    }
                else
                    {
                    std::vector<DPoint3d> pts(subMeshNode->size());
                    std::transform(subMeshNode->begin(), subMeshNode->end(), &pts[0], PtToPtConverter());
                    subMeshNode->ReadFeatureDefinitions(polylines, types);
                    std::vector<int> pointsToDestPointsMap(pts.size());
                    std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);
                    std::map<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> stitchedSet(DPoint3dZYXTolerancedSortComparison(1e-3, 0));

                    for (int i = 0; i < (int)inputPts.size(); i++)
                        {
                        stitchedSet.insert(std::make_pair(inputPts[i], i));
                        //stitchedSet.push_back(std::make_pair(stitchedPoints[i], i));
                        }

                    for (size_t i = 0; i < pts.size(); i++)
                        {
                        DPoint3d pt = pts[i];
                        if (stitchedSet.count(pt) != 0) pointsToDestPointsMap[i] = stitchedSet[pt];
                        else pointsToDestPointsMap[i] = -1;
                        }
                    bvector<int> contours;
/*                    std::ofstream fn;
                    fn.open((path +"_processidx.log").c_str(), std::ios_base::trunc);
                    for (size_t n = 0; n < pointsToDestPointsMap.size(); n++)
                        {
                        fn << " ORIGINAL " + std::to_string(n + 1) + " NEW " + std::to_string(pointsToDestPointsMap[n] + 1) << std::endl;
                        }
                    fn.close();*/
                   /* Utf8String str2 = "beforeFilter_beforeMerge_sub_";
                    str2 += std::to_string(pts.size()).c_str();
                    PrintGraph(path, str2, subMeshNode->GetGraphPtr());
                    str2 += "_init2.g";
                    void* graphData;
                    size_t ct = subMeshNode->GetGraphPtr()->WriteToBinaryStream(graphData);
                    FILE* graphSaved = fopen((path + str2).c_str(), "wb");
                    fwrite(&ct, sizeof(size_t), 1, graphSaved);
                    fwrite(graphData, 1, ct, graphSaved);
                    size_t npts = pts.size();
                    fwrite(&npts, sizeof(size_t), 1, graphSaved);
                    fwrite(&pts[0], sizeof(DPoint3d), npts, graphSaved);
                    fclose(graphSaved);*/
                    if (NULL == subMeshNode->GetGraphPtr()) subMeshNode->LoadGraph();

                    MergeGraphs(meshInput, inputPts, subMeshNode->GetGraphPtr(), pts, extentMin, extentMax, pointsToDestPointsMap, contours);
                   /* if (pParentMeshNode->GetBlockID().m_integerID == 15)
                        {
                        bvector<TaggedEdge> edges;
                        std::vector<int> temp;
                        ReadFeatureEndTags(meshInput, temp, edges);
                        }*/
                    /*Utf8String str1 = "beforeFilter_afterMerge_sub_";
                    str1 += std::to_string(pts.size()).c_str();
                    PrintGraph(path, str1, meshInput);
                    str1 += "_.g";
                    delete []graphData;
                    ct = meshInput->WriteToBinaryStream(graphData);
                    graphSaved = fopen((path + str1).c_str(), "wb");
                    fwrite(&ct, sizeof(size_t), 1, graphSaved);
                    fwrite(graphData, 1, ct, graphSaved);
                    npts = inputPts.size();
                    fwrite(&npts, sizeof(size_t), 1, graphSaved);
                    fwrite(&inputPts[0], sizeof(DPoint3d), npts, graphSaved);
                    fclose(graphSaved);*/
                    }
                subMeshNode->UnPin();
                }
            }
        }
    if (meshInput == nullptr) return false;
    ResolveUnmergedBoundaries(meshInput);

    bool ret = CGALEdgeCollapse(meshInput, inputPts, pParentMeshNode->GetBlockID().m_integerID);
    bvector<DPoint3d> vecPts;
    vecPts.assign(inputPts.begin(), inputPts.end());
    if (polylines.size() > 0)
        {
        HFCPtr<HGF2DCoordSys>   coordSysPtr(new HGF2DCoordSys());
        HFCPtr<HVEShape> allPolyShape = new HVEShape(coordSysPtr);
        bvector<DTMFeatureType> newTypes;
        bvector<bvector<DPoint3d>> newLines;

        for (auto& line : polylines)
            {
            if (!IsVoidFeature((IDTMFile::FeatureType)types[&line - &polylines.front()]))
                {
                newLines.push_back(line);
                newTypes.push_back(types[&line - &polylines.front()]);
                }
            HArrayAutoPtr<double> tempBuffer(new double[line.size() * 2]);

            int bufferInd = 0;

            for (size_t pointInd = 0; pointInd < line.size(); pointInd++)
                {
                tempBuffer[bufferInd * 2] = line[pointInd].x;
                tempBuffer[bufferInd * 2 + 1] = line[pointInd].y;
                bufferInd++;
                }
            HVE2DPolygonOfSegments polygon(line.size() * 2, tempBuffer, coordSysPtr);

            HFCPtr<HVEShape> subShapePtr = new HVEShape(polygon);
            allPolyShape->Unify(*subShapePtr);
            }
        if (allPolyShape->GetLightShape()->IsComplex())
            {
            for (auto& elem : allPolyShape->GetLightShape()->GetShapeList())
                {
                HGF2DPositionCollection thePoints;
                elem->Drop(&thePoints, elem->GetTolerance());

                bvector<DPoint3d> vec(thePoints.size());

                for (size_t idx = 0; idx < thePoints.size(); idx++)
                    {
                    vec[idx].x = thePoints[idx].GetX();
                    vec[idx].y = thePoints[idx].GetY();
                    vec[idx].z = 0; // As mentionned below the Z is disregarded
                    }

                newLines.push_back(vec);
                newTypes.push_back(DTMFeatureType::DrapeVoid);
                }
            }
        else if (!allPolyShape->GetLightShape()->IsEmpty())
            {
            HGF2DPositionCollection thePoints;
            allPolyShape->GetLightShape()->Drop(&thePoints, allPolyShape->GetLightShape()->GetTolerance());

            bvector<DPoint3d> vec(thePoints.size());

            for (size_t idx = 0; idx < thePoints.size(); idx++)
                {
                vec[idx].x = thePoints[idx].GetX();
                vec[idx].y = thePoints[idx].GetY();
                vec[idx].z = 0; // As mentionned below the Z is disregarded
                }

            newLines.push_back(vec);
            newTypes.push_back(DTMFeatureType::DrapeVoid);
            }
        polylines = newLines;
        types = newTypes;
        }

    SimplifyPolylines(polylines);

    pParentMeshNode->UnPin();

    if (ret)
        {
        volatile size_t n2 = 0;
        MTGARRAY_SET_LOOP(edgeID, meshInput)
            {
            int tag = -1;
            meshInput->TryGetLabel(edgeID, 2, tag);
            if (tag != -1) n2++;
            }
        MTGARRAY_END_SET_LOOP(edgeID, meshInput)
                
        dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode)->UpdateFromGraph(meshInput, vecPts);
        for (auto& polyline : polylines)
            {
            DRange3d extent = DRange3d::From(polyline);
            pParentMeshNode->AddFeatureDefinitionSingleNode((IDTMFile::FeatureType)types[&polyline - &polylines.front()], polyline, extent);
            }
        }
    else
        {
        std::random_shuffle(vecPts.begin(), vecPts.end());
        vecPts.resize(vecPts.size() / pParentMeshNode->GetNumberOfSubNodesOnSplit());
        //pParentMeshNode->clear();
        pParentMeshNode->push_back(&vecPts[0], vecPts.size());
        for (auto& polyline : polylines)
            {
            DRange3d extent = DRange3d::From(polyline);
            pParentMeshNode->AddFeatureDefinitionSingleNode((IDTMFile::FeatureType)types[&polyline - &polylines.front()], polyline, extent);
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
    delete meshInput;
    return true;
    }