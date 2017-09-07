#include "PublicApi/GridPlaneSurface.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridPlaneSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlaneSurface::GridPlaneSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlaneSurface::GridPlaneSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params, surfaceVector) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlaneSurface::GridPlaneSurface
(
CreateParams const& params,
ISolidPrimitivePtr surface
) : T_Super(params, surface) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlaneSurfacePtr             GridPlaneSurface::Create 
(
Dgn::SpatialLocationModelCR model,
CurveVectorPtr  surfaceVector
)
    {
    return new GridPlaneSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surfaceVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPlaneSurfacePtr             GridPlaneSurface::Create 
(
Dgn::SpatialLocationModelCR model,
ISolidPrimitivePtr surface
)
    {
    return new GridPlaneSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d                        GridPlaneSurface::GetPlane
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource ();

    if (geomData.begin () == geomData.end ())
        return DPlane3d ();

    ISolidPrimitivePtr solidPrimitive = (*geomData.begin ()).GetGeometryPtr ()->GetAsISolidPrimitive ();
    if (!solidPrimitive.IsValid ())
        {
        CurveVectorPtr curveVector = (*geomData.begin()).GetGeometryPtr()->GetAsCurveVector();
        if (!curveVector.IsValid())
            return DPlane3d();

        curveVector->TransformInPlace((*geomData.begin()).GetGeometryToWorld());

        Transform localToWorld, worldToLocal;
        DRange3d range;
        curveVector->IsPlanar(localToWorld, worldToLocal, range);
        DPlane3d retPlane;
        bsiTransform_getOriginAndVectors(&localToWorld, &retPlane.origin, NULL, NULL, &retPlane.normal);
        return retPlane;
        }

    DgnExtrusionDetail extDetail;
    solidPrimitive->TransformInPlace ((*geomData.begin ()).GetGeometryToWorld ());
    if (!solidPrimitive->TryGetDgnExtrusionDetail (extDetail))
        {
        return DPlane3d ();
        }

    bvector<bvector<DPoint3d>> baseShapePoints;
    extDetail.m_baseCurve->CollectLinearGeometry (baseShapePoints);
    if (baseShapePoints.size () < 1 || baseShapePoints[0].size () < 2)
        {
        return DPlane3d ();
        }

    DPoint3d point3 = DPoint3d::FromSumOf (baseShapePoints[0][0], extDetail.m_extrusionVector);
    return DPlane3d::From3Points (baseShapePoints[0][0], baseShapePoints[0][1], point3);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                        GridPlaneSurface::SetCurveVector
(
CurveVectorR newShape
)
    {
    //clean the existing geometry
    GetGeometryStreamR ().Clear ();


    Transform localToWorld, worldToLocal;
    DRange3d range;
    newShape.IsPlanar (localToWorld, worldToLocal, range);
    DPlane3d retPlane;
    bsiTransform_getOriginAndVectors (&localToWorld, &retPlane.origin, NULL, NULL, &retPlane.normal);

    Placement3d newPlacement (retPlane.origin, GetPlacement ().GetAngles ());
    SetPlacement (newPlacement);

    GeometrySource3dP pGeomElem = DgnElement::ToGeometrySource3dP ();

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*pGeomElem);

    builder->Append (newShape, Dgn::GeometryBuilder::CoordSystem::World);
    builder->Finish (*pGeomElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            GridPlaneSurface::GetGeomIdPlane 
(
    int geomId, 
    DPlane3dR planeOut
) const
    {
    if (geomId != 0 && geomId != 1)
        return false;

    planeOut = GetPlane();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            GridPlaneSurface::StretchGeomIdToPlane
(
    int geomId, 
    DPlane3dR targetPlane
)
    {
    BeAssert(!"Not yet implemented");
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridPlaneSurfacePtr GridPlaneSurface::Create(Dgn::SpatialLocationModelCR model, DgnExtrusionDetail extDetail)
    {
    return GridPlaneSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }

END_GRIDS_NAMESPACE

