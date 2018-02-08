/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceFastNormalBuilder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
#include "pf_halfEdgeArray.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PolyfaceHeader::BuildNormalsFast (double createTolerance)
    {
    if (0 != GetNormalCount())
        return;    

    // So we can skip expensive visitors.
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

    constexpr   double              s_minArea = 1.0E-14;
    static      DVec3d              s_defaultNormal = DVec3d::From(0.0, 0.0, 1.0);

    bvector<DPoint3d>               facePoints;
    DPoint3dCP                      points = GetPointCP();
    size_t                          pointIndexCount = GetPointIndexCount();
    LightweightPolyfaceBuilderPtr   builder = LightweightPolyfaceBuilder::Create(*this);

    for (int32_t const *index = GetPointIndexCP(), *end = index + pointIndexCount; index < end; index++)
        {
        if (0 == *index)
            {
            size_t      faceCount = facePoints.size();
            DVec3d      normal;

            if (bsiPolygon_polygonNormalAndArea(&normal, nullptr, facePoints.data(), (int) faceCount) < s_minArea)
                normal = s_defaultNormal;
            
            size_t      normalIndex = builder->FindOrAddNormal(normal);

            for (size_t i=0; i<faceCount; i++)
               builder->AddNormalIndex(normalIndex);

            builder->AddNormalIndexTerminator();

            facePoints.clear();
            }
        else
            {
            int32_t     zeroBasedIndex = abs(*index) - 1;
            BeAssert (zeroBasedIndex < GetPointCount());
            facePoints.push_back(points[zeroBasedIndex]);
            }
        }
    };

