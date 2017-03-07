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
bool ECSchemaValidator::ValidateSchemas(IssueReporter const& issues, bvector<ECN::ECSchemaCP> const& schemas, bool doNotFailOnLegacyIssues)
    {
    ECSchemaValidationRules rules;
    rules.m_classRules.push_back(std::make_unique<ValidBaseClassesRule>(doNotFailOnLegacyIssues));
    rules.m_classRules.push_back(std::make_unique<ValidRelationshipRule>());

    rules.m_propertyRules.push_back(std::make_unique<NoPropertiesOfSameTypeAsClassRule>());
    rules.m_propertyRules.push_back(std::make_unique<ValidPropertyNameRule>());
    rules.m_propertyRules.push_back(std::make_unique<ValidNavigationPropertyRule>());

    ECSchemaValidationResult result;
    bool valid = true;
    for (ECSchemaCP schema : schemas)
        {
        if (schema->GetName().EqualsIAscii(ECSCHEMA_ECDbSystem))
            continue; //skip because it would violate by design to ValidPropertyNameRule as it defines the ECSQL system props

        bool succeeded = ValidateSchema(result, rules, *schema);
        if (!succeeded)
            valid = false;
        }

    Log(issues, result);
    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateSchema(ECSchemaValidationResult& result, ECSchemaValidationRules const& rules, ECN::ECSchemaCR schema)
    {
    bool valid = true;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        for (std::unique_ptr<IECSchemaValidationRule> const& rule : rules.m_classRules)
            {
            bool succeeded = rule->ValidateSchema(result, schema, *ecClass);
            if (!succeeded)
                valid = false;
            }

        bool succeeded = ValidateClass(result, rules, *ecClass);
        if (!succeeded)
            valid = false;
        }

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateClass(ECSchemaValidationResult& result, ECSchemaValidationRules const& rules, ECN::ECClassCR ecClass)
    {
    bool valid = true;
    for (ECPropertyCP prop : ecClass.GetProperties(true))
        {
        for (std::unique_ptr<IECSchemaValidationRule> const& rule : rules.m_propertyRules)
            {
            bool succeeded = rule->ValidateClass(result, ecClass, *prop);
            if (!succeeded)
                valid = false;
            }
        }

    return valid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
//static
void ECSchemaValidator::Log(IssueReporter const& issues, ECSchemaValidationResult const& result)
    {
    for (std::pair<const IECSchemaValidationRule::Type, std::unique_ptr<IECSchemaValidationRule::IError>> const& error : result.GetErrors())
        {
        error.second->Log(issues);
        }
    }

//**********************************************************************
// ECSchemaValidationResult
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
IECSchemaValidationRule::IError* ECSchemaValidationResult::operator[](IECSchemaValidationRule::Type ruleType)
    {
    auto it = m_errors.find(ruleType);
    if (it == m_errors.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
IECSchemaValidationRule::IError& ECSchemaValidationResult::AddError(std::unique_ptr<IECSchemaValidationRule::IError> error)
    {
    IECSchemaValidationRule::IError* errorP = error.get();
    m_errors[errorP->GetRuleType()] = std::move(error);
    return *errorP;
    }

//**********************************************************************
// ValidBaseClassesRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidBaseClassesRule::_ValidateSchema(ECSchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    IError* errorP = result[GetType()];
    if (errorP == nullptr)
        errorP = &result.AddError(std::make_unique<Error>(m_doNotFailForLegacyIssues));
    
    Error& error = *static_cast<Error*>(errorP);

    ECBaseClassesList const& baseClasses = ecClass.GetBaseClasses();
    if (baseClasses.empty())
        return true;

    const bool isAbstract = ecClass.GetClassModifier() == ECClassModifier::Abstract;
    bool isFirstBaseClass = true;
    for (ECClassCP baseClass : baseClasses)
        {
        if (isAbstract && baseClass->GetClassModifier() == ECClassModifier::None)
            {
            error.AddViolatingClass(ecClass, Error::Kind::AbstractClassHasNonAbstractBaseClass);

            if (m_doNotFailForLegacyIssues)
                continue; //in legacy mode we log all issues as warning, so do not return on first issue

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
            error.AddViolatingClass(ecClass, Error::Kind::MultiInheritance);

            if (m_doNotFailForLegacyIssues)
                continue; //in legacy mode we log all issues as warning, so do not return on first issue

            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
void ValidBaseClassesRule::Error::_Log(IssueReporter const& issues) const
    {
    if (m_violatingClasses.empty() || !issues.IsEnabled())
        return;

    for (auto const& kvPair : m_violatingClasses)
        {
        for (std::pair<ECClassCP, Kind> const& violatingClass : kvPair.second)
            {
            Utf8CP className = violatingClass.first->GetFullName();
            Utf8CP error = nullptr;
            switch (violatingClass.second)
                {
                    case Kind::AbstractClassHasNonAbstractBaseClass:
                        error = "An abstract class must not have a non-abstract base class.";
                        break;

                    case Kind::MultiInheritance:
                        error = "Multi-inheritance is not supported. Use mixins instead.";
                        break;

                    default:
                        BeAssert(false);
                }

            if (m_doNotFailForLegacyIssues)
                {
                if (LOG.isSeverityEnabled(NativeLogging::LOG_WARNING))
                    {
                    LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: %s",
                                 className, error);
                    }
                }
            else
                issues.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: %s",
                              className, error);

            }
        }
    }



//**********************************************************************
// ValidRelationshipRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::_ValidateSchema(ECSchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    IError* errorP = result[GetType()];
    if (errorP == nullptr)
        errorP = &result.AddError(std::make_unique<Error>());

    Error& error = *static_cast<Error*>(errorP);

    return ValidateConstraint(error, *relClass, relClass->GetSource()) && ValidateConstraint(error, *relClass, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::ValidateConstraint(Error& error, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        error.AddInconsistency(relClass, Error::Kind::HasIncompleteConstraintDefinition);
        return false;
        }

    bool valid = true;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (ClassMap::IsAnyClass(*constraintClass))
            {
            error.AddInconsistency(relClass, Error::Kind::HasAnyClassConstraint);
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            error.AddInconsistency(relClass, Error::Kind::HasRelationshipClassAsConstraint, relClassAsConstraint);
            valid = false;
            }
        }

    return valid;
    }


//**********************************************************************
// ValidRelationshipRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
void ValidRelationshipRule::Error::_Log(IssueReporter const& issues) const
    {
    if (m_inconsistencies.empty() || !issues.IsEnabled())
        return;

    for (auto const& kvPair : m_inconsistencies)
        {
        ECSchemaCR schema = *kvPair.first;

        Utf8String str;
        bool isFirstItem = true;
        for (Inconsistency const& inconsistency : kvPair.second)
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

        issues.Report("ECSchema '%s' contains invalid ECRelationshipClasses: %s", schema.GetFullSchemaName().c_str(), str.c_str());
        }
    }


//**********************************************************************
// ValidPropertyNameRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
bool ValidPropertyNameRule::_ValidateClass(ECSchemaValidationResult& result, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const
    {
    Utf8StringCR propName = ecProperty.GetName();

    bool isCollision = false;
    if (propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId) ||
        propName.EqualsIAscii(ECDBSYS_PROPALIAS_Id) ||
        propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
        {
        isCollision = true;
        }

    if (!isCollision && ecClass.GetClassType() == ECClassType::Relationship)
        {
        if (propName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId) ||
            propName.EqualsIAscii(ECDBSYS_PROPALIAS_SourceId) ||
            propName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId) ||
            propName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId) ||
            propName.EqualsIAscii(ECDBSYS_PROPALIAS_TargetId) ||
            propName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
            {
            isCollision = true;
            }
        }

    if (isCollision)
        {
        IError* errorP = result[GetType()];
        if (errorP == nullptr)
            errorP = &result.AddError(std::make_unique<Error>());

        Error& error = *static_cast<Error*>(errorP);
        error.AddInconsistency(ecProperty, Error::Kind::SystemPropertyNamingCollision);
        return false;
        }

    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
void ValidPropertyNameRule::Error::_Log(IssueReporter const& issues) const
    {
    if (m_inconsistencies.empty() || !issues.IsEnabled())
        return;

    for (auto const& kvPair : m_inconsistencies)
        {
        ECClassCR ecClass = *kvPair.first;

        Utf8String str;
        bool isFirstItem = true;
        for (Inconsistency const& inconsistency : kvPair.second)
            {
            if (!isFirstItem)
                str.append("| ");

            switch (inconsistency.m_kind)
                {
                    case Kind::SystemPropertyNamingCollision:
                        str.Sprintf("%s: ECSQL system property name conflict.", inconsistency.m_prop->GetName().c_str());
                        break;

                    default:
                        BeAssert(false);
                        break;
                }

            isFirstItem = false;
            }

        issues.Report("ECClass '%s' contains ECProperties with invalid names: %s", ecClass.GetFullName(), str.c_str());
        }
    }


//**********************************************************************
// ValidNavigationPropertyRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
bool ValidNavigationPropertyRule::_ValidateClass(ECSchemaValidationResult& result, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const
    {
    NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
    if (navProp == nullptr)
        return true;

    if (navProp->IsMultiple())
        {
        IError* errorP = result[GetType()];
        if (errorP == nullptr)
            errorP = &result.AddError(std::make_unique<Error>());

        Error& error = *static_cast<Error*>(errorP);
        error.AddInconsistency(*navProp, Error::Kind::MultiplicityGreaterThanOne);
        return false;
        }

    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
void ValidNavigationPropertyRule::Error::_Log(IssueReporter const& issues) const
    {
    if (m_inconsistencies.empty() || !issues.IsEnabled())
        return;

    for (auto const& kvPair : m_inconsistencies)
        {
        ECClassCR ecClass = *kvPair.first;

        Utf8String str;
        bool isFirstItem = true;
        for (Inconsistency const& inconsistency : kvPair.second)
            {
            if (!isFirstItem)
                str.append("| ");

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

        issues.Report("ECClass '%s' contains invalid NavigationECProperties: %s", ecClass.GetFullName(), str.c_str());
        }
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool NoPropertiesOfSameTypeAsClassRule::_ValidateClass(ECSchemaValidationResult& result, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const
    {
    IError* errorP = result[GetType()];
    if (errorP == nullptr)
        errorP = &result.AddError(std::make_unique<Error>());

    Error& error = *static_cast<Error*>(errorP);

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
        error.AddInvalidProperty(ecProperty);

    return isValid;
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule::Error
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void NoPropertiesOfSameTypeAsClassRule::Error::_Log(IssueReporter const& issues) const
    {
    if (m_invalidProperties.empty() || !issues.IsEnabled())
        return;

    for (auto const& kvPair : m_invalidProperties)
        {
        ECClassCR ecClass = *kvPair.first;

        Utf8String violatingPropsStr;

        bool isFirstProp = true;
        for (ECPropertyCP violatingProp : kvPair.second)
            {
            if (!isFirstProp)
                violatingPropsStr.append(", ");

            violatingPropsStr.append(violatingProp->GetName());
            isFirstProp = false;
            }

        issues.Report("ECClass '%s' contains struct or array ECProperties which are of the same type or a derived type than the ECClass. Conflicting ECProperties: %s.",
                      ecClass.GetFullName(), violatingPropsStr.c_str());
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
