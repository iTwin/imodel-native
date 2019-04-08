/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/CameraDevice.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(CameraDeviceHandler)
HANDLER_DEFINE_MEMBERS(CameraDeviceModelHandler)
HANDLER_DEFINE_MEMBERS(RadialDistortionHandler)
HANDLER_DEFINE_MEMBERS(TangentialDistortionHandler)

#define CAMERA_PROPNAME_FocalLength             "FocalLength"
#define CAMERA_PROPNAME_ImageWidth              "ImageWidth"
#define CAMERA_PROPNAME_ImageHeight             "ImageHeight"
#define CAMERA_PROPNAME_PrincipalPoint          "PrincipalPoint"
#define CAMERA_PROPNAME_RadialDistortion        "RadialDistortion"
#define CAMERA_PROPNAME_TangentialDistortion    "TangentialDistortion"
#define CAMERA_PROPNAME_AspectRatio             "AspectRatio"
#define CAMERA_PROPNAME_Skew                    "Skew"
#define CAMERA_PROPNAME_ModelType               "ModelType"
#define CAMERA_PROPNAME_SensorSize              "SensorSize"

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
void CameraDeviceModelHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(CAMERA_PROPNAME_ImageWidth);
    params.Add(CAMERA_PROPNAME_ImageHeight);
    params.Add(CAMERA_PROPNAME_AspectRatio);
    params.Add(CAMERA_PROPNAME_Skew);
    params.Add(CAMERA_PROPNAME_FocalLength);
    params.Add(CAMERA_PROPNAME_PrincipalPoint);
    params.Add(CAMERA_PROPNAME_ModelType);
    params.Add(CAMERA_PROPNAME_SensorSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDeviceHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(CAMERA_PROPNAME_ImageWidth);
    params.Add(CAMERA_PROPNAME_ImageHeight);
    params.Add(CAMERA_PROPNAME_AspectRatio);
    params.Add(CAMERA_PROPNAME_Skew);
    params.Add(CAMERA_PROPNAME_FocalLength);
    params.Add(CAMERA_PROPNAME_PrincipalPoint);
    params.Add(CAMERA_PROPNAME_SensorSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceModelPtr CameraDeviceModel::Create(Dgn::DefinitionModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());

    CameraDeviceModelPtr cp = new CameraDeviceModel(CreateParams(model.GetDgnDb(), model.GetModelId(), classId));
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CameraDeviceModel::CreateCode(Dgn::DgnDbR db, Utf8StringCR value) 
    {
    return DataCaptureDomain::CreateCode(db,BDCP_CLASS_CameraDeviceModel,value);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CameraDeviceModel::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());
    return CreateCode(GetDgnDb(), defaultName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int                     CameraDeviceModel::GetImageWidth() const { return m_imageWidth; }
int                     CameraDeviceModel::GetImageHeight() const { return m_imageHeight; }
void                    CameraDeviceModel::SetImageWidth(int val) { m_imageWidth = val; }
void                    CameraDeviceModel::SetImageHeight(int val) { m_imageHeight = val; }
CameraDeviceModelElementId     CameraDeviceModel::GetId() const { return CameraDeviceModelElementId(GetElementId().GetValueUnchecked()); }
double                  CameraDeviceModel::GetFocalLength() const { return m_focalLength; }
DPoint2d                CameraDeviceModel::GetPrincipalPoint() const { return m_principalPoint; }
double                  CameraDeviceModel::GetAspectRatio() const { return m_aspectRatio; }
double                  CameraDeviceModel::GetSkew() const { return m_skew; }
void                    CameraDeviceModel::SetFocalLength(double val) { m_focalLength = val; }
void                    CameraDeviceModel::SetPrincipalPoint(DPoint2dCR val) { m_principalPoint = val; }
void                    CameraDeviceModel::SetAspectRatio(double val) { m_aspectRatio = val; }
void                    CameraDeviceModel::SetSkew(double val) { m_skew = val; }
CameraDeviceModel::ModelType CameraDeviceModel::GetModelType() const { return m_modelType; }
void                    CameraDeviceModel::SetModelType(ModelType val) { m_modelType = val; }
double                  CameraDeviceModel::GetSensorSize() const { return m_sensorSize; }
void                    CameraDeviceModel::SetSensorSize(double val) { m_sensorSize = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDeviceModel::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_FocalLength), GetFocalLength()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageWidth),GetImageWidth()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageHeight), GetImageHeight()) ||
        ECSqlStatus::Success != statement.BindPoint2D(statement.GetParameterIndex(CAMERA_PROPNAME_PrincipalPoint), GetPrincipalPoint()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_AspectRatio), GetAspectRatio()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_Skew), GetSkew()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ModelType), (int)GetModelType()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_SensorSize), GetSensorSize()))
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDeviceModel::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDeviceModel::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDeviceModel::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        //read cameraDevice properties
        SetFocalLength (stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_FocalLength)));
        SetImageWidth(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageWidth)));
        SetImageHeight(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageHeight)));
        SetPrincipalPoint(stmt.GetValuePoint2D(params.GetSelectIndex(CAMERA_PROPNAME_PrincipalPoint))); 
        SetAspectRatio(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_AspectRatio)));
        SetSkew(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_Skew)));
        SetModelType((ModelType)stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ModelType)));
        SetSensorSize(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_SensorSize)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CameraDeviceModel::_OnDelete() const
    {
    //If we delete a cameraDevice, we should also delete all related shots
    for (CameraDeviceEntry const& cameraDevice : MakeCameraDeviceIterator(GetDgnDb(), GetId()))
        {
        CameraDeviceCPtr myCameraDevicePtr = CameraDevice::Get(GetDgnDb(), cameraDevice.GeCameraDeviceElementId());
        //delete them all
        myCameraDevicePtr->Delete();
        }
    return DgnDbStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDeviceModel::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = dynamic_cast<CameraDeviceModelCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetImageWidth(other->GetImageWidth());
    SetImageHeight(other->GetImageHeight());
    SetFocalLength(other->GetFocalLength());
    SetPrincipalPoint(other->GetPrincipalPoint());
    SetAspectRatio(other->GetAspectRatio());
    SetSkew(other->GetSkew());
    SetModelType(other->GetModelType());
    SetSensorSize(other->GetSensorSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceModelElementId CameraDeviceModel::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT cameraDeviceModel.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_CameraDeviceModel) " cameraDeviceModel " \
        "WHERE cameraDeviceModel.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraDeviceModelElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraDeviceModelElementId();
                                                                 
    return statement->GetValueId<CameraDeviceModelElementId>(0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceModel::CameraDeviceIterator CameraDeviceModel::MakeCameraDeviceIterator(Dgn::DgnDbCR dgndb, CameraDeviceModelElementId cameraDeviceModelId)
    {
    Utf8CP ecSql = "SELECT SourceECInstanceId  FROM " BDCP_SCHEMA(BDCP_REL_CameraDeviceIsDefinedByCameraDeviceModel) " WHERE TargetECInstanceId=?";

    CameraDeviceModel::CameraDeviceIterator iterator;
    int idSelectColumnIndex = 0;
    ECSqlStatement* statement = iterator.Prepare(dgndb, ecSql, idSelectColumnIndex);
    if (statement != nullptr)
        statement->BindId(1, cameraDeviceModelId);

    return iterator;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDevicePtr CameraDevice::Create(Dgn::SpatialModelR model, CameraDeviceModelElementId cameraDeviceModel)
    {
    if (!cameraDeviceModel.IsValid())
        {
        BeAssert(false && "Cannot create a cameraDevice with an invalid cameraDeviceModel");
        return nullptr;
        }

    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_AcquisitionDevice, model.GetDgnDb());

    CameraDevicePtr cp = new CameraDevice(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId),cameraDeviceModel);
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CameraDevice::CreateCode(Dgn::DgnDbR db, Utf8StringCR value) 
    {
    return DataCaptureDomain::CreateCode(db,BDCP_CLASS_CameraDevice,value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CameraDevice::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceModelElementId  CameraDevice::QueryCameraDeviceIsDefinedByCameraDeviceModelRelationship(DgnDbR dgndb, CameraDeviceElementId cameraDeviceElmId)
    {
    Utf8CP ecSql = "SELECT [TargetECInstanceId]  FROM " BDCP_SCHEMA(BDCP_REL_CameraDeviceIsDefinedByCameraDeviceModel) " WHERE SourceECInstanceId=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraDeviceModelElementId();
        }

    statement->BindId(1, cameraDeviceElmId);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraDeviceModelElementId();

    return statement->GetValueId<CameraDeviceModelElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void   CameraDevice::SetCameraDeviceModelId(CameraDeviceModelElementId val) { m_cameraDeviceModel = val; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceModelElementId  CameraDevice::GetCameraDeviceModelId() const
    {
    //Query and cache the result
    if (!m_cameraDeviceModel.IsValid())
        m_cameraDeviceModel = QueryCameraDeviceIsDefinedByCameraDeviceModelRelationship(GetDgnDb(),GetId());
    return m_cameraDeviceModel;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int                     CameraDevice::GetImageWidth() const { return m_imageWidth; }
void                    CameraDevice::SetImageWidth(int val) { m_imageWidth = val; }
int                     CameraDevice::GetImageHeight() const { return m_imageHeight; }
void                    CameraDevice::SetImageHeight(int val) { m_imageHeight = val; }
CameraDeviceElementId         CameraDevice::GetId() const { return CameraDeviceElementId(GetElementId().GetValueUnchecked()); }
double                  CameraDevice::GetFocalLength() const { return m_focalLength; }
void                    CameraDevice::SetFocalLength(double val) { m_focalLength = val; }
DPoint2d                CameraDevice::GetPrincipalPoint() const { return m_principalPoint; }
void                    CameraDevice::SetPrincipalPoint(DPoint2dCR val) { m_principalPoint = val; }
double                  CameraDevice::GetAspectRatio() const { return m_aspectRatio; }
void                    CameraDevice::SetAspectRatio(double val) { m_aspectRatio = val; }
double                  CameraDevice::GetSkew() const { return m_skew; }
void                    CameraDevice::SetSkew(double val) { m_skew = val; }
double                  CameraDevice::GetSensorSize() const { return m_sensorSize; }
void                    CameraDevice::SetSensorSize(double val) { m_sensorSize= val; }

//double                  CameraDevice::GetPixelToMeterRatio() const { return m_pixelToMeterRatio;  };
//void                    CameraDevice::SetPixelToMeterRatio(double val) { m_pixelToMeterRatio = val; };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionP       CameraDevice::GetRadialDistortionP()
    {
    return DgnElement::UniqueAspect::GetP<RadialDistortion>(*this, *RadialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RadialDistortionCP       CameraDevice::GetRadialDistortion() const
    {
    return DgnElement::UniqueAspect::Get<RadialDistortion>(*this, *RadialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::SetRadialDistortion(RadialDistortionP pValue)
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
TangentialDistortionP       CameraDevice::GetTangentialDistortionP()
    {
    return DgnElement::UniqueAspect::GetP<TangentialDistortion>(*this, *TangentialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TangentialDistortionCP       CameraDevice::GetTangentialDistortion() const
    {
    return DgnElement::UniqueAspect::Get<TangentialDistortion>(*this, *TangentialDistortion::QueryClass(GetDgnDb()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::SetTangentialDistortion(TangentialDistortionP pValue)
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
DgnDbStatus CameraDevice::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_FocalLength), GetFocalLength()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageWidth), GetImageWidth()) ||
        ECSqlStatus::Success != statement.BindInt(statement.GetParameterIndex(CAMERA_PROPNAME_ImageHeight), GetImageHeight()) ||
        ECSqlStatus::Success != statement.BindPoint2D(statement.GetParameterIndex(CAMERA_PROPNAME_PrincipalPoint), GetPrincipalPoint()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_AspectRatio), GetAspectRatio()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_Skew), GetSkew()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(CAMERA_PROPNAME_SensorSize), GetSensorSize()))
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDevice::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDevice::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDevice::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        //read cameraDevice properties
        SetFocalLength(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_FocalLength)));
        SetImageWidth(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageWidth)));
        SetImageHeight(stmt.GetValueInt(params.GetSelectIndex(CAMERA_PROPNAME_ImageHeight)));
        SetPrincipalPoint(stmt.GetValuePoint2D(params.GetSelectIndex(CAMERA_PROPNAME_PrincipalPoint)));
        SetAspectRatio(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_AspectRatio)));
        SetSkew(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_Skew)));
        SetSensorSize(stmt.GetValueDouble(params.GetSelectIndex(CAMERA_PROPNAME_SensorSize)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraDevice::_OnInsert()
    {
    if (!m_cameraDeviceModel.IsValid())
        {
        BeAssert(false && "Cannot insert a cameraDevice with an invalid cameraDeviceModel");
        return DgnDbStatus::ValidationFailed;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(Dgn::DgnDbR dgndb) const
    {
    InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(dgndb,GetId(),GetCameraDeviceModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CameraDevice::InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(DgnDbR dgndb, CameraDeviceElementId cameraDeviceElmId, CameraDeviceModelElementId cameraDeviceModelElmId)
    {
    if (!cameraDeviceElmId.IsValid() || !cameraDeviceModelElmId.IsValid())
        {
        BeAssert(false && "Attempt to add invalid cameraDevice Is Defined By CameraDeviceModel relationship");
        return ERROR;
        }

    Utf8CP ecSql = "INSERT INTO " BDCP_SCHEMA(BDCP_REL_CameraDeviceIsDefinedByCameraDeviceModel) " (SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());

    statement->BindId(1, cameraDeviceElmId);
    statement->BindId(2, cameraDeviceModelElmId);

    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error creating cameraDevice Is Defined By CameraDeviceModel Relationship");
        return ERROR;
        }
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);

    //Update relationship
    InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::DeleteCameraDeviceIsDefinedByCameraDeviceModelRelationship(DgnDbR dgndb) const
    {
    if (!GetId().IsValid())
        {
        BeAssert(false && "Attempt to delete an invalid cameraDevice Is Defined By CameraDeviceModel relationship");
        return;
        }

    //Delete old one 
    Utf8CP ecSql = "DELETE FROM " BDCP_SCHEMA(BDCP_REL_CameraDeviceIsDefinedByCameraDeviceModel) " WHERE SourceECInstanceId=?";
    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    BeAssert(statement.IsValid());
    statement->BindId(1, GetId());        //Source
    DbResult stepStatus = statement->Step();
    if (BE_SQLITE_DONE != stepStatus)
        {
        BeAssert(false && "Error deleting cameraDevice Is Defined By CameraDeviceModel Relationship");
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::UpdateCameraDeviceIsDefinedByCameraDeviceModelRelationship(DgnDbR dgndb) const
    {
    //Delete old one 
    DeleteCameraDeviceIsDefinedByCameraDeviceModelRelationship(dgndb);
    //and then insert new one
    InsertCameraDeviceIsDefinedByCameraDeviceModelRelationship(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);

    auto other = dynamic_cast<CameraDeviceCP>(&original);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    
    //Update relationship
    if (GetCameraDeviceModelId() != other->GetCameraDeviceModelId())
        UpdateCameraDeviceIsDefinedByCameraDeviceModelRelationship(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    DeleteCameraDeviceIsDefinedByCameraDeviceModelRelationship(GetDgnDb());
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CameraDevice::_OnDelete() const
    {
    //If we delete a cameraDevice, we should also delete all related shots
    for (ShotEntry const& shot : MakeShotIterator(GetDgnDb(), GetId()))
        {
        ShotCPtr myShotPtr = Shot::Get(GetDgnDb(), shot.GeShotElementId());
        //delete them all
        myShotPtr->Delete();
        }
    return DgnDbStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = dynamic_cast<CameraDeviceCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    SetImageWidth(other->GetImageWidth());
    SetImageHeight(other->GetImageHeight());
    SetAspectRatio(other->GetAspectRatio());
    SetSkew(other->GetSkew());
    SetFocalLength(other->GetFocalLength());
    SetPrincipalPoint(other->GetPrincipalPoint());
    SetCameraDeviceModelId(other->GetCameraDeviceModelId());
    SetSensorSize(other->GetSensorSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraDevice::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    T_Super::_RemapIds(importer);
    DgnElementId cameraModelId(GetCameraDeviceModelId());
    DgnElementId newId = importer.FindElementId(cameraModelId);
    CameraDeviceModelElementId newCameraModelId(newId.GetValue());
    SetCameraDeviceModelId(newCameraModelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDeviceElementId CameraDevice::QueryForIdByLabel(DgnDbR dgndb, Utf8CP label)
    {
    Utf8CP ecSql = "SELECT cameraDevice.[ECInstanceId] FROM " BDCP_SCHEMA(BDCP_CLASS_CameraDevice) " cameraDevice " \
        "WHERE cameraDevice.Label=?";

    CachedECSqlStatementPtr statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (!statement.IsValid())
        {
        BeAssert(statement.IsValid() && "Error preparing query. Check if DataCapture schema has been imported.");
        return CameraDeviceElementId();
        }

    statement->BindText(1, label, IECSqlBinder::MakeCopy::No);

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        return CameraDeviceElementId();
                                                                 
    return statement->GetValueId<CameraDeviceElementId>(0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CameraDevice::ShotIterator CameraDevice::MakeShotIterator(Dgn::DgnDbCR dgndb, CameraDeviceElementId cameraDeviceId)
    {
    Utf8CP ecSql = "SELECT SourceECInstanceId  FROM " BDCP_SCHEMA(BDCP_REL_ShotIsTakenByCameraDevice) " WHERE TargetECInstanceId=?";

    CameraDevice::ShotIterator iterator;
    int idSelectColumnIndex = 0;
    ECSqlStatement* statement = iterator.Prepare(dgndb, ecSql, idSelectColumnIndex);
    if (statement != nullptr)
        statement->BindId(1, cameraDeviceId);

    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Daniel.McKenzie 01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d CameraDevice::ComputeFieldOfView(CameraDeviceCR camDevice)
    {
    DPoint2d fieldofView;

    //Everything must be in meters
    fieldofView.x = atan2(camDevice.GetImageWidth() * camDevice.GetSensorSize(), 2 * camDevice.GetFocalLength());
    fieldofView.y = atan2(camDevice.GetImageHeight() * camDevice.GetSensorSize(), 2 * camDevice.GetFocalLength());

    return fieldofView;
    }


END_BENTLEY_DATACAPTURE_NAMESPACE

