/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <json/json.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECObjects/ECObjectsAPI.h>

#include <UnitTests/BackDoor/ECDb/ECDbTests.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

typedef bmap<ECInstanceId, ECN::IECInstancePtr> ECInstanceMap;
typedef const ECInstanceMap& ECInstanceMapCR;

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECDbTestSchemaManager : NonCopyableClass
    {
private:
    BeSQLite::EC::ECDb& m_ecdb;
    Utf8String m_testSchemaName;
    ECN::ECSchemaPtr m_testSchema;

    Utf8StringCR GetTestSchemaName () const;
    BeSQLite::EC::ECDbSchemaManagerCR Schemas () const;
    BeSQLite::EC::ECDbR GetECDb () const;
public:
    explicit ECDbTestSchemaManager (BeSQLite::EC::ECDb& ecdb);
    ~ECDbTestSchemaManager () {}

    ECN::ECClassCP GetClass (Utf8CP className) const;
    ECN::ECClassCP GetClass (Utf8CP schemaName, Utf8CP className) const;

    ECN::ECSchemaPtr GetTestSchema () const;

    BentleyStatus ImportTestSchema (WCharCP testSchemaFileName);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct ECDbIssue
    {
private:
    BeSQLite::EC::ECDbIssueSeverity m_severity;
    Utf8String m_issue;

public:
    explicit ECDbIssue(BeSQLite::EC::ECDbIssueSeverity severity = BeSQLite::EC::ECDbIssueSeverity::Error, Utf8CP issue = nullptr) : m_severity(severity), m_issue(issue) {}

    bool IsIssue() const { return !m_issue.empty(); }
    BeSQLite::EC::ECDbIssueSeverity GetSeverity() const { return m_severity; }
    Utf8CP GetMessage() const { return m_issue.c_str(); }
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestProject
{
public:
    struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
        {
    private:
        mutable ECDbIssue m_issue;
        virtual void _OnIssueReported(BeSQLite::EC::ECDbIssueSeverity severity, Utf8CP message) const override;

    public:
        ECDbIssueListener() : BeSQLite::EC::ECDb::IIssueListener() {}

        //Can only be called once for a given issue. A second call will report whatever issue has occurred (or not) since the first call
        ECDbIssue GetIssue() const;
        };

typedef void (*PopulatePrimitiveValueCallback)(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecproperty);
private:
    BeSQLite::EC::ECDb*    m_ecdb;
    ECDbIssueListener      m_issueListener;
    WCharCP                m_testSchemaFileName;
    ECDbTestSchemaManager  m_testSchemaManager;
    ECInstanceMap          m_ecInstances;
    std::map<ECN::ECClassCP, std::unique_ptr<ECInstanceInserter>> m_inserterCache;

    static bool            s_isInitialized;

    void                   CreateEmpty (Utf8CP ecdbFileName);
    BentleyStatus          ImportECSchema(WCharCP testSchemaXmlFileName);
    void                   ImportArbitraryECInstances (int numberOfECInstances);
    void                   ImportECInstance (ECN::IECInstancePtr ecInstance);
    static void            PopulateStructValue (ECN::ECValueR value, ECN::ECClassCR structType, PopulatePrimitiveValueCallback callback);
    static void            PopulatePrimitiveValue (ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);
    static void            GenerateRandomValue (ECN::ECValueR value, ECN::PrimitiveType type, ECN::ECPropertyCP ecproperty = nullptr);

public:
                           ECDbTestProject ();
                           ~ECDbTestProject ();
    
    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when constructing an ECDbTestProject, too. Tests that don't use
    //! the ECDbTestProject can call this method statically.
    static void            Initialize ();
    static bool            IsInitialized ();
    Utf8CP                 GetECDbPath () const;
    static Utf8String      BuildECDbPath (Utf8CP ecdbFileName);

    //! Creates an empty DgnDb file without importing a test schema and without inserting any instances.
    //! @return ECDb pointing to the creating DgnDb file
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName);
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, int numberOfArbitraryECInstancesToImport);
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, bool importArbitraryECInstances);
    DbResult               Open (Utf8CP ecdbFilePath, BeSQLite::EC::ECDb::OpenParams openParams = BeSQLite::EC::ECDb::OpenParams (BeSQLite::EC::ECDb::OpenMode::Readonly));
    ECDbTestSchemaManager const& GetTestSchemaManager () const;
    BeSQLite::EC::ECDbR    GetECDb ();
    BeSQLite::EC::ECDbCR   GetECDbCR () const;
    //Can only be called once for a given issue. A second call will report whatever issue has occurred (or not) since the first call
    ECDbIssue GetLastIssue() const { return m_issueListener.GetIssue(); }
    ECInstanceMapCR        GetImportedECInstances() const   {return m_ecInstances;}
    BentleyStatus          GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP className);
    static ECN::IECInstancePtr  CreateArbitraryECInstance(ECN::ECClassCR ecClass, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false);
    static void                 PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false);
    static ECN::IECInstancePtr  CreateECInstance (ECN::ECClassCR ecClass);
    static void            AssignRandomValueToECInstance (ECN::ECValueP createdValue, ECN::IECInstancePtr instance, Utf8CP propertyName);
    BentleyStatus InsertECInstance (BeSQLite::EC::ECInstanceKey& ecInstanceKey, ECN::IECInstancePtr ecInstance);
    static ECN::ECObjectsStatus CopyStruct (ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, Utf8CP baseAccessPath);
    static ECN::ECObjectsStatus CopyStruct (ECN::IECInstanceR target, ECN::IECInstanceCR structValue, Utf8CP propertyName);

    static void PopulatePrimitiveValueWithRandomValues (ECN::ECValueR ecValue, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestUtility
{
private:
    static size_t   GetIterableCount (ECN::ECCustomAttributeInstanceIterable const& iterable);
    static size_t   GetIterableCount (ECN::ECPropertyIterable const& iterable);
    static void     GetClassUsageStatistics
        (
        size_t& instanceCount,
        size_t& propertyCount,
        size_t& nullPropertyCount,
        size_t& customAttributeInstanceCount,
        ECN::ECClassCR ecClass,
        BeSQLite::EC::ECDbR ecdb
        );

    static bool     IsECValueNullOrEmpty (ECN::ECValueCR value);
    static bool     CompareJsonWithECValue (const Json::Value& jsonValue, ECN::ECValueCR referenceValue, ECN::IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString);
    static bool     CompareJsonWithECPrimitiveValue (const Json::Value& jsonValue, ECN::ECValueCR referenceValue);
    static bool     CompareJsonWithECArrayValue (const Json::Value& jsonValue, ECN::ECValueCR referenceValue, ECN::IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString);
    static bool     CompareJsonWithECStructValue (const Json::Value& jsonValue, ECN::ECValueCR referenceValue);

    static int64_t  JulianDayToCommonEraTicks (double jd);
public:
    static DbResult CreateECDb (ECDbR ecdb, BeFileNameP ecdbFullPath, WCharCP ecdbFileName);
    static ECN::ECSchemaPtr ReadECSchemaFromDisk (WCharCP ecSchemaFileName, WCharCP ecSchemaSearchPath = nullptr);
    static void     ReadECSchemaFromDisk (ECN::ECSchemaPtr& ecSchema, ECN::ECSchemaReadContextPtr& ecSchemaContext, WCharCP ecSchemaFileName, WCharCP ecSchemaSearchPath = nullptr);
    static BentleyStatus ReadECSchemaFromString (ECN::ECSchemaReadContextPtr& schemaContext, Utf8CP ecschemaXmlString);
    static ECN::ECSchemaCachePtr ReadECSchemaFromString (Utf8CP ecschemaXmlString);

    // If file name is not supplied, constructs one from the schema name
    static void     WriteECSchemaToDisk (ECN::ECSchemaCR ecSchema, WCharCP filenameNoVerExt = nullptr);
    static bool     CompareECInstances (ECN::IECInstanceCR a, ECN::IECInstanceCR b);
    static void     DumpECSchemaUsageStatistics (ECN::ECSchemaCR schema, BeSQLite::EC::ECDbR ecdb, bool dumpEmptyClasses);
    static int64_t  ReadCellValueAsInt64 (BeSQLite::DbR db, Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause);
    static bool     CompareJsonWithECInstance (const Json::Value& json, ECN::IECInstanceCR referenceInstance);
    static void     DebugDumpJson (const Json::Value& jsonValue);
    static bool     IsECValueNull (ECN::ECValueCR value);
    
    static bool     CompareECDateTimes (int64_t expectedECTicks, int64_t actualECTicks);
    static void     AssertECDateTime (ECN::ECValueCR expectedECValue, const Db& db, double actualJd);
    static void     AssertECDateTime (int64_t expectedCETicks, int64_t actualCETicks, Utf8CP assertMessageHeader);

    static BentleyStatus SetECInstanceId (ECN::IECInstanceR instance, ECInstanceId const& instanceId);
};

// *** WIP_STUB_FOR_MISSING_CLASS_PerformanceResultRecorder
//struct PerformanceResultRecorder
//{
//static void LogToDb(Utf8String,Utf8String,double,Utf8String,int optional=0) {;}
//};

END_ECDBUNITTESTS_NAMESPACE