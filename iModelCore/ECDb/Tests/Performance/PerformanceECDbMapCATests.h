/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h"
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiClass                                       Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECDbMapCATestFixture : public PerformanceTestFixtureBase
    {
    public:
        size_t m_classNamePostFix = 1;
        size_t m_instancesPerClass = 0;
        size_t m_propertiesPerClass = 0;
        double m_insertTime = 0.0;
        double m_updateTime = 0.0;
        double m_selectTime = 0.0;
        double m_deleteTime = 0.0;

        struct ECSqlTestItem
            {
            Utf8String m_insertECSql;
            Utf8String m_selectECSql;
            Utf8String m_updateECSql;
            Utf8String m_deleteECSql;
            };
        bmap<ECN::ECClassCP, ECSqlTestItem> m_sqlTestItems;
        void CreateClassHierarchy (ECSchemaR testSchema, size_t LevelCount, ECClassR baseClass);
        void CreatePrimitiveProperties (ECClassR ecClass);
        void GenerateECSqlCRUDTestStatements (ECSchemaR ecSchema);
        void GenerateSqlCRUDTestStatements (ECSchemaR ecSchema, ECClassR ecClass, Utf8StringR insertSql, Utf8StringR selectSql, Utf8StringR updateSql, Utf8StringR deleteSql);
        void InsertInstances (ECDbR ecdb);
        void ReadInstances (ECDbR ecdb);
        void UpdateInstances (ECDbR ecdb);
        void DeleteInstances (ECDbR ecdb);
        void GenerateReadUpdateDeleteStatements (ECSchemaR ecSchema);
    };

END_ECDBUNITTESTS_NAMESPACE