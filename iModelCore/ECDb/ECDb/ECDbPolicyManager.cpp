/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPolicyManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************* ECDbPolicy ******************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy& ECDbPolicy::operator= (ECDbPolicy const& rhs)
    {
    if (this != &rhs)
        {
        m_isSupported = rhs.m_isSupported;
        m_notSupportedMessage = rhs.m_notSupportedMessage;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy& ECDbPolicy::operator= (ECDbPolicy&& rhs)
    {
    if (this != &rhs)
        {
        m_isSupported = std::move(rhs.m_isSupported);
        m_notSupportedMessage = std::move(rhs.m_notSupportedMessage);
        }

    return *this;
    }


//********************* ECDbPolicyManager ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::GetPolicy(ECDbPolicyAssertion const& assertion)
    {
    switch (assertion.GetType())
        {
            case ECDbPolicyAssertion::Type::ClassIsValidInECSql:
                return DoGetPolicy(static_cast<ClassIsValidInECSqlPolicyAssertion const&> (assertion));

            case ECDbPolicyAssertion::Type::ECCrudPermission:
                return DoGetPolicy(static_cast<ECCrudPermissionPolicyAssertion const&> (assertion));

            case ECDbPolicyAssertion::Type::ECSchemaImportPermission:
                return DoGetPolicy(static_cast<ECSchemaImportPermissionPolicyAssertion const&> (assertion));

            default:
                return ECDbPolicy::CreateNotSupported();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::DoGetPolicy(ClassIsValidInECSqlPolicyAssertion const& assertion)
    {
    ECClassCR ecClass = assertion.GetClassMap().GetClass();
    Utf8StringCR className = ecClass.GetName();
    //generally not supported - regardless of ECSqlType
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it is neither an entity class nor a relationship class.",
                                    className.c_str());
        return ECDbPolicy::CreateNotSupported(notSupportedMessage);
        }

    if (assertion.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it was not mapped to a table."
                                    " The ECClass might have been marked with 'NotMapped' in the ECSchema or is generally not supported by ECDb."
                                    " In that case, please see the log for details about why the class was not mapped.",
                                    className.c_str());
        return ECDbPolicy::CreateNotSupported(notSupportedMessage);
        }

    BeAssert(!ecClass.GetSchema().IsStandardSchema() || (!className.Equals("AnyClass") && !className.Equals("InstanceCount")) && "AnyClass or InstanceCount class should already be caught by IsNotMapped check.");

    if (assertion.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable && !assertion.GetClassMap().IsMappedToSingleTable()
        && assertion.GetECSqlType() != ECSqlType::Select)
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECRelationshipClass '%s' is mapped to more than one table on its Foreign Key end. Therefore it cannot be used in ECSQL. Consider exposing the ECRelationshipClass as NavigationECProperty.",
                                    className.c_str());
        return ECDbPolicy::CreateNotSupported(notSupportedMessage);
        }

    //if policy for specific ECSQL type was requested, check that now
    if (assertion.UseECSqlTypeFilter())
        {
        const ECSqlType ecsqlType = assertion.GetECSqlType();
        if (ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Insert || ecsqlType == ECSqlType::Update)
            {
            if (assertion.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
                {
                Utf8String notSupportedMessage;
                notSupportedMessage.Sprintf("ECClass '%s' is mapped to an existing table not owned by ECDb. Therefore only ECSQL SELECT statements can be used against the class.",
                                            className.c_str());

                return ECDbPolicy::CreateNotSupported(notSupportedMessage);
                }

            if (assertion.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
                {
                std::vector<DbTable*> tables = assertion.GetClassMap().GetTables();
                if (tables.empty())
                    {
                    BeAssert(false && "ClassMap.GetTables is not expected to be empty.");
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("Programmer error: ECRelationshipClass '%s' is not mapped to a table.",
                                                className.c_str());
                    return ECDbPolicy::CreateNotSupported(notSupportedMessage);
                    }

                if (tables[0]->GetType() == DbTable::Type::Existing)
                    {
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("ECRelationshipClass '%s' is mapped to an existing table on its Foreign Key end, not owned by ECDb. Therefore only ECSQL SELECT statements can be used against the relationship class.",
                                                className.c_str());
                    return ECDbPolicy::CreateNotSupported(notSupportedMessage);
                    }
                }

            if (ecsqlType == ECSqlType::Insert)
                {
                //Inserting into abstract classes is not possible (by definition of abstractness)
                if (ecClass.GetClassModifier() == ECClassModifier::Abstract)
                    {
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("ECClass '%s' is an abstract class which is not instantiable and therefore cannot be used in an ECSQL INSERT statement.",
                                                className.c_str());

                    return ECDbPolicy::CreateNotSupported(notSupportedMessage);
                    }

                }
            }
        }

    return ECDbPolicy::CreateSupported();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2016
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::DoGetPolicy(ECCrudPermissionPolicyAssertion const& assertion)
    {
    if (!assertion.IsWriteOperation())
        return ECDbPolicy::CreateSupported(); //reading is always allowed

    ECDbCR ecdb = assertion.GetECDb();
    if (ecdb.IsReadonly())
        return ECDbPolicy::CreateNotSupported(Utf8String("Cannot modify EC data in a file opened in read-only mode"));

    ECCrudWriteToken const* expectedToken = ecdb.GetECDbImplR().GetSettings().GetECCrudWriteToken();
    if (expectedToken != nullptr && expectedToken != assertion.GetToken())
        return ECDbPolicy::CreateNotSupported(Utf8String("Cannot modify EC data without ECCrudWriteToken."));

    return ECDbPolicy::CreateSupported();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2016
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::DoGetPolicy(ECSchemaImportPermissionPolicyAssertion const& assertion)
    {
    ECSchemaImportToken const* expectedToken = assertion.GetECDb().GetECDbImplR().GetSettings().GetECSchemaImportToken();
    if (expectedToken != nullptr && expectedToken != assertion.GetToken())
        return ECDbPolicy::CreateNotSupported();

    return ECDbPolicy::CreateSupported();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
