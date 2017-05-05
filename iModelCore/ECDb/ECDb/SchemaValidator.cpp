/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool SchemaValidator::ValidateSchemas(IssueReporter const& issues, bvector<ECN::ECSchemaCP> const& schemas, bool doNotFailOnLegacyIssues)
    {
    std::vector<std::unique_ptr<ISchemaValidationRule>> rules;
    rules.push_back(std::make_unique<ValidBaseClassesRule>(doNotFailOnLegacyIssues));
    rules.push_back(std::make_unique<ValidRelationshipRule>());
    rules.push_back(std::make_unique<NoPropertiesOfSameTypeAsClassRule>());
    rules.push_back(std::make_unique<ValidPropertyNameRule>());
    rules.push_back(std::make_unique<ValidNavigationPropertyRule>());

    SchemaValidationResult result;
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
bool SchemaValidator::ValidateSchema(SchemaValidationResult& result, std::vector<std::unique_ptr<ISchemaValidationRule>> const& rules, ECN::ECSchemaCR schema)
    {
    bool valid = true;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        for (std::unique_ptr<ISchemaValidationRule> const& rule : rules)
            {
            bool succeeded = rule->ValidateSchema(result, schema, *ecClass);
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
void SchemaValidator::Log(IssueReporter const& issues, SchemaValidationResult const& result)
    {
    for (std::pair<const ISchemaValidationRule::Type, std::unique_ptr<ISchemaValidationRule::IError>> const& error : result.GetErrors())
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
ISchemaValidationRule::IError* SchemaValidationResult::operator[](ISchemaValidationRule::Type ruleType)
    {
    auto it = m_errors.find(ruleType);
    if (it == m_errors.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2017
//---------------------------------------------------------------------------------------
ISchemaValidationRule::IError& SchemaValidationResult::AddError(std::unique_ptr<ISchemaValidationRule::IError> error)
    {
    ISchemaValidationRule::IError* errorP = error.get();
    m_errors[errorP->GetRuleType()] = std::move(error);
    return *errorP;
    }

//**********************************************************************
// ValidBaseClassesRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidBaseClassesRule::_ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
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

            if (entityBaseClass != nullptr && m_doNotFailForLegacyIssues)
                continue; //in legacy mode entity class multi-inheritance must be supported, but  not for other class types

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
    if (m_violatingClasses.empty())
        return;

    for (auto const& kvPair : m_violatingClasses)
        {
        for (std::pair<ECClassCP, Kind> const& violatingClass : kvPair.second)
            {
            ECClassCP violatingClassP = violatingClass.first;
            switch (violatingClass.second)
                {
                    case Kind::AbstractClassHasNonAbstractBaseClass:
                    {
                    if (m_doNotFailForLegacyIssues)
                       LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                                      violatingClassP->GetFullName());
                    else
                        issues.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                                      violatingClassP->GetFullName());

                    break;
                    }

                    case Kind::MultiInheritance:
                    {
                    if (!m_doNotFailForLegacyIssues || !violatingClassP->IsEntityClass())
                        issues.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported.",
                                      violatingClassP->GetFullName());
                    else
                        LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported. Use mixins instead.",
                                      violatingClassP->GetFullName());

                    break;
                    }

                    default:
                        BeAssert(false);
                }
            }
        }
    }

//**********************************************************************
// ValidRelationshipRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::_ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
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
bool ValidPropertyNameRule::_ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    //only iterate local props as every class will be validated separately
    bool isValid = true;
    for (ECPropertyCP prop : ecClass.GetProperties(false))
        {
        Utf8StringCR propName = prop->GetName();

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
            error.AddInconsistency(*prop, Error::Kind::SystemPropertyNamingCollision);
            isValid = false;
            }
        }

    return isValid;
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
bool ValidNavigationPropertyRule::_ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    bool isValid = true;
    //only iterate local props as every class will be validated separately
    for (ECPropertyCP prop : ecClass.GetProperties(false))
        {
        NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
        if (navProp == nullptr)
            continue;

        //Multiplicity validation
        if (navProp->IsMultiple())
            {
            GetError(result).AddMultiplicityInconsistency(ecClass, *navProp);
            isValid = false;
            continue;
            }

        //Duplicate relationships validation
        ECRelationshipClassCR rootRelClass = GetRootRelationship(*navProp->GetRelationshipClass());

        bset<NavigationECPropertyCP>& duplicateNavProps = m_navPropsPerRelClass[&rootRelClass][navProp->GetDirection()];
        duplicateNavProps.insert(navProp);
        
        if (duplicateNavProps.size() > 1)
            {
            GetError(result).AddMultipleNavPropsWithSameRelHierarchyInconsistency(m_navPropsPerRelClass);
            isValid = false;
            }
        }

    return isValid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
ValidNavigationPropertyRule::Error& ValidNavigationPropertyRule::GetError(SchemaValidationResult& result) const
    {
    IError* error = result[GetType()];
    if (error == nullptr)
        error = &result.AddError(std::make_unique<Error>());

    return *static_cast<Error*>(error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
//static
ECRelationshipClassCR ValidNavigationPropertyRule::GetRootRelationship(ECN::ECRelationshipClassCR relClass)
    {
    if (!relClass.HasBaseClasses())
        return relClass;

    //multi-inheritance is not support for relationships (caught by another rule), so we
    //can safely just use the first base class
    return GetRootRelationship(*relClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
void ValidNavigationPropertyRule::Error::_Log(IssueReporter const& issues) const
    {
    if (!issues.IsEnabled())
        return;

    for (auto const& kvPair : m_multiplicityGreaterOneInconsistencies)
        {
        ECClassCR ecClass = *kvPair.first;

        Utf8String str;
        bool isFirstItem = true;
        for (NavigationECPropertyCP navProp : kvPair.second)
            {
            if (!isFirstItem)
                str.append("| ");

            ECRelationshipClassCR relClass = *navProp->GetRelationshipClass();
            ECRelationshipConstraintCR toConstraint = navProp->GetDirection() == ECRelatedInstanceDirection::Forward ? relClass.GetTarget() : relClass.GetSource();
            str.append("navigation property '").append(navProp->GetName());
            str.append("' has a multiplicity of '").append(toConstraint.GetMultiplicity().ToString().c_str());
            str.append("ECDb only supports navigation property with a maximum multiplicity of 1.");
            isFirstItem = false;
            }

        issues.Report("ECClass '%s' contains invalid NavigationECProperties: %s", ecClass.GetFullName(), str.c_str());
        }

    if (m_multipleNavPropsWithSameRelHierarchyInconsistency == nullptr)
        return;

    for (auto const& kvPair : *m_multipleNavPropsWithSameRelHierarchyInconsistency)
        {
        ECRelationshipClassCP rootRel = kvPair.first;
        for (auto const& innerKvPair : kvPair.second)
            {
            ECRelatedInstanceDirection direction = innerKvPair.first;

            Utf8String msg;
            msg.Sprintf("More than one navigation property is defined on the relationship '%s' or a subclass thereof with direction '%s': ",
                        rootRel->GetFullName(), direction == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward");

            bool isFirstProp = true;
            for (NavigationECPropertyCP navProp : innerKvPair.second)
                {
                if (!isFirstProp)
                    msg.append(", ");

                msg.append(navProp->GetClass().GetFullName()).append(".").append(navProp->GetName());
                }

            issues.Report(msg.c_str());
            }

        }
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool NoPropertiesOfSameTypeAsClassRule::_ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    bool isValid = true;
    //only iterate local props as every class will be validated separately
    for (ECPropertyCP prop : ecClass.GetProperties(false))
        {
        ECClassCP structType = nullptr;
        if (prop->GetIsStruct())
            structType = &prop->GetAsStructProperty()->GetType();
        else if (prop->GetIsArray())
            {
            auto structArrayProp = prop->GetAsStructArrayProperty();
            if (nullptr != structArrayProp)
                structType = &structArrayProp->GetStructElementType();
            }

        if (structType == nullptr)
            continue; //prop is of primitive type or prim array type -> no validation needed

        if (structType->Is(&ecClass))
            {
            IError* errorP = result[GetType()];
            if (errorP == nullptr)
                errorP = &result.AddError(std::make_unique<Error>());

            Error& error = *static_cast<Error*>(errorP);
            error.AddInvalidProperty(*prop);
            isValid = false;
            }
        }

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
