/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Pose.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

HANDLER_DEFINE_MEMBERS(PoseHandler)

#define POSE_PROPNAME_Center            "Center"
#define POSE_PROPNAME_Omega             "Omega"
#define POSE_PROPNAME_Phi               "Phi"
#define POSE_PROPNAME_Kappa             "Kappa"

#define POSE_PROPNAME_IsECEF            "IsECEF"
#define POSE_PROPNAME_CenterECEF        "CenterECEF"
#define POSE_PROPNAME_M_00              "M_00"
#define POSE_PROPNAME_M_01              "M_01"
#define POSE_PROPNAME_M_02              "M_02"
#define POSE_PROPNAME_M_10              "M_10"
#define POSE_PROPNAME_M_11              "M_11"
#define POSE_PROPNAME_M_12              "M_12"
#define POSE_PROPNAME_M_20              "M_20"
#define POSE_PROPNAME_M_21              "M_21"
#define POSE_PROPNAME_M_22              "M_22"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PoseHandler::_GetClassParams(Dgn::ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(POSE_PROPNAME_Center);
    params.Add(POSE_PROPNAME_Omega);
    params.Add(POSE_PROPNAME_Phi);
    params.Add(POSE_PROPNAME_Kappa);
    params.Add(POSE_PROPNAME_IsECEF);
    params.Add(POSE_PROPNAME_CenterECEF);
    params.Add(POSE_PROPNAME_M_00);
    params.Add(POSE_PROPNAME_M_01);
    params.Add(POSE_PROPNAME_M_02);
    params.Add(POSE_PROPNAME_M_10);
    params.Add(POSE_PROPNAME_M_11);
    params.Add(POSE_PROPNAME_M_12);
    params.Add(POSE_PROPNAME_M_20);
    params.Add(POSE_PROPNAME_M_21);
    params.Add(POSE_PROPNAME_M_22);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PosePtr Pose::Create(Dgn::SpatialModelR model)
    {
    DgnClassId classId = QueryClassId(model.GetDgnDb());
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(BDCP_CATEGORY_Pose, model.GetDgnDb());

    PosePtr cp = new Pose(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, categoryId));
    return cp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix Pose::GetRotMatrix() const
    {
    return m_rotationLocal;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetRotMatrix(RotMatrixCR rotMatrixLocal, bool synchECEF)
    {
    m_rotationLocal = rotMatrixLocal;

    GetRotationFromRotMatrix(m_omega, m_phi, m_kappa, rotMatrixLocal);

    if (synchECEF)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        //Convert to rotation in ECEF coordinate system
        if (!m_isECEFSupported || !gcs.IsValid()) //gcs is not valid return , store in local GCS
            {
            m_rotationECEF = rotMatrixLocal;
            return;
            }

        GeoPoint lla;
        if (SUCCESS != gcs->LatLongFromUors(lla, m_centerLocal))
            {
            BeAssert(!L"Does not support latlong");
            m_rotationECEF = rotMatrixLocal;
            return;
            }

        //NEEDSWORK_WGS84 - What is not WGS84?
        //Assume we have ENU, it's a good approximation
        Ellipsoid elWGS84(Ellipsoid::WGS84());

        Transform tENU2ECEF(elWGS84.ENU2ECEF(lla.latitude, lla.longitude));

        RotMatrix rotECEF;
        rotECEF.InitProduct(tENU2ECEF, rotMatrixLocal);

        SetRotMatrixECEF(rotECEF, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::GetRotationFromRotMatrix(AngleR omega, AngleR phi, AngleR kappa, RotMatrixCR rotation)
    {
    double o;
    double p;
    double k;
    ContextCaptureFacility::Matrix2OmegaPhiKappa(rotation,o,p,k);
    omega = Angle::FromRadians(o);
    phi = Angle::FromRadians(p);
    kappa = Angle::FromRadians(k);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::FrustumCornersFromCameraPose(DPoint3dP points, PoseCR pose, DPoint2dCR fieldofView, DPoint3dCR target)
    {
    RotMatrix rotMatrix = pose.GetRotMatrix();
    DPoint3d center = pose.GetCenter();

    double zOff = 1.10 * center.Distance(target);
    double xOff = zOff * tan(fieldofView.x);
    double yOff = zOff * tan(fieldofView.y);

    DVec3d xCameraAxis = DVec3d::FromRow(rotMatrix, 0);
    DVec3d yCameraAxis = DVec3d::FromRow(rotMatrix, 1);
    DVec3d zCameraAxis = DVec3d::FromRow(rotMatrix, 2);

    xCameraAxis.ScaleToLength(xOff);
    yCameraAxis.ScaleToLength(yOff);
    zCameraAxis.ScaleToLength(zOff);

    DPoint3d corner = center;
    corner.Add(zCameraAxis);

    corner.Add(xCameraAxis);
    corner.Add(yCameraAxis);
    points[1] = corner;
    corner.Subtract(xCameraAxis);
    corner.Subtract(xCameraAxis);
    points[0] = corner;
    corner.Subtract(yCameraAxis);
    corner.Subtract(yCameraAxis);
    points[2] = corner;
    corner.Add(xCameraAxis);
    corner.Add(xCameraAxis);
    points[3] = corner;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                  01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeoPoint Pose::GetCenterAsLatLongValue() const
    {
    GeoPoint geopoint;
    geopoint.Init(0, 0, 0);
    DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

    if (!gcs.IsValid()) //gcs is not valid return an empty GeoPoint
        {
        BeAssert(!L"DGNGCS is not valid");
        return geopoint;;
        }

    if (SUCCESS != gcs->LatLongFromXYZ(geopoint, GetCenterECEF()))
        BeAssert(!L"Does not support latlong");

    return geopoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                  01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetCenterFromLatLongValue(GeoPointCR geoPoint)
    {
    DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();

    if (!gcs.IsValid())
        {
        BeAssert(!L"DGNGCS is not valid");
        return;
        }

    DPoint3d ecef = { 0,0,0 };

    if (SUCCESS != gcs->XYZFromLatLong(ecef, geoPoint))
        BeAssert(!L"Does not support latlong");

    SetCenterECEF(ecef,true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Pose::CreateCode(Dgn::DgnDbR db, Utf8StringCR value) 
    {
    Utf8PrintfString internalValue("%s",value.c_str());
    return DataCaptureDomain::CreateCode(db,BDCP_CLASS_Pose,internalValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Pose::_GenerateDefaultCode() const
    {
    Utf8String defaultName = DataCaptureDomain::BuildDefaultName(MyECClassName(), GetElementId());

    return CreateCode(GetDgnDb(), defaultName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Pose::Pose(CreateParams const& params): T_Super(params)
    {
    DgnGCSCPtr pGCS =  params.m_dgndb.Units().GetDgnGCS();
    //NEEDSWORK_ECEF ; this mode is not working for now, camera have wrong orientation and probably position...
    m_isECEFSupported = pGCS.IsValid();
    //m_isECEFSupported = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::IsECEF() const
    {
    return m_isECEFSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetIsECEF(bool isECEF)
    {
    m_isECEFSupported =  isECEF;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::IsEqual(PoseCR rhs) const
    {
    if ((GetCenter()             == rhs.GetCenter())              &&
        (GetOmega().Radians()    == rhs.GetOmega().Radians())     && 
        (GetPhi().Radians()      == rhs.GetPhi().Radians())       &&
        (GetKappa().Radians()    == rhs.GetKappa().Radians())     &&
        (GetCenterECEF()         == rhs.GetCenterECEF())          &&
        (IsECEF()                == rhs.IsECEF())                 &&
        (GetRotMatrix().IsEqual(rhs.GetRotMatrix()))              &&
        (GetRotMatrixECEF().IsEqual(rhs.GetRotMatrixECEF()))       
        )
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
YawPitchRollAngles Pose::GetYawPitchRoll() const
    {
    RotMatrix rotation(m_rotationLocal);
    rotation.Transpose();

    YawPitchRollAngles angles;
    if (!YawPitchRollAngles::TryFromRotMatrix(angles, rotation))
        BeAssert(!"Cannot convert rotation matrix to angles");
    return angles;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetYawPitchRoll(YawPitchRollAnglesCR angles, bool synchECEF)
    {
    RotMatrix rotMatrixLocal(angles.ToRotMatrix());
    SetRotMatrix(rotMatrixLocal,synchECEF);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PoseElementId Pose::GetId() const 
    {
    return PoseElementId(GetElementId().GetValueUnchecked()); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR Pose::GetCenter() const 
    {
    return m_centerLocal;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetCenter(DPoint3dCR val, bool synchECEF) 
    {
    m_centerLocal = val;
    //Also set centerECEF
    if (synchECEF)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        if (!m_isECEFSupported || !gcs.IsValid()) //gcs is not valid return an empty GeoPoint
            {
            m_centerECEF=val;
            return;
            }

        GeoPoint geopoint;
        if (SUCCESS != gcs->LatLongFromUors(geopoint, m_centerLocal))
            {
            BeAssert(!L"Does not support latlong");
            m_centerECEF = val;
            return;
            }
        DPoint3d ecef = { 0,0,0 };

        if (SUCCESS != gcs->XYZFromLatLong(ecef, geopoint))
            BeAssert(!L"Does not support latlong");

        SetCenterECEF(ecef,false);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AngleCR Pose::GetOmega() const 
    {
    return m_omega;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AngleCR Pose::GetPhi() const
    {
    return m_phi;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AngleCR Pose::GetKappa() const
    {
    return m_kappa;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetOmega(AngleCR omega, bool synchECEF,bool synchRotMatrix) 
    {
    m_omega = omega;
    RotMatrix rotationLocal(ContextCaptureFacility::OmegaPhiKappa2Matrix(GetOmega().Radians(), GetPhi().Radians(), GetKappa().Radians()));
    //NEEDSWORK_OPK; Not sure about transpose here!
    //rotationLocal.Transpose();
    if (synchRotMatrix)
        SetRotMatrix(rotationLocal,synchECEF);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetPhi(AngleCR phi, bool synchECEF,bool synchRotMatrix) 
    {
    m_phi = phi;
    RotMatrix rotationLocal(ContextCaptureFacility::OmegaPhiKappa2Matrix(GetOmega().Radians(), GetPhi().Radians(), GetKappa().Radians()));
    //NEEDSWORK_OPK; Not sure about transpose here!
    //rotationLocal.Transpose();
    if (synchRotMatrix)
        SetRotMatrix(rotationLocal, synchECEF);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetKappa(AngleCR kappa, bool synchECEF,bool synchRotMatrix) 
    {
    m_kappa = kappa;
    RotMatrix rotationLocal(ContextCaptureFacility::OmegaPhiKappa2Matrix(GetOmega().Radians(), GetPhi().Radians(), GetKappa().Radians()));
    //NEEDSWORK_OPK; Not sure about transpose here!
    //rotationLocal.Transpose();
    if (synchRotMatrix)
        SetRotMatrix(rotationLocal, synchECEF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR Pose::GetCenterECEF() const 
    {
    return m_centerECEF;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetCenterECEF(DPoint3dCR val, bool synchLocal) 
    {
    m_centerECEF = val;
    //Also set center in local GCS
    if (synchLocal)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        if (!m_isECEFSupported || !gcs.IsValid()) //gcs is not valid return an empty GeoPoint
            {
            m_centerLocal = val;
            return;
            }

        GeoPoint geopoint;
        if (SUCCESS != gcs->LatLongFromXYZ(geopoint, m_centerECEF))
            {
            BeAssert(!L"Does not support latlong");
            m_centerLocal = val;
            return;
            }
        DPoint3d point = { 0,0,0 };

        if (SUCCESS != gcs->UorsFromLatLong(point, geopoint))
            BeAssert(!L"Does not support latlong");

        SetCenter(point, false);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix Pose::GetRotMatrixECEF() const
    {
    return m_rotationECEF;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetRotMatrixECEF(RotMatrixCR rotation,bool synchLocal)
    {
    m_rotationECEF=rotation;
    if (synchLocal)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        if (!m_isECEFSupported || !gcs.IsValid()) //gcs is not valid use current rotation as if it is local.
            {
            SetRotMatrix(m_rotationECEF, false);
            return;
            }

        GeoPoint lla;
        if (SUCCESS != gcs->LatLongFromXYZ(lla, m_centerECEF))
            {
            BeAssert(!L"Does not support latlong");
            SetRotMatrix(m_rotationECEF, false);
            return;
            }

        //Assume we have ENU, it's a good approximation
        Ellipsoid elWGS84(Ellipsoid::WGS84());

        Transform tECEF2ENU(elWGS84.ECEF2ENU(lla.latitude,lla.longitude));

        RotMatrix rotLocal;
        rotLocal.InitProduct(tECEF2ENU, m_rotationECEF);

        SetRotMatrix(rotLocal,false);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex(POSE_PROPNAME_Center), GetCenter()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Omega), GetOmega().Radians()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Phi),   GetPhi().Radians()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Kappa), GetKappa().Radians()) ||
        ECSqlStatus::Success != statement.BindBoolean(statement.GetParameterIndex(POSE_PROPNAME_IsECEF), IsECEF())            ||
        ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex(POSE_PROPNAME_CenterECEF), GetCenterECEF()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_00), GetRotMatrixECEF().GetComponentByRowAndColumn(0, 0)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_01), GetRotMatrixECEF().GetComponentByRowAndColumn(0, 1)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_02), GetRotMatrixECEF().GetComponentByRowAndColumn(0, 2)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_10), GetRotMatrixECEF().GetComponentByRowAndColumn(1, 0)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_11), GetRotMatrixECEF().GetComponentByRowAndColumn(1, 1)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_12), GetRotMatrixECEF().GetComponentByRowAndColumn(1, 2)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_20), GetRotMatrixECEF().GetComponentByRowAndColumn(2, 0)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_21), GetRotMatrixECEF().GetComponentByRowAndColumn(2, 1)) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_M_22), GetRotMatrixECEF().GetComponentByRowAndColumn(2, 2)) 
        )
        {
        return DgnDbStatus::BadArg;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat =  BindParameters(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        SetIsECEF(stmt.GetValueBoolean(params.GetSelectIndex(POSE_PROPNAME_IsECEF)));
        SetCenter(stmt.GetValuePoint3D(params.GetSelectIndex(POSE_PROPNAME_Center)),false);
        SetOmega(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Omega))),false,false);
        SetPhi(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Phi))),false,false);
        //TRICKY: last param is true because we want to compute local rotation from 3 angles
        SetKappa(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Kappa))),false,true); 
        SetCenterECEF(stmt.GetValuePoint3D(params.GetSelectIndex(POSE_PROPNAME_CenterECEF)),false);
        RotMatrix rotationECEF(RotMatrix::FromIdentity());
        rotationECEF.SetComponentByRowAndColumn(0, 0, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_00)));
        rotationECEF.SetComponentByRowAndColumn(0, 1, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_01)));
        rotationECEF.SetComponentByRowAndColumn(0, 2, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_02)));
        rotationECEF.SetComponentByRowAndColumn(1, 0, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_10)));
        rotationECEF.SetComponentByRowAndColumn(1, 1, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_11)));
        rotationECEF.SetComponentByRowAndColumn(1, 2, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_12)));
        rotationECEF.SetComponentByRowAndColumn(2, 0, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_20)));
        rotationECEF.SetComponentByRowAndColumn(2, 1, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_21)));
        rotationECEF.SetComponentByRowAndColumn(2, 2, stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_M_22)));
        SetRotMatrixECEF(rotationECEF,false);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::_OnInsert()
    {
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnInserted(DgnElementP copiedFrom) const
    {
    T_Super::_OnInserted(copiedFrom);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnUpdated(DgnElementCR original) const
    {
    T_Super::_OnUpdated(original);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<PoseCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;
    SetIsECEF(other->IsECEF());
    SetCenterECEF(other->GetCenterECEF(),false);
    SetRotMatrixECEF(other->GetRotMatrixECEF(),false);
    SetCenter(other->GetCenter(),false);
    SetRotMatrix(other->GetRotMatrix(), false);
    SetOmega(other->GetOmega(),false,false);
    SetPhi(other->GetPhi(),false,false);
    SetKappa(other->GetKappa(),false,false);
    }


END_BENTLEY_DATACAPTURE_NAMESPACE

