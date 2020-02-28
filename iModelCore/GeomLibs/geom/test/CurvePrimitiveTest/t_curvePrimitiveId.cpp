/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/* unused - static int s_noisy = 0;*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitiveId, Hello)
    {
    //bvector<ICurvePrimitivePtr> curves = SampleGeometryCreator::AddAllCurveTypes (curves);
    for (auto type : {
        CurvePrimitiveId::Type::SolidPrimitive,
        CurvePrimitiveId::Type::CurveVector
        })
        {
        auto idA = CurvePrimitiveId::Create (type,CurveTopologyId (CurveTopologyId::Type::CutFill, 0));
    
        Check::True (type == idA->GetType (), "Id type");
        auto idB = CurvePrimitiveId::Create (*idA);
        auto idC = idA->Clone ();

        bvector<BoolTypeForVector>   bytes;
        idA->Store(bytes);
        auto idD = CurvePrimitiveId::Create(bytes.data(), bytes.size());
        
        Check::Int (0, idA->GetGeometryStreamIndex ());
        Check::Int (0, idA->GetPartGeometryStreamIndex ());
        Check::False(0 == idA->GetIdSize ());
        Check::True(*idA == *idB);  
        Check::True(*idA == *idC);  
        Check::True(*idA == *idD);  
        auto s = idA->GetDebugString ();
        GEOMAPI_PRINTF (" Debug String %s\n", s.c_str ());
        }

    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Ray.Bentley    07/18
+---------------+---------------+---------------+---------------+---------------+------*/
void testTopologyId(CurveTopologyIdCR idA)
    {
    auto    idB = CurveTopologyId(idA);

    bvector<BoolTypeForVector>   bytes;

    idA.Pack(bytes);
    auto    idC = CurveTopologyId(bytes.data(), bytes.size());

    Check::True(idA == idB);  
    Check::True(idA == idC);
    auto s = idA.GetDebugString ();
    GEOMAPI_PRINTF (" Debug String %s\n", s.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Ray.Bentley    07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveTopologyId, Hello)
    {
    FaceId      faceId0, faceId1;

    faceId0.nodeId = faceId1.nodeId = 1;
    faceId0.entityId = faceId1.entityId = 2;

    testTopologyId (CurveTopologyId::FromBRepIsoline(faceId0, 3));
    testTopologyId (CurveTopologyId::FromBRepSharedEdge(faceId0, faceId1));
    testTopologyId (CurveTopologyId::FromBRepSheetEdge(faceId0));
    testTopologyId (CurveTopologyId::FromBRepSilhouette(faceId0));
    testTopologyId (CurveTopologyId::FromBRepPlanarFace(faceId0));
    testTopologyId (CurveTopologyId::FromSweepProfile(1));
    testTopologyId (CurveTopologyId::FromSweepLateral(1));
    testTopologyId (CurveTopologyId::FromSweepSilhouette(1));
    testTopologyId (CurveTopologyId::FromMeshSharedEdge(1, 2));
    testTopologyId (CurveTopologyId::FromMeshEdgeVertices(1, 2));
    testTopologyId (CurveTopologyId::FromGeometryMap());
    testTopologyId (CurveTopologyId::FromWire());
    testTopologyId (CurveTopologyId::FromUnknownCurve(1));
    testTopologyId (CurveTopologyId::FromVisEdgesBoundedPlane(1));
    testTopologyId (CurveTopologyId::FromParasolidGPArrayId(1, 2));
    testTopologyId (CurveTopologyId::FromVisEdgesIntersection());
    testTopologyId (CurveTopologyId::FromCurveVector());
    }
    
    
    
