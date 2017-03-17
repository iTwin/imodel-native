/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement_Old.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/ECSqlStatement.h>
#include "ECSqlParser.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlField.h"
#include "ECSqlBinder.h"
#include "DynamicSelectClauseECClass.h"

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR


BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


struct ParentOfJoinedTableECSqlStatement;

//=======================================================================================
//! Represents a prepared ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPreparedStatement_Old : NonCopyableClass
    {
    private:
        ECSqlType m_type;

        ECDb const& m_ecdb;
        Utf8String m_ecsql;
        Utf8String m_nativeSql;
        mutable BeSQLite::Statement m_sqliteStatement;
        bool m_isNoopInSqlite;
        ECSqlParameterMap m_parameterMap;
        std::unique_ptr<ParentOfJoinedTableECSqlStatement> m_parentOfJoinedTableECSqlStatement;

        ECDb::Impl::ClearCacheCounter m_preparationClearCacheCounter;

        virtual ECSqlStatus _Reset() = 0;

    protected:
        ECSqlPreparedStatement_Old(ECSqlType, ECDb const&);

        DbResult DoStep();
        ECSqlStatus DoReset();

        ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
        bool IsNoopInSqlite() const { return m_isNoopInSqlite; }

        BentleyStatus AssertIsValid() const;

    public:
        virtual ~ECSqlPreparedStatement_Old() {}

        ECSqlType GetType() const { return m_type; }
        ECSqlStatus Prepare(ECSqlPrepareContext&, Exp const&, Utf8CP ecsql);
        IECSqlBinder& GetBinder(int parameterIndex);
        int GetParameterIndex(Utf8CP parameterName) const;

        ECSqlStatus ClearBindings();
        ECSqlStatus Reset();

        Utf8CP GetECSql() const { return m_ecsql.c_str(); }
        Utf8CP GetNativeSql() const;

        ParentOfJoinedTableECSqlStatement* CreateParentOfJoinedTableECSqlStatement(ECN::ECClassId joinedTableClassId);
        ParentOfJoinedTableECSqlStatement* GetParentOfJoinedTableECSqlStatement() const;

        ECDbCR GetECDb() const { return m_ecdb; }
        BeSQLite::Statement& GetSqliteStatementR() const { return m_sqliteStatement; }

        ECSqlParameterMap& GetParameterMapR() { return m_parameterMap; }
    };

typedef ECSqlPreparedStatement_Old IECSqlPreparedStatement;

//=======================================================================================
//! Represents a prepared SELECT ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlSelectPreparedStatement : public ECSqlPreparedStatement_Old
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
        explicit ECSqlSelectPreparedStatement(ECDbCR ecdb) : ECSqlPreparedStatement_Old(ECSqlType::Select, ecdb) {}
        ~ECSqlSelectPreparedStatement() {}

        DbResult Step();

        int GetColumnCount() const;
        IECSqlValue const& GetValue(int columnIndex) const;

        std::vector<std::unique_ptr<ECSqlField>> const& GetFields() const { return m_fields; }
        void AddField(std::unique_ptr<ECSqlField>);

        DynamicSelectClauseECClass& GetDynamicSelectClauseECClassR() { return m_dynamicSelectClauseECClass; }
    };

//=======================================================================================
//! Represents a prepared non-SELECT (INSERT, UPDATE and DELETE) ECSqlStatement with all 
//! additional information needed for post-prepare operations
// @bsiclass                                                Affan.Khan           02/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlNonSelectPreparedStatement_Old : public ECSqlPreparedStatement_Old
    {
    protected:
        ECSqlNonSelectPreparedStatement_Old(ECSqlType statementType, ECDbCR ecdb) :ECSqlPreparedStatement_Old(statementType, ecdb) {}
        ECSqlStatus _Reset() override { return DoReset(); }

    public:
        virtual ~ECSqlNonSelectPreparedStatement_Old() {}
    };

//=======================================================================================
//! Represents a prepared SELECT ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlInsertPreparedStatement_Old : public ECSqlNonSelectPreparedStatement_Old
    {
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

                ECInstanceKeyInfo(ECN::ECClassId ecClassId, ECSqlBinder& ecInstanceIdBinder)
                    : m_ecClassId(ecClassId), m_ecInstanceIdBinder(&ecInstanceIdBinder)
                    {}

                ECInstanceKeyInfo(ECN::ECClassId ecClassId, ECInstanceId userProvidedLiteral)
                    : m_ecClassId(ecClassId), m_ecInstanceIdBinder(nullptr), m_userProvidedECInstanceId(userProvidedLiteral)
                    {}

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

        ECSqlStatus GenerateECInstanceIdAndBindToInsertStatement(ECInstanceId& generatedECInstanceId);

    public:
        explicit ECSqlInsertPreparedStatement_Old(ECDbCR ecdb) : ECSqlNonSelectPreparedStatement_Old(ECSqlType::Insert, ecdb) {}
        ~ECSqlInsertPreparedStatement_Old() {}

        DbResult Step(ECInstanceKey& instanceKey);

        ECInstanceKeyInfo& GetECInstanceKeyInfo() { return m_ecInstanceKeyInfo; }
        void SetECInstanceKeyInfo(ECInstanceKeyInfo const& ecInstanceKeyInfo);
    };

typedef ECSqlInsertPreparedStatement_Old ECSqlInsertPreparedStatement;


//=======================================================================================
//! Represents a prepared Update ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparedStatement_Old : public ECSqlNonSelectPreparedStatement_Old
    {
    public:
        explicit ECSqlUpdatePreparedStatement_Old(ECDbCR ecdb) : ECSqlNonSelectPreparedStatement_Old(ECSqlType::Update, ecdb) {}
        ~ECSqlUpdatePreparedStatement_Old() {}
        DbResult Step();
    };

typedef ECSqlUpdatePreparedStatement_Old ECSqlUpdatePreparedStatement;

//=======================================================================================
//! Represents a prepared Delete ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparedStatement_Old : public ECSqlNonSelectPreparedStatement_Old
    {
    public:
        explicit ECSqlDeletePreparedStatement_Old(ECDbCR ecdb) : ECSqlNonSelectPreparedStatement_Old(ECSqlType::Delete, ecdb) {}
        ~ECSqlDeletePreparedStatement_Old() {}
        DbResult Step();
    };

typedef ECSqlDeletePreparedStatement_Old ECSqlDeletePreparedStatement;

END_BENTLEY_SQLITE_EC_NAMESPACE

#endif
