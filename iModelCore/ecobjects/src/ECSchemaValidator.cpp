/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSchemaValidatorP ECSchemaValidator::GetSingleton()
    {
    static ECSchemaValidatorP ECSchemaValidatorSingleton = nullptr;

    if (nullptr == ECSchemaValidatorSingleton)
        {
        ECSchemaValidatorSingleton = new ECSchemaValidator();

        IECSchemaValidatorPtr baseECValidater = new BaseECValidator();
        ECSchemaValidatorSingleton->AddValidator(baseECValidater);

        IECClassValidatorPtr mixinValidator = new MixinValidator();
        ECSchemaValidatorSingleton->AddClassValidator(mixinValidator);

        IECClassValidatorPtr entityValidator = new EntityValidator();
        ECSchemaValidatorSingleton->AddClassValidator(entityValidator);

        IECClassValidatorPtr relationshipValidator = new RelationshipValidator();
        ECSchemaValidatorSingleton->AddClassValidator(relationshipValidator);
        }    

    return ECSchemaValidatorSingleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AddValidator(IECSchemaValidatorPtr& validator)
    {
    ECSchemaValidatorP schemaValidator = GetSingleton();
    schemaValidator->m_validators.push_back(validator);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AddClassValidator(IECClassValidatorPtr& validator)
{
    ECSchemaValidatorP schemaValidator = GetSingleton();
    schemaValidator->m_classValidators.push_back(validator);
    return ECObjectsStatus::Success;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
bool ECSchemaValidator::Validate(ECSchemaR schema)
    {
    ECSchemaValidatorP schemaValidator = GetSingleton();
    schemaValidator->m_validated = true;

    schemaValidator->ValidateSchema(schema);

    return schemaValidator->m_validated;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::ValidateSchema(ECSchemaR schema)
    {
     for (ECClassCP ecClass : schema.GetClasses())
        {
        for (IECClassValidatorPtr classValidator : GetClassValidators())
            {
            if (!classValidator->CanValidate(*ecClass))
                continue;
            ECObjectsStatus status = classValidator->Validate(*ecClass);
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed class validation of class '%s'", ecClass->GetName().c_str());
                m_validated = false;
                }
            else
                LOG.debugv("Succeeded class validation of class '%s'", ecClass->GetName().c_str());
            }
        }
    for (IECSchemaValidatorPtr validator : GetValidators())
        {
        ECObjectsStatus status = validator->Validate(schema);
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed validation '%s'", schema.GetFullSchemaName().c_str());
            m_validated = false;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus BaseECValidator::Validate(ECSchemaR schema) const
    {
    if (!schema.Validate() || !schema.IsECVersion(ECVersion::Latest))
        {
        LOG.errorv("Failed to validate %s as ECVersion, %s", schema.GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest));
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus MixinValidator::Validate(ECClassCR mixin) const
    {
    if (mixin.GetBaseClasses().size() > 1)
        {
        LOG.errorv("Mixin %s has more than 1 base class", mixin.GetFullName());
        return ECObjectsStatus::Error;
        }
 
    for (ECPropertyP prop : mixin.GetProperties(false)) // Check local properties
        {
        if (prop->GetBaseProperty() != nullptr)
            {
            LOG.errorv("Error at property '%s'. Mixin '%s' overrides an inherited property", prop->GetName().c_str(), mixin.GetFullName());
            return ECObjectsStatus::Error;
            }
        }
    
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus EntityValidator::Validate(ECClassCR entity) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    int numBaseClasses;
    
    for (ECPropertyP prop : entity.GetProperties(false))
        {
        numBaseClasses = 0;
        if (prop->GetBaseProperty() == nullptr)
            continue;
        for (ECClassP baseClass : entity.GetBaseClasses())
            {
            if (baseClass->GetPropertyP(prop->GetName().c_str()) != nullptr)
                numBaseClasses++;
            }
        if (numBaseClasses > 1)
            {
            LOG.errorv("Error at property '%s'. There are %i base classes and entity class '%s' may not inherit a property from more than one base class",
                prop->GetName().c_str(), numBaseClasses, entity.GetFullName());

            status = ECObjectsStatus::Error;
            }
        if (!prop->GetBaseProperty()->GetClass().GetEntityClassCP()->IsMixin())
            continue;
        LOG.errorv("Error at property '%s'. Entity class '%s' overrides a property inherited from mixin class '%s'",
            prop->GetName().c_str(), entity.GetFullName(), prop->GetBaseProperty()->GetClass().GetFullName());

        status = ECObjectsStatus::Error;
        }
    
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus RelationshipValidator::Validate(ECClassCR ecClass) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (nullptr == relClass)
        return status;

    ECRelationshipConstraintCR targetConstraint = relClass->GetTarget();
    ECRelationshipConstraintCR sourceConstraint = relClass->GetSource();
    
    // Validate both target and source.  If one of them fails, the class fails.
    ECObjectsStatus targetStatus, sourceStatus;
    targetStatus = RelationshipValidator::CheckLocalDefinitions(targetConstraint, "Target");
    sourceStatus = RelationshipValidator::CheckLocalDefinitions(sourceConstraint, "Source");

    status = (ECObjectsStatus::Error == targetStatus) || (ECObjectsStatus::Error == sourceStatus) ? ECObjectsStatus::Error : ECObjectsStatus::Success;
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus RelationshipValidator::CheckLocalDefinitions(ECRelationshipConstraintCR constraint, Utf8String constraintType) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    Utf8String className = constraint.GetRelationshipClass().GetFullName();
    if (!constraint.AreConstraintClassesDefinedLocally())
        {
        LOG.errorv("Relationship class '%s' has one or more constraint classes that are not defined locally in %s", className, constraintType);
        status = ECObjectsStatus::Error;
        }

    if (!constraint.IsAbstractConstraintDefinedLocally())
        {
        if (constraint.IsAbstractConstraintDefined())
            LOG.errorv("Relationship class '%s' has an abstract class, '%s', that is not defined locally in %s",
                className, constraint.GetAbstractConstraint()->GetFullName(), constraintType);
        else
            LOG.errorv("Abstract constraint is not defined in '%s'", constraintType);

        status = ECObjectsStatus::Error;
        }

    if (!constraint.IsRoleLabelDefinedLocally())
        {
        LOG.errorv("Relationship class '%s' has a role label, '%s', that is not defined locally in %s", className, constraint.GetRoleLabel(), constraintType);
        status = ECObjectsStatus::Error;
        }

    return status;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
