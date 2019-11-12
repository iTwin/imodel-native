/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************* ECDbPolicy ******************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
Policy& Policy::operator= (Policy const& rhs)
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
Policy& Policy::operator= (Policy&& rhs)
    {
    if (this != &rhs)
        {
        m_isSupported = std::move(rhs.m_isSupported);
        m_notSupportedMessage = std::move(rhs.m_notSupportedMessage);
        }

    return *this;
    }


//********************* PolicyManager ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
Policy PolicyManager::GetPolicy(PolicyAssertion const& assertion)
    {
    switch (assertion.GetType())
        {
            case PolicyAssertion::Type::ClassIsValidInECSql:
                return DoGetPolicy(static_cast<ClassIsValidInECSqlPolicyAssertion const&> (assertion));

            case PolicyAssertion::Type::ECCrudPermission:
                return DoGetPolicy(static_cast<ECCrudPermissionPolicyAssertion const&> (assertion));

            case PolicyAssertion::Type::ECSchemaImportPermission:
                return DoGetPolicy(static_cast<SchemaImportPermissionPolicyAssertion const&> (assertion));

            default:
                return Policy::CreateNotSupported();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
Policy PolicyManager::DoGetPolicy(ClassIsValidInECSqlPolicyAssertion const& assertion)
    {
    ECClassCR ecClass = assertion.GetClassMap().GetClass();
    Utf8StringCR className = ecClass.GetName();
    //generally not supported - regardless of ECSqlType
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it is neither an entity class nor a relationship class.",
                                    className.c_str());
        return Policy::CreateNotSupported(notSupportedMessage);
        }

    if (assertion.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it was not mapped to a table."
                                    " The ECClass might have been marked with 'NotMapped' in the ECSchema or is generally not supported by ECDb."
                                    " In that case, please see the log for details about why the class was not mapped.",
                                    className.c_str());
        return Policy::CreateNotSupported(notSupportedMessage);
        }

    BeAssert(!ecClass.GetSchema().IsStandardSchema() || (!className.Equals("AnyClass") && !className.Equals("InstanceCount")) && "AnyClass or InstanceCount class should already be caught by IsNotMapped check.");

    const ECSqlType ecsqlType = assertion.GetECSqlType();
    if (ecsqlType == ECSqlType::Select)
        return Policy::CreateSupported(); //no more policies for SELECT

    BeAssert(ecsqlType == ECSqlType::Insert || ecsqlType == ECSqlType::Update || ecsqlType == ECSqlType::Delete);

    if (assertion.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("Cannot run ECSQL INSERT, UPDATE, or DELETE on ECRelationshipClass '%s'. Use the respective navigation property to modify it.",
                                    className.c_str());
        return Policy::CreateNotSupported(notSupportedMessage);
        }

    if (ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is a mixin which cannot be modified via ECSQL. Therefore only ECSQL SELECT statements can be used against a mixin class.",
                                    className.c_str());

        return Policy::CreateNotSupported(notSupportedMessage);
        }

    if (assertion.GetClassMap().GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable)
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is mapped to an existing table not owned by ECDb. Therefore only ECSQL SELECT statements can be used against the class.",
                                    className.c_str());

        return Policy::CreateNotSupported(notSupportedMessage);
        }

    if (!assertion.IsPolymorphicClassExpression())
        {
        if (ecClass.GetClassModifier() == ECClassModifier::Abstract)
            {
            Utf8String notSupportedMessage;
            notSupportedMessage.Sprintf("Cannot run non-polymorphic ECSQL INSERT, UPDATE or DELETE on an abstract ECClass '%s'. Abstract classes are not instantiable.",
                                        className.c_str());

            return Policy::CreateNotSupported(notSupportedMessage);
            }

        }
    else
        {
        StorageDescription const& desc = assertion.GetClassMap().GetStorageDescription();
        if (desc.HasMultipleNonVirtualHorizontalPartitions() || assertion.GetClassMap().GetPrimaryTable().GetType() == DbTable::Type::Virtual)
            {
            Utf8String notSupportedMessage;
            notSupportedMessage.Sprintf("Cannot run polymorphic ECSQL INSERT, UPDATE or DELETE on ECClass '%s' because it has subclasses mapped to different tables. Consider mapping the class with strategy 'TablePerHierarchy'.",
                                        className.c_str());

            return Policy::CreateNotSupported(notSupportedMessage);
            }

        if (!desc.HasNonVirtualHorizontalPartitions())
            {
            Utf8String notSupportedMessage;
            notSupportedMessage.Sprintf("Cannot run polymorphic ECSQL INSERT, UPDATE or DELETE on ECClass '%s' because it and all of its subclasses are abstract.",
                                        className.c_str());

            return Policy::CreateNotSupported(notSupportedMessage);
            }
        }

    return Policy::CreateSupported();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2016
//---------------------------------------------------------------------------------------
//static
Policy PolicyManager::DoGetPolicy(ECCrudPermissionPolicyAssertion const& assertion)
    {
    if (!assertion.IsWriteOperation())
        return Policy::CreateSupported(); //reading is always allowed

    ECDbCR ecdb = assertion.GetECDb();
    if (ecdb.IsReadonly())
        return Policy::CreateNotSupported(Utf8String("Cannot modify EC data in a file opened in read-only mode"));

    ECCrudWriteToken const* expectedToken = ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken();
    if (expectedToken != nullptr && expectedToken != assertion.GetToken())
        return Policy::CreateNotSupported(Utf8String("Cannot modify EC data without ECCrudWriteToken."));

    return Policy::CreateSupported();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2016
//---------------------------------------------------------------------------------------
//static
Policy PolicyManager::DoGetPolicy(SchemaImportPermissionPolicyAssertion const& assertion)
    {
    SchemaImportToken const* expectedToken = assertion.GetECDb().GetImpl().GetSettingsManager().GetSchemaImportToken();
    if (expectedToken != nullptr && expectedToken != assertion.GetToken())
        return Policy::CreateNotSupported();

    return Policy::CreateSupported();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
