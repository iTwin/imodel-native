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
bool SchemaValidator::ValidateSchemas(SchemaImportContext& ctx, IssueReporter const& issueReporter, bvector<ECN::ECSchemaCP> const& schemas)
    {
    PERFLOG_START("ECDb", "Schema Validation");
    bool valid = true;

    ValidBaseClassesRule baseClassesRule;
    ValidRelationshipRule relRule;

    ValidPropertyRule validPropertyRule;
    for (ECSchemaCP schema : schemas)
        {
        if (schema->GetName().EqualsIAscii(ECSCHEMA_ECDbSystem))
            continue; //skip because it would violate by design some of the property naming rules

        for (ECClassCP ecClass : schema->GetClasses())
            {
            //per class rules
            bool succeeded = baseClassesRule.Validate(ctx, issueReporter, *schema, *ecClass);
            if (!succeeded)
                valid = false;

            succeeded = relRule.Validate(issueReporter, *schema, *ecClass);
            if (!succeeded)
                valid = false;

            //per property rules
            for (ECPropertyCP prop : ecClass->GetProperties(false))
                {
                if (!validPropertyRule.Validate(issueReporter, *ecClass, *prop))
                    valid = false;
                }
            }
        }

    PERFLOG_FINISH("ECDb", "Schema Validation");
    return valid;
    }

//*************************************************************************
//SchemaValidator::ValidBaseClassesRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidBaseClassesRule::Validate(SchemaImportContext const& ctx, IssueReporter const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
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
            if (ctx.GetOptions() == SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues)
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
            if (ctx.GetOptions() == SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues && ecClass.IsEntityClass())
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

    return ValidateConstraint(issueReporter, *relClass, ECRelationshipEnd::ECRelationshipEnd_Source, relClass->GetSource()) && ValidateConstraint(issueReporter, *relClass, ECRelationshipEnd::ECRelationshipEnd_Target, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidRelationshipRule::ValidateConstraint(IssueReporter const& issueReporter, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        issueReporter.Report("The relationship class '%'s is not abstract and therefore constraints must be defined. The %s constraint is empty though.", 
                             relClass.GetFullName(), constraintEnd == ECRelationshipEnd_Source ? "source" : "target");
        return false;
        }

    bool valid = true;
    bset<ECClassCP> duplicateConstraintClasses;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (ClassMap::IsAnyClass(*constraintClass))
            {
            issueReporter.Report("The relationship class '%s' uses the AnyClass constraint. AnyClass is not supported.", relClass.GetFullName());
            valid = false;
            }

        if (duplicateConstraintClasses.find(constraintClass) != duplicateConstraintClasses.end())
            {
            issueReporter.Report(" The relationship class '%s' defines class '%s' more than once in the %s constraint. This is not supported.", 
                                 relClass.GetFullName(), constraintClass->GetFullName(), constraintEnd == ECRelationshipEnd_Source ? "source" : "target");
            valid = false;
            }
        else
            duplicateConstraintClasses.insert(constraintClass);
        }

    return valid;
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

    if (prop.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
        if (navProp->GetRelationshipClass()->HasBaseClasses())
            {
            issueReporter.Report("Invalid navigation property in ECClass '%s': The navigation property '%s' references the relationship class '%s' which has a base class. Navigation properties must always reference the root relationship class though.", 
                                 ecClass.GetFullName(), navProp->GetName().c_str(), navProp->GetRelationshipClass()->GetFullName());
            isValid = false;
            }
        }

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

END_BENTLEY_SQLITE_EC_NAMESPACE
