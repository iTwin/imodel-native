#include "PublicApi/DrivingSurface.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>


BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (DrivingSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params) 
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    Placement3d newPlacement ((*(*surfaceVector->begin ())->GetLineStringCP ())[0], GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*surfaceVector, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create DrivingSurface Geometry");
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurface::DrivingSurface
(
CreateParams const& params,
ISolidPrimitivePtr  surface
) : T_Super(params) 
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP ();

    DgnExtrusionDetail extrDetail;
    surface->TryGetDgnExtrusionDetail (extrDetail);

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin();
    DPoint3d originPoint;

    bvector<DPoint3d> const* lineString;
    DEllipse3dCP arc;
    MSBsplineCurveCP spline;

    if (nullptr != (lineString = curve->GetLineStringCP()))
        originPoint = (*lineString)[0];
    else if (nullptr != (arc = curve->GetArcCP()))
        originPoint = arc->center;
    else if (nullptr != (spline = curve->GetBsplineCurveCP()))
        originPoint = spline->GetPole(0);
    else
        BeAssert(!"Unrecognized DrivingSurface Base Curve");


    Placement3d newPlacement (originPoint, GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*surface, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create DrivingSurface Geometry");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           DrivingSurface::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb().GetDictionaryModel (), GRIDS_CATEGORY_CODE_DrivingSurface);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurfacePtr                 DrivingSurface::Create 
(
Dgn::DgnModelCR model,
CurveVectorPtr  surfaceVector
)
    {
    return new DrivingSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surfaceVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DrivingSurfacePtr                 DrivingSurface::Create 
(
Dgn::DgnModelCR model,
ISolidPrimitivePtr surface
)
    {
    return new DrivingSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr                  DrivingSurface::GetSurfaceVector
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();
    CurveVectorPtr curveVec = (*(geomData.begin ())).GetGeometryPtr ()->GetAsCurveVector ();
    curveVec->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());

    return curveVec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            DrivingSurface::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

END_GRIDS_NAMESPACE