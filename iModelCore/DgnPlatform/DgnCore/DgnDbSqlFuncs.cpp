/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDbSqlFuncs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct PlacementFunc : ScalarFunction, ScalarFunction::IScalar
{
    void SetInputError(Context* ctx)
        {
        Utf8PrintfString str("Illegal input to %s", GetName());
        ctx->SetResultError(str.c_str());
        }
    Placement3d& ToPlacement3d(DbValue* args) {return *(Placement3d*)(args[0].GetValueBlob());}
    Placement2d& ToPlacement2d(DbValue* args) {return *(Placement2d*)(args[0].GetValueBlob());}

    PlacementFunc(Utf8CP name, DbValueType valType) : ScalarFunction(name, 1, valType, this) {}
};

struct AABB_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    AABB_From_Placement() : PlacementFunc("DGN_PLACEMENT_AABB", DbValueType::BlobVal) {}
};

struct Origin_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Origin_From_Placement() : PlacementFunc("DGN_PLACEMENT_Origin", DbValueType::BlobVal) {}
};

struct Yaw_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Yaw_From_Placement() : PlacementFunc("DGN_PLACEMENT_Yaw", DbValueType::FloatVal) {}
};
struct Pitch_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Pitch_From_Placement() : PlacementFunc("DGN_PLACEMENT_Pitch", DbValueType::FloatVal) {}
};
struct Roll_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Roll_From_Placement() : PlacementFunc("DGN_PLACEMENT_Roll", DbValueType::FloatVal) {}
};
struct Width_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Width_From_Placement() : PlacementFunc("DGN_PLACEMENT_Width", DbValueType::FloatVal) {}
};
struct Height_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Height_From_Placement() : PlacementFunc("DGN_PLACEMENT_Height", DbValueType::FloatVal) {}
};
struct Left_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Left_From_Placement() : PlacementFunc("DGN_PLACEMENT_Left", DbValueType::FloatVal) {}
};
struct Right_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Right_From_Placement() : PlacementFunc("DGN_PLACEMENT_Right", DbValueType::FloatVal) {}
};
struct Bottom_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Bottom_From_Placement() : PlacementFunc("DGN_PLACEMENT_Bottom", DbValueType::FloatVal) {}
};
struct Top_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Top_From_Placement() : PlacementFunc("DGN_PLACEMENT_Top", DbValueType::FloatVal) {}
};
struct BoxVolume_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    BoxVolume_From_Placement() : PlacementFunc("DGN_PLACEMENT_Volume", DbValueType::FloatVal) {}
};
struct BoxArea_From_Placement : PlacementFunc
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    BoxArea_From_Placement() : PlacementFunc("DGN_PLACEMENT_Area2d", DbValueType::FloatVal) {}
};

struct AABB_Union : AggregateFunction, AggregateFunction::IAggregate
{
    void _StepAggregate(Context*, int nArgs, DbValue* args) override;
    void _FinishAggregate(Context*) override;

    AABB_Union() : AggregateFunction("DGN_AABB_UNION", 1, DbValueType::BlobVal, this) {}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AABB_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
    AxisAlignedBox3d aabb;
    switch (args[0].GetValueBytes())
        {
        case sizeof(Placement3d):
            aabb = ToPlacement3d(args).CalculateRange();
            break;
        case sizeof(Placement2d):
            aabb = ToPlacement2d(args).CalculateRange();
            break;

        default:
            SetInputError(ctx);
            return;
        }

    ctx->SetResultBlob(&aabb, sizeof(aabb), DbFunction::Context::CopyData::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Origin_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
    DPoint3d org;
    switch (args[0].GetValueBytes())
        {
        case sizeof(Placement3d):
            org = ToPlacement3d(args).GetOrigin();
            break;

        case sizeof(Placement2d):
            org = DPoint3d::From(ToPlacement2d(args).GetOrigin());
            break;

        default:
            SetInputError(ctx);
            return;
        }

    ctx->SetResultBlob(&org, sizeof(org), DbFunction::Context::CopyData::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Yaw_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
    switch (args[0].GetValueBytes())
        {
        case sizeof(Placement3d):
            ctx->SetResultDouble(ToPlacement3d(args).GetAngles().GetYaw().Degrees());
            return;

        case sizeof(Placement2d):
            ctx->SetResultDouble(ToPlacement2d(args).GetAngle());
            return;
        }

    SetInputError(ctx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Pitch_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
    switch (args[0].GetValueBytes())
        {
        case sizeof(Placement3d):
            ctx->SetResultDouble(ToPlacement3d(args).GetAngles().GetPitch().Degrees());
            return;

        case sizeof(Placement2d):
            ctx->SetResultDouble(0.0);
            return;
        }

    SetInputError(ctx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Roll_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
    switch (args[0].GetValueBytes())
        {
        case sizeof(Placement3d):
            ctx->SetResultDouble(ToPlacement3d(args).GetAngles().GetRoll().Degrees());
            return;

        case sizeof(Placement2d):
            ctx->SetResultDouble(0.0);
            return;
        }

    SetInputError(ctx);
    }

