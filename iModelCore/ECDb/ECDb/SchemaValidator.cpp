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
    std::vector<std::unique_ptr<IClassValidationRule>> classRules;
    classRules.push_back(std::make_unique<ValidBaseClassesRule>(doNotFailOnLegacyIssues));
    classRules.push_back(std::make_unique<ValidRelationshipRule>());
    classRules.push_back(std::make_unique<ValidPropertiesRule>());

    bool valid = true;
    for (ECSchemaCP schema : schemas)
        {
        if (schema->GetName().EqualsIAscii(ECSCHEMA_ECDbSystem))
            continue; //skip because it would violate by design to ValidPropertyNameRule as it defines the ECSQL system props

        for (ECClassCP ecClass : schema->GetClasses())
            {
            for (std::unique_ptr<IClassValidationRule> const& classRule : classRules)
                {
                const bool succeeded = classRule->ValidateClass(issues, *schema, *ecClass);
                if (!succeeded)
                    valid = false;
                }
            }
        }

    return valid;
    }


//**********************************************************************
// ValidBaseClassesRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidBaseClassesRule::_ValidateClass(IssueReporter const& issues, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
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
            if (m_doNotFailForLegacyIssues)
                {
                //in legacy mode we log all issues as warning, so do not return on first issue
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                             ecClass.GetFullName());
                continue; 
                }

            issues.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                              ecClass.GetFullName());
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
            if (m_doNotFailForLegacyIssues && ecClass.IsEntityClass())
                {
                //in legacy mode entity class multi-inheritance must be supported, but  not for other class types
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported. Use mixins instead.",
                             ecClass.GetFullName());
                continue;
                }

            issues.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported.",
                          ecClass.GetFullName());
            return false;
            }
        }

    return true;
    }


//**********************************************************************
// ValidRelationshipRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::_ValidateClass(IssueReporter const& issues, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    return ValidateConstraint(issues, *relClass, relClass->GetSource()) && ValidateConstraint(issues, *relClass, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipRule::ValidateConstraint(IssueReporter const& issues, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        issues.Report("The relationship class '%'s is not abstract and therefore constraints must be defined.", relClass.GetFullName());
        return false;
        }

    bool valid = true;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (ClassMap::IsAnyClass(*constraintClass))
            {
            issues.Report("The relationship class '%s' uses the AnyClass constraint. AnyClass is not supported.", relClass.GetFullName());
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            issues.Report(" The relationship class '%s' has the constraint class '%s' which is a relationship class. This is not supported.", relClass.GetFullName(), relClassAsConstraint->GetFullName());
            valid = false;
            }
        }

    return valid;
    }


//**********************************************************************
// ValidPropertiesRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
bool ValidPropertiesRule::_ValidateClass(IssueReporter const& issues, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    //only iterate local props as every class will be validated separately
    bool isValid = true;

    NavigationPropertyValidationContext navPropCtx(issues, ecClass);
    for (ECPropertyCP prop : ecClass.GetProperties(false))
        {
        if (!ValidatePropertyName(issues, ecClass, *prop))
            isValid = false;

        if (!ValidatePropertyStructType(issues, ecClass, *prop))
            isValid = false;

        if (!ValidateNavigationProperty(navPropCtx, *prop))
            isValid = false;
        }


    if (navPropCtx.HasNavigationProperties())
        {
        if (!ValidateInheritedNavigationProperties(navPropCtx))
            isValid = false;

        navPropCtx.LogIssues();
        }

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
bool ValidPropertiesRule::ValidatePropertyName(IssueReporter const& issues, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    Utf8StringCR propName = prop.GetName();

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
        issues.Report("Invalid property in ECClass '%s': The property '%s' has a name of an ECSQL system property which is not allowed.", ecClass.GetFullName(), propName.c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool ValidPropertiesRule::ValidatePropertyStructType(IssueReporter const& issues, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    ECClassCP structType = nullptr;
    if (prop.GetIsStruct())
        structType = &prop.GetAsStructProperty()->GetType();
    else if (prop.GetIsArray())
        {
        StructArrayECPropertyCP structArrayProp = prop.GetAsStructArrayProperty();
        if (nullptr != structArrayProp)
            structType = &structArrayProp->GetStructElementType();
        }

    if (structType == nullptr)
        return true; //prop is of primitive type or prim array type -> no validation needed

    if (structType->Is(&ecClass))
        {
        issues.Report("ECClass '%s' contains the ECProperty '%s' which is of the same type or a derived type than this ECClass.",
                      ecClass.GetFullName(), prop.GetName().c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2016
//---------------------------------------------------------------------------------------
bool ValidPropertiesRule::ValidateNavigationProperty(NavigationPropertyValidationContext& ctx, ECN::ECPropertyCR prop) const
    {
    NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
    if (navProp == nullptr)
        return true;

    //Multiplicity validation
    if (&navProp->GetClass() == &ctx.m_ecClass && navProp->IsMultiple())
        {
        if (ctx.m_issues.IsEnabled())
            {
            ECRelationshipClassCR relClass = *navProp->GetRelationshipClass();
            ECRelationshipConstraintCR toConstraint = navProp->GetDirection() == ECRelatedInstanceDirection::Forward ? relClass.GetTarget() : relClass.GetSource();

            ctx.m_issues.Report("Invalid navigation property in ECClass '%s': '%s' has a multiplicity of '%s' although the maximum supported multiplicity is 1.",
                                ctx.m_ecClass.GetFullName(), navProp->GetName().c_str(), toConstraint.GetMultiplicity().ToString().c_str());
            }

        return false;
        }

    //Duplicate relationships validation
    ECRelationshipClassCR rootRelClass = ctx.GetRootRelationship(*navProp->GetRelationshipClass());

    bset<NavigationECPropertyCP>& duplicateNavProps = ctx.m_navPropsByRelAndDirection[&rootRelClass][navProp->GetDirection()];
    duplicateNavProps.insert(navProp);
    if (duplicateNavProps.size() > 1)
        {
        ctx.m_hasDuplicates = true;
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
bool ValidPropertiesRule::ValidateInheritedNavigationProperties(NavigationPropertyValidationContext& ctx) const
    {
    bool isValid = true;
    for (ECClassCP baseClass : ctx.m_ecClass.GetBaseClasses())
        {
        //now include inherited nav props to avoid recursion
        for (ECPropertyCP prop : baseClass->GetProperties(true))
            {
            NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
            if (navProp == nullptr)
                continue;

            //Duplicate relationships validation
            ECRelationshipClassCR rootRelClass = ctx.GetRootRelationship(*navProp->GetRelationshipClass());
            bset<NavigationECPropertyCP>& duplicateNavProps = ctx.m_navPropsByRelAndDirection[&rootRelClass][navProp->GetDirection()];
            duplicateNavProps.insert(navProp);
            if (duplicateNavProps.size() > 1)
                {
                ctx.m_hasDuplicates = true;
                isValid = false;
                }
            }
        }

    return isValid;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
void ValidPropertiesRule::NavigationPropertyValidationContext::LogIssues() const
    {
    if (!m_hasDuplicates || !m_issues.IsEnabled())
        return;

    for (auto const& kvPair1 : m_navPropsByRelAndDirection)
        {
        ECRelationshipClassCP rootRelClass = kvPair1.first;
        for (auto const& kvPair2 : kvPair1.second)
            {
            ECRelatedInstanceDirection direction = kvPair2.first;
            bset<NavigationECPropertyCP> const& duplicateNavProps = kvPair2.second;
            if (duplicateNavProps.size() > 1)
                {
                Utf8String violatingNavProps;
                bool isFirstItem = true;
                for (NavigationECPropertyCP navProp : duplicateNavProps)
                    {
                    if (!isFirstItem)
                        violatingNavProps.append(",");

                    violatingNavProps.append(navProp->GetName());
                    isFirstItem = false;
                    }

                m_issues.Report("ECClass '%s' has violating navigation properties: More than one navigation property is defined for the relationship '%s' (or a subclass thereof) with direction '%s': %s",
                                m_ecClass.GetFullName(), rootRelClass->GetFullName(), direction == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward",
                                violatingNavProps.c_str());
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
//static
ECRelationshipClassCR ValidPropertiesRule::NavigationPropertyValidationContext::GetRootRelationship(ECN::ECRelationshipClassCR relClass)
    {
    if (!relClass.HasBaseClasses())
        return relClass;

    //multi-inheritance is not support for relationships (caught by another rule), so we
    //can safely just use the first base class
    return GetRootRelationship(*relClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
