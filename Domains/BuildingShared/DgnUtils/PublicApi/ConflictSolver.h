/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/PublicApi/ConflictSolver.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class EdgeUserType
    {
    Undefined = -1,
    Inner = 0,
    Container = 1
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               03/2018
//=======================================================================================
struct Conflict
    {
    bset<BeInt64Id> m_conflictingBoundaries;
    CurveVectorPtr m_geometry = nullptr;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               03/2018
//=======================================================================================
struct ConflictSolver : NonCopyableClass
    {
    private:
        BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::RegionsApiWrapper m_regionsApi;
        bool m_hasContainerBoundary;

        bool IsBoundaryValid(CurveVectorCR boundary);
        bvector<Conflict> GetProblemIndicator();
        bmap<MTGNodeId, Conflict> GetProblemFaces();
        bool IsFaceProblem(MTGNodeId fid, Conflict& conflict);
        bvector<Conflict> GetConflictsWithGeometry(bmap<MTGNodeId, Conflict>& conflictsMap, bool includeHoles = false);

    public:
        //! @param[in] localToWorld LocalToWorld transform.
        BUILDINGSHAREDDGNUTILS_EXPORT ConflictSolver(Transform const& localToWorld);
        ConflictSolver() : ConflictSolver(Transform::FromIdentity()) {}

        //! Add a container boundary to the solver. Any parts of inner boundaries that are
        //! fully or partially outside of the added container boundaries will be considered as conflicts.
        //! Boundaries with no area are ignored.
        //! @param[in] boundary The outer boundary to be added. Outer edges must be CCW, inner edges (holes) must be CW.
        //! @param[in] id       Identifier that is used to map added boundaries to found conflicts.
        BUILDINGSHAREDDGNUTILS_EXPORT void AddContainerBoundary(CurveVectorCR boundary, BeInt64Id const& id);

        //! Add an inner boundary to the solver. Any parts of inner boundaries that are
        //! inside of the other added inner boundaries will be considered as conflicts.
        //! Boundaries with no area are ignored.
        //! @param[in] boundary The inner boundary to be added. Outer edges must be CCW, inner edges (holes) must be CW.
        //! @param[in] id       Identifier that is used to map added boundaries to found conflicts.
        BUILDINGSHAREDDGNUTILS_EXPORT void AddInnerBoundary(CurveVectorCR boundary, BeInt64Id const& id);

        //! Runs the conflict solver.
        //! @return conflicts that are found.
        BUILDINGSHAREDDGNUTILS_EXPORT bvector<Conflict> Solve();
    };

END_BUILDING_SHARED_NAMESPACE