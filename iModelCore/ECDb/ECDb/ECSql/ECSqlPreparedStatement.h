/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Exp.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlBinder.h"
#include "ECSqlField.h"
#include "DynamicSelectClauseECClass.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct IECSqlPreparedStatement
    {
    protected:
        ECDb const& m_ecdb;
        ECSqlType m_type;
        bool m_isCompoundStatement;
        bool m_isNoopInSqlite = false;

    private:
        Utf8String m_ecsql;
        ECDb::Impl::ClearCacheCounter m_preparationClearCacheCounter;

        //not copyable
        IECSqlPreparedStatement(IECSqlPreparedStatement const&) = delete;
        IECSqlPreparedStatement& operator=(IECSqlPreparedStatement const&) = delete;

        virtual ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) = 0;
        virtual IECSqlBinder& _GetBinder(int parameterIndex) const = 0;
        virtual int _GetParameterIndex(Utf8CP parameterName) const = 0;
        virtual int _TryGetParameterIndex(Utf8CP parameterName) const = 0;
        virtual ECSqlStatus _Reset() = 0;
        virtual ECSqlStatus _ClearBindings() = 0;
        virtual Utf8CP _GetNativeSql() const = 0;

    protected:
        IECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type, bool isCompoundStmt) : m_ecdb(ecdb), m_type(type), m_isCompoundStatement(isCompoundStmt) {}

        BentleyStatus AssertIsValid() const;

    public:
        virtual ~IECSqlPreparedStatement() {}

        ECSqlStatus Prepare(ECSqlPrepareContext&, Exp const&, Utf8CP ecsql);
        IECSqlBinder& GetBinder(int parameterIndex) const;
        int GetParameterIndex(Utf8CP parameterName) const;
        int TryGetParameterIndex(Utf8CP parameterName) const;

        ECSqlStatus ClearBindings();
        ECSqlStatus Reset();

        Utf8CP GetECSql() const { return m_ecsql.c_str(); }
        Utf8CP GetNativeSql() const;
        bool IsNoopInSqlite() const { return m_isNoopInSqlite; }

        ECDb const& GetECDb() const { return m_ecdb; }
        bool IsCompoundStatement() const { return m_isCompoundStatement; }
        ECSqlType GetType() const { return m_type; }
    };

//=======================================================================================
//! IECSqlPreparedStatement for ECSQL that only requires a single SQLite statement to 
//! be executed
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct SingleECSqlPreparedStatement : IECSqlPreparedStatement
    {
private:
    mutable BeSQLite::Statement m_sqliteStatement;
    ECSqlParameterMap m_parameterMap;

    IECSqlBinder& _GetBinder(int parameterIndex) const override;
    int _GetParameterIndex(Utf8CP parameterName) const override;
    int _TryGetParameterIndex(Utf8CP parameterName) const override;
    ECSqlStatus _ClearBindings() override;
    Utf8CP _GetNativeSql() const override { return m_sqliteStatement.GetSql(); }

protected:
    SingleECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : IECSqlPreparedStatement(ecdb, type, false) {}

    ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;
    ECSqlStatus _Reset() override;

public:
    virtual ~SingleECSqlPreparedStatement() {}

    DbResult DoStep();

    ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
    ECSqlParameterMap& GetParameterMapR() { return m_parameterMap; }
    BeSQLite::Statement& GetSqliteStatement() { return m_sqliteStatement; }
    };

//=======================================================================================
//! IECSqlPreparedStatement for ECSQL that only requires a single SQLite statement and
//! refers to a single table
// @bsiclass                                                Krischan.Eberle      04/2017
//+===============+===============+===============+===============+===============+======
struct SingleContextTableECSqlPreparedStatement final : SingleECSqlPreparedStatement
    {
private:
    DbTable const* m_contextTable = nullptr;

public:
    SingleContextTableECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type, DbTable const& contextTable) : SingleECSqlPreparedStatement(ecdb, type), m_contextTable(&contextTable) {}
    ~SingleContextTableECSqlPreparedStatement() {}
    DbTable const& GetContextTable() const { return *m_contextTable; }

    };
//=======================================================================================
//! IECSqlPreparedStatement for ECSQL that requires multiple SQLite statements to be executed
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct CompoundECSqlPreparedStatement : IECSqlPreparedStatement
    {
    public:
        struct IProxyECSqlBinder : IECSqlBinder
            {
        private:
            virtual void _AddBinder(IECSqlBinder&) = 0;

        protected:
            IProxyECSqlBinder() : IECSqlBinder() {}

        public:
            virtual ~IProxyECSqlBinder() {}
            void AddBinder(IECSqlBinder& binder) { _AddBinder(binder); }
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    03/2017
        //+===============+===============+===============+===============+===============+======
        struct ProxyECInstanceIdECSqlBinder final : IProxyECSqlBinder
            {
            private:
                IECSqlBinder* m_idBinder = nullptr;
                bool m_boundValueIsNull = true;

                ECSqlStatus _BindNull() override { m_boundValueIsNull = true; return GetBinder().BindNull(); }
                ECSqlStatus _BindInt64(int64_t value) override { m_boundValueIsNull = false; return GetBinder().BindInt64(value); }
                ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override { m_boundValueIsNull = false; return GetBinder().BindText(value, makeCopy, byteCount); }

                ECSqlStatus _BindBoolean(bool value) override;
                ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
                ECSqlStatus _BindZeroBlob(int blobSize) override;
                ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const& dtInfo) override;
                ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo) override;
                ECSqlStatus _BindDouble(double value) override;
                ECSqlStatus _BindInt(int value) override;
                ECSqlStatus _BindPoint2d(DPoint2dCR value) override;
                ECSqlStatus _BindPoint3d(DPoint3dCR value) override;

                IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
                IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;
                IECSqlBinder& _AddArrayElement() override;

                void _AddBinder(IECSqlBinder& binder) override { BeAssert(m_idBinder == nullptr); m_idBinder = &binder; }
            
            public:
                ProxyECInstanceIdECSqlBinder() : IProxyECSqlBinder() {}

                IECSqlBinder& GetBinder() { BeAssert(m_idBinder != nullptr); return *m_idBinder; }

                void ClearState() { m_boundValueIsNull = true; }
                bool IsBoundValueNull() const { return m_boundValueIsNull; }
            };

    protected:
        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    03/2017
        //+===============+===============+===============+===============+===============+======
        struct ProxyECSqlBinder final : IProxyECSqlBinder
            {
            private:
                std::vector<IECSqlBinder*> m_binders;
                std::map<ECN::ECPropertyId, std::unique_ptr<ProxyECSqlBinder>> m_structMemberProxyBindersById;
                std::map<Utf8String, std::unique_ptr<ProxyECSqlBinder>, CompareIUtf8Ascii> m_structMemberProxyBindersByName;
                std::unique_ptr<ProxyECSqlBinder> m_arrayElementProxyBinder;

                ECSqlStatus _BindNull() override;
                ECSqlStatus _BindBoolean(bool value) override;
                ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
                ECSqlStatus _BindZeroBlob(int blobSize) override;
                ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const& dtInfo) override;
                ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo) override;
                ECSqlStatus _BindDouble(double value) override;
                ECSqlStatus _BindInt(int value) override;
                ECSqlStatus _BindInt64(int64_t value) override;
                ECSqlStatus _BindPoint2d(DPoint2dCR value) override;
                ECSqlStatus _BindPoint3d(DPoint3dCR value) override;
                ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
                IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
                IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;
                IECSqlBinder& _AddArrayElement() override;

                void _AddBinder(IECSqlBinder& binder) override { m_binders.push_back(&binder); }

            public:
                ProxyECSqlBinder() : IProxyECSqlBinder() {}
            };


        std::vector<std::unique_ptr<SingleContextTableECSqlPreparedStatement>> m_statements;
        mutable std::vector<std::unique_ptr<IProxyECSqlBinder>> m_proxyBinders;
        mutable bmap<Utf8String, int, CompareIUtf8Ascii> m_parameterNameMap;

    private:
        mutable Utf8String m_compoundNativeSql;

        IECSqlBinder& _GetBinder(int parameterIndex) const override;
        int _GetParameterIndex(Utf8CP parameterName) const override;
        int _TryGetParameterIndex(Utf8CP parameterName) const override;
        ECSqlStatus _Reset() override;
        ECSqlStatus _ClearBindings() override;
        Utf8CP _GetNativeSql() const override;

    protected:
        CompoundECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : IECSqlPreparedStatement(ecdb, type, true) {}

    public:
        virtual ~CompoundECSqlPreparedStatement() {}

        SingleContextTableECSqlPreparedStatement& GetPrimaryTableECSqlStatement() { BeAssert(!m_statements.empty()); return *m_statements[0]; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSqlSelectPreparedStatement final : SingleECSqlPreparedStatement
    {
    private:
        DynamicSelectClauseECClass m_dynamicSelectClauseECClass;
        std::vector<std::unique_ptr<ECSqlField>> m_fields;
        //Calls to OnAfterStep/Reset on ECSqlFields can be very many, so only call it on fields that require it.
        std::vector<ECSqlField*> m_fieldsRequiringOnAfterStep;
        std::vector<ECSqlField*> m_fieldsRequiringReset;

        ECSqlStatus _Reset() override;

        ECSqlStatus ResetFields() const;
        ECSqlStatus OnAfterStep() const;

    public:
        explicit ECSqlSelectPreparedStatement(ECDb const& ecdb) : SingleECSqlPreparedStatement(ecdb, ECSqlType::Select) {}

        DbResult Step();

        int GetColumnCount() const;
        IECSqlValue const& GetValue(int columnIndex) const;

        void AddField(std::unique_ptr<ECSqlField>);
        DynamicSelectClauseECClass& GetDynamicSelectClauseECClassR() { return m_dynamicSelectClauseECClass; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSqlInsertPreparedStatement final : CompoundECSqlPreparedStatement
    {
private:
    struct PrepareInfo final
        {
        public:
            struct PropNameValueInfo
                {
                public:
                    PropertyNameExp const* m_propNameExp = nullptr;
                    ValueExp const* m_valueExp = nullptr;
                    bool m_isIdPropNameExp = false;

                    PropNameValueInfo(PropertyNameExp const& propNameExp, ValueExp const& valueExp, bool isIdPropNameExp) : m_propNameExp(&propNameExp), m_valueExp(&valueExp), m_isIdPropNameExp(isIdPropNameExp) {}
                };
        private:
            ECSqlPrepareContext& m_ctx;
            InsertStatementExp const& m_exp;
            ClassNameExp const& m_classNameExp;
            PropertyNameListExp const& m_propNameListExp;
            ValueExpListExp const& m_valuesListExp;
            Exp::ECSqlRenderContext m_ecsqlRenderContext;
            bmap<DbTable const*, bvector<PropNameValueInfo>> m_propNameValueInfosByMappedTable;
            //Holds all tables per parameter index
            bmap<uint32_t, bset<DbTable const*>> m_parameterIndexInTables;
            bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> m_perTableStatements;

            //not copyable
            PrepareInfo(PrepareInfo const&) = delete;
            PrepareInfo& operator=(PrepareInfo const&) = delete;

        public:
            PrepareInfo(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
                : m_ctx(ctx), m_exp(exp), m_classNameExp(*exp.GetClassNameExp()), m_propNameListExp(*exp.GetPropertyNameListExp()), m_valuesListExp(*exp.GetValuesExp()),
                m_ecsqlRenderContext(Exp::ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter)
                {}

            ECSqlPrepareContext& GetContext() const { return m_ctx; }
            InsertStatementExp const& GetExp() const { return m_exp; }
            ClassNameExp const& GetClassNameExp() const { return m_classNameExp; }
            PropertyNameListExp const& GetPropertyNameListExp() const { return m_propNameListExp; }
            ValueExpListExp const& GetValuesExp() const { return m_valuesListExp; }

            Exp::ECSqlRenderContext const& GetECSqlRenderContext() const { return m_ecsqlRenderContext; }
            Exp::ECSqlRenderContext& GetECSqlRenderContextR() { return m_ecsqlRenderContext; }

            void AddPropNameValueInfo(PropNameValueInfo const& info, DbTable const& table) { m_propNameValueInfosByMappedTable[&table].push_back(info); }
            bmap<DbTable const*, bvector<PropNameValueInfo>> const& GetPropNameValueInfosByTable() const { return m_propNameValueInfosByMappedTable; }
            bmap<uint32_t, bset<DbTable const*>> const& GetTablesByParameterIndex() const { return m_parameterIndexInTables; }
            bool ParameterIndexExists(uint32_t paramIndex) const { return m_parameterIndexInTables.find(paramIndex) != m_parameterIndexInTables.end(); }
            //!@return true if index already existed, false otherwise
            void AddParameterIndex(uint32_t paramIndex, DbTable const& table) { m_parameterIndexInTables[paramIndex].insert(&table); }

            bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> const& GetLeafStatementsByTable() const { return m_perTableStatements; }
            void AddLeafStatement(SingleContextTableECSqlPreparedStatement const& stmt, DbTable const& table) { m_perTableStatements[&table] = &stmt; }
        };

    struct ECInstanceKeyHelper final
            {
            public:
                enum class Mode
                    {
                    NotUserProvided = 1,
                    UserProvidedNullExp,
                    UserProvidedParameterExp,
                    UserProvidedOtherExp
                    };

                struct UpdateHook final 
                    {
                    private:
                        struct Callback final : DataUpdateCallback
                            {
                            private:
                                ECInstanceId m_rowId;

                                void _OnRowModified(DataUpdateCallback::SqlType sqlType, Utf8CP dbName, Utf8CP tableName, BeInt64Id rowid) override
                                    {
                                    BeAssert(sqlType == DataUpdateCallback::SqlType::Update);
                                    m_rowId = ECInstanceId(rowid);
                                    }

                            public:
                                Callback() : DataUpdateCallback() {}
                                ~Callback() {}

                                ECInstanceId GetRowId() const { return m_rowId; }
                            };

                        ECDb const& m_ecdb;
                        Callback m_callback;

                        //not copyable
                        UpdateHook(UpdateHook const&) = delete;
                        UpdateHook& operator=(UpdateHook const&) = delete;

                    public:
                        explicit UpdateHook(ECDb const& ecdb) : m_ecdb(ecdb) { m_ecdb.AddDataUpdateCallback(m_callback); }
                        ~UpdateHook() { m_ecdb.RemoveDataUpdateCallback(); }
                        ECInstanceId GetUpdatedRowid() const { return m_callback.GetRowId(); }
                    };

            private:
                ECSqlSystemPropertyInfo const* m_sysPropInfo = nullptr;
                Mode m_mode = Mode::NotUserProvided;
                DbTableSpace const* m_tableSpace = nullptr;
                ECN::ECClassId m_classId;
                ProxyECInstanceIdECSqlBinder* m_idProxyBinder = nullptr; //in case user specified parametrized id expression
                mutable ECInstanceId m_generatedECInstanceId;

                //not copyable
                ECInstanceKeyHelper(ECInstanceKeyHelper const&) = delete;
                ECInstanceKeyHelper& operator=(ECInstanceKeyHelper const&) = delete;

                static Mode DetermineMode(int& expIx, ECSqlSystemPropertyInfo const&, PrepareInfo const&);

            public:
                ECInstanceKeyHelper() {}

                void Initialize(int &idPropNameExpIx, PrepareInfo const&);

                void SetUserProvidedParameterBinder(ProxyECInstanceIdECSqlBinder& idBinder) 
                    { 
                    BeAssert(m_mode == Mode::UserProvidedParameterExp && "Must only be used with mode Mode::UserProvidedParameterExp");
                    BeAssert(m_idProxyBinder == nullptr && "Must not be called twice"); 
                    m_idProxyBinder = &idBinder; 
                    }

                ECN::ECClassId GetClassId() const { return m_classId; }
                bool IsEndTableRelationshipInsert() const { return *m_sysPropInfo != ECSqlSystemPropertyInfo::ECInstanceId(); }
                bool MustGenerateECInstanceId() const;
                DbTableSpace const& GetTableSpace() const { BeAssert(m_tableSpace != nullptr); return *m_tableSpace; }
                //Is not null in case user specified parametrized id expression
                ProxyECInstanceIdECSqlBinder* GetIdProxyBinder() const { return m_idProxyBinder; }
                Mode GetMode() const { return m_mode; }
            };

    

        ECInstanceKeyHelper m_ecInstanceKeyHelper;

        ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;
        ECSqlStatus PrepareLeafStatements(PrepareInfo&);
        ECSqlStatus PopulateProxyBinders(PrepareInfo const&);

        DbResult StepForEndTableRelationship(ECInstanceKey&);

    public:
        explicit ECSqlInsertPreparedStatement(ECDb const& ecdb) : CompoundECSqlPreparedStatement(ecdb, ECSqlType::Insert) {}
        DbResult Step(ECInstanceKey&);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparedStatement final : CompoundECSqlPreparedStatement
    {
    private:
        struct PrepareInfo final
            {
        private:
            ECSqlPrepareContext& m_ctx;
            UpdateStatementExp const& m_exp;
            ClassNameExp const& m_classNameExp;
            AssignmentListExp const& m_assignmentListExp;
            WhereExp const* m_whereExp = nullptr;
            OptionsExp const* m_optionsExp = nullptr;
            Exp::ECSqlRenderContext m_ecsqlRenderContext;
            bset<DbTable const*> m_tablesInvolvedInAssignmentClause;
            bmap<DbTable const*, bvector<AssignmentExp const*>> m_assignmentExpsByTable;
            //Holds all tables per parameter index
            bmap<uint32_t, bset<DbTable const*>> m_parameterIndexInTables;
            bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> m_perTableStatements;

            //not copyable
            PrepareInfo(PrepareInfo const&) = delete;
            PrepareInfo& operator=(PrepareInfo const&) = delete;

        public:
            PrepareInfo(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp) 
                : m_ctx(ctx), m_exp(exp), m_classNameExp(*exp.GetClassNameExp()), m_assignmentListExp(*exp.GetAssignmentListExp()), m_whereExp(exp.GetWhereClauseExp()), m_optionsExp(exp.GetOptionsClauseExp()),
                m_ecsqlRenderContext(Exp::ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter) {}

            ECSqlPrepareContext& GetContext() const { return m_ctx; }
            UpdateStatementExp const& GetExp() const { return m_exp; }
            ClassNameExp const& GetClassNameExp() const { return m_classNameExp; }
            AssignmentListExp const& GetAssignmentListExp() const { return m_assignmentListExp; }
            bool HasWhereExp() const { return m_whereExp != nullptr; }
            WhereExp const* GetWhereExp() const { return m_whereExp; }
            bool HasOptionsExp() const { return m_optionsExp != nullptr; }
            OptionsExp const* GetOptionsExp() const { return m_optionsExp; }

            Exp::ECSqlRenderContext const& GetECSqlRenderContext() const { return m_ecsqlRenderContext; }
            Exp::ECSqlRenderContext& GetECSqlRenderContextR() { return m_ecsqlRenderContext; }
            void AddAssignmentExp(AssignmentExp const& exp, DbTable const& table)
                {
                m_tablesInvolvedInAssignmentClause.insert(&table);
                m_assignmentExpsByTable[&table].push_back(&exp);
                }

            bmap<DbTable const*, bvector<AssignmentExp const*>> const& GetAssignmentExpsByTable() const { return m_assignmentExpsByTable; }

            bmap<uint32_t, bset<DbTable const*>> const& GetTablesByParameterIndex() const { return m_parameterIndexInTables; }
            bool ParameterIndexExists(uint32_t paramIndex) const { return m_parameterIndexInTables.find(paramIndex) != m_parameterIndexInTables.end(); }
            //!@return true if index already existed, false otherwise
            void AddParameterIndex(uint32_t paramIndex, DbTable const& table){ m_parameterIndexInTables[paramIndex].insert(&table); }

            void AddWhereClauseParameterIndex(uint32_t paramIndex)
                {
                m_parameterIndexInTables[paramIndex].insert(m_tablesInvolvedInAssignmentClause.begin(), m_tablesInvolvedInAssignmentClause.end());
                }

            bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> const& GetLeafStatementsByTable() const { return m_perTableStatements; }
            void AddLeafStatement(SingleContextTableECSqlPreparedStatement const& stmt, DbTable const& table) { m_perTableStatements[&table] = &stmt; }

            bool IsSingleTableInvolvedInAssignmentClause() const { return m_assignmentExpsByTable.size() == 1; }
            DbTable const* GetSingleTableInvolvedInAssignmentClause() const { BeAssert(IsSingleTableInvolvedInAssignmentClause()); return m_assignmentExpsByTable.begin()->first; }
            };

        std::unique_ptr<ECSqlSelectPreparedStatement> m_whereClauseSelector;

        ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;

        ECSqlStatus PreprocessWhereClause(PrepareInfo&);
        bool IsWhereClauseSelectorStatementNeeded(PrepareInfo const&) const;
        ECSqlStatus PrepareLeafStatements(PrepareInfo&);
        ECSqlStatus PopulateProxyBinders(PrepareInfo const&);

        ECSqlStatus CheckForReadonlyProperties(PrepareInfo const&) const;

    public:
        explicit ECSqlUpdatePreparedStatement(ECDb const& ecdb) : CompoundECSqlPreparedStatement(ecdb, ECSqlType::Update) {}

        DbResult Step();
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparedStatement final : SingleECSqlPreparedStatement
    {
private:
    ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;

public:
    explicit ECSqlDeletePreparedStatement(ECDb const& ecdb) : SingleECSqlPreparedStatement(ecdb, ECSqlType::Delete) {}
    DbResult Step();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
