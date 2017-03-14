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
RotMatrix Pose::GetRotMatrixFromRotation(AngleCR omega, AngleCR phi, AngleCR kappa)
    {
    RotMatrix rotation(ContextCaptureFacility::OmegaPhiKappa2Matrix(omega.Radians(),phi.Radians(),kappa.Radians()));
    return rotation;
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
    omega.FromRadians(o);
    phi.FromRadians(p);
    kappa.FromRadians(k);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::FrustumCornersFromCameraPose(DPoint3dP points, PoseCR pose, DPoint2dCR fieldofView, DPoint3dCR target)
    {
    RotMatrix rotMatrix = pose.GetRotMatrixFromRotation(pose.GetOmega(), pose.GetPhi(), pose.GetKappa());
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

    SetCenterECEF(ecef,false);

    DPoint3d point = { 0,0,0 };
    if (SUCCESS != gcs->UorsFromLatLong(point, geoPoint))
        BeAssert(!L"Does not support latlong");

    SetCenter(point,false);

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
    RotMatrix rotation(ContextCaptureFacility::OmegaPhiKappa2Matrix(GetOmega().Radians(), GetPhi().Radians(), GetKappa().Radians()));
    rotation.Transpose();
    YawPitchRollAngles angles;
    if (!YawPitchRollAngles::TryFromRotMatrix(angles,rotation))
        BeAssert(!"Cannot convert rotation matrix to angles");

    return angles;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetYawPitchRoll(YawPitchRollAnglesCR angles, bool synchRotMatrix)
    {
    RotMatrix rotationMatrix(angles.ToRotMatrix());
    double o;
    double p;
    double k;
    ContextCaptureFacility::Matrix2OmegaPhiKappa(rotationMatrix, o, p, k);

    SetOmega(Angle::FromRadians(o));
    SetPhi(Angle::FromRadians(p));
    SetKappa(Angle::FromRadians(k));
    if (synchRotMatrix)
        {
        SetRotMatrixECEF(rotationMatrix,false);
        }
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
    return m_center;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetCenter(DPoint3dCR val, bool synchECEF) 
    {
    m_center = val;
    //Also set centerECEF
    if (synchECEF)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        if (!gcs.IsValid()) //gcs is not valid return an empty GeoPoint
            {
            m_centerECEF=val;
            return;
            }

        GeoPoint geopoint;
        if (SUCCESS != gcs->LatLongFromUors(geopoint, m_center))
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
void Pose::SetOmega(AngleCR omega) 
    {
    m_omega = omega;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetPhi(AngleCR phi) 
    {
    m_phi = phi;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetKappa(AngleCR kappa) 
    {
    m_kappa = kappa;
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
void Pose::SetCenterECEF(DPoint3dCR val, bool synchlocalGCS) 
    {
    m_centerECEF = val;
    //Also set center in local GCS
    if (synchlocalGCS)
        {
        DgnGCSCPtr gcs = GetDgnDb().Units().GetDgnGCS();// : customGcs;

        if (!gcs.IsValid()) //gcs is not valid return an empty GeoPoint
            {
            SetCenter(val, false);
            return;
            }

        GeoPoint geopoint;
        if (SUCCESS != gcs->LatLongFromXYZ(geopoint, m_centerECEF))
            {
            BeAssert(!L"Does not support latlong");
            m_center = val;
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
void Pose::SetRotMatrixECEF(RotMatrixCR rotation,bool synchYPR)
    {
    m_rotationECEF=rotation;
    if (synchYPR)
        {
        YawPitchRollAngles YPRAngles;
        if (YawPitchRollAngles::TryFromRotMatrix(YPRAngles,rotation))
            SetYawPitchRoll(YPRAngles,false);
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
        //read cameraDevice properties
        SetCenter(stmt.GetValuePoint3D(params.GetSelectIndex(POSE_PROPNAME_Center)),false);
        SetOmega(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Omega))));
        SetPhi(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Phi))));
        SetKappa(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Kappa))));
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
        SetRotMatrixECEF(rotationECEF);
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
    SetCenter(other->GetCenter(),false);
    SetOmega(other->GetOmega());
    SetPhi(other->GetPhi());
    SetKappa(other->GetKappa());
    SetCenterECEF(other->GetCenterECEF(),false);
    SetRotMatrixECEF(other->GetRotMatrixECEF());
    }


END_BENTLEY_DATACAPTURE_NAMESPACE

