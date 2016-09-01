/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include  <ECDb/ECSqlStatement.h>
#include "ECSqlParser.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlField.h"
#include "ECSqlBinder.h"
#include "DynamicSelectClauseECClass.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct ParentOfJoinedTableECSqlStatement;

//=======================================================================================
//! Represents a prepared ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPreparedStatement : NonCopyableClass
    {
    private:
        ECSqlType m_type;

        ECDbCP m_ecdb;
        Utf8String m_ecsql;
        Utf8String m_nativeSql;
        mutable BeSQLite::Statement m_sqliteStatement;
        bool m_isNoopInSqlite;
        ECSqlParameterMap m_parameterMap;
        std::unique_ptr<ParentOfJoinedTableECSqlStatement> m_parentOfJoinedTableECSqlStatement;

        virtual ECSqlStatus _Reset() = 0;

    protected:
        ECSqlPreparedStatement(ECSqlType, ECDbCR);

        DbResult DoStep();
        ECSqlStatus DoReset();

        ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
        bool IsNoopInSqlite() const { return m_isNoopInSqlite; }

        IssueReporter const& GetIssueReporter() const { return m_ecdb->GetECDbImplR().GetIssueReporter(); }

    public:
        virtual ~ECSqlPreparedStatement() {}

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

        ECDbCR GetECDb() const { return *m_ecdb; }
        BeSQLite::Statement& GetSqliteStatementR() const { return m_sqliteStatement; }

        ECSqlParameterMap& GetParameterMapR() { return m_parameterMap; }
    };

//=======================================================================================
//! Represents a prepared SELECT ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlSelectPreparedStatement : public ECSqlPreparedStatement
    {
private:
    DynamicSelectClauseECClass m_dynamicSelectClauseECClass;
    ECSqlField::Collection m_fields;
    //Calls to OnAfterStep/Reset on ECSqlFields can be very many, so only call it on fields that require it.
    std::vector<ECSqlField*> m_fieldsRequiringOnAfterStep;
    std::vector<ECSqlField*> m_fieldsRequiringReset;

    virtual ECSqlStatus _Reset () override;

    ECSqlStatus ResetFields () const;
    ECSqlStatus OnAfterStep () const;

public:
    explicit ECSqlSelectPreparedStatement (ECDbCR ecdb) : ECSqlPreparedStatement (ECSqlType::Select, ecdb) {}
    ~ECSqlSelectPreparedStatement () {}

    DbResult Step ();

    int GetColumnCount () const;
    IECSqlValue const& GetValue (int columnIndex) const;

    ECSqlField::Collection const& GetFields () const { return m_fields; }
    void AddField (std::unique_ptr<ECSqlField> field);

    DynamicSelectClauseECClass& GetDynamicSelectClauseECClassR() { return m_dynamicSelectClauseECClass; }
    };

//=======================================================================================
//! Represents a prepared non-SELECT (INSERT, UPDATE and DELETE) ECSqlStatement with all 
//! additional information needed for post-prepare operations
// @bsiclass                                                Affan.Khan           02/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlNonSelectPreparedStatement : public ECSqlPreparedStatement
    {
protected:
    ECSqlNonSelectPreparedStatement (ECSqlType statementType, ECDbCR ecdb) :ECSqlPreparedStatement (statementType, ecdb)  {}
    virtual ECSqlStatus _Reset () override;

public:
    virtual ~ECSqlNonSelectPreparedStatement () {}
    };

//=======================================================================================
//! Represents a prepared SELECT ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlInsertPreparedStatement : public ECSqlNonSelectPreparedStatement
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

        explicit ECInstanceKeyInfo () :  m_ecInstanceIdBinder(nullptr) {}

        ECInstanceKeyInfo (ECN::ECClassId ecClassId, ECSqlBinder& ecInstanceIdBinder)
            : m_ecClassId (ecClassId), m_ecInstanceIdBinder (&ecInstanceIdBinder)
            {}

        ECInstanceKeyInfo (ECN::ECClassId ecClassId, ECInstanceId userProvidedLiteral)
            : m_ecClassId (ecClassId), m_ecInstanceIdBinder (nullptr), m_userProvidedECInstanceId (userProvidedLiteral)
            {}

        ECN::ECClassId GetECClassId() const { BeAssert(m_ecClassId.IsValid()); return m_ecClassId; }

        ECSqlBinder* GetECInstanceIdBinder () const { return m_ecInstanceIdBinder; }
        bool HasUserProvidedECInstanceId () const {return m_userProvidedECInstanceId.IsValid ();}
        ECInstanceId GetUserProvidedECInstanceId () const { return m_userProvidedECInstanceId; }

        void SetBoundECInstanceId (ECInstanceId ecinstanceId) {m_userProvidedECInstanceId = ecinstanceId;}
        void ResetBoundECInstanceId ()
            {
            if (m_ecInstanceIdBinder != nullptr)
                m_userProvidedECInstanceId.Invalidate ();
            }
        };

private:
    ECInstanceKeyInfo m_ecInstanceKeyInfo;

    bool m_isECInstanceIdAutogenerationDisabled;

    ECSqlStatus GenerateECInstanceIdAndBindToInsertStatement (ECInstanceId& generatedECInstanceId);
  

public:
    explicit ECSqlInsertPreparedStatement (ECDbCR ecdb) : ECSqlNonSelectPreparedStatement(ECSqlType::Insert, ecdb), m_isECInstanceIdAutogenerationDisabled(false) {}
    ~ECSqlInsertPreparedStatement () {}

    DbResult Step (ECInstanceKey& instanceKey);

    ECInstanceKeyInfo& GetECInstanceKeyInfo () {return m_ecInstanceKeyInfo;}
    void SetECInstanceKeyInfo (ECInstanceKeyInfo const& ecInstanceKeyInfo);
    void SetIsECInstanceIdAutogenerationDisabled() { m_isECInstanceIdAutogenerationDisabled = true; }
    };



//=======================================================================================
//! Represents a prepared Update ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparedStatement : public ECSqlNonSelectPreparedStatement
    {
public:
    explicit ECSqlUpdatePreparedStatement(ECDbCR ecdb) : ECSqlNonSelectPreparedStatement(ECSqlType::Update, ecdb) {}
    ~ECSqlUpdatePreparedStatement() {}
    DbResult Step();
    };


//=======================================================================================
//! Represents a prepared Delete ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparedStatement : public ECSqlNonSelectPreparedStatement
    {
public:
    explicit ECSqlDeletePreparedStatement(ECDbCR ecdb) : ECSqlNonSelectPreparedStatement(ECSqlType::Delete, ecdb) {}
    ~ECSqlDeletePreparedStatement() {}
    DbResult Step();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

