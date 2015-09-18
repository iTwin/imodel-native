/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/EmbeddedECSqlStatement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlStatementBase.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      10/2013
//+===============+===============+===============+===============+===============+======
struct EmbeddedECSqlStatement : public ECSqlStatementBase
    {
private:
    ECSqlStatementBase* m_parentStatement;
    ArrayECPropertyCP m_arrayProperty;
    ECSqlPrepareContext* m_parentPrepareContext;
    ECSqlColumnInfo const* m_parentColumnInfo;

    virtual ECSqlPrepareContext _InitializePrepare(ECDbCR ecdb, Utf8CP ecsql) override;

    ECSqlStatementBase& GetParentStatement () const;
    ArrayECPropertyCP GetArrayProperty () const;
    ECSqlPrepareContext& GetParentPrepareContext () const;
    ECSqlColumnInfo const* GetParentColumnInfo () const;
        
public:
    EmbeddedECSqlStatement () : ECSqlStatementBase() {}
    ~EmbeddedECSqlStatement () {}

    void Initialize (ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCP arrayProperty = nullptr, ECSqlColumnInfo const* parentColumnInfo = nullptr);
    bool HasColumnInfo () const { return m_parentColumnInfo != nullptr; }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE