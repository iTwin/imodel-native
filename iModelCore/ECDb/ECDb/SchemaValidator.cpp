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
    PERFLOG_START("ECDb", "SchemaValidation");
    bool valid = true;

    ValidBaseClassesRule baseClassesRule;
    ValidRelationshipRule relRule;
    RelationshipHasNavigationPropertyRule relsHaveNavPropsRule;
    RelationshipHasNavigationPropertyRule::Context relsHaveNavPropsRuleCtx;

    ValidPropertyRule validPropertyRule;
    ClassHasNoDuplicateNavigationPropertiesRule classHasNoDupNavPropRule;
    for (ECSchemaCP schema : schemas)
        {
        if (schema->GetName().EqualsIAscii(ECSCHEMA_ECDbSystem))
            continue; //skip because it would violate by design to ValidPropertyNameRule as it defines the ECSQL system props

        for (ECClassCP ecClass : schema->GetClasses())
            {
            //per class rules
            bool succeeded = baseClassesRule.Validate(issues, *schema, *ecClass, doNotFailOnLegacyIssues);
            if (!succeeded)
                valid = false;

            succeeded = relRule.Validate(issues, *schema, *ecClass);
            if (!succeeded)
                valid = false;

            succeeded = relsHaveNavPropsRule.Validate(relsHaveNavPropsRuleCtx, issues, *schema, *ecClass);
            if (!succeeded)
                valid = false;

            ClassHasNoDuplicateNavigationPropertiesRule::Context classHasNoDupNavPropRuleCtx(*ecClass);
            
            //per property rules
            for (ECPropertyCP prop : ecClass->GetProperties(false))
                {
                if (!validPropertyRule.Validate(issues, *ecClass, *prop))
                    valid = false;

                if (!classHasNoDupNavPropRule.Validate(classHasNoDupNavPropRuleCtx, issues, *prop))
                    valid = false;

                if (!relsHaveNavPropsRule.Validate(relsHaveNavPropsRuleCtx, issues, *ecClass, *prop))
                    valid = false;
                }

            if (!classHasNoDupNavPropRule.PostProcessValidation(classHasNoDupNavPropRuleCtx, issues))
                valid = false;
            }
        }

    if (!relsHaveNavPropsRule.PostProcessValidation(relsHaveNavPropsRuleCtx, issues))
        valid = false;

    PERFLOG_FINISH("ECDb", "SchemaValidation");

    return valid;
    }

//*************************************************************************
//SchemaValidator::ValidBaseClassesRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidBaseClassesRule::Validate(IssueReporter const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass, bool doNotFailForLegacyIssues) const
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
            if (doNotFailForLegacyIssues)
                {
                //in legacy mode we log all issues as warning, so do not return on first issue
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                             ecClass.GetFullName());
                continue; 
                }

            issueReporter.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
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
            if (doNotFailForLegacyIssues && ecClass.IsEntityClass())
                {
                //in legacy mode entity class multi-inheritance must be supported, but  not for other class types
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported. Use mixins instead.",
                             ecClass.GetFullName());
                continue;
                }

            issueReporter.Report("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported.",
                          ecClass.GetFullName());
            return false;
            }
        }

    return true;
    }

//*************************************************************************
//SchemaValidator::ValidRelationshipRule
//*************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidRelationshipRule::Validate(IssueReporter const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    return ValidateConstraint(issueReporter, *relClass, relClass->GetSource()) && ValidateConstraint(issueReporter, *relClass, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidRelationshipRule::ValidateConstraint(IssueReporter const& issueReporter, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        issueReporter.Report("The relationship class '%'s is not abstract and therefore constraints must be defined.", relClass.GetFullName());
        return false;
        }

    bool valid = true;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (ClassMap::IsAnyClass(*constraintClass))
            {
            issueReporter.Report("The relationship class '%s' uses the AnyClass constraint. AnyClass is not supported.", relClass.GetFullName());
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            issueReporter.Report(" The relationship class '%s' has the constraint class '%s' which is a relationship class. This is not supported.", relClass.GetFullName(), relClassAsConstraint->GetFullName());
            valid = false;
            }
        }

    return valid;
    }


//*************************************************************************
//SchemaValidator::RelationshipHasNavigationPropertyRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
bool SchemaValidator::RelationshipHasNavigationPropertyRule::Validate(Context& ctx, IssueReporter const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    if (!ecClass.IsRelationshipClass())
        return true;

    //Rule: End table root relationships must have a nav prop
    ECRelationshipClassCR relClass = *ecClass.GetRelationshipClassCP();
    if (relClass.HasBaseClasses() || RelationshipMappingInfo::RequiresLinkTableMapping(relClass))
        return true;
    
    ECRelationshipEnd fkEnd;
    if (SUCCESS != RelationshipMappingInfo::TryDetermineFkEnd(fkEnd, relClass, issueReporter))
        return false;

    ctx.AddCandidateRelationship(relClass, fkEnd);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
bool SchemaValidator::RelationshipHasNavigationPropertyRule::Validate(Context& ctx, IssueReporter const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    if (!prop.GetIsNavigation())
        return true;

    BeAssert(prop.GetAsNavigationProperty()->GetRelationshipClass() != nullptr);
    ctx.CacheRelationshipWithNavProp(*prop.GetAsNavigationProperty());
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
bool SchemaValidator::RelationshipHasNavigationPropertyRule::PostProcessValidation(Context& ctx, IssueReporter const& issueReporter) const
    {
    bool isValid = true;
    for (bpair<ECRelatedInstanceDirection, bset<ECRelationshipClassCP>> const& kvPair : ctx.GetCandidateRelationships())
        {
        const ECRelatedInstanceDirection expectedNavPropDirection = kvPair.first;
        bset<ECRelationshipClassCP> const& navPropRelationships = ctx.GetNavPropRelationships(expectedNavPropDirection);
        //iterate every candidate relationship and test whether it exists in the list of nav prop relationships. If not, it is a schema failure.
        for (ECRelationshipClassCP candidateRel : kvPair.second)
            {
            if (navPropRelationships.find(candidateRel) == navPropRelationships.end())
                {
                issueReporter.Report("Invalid relationship class '%s'. A navigation property must be defined on the %s constraint class.", candidateRel->GetFullName(),
                                     expectedNavPropDirection == ECRelatedInstanceDirection::Backward ? "target" : "source");
                isValid = false;
                }
            }
        }

    return isValid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
void SchemaValidator::RelationshipHasNavigationPropertyRule::Context::CacheRelationshipWithNavProp(ECN::NavigationECPropertyCR navProp)
    {
    ECRelationshipClassCP navPropRelClass = navProp.GetRelationshipClass();
    BeAssert(navPropRelClass != nullptr);
    if (navProp.GetDirection() == ECRelatedInstanceDirection::Forward)
        m_navPropForwardRelationships.insert(navPropRelClass);
    else
        m_navPropBackwardRelationships.insert(navPropRelClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
void SchemaValidator::RelationshipHasNavigationPropertyRule::Context::AddCandidateRelationship(ECN::ECRelationshipClassCR rel, ECN::ECRelationshipEnd fkEnd)
    {
    const ECRelatedInstanceDirection expectedNavPropDirection = fkEnd == ECRelationshipEnd::ECRelationshipEnd_Target ? ECRelatedInstanceDirection::Backward : ECRelatedInstanceDirection::Forward;
    m_candidateRels[expectedNavPropDirection].insert(&rel);
    }


//*************************************************************************
//SchemaValidator::ValidPropertyRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::Validate(IssueReporter const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    bool isValid = true;
    if (!ValidatePropertyName(issueReporter, ecClass, prop))
        isValid = false;

    if (!ValidatePropertyStructType(issueReporter, ecClass, prop))
        isValid = false;

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::ValidatePropertyName(IssueReporter const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
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
        issueReporter.Report("Invalid property in ECClass '%s': The property '%s' has a name of an ECSQL system property which is not allowed.", ecClass.GetFullName(), propName.c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::ValidatePropertyStructType(IssueReporter const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
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
        issueReporter.Report("ECClass '%s' contains the ECProperty '%s' which is of the same type or a derived type than this ECClass.",
                             ecClass.GetFullName(), prop.GetName().c_str());
        return false;
        }

    return true;
    }

//*************************************************************************
//SchemaValidator::ClassHasNoDuplicateNavigationPropertiesRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ClassHasNoDuplicateNavigationPropertiesRule::Validate(Context& ctx, IssueReporter const& issueReporter, ECN::ECPropertyCR prop) const
    {
    NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
    if (navProp == nullptr)
        return true;

    ECRelationshipClassCR rootRelClass = GetRootRelationship(*navProp->GetRelationshipClass());

    bset<NavigationECPropertyCP>& duplicateNavProps = ctx.m_navPropsByRelAndDirection[&rootRelClass][navProp->GetDirection()];
    duplicateNavProps.insert(navProp);
    if (duplicateNavProps.size() > 1)
        {
        ctx.m_hasDuplicates = true;
        return false;
        }

    return true;
    }

//-------------------------------------------------------------------------------------- -
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ClassHasNoDuplicateNavigationPropertiesRule::PostProcessValidation(Context& ctx, IssueReporter const& issueReporter) const
    {
    bool isValid = true;

    if (ctx.HasNavigationProperties())
        {
        for (ECClassCP baseClass : ctx.m_ecClass.GetBaseClasses())
            {
            //now include inherited nav props to avoid recursion
            for (ECPropertyCP prop : baseClass->GetProperties(true))
                {
                NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
                if (navProp == nullptr)
                    continue;

                //Duplicate relationships validation
                ECRelationshipClassCR rootRelClass = GetRootRelationship(*navProp->GetRelationshipClass());
                bset<NavigationECPropertyCP>& duplicateNavProps = ctx.m_navPropsByRelAndDirection[&rootRelClass][navProp->GetDirection()];
                duplicateNavProps.insert(navProp);
                if (duplicateNavProps.size() > 1)
                    {
                    ctx.m_hasDuplicates = true;
                    isValid = false;
                    }
                }
            }
        }

    LogIssues(ctx, issueReporter);
    return isValid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
void SchemaValidator::ClassHasNoDuplicateNavigationPropertiesRule::LogIssues(Context const& ctx, IssueReporter const& issueReporter) const
    {
    if (!ctx.m_hasDuplicates || !issueReporter.IsEnabled())
        return;

    for (auto const& kvPair1 : ctx.m_navPropsByRelAndDirection)
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

                issueReporter.Report("ECClass '%s' has violating navigation properties: More than one navigation property is defined for the relationship '%s' (or a subclass thereof) with direction '%s': %s",
                                     ctx.m_ecClass.GetFullName(), rootRelClass->GetFullName(), direction == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward",
                                violatingNavProps.c_str());
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2017
//---------------------------------------------------------------------------------------
//static
ECRelationshipClassCR SchemaValidator::ClassHasNoDuplicateNavigationPropertiesRule::GetRootRelationship(ECN::ECRelationshipClassCR relClass)
    {
    if (!relClass.HasBaseClasses())
        return relClass;

    //multi-inheritance is not support for relationships (caught by another rule), so we
    //can safely just use the first base class
    return GetRootRelationship(*relClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
