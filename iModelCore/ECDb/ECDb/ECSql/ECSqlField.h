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

    virtual ECSqlColumnInfoCR _GetColumnInfo () const override;

    virtual ECSqlStatus _Reset () { return ECSqlStatus::Success; }
    virtual ECSqlStatus _Init () { return ECSqlStatus::Success; }

    static Collection s_emptyChildCollection;

protected:
    bool m_requiresInit;
    bool m_requiresReset;

    ECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, bool needsInit, bool needsReset)
        : m_ecsqlStatement(ecsqlStatement), m_ecsqlColumnInfo(std::move(ecsqlColumnInfo)), m_requiresInit(needsInit), m_requiresReset(needsReset)
        {}

    ECSqlStatus ReportError (ECSqlStatus status, Utf8CP errorMessage) const;
    ECSqlStatementBase& GetECSqlStatementR () const;
    Statement& GetSqliteStatement () const;

public:
    virtual ~ECSqlField () {}

    virtual Collection const& GetChildren () const {return s_emptyChildCollection;}

    bool RequiresInit() const { return m_requiresInit; }
    ECSqlStatus Init ();
    bool RequiresReset() const { return m_requiresReset; }
    ECSqlStatus Reset ();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE