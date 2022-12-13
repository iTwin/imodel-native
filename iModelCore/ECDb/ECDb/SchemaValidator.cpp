/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
bool SchemaValidator::ValidateSchemas(SchemaImportContext& ctx, IssueDataSource const& issueReporter, bvector<ECN::ECSchemaCP> const& schemas)
    {
    ECDB_PERF_LOG_SCOPE("Schema Validation");

    bool valid = true;

    NoUnitsInEC31SchemaRule noUnitsInEC31SchemaRule;
    ValidBaseClassesRule baseClassesRule;
    ValidRelationshipRule relRule;
    ValidPropertyRule validPropertyRule;
    for (ECSchemaCP schema : schemas)
        {
        bool succeeded = noUnitsInEC31SchemaRule.Validate(ctx, issueReporter, *schema);
        if (!succeeded)
            valid = false;

        if (schema->GetName().EqualsIAscii(ECSCHEMA_ECDbSystem))
            continue; //skip because it would violate by design some of the property naming rules

        for (ECClassCP ecClass : schema->GetClasses())
            {
            //per class rules
            succeeded = baseClassesRule.Validate(ctx, issueReporter, *schema, *ecClass);
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

    return valid;
    }

//*************************************************************************
//SchemaValidator::NoUnitsInEC31SchemaRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::NoUnitsInEC31SchemaRule::Validate(SchemaImportContext const& ctx, IssueDataSource const& issueReporter, ECN::ECSchemaCR schema) const
    {
    if (schema.OriginalECXmlVersionAtLeast(ECVersion::V3_2))
        return true;

    if (schema.GetUnitSystemCount() != 0 || schema.GetPhenomenonCount() != 0 || schema.GetUnitCount() != 0 || schema.GetFormatCount() != 0)
        {
        issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema '%s' defines units, formats, unit systems, or phenomena which requires its original XML version to be at least 3.2. "
                              "It is %" PRIu32 ".%" PRIu32 " though.",
                              schema.GetFullSchemaName().c_str(), schema.GetOriginalECXmlVersionMajor(), schema.GetOriginalECXmlVersionMinor());
        return false;
        }

    return true;
    }

//*************************************************************************
//SchemaValidator::ValidBaseClassesRule
//*************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidBaseClassesRule::Validate(SchemaImportContext const& ctx, IssueDataSource const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
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
            if (Enum::Contains(ctx.GetOptions(), SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues))
                {
                //in legacy mode we log all issues as warning, so do not return on first issue
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: An abstract class must not have a non-abstract base class.",
                             ecClass.GetFullName());
                continue;
                }

            issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECClass '%s' has invalid base class: An abstract class must not have a non-abstract base class.",
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
            if (Enum::Contains(ctx.GetOptions(), SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues))
                {
                //in legacy mode entity class multi-inheritance must be supported, but  not for other class types
                LOG.warningv("ECClass '%s' has invalid base classes which can lead to data corruption. Error: Multi-inheritance is not supported. Use mixins instead.",
                             ecClass.GetFullName());
                continue;
                }

            issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECClass '%s' has multiple base classes. Order is important, only the first base class can be an entity, additional ones must use mixins instead.",
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
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidRelationshipRule::Validate(IssueDataSource const& issueReporter, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    return ValidateConstraint(issueReporter, *relClass, ECRelationshipEnd::ECRelationshipEnd_Source, relClass->GetSource()) && ValidateConstraint(issueReporter, *relClass, ECRelationshipEnd::ECRelationshipEnd_Target, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidRelationshipRule::ValidateConstraint(IssueDataSource const& issueReporter, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    //we cannot yet enforce one class per constraint.
    if (constraintClassCount == 0)
        {
        issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "The relationship class '%'s is not abstract and therefore constraints must be defined. The %s constraint is empty though.",
                             relClass.GetFullName(), constraintEnd == ECRelationshipEnd_Source ? "source" : "target");
        return false;
        }

    bool valid = true;
    bset<ECClassCP> duplicateConstraintClasses;
    for (ECClassCP constraintClass : constraintClasses)
        {
        if (constraintClass->GetSchema().IsStandardSchema() && constraintClass->GetName().EqualsIAscii("AnyClass"))
            {
            issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "The relationship class '%s' uses the AnyClass constraint. AnyClass is not supported.", relClass.GetFullName());
            valid = false;
            }

        if (duplicateConstraintClasses.find(constraintClass) != duplicateConstraintClasses.end())
            {
            issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, " The relationship class '%s' defines class '%s' more than once in the %s constraint. This is not supported.",
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
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::Validate(IssueDataSource const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    bool isValid = true;
    if (!ValidatePropertyName(issueReporter, ecClass, prop))
        isValid = false;

    if (ecClass.IsStructClass())
        {
        if (!ValidatePropertyStructType(issueReporter, *ecClass.GetStructClassCP(), prop))
            isValid = false;
        }

    if (prop.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
        if (navProp->GetRelationshipClass()->HasBaseClasses())
            {
            issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Invalid navigation property in ECClass '%s': The navigation property '%s' references the relationship class '%s' which has a base class. Navigation properties must always reference the root relationship class though.",
                                 ecClass.GetFullName(), navProp->GetName().c_str(), navProp->GetRelationshipClass()->GetFullName());
            isValid = false;
            }
        }

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::ValidatePropertyName(IssueDataSource const& issueReporter, ECN::ECClassCR ecClass, ECN::ECPropertyCR prop) const
    {
    Utf8StringCR propName = prop.GetName();

    bool isCollision = false;
    if (ecClass.GetClassType() != ECClassType::Struct)
        {
        if (propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId) ||
            propName.EqualsIAscii(ECDBSYS_PROPALIAS_Id) ||
            propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
            {
            isCollision = true;
            }
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
        issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Invalid property in ECClass '%s': The property '%s' has a name of an ECSQL system property which is not allowed.", ecClass.GetFullName(), propName.c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaValidator::ValidPropertyRule::ValidatePropertyStructType(IssueDataSource const& issueReporter, ECN::ECStructClassCR owningStruct, ECN::ECPropertyCR prop) const
    {
    ECStructClassCP structType = nullptr;
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

    if (structType->Is(&owningStruct))
        {
        issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Invalid cyclic struct reference: ECStructClass '%s' contains the property '%s' which is of the same type or a derived type than this ECStructClass.",
                             owningStruct.GetFullName(), prop.GetName().c_str());
        return false;
        }

    for (ECPropertyCP structProp : structType->GetProperties(false))
        {
        if (!ValidatePropertyStructType(issueReporter, owningStruct, *structProp))
            return false;
        }

    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
