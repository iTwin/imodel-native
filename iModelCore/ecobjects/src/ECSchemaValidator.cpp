/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#define BISCORE_CLASS_IParentElement        "BisCore:IParentElement"
#define BISCORE_CLASS_ISubModeledElement    "BisCore:ISubModeledElement"
#define ElementMultiAspect                  "ElementMultiAspect"
#define ElementUniqueAspect                 "ElementUniqueAspect"
#define ElementOwnsUniqueAspect             "ElementOwnsUniqueAspect"
#define ElementOwnsMultiAspects             "ElementOwnsMultiAspects"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

Utf8CP oldStandardSchemaNames[] =
    {
    "Bentley_Standard_CustomAttributes",
    "Bentley_Standard_Classes",
    "Bentley_ECSchemaMap",
    "EditorCustomAttributes",
    "Bentley_Common_Classes",
    "Dimension_Schema",
    "iip_mdb_customAttributes",
    "KindOfQuantity_Schema",
    "rdl_customAttributes",
    "SIUnitSystemDefaults",
    "Unit_Attributes",
    "Units_Schema",
    "USCustomaryUnitSystemDefaults",
    "ECDbMap"
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                 05/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool IsOldStandardSchema(Utf8String schemaName)
    {
    for (Utf8CP* cur = oldStandardSchemaNames, *end = cur + _countof(oldStandardSchemaNames); cur < end; ++cur)
        if (schemaName.Equals(*cur))
            return true;

    return false;
    }

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

        IKindOfQuantityValidatorPtr kindOfQuantityValidator = new KindOfQuantityValidator();
        ECSchemaValidatorSingleton->AddKindOfQuantityValidator(kindOfQuantityValidator);
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
// @bsimethod                                    Dan.Perlman                  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AddKindOfQuantityValidator(IKindOfQuantityValidatorPtr& validator)
    {
    ECSchemaValidatorP schemaValidator = GetSingleton();
    schemaValidator->m_koqValidators.push_back(validator);
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
    // Clases
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

    // KOQ
    for (KindOfQuantityCP koq : schema.GetKindOfQuantities())
        {
        for (IKindOfQuantityValidatorPtr koqValidator : GetKindOfQuantityValidators())
            {
            ECObjectsStatus status = koqValidator->Validate(koq);
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed validation of KindOfQuantity '%s'", koq->GetName().c_str());
                m_validated = false;
                }
            else
                LOG.debugv("Succeeded validation of KindOfQuantity '%s'", koq->GetName().c_str());
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
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (!schema.Validate() || !schema.IsECVersion(ECVersion::Latest))
        {
        LOG.errorv("Failed to validate '%s' as ECVersion, %s", schema.GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest));
        status = ECObjectsStatus::Error;
        }

    if (!schema.OriginalECXmlVersionAtLeast(ECVersion::Latest))
        {
        LOG.errorv("Failed to validate '%s' since its original ECXML version is not ECVersion, %s", schema.GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest));
        status = ECObjectsStatus::Error;
        }

    /* 
    for (bpair <SchemaKey, ECSchemaPtr> ref : schema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = ref.second;
        Utf8String refName = refSchema->GetName();

        if (!IsOldStandardSchema(refName))
            continue;
        if (refName.EqualsIAscii("ECDbMap"))
            {
            if (refSchema->GetVersionRead() <= 1) // Only the latest ECDbMap is valid
                {
                LOG.errorv("Failed to validate '%s' as the read version is less than 2.0",
                    schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str());

                status = ECObjectsStatus::Error;
                }
            }
        else
            {
            LOG.errorv("Failed to validate '%s' since it references the old standard schema '%s'. Only new standard schemas should be used.",
                schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str());

            status = ECObjectsStatus::Error;
            }
        }
        */
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus MixinValidator::Validate(ECClassCR mixin) const
    {
    if (mixin.GetBaseClasses().size() > 1)
        {
        LOG.errorv("Mixin '%s' has more than 1 base class", mixin.GetFullName());
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

ECObjectsStatus CheckBisAspects(ECClassCR entity, Utf8CP derivedClassName, Utf8CP derivedRelationshipClassName, bool &entityDerivesFromSpecifiedClass)
    {
    if (entity.GetName().Equals(derivedClassName) || !entity.Is("BisCore", derivedClassName))
        return ECObjectsStatus::Success;
       
    bool foundValidRelationshipConstraint = false;
    
    // There must be a relationship that derives from derivedClassName with this class as its constraint
    for (ECClassCP classInCurrentSchema : entity.GetSchema().GetClasses())
        {
        ECRelationshipClassCP relClass = classInCurrentSchema->GetRelationshipClassCP();
        if (nullptr == relClass)
            continue;

        if (ECClass::ClassesAreEqualByName(&entity, relClass->GetTarget().GetConstraintClasses()[0]) && !relClass->GetTarget().GetIsPolymorphic() &&
            !relClass->GetName().Equals(derivedRelationshipClassName) && relClass->Is("BisCore", derivedRelationshipClassName))
            {
            foundValidRelationshipConstraint = true;
            break;
            }
        }

    entityDerivesFromSpecifiedClass = true;
    if (!foundValidRelationshipConstraint)
        {
        LOG.errorv("Entity class '%s' derives from '%s' so it must be in a non-polymorphic relationship that derives from '%s'", entity.GetFullName(), derivedClassName, derivedRelationshipClassName);
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CheckPropertiesForLongAndId(ECClassCR ecClass)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    for (ECPropertyP prop : ecClass.GetProperties(false))
        {
        if (prop->GetTypeName() == "long" && prop->GetName().EndsWith("Id"))
            {
            LOG.errorv("Warning treated as error in class '%s:%s' as it is of type 'long' and has a name ending with 'Id'", ecClass.GetFullName(), prop->GetName().c_str());
            status = ECObjectsStatus::Error;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus EntityValidator::Validate(ECClassCR entity) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    int numBaseClasses;
    bool entityDerivesFromSpecifiedClass = false;
    
    // Bis specific rule
    status = CheckBisAspects(entity, ElementMultiAspect, ElementOwnsMultiAspects, entityDerivesFromSpecifiedClass);
    if (!entityDerivesFromSpecifiedClass)
        status = CheckBisAspects(entity, ElementUniqueAspect, ElementOwnsUniqueAspect, entityDerivesFromSpecifiedClass);
    
    // Validate relationship properties of type long and ending in Id
    ECObjectsStatus propertyLongAndIdStatus = CheckPropertiesForLongAndId(entity);
    if (status == ECObjectsStatus::Success)
        status = propertyLongAndIdStatus;

    // Class may not implement both bis:IParentElement and bis:ISubModeledElement
    bool foundIParentElement = false;
    bool foundISubModelElement = false;
    for (ECClassCP baseClass : entity.GetBaseClasses())
        {
        if (strcmp(baseClass->GetFullName(), BISCORE_CLASS_IParentElement) == 0)
            foundIParentElement = true;
        if (strcmp(baseClass->GetFullName(), BISCORE_CLASS_ISubModeledElement) == 0)
            foundISubModelElement = true;
        }
    if (foundIParentElement && foundISubModelElement)
        {
        LOG.errorv("Entity class '%s' implements both bis:IParentElement and bis:ISubModeledElement", entity.GetFullName());
        status = ECObjectsStatus::Error;
        }
        
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
// @bsimethod                                    Dan.Perlman                  05/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CheckStrength(ECRelationshipClassCP relClass)
    {
    ECObjectsStatus returnStatus = ECObjectsStatus::Success;
    if (relClass->GetStrength() == StrengthType::Holding)
        {
        LOG.errorv("Relationship class '%s' strength must not be set to 'holding'.", relClass->GetFullName());
        returnStatus = ECObjectsStatus::Error;
        }

    if (relClass->GetStrength() == StrengthType::Embedding)
        {
        if (relClass->GetStrengthDirection() == ECRelatedInstanceDirection::Forward && relClass->GetSource().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class '%s' has an 'embedding' strength with a forward direction so the source constraint may not have a multiplicity upper bound greater than 1",
                       relClass->GetFullName());
            returnStatus = ECObjectsStatus::Error;
            }
        if (relClass->GetStrengthDirection() == ECRelatedInstanceDirection::Backward && relClass->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class '%s' has an 'embedding' strength with a backward direction so the target constraint may not have a multiplicity upper bound greater than 1",
                       relClass->GetFullName());
            returnStatus = ECObjectsStatus::Error;
            }
        if (relClass->GetName().Contains("Has"))
            {
            LOG.errorv("Relationship class '%s' has an 'embedding' strength and contains 'Has' in its name. Consider renaming this class.",
                       relClass->GetFullName());
            returnStatus = ECObjectsStatus::Error;
            }
        }
    return returnStatus;
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

    // Validate relationship strength
    status = CheckStrength(relClass);

    // Validate relationship properties of type long and ending in Id
    ECObjectsStatus propertyLongAndIdStatus = CheckPropertiesForLongAndId(ecClass);     
    if (status == ECObjectsStatus::Success)
        status = propertyLongAndIdStatus;

    ECRelationshipConstraintCR targetConstraint = relClass->GetTarget();
    ECRelationshipConstraintCR sourceConstraint = relClass->GetSource();
    
    // Validate both target and source.  If one of them fails, the class fails.
    ECObjectsStatus targetStatus = RelationshipValidator::CheckLocalDefinitions(targetConstraint, "Target");
    ECObjectsStatus sourceStatus = RelationshipValidator::CheckLocalDefinitions(sourceConstraint, "Source");

    if (status == ECObjectsStatus::Success)
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
        LOG.errorv("Relationship class '%s' constraint does not define any constraint classes defined locally in %s. Each constraint must locally define at least one constraint class.",
            className.c_str(), constraintType.c_str());

        status = ECObjectsStatus::Error;
        }

    if (!constraint.IsAbstractConstraintDefinedLocally() && constraint.GetConstraintClasses().size() > 1)
        {
        if (constraint.AreConstraintClassesDefinedLocally())
            LOG.errorv("Relationship class '%s' has more than one constraint class but does not have an abstract constraint in %s. An abstract constraint is required when there are more than one constraint classes defined.",
                className.c_str(), constraint.GetAbstractConstraint()->GetFullName());
        else
            LOG.errorv("Relationship class '%s' must define one constraint class locally in %s, or if multiple constraint classes are desired, then an abstract constraint is required.",
               className.c_str(), constraintType.c_str());

        status = ECObjectsStatus::Error;
        }

    if (constraint.IsAbstractConstraintDefinedLocally() && constraint.GetConstraintClasses().size() == 1)
        {
        LOG.errorv("Relationship class '%s' has an abstract constraint, '%s', and only one concrete constraint set in '%s'",
                   className.c_str(), constraint.GetAbstractConstraint()->GetFullName(), constraintType.c_str());

        status = ECObjectsStatus::Error;
        }

    if (!constraint.IsRoleLabelDefinedLocally())
        {
        LOG.errorv("Relationship class '%s' has a role label, '%s', that is not defined locally in %s. Each constraint must define a role label locally.",
            className.c_str(), constraint.GetRoleLabel().c_str(), constraintType.c_str());

        status = ECObjectsStatus::Error;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus KindOfQuantityValidator::Validate(KindOfQuantityCP koq) const
    {
    if (strcmp(koq->GetPersistenceUnit().GetUnit()->GetUnitSystem(), "SI") == 0)
        return ECObjectsStatus::Success;
 
    LOG.errorv("KindOfQuantity %s has persistence unit of unit system '%s' but must have an SI unit system", koq->GetFullName().c_str(), koq->GetPersistenceUnit().GetUnit()->GetUnitSystem());
    return ECObjectsStatus::Error;
    }
END_BENTLEY_ECOBJECT_NAMESPACE
