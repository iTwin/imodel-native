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
struct SelfScalar : ScalarFunction, ScalarFunction::IScalar
{
    SelfScalar(Utf8CP name, int nArgs, DbValueType valType) : ScalarFunction(name, nArgs, valType, this) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct PlacementFunc : SelfScalar
{
    Placement3d& ToPlacement3d(DbValue* args) {return *(Placement3d*)(args[0].GetValueBlob());}
    Placement2d& ToPlacement2d(DbValue* args) {return *(Placement2d*)(args[0].GetValueBlob());}

    PlacementFunc(Utf8CP name, DbValueType valType) : SelfScalar(name, 1, valType) {}
};

//=======================================================================================
// Get the AxisAlignedBox3d from a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_PLACEMENT_AABB : PlacementFunc
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        AxisAlignedBox3d bb;
        switch (args[0].GetValueBytes())
            {
            case sizeof(Placement3d):
                bb = ToPlacement3d(args).CalculateRange();
                break;
            case sizeof(Placement2d):
                bb = ToPlacement2d(args).CalculateRange();
                break;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&bb, sizeof(bb));
        }
    DGN_PLACEMENT_AABB() : PlacementFunc("DGN_PLACEMENT_AABB", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the ElementAlignedBox3d from a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_PLACEMENT_EABB : PlacementFunc
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        ElementAlignedBox3d bb;
        switch (args[0].GetValueBytes())
            {
            case sizeof(Placement3d):
                bb = ToPlacement3d(args).GetElementBox();
                break;
            case sizeof(Placement2d):
                bb = ElementAlignedBox3d(ToPlacement2d(args).GetElementBox());
                break;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&bb, sizeof(bb));
        }
    DGN_PLACEMENT_EABB() : PlacementFunc("DGN_PLACEMENT_EABB", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the DPoint3d Origin fron a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_PLACEMENT_Origin : PlacementFunc
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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

        ctx.SetResultBlob(&org, sizeof(org));
        }
    DGN_PLACEMENT_Origin() : PlacementFunc("DGN_PLACEMENT_Origin", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the YawPitchRollAngles fron a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_PLACEMENT_Angles : PlacementFunc
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        YawPitchRollAngles angles;
        switch (args[0].GetValueBytes())
            {
            case sizeof(Placement3d):
                angles = ToPlacement3d(args).GetAngles();
                break;

            case sizeof(Placement2d):
                angles = YawPitchRollAngles::FromDegrees(ToPlacement2d(args).GetAngle(), 0.0, 0.0);
                break;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&angles, sizeof(angles));
        }

    DGN_PLACEMENT_Angles() : PlacementFunc("DGN_PLACEMENT_Angles", DbValueType::BlobVal) {}
};

//=======================================================================================
// Create a YawPitchRollAngle from 3 values, in degrees {Yaw, Pitch, Roll}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_Angles : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        YawPitchRollAngles angles = YawPitchRollAngles::FromDegrees(args[0].GetValueDouble(),args[1].GetValueDouble(),args[2].GetValueDouble());
        ctx.SetResultBlob(&angles, sizeof(angles));
        }
    DGN_Angles() : SelfScalar("DGN_Angles", 3, DbValueType::BlobVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {Yaw=0, Pitch=1, Roll=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_ANGLES_Value : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(YawPitchRollAngles) || member<0 || member>2)
            return SetInputError(ctx);

        double const* angles= (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(angles[member]);
        }
    DGN_ANGLES_Value() : SelfScalar("DGN_ANGLES_Value", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// Create a BoundingBox from 6 values in this order: {XLow, YLow, Zlow, XHigh, YHigh, ZHigh}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBox : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        DRange3d box;
        box.InitFrom(args[0].GetValueDouble(),
                     args[1].GetValueDouble(),
                     args[2].GetValueDouble(),
                     args[3].GetValueDouble(),
                     args[4].GetValueDouble(),
                     args[5].GetValueDouble());

        ctx.SetResultBlob(&box, sizeof(box));
        }
    DGN_BBox() : SelfScalar("DGN_BBox", 6, DbValueType::BlobVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Width : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetWidth());
        }
    DGN_BBOX_Width() : SelfScalar("DGN_BBOX_Width", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Height : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetHeight());
        }
    DGN_BBOX_Height() : SelfScalar("DGN_BBOX_Height", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Depth : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetDepth());
        }
    DGN_BBOX_Depth() : SelfScalar("DGN_BBOX_Depth", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Volume : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->Volume());
        }
    DGN_BBOX_Volume() : SelfScalar("DGN_BBOX_Volume", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_AreaXY : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);


        ElementAlignedBox3d& box = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        ctx.SetResultDouble(box.GetWidth() * box.GetHeight());
        }

    DGN_BBOX_AreaXY() : SelfScalar("DGN_BBOX_AreaXY", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Overlaps : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d) ||
            args[1].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ElementAlignedBox3d& box1 = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        ElementAlignedBox3d& box2 = *((ElementAlignedBox3d*)(args[1].GetValueBlob()));
        ctx.SetResultInt(box1.IntersectsWith(box2));
        }
    DGN_BBOX_Overlaps() : SelfScalar("DGN_BBOX_Overlaps", 2, DbValueType::IntegerVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_IsContained : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d) ||
            args[1].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ElementAlignedBox3d& box1 = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        ElementAlignedBox3d& box2 = *((ElementAlignedBox3d*)(args[1].GetValueBlob()));
        ctx.SetResultInt(box2.IsContained(box1));
        }
    DGN_BBOX_IsContained() : SelfScalar("DGN_BBOX_IsContained", 2, DbValueType::IntegerVal) {}
};

//=======================================================================================
// Get one of the values of a BoundingBox by index: {XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Value : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d) || member<0 || member>5)
            return SetInputError(ctx);

        double const* box = (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(box[member]);
        }
    DGN_BBOX_Value() : SelfScalar("DGN_BBOX_Value", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_BBOX_Union : AggregateFunction, AggregateFunction::IAggregate
{
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override;
    void _FinishAggregate(Context& ctx) override;

    DGN_BBOX_Union() : AggregateFunction("DGN_BBOX_Union", 1, DbValueType::BlobVal, this) {}
};

//=======================================================================================
// Get the distance between two points
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_POINT_Distance : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(DPoint3d) ||
            args[1].GetValueBytes() != sizeof(DPoint3d))
            return SetInputError(ctx);

        DPoint3d& pt1 = *((DPoint3d*)(args[0].GetValueBlob()));
        DPoint3d& pt2 = *((DPoint3d*)(args[1].GetValueBlob()));
        ctx.SetResultDouble(pt1.Distance(pt2));
        }
    DGN_POINT_Distance() : SelfScalar("DGN_POINT_Distance", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {X=0, Y=1, Z=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_POINT_Value : SelfScalar
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(DPoint3d) || member<0 || member>2)
            return SetInputError(ctx);

        double const* pt= (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(pt[member]);
        }
    DGN_POINT_Value() : SelfScalar("DGN_POINT_Value", 2, DbValueType::FloatVal) {}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::RegisterSQLFuncs()
    {
    static DbFunction* s_funcs[] = {
                          new DGN_ANGLES_Value,
                          new DGN_Angles,
                          new DGN_BBOX_AreaXY,
                          new DGN_BBOX_Depth,
                          new DGN_BBOX_Height,
                          new DGN_BBOX_IsContained,
                          new DGN_BBOX_Overlaps,
//                          new DGN_BBOX_Union,
                          new DGN_BBOX_Value,
                          new DGN_BBOX_Volume,
                          new DGN_BBOX_Width,
                          new DGN_BBox,
                          new DGN_PLACEMENT_AABB,
                          new DGN_PLACEMENT_Angles,
                          new DGN_PLACEMENT_EABB,
                          new DGN_PLACEMENT_Origin,
                          new DGN_POINT_Distance,
                          new DGN_POINT_Value};

    for (DbFunction* func : s_funcs)
        AddFunction(*func);
    }
