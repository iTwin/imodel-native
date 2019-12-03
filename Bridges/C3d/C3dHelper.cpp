/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dImporter.h"
#include "C3dHelper.h"

BEGIN_C3D_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dHelper::GetLinearCurves (CurveVectorR curves, GeometrySourceCP source)
    {
    // extract  from element created by the base importer and add it to the new Civil element
    if (nullptr == source)
        return  BentleyStatus::BSIERROR;

    ICurvePrimitivePtr  curvePrimitive;
    CurveVectorPtr  curveVector;

    GeometryCollection  sourceGeoms(*source);
    for (auto entry : sourceGeoms)
        {
        auto geometry = entry.GetGeometryPtr ();
        if (!geometry.IsValid())
            return  BentleyStatus::BSIERROR;

        // filter in linear geometries and add them to output collection
        if ((curvePrimitive = geometry->GetAsICurvePrimitive()).IsValid())
            curves.Add (curvePrimitive);
        else if ((curveVector = geometry->GetAsCurveVector()).IsValid())
            curves.Add (curveVector);
        else
            return  BentleyStatus::BSIERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dHelper::CopyGeometrySource (GeometricElement3dP target, GeometrySourceCP source)
    {
    // extract geometry from element created by the base importer and add it to the new Civil element
    if (nullptr == target || nullptr == source)
        return  BentleyStatus::BSIERROR;

    auto model = target->GetModel ();
    GeometryCollection sourceGeoms (*source);
    if (sourceGeoms.begin() == sourceGeoms.end() || !model.IsValid())
        return  BentleyStatus::BSIERROR;
        
    DPoint3d    origin;
    YawPitchRollAngles angles;
    if (source->Is2d())
        {
        auto& placement = source->GetAsGeometrySource2d()->GetPlacement ();
        origin.Init (placement.GetOrigin());
        angles.SetYaw (placement.GetAngle());
        }
    else
        {
        auto& placement = source->GetAsGeometrySource3d()->GetPlacement ();
        origin = placement.GetOrigin ();
        angles = placement.GetAngles ();
        }

    auto categoryId = target->GetCategoryId ();
    auto builder = GeometryBuilder::Create (*model, categoryId, origin, angles);
    if (builder.IsValid())
        {
        size_t  count = 0;
        for (auto entry : sourceGeoms)
            {
            auto primitiveGeom = entry.GetGeometryPtr ();
            if (primitiveGeom.IsValid())
                {
                auto params = entry.GetGeometryParams ();
                params.SetCategoryId (categoryId);

                // WIP - create/get sub-category for a civil model?
                params.SetSubCategoryId (DgnCategory::GetDefaultSubCategoryId(categoryId));

                if (builder->Append(params) && builder->Append(*primitiveGeom))
                    count++;
                }
            }
        if (count >= 0 && BentleyStatus::BSISUCCESS == builder->Finish(*target))
            return BentleyStatus::BSISUCCESS;
        }

    return BentleyStatus::BSIERROR;
    }

END_C3D_NAMESPACE
