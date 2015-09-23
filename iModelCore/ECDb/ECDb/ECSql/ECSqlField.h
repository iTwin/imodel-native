/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    bool m_needsInit;
    bool m_needsReset;

    ECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, bool needsInit, bool needsReset)
        : m_ecsqlStatement(ecsqlStatement), m_ecsqlColumnInfo(std::move(ecsqlColumnInfo)), m_needsInit(needsInit), m_needsReset(needsReset)
        {}

    ECSqlStatus ReportError (ECSqlStatus status, Utf8CP errorMessage) const;
    ECSqlStatementBase& GetECSqlStatementR () const;
    Statement& GetSqliteStatement () const;

public:
    virtual ~ECSqlField () {}

    virtual Collection const& GetChildren () const {return s_emptyChildCollection;}

    bool RequiresInit() const { return m_needsInit; }
    ECSqlStatus Init ();
    bool RequiresReset() const { return m_needsReset; }
    ECSqlStatus Reset ();
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      09/2013
//ECSQL_TODO: Need to move this to another file
//+===============+===============+===============+===============+===============+======
struct ECSqlPrimitiveBinder
    {
    enum class StatementType
        {
        ECSql, Sqlite, Unknown
        };
private:
    Utf8String m_sourcePropertyPath;
    int m_sourceColumnIndex;
    int m_targetParameterIndex;
    StatementType m_sourceStmtType;
public:
    ECSqlPrimitiveBinder();
    void SetSourcePropertyPath(Utf8CP column);
    void SetSourceStatementType(StatementType type);
    void SetSourceColumnIndex(int columnIndex);
    void SetTargetParamterIndex(int parameterIndex);
    bool IsResolved() const;
    Utf8StringCR GetSourcePropertyPath() const;
    int GetSourceColumnIndex() const;
    int GetTargetParameterIndex() const;
    StatementType GetSourceStatementType () const;
    ECSqlStatus Execute(ECSqlStatementBase& sourceStmt, ECSqlStatementBase& targetStmt, IECSqlBinder::MakeCopy makeCopy);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE