/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceUpdater.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
    //************* InstanceUpdater ****************************************************

    /*---------------------------------------------------------------------------------------
    * @bsimethod                                                    Affan.Khan      04/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    InstanceUpdater::InstanceUpdater (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass)
    : m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecProperty (nullptr) 
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceUpdater::InstanceUpdater (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCP ecProperty)
    : m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecProperty (ecProperty)    
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceUpdater::~InstanceUpdater()
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus InstanceUpdater::Initialize(BeRepositoryBasedIdSequenceR ecInstanceIdSequence)
    {
    m_reInsertInstance = false;

    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    if (classMap == nullptr || classMap->IsUnmapped ())
        return MapStatus::Error;
    DbTableR table = classMap->GetTable();
    if (!m_ecDbMap.GetECDbR().TableExists (table.GetName()))
        return MapStatus::Error;

    if (classMap->ContainsPropertyMapToTable () || classMap->IsRelationshipClassMap ())
        {
        //At the moment we RE-INSERT an ECInstance if it has a column that maps to tables.
        m_reInsertInstance = true;
        InsertStatus status;
        m_inserter = InstanceInserter::Create (status, m_ecDbMap, m_ecClass, ecInstanceIdSequence, nullptr);
        bool deleteDependencies = false;
        m_deleter = InstanceDeleter::Create (m_ecDbMap, m_ecClass, deleteDependencies);
        return m_deleter.IsNull() || m_inserter.IsNull() ? MapStatus::Error : MapStatus::Success;
        }

    SqlUpdate sqlUpdate;
    sqlUpdate.SetTable (table.GetName());

    auto classMap2 = dynamic_cast<ClassMapCP> (classMap);
    BeAssert (classMap2 != nullptr);
    m_nextParameterIndex = 1;
    BentleyStatus s = classMap2->GenerateParameterBindings(m_parameterBindings, m_nextParameterIndex);
    POSTCONDITION (s == SUCCESS, MapStatus::Error);
    FOR_EACH (BindingR binding, m_parameterBindings)
        {
        if (!binding.m_column)
            continue;

        sqlUpdate.AddUpdateColumn (binding.m_column->GetName (), "?");
        m_nextParameterIndex++;
        }

    auto classIdColumn = table.GetClassIdColumn(); 
    if (nullptr != classIdColumn)
        sqlUpdate.AddWhere (classIdColumn->GetName(), SqlPrintfString (" = %lld", classMap->GetClass ().GetId()));

    sqlUpdate.AddWhere (ECDB_COL_ECInstanceId, " = ?");

    if (m_ecClass.GetIsStruct())  // ClassMappingRule: Here, we are assuming that all structs (even embedded ones!) are mapped to separate tables
        {
        sqlUpdate.AddWhere (ECDB_COL_ECPropertyId, " is null");
        sqlUpdate.AddWhere (ECDB_COL_ECArrayIndex, " is null");
        }

    return sqlUpdate.GetSql (m_sqlString) ? MapStatus::Success : MapStatus::Error;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceUpdaterPtr InstanceUpdater::Create (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence)
    {
    InstanceUpdaterPtr updater = new InstanceUpdater (ecDbMap, ecClass);
    MapStatus status = updater->Initialize(ecInstanceIdSequence);
    if (MapStatus::Success != status)
        return nullptr;

    return updater;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceUpdater::IsTopLevelUpdater()
    {
    return m_ecProperty == nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceUpdater::Bind (ECInstanceId ecInstanceId, ECN::IECInstanceR ecInstance)
    {
    m_statement.Reset();
    m_statement.ClearBindings();

    DbResult r = Binding::Bind(m_statement, ecInstance, m_parameterBindings);
    POSTCONDITION (BE_SQLITE_OK == r, r);

    m_statement.BindId (m_nextParameterIndex, ecInstanceId);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceUpdater::Update (ECN::IECInstanceR ecInstance)
    {
    WString instanceIdStr = ecInstance.GetInstanceId ();
    ECInstanceId ecInstanceId;
    if (instanceIdStr.empty () || !ECInstanceIdHelper::FromString (ecInstanceId, instanceIdStr.c_str ()))
        return ERROR;

    if (m_reInsertInstance) //Some columns are mapped to other tables so we will reinsert the instance.
        {
        BentleyStatus deleteStatus = m_deleter->Delete (ecInstanceId, nullptr);
        if (deleteStatus != SUCCESS)
            return ERROR;

        // there is no error if no row is deleted so we need to check following to verify if one or more rows were deleted
        if (m_ecDbMap.GetECDbR().GetModifiedRowCount() == 0)
            return ERROR;

        InsertStatus insertStatus = m_inserter->Insert (ecInstance, ecInstanceId);
        if (insertStatus != INSERT_Success)
            return ERROR;
        }
    else // All columns are map to a single table we can just use UPDATE statement to update all columns
        {
        DbResult result;
        if (!m_statement.IsPrepared())
            {
            result = m_statement.Prepare (m_ecDbMap.GetECDbR (), m_sqlString.c_str ());
            if (BE_SQLITE_OK != result)
                return ERROR;
            }

        result = Bind (ecInstanceId, ecInstance);
        if (BE_SQLITE_OK != result)
            return ERROR;

        result = m_statement.Step();
        if (result != BE_SQLITE_DONE)
            return ERROR;
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
