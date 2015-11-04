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

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME    "TestSchema"
#define ELEMENT_PERFORMANCE_ELEMENT1_CLASS      "Element1"
#define ELEMENT_PERFORMANCE_ELEMENT2_CLASS      "Element2"
#define ELEMENT_PERFORMANCE_ELEMENT3_CLASS      "Element3"
#define ELEMENT_PERFORMANCE_ELEMENT4_CLASS      "Element4"

struct PerformanceElement;

DEFINE_REF_COUNTED_PTR (PerformanceElement)
//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElement : Dgn::PhysicalElement
    {
    friend struct PerformanceElementHandler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, Dgn::PhysicalElement);

    protected:
        PerformanceElement (CreateParams const& params) : T_Super (params) {}

    public:
        static PerformanceElementPtr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id);
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, PerformanceElement, PerformanceElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

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

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementsCRUDTestFixture : public PerformanceElementTestFixture
    {
    private:
        static const int64_t m_firstInstanceId = INT64_C (11);
        static const int m_initialInstanceCount = 1000000;
        void SetUpPopulatedDb (WCharCP dbName, Utf8CP className);

        DgnModelPtr CreateElements (int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements)
            {
            DgnModelPtr modelPtr;
            CreateElements (numInstances, schemaName, className, elements, modelPtr);
            return modelPtr;
            }

        void CreateElements (int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements, DgnModelPtr& modelPtr);

        //Generate Sql CRUD Statements.
        void GetInsertSql (Utf8CP className, Utf8StringR insertSql, DgnClassId classId);
        void GetSelectSql (Utf8CP className, Utf8StringR selectSql);
        void GetUpdateSql (Utf8CP className, Utf8StringR updateSql);
        void GetDeleteSql (Utf8StringR deleteSql);

        //Generate ECSql CRUD Statements. 
        void GetInsertECSql (Utf8CP className, Utf8StringR insertECSql);
        void GetSelectECSql (Utf8CP className, Utf8StringR selectECSql);
        void GetUpdateECSql (Utf8CP className, Utf8StringR updateECSql);
        void GetDeleteECSql (Utf8CP className, Utf8StringR deleteECSql);

        //Methods to Bind Business Property Values for Sql Statements
        DgnDbStatus BindElement1PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement2PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement3PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement4PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        void BindParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className);
        void BindUpdateParams (BeSQLite::Statement& stmt, Utf8CP className);

        //ECsql Overloads to Bind business Property values
        DgnDbStatus BindElement1PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement2PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement3PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement4PropertyParams (ECSqlStatement& stmt, bool updateParams);
        void BindParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className);
        void BindUpdateParams (ECSqlStatement& stmt, Utf8CP className);

        //Methods to verify Business Property Values returned by Sql Statements. 
        DgnDbStatus GetElement1Params (BeSQLite::Statement& stmt);
        DgnDbStatus GetElement2Params (BeSQLite::Statement& stmt);
        DgnDbStatus GetElement3Params (BeSQLite::Statement& stmt);
        DgnDbStatus GetElement4Params (BeSQLite::Statement& stmt);
        void GetPropertyValues (BeSQLite::Statement& stmt, Utf8CP className);

        //OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
        DgnDbStatus GetElement1Params (ECSqlStatement& stmt);
        DgnDbStatus GetElement2Params (ECSqlStatement& stmt);
        DgnDbStatus GetElement3Params (ECSqlStatement& stmt);
        DgnDbStatus GetElement4Params (ECSqlStatement& stmt);
        void GetPropertyValues (ECSqlStatement& stmt, Utf8CP className);

    protected:

        void ECSqlInsertTime (int numInstances, Utf8CP className);
        void ECSqlSelectTime (int numInstances, Utf8CP className);
        void ECSqlUpdateTime (int numInstances, Utf8CP className);
        void ECSqlDeleteTime (int numInstances, Utf8CP className);

        void SqlInsertTime (int numInstances, Utf8CP className);
        void SqlSelectTime (int numInstances, Utf8CP className);
        void SqlUpdateTime (int numInstances, Utf8CP className);
        void SqlDeleteTime (int numInstances, Utf8CP className);
    };