//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

extern bool s_useSpecialTriangulationOnGrids;

//#define SM_EXPORT_DTM_DEBUG

#ifdef SM_EXPORT_DTM_DEBUG
#define EXPORT_DTM_3(node, dtm, name, status) \
{                                                                                                           \
WString dtmFileName(LOG_PATH_STR_W);                                                                        \
dtmFileName.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());                                \
dtmFileName.append(L"_");                                                                                   \
dtmFileName.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());                                    \
dtmFileName.append(name);                                                                 \
dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());    \
dtmFileName.append(L"_");                                                                                   \
dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());    \
dtmFileName.append(L".tin");                                                                                \
status = bcdtmWrite_toFileDtmObject(dtm, dtmFileName.c_str());                                                   \
}

#define EXPORT_DTM(node, name, status) \
EXPORT_DTM_3(node, dtmObjP, name, status)
#else
#define EXPORT_DTM_3(node, dtmObjP, name, status)
#define EXPORT_DTM(node, name,status)
#endif

template<class POINT, class EXTENT> 
void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::FilterFeaturePoints(bvector<DPoint3d>& filteredPoints, bvector<DPoint3d>& points, bvector<bvector<int32_t>>& featureDefs) const
    {
    map<DPoint3d, DTMFeatureType, DPoint3dZYXTolerancedSortComparison> featuresMap(DPoint3dZYXTolerancedSortComparison(1e-5, 0));

    for(size_t i = 0; i < featureDefs.size(); ++i)
        {
        DTMFeatureType type = (DTMFeatureType)featureDefs[i][1];
        if(type == DTMFeatureType::ContourLine)
            continue; // Contour lines should also be considered as point features
        for(size_t j = 2; j < featureDefs[i].size(); ++j)
            {
            if(featureDefs[i][j] < points.size()) featuresMap.insert(std::make_pair(points[featureDefs[i][j]], type));
            }
        }

    // Filter feature points
    if(featuresMap.empty())
        filteredPoints = points;
    else
        {
        for(size_t i = 0; i < points.size(); ++i)
            {
            if(featuresMap.count(points[i]) == 0) filteredPoints.push_back(points[i]);
            }
        if(filteredPoints.empty()) filteredPoints = points;
        }
    }

template<class POINT, class EXTENT> 
bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::ExtractDataForMeshing(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, bvector<DPoint3d>& points, bvector<bvector<int32_t>>& featureDefs, CurveVectorPtr& nonHullFeatures, bvector< std::pair<DTMFeatureType,DTMFeatureId>>& nonHullFeatureInfo, CurveVectorPtr& hullFeatures, int& hullID, bvector<bvector<int>>& idsOfPrunedVoidIslandFeatures) const
    {
    // Fetch node points
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

    if(pointsPtr->size() == 0)
        return false; // Nothing to mesh!

    bvector<DPoint3d> points2d; // Used to test for colinearity of the points
    for(size_t i = 0; i < pointsPtr->size(); ++i)
        {
        if((*pointsPtr)[i].x < 1e15 && (*pointsPtr)[i].y < 1e15 && !std::isnan((*pointsPtr)[i].y) && !std::isnan((*pointsPtr)[i].x))
            {
            points.push_back((*pointsPtr)[i]);
            if(fabs(points.back().x) < 1e-8) points.back().x = 0;
            if(fabs(points.back().y) < 1e-8) points.back().y = 0;
            points2d.push_back(points.back());
            points2d.back().z = 0;
            }
        }

    // Fetch node linear features
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();

    bvector<DTMFeatureType> types;
    bvector<bvector<DPoint3d>> voidFeatures;
    bvector<bvector<DPoint3d>> islandFeatures;
    bvector<bvector<DTMFeatureId>> idsOfVoidIslandFeatures(2);
    CurveVectorPtr hullFeaturesToInvert;

    if(linearFeaturesPtr->size() > 0)
        {
        if(!nonHullFeatures.IsValid()) nonHullFeatures = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
        node->GetFeatureDefinitions(featureDefs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
        }

    for(size_t i = 0; i < featureDefs.size(); ++i)
        {
        bvector<DPoint3d> feature;
        for(size_t j = 2; j < featureDefs[i].size(); ++j)
            {
            if(featureDefs[i][j] < points.size()) feature.push_back(points[featureDefs[i][j]]);
            }
        if(feature.empty()) continue;

        auto linearPrimitivePtr = ICurvePrimitive::CreateLineString(feature);

        DTMFeatureId id = (DTMFeatureId)featureDefs[i][0];
        DTMFeatureType type = (DTMFeatureType)featureDefs[i][1];

        if(type == DTMFeatureType::Void || type == DTMFeatureType::Hole
           || type == DTMFeatureType::BreakVoid || type == DTMFeatureType::DrapeVoid)
            {
            if(!feature.back().AlmostEqual(feature.front())) feature.push_back(feature.front());
            voidFeatures.push_back(feature);
            idsOfVoidIslandFeatures[0].push_back(id);
            types.push_back(type);
            }
        else if(type == DTMFeatureType::Island)
            {
            if(!feature.back().AlmostEqual(feature.front())) feature.push_back(feature.front());
            islandFeatures.push_back(feature);
            idsOfVoidIslandFeatures[1].push_back(id);
            }
        else
            {
            if(IsClosedFeature((ISMStore::FeatureType)type) && DVec3d::FromStartEnd(feature.front(), feature.back()).Magnitude() > 0)
                feature.push_back(feature.front());

            // "Drape" type features are tricky on sub resolutions because the underlying terrain may no longer be there.
            // For now, only apply the "drape" modifier on the best resolution (the "true" one)
            if(type == DTMFeatureType::DrapeVoid && !node->m_nodeHeader.m_IsLeaf)
                {
                type = DTMFeatureType::BreakVoid;
                }

            if(type == DTMFeatureType::Hull || type == DTMFeatureType::TinHull)
                {
                hullID = id;
                if(!hullFeatures.IsValid()) hullFeatures = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
                hullFeatures->Add(linearPrimitivePtr);
                }
            else
                {
                nonHullFeatures->Add(linearPrimitivePtr);
                nonHullFeatureInfo.push_back(std::make_pair(type, id));
                }
            }
        }

    // Prune exactly overlapping voids and islands
    PruneFeatures(idsOfPrunedVoidIslandFeatures, islandFeatures, voidFeatures);
    
    for(int i = 0; i < idsOfPrunedVoidIslandFeatures[0].size(); ++i)
        {
        auto voidIndex = idsOfPrunedVoidIslandFeatures[0][i];
        auto linearPrimitivePtr = ICurvePrimitive::CreateLineString(voidFeatures[voidIndex]);
        nonHullFeatures->Add(linearPrimitivePtr);
        nonHullFeatureInfo.push_back(std::make_pair(types[voidIndex], idsOfVoidIslandFeatures[0][voidIndex]));
        }

    for(int i = 0; i < idsOfPrunedVoidIslandFeatures[1].size(); ++i)
        {
        auto islandIndex = idsOfPrunedVoidIslandFeatures[1][i];
        auto linearPrimitivePtr = ICurvePrimitive::CreateLineString(islandFeatures[islandIndex]);
        nonHullFeatures->Add(linearPrimitivePtr);
        nonHullFeatureInfo.push_back(std::make_pair(DTMFeatureType::Island, idsOfVoidIslandFeatures[1][islandIndex]));
        }

    // bclib doesn't like colinear points, even when they're not exactly colinear, but just fall on the same 2d line.
    bool isColinear = (bsiGeom_isUnorderedDPoint3dArrayColinear(&points2d.front(), (int)points2d.size(), 1e-5)); //We use the same tolerance as bclib here, even though normally we use 1e-8 otherwise
    if(isColinear)
        {
        // We can't triangulate tiles where all points are colinear. We'll attempt to stitch them w/o meshing first
        return false;
        }

    return true;
    }

template<class POINT, class EXTENT> 
bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::UpdateNodeWithNewMesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, IScalableMeshMeshPtr meshPtr, bvector<bvector<POINT>>& newFeaturePoints, bvector<DTMFeatureType>& newFeatureTypes, bvector<DTMFeatureId>& newFeatureIds, CurveVectorPtr invertedHulls, int hullFeatureID) const
    {
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> pointIds(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    bvector<POINT> newPoints;

    //BeAssert(meshPtr.IsValid() && meshPtr.get() != nullptr); // Should contain at least the original points

    ScalableMeshMesh* meshP = (ScalableMeshMesh*)meshPtr.get();
    if(meshP != 0)
        {
        newPoints.resize(meshP->GetNbPoints());
        for(size_t pointInd = 0; pointInd < meshP->GetNbPoints(); pointInd++)
            {
            auto pt = meshP->GetPoints()[pointInd];
            newPoints[pointInd].x = pt.x;
            newPoints[pointInd].y = pt.y;
            newPoints[pointInd].z = pt.z;
            if(linearFeaturesPtr->size() > 0)
                {
                pointIds.insert(std::make_pair(pt, (int32_t)pointInd));
                }
            }
        node->m_nodeHeader.m_contentExtent.Init();
        node->m_nodeHeader.m_contentExtent.Extend(&newPoints[0], (int)newPoints.size());
        }

    // Update linear features
    if(linearFeaturesPtr->size() > 0)
        {
        linearFeaturesPtr->clear();
        size_t count = 0;
        bvector<bvector<int32_t>> newDefs;

        if(invertedHulls.IsValid())
            {
            // Update inverted hull (will be treated as a BreakVoid from now on)
            for(auto const& iHull : *invertedHulls)
                {
                auto iHullLSCP = iHull->GetLineStringCP();
                newDefs.push_back(bvector<int>(2 + iHullLSCP->size()));
                newDefs.back()[0] = hullFeatureID;
                newDefs.back()[1] = (int)DTMFeatureType::Hull;
                for(auto const& pt : *iHullLSCP)
                    {
                    if(pointIds.count(pt) > 0)
                        {
                        newDefs.back()[2 + (&pt - &(*iHullLSCP)[0])] = pointIds[pt];
                        }
                    else
                        {
                        newPoints.push_back(PointOp<POINT>::Create(pt.x, pt.y, pt.z));
                        newDefs.back()[2 + (&pt - &(*iHullLSCP)[0])] = (int32_t)newPoints.size() - 1;
                        pointIds.insert(std::make_pair(pt, (int32_t)newPoints.size() - 1));
                        }
                    }
                count += 1 + newDefs.back().size(); // The size of the feature definition will also be recorded
                }
            }

        // Update other feature types
        for(size_t i = 0; i < newFeaturePoints.size(); ++i)
            {
            newDefs.push_back(bvector<int>(2 + newFeaturePoints[i].size()));
            newDefs.back()[0] = (int)(newFeatureIds[i] & 0xFFFFFFFF); // -1;
            newDefs.back()[1] = (int)newFeatureTypes[i];
            for(auto const& pt : newFeaturePoints[i])
                {
                if(pointIds.count(pt) > 0)
                    {
                    newDefs.back()[2 + (&pt - &newFeaturePoints[i][0])] = pointIds[pt];
                    }
                else
                    {
                    newPoints.push_back(PointOp<POINT>::Create(pt.x, pt.y, pt.z));
                    newDefs.back()[2 + (&pt - &newFeaturePoints[i][0])] = (int32_t)newPoints.size() - 1;
                    pointIds.insert(std::make_pair(pt, (int32_t)newPoints.size() - 1));
                    }
                }
            count += 1 + newDefs.back().size(); // The size of the feature definition will also be recorded
            }

        if(count > 0)
            {
            linearFeaturesPtr->reserve(count);
            for(size_t j = linearFeaturesPtr->size(); j < count; ++j) linearFeaturesPtr->push_back(INT_MAX);
            if(linearFeaturesPtr->size() > 0)node->SaveFeatureDefinitions(const_cast<int32_t*>(&*linearFeaturesPtr->begin()), count, newDefs);
            }

        }

    // Store new mesh data in the node
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
    pointsPtr->clear();
    pointsPtr->push_back(&newPoints[0], newPoints.size());

    if(meshP != 0)
        {
        if(meshP->GetNbFaceIndexes() > 0)
            node->PushPtsIndices(meshP->GetFaceIndexes(), meshP->GetNbFaceIndexes());
        else
            node->ClearPtsIndices();
        }

    // Update count
    if(node->IsLeaf() && pointsPtr->size() != node->m_nodeHeader.m_totalCount)
        {
        node->m_nodeHeader.m_totalCount = pointsPtr->size();
        }

    //BeAssert(node->GetPtsIndicePtr()->size() > 0 || pointsPtr->size() <= 10); // This node should have indices

    node->SetDirty(true);

    return true;
    }

template<class POINT, class EXTENT>
bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::FastMesherForRegularGrids(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, BC_DTM_OBJ* dtmObjP) const
    {
    int status = SUCCESS;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

    bvector<DPoint3d> points;
    for(size_t i = 0; i < pointsPtr->size(); ++i)
        {
        if((*pointsPtr)[i].x < 1e15 && (*pointsPtr)[i].y < 1e15 && !std::isnan((*pointsPtr)[i].y) && !std::isnan((*pointsPtr)[i].x))
            {
            points.push_back((*pointsPtr)[i]);
            if(fabs(points.back().x) < 1e-8) points.back().x = 0;
            if(fabs(points.back().y) < 1e-8) points.back().y = 0;
            }
        }

    std::sort(points.begin(), points.end(), [] (const DPoint3d& a, const DPoint3d&b)
              {
              if(a.x < b.x) return true;
              if(a.x > b.x) return false;
              if(a.y < b.y) return true;
              if(a.y > b.y) return false;
              if(a.z < b.z) return true;
              return false;
              });
    double currentX = points[0].x;
    double currentY = points[0].y;
    size_t nCols = 1;
    size_t nRows = 1;
    DPoint2d increment = DPoint2d::From(DBL_MAX, DBL_MAX);
    DPoint3d* lastIdx = &points[0];
    for(auto& pt : points)
        {
        if(fabs(pt.x - currentX) > 1e-6)
            {
            increment.x = std::min(increment.x, pt.x - currentX);
            currentX = pt.x;
            ++nCols;
            // nRows = std::max(nRows, (size_t)(&pt - lastIdx));
            lastIdx = &pt + 1;
            }
        else if(fabs(pt.y - currentY) > 1e-6 && pt.y - currentY > 0)
            {
            increment.y = std::min(increment.y, pt.y - currentY);
            currentY = pt.y;
            }
        //status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &pt, 1);
        }
    //nRows = std::max(nRows, (size_t)(&points.back() - lastIdx) + 1);
    nRows = 1 + ((ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_contentExtent) - ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_contentExtent)) / increment.y);
    if(points.size() == nRows * nCols)
        for(auto&pt : points)
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &pt, 1);
    else
        {
        vector<DPoint3d> completedPts;
        size_t currentPtInSet = 0;
        size_t currentPosX = 0, currentPosY = 0;
        for(currentPosX = 0; currentPosX < nCols; currentPosX++)
            {
            for(currentPosY = 0; currentPosY < nRows; currentPosY++)
                {
                DPoint3d targetPt = DPoint3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_contentExtent) + currentPosX * increment.x,
                                                   ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_contentExtent) + currentPosY * increment.y,
                                                   DBL_MIN);
                if(currentPtInSet < points.size() && fabs(points[currentPtInSet].x - targetPt.x) < 1e-6 && fabs(points[currentPtInSet].y - targetPt.y) < 1e-6)
                    {
                    targetPt.z = points[currentPtInSet].z;
                    currentPtInSet++;
                    }
                completedPts.push_back(targetPt);
                }
            }
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &completedPts.front(), (int)completedPts.size());
        }
    status = bcdtmLattice_createDemTinDtmObject(dtmObjP, (int)nRows, (int)nCols, DBL_MIN);
    return (DTM_SUCCESS == status);
    }

template<class POINT, class EXTENT> 
bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    bool isMeshingDone = false;

#if defined(SM_EXPORT_DTM_DEBUG)
    LOG_SET_PATH("e:\\Elenie\\mesh\\")
    LOG_SET_PATH_W("e:\\Elenie\\mesh\\")
#endif

        //If already mesh just compute the content extent.
        if(node->IsExistingMesh())
            {
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

            assert(pointsPtr->size() > 0);

            node->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::Create((*pointsPtr)[0].x, (*pointsPtr)[0].y, (*pointsPtr)[0].z, (*pointsPtr)[0].x, (*pointsPtr)[0].y, (*pointsPtr)[0].z);

            for(size_t ind = 1; ind < pointsPtr->size(); ind++)
                {
                node->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(node->m_nodeHeader.m_contentExtent, ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX((*pointsPtr)[ind]), PointOp<POINT>::GetY((*pointsPtr)[ind]), PointOp<POINT>::GetZ((*pointsPtr)[ind]),
                                                                                                                                                 PointOp<POINT>::GetX((*pointsPtr)[ind]), PointOp<POINT>::GetY((*pointsPtr)[ind]), PointOp<POINT>::GetZ((*pointsPtr)[ind])));
                }

            node->SetDirty(true);

            return true;
            }

    int status = SUCCESS;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;

    status = CreateBcDTM(dtmPtr);
    BeAssert(status == SUCCESS);

    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();

    IScalableMeshMeshPtr meshPtr; // To store new triangles
    bvector<DPoint3d> points;
    bvector<bvector<int32_t>> featureDefs;
    CurveVectorPtr nonHullFeatures, hullFeatures;
    bvector< std::pair<DTMFeatureType, DTMFeatureId>> nonHullFeatureTypes;
    bvector<bvector<int>> idsOfPrunedVoidIslandFeatures;
    int hullID = -1;

    struct NewFeaturesInfo { bvector<bvector<DPoint3d>> points; bvector<DTMFeatureType> types; bvector<DTMUserTag> tags; bvector<DTMFeatureId> feature_ids; CurveVectorPtr hullFeatures;};
    NewFeaturesInfo newFeaturesInfo, crossingFeaturesInfo;

    // Triangulate data
    if(!node->m_isGrid || linearFeaturesPtr->size() > 0 || !s_useSpecialTriangulationOnGrids)
        {
        bool isOKForMeshing = ExtractDataForMeshing(node, points, featureDefs, nonHullFeatures, nonHullFeatureTypes, newFeaturesInfo.hullFeatures, hullID, idsOfPrunedVoidIslandFeatures);
        if(!points.empty())
            {
            bvector<DPoint3d> filteredPoints;
            FilterFeaturePoints(filteredPoints, points, featureDefs);
            AddToBCDtm(dtmObjP, filteredPoints, nonHullFeatures, nonHullFeatureTypes, newFeaturesInfo.hullFeatures);
            }
        if (isOKForMeshing)
            {
            EXPORT_DTM(node, L"_beforeMeshing_", status)
            dtmObjP->edgeOption = 0;
            dtmObjP->dtmCleanUp = DTMCleanupFlags::VoidsAndIslands;
            status = bcdtmObject_triangulateDtmObject(dtmObjP);
            isMeshingDone = SUCCESS == status;
            EXPORT_DTM(node, L"_afterMeshing_", status)

            static const DTMFeatureType LINEAR_TYPES[] =
                {
                DTMFeatureType::Breakline,
                DTMFeatureType::Hull,
                DTMFeatureType::ContourLine,
                DTMFeatureType::Void,
                DTMFeatureType::BreakVoid,
                DTMFeatureType::Island,
                DTMFeatureType::Hole,
                DTMFeatureType::Polygon,
                DTMFeatureType::ZeroSlopePolygon,
                DTMFeatureType::SoftBreakline
                };
            for(auto featureType : LINEAR_TYPES)
                {
                if(DTM_SUCCESS != dtmPtr->GetBcDTM()->BrowseFeatures(featureType, dtmObjP->numPoints, &newFeaturesInfo,
                                                                     [](DTMFeatureType pi_FeatureType, DTMUserTag pi_FeatureTag, DTMFeatureId pi_FeatureId,
                                                                        DPoint3d* pi_pPoints, size_t pi_PointQty,
                                                                        void* pi_pUserArg) -> int
                                                                     {
                                                                     NewFeaturesInfo& info = *(NewFeaturesInfo*)(pi_pUserArg);
                                                                     if(pi_FeatureType == DTMFeatureType::Hull || pi_FeatureType == DTMFeatureType::TinHull /*|| pi_FeatureTag == HULL_FEATURES_USER_TAG*/)
                                                                         {
                                                                         // Skip hull features, they are treated in a different way
                                                                         return DTM_SUCCESS;
                                                                         }

                                                                     info.points.push_back(bvector<DPoint3d>(pi_PointQty));
                                                                     memcpy(&info.points.back()[0], pi_pPoints, pi_PointQty * sizeof(DPoint3d));

                                                                     info.types.push_back(pi_FeatureType);
                                                                     info.tags.push_back(pi_FeatureTag);
                                                                     info.feature_ids.push_back(pi_FeatureId);

                                                                     return DTM_SUCCESS;
                                                                     }))
                    {
                    BeAssert(nonHullFeatureTypes.end() == find_if(nonHullFeatureTypes.begin(), nonHullFeatureTypes.end(), [featureType] (const std::pair<DTMFeatureType, DTMFeatureId>& feature) -> bool
                                     {
                                     return feature.first == featureType;
                                     })); // Problem reading back linear features
                    }
                }

            }
        }
    else
        {
        // Use faster algorithm in special case
        isMeshingDone = FastMesherForRegularGrids(node, dtmObjP);
        }

    if(isMeshingDone)
        {
        // Apply clip to triangulation if any
        if(status == SUCCESS && !m_clip.empty())
            {
            HFCPtr<HVE2DShape> clipShape = CreateShapeFromPoints(&m_clip[0], m_clip.size(), new HGF2DCoordSys());
            SetClipToDTM(dtmPtr, node->m_nodeHeader.m_contentExtent, *clipShape);
            status = dtmPtr->GetBcDTM()->Triangulate();
            }

        if(status == SUCCESS)
            {
            if(newFeaturesInfo.hullFeatures.IsValid())
                {
                for(auto& hull : *newFeaturesInfo.hullFeatures)
                    {
                    DTMUserTag    userTag = &hull - &(*newFeaturesInfo.hullFeatures)[0];
                    DTMFeatureId* regionIdsP = 0;
                    long          numRegionIds = 0;

                    status = bcdtmInsert_internalDtmFeatureMrDtmObject(dtmObjP,
                                                                       DTMFeatureType::Region,
                                                                       1,
                                                                       2,
                                                                       userTag,
                                                                       &regionIdsP,
                                                                       &numRegionIds,
                                                                       &(*hull->GetLineStringP())[0],
                                                                       (long)hull->GetLineStringP()->size());
                    }
                EXPORT_DTM(node, L"_SplitToTileHull_", status)
                }
            }

        // Reduce triangulation to node extent
        bvector<DPoint3d> removedPoints;
        CurveVectorPtr boundary = nullptr; // Final list of polygonal boundaries

        if(status == SUCCESS)
            {
            bcdtmTin_removePossiblyNonDelaunayTrianglesOnEdgeDtmObject(dtmObjP, node->m_nodeHeader.m_nodeExtent, removedPoints, boundary);

            EXPORT_DTM(node, L"_afterNonDelaunayTrianglesRemoval_", status)
            }

        // Convert DTM triangulation to ScalableMesh mesh
        if(status == SUCCESS)
            {
            // Extract new list of points and indices from the DTM
            bvector<DPoint3d> newPoints;
            bvector<int> newIndices;
            DRange3d range = DRange3d::NullRange();
            ExtractPointsAndIndicesFromDTM(dtmPtr->GetBcDTM(), newPoints, newIndices, range);

            if(!newPoints.empty() && !newIndices.empty())
                {
                meshPtr = IScalableMeshMesh::Create(newPoints.size(), &newPoints[0], newIndices.size(), &newIndices[0], 0, nullptr, nullptr, 0, nullptr, nullptr);

#ifdef SM_EXPORT_DTM_DEBUG

                TerrainModel::BcDTMPtr meshingDTMP;
                (ScalableMeshMesh*)meshPtr.get()->GetAsBcDTM(meshingDTMP);
                if(meshingDTMP != nullptr)
                    EXPORT_DTM_3(node, meshingDTMP->GetTinHandle(), L"_meshAsBcDTM_", status)
#endif
                }
            }

        if(status == SUCCESS)
            {
            // Prepare DTM for stitching

            BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmForStitchingPtr;

            status = CreateBcDTM(dtmForStitchingPtr);
            BeAssert(status == SUCCESS);

            if(status != SUCCESS)
                return true;

            BC_DTM_OBJ* dtmForStitchingObjP(dtmForStitchingPtr->GetBcDTM()->GetTinHandle());

            // Add features that intersect boundary
            CurveVectorPtr consideredFeatures = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
            bvector<std::pair<DTMFeatureType, DTMFeatureId>> consideredFeatureTypes;
            if(boundary.IsValid() && !newFeaturesInfo.points.empty())
                {
                DMatrix4d matrix;
                matrix.InitIdentity();
                for(auto const& feature : newFeaturesInfo.points)
                    {
                    auto type = newFeaturesInfo.types[&feature - &newFeaturesInfo.points[0]];
                    if(IsLinearFeature((ISMStore::FeatureType)type))
                        {
                        CurveVectorPtr intersectionA = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
                        CurveVectorPtr intersectionB = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

                        auto featurePrimitive = ICurvePrimitive::CreateLineString(feature);
                        CurveCurve::IntersectionsXY(*intersectionA, *intersectionB, *featurePrimitive, *boundary, &matrix);
                        if(!intersectionA->empty())
                            {
                            consideredFeatureTypes.push_back(std::make_pair(type, newFeaturesInfo.feature_ids[&feature - &newFeaturesInfo.points[0]]));
                            consideredFeatures->Add(featurePrimitive);
                            }
                        }
                    }
                }

            // Add valid triangulation boundary as internal void feature (don't retriangulate valid internal triangles
            bvector<bvector<bvector<DPoint3d>>> regions;
            if(boundary.IsValid())
                {
                boundary->CollectLinearGeometry(regions);
                for(auto const& region : regions)
                    {
                    BeAssert(region.size() == 1); // Only one loop per region
                    consideredFeatureTypes.push_back(std::make_pair(DTMFeatureType::BreakVoid, -1));
                    consideredFeatures->Add(ICurvePrimitive::CreateLineString(region[0]));
                    consideredFeatureTypes.push_back(std::make_pair(DTMFeatureType::Breakline,-1));
                    consideredFeatures->Add(ICurvePrimitive::CreateLineString(region[0]));
                    }
                }

            AddToBCDtm(dtmForStitchingObjP, removedPoints, consideredFeatures, consideredFeatureTypes, newFeaturesInfo.hullFeatures);

            bcdtmCleanUp_cleanDtmObject(dtmForStitchingObjP);

            dtmPtr = dtmForStitchingPtr;
            }
        }

    // Save DTM in the node for later processing in the stitching
    node->GetTileDTM()->SetData(new BcDTMPtr(dtmPtr->GetBcDTM()));
    EXPORT_DTM_3(node, dtmPtr->GetBcDTM()->GetTinHandle(), L"_readyToStitch_", status)

    if(status == SUCCESS)
        {
        UpdateNodeWithNewMesh(node, meshPtr, newFeaturesInfo.points, newFeaturesInfo.types, newFeaturesInfo.feature_ids, newFeaturesInfo.hullFeatures, hullID);
        }
    else
        {
        BeAssert(!"Triangulation failed");
        }

    return isMeshingDone;
    }

template<class POINT, class EXTENT> bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
        {
#if defined(SM_EXPORT_DTM_DEBUG)
        LOG_SET_PATH("e:\\Elenie\\stitch07\\")
        LOG_SET_PATH_W("e:\\Elenie\\stitch07\\")
#endif

        int status = SUCCESS;

        bool shouldExtractBoundary = true;
        if(node->m_nodeHeader.m_nbFaceIndexes == 0) //this is an unmeshed node. Try to save it...
            {
            shouldExtractBoundary = false;
            }

        static size_t neighborIndices[26] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
        static size_t nodeIndicesInNeighbor[26] = { 7, 6, 5, 4, 3, 2, 1, 0, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

        if(s_useThreadsInStitching) node->LockPts();

        auto initialNumberPoints = pointsPtr->size();
        bvector<DPoint3d> nodePoints(pointsPtr->size());

        for(size_t i = 0; i < pointsPtr->size(); i++)
            {
            nodePoints[i].x = (*pointsPtr)[i].x;
            nodePoints[i].y = (*pointsPtr)[i].y;
            nodePoints[i].z = (*pointsPtr)[i].z;
            }

        if(s_useThreadsInStitching) node->UnlockPts();

        // Fetch tile DTM - should already be ready for stitching, just need to attach neighbor DTMs
        // Clone it so that we can still stitch the neighbors without contention
        auto nodeDTM = (*node->GetTileDTM()->GetData())->Clone();

        BC_DTM_OBJ* dtmObjP = nodeDTM->GetTinHandle();

        EXPORT_DTM_3(node, dtmObjP, L"_MemPoolDTM_", status);

        // Add neighbors in node DTM
        for(size_t& neighborInd : neighborIndices)
            {
            size_t idx = &neighborInd - &neighborIndices[0];
            if((node->m_apNeighborNodes[neighborInd].size() > 0))
                {
                for(size_t neighborSubInd = 0; neighborSubInd < node->m_apNeighborNodes[neighborInd].size(); neighborSubInd++)
                    {
                    HFCPtr < SMMeshIndexNode<POINT, EXTENT>> meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apNeighborNodes[neighborInd][neighborSubInd]);

                    if(!meshNode->IsLoaded())
                        meshNode->Load();

                    if(meshNode->m_nodeHeader.m_nodeCount == 0)
                        continue; // Nothing to add

                    // Fetch neighbor DTM
                    auto tileDTM = meshNode->GetTileDTM();
                    BeAssert(tileDTM.IsValid());

                    if(!tileDTM.IsValid())
                        continue; // Nothing to add

                    auto neighborDTM = *tileDTM->GetData();
                    if(s_useThreadsInStitching)
                        {
                        meshNode->LockDTM();
                        neighborDTM = (*tileDTM->GetData());
                        if (neighborDTM.IsValid()) neighborDTM = neighborDTM->Clone();
                        else neighborDTM = nullptr;
                        meshNode->UnlockDTM();
                        }

                    if(!neighborDTM.IsValid())
                        continue; // Nothing to add

#ifdef SM_EXPORT_DTM_DEBUG
                    WPrintfString neighborFileName(L"_NeighborMemPoolDTM_%u_%u,", idx, neighborSubInd);
                    EXPORT_DTM_3(node, neighborDTM->GetTinHandle(), neighborFileName.c_str(), status);
#endif

                    // Append neighbor data in this node dtm
                    if(meshNode->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] == false)
                        {
                        nodeDTM->Append(*neighborDTM);
                        }
                    else
                        {
                        bvector<DPoint3d> neighborBoundary;
                        neighborDTM->GetBoundary(neighborBoundary);

                        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &neighborBoundary[0], (long)neighborBoundary.size());

                        auto nodeExtent = meshNode->GetNodeExtent();
                        DRange3d nodeBox = DRange3d::From(ExtentOp<EXTENT>::GetXMin(nodeExtent), ExtentOp<EXTENT>::GetYMin(nodeExtent), ExtentOp<EXTENT>::GetZMin(nodeExtent),
                                                          +ExtentOp<EXTENT>::GetXMax(nodeExtent), ExtentOp<EXTENT>::GetYMax(nodeExtent), ExtentOp<EXTENT>::GetZMax(nodeExtent));

                        // Add artificial grid of points to help triangulation
                        bvector<DPoint3d> artificialGridPoints;
                        const size_t gridSize = 10;
                        const double gridSpaceX = (nodeBox.high.x - nodeBox.low.x) / gridSize;
                        const double gridSpaceY = (nodeBox.high.y - nodeBox.low.y) / gridSize;
                        for(size_t i = 1; i < gridSize; i++)
                            {
                            double yPos = nodeBox.low.y + i * gridSpaceY;
                            for(size_t j = 1; j < gridSize; j++)
                                {
                                double xPos = nodeBox.low.x + j * gridSpaceX;
                                artificialGridPoints.push_back(DPoint3d::From(xPos, yPos, 0.0));
                                }
                            }
                        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &artificialGridPoints[0], (long)artificialGridPoints.size());
                        }
                    }
                }
            }

        EXPORT_DTM_3(node, dtmObjP, L"_CombinedDTM_", status);

        // Triangulate DTM
        dtmObjP->edgeOption = 0;
        dtmObjP->dtmCleanUp = DTMCleanupFlags::VoidsAndIslands;
        status = bcdtmObject_triangulateDtmObject(dtmObjP);

        EXPORT_DTM_3(node, dtmObjP, L"_afterTriangulate_", status)

        // Clip DTM to node extent
        DPoint3d extentMin, extentMax;
        extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent));
        extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));

        DPoint3d extentPoly[5] =
            {
            DPoint3d::From(extentMin.x, extentMin.y, 0),
            DPoint3d::From(extentMax.x, extentMin.y, 0),
            DPoint3d::From(extentMax.x, extentMax.y, 0),
            DPoint3d::From(extentMin.x, extentMax.y, 0),
            DPoint3d::From(extentMin.x, extentMin.y, 0)
            };

        // NOTE: The Clip function isn't robust enough, we will clip the mesh using regions
        //       because it's a faster algorithm in any case.
        //nodeDTM->Clip(extentPoly, 5, DTMClipOption::External); // SLOWER than computing regions

        // Split the mesh into a region which is delimited by the node extent
        DTMUserTag    userTag = 0;
        DTMFeatureId* regionIdsP = 0;
        long          numRegionIds = 0;

        status = bcdtmInsert_internalDtmFeatureMrDtmObject(nodeDTM->GetTinHandle(),
                                                           DTMFeatureType::Region,
                                                           1,
                                                           2,
                                                           userTag,
                                                           &regionIdsP,
                                                           &numRegionIds,
                                                           extentPoly,
                                                           5);

        EXPORT_DTM_3(node, dtmObjP, L"_SplitToTileBoundary_", status)

        // NOTES: 1) Merging using Civil's BcDTM::Merge function doesn't work in this particular use case.
        //           When merging, Civil tries to clip the stitched DTM with the boundary of the meshed node but
        //           it doesn't need to in this case because it is already considered as a voided region in the 
        //           stitched DTM.
        //
        //        2) Extracting the DTM triangles using Civil's shader function doesn't work properly in this case.
        //           We use the triangle enumerator instead.

        // Aggregate results into new node mesh info (points and indices)
        auto& nodeIndicesPtr = *node->GetPtsIndicePtr();

        bvector<int32_t> nodeIndices(nodeIndicesPtr.size());
        if(nodeIndicesPtr.size() > 0)
            {
            memcpy(&nodeIndices[0], &(nodeIndicesPtr[0]), nodeIndicesPtr.size() * sizeof(int32_t));
            }

        // Extract new list of points and indices from the DTM
        ExtractPointsAndIndicesFromDTM(nodeDTM, nodePoints, nodeIndices, node->m_nodeHeader.m_contentExtent, userTag);

        // Save new list of points and indices in the node
        if(!nodePoints.empty())
            {
            if(s_useThreadsInStitching) node->LockPts();
            pointsPtr->clear();
            pointsPtr->push_back(&nodePoints[0], nodePoints.size());

            if (!nodeIndices.empty()) node->ReplacePtsIndices((int32_t*)&nodeIndices[0], nodeIndices.size());

            // Update node content extent
            if (!node->m_nodeHeader.m_contentExtent.IsNull() && !node->m_nodeHeader.m_contentExtent.IsEmpty())
            node->m_nodeHeader.m_contentExtentDefined = true;

            // Update node counts (includes voided hull points)
            if(node->IsLeaf())
                node->m_nodeHeader.m_totalCount = pointsPtr->size();
            else
                {
                node->m_nodeHeader.m_totalCount -= initialNumberPoints;
                node->m_nodeHeader.m_totalCount += pointsPtr->size();
                }

            assert(pointsPtr->size() == nodePoints.size());
            if(s_useThreadsInStitching) node->UnlockPts();

            // Clear node DTM data, we are done with it... take a pointer to the node DTM now to avoid locking mutexes more than once.
            auto nodeDTMPtr = node->GetTileDTM();
            if(s_useThreadsInStitching) node->LockDTM();
            nodeDTMPtr->SetData(new BcDTMPtr(nullptr));

            // Indicate to neighbors that we are done stitching this node
            for(size_t& neighborInd : neighborIndices)
                {
                node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] = true;
                }
            if(s_useThreadsInStitching) node->UnlockDTM();
            }

#ifdef SM_EXPORT_DTM_DEBUG
        if (node->GetTileDTM().IsValid() && (*node->GetTileDTM()->GetData()).IsValid() && (*node->GetTileDTM()->GetData())->GetTinHandle() != nullptr)
            {
            dtmObjP = (*node->GetTileDTM()->GetData())->GetTinHandle();
            EXPORT_DTM_3(node, dtmObjP, L"_finalStitchedMesh_", status)
            }
#endif

        return true;
        }

inline void ValidateTriangleMesh(vector<int> faces, vector<DPoint3d> stitchedPoints)
    {
    std::map<long long, int> edges;
    std::vector<DSegment3d> lines;
    for(size_t i = 0; i < faces.size(); i += 3)
        {
        assert(faces[i] <= (int)stitchedPoints.size() && faces[i] > 0);
        assert(faces[i + 1] <= (int)stitchedPoints.size() && faces[i + 1] > 0);
        assert(faces[i + 2] <= (int)stitchedPoints.size() && faces[i + 2] > 0);
        long long edge1 = (((long long)faces[i] << 32) | faces[i + 1]);
        long long edge2 = (((long long)faces[i + 1] << 32) | faces[i + 2]);
        long long edge3 = (((long long)faces[i + 2] << 32) | faces[i]);
        long long edge4 = (((long long)faces[i + 1] << 32) | faces[i]);
        long long edge5 = (((long long)faces[i + 2] << 32) | faces[i + 1]);
        long long edge6 = (((long long)faces[i] << 32) | faces[i + 2]);
        if(edges.count(edge1) == 0)
            {
            edges[edge1] = edges[edge4] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i] - 1], stitchedPoints[faces[i + 1] - 1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for(size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else {
            assert(edges[edge4] == edges[edge1]);
            edges[edge4] = ++edges[edge1];
            assert(edges[edge1] <= 2);
            }
        if(edges.count(edge2) == 0)
            {
            edges[edge2] = edges[edge5] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i + 1] - 1], stitchedPoints[faces[i + 2] - 1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for(size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else
            {
            assert(edges[edge5] == edges[edge2]);
            edges[edge5] = ++edges[edge2];
            assert(edges[edge2] <= 2);
            }
        if(edges.count(edge3) == 0)
            {
            edges[edge3] = edges[edge6] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i + 2] - 1], stitchedPoints[faces[i] - 1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for(size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else
            {
            assert(edges[edge6] == edges[edge3]);
            edges[edge6] = ++edges[edge3];
            assert(edges[edge3] <= 2);
            }
        }
    }

inline void ExtractPointsAndIndicesFromDTM(BcDTMPtr nodeDTM, bvector<DPoint3d>& points, bvector<int>& indices, DRange3d& range, DTMUserTag userTag = -1)
    {
    // To keep track of unique points and their corresponding index
    std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> uniquePointsSet(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    if(!points.empty() && !indices.empty())
        {
        // Prime the unique set with the original data
        for(int32_t i = 0; i < indices.size(); i++)
            {
            DPoint3d currentPt = points[indices[i] - 1];

            if(uniquePointsSet.count(currentPt) != 0)
                continue; // Skip already considered points

            uniquePointsSet.insert(std::make_pair(currentPt, indices[i] - 1));
            }
        }

    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*nodeDTM);
    if(userTag != -1)
        en->SetFilterRegionByUserTag(userTag);
    else
        en->SetExcludeAllRegions();
    en->SetMaxTriangles(nodeDTM->GetBcDTM()->GetTinHandle()->numTriangles);
    for(PolyfaceQueryP pf : *en)
        {
        PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
        vec->CopyFrom(*pf);
        for(PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*vec); addedFacets->AdvanceToNextFace();)
            {
            DPoint3d face[3];
            int32_t idx[3] = { -1, -1, -1 };
            for(size_t i = 0; i < 3; ++i)
                {
                face[i] = addedFacets->GetPointCP()[i];
                idx[i] = uniquePointsSet.count(face[i]) != 0 ? uniquePointsSet[face[i]] : -1;
                }
            for(size_t i = 0; i < 3; ++i)
                {
                if(idx[i] == -1)
                    {
                    points.push_back(face[i]);
                    idx[i] = (int)points.size();
                    uniquePointsSet[face[i]] = idx[i] - 1;
                    range.Extend(points.back());
                    }
                else idx[i]++;
                }
            indices.push_back(idx[0]);
            indices.push_back(idx[1]);
            indices.push_back(idx[2]);
            }
        }
    }