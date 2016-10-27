/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Camera.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(CameraHandler)

#define CAMERA_PROPNAME_FocalLenghtPixels    "FocalLenghtPixels"
#define CAMERA_PROPNAME_ImageDimension       "ImageDimension"
#define CAMERA_PROPNAME_ImageDimension_Width "Width"
#define CAMERA_PROPNAME_ImageDimension_Height "Height"
#define CAMERA_PROPNAME_PrincipalPoint       "PrincipalPoint"
#define CAMERA_PROPNAME_Distortion           "Distortion"
#define CAMERA_PROPNAME_Distortion_K1        "K1"
#define CAMERA_PROPNAME_Distortion_K2        "K2"
#define CAMERA_PROPNAME_Distortion_K3        "K3"
#define CAMERA_PROPNAME_Distortion_P1        "P1"
#define CAMERA_PROPNAME_Distortion_P2        "P2"
#define CAMERA_PROPNAME_AspectRatio          "AspectRatio"
#define CAMERA_PROPNAME_Skew                 "Skew"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus ImageDimensionType::BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, ImageDimensionTypeCR val)
    {
    if (!val.IsValid())
        {
        statement.BindNull(columnIndex);
        return BeSQLite::EC::ECSqlStatus::Error;
        }

    IECSqlStructBinder& binder = statement.BindStruct(columnIndex);
    BeSQLite::EC::ECSqlStatus status;
    status = binder.GetMember(CAMERA_PROPNAME_ImageDimension_Width).BindInt(val.GetWidth());
    BeAssert(status == ECSqlStatus::Success);
    status = binder.GetMember(CAMERA_PROPNAME_ImageDimension_Height).BindInt(val.GetHeight());
    BeAssert(status == ECSqlStatus::Success);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ImageDimensionType ImageDimensionType::GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex)
    {
    if (statement.IsValueNull(columnIndex))
        return ImageDimensionType();

    ImageDimensionType imageDimension;
    IECSqlStructValue const& imageDimensionValue = statement.GetValueStruct(columnIndex);
    for (int ii = 0; ii < imageDimensionValue.GetMemberCount(); ii++)
        {
        IECSqlValue const& memberValue = imageDimensionValue.GetValue(ii);
        ECPropertyCP memberProperty = memberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProperty != nullptr);
        Utf8CP memberName = memberProperty->GetName().c_str();

        if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_ImageDimension_Width, memberName))
            imageDimension.SetWidth(memberValue.GetInt());
        else if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_ImageDimension_Height, memberName))
            imageDimension.SetHeight(memberValue.GetInt());
        else
            BeAssert(false);
        }
    return imageDimension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECSqlStatus CameraDistortionType::BindParameter(BeSQLite::EC::ECSqlStatement& statement, uint32_t columnIndex, CameraDistortionTypeCR val)
    {
    if (!val.IsValid())
        {
        statement.BindNull(columnIndex);
        return BeSQLite::EC::ECSqlStatus::Error;
        }

    IECSqlStructBinder& binder = statement.BindStruct(columnIndex);
    BeSQLite::EC::ECSqlStatus status;
    status = binder.GetMember(CAMERA_PROPNAME_Distortion_K1).BindDouble(val.GetK1());
    BeAssert(status == ECSqlStatus::Success);
    status = binder.GetMember(CAMERA_PROPNAME_Distortion_K2).BindDouble(val.GetK2());
    BeAssert(status == ECSqlStatus::Success);
    status = binder.GetMember(CAMERA_PROPNAME_Distortion_K3).BindDouble(val.GetK3());
    BeAssert(status == ECSqlStatus::Success);
    status = binder.GetMember(CAMERA_PROPNAME_Distortion_P1).BindDouble(val.GetP1());
    BeAssert(status == ECSqlStatus::Success);
    status = binder.GetMember(CAMERA_PROPNAME_Distortion_P2).BindDouble(val.GetP2());
    BeAssert(status == ECSqlStatus::Success);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDistortionType CameraDistortionType::GetValue(BeSQLite::EC::ECSqlStatement const& statement, uint32_t columnIndex)
    {
    if (statement.IsValueNull(columnIndex))
        return CameraDistortionType();

    CameraDistortionType Distortion;
    IECSqlStructValue const& structValue = statement.GetValueStruct(columnIndex);
    for (int ii = 0; ii < structValue.GetMemberCount(); ii++)
        {
        IECSqlValue const& memberValue = structValue.GetValue(ii);
        ECPropertyCP memberProperty = memberValue.GetColumnInfo().GetProperty();
        BeAssert(memberProperty != nullptr);
        Utf8CP memberName = memberProperty->GetName().c_str();

        if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_Distortion_K1, memberName))
            Distortion.SetK1(memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_Distortion_K2, memberName))
            Distortion.SetK2(memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_Distortion_K3, memberName))
            Distortion.SetK3(memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_Distortion_P1, memberName))
            Distortion.SetP1(memberValue.GetDouble());
        else if (0 == BeStringUtilities::Stricmp(CAMERA_PROPNAME_Distortion_P2, memberName))
            Distortion.SetP2(memberValue.GetDouble());
        else
            BeAssert(false);
        }
    return Distortion;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(CAMERA_PROPNAME_FocalLenghtPixels);
    params.Add(CAMERA_PROPNAME_ImageDimension);
    params.Add(CAMERA_PROPNAME_PrincipalPoint);
    params.Add(CAMERA_PROPNAME_Distortion);
    params.Add(CAMERA_PROPNAME_AspectRatio);
    params.Add(CAMERA_PROPNAME_Skew);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraPtr Camera::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Camera, model.GetDgnDb());

    CameraPtr cp = new Camera(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraElementId         Camera::GetId() const { return CameraElementId(GetElementId().GetValueUnchecked()); }
double                  Camera::GetFocalLenghtPixels() const { return m_focalLenghtPixels; }
ImageDimensionType      Camera::GetImageDimension() const { return m_imageDimension; }
DPoint2d                Camera::GetPrincipalPoint() const { return m_principalPoint; }
CameraDistortionType    Camera::GetDistortion() const { return m_Distortion; }
double                  Camera::GetAspectRatio() const { return m_aspectRatio; }
double                  Camera::GetSkew() const { return m_skew; }
void                    Camera::SetFocalLenghtPixels(double val) { m_focalLenghtPixels = val; }
void                    Camera::SetImageDimension(ImageDimensionTypeCR val) { m_imageDimension = val; }
void                    Camera::SetPrincipalPoint(DPoint2dCR val) { m_principalPoint = val; }
void                    Camera::SetDistortion(CameraDistortionTypeCR val) { m_Distortion = val; }
void                    Camera::SetAspectRatio(double val) { m_aspectRatio = val; }
void                    Camera::SetSkew(double val) { m_skew = val; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_FocalLenghtPixels), GetFocalLenghtPixels()) ||
        ECSqlStatus::Success != ImageDimensionType::BindParameter(statement, statement.GetParameterIndex(CAMERA_PROPNAME_ImageDimension),GetImageDimension()) ||
        ECSqlStatus::Success != statement.BindPoint2D(statement.GetParameterIndex(CAMERA_PROPNAME_PrincipalPoint), GetPrincipalPoint()) ||
        ECSqlStatus::Success != CameraDistortionType::BindParameter(statement, statement.GetParameterIndex(CAMERA_PROPNAME_Distortion), GetDistortion()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_AspectRatio), GetAspectRatio()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_Skew), GetSkew())         )
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        //read camera properties
        SetFocalLenghtPixels (stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_FocalLenghtPixels)));
        SetImageDimension(ImageDimensionType::GetValue(stmt, params.GetSelectIndex(CAMERA_PROPNAME_ImageDimension)));
        SetPrincipalPoint(stmt.GetValuePoint2D(params.GetSelectIndex(CAMERA_PROPNAME_PrincipalPoint))); 
        SetDistortion(CameraDistortionType::GetValue(stmt, params.GetSelectIndex(CAMERA_PROPNAME_Distortion)));
        SetAspectRatio(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_AspectRatio)));
        SetSkew(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_Skew)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::_OnInsert()
    {
//     PlanningModelP planningModel = dynamic_cast<PlanningModelP> (GetModel().get());
//     if (nullptr == planningModel)
//         {
//         BeAssert(false && "Can insert Camera only in a PlanningModel");
//         return DgnDbStatus::WrongModel;
//         }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Camera::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<CameraCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetFocalLenghtPixels(other->GetFocalLenghtPixels());
    SetImageDimension(other->GetImageDimension());
    SetPrincipalPoint(other->GetPrincipalPoint());
    SetDistortion(other->GetDistortion());
    SetAspectRatio(other->GetAspectRatio());
    SetSkew(other->GetSkew());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraElementId Camera::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT camera.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_Camera) " camera " \
        "WHERE camera.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraElementId();

    return statement->GetValueId<CameraElementId>(0);
    }



END_BENTLEY_DATACAPTURE_NAMESPACE

