/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECDbSchemaManager.h>
#include "ECDbMap.h"
#include "BeBriefcaseBasedIdSequence.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      09/2015
//+===============+===============+===============+===============+===============+======
struct IssueReporter : NonCopyableClass
    {
private:
    mutable BeMutex m_mutex;
    ECDbCR m_ecdb;
    ECDb::IIssueListener const* m_issueListener;

public:
    explicit IssueReporter(ECDbCR ecdb) : m_ecdb(ecdb), m_issueListener(nullptr) {}
    ~IssueReporter() {}

    BentleyStatus AddListener(ECDb::IIssueListener const& listener);
    void RemoveListener();

    bool IsSeverityEnabled(ECDbIssueSeverity severity) const;

    void Report(ECDbIssueSeverity, Utf8CP message, ...) const;
    void ReportSqliteIssue(ECDbIssueSeverity, DbResult, Utf8CP messageHeader = nullptr) const;

    static NativeLogging::SEVERITY ToLogSeverity(ECDbIssueSeverity sev) { return sev == ECDbIssueSeverity::Warning ? NativeLogging::LOG_WARNING : NativeLogging::LOG_ERROR; }
    };

//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass                                                Krischan.Eberle      12/2014
//+===============+===============+===============+===============+===============+======
struct ECDb::Impl : NonCopyableClass
    {
    friend struct ECDb;
        
private:
    struct DbFunctionKey
        {
        Utf8CP m_functionName;
        int m_argCount;

        DbFunctionKey() : m_functionName(nullptr), m_argCount(-1) {}
        DbFunctionKey(Utf8CP functionName, int argCount) : m_functionName(functionName), m_argCount(argCount) {}
        explicit DbFunctionKey(DbFunction& function) : m_functionName(function.GetName()), m_argCount(function.GetNumArgs()) {}

        struct Comparer
            {
            bool operator() (DbFunctionKey const& lhs, DbFunctionKey const& rhs) const
                {
                int nameCompareResult = BeStringUtilities::Stricmp(lhs.m_functionName, rhs.m_functionName);
                return nameCompareResult < 0 || nameCompareResult == 0 && lhs.m_argCount < rhs.m_argCount;
                }
            };
        };

    static Utf8CP const ECINSTANCEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECSCHEMAIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECCLASSIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECPROPERTYIDSEQUENCE_BELOCALKEY;

    static Utf8CP const TABLEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const COLUMNIDSEQUENCE_BELOCALKEY;
    static Utf8CP const INDEXIDSEQUENCE_BELOCALKEY;
    static Utf8CP const CONSTRAINTIDSEQUENCE_BELOCALKEY;
    static Utf8CP const CLASSMAPIDSEQUENCE_BELOCALKEY;
    static Utf8CP const PROPERTYPATHIDSEQUENCE_BELOCALKEY;

    ECDbR m_ecdb;
    std::unique_ptr<ECDbSchemaManager> m_schemaManager;
    std::unique_ptr<ECDbMap> m_ecdbMap;

    mutable BeBriefcaseBasedIdSequence m_ecInstanceIdSequence;
    BeBriefcaseBasedIdSequence m_ecSchemaIdSequence;
    BeBriefcaseBasedIdSequence m_ecClassIdSequence;
    BeBriefcaseBasedIdSequence m_ecPropertyIdSequence;
    BeBriefcaseBasedIdSequence m_tableIdSequence;
    BeBriefcaseBasedIdSequence m_columnIdSequence;
    BeBriefcaseBasedIdSequence m_indexIdSequence;
    BeBriefcaseBasedIdSequence m_constraintIdSequence;
    BeBriefcaseBasedIdSequence m_classmapIdSequence;
    BeBriefcaseBasedIdSequence m_propertypathIdSequence;

    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;

    IssueReporter m_issueReporter;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl (ECDbR ecdb);
    static DbResult Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

    ECDbSchemaManager const& Schemas () const;
    ECN::IECSchemaLocaterR GetSchemaLocater () const;
    ECN::IECClassLocaterR GetClassLocater () const;

    BentleyStatus OnAddFunction(DbFunction&) const;
    void OnRemoveFunction(DbFunction&) const;

    BentleyStatus Purge(ECDb::PurgeMode) const;
    BentleyStatus PurgeFileInfos() const;

    BentleyStatus AddIssueListener(IIssueListener const& issueListener) { return m_issueReporter.AddListener(issueListener); }
    void RemoveIssueListener() { m_issueReporter.RemoveListener(); }

    void ClearECDbCache() const;

    DbResult OnDbOpened () const;
    DbResult OnDbCreated () const;
    DbResult OnBriefcaseIdChanged (BeBriefcaseId newBriefcaseId);
    void OnDbChangedByOtherConnection () const;
    DbResult VerifySchemaVersion (Db::OpenParams const& params) const;

    //other private methods
    std::vector<BeBriefcaseBasedIdSequence const*> GetSequences () const;

        
public:
    ~Impl () {}

    ECDbMap const& GetECDbMap () const;

    DbResult ResetSequences (BeBriefcaseId* repoId = nullptr);
    BeBriefcaseBasedIdSequence& GetECInstanceIdSequence () { return m_ecInstanceIdSequence; }
    BeBriefcaseBasedIdSequence& GetECSchemaIdSequence () {return m_ecSchemaIdSequence; }
    BeBriefcaseBasedIdSequence& GetECClassIdSequence () { return m_ecClassIdSequence; }
    BeBriefcaseBasedIdSequence& GetECPropertyIdSequence () { return m_ecPropertyIdSequence; }
    BeBriefcaseBasedIdSequence& GetTableIdSequence () { return m_tableIdSequence; }
    BeBriefcaseBasedIdSequence& GetColumnIdSequence () { return m_columnIdSequence; }
    BeBriefcaseBasedIdSequence& GetIndexIdSequence () { return m_indexIdSequence; }
    BeBriefcaseBasedIdSequence& GetConstraintIdSequence () { return m_constraintIdSequence; }
    BeBriefcaseBasedIdSequence& GetClassMapIdSequence () { return m_classmapIdSequence; }
    BeBriefcaseBasedIdSequence& GetPropertyMapIdSequence () { return m_propertypathIdSequence; }

    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;

    IssueReporter const& GetIssueReporter() const { return m_issueReporter; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
