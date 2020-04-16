//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#ifdef WIN32
#include <windows.h> //for showing info.
#endif
#include <ImagePP/all/h/HFCException.h>

#include "ScalableMesh/ScalableMeshGraph.h"
#include "ScalableMeshMesher.h"
#include "CGALEdgeCollapse.h"
#include "LogUtils.h"

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
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(subNodes[indexNodes]->GetPointsPtr());
            totalNumberOfPoints += pointsPtr->size();
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
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
                parentPointsPtr->push_back(&(*subNodePointsPtr)[0], subNodePointsPtr->size());
                }
        }
    }
    else
    {
        size_t pointArrayInitialNumber[8];
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
        parentPointsPtr->reserve (parentPointsPtr->size() + (totalNumberOfPoints * 1 /8) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());

                if (subNodePointsPtr->size() <= 10)
                    {
                    // Too few content in node ... promote them all
                    parentPointsPtr = parentNode->GetPointsPtr();
                    parentPointsPtr->push_back(&(*subNodePointsPtr)[0], subNodePointsPtr->size());
                    }
                else
                    {
                    pointArrayInitialNumber[indexNodes] = subNodePointsPtr->size();

                    // Randomize the node content
                    subNodePointsPtr->random_shuffle();

                    size_t indexStart = (subNodePointsPtr->size() * 7 / 8) + 1;
                    HASSERT ((indexStart > 0) && (indexStart <= (subNodePointsPtr->size() - 1)));

                    parentPointsPtr = parentNode->GetPointsPtr();
                    parentPointsPtr->push_back(&(*subNodePointsPtr)[indexStart], subNodePointsPtr->size() - 1);
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
#if 0 //Currently deactivated, could be useful for handling point cloud with ScalableMesh technology.
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
#endif
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

    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
    parentPointsPtr->clear();

    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    bmap<uint64_t, bmap<DTMFeatureType, bvector<bvector<DPoint3d>>>> polylines;

    bvector<bvector<DPoint3d>> emptyNodeExtentsToMergeWithHull;
    uint64_t hullFeatureID = -1;

    // Collect points which are feature points
    std::set<DPoint3d, DPoint3dZYXTolerancedSortComparison> featurePtsToInclude(DPoint3dZYXTolerancedSortComparison(1e-6, 0));

    // Collect points to include as non-feature points
    bvector<POINT> ptsToInclude;


    // Get children linear features
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {
            if (!subNodes[indexNodes]->IsLoaded())
                subNodes[indexNodes]->Load();

            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());
            totalNumberOfPoints += subNodePointsPtr->size();

            HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);

            if(subNodePointsPtr->size() < 3)
                {
                // Add this child node extent to the hull feature
                emptyNodeExtentsToMergeWithHull.push_back(bvector<DPoint3d>());

                auto subNodeExtent = subMeshNode->m_nodeHeader.m_nodeExtent;
                emptyNodeExtentsToMergeWithHull.back().push_back(DPoint3d::From(subNodeExtent.low.x,  subNodeExtent.low.y, 0));
                emptyNodeExtentsToMergeWithHull.back().push_back(DPoint3d::From(subNodeExtent.high.x, subNodeExtent.low.y, 0));
                emptyNodeExtentsToMergeWithHull.back().push_back(DPoint3d::From(subNodeExtent.high.x, subNodeExtent.high.y, 0));
                emptyNodeExtentsToMergeWithHull.back().push_back(DPoint3d::From(subNodeExtent.low.x,  subNodeExtent.high.y, 0));
                emptyNodeExtentsToMergeWithHull.back().push_back(DPoint3d::From(subNodeExtent.low.x,  subNodeExtent.low.y, 0));

                continue;
                }
            bvector<bvector<DPoint3d>> polylinesNode;
            bvector<DTMFeatureType> typesNode;
            bvector<int32_t> ids;
            subMeshNode->ReadFeatureDefinitions(polylinesNode, typesNode, ids, false);

            for(size_t i = 0; i < polylinesNode.size(); i++)
                {
                auto id = ids[i];
                auto type = typesNode[i];
                auto const& polyline = polylinesNode[i];
                if(IsLinearFeature((ISMStore::FeatureType)type))
                    {
                    std::copy(begin(polyline), end(polyline), std::back_inserter(ptsToInclude));
                    continue;
                    }
                if(type == DTMFeatureType::TinHull || type == DTMFeatureType::Hull)
                    {
                    if(id == -1)
                        {
                        emptyNodeExtentsToMergeWithHull.push_back(polyline);
                        continue;
                        }
                    BeAssert(hullFeatureID == uint64_t(-1) || hullFeatureID == id); // Should contain only 1 hull feature
                    hullFeatureID = id;
                    }
                if(!IsClosedFeature((ISMStore::FeatureType)type) && polyline.size() <= 4 && IsClosedPolygon(polyline))
                    {
                    std::copy(begin(polyline), end(polyline), std::back_inserter(ptsToInclude));
                    continue;
                    }
                polylines[id][type].push_back(polyline);
                for(auto const& pt : polyline)
                    if(featurePtsToInclude.count(pt) == 0) featurePtsToInclude.insert(pt);
                }

        }
    }

    if(!emptyNodeExtentsToMergeWithHull.empty())
        {
        if(hullFeatureID != uint64_t(-1))
            {
            std::copy(emptyNodeExtentsToMergeWithHull.begin(), emptyNodeExtentsToMergeWithHull.end(),
                      std::back_inserter(polylines[hullFeatureID][DTMFeatureType::Hull]));
            }
        }

    size_t numberOfLinearFeaturePointsToKeep = 0;

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        DRange3d extent = DRange3d::NullRange();
        for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                if (subNodes[indexNodes]->GetNbObjects() == 0) continue;

                extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);

                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodesPointsPtr(subNodes[indexNodes]->GetPointsPtr());
                ptsToInclude.resize(subNodesPointsPtr->size());
                memcpy(&ptsToInclude[0], &(*subNodesPointsPtr)[0], ptsToInclude.size() * sizeof(POINT));

            }
        }
        if (!extent.IsNull()) parentNode->m_nodeHeader.m_contentExtent = extent;
    }
    else
    {
        // Get children points

        DRange3d extent = DRange3d::NullRange();

        parentPointsPtr->reserve(parentPointsPtr->size() + (totalNumberOfPoints * 1 / pParentMeshNode->m_nodeHeader.m_numberOfSubNodesOnSplit) + 20);

        for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                if (!subNodes[indexNodes]->IsLoaded()) subNodes[indexNodes]->Load();
                if (subNodes[indexNodes]->m_nodeHeader.m_contentExtentDefined) extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());

                if(subNodePointsPtr->size() == 0)
                    continue;

                size_t currentNbPoints = ptsToInclude.size();

                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodePointsPtr->size() <= 10)
                {
                    // Too few content in node ... promote them all
                    ptsToInclude.resize(ptsToInclude.size() + subNodePointsPtr->size());
                    memcpy(&ptsToInclude[currentNbPoints], &(*subNodePointsPtr)[0], subNodePointsPtr->size() * sizeof(POINT));
                }
                else
                {
                    subNodePointsPtr = subNodes[indexNodes]->GetPointsPtr();

                    vector<POINT> points;

                    for(size_t i = 0; i < subNodePointsPtr->size(); i++)
                        {
                        if(featurePtsToInclude.count((*subNodePointsPtr)[i]) == 0)
                            points.push_back((*subNodePointsPtr)[i]);
                        }

                    numberOfLinearFeaturePointsToKeep += ((subNodePointsPtr->size() - points.size()) / pParentMeshNode->m_nodeHeader.m_numberOfSubNodesOnSplit) + 1;
                    ptsToInclude.resize(currentNbPoints + points.size());
                    memcpy(&ptsToInclude[currentNbPoints], &points[0], points.size() * sizeof(POINT));
                }
            }
            if (!extent.IsNull())
                parentNode->m_nodeHeader.m_contentExtent = extent;
        }
    }

    // Merge and simplify children linear features
    bvector<bvector<DPoint3d>> newLines;
    bvector<DTMFeatureType> types;
    bvector<uint64_t> ids;
    if (polylines.size() > 0)
        {
        double tolerance = pParentMeshNode->m_nodeHeader.m_nodeExtent.DiagonalVector().MagnitudeSquaredXY() * 1.0e-07;
        for(auto& pair : polylines)
            {
            // Merge together polylines with the same id
            for(auto& poly : pair.second)
                {
                bvector<bvector<DPoint3d>> newFeatures;
                if(parentNode->m_nodeHeader.m_level > 0 && (poly.first == DTMFeatureType::Hull || poly.first == DTMFeatureType::TinHull || !IsClosedFeature((ISMStore::FeatureType)poly.first)))
                    {
                    CurveVectorPtr mergedHull;
                    MergeFeatures(poly.second, mergedHull, IsClosedFeature((ISMStore::FeatureType)poly.first));
                    bvector<bvector<bvector<DPoint3d>>> regions;
                    mergedHull->CollectLinearGeometry(regions);

                    bool isParityType = mergedHull->IsParityRegion();

                    for(auto &region : regions)
                        {
                        CurveVector::BoundaryType regionBoundaryType;
                        for(auto const& xyz : region)
                            {
                            ids.push_back(pair.first);
                            newLines.push_back(xyz);
                            if(isParityType)
                                {
                                if(mergedHull->GetChildBoundaryType(&xyz - &region[0], regionBoundaryType))
                                    {
                                    if(regionBoundaryType == CurveVector::BoundaryType::BOUNDARY_TYPE_Outer)
                                        types.push_back(DTMFeatureType(uint16_t(1) << 16 | (uint16_t)DTMFeatureType::Void));
                                    else if(regionBoundaryType == CurveVector::BoundaryType::BOUNDARY_TYPE_Inner)
                                        types.push_back(DTMFeatureType(uint16_t(1) << 16 | (uint16_t)DTMFeatureType::Island));
                                    else
                                        {
                                        BeAssert(!"Unsupported child boundary type...");
                                        }
                                    }
                                }
                            else
                                {
                                types.push_back(poly.first);
                                }
                            }
                        }
                    }
                else
                    {
                    std::copy(poly.second.begin(), poly.second.end(), std::back_inserter(newLines));
                    types.insert(types.end(), poly.second.size(), poly.first);
                    ids.insert(ids.end(), poly.second.size(), pair.first);
                    }
                }
            }
        if(numberOfLinearFeaturePointsToKeep > 0)
            {
#if 0
            Utf8String namePts = "E:\\Elenie\\filter_features\\";
            LOGSTRING_NODE_INFO(pParentMeshNode, namePts)
                namePts.append(".feature_pts");
            
            FILE* _meshFile = fopen(namePts.c_str(), "wb");
            size_t _nFeatures = newLines.size();
            fwrite(&_nFeatures, sizeof(size_t), 1, _meshFile);
            
            fwrite(&tolerance, sizeof(tolerance), 1, _meshFile);
            fwrite(&numberOfLinearFeaturePointsToKeep, sizeof(numberOfLinearFeaturePointsToKeep), 1, _meshFile);
            
            for(auto feature : newLines)
                {
                size_t _nVertices = feature.size();
                fwrite(&_nVertices, sizeof(size_t), 1, _meshFile);
                fwrite(&feature[0], sizeof(DPoint3d), _nVertices, _meshFile);
                }
            size_t nTypes = types.size();
            fwrite(&nTypes, sizeof(nTypes), 1, _meshFile);
            fwrite(&types[0], sizeof(types[0]), nTypes, _meshFile);
            fclose(_meshFile);
#endif
            bvector<uint32_t> idOfSurroundingFeature;
            bool mustResolveCrossingFeatures = false;
            
            if(types.end() != std::find(begin(types), end(types), DTMFeatureType::Island))
                {
                idOfSurroundingFeature = bvector<uint32_t>(newLines.size(), -1);
                for(auto const& type : types)
                    {
                    CurveVectorPtr currentCurveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
                    currentCurveVectorPtr->Add(ICurvePrimitive::CreateLineString(newLines[&type - &types[0]]));

                    if(type == DTMFeatureType::Void || type == DTMFeatureType::BreakVoid || type == DTMFeatureType::DrapeVoid)
                        {
                        // Find all islands contained in this void
                        std::for_each(begin(newLines), end(newLines), [currentCurveVectorPtr, &idOfSurroundingFeature, &mustResolveCrossingFeatures, &type, &types, &newLines] (const bvector<DPoint3d>& lineToCheck)
                                     {
                                     if(types[&lineToCheck - &newLines[0]] != DTMFeatureType::Island)
                                         return;
                                     CurveVectorPtr islandCurveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Inner);
                                     islandCurveVectorPtr->Add(ICurvePrimitive::CreateLineString(lineToCheck));
                                     auto resultCurveVectorPtr = CurveVector::AreaDifference(*currentCurveVectorPtr, *islandCurveVectorPtr);
                                     if(resultCurveVectorPtr.IsValid() && resultCurveVectorPtr->IsParityRegion())
                                         {
                                         idOfSurroundingFeature[&lineToCheck - &newLines[0]] = &type - &types[0];
                                         mustResolveCrossingFeatures = true;
                                         }
                                     });
                        }
                    else if(type == DTMFeatureType::Island)
                        {
                        // Find all voids contained in this island
                        std::for_each(begin(newLines), end(newLines), [currentCurveVectorPtr, &idOfSurroundingFeature, &mustResolveCrossingFeatures, &type, &types, &newLines] (const bvector<DPoint3d>& lineToCheck)
                                     {
                                     if(types[&lineToCheck - &newLines[0]] != DTMFeatureType::Void &&
                                        types[&lineToCheck - &newLines[0]] != DTMFeatureType::BreakVoid &&
                                        types[&lineToCheck - &newLines[0]] != DTMFeatureType::DrapeVoid)
                                         return;
                                     CurveVectorPtr voidCurveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Inner);
                                     voidCurveVectorPtr->Add(ICurvePrimitive::CreateLineString(lineToCheck));
                                     auto resultCurveVectorPtr = CurveVector::AreaDifference(*currentCurveVectorPtr, *voidCurveVectorPtr);
                                     if(resultCurveVectorPtr.IsValid() && resultCurveVectorPtr->IsParityRegion())
                                         {
                                         idOfSurroundingFeature[&lineToCheck - &newLines[0]] = &type - &types[0];
                                         mustResolveCrossingFeatures = true;
                                         }
                                     });
                        }
                    }
                }

            // Simplify polylines
            bvector<DPoint3d> removedPoints;
            if (!newLines.empty())
                {
                SimplifyPolylines(newLines, types, removedPoints, tolerance, numberOfLinearFeaturePointsToKeep);

                if(mustResolveCrossingFeatures)
                    ResolveCrossingVoidIslands(newLines, types, idOfSurroundingFeature, ids);
                }
            // Include removed points as regular mesh points (points which are not feature points)
            std::copy(begin(removedPoints), end(removedPoints), std::back_inserter(ptsToInclude));
            }
        }

    if(!ptsToInclude.empty())
        {
        std::random_shuffle(ptsToInclude.begin(), ptsToInclude.end());

        size_t subNodePointTarget = (ptsToInclude.size() / pParentMeshNode->m_nodeHeader.m_numberOfSubNodesOnSplit) + numSubNodes;
        ptsToInclude.resize(subNodePointTarget);
        }

    // Add points and features
    if (!ptsToInclude.empty()) parentPointsPtr->push_back(&(*ptsToInclude.begin()), ptsToInclude.size());

    auto isTypeSpecialHull = [] (uint32_t type) -> bool
        {
        // Check high 16 bits are set
        return ((type >> 16) > 0);
        };
    for (auto& polyline : newLines)
    {
        if (polyline.empty()) continue;
        DRange3d extent2 = DRange3d::From(polyline);
        uint32_t type = (uint32_t)types[&polyline - &newLines.front()];
        type = isTypeSpecialHull(type) ? (type & std::numeric_limits<uint16_t>::max()) : type;
        pParentMeshNode->AddFeatureDefinitionSingleNode((ISMStore::FeatureType)type, ids[&polyline - &newLines.front()], polyline, extent2);
    }

    //In multi-process flushing to disk needs to be controlled by the the workers.
    if(!m_isMultiProcessGeneration)
        {
        SMMemoryPool::GetInstance()->RemoveItem(pParentMeshNode->m_pointsPoolItemId, pParentMeshNode->GetBlockID().m_integerID, SMStoreDataType::Points, (uint64_t)pParentMeshNode->m_SMIndex);
        parentPointsPtr = 0;
        pParentMeshNode->m_pointsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        }

    BeAssert(!pParentMeshNode->m_nodeHeader.m_arePoints3d);
    if (!pParentMeshNode->m_nodeHeader.m_arePoints3d) pParentMeshNode->GetMesher2_5d()->Mesh(pParentMeshNode);

    //BeAssert(pParentMeshNode->GetPointsPtr()->size() > 10  && pParentMeshNode->GetPtsIndicePtr()->size() > 0 ||
    //         pParentMeshNode->GetPointsPtr()->size() <= 10 && pParentMeshNode->GetPtsIndicePtr()->size() == 0);

    //In multi-process flushing to disk needs to be controlled by the the workers.
    if (!m_isMultiProcessGeneration)
        {
        //Flushing tiles to disk during filtering result in less tiles being flushed sequentially at the end of the generation process, leading to potential huge performance gain.
        for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
            {
            if (subNodes[indexNodes] != NULL)
                {
                subNodes[indexNodes]->Discard();
                }
            }
        }

    return true;
}

//#ifdef WIP_MESH_IMPORT
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/16
//=======================================================================================
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIB_UserMeshFilter<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
    size_t numSubNodes) const
    {
    assert(!"not yet implemented");
    return false;

#ifdef WIP_MESH_IMPORT
    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);
    DRange3d extent = DRange3d::NullRange();
    bvector<IScalableMeshMeshPtr> subMeshes;
    bvector<Utf8String> subMetadata;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
        if (subNodes[indexNodes] != NULL)
            {
            if (subNodes[indexNodes]->m_nodeHeader.m_contentExtentDefined) extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);
            size_t numFaceIndexes = subNodes[indexNodes]->m_nodeHeader.m_nbFaceIndexes;

            if (numFaceIndexes > 0)
                {
                HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);
                if(!subMeshNode->IsLoaded())subMeshNode->Load();
                bvector<IScalableMeshMeshPtr> meshParts;
                bvector<Utf8String> meshMetadata;
                bvector<bvector<uint8_t>> texData;
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(subMeshNode->GetPointsPtr());
                if (pointsPtr->size() > 65000)
                    std::cout << " TOO MANY POINTS BEFORE FILTER :" << pointsPtr->size() << std::endl;
                subMeshNode->GetMetadata();
                subMeshNode->GetMeshParts();
                subMeshNode->GetMeshParts(meshParts, meshMetadata, texData);
                if (meshParts.size() > 0)
                    {
                    subMeshes.insert(subMeshes.end(), meshParts.begin(), meshParts.end());
                    subMetadata.insert(subMetadata.end(), meshMetadata.begin(), meshMetadata.end());
                    }
                }
            }
        }
    if (!extent.IsNull()) pParentMeshNode->m_nodeHeader.m_contentExtent = extent;
    if (m_callback != nullptr)
        {
        bool shouldCreateGraph = false;
        bvector<bvector<DPoint3d>> parentMeshPts;
        bvector<bvector<int32_t>> parentMeshIdx;
        bvector<Utf8String> parentMetadata;
        bvector<bvector<DPoint2d>> parentMeshUvs;
        bvector<bvector<uint8_t>> parentMeshTex;
        bool filterSuccess = m_callback(shouldCreateGraph, parentMeshPts, parentMeshIdx, parentMetadata,parentMeshUvs, parentMeshTex, subMeshes, subMetadata, pParentMeshNode->m_nodeHeader.m_nodeExtent);
        if (filterSuccess)
            {
            //user function needs to provide info for all mesh parts
            assert(parentMeshPts.size() == parentMeshIdx.size() && parentMeshPts.size() == parentMetadata.size());
            size_t partMeshId =0;
            for(auto& parentData: parentMetadata)
                {
                Json::Value val;
                Json::Reader reader;
                reader.parse(parentData, val);
                bvector<int> parts;
                bvector<int64_t> texId;
                bmap<int64_t,uint64_t> dgnIds;
                for (Json::Value& id : val["texId"])
                    {
                    texId.push_back(id.asInt64());
                    }
                for (Json::Value& id : val["mapIds"])
                    {
                    dgnIds[id["dgn"].asUInt64()] = id["SM"].asInt64();
                    }

                for (const Json::Value& id : val["parts"])
                    {
                    parts.push_back(id.asInt());
                    }
                if(!parentMeshTex[partMeshId].empty())
                    {
                    int width, height, nChannels;
                    memcpy(&width, &parentMeshTex[partMeshId][0],sizeof(int));
                    memcpy(&height, &parentMeshTex[partMeshId][0]+sizeof(int),sizeof(int));
                    memcpy(&nChannels, &parentMeshTex[partMeshId][0]+2*sizeof(int),sizeof(int));
                    int64_t newTexId = ((SMMeshIndex<POINT,EXTENT>*)pParentMeshNode->m_SMIndex)->AddTexture(width, height, nChannels, &parentMeshTex[partMeshId][0]+3*sizeof(int), parentMeshTex[partMeshId].size()-3*sizeof(int));
                    uint64_t dgnId = texId[0];
                    dgnIds[dgnId] = newTexId;
                    texId[0] = newTexId;
                    parentMeshTex[partMeshId].clear();
                    }
                partMeshId++;
                val["texId"] = Json::arrayValue;

                for(auto& id: texId) val["texId"].append(id);
                val["mapIds"] = Json::arrayValue;

                for(auto& mapId: dgnIds)
                    {
                    Json::Value newEntry = Json::objectValue;
                    newEntry["dgn"] = mapId.first;
                    newEntry["SM"] = mapId.second;
                    val["mapIds"].append(newEntry);
                    }

                parentData=Json::FastWriter().write(val);
                }
            pParentMeshNode->AppendMeshParts(parentMeshPts, parentMeshIdx, parentMetadata, parentMeshUvs, parentMeshTex,shouldCreateGraph);
            }
        return filterSuccess;
        }

    return true;
#endif
    }
//#endif
