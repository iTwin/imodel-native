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

        ECSqlStatus Prepare(ECSqlPrepareContext& ctx, Exp const& exp, Utf8CP ecsql);
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
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct LeafECSqlPreparedStatement : IECSqlPreparedStatement
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
    LeafECSqlPreparedStatement(ECDb const& ecdb, ECSqlType type) : IECSqlPreparedStatement(ecdb, type) {}
    virtual ~LeafECSqlPreparedStatement() {}

    DbResult DoStep();

    ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct CompoundECSqlPreparedStatement : IECSqlPreparedStatement
    {
    protected:
        std::vector<std::unique_ptr<LeafECSqlPreparedStatement>> m_statements;
        mutable bmap<int, ProxyECSqlBinder> m_proxyBinderMap;
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
struct ECSqlSelectPreparedStatement final : LeafECSqlPreparedStatement
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
        explicit ECSqlSelectPreparedStatement(ECDb const& ecdb) : LeafECSqlPreparedStatement(ecdb, ECSqlType::Select) {}

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
        struct ECInstanceKeyInfo
            {
            private:
                ECN::ECClassId m_ecClassId;
                ECSqlBinder* m_ecInstanceIdBinder;
                ECInstanceId m_userProvidedECInstanceId;

            public:
                //compiler generated copy ctor and copy assignment
                explicit ECInstanceKeyInfo() : m_ecInstanceIdBinder(nullptr) {}
                ECInstanceKeyInfo(ECN::ECClassId ecClassId, ECSqlBinder& ecInstanceIdBinder) : m_ecClassId(ecClassId), m_ecInstanceIdBinder(&ecInstanceIdBinder) {}
                ECInstanceKeyInfo(ECN::ECClassId ecClassId, ECInstanceId userProvidedLiteral) : m_ecClassId(ecClassId), m_ecInstanceIdBinder(nullptr), m_userProvidedECInstanceId(userProvidedLiteral) {}

                ECN::ECClassId GetECClassId() const { BeAssert(m_ecClassId.IsValid()); return m_ecClassId; }

                ECSqlBinder* GetECInstanceIdBinder() const { return m_ecInstanceIdBinder; }
                bool HasUserProvidedECInstanceId() const { return m_userProvidedECInstanceId.IsValid(); }
                ECInstanceId GetUserProvidedECInstanceId() const { return m_userProvidedECInstanceId; }

                void SetBoundECInstanceId(ECInstanceId ecinstanceId) { m_userProvidedECInstanceId = ecinstanceId; }
                void ResetBoundECInstanceId()
                    {
                    if (m_ecInstanceIdBinder != nullptr)
                        m_userProvidedECInstanceId.Invalidate();
                    }
            };

    private:
        ECInstanceKeyInfo m_ecInstanceKeyInfo;

        ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;

        ECSqlStatus GenerateECInstanceIdAndBindToInsertStatement(ECInstanceId& generatedECInstanceId);

    public:
        explicit ECSqlInsertPreparedStatement(ECDb const& ecdb) : CompoundECSqlPreparedStatement(ecdb, ECSqlType::Insert) {}

        DbResult Step(ECInstanceKey&);
        ECInstanceKeyInfo& GetECInstanceKeyInfo() { return m_ecInstanceKeyInfo; }
        void SetECInstanceKeyInfo(ECInstanceKeyInfo const& ecInstanceKeyInfo) { BeAssert(ecInstanceKeyInfo.GetECClassId().IsValid()); m_ecInstanceKeyInfo = ecInstanceKeyInfo; }
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
struct ECSqlDeletePreparedStatement final : LeafECSqlPreparedStatement
    {
public:
    explicit ECSqlDeletePreparedStatement(ECDb const& ecdb) : LeafECSqlPreparedStatement(ecdb, ECSqlType::Delete) {}
    DbResult Step();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
