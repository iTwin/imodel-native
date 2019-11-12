/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/BuildingDgnUtilsApi.h"

USING_NAMESPACE_BUILDING_SHARED

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Wouter.Rombouts 08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ConflictSolver::ConflictSolver
(
    Transform const& localToWorld
)
    : m_regionsApi(localToWorld)
    , m_hasContainerBoundary(false)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Nerijus.Jakeliunas                 09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Conflict> ConflictSolver::GetConflictsWithGeometry
(
    bmap<MTGNodeId, Conflict>& conflictsMap, 
    bool includeHoles
)
    {
    bvector<Conflict> cvs;
    for (bmap<MTGNodeId, Conflict>::iterator ppIter = conflictsMap.begin(); ppIter != conflictsMap.end(); ppIter++)
        {
        bvector<DPoint3d> facePoints;
        if (BentleyStatus::SUCCESS != m_regionsApi.GetFacePoints(facePoints, ppIter->first))
            continue;

        bool hasHoles = includeHoles && m_regionsApi.DoesFaceHaveHole(ppIter->first);
        CurveVectorPtr cvPtr = CurveVector::CreateLinear(facePoints, hasHoles ? CurveVector::BOUNDARY_TYPE_ParityRegion : CurveVector::BOUNDARY_TYPE_Outer);

        if (hasHoles)
            {
            EmbeddedIntArray holeNodeIdArray;
            m_regionsApi.SearchFaceHoleArray(&holeNodeIdArray, ppIter->first);

            MTGNodeId holeNodeId;
            int iFace = 0;
            while (jmdlEmbeddedIntArray_getInt(&holeNodeIdArray, &holeNodeId, iFace++))
                {
                bvector<DPoint3d> holePoints;
                if (BentleyStatus::SUCCESS != m_regionsApi.GetFacePoints(holePoints, holeNodeId))
                    continue;

                CurveVectorPtr cvHolePtr = CurveVector::CreateLinear(holePoints, CurveVector::BOUNDARY_TYPE_Inner);
                double   areaHole;
                DPoint3d centroidHole;
                cvHolePtr->CentroidAreaXY(centroidHole, areaHole);

                if (fabs(areaHole) > 0.1)
                    {
                    cvPtr->Add(cvHolePtr);
                    }
                }
            }

        double   area;
        DPoint3d centroid;
        cvPtr->CentroidAreaXY(centroid, area);

        if (cvPtr.IsValid() && (area > .1))
            {
            ppIter->second.m_geometry = cvPtr;
            cvs.push_back(ppIter->second);
            }
        }

    return cvs;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/00
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Conflict> ConflictSolver::GetProblemIndicator()
    {
    bmap<MTGNodeId, Conflict>  pfcs = GetProblemFaces();

    bvector<Conflict> cvs = GetConflictsWithGeometry(pfcs, true);

    return cvs;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Wouter.Rombouts                 04/00
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<MTGNodeId, Conflict> ConflictSolver::GetProblemFaces()
    {
    bmap<MTGNodeId, Conflict> pfcs;
    bvector<MTGNodeId> fcs = m_regionsApi.GetFaces(NULL);

    for (bvector<MTGNodeId>::iterator ppIter = fcs.begin(); ppIter != fcs.end(); ppIter++)
        {
        Conflict conflict;
        if (IsFaceProblem(*ppIter, conflict))
            {
            pfcs.insert({*ppIter, conflict});
            }
        }

    return pfcs;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ConflictSolver::IsFaceProblem(MTGNodeId fid, Conflict& conflict)
    {
    if(m_regionsApi.IsFaceNull(fid) || m_regionsApi.DoesFaceHaveNegativeArea(fid))
        {
        return false;
        }

    bmap<int64_t, RegionsApiWrapper::CurveInfo> isInsideIds;
    bmap<int64_t, RegionsApiWrapper::CurveInfo> isOutsideIds;
    m_regionsApi.GetFaceParentCurvesDeep(fid, isInsideIds, isOutsideIds);

    int insideInnerCount = (int)std::count_if(isInsideIds.begin(), isInsideIds.end(), [] (bpair<int64_t, RegionsApiWrapper::CurveInfo> const& current)
        {
        return EdgeUserType::Inner == static_cast<EdgeUserType>(current.second.ownerType);
        });
    int insideContainerCount = (int)std::count_if(isInsideIds.begin(), isInsideIds.end(), [] (bpair<int64_t, RegionsApiWrapper::CurveInfo> const& current)
        {
        return EdgeUserType::Container == static_cast<EdgeUserType>(current.second.ownerType);
        });
    /* unused - int outsideInnerCount = (int)*/std::count_if(isOutsideIds.begin(), isOutsideIds.end(), [] (bpair<int64_t, RegionsApiWrapper::CurveInfo> const& current)
        {
        return EdgeUserType::Inner == static_cast<EdgeUserType>(current.second.ownerType);
        });
    /* unused - int outsideContainerCount = (int)*/std::count_if(isOutsideIds.begin(), isOutsideIds.end(), [] (bpair<int64_t, RegionsApiWrapper::CurveInfo> const& current)
        {
        return EdgeUserType::Container == static_cast<EdgeUserType>(current.second.ownerType);
        });

    bool isConflict = false;
    if (m_hasContainerBoundary && insideInnerCount > 0 && insideContainerCount == 0) // inner boundary is not inside a container boundary
        isConflict = true;
    if (insideInnerCount > 1) // inner boundary overlaps another inner boundary
        isConflict = true;

    if (!isConflict)
        return false;

    for (bmap<int64_t, RegionsApiWrapper::CurveInfo>::iterator pIter = isInsideIds.begin(); pIter != isInsideIds.end(); pIter++)
        {
        if (EdgeUserType::Container == static_cast<EdgeUserType>(pIter->second.ownerType))
            continue;

        BeInt64Id cid((uint64_t) pIter->first);
        conflict.m_conflictingBoundaries.insert(cid);
        }
    
    for (bmap<int64_t, RegionsApiWrapper::CurveInfo>::iterator pIter = isOutsideIds.begin(); pIter != isOutsideIds.end(); pIter++)
        {
        if (EdgeUserType::Inner == static_cast<EdgeUserType>(pIter->second.ownerType))
            continue;

        BeInt64Id cid((uint64_t) pIter->first);
        conflict.m_conflictingBoundaries.insert(cid);
        }
        
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ConflictSolver::IsBoundaryValid
(
    CurveVectorCR boundary
)
    {
    double area;
    DPoint3d centroid;
    DVec3d normal;

    return boundary.CentroidNormalArea(centroid, normal, area) 
        && !DoubleOps::AlmostEqual(area, 0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ConflictSolver::AddContainerBoundary
(
    CurveVectorCR boundary,
    BeInt64Id const& id
)
    {
    if (!IsBoundaryValid(boundary))
        return;

    m_regionsApi.ImprintCurveVector(
        boundary, 
        static_cast<int32_t>(EdgeUserType::Container), 
        id.GetValueUnchecked(), 
        nullptr);

    m_hasContainerBoundary = true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void ConflictSolver::AddInnerBoundary
(
    CurveVectorCR boundary,
    BeInt64Id const& id
)
    {
    if (!IsBoundaryValid(boundary))
        return;

    m_regionsApi.ImprintCurveVector(
        boundary, 
        static_cast<int32_t>(EdgeUserType::Inner), 
        id.GetValueUnchecked(), 
        nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<Conflict> ConflictSolver::Solve()
    {
    if (m_regionsApi.InferTopology(nullptr, 1.0, 1.0))
        {
        return GetProblemIndicator();
        }
    return bvector<Conflict>();
    }