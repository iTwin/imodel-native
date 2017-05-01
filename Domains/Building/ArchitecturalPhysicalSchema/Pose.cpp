/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/Pose.cpp $
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
    RotMatrix rotation(RotMatrix::FromPrincipleAxisRotations(RotMatrix::FromIdentity(), omega.Radians(), phi.Radians(), kappa.Radians()));
    //Not sure we need this - it was in MS Connect mdlApps\RMUtilImage.cpp on MS Connect on vancouver stream
    rotation.Transpose();
    return rotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Pose::GetRotationFromRotMatrix(AngleR omega, AngleR phi, AngleR kappa, RotMatrixCR rotation)
    {
    //From: http://danceswithcode.net/engineeringnotes/rotations_in_3d/rotations_in_3d_part1.html
    double OmegaRadian = 0; //arbitrary
    double KappaRadian = 0;
    double PhiRadian   = -asin(rotation.GetComponentByRowAndColumn(2, 0));
    phi = Angle::FromRadians(PhiRadian);
    //Check for Gimbal Lock
    if (phi.Degrees() == 90)
        {
        KappaRadian = atan2(-rotation.GetComponentByRowAndColumn(0, 1), -rotation.GetComponentByRowAndColumn(0, 2));
        }
    else if (phi.Degrees() == -90)
        {
        KappaRadian = atan2(-rotation.GetComponentByRowAndColumn(0, 1), -rotation.GetComponentByRowAndColumn(0, 2));
        }
    else
        {
        OmegaRadian = atan2(rotation.GetComponentByRowAndColumn(1, 0), rotation.GetComponentByRowAndColumn(0, 0));
        KappaRadian = atan2(rotation.GetComponentByRowAndColumn(2, 1), rotation.GetComponentByRowAndColumn(2, 2));
        }
    omega =  Angle::FromRadians(OmegaRadian);
    kappa =  Angle::FromRadians(KappaRadian);
    return true;
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
    if ((GetCenter()   == rhs.GetCenter())                        &&
        (GetOmega().Radians()    == rhs.GetOmega().Radians())     && 
        (GetPhi().Radians()      == rhs.GetPhi().Radians())       &&
        (GetKappa().Radians()    == rhs.GetKappa().Radians())       )
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
YawPitchRollAngles Pose::GetYawPitchRoll() const
    {
    //NEEDSWORK: I know this is not "technically" correct. We will need to find the real conversion
    YawPitchRollAngles angles(GetKappa(),GetOmega(),GetPhi());
    return angles;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Pose::SetYawPitchRoll(YawPitchRollAnglesCR angles)
    {
    //NEEDSWORK: I know this is not "technically" correct. We will need to find the real conversion
    SetOmega(Angle::FromDegrees(angles.GetPitch().Degrees()));
    SetPhi(Angle::FromDegrees(angles.GetRoll().Degrees()));
    SetKappa(Angle::FromDegrees(angles.GetYaw().Degrees()));
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
void Pose::SetCenter(DPoint3dCR val) 
    {
    m_center = val;
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
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Pose::BindParameters(BeSQLite::EC::ECSqlStatement& statement)
    {
    if (ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex(POSE_PROPNAME_Center), GetCenter()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Omega), GetOmega().Radians()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Phi),   GetPhi().Radians()) ||
        ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex(POSE_PROPNAME_Kappa), GetKappa().Radians()))
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
        SetCenter(stmt.GetValuePoint3D(params.GetSelectIndex(POSE_PROPNAME_Center)));
        SetOmega(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Omega))));
        SetPhi(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Phi))));
        SetKappa(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(POSE_PROPNAME_Kappa))));
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
    SetCenter(other->GetCenter());
    SetOmega(other->GetOmega());
    SetPhi(other->GetPhi());
    SetKappa(other->GetKappa());
    }


END_BENTLEY_DATACAPTURE_NAMESPACE

