/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnSqlFuncs.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>
#include <Geom/GeomApi.h>
#include <PlacementOnEarth/Placement.h>

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct PlacementFunc : ScalarFunction
{
  Placement3d &ToPlacement3d(DbValue *args) { return *(Placement3d *)(args[0].GetValueBlob()); }
  Placement2d &ToPlacement2d(DbValue *args) { return *(Placement2d *)(args[0].GetValueBlob()); }

  PlacementFunc(Utf8CP name, DbValueType valType) : ScalarFunction(name, 1, valType) {}
};

/*---------------------------------------------------------------------------------**/ /**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct BlobFunction : ScalarFunction
{
protected:
  BlobFunction(Utf8CP name, int nArgs) : ScalarFunction(name, nArgs, DbValueType::BlobVal) {}

  DPoint3d const &ToPoint(DbValue &value) const { return *reinterpret_cast<DPoint3d const *>(value.GetValueBlob()); }
  YawPitchRollAngles const &ToAngles(DbValue &value) const { return *reinterpret_cast<YawPitchRollAngles const *>(value.GetValueBlob()); }
  ElementAlignedBox3d const &ToBBox(DbValue &value) const { return *reinterpret_cast<ElementAlignedBox3d const *>(value.GetValueBlob()); }
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson    07/2018
//=======================================================================================
struct DgnSqlFuncsForTriggers
{
  static void Register(Db &);
};

END_BENTLEY_NAMESPACE
