/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/GeometricTools.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "GeometricTools.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateStructuralMemberGeometry(
    Dgn::PhysicalElementPtr element,
    StructuralPhysical::StructuralPhysicalModelR model,
    ECN::ECSchemaCP schema,
    StructuralMemberGeometricProperties* properties)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    Dgn::DgnCategoryId categoryId = Concrete::ConcreteCategory::QueryStructuralPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());
    if (schema->GetName() == BENTLEY_CONCRETE_SCHEMA_NAME)
        {
        //categoryId = Concrete::ConcreteCategory::QueryStructuralPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());
        }
    else if (schema->GetName() == BENTLEY_STEEL_SCHEMA_NAME)
        {
        categoryId = Steel::SteelCategory::QueryStructuralPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());
        }

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*element);
    if (!builder.IsValid())
        {
        return BentleyStatus::BSIERROR;
        }

    // Append geometry/params for tile casing

    builder->Append(categoryId);

    Dgn::Render::GeometryParams params;
    params.SetCategoryId(categoryId);

    params.SetLineColor(properties->GetLineColor());
    params.SetFillColor(properties->GetFillColor());
    builder->Append(params);

    DPoint3d points[4];
    points[0] = DPoint3d::From(0.0, 0.0, 0.0);
    points[1] = DPoint3d::From(properties->GetXDimension(), 0.0, 0.0);
    points[2] = DPoint3d::From(properties->GetXDimension(), properties->GetYDimension(), 0.0);
    points[3] = DPoint3d::From(0.0, properties->GetYDimension(), 0.0);

    DVec3d vec = DVec3d::From(0.0, 0.0, properties->GetZDimension());

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        {
        return BentleyStatus::BSIERROR;
        }

    ISolidPrimitivePtr panel = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));
    if (!panel.IsValid())
        {
        return BentleyStatus::BSIERROR;
        }

    builder->Append(*panel);

    if (BentleyStatus::SUCCESS != builder->Finish(*element))
        {
        return BentleyStatus::BSIERROR;
        }

    return BentleyStatus::SUCCESS;
    }
