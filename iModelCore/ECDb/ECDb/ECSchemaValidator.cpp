/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
bool ECSchemaValidator::ValidateSchemas(ECSchemaValidationResult& result, bvector<ECN::ECSchemaP> const& schemas)
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
    validationTasks.push_back(std::unique_ptr<ECSchemaValidationRule>(new ValidRelationshipRule()));

    bool valid = true;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        for (auto& task : validationTasks)
            {
            bool succeeded = task->ValidateSchema(schema, *ecClass);
            if (!succeeded)
                valid = false;
            }

        bool succeeded = ValidateClass(result, *ecClass);
        if (!succeeded)
            valid = false;
        }

    for (auto& task : validationTasks)
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

    bool valid = true;
    for (ECPropertyCP prop : ecClass.GetProperties(true))
        {
        for (auto& task : validationTasks)
            {
            bool succeeded = task->ValidateClass(ecClass, *prop);
            if (!succeeded)
                valid = false;
            }
        }

    for (auto& task : validationTasks)
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
// @bsimethod                                 Krischan.Eberle                    09/2015
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateSchemas(bvector<ECN::ECSchemaP> const& schemas, ECN::ECSchemaCR schema)
    {
    return _ValidateSchemas(schemas, schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    return _ValidateSchema(schema, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    return _ValidateClass(ecClass, ecProperty);
    }

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
// ECSchemaValidationRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String ECSchemaValidationRule::Error::ToString() const
    {
    return _ToString();
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
            structType = structArrayProp->GetStructElementType();
        }

    if (structType == nullptr)
        return true; //prop is of primitive type or prim array type -> no validation needed

    bool isValid = !structType->Is(&ecClass);
    if (!isValid)
        m_error->GetInvalidPropertiesR().push_back(&ecProperty);

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> NoPropertiesOfSameTypeAsClassRule::_GetError() const
    {
    if (m_error->GetInvalidProperties().empty())
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
    if (GetInvalidProperties().empty())
        return "";

    Utf8String violatingPropsStr;

    bool isFirstProp = true;
    for (auto violatingProp : GetInvalidProperties())
        {
        if (!isFirstProp)
            violatingPropsStr.append(", ");

        violatingPropsStr.append(violatingProp->GetName());
        isFirstProp = false;
        }

    Utf8CP strTemplate = "ECClass '%s' contains struct or array ECProperties which are of the same type or a derived type than the ECClass. Conflicting ECProperties: %s.";

    Utf8String str;
    str.Sprintf(strTemplate, m_ecClass.GetFullName(), violatingPropsStr.c_str());

    return str;
    }

//**********************************************************************
// ValidRelationshipRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
ValidRelationshipRule::ValidRelationshipRule()
    : ECSchemaValidationRule(Type::ValidRelationshipRule), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    if (relClass->HasBaseClasses())
        {
        if (relClass->GetBaseClasses().size() > 1)
            {
            m_error->AddInconsistency(*relClass, Error::Kind::MultiInheritance);
            return false;
            }

        /*Need to verify whether we can enforce this rule.
        if (relClass->GetPropertyCount(false) != 0)
            {
            m_error->AddInconsistency(*relClass, Error::Kind::HasAdditionalProperties);
            return false;
            }
            */
        }

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
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        ECClassCR constraintECClass = constraintClass->GetClass();
        if (ClassMap::IsAnyClass(constraintECClass))
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasAnyClassConstraint);
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetClass().GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasRelationshipClassAsConstraint, relClassAsConstraint);
            valid = false;
            }

        if (!constraintClass->GetKeys().empty())
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasKeyProperties);
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

    Utf8String str("Found invalid ECRelationshipClasses: ");
    bool isFirstItem = true;
    for (Inconsistency const& inconsistency : m_inconsistencies)
        {
        if (!isFirstItem)
            str.append(" - ");

        str.append("Relationship ").append(inconsistency.m_relationshipClass->GetFullName()).append(":");

        const Kind kind = inconsistency.m_kind;
        if (Enum::Contains(kind, Kind::MultiInheritance))
            str.append(" It has more than one base class which is not supported for ECRelationshipClasses.");

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

        if (Enum::Contains(kind, Kind::HasKeyProperties))
            str.append(" At least one constraint defines Key properties. Key properties are not supported in EC3 ECSchemas.");

        isFirstItem = false;
        }

    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
