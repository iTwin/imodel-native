/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPolicyManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

//********************* IsValidInECSqlPolicyAssertion ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
IsValidInECSqlPolicyAssertion IsValidInECSqlPolicyAssertion::s_noECSqlTypeFilterAssertionFlyweight;

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicyAssertion const& IsValidInECSqlPolicyAssertion::Get()
    {
    return s_noECSqlTypeFilterAssertionFlyweight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
IsValidInECSqlPolicyAssertion IsValidInECSqlPolicyAssertion::Get(ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression)
    {
    return IsValidInECSqlPolicyAssertion(ecSqlTypeFilter, isPolymorphicClassExpression);
    }


//********************* ECDbPolicyManager ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::GetClassPolicy(ClassMap const& classMap, ECDbPolicyAssertion const& assertion)
    {
    switch (assertion.GetType())
        {
            case ECDbPolicyAssertion::Type::IsValidInECSql:
                return DoGetClassPolicy(classMap, static_cast<IsValidInECSqlPolicyAssertion const&> (assertion));

            default:
                return ECDbPolicy::CreateNotSupported();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::DoGetClassPolicy(ClassMap const& classMap, IsValidInECSqlPolicyAssertion const& assertion)
    {
    ECClassCR ecClass = classMap.GetClass();
    Utf8StringCR className = ecClass.GetName();
    //generally not supported - regardless of ECSqlType
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it is neither an entity class nor a relationship class.",
                                    className.c_str());
        return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
        }

    if (classMap.GetMapStrategy().IsNotMapped())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf("ECClass '%s' is not supported in ECSQL as it was not mapped to a table."
                                    " The ECClass might have been marked with 'NotMapped' in the ECSchema or is generally not supported by ECDb."
                                    " In that case, please see the log for details about why the class was not mapped.",
                                    className.c_str());
        return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
        }

    BeAssert(!ecClass.GetSchema().IsStandardSchema() || (!className.Equals("AnyClass") && !className.Equals("InstanceCount")) && "AnyClass or InstanceCount class should already be caught by IsNotMapped check.");

    //if policy for specific ECSQL type was requested, check that now
    if (assertion.UseECSqlTypeFilter())
        {
        const ECSqlType ecsqlType = assertion.GetECSqlType();
        if (ecsqlType == ECSqlType::Delete || ecsqlType == ECSqlType::Insert || ecsqlType == ECSqlType::Update)
            {
            if (classMap.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable)
                {
                Utf8String notSupportedMessage;
                notSupportedMessage.Sprintf("ECClass '%s' is mapped to an existing table not owned by ECDb. Therefore only ECSQL SELECT statements can be used against the class.",
                                            className.c_str());

                return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
                }

            if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
                {
                if (!classMap.IsMappedToSingleTable())
                    {
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("ECRelationshipClass '%s' is mapped to more than one table on its Foreign Key end. Therefore only ECSQL SELECT statements can be used against the relationship class.",
                                                className.c_str());
                    return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
                    }

                if (classMap.GetTables().empty())
                    {
                    BeAssert(false && "ClassMap.GetTables is not expected to be empty.");
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("Programmer error: ECRelationshipClass '%s' is not mapped to a table.",
                                                className.c_str());
                    return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
                    }

                if (classMap.GetTables()[0]->GetType() == DbTable::Type::Existing)
                    {
                    Utf8String notSupportedMessage;
                    notSupportedMessage.Sprintf("ECRelationshipClass '%s' is mapped to an existing table on its Foreign Key end, not owned by ECDb. Therefore only ECSQL SELECT statements can be used against the relationship class.",
                                                className.c_str());
                    return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
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

                    return ECDbPolicy::CreateNotSupported(notSupportedMessage.c_str());
                    }

                }
            }
        }

    return ECDbPolicy::CreateSupported();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
