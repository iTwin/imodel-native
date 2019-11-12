/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "stdafx.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
CurveVectorPtr ShapeTools::GetRectSolidShape(double width, double depth)
    {
    DPoint3d points[4];
    points[0] = DPoint3d::From(0.0, 0.0, 0.0);
    points[1] = DPoint3d::From(width, 0.0, 0.0);
    points[2] = DPoint3d::From(width, depth, 0.0);
    points[3] = DPoint3d::From(0.0, depth, 0.0);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    return shape;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
CurveVectorPtr ShapeTools::GetHSSRectShape(double width, double depth)
    {
    // NOTE: This method generates a half of the shape as a "hack" to render the full shape
    double wallThickness = (width + depth) / 16;

    DPoint3d points[8];
    points[0] = DPoint3d::From(0.0, 0.0, 0.0);
    points[1] = DPoint3d::From(width / 2, 0.0, 0.0);
    points[2] = DPoint3d::From(width / 2, wallThickness, 0.0);
    points[3] = DPoint3d::From(wallThickness, wallThickness, 0.0);

    points[4] = DPoint3d::From(wallThickness, depth - wallThickness, 0.0);
    points[5] = DPoint3d::From(width / 2, depth - wallThickness, 0.0);
    points[6] = DPoint3d::From(width / 2, depth, 0.0);
    points[7] = DPoint3d::From(0.0, depth, 0.0);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    return shape;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
CurveVectorPtr ShapeTools::GetIShape(double width, double depth)
    {
    // TODO: Convert to function parameter
    double flangeThickness = depth / 8;
    double webThickness = width / 4;

    DPoint3d points[12];
    points[0] = DPoint3d::From(0.0, 0.0, 0.0);
    points[1] = DPoint3d::From(width, 0.0, 0.0);

    points[2] = DPoint3d::From(width, flangeThickness, 0.0);
    points[3] = DPoint3d::From(width / 2 + webThickness / 2, flangeThickness, 0.0);

    points[4] = DPoint3d::From(width / 2 + webThickness / 2, depth - flangeThickness, 0.0);
    points[5] = DPoint3d::From(width, depth - flangeThickness, 0.0);

    points[6] = DPoint3d::From(width, depth, 0.0);
    points[7] = DPoint3d::From(0.0, depth, 0.0);

    points[8] = DPoint3d::From(0.0, depth - flangeThickness, 0.0);
    points[9] = DPoint3d::From(width / 2 - webThickness / 2, depth - flangeThickness, 0.0);

    points[10] = DPoint3d::From(width / 2 - webThickness / 2, flangeThickness, 0.0);
    points[11] = DPoint3d::From(0.0, flangeThickness, 0.0);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    return shape;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
CurveVectorPtr ShapeTools::GetLShape(double xFlangeSize, double yFlangeSize)
    {
    // TODO: Convert to function parameter
    double flangeThickness = (xFlangeSize + yFlangeSize) / 24;

    DPoint3d points[6];
    points[0] = DPoint3d::From(0.0, 0.0, 0.0);
    points[1] = DPoint3d::From(xFlangeSize, 0.0, 0.0);

    points[2] = DPoint3d::From(xFlangeSize, yFlangeSize, 0.0);
    points[3] = DPoint3d::From(xFlangeSize - flangeThickness, yFlangeSize, 0.0);

    points[4] = DPoint3d::From(xFlangeSize - flangeThickness, flangeThickness, 0.0);
    points[5] = DPoint3d::From(0.0, flangeThickness, 0.0);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    return shape;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::AppendMemberToBuilder(Dgn::GeometryBuilderPtr builder, PhysicalProperties* properties, Transform rotationMatrix, Transform linearMatrix)
    {
    // Special handling for HSS shape; we are using a "hack" to join to identical halves of the shape
    if (properties->GetShape() == ShapeTools::Shape::HSSRectangle)
        {
        // Get member shapes
        CurveVectorPtr shapeHalfLeft = ShapeTools::GetHSSRectShape(properties->GetXDimension(), properties->GetYDimension());
        if (!shapeHalfLeft.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        CurveVectorPtr shapeHalfRight = ShapeTools::GetHSSRectShape(properties->GetXDimension(), properties->GetYDimension());
        if (!shapeHalfRight.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        // Get members "length"
        DVec3d vec = DVec3d::From(0.0, 0.0, properties->GetZDimension());

        // Create members
        ISolidPrimitivePtr memberLeft = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shapeHalfLeft, vec, true));
        if (!memberLeft.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        ISolidPrimitivePtr memberRight = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shapeHalfRight, vec, true));
        if (!memberRight.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        // Transform right member
        Transform rotationMatrixR;
        rotationMatrixR.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(-1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
        memberRight->TransformInPlace(rotationMatrixR);

        Transform linearMatrixR;
        linearMatrixR.InitFromOriginAndVectors(DPoint3d::From(properties->GetXDimension(), 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
        memberRight->TransformInPlace(linearMatrixR);

        // Transform members as needed
        memberLeft->TransformInPlace(rotationMatrix);
        memberLeft->TransformInPlace(linearMatrix);

        memberRight->TransformInPlace(rotationMatrix);
        memberRight->TransformInPlace(linearMatrix);

        // Append members
        if (!builder->Append(*memberLeft))
            {
            return BentleyStatus::BSIERROR;
            }

        if (!builder->Append(*memberRight))
            {
            return BentleyStatus::BSIERROR;
            }
        }
    else
        {
        // Get member shape
        CurveVectorPtr shape;
        switch (properties->GetShape())
            {
            case ShapeTools::Shape::I:
                shape = ShapeTools::GetIShape(properties->GetXDimension(), properties->GetYDimension());
                break;
            case ShapeTools::Shape::L:
                shape = ShapeTools::GetLShape(properties->GetXDimension(), properties->GetYDimension());
                break;
            case ShapeTools::Shape::Rectangle:
            default:
                shape = ShapeTools::GetRectSolidShape(properties->GetXDimension(), properties->GetYDimension());
                break;
            }
        if (!shape.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        // Get member "length"
        DVec3d vec = DVec3d::From(0.0, 0.0, properties->GetZDimension());

        // Create member
        ISolidPrimitivePtr member = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));
        if (!member.IsValid())
            {
            return BentleyStatus::BSIERROR;
            }

        // Transform member as needed
        member->TransformInPlace(rotationMatrix);
        member->TransformInPlace(linearMatrix);

        // Append member
        if (!builder->Append(*member))
            {
            return BentleyStatus::BSIERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateStructuralMemberGeometry(
    Dgn::PhysicalElementPtr element,
    BentleyApi::Structural::StructuralPhysicalModelR model,
    ECN::ECSchemaCP schema,
    PhysicalProperties* properties,
    Transform rotationMatrix,
    Transform linearMatrix)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    Dgn::DgnCategoryId categoryId = BentleyApi::Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());

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

    if (BentleyStatus::SUCCESS != AppendMemberToBuilder(builder, properties, rotationMatrix, linearMatrix))
        {
        return BentleyStatus::BSIERROR;
        }

    if (BentleyStatus::SUCCESS != builder->Finish(*element))
        {
        return BentleyStatus::BSIERROR;
        }

    return BentleyStatus::SUCCESS;
    }

