    /*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/TestDomainElements.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestDomainElements.h"


HANDLER_DEFINE_MEMBERS(TestElementSub1Handler)
HANDLER_DEFINE_MEMBERS(TestElementSub2Handler)
HANDLER_DEFINE_MEMBERS(TestElementSub3Handler)
HANDLER_DEFINE_MEMBERS(TestElementComplexHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub1::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop1_1"), m_prop1_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop1_2"), m_prop1_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop1_3"), m_prop1_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub1::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElementSub1::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ(0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop1_1")), "Element1 - InitValue"));
    EXPECT_EQ(10000000, stmt.GetValueInt64(params.GetSelectIndex("Prop1_2")));
    EXPECT_EQ(-3.1415, stmt.GetValueDouble(params.GetSelectIndex("Prop1_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub1::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop1_1 = "Element1 - UpdatedValue";
    m_prop1_2 = 20000000LL;
    m_prop1_3 = -6.283;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub1Ptr TestElementSub1::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new TestElementSub1(CreateParams(db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415);
    else
        return new TestElementSub1(CreateParams(db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub1CPtr TestElementSub1::Insert()
    {
    return GetDgnDb().Elements().Insert<TestElementSub1>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub1CPtr TestElementSub1::Update()
    {
    return GetDgnDb().Elements().Update<TestElementSub1>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub2::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop2_1"), m_prop2_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop2_2"), m_prop2_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop2_3"), m_prop2_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub2::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElementSub2::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ(DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ(0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop2_1")), "Element2 - InitValue"));
    EXPECT_EQ(20000000, stmt.GetValueInt64(params.GetSelectIndex("Prop2_2")));
    EXPECT_EQ(2.71828, stmt.GetValueDouble(params.GetSelectIndex("Prop2_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub2::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop2_1 = "Element2 - UpdatedValue";
    m_prop2_2 = 40000000LL;
    m_prop2_3 = 5.43656;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub2Ptr TestElementSub2::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new TestElementSub2(TestElementSub2::CreateParams(db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828);
    else
        return new TestElementSub2(TestElementSub2::CreateParams(db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub2CPtr TestElementSub2::Insert()
    {
    return GetDgnDb().Elements().Insert<TestElementSub2>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub2CPtr TestElementSub2::Update()
    {
    return GetDgnDb().Elements().Update<TestElementSub2>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub3::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex("Prop3_1"), m_prop3_1.c_str(), IECSqlBinder::MakeCopy::Yes)) ||
        (ECSqlStatus::Success != statement.BindInt64(statement.GetParameterIndex("Prop3_2"), m_prop3_2)) ||
        (ECSqlStatus::Success != statement.BindDouble(statement.GetParameterIndex("Prop3_3"), m_prop3_3)))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub3::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElementSub3::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    EXPECT_EQ(DgnDbStatus::Success, T_Super::_ReadSelectParams(stmt, params));
    EXPECT_EQ(0, strcmp(stmt.GetValueText(params.GetSelectIndex("Prop3_1")), "Element3 - InitValue"));
    EXPECT_EQ(30000000, stmt.GetValueInt64(params.GetSelectIndex("Prop3_2")));
    EXPECT_EQ(1.414121, stmt.GetValueDouble(params.GetSelectIndex("Prop3_3")));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementSub3::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop3_1 = "Element3 - UpdatedValue";
    m_prop3_2 = 60000000LL;
    m_prop3_3 = 2.828242;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub3Ptr TestElementSub3::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        return new TestElementSub3(SpatialElement::CreateParams(db, modelId, classId, category), "Element1 - InitValue", 10000000LL, -3.1415, "Element2 - InitValue", 20000000LL, 2.71828, "Element3 - InitValue", 30000000LL, 1.414121);
    else
        return new TestElementSub3(SpatialElement::CreateParams(db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub3CPtr TestElementSub3::Insert()
    {
    return GetDgnDb().Elements().Insert<TestElementSub3>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementSub3CPtr TestElementSub3::Update()
    {
    return GetDgnDb().Elements().Update<TestElementSub3>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementComplex::BindParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    if ((ECSqlStatus::Success != statement.BindPoint3D(statement.GetParameterIndex("Prop_point3d"), m_prop_DPoint3d)) )
        return DgnDbStatus::BadArg;
    IECSqlStructBinder& structBinder = statement.BindStruct(statement.GetParameterIndex("Prop_struct"));

    EXPECT_EQ(ECSqlStatus::Success, structBinder.GetMember("TestStructDoubleMember").BindDouble(m_prop_TestStruct.DoubleMember));
    EXPECT_EQ(ECSqlStatus::Success, structBinder.GetMember("TestStructIntMember").BindInt(m_prop_TestStruct.IntMember));
    EXPECT_EQ(ECSqlStatus::Success, structBinder.GetMember("TestStructBoolMember").BindBoolean(m_prop_TestStruct.BoolMember));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementComplex::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;

    return T_Super::_BindInsertParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElementComplex::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    //IECSqlStructValue const& point3dValue =stmt.GetValueStruct(params.GetSelectIndex("Prop_point3d"));
    //IECSqlStructValue const& structValue =stmt.GetValueStruct(params.GetSelectIndex("Prop_struct"));

    EXPECT_DOUBLE_EQ(1.0, m_prop_DPoint3d.x);
    EXPECT_DOUBLE_EQ(2.0, m_prop_DPoint3d.y);
    EXPECT_DOUBLE_EQ(3.0, m_prop_DPoint3d.z);

    EXPECT_DOUBLE_EQ(10, m_prop_TestStruct.IntMember);
    EXPECT_DOUBLE_EQ(10.0, m_prop_TestStruct.DoubleMember);
    EXPECT_FALSE(m_prop_TestStruct.BoolMember);

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus TestElementComplex::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    m_prop_TestStruct.BoolMember    = true;
    m_prop_TestStruct.DoubleMember  = 2.0;
    m_prop_TestStruct.IntMember     = 20;

    m_prop_DPoint3d.x = 2.0;
    m_prop_DPoint3d.y = 4.0;
    m_prop_DPoint3d.z = 6.0;

    return BindParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementComplexPtr TestElementComplex::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues)
    {
    if (specifyProperyValues)
        {
            DPoint3d point;
            point.Init(1.0, 2.0, 3.0);
            TestStruct testStruct(10, 10.0, false);
        return new TestElementComplex(SpatialElement::CreateParams(db, modelId, classId, category), point, testStruct);
        }
    else
        return new TestElementComplex(SpatialElement::CreateParams(db, modelId, classId, category));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementComplexCPtr TestElementComplex::Insert()
    {
    return GetDgnDb().Elements().Insert<TestElementComplex>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Umar.Hayat            12/05
//---------------+---------------+---------------+---------------+---------------+-------
TestElementComplexCPtr TestElementComplex::Update()
    {
    return GetDgnDb().Elements().Update<TestElementComplex>(*this);
    }

