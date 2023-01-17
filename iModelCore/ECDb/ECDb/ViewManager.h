/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include <functional>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct ViewManager;
//======================================================================================
// @bsiclass
//======================================================================================

struct ViewDef final  {
    struct Issues final {
        private:
            EC::IssueDataSource const& m_issues;
            bool m_logIssues;
        public:
            Issues(EC::IssueDataSource const& issues, bool logIssues)
                :m_issues(issues),m_logIssues(logIssues){}
            template<typename ...FmtArgs>
            void Log(IssueSeverity severity, Utf8CP message, FmtArgs&& ...fmtArgs)  const{
                const auto category = IssueCategory::BusinessProperties;
                const auto type = IssueType::ECDbIssue;
                if (m_logIssues) {
                    Utf8String formattedMessage;
                    formattedMessage.Sprintf(message, std::forward<FmtArgs>(fmtArgs)...);
                    m_issues.Report(severity, category, type, formattedMessage.c_str());
                }
            }
            template<typename ...FmtArgs>
            void Error(Utf8CP message, FmtArgs&& ...fmtArgs) const{
                Log(IssueSeverity::Error, message, std::forward<FmtArgs>(fmtArgs)...);
            }
            template<typename ...FmtArgs>
            void Warn(Utf8CP message, FmtArgs&& ...fmtArgs) const{
                Log(IssueSeverity::Warning, message, std::forward<FmtArgs>(fmtArgs)...);
            }
    };

    using SharedPtr = std::shared_ptr<ViewDef>;
    using WeakPtr = std::weak_ptr<ViewDef>;

    /* View persistence type */
    enum class PersistenceMethod {
        None = 0,
        Permanent,
        Temporary,
    };

    /* How view data is refreshed */
    enum class RefreshMethod {
        None = 0,
        Recompute,
    };

    /* Validation method use to validate view*/
    enum class ValidationMethod {
        Full,       // Validate by preparing ECSql
        MetaData    // Validate only custom attribute definition and other meta data.
    };

    /* Validation method use to validate view*/
    enum class ValidationResult {
        NotView,
        Success,
        Warning,
        Error,
    };

    struct ViewProps final {
        uint64_t m_dataVersion;
        DbResult m_errorCode;
        Utf8String m_errorMessage;
    };

    private:
        std::vector<Utf8String> m_propertyMaps;
        PersistenceMethod m_persistenceMethod;
        RefreshMethod m_refreshMethod;
        ECN::ECClassCR m_classDef;
        Utf8String m_query;
        ViewManager& m_mgr;
        mutable std::unique_ptr<ViewProps> m_viewProps;

    private:
        static bool TryGetPropertyMaps(std::vector<Utf8String>&, ECN::IECInstancePtr&);
        static bool TryGetPersistenceMethod(PersistenceMethod&, ECN::IECInstancePtr&);
        static bool TryGetRefreshMethod(RefreshMethod&, ECN::IECInstancePtr&);
        static bool TryGetQuery(Utf8StringR, ECN::IECInstancePtr&);
        static ValidationResult ValidatePropMaps(ECN::ECClassCR, std::vector<Utf8String> const&, Issues const& issues);
        static ValidationResult ValidateFull(ECDbCR, ECN::ECClassCR, Utf8StringCR, PersistenceMethod, RefreshMethod, std::vector<Utf8String> const&, Issues const& issues);
        static ValidationResult ValidateECSqlStatementMap(ECSqlStatement& queryStmt, ClassMapCR targetMap, std::vector<Utf8String> const& propMaps, Issues const& issues);
        static DbResult CreateInsertECSql(Utf8StringR, ECN::ECClassCR, std::vector<Utf8String> const&);
        static DbResult CreateTruncateECSql(Utf8StringR, ECN::ECClassCR);
        static DbResult CreateTransientECSql(Utf8StringR wrapECSql, ECN::ECClassCR classDef, Utf8StringCR userQuery);
        static bool TryGetViewProps (ViewProps& props, ECDbCR ecdb, ViewDef const& viewDef);
        static bool SaveViewProps(ECDbR ecdb, ViewDef const& viewDef, ViewProps const & props);
        DbResult TruncateView();
        ViewProps& GetViewProps() const;
    public:
        ViewDef (
            ViewManager& mgr,
            ECN::ECClassCR classDef,
            Utf8StringCR query,
            PersistenceMethod persistenceMethod,
            RefreshMethod refreshMethod,
            std::vector<Utf8String> const& propertyMap)
                : m_classDef(classDef),
                  m_query(query),
                  m_persistenceMethod(persistenceMethod),
                  m_refreshMethod(refreshMethod),
                  m_propertyMaps(propertyMap),
                  m_mgr(mgr){}
        ~ViewDef(){}
        ViewManager const& GetViewManager() const { return m_mgr; }
        PersistenceMethod GetPersistenceMethod() const { return m_persistenceMethod; }
        RefreshMethod GetRefreshMethod() const { return m_refreshMethod; }
        ECN::ECClassCR GetClass() const {return m_classDef; }
        bool IsRelationshipClass() const { return m_classDef.IsRelationshipClass();}
        bool IsEntityClass() const { return m_classDef.IsEntityClass();}
        bool IsTransient() const { return m_persistenceMethod == PersistenceMethod::None;}
        Utf8StringCR GetQuery() const {return m_query; }
        DbResult RefreshData(bool forceRefresh = false);
        bool NeedRefresh() const;
        const Utf8String CreateInsertECSql() const;
        const Utf8String CreateTruncateECSql() const;
        const Utf8String CreateTransientECSql() const;
        bool CanQuery() const;
        bool SupportRefresh() const;
        static ValidationResult Validate(ECDbCR ecdb, ECN::ECClassCR classDef, ValidationMethod method, bool reportIssues = false);
        static bool HasViewDef(ECN::ECClassCR classDef);
        static bool HasPersistedViewDef(ECN::ECClassCR classDef);
        static bool HasTransientViewDef(ECN::ECClassCR classDef);
        static SharedPtr Create(ViewManager& mgr, ECN::ECClassCR classDef, bool reportIssues);
        static std::set<ECN::ECClassCP> GetClassReferencedInECSql(ECDbCR, Utf8StringCR query);
};

//======================================================================================
// @bsiclass
//======================================================================================
struct RowCopier {
    private:
        ECSqlStatement& m_reader;
        ECSqlStatement& m_writer;
        IECSqlBinder::MakeCopy m_makeCopy = IECSqlBinder::MakeCopy::No;

    private:
        ECSqlStatus BindPrimitive(IECSqlValue const& source, IECSqlBinder& target, ECN::PrimitiveType primitiveType);
        ECSqlStatus BindStruct(IECSqlValue const& source, IECSqlBinder& target);
        ECSqlStatus BindStructArray(IECSqlValue const& source, IECSqlBinder& target);
        ECSqlStatus BindPrimitiveArray(IECSqlValue const& source, IECSqlBinder& target);
        ECSqlStatus BindValue(IECSqlValue const& source, IECSqlBinder& target);
        ECSqlStatus BindValues();

    public:
        RowCopier(ECSqlStatement& reader, ECSqlStatement& writer, IECSqlBinder::MakeCopy makeCopy=IECSqlBinder::MakeCopy::No)
            :m_reader(reader),m_writer(writer), m_makeCopy(makeCopy) {}
        DbResult Update(int64_t& rowCount);
};

//======================================================================================
// @bsiclass
//======================================================================================
struct ViewManager final {
    using ClassList = std::vector<ECN::ECClassCP>;
    enum class WritePolicy {
        Writable,
        Readonly
    };
    private:
        mutable bool m_initialized;
        // m_cachedViewDef can contain nulls for view that failed to load.
        mutable std::map<ECN::ECClassId, ViewDef::SharedPtr> m_cachedViewDef;
        mutable std::map<ECN::ECClassId, WritePolicy> m_writePolicy;

        ECDbR m_ecdb;

    private:
        mutable std::atomic_bool m_loading;
        static ClassList GetViewClasses(ECDbCR ecdb);
        void LoadAndCacheViewDefs(bool forced) const;

    public:
        explicit ViewManager(ECDbR ecdb);
        ECDbCR GetECDb() const { return m_ecdb; }
        ECDbR GetECDbR() { return m_ecdb; }
        void ClearCache() const;
        bool Loading() const { return m_loading.load(); }
        /* For internal use by ecdb to allow write access to view table */
        bool SetWritePolicy(ECN::ECClassCR viewClass, WritePolicy policy) const;
        bool GetWritePolicy(WritePolicy& policy, ECN::ECClassCR viewClass) const;
        ClassList GetViewClasses (bool onlyValidViewClasses) const;
        ViewDef::WeakPtr GetViewDef(ECN::ECClassCR viewClass) const;
        BentleyStatus RefreshViews() const;
        bool HasTransientViews() const;
        bool ValidateViews(ClassList& failedViews, ClassList& validViews, bool reportIssues) const;
        int SubstituteTransientViews(Utf8StringR out, Utf8StringCR originalQuery) const;
};
END_BENTLEY_SQLITE_EC_NAMESPACE