/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSqlFuncs.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnSqlFuncsForTriggers.h"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// Get the AxisAlignedBox3d from a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct iModel_placement_aabb : PlacementFunc
{
  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
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
  iModel_placement_aabb(Utf8CP name) : PlacementFunc(name, DbValueType::BlobVal) {}
};

//=======================================================================================
// Create a BoundingBox from 6 values in this order: {XLow, YLow, Zlow, XHigh, YHigh, ZHigh}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct iModel_bbox : ScalarFunction
{
  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
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
  iModel_bbox(Utf8CP name) : ScalarFunction(name, 6, DbValueType::BlobVal) {}
};

//=======================================================================================
// Get one of the values of a BoundingBox by index: {XLow=0, YLow=1, Zlow=2, XHigh=3, YHigh=4, ZHigh=5}
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct iModel_bbox_value : ScalarFunction
{
  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
  {
    int member = args[1].GetValueInt();
    if (args[0].GetValueBytes() != sizeof(ElementAlignedBox3d) || member < 0 || member > 5)
      return SetInputError(ctx);

    double const *box = (double const *)(args[0].GetValueBlob());
    ctx.SetResultDouble(box[member]);
  }
  iModel_bbox_value(Utf8CP name) : ScalarFunction(name, 2, DbValueType::FloatVal) {}
};

/*---------------------------------------------------------------------------------**/ /**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct iModel_point : BlobFunction
{
  iModel_point(Utf8CP name) : BlobFunction(name, 3) {}

  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
  {
    DPoint3d pt = DPoint3d::FromXYZ(args[0].GetValueDouble(), args[1].GetValueDouble(), args[2].GetValueDouble());
    ctx.SetResultBlob(&pt, sizeof(pt));
  }
}; /*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct iModel_placement : BlobFunction
{
  iModel_placement(Utf8CP name) : BlobFunction(name, 3) {}

  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
  {
    Placement3d placement(ToPoint(args[0]), ToAngles(args[1]), ToBBox(args[2]));
    ctx.SetResultBlob(&placement, sizeof(placement));
  }
};

//=======================================================================================
// Get the YawPitchRollAngles from a placement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct iModel_angles : ScalarFunction
{
  void _ComputeScalar(Context &ctx, int nArgs, DbValue *args) override
  {
    YawPitchRollAngles angles = YawPitchRollAngles::FromDegrees(args[0].GetValueDouble(), args[1].GetValueDouble(), args[2].GetValueDouble());
    ctx.SetResultBlob(&angles, sizeof(angles));
  }
  iModel_angles(Utf8CP name) : ScalarFunction(name, 3, DbValueType::BlobVal) {}
};

END_UNNAMED_NAMESPACE

#define ADD_FUNC(name) new iModel_##name("DGN_" #name), new iModel_##name("iModel_" #name)
/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSqlFuncsForTriggers::Register(Db &db)
{
  static DbFunction *s_funcs[] =
      {
          ADD_FUNC(angles),
          ADD_FUNC(bbox),
          ADD_FUNC(bbox_value),
          ADD_FUNC(placement_aabb),
          ADD_FUNC(point),
          ADD_FUNC(placement),
      };

  for (DbFunction *func : s_funcs)
    db.AddFunction(*func);
}
