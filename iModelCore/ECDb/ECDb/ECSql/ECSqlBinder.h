/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/IECSqlBinder.h>
#include "ECSqlBinderFactory.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlStatementBase;

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBinder : IECSqlBinder
    {
private:
    std::function<void (ECInstanceId const& bindValue)> m_onBindRepositoyBasedIdEventHandler;
    ECSqlStatementBase& m_ecsqlStatement;
    ECSqlTypeInfo m_typeInfo;
    int m_mappedSqlParameterCount;
    IECSqlBinder* m_onBindEventHandler;
    bool m_hasToCallOnBeforeStep;
    bool m_hasToCallOnClearBindings;

    virtual void _SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteIndex) = 0;
    virtual ECSqlStatus _OnBeforeStep () { return ECSqlStatus::Success; }
    virtual void _OnClearBindings () {}

protected:
    ECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings)
        : m_ecsqlStatement (ecsqlStatement), m_typeInfo (typeInfo), m_mappedSqlParameterCount (mappedSqlParameterCount), m_onBindEventHandler (nullptr), m_hasToCallOnBeforeStep (hasToCallOnBeforeStep), m_hasToCallOnClearBindings (hasToCallOnClearBindings)
        {}

    //Part of initialization. Must only called in constructor.
    void SetMappedSqlParameterCount (int mappedSqlParameterCount) { m_mappedSqlParameterCount = mappedSqlParameterCount; }
        
    std::function<void (ECInstanceId const& bindValue)> GetOnBindRepositoryBasedIdEventHandler () const { return m_onBindRepositoyBasedIdEventHandler; }
    static NoopECSqlBinder& GetNoopBinder (ECSqlStatus status);
    IECSqlBinder* GetOnBindEventHandler () { return m_onBindEventHandler; }

    ECSqlStatus ReportError(DbResult sqliteStat, Utf8CP errorMessageHeader = nullptr) const;

    Statement& GetSqliteStatementR() const;
    ECSqlStatementBase& GetECSqlStatementR() const { return m_ecsqlStatement; }
    ECDbCR GetECDb() const;
    static Statement::MakeCopy ToBeSQliteBindMakeCopy (IECSqlBinder::MakeCopy makeCopy);

public:
    virtual ~ECSqlBinder () {}

    bool HasToCallOnBeforeStep() const { return m_hasToCallOnBeforeStep; }
    bool HasToCallOnClearBindings() const { return m_hasToCallOnClearBindings; }

    int GetMappedSqlParameterCount () const;
    void SetSqliteIndex (size_t sqliteIndex);
    void SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteIndex);

    ECSqlTypeInfo const& GetTypeInfo () const { return m_typeInfo; }

    ECSqlStatus OnBeforeStep ();
    void OnClearBindings ();
    void SetOnBindEventHandler (IECSqlBinder& binder) { BeAssert (m_onBindEventHandler == nullptr); m_onBindEventHandler = &binder; }
    void SetOnBindRepositoryBasedIdEventHandler (std::function<void (ECInstanceId const& bindValue)> eventHandler) { BeAssert (m_onBindRepositoyBasedIdEventHandler == nullptr); m_onBindRepositoyBasedIdEventHandler = eventHandler; }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParameterMap : NonCopyableClass
    {
private:
    std::vector<std::unique_ptr<ECSqlBinder>> m_binders;
    std::vector<std::unique_ptr<ECSqlBinder>> m_internalSqlParameterBinders;
    bmap<Utf8String, int> m_nameToIndexMapping;

    std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;
    std::vector<ECSqlBinder*> m_bindersToCallOnStep;

    bool Contains (int& ecsqlParameterIndex, Utf8CP ecsqlParameterName) const;

public:
    ECSqlParameterMap () {}
    ~ECSqlParameterMap() {}

    size_t Count () const { return m_binders.size(); }
    //! @remarks only named parameters hav an identity. Therefore each unnamed parameters has its own binder
    bool TryGetBinder (ECSqlBinder*& binder, Utf8CP ecsqlParameterName) const;
    //!@param[in] ecsqlParameterIndex ECSQL parameter index (1-based)
    ECSqlStatus TryGetBinder (ECSqlBinder*& binder, int ecsqlParameterIndex) const;

    //!@param[in] internalBinderIndex Index of the internal binder as stored in the internal binder vector (0-based)
    ECSqlStatus TryGetInternalBinder (ECSqlBinder*& binder, size_t internalBinderIndex) const;

    //!@return ECSQL Parameter index (1-based) or -1 if index could not be found for @p ecsqlParameterName
    int GetIndexForName (Utf8CP ecsqlParameterName) const;

    ECSqlBinder* AddBinder (ECSqlStatementBase& ecsqlStatement, ParameterExp const& parameterExp, bool targetIsVirtual = false, bool enforceConstraints = false);
    ECSqlBinder* AddInternalBinder (size_t& index, ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo);

    ECSqlStatus OnBeforeStep ();

    //Bindings in SQLite have already been cleared at this point. The method
    //allows subclasses to clean-up additional resources tied to binding parameters
    void OnClearBindings ();
    };



//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    07/2014
//+===============+===============+===============+===============+===============+======
struct ArrayConstraintValidator
    {
private:
    ArrayConstraintValidator ();
    ~ArrayConstraintValidator ();

public:
    static ECSqlStatus Validate (ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
    static ECSqlStatus ValidateMaximum(ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE