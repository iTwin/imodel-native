/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "PerformanceTestFixture.h"
#include "../TestFixture/DgnDbTestFixtures.h"

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME    "TestSchema"
#define ELEMENT_PERFORMANCE_ELEMENT1_CLASS      "Element1"
#define ELEMENT_PERFORMANCE_ELEMENT2_CLASS      "Element2"
#define ELEMENT_PERFORMANCE_ELEMENT3_CLASS      "Element3"
#define ELEMENT_PERFORMANCE_ELEMENT4_CLASS      "Element4"
//#define ELEMENT_PERFORMANCE_ELEMENT4b_CLASS     "Element4b"
//#define ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS "SimpleElement"

struct PerformanceElement1;
struct PerformanceElement2;
struct PerformanceElement3;
struct PerformanceElement4;
//struct PerformanceSqlElement4b;

DEFINE_REF_COUNTED_PTR (PerformanceElement1)
DEFINE_REF_COUNTED_PTR (PerformanceElement2)
DEFINE_REF_COUNTED_PTR (PerformanceElement3)
DEFINE_REF_COUNTED_PTR (PerformanceElement4)
//DEFINE_REF_COUNTED_PTR (PerformanceSqlElement4b)

//WIP separate Handler for every Class to Mimic PerformanceElementsTests
//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement1 : Dgn::PhysicalElement
    {
    friend struct PerformanceElement1Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, Dgn::PhysicalElement);

    protected:
        PerformanceElement1 (CreateParams const& params): T_Super(params) {}

    public:
        static PerformanceElement1Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement2 : PerformanceElement1
    {
    friend struct PerformanceElement2Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement1);

    protected:

        PerformanceElement2 (CreateParams const& params) : T_Super (params) {}

    public:
        static PerformanceElement2Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement3 : PerformanceElement2
    {
    friend struct PerformanceElement3Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement2);

    protected:
        PerformanceElement3 (CreateParams const& params) : T_Super (params) {}

    public:
        static PerformanceElement3Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement4 : PerformanceElement3
    {
    friend struct PerformanceElement4Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement3);

    protected:
        PerformanceElement4 (CreateParams const& params) : T_Super (params) {}

    public:
        static PerformanceElement4Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
    };

//struct PerformanceElement4b : PerformanceElement3
//    {
//    friend struct PerformanceElement4bHandler;
//    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, PerformanceElement3);
//
//    protected:
//        PerformanceElement4b (CreateParams const& params) : T_Super (params) {}
//
//    public:
//        static PerformanceElement4bPtr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
//    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement1Handler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, PerformanceElement1, PerformanceElement1Handler, Dgn::dgn_ElementHandler::Physical, )
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement2Handler : PerformanceElement1Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement2, PerformanceElement2Handler, PerformanceElement1Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement3Handler : PerformanceElement2Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement3, PerformanceElement3Handler, PerformanceElement2Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement4Handler : PerformanceElement3Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement4, PerformanceElement4Handler, PerformanceElement3Handler, )
    };

//struct PerformanceElement4bHandler : PerformanceElement3Handler
//    {
//    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, PerformanceElement4b, PerformanceElement4bHandler, PerformanceElement3Handler, )
//    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (PerformanceElementTestDomain, )
    public:
        PerformanceElementTestDomain ();
        static void RegisterDomainAndImportSchema (DgnDbR db, ECN::ECSchemaR schema);
    };