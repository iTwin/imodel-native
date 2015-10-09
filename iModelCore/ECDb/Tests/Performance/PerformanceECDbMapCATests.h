/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiClass                                       Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECDbMapCATestFixture : public ECDbTestFixture
    {
    public:
        size_t m_classNamePostFix = 1;
        size_t m_instancesPerClass = 0;
        size_t m_propertiesPerClass = 0;
        double m_insertTime = 0.0;
        double m_updateTime = 0.0;
        double m_selectTime = 0.0;
        double m_deleteTime = 0.0;

        Utf8String m_insertSql;
        Utf8String m_updateSql;
        Utf8String m_selectSql;
        Utf8String m_deleteSql;

        struct ECSqlTestItem
            {
            Utf8String m_insertECSql;
            Utf8String m_selectECSql;
            Utf8String m_updateECSql;
            Utf8String m_deleteECSql;
            };
        bmap<ECN::ECClassCP, ECSqlTestItem> m_ecsqlTestItems;
        void CreateClassHierarchy (ECN::ECSchemaR testSchema, size_t LevelCount, ECN::ECClassR baseClass);
        void CreatePrimitiveProperties (ECN::ECClassR ecClass);

        void GenerateECSqlCRUDTestStatements (ECN::ECSchemaR ecSchema, bool hardCodePropertyValues);
        void ECSqlInsertInstances (ECDbR ecdb, bool bindPropertyValues);
        void ECSqlReadInstances (ECDbR ecdb, bool iterateResultSet);
        void ECSqlUpdateInstances (ECDbR ecdb, bool bindPropertyValues);
        void ECSqlDeleteInstances (ECDbR ecdb);

        void GenerateSqlCRUDTestStatements (ECN::ECSchemaR ecSchema, ECN::ECClassR ecClass, bool hardCodePropertyValues);
        void SqlInsertInstances (ECDbR ecdb, ECClassR ecClass, bool bindPropertyValues);
        void SqlReadInstances (ECDbR ecdb, ECClassR ecClass, bool iterateResultSet);
        void SqlUpdateInstances (ECDbR ecdb, ECClassR ecClass, bool bindPropertyValues);
        void SqlDeleteInstances (ECDbR ecdb, ECClassR ecClass);
    };

END_ECDBUNITTESTS_NAMESPACE