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
struct AABB_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    AABB_From_Placement() : ScalarFunction("DGN_AABB", 1, DbValueType::BlobVal, this) {}
};

struct Origin_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Origin_From_Placement() : ScalarFunction("DGN_Origin", 1, DbValueType::BlobVal, this) {}
};

struct Yaw_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Yaw_From_Placement() : ScalarFunction("DGN_Yaw", 1, DbValueType::FloatVal, this) {}
};
struct Pitch_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Pitch_From_Placement() : ScalarFunction("DGN_Pitch", 1, DbValueType::FloatVal, this) {}
};
struct Roll_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Roll_From_Placement() : ScalarFunction("DGN_Roll", 1, DbValueType::FloatVal, this) {}
};
struct Width_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Width_From_Placement() : ScalarFunction("DGN_Width", 1, DbValueType::FloatVal, this) {}
};
struct Height_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Height_From_Placement() : ScalarFunction("DGN_Height", 1, DbValueType::FloatVal, this) {}
};
struct Left_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Left_From_Placement() : ScalarFunction("DGN_Left", 1, DbValueType::FloatVal, this) {}
};
struct Right_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Right_From_Placement() : ScalarFunction("DGN_Right", 1, DbValueType::FloatVal, this) {}
};
struct Bottom_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Bottom_From_Placement() : ScalarFunction("DGN_Bottom", 1, DbValueType::FloatVal, this) {}
};
struct Top_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    Top_From_Placement() : ScalarFunction("DGN_Top", 1, DbValueType::FloatVal, this) {}
};
struct BoxVolume_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    BoxVolume_From_Placement() : ScalarFunction("DGN_BoxVolume", 1, DbValueType::FloatVal, this) {}
};
struct BoxArea_From_Placement : ScalarFunction, ScalarFunction::IScalar
{
    void _ComputeScalar(Context*, int nArgs, DbValue* args) override;
    BoxArea_From_Placement() : ScalarFunction("DGN_BoxArea2d", 1, DbValueType::FloatVal, this) {}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AABB_From_Placement::_ComputeScalar(Context* ctx, int nArgs, DbValue* args) 
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
   if (!CheckArgs(ctx, nArgs))
        return;

    const void* blob = nullptr;
            int blobSize = 0;
            if (!args[0].IsNull())
                {
                int64_t i = args[0].GetValueInt64();
                blob = &i;
                blobSize = (int) sizeof(i);
                }

            ctx->SetResultBlob(blob, blobSize, DbFunction::Context::CopyData::Yes);
#endif
    }

