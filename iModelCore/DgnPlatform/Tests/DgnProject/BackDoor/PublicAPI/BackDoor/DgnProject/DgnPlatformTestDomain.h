/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/DgnDomain.h>
#include <DgnPlatform/DgnCore/DgnDb.h>
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/DgnModel.h>
#include <DgnPlatform/DgnCore/ElementHandler.h>
#include <DgnPlatform/DgnCore/DgnElementDependency.h>

#define DPTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define DPTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define DPTEST_DUMMY_SCHEMA_NAMEW                       L"DgnPlatformTestDummy"
#define DPTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define DPTEST_TEST_ELEMENT2d_CLASS_NAME                 "TestElement2d"
#define DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define DPTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define DPTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define DPTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"
#define DPTEST_TEST_ITEM_TestItemLength                  "Length"
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

struct TestItemHandler;
struct TestElementHandler;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestElement : Dgn::PhysicalElement
{
    friend struct TestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT_CLASS_NAME, Dgn::PhysicalElement) 

    TestElement(CreateParams const& params) : T_Super(params) {} 

public:
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) {return Dgn::DgnClassId(db.Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));}
    
    // This Create function does not put any geometry on the new element. The caller is expected to add a TestItem.
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Utf8CP elementCode="");

    // This Create function sets the element's geometry to a shape
    static RefCountedPtr<TestElement> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Utf8CP elementCode, double shapeSize);

    // Change the shape size
    void ChangeElement(double shapeSize);
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
};

//=======================================================================================
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement2d : Dgn::DrawingElement
{
    DGNELEMENT_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT2d_CLASS_NAME, Dgn::DrawingElement) 

public:
    TestElement2d(CreateParams const& params) : T_Super(params) {}
    static RefCountedPtr<TestElement2d> Create(Dgn::DgnDbR db, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, Code elementCode, double length);
};

typedef RefCountedPtr<TestElement2d> TestElement2dPtr;
typedef RefCountedCPtr<TestElement2d> TestElement2dCPtr;
typedef TestElement2d& TestElement2dR;
typedef TestElement2d const& TestElement2dCR;

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElement2dHandler : Dgn::dgn_ElementHandler::Drawing
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT2d_CLASS_NAME, TestElement2d, TestElement2dHandler, Dgn::dgn_ElementHandler::Drawing, )
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestItem : Dgn::DgnElement::Item
{
    DEFINE_T_SUPER(Dgn::DgnElement::Item)
private:
    friend struct TestItemHandler;

    double m_length;
    Utf8String m_testItemProperty;

    explicit TestItem(Utf8CP prop) : m_testItemProperty(prop) {;}

    Utf8String _GetECSchemaName() const override {return DPTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() const override {return DPTEST_TEST_ITEM_CLASS_NAME;}
    Dgn::DgnDbStatus _GenerateElementGeometry(Dgn::GeometricElementR el) override;
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;

public:
    static RefCountedPtr<TestItem> Create(Utf8CP prop) {return new TestItem(prop);}

    Utf8StringCR GetTestItemProperty() const {return m_testItemProperty;}
    void SetTestItemProperty(Utf8CP s) {m_testItemProperty = s;}

    double GetLength() const {return m_length;}
    void SetLength(double v) {m_length=v;}
};

typedef RefCountedPtr<TestItem> TestItemPtr;
typedef RefCountedCPtr<TestItem> TestItemCPtr;
typedef TestItem& TestItemR;
typedef TestItem const& TestItemCR;
typedef TestItem const* TestItemCP;
typedef TestItem* TestItemP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestItemHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ITEM_CLASS_NAME, TestItemHandler, Dgn::dgn_AspectHandler::Aspect, )
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override {return new TestItem("");}
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestUniqueAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect)
private:
    friend struct TestUniqueAspectHandler;

    Utf8String m_testUniqueAspectProperty;

    explicit TestUniqueAspect(Utf8CP prop) : m_testUniqueAspectProperty(prop) {;}

    Utf8String _GetECSchemaName() const override {return DPTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() const override {return DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME;}
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;

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
    DEFINE_T_SUPER(Dgn::DgnElement::MultiAspect)
private:
    friend struct TestMultiAspectHandler;

    Utf8String m_testMultiAspectProperty;

    explicit TestMultiAspect(Utf8CP prop) : m_testMultiAspectProperty(prop) {;}

    Utf8String _GetECSchemaName() const override {return DPTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() const override {return DPTEST_TEST_MULTI_ASPECT_CLASS_NAME;}
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override;

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
private:
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME, TestElementDrivesElementHandler, Dgn::DgnDomain::Handler, )
    static bool s_shouldFail;
    bvector<EC::ECInstanceId> m_relIds;

    void _OnRootChanged(Dgn::DgnDbR db, ECInstanceId relationshipId, Dgn::DgnElementId source, Dgn::DgnElementId target) override;

public:
    void Clear() {m_relIds.clear();}
    static void SetShouldFail(bool b) {s_shouldFail = b;}

    static void UpdateProperty1(Dgn::DgnDbR, EC::ECInstanceKeyCR);

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