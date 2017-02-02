/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnView.h>
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

#define DPTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define DPTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define DPTEST_SCHEMA(className)                        DPTEST_SCHEMA_NAME "." className
#define DPTEST_DUMMY_SCHEMA_NAMEW                       L"DgnPlatformTestDummy"
#define DPTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define DPTEST_TEST_ELEMENT2d_CLASS_NAME                 "TestElement2d"
#define DPTEST_TEST_GROUP_CLASS_NAME                     "TestGroup"
#define DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define DPTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define DPTEST_TEST_ELEMENT_IntegerProperty1 "TestIntegerProperty1"
#define DPTEST_TEST_ELEMENT_IntegerProperty2 "TestIntegerProperty2"
#define DPTEST_TEST_ELEMENT_IntegerProperty3 "TestIntegerProperty3"
#define DPTEST_TEST_ELEMENT_IntegerProperty4 "TestIntegerProperty4"
#define DPTEST_TEST_ELEMENT_DoubleProperty1 "TestDoubleProperty1" 
#define DPTEST_TEST_ELEMENT_DoubleProperty2 "TestDoubleProperty2" 
#define DPTEST_TEST_ELEMENT_DoubleProperty3 "TestDoubleProperty3" 
#define DPTEST_TEST_ELEMENT_DoubleProperty4 "TestDoubleProperty4" 
#define DPTEST_TEST_ELEMENT_PointProperty1 "TestPointProperty1"  
#define DPTEST_TEST_ELEMENT_PointProperty2 "TestPointProperty2"  
#define DPTEST_TEST_ELEMENT_PointProperty3 "TestPointProperty3"  
#define DPTEST_TEST_ELEMENT_PointProperty4 "TestPointProperty4"  

#define DPTEST_CLASS_TestSpatialLocation "TestSpatialLocation"
#define DPTEST_CLASS_TestPhysicalType "TestPhysicalType"
#define DPTEST_CLASS_TestGraphicalType2d "TestGraphicalType2d"

#define DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME   "TestElementWithNoHandler"
#define DPTEST_TEST_ELEMENT_CLASS_OVERRIDE_AUTOHADLEPROPERTIES "TestOverrideAutohadledProperties"

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
#define DPTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define DPTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"
#define DPTEST_TEST_ITEM_TestItemLength                  "Length"
#endif
#define DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME             "TestUniqueAspect"
#define DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "TestUniqueAspectProperty"
#define DPTEST_TEST_MULTI_ASPECT_CLASS_NAME              "TestMultiAspect"
#define DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "TestMultiAspectProperty"
#define DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define BEGIN_BENTLEY_DPTEST_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace DPTest {
#define END_BENTLEY_DPTEST_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DPTEST using namespace BentleyApi::DPTest;

BEGIN_BENTLEY_DPTEST_NAMESPACE

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
struct TestItemHandler;
#endif
struct TestElementHandler;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestElement : Dgn::PhysicalElement
{
    friend struct TestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT_CLASS_NAME, Dgn::PhysicalElement) 
public:
    TestElement(CreateParams const& params);

protected:
    Utf8String  m_testItemProperty;
    Utf8String  m_testElemProperty;
    int32_t     m_intProps[4];
    double      m_doubleProps[4];
    DPoint3d    m_pointProps[4];

    Dgn::DgnDbStatus _InsertInDb() override;
    Dgn::DgnDbStatus _UpdateInDb() override;
    Dgn::DgnDbStatus _DeleteInDb() const override;
    Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;
    void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    void _CopyFrom(Dgn::DgnElementCR el) override;

public:
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME)); }
    static ECN::ECClassCP GetTestElementECClass(Dgn::DgnDbR db) { return db.Schemas().GetECClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME); }
    
    // This Create function does not put any geometry on the new element. The caller is expected to add a TestItem.
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Utf8CP elementCode="");
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnCode const& elementCode);

    // This Create function sets the element's geometry to a shape
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Utf8CP elementCode, double shapeSize);

    // Create element with display params 
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::Render::GeometryParamsCR ep, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnCode elementCode, double shapeSize);
    static RefCountedPtr<TestElement> CreateWithoutGeometry(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);

    // Change the shape size
    void ChangeElement(double shapeSize);

    // Set get property value
    Utf8StringCR GetTestElementProperty() const { return m_testElemProperty; }
    void SetTestElementProperty(Utf8StringCR value) { m_testElemProperty = value; }

    int32_t GetIntegerProperty(uint8_t which) const { if (4 <= which) { BeAssert(false); return 0; } return m_intProps[which]; }
    double GetDoubleProperty(uint8_t which) const { if (4 <= which) { BeAssert(false); return 0.0; } return m_doubleProps[which]; }
    DPoint3d GetPointProperty(uint8_t which) const { if (4 <= which) { BeAssert(false); return DPoint3d::FromZero(); } return m_pointProps[which]; }

    void SetIntegerProperty(uint8_t which, int32_t i) { if (4 > which) m_intProps[which] = i; }
    void SetDoubleProperty(uint8_t which, double d) { if (4 > which) m_doubleProps[which] = d; }
    void SetPointProperty(uint8_t which, DPoint3dCR p) { if (4 > which) m_pointProps[which] = p; }

    void SetAutoHandledProperty(int32_t i) { SetPropertyValue("IntegerProperty1", i); }
};

typedef RefCountedPtr<TestElement> TestElementPtr;
typedef RefCountedCPtr<TestElement> TestElementCPtr;
typedef TestElement& TestElementR;
typedef TestElement const& TestElementCR;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestElementHandler : Dgn::dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, Dgn::dgn_ElementHandler::Physical, )
    void _RegisterPropertyAccessors(Dgn::ECSqlClassInfo&, ECN::ClassLayoutCR) override;
};

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
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENT_CLASS_NAME)); }
        static PerfElementPtr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);
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
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME)); }
        static PerfElementSub1Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);
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
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME)); }
        static PerfElementSub2Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);
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
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME)); }
        static PerfElementSub3Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId);
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
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, # CN)); }    \
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
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement2d : Dgn::AnnotationElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT2d_CLASS_NAME, Dgn::AnnotationElement2d) 

public:
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT2d_CLASS_NAME)); }
    TestElement2d(CreateParams const& params) : T_Super(params) {}
    static RefCountedPtr<TestElement2d> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Dgn::DgnCode elementCode, double length);
};

typedef RefCountedPtr<TestElement2d> TestElement2dPtr;
typedef RefCountedCPtr<TestElement2d> TestElement2dCPtr;
typedef TestElement2d& TestElement2dR;
typedef TestElement2d const& TestElement2dCR;

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElement2dHandler : Dgn::dgn_ElementHandler::Annotation2d
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT2d_CLASS_NAME, TestElement2d, TestElement2dHandler, Dgn::dgn_ElementHandler::Annotation2d, )
};

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    11/15
//=======================================================================================
struct TestGroup : Dgn::PhysicalElement, Dgn::IElementGroupOf<Dgn::PhysicalElement>
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_TEST_GROUP_CLASS_NAME, Dgn::PhysicalElement)
    friend struct TestGroupHandler;

protected:
    Dgn::IElementGroupCP _ToIElementGroup() const override {return this;}
    Dgn::DgnElementCP _ToGroupElement() const override {return this;}

    explicit TestGroup(CreateParams const& params) : T_Super(params) {}

public:
    static RefCountedPtr<TestGroup> Create(Dgn::DgnDbR, Dgn::DgnModelId, Dgn::DgnCategoryId);
};

typedef RefCountedPtr<TestGroup> TestGroupPtr;
typedef RefCountedCPtr<TestGroup> TestGroupCPtr;
typedef TestGroup& TestGroupR;
typedef TestGroup const& TestGroupCR;

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    11/15
//=======================================================================================
struct TestGroupHandler : Dgn::dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_TEST_GROUP_CLASS_NAME, TestGroup, TestGroupHandler, Dgn::dgn_ElementHandler::Physical, )
};

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    11/16
//=======================================================================================
struct TestSpatialLocation : Dgn::SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_CLASS_TestSpatialLocation, Dgn::SpatialLocationElement)
    friend struct TestSpatialLocationHandler;

protected:
    explicit TestSpatialLocation(CreateParams const& params) : T_Super(params) {}

public:
    static RefCountedPtr<TestSpatialLocation> Create(Dgn::SpatialModelR, Dgn::DgnCategoryId);
};

typedef RefCountedPtr<TestSpatialLocation> TestSpatialLocationPtr;
typedef RefCountedCPtr<TestSpatialLocation> TestSpatialLocationCPtr;

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    11/16
//=======================================================================================
struct TestSpatialLocationHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_CLASS_TestSpatialLocation, TestSpatialLocation, TestSpatialLocationHandler, Dgn::dgn_ElementHandler::SpatialLocation, )
};

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    08/16
//=======================================================================================
struct TestPhysicalType : Dgn::PhysicalType
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_CLASS_TestPhysicalType, Dgn::PhysicalType)
    friend struct TestPhysicalTypeHandler;

protected:
    explicit TestPhysicalType(CreateParams const& params) : T_Super(params) {}

public:
    static RefCountedPtr<TestPhysicalType> Create(Dgn::DgnDbR);

    Utf8String GetStringProperty() const {return GetPropertyValueString("StringProperty");}
    void SetStringProperty(Utf8CP s) {SetPropertyValue("StringProperty", s);}

    int32_t GetIntProperty() const {return GetPropertyValueInt32("IntProperty");}
    void SetIntProperty(int32_t i) {SetPropertyValue("IntProperty", i);}
};

typedef RefCountedPtr<TestPhysicalType> TestPhysicalTypePtr;
typedef RefCountedCPtr<TestPhysicalType> TestPhysicalTypeCPtr;

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    08/16
//=======================================================================================
struct TestPhysicalTypeHandler : Dgn::dgn_ElementHandler::PhysicalType
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_CLASS_TestPhysicalType, TestPhysicalType, TestPhysicalTypeHandler, Dgn::dgn_ElementHandler::PhysicalType, )
};

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    08/16
//=======================================================================================
struct TestGraphicalType2d : Dgn::GraphicalType2d
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_CLASS_TestGraphicalType2d, Dgn::GraphicalType2d)
    friend struct TestGraphicalType2dHandler;

protected:
    explicit TestGraphicalType2d(CreateParams const& params) : T_Super(params) {}

public:
    static RefCountedPtr<TestGraphicalType2d> Create(Dgn::DgnDbR);

    Utf8String GetStringProperty() const {return GetPropertyValueString("StringProperty");}
    void SetStringProperty(Utf8CP s) {SetPropertyValue("StringProperty", s);}

    int32_t GetIntProperty() const {return GetPropertyValueInt32("IntProperty");}
    void SetIntProperty(int32_t i) {SetPropertyValue("IntProperty", i);}
};

typedef RefCountedPtr<TestGraphicalType2d> TestGraphicalType2dPtr;
typedef RefCountedCPtr<TestGraphicalType2d> TestGraphicalType2dCPtr;

//=======================================================================================
// @bsiclass                                                     Shaun.Sewall    08/16
//=======================================================================================
struct TestGraphicalType2dHandler : Dgn::dgn_ElementHandler::GraphicalType2d
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_CLASS_TestGraphicalType2d, TestGraphicalType2d, TestGraphicalType2dHandler, Dgn::dgn_ElementHandler::GraphicalType2d, )
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestUniqueAspect : Dgn::DgnElement::UniqueAspect
{
    DGNASPECT_DECLARE_MEMBERS(DPTEST_SCHEMA_NAME, DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME, Dgn::DgnElement::UniqueAspect);
private:
    friend struct TestUniqueAspectHandler;

    Utf8String m_testUniqueAspectProperty;

    explicit TestUniqueAspect(Utf8CP prop) : m_testUniqueAspectProperty(prop) {;}

    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;

public:
    static RefCountedPtr<TestUniqueAspect> Create(Utf8CP prop) {return new TestUniqueAspect(prop);}

    static ECN::ECClassCP GetECClass(Dgn::DgnDbR db) {return db.Schemas().GetECClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME);}

    Utf8StringCR GetTestUniqueAspectProperty() const {return m_testUniqueAspectProperty;}
    void SetTestUniqueAspectProperty(Utf8CP s) {m_testUniqueAspectProperty = s;}
};

typedef RefCountedPtr<TestUniqueAspect> TestUniqueAspectPtr;
typedef RefCountedCPtr<TestUniqueAspect> TestUniqueAspectCPtr;
typedef TestUniqueAspect& TestUniqueAspectR;
typedef TestUniqueAspect const& TestUniqueAspectCR;
typedef TestUniqueAspect const* TestUniqueAspectCP;
typedef TestUniqueAspect* TestUniqueAspectP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestUniqueAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME, TestUniqueAspectHandler, Dgn::dgn_AspectHandler::Aspect, )
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override {return new TestUniqueAspect("");}
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspect : Dgn::DgnElement::MultiAspect
{
    DGNASPECT_DECLARE_MEMBERS(DPTEST_SCHEMA_NAME, DPTEST_TEST_MULTI_ASPECT_CLASS_NAME, Dgn::DgnElement::MultiAspect);
private:
    friend struct TestMultiAspectHandler;

    Utf8String m_testMultiAspectProperty;

    explicit TestMultiAspect(Utf8CP prop) : m_testMultiAspectProperty(prop) {;}

    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;

public:
    static RefCountedPtr<TestMultiAspect> Create(Utf8CP prop) {return new TestMultiAspect(prop);}

    static ECN::ECClassCP GetECClass(Dgn::DgnDbR db) {return db.Schemas().GetECClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_MULTI_ASPECT_CLASS_NAME);}

    Utf8StringCR GetTestMultiAspectProperty() const {return m_testMultiAspectProperty;}
    void SetTestMultiAspectProperty(Utf8CP s) {m_testMultiAspectProperty = s;}
};

typedef RefCountedPtr<TestMultiAspect> TestMultiAspectPtr;
typedef RefCountedCPtr<TestMultiAspect> TestMultiAspectCPtr;
typedef TestMultiAspect& TestMultiAspectR;
typedef TestMultiAspect const& TestMultiAspectCR;
typedef TestMultiAspect const* TestMultiAspectCP;
typedef TestMultiAspect* TestMultiAspectP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_MULTI_ASPECT_CLASS_NAME, TestMultiAspectHandler, Dgn::dgn_AspectHandler::Aspect, )
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override {return new TestMultiAspect("");}
};

//=======================================================================================
//! A test IDgnElementDependencyHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementDrivesElementHandler : Dgn::DgnElementDependencyHandler
    {
    struct Callback
    {
        virtual void _OnRootChanged(Dgn::DgnDbR db, ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) = 0;
        virtual void _ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) = 0;
    };
private:
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME, TestElementDrivesElementHandler, Dgn::DgnDomain::Handler, )
    static bool s_shouldFail;
    bvector<EC::ECInstanceId> m_relIds;
    bvector<Dgn::dgn_TxnTable::ElementDep::DepRelData> m_deletedRels;
    static Callback* s_callback;

    void _OnRootChanged(Dgn::DgnDbR db, ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;
    void _ProcessDeletedDependency(Dgn::DgnDbR db, Dgn::dgn_TxnTable::ElementDep::DepRelData const& relData) override;

public:
    void Clear() {m_relIds.clear(); m_deletedRels.clear();}
    static void SetCallback(Callback* cb) { s_callback = cb; }

    static void SetShouldFail(bool b) {s_shouldFail = b;}

    static void UpdateProperty1(Dgn::DgnDbR, EC::ECInstanceKeyCR);
    static void SetProperty1(Dgn::DgnDbR, Utf8CP, EC::ECInstanceKeyCR);
    static Utf8String GetProperty1(Dgn::DgnDbR, EC::ECInstanceId);

    static ECN::ECClassCR GetECClass(Dgn::DgnDbR db) {return *db.Schemas().GetECClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME);}

    static ECInstanceKey Insert(Dgn::DgnDbR db, Dgn::DgnElementId root, Dgn::DgnElementId dependent);
    };

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct DgnPlatformTestDomain : Dgn::DgnDomain
    {
private:
    DOMAIN_DECLARE_MEMBERS(DgnPlatformTestDomain, )
    DgnPlatformTestDomain();

public:
    static Dgn::DgnDbStatus Register();
    static Dgn::DgnDbStatus ImportSchema(Dgn::DgnDbR);
    static Dgn::DgnDbStatus ImportDummySchema(Dgn::DgnDbR);
    };

END_BENTLEY_DPTEST_NAMESPACE
