/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
public:
    typedef std::vector<std::unique_ptr<ECSqlField>> Collection;

protected:
    ECSqlColumnInfo m_ecsqlColumnInfo;

private:
    ECSqlStatementBase& m_ecsqlStatement;

    virtual ECSqlColumnInfoCR _GetColumnInfo() const override;

    virtual ECSqlStatus _OnAfterReset() { return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }

    static Collection s_emptyChildCollection;

protected:
    bool m_requiresOnAfterStep;
    bool m_requiresOnAfterReset;

    ECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecsqlStatement(ecsqlStatement), m_ecsqlColumnInfo(std::move(ecsqlColumnInfo)), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    ECSqlStatus ReportError(ECSqlStatus status, Utf8CP errorMessage) const;
    ECSqlStatementBase& GetECSqlStatementR() const;
    Statement& GetSqliteStatement() const;

public:
    virtual ~ECSqlField() {}

    virtual Collection const& GetChildren() const { return s_emptyChildCollection; }

    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep();
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset; }
    ECSqlStatus OnAfterReset();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE