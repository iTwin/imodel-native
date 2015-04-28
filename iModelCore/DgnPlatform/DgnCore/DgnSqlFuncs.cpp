/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSqlFuncs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct PlacementFunc : ScalarFunction
{
    Placement3d& ToPlacement3d(DbValue* args) {return *(Placement3d*)(args[0].GetValueBlob());}
    Placement2d& ToPlacement2d(DbValue* args) {return *(Placement2d*)(args[0].GetValueBlob());}

    PlacementFunc(Utf8CP name, DbValueType valType) : ScalarFunction(name, 1, valType) {}
};

//=======================================================================================
// Get the AxisAlignedBox3d from a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_placement_aabb : PlacementFunc
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
    DGN_placement_aabb() : PlacementFunc("DGN_placement_aabb", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the ElementAlignedBox3d from a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_placement_eabb : PlacementFunc
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
    DGN_placement_eabb() : PlacementFunc("DGN_placement_eabb", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the DPoint3d Origin fron a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_placement_origin : PlacementFunc
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
    DGN_placement_origin() : PlacementFunc("DGN_placement_origin", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the YawPitchRollAngles fron a Placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_placement_angles : PlacementFunc
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

    DGN_placement_angles() : PlacementFunc("DGN_placement_angles", DbValueType::BlobVal) {}
};

//=======================================================================================
// Create a YawPitchRollAngle from 3 values, in degrees {Yaw, Pitch, Roll}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_angles : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        YawPitchRollAngles angles = YawPitchRollAngles::FromDegrees(args[0].GetValueDouble(),args[1].GetValueDouble(),args[2].GetValueDouble());
        ctx.SetResultBlob(&angles, sizeof(angles));
        }
    DGN_angles() : ScalarFunction("DGN_angles", 3, DbValueType::BlobVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {Yaw=0, Pitch=1, Roll=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_angles_value : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(YawPitchRollAngles) || member<0 || member>2)
            return SetInputError(ctx);

        double const* angles= (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(angles[member]);
        }
    DGN_angles_value() : ScalarFunction("DGN_angles_value", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {Yaw=0, Pitch=1, Roll=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_angles_maxdiff : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(YawPitchRollAngles) ||
            args[1].GetValueBytes() != sizeof(YawPitchRollAngles))
            return SetInputError(ctx);

        YawPitchRollAngles& angle1 = *((YawPitchRollAngles*)(args[0].GetValueBlob()));
        YawPitchRollAngles& angle2 = *((YawPitchRollAngles*)(args[1].GetValueBlob()));

        ctx.SetResultDouble(angle1.MaxDiffDegrees(angle2));
        }
    DGN_angles_maxdiff() : ScalarFunction("DGN_angles_maxdiff", 2, DbValueType::FloatVal) {}
};


//=======================================================================================
// Create a BoundingBox from 6 values in this order: {XLow, YLow, Zlow, XHigh, YHigh, ZHigh}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox : ScalarFunction
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
    DGN_bbox() : ScalarFunction("DGN_bbox", 6, DbValueType::BlobVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_width : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetWidth());
        }
    DGN_bbox_width() : ScalarFunction("DGN_bbox_width", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_height : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetHeight());
        }
    DGN_bbox_height() : ScalarFunction("DGN_bbox_height", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_depth : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->GetDepth());
        }
    DGN_bbox_depth() : ScalarFunction("DGN_bbox_depth", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_volume : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);

        ctx.SetResultDouble(((ElementAlignedBox3d*)(args[0].GetValueBlob()))->Volume());
        }
    DGN_bbox_volume() : ScalarFunction("DGN_bbox_volume", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_areaxy : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);


        ElementAlignedBox3d& box = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        ctx.SetResultDouble(box.GetWidth() * box.GetHeight());
        }

    DGN_bbox_areaxy() : ScalarFunction("DGN_bbox_areaxy", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_overlaps : ScalarFunction
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
    DGN_bbox_overlaps() : ScalarFunction("DGN_bbox_overlaps", 2, DbValueType::IntegerVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_contained : ScalarFunction
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
    DGN_bbox_contained() : ScalarFunction("DGN_bbox_contained", 2, DbValueType::IntegerVal) {}
};

//=======================================================================================
// Get one of the values of a BoundingBox by index: {XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_value : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d) || member<0 || member>5)
            return SetInputError(ctx);

        double const* box = (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(box[member]);
        }
    DGN_bbox_value() : ScalarFunction("DGN_bbox_value", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_bbox_union : AggregateFunction
{
    struct Result {bool m_valid; DRange3d m_range;};

    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(DRange3d))
            return SetInputError(ctx);
        
        DRange3d&  thisRange  = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        Result&    totalRange = *((Result*)ctx.GetAggregateContext(sizeof(Result)));
        if (!totalRange.m_valid)
            {
            totalRange.m_range = thisRange;
            totalRange.m_valid = true;
            }
        else
            {
            totalRange.m_range.Extend(thisRange);
            }
        }

    void _FinishAggregate(Context& ctx) override
        {
        Result* totalRange = (Result*) ctx.GetAggregateContext(0);
        if (totalRange && totalRange->m_valid)
            ctx.SetResultBlob(&totalRange->m_range, sizeof(totalRange->m_range));
        else
            ctx.SetResultNull();
        }

    DGN_bbox_union() : AggregateFunction("DGN_bbox_union", 1, DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the distance between two points
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_point_distance : ScalarFunction
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
    DGN_point_distance() : ScalarFunction("DGN_point_distance", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {X=0, Y=1, Z=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DGN_point_value : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(DPoint3d) || member<0 || member>2)
            return SetInputError(ctx);

        double const* pt= (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(pt[member]);
        }
    DGN_point_value() : ScalarFunction("DGN_point_value", 2, DbValueType::FloatVal) {}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSchemaDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    static DbFunction* s_funcs[] = 
                          {
                          new DGN_angles_value,
                          new DGN_angles,
                          new DGN_angles_maxdiff,
                          new DGN_bbox_areaxy,
                          new DGN_bbox_depth,
                          new DGN_bbox_height,
                          new DGN_bbox_contained,
                          new DGN_bbox_overlaps,
                          new DGN_bbox_union,
                          new DGN_bbox_value,
                          new DGN_bbox_volume,
                          new DGN_bbox_width,
                          new DGN_bbox,
                          new DGN_placement_aabb,
                          new DGN_placement_angles,
                          new DGN_placement_eabb,
                          new DGN_placement_origin,
                          new DGN_point_distance,
                          new DGN_point_value
                          };

    for (DbFunction* func : s_funcs)
        db.AddFunction(*func);
    }
