/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceUpdater.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan      04/2012
+===============+===============+===============+===============+===============+======*/
struct InstanceUpdater : RefCountedBase
    {
private:
    ECDbMapCR                        m_ecDbMap;
    ECN::ECClassCR                   m_ecClass;
    ECN::ECPropertyCP                m_ecProperty;
    InstanceDeleterPtr              m_deleter;
    InstanceInserterPtr             m_inserter;
    Bindings                        m_parameterBindings;
    Utf8String                      m_sqlString;
    bool                            m_reInsertInstance;
    BeSQLite::Statement             m_statement;
    int                             m_nextParameterIndex;
    InstanceUpdater (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass);
    InstanceUpdater (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCP ecProperty);
    bool IsTopLevelUpdater();
    ~InstanceUpdater();
    MapStatus Initialize (BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    BeSQLite::DbResult              Bind (ECInstanceId ecInstanceId, ECN::IECInstanceR ecInstance);

public:
    static InstanceUpdaterPtr Create (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    BentleyStatus Update (ECN::IECInstanceR ecInstance);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
