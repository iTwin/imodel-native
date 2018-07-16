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
        auto idA = CurvePrimitiveId::Create (type,
            CurveTopologyId (CurveTopologyId::Type::CutFill, 0));
        Check::True (type == idA->GetType (), "Id type");
        auto idB = CurvePrimitiveId::Create (*idA);
        auto idC = idA->Clone ();
        Check::Int (0, idA->GetGeometryStreamIndex ());
        Check::Int (0, idA->GetPartGeometryStreamIndex ());
        Check::False(0 == idA->GetIdSize ());
        //Check::Size (0, (size_t)idA->PeekId ());
        auto s = idA->GetDebugString ();
        GEOMAPI_PRINTF (" Debug String %s\n", s.c_str ());
        }
    }
    
    
