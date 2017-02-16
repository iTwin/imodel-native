/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlValue.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlStatementBase;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlField : public IECSqlValue
    {
protected:
    ECSqlColumnInfo m_ecsqlColumnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;

private:
    ECSqlStatementBase& m_ecsqlStatement;

    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_ecsqlColumnInfo; }

    virtual ECSqlStatus _OnAfterReset() { return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }

protected:
    ECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecsqlStatement(ecsqlStatement), m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    ECSqlStatementBase& GetECSqlStatementR() const;
    Statement& GetSqliteStatement() const;

public:
    virtual ~ECSqlField() {}

    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset; }
    ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE