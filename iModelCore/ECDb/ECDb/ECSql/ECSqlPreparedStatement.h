/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
#include "ECSqlStatusContext.h"
#include "DynamicSelectClauseECClass.h"
#include "ECSqlStepTask.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
    mutable BeSQLite::Statement m_sqliteStatement;
    bool m_isNoopInSqlite;
    bool m_isNothingToUpdate;
    ECSqlParameterMap m_parameterMap;
    std::vector<ECN::ECSchemaPtr> m_keepAliveSchemas; //Hold on to dependent ECSchema just in case some process flush original ECSchema
    ECSqlStatusContext& m_statusContext;

    virtual ECSqlStatus _Reset () = 0;

protected:
    ECSqlPreparedStatement (ECSqlType statementType, ECDbCR ecdb, ECSqlStatusContext& statusContext);

    void OnBeforeStep();

    ECSqlStepStatus DoStep ();
    ECSqlStatus DoReset ();

    ECSqlParameterMap const& GetParameterMap () const { return m_parameterMap; }
    ECSqlStatus ResetStatus () const;
    ECSqlStatusContext& GetStatusContextR () const { return m_statusContext; }

    void PrepareBinders ();

    bool IsNoopInSqlite () const { return m_isNoopInSqlite; }
    bool IsNothingToUpdate() const { return m_isNothingToUpdate; }

public:
    virtual ~ECSqlPreparedStatement () {}



    ECSqlType GetType () const { return m_type; }

    ECSqlStatus Prepare (ECSqlPrepareContext& prepareContext, ECSqlParseTreeCR ecsqlParseTree, Utf8CP ecsql);

    IECSqlBinder& GetBinder (int parameterIndex);
    int GetParameterIndex (Utf8CP parameterName) const;

    ECSqlStatus ClearBindings ();
    ECSqlStatus Reset ();

    Utf8CP GetECSql () const;
    Utf8CP GetNativeSql () const;

    ECDbCR GetECDb () const { return *m_ecdb; }
    BeSQLite::Statement& GetSqliteStatementR () const { return m_sqliteStatement; }

    ECSqlParameterMap& GetParameterMapR () { return m_parameterMap; }

    void AddKeepAliveSchema (ECN::ECSchemaCR schema);
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

    virtual ECSqlStatus _Reset () override;

    ECSqlStatus ResetFields () const;
    ECSqlStatus InitFields () const;

public:
    ECSqlSelectPreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext)
        : ECSqlPreparedStatement (ECSqlType::Select, ecdb, statusContext)
        {}

    ~ECSqlSelectPreparedStatement () {}

    ECSqlStepStatus Step ();

    int GetColumnCount () const;
    IECSqlValue const& GetValue (int columnIndex) const;

    ECSqlField::Collection const& GetFields () const { return m_fields; }
    void AddField (std::unique_ptr<ECSqlField> field);

    DynamicSelectClauseECClass& GetDynamicSelectClauseECClassR ();
    };

//=======================================================================================
//! Represents a prepared non-SELECT (INSERT, UPDATE and DELETE) ECSqlStatement with all 
//! additional information needed for post-prepare operations
// @bsiclass                                                Affan.Khan           02/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlNonSelectPreparedStatement : public ECSqlPreparedStatement
    {
private:
    ECSqlStepTask::Collection m_stepTasks;

protected:
    ECSqlNonSelectPreparedStatement (ECSqlType statementType, ECDbCR ecdb, ECSqlStatusContext& statusContext)
        :ECSqlPreparedStatement (statementType, ecdb, statusContext), m_stepTasks ()
        {}


    virtual ECSqlStatus _Reset () override;

public:

    virtual ~ECSqlNonSelectPreparedStatement () {}
    ECSqlStepTask::Collection& GetStepTasks () { return m_stepTasks; }
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

        explicit ECInstanceKeyInfo ()
            : m_ecClassId(ECN::ECClass::UNSET_ECCLASSID), m_ecInstanceIdBinder(nullptr)
            {}

        ECInstanceKeyInfo (ECN::ECClassId ecClassId, ECSqlBinder& ecInstanceIdBinder)
            : m_ecClassId (ecClassId), m_ecInstanceIdBinder (&ecInstanceIdBinder)
            {}

        ECInstanceKeyInfo (ECN::ECClassId ecClassId, ECInstanceId userProvidedLiteral)
            : m_ecClassId (ecClassId), m_ecInstanceIdBinder (nullptr), m_userProvidedECInstanceId (userProvidedLiteral)
            {}

        ECN::ECClassId GetECClassId() const { BeAssert(m_ecClassId > ECN::ECClass::UNSET_ECCLASSID); return m_ecClassId; }

        ECSqlBinder* GetECInstanceIdBinder () const { return m_ecInstanceIdBinder; }
        bool HasUserProvidedECInstanceId () const {return m_userProvidedECInstanceId.IsValid ();}
        ECInstanceId const& GetUserProvidedECInstanceId () const { return m_userProvidedECInstanceId; }

        void SetBoundECInstanceId (ECInstanceId const& ecinstanceId) {m_userProvidedECInstanceId = ecinstanceId;}
        void ResetBoundECInstanceId ()
            {
            if (m_ecInstanceIdBinder != nullptr)
                m_userProvidedECInstanceId.Invalidate ();
            }
        };

private:
    ECInstanceKeyInfo m_ecInstanceKeyInfo;
    ECSqlStatus GenerateECInstanceIdAndBindToInsertStatement (ECInstanceId& generatedECInstanceId);
  

public:
    ECSqlInsertPreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext);

    ~ECSqlInsertPreparedStatement () {}

    ECSqlStepStatus Step (ECInstanceKey& instanceKey);

    ECInstanceKeyInfo& GetECInstanceKeyInfo () {return m_ecInstanceKeyInfo;}
    void SetECInstanceKeyInfo (ECInstanceKeyInfo const& ecInstanceKeyInfo);
    };



//=======================================================================================
//! Represents a prepared Update ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlUpdatePreparedStatement : public ECSqlNonSelectPreparedStatement
    {
private:

public:
    ECSqlUpdatePreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext);
    ~ECSqlUpdatePreparedStatement () {}

    ECSqlStepStatus Step ();
    };


//=======================================================================================
//! Represents a prepared Delete ECSqlStatement with all additional information needed for
//! post-prepare operations
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparedStatement : public ECSqlNonSelectPreparedStatement
    {
private:
   

public:
    ECSqlDeletePreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext);
    ~ECSqlDeletePreparedStatement () {}

    ECSqlStepStatus Step ();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

