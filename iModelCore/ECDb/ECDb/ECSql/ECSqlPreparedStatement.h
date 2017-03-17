/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Exp.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlBinder.h"
#include "ECSqlField.h"
#include "DynamicSelectClauseECClass.h"

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct IECSqlPreparedStatement : NonCopyableClass
    {
    protected:
        ECDb const& m_ecdb;
        ECSqlType m_type;
        bool m_isNoopInSqlite = false;

    private:
        Utf8String m_ecsql;
        ECDb::Impl::ClearCacheCounter m_preparationClearCacheCounter;

        virtual ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) = 0;
        virtual IECSqlBinder& _GetBinder(int parameterIndex) const = 0;
        virtual int _GetParameterIndex(Utf8CP parameterName) const = 0;
        virtual ECSqlStatus _Reset() = 0;
        virtual ECSqlStatus _ClearBindings() = 0;
        virtual Utf8CP _GetNativeSql() const = 0;

    protected:
        IECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : m_ecdb(ecdb), m_type(type) {}

        BentleyStatus AssertIsValid() const;

    public:
        virtual ~IECSqlPreparedStatement() {}

        ECSqlStatus Prepare(ECSqlPrepareContext&, Exp const&, Utf8CP ecsql);
        IECSqlBinder& GetBinder(int parameterIndex) const;
        int GetParameterIndex(Utf8CP parameterName) const;

        ECSqlStatus ClearBindings();
        ECSqlStatus Reset();

        Utf8CP GetECSql() const { return m_ecsql.c_str(); }
        Utf8CP GetNativeSql() const;

        ECDb const& GetECDb() const { return m_ecdb; }
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

    ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;
    IECSqlBinder& _GetBinder(int parameterIndex) const override;
    int _GetParameterIndex(Utf8CP parameterName) const override;
    ECSqlStatus _ClearBindings() override;
    Utf8CP _GetNativeSql() const override { return m_sqliteStatement.GetSql(); }

protected:

    ECSqlStatus _Reset() override;

public:
    SingleECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : IECSqlPreparedStatement(ecdb, type) {}
    virtual ~SingleECSqlPreparedStatement() {}

    DbResult DoStep();

    ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
    ECSqlParameterMap& GetParameterMapR() { return m_parameterMap; }
    BeSQLite::Statement& GetSqliteStatement() { return m_sqliteStatement; }
    };

//=======================================================================================
//! IECSqlPreparedStatement for ECSQL that requires multiple SQLite statements to be executed
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct CompoundECSqlPreparedStatement : IECSqlPreparedStatement
    {
    protected:
        std::vector<std::unique_ptr<SingleECSqlPreparedStatement>> m_statements;
        mutable std::vector<std::unique_ptr<ProxyECSqlBinder>> m_proxyBinders;
        mutable bmap<Utf8CP, int, CompareIUtf8Ascii> m_parameterNameMap;

    private:
        mutable Utf8String m_compoundNativeSql;

        IECSqlBinder& _GetBinder(int parameterIndex) const override;
        int _GetParameterIndex(Utf8CP parameterName) const override;
        ECSqlStatus _Reset() override;
        ECSqlStatus _ClearBindings() override;
        Utf8CP _GetNativeSql() const override;

    protected:
        CompoundECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : IECSqlPreparedStatement(ecdb, type) {}

    public:
        virtual ~CompoundECSqlPreparedStatement() {}
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
public:
    public:
        struct ECInstanceKeyHelper final : NonCopyableClass
            {
            public:
                enum class Mode
                    {
                    NotUserProvided = 1,
                    UserProvidedNullExp,
                    UserProvidedNonNullExp
                    };

            private:
                Mode m_mode = Mode::NotUserProvided;
                ECN::ECClassId m_ecClassId;
                std::unique_ptr<ProxyECSqlBinder> m_idBinder = nullptr;
                ECInstanceId m_generatedECInstanceId;

            public:
                ECInstanceKeyHelper() {}

                void Initialize(int &ecInstanceIdPropNameExpIx, ECN::ECClassId, PropertyNameListExp const& propNameListExp, ValueExpListExp const& valueListExp);
                ECSqlStatus InitializeAutogenerateBinder(std::vector<std::unique_ptr<SingleECSqlPreparedStatement>>&) const;
                DbResult AutogenerateAndBindECInstanceId(ECDbCR);
                ECInstanceKey RetrieveLastInsertedKey(ECDbCR) const;

                Mode GetMode() const { return m_mode; }
                bool IsAutogenerateIdMode() const { return m_mode == Mode::NotUserProvided || m_mode == Mode::UserProvidedNullExp; }
            };

    private:
        ECInstanceKeyHelper m_ecInstanceKeyHelper;

        ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;

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
        ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;

    public:
        explicit ECSqlUpdatePreparedStatement(ECDb const& ecdb) : CompoundECSqlPreparedStatement(ecdb, ECSqlType::Update) {}

        DbResult Step();
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparedStatement final : SingleECSqlPreparedStatement
    {
public:
    explicit ECSqlDeletePreparedStatement(ECDb const& ecdb) : SingleECSqlPreparedStatement(ecdb, ECSqlType::Delete) {}
    DbResult Step();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

#endif