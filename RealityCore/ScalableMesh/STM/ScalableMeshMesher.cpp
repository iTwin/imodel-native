//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
        
           
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/
//#include <toolsubs.h>
/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshMesher.h"
#include "ScalableMeshMesher.hpp"
#include <ScalableMesh/IScalableMeshQuery.h>
#include "Edits/ClipUtilities.hpp"
using namespace BENTLEY_NAMESPACE_NAME::TerrainModel;

template class ScalableMesh2DDelaunayMesher<DPoint3d, DRange3d>;

int AddPolygonsToDTMObject(bvector<bvector<DPoint3d>>& polygons, DTMFeatureType type, BC_DTM_OBJ* dtmObjP)
    {
    int status = DTM_SUCCESS;
    for (auto& polygon : polygons)
        {
        if (polygon.size() > 2)
            {
            if (!polygon.front().IsEqual (*(&polygon.back()), 1e-8)) polygon.push_back(polygon.front());
            if (type == DTMFeatureType::Void)
                {
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
                assert(status == SUCCESS);
                }
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, type, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
            assert(status == SUCCESS);
            }
        }
    return status;
    }

//Limited case. If a void and an island exactly overlap, remove them both.
void PruneFeatures(bvector<bvector<int>>& idsOfPrunedVoidIslandFeatures, bvector<bvector<DPoint3d>>& islandFeatures, bvector<bvector<DPoint3d>>& voidFeatures)
    {
    idsOfPrunedVoidIslandFeatures.resize(2);
    bvector<bool> deleteIslands(islandFeatures.size());
    bvector<bool> deleteVoids(voidFeatures.size());
    for (auto& island : islandFeatures)
    {
        ICurvePrimitivePtr curvePtr;
        CurveVectorPtr curveVectorPtr;

        curvePtr = ICurvePrimitive::CreateLineString(island);
        curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);

        for (auto& hole : voidFeatures)
        {

            bool allInsidePolygon = true;
            for (size_t i = 0; i < hole.size() && allInsidePolygon; ++i)
            {
                auto classif = curveVectorPtr->PointInOnOutXY(hole[i]);
                if (classif == CurveVector::InOutClassification::INOUT_Out) allInsidePolygon = false;
            }

            if(allInsidePolygon)
                {

                bool allInsideVoid = true;
                ICurvePrimitivePtr curvePtrVoid;
                CurveVectorPtr curveVectorPtrVoid;

                curvePtrVoid = ICurvePrimitive::CreateLineString(hole);
                curveVectorPtrVoid = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtrVoid);
                for(size_t i = 0; i < island.size() && allInsideVoid; ++i)
                    {
                    auto classif = curveVectorPtrVoid->PointInOnOutXY(island[i]);
                    if(classif == CurveVector::InOutClassification::INOUT_Out) allInsideVoid = false;
                    }
                if(allInsideVoid)
                    {
                    deleteIslands[&island - &islandFeatures[0]] = true;
                    deleteVoids[&hole - &voidFeatures[0]] = true;
                    }
                //else
                //    {
                //    if(!idsOfPrunedVoidIslandFeatures.empty() && !idsOfVoidIslandFeatures.empty() && !idsOfVoidIslandFeatures[0].empty())
                //        idsOfPrunedVoidIslandFeatures[0].push_back(idsOfVoidIslandFeatures[0][&hole - &voidFeatures[0]]);
                //    if(idsOfPrunedVoidIslandFeatures.size() > 1 && idsOfVoidIslandFeatures.size() > 1 && !idsOfVoidIslandFeatures[1].empty())
                //        idsOfPrunedVoidIslandFeatures[1].push_back(idsOfVoidIslandFeatures[1][&island - &islandFeatures[0]]);
                //    }
                }
            }
        }

    for(auto& island : islandFeatures)
        {
        if (!deleteIslands[&island - &islandFeatures[0]] && idsOfPrunedVoidIslandFeatures.size() > 1)
            idsOfPrunedVoidIslandFeatures[1].push_back(&island - &islandFeatures[0]);
        }
    for(auto& hole : voidFeatures)
        {
        if (!deleteVoids[&hole - &voidFeatures[0]] && !idsOfPrunedVoidIslandFeatures.empty())
            idsOfPrunedVoidIslandFeatures[0].push_back(&hole - &voidFeatures[0]);
        }
    //bvector<bvector<DPoint3d>> newIslands;
    //bvector<bvector<DPoint3d>> newVoids;
    //bvector<DTMFeatureType> newTypes;
    //for (auto& island : islandFeatures)
    //{
    //    if (!deleteIslands[&island - &islandFeatures[0]])
    //        newIslands.push_back(island);
    //}
    //
    //for (auto& hole : voidFeatures)
    //{
    //    if (!deleteVoids[&hole - &voidFeatures[0]])
    //    {
    //        newVoids.push_back(hole);
    //        if (!types.empty()) newTypes.push_back(types[&hole - &voidFeatures[0]]);
    //    }
    //}
    //
    //islandFeatures.swap(newIslands);
    //voidFeatures.swap(newVoids);
    //if (!types.empty()) types.swap(newTypes);
    }


void circumcircle(DPoint3d& center, double& radius, const DPoint3d* triangle)
    {
    double det = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, 1, triangle[1].x, triangle[1].y, 1, triangle[2].x, triangle[2].y, 1).Determinant();
    double mags[3] = { triangle[0].MagnitudeSquaredXY(),
        triangle[1].MagnitudeSquaredXY(),
        triangle[2].MagnitudeSquaredXY() };
    double det1 = 0.5*(RotMatrix::FromRowValues(mags[0], triangle[0].y, 1, mags[1], triangle[1].y, 1, mags[2], triangle[2].y, 1).Determinant());
    double det2 = 0.5*RotMatrix::FromRowValues(triangle[0].x, mags[0], 1, triangle[1].x, mags[1], 1, triangle[2].x, mags[2], 1).Determinant();
    center.x = det1 / det;
    center.y = det2 / det;
    center.z = triangle[0].z;
    radius = center.Distance(triangle[0]);
    }

static bool s_passNormal = false;

int draw(DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP)
{
    IScalableMeshMeshPtr* meshPtr = (IScalableMeshMeshPtr*)userP;
    /*
    uint32_t numPerFace = 3;
    bool   twoSided = false;
    size_t indexCount = numMeshFaces;
    size_t pointCount = numMeshPts;
    DPoint3dCP pPoint = meshPtsP;
    int32_t const* pPointIndex = (int32_t*)meshFacesP;
    size_t normalCount = numMeshPts;
    DVec3dCP  pNormal = (DVec3dCP)meshVectorsP;
    int32_t const* pNormalIndex = (int32_t*)meshFacesP;
    */
    if (s_passNormal)
    {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, numMeshPts, (DVec3d*)meshVectorsP, (int32_t*)meshFacesP, 0, 0, 0);
    }
    else
    {
        *meshPtr = IScalableMeshMesh::Create(numMeshPts, meshPtsP, numMeshFaces, (int32_t*)meshFacesP, 0, 0, 0, 0, 0, 0);
    }

    return SUCCESS;
}
