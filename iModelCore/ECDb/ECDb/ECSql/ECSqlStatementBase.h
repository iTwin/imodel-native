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

    ECSqlStatusContext* m_statusContext;

    virtual ECSqlPrepareContext _InitializePrepare(ECDbCR ecdb, Utf8CP ecsql) = 0;

    ECSqlParseTreePtr ParseECSql (ECDbCR ecdb, Utf8CP ecsql, IClassMap::View classView);

    ECSqlPreparedStatement& CreatePreparedStatement (ECDbCR ecdb, ECSqlParseTreeCR parseTree);

    ECSqlStatus FailIfNotPrepared (Utf8CP errorMessage) const;
    ECSqlStatus FailIfWrongType (ECSqlType expectedType, Utf8CP errorMessage) const;

    void Finalize (bool resetStatus);

protected:
    ECSqlStatementBase ();
    void Initialize (ECSqlStatusContext& statusContext);

    virtual ECSqlStatus _Prepare (ECDbCR ecdb, Utf8CP ecsql);

public:
    virtual ~ECSqlStatementBase ();
    //Public API mirrors
    ECSqlStatus Prepare (ECDbCR ecdb, Utf8CP ecsql);
    bool IsPrepared () const;

    IECSqlBinder& GetBinder (int parameterIndex) const;
    int GetParameterIndex (Utf8CP parameterName) const;
    ECSqlStatus ClearBindings ();

    ECSqlStepStatus Step ();
    ECSqlStepStatus Step (ECInstanceKey& ecInstanceKey);
    ECSqlStatus Reset ();

    int GetColumnCount () const;
    IECSqlValue const& GetValue (int columnIndex) const;

    Utf8String GetLastStatusMessage () const;
    ECSqlStatus GetLastStatus () const;

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

    ECSqlStatusContext& GetStatusContextR () const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
