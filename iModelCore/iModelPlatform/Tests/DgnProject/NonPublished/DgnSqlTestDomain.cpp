/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnSqlTestDomain.h"
#include <Bentley/BeTest.h>

using namespace DgnSqlTestNamespace;

HANDLER_DEFINE_MEMBERS(RobotElementHandler)
HANDLER_DEFINE_MEMBERS(ObstacleElementHandler)
DOMAIN_DEFINE_MEMBERS(DgnSqlTestDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSqlTestDomain::DgnSqlTestDomain() : DgnDomain(DGN_SQL_TEST_SCHEMA_NAME, "Sql Test Schema", 1)
    {
    RegisterHandler(RobotElementHandler::GetHandler());
    RegisterHandler(ObstacleElementHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr createBox (DPoint3dCR low, DPoint3dCR high)
    {
    DPoint3d corners[8];
    DRange3d::From(low,high).Get8Corners(corners);
    return ICurvePrimitive::CreatePointString(corners, _countof(corners));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUpElement(PhysicalElementR el, DgnModelR model, DPoint3dCR origin, double yaw, ICurvePrimitiveR curve, DgnCode elementCode)
    {
    el.SetCode(elementCode);
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, el.GetCategoryId(), origin, YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(0), Angle::FromDegrees(0)));
    builder->Append(curve);
    StatusInt status = builder->Finish(el);
    ASSERT_TRUE( SUCCESS == status );
    }

/*---------------------------------------------------------------------------------**//**
* Robots are always 1 meter cubes
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RobotElement::RobotElement(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode)
    : PhysicalElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId))
    {
    setUpElement(*this, model, origin, yaw, *createBox(DPoint3d::From(0,0,0), DPoint3d::From(1,1,1)), elementCode);
    }

/*---------------------------------------------------------------------------------**//**
* Obstacles are always slabs 10 meters long, 1 meter high, and 1 cm thick
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ObstacleElement::ObstacleElement(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode)
    : PhysicalElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId))
    {
    setUpElement(*this, model, origin, yaw, *createBox(DPoint3d::From(0,0,0), DPoint3d::From(10,0.1,1)), elementCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ObstacleElement::SetSomeProperty(Utf8CP value)
    {
    ASSERT_EQ(DgnDbStatus::Success, SetPropertyValue("SomeProperty", value));
    ASSERT_EQ(DgnDbStatus::Success, Update());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ObstacleElement::SetTestUniqueAspect(Utf8CP value)
    {
    auto eclass = GetDgnDb().Schemas().GetClass(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_TEST_UNIQUE_ASPECT_CLASS_NAME);
    auto instance = eclass->GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue("TestUniqueAspectProperty", ECN::ECValue(value));
    DgnDbStatus stat;
    auto createdAspectPtr = DgnElement::GenericUniqueAspect::SetAspect(*this, *instance, nullptr, &stat);
    EXPECT_TRUE(createdAspectPtr.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_EQ(DgnDbStatus::Success, Update());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ObstacleElement::GetTestUniqueAspect() const
    {
    auto eclass = GetDgnDb().Schemas().GetClass(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_TEST_UNIQUE_ASPECT_CLASS_NAME);
    auto inst = DgnElement::GenericUniqueAspect::GetAspect(*this, *eclass);
    if (nullptr == inst)
        return "";
    ECN::ECValue value;
    inst->GetValue(value, "TestUniqueAspectProperty");
    return value.ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RobotElement::Translate(DVec3dCR offset)
    {
    Placement3d rplacement = GetPlacement();
    ASSERT_TRUE( rplacement.IsValid() );
    rplacement.GetOriginR().Add(offset);
    SetPlacement(rplacement);
    }
