/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSchemaValidator.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateSchemas(ECSchemaValidationResult& result, bvector<ECN::ECSchemaCP> const& schemas)
    {
    bool valid = true;
    for (ECSchemaCP schema : schemas)
        {
        bool succeeded = ValidateSchema(result, *schema);
        if (!succeeded)
            valid = false;
        }

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateSchema(ECSchemaValidationResult& result, ECN::ECSchemaCR schema)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back(std::unique_ptr<ECSchemaValidationRule>(new ValidBaseClassesRule(schema)));
    validationTasks.push_back(std::unique_ptr<ECSchemaValidationRule>(new ValidRelationshipRule(schema)));

    bool valid = true;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        for (std::unique_ptr<ECSchemaValidationRule> const& task : validationTasks)
            {
            bool succeeded = task->ValidateSchema(schema, *ecClass);
            if (!succeeded)
                valid = false;
            }

        bool succeeded = ValidateClass(result, *ecClass);
        if (!succeeded)
            valid = false;
        }

    for (std::unique_ptr<ECSchemaValidationRule> const& task : validationTasks)
        {
        task->AddErrorToResult(result);
        }

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateClass(ECSchemaValidationResult& result, ECN::ECClassCR ecClass)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back(std::unique_ptr<ECSchemaValidationRule>(new NoPropertiesOfSameTypeAsClassRule(ecClass)));
    validationTasks.push_back(std::unique_ptr<ECSchemaValidationRule>(new ValidNavigationPropertyRule(ecClass)));

    bool valid = true;
    for (ECPropertyCP prop : ecClass.GetProperties(true))
        {
        for (std::unique_ptr<ECSchemaValidationRule>& task : validationTasks)
            {
            bool succeeded = task->ValidateClass(ecClass, *prop);
            if (!succeeded)
                valid = false;
            }
        }

    for (std::unique_ptr<ECSchemaValidationRule>& task : validationTasks)
        {
        task->AddErrorToResult(result);
        }

    return valid;
    }


//**********************************************************************
// ECSchemaValidationResult
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationResult::AddError(std::unique_ptr<ECSchemaValidationRule::Error> error)
    {
    m_errors.push_back(std::move(error));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationResult::ToString(std::vector<Utf8String>& errorMessages) const
    {
    for (auto& error : m_errors)
        {
        errorMessages.push_back(error->ToString());
        }
    }


//**********************************************************************
// ECSchemaValidationRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationRule::AddErrorToResult(ECSchemaValidationResult& result) const
    {
    auto error = _GetError();
    if (error != nullptr)
        result.AddError(std::move(error));
    }


//**********************************************************************
// NoMultiInheritanceRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
ValidBaseClassesRule::ValidBaseClassesRule(ECN::ECSchemaCR schema)
    : ECSchemaValidationRule(Type::ValidBaseClasses), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType(), schema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidBaseClassesRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    ECBaseClassesList const& baseClasses = ecClass.GetBaseClasses();
    if (baseClasses.empty())
        return true;

    const bool isAbstract = ecClass.GetClassModifier() == ECClassModifier::Abstract;
    bool isFirstBaseClass = true;
    for (ECClassCP baseClass : baseClasses)
        {
        if (isAbstract && baseClass->GetClassModifier() == ECClassModifier::None)
            {
            m_error->AddViolatingClass(ecClass, Error::Kind::AbstractClassHasNonAbstractBaseClass);
            return false;
            }


        if (isFirstBaseClass)
            {
            isFirstBaseClass = false;
            continue;
            }

        ECEntityClassCP entityBaseClass = baseClass->GetEntityClassCP();
        if (entityBaseClass == nullptr || !entityBaseClass->IsMixin())
            {
            m_error->AddViolatingClass(ecClass, Error::Kind::MultiInheritance);
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> ValidBaseClassesRule::_GetError() const
    {
    if (!m_error->HasErrors())
        return nullptr;

    return std::move(m_error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
Utf8String ValidBaseClassesRule::Error::_ToString() const
    {
    if (!HasErrors())
        return "";

    Utf8String violatingClassesStr;
    for (std::pair<ECClassCP, Kind> const& violatingClass : m_violatingClasses)
        {
        violatingClassesStr.append(violatingClass.first->GetName()).append(": ");
        
        switch (violatingClass.second)
            {
                case Kind::AbstractClassHasNonAbstractBaseClass:
                    violatingClassesStr.append("An abstract class must not have a non-abstract base class.");
                    break;

                case Kind::MultiInheritance:
                    violatingClassesStr.append("Multi-inheritance is not supported. Use mixins instead.");
                    break;

                default:
                    BeAssert(false);
            }
        }

    Utf8String str;
    str.Sprintf("ECSchema '%s' contains ECClasses with invalid base classes. Violating ECClasses: %s",
            m_ecSchema.GetFullSchemaName().c_str(), violatingClassesStr.c_str());

    return str;
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
NoPropertiesOfSameTypeAsClassRule::NoPropertiesOfSameTypeAsClassRule(ECClassCR ecClass)
    : ECSchemaValidationRule(Type::NoPropertiesOfSameTypeAsClass), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType(), ecClass));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool NoPropertiesOfSameTypeAsClassRule::_ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    ECClassCP structType = nullptr;
    if (ecProperty.GetIsStruct())
        structType = &ecProperty.GetAsStructProperty()->GetType();
    else if (ecProperty.GetIsArray())
        {
        auto structArrayProp = ecProperty.GetAsStructArrayProperty();
        if (nullptr != structArrayProp)
            structType = &structArrayProp->GetStructElementType();
        }

    if (structType == nullptr)
        return true; //prop is of primitive type or prim array type -> no validation needed

    bool isValid = !structType->Is(&ecClass);
    if (!isValid)
        m_error->AddInvalidProperty(ecProperty);

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> NoPropertiesOfSameTypeAsClassRule::_GetError() const
    {
    if (!m_error->HasErrors())
        return nullptr;

    return std::move(m_error);
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule::Error
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String NoPropertiesOfSameTypeAsClassRule::Error::_ToString() const
    {
    if (!HasErrors())
        return "";

    Utf8String violatingPropsStr;

    bool isFirstProp = true;
    for (ECPropertyCP violatingProp : m_invalidProperties)
        {
        if (!isFirstProp)
            violatingPropsStr.append(", ");

        violatingPropsStr.append(violatingProp->GetName());
        isFirstProp = false;
        }

    Utf8String str;
    str.Sprintf("ECClass '%s' contains struct or array ECProperties which are of the same type or a derived type than the ECClass. Conflicting ECProperties: %s.",
                m_ecClass.GetFullName(), violatingPropsStr.c_str());

    return str;
    }

//**********************************************************************
// ValidRelationshipRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
ValidRelationshipRule::ValidRelationshipRule(ECN::ECSchemaCR schema)
    : ECSchemaValidationRule(Type::ValidRelationshipClass), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType(), schema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    return ValidateConstraint(*relClass, relClass->GetSource()) && ValidateConstraint(*relClass, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::ValidateConstraint(ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        m_error->AddInconsistency(relClass, Error::Kind::HasIncompleteConstraintDefinition);
        return false;
        }

    bool valid = true;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (ClassMap::IsAnyClass(*constraintClass))
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasAnyClassConstraint);
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasRelationshipClassAsConstraint, relClassAsConstraint);
            valid = false;
            }
        }

    return valid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> ValidRelationshipRule::_GetError() const
    {
    if (!m_error->HasInconsistencies())
        return nullptr;

    return std::move(m_error);
    }


//**********************************************************************
// ValidRelationshipRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
Utf8String ValidRelationshipRule::Error::_ToString() const
    {
    if (!HasInconsistencies())
        return "";

    Utf8String str("ECSchema '");
    str.append(m_ecSchema.GetFullSchemaName()).append("' contains invalid ECRelationshipClasses : ");
    bool isFirstItem = true;
    for (Inconsistency const& inconsistency : m_inconsistencies)
        {
        if (!isFirstItem)
            str.append(" - ");

        str.append("Relationship ").append(inconsistency.m_relationshipClass->GetFullName()).append(":");

        const Kind kind = inconsistency.m_kind;
        if (Enum::Contains(kind, Kind::HasAdditionalProperties))
            str.append(" It has a base class, and it defines ECProperties. This is only allowed for ECRelationshipClasses which don't have any base classes.");

        if (Enum::Contains(kind, Kind::HasAnyClassConstraint))
            str.append(" AnyClass must not be used as constraint.");

        if (Enum::Contains(kind, Kind::HasRelationshipClassAsConstraint))
            {
            BeAssert(inconsistency.m_relationshipClassAsConstraintClass != nullptr);
            str.append(" The relationship class ").append(inconsistency.m_relationshipClassAsConstraintClass->GetFullName()).append(" is specified as constraint class which is not supported.");
            }

        if (Enum::Contains(kind, Kind::HasIncompleteConstraintDefinition))
            str.append(" The relationship class is not abstract and therefore constraints must be defined.");

        isFirstItem = false;
        }

    return str;
    }

//**********************************************************************
// ValidRelationshipRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
ValidNavigationPropertyRule::ValidNavigationPropertyRule(ECClassCR ecClass)
    : ECSchemaValidationRule(Type::ValidNavigationProperty), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType(), ecClass));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
bool ValidNavigationPropertyRule::_ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
    if (navProp == nullptr)
        return true;

    if (navProp->IsMultiple())
        {
        m_error->AddInconsistency(*navProp, Error::Kind::MultiplicityGreaterThanOne);
        return false;
        }

    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
Utf8String ValidNavigationPropertyRule::Error::_ToString() const
    {
    if (!HasInconsistencies())
        return "";

    Utf8String str("ECClass '");
    str.append(m_ecClass->GetFullName()).append("' contains invalid NavigationECProperties: ");

    bool isFirstItem = true;
    for (Inconsistency const& inconsistency : m_inconsistencies)
        {
        if (!isFirstItem)
            str.append(" - ");

        const Kind kind = inconsistency.m_kind;
        if (Enum::Contains(kind, Kind::MultiplicityGreaterThanOne))
            {
            NavigationECProperty const& navProp = *inconsistency.m_navProp;
            ECRelationshipClassCR relClass = *navProp.GetRelationshipClass();
            ECRelationshipConstraintCR toConstraint = navProp.GetDirection() == ECRelatedInstanceDirection::Forward ? relClass.GetTarget() : relClass.GetSource();
            str.append("NavigationECProperty '").append(navProp.GetName());
            str.append("' has a multiplicity of '").append(toConstraint.GetMultiplicity().ToString().c_str());
            str.append("ECDb only supports NavigationECProperties with a maximum multiplicity of 1.");
            }

        isFirstItem = false;
        }

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> ValidNavigationPropertyRule::_GetError() const
    {
    if (!m_error->HasInconsistencies())
        return nullptr;

    return std::move(m_error);
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
