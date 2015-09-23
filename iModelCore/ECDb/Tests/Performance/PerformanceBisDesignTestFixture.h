/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceBisDesignTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>

#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================  
// @bsiclass                                                 Krischan.Eberle     05/2014
//=======================================================================================    
struct PerformanceBisDesignTestFixture : public ::testing::Test
    {
protected:
    enum class Scenario
        {
        TablePerClass,
        MasterTable,
        MasterTableAndDomainTables
        };

    struct Context
        {
    private:
        int m_domainPropertyCount;
        int m_instancesPerClassCount;

    public:
        Context (int domainPropertyCount, int instancesPerClassCount)
            : m_domainPropertyCount (domainPropertyCount), m_instancesPerClassCount (instancesPerClassCount)
            {}

        int GetDomainPropertyCount () const { return m_domainPropertyCount; }
        int GetInstancesPerClassCount () const { return m_instancesPerClassCount; }
        };

    static Utf8CP const BASE_CLASS_NAME;
    static Utf8CP const ECINSTANCEID_COLUMN_NAME;
    static Utf8CP const CLASSID_COLUMN_NAME;
    static Utf8CP const PARENTECINSTANCEID_COLUMN_NAME;
    static Utf8CP const BUSINESSKEY_COLUMN_NAME;
    static Utf8CP const GLOBALID_COLUMN_NAME;

private:
    mutable uint64_t m_ecInstanceIdSequence;
    bmap<ECN::ECPropertyCP, int> m_parameterMapping;

    virtual void _RunInsertTest(BeSQLite::DbR db, Context const& context) const = 0;
    virtual void _RunSelectTest(BeSQLite::DbR db, Context const& context) const = 0;
    virtual void _GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema) = 0;
    virtual void _GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECN::ECSchemaCR testSchema) const = 0;

    virtual Scenario _GetScenario () const = 0;
    virtual Utf8String _ToString () const = 0;

    static void RetrieveValues (BeSQLite::Statement& stmt, bmap<int, DbValueType> const& columnTypeMap, int firstColumnIndex, int lastColumnIndex);

    static ECN::ECSchemaPtr CreateTestSchema (int domainPropCount);
    static ECN::ECClassP AddTestBaseClass (ECN::ECSchemaR schema, ECN::ECClassId ecClassId);
    static ECN::ECClassP AddTestClassStub (ECN::ECSchemaR schema, ECN::ECClassCR baseClass, Utf8CP name, ECN::ECClassId ecClassId);
    static BentleyStatus AddTestClassProperty (ECN::ECClassR testClass, int propertyNumber, ECN::PrimitiveType const* type);

    static void CreateTestDb (BeSQLite::DbR db, Utf8CP fileName);
    void Setup (BeSQLite::DbR db, Context& context, ECN::ECSchemaCR testSchema);
    static void OpenTestDb (BeSQLite::DbR db, Utf8CP fileName, bool readonly = true);

protected:
    PerformanceBisDesignTestFixture ()
        : m_ecInstanceIdSequence (1ULL)
        {}

    void RunTest (Utf8CP dbFileName, Context& context);

    uint64_t GetNextECInstanceId () const;
    void BindToParameter (BeSQLite::Statement&, ECN::ECClassCR testClass, ECN::ECPropertyCR property, uint64_t ecInstanceId) const;

    static void IterateResultSet (BeSQLite::Statement& stmt, std::vector<std::pair<int, int>> const& columnRanges);
    static void IterateResultSet (BeSQLite::Statement& stmt);

    void AddParameterMapping (ECN::ECPropertyCR property, int parameterIndex);

    static void AppendPropToSql (Utf8StringR sql, ECN::ECPropertyCR prop, Utf8CP tableName = nullptr);

    static void AppendPropDdlToSql (Utf8StringR sql, ECN::ECPropertyCR prop, Utf8CP tableName = nullptr);
    static void AppendIndexSql (Utf8StringR sql, Utf8CP tableName, Utf8CP columnName, bool isUnique);
    static Utf8String GetTableName (ECN::ECClassCR testClass);

    Scenario GetScenario () const;
    Utf8String ToString () const;

public:
    virtual ~PerformanceBisDesignTestFixture () {}

    };


//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     05/2014
//=======================================================================================    
struct Performance_BisDesign_TablePerClassScenario_TestFixture : PerformanceBisDesignTestFixture
    {
private:
    struct SqlTestItem
        {
        Utf8String m_insertSql;
        Utf8String m_selectSql;
        };

    typedef bmap<ECN::ECClassCP, SqlTestItem> SqlTestItems;
    SqlTestItems m_sqlTestItems;

    virtual void _RunInsertTest(BeSQLite::DbR db, Context const& context) const override;
    virtual void _RunSelectTest(BeSQLite::DbR db, Context const& context) const override;
    virtual void _GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema) override;
    virtual void _GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECN::ECSchemaCR testSchema) const override;

    virtual Scenario _GetScenario () const override { return Scenario::TablePerClass; }
    virtual Utf8String _ToString () const override { return "Table per ECClass"; }

    static bool IsClassIdProperty (ECN::ECPropertyCR prop);

public:
    Performance_BisDesign_TablePerClassScenario_TestFixture ()
        : PerformanceBisDesignTestFixture ()
        {}

    virtual ~Performance_BisDesign_TablePerClassScenario_TestFixture () {}

    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     05/2014
//=======================================================================================    
struct Performance_BisDesign_MasterTableScenario_TestFixture : PerformanceBisDesignTestFixture
    {
private:
    struct SqlTestItem
        {
        std::pair<int, int> m_domainColumnsSelectClauseRangeInGenericSelect;
        Utf8String m_nonGenericSelectSql;
        };

    Utf8String m_genericInsertSql;

    Utf8String m_genericSelectSql;
    std::pair<int, int> m_systemColumnsSelectClauseRangeInGenericSelect;
    bmap<ECN::ECClassCP, SqlTestItem> m_sqlTestItems;

    virtual void _RunInsertTest(BeSQLite::DbR db, Context const& context ) const override;
    virtual void _RunSelectTest(BeSQLite::DbR db, Context const& context ) const override;
    void RunNonGenericSelectTest (BeSQLite::DbR db, Context const& context) const;
    void RunGenericSelectTest (BeSQLite::DbR db, Context const& context) const;

    virtual void _GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema) override;
    virtual void _GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECN::ECSchemaCR testSchema) const override;

    virtual Scenario _GetScenario () const override { return Scenario::MasterTable; }
    virtual Utf8String _ToString () const override { return "Master table"; }

public:
    Performance_BisDesign_MasterTableScenario_TestFixture ()
        : PerformanceBisDesignTestFixture ()
        {}

    virtual ~Performance_BisDesign_MasterTableScenario_TestFixture () {}
    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     05/2014
//=======================================================================================    
struct Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture : PerformanceBisDesignTestFixture
    {
private:
    struct SqlTestItem
        {
        Utf8String m_domainInsertSql;
        Utf8String m_selectSql;
        };

    Utf8String m_masterInsertSql;
    bmap<ECN::ECClassCP, SqlTestItem> m_sqlTestItems;

    virtual void _RunInsertTest(BeSQLite::DbR db, Context const& context ) const override;
    virtual void _RunSelectTest(BeSQLite::DbR db, Context const& context ) const override;
    virtual void _GenerateSqlTestItems (Context const& context, ECN::ECSchemaCR testSchema) override;
    virtual void _GenerateDdlSql (Utf8StringR createTablesSql, Utf8StringR createIndicesSql, ECN::ECSchemaCR testSchema) const override;

    virtual Scenario _GetScenario () const override { return Scenario::MasterTableAndDomainTables; }
    virtual Utf8String _ToString () const override { return "Master table and domain tables"; }

public:
    Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture ()
        : PerformanceBisDesignTestFixture ()
        {}

    virtual ~Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture () {}

    };

END_ECDBUNITTESTS_NAMESPACE
