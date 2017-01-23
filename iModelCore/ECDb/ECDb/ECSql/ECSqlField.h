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
public:
    typedef std::vector<std::unique_ptr<ECSqlField>> Collection;

protected:
    ECSqlColumnInfo m_ecsqlColumnInfo;

private:
    static Collection* s_emptyChildCollection;
    ECSqlStatementBase& m_ecsqlStatement;

    ECSqlColumnInfoCR _GetColumnInfo() const override;

    virtual ECSqlStatus _OnAfterReset() { return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }
    virtual Collection const& _GetChildren() const { return GetEmptyChildCollection(); }

    static Collection const& GetEmptyChildCollection();

protected:
    bool m_requiresOnAfterStep;
    bool m_requiresOnAfterReset;

    ECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecsqlStatement(ecsqlStatement), m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    ECSqlStatus ReportError(ECSqlStatus status, Utf8CP errorMessage) const;
    ECSqlStatementBase& GetECSqlStatementR() const;
    Statement& GetSqliteStatement() const;

public:
    virtual ~ECSqlField() {}

    Collection const& GetChildren() const { return _GetChildren(); }

    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep();
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset; }
    ECSqlStatus OnAfterReset();

    static Utf8CP GetPrimitiveGetMethodName(ECN::PrimitiveType getMethodType);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE