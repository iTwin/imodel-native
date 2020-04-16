//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

/** ----------------------------------------------------------------------------------------
    This class implements a 2D Delaunay meshing algorithm to be used on an unmeshed index.
    ----------------------------------------------------------------------------------------
*/
template<class POINT, class EXTENT> class ScalableMesh2DDelaunayMesher : public ISMPointIndexMesher<POINT, EXTENT>
{

    public:

                            ScalableMesh2DDelaunayMesher() {};
        virtual             ~ScalableMesh2DDelaunayMesher() {};


        virtual bool        Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;

        virtual bool        Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const override;

        virtual void        AddClip(bvector<DPoint3d>& clip) override { m_clip = clip; }
        
        
    private:

        bool   ExtractDataForMeshing(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, bvector<DPoint3d>& points, bvector<bvector<int32_t>>& featureDefs, CurveVectorPtr& nonHullFeatures, bvector< std::pair<DTMFeatureType, DTMFeatureId>>& nonHullFeatureInfo, CurveVectorPtr& hullFeatures, int& hullID, bvector<bvector<int>>& idsOfPrunedVoidIslandFeatures) const;
        bool   UpdateNodeWithNewMesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, IScalableMeshMeshPtr meshPtr, bvector<bvector<POINT>>& newFeaturePoints, bvector<DTMFeatureType>& newFeatureTypes, bvector<DTMFeatureId>& newFeatureIds, CurveVectorPtr invertedHulls, int hullFeatureID) const;
        void   FilterFeaturePoints(bvector<DPoint3d>& filteredPoints, bvector<DPoint3d>& points, bvector<bvector<int32_t>>& featureDefs) const;
        bool   FastMesherForRegularGrids(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, BC_DTM_OBJ* dtmObj) const;
        bvector<DPoint3d> m_clip; // Used to restrict meshing to a nonconvex polygonal shape (when importing within a nonconvex polygon)
    };
