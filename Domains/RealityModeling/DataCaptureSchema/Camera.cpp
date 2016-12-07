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
HANDLER_DEFINE_MEMBERS(RadialDistortionHandler)
HANDLER_DEFINE_MEMBERS(TangentialDistortionHandler)

#define CAMERA_PROPNAME_FocalLenghtPixels       "FocalLenghtPixels"
#define CAMERA_PROPNAME_ImageWidth              "ImageWidth"
#define CAMERA_PROPNAME_ImageHeight             "ImageHeight"
#define CAMERA_PROPNAME_PrincipalPoint          "PrincipalPoint"
#define CAMERA_PROPNAME_RadialDistortion        "RadialDistortion"
#define CAMERA_PROPNAME_TangentialDistortion    "TangentialDistortion"
#define CAMERA_PROPNAME_AspectRatio             "AspectRatio"
#define CAMERA_PROPNAME_Skew                    "Skew"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionPtr RadialDistortion::Create() 
    { return new RadialDistortion(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionPtr RadialDistortion::Create(double k1, double k2, double k3) 
    { return new RadialDistortion(k1, k2, k3); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionPtr RadialDistortion::Clone() const
    { return new RadialDistortion(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortion::RadialDistortion(RadialDistortionCR rhs) 
    { *this = rhs; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortion& RadialDistortion::operator= (RadialDistortionCR rhs)
    {
    m_k1 = rhs.m_k1;
    m_k2 = rhs.m_k2;
    m_k3 = rhs.m_k3;
    return *this;
    }
bool RadialDistortion::IsValid() const { return true; }
bool RadialDistortion::IsEqual(RadialDistortionCR rhs) const
    {
    if (m_k1 == rhs.m_k1 && m_k2 == rhs.m_k2 && m_k3 == rhs.m_k3)
        return true;
    return false;
    }
double RadialDistortion::GetK1() const { return m_k1; }
double RadialDistortion::GetK2() const { return m_k2; }
double RadialDistortion::GetK3() const { return m_k3; }
void   RadialDistortion::SetK1(double val) { m_k1 = val; }
void   RadialDistortion::SetK2(double val) { m_k2 = val; }
void   RadialDistortion::SetK3(double val) { m_k3 = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<Dgn::DgnElement::Aspect> RadialDistortionHandler::_CreateInstance() 
    { return RadialDistortion::Create(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionPtr TangentialDistortion::Create() 
    { return new TangentialDistortion(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionPtr TangentialDistortion::Create(double p1, double p2) 
    { return new TangentialDistortion(p1, p2); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionPtr TangentialDistortion::Clone() const 
    { return new TangentialDistortion(*this); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortion::TangentialDistortion(TangentialDistortionCR rhs)
    { *this = rhs; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortion& TangentialDistortion::operator= (TangentialDistortionCR rhs)
    {
    m_p1 = rhs.m_p1;
    m_p2 = rhs.m_p2;
    return *this;
    }
bool TangentialDistortion::IsValid() const { return true; }
bool TangentialDistortion::IsEqual(TangentialDistortionCR rhs) const
    {
    if (m_p1 == rhs.m_p1 && m_p2 == rhs.m_p2)
        return true;
    return false;
    }
double TangentialDistortion::GetP1() const { return m_p1; }
double TangentialDistortion::GetP2() const { return m_p2; }
void   TangentialDistortion::SetP1(double val) { m_p1 = val; }
void   TangentialDistortion::SetP2(double val) { m_p2 = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<Dgn::DgnElement::Aspect> TangentialDistortionHandler::_CreateInstance() 
    { return TangentialDistortion::Create(); }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RadialDistortion::_LoadProperties(DgnElementCR el)
    {
    Utf8PrintfString ecsql("SELECT K1, K2, K3 FROM ONLY %s.%s WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);
    stmt->BindId(1, GetAspectInstanceId(el));

    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::BadElement;

    double  k1 = (stmt->IsValueNull(0)) ? 0 : stmt->GetValueDouble(0);
    SetK1(k1);

    double  k2 = (stmt->IsValueNull(1)) ? 0 : stmt->GetValueDouble(1);
    SetK2(k2);

    double  k3 = (stmt->IsValueNull(2)) ? 0 : stmt->GetValueDouble(2);
    SetK3(k3);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RadialDistortion::_UpdateProperties(DgnElementCR el)
    {
    Utf8PrintfString ecsql("UPDATE %s.%s SET K1 = ?, K2 = ?, K3 = ? WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);

    double k1 = GetK1();
    stmt->BindDouble(1, k1);

    double k2 = GetK2();
    stmt->BindDouble(2, k2);

    double k3 = GetK3();
    stmt->BindDouble(3, k3);

    stmt->BindId(4, GetAspectInstanceId(el));

    if (BE_SQLITE_DONE != stmt->Step())
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TangentialDistortion::_LoadProperties(DgnElementCR el)
    {
    Utf8PrintfString ecsql("SELECT P1, P2 FROM ONLY %s.%s WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);
    stmt->BindId(1, GetAspectInstanceId(el));

    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::BadElement;

    double  p1 = (stmt->IsValueNull(0)) ? 0 : stmt->GetValueDouble(0);
    SetP1(p1);

    double  p2 = (stmt->IsValueNull(1)) ? 0 : stmt->GetValueDouble(1);
    SetP2(p2);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TangentialDistortion::_UpdateProperties(DgnElementCR el)
    {
    Utf8PrintfString ecsql("UPDATE %s.%s SET P1 = ?, P2 = ? WHERE ECInstanceId = ?;", _GetECSchemaName(), _GetECClassName());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(ecsql);

    double p1 = GetP1();
    stmt->BindDouble(1, p1);

    double p2 = GetP2();
    stmt->BindDouble(2, p2);

    stmt->BindId(3, GetAspectInstanceId(el));

    if (BE_SQLITE_DONE != stmt->Step())
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(CAMERA_PROPNAME_FocalLenghtPixels);
    params.Add(CAMERA_PROPNAME_ImageWidth);
    params.Add(CAMERA_PROPNAME_ImageHeight);
    params.Add(CAMERA_PROPNAME_PrincipalPoint);
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
int                     Camera::GetImageWidth() const { return m_imageWidth; }
int                     Camera::GetImageHeight() const { return m_imageHeight; }
void                    Camera::SetImageWidth(int val) { m_imageWidth = val; }
void                    Camera::SetImageHeight(int val) { m_imageHeight = val; }
CameraElementId         Camera::GetId() const { return CameraElementId(GetElementId().GetValueUnchecked()); }
double                  Camera::GetFocalLenghtPixels() const { return m_focalLenghtPixels; }
DPoint2d                Camera::GetPrincipalPoint() const { return m_principalPoint; }
double                  Camera::GetAspectRatio() const { return m_aspectRatio; }
double                  Camera::GetSkew() const { return m_skew; }
void                    Camera::SetFocalLenghtPixels(double val) { m_focalLenghtPixels = val; }
void                    Camera::SetPrincipalPoint(DPoint2dCR val) { m_principalPoint = val; }
void                    Camera::SetAspectRatio(double val) { m_aspectRatio = val; }
void                    Camera::SetSkew(double val) { m_skew = val; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionP       Camera::GetRadialDistortionP()
    {
    return DgnElement::UniqueAspect::GetP<RadialDistortion>(*this, *RadialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionCP       Camera::GetRadialDistortion() const
    {
    return DgnElement::UniqueAspect::Get<RadialDistortion>(*this, *RadialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Camera::SetRadialDistortion(RadialDistortionP pValue)
    {
    if (nullptr == pValue)
        {
        if (auto pExistingValue = GetRadialDistortionP())
            pExistingValue->Delete();
        }
    else 
        {
        DgnElement::UniqueAspect::SetAspect(*this, *pValue);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionP       Camera::GetTangentialDistortionP()
    {
    return DgnElement::UniqueAspect::GetP<TangentialDistortion>(*this, *TangentialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionCP       Camera::GetTangentialDistortion() const
    {
    return DgnElement::UniqueAspect::Get<TangentialDistortion>(*this, *TangentialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Camera::SetTangentialDistortion(TangentialDistortionP pValue)
    {
    if (nullptr == pValue)
        {
        if (auto pExistingValue = GetTangentialDistortionP())
            pExistingValue->Delete();
        }
    else 
        {
        DgnElement::UniqueAspect::SetAspect(*this, *pValue);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Camera::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_FocalLenghtPixels), GetFocalLenghtPixels()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageWidth),GetImageWidth()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageHeight), GetImageHeight()) ||
        ECSqlStatus::Success != statement.BindPoint2D(statement.GetParameterIndex(CAMERA_PROPNAME_PrincipalPoint), GetPrincipalPoint()) ||
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
        SetImageWidth(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageWidth)));
        SetImageHeight(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageHeight)));
        SetPrincipalPoint(stmt.GetValuePoint2D(params.GetSelectIndex(CAMERA_PROPNAME_PrincipalPoint))); 
        SetAspectRatio(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_AspectRatio)));
        SetSkew(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_Skew)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus Camera::_OnDelete() const
    {
    //If we delete a camera, we should also delete all related photos
    for (PhotoEntry const& photo : MakePhotoIterator(GetDgnDb(), GetId()))
        {
        PhotoCPtr myPhotoPtr = Photo::Get(GetDgnDb(), photo.GePhotoElementId());
        //delete them all
        myPhotoPtr->Delete();
        }
    return DgnDbStatus::Success;
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
    SetImageWidth(other->GetImageWidth());
    SetImageHeight(other->GetImageHeight());
    SetPrincipalPoint(other->GetPrincipalPoint());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Camera::PhotoIterator Camera::MakePhotoIterator(Dgn::DgnDbCR dgndb, CameraElementId cameraId)
    {
    Utf8CP ecSql = "SELECT SourceECInstanceId  FROM " BDCP_SCHEMA(BDCP_REL_PhotoIsTakenByCamera) " WHERE TargetECInstanceId=?";

    Camera::PhotoIterator iterator;
    int idSelectColumnIndex = 0;
    ECSqlStatement* statement = iterator.Prepare(dgndb, ecSql, idSelectColumnIndex);
    if (statement != nullptr)
        statement->BindId(1, cameraId);

    return iterator;
    }



END_BENTLEY_DATACAPTURE_NAMESPACE

