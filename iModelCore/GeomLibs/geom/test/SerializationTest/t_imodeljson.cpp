/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

#include "SampleGeometry.h"
#include "FileOps.h"
#include <GeomSerialization/GeomSerializationApi.h>


bool ReadIModelJson (bvector<IGeometryPtr> &geometry,
    WCharCP inputDirectory, 
    WCharCP outputDirectory, 
    WCharCP nameB, WCharCP nameC, WCharCP extension)
    {
    geometry.clear ();
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    dataPath.AppendToPath (L"GeomLibsTestData");
    if (inputDirectory)
        dataPath.AppendToPath (inputDirectory);
    if (nameB)
        dataPath.AppendToPath (nameB);
    if (nameC)
        dataPath.AppendToPath (nameC);
    if (extension)
        dataPath.AppendExtension (extension);
    bool stat = false;
    Check::StartScope ("ReadIModelJson");
    Utf8String string;
    printf (" ReadIModelJson file %ls\n", dataPath.c_str ());

    if (Check::True (GTestFileOps::ReadAsString (dataPath, string), "Read file to string"))
        {
        if (Check::True (IModelJson::TryIModelJsonStringToGeometry (string, geometry), "json string to geometry"))
            {
            Utf8String stringB;
            if (IModelJson::TryGeometryToIModelJsonString (stringB, geometry))
                GTestFileOps::WriteToFile (stringB, L"IModelJsonFromNative", nameB, nameC, extension);
            stat = true;
            }
        else
            printf (" Failed file \n%ls\n", dataPath.c_str ());
        }
    Check::EndScope();
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelJson,ReadFiles)
    {
    for (auto & name : 
        bvector<WCharCP> {
            L"arc",
            L"lineSegment",
            L"lineString",
            L"pointString",
            L"path",
            L"loop",
            L"parityRegion",
            L"bcurve",
            L"cylinder",
            L"cone",
            L"box",
            L"sphere",
            L"torusPipe",
            L"linearSweep",
            L"rotationalSweep",
            L"ruledSweep",
            L"bsurf",
            L"indexedMesh",
            })
        {
        bvector<IGeometryPtr> geometry;
        if (ReadIModelJson (geometry, L"IModelJson", L"IModelJsonFromNative", name, nullptr, L"imjs"))
            {
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelJson,LineSegmentInCurveCollection)
    {
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    cv->Add (ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 4,5,6)));
    Check::SaveTransformed (*cv);
    Check::ClearGeometry ("IModelJson.LineSegmentInCurveCollection");
    }