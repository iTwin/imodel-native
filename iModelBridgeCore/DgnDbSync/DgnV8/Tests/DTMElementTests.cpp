/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/DgnPlatform/LevelTypes.h>
#include "GeomTestHelper.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      02/16
//----------------------------------------------------------------------------------------
struct DTMElementTests : public GeomTestFixture
{
    DEFINE_T_SUPER(GeomTestFixture);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DTMElementTests, TerrainModel)
    {
    m_params.SetDoTerrainModelConversion(true);

    LineUpFiles(L"dtm.bim", L"dtm.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1 = 0;
    for (auto el : *v8editor.m_defaultModel->GetGraphicElementsP())
        {
        if (el->GetElementType() == 106)
            {
            eid1 = el->GetElementId();
                break;
            }
        }
    ////v8editor.AddLine(&eid1);
    //v8editor.Save();
    

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
//    VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

    DgnElementCPtr elem1 = FindV8ElementInDgnDb(*db, eid1);
    ASSERT_TRUE(elem1.IsValid());
    ASSERT_TRUE(Utf8String(elem1->GetUserLabel()) == "aussie-met");

    auto children = elem1->QueryChildren();

    ASSERT_TRUE(!children.empty());

    struct FeatureDetails
        {
        Utf8String name;
        int numGeometry;
        GeometricPrimitive::GeometryType expectedGeomType;
        int color;
        bool found = false;

        FeatureDetails(Utf8CP name, int numGeometry, GeometricPrimitive::GeometryType expectedGeomType, int color) : name(name), numGeometry(numGeometry), expectedGeomType(expectedGeomType), color(color)
            { }
        };

    bvector<FeatureDetails> features;

    features.push_back(FeatureDetails("Boundary", 1, GeometricPrimitive::GeometryType::CurvePrimitive, 0xff0000));
    features.push_back(FeatureDetails("Mesh", 1, GeometricPrimitive::GeometryType::Polyface, 0xffff00));
    features.push_back(FeatureDetails("Breakline", 318, GeometricPrimitive::GeometryType::CurvePrimitive, 0x00ff00));
    features.push_back(FeatureDetails("Void", 12, GeometricPrimitive::GeometryType::CurvePrimitive, 0x0000ff));
    features.push_back(FeatureDetails("FeatureSpots", 1, GeometricPrimitive::GeometryType::CurvePrimitive, 0x00ffff));

    auto range = elem1->ToGeometrySource3d()->CalculateRange3d();
    for (auto childId : children)
        {
        auto child = db->Elements().GetElement(childId);
        FeatureDetails* feature = nullptr;

        for (auto&& f : features)
            {
            if (f.name == child->GetUserLabel())
                {
                feature = &f;
                break;
                }
            }
        ASSERT_TRUE(feature != nullptr);

        feature->found = true;

        GeometryCollection geomCollection(*child->ToGeometrySource());
        auto count = GetGeometryCount(geomCollection);
        ASSERT_TRUE(count == feature->numGeometry);

        // ToDo test subCategory
        for (auto geomEntry : geomCollection)
            {
            auto lineCol = geomEntry.GetGeometryParams().GetLineColor();
            ASSERT_TRUE(lineCol.GetValue() == feature->color);

//            auto subCat = geomEntry.GetGeometryParams().GetSubCategoryId();

            auto geom = geomEntry.GetGeometryPtr();
            ASSERT_TRUE(geom.IsValid());

            ASSERT_TRUE(feature->expectedGeomType == geom->GetGeometryType()) << "Expected Type : " << GeometryTypeToString(feature->expectedGeomType).c_str() << "\nActual Type : " << GeometryTypeToString(geom->GetGeometryType()).c_str();
            }
        }

    for (auto&& f : features)
        {
        ASSERT_TRUE(f.found);
        }


    }
