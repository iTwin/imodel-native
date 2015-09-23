/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementBase.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    virtual ECSqlPrepareContext _InitializePrepare(ECDbCR ecdb, Utf8CP ecsql) = 0;

    ECSqlPreparedStatement& CreatePreparedStatement (ECDbCR ecdb, ECSqlParseTreeCR parseTree);

    ECSqlStatus FailIfNotPrepared (Utf8CP errorMessage) const;
    ECSqlStatus FailIfWrongType (ECSqlType expectedType, Utf8CP errorMessage) const;

protected:
    ECSqlStatementBase () : m_preparedStatement(nullptr) {}

    virtual ECSqlStatus _Prepare (ECDbCR ecdb, Utf8CP ecsql);

public:
    virtual ~ECSqlStatementBase () {}
    //Public API mirrors
    ECSqlStatus Prepare (ECDbCR ecdb, Utf8CP ecsql);
    bool IsPrepared () const;

    IECSqlBinder& GetBinder (int parameterIndex) const;
    int GetParameterIndex (Utf8CP parameterName) const;
    ECSqlStatus ClearBindings ();

    DbResult Step ();
    DbResult Step (ECInstanceKey& ecInstanceKey);
    ECSqlStatus Reset ();

    int GetColumnCount () const;
    IECSqlValue const& GetValue (int columnIndex) const;

    Utf8CP GetECSql () const;
    Utf8CP GetNativeSql () const;
    ECDbCP GetECDb () const;

    void Finalize ();

    // Helpers
    ECSqlPreparedStatement* GetPreparedStatementP () const { return m_preparedStatement.get (); }
    
    template <class TECSqlPreparedStatement>
    TECSqlPreparedStatement* GetPreparedStatementP () const 
        { 
        BeAssert (dynamic_cast<TECSqlPreparedStatement*> (GetPreparedStatementP ()) != nullptr);
        return static_cast<TECSqlPreparedStatement*> (GetPreparedStatementP ());
        }

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
