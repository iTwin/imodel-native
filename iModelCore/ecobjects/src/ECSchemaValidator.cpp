/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <functional>

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

#define CORECA_SCHEMA_NAME                  "CoreCustomAttributes"
#define CORECA_DEPRECATED                   "Deprecated"

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static bool TraverseBaseClasses(ECClassCR ecClass, std::function<bool(ECClassCP)> delegate, bool recursive)
    {
    const auto &baseList = ecClass.GetBaseClasses();
    if (baseList.empty())
        return false;

    for (ECClassCP base : baseList)
        {
        if (delegate(base))
            return true;
        if (!recursive && TraverseBaseClasses(*base, delegate, recursive))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECClassCP GetClassDefinedCustomAttribute(ECClassCR ecClass, Utf8StringCR schemaName, Utf8StringCR customAttributeName)
    {
    if (ecClass.IsDefinedLocal(schemaName, customAttributeName))
        return &ecClass;

    ECClassCP res = nullptr;
    auto CABaseGetter = [&](ECClassCP base)
        {
        bool found = false;
        if (base->IsDefinedLocal(schemaName, customAttributeName))
            {
            res = base;
            found = true;
            }
        return found;
        };

    TraverseBaseClasses(ecClass, CABaseGetter, true);
    return res;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunSchemaValidators(ECSchemaCR schema)
    {
    for (auto const& validator : m_schemaValidators)
        {
        if (ECObjectsStatus::Success != validator(schema))
            m_validated = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunClassValidators(ECClassCR ecClass)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, EC_Class, ecClass.GetName().c_str());
    bool classValidated = true;
    for (auto const& validator : m_classValidators)
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::RunKindOfQuantityValidators(KindOfQuantityCR koq)
    {
    LOG.debugv(FMTSTR_VALIDATION_START, EC_KOQ, koq.GetName().c_str());
    bool koqValidated = true;
    for (auto const& validator : m_koqValidators)
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
            if (!schema.IsDynamicSchema() || !refName.EqualsIAscii("ECv3ConversionAttributes"))
                {
                LOG.errorv("Failed to validate '%s' since it references '%s' using EC%" PRIu32 ".%" PRIu32 ". A schema may not reference any EC2 or EC3.0 schemas.",
                           schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str(), refSchema->GetOriginalECXmlVersionMajor(), refSchema->GetOriginalECXmlVersionMinor());
                 status = ECObjectsStatus::Error;
                }
            }

        // RULE: Schema Reference's alias must be the same as the schema alias
        auto refOriginalAlias = refSchema->GetAlias();
        Utf8String refCurrentAlias;
        if (schema.ResolveAlias(*refSchema, refCurrentAlias) == ECObjectsStatus::Success && !refCurrentAlias.Equals(refOriginalAlias))
            {
            LOG.errorv("Failed to validate '%s' since the alias '%s' defined for reference schema '%s' is different than the alias defined by the schema, '%s'.",
                        schema.GetFullSchemaName().c_str(), refCurrentAlias.c_str(), refSchema->GetFullSchemaName().c_str(), refOriginalAlias.c_str());
            status = ECObjectsStatus::Error;
            }

        // RULE: Warn about Schema using deprecated Schema Reference. No check if schema is already deprecated
        if (!schema.IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED) && refSchema->IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
            LOG.warningv("Schema '%s' references a deprecated schema, '%s'.", schema.GetFullSchemaName().c_str(), refSchema->GetFullSchemaName().c_str());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AllClassValidator(ECClassCR ecClass)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    // RULE: All classes should have a description (warn if not dynamic schema)
    if (ecClass.GetInvariantDescription().empty() && !ecClass.GetSchema().IsDynamicSchema())
        LOG.warningv("Class '%s' has no description. Please add a description.", ecClass.GetFullName());

    bool isModel = (!ecClass.GetSchema().GetName().EqualsI(BISCORE_SCHEMA_NAME)) && ecClass.Is(BISCORE_SCHEMA_NAME, "Model");

    // RULE: Class should not derived from a class or implements a mixin which is deprecated (warn). No check if class is already deprecated
    bool isThisClassDeprecated = ecClass.IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED);
    if (!isThisClassDeprecated)
        {
        // We only check the first base because all ECClasses except ECEntityClass will only have one base class. This is consitent with
        // typescript validator side as well where we check deprecated only for the main base of all ECClasses. We will check deprecated for mixin bases in the ECEntityClass rule
        const auto &baseList = ecClass.GetBaseClasses();
        auto mainBaseIter = baseList.begin();
        if (mainBaseIter != baseList.end())
            {
            auto mainBase = *mainBaseIter;
            auto deprecated = GetClassDefinedCustomAttribute(*mainBase, CORECA_SCHEMA_NAME, CORECA_DEPRECATED);
            if (deprecated)
                LOG.warningv("Class '%s' derived from a deprecated class, '%s'.",
                    ecClass.GetFullName(), deprecated->GetFullName());
            }
        }

    // RULE: Class should not use deprecated custom attributes. No check if class is already deprecated
    if (!isThisClassDeprecated)
        {
        for (const auto &customAttribute : ecClass.GetCustomAttributes(false))
            {
            const auto &customAttributeClass = customAttribute->GetClass();
            if (customAttributeClass.IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
                LOG.warningv("Class '%s' uses a deprecated custom attribute '%s'", ecClass.GetFullName(), customAttributeClass.GetFullName());
            }
        }

    // RULE: Properties should have a description (warn)
    // RULE: Properties should not be of type long.
    // RULE: All extended types of properties should be on the valid list
    // RULE: All Model subclasses outside of BisCore should not add properties
    //      Except: Raster:RasterModel.Clip, ScalableMesh:ScalableMeshModel.SmModelClips, ScalableMesh:ScalableMeshModel.SmGroundCoverages, ScalableMesh:ScalableMeshModel.SmModelClipVectors
    // RULE: Properties should not be deprecated (warn). No warning issued if class is deprecated.
    // RULE: Properties should not be of deprecated struct (warn). No warning issued if class is deprecated or property is deprecated.
    for (ECPropertyP prop : ecClass.GetProperties(false))
        {
        if (prop->GetInvariantDescription().empty() && !ecClass.GetSchema().IsDynamicSchema())
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

        if (prop->GetCustomAttributeLocal("CustomHandledProperty").IsValid() && ecClass.GetCustomAttributeLocal("ClassHasHandler").IsNull())
            {
            LOG.errorv("Property '%s.%s' cannot have CustomAttribute 'CustomHandledProperty'. Class '%s' must define CustomAttribute 'ClassHasHandler' and not derive it from a base class.",
                       ecClass.GetFullName(), prop->GetName().c_str(), ecClass.GetFullName());
            status = ECObjectsStatus::Error;
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

        if (!isThisClassDeprecated)
            {
            if (prop->IsDefined(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
                LOG.warningv("Class '%s' has property '%s' which is deprecated", ecClass.GetFullName(), prop->GetName().c_str());
            else if (prop->GetIsStruct() || prop->GetIsStructArray())
                {
                ECStructClassCP structClass;
                if (prop->GetIsStruct())
                    structClass = &(prop->GetAsStructProperty()->GetType());
                else
                    structClass = &(prop->GetAsStructArrayProperty()->GetStructElementType());

                if (structClass->IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
                    LOG.warningv("Class '%s' has property '%s' which is of a deprecated struct class, '%s'", ecClass.GetFullName(), prop->GetName().c_str(), structClass->GetFullName());
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
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    auto const findProperty = [&isPropertyMatch, &seenProperties] (ECPropertyP prop) -> auto
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

        // RULE: Class should not subclass deprecated mixin classes
        if (!entity.IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED) && !entity.IsMixin() && baseClass->IsMixin())
            {
            auto deprecated = GetClassDefinedCustomAttribute(*baseClass, CORECA_SCHEMA_NAME, CORECA_DEPRECATED);
            if (deprecated)
                LOG.warningv("Entity class '%s' derives from a deprecated mixin, '%s'.", entity.GetFullName(), deprecated->GetFullName());
            }

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
            auto propertyClassPair = findProperty(prop);
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
                        ECClassP mixinClass = prevIsMixin ? prevClass : baseClass;
                        ECClassP entityClass = prevIsMixin ? baseClass : prevClass;
                        if (prop->GetName().EqualsI("MODEL_NUMBER") && mixinClass->GetName().EqualsI("INSTRUMENT") && prevClass->GetSchema().GetFullSchemaName().StartsWithI("ProcessPhysical.01") &&
                            (entityClass->GetName().EqualsI("VALVE") || entityClass->GetName().EqualsI("PIPING_COMPONENT") || entityClass->GetName().EqualsI("SPACER") ||
                             entityClass->GetName().EqualsI("PIPING_AND_INSTRUMENT_COMPONENT")))
                            {
                            LOG.warningv("Warning at property '%s'. Mixin class '%s' overrides a property inherited from entity class '%s'.  Supppressed for some ProcessPhysical instruments.",
                                        prop->GetName().c_str(), mixinClass->GetFullName(), entityClass->GetFullName());
                            }
                        else
                            {
                            LOG.errorv("Error at property '%s'. Mixin class '%s' overrides a property inherited from entity class '%s'",
                                       prop->GetName().c_str(), mixinClass->GetFullName(), entityClass->GetFullName());
                            status = ECObjectsStatus::Error;
                            }
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECObjectsStatus CheckStrength(ECRelationshipClassCR relClass)
    {
    ECObjectsStatus returnStatus = ECObjectsStatus::Success;

    // RULE: Relationship classes must not have strength set to 'holding'.
    if (StrengthType::Holding == relClass.GetStrength())
        {
        LOG.errorv("Relationship class strength must not be set to 'holding'.");
        returnStatus = ECObjectsStatus::Error;
        }

    if (StrengthType::Embedding == relClass.GetStrength())
        {
        // RULE: Relationship classes must not have a source constraint multiplicity upper bound greater than 1 if the strength is embedding and the direction is forward.
        if (ECRelatedInstanceDirection::Forward == relClass.GetStrengthDirection() && relClass.GetSource().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class has an 'embedding' strength with a forward direction so the source constraint may not have a multiplicity upper bound greater than 1.");
            returnStatus = ECObjectsStatus::Error;
            }
        // RULE: Relationship classes must not have a target constraint multiplicity upper bound greater than 1 if the strength is embedding and the direction is backward.
        else if (ECRelatedInstanceDirection::Backward == relClass.GetStrengthDirection() && relClass.GetTarget().GetMultiplicity().GetUpperLimit() > 1)
            {
            LOG.errorv("Relationship class has an 'embedding' strength with a backward direction so the target constraint may not have a multiplicity upper bound greater than 1.");
            returnStatus = ECObjectsStatus::Error;
            }

        // RULE: Embedding relationships should not have 'Has' (case-sensitive) in the class name.
        //      Exception for all SP3D schemas.
        if (relClass.GetName().Contains("Has") || relClass.GetName().ContainsI("_has_"))
            {
            if (relClass.GetSchema().GetName().StartsWith("SP3D") || relClass.GetSchema().GetName().EqualsI("ProcessFunctional") || relClass.GetSchema().GetName().EqualsI("ProcessPhysical"))
                {
                LOG.warningv("Relationship class '%s' should not have 'Has' in its name but the rule is being ignored for all SP3D and ProcessFunctional schemas.", relClass.GetFullName());
                return returnStatus;
                }
            LOG.errorv("Relationship class '%s' has an 'embedding' strength and contains 'Has' in its name. Consider renaming this class.", relClass.GetFullName());
            returnStatus = ECObjectsStatus::Error;
            }
        }

    return returnStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static void CheckDeprecatedRelationshipConstraint(ECRelationshipConstraintCR endpoint, Utf8CP classname, Utf8CP constraintType)
    {
    // check abstract constraint class
    const auto abstractConstraint = endpoint.GetAbstractConstraint();
    if (abstractConstraint)
        {
        if (abstractConstraint->IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
            {
            LOG.warningv("%s Relationship Constraint of Relationship class '%s' has deprecated abstract constraint '%s'",
                constraintType, classname, abstractConstraint->GetFullName());
            }
        else
            {
            for (auto base : abstractConstraint->GetBaseClasses())
                {
                auto deprecated = GetClassDefinedCustomAttribute(*base, CORECA_SCHEMA_NAME, CORECA_DEPRECATED);
                if (deprecated)
                    LOG.warningv("%s Relationship Constraint of Relationship class '%s' has abstract constraint '%s' derived from a deprecated base, '%s'.",
                        constraintType, classname, abstractConstraint->GetFullName(), deprecated->GetFullName());
                }
            }
        }

    // check constraint classes
    for (auto constraintClass : endpoint.GetConstraintClasses())
        {
        if (constraintClass->IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
            {
            LOG.warningv("%s Relationship Constraint of Relationship class '%s' has deprecated constraint class '%s'",
                constraintType, classname, constraintClass->GetFullName());
            continue;
            }

        for (auto base : constraintClass->GetBaseClasses())
            {
            auto deprecated = GetClassDefinedCustomAttribute(*base, CORECA_SCHEMA_NAME, CORECA_DEPRECATED);
            if (deprecated)
                LOG.warningv("%s Relationship Constraint of Relationship class '%s' has constraint class '%s' derived from a deprecated base, '%s'.",
                    constraintType, classname, constraintClass->GetFullName(), deprecated->GetFullName());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    status = CheckStrength(*relClass);

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

    // RULE: Relationship class has constraint classes or mixin which is deprecated (warn). No check if relationship is already deprecated
    if (!ecClass.IsDefinedLocal(CORECA_SCHEMA_NAME, CORECA_DEPRECATED))
        {
        CheckDeprecatedRelationshipConstraint(sourceConstraint, relClass->GetFullName(), EC_Source);
        CheckDeprecatedRelationshipConstraint(targetConstraint, relClass->GetFullName(), EC_Target);
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::KindOfQuantityValidator(KindOfQuantityCR koq)
    {
    bool isProcessONE = koq.GetFullName().Equals("ProcessPhysical:ONE") || koq.GetFullName().Equals("ProcessFunctional:ONE");
    // RULE: Persistence unit of phenomenon 'PERCENTAGE' (or other unitless ratios) are not allowed.
    if (0 == strcmp(koq.GetPersistenceUnit()->GetPhenomenon()->GetName().c_str(), "PERCENTAGE"))
        {
        if (isProcessONE)
            LOG.warningv("KindOfQuantity %s has persistence unit of Phenomenon 'PERCENTAGE'. Unitless ratios are not allowed, but an exception has been made for this KOQ.", koq.GetFullName().c_str());
        else
            {
            LOG.errorv("KindOfQuantity %s has persistence unit of Phenomenon 'PERCENTAGE'. Unitless ratios are not allowed. Use a ratio phenomenon which includes units like VOLUME_RATIO", koq.GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        }

    // RULE: KindOfQuantity must have an SI unit for its persistence unit.
    if (0 != strcmp(koq.GetPersistenceUnit()->GetUnitSystem()->GetName().c_str(), "SI"))
        {
        if(isProcessONE)
            LOG.warningv("KindOfQuantity has persistence unit of unit system '%s' but must have an SI unit system, but an exception has been made for this KOQ.", koq.GetPersistenceUnit()->GetUnitSystem()->GetName().c_str());
        else
            {
            LOG.errorv("KindOfQuantity has persistence unit of unit system '%s' but must have an SI unit system", koq.GetPersistenceUnit()->GetUnitSystem()->GetName().c_str());
            return ECObjectsStatus::Error;
            }
        }

    // Add an exception for the already released versions of the AecUnits schema.
    // Since presentation formats can be changed without a write or read version bump, this exception will only apply to versions,
    // 1.0.0 and 1.0.1.
    static SchemaKey maxAllowedVersion("AecUnits", 1, 0, 2);
    bool isException = koq.GetFullName().Equals("AecUnits:LENGTH_SHORT") && koq.GetSchema().GetSchemaKey().LessThan(maxAllowedVersion, SchemaMatchType::Exact);

    ECObjectsStatus validationStatus = ECObjectsStatus::Success;

    // RULE: KindOfQuantity must not have duplicate presentation formats
    ECSchemaCR schema = koq.GetSchema();
    bset<Utf8String> uniqueFormats;
    const auto &presentFormats = koq.GetPresentationFormats();
    for (const auto &format : presentFormats)
        {
        auto formatQualifiedName = format.GetQualifiedFormatString(schema);
        auto resultInsertion = uniqueFormats.insert(formatQualifiedName);
        if (resultInsertion.second)
            continue;

        if (isException)
            LOG.warningv("KindOfQuantity '%s' has a duplicate presentation format '%s' which is not allowed but was released before the error was caught.", koq.GetFullName().c_str(), formatQualifiedName.c_str());
        else
            {
            LOG.errorv("KindOfQuantity '%s' has a duplicate presentation format %s which is not allowed.", koq.GetFullName().c_str(), formatQualifiedName.c_str());
            validationStatus = ECObjectsStatus::Error;
            }
        }

    return validationStatus;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
