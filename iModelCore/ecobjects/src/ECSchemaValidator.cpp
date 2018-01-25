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
        if (!IsOldStandardSchema(refName))
            continue;
        if (refName.EqualsIAscii("ECDbMap"))
            {
            // RULE: Only the latest ECDbMap is valid.
            if (refSchema->GetVersionRead() < 2)
                {
                LOG.errorv("Referenced Schema ECDbMap has read version less than 2.0. Only ECDbMap.2.0 is allowed.");
                status = ECObjectsStatus::Error;
                }
            }
        else
            {
            // RULE* (cont.)
            LOG.errorv("Schema references the old standard schema '%s'. Only new standard schemas should be used.",
                refSchema->GetFullSchemaName().c_str());
            status = ECObjectsStatus::Error;
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

    return status;
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
        }
    if (foundIParentElement && foundISubModelElement)
        {
        LOG.errorv("Entity class implements both bis:IParentElement and bis:ISubModeledElement. Entity Classes may implement bis:IParentElement or bis:ISubModeledElement but not both.");
        status = ECObjectsStatus::Error;
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
        LOG.errorv("Entity class does not derive from the bis hierarchy");
        status = ECObjectsStatus::Error;
        }

    for (ECPropertyP prop : entity.GetProperties(false))
        {
        if (nullptr == prop->GetBaseProperty())
            continue;

        // RULE: Entity classes may not inherit a property from more than one base class.
        size_t numBaseClasses = 0;
        for (ECClassP baseClass : entity.GetBaseClasses())
            {
            if (nullptr != baseClass->GetPropertyP(prop->GetName().c_str()))
                numBaseClasses += 1;
            }
        if (numBaseClasses > 1)
            {
            LOG.errorv("Property '%s.%s' is inherited from more than one base class. A property may only be inherited from a single base class.",
                prop->GetClass().GetFullName(), prop->GetName().c_str());

            status = ECObjectsStatus::Error;
            }

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
            // RULE: Property overrides must not change the persistence unit.
            else if (!Units::Unit::AreEqual(propKOQ->GetPersistenceUnit().GetUnit(), basePropKOQ->GetPersistenceUnit().GetUnit()))
                {
                LOG.errorv("Property '%s.%s' specifies a KindOfQuantity '%s' which has a different persistence unit than the KindOfQuantity '%s' specified on the base property '%s.%s'",
                    prop->GetClass().GetFullName(), prop->GetName().c_str(), propKOQ->GetFullName().c_str(),
                    prop->GetBaseProperty()->GetClass().GetFullName(), prop->GetBaseProperty()->GetName().c_str(), basePropKOQ->GetFullName().c_str());
                status = ECObjectsStatus::Error;
                }
            }

        // RULE: Entity classes must not override a property inherited from a mixin class.
        if (prop->GetBaseProperty()->GetClass().GetEntityClassCP()->IsMixin())
            {
            LOG.errorv("Property '%s.%s' overrides an inherited property from MixinClass %s. Entity Classes cannot override Mixin class properties",
                prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetBaseProperty()->GetClass().GetFullName());
            status = ECObjectsStatus::Error;
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
    if (0 == strcmp(koq.GetPersistenceUnit().GetUnit()->GetPhenomenon()->GetName(), "PERCENTAGE"))
        {
        LOG.errorv("KindOfQuantity has persistence unit of Phenomenon 'PERCENTAGE'. Unitless ratios are not allowed. Use a ratio phenomenon which includes units like VOLUME_RATIO");
        return ECObjectsStatus::Error;
        }

    // RULE: KindOfQuantity must have an SI unit for its persistence unit.
    if (0 != strcmp(koq.GetPersistenceUnit().GetUnit()->GetUnitSystem()->GetName(), "SI"))
        {
        LOG.errorv("KindOfQuantity has persistence unit of unit system '%s' but must have an SI unit system", koq.GetPersistenceUnit().GetUnit()->GetUnitSystem()->GetName());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
