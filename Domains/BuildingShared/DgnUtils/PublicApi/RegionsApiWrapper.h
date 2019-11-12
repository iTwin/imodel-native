/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               03/2018
//=======================================================================================
struct RegionsApiWrapper : NonCopyableClass
    {
    struct CurveInfo
        {
        int             primaryUse;
        int32_t         ownerType;
        bset<MTGNodeId> edgeNodeIds;
        };

    private:
        RG_Header* m_pRG;
        double m_uorToMeter; //RG works in UORS!
        RIMSBS_Context* m_pCurves;
        Transform m_localToWorldTransform;
        Transform m_worldToLocalTransform;

        void AddEdgeCurve(bmap<int64_t, CurveInfo>& ids, int32_t userInt32, int64_t userInt64, int generation, MTGNodeId edgeNodeId);
        void AnalyzeEdgeCurve(RG_Header* pRG, MTGNodeId edgeNodeId, bmap<int64_t, CurveInfo>& isInsideIds, bmap<int64_t, CurveInfo>& isOutsideIds, int32_t userInt32, int64_t userInt64, int generation, MTGNodeId mainEdgeNodeId);

    public:
        //! @param[in] localToWorld LocalToWorld transform.
        BUILDINGSHAREDDGNUTILS_EXPORT RegionsApiWrapper(Transform const& localToWorld);
        RegionsApiWrapper() : RegionsApiWrapper(Transform::FromIdentity()) {}
        BUILDINGSHAREDDGNUTILS_EXPORT ~RegionsApiWrapper();

        bool ImprintPoly(DPoint3d* pPointArray, size_t pointArrayCount, int32_t userInt32, int64_t userInt64, void* pUserData);
        BUILDINGSHAREDDGNUTILS_EXPORT bool ImprintCurveVector(CurveVectorCR cv, int32_t userInt32, int64_t userInt64, void* pUserData);
        BUILDINGSHAREDDGNUTILS_EXPORT bool InferTopology(void* pAbortFunc, double vertexVertexTolerance, double vertexEdgeTolerance);
        bool GetEndPoints(MTGNodeId edgeNode, DPoint3d* pStartPoint, DPoint3d* pEndPoint);
        BUILDINGSHAREDDGNUTILS_EXPORT BentleyStatus GetFacePoints(bvector<DPoint3d>& points, MTGNodeId faceId);
        BUILDINGSHAREDDGNUTILS_EXPORT bool DoesFaceHaveHole(MTGNodeId faceId);
        BUILDINGSHAREDDGNUTILS_EXPORT void SearchFaceHoleArray(EmbeddedIntArray* pHoleNodeIdArray, MTGNodeId outerFaceNodeId);
        MTGNodeId GetLowestFaceNodeId(MTGNodeId thisNode);
        bool GetEdgeUserData(MTGNodeId edgeNode, int32_t& userInt32, int64_t& userInt64);
        MTGNodeId GetParentFace(MTGNodeId faceNode /* => subject */);
        bool GetFaceParentCurves(MTGNodeId thisNode, bset<MTGNodeId>& addList, bmap<int64_t, CurveInfo>& insideIds, bmap<int64_t, CurveInfo>& outsideIds, int generation);
        void AnalyzeEdgeCurveDeep(MTGNodeId currNodeId, bset<MTGNodeId>& addList, bmap<int64_t, CurveInfo>& isInsideIds, bmap<int64_t, CurveInfo>& isOutsideIds, int generation);
        BUILDINGSHAREDDGNUTILS_EXPORT bool GetFaceParentCurvesDeep(MTGNodeId thisNode, bmap<int64_t, CurveInfo>& isInsideIds, bmap<int64_t, CurveInfo>& isOutsideIds);
        BUILDINGSHAREDDGNUTILS_EXPORT bool IsFaceNull(MTGNodeId faceId);
        BUILDINGSHAREDDGNUTILS_EXPORT bool DoesFaceHaveNegativeArea(MTGNodeId faceId);
        BUILDINGSHAREDDGNUTILS_EXPORT bvector<MTGNodeId> GetFaces(bool* pReqSign);
    };

END_BUILDING_SHARED_NAMESPACE