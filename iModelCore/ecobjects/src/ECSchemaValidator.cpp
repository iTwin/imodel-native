/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#define BISCORE_CLASS_IParentElement        "BisCore:IParentElement"
#define BISCORE_CLASS_ISubModeledElement    "BisCore:ISubModeledElement"
#define ElementMultiAspect                  "ElementMultiAspect"
#define ElementUniqueAspect                 "ElementUniqueAspect"
#define ElementOwnsUniqueAspect             "ElementOwnsUniqueAspect"
#define ElementOwnsMultiAspects             "ElementOwnsMultiAspects"
#define LinkedElementId                     "LinkedElementId"
#define MarkupSchema                        "Markup"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// Format strings used by validator components for logging.
// "<action> <element-type> '<element-name>' validation"
// example: "Starting Schema 'MyExampleSchema' validation"
#define FMTSTR_VALIDATION_START   "Starting validation of %s '%s'"
#define FMTSTR_VALIDATION_SUCCESS "Succeeded validation of %s '%s'"
#define FMTSTR_VALIDATION_FAILURE "Failed validation of %s '%s'"

static Utf8CP oldStandardSchemaNames[] =
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
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunSchemaValidators(ECSchemaCR schema)
    {
    for (auto const validator : m_schemaValidators)
        {
        if (ECObjectsStatus::Success != validator(schema))
            m_validated = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunClassValidators(ECClassCR ecClass)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, "Class", ecClass.GetName().c_str());
    bool classValidated = true;
    for (auto const validator : m_classValidators)
        {
        if (ECObjectsStatus::Success != validator(ecClass))
            classValidated = false;
        }
    if (classValidated)
        LOG.debugv(FMTSTR_VALIDATION_SUCCESS, "Class", ecClass.GetName().c_str());
    else
        {
        LOG.errorv(FMTSTR_VALIDATION_FAILURE, "Class", ecClass.GetName().c_str());
        m_validated = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunKindOfQuantityValidators(KindOfQuantityCR koq)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, "KindOfQuantity", koq.GetName().c_str());
    bool koqValidated = true;
    for (auto const validator : m_koqValidators)
        {
        if (ECObjectsStatus::Success != validator(koq))
            koqValidated = false;
        }
    if (koqValidated)
        LOG.debugv(FMTSTR_VALIDATION_SUCCESS, "KindOfQuantity", koq.GetName().c_str());
    else
        {
        LOG.errorv(FMTSTR_VALIDATION_FAILURE, "KindOfQuantity", koq.GetName().c_str());
        m_validated = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaValidator::ECSchemaValidator()
    {
    AddSchemaValidator(BaseECValidator);
    AddClassValidator(AllClassValidator);
    AddClassValidator(EntityValidator);
    AddClassValidator(MixinValidator);
    AddClassValidator(StructValidator);
    AddClassValidator(CustomAttributeClassValidator);
    AddClassValidator(RelationshipValidator);
    AddKindOfQuantityValidator(KindOfQuantityValidator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchemaValidator::Validate(ECSchemaCR schema)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, "Schema", schema.GetName().c_str());
    m_validated = true;

    RunSchemaValidators(schema);
    for (ECClassCP c : schema.GetClasses())
        RunClassValidators(*c);
    for (KindOfQuantityCP koq : schema.GetKindOfQuantities())
        RunKindOfQuantityValidators(*koq);

    if (m_validated)
        LOG.debugv(FMTSTR_VALIDATION_SUCCESS, "Schema", schema.GetName().c_str());
    else
        LOG.errorv(FMTSTR_VALIDATION_FAILURE, "Schema", schema.GetName().c_str());
    return m_validated;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::AddSchemaValidator(Validator<ECSchemaCR> validator)
    {
    m_schemaValidators.push_back(validator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::AddClassValidator(Validator<ECClassCR> validator)
    {
    m_classValidators.push_back(validator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::AddKindOfQuantityValidator(Validator<KindOfQuantityCR> validator)
    {
    m_koqValidators.push_back(validator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::BaseECValidator(ECSchemaCR schema)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    // RULE: The schema must be valid and convertible to the latest ECVersion.
    // TODO: const_cast is evil, but the Validate method is const iff called with the argument of false.
    //       An enhancement should go towards making a const correct Validate in the future.
    if (!const_cast<ECSchemaR>(schema).Validate(false) || !schema.IsECVersion(ECVersion::Latest))
        {
        LOG.errorv("Schema is not valid as ECVersion, %s", ECSchema::GetECVersionString(ECVersion::Latest));
        status = ECObjectsStatus::Error;
        }

    // RULE: The schema's written version must be the lastest ECVersion.
    if (!schema.OriginalECXmlVersionAtLeast(ECVersion::Latest))
        {
        LOG.errorv("Schema ECXML Version is not the latest ECVersion, %s", ECSchema::GetECVersionString(ECVersion::Latest));
        status = ECObjectsStatus::Error;
        }

    // RULE: If the schema contains 'dynamic' (case-insensitive) in its name it must apply the CoreCA:DynamicSchema custom attribute.
    if (schema.GetName().ContainsI("dynamic"))
        {
        bool containsDynamicSchemaCA = schema.GetCustomAttributes(true).end() != std::find_if(
            schema.GetCustomAttributes(true).begin(),
            schema.GetCustomAttributes(true).end(),
            [](auto const& custAttr) {return custAttr->GetClass().GetName().Equals("DynamicSchema");});
        if (!containsDynamicSchemaCA)
            {
            LOG.errorv("Schema name contains 'dynamic' but does not appy the 'DynamicSchema' ECCustomAttribute");
            status = ECObjectsStatus::Error;
            }
        }

    for (bpair<SchemaKey, ECSchemaPtr> ref : schema.GetReferencedSchemas())
        {
        ECSchemaPtr refSchema = ref.second;
        Utf8String refName = refSchema->GetName();

        // RULE*: The schema may not reference an EC2 standard schema (with the exception of the latest ECDbMap schema).
        static auto const IsOldStandardSchema = [](Utf8String schemaName) -> bool
            {
            for (auto oldSchemaName : oldStandardSchemaNames)
                {
                if (schemaName.Equals(oldSchemaName))
                    return true;
                }
            return false;
            };
        if (IsOldStandardSchema(refName))
            {
            if (refName.EqualsIAscii("ECDbMap"))
                {
                // RULE: Only the latest ECDbMap is valid
                if (refSchema->GetVersionRead() <= 1) 
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

        if (refSchema->OriginalECXmlVersionLessThan(ECVersion::V3_1))
            {
            LOG.errorv("Failed to validate '%s' since it references '%s' using EC%d.%d. A schema may not reference any EC2 or EC3.0 schemas.",
                       schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str(), refSchema->GetOriginalECXmlVersionMajor(), refSchema->GetOriginalECXmlVersionMinor());
             status = ECObjectsStatus::Error;
            }
        }

    // RULE: Entity classes within the same schema should not have the same display label
    bvector<ECClassCP> entityClasses;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (ecClass->IsEntityClass() && !ecClass->GetEntityClassCP()->IsMixin())
            {
            for (ECClassCP prevClass : entityClasses)
                {
                if (ecClass->GetDisplayLabel().EqualsIAscii(prevClass->GetDisplayLabel()))
                    {
                    LOG.errorv("Failed to validate '%s'. Entity classes '%s' and '%s' have the same display label '%s'",
                               schema.GetFullSchemaName().c_str(), prevClass->GetFullName(), ecClass->GetFullName(), ecClass->GetDisplayLabel().c_str());
                    status = ECObjectsStatus::Error;
                    }
                }
            entityClasses.push_back(ecClass);
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                  09/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AllClassValidator(ECClassCR ecClass)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    // RULE: Properties should not be of type long.
    for (ECPropertyP prop : ecClass.GetProperties(false))
        {
        if (prop->GetTypeName().Equals("long") && !prop->GetIsNavigation())
            {
            LOG.errorv("Warning treated as error. Property '%s.%s' is of type 'long' and long properties are not allowed. Use int, double or if this represents a FK use a navigiation property",
                ecClass.GetFullName(), prop->GetName().c_str());
            status = ECObjectsStatus::Error;
            }
        }

    // RULE: No properties can have the same display label and category
    bvector<ECPropertyP> propertiesList;
    for (ECPropertyP prop : ecClass.GetProperties(true))
        {
        Utf8StringCR displayLabel = prop->GetDisplayLabel();
        PropertyCategoryCP category = prop->GetCategory();

        for (ECPropertyP prevProp : propertiesList)
            {
            if (prevProp->GetDisplayLabel().EqualsIAscii(displayLabel))
                {
                PropertyCategoryCP prevCategory = prevProp->GetCategory();
                if (prevCategory == nullptr && category == nullptr) // neither have defined category
                    {
                    LOG.errorv("Class '%s' has properties '%s' and '%s' with the same display label '%s'",
                               ecClass.GetFullName(), prevProp->GetName().c_str(), prop->GetName().c_str(), displayLabel.c_str());
                    status = ECObjectsStatus::Error;
                    }

                if (prevCategory == nullptr || category == nullptr)
                    continue;
                if (prevCategory->GetFullName().EqualsIAscii(category->GetFullName()))
                    {
                    LOG.errorv("Class '%s' has properties '%s' and '%s' with the same display label '%s' and category '%s'",
                               ecClass.GetFullName(), prop->GetName().c_str(), prevProp->GetName().c_str(), displayLabel.c_str(), category->GetFullName().c_str());
                    status = ECObjectsStatus::Error;
                    }
                }
            }
        propertiesList.push_back(prop);
        }

    return status;
    }

ECObjectsStatus CheckForModelBaseClasses(ECClassCR baseClass, ECClassCR entity)
    {
    // Class may not subclass bis:SpatialLocationModel
    if (baseClass.Is("BisCore", "SpatialLocationModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:SpatialLocationModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:PhysicalModel
    if (baseClass.Is("BisCore", "PhysicalModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:PhysicalModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:GroupInformationModel
    if (baseClass.Is("BisCore", "GroupInformationModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:GroupInformationModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:InformationRecordModel
    if (baseClass.Is("BisCore", "InformationRecordModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:InformationRecordModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:DocumentListModel
    if (baseClass.Is("BisCore", "DocumentListModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:DocumentListModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:LinkModel
    if (baseClass.Is("BisCore", "LinkModel"))
        {
        LOG.errorv("Entity class '%s' may not subclass bis:LinkModel.", entity.GetFullName());
        return ECObjectsStatus::Error;
        }
    // Class may not subclass bis:DefinitionModel, except for DictionaryModel and RepositoryModel
    if (baseClass.Is("BisCore", "DefinitionModel"))
        {
        if (!(strcmp(entity.GetFullName(), "BisCore:DictionaryModel") == 0) && !(strcmp(entity.GetFullName(), "BisCore:RepositoryModel") == 0))
            {
            LOG.errorv("Entity class '%s' may not subclass bis:DefinitionModel.", entity.GetFullName());
            return ECObjectsStatus::Error;
            }
        }
    
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckBisAspects(bool& entityDerivesFromSpecifiedClass, ECClassCR entity, Utf8CP derivedClassName, Utf8CP derivedRelationshipClassName)
    {
    if (ECClassModifier::Abstract == entity.GetClassModifier() || entity.GetName().Equals(derivedClassName) || !entity.Is("BisCore", derivedClassName))
        return ECObjectsStatus::Success;

    bool foundValidRelationshipConstraint = false;

    // There must be a relationship that derives from derivedClassName with this class as its constraint
    for (ECClassCP classInCurrentSchema : entity.GetSchema().GetClasses())
        {
        ECRelationshipClassCP relClass = classInCurrentSchema->GetRelationshipClassCP();
        if (nullptr == relClass)
            continue;

        if (!relClass->GetName().Equals(derivedRelationshipClassName) && relClass->Is("BisCore", derivedRelationshipClassName) &&
            (relClass->GetTarget().SupportsClass(entity)) && !relClass->GetTarget().GetConstraintClasses()[0]->GetName().Equals(derivedClassName))
            {
            foundValidRelationshipConstraint = true;
            break;
            }
        }

    entityDerivesFromSpecifiedClass = true;
    if (!foundValidRelationshipConstraint)
        {
        LOG.errorv("Entity class derives from '%s' so it must be a supported target constraint in a relationship that derives from '%s'",
            derivedClassName, derivedRelationshipClassName);
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::EntityValidator(ECClassCR entity)
    {
    if (!entity.IsEntityClass())
        return ECObjectsStatus::Success;

    ECObjectsStatus status = ECObjectsStatus::Success;
    bool entityDerivesFromSpecifiedClass = false;

    // BisAspects
    // RULE: If any aspect (ECClass which derives from ElementMultiAspect) exists, there must be a relationship that
    //       derives from the ElementOwnsMultiAspects relationship with this class supported as a target constraint.
    // RULE: If any aspect (ECClass which derives from ElementUniqueAspect) exists, there must be a relationship that
    //       derives from the ElementOwnsUniqueAspect relationship with this class supported as a target constraint.
    if (ECObjectsStatus::Success != CheckBisAspects(entityDerivesFromSpecifiedClass, entity, ElementMultiAspect, ElementOwnsMultiAspects))
        status = ECObjectsStatus::Error;
    if (!entityDerivesFromSpecifiedClass && (ECObjectsStatus::Success != CheckBisAspects(entityDerivesFromSpecifiedClass, entity, ElementUniqueAspect, ElementOwnsUniqueAspect)))
        status = ECObjectsStatus::Error;

    // RULE: An Entity Class may not implement both bis:IParentElement and bis:ISubModeledElement.
    bool foundIParentElement = false;
    bool foundISubModelElement = false;
    for (ECClassCP baseClass : entity.GetBaseClasses())
        {
        if (strcmp(baseClass->GetFullName(), BISCORE_CLASS_IParentElement) == 0)
            foundIParentElement = true;
        if (strcmp(baseClass->GetFullName(), BISCORE_CLASS_ISubModeledElement) == 0)
            foundISubModelElement = true;

        if (ECObjectsStatus::Success != CheckForModelBaseClasses(*baseClass, entity))
            status = ECObjectsStatus::Error;
        }
    if (foundIParentElement && foundISubModelElement)
        {
        LOG.errorv("Entity class implements both bis:IParentElement and bis:ISubModeledElement. Entity Classes may implement bis:IParentElement or bis:ISubModeledElement but not both.");
        status = ECObjectsStatus::Error;
        }

    // RULE: Class may not subclass bis:PhysicalModel
    for (ECClassCP baseClass : entity.GetBaseClasses())
        {
        if (baseClass->Is("BisCore", "PhysicalModel"))
            {
            LOG.errorv("Entity class '%s' may not subclass bis:PhysicalModel.", entity.GetFullName());
            status = ECObjectsStatus::Error;
            }
        }

    // RULE: Root entity classes must derive from the bis hierarchy.
    auto const isBisCoreClass = [](ECClassCP entity) -> bool {return entity->GetSchema().GetName().Equals("BisCore");};
    std::function<bool(ECClassCP entity)> derivesFromBisHierarchy = [&derivesFromBisHierarchy, &isBisCoreClass](ECClassCP entity) -> bool
        {
        return isBisCoreClass(entity) ||
            (entity->GetBaseClasses().end() != std::find_if(
                entity->GetBaseClasses().begin(),
                entity->GetBaseClasses().end(),
                derivesFromBisHierarchy));
        };
    if (!entity.GetEntityClassCP()->IsMixin() && !isBisCoreClass(&entity) && !derivesFromBisHierarchy(&entity))
        {
        LOG.errorv("Root entity class '%s' does not derive from bis hierarchy", entity.GetFullName());
        status = ECObjectsStatus::Error;
        }
        // RULE: entity class may only have one entity base class
        int numBaseClasses = 0;
        for (ECClassCP baseClass : entity.GetBaseClasses())
            {
            if (!baseClass->GetEntityClassCP()->IsMixin())
                numBaseClasses++;
            }
        if (numBaseClasses > 1)
            {
            LOG.errorv("Entity class '%s' inherits from more than one entity class", entity.GetFullName());
            status = ECObjectsStatus::Error;
        }

    for (ECPropertyP prop : entity.GetProperties(false))
        {
        if (nullptr == prop->GetBaseProperty())
            continue;

        if (prop->IsKindOfQuantityDefinedLocally())
            {
            auto propKOQ = prop->GetKindOfQuantity();
            auto basePropKOQ = prop->GetBaseProperty()->GetKindOfQuantity();
            // RULE: Defined kind of quantities on derived properties of entity classes must override their base property's kind of quantity. 
            if (nullptr == basePropKOQ)
                {
                LOG.errorv("Property '%s.%s' specifies a KindOfQuantity locally but its base property '%s.%s' does not define or inherit a KindOfQuantity",
                    prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetBaseProperty()->GetClass().GetFullName(), prop->GetBaseProperty()->GetName().c_str());
                status = ECObjectsStatus::Error;
                }
            // RULE: Property overrides must not change the persistence unit. -- no longer possible because of changes to make this invalid directly in EC API
            else if (!Units::Unit::AreEqual(propKOQ->GetPersistenceUnit(), basePropKOQ->GetPersistenceUnit()))
                {
                LOG.errorv("Property '%s.%s' specifies a KindOfQuantity '%s' which has a different persistence unit than the KindOfQuantity '%s' specified on the base property '%s.%s'",
                    prop->GetClass().GetFullName(), prop->GetName().c_str(), propKOQ->GetFullName().c_str(),
                    prop->GetBaseProperty()->GetClass().GetFullName(), prop->GetBaseProperty()->GetName().c_str(), basePropKOQ->GetFullName().c_str());
                status = ECObjectsStatus::Error;
                }
            }

        }

    // mixin property may not override an inherited property
    bvector<bpair<ECPropertyP, ECClassP>> seenProperties;
    
    auto const isPropertyMatch = [](bpair<ECPropertyP, ECClassP> propPair, ECPropertyP prop) -> bool
        {
        return propPair.first->GetName().EqualsIAscii(prop->GetName());
        };
    auto const findProperty = [&isPropertyMatch, &seenProperties] (ECPropertyP prop) -> bpair<ECPropertyP, ECClassP>*
        {
        auto isMatch = std::bind(isPropertyMatch, std::placeholders::_1, prop);
        return std::find_if(
            seenProperties.begin(),
            seenProperties.end(),
            isMatch);
        };

    // iterate through all inherited properties and check for duplicates
    for (ECClassP baseClass : entity.GetBaseClasses())
        {
        for (ECPropertyP prop : baseClass->GetProperties(true))
            {
            bpair<ECPropertyP, ECClassP>* propertyClassPair = findProperty(prop);
            if (seenProperties.end() != propertyClassPair) // already seen property
                {
                if (prop != propertyClassPair->first)
                    {
                    ECClassP prevClass = propertyClassPair->second;
                    bool prevIsMixin = prevClass->GetEntityClassCP()->IsMixin();
                    if (baseClass->GetEntityClassCP()->IsMixin() && prevIsMixin)
                        {
                        LOG.errorv("Error at property '%s'. Mixin class '%s' overrides a property inherited from mixin class '%s'",
                                   prop->GetName().c_str(), baseClass->GetFullName(), prevClass->GetFullName());
                        status = ECObjectsStatus::Error;
                        }
                    else
                        {
                        LOG.errorv("Error at property '%s'. Mixin class '%s' overrides a property inherited from entity class '%s'",
                                   prop->GetName().c_str(),
                                   prevIsMixin ? prevClass->GetFullName() : baseClass->GetFullName(),
                                   prevIsMixin ? baseClass->GetFullName() : prevClass->GetFullName());
                        status = ECObjectsStatus::Error;
                        }
                    }
                }
            else
                {
                seenProperties.push_back(make_bpair<ECPropertyP, ECClassP>(prop, baseClass));
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::MixinValidator(ECClassCR mixin)
    {
    if (!(mixin.IsEntityClass() && mixin.GetEntityClassCP()->IsMixin()))
        return ECObjectsStatus::Success;

    for (ECPropertyP prop : mixin.GetProperties(false))
        {
        // RULE: A mixin class must not override an inherited property.
        if (nullptr != prop->GetBaseProperty())
            {
            LOG.errorv("Mixin overrides an inherited property '%s'. A mixin class must not override an inherited property.", prop->GetName().c_str());
            return ECObjectsStatus::Error;
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                  09/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::StructValidator(ECClassCR structClass)
    {
    if (!structClass.IsStructClass())
        return ECObjectsStatus::Success;

    // RULE: Struct classes must not have base classes.
    if (structClass.HasBaseClasses())
        {
        LOG.errorv("Struct class has base classes, but structs should not have base classes");
        return ECObjectsStatus::Error;
        }
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                  09/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::CustomAttributeClassValidator(ECClassCR caClass)
    {
    if (!caClass.IsCustomAttributeClass())
        return ECObjectsStatus::Success;

    // RULE: Custom attribute classes must not have base classes.
    if (caClass.HasBaseClasses())
        {
        LOG.errorv("Custom Attribute class has base classes, but a custom attribute class should not have base classes");
        return ECObjectsStatus::Error;
        }
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  05/2017
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckStrength(ECRelationshipClassCP relClass)
    {
    ECObjectsStatus returnStatus = ECObjectsStatus::Success;

    // RULE: Relationship classes must not have strength set to 'holding'.
    if (StrengthType::Holding == relClass->GetStrength())
        {
        LOG.errorv("Relationship class strength must not be set to 'holding'.");
        returnStatus = ECObjectsStatus::Error;
        }

    if (StrengthType::Embedding == relClass->GetStrength())
        {
        // RULE: Relationship classes must not have a source constraint multiplicity upper bound greater than 1 if the strength is embedding and the direction is forward.
        if (ECRelatedInstanceDirection::Forward == relClass->GetStrengthDirection() && relClass->GetSource().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class has an 'embedding' strength with a forward direction so the source constraint may not have a multiplicity upper bound greater than 1");
            returnStatus = ECObjectsStatus::Error;
            }
        // RULE: Relationship classes must not have a target constraint multiplicity upper bound greater than 1 if the strength is embedding and the direction is backward.
        else if (ECRelatedInstanceDirection::Backward == relClass->GetStrengthDirection() && relClass->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class has an 'embedding' strength with a backward direction so the target constraint may not have a multiplicity upper bound greater than 1");
            returnStatus = ECObjectsStatus::Error;
            }

        // RULE: Embedding relationships should not have 'Has' (case-sensitive) in the class name.
        if (relClass->GetName().Contains("Has"))
            {
            LOG.errorv("Relationship class has an 'embedding' strength and contains 'Has' in its name. Consider renaming this class.");
            returnStatus = ECObjectsStatus::Error;
            }
        }

    return returnStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckLocalDefinitions(ECRelationshipConstraintCR constraint, Utf8String constraintType)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    if (constraint.IsAbstractConstraintDefined() && constraint.GetConstraintClasses().size() == 1)
        {
        LOG.errorv("Relationship class has an abstract constraint, '%s', and only one concrete constraint set in '%s'",
            constraint.GetAbstractConstraint()->GetFullName(), constraintType.c_str());
        status = ECObjectsStatus::Error;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::RelationshipValidator(ECClassCR ecClass)
    {
    if (!ecClass.IsRelationshipClass())
        return ECObjectsStatus::Success;

    ECObjectsStatus status = ECObjectsStatus::Success;
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (nullptr == relClass)
        return status;

    // Validate relationship strength
    status = CheckStrength(relClass);

    ECRelationshipConstraintCR targetConstraint = relClass->GetTarget();
    ECRelationshipConstraintCR sourceConstraint = relClass->GetSource();

    // RULE: Relationship classes must not have an abstract constraint if there is only one concrete constraint set.
    if (ECObjectsStatus::Success != CheckLocalDefinitions(targetConstraint, "Target"))
        status = ECObjectsStatus::Error;
    if (ECObjectsStatus::Success != CheckLocalDefinitions(sourceConstraint, "Source"))
        status = ECObjectsStatus::Error;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::KindOfQuantityValidator(KindOfQuantityCR koq)
    {
    // RULE: Persistence unit of phenomenon 'PERCENTAGE' (or other unitless ratios) are not allowed.
    if (0 == strcmp(koq.GetPersistenceUnit()->GetPhenomenon()->GetName().c_str(), "PERCENTAGE"))
        {
        LOG.errorv("KindOfQuantity %s has persistence unit of Phenomenon 'PERCENTAGE'. Unitless ratios are not allowed. Use a ratio phenomenon which includes units like VOLUME_RATIO", koq.GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    // RULE: KindOfQuantity must have an SI unit for its persistence unit.
    if (0 != strcmp(koq.GetPersistenceUnit()->GetUnitSystem()->GetName().c_str(), "SI"))
        {
        LOG.errorv("KindOfQuantity has persistence unit of unit system '%s' but must have an SI unit system", koq.GetPersistenceUnit()->GetUnitSystem()->GetName().c_str());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
 