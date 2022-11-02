/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
bool ReadDgnjsGeometry(bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC);
void DoRoundTrip(IGeometryPtr g0, bool emitGeometry, int serializerSelect);

static const WCharCP s_alexGSpirals[] = 
    {
    L"sine.imjs"
    /* ,
    L"biquadratic.imjs",
    L"bloss.imjs",
    L"clothoid.imjs",
    L"cosine.imjs",
    L"arema.imjs",
    L"australianRailCorp.imjs",
    L"chineseCubic.imjs",
    L"czech.imjs",
    L"halfCosine.imjs",
    L"italian.imjs",
    L"mxCubicAlongArc.imjs",
    L"polishCubic.imjs",
    L"westernAustralian.imjs",
    */
    };

TEST(Spiral, ReadAlignments)
    {
    for (WCharCP filename : s_alexGSpirals)
        {
        bvector<IGeometryPtr> geometryFromFile;
        ReadDgnjsGeometry(geometryFromFile, 1, L"spirals", L"AlexG0421", filename);
        Check::True (geometryFromFile.size () > 0);
        Check::SaveTransformed(geometryFromFile);
        for (auto &g : geometryFromFile)
            {
            DoRoundTrip(g, false, 0);
            DoRoundTrip(g, false, 1);
            DoRoundTrip(g, false, 2);
            CurveVectorPtr cv = g->GetAsCurveVector ();
            if (Check::True (cv.IsValid (), "Expect curve vector in alignment file"))
                {
                Check::Size (9, cv->size(), "Expected primitives in alignment with 3 arc-spiral-arc parts");
                double maxGap = cv->MaxGapWithinPath();
                Check::LessThanOrEqual (maxGap, 1.0e-7, "maxGap in alignment");
                }
            }
        }
    Check::ClearGeometry("Spiral.ReadAlignments");
    }




