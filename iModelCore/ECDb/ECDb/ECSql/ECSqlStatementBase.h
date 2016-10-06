/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementBase.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include "ECSqlPreparedStatement.h"
#include "ECSqlParser.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatementBase is the base class used to implement ECSqlStatement
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlStatementBase
    {
    private:
        std::unique_ptr<ECSqlPreparedStatement> m_preparedStatement;

        virtual ECSqlPrepareContext _InitializePrepare(ECDb const&, Utf8CP ecsql) = 0;

        ECSqlPreparedStatement& CreatePreparedStatement(ECDb const&, Exp const&);

        ECSqlStatus FailIfNotPrepared(Utf8CP errorMessage) const;
        ECSqlStatus FailIfWrongType(ECSqlType expectedType, Utf8CP errorMessage) const;

    protected:
        ECSqlStatementBase() : m_preparedStatement(nullptr) {}

        virtual ECSqlStatus _Prepare(ECDb const&, Utf8CP ecsql);
        virtual void _Finalize();

    public:
        virtual ~ECSqlStatementBase() {}
        //Public API mirrors
        ECSqlStatus Prepare(ECDb const&, Utf8CP ecsql);
        bool IsPrepared() const { return GetPreparedStatementP() != nullptr; }

        IECSqlBinder& GetBinder(int parameterIndex) const;
        int GetParameterIndex(Utf8CP parameterName) const;
        ECSqlStatus ClearBindings();

        DbResult Step();
        DbResult Step(ECInstanceKey&);
        ECSqlStatus Reset();

        int GetColumnCount() const;
        IECSqlValue const& GetValue(int columnIndex) const;

        Utf8CP GetECSql() const;
        Utf8CP GetNativeSql() const;
        ECDb const* GetECDb() const;

        void Finalize() { _Finalize(); }

        // Helpers
        ECSqlPreparedStatement* GetPreparedStatementP() const { return m_preparedStatement.get(); }

        template <class TECSqlPreparedStatement>
        TECSqlPreparedStatement* GetPreparedStatementP() const
            {
            BeAssert(dynamic_cast<TECSqlPreparedStatement*> (GetPreparedStatementP()) != nullptr);
            return static_cast<TECSqlPreparedStatement*> (GetPreparedStatementP());
            }

    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      12/2015
//+===============+===============+===============+===============+===============+======
struct ParentOfJoinedTableECSqlStatement : public ECSqlStatementBase
    {
    private:
        ECN::ECClassId m_classId;
        IECSqlBinder* m_ecInstanceIdBinder;

        virtual ECSqlPrepareContext _InitializePrepare(ECDb const& ecdb, Utf8CP ecsql) override { return ECSqlPrepareContext(ecdb, *this, m_classId); }

    public:
        explicit ParentOfJoinedTableECSqlStatement(ECN::ECClassId joinTableClassId) : ECSqlStatementBase(), m_classId(joinTableClassId), m_ecInstanceIdBinder(nullptr) {}
        ~ParentOfJoinedTableECSqlStatement() {}

        ECN::ECClassId GetClassId() const { return m_classId; }
        void SetECInstanceIdBinder(int ecsqlParameterIndex) { m_ecInstanceIdBinder = &GetBinder(ecsqlParameterIndex); }
        IECSqlBinder* GetECInstanceIdBinder() { return m_ecInstanceIdBinder; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
