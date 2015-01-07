/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECPersistence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECPersistence.h"
#include <limits>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPersistence::ECPersistence (IClassMap const& classMap)
    : m_ecdb (classMap.GetECDbMap().GetECDbR()),
                m_ecClass (classMap.GetClass()), m_ecClassId (classMap.GetClass().GetId())
    {
    // TODO: Further refactor this to just keep the class map and pass that around
    // TODO: Further refactor this to keep a const ref to Map - this shouldn't have to modify the map at all. 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DeleteStatus ECPersistence::Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler /*= nullptr*/)
    {
    ECInstanceIdSet ecInstanceIdSet;
    ecInstanceIdSet.insert (ecInstanceId);
    return Delete (nullptr, ecInstanceIdSet, deleteHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
InstanceDeleterP ECPersistence::GetInstanceDeleter()
    {
    if (m_instanceDeleter.IsNull())
        m_instanceDeleter = InstanceDeleter::Create (m_ecdb.GetImplR ().GetECDbMap (), m_ecClass, true);
    return m_instanceDeleter.get();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DeleteStatus ECPersistence::Delete (int32_t* nDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler /*= nullptr*/)
    {
    InstanceDeleterP instanceDeleter = GetInstanceDeleter();
    if (nullptr == instanceDeleter)
        return DELETE_FailedToGenerateSql;

    BentleyStatus status = instanceDeleter->Delete (nDeleted, ecInstanceIdSet, deleteHandler);
    if (status != SUCCESS)
        {
        LogLastSqliteError (m_ecdb);
        return DELETE_SqliteError;
        }

    return DELETE_Success;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId  ECPersistence::GetClassId () const {return m_ecClassId;}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCR  ECPersistence::GetClass() const {return m_ecClass;}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                05/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECPersistence::LogLastSqliteError (BeSQLiteDbR db)
    {
    DbResult result;
    Utf8CP details = db.GetLastError (&result);
    LOG.errorv ("DbResult=%s; %s", Db::InterpretDbResult (result), details);
    }




//************* Binding ****************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Binding::Binding (ECEnablerCR enabler, PropertyMapCR propertyMap, uint32_t propertyIndex, uint16_t componentMask, int sqlIndex, ECDbSqlColumn const* column)
    : m_enabler (enabler), m_propertyMap (propertyMap), m_propertyIndex (propertyIndex), m_componentMask (componentMask),
    m_sqlIndex (sqlIndex), m_column (column)
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Binding::Binding (Binding const & other) :
m_enabler (other.m_enabler),
m_propertyMap (other.m_propertyMap),
m_propertyIndex (other.m_propertyIndex),
m_componentMask (other.m_componentMask),
m_sqlIndex (other.m_sqlIndex),
m_column (other.m_column)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
Binding const& Binding::operator= (Binding const& other)
    {
    memcpy (this, &other, sizeof (Binding));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult Binding::Bind (BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance, Bindings const& parameterBindings)
    {
    for (int iBinding = 0; iBinding < (int) parameterBindings.size ();)
        {
        Binding const& binding = parameterBindings[iBinding];
        // binding.m_propertyMap->Bind will increment iBinding by 1 or more
        DbResult r = binding.m_propertyMap.Bind (iBinding, parameterBindings, statement, ecInstance);
        if (BE_SQLITE_OK != r)
            return r;
        }

    return BE_SQLITE_OK;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
