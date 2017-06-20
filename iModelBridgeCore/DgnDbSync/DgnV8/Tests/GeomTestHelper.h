/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/GeomTestHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Tests.h"
#include "ConverterTestsBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                          03/16
//----------------------------------------------------------------------------------------
struct GeomTestFixture : public ConverterTestBaseFixture
{
DEFINE_T_SUPER(ConverterTestBaseFixture);

protected:
    size_t GetGeometryCount(GeometryCollection& gel);
    Utf8String GeometryTypeToString(GeometricPrimitive::GeometryType type);
    void VerifyElement(DgnDbR db, DgnV8Api::ElementId eV8Id, GeometricPrimitive::GeometryType expectedGeomType);
    void VerifyCellElement(DgnDbR db, DgnV8Api::ElementId eV8Id, int32_t expectedChildCount);
};

