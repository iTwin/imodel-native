/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/UnitTests/PerfTestDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/ViewDefinition.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/DgnElementDependency.h>
#include <DgnPlatform/TxnManager.h>

#define PERF_TEST_PERFELEMENT_CLASS_NAME "PerfElement"
#define PERF_TEST_PERFELEMENTSUB1_CLASS_NAME "PerfElementSub1"
#define PERF_TEST_PERFELEMENTSUB2_CLASS_NAME "PerfElementSub2"
#define PERF_TEST_PERFELEMENTSUB3_CLASS_NAME "PerfElementSub3"
#define PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME "PerfElementCHBase"
#define PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME "PerfElementCHSub1"
#define PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME "PerfElementCHSub2"
#define PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME "PerfElementCHSub3"

#define PTEST_SCHEMA_NAME                               "PerfTestDomain"
#define PTEST_SCHEMA_NAMEW                             L"PerfTestDomain"
#define PTEST_SCHEMA(className)                        PTEST_SCHEMA_NAME "." className
#define PTEST_DUMMY_SCHEMA_NAMEW                       L"DgnPlatformTestDummy"
#define PTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define PTEST_TEST_ELEMENT2d_CLASS_NAME                 "TestElement2d"
#define PTEST_TEST_GROUP_CLASS_NAME                     "TestGroup"
#define PTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define PTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define PTEST_TEST_ELEMENT_IntegerProperty1 "TestIntegerProperty1"
#define PTEST_TEST_ELEMENT_IntegerProperty2 "TestIntegerProperty2"
#define PTEST_TEST_ELEMENT_IntegerProperty3 "TestIntegerProperty3"
#define PTEST_TEST_ELEMENT_IntegerProperty4 "TestIntegerProperty4"
#define PTEST_TEST_ELEMENT_DoubleProperty1 "TestDoubleProperty1" 
#define PTEST_TEST_ELEMENT_DoubleProperty2 "TestDoubleProperty2" 
#define PTEST_TEST_ELEMENT_DoubleProperty3 "TestDoubleProperty3" 
#define PTEST_TEST_ELEMENT_DoubleProperty4 "TestDoubleProperty4" 
#define PTEST_TEST_ELEMENT_PointProperty1 "TestPointProperty1"  
#define PTEST_TEST_ELEMENT_PointProperty2 "TestPointProperty2"  
#define PTEST_TEST_ELEMENT_PointProperty3 "TestPointProperty3"  
#define PTEST_TEST_ELEMENT_PointProperty4 "TestPointProperty4"  

#define PTEST_CLASS_TestSpatialLocation "TestSpatialLocation"
#define PTEST_CLASS_TestPhysicalType "TestPhysicalType"
#define PTEST_CLASS_TestGraphicalType2d "TestGraphicalType2d"
#define PTEST_CLASS_TestInformationRecord "TestInformationRecord"
#define PTEST_CLASS_TestUniqueAspectNoHandler "TestUniqueAspectNoHandler"
#define PTEST_CLASS_TestMultiAspectNoHandler "TestMultiAspectNoHandler"

#define PTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME   "TestElementWithNoHandler"
#define PTEST_TEST_ELEMENT_CLASS_OVERRIDE_AUTOHADLEPROPERTIES "TestOverrideAutohadledProperties"

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
#define PTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define PTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"
#define PTEST_TEST_ITEM_TestItemLength                  "Length"
#endif
#define PTEST_TEST_UNIQUE_ASPECT_CLASS_NAME             "TestUniqueAspect"
#define PTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "TestUniqueAspectProperty"
#define PTEST_TEST_UNIQUE_ASPECT_LengthProperty         "Length"
#define PTEST_TEST_MULTI_ASPECT_CLASS_NAME              "TestMultiAspect"
#define PTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "TestMultiAspectProperty"
#define PTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"

#define PTEST_TEST_LOCATION_STRUCT_CLASS_NAME  "LocationStruct"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define BEGIN_BENTLEY_PTEST_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace DPTest {
#define END_BENTLEY_PTEST_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DPTEST using namespace BentleyApi::DPTest;

BEGIN_BENTLEY_PTEST_NAMESPACE


struct PerfElement;
struct PerfElementHandler;
DEFINE_REF_COUNTED_PTR(PerfElement)

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElement : Dgn::PhysicalElement
    {
    friend struct PerfElementHandler;
    DGNELEMENT_DECLARE_MEMBERS(PERF_TEST_PERFELEMENT_CLASS_NAME, Dgn::PhysicalElement)
        PerfElement(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENT_CLASS_NAME)); }
        static PerfElementPtr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId = Dgn::DgnElementId(), Dgn::DgnClassId parentRelClassId = Dgn::DgnClassId());
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PERF_TEST_PERFELEMENT_CLASS_NAME, PerfElement, PerfElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

struct PerfElementSub1;
struct PerfElementSub1Handler;
DEFINE_REF_COUNTED_PTR(PerfElementSub1)

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub1 : PerfElement
    {
    friend struct PerfElementSub1Handler;
    DGNELEMENT_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME, PerfElement)
        PerfElementSub1(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME)); }
        static PerfElementSub1Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId = Dgn::DgnElementId(), Dgn::DgnClassId parentRelClassId = Dgn::DgnClassId());
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub1Handler : PerfElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME, PerfElementSub1, PerfElementSub1Handler, PerfElementHandler, )
    };

struct PerfElementSub2;
struct PerfElementSub2Handler;
DEFINE_REF_COUNTED_PTR(PerfElementSub2)

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub2 : PerfElementSub1
    {
    friend struct PerfElementSub2Handler;
    DGNELEMENT_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME, PerfElementSub1)
        PerfElementSub2(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME)); }
        static PerfElementSub2Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId = Dgn::DgnElementId(), Dgn::DgnClassId parentRelClassId = Dgn::DgnClassId());
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub2Handler : PerfElementSub1Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME, PerfElementSub2, PerfElementSub2Handler, PerfElementSub1Handler, )
    };

struct PerfElementSub3;
struct PerfElementSub3Handler;
DEFINE_REF_COUNTED_PTR(PerfElementSub3)

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub3 : PerfElementSub2
    {
    friend struct PerfElementSub3Handler;
    DGNELEMENT_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, PerfElementSub2)
        PerfElementSub3(CreateParams const& params) : T_Super(params) {}

    public:
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME)); }
        static PerfElementSub3Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnElementId parentId = Dgn::DgnElementId(), Dgn::DgnClassId parentRelClassId = Dgn::DgnClassId());
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PerfElementSub3Handler : PerfElementSub2Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, PerfElementSub3, PerfElementSub3Handler, PerfElementSub2Handler, )
    };

#define DECLARE_PERF_ELEMENTCH_CLASS(CN,SCN,SUPERCN,SUPERHCN)                                                                               \
struct CN;                                                                                                                                  \
struct CN ## Handler;                                                                                                                       \
DEFINE_REF_COUNTED_PTR(CN)                                                                                                                  \
struct CN : SUPERCN                                                                                                                         \
    {                                                                                                                                       \
    friend struct PerfElementCHHandler;                                                                                                     \
    DGNELEMENT_DECLARE_MEMBERS(# CN, SUPERCN)                                                                                               \
  public:                                                                                                                                   \
    CN (CreateParams const& params) : T_Super(params) {}                                                                                    \
                                                                                                                                            \
    Utf8String m_ ## SCN ## _str;                                                                                                           \
    uint64_t m_ ## SCN ## _long;                                                                                                            \
    double m_ ## SCN ## _double;                                                                                                            \
                                                                                                                                            \
    Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;        \
    void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;                                                               \
    void _CopyFrom(Dgn::DgnElementCR el) override {/*TBD*/BeAssert("false");}                                                               \
  public:                                                                                                                                   \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PTEST_SCHEMA_NAME, # CN)); }    \
    static CN ## Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);                                            \
    };                                                                                                                                      \
                                                                                                                                            \
struct CN ## Handler : SUPERHCN                                                                                                             \
    {                                                                                                                                       \
    ELEMENTHANDLER_DECLARE_MEMBERS(# CN, CN, CN ## Handler, SUPERHCN, )                                                                     \
    void _RegisterPropertyAccessors(Dgn::ECSqlClassInfo&, ECN::ClassLayoutCR) override;                                                     \
    };

DECLARE_PERF_ELEMENTCH_CLASS(PerfElementCHBase,Base,Dgn::PhysicalElement,Dgn::dgn_ElementHandler::Physical)
DECLARE_PERF_ELEMENTCH_CLASS(PerfElementCHSub1,Sub1,PerfElementCHBase,PerfElementCHBaseHandler)
DECLARE_PERF_ELEMENTCH_CLASS(PerfElementCHSub2,Sub2,PerfElementCHSub1,PerfElementCHSub1Handler)
DECLARE_PERF_ELEMENTCH_CLASS(PerfElementCHSub3,Sub3,PerfElementCHSub2,PerfElementCHSub2Handler)


//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct PerfTestDomain : Dgn::DgnDomain
    {
private:
    DOMAIN_DECLARE_MEMBERS(PerfTestDomain, )
    PerfTestDomain();
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/" PTEST_SCHEMA_NAMEW L".ecschema.xml"; }
    };

END_BENTLEY_PTEST_NAMESPACE
