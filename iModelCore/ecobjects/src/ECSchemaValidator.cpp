/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#define EC_Class    "Class"
#define EC_Source   "Source"
#define EC_Target   "Target"
#define EC_KOQ      "KindOfQuantity"

#define BISCORE_SCHEMA_NAME                 "BisCore"
#define BISCORE_CLASS_IParentElement        "BisCore:IParentElement"
#define BISCORE_CLASS_ISubModeledElement    "BisCore:ISubModeledElement"
#define ElementMultiAspect                  "ElementMultiAspect"
#define ElementUniqueAspect                 "ElementUniqueAspect"
#define ElementOwnsUniqueAspect             "ElementOwnsUniqueAspect"
#define ElementOwnsMultiAspects             "ElementOwnsMultiAspects"

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

static Utf8CP validExtendedTypes[] =
    {
    "BeGuid",
    "GeometryStream",
    "Json"
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
    LOG.debugv(FMTSTR_VALIDATION_START, EC_Class, ecClass.GetName().c_str());
    bool classValidated = true;
    for (auto const validator : m_classValidators)
        {
        if (ECObjectsStatus::Success != validator(ecClass))
            classValidated = false;
        }
    if (classValidated)
        LOG.debugv(FMTSTR_VALIDATION_SUCCESS, EC_Class, ecClass.GetName().c_str());
    else
        {
        LOG.errorv(FMTSTR_VALIDATION_FAILURE, EC_Class, ecClass.GetName().c_str());
        m_validated = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman              01/2018
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunKindOfQuantityValidators(KindOfQuantityCR koq)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, EC_KOQ, koq.GetName().c_str());
    bool koqValidated = true;
    for (auto const validator : m_koqValidators)
        {
        if (ECObjectsStatus::Success != validator(koq))
            koqValidated = false;
        }
    if (koqValidated)
        LOG.debugv(FMTSTR_VALIDATION_SUCCESS, EC_KOQ, koq.GetName().c_str());
    else
        {
        LOG.errorv(FMTSTR_VALIDATION_FAILURE, EC_KOQ, koq.GetName().c_str());
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

bool isADesignSpeedClass(ECClassCP ecClass)
    {
    return 0 == strcmp(ecClass->GetFullName(), "RoadRailPhysical:DesignSpeed") || 0 == strcmp(ecClass->GetFullName(), "RoadRailPhysical:DesignSpeedElement");
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
        LOG.errorv("Schema '%s' does not pass EC validation required to convert to the latest ECVersion, %s", schema.GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest));
        status = ECObjectsStatus::Error;
        }

    // RULE: The schema's written version must be at least EC3.1.
    if (!schema.OriginalECXmlVersionAtLeast(ECVersion::V3_1))
        {
        Utf8String message = Utf8PrintfString("Schema '%s' original ECXML Version is '%u.%u' but must be at least EC 3.1", schema.GetFullSchemaName().c_str(),
                                              schema.GetOriginalECXmlVersionMajor(), schema.GetOriginalECXmlVersionMinor());
        if (schema.IsDynamicSchema())
            LOG.warning(message.c_str());
        else
            {
            LOG.errorv(message.c_str());
            status = ECObjectsStatus::Error;
            }
        }

    // RULE: If the schema contains 'dynamic' (case-insensitive) in its name it must apply the CoreCA:DynamicSchema custom attribute.
    if (schema.GetName().ContainsI("dynamic") && !schema.IsDynamicSchema())
        {
        LOG.errorv("Schema '%s' contains 'dynamic' in it's name but does not appy the 'DynamicSchema' ECCustomAttribute", schema.GetFullSchemaName().c_str());
        status = ECObjectsStatus::Error;
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

        // RULE: A schema may not reference any ECXml less than EC3.1.
        if (refSchema->OriginalECXmlVersionLessThan(ECVersion::V3_1))
            {
            if (schema.IsDynamicSchema() && refName.EqualsIAscii("ECv3ConversionAttributes"))
                continue;
            LOG.errorv("Failed to validate '%s' since it references '%s' using EC%" PRIu32 ".%" PRIu32 ". A schema may not reference any EC2 or EC3.0 schemas.",
                       schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str(), refSchema->GetOriginalECXmlVersionMajor(), refSchema->GetOriginalECXmlVersionMinor());
             status = ECObjectsStatus::Error;
            }
        }

    // RULE: Entity classes within the same schema should not have the same display label
    //      Except: RoadRailPhysical:DesignSpeed and RoadRailPhysical:DesignSpeedElement
    // skip this rule for dynamic schemas; show it only as a warning
    bool isDynamicSchema = schema.IsDynamicSchema();

    bvector<ECClassCP> entityClasses;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (!ecClass->IsEntityClass() || ecClass->IsMixin())
            continue;

        for (ECClassCP prevClass : entityClasses)
            {
            if (ecClass->GetDisplayLabel().EqualsIAscii(prevClass->GetDisplayLabel()))
                {
                Utf8String errMsg;
                errMsg.Sprintf("Failed to validate '%s'. Entity classes '%s' and '%s' have the same display label '%s'",
                    schema.GetFullSchemaName().c_str(), prevClass->GetFullName(), ecClass->GetFullName(), ecClass->GetDisplayLabel().c_str());

                // make this a warning for dynamic schemas
                if (isDynamicSchema || (isADesignSpeedClass(ecClass) && isADesignSpeedClass(prevClass)))
                    {
                    LOG.warning(errMsg.c_str());
                    continue;
                    }

                LOG.error(errMsg.c_str());
                status = ECObjectsStatus::Error;
                }
            }

        entityClasses.push_back(ecClass);
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

    // RULE: All classes should have a description (warn)
    if (ecClass.GetInvariantDescription().empty())
        LOG.warningv("Class '%s' has no description. Please add a description.", ecClass.GetFullName());

    bool isModel = (!ecClass.GetSchema().GetName().EqualsI(BISCORE_SCHEMA_NAME)) && ecClass.Is(BISCORE_SCHEMA_NAME, "Model");

    // RULE: Properties should have a description (warn)
    // RULE: Properties should not be of type long.
    // RULE: All extended types of properties should be on the valid list
    // RULE: All Model subclasses outside of BisCore should not add properties
    //      Except: Raster:RasterModel.Clip, ScalableMesh:ScalableMeshModel.SmModelClips, ScalableMesh:ScalableMeshModel.SmGroundCoverages, ScalableMesh:ScalableMeshModel.SmModelClipVectors
    for (ECPropertyP prop : ecClass.GetProperties(false))
        {
        if (prop->GetInvariantDescription().empty())
            LOG.warningv("Property '%s.%s' has no description. Please add a description.", ecClass.GetFullName(), prop->GetName().c_str());

        if (prop->GetTypeName().Equals("long") && !prop->GetIsNavigation())
            {
            LOG.errorv("Warning treated as error. Property '%s.%s' is of type 'long' and long properties are not allowed. Use int, double or if this represents a FK use a navigiation property",
                ecClass.GetFullName(), prop->GetName().c_str());
            status = ECObjectsStatus::Error;
            }

        if (isModel && nullptr == prop->GetBaseProperty())
            {
            static bmap<Utf8String, bvector<Utf8String>> exceptions;
            exceptions["Raster:RasterModel"] = { "Clip" };
            exceptions["ScalableMesh:ScalableMeshModel"] = { "SmModelClips", "SmGroundCoverages", "SmModelClipVectors" };

            auto modelException = exceptions.find(ecClass.GetFullName());
            if (modelException != exceptions.end() && std::any_of(modelException->second.begin(), modelException->second.end(), [prop](Utf8StringCR propName) { return propName.Equals(prop->GetName()); }))
                LOG.warningv("Class '%s' adds property '%s.%s' dispite being a subclass of 'bis:Model' and is defined outside of BisCore.  Model subclasses should not add new properties, but this property was released before this rule was written",
                             ecClass.GetFullName(), ecClass.GetFullName(), prop->GetName().c_str());
            else
                {
                status = ECObjectsStatus::Error;
                LOG.errorv("Class '%s' adds property '%s.%s' despite being a subclass of 'BisCore:Model' defined outside 'BisCore'. Model subclasses should not add new properties.",
                           ecClass.GetFullName(), ecClass.GetFullName(), prop->GetName().c_str());
                }
            }

        if (prop->HasExtendedType())
            {
            static auto const IsValidExtendedType = [] (Utf8StringCR eType) -> bool
                {
                for (auto validType : validExtendedTypes)
                    {
                    if (eType.EqualsI(validType))
                        return true;
                    }
                return false;
                };

            static auto const CheckForValidExtendedType = [] (Utf8StringCR eType, Utf8StringCR className, Utf8StringCR propName) -> ECObjectsStatus
                {
                ECObjectsStatus status = ECObjectsStatus::Success;
                if (!eType.empty() && !IsValidExtendedType(eType))
                    {
                    status = ECObjectsStatus::Error;
                    LOG.errorv("Property '%s.%s' has extended type '%s', which is not on the list of valid extended types (currently 'BeGuid', 'GeometryStream', and 'Json').",
                               className.c_str(), propName.c_str(), eType.c_str());
                    }
                return status;
                };

            if (nullptr != prop->GetAsPrimitiveProperty())
                {
                if (ECObjectsStatus::Success != CheckForValidExtendedType(prop->GetAsPrimitiveProperty()->GetExtendedTypeName(), ecClass.GetFullName(), prop->GetName()))
                    status = ECObjectsStatus::Error;
                }
            else if (nullptr != prop->GetAsPrimitiveArrayProperty())
                {
                if (ECObjectsStatus::Success != CheckForValidExtendedType(prop->GetAsPrimitiveArrayProperty()->GetExtendedTypeName(), ecClass.GetFullName(), prop->GetName()))
                    status = ECObjectsStatus::Error;
                }
            }
        }

    // RULE: No properties can have the same display label and category, including properties that are not within a category.
    // For dynamic schemas; only show as a warning
    bool isDynamicSchema = ecClass.GetSchema().IsDynamicSchema();

    bvector<ECPropertyP> propertiesList;
    for (ECPropertyP prop : ecClass.GetProperties(true))
        {
        Utf8StringCR displayLabel = prop->GetInvariantDisplayLabel();
        PropertyCategoryCP category = prop->GetCategory();

        for (ECPropertyP prevProp : propertiesList)
            {
            if (!prevProp->GetInvariantDisplayLabel().EqualsIAscii(displayLabel))
                continue;

            PropertyCategoryCP prevCategory = prevProp->GetCategory();
            if (prevCategory == nullptr && category == nullptr) // neither have defined category
                {
                Utf8String errMsg;
                errMsg.Sprintf("Class '%s' has properties '%s' and '%s' with the same display label '%s'",
                    ecClass.GetFullName(), prevProp->GetName().c_str(), prop->GetName().c_str(), displayLabel.c_str());

                if (!isDynamicSchema)
                    {
                    LOG.error(errMsg.c_str());
                    status = ECObjectsStatus::Error;
                    }
                else
                    LOG.warning(errMsg.c_str());
                }

            if (prevCategory == nullptr || category == nullptr)
                continue;

            if (!prevCategory->GetFullName().EqualsIAscii(category->GetFullName()))
                continue;

            Utf8String errMsg;
            errMsg.Sprintf("Class '%s' has properties '%s' and '%s' with the same display label '%s' and category '%s'",
                        ecClass.GetFullName(), prop->GetName().c_str(), prevProp->GetName().c_str(), displayLabel.c_str(), category->GetFullName().c_str());

            if (!isDynamicSchema)
                {
                LOG.error(errMsg.c_str());
                status = ECObjectsStatus::Error;
                }
            else
                LOG.warning(errMsg.c_str());
            }
        propertiesList.push_back(prop);
        }

    return status;
    }

// Rule: Class may not subclass the following Model types:
//  - bis:SpatialLocationModel
//      - Except: RoadRailAlignment:AlignmentModel, RoadRailAlignment:AlignmentModel, RoadRailAlignment:HorizontalAlignmentModel
//  - bis:PhysicalModel
//      - Except: BuildingPhysical:BuildingPhysicalModel, StructuralPhysical:StructuralPhysicalModel
//  - bis:InformationRecordModel
//  - bis:DocumentListModel
//  - bis:LinkModel
//  - bis:DefinitionModel
//      - Except:   BisCore:DictionaryModel, BisCore:RepositoryModel,
//                  BuildingPhysical:BuildingTypeDefinitionModel,
//                  RoadRailAlignment:ConfigurationModel, RoadRailAlignment:RoadRailCategoryModel,
//                  RoadRailPhysical:RailwayStandardsModel

ECObjectsStatus CheckForModelBaseClasses(ECClassCR baseClass, ECClassCR entity)
    {
    static bmap<Utf8String, bvector<Utf8String>> notSubClassable;
    notSubClassable["SpatialLocationModel"] = { "RoadRailAlignment:AlignmentModel", "RoadRailAlignment:AlignmentModel", "RoadRailAlignment:HorizontalAlignmentModel" };
    notSubClassable["PhysicalModel"] = { "BuildingPhysical:BuildingPhysicalModel", "StructuralPhysical:StructuralPhysicalModel" };
    notSubClassable["DefinitionModel"] = { "BisCore:DictionaryModel", "BisCore:RepositoryModel",
        "BuildingPhysical:BuildingTypeDefinitionModel", 
        "RoadRailAlignment:ConfigurationModel", "RoadRailAlignment:RoadRailCategoryModel",
        "RoadRailPhysical:RailwayStandardsModel" };
    notSubClassable["InformationRecordModel"] = {};
    notSubClassable["DocumentListModel"] = {};
    notSubClassable["LinkModel"] = {};

    if (baseClass.GetSchema().GetName().Equals(BISCORE_SCHEMA_NAME))
        {
        auto bfound = notSubClassable.find(baseClass.GetName().c_str());
        if (bfound != notSubClassable.end())
            {
            if (std::any_of(bfound->second.begin(), bfound->second.end(), [&entity](Utf8StringCR exception) { return exception.Equals(entity.GetFullName()); }))
                LOG.warningv("Entity class '%s' should not subclass 'bis:%s' but was released before the error was caught.", entity.GetFullName(), bfound->first.c_str());
            else
                {
                LOG.errorv("Entity class '%s' may not subclass bis:%s.", entity.GetFullName(), bfound->first.c_str());
                return ECObjectsStatus::Error;
                }
            }
        }

    const ECBaseClassesList& baseClassList = baseClass.GetBaseClasses();
    for (ECBaseClassesList::const_iterator iter = baseClassList.begin(); iter != baseClassList.end(); ++iter)
        {
        if (ECObjectsStatus::Success != CheckForModelBaseClasses(**iter, entity))
            return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckBisAspects(bool& entityDerivesFromSpecifiedClass, ECClassCR entity, Utf8CP derivedClassName, Utf8CP derivedRelationshipClassName)
    {
    if (ECClassModifier::Abstract == entity.GetClassModifier() || entity.GetName().Equals(derivedClassName) || !entity.Is(BISCORE_SCHEMA_NAME, derivedClassName))
        return ECObjectsStatus::Success;

    bool foundValidRelationshipConstraint = false;

    // There must be a relationship that derives from derivedClassName with this class as its constraint
    for (ECClassCP classInCurrentSchema : entity.GetSchema().GetClasses())
        {
        ECRelationshipClassCP relClass = classInCurrentSchema->GetRelationshipClassCP();
        if (nullptr == relClass)
            continue;

        if (!relClass->GetName().Equals(derivedRelationshipClassName) && relClass->Is(BISCORE_SCHEMA_NAME, derivedRelationshipClassName) &&
            (relClass->GetTarget().SupportsClass(entity)) && !relClass->GetTarget().GetConstraintClasses()[0]->GetName().Equals(derivedClassName))
            {
            foundValidRelationshipConstraint = true;
            break;
            }
        }

    entityDerivesFromSpecifiedClass = true;
    if (!foundValidRelationshipConstraint)
        {
        Utf8String errorMessage = Utf8PrintfString("Entity class derives from '%s' so it must be a supported target constraint in a relationship that derives from '%s'",
                                                   derivedClassName, derivedRelationshipClassName);
        if (!entity.GetSchema().IsDynamicSchema())
            {
            LOG.error(errorMessage.c_str());
            return ECObjectsStatus::Error;
            }
        else
            LOG.warning(errorMessage.c_str());
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

    // RULE: Root entity classes must derive from the bis hierarchy.
    auto const isBisCoreClass = [](ECClassCP entity) -> bool {return entity->GetSchema().GetName().Equals(BISCORE_SCHEMA_NAME);};
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

    bool foundIParentElement = false;
    bool foundISubModelElement = false;
    int numBaseClasses = 0;
    for (ECClassP baseClass : entity.GetBaseClasses())
        {
        if (!baseClass->GetEntityClassCP()->IsMixin())
            numBaseClasses++;

        if (0 == BeStringUtilities::Stricmp(baseClass->GetFullName(), BISCORE_CLASS_IParentElement))
            foundIParentElement = true;
        if (0 == BeStringUtilities::Stricmp(baseClass->GetFullName(), BISCORE_CLASS_ISubModeledElement))
            foundISubModelElement = true;

        // RULE: Class may not subclass certain BisCore Models
        if (ECObjectsStatus::Success != CheckForModelBaseClasses(*baseClass, entity))
            status = ECObjectsStatus::Error;

        // iterate through all inherited properties and check for duplicates
        for (ECPropertyP prop : baseClass->GetProperties(true))
            {
            bpair<ECPropertyP, ECClassP>* propertyClassPair = findProperty(prop);
            if (seenProperties.end() == propertyClassPair)
                seenProperties.push_back(make_bpair<ECPropertyP, ECClassP>(prop, baseClass));
            else
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
            }
        }

    if (numBaseClasses > 1)
        {
        LOG.errorv("Entity class '%s' inherits from more than one entity class", entity.GetFullName());
        status = ECObjectsStatus::Error;
        }

    // RULE: An Entity Class may not implement both bis:IParentElement and bis:ISubModeledElement.
    if (foundIParentElement && foundISubModelElement)
        {
        LOG.errorv("Entity class implements both bis:IParentElement and bis:ISubModeledElement. Entity Classes may implement bis:IParentElement or bis:ISubModeledElement but not both.");
        status = ECObjectsStatus::Error;
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
        LOG.errorv("Struct class has base classes, but structs should not have base classes.");
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
        LOG.errorv("Custom Attribute class has base classes, but a custom attribute class should not have base classes.");
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
            LOG.errorv("Relationship class has an 'embedding' strength with a forward direction so the source constraint may not have a multiplicity upper bound greater than 1.");
            returnStatus = ECObjectsStatus::Error;
            }
        // RULE: Relationship classes must not have a target constraint multiplicity upper bound greater than 1 if the strength is embedding and the direction is backward.
        else if (ECRelatedInstanceDirection::Backward == relClass->GetStrengthDirection() && relClass->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class has an 'embedding' strength with a backward direction so the target constraint may not have a multiplicity upper bound greater than 1.");
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
    if (constraint.IsAbstractConstraintDefined() && 1 == constraint.GetConstraintClasses().size())
        {
        LOG.errorv("Relationship class has an abstract constraint, '%s', and only one concrete constraint set in '%s'",
            constraint.GetAbstractConstraint()->GetFullName(), constraintType.c_str());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Aurora.Lane                  10/2018
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckEndpointForElementAspect(ECRelationshipConstraintCR endpoint, Utf8CP classname, Utf8CP constraintType)
    {
    ECClassCP abstractClass = endpoint.GetAbstractConstraint();
    if (nullptr != abstractClass && abstractClass->Is("BisCore", "ElementAspect"))
        {
        LOG.errorv("Relationship class '%s' has ElementAspect '%s' listed as a %s. ElementAspects should not be at the receiving end of relationships.",
                   classname, abstractClass->GetFullName(), constraintType);
        return ECObjectsStatus::Error;
        }
    return ECObjectsStatus::Success;
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
    if (ECObjectsStatus::Success != CheckLocalDefinitions(targetConstraint, EC_Target))
        status = ECObjectsStatus::Error;
    if (ECObjectsStatus::Success != CheckLocalDefinitions(sourceConstraint, EC_Source))
        status = ECObjectsStatus::Error;

    // RULE: ElementAspects should not be on receiving end of a relationship
    //       if not derived from ElementOwns.
    if (relClass->Is(BISCORE_SCHEMA_NAME, ElementOwnsUniqueAspect) || relClass->Is(BISCORE_SCHEMA_NAME, ElementOwnsMultiAspects))
        return status;

    if (ECRelatedInstanceDirection::Backward == relClass->GetStrengthDirection())
        {
        if (ECObjectsStatus::Success != CheckEndpointForElementAspect(sourceConstraint, relClass->GetFullName(), EC_Source))
            status = ECObjectsStatus::Error;
        }
    else
        {
        if (ECObjectsStatus::Success != CheckEndpointForElementAspect(targetConstraint, relClass->GetFullName(), EC_Target))
            status = ECObjectsStatus::Error;
        }
    
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
 