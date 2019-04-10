/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/GeomTestHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "GeomTestHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomTestFixture::GetGeometryCount(GeometryCollection& gel)
    {
    //  Verify that item generated a line
    size_t count = 0;
    for (auto geom : gel)
        {
        ++count;
        }
    return count;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GeomTestFixture::GeometryTypeToString(GeometricPrimitive::GeometryType type)
    {
    switch (type)
        {
        case GeometricPrimitive::GeometryType::BsplineSurface:
            return "BsplineSurface";
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            return "CurvePrimitive";
        case GeometricPrimitive::GeometryType::CurveVector:
            return "CurveVector";
        case GeometricPrimitive::GeometryType::Polyface:
            return "Polyface";
        case GeometricPrimitive::GeometryType::BRepEntity:
            return "BRepEntity";
        case GeometricPrimitive::GeometryType::SolidPrimitive:
            return "SolidPrimitive";
        case GeometricPrimitive::GeometryType::TextString:
            return "TextString";
        default:
            return "Invalid";
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomTestFixture::VerifyElement(DgnDbR db, DgnV8Api::ElementId eV8Id, GeometricPrimitive::GeometryType expectedGeomType)
    {
    DgnElementCPtr elem1 = FindV8ElementInDgnDb(db, eV8Id);
    ASSERT_TRUE(elem1.IsValid());
    GeometryCollection geomCollection(*elem1->ToGeometrySource());
    ASSERT_TRUE(GetGeometryCount(geomCollection) >= 1);
    GeometricPrimitivePtr geom = (geomCollection.begin()).GetGeometryPtr();
    ASSERT_TRUE(geom.IsValid());
    ASSERT_TRUE(expectedGeomType == geom->GetGeometryType()) << "Expected Type : " << GeometryTypeToString(expectedGeomType).c_str() << "\nActual Type : " << GeometryTypeToString(geom->GetGeometryType()).c_str();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomTestFixture::VerifyCellElement(DgnDbR db, DgnV8Api::ElementId eV8Id, int32_t expectedChildCount)
    {
    DgnElementCPtr elem1 = FindV8ElementInDgnDb(db, eV8Id);
    ASSERT_TRUE(elem1.IsValid());
    DgnElementIdSet children = elem1->QueryChildren();
    EXPECT_EQ(expectedChildCount, (int32_t)children.size())<<"Number of child in cell are not equal";
    }
