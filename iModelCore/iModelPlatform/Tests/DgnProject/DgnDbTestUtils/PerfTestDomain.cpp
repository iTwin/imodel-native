    /*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/PerfTestDomain.h>
#include <DgnPlatform/GeomPart.h>
#include <DgnPlatform/ElementGeometry.h>
#include <ECDb/ECDbApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGN

DOMAIN_DEFINE_MEMBERS(PerfTestDomain)

HANDLER_DEFINE_MEMBERS(PerfElementHandler)
HANDLER_DEFINE_MEMBERS(PerfElementSub1Handler)
HANDLER_DEFINE_MEMBERS(PerfElementSub2Handler)
HANDLER_DEFINE_MEMBERS(PerfElementSub3Handler)
HANDLER_DEFINE_MEMBERS(PerfElementCHBaseHandler)
HANDLER_DEFINE_MEMBERS(PerfElementCHSub1Handler)
HANDLER_DEFINE_MEMBERS(PerfElementCHSub2Handler)
HANDLER_DEFINE_MEMBERS(PerfElementCHSub3Handler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PerfTestDomain::PerfTestDomain() : DgnDomain(PTEST_SCHEMA_NAME, "DgnPlatform Test Schema", 1)
    {
    RegisterHandler(PerfElementHandler::GetHandler());
    RegisterHandler(PerfElementSub1Handler::GetHandler());
    RegisterHandler(PerfElementSub2Handler::GetHandler());
    RegisterHandler(PerfElementSub3Handler::GetHandler());
    RegisterHandler(PerfElementCHBaseHandler::GetHandler());
    RegisterHandler(PerfElementCHSub1Handler::GetHandler());
    RegisterHandler(PerfElementCHSub2Handler::GetHandler());
    RegisterHandler(PerfElementCHSub3Handler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PerfElementPtr PerfElement::Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId, Dgn::DgnClassId parentRelClassId)
    {
    PerfElementPtr element = new PerfElement(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d(), DgnCode(), nullptr, parentId, parentRelClassId));
    return element;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PerfElementSub1Ptr PerfElementSub1::Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId, Dgn::DgnClassId parentRelClassId)
    {
    PerfElementSub1Ptr element = new PerfElementSub1(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d(), DgnCode(), nullptr, parentId, parentRelClassId));
    return element;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PerfElementSub2Ptr PerfElementSub2::Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId, Dgn::DgnClassId parentRelClassId)
    {
    PerfElementSub2Ptr element = new PerfElementSub2(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d(), DgnCode(), nullptr, parentId, parentRelClassId));
    return element;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PerfElementSub3Ptr PerfElementSub3::Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId, Dgn::DgnClassId parentRelClassId)
    {
    PerfElementSub3Ptr element = new PerfElementSub3(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d(), DgnCode(), nullptr, parentId, parentRelClassId));
    return element;
    }

#define DEFINE_PERF_ELEMENTCH_CLASS(CN,SCN)\
CN ## Ptr CN::Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId)\
    {\
    CN ## Ptr element = new CN(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d()));\
    return element;\
    }\
void CN::_BindWriteParams(BeSQLite::EC::ECSqlStatement& statement, ForInsert fi)\
    {\
    T_Super::_BindWriteParams(statement, fi);\
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(statement.GetParameterIndex(#SCN "Str"), m_ ## SCN ## _str.c_str(), IECSqlBinder::MakeCopy::Yes));\
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt64(statement.GetParameterIndex(#SCN "Long"), m_ ## SCN ## _long));\
    ASSERT_EQ(ECSqlStatus::Success, statement.BindDouble(statement.GetParameterIndex(#SCN "Double"), m_ ## SCN ## _double));\
    }\
DgnDbStatus CN::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)\
    {\
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);\
    if (DgnDbStatus::Success != status)\
        return status;\
    m_ ## SCN ## _str = stmt.GetValueText(params.GetSelectIndex(#CN "Str"));\
    m_ ## SCN ## _long = stmt.GetValueInt64(params.GetSelectIndex(#CN "Long"));\
    m_ ## SCN ## _double = stmt.GetValueDouble(params.GetSelectIndex(#CN "Double"));\
    return DgnDbStatus::Success;\
    }\
void CN ## Handler::_RegisterPropertyAccessors(Dgn::ECSqlClassInfo& params, ECN::ClassLayoutCR layout)\
    {\
    T_Super::_RegisterPropertyAccessors(params, layout);\
    params.RegisterPropertyAccessors(layout, #SCN "Str",\
        [] (ECN::ECValueR value, DgnElementCR elIn)\
            {\
            auto const& el = (CN const&) elIn;\
            value.SetUtf8CP(el.m_ ## SCN ## _str.c_str());\
            return DgnDbStatus::Success;\
            },\
        [] (DgnElementR elIn, ECN::ECValueCR value)\
            {\
            if (!value.IsString())\
                return DgnDbStatus::BadArg;\
            auto& el = (CN&) elIn;\
            el.m_ ## SCN ## _str = value.GetUtf8CP();\
            return DgnDbStatus::Success;\
            });\
    params.RegisterPropertyAccessors(layout, #SCN "Long",\
        [] (ECN::ECValueR value, DgnElementCR elIn)\
            {\
            auto const& el = (CN const&) elIn;\
            value.SetLong(el.m_ ## SCN ## _long);\
            return DgnDbStatus::Success;\
            },\
        [] (DgnElementR elIn, ECN::ECValueCR value)\
            {\
            if (!value.IsLong())\
                return DgnDbStatus::BadArg;\
            auto& el = (CN&) elIn;\
            el.m_ ## SCN ## _long = value.GetLong();\
            return DgnDbStatus::Success;\
            });\
    params.RegisterPropertyAccessors(layout, #SCN "Double",\
        [] (ECN::ECValueR value, DgnElementCR elIn)\
            {\
            auto const& el = (CN const&) elIn;\
            value.SetDouble(el.m_ ## SCN ## _double);\
            return DgnDbStatus::Success;\
            },\
        [] (DgnElementR elIn, ECN::ECValueCR value)\
            {\
            if (!value.IsDouble())\
                return DgnDbStatus::BadArg;\
            auto& el = (CN&) elIn;\
            el.m_ ## SCN ## _double = value.GetDouble();\
            return DgnDbStatus::Success;\
            });\
    }

DEFINE_PERF_ELEMENTCH_CLASS(PerfElementCHBase,Base)
DEFINE_PERF_ELEMENTCH_CLASS(PerfElementCHSub1,Sub1)
DEFINE_PERF_ELEMENTCH_CLASS(PerfElementCHSub2,Sub2)
DEFINE_PERF_ELEMENTCH_CLASS(PerfElementCHSub3,Sub3)
