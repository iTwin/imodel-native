/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitTests/NonPublished/ECDb/ECDbTestProject.h"
#include "PerformanceTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiClass                                       Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECDbMapCATestFixture : public PerformanceTestFixture
    {
    public:
        size_t m_classNamePostFix = 1;
        size_t m_instancesPerClass = 0;
        size_t m_propertiesPerClass = 0;
        double m_InsertTime, m_UpdateTime, m_SelectTime, m_DeleteTime;
        struct ECSqlTestItem
            {
            Utf8String m_insertECSql;
            Utf8String m_selectECSql;
            Utf8String m_updateECSql;
            Utf8String m_deleteECSql;
            };
        bmap<ECN::ECClassCP, ECSqlTestItem> m_sqlTestItems;
        virtual void InitializeTestDb () override {}
        void CreateClassHierarchy (ECSchemaR testSchema, size_t LevelCount, ECClassR baseClass);
        void CreatePrimitiveProperties (ECClassR ecClass, size_t noOfProperties);
        void GenerateCRUDTestStatements (ECSchemaR ecSchema);
        void InsertInstances (ECDbR ecdb);
        void ReadInstances (ECDbR ecdb);
        void UpdateInstances (ECDbR ecdb);
        void DeleteInstances (ECDbR ecdb);
        void GenerateReadUpdateDeleteStatements (ECSchemaR ecSchema);
    };

END_ECDBUNITTESTS_NAMESPACE