/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSqlTestDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnSqlTestDomain.h"

using namespace DgnSqlTestNamespace;

HANDLER_DEFINE_MEMBERS(RobotElementHandler)
HANDLER_DEFINE_MEMBERS(ObstacleElementHandler)
DOMAIN_DEFINE_MEMBERS(DgnSqlTestDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSqlTestDomain::DgnSqlTestDomain() : DgnDomain(DGN_SQL_TEST_SCHEMA_NAME, "Sql Test Schema", 1)
    {
    RegisterHandler(RobotElementHandler::GetHandler());
    RegisterHandler(ObstacleElementHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSqlTestDomain::RegisterDomainAndImportSchema(DgnDbR db, BeFileNameCR schemasDir)
    {
    DgnDomains::RegisterDomain(DgnSqlTestDomain::GetDomain()); 

    BeFileName schemaFile = schemasDir;
    schemaFile.AppendToPath(L"ECSchemas/" DGN_SQL_TEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    BentleyStatus status = DgnBaseDomain::GetDomain().ImportSchema(db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr TestPhysicalElement::CreateBox (DPoint3dCR low, DPoint3dCR high)
    {
    DPoint3d corners[8];
    DRange3d::From(low,high).Get8Corners(corners);
    return ICurvePrimitive::CreatePointString(corners, _countof(corners));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestPhysicalElement::TestPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, DgnClassId ecClassId, DPoint3dCR origin, double yaw, ICurvePrimitiveR curve, Utf8CP elementCode)
    : PhysicalElement(CreateParams(model, ecClassId, categoryId))
    {
    SetCode(elementCode);
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*this, origin, YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(0), Angle::FromDegrees(0)));
    builder->Append(curve);
    StatusInt status = builder->SetGeomStreamAndPlacement(*this);
    BeAssert( SUCCESS == status );
    }

/*---------------------------------------------------------------------------------**//**
* Robots are always 1 meter cubes 
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
RobotElement::RobotElement(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, Utf8CP elementCode)
    : TestPhysicalElement(model, categoryId, QueryClassId(model.GetDgnDb()), origin, yaw, *CreateBox(DPoint3d::From(0,0,0), DPoint3d::From(1,1,1)), elementCode) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* Obstacles are always slabs 10 meters long, 1 meter high, and 1 cm thick
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ObstacleElement::ObstacleElement(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, Utf8CP elementCode)
    : TestPhysicalElement(model, categoryId, QueryClassId(model.GetDgnDb()), origin, yaw, *CreateBox(DPoint3d::From(0,0,0), DPoint3d::From(10,0.1,1)), elementCode) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* Obstacles are always slabs 10 meters long, 1 meter high, and 1 cm thick
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ObstacleElement::SetSomeProperty(DgnDbR db, Utf8CP value)
    {
    ECSqlStatement updateStmt;
    updateStmt.Prepare(db, "UPDATE " DGN_SQL_TEST_SCHEMA_NAME "." DGN_SQL_TEST_OBSTACLE_CLASS " SET SomeProperty=? WHERE ECInstanceId=?");
    updateStmt.BindText(1, value, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    updateStmt.BindId(2, GetElementId());
    updateStmt.Step();
    ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::Done , updateStmt.Step() );
    }

/*---------------------------------------------------------------------------------**//**
* Obstacles are always slabs 10 meters long, 1 meter high, and 1 cm thick
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ObstacleElement::SetTestItem(DgnDbR db, Utf8CP value)
    {
    ECSqlStatement insertStmt;
    insertStmt.Prepare(db, "INSERT INTO " DGN_SQL_TEST_SCHEMA_NAME "." DGN_SQL_TEST_TEST_ITEM_CLASS_NAME " (TestItemProperty,ECInstanceId) VALUES(?,?)");
    insertStmt.BindText(1, value, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    insertStmt.BindId(2, GetElementId());
    if (insertStmt.Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        {
        ECSqlStatement updateStmt;
        updateStmt.Prepare(db, "UPDATE " DGN_SQL_TEST_SCHEMA_NAME "." DGN_SQL_TEST_TEST_ITEM_CLASS_NAME " SET TestItemProperty=? WHERE ECInstanceId=?");
        updateStmt.BindText(1, value, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        updateStmt.BindId(2, GetElementId());
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::Done , updateStmt.Step() );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestPhysicalElement::Translate(DVec3dCR offset)
    {
    Placement3d rplacement = GetPlacement();
    ASSERT_TRUE( rplacement.IsValid() );
    rplacement.GetOriginR().Add(offset);
    SetPlacement(rplacement);
    }
