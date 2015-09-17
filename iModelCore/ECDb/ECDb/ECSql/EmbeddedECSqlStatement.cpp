/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/EmbeddedECSqlStatement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//**************** EmbeddedECSqlStatement **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
void EmbeddedECSqlStatement::Initialize (ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCP arrayProperty, ECSqlColumnInfo const* parentColumnInfo)
    {
    m_parentStatement = &parentPrepareContext.GetECSqlStatementR ();
    m_arrayProperty = arrayProperty;
    m_parentPrepareContext = &parentPrepareContext;
    m_parentColumnInfo = parentColumnInfo;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
ECSqlPrepareContext EmbeddedECSqlStatement::_InitializePrepare(ECDbCR ecdb, Utf8CP ecsql)
    {
    if (GetArrayProperty () == nullptr)
        return ECSqlPrepareContext(ecdb, *this, GetParentPrepareContext());
    
    return ECSqlPrepareContext(ecdb, *this, GetParentPrepareContext(), *GetArrayProperty(), GetParentColumnInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
ECSqlStatementBase& EmbeddedECSqlStatement::GetParentStatement () const
    {
    BeAssert (m_parentStatement != nullptr && "must never be nullptr");
    return *m_parentStatement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
ArrayECPropertyCP EmbeddedECSqlStatement::GetArrayProperty () const
    {
    return m_arrayProperty;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
ECSqlPrepareContext& EmbeddedECSqlStatement::GetParentPrepareContext () const
    {
    BeAssert (m_parentPrepareContext != nullptr && "must never be nullptr");
    return *m_parentPrepareContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      02/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfo const* EmbeddedECSqlStatement::GetParentColumnInfo () const
    {
    return m_parentColumnInfo;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE