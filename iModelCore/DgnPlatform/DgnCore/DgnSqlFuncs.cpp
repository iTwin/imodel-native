/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSqlFuncs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

BEGIN_UNNAMED_NAMESPACE

#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/** @addtogroup DgnDbSqlFunctions DgnDb SQL Functions
 *  @{

DgnDb defines types and built-in functions that you can use in SQL statements.

@see DGN_placement, DGN_angles, DGN_bbox, DGN_point
*/

//  The following are not real C++ structs. They are here to document the objects that our SQL extensions work with.
//  Likewise, all of the function declarations within #ifdef DOCUMENTATION_GENERATOR/#endif are not real C functions.
//  They are here to represent the SQL extension functions, so that Doyxgen will generate doc for them.

//! A point in the DgnDb's Cartesian coordinate system. All coordinates are in meters. If the point represents a location in a 2D model, then the z-coordinate will be zero.
//! @see DGN_point_value, DGN_point_distance, DGN_point_min_distance_to_bbox 
struct DGN_point
{
protected:
    double x;   //!< The x-coordinate
    double y;   //!< The y-coordinate
    double z;   //!< The z-coordinate
};

//! An object that contains Yaw, Pitch, and Roll angles in degrees. If this object represents a rotation in a 2D model, then the pitch and roll members will be zero.
//! @see DGN_angles_value, DGN_angles_maxdiff
struct DGN_angles
{
protected:
    double yaw;  //!< The Yaw angle in degrees
    double pitch;//!< The Yaw angle in degrees
    double roll; //!< The Yaw angle in degrees
};

//! An object that defines a range.
//! If the box represents a range in a 3-D model, then the box will have 8 corners and will have width(X), depth(Y), and height(Z). 
//! If the box represents a range in a 2-D model, then the box will still have 8 corners but the z-coordinates will be all be zero, and the height will be zero.
//! All coordinates are in meters.
//! @see DGN_bbox_value, DGN_bbox_width, DGN_bbox_height, DGN_bbox_depth, DGN_bbox_volume, DGN_bbox_areaxy, DGN_bbox_overlaps, DGN_bbox_contains, DGN_bbox_union, DGN_point_min_distance_to_bbox
struct DGN_bbox
{
protected:
    double XLow;  //!< The low X coordinate of the bounding box
    double YLow;  //!< The low Y coordinate of the bounding box
    double Zlow;  //!< The low Z coordinate of the bounding box
    double XHigh; //!< The high X coordinate of the bounding box
    double YHigh; //!< The high Y coordinate of the bounding box
    double ZHigh; //!< The high Z coordinate of the bounding box
};

//! An object that contains an origin and rotation angles, plus a bounding box.
//! You can obtain an element's placement by selecting the placement column of the dgn_ElementGeom table.
//! @see DGN_placement_origin, DGN_placement_angles, DGN_placement_eabb, DGN_placement_aabb
struct DGN_placement
{
protected:
    DGN_point origin;   //!< Origin
    DGN_angles angles;  //!< Angles
    DGN_bbox bbox;      //!< Element-aligned bounding box
};
// __PUBLISH_SECTION_END__
#endif

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
// Get the AxisAlignedBox3d from a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get the axis-aligned bounding box from a placement
    @param placement   The DGN_placement object to query
    @return the bounding box
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_bbox_union.sampleCode
*/
DGN_bbox DGN_placement_aabb(DGN_placement placement);
// __PUBLISH_SECTION_END__
#endif
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
            case 0:
                ctx.SetResultNull();
                return;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&bb, sizeof(bb));
        }
    DGN_placement_aabb() : PlacementFunc("DGN_placement_aabb", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the ElementAlignedBox3d from a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get the element-aligned bounding box from a placement
    @param placement   The DGN_placement object to query
    @return the bounding box
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_sum.sampleCode
*/
DGN_bbox DGN_placement_eabb(DGN_placement placement);
// __PUBLISH_SECTION_END__
#endif
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
            case 0:
                ctx.SetResultNull();
                return;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&bb, sizeof(bb));
        }
    DGN_placement_eabb() : PlacementFunc("DGN_placement_eabb", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the DPoint3d Origin fron a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get the placement origin
    @param placement   The DGN_placement object to query
    @return the origin in world coordinates
*/
DGN_point DGN_placement_origin(DGN_placement placement);
// __PUBLISH_SECTION_END__
#endif
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

            case 0:
                ctx.SetResultNull();
                return;

            default:
                SetInputError(ctx);
                return;
            }

        ctx.SetResultBlob(&org, sizeof(org));
        }
    DGN_placement_origin() : PlacementFunc("DGN_placement_origin", DbValueType::BlobVal) {}
};

//=======================================================================================
// Get the YawPitchRollAngles fron a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get the placement angles
    @param placement   The DGN_placement object to query
    @return the placement angles
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_angles.sampleCode
*/
DGN_angles DGN_placement_angles(DGN_placement placement);
// __PUBLISH_SECTION_END__
#endif
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
                angles = YawPitchRollAngles(ToPlacement2d(args).GetAngle(), AngleInDegrees(), AngleInDegrees());
                break;

            case 0:
                ctx.SetResultNull();
                return;

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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Construct a DGN_angles from 3 values
    @param yaw The Yaw angle in degrees
    @param pitch The Pitch angle in degrees
    @param roll The Roll angle in degrees
    @return a DGN_angles object
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_angles.sampleCode
*/
DGN_angles DGN_angles(double yaw, double pitch, double roll);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get a member of a DGN_angles object
    @param angles   The DGN_angles object to query
    @param member   The index of the member to get: Yaw=0, Pitch=1, Roll=2 
    @return the selected angle (in degrees); or an error if member is out of range or if \a angles is not a DGN_angles object
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_angles_value.sampleCode
*/
double DGN_angles_value(DGN_angles angles, int member);
// __PUBLISH_SECTION_END__
#endif
struct DGN_angles_value : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].IsNull())
            {
            ctx.SetResultNull();
            return;
            }

        int member = args[1].GetValueInt();
        if (args[0].GetValueBytes() != sizeof(YawPitchRollAngles) || member<0 || member>2)
            return SetInputError(ctx);

        double const* angles= (double const*)(args[0].GetValueBlob());
        ctx.SetResultDouble(angles[member]);
        }
    DGN_angles_value() : ScalarFunction("DGN_angles_value", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// return the maximum absolute difference among the angles.
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Return the maximum absolute difference among the angles in degrees.
    @param angle1 a DGN_angles object
    @param angle2 a DGN_angles object
    @return the maximum absolute difference among the angles in degrees.
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_angles_maxdiff.sampleCode
*/
double DGN_angles_maxdiff(DGN_angles angle1, DGN_angles angle2);
// __PUBLISH_SECTION_END__
#endif
struct DGN_angles_maxdiff : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].IsNull() || args[1].IsNull())
            {
            ctx.SetResultNull();
            return;
            }

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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Create a bounding box from 6 values
    All coordinates are in meters.
    @param XLow     The low X coordinate of the bounding box
    @param YLow     The low Y coordinate of the bounding box
    @param Zlow     The low Z coordinate of the bounding box
    @param XHigh    The high X coordinate of the bounding box
    @param YHigh    The high Y coordinate of the bounding box
    @param ZHigh    The high Z coordinate of the bounding box
    @return a DGN_bbox object
*/
DGN_bbox DGN_bbox(double XLow, double YLow, double Zlow, double XHigh, double YHigh, double ZHigh);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the "width" of a bounding box
    @param bb       a bounding box
    @return the difference between the high and low X coordinates of the box, in meters.
    @see DGN_bbox_areaxy
*/
double DGN_bbox_width(DGN_bbox bb);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the "height" of a bounding box
    @param bb       a bounding box
    @return the difference between the high and low Z coordinates of the box, in meters.
*/
double DGN_bbox_height(DGN_bbox bb);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the "depth" of a bounding box
    @param bb       a bounding box
    @return the difference between the high and low Y coordinates of the box, in meters.
    @see DGN_bbox_areaxy
*/
double DGN_bbox_depth(DGN_bbox bb);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the volume of the bounding box
    @param bb       a bounding box
    @return Its volume in cubic meters
    @see DGN_bbox_areaxy
*/
double DGN_bbox_volume(DGN_bbox bb);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the depth times the width of a bounding box
    @param bb       a bounding box
    @return the depth of \a bb times its width; or, an error if the input object is not a DGN_bbox
    @see DGN_bbox_volume
    @see DGN_bbox_depth, DGN_bbox_width
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_sum.sampleCode
    <p>
    <p>Here is an example of an \em incorrect call to DGN_bbox_areaxy
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_bbox_areaxy_error.sampleCode
*/
double DGN_bbox_areaxy(DGN_bbox bb);
// __PUBLISH_SECTION_END__
#endif
struct DGN_bbox_areaxy : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d))
            return SetInputError(ctx);


        ElementAlignedBox3d& box = *((ElementAlignedBox3d*)(args[0].GetValueBlob()));
        ctx.SetResultDouble(box.GetWidth() * box.GetDepth());
        }

    DGN_bbox_areaxy() : ScalarFunction("DGN_bbox_areaxy", 1, DbValueType::FloatVal) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Determine if the areas enclosed by two 3-D bounding boxes overlap
    @param bb1       The first bounding box
    @param bb2       The second bounding box
    @return 1 if the boxes overlap or 0 if not.
    @see DGN_bbox_contains
*/
int DGN_bbox_overlaps(DGN_bbox bb1, DGN_bbox bb2);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Determine of the first bounding box contains the second bounding box
    @param bb_outer  The containing bounding box
    @param bb_inner  The contained bounding box
    @return 1 if bb_outer contains bb_inner or 0 if not.
    @see DGN_bbox_overlaps
*/
int DGN_bbox_contains(DGN_bbox bb_outer, DGN_bbox bb_inner);
// __PUBLISH_SECTION_END__
#endif
struct DGN_bbox_contains : ScalarFunction
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
    DGN_bbox_contains() : ScalarFunction("DGN_bbox_contains", 2, DbValueType::IntegerVal) {}
};

//=======================================================================================
// Get one of the values of a BoundingBox by index: {XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get a member of a DGN_bbox object
    @param bb       a bounding box
    @param member   The index of the member to get: XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5
    @return the requested member of the bounding box; or an error if member is out of range or bb is not a DGN_bbox object.
*/
double DGN_bbox_value(DGN_bbox bb, int member);
// __PUBLISH_SECTION_END__
#endif
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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    \em Aggregate function that computes the union of a series of bounding boxes
    @return a bounding box that contains the aggregated range.
    <p><b>Example (C++)</b>
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_bbox_union.sampleCode
*/
DGN_bbox DGN_bbox_union(DGN_bbox);
// __PUBLISH_SECTION_END__
#endif
struct DGN_bbox_union : AggregateFunction
{
    struct Result {bool m_valid; DRange3d m_range;};

    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].IsNull())
            return;

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
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the distance between two DGN_Points, in meters.
    @param point1   A point 
    @param point2   A second point 
    @return the distance between the two points; or an error if either input is not a DGN_point object
*/
double DGN_point_distance(DGN_point point1, DGN_point point2);
// __PUBLISH_SECTION_END__
#endif
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
// Get the minimun distance from a point to a bounding box
// @bsiclass                                                   Sam.Wilson   05/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Compute the minimum distance from a point to a bounding box, in meters.
    @param point    A point 
    @param bbox     A bounding box
    @return the distance from \a point to the closest point on \a bbox; or an error if either input is of the wrong type.
*/
double DGN_point_min_distance_to_bbox(DGN_point point, DGN_bbox bbox);
// __PUBLISH_SECTION_END__
#endif
struct DGN_point_min_distance_to_bbox : ScalarFunction
{
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        if (args[0].GetValueBytes() != sizeof(DPoint3d) ||
            args[1].GetValueBytes() != sizeof(DRange3d))
            return SetInputError(ctx);

        DPoint3d& pt   = *((DPoint3d*)(args[0].GetValueBlob()));
        DRange3d& bbox = *((DRange3d*)(args[1].GetValueBlob()));
        ctx.SetResultDouble(bbox.DistanceOutside(pt));
        }
    DGN_point_min_distance_to_bbox() : ScalarFunction("DGN_point_min_distance_to_bbox", 2, DbValueType::FloatVal) {}
};

//=======================================================================================
// Get one of the values of a DPoint3d by index: {X=0, Y=1, Z=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    Get a member of a DGN_Point object.
    @param point    The point to query
    @param member   The index of the coordinate to get: X=0, Y=1, Z=2
    @return a coordindate of the point in meters; or an error if \a member is out of range or \a point is not a point object
*/
double DGN_point_value(DGN_point point, int member);
// __PUBLISH_SECTION_END__
#endif
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

//=======================================================================================
// Get one of the values of a DPoint3d by index: {X=0, Y=1, Z=2}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
#ifdef DOCUMENTATION_GENERATOR
// __PUBLISH_SECTION_START__
/**
    An rtree MATCH function that only accepts objects from the RTree whose range overlap an aabb.
    <p><b>Example (C++)</b>
    <p>Here is an example of DGN_rtree_aabb_overlap that searches for elements using both an Axis-aligned box and addtional WHERE criteria.
    __PUBLISH_INSERT_FILE__ DgnSchemaDomain_SqlFuncs_DGN_rtree_overlap_aabb.sampleCode
*/
void DGN_rtree_aabb_overlap(DGN_aabb);
// __PUBLISH_SECTION_END__
#endif
struct DGN_rtree_overlap_aabb : RTreeMatchFunction
{
    int _TestRange(QueryInfo const& info) override
        {
        if (info.m_nParam != 1 || info.m_args[0].GetValueBytes() != sizeof(DRange3d))
            return BE_SQLITE_ERROR;

        info.m_within = Within::Outside;

        RTree3dVal bounds(*(DRange3dCP) info.m_args[0].GetValueBlob());
        RTree3dValCP pt = (RTree3dValCP) info.m_coords;

        bool passedTest = (info.m_parentWithin == Within::Inside) ? true : bounds.Intersects(*pt);
        if (!passedTest)
            return BE_SQLITE_OK;

        if (info.m_level>0)
            {
            // For nodes, return 'level-score'.
            info.m_score = info.m_level;
            info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : bounds.Contains(*pt) ? Within::Inside : Within::Partly;
            }
        else
            {
            // For entries (ilevel==0), we return 0 so they are processed immediately (lowest score has highest priority).
            info.m_score = 0;
            info.m_within = Within::Partly;
            }
        return BE_SQLITE_OK;
        }

    DGN_rtree_overlap_aabb() : RTreeMatchFunction("DGN_rtree_overlap_aabb", 1) {}
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnBaseDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    db.Memory().AddConsumer(db.Elements(), IMemoryConsumer::Priority::Highest);

    static DbFunction* s_funcs[] = 
                          {
                          new DGN_angles_value,
                          new DGN_angles,
                          new DGN_angles_maxdiff,
                          new DGN_bbox_areaxy,
                          new DGN_bbox_depth,
                          new DGN_bbox_height,
                          new DGN_bbox_contains,
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
                          new DGN_point_min_distance_to_bbox,
                          new DGN_point_value
                          };

    static RTreeMatchFunction* s_matchFuncs[] = 
                         {
                         new DGN_rtree_overlap_aabb
                         };

    for (DbFunction* func : s_funcs)
        db.AddFunction(*func);

    for (RTreeMatchFunction* func : s_matchFuncs)
        db.AddRTreeMatchFunction(*func);
    }

#ifdef DOCUMENTATION_GENERATOR  // -- NB: This closing @}  closes the @addtogroup DgnDbSqlFunctions @{  at the top of the file
// __PUBLISH_SECTION_START__
//! @}
// __PUBLISH_SECTION_END__
#endif
