
#include <Grids/Elements/GridElementsAPI.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

#define BUBBLE_RADIUS 1.5

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridCurve)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
(
CreateParams const& params
) : T_Super(params) 
    {
    SetBubbleAtStart (false);
    SetBubbleAtEnd (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params) 
    {
    InitGeometry (curve);
    SetBubbleAtStart (false);
    SetBubbleAtEnd (false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurve::GridCurve
(
CreateParams const& params,
CurveVectorPtr  curve
) : T_Super(params) 
    {
    InitGeometry (curve);
    SetBubbleAtStart (false);
    SetBubbleAtEnd (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           GridCurve::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridCurve);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                02/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridCurve::CheckDependancyToModel
(
) const 
{ 
    DgnModelPtr model = GetModel();
    GridCurvesSetCPtr portion = GetDgnDb().Elements().Get<GridCurvesSet>(model->GetModeledElementId());
    if (portion.IsNull()) //pointer must not be null
        return DgnDbStatus::ValidationFailed;

    return DgnDbStatus::Success;
}


//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurve::_OnInsert()
    {
    DgnDbStatus status = CheckDependancyToModel();
    if (status != DgnDbStatus::Success)
        return status;
    if (!_ValidateGeometry(GetCurve()))
        return DgnDbStatus::ValidationFailed;
    return T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  12/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurve::_OnUpdate(Dgn::DgnElementCR original)
    {
    DgnDbStatus status = CheckDependancyToModel();
    if (status != DgnDbStatus::Success)
        return status;
    InitGeometry (GetCurve());
    if (!_ValidateGeometry(GetCurve()))
        return DgnDbStatus::ValidationFailed;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              03/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<Dgn::DgnElementId> GridCurve::GetIntersectingSurfaceIds() const
    {
    return GridCurveBundle::MakeDrivingSurfaceIterator(*this).BuildIdList<Dgn::DgnElementId>();
    }

#pragma region Bubble Geometry

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DVec3d getDirectionAtEndPoint (ICurvePrimitiveCR primitive, bool atStart)
    {
    // FractionToPointAndUnitTangent doesn't work on child curve vector, so extract very first leaf curve primitive
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == primitive.GetCurvePrimitiveType())
        {
        CurveVectorCPtr curve = primitive.GetChildCurveVectorCP();
        if (curve->empty() || curve->at (0).IsNull())
            return DVec3d::FromZero();

        return getDirectionAtEndPoint (*curve->at (0), atStart);
        }

    ValidatedDRay3d pointAndTangent = primitive.FractionToPointAndUnitTangent (atStart ? 0.0 : 1.0);
    DVec3d direction = pointAndTangent.IsValid() ? pointAndTangent.Value().direction : DVec3d::FromZero();

    if (!atStart)
        direction.Negate();

    return direction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getBubbleOrigin (DPoint3dR origin, CurveVectorCR curve, double bubbleRadius, bool atStart)
    {
    if (curve.empty() || curve.at (0).IsNull())
        return false;

    // Put bubble so that it touches the curve start point
    // And its center-to-curveStart direction is in the same as beginning of the curve.
    DPoint3d start, end;
    if (!curve.GetStartEnd(start, end))
        return false;

    DPoint3d point = atStart ? start : end;
    DVec3d direction = getDirectionAtEndPoint (*curve.at(0), atStart);
    if (direction.IsZero())
        return false;

    direction.ScaleToLength (bubbleRadius);
    point.Subtract (direction);
    origin = point;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool addBubble (Dgn::GeometryBuilderPtr& builder, TextString labelGeometry, ICurvePrimitiveCR bubbleGeometry, CurveVectorCR curve, TransformCR toWorld, bool atStart)
    {
    DPoint3d origin;
    if (!getBubbleOrigin (origin, curve, bubbleGeometry.GetArcCP()->vector0.Magnitude(), atStart))
        return false;
    
    // Bubble position is at world coordinates 
    // and geometry stream is at local coordinates 
    // so bubble and label geometry needs to be adjusted.
    Transform toLocal;
    if (!toLocal.InverseOf (toWorld))
        return false;

    toLocal.Multiply (origin);
    ICurvePrimitivePtr bubbleGeometryCopy = bubbleGeometry.Clone();
    bubbleGeometryCopy->TransformInPlace (Transform::From (origin));
    labelGeometry.SetOriginFromJustificationOrigin (origin, TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);
    
    builder->SetAppendAsSubGraphics();
    return builder->Append (labelGeometry) && builder->Append (*bubbleGeometryCopy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUpBubbleAndLabelGeometry (TextStringPtr& labelGeometry, ICurvePrimitivePtr& bubbleGeometry, Utf8CP text)
    {
    TextStringStylePtr labelTextStyle = TextStringStyle::Create();
    labelTextStyle->SetWidth (BUBBLE_RADIUS / 2);
    labelTextStyle->SetHeight (BUBBLE_RADIUS / 2);

    labelGeometry = TextString::Create();
    labelGeometry->SetText (text);
    labelGeometry->SetStyle (*labelTextStyle);

    bubbleGeometry = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::FromZero(), BUBBLE_RADIUS));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool addBubbles (Dgn::GeometryBuilderPtr& builder, CurveVectorCR geometry, TransformCR toWorld, Utf8CP label, bool addAtStart, bool addAtEnd)
    {
    TextStringPtr labelGeometry;
    ICurvePrimitivePtr bubbleGeometry;
    setUpBubbleAndLabelGeometry (labelGeometry, bubbleGeometry, label);

    if (addAtStart && !addBubble (builder, *labelGeometry, *bubbleGeometry, geometry, toWorld, true))
        return false;

    if (addAtEnd && !addBubble (builder, *labelGeometry, *bubbleGeometry, geometry, toWorld, false))
        return false;

    return true;
    }

#pragma endregion Bubble Geometry

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::InitGeometry
(
CurveVectorPtr  curve
)
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP();

    DPoint3d originPoint;
    (*curve)[0]->GetStartPoint (originPoint);

    Placement3d newPlacement (originPoint, GetPlacement().GetAngles());
    SetPlacement (newPlacement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create (*geomElem);

    if (builder->Append (*curve, Dgn::GeometryBuilder::CoordSystem::World))
        {
        Utf8CP label = GetUserLabel();
        if (!Utf8String::IsNullOrEmpty(label))
            addBubbles (builder, *curve, newPlacement.GetTransform(), label, GetBubbleAtStart(), GetBubbleAtEnd());
        if (SUCCESS != builder->Finish (*geomElem))
            BeAssert (!"Failed to create IntersectionCurve Geometry");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::InitGeometry
(
ICurvePrimitivePtr  curve
)
    {
    return InitGeometry (CurveVector::Create (curve));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::SetCurve
(
ICurvePrimitivePtr  curve
)
    {
    //clean the existing geometry
    GetGeometryStreamR().Clear();
    InitGeometry (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridCurve::SetCurve
(
CurveVectorPtr  curve
)
    {
    //clean the existing geometry
    GetGeometryStreamR().Clear();
    InitGeometry (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr                  GridCurve::GetCurve
(
) const
    {
    GeometryCollection geomData = *ToGeometrySource();
    ICurvePrimitivePtr curve = nullptr;
    GeometricPrimitivePtr geometricPrimitivePtr = (*(geomData.begin())).GetGeometryPtr();
    if (geometricPrimitivePtr.IsValid())
        {
        switch (geometricPrimitivePtr->GetGeometryType())
            {
            case GeometricPrimitive::GeometryType::CurvePrimitive:
                curve = geometricPrimitivePtr->GetAsICurvePrimitive();
                break;
            case GeometricPrimitive::GeometryType::CurveVector:
                {
                CurveVectorPtr curveVector = geometricPrimitivePtr->GetAsCurveVector();
                if (1 == curveVector->size())
                    curve = curveVector->at (0);
                else
                    curve = ICurvePrimitive::CreateChildCurveVector (curveVector);
                }
            break;
            default:
                return nullptr;
            }
        curve->TransformInPlace ((*geomData.begin()).GetGeometryToWorld());
        }

    return curve;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurve::GeneralGridCurve
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurve::GeneralGridCurve
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridCurvePtr                 GeneralGridCurve::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GeneralGridCurve (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

END_GRIDS_NAMESPACE
