/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static Utf8CP const  STANDARDVALUES_CUSTOMATTRIBUTE = "StandardValues";
static Utf8CP const  BECA_SCHEMANAME = "EditorCustomAttributes";
static Utf8CP const  BSCA_SCHEMANAME = "Bentley_Standard_CustomAttributes";
static Utf8CP const  ECDB_SCHEMANAME = "ECDbMap";

static Utf8CP const  UNIT_ATTRIBUTES                = "Unit_Attributes";
static Utf8CP const  KOQ_NAME                       = "KindOfQuantityName";
static Utf8CP const  DIMENSION_NAME                 = "DimensionName";
static Utf8CP const  UNIT_SPECIFICATION             = "UnitSpecificationAttr";
static Utf8CP const  UNIT_SPECIFICATIONS            = "UnitSpecifications";
static Utf8CP const  DISPLAY_UNIT_SPECIFICATION     = "DisplayUnitSpecificationAttr";
static Utf8CP const IS_UNIT_SYSTEM                  = "IsUnitSystemSchema";
static Utf8CP const MIXED_UNIT_SYSTEM               = "Mixed_UnitSystem";
static Utf8CP const SI_UNIT_SYSTEM                  = "SI_UnitSystem";
static Utf8CP const US_UNIT_SYSTEM                  = "US_UnitSystem";
static Utf8CP const PROPERTY_PRIORITY               = "PropertyPriority";
static Utf8CP const CATEGORY                        = "Category";
static Utf8CP const HIDE_PROPERTY                   = "HideProperty";
static Utf8CP const IF2D                            = "If2D";
static Utf8CP const IF3D                            = "If3D";
static Utf8CP const DISPLAY_OPTIONS                 = "DisplayOptions";
static Utf8CP const HIDDEN                          = "Hidden";
static Utf8CP const HIDE_INSTANCES                  = "HideInstances";

static Utf8CP const CLASS_MAP                       = "ClassMap";
static Utf8CP const MAP_STRATEGY                    = "MapStrategy";
static Utf8CP const STRATEGY                        = "MapStrategy.Strategy";
static Utf8CP const APPLIES_TO_SUBCLASS             = "MapStrategy.AppliesToSubclasses";
static Utf8CP const SHARED_TABLE                    = "SharedTable";
static Utf8CP const TABLE_PER_HIERARCHY             = "TablePerHierarchy";
static Utf8CP const NOT_MAPPED                      = "NotMapped";

static Utf8CP const HIDDEN_PROPERTY                 = "HiddenProperty";
static Utf8CP const SHOW                            = "Show";
static Utf8CP const HIDDEN_SCHEMA                   = "HiddenSchema";
static Utf8CP const HIDDEN_CLASS                    = "HiddenClass";

//=======================================================================================
//! ECSchemaConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
CustomECSchemaConverterP ECSchemaConverter::GetSingleton()
    {
    static CustomECSchemaConverterP converterSingleton = nullptr;

    if (nullptr == converterSingleton)
        {
        converterSingleton = new CustomECSchemaConverter();
        converterSingleton->SetRemoveLegacyStandardCustomAttributes(true);

        IECCustomAttributeConverterPtr scConv = new StandardValuesConverter();
        converterSingleton->AddConverter(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE, scConv);

        IECCustomAttributeConverterPtr priorityConv = new PropertyPriorityConverter();
        converterSingleton->AddConverter(BECA_SCHEMANAME, PROPERTY_PRIORITY, priorityConv);

        IECCustomAttributeConverterPtr categoryConv = new CategoryConverter();
        converterSingleton->AddConverter(BECA_SCHEMANAME, CATEGORY, categoryConv);

        IECCustomAttributeConverterPtr unitSchemaConv = new UnitSpecificationsConverter();
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS, unitSchemaConv);

        IECCustomAttributeConverterPtr unitSystemConv = new UnitSystemConverter();
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, IS_UNIT_SYSTEM, unitSystemConv);
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, MIXED_UNIT_SYSTEM, unitSystemConv);
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, SI_UNIT_SYSTEM, unitSystemConv);
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, US_UNIT_SYSTEM, unitSystemConv);

        IECCustomAttributeConverterPtr unitPropConv = new UnitSpecificationConverter();
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, UNIT_SPECIFICATION, unitPropConv);
        converterSingleton->AddConverter(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION, unitPropConv);

        converterSingleton->AddSchemaReferenceToRemove(UNIT_ATTRIBUTES);

        // Iterates over the Custom Attributes classes that will be converted. This converter basically
        // handles Custom Attributes that moved into a new schema but with no content change.
        auto const& mappingDictionary = StandardCustomAttributeReferencesConverter::GetCustomAttributesMapping();
        IECCustomAttributeConverterPtr standardClassConverter = new StandardCustomAttributeReferencesConverter();
        for (auto classMapping : mappingDictionary)
            converterSingleton->AddConverter(classMapping.first, standardClassConverter);

        IECCustomAttributeConverterPtr hideProp = new HidePropertyConverter();
        converterSingleton->AddConverter(BECA_SCHEMANAME, HIDE_PROPERTY, hideProp);
        IECCustomAttributeConverterPtr displayOpt = new DisplayOptionsConverter();
        converterSingleton->AddConverter(BSCA_SCHEMANAME, DISPLAY_OPTIONS, displayOpt);

        }

    return converterSingleton;
    }

//=======================================================================================
//! CustomECSchemaConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool CustomECSchemaConverter::Convert(ECSchemaR schema, bool doValidate)
    {
    m_convertedOK = true;

    auto classes = GetHierarchicallySortedClasses(schema);
    ConvertClassLevel(classes);

    ConvertPropertyLevel(classes);

    ConvertSchemaLevel(schema);

    RemoveSchemaReferences(schema);

    schema.RemoveUnusedSchemaReferences();

    if (m_convertedOK && doValidate)
        schema.Validate(true);

    return m_convertedOK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CustomECSchemaConverter::AddConverter(Utf8StringCR schemaName, Utf8StringCR customAttributeName, IECCustomAttributeConverterPtr& converter)
    {
    Utf8String converterName = GetQualifiedClassName(schemaName, customAttributeName);
    return AddConverter(converterName, converter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Stefan.Apfel                  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CustomECSchemaConverter::AddConverter(Utf8StringCR customAttributeKey, IECCustomAttributeConverterPtr& converter)
    {
    m_converterMap[customAttributeKey] = converter;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus CustomECSchemaConverter::RemoveConverter(Utf8StringCR customAttributeQualifiedName)
    {
    auto it = m_converterMap.find(customAttributeQualifiedName);
    if (it != m_converterMap.end())
        m_converterMap.erase(it);
    return ECObjectsStatus::Success;
    }

void GatherRootBaseClasses(ECClassCP ecClass, Utf8CP propertyName, bvector<ECClassP>& rootBaseClasses)
    {
    ECPropertyP ecProperty = ecClass->GetPropertyP(propertyName, false);
    if (nullptr != ecProperty && nullptr == ecProperty->GetBaseProperty())
        rootBaseClasses.push_back(const_cast<ECClassP>(ecClass));
    else
        for (auto const& baseClass : ecClass->GetBaseClasses())
            GatherRootBaseClasses(baseClass, propertyName, rootBaseClasses);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::FindRootBaseClasses(ECPropertyP ecProperty, bvector<ECClassP>& rootClasses)
    {
    GatherRootBaseClasses(&ecProperty->GetClass(), ecProperty->GetName().c_str(), rootClasses);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
bvector<ECClassP> CustomECSchemaConverter::GetDerivedAndBaseClasses(ECClassCR ecClass)
    {
    bvector<ECClassP> baseClasses = ecClass.GetBaseClasses();
    SortClassesByNameAndHierarchy(baseClasses, true);

    bvector<ECClassP> derivedClasses = ecClass.GetDerivedClasses();
    SortClassesByNameAndHierarchy(derivedClasses, true);

    baseClasses.insert(baseClasses.begin(), derivedClasses.begin(), derivedClasses.end());

    return baseClasses;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
IECCustomAttributeConverterP CustomECSchemaConverter::GetConverter(Utf8StringCR converterName)
    {
    auto iterator = m_converterMap.find(converterName);
    if (m_converterMap.end() != iterator)
        return iterator->second.get();
    return nullptr;
    }

static Utf8CP s_oldStandardSchemaNames[] =
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
    "USCustomaryUnitSystemDefaults"
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   06/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool IsCustomAttributeFromOldStandardSchemas(IECInstanceR customAttribute)
    {
    // Skip these for now... Once the conversions are added remove this check
    bvector<Utf8CP> oldCAs = { "HideProperty", "DisplayOptions", "CalculatedECPropertySpecification" };
    for (auto oldCA : oldCAs)
        if (0 == customAttribute.GetClass().GetName().CompareTo(oldCA))
            return false;

    SchemaKeyCR caSchemaKey = customAttribute.GetClass().GetSchema().GetSchemaKey();

    // Only the ECDbMap.1.0 is an old version
    if (0 == caSchemaKey.CompareByName("ECDbMap") && caSchemaKey.GetVersionRead() <= 1)
        return true;

    for (auto schemaName : s_oldStandardSchemaNames)
        if (0 == caSchemaKey.CompareByName(schemaName))
            return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::ProcessCustomAttributeInstance(ECCustomAttributeInstanceIterable iterable, IECCustomAttributeContainerR container, Utf8String containerName)
    {
    ECSchemaP schema = container.GetContainerSchema();
    for (auto const& attr : iterable)
        {
        Utf8CP fullName = attr->GetClass().GetFullName();
        IECCustomAttributeConverterP converter = GetConverter(fullName);
        if (nullptr != converter)
            {
            LOG.debugv("Started [%s Converter][Container %s]. ", fullName, containerName.c_str());
            auto status = converter->Convert(*schema, container, *attr, m_schemaContext.get());
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed [%s Converter][Container %s]. ", fullName, containerName.c_str());
                m_convertedOK = false;
                }
            else
                LOG.debugv("Succeeded [%s Converter][Container %s]. ", fullName, containerName.c_str());
            }
        else if (GetRemoveLegacyStandardCustomAttributes() && IsCustomAttributeFromOldStandardSchemas(*attr))
            {
            if (container.IsDefinedLocal(attr->GetClass()))
                m_convertedOK = container.RemoveCustomAttribute(attr->GetClass());
            else 
                m_convertedOK = container.RemoveSupplementedCustomAttribute(attr->GetClass());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                 5/2017
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::ProcessRelationshipConstraint(ECRelationshipConstraintR constraint, bool isSource)
    {
    if (!constraint.GetRelationshipClass().HasBaseClasses() || 0 != constraint.GetConstraintClasses().size())
        return;
    
    ECRelationshipClassCP baseRelClass = constraint.GetRelationshipClass().GetBaseClasses()[0]->GetRelationshipClassCP();
    ECRelationshipConstraintR baseConstraint = isSource ? baseRelClass->GetSource() : baseRelClass->GetTarget();

    // Need to set the abstract constraint, if one is needed, before adding constraint classes.
    if (1 < baseConstraint.GetConstraintClasses().size())
        {
        ECClassCP baseAbstractConstraint = baseConstraint.GetAbstractConstraint();
        (baseAbstractConstraint->IsEntityClass()) ? constraint.SetAbstractConstraint(*baseAbstractConstraint->GetEntityClassCP())
                                                : constraint.SetAbstractConstraint(*baseAbstractConstraint->GetRelationshipClassCP());
        }

    for (ECClassCP baseConstraintClass : baseConstraint.GetConstraintClasses())
        {
        (baseConstraintClass->IsEntityClass()) ? constraint.AddClass(*baseConstraintClass->GetEntityClassCP()) : 
                                                constraint.AddClass(*baseConstraintClass->GetRelationshipClassCP());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::ConvertClassLevel(bvector<ECClassP>& classes)
    {
    for (auto const& ecClass : classes)
        {
        ProcessCustomAttributeInstance(ecClass->GetCustomAttributes(false), *ecClass, "ECClass:" + ecClass->GetName());
        if (ecClass->IsRelationshipClass())
            {
            ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
            ECRelationshipConstraintR source = relClass->GetSource();
            ProcessCustomAttributeInstance(source.GetCustomAttributes(false), source, "ECRelationshipConstraint:" + source.GetRoleLabel());
            ProcessRelationshipConstraint(source, true);

            ECRelationshipConstraintR target = relClass->GetTarget();
            ProcessCustomAttributeInstance(target.GetCustomAttributes(false), target, "ECRelationshipConstraint:" + target.GetRoleLabel());
            ProcessRelationshipConstraint(target, false);
            }        
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::ConvertPropertyLevel(bvector<ECClassP>& classes)
    {
    for (auto const& ecClass : classes)
        {
        ECClassP nonConstClass = const_cast<ECClassP>(ecClass);
        for (auto const& ecProp : ecClass->GetProperties(false))
            {
            Utf8String debugName = Utf8String("ECProperty:") + ecClass->GetFullName() + Utf8String(".") + ecProp->GetName();
            ProcessCustomAttributeInstance(ecProp->GetCustomAttributes(false), *ecProp, debugName);

            // Need to make sure that property name does not conflict with one of the reserved system properties or aliases.
            Utf8CP thisName = ecProp->GetName().c_str();

            // ECInstanceId, Id, ECClassId are not allowed, except for structs.
            static bvector<Utf8CP> reservedNames{ "ECInstanceId", "Id", "ECClassId" };
            if (!ecClass->IsStructClass())
                {
                auto found = std::find_if(reservedNames.begin(), reservedNames.end(), [thisName](Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(thisName, reserved) == 0; });
                if (found != reservedNames.end())
                    {
                    LOG.warningv("The %s has a reserved property name. It is being renamed...", debugName.c_str());
                    ECPropertyP newProperty;
                    nonConstClass->RenameConflictProperty(ecProp, true, newProperty);
                    }
                }

            // SourceECInstanceId, SourceId, SourceECClassId, TargetECInstanceId, TargetId, TargetECClassId are not allowed on relationships(but are allowed on other types of classes)
            static bvector<Utf8CP> relationshipReservedName{ "SourceECInstanceId", "SourceId", "SourceECClassId", "TargetECInstanceId", "TargetId", "TargetECClassId" };
            if (ecClass->IsRelationshipClass())
                {
                auto found = std::find_if(relationshipReservedName.begin(), relationshipReservedName.end(), [thisName](Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(thisName, reserved) == 0; });
                if (found != relationshipReservedName.end())
                    {
                    LOG.warningv("The %s has a reserved property name. It is being renamed...", debugName.c_str());
                    ECPropertyP newProperty;
                    nonConstClass->RenameConflictProperty(ecProp, true, newProperty);
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                         4/2017
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::RemoveSchemaReferences(ECSchemaR schema)
    {
    for(Utf8StringCR schemaName : m_schemaReferencesToRemove)
        {
        SchemaKey key(schemaName.c_str(), 1, 0, 0);
        ECObjectsStatus status = schema.RemoveReferencedSchema(key, SchemaMatchType::Latest);
        if (ECObjectsStatus::Success == status || ECObjectsStatus::SchemaNotFound == status)
            continue;   // Schema successfully removed
        
        LOG.errorv("Failed to remove schema referenced for '%s' because it is still referenced by '%s'", schemaName.c_str(), schema.GetFullSchemaName().c_str());
        m_convertedOK = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::SortClassesByNameAndHierarchy(bvector<ECClassP>& ecClasses, bool reverse)
    {
    SortClassesByName(ecClasses, true);
    SortClassesByHierarchy(ecClasses);

    if (reverse)
        std::reverse(ecClasses.begin(), ecClasses.end());
    }

void AddClassesRootsFirst(bvector<ECClassP>& classList, const bvector<ECClassP>& classesToAdd, bmap<Utf8CP, ECClassP>& visitedClasses)
    {
    for (auto const& ecClass : classesToAdd)
        {
        auto it = visitedClasses.find(ecClass->GetName().c_str());
        if (it != visitedClasses.end())
            continue;

        if (!ecClass->HasBaseClasses())
            {
            visitedClasses.Insert(ecClass->GetName().c_str(), ecClass);
            classList.push_back(ecClass);
            continue;
            }

        AddClassesRootsFirst(classList, ecClass->GetBaseClasses(), visitedClasses);
        it = visitedClasses.find(ecClass->GetName().c_str());
        if (it != visitedClasses.end())
            continue;
        visitedClasses.Insert(ecClass->GetName().c_str(), ecClass);
        classList.push_back(ecClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::SortClassesByHierarchy(bvector<ECClassP>& ecClasses)
    {
    bvector<ECClassP> classes;
    bmap<Utf8CP, ECClassP> visited;
    AddClassesRootsFirst(classes, ecClasses, visited);
    
    ecClasses.assign(classes.begin(), classes.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomECSchemaConverter::SortClassesByName(bvector<ECClassP>& ecClasses, bool ascending)
    {
    auto classComparer = [](ECClassP ecClass1, ECClassP ecClass2)
        {
        return BeStringUtilities::StricmpAscii(ecClass1->GetName().c_str(), ecClass2->GetName().c_str()) < 0 ;
        };
    std::sort(ecClasses.begin(), ecClasses.end(), classComparer);
    if (!ascending)
        std::reverse(ecClasses.begin(), ecClasses.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
bvector<ECClassP> CustomECSchemaConverter::GetHierarchicallySortedClasses(ECSchemaR schema)
    {
    bvector<ECClassP> classes;
    Utf8String defaultOrder = "";
    for (auto const& ecClass : schema.GetClasses())
        {
        if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_TRACE))
            defaultOrder += ecClass->GetName() + ",";
        classes.push_back(ecClass);
        }

    if (classes.size() == 0)
        return classes;

    SortClassesByNameAndHierarchy(classes);
    Utf8String sortedOrder = "";
    if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_TRACE))
        {
        std::for_each(classes.begin(), classes.end(),
                      [&sortedOrder](ECClassP& ecClass)
            {
            sortedOrder += ecClass->GetName() + ",";
            });
        }

    LOG.tracev("Default Order For classes in schema: %s", defaultOrder.c_str());
    LOG.tracev("Name and Hierarchical Sorted Order For classes in schema: %s", sortedOrder.c_str());
    return classes;
    }

//=======================================================================================
//! ECDbClassMapConverter
//=======================================================================================

bool convertStrategyName(ECValueR strategy, IECInstanceCR classMap, Utf8StringR convertedStrategy)
    {
    if (strategy.IsNull() || !strategy.IsString())
        return false;
    Utf8CP originalStrategy = strategy.GetUtf8CP();
    if (0 == strcmp(originalStrategy, SHARED_TABLE))
        {
        ECValue appliesToSubClasses;
        if (ECObjectsStatus::Success != classMap.GetValue(appliesToSubClasses, APPLIES_TO_SUBCLASS) ||
            appliesToSubClasses.IsNull() || !appliesToSubClasses.IsBoolean() || !appliesToSubClasses.GetBoolean())
            {
            return false;
            }
        convertedStrategy = TABLE_PER_HIERARCHY;
        return true;
        }
    if (0 == strcmp(originalStrategy, NOT_MAPPED))
        {
        convertedStrategy = originalStrategy;
        return true;
        }
    return false;
    }

Utf8CP ECDbClassMapConverter::GetSchemaName() { return ECDB_SCHEMANAME; }
Utf8CP ECDbClassMapConverter::GetClassName() { return CLASS_MAP; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECObjectsStatus ECDbClassMapConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECClassP ecClass = dynamic_cast<ECClassP> (&container);
    if (instance.GetClass().GetSchema().GetVersionRead() > 1)
        {
        LOG.infov("Found ECDbMap:ClassMap custom attribute from schema version greater than 1.0.  Skipping because we're only trying to convert ClassMap custom attributes from ECDbMap 1.0");
        return ECObjectsStatus::Success;
        }
    container.RemoveCustomAttribute(ECDB_SCHEMANAME, CLASS_MAP);
    container.RemoveSupplementedCustomAttribute(ECDB_SCHEMANAME, CLASS_MAP);

    if (nullptr == ecClass)
        {
        LOG.warningv("ECDbMap:ClassMap custom attribute applied to a container which is not a class.  Removing Custom Attribute from %s and skipping.", container.GetContainerName());
        return ECObjectsStatus::Success;
        }
    
    Utf8String convertedStrategyName;
    ECValue strategy;
    if (ECObjectsStatus::Success != instance.GetValue(strategy, STRATEGY) || !convertStrategyName(strategy, instance, convertedStrategyName))
        {
        LOG.warningv("Failed to convert ECDbMap:ClassMap on %s because the MapStrategy is not 'SharedTable with AppliesToSubclasses == true' or 'NotMapped'.  Removing and skipping.", container.GetContainerName());
        return ECObjectsStatus::Success;
        }

    if (nullptr == context)
        {
        BeAssert(true);
        LOG.error("Missing ECSchemaReadContext, it is necessary to perform conversion on a ECDbMap:ClassMap custom attribute.");
        return ECObjectsStatus::Error;
        }

    static SchemaKey key(ECDB_SCHEMANAME, 2, 0, 0);
    auto ecdbMapSchema = ECSchema::LocateSchema(key, *context);
    if (ecdbMapSchema.IsNull() || !ecdbMapSchema.IsValid())
        {
        LOG.errorv("Failed to convert ECDbMap::ClassMap on %s because the ECDbMap 2.0.0 schema could not be found.  Removing and skipping.", container.GetContainerName());
        return ECObjectsStatus::Error;
        }
        
    IECInstancePtr classMap = ecdbMapSchema->GetClassCP(CLASS_MAP)->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue convertedMapStrategy(convertedStrategyName.c_str());
    classMap->SetValue(MAP_STRATEGY, convertedMapStrategy);
    schema.AddReferencedSchema(*ecdbMapSchema);
    container.SetCustomAttribute(*classMap);

    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! StandardValueInfo
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool StandardValueInfo::Equals(const StandardValueInfo& sd) const
    {
    //let them be equal even if mustBeFromList is different
    if (m_valuesMap.size() != sd.m_valuesMap.size())
        return false;
    for (auto const& pair : m_valuesMap)
        {
        auto it = sd.m_valuesMap.find(pair.first);
        if (it == sd.m_valuesMap.end())
            return false;

        if (!it->second.EqualsI(pair.second))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Aurora.Lane                    01/2019
//+---------------+---------------+---------------+---------------+---------------+------
bool StandardValueInfo::Equals(ECEnumerationCP ecEnum) const
    {
    if (m_mustBeFromList == false || m_mustBeFromList != ecEnum->GetIsStrict())
        return false;
    if ((size_t) (m_valuesMap.size()) != ecEnum->GetEnumeratorCount())
        return false;
    for (auto const& enumerator : ecEnum->GetEnumerators())
        {
        auto it = m_valuesMap.find(enumerator->GetInteger());
        if (it == m_valuesMap.end())
            return false;
        if (0 != BeStringUtilities::Stricmp(it->second.c_str(), enumerator->GetDisplayLabel().c_str()))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Aurora.Lane                    01/2019
//+---------------+---------------+---------------+---------------+---------------+------
// Determines if ECEnumeration contains SVI's enumerators without unnecessary conversion
bool StandardValueInfo::ContainedBy(ECEnumerationCP ecEnum) const
    {
    if ((size_t) (m_valuesMap.size()) > ecEnum->GetEnumeratorCount())
        return false;
    for (auto const& valuePair : m_valuesMap)
        {
        auto const pair = ecEnum->FindEnumerator(valuePair.first);
        if (nullptr == pair)
            return false;
        if (0 != BeStringUtilities::Stricmp(valuePair.second.c_str(), pair->GetDisplayLabel().c_str()))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   09/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool StandardValueInfo::Contains(const StandardValueInfo& sd) const
    {
    for (auto const& pair : sd.m_valuesMap)
        {
        auto it = m_valuesMap.find(pair.first);
        if (it == m_valuesMap.end() || !it->second.Equals(pair.second))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
StandardValueInfo::StandardValueInfo(ECEnumerationP& ecEnum)
    {
    m_mustBeFromList = ecEnum->GetIsStrict();
    for (auto const& enumerator : ecEnum->GetEnumerators())
        m_valuesMap[enumerator->GetInteger()] = enumerator->GetDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                   02/2017
//+---------------+---------------+---------------+---------------+---------------+------
StandardValueInfo::StandardValueInfo(ECEnumerationCR ecEnum)
    {
    m_mustBeFromList = ecEnum.GetIsStrict();
    for (auto const& enumerator : ecEnum.GetEnumerators())
        m_valuesMap[enumerator->GetInteger()] = enumerator->GetDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValueInfo::ExtractInstanceData(IECInstanceR instance, StandardValueInfo& sdInfo)
    {
    sdInfo.m_mustBeFromList = true; //default is true
    ECValue value;
    if (ECObjectsStatus::Success == instance.GetValue(value, "MustBeFromList")
        && !value.IsNull() && value.IsBoolean())
        {
        sdInfo.m_mustBeFromList = value.GetBoolean();
        }

    Utf8String accessString = "ValueMap";
    ECObjectsStatus status;
    status = instance.GetValue(value, accessString.c_str());
    if (ECObjectsStatus::Success != status)
        return status;

    uint32_t arraySize = value.GetArrayInfo().GetCount();
    for (uint32_t i = 0; i < arraySize; i++)
        {
        status = instance.GetValue(value, accessString.c_str(), i);
        if (ECObjectsStatus::Success != status && !value.IsStruct())
            return status;

        IECInstancePtr  structInstance = value.GetStruct();
        if (!structInstance.IsValid())
            return ECObjectsStatus::Error;

        if (ECObjectsStatus::Success != (status = structInstance->GetValue(value, "Value")) || !value.IsLoaded() || value.IsNull())
            return status;
        int index = value.GetInteger();

        if (ECObjectsStatus::Success != (status = structInstance->GetValue(value, "DisplayString")) || !value.IsLoaded() || value.IsNull())
            return status;
        sdInfo.m_valuesMap[index] = value.ToString();
        }
    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! StandardValuesConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::Merge(ECPropertyP prop, StandardValueInfo* sdInfo, ECEnumerationP enumeration)
    {
    IECInstancePtr currentInstance = prop->GetCustomAttributeLocal(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
    if (!currentInstance.IsValid())
        return ECObjectsStatus::Success;

    StandardValueInfo localInfo;
    StandardValueInfo::ExtractInstanceData(*currentInstance, localInfo);

    // They are the same, no reason to try and merge
    if (sdInfo->Equals(localInfo))
        return ECObjectsStatus::Success;

    if (sdInfo->m_mustBeFromList || localInfo.m_mustBeFromList)
        return ECObjectsStatus::DataTypeMismatch;

    // Check if there are any conflicts and also build up a list of non-conflicting items
    bmap<int, Utf8String> nonConflictingValues;
    for (auto const& pair : localInfo.m_valuesMap)
        {
        auto it = sdInfo->m_valuesMap.find(pair.first);
        if (it == sdInfo->m_valuesMap.end())
            // Attempt to add to enumeration here, possibly within another method to avoid having 2 loops
            nonConflictingValues[pair.first] = pair.second;
        else if (!it->second.Equals(pair.second))
            return ECObjectsStatus::DataTypeMismatch;
        }

    for (auto const& pair : nonConflictingValues)
        {
        ECEnumeratorP enumerator;
        enumeration->CreateEnumerator(enumerator, ECEnumerator::DetermineName(enumeration->GetName(), nullptr, &pair.first), pair.first);
        enumerator->SetDisplayLabel(pair.second.c_str());

        // Add it to the sdInfo so that subsequent classes can be compared against it. 
        sdInfo->m_valuesMap[pair.first] = pair.second;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::ConvertToEnum(ECClassP rootClass, ECClassP currentClass, Utf8CP propName, ECEnumerationP enumeration, StandardValueInfo sdInfo)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    ECPropertyP prop = currentClass->GetPropertyP(propName, false);
    if (nullptr != prop)
        {
        if (!prop->GetIsPrimitive() && !prop->GetIsPrimitiveArray())
            {
            LOG.errorv("Failed to convert to enumeration because the derived property %s.%s is not a primitive property but the base property from class %s is.",
                       currentClass->GetFullName(), propName, rootClass->GetFullName());
            return ECObjectsStatus::DataTypeMismatch;
            }

        if (ECObjectsStatus::Success != (status = Merge(prop, &sdInfo, enumeration)))
            {
            // If the current class is derived from the root class check if the derived standard value is a subset of the base.
            if (ECClass::ClassesAreEqualByName(rootClass, currentClass) || !currentClass->Is(rootClass))
                {
                LOG.errorv("Failed to convert to enumeration because the derived property %s.%s has a StandardValues attribute that does not match the base property from class %s.",
                           currentClass->GetFullName(), propName, rootClass->GetFullName());
                return status;
                }
                
            IECInstancePtr currentInstance = prop->GetCustomAttributeLocal(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
            if (!currentInstance.IsValid())
                return ECObjectsStatus::Success;

            StandardValueInfo localInfo;
            StandardValueInfo::ExtractInstanceData(*currentInstance, localInfo);
            // Check if the localInfo is just a subset before testing if it should be strict.
            // This is to get around the issue where a derived property has less standard values, but all of the ones it has are the same as the base class.
            // Therefore, we just set the derived property to the base property's Enumeration.
            if (!sdInfo.Contains(localInfo))
                {
                LOG.errorv("Failed to convert to enumeration because the derived property %s.%s has a StandardValues attribute that does not match the base property from class %s.",
                            currentClass->GetFullName(), propName, rootClass->GetFullName());
                return status;
                }
            else
                {
                LOG.warningv("The property %s.%s has a StandardValues attribute that is a subset of its base property %s.%s. Using the enumeration, %s, even though it isn't an exact match.",
                             currentClass->GetFullName(), propName, rootClass->GetFullName(), propName, enumeration->GetFullName().c_str());
                status = ECObjectsStatus::Success;
                }
            }
        }

    for (auto const& derivedClass : currentClass->GetDerivedClasses())
        {
        if (ECObjectsStatus::Success != (status = ConvertToEnum(rootClass, derivedClass, propName, enumeration, sdInfo)))
            {
            if (ECObjectsStatus::DataTypeMismatch == status)
                {
                ECClassP nonConstClass = const_cast<ECClassP>(derivedClass);
                ECPropertyP derivedProperty = nonConstClass->GetPropertyP(propName);
                ECPropertyP renamedProperty = nullptr;
                if (ECObjectsStatus::Success != derivedClass->RenameConflictProperty(derivedProperty, true, renamedProperty))
                    return status;
                else
                    {
                    IECInstancePtr renamedPropInstance = renamedProperty->GetCustomAttributeLocal(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
                    if (!renamedPropInstance.IsValid())
                        return ECObjectsStatus::Success;

                    if (ECObjectsStatus::Success != (status = Convert(nonConstClass->GetSchemaR(), *renamedProperty, *renamedPropInstance)))
                        return status;
                    }
                }
            else
                return status;
            }
        }
    
    if (nullptr != prop && ECObjectsStatus::Success == status)
        {
        PrimitiveECPropertyP primitive = prop->GetAsPrimitivePropertyP();
        PrimitiveArrayECPropertyP primitiveArray = prop->GetAsPrimitiveArrayPropertyP();

        ECEnumerationCP primitiveEnumeration = (nullptr != primitive) ? primitive->GetEnumeration() : primitiveArray->GetEnumeration();

        if (nullptr == primitiveEnumeration)
            status = (nullptr != primitive) ? primitive->SetType(*enumeration) : primitiveArray->SetType(*enumeration);
        else if (primitiveEnumeration != enumeration)
            {
            LOG.errorv("Failed to convert to enumeration because the derived property %s.%s already has an ECEnumeration '%s' as its type but it is not the same as the type '%s' from the base property in class %s.%s",
                       currentClass->GetFullName(), propName, primitiveEnumeration->GetFullName().c_str(), enumeration->GetFullName().c_str(), rootClass->GetFullName(), propName);
            return ECObjectsStatus::DataTypeMismatch;
            }
        
        prop->RemoveCustomAttribute(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
        prop->RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
        }
    
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    //are standard values even added to ecclass or ecschema?
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (nullptr == prop)
        {
        LOG.warningv("StandardValues custom attribute applied to a container which is not a property.  Removing Custom Attribute from %s and skipping.", container.GetContainerName());
        container.RemoveCustomAttribute(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
        return ECObjectsStatus::Success;
        }

    ECObjectsStatus status;
    StandardValueInfo sdInfo;
    if (ECObjectsStatus::Success != (status = StandardValueInfo::ExtractInstanceData(instance, sdInfo)))
        {
        LOG.errorv("Unable to extract '%s' Standard Value. Status %d", prop->GetName().c_str(), status);
        return status;
        }

    // Probably is not the best way to do this, but need to check if the property has been previously converted.
    // Run into the issue where a supplemental schema wasn't previously present when the property was converted if the property was not
    // in the primary schema initially
    PrimitiveArrayECPropertyP primArrProp = prop->GetAsPrimitiveArrayPropertyP();
    PrimitiveECPropertyP primProp = prop->GetAsPrimitivePropertyP();
    ECEnumerationCP existingEnum = (nullptr == primArrProp) ? primProp->GetEnumeration() : primArrProp->GetEnumeration();
    if (nullptr != existingEnum)
        {
        if (sdInfo.Equals(existingEnum) || sdInfo.ContainedBy(existingEnum))
            {
            // Already successfully converted
            prop->RemoveCustomAttribute(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
            prop->RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, STANDARDVALUES_CUSTOMATTRIBUTE);
            return ECObjectsStatus::Success;
            }
        }
    
    // We traverse from most base to most derived class so if we come across a property with a StandardValues CA AND a base prop we must 
    // set from list to false because we're going to change the entire property hierarchy to be an enum.
    if (nullptr != prop->GetBaseProperty())
        sdInfo.m_mustBeFromList = false;

    bvector<ECClassP> rootClasses;
    ECEnumerationP enumeration;
    if (ECObjectsStatus::Success != FindEnumeration(schema, enumeration, sdInfo))
        {
        Utf8String enumName = CreateEnumerationName(rootClasses, prop);
        status = CreateEnumeration(enumeration, rootClasses[0]->GetSchemaR(), enumName.c_str(), sdInfo);
        int number = 1;
        while (ECObjectsStatus::NamedItemAlreadyExists == status)
            {
            Utf8String newEnumName = AppendNumberToString(enumName.c_str(), number);
            status = CreateEnumeration(enumeration, rootClasses[0]->GetSchemaR(), newEnumName.c_str(), sdInfo);
            number++;
            if (number > 420)
                {
                LOG.errorv("Failed to create enumeration '%s' for property '%s.%s' because a unique name could not be determined in %d",
                           enumName.c_str(), rootClasses[0]->GetName().c_str(), prop->GetName().c_str(), number);
                return ECObjectsStatus::NamedItemAlreadyExists;
                }
            }
        }
    else
        ECSchemaConverter::FindRootBaseClasses(prop, rootClasses);

    status = ConvertToEnum(rootClasses[0], rootClasses[0], prop->GetName().c_str(), enumeration, sdInfo);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Failed to convert StandardValues to ECEnumeration on property %s.%s.", rootClasses[0]->GetName().c_str(), prop->GetName().c_str());
        schema.DeleteEnumeration(*enumeration);
        return status;
        }

    // Attempt to convert all other root classes to the Enum
    for (size_t i = 1; i < rootClasses.size(); ++i)
        {
        status = ConvertToEnum(rootClasses[i], rootClasses[i], prop->GetName().c_str(), enumeration, sdInfo);
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed to convert StandardValues to ECEnumeration on property %s.%s.", rootClasses[0]->GetName().c_str(), prop->GetName().c_str());
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StandardValuesConverter::CreateEnumerationName(bvector<ECClassP>& rootClasses, ECPropertyP& ecProperty)
    {
    ECSchemaConverter::FindRootBaseClasses(ecProperty, rootClasses);
    Utf8String baseClassName = rootClasses[0]->GetName();
    Utf8String propertyName = ecProperty->GetName();

    return baseClassName + "_" + propertyName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StandardValuesConverter::AppendNumberToString(Utf8CP name, int number)
    {
    Utf8PrintfString formattedString("%s_%" PRId32, name, number);
    return formattedString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                    09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::MergeEnumeration(ECEnumerationP& enumeration, StandardValueInfo& sdInfo)
    {
    for (auto const& pair : sdInfo.m_valuesMap)
        {
        ECEnumeratorP enumerator = enumeration->FindEnumerator(pair.first);
        if (enumerator == nullptr)
            {
            enumeration->CreateEnumerator(enumerator, ECEnumerator::DetermineName(enumeration->GetName(), nullptr, &pair.first), pair.first);
            enumerator->SetDisplayLabel(pair.second.c_str());
            }
        else if (enumerator->GetInvariantDisplayLabel() != pair.second)
            return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                    09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::FindEnumeration(ECSchemaR schema, ECEnumerationP& enumeration, StandardValueInfo& sdInfo)
    {
    for (auto ecEnum : schema.GetEnumerations())
        {
        if ((PrimitiveType::PRIMITIVETYPE_Integer != ecEnum->GetType())
            || (sdInfo.m_mustBeFromList != ecEnum->GetIsStrict()))
            continue;

        if (sdInfo.Equals(ecEnum) || (sdInfo.ContainedBy(ecEnum)))
            {
            enumeration = ecEnum;
            return ECObjectsStatus::Success;
            }
        else if (!sdInfo.m_mustBeFromList && ECObjectsStatus::Success == MergeEnumeration(ecEnum, sdInfo))
            {
            enumeration = ecEnum;
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::EnumerationNotFound;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::CreateEnumeration(ECEnumerationP& enumeration, ECSchemaR schema, Utf8CP enumName, StandardValueInfo& sdInfo)
    {
    ECObjectsStatus status = schema.CreateEnumeration(enumeration, enumName, PrimitiveType::PRIMITIVETYPE_Integer);
    if (ECObjectsStatus::Success != status)
        return status;
    
    enumeration->SetIsStrict(sdInfo.m_mustBeFromList);
    ECEnumeratorP enumerator;

    for (auto const& pair : sdInfo.m_valuesMap)
        {
        status = enumeration->CreateEnumerator(enumerator, ECEnumerator::DetermineName(enumeration->GetName(), nullptr, &pair.first), pair.first);
        if (ECObjectsStatus::Success != status)
            return status;

        enumerator->SetDisplayLabel(pair.second.c_str());
        }
    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! UnitSpecificationConverter
//=======================================================================================

struct UnitSpecification
    {
    UnitSpecification() = delete;
    static bool TryGetNewKOQName(ECPropertyCR ecprop, Utf8StringR newKindOfQuantityName)
        {
        IECInstancePtr instance = ecprop.GetCustomAttributeLocal(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
        if (instance.IsValid())
            return TryGetStringValue(*instance, KOQ_NAME, newKindOfQuantityName) || TryGetStringValue(*instance, DIMENSION_NAME, newKindOfQuantityName);
        return false;
        }

    static bool TryGetStringValue(IECInstanceCR instance, Utf8CP propName, Utf8StringR stringValue)
        {
        ECValue v;
        if (ECObjectsStatus::Success == instance.GetValue(v, propName) && !v.IsNull() && !Utf8String::IsNullOrEmpty(v.GetUtf8CP()))
            {
            stringValue = v.GetUtf8CP();
            return true;
            }
        
        return false;
        }
    };

ECObjectsStatus addOldUnitCustomAttribute(ECSchemaR schema, ECPropertyP prop, Utf8CP oldUnitName)
    {
    ECObjectsStatus status;

    IECInstancePtr oldUnitInstance = ConversionCustomAttributeHelper::CreateCustomAttributeInstance("OldPersistenceUnit");
    ECValue oldUnitNameValue(oldUnitName);
    oldUnitInstance->SetValue("Name", oldUnitNameValue);

    if (!ECSchema::IsSchemaReferenced(schema, oldUnitInstance->GetClass().GetSchema()))
        {
        ECClassP nonConstClass = const_cast<ECClassP>(&oldUnitInstance->GetClass());
        if (ECObjectsStatus::Success != (status = schema.AddReferencedSchema(nonConstClass->GetSchemaR())))
            {
            LOG.warningv("Failed to add %s as a referenced schema to %s.", oldUnitInstance->GetClass().GetSchema().GetName().c_str(), schema.GetName().c_str());
            LOG.warningv("Failed to add 'OldPersistenceUnit' custom attribute to ECProperty '%s.%s'.", prop->GetClass().GetFullName(), prop->GetName().c_str());
            return status;
            }
        }

    status = prop->SetCustomAttribute(*oldUnitInstance);

    return status;
    }

bool kindOfQuantityHasMatchingPersitenceUnit(KindOfQuantityCP koq, Units::UnitCP unit)
    {
    if (nullptr == koq)
        return true;

    return Units::Unit::AreEqual(koq->GetPersistenceUnit(), unit);
    }

bool kindOfQuantityHasMatchingPresentationUnit(KindOfQuantityCP koq, Units::UnitCP displayUnit, Units::UnitCP persistenceUnit)
    {
    if (koq->GetDefaultPresentationFormat()->HasCompositeMajorUnit())
        return Units::Unit::AreEqual(koq->GetDefaultPresentationFormat()->GetCompositeMajorUnit(), (nullptr == displayUnit) ? persistenceUnit : displayUnit);
    return false; 
    }

bool unitIsAcceptable (Units::UnitCP unit)
    {
    return  unit->IsSI() ||
            0 == strcmp(unit->GetPhenomenon()->GetName().c_str(), "PERCENTAGE") ||
            0 == strcmp(unit->GetPhenomenon()->GetName().c_str(), "NUMBER") ||
            0 == strcmp(unit->GetPhenomenon()->GetName().c_str(), "CURRENCY");
    }

ECObjectsStatus createNewKindOfQuantity(ECSchemaR schema, KindOfQuantityP& newKOQ, KindOfQuantityCP baseKOQ, ECUnitCP newUnit, ECUnitCP newDisplayUnit, bool& persistenceUnitChanged, Utf8CP newKoqName)
    {
    ECObjectsStatus status = schema.CreateKindOfQuantity(newKOQ, newKoqName);
    if (ECObjectsStatus::Success != status)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Failed to create new KindOfQuantity in schema %s with name %s.", fullName.c_str(), newKoqName);
        return status;
        }

    ECUnitCP originalUnit = newUnit;
    if (!unitIsAcceptable(newUnit)) 
        {
        newUnit = newUnit->GetPhenomenon()->GetSIUnit();
        if (nullptr == newUnit)
            {
            Utf8String fullName = schema.GetFullSchemaName();
            LOG.warningv("Failed to resolve SI unit for %s while converting KOQ %s in schema %s", originalUnit->GetName().c_str(), newKoqName, fullName.c_str());
            return ECObjectsStatus::Error;
            }
        persistenceUnitChanged = true;
        }

    if (kindOfQuantityHasMatchingPersitenceUnit(baseKOQ, newUnit))
        {
        newKOQ->SetPersistenceUnit(*newUnit);
        newKOQ->SetRelativeError(1e-4);
        }
    else
        {
        newKOQ->SetPersistenceUnit((ECUnitCR) *baseKOQ->GetPersistenceUnit());
        newKOQ->SetRelativeError(baseKOQ->GetRelativeError());
        persistenceUnitChanged = true;
        }

    if (nullptr != newDisplayUnit)
        {
        if (ECObjectsStatus::Success != newKOQ->AddPresentationFormatSingleUnitOverride(*schema.LookupFormat("f:DefaultRealU"), nullptr, newDisplayUnit))
            LOG.warningv("During conversion on KOQ '%s' failed to add unit '%s' as a presentation format override", newKOQ->GetFullName().c_str(), newDisplayUnit->GetFullName().c_str());
        }
    else if (persistenceUnitChanged)
        { 
        if (ECObjectsStatus::Success != newKOQ->AddPresentationFormatSingleUnitOverride(*schema.LookupFormat("f:DefaultRealU"), nullptr, originalUnit))
            LOG.warningv("During conversion on KOQ '%s' failed to add unit '%s' as a presentation format override", newKOQ->GetFullName().c_str(), originalUnit->GetFullName().c_str());
        }

    return ECObjectsStatus::Success;
    }

bool baseAndNewUnitAreIncompatible(KindOfQuantityCP baseKOQ, ECUnitCP newUnit)
    {
    if (nullptr == baseKOQ)
        return false;
    return !Units::Unit::AreCompatible(baseKOQ->GetPersistenceUnit(), newUnit);
    }

bool kindOfQuantityIsAcceptable(KindOfQuantityCP newKOQ, KindOfQuantityCP baseKOQ, ECUnitCP newUnit, ECUnitCP newDisplayUnit, bool& persistenceUnitChanged)
    {
    if (!kindOfQuantityHasMatchingPersitenceUnit(baseKOQ, newKOQ->GetPersistenceUnit()))
        return false;

    if (!unitIsAcceptable(newUnit) && !baseAndNewUnitAreIncompatible(newKOQ, newUnit))
        {
        persistenceUnitChanged = true;
        return kindOfQuantityHasMatchingPresentationUnit(newKOQ, newDisplayUnit, newUnit);
        }

    if (!kindOfQuantityHasMatchingPersitenceUnit(newKOQ, newUnit))
        return false;

    return kindOfQuantityHasMatchingPresentationUnit(newKOQ, newDisplayUnit, newUnit);
    }

ECObjectsStatus obtainKindOfQuantity(ECSchemaR schema, ECPropertyP prop, KindOfQuantityP& newKOQ, IECInstanceR unitSpecCA, ECUnitCP newUnit, ECUnitCP newDisplayUnit, bool& persistenceUnitChanged, Utf8CP newKoqName)
    {
    persistenceUnitChanged = false;
    KindOfQuantityCP baseKOQ = prop->GetKindOfQuantity();
    if (baseAndNewUnitAreIncompatible(baseKOQ, newUnit))
        {
        LOG.errorv("Cannot convert UnitSpecification on '%s.%s' because the base property unit '%s' is not compatible with this properties unit '%s'",
                   prop->GetClass().GetFullName(), prop->GetName().c_str(), baseKOQ->GetPersistenceUnit()->GetName().c_str(), newUnit->GetName().c_str());
        return ECObjectsStatus::KindOfQuantityNotCompatible;
        }

    newKOQ = schema.GetKindOfQuantityP(newKoqName);
    if (nullptr == newKOQ)
        {
        if (ECObjectsStatus::Success == createNewKindOfQuantity(schema, newKOQ, baseKOQ, newUnit, newDisplayUnit, persistenceUnitChanged, newKoqName))
            return ECObjectsStatus::Success;
        }
    else if (kindOfQuantityIsAcceptable(newKOQ, baseKOQ, newUnit, newDisplayUnit, persistenceUnitChanged))
        return ECObjectsStatus::Success;

    Utf8PrintfString newKoqNameWithClass("%s_%s", newKoqName, prop->GetClass().GetName().c_str());
    newKOQ = schema.GetKindOfQuantityP(newKoqNameWithClass.c_str());
    if (nullptr == newKOQ)
        {
        if (ECObjectsStatus::Success == createNewKindOfQuantity(schema, newKOQ, baseKOQ, newUnit, newDisplayUnit, persistenceUnitChanged, newKoqNameWithClass.c_str()))
            return ECObjectsStatus::Success;
        }
    else if (kindOfQuantityIsAcceptable(newKOQ, baseKOQ, newUnit, newDisplayUnit, persistenceUnitChanged))
        return ECObjectsStatus::Success;

    Utf8PrintfString newKoqNameWithClassAndProperty("%s_%s", newKoqNameWithClass.c_str(), prop->GetName().c_str());
    return createNewKindOfQuantity(schema, newKOQ, baseKOQ, newUnit, newDisplayUnit, persistenceUnitChanged, newKoqNameWithClassAndProperty.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void removePropertyUnitCustomAttributes(IECCustomAttributeContainerR container, Utf8StringCR schemaName, Utf8StringCR className)
    {
    container.RemoveCustomAttribute(schemaName, className);
    container.RemoveSupplementedCustomAttribute(schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus UnitSpecificationConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        LOG.warningv("Found UnitSpecification or DisplayUnitSpecification custom attribute on a container which is not a property, removing.  Container is %s.", container.GetContainerName());
        removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
        return ECObjectsStatus::Success;
        }

    auto koq = prop->GetKindOfQuantity();
    if (nullptr != koq && prop->IsKindOfQuantityDefinedLocally())
        {
        LOG.warningv("Found DisplayUnitSpecification on property %s:%s.%s with UnitSpecification defined locally. Skipping conversion since the DisplayUnitSpecificaiton was already processed.", schema.GetFullSchemaName().c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str());
        removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
        return ECObjectsStatus::Success;
        }

    Unit oldUnit;
    ECUnitCP newUnit = nullptr;
    Utf8String koqName;
    if(!Unit::GetUnitForECProperty(oldUnit, *prop))
        {
        if (nullptr == koq)
            {
            Utf8String fullName = schema.GetFullSchemaName();
            LOG.warningv("The property %s:%s.%s has a UnitSpecification or DisplayUnitSpecification but the persistence unit cannot be resloved.  Dropping units information.", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str());
            removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
            return ECObjectsStatus::Success;
            }
        // Base property has already been converted and this property just has a DisplayUnitSpecification applied
        newUnit = koq->GetPersistenceUnit();
        if (nullptr == newUnit)
            {
            Utf8String fullName = schema.GetFullSchemaName();
            LOG.errorv("The property %s:%s.%s has a KOQ %s:%s that does not have a unit, failing conversion.",
                fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), koq->GetSchema().GetName().c_str(), koq->GetName().c_str());
            return ECObjectsStatus::Error;
            }
        koqName = koq->GetName();
        }

    if (nullptr == context)
        {
        BeAssert(true);
        LOG.error("Missing ECSchemaReadContext, it is necessary to perform conversion on a UnitSpecification custom attribute.");
        return ECObjectsStatus::Error;
        }

    static SchemaKey unitsKey("Units", 1, 0, 0); // These can be static since we already know the name and version we need to be able to locate.
    static SchemaKey formatsKey("Formats", 1, 0, 0);

    ECSchemaPtr unitSchema = context->LocateSchema(unitsKey, SchemaMatchType::LatestReadCompatible);
    if (!unitSchema.IsValid())
        {
        LOG.errorv("Unable to locate the %s necessary for converting a UnitSpecification custom attribute", unitsKey.GetFullSchemaName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    ECSchemaPtr formatSchema = context->LocateSchema(formatsKey, SchemaMatchType::LatestReadCompatible);
    if (!formatSchema.IsValid())
        {
        LOG.errorv("Unable to locate the %s necessary for converting a UnitSpecification custom attribute", formatsKey.GetFullSchemaName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    if (nullptr == newUnit)
        {
        Utf8CP ecName = Units::UnitNameMappings::TryGetECNameFromOldName(oldUnit.GetName());
        if (!Utf8String::IsNullOrEmpty(ecName))
            {
            Utf8String schemaName;
            Utf8String name;
            if (ECObjectsStatus::Success == ECClass::ParseClassName(schemaName, name, ecName))
                newUnit = unitSchema->GetUnitCP(name.c_str());
            }
        }
    if (nullptr == newUnit)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("The property %s:%s.%s has an old unit '%s' that does not resolve to a new unit.  Dropping unit to continue", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldUnit.GetName());
        removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
        return ECObjectsStatus::Success;
        }

    if (!ECSchema::IsSchemaReferenced(schema, *unitSchema) && ECObjectsStatus::Success != schema.AddReferencedSchema(*unitSchema))
        {
        LOG.errorv("Unable to add the %s schema as a reference to %s.", unitSchema->GetFullSchemaName().c_str(), schema.GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    if (!ECSchema::IsSchemaReferenced(schema, *formatSchema) && ECObjectsStatus::Success != schema.AddReferencedSchema(*formatSchema))
        {
        LOG.errorv("Unable to add the %s schema as a reference to %s.", formatSchema->GetFullSchemaName().c_str(), schema.GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    KindOfQuantityP newKOQ;
    Utf8String newKOQName;
    if (!UnitSpecification::TryGetNewKOQName(*prop, newKOQName))
        newKOQName = Utf8String::IsNullOrEmpty(koqName.c_str()) ? newUnit->GetPhenomenon()->GetName() : koqName;

    ECUnitCP newDisplayUnit = nullptr;
    Unit oldDisplayUnit;
    Utf8String oldFormatString;
    if (Unit::GetDisplayUnitAndFormatForECProperty(oldDisplayUnit, oldFormatString, oldUnit, *prop) && (0 != strcmp(oldDisplayUnit.GetName(), oldUnit.GetName())))
        {
        Utf8CP ecName = Units::UnitNameMappings::TryGetECNameFromOldName(oldDisplayUnit.GetName());

        if (nullptr != ecName)
            {
            Utf8String alias;
            Utf8String name;
            ECClass::ParseClassName(alias, name, ecName);

            newDisplayUnit = unitSchema->GetUnitCP(name.c_str());
            if (nullptr != newDisplayUnit)
                {
                if (!ECSchema::IsSchemaReferenced(schema, *unitSchema) && ECObjectsStatus::Success != schema.AddReferencedSchema(*unitSchema))
                    {
                    LOG.errorv("Unable to add the %s schema as a reference to %s.", unitSchema->GetFullSchemaName().c_str(), schema.GetName().c_str());
                    return ECObjectsStatus::SchemaNotFound;
                    }
                }
            }

        if (nullptr == newDisplayUnit)
            LOG.warningv("The property %s:%s.%s has an old display unit '%s' that does not resolve to a new unit.", schema.GetFullSchemaName().c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldDisplayUnit.GetName());
        }

    bool persistenceUnitChanged;
    ECObjectsStatus status = obtainKindOfQuantity(schema, prop, newKOQ, instance, newUnit, newDisplayUnit, persistenceUnitChanged, newKOQName.c_str());
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Failed to create KindOfQuantity '%s' for property '%s.%s'", newKOQName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str());
        removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
        return status;
        }

    if (persistenceUnitChanged)
        addOldUnitCustomAttribute(schema, prop, oldUnit.GetName());

    if (ECObjectsStatus::KindOfQuantityNotCompatible == prop->SetKindOfQuantity(newKOQ))
        {
        LOG.warningv("Unable to convert KindOfQuantity '%s' on property %s.%s because it conflicts with another KindOfQuantity in the base class hierarchy.", newKOQName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str());
        }

    removePropertyUnitCustomAttributes(container, instance.GetClass().GetSchema().GetName(), instance.GetClass().GetName());
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus UnitSpecificationsConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECSchemaP containerAsSchema = dynamic_cast<ECSchemaP>(&container);
    if (containerAsSchema == nullptr)
        {
        LOG.warningv("Found a 'UnitSpecifications' custom attribute on a container which is not an ECSchema, removing.  Container is %s", container.GetContainerName());
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
        return ECObjectsStatus::Success;
        }

    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! UnitSystemConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus UnitSystemConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECSchemaP containerAsSchema = dynamic_cast<ECSchemaP>(&container);
    if (containerAsSchema == nullptr)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Found a 'UnitSystem' custom attribute on a container which is not an ECSchema, removing.  Container is in schema %s", fullName.c_str());
        }

    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, IS_UNIT_SYSTEM);
    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, MIXED_UNIT_SYSTEM);
    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, SI_UNIT_SYSTEM);
    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, US_UNIT_SYSTEM);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, IS_UNIT_SYSTEM);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, MIXED_UNIT_SYSTEM);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, SI_UNIT_SYSTEM);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, US_UNIT_SYSTEM);
    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! CustomAttributeReplacement
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CustomAttributeReplacement::AddPropertyMapping(Utf8CP oldPropertyName, Utf8CP newPropertyName)
    {
    auto const it = m_propertyMapping.find(oldPropertyName);
    if (it != m_propertyMapping.end())
        {
        if (0 != BeStringUtilities::Stricmp(newPropertyName, it->second.c_str()))
            return ECObjectsStatus::Error; // The existing property mapping is different than the one that is trying to be added
        
        return ECObjectsStatus::Success;
        }

    m_propertyMapping[oldPropertyName] = newPropertyName;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CustomAttributeReplacement::GetPropertyMapping(Utf8CP oldPropertyName)
    {
    auto const it = m_propertyMapping.find(oldPropertyName);
    if (m_propertyMapping.end() == it)
        return Utf8String();

    return it->second;
    }

//=======================================================================================
//! StandardCustomAttributeReferencesConverter
//=======================================================================================

bmap<Utf8String, CustomAttributeReplacement> StandardCustomAttributeReferencesConverter::s_entries = bmap<Utf8String, CustomAttributeReplacement>();
bool StandardCustomAttributeReferencesConverter::s_isInitialized = false;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR sourceCustomAttribute, ECSchemaReadContextP context)
    {
    auto sourceCustomAttributeClass = &sourceCustomAttribute.GetClass();
    auto sourceCustomAttributeKey = ECSchemaConverter::GetQualifiedClassName(sourceCustomAttributeClass->GetSchema().GetName(), sourceCustomAttributeClass->GetName());

    auto const& mappingCollection = GetCustomAttributesMapping();
    auto const it = mappingCollection.find(sourceCustomAttributeKey);
    if (it == mappingCollection.end())
        {
        return ECObjectsStatus::Error;
        }
    auto mapping = it->second;

    auto customAttributeSchema = CoreCustomAttributeHelper::GetSchema();

    ECClassP customAttributeClass = customAttributeSchema->GetClassP(mapping.GetNewCustomAttributeName().c_str());
    IECInstancePtr targetAttributeInstance = customAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECObjectsStatus propertyConversionStatus;
    for (uint32_t i = 0; i < sourceCustomAttributeClass->GetPropertyCount(); i++)
        {
        ECPropertyP propP = sourceCustomAttributeClass->GetPropertyByIndex((uint32_t)i);

        propertyConversionStatus = ConvertPropertyValue(propP->GetName().c_str(), mapping, sourceCustomAttribute, *targetAttributeInstance);
        if (ECObjectsStatus::Success != propertyConversionStatus)
            return propertyConversionStatus;
        }

    // Remove the old Custom Attribute and add the new one to the container
    ECObjectsStatus status;
    if (container.RemoveCustomAttribute(mapping.GetOldSchemaName(), mapping.GetOldCustomAttributeName()))
        {
        schema.AddReferencedSchema(*customAttributeSchema);
        if ((status = container.SetCustomAttribute(*targetAttributeInstance)) != ECObjectsStatus::Success)
            return status;
        }
    else if (container.RemoveSupplementedCustomAttribute(mapping.GetOldSchemaName(), mapping.GetOldCustomAttributeName()))
        {
        if ((status = container.SetSupplementedCustomAttribute(*targetAttributeInstance)) != ECObjectsStatus::Success)
            return status;
        }
    else
        {
        LOG.errorv("Couldn't remove the CustomAttribute %s from %s", sourceCustomAttributeClass->GetName().c_str(), GetContainerName(container).c_str());
        return ECObjectsStatus::Error;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bmap<Utf8String, CustomAttributeReplacement> const& StandardCustomAttributeReferencesConverter::GetCustomAttributesMapping()
    {
    // Only works with schemas from the CoreCustomAttributes schema.
    if (!s_isInitialized)
        {
        // Converts reference of DateTimeInfo CA to the new class
        AddMapping("Bentley_Standard_CustomAttributes", "DateTimeInfo", "CoreCustomAttributes", "DateTimeInfo");
        AddMapping("Bentley_Standard_CustomAttributes", "ClassHasCurrentTimeStampProperty", "CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
        
        // SupplementalProvenance does not need to be converted since it is never serialized.
        AddMapping("Bentley_Standard_CustomAttributes", "SupplementalSchemaMetaData", "CoreCustomAttributes", "SupplementalSchema");
        Utf8String oldSupplementalSchemaQualifiedName = ECSchemaConverter::GetQualifiedClassName("Bentley_Standard_CustomAttributes", "SupplementalSchemaMetaData");
        AddPropertyMapping(oldSupplementalSchemaQualifiedName.c_str(), "PrimarySchemaName", "PrimarySchemaReference.SchemaName");
        AddPropertyMapping(oldSupplementalSchemaQualifiedName.c_str(), "PrimarySchemaMajorVersion", "PrimarySchemaReference.MajorVersion");
        AddPropertyMapping(oldSupplementalSchemaQualifiedName.c_str(), "PrimarySchemaMinorVersion", "PrimarySchemaReference.MinorVersion");

        AddMapping("Bentley_Standard_CustomAttributes", "DynamicSchema", "CoreCustomAttributes", "DynamicSchema");

        // Add More...
            
        s_isInitialized = true;
        }
    
        return s_entries;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::AddMapping(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName)
    {
    Utf8String qualifiedName = ECSchemaConverter::GetQualifiedClassName(oSchema, oName);
    s_entries[qualifiedName] = CustomAttributeReplacement(oSchema, oName, nSchema, nName);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::AddPropertyMapping(Utf8CP oldFullyQualifiedName, Utf8CP oldPropertyName, Utf8CP newPropertyName)
    {
    auto it = s_entries.find(oldFullyQualifiedName);
    if (s_entries.end() == it)
        return ECObjectsStatus::Error;

    return it->second.AddPropertyMapping(oldPropertyName, newPropertyName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StandardCustomAttributeReferencesConverter::GetContainerName(IECCustomAttributeContainerR container) const
    {
    ECSchemaP containerAsSchema = dynamic_cast<ECSchemaP>(&container);
    if (containerAsSchema == nullptr)
        return "ECSchema " + containerAsSchema->GetName();

    ECClassP containerAsClass = dynamic_cast<ECClassP>(&container);
    if (containerAsClass == nullptr)
        return "ECClass " + containerAsClass->GetName();

    ECPropertyP containerAsProperty = dynamic_cast<ECPropertyP>(&container);
    if (containerAsProperty == nullptr)
        return "ECProperty " + containerAsProperty->GetName();

    return "Unsupported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::ConvertPropertyValue(Utf8CP sourceProp, CustomAttributeReplacement mapping, IECInstanceR sourceCustomAttribute, IECInstanceR targetCustomAttribute)
    {
    ECObjectsStatus status;

    ECValue sourceValue;
    if ((status = sourceCustomAttribute.GetValue(sourceValue, sourceProp)) != ECObjectsStatus::Success)
        return status;

    // Checks if property Value is actually set. In case it isn't the conversion will be stopped
    // as only properties that are defined on the source Custom Attribute will be set on the target
    // Custom Attribute.
    if (sourceValue.IsUninitialized() || sourceValue.IsNull())
        return ECObjectsStatus::Success;

    if (!sourceValue.IsPrimitive())
        {
        LOG.errorv("The current version of StandardCustomAttributeReferencesConverter only supports converting from primitive properties.");
        return ECObjectsStatus::Error;
        }

    Utf8String targetAccessor = mapping.GetPropertyMapping(sourceProp);
    if (targetAccessor.empty())
        targetAccessor = sourceProp;

    ECValue targetValue;
    if ((status = targetCustomAttribute.GetValue(targetValue, targetAccessor.c_str())) != ECObjectsStatus::Success)
        return ECObjectsStatus::Success; // drops the property

    if (!targetValue.IsPrimitive())
        {
        LOG.errorv("The current version of StandardCustomAttributeReferencesConverter only supports converting to primitive or struct properties.");
        return ECObjectsStatus::Error;
        }

    ECPropertyP targetProperty = targetCustomAttribute.GetClass().GetPropertyP(targetAccessor);
    if (nullptr != targetProperty)
        {
        auto targetPrimitiveProperty = targetProperty->GetAsPrimitiveProperty();
        auto enumeration = targetPrimitiveProperty->GetEnumeration();
        if (enumeration != nullptr)
            {
            return ConvertPropertyToEnum(targetAccessor, *enumeration, targetCustomAttribute, targetValue, sourceValue);
            }
        }

    targetValue.From(sourceValue);
    targetCustomAttribute.SetValue(targetAccessor.c_str(), targetValue);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::ConvertPropertyToEnum(Utf8StringCR propertyName, ECEnumerationCR enumeration, IECInstanceR targetCustomAttribute, ECValueR targetValue, ECValueR sourceValue)
    {
    ECEnumeratorCP targetEnumerator = nullptr;
    if (sourceValue.IsString())
        targetEnumerator = enumeration.FindEnumerator(sourceValue.GetUtf8CP());
    else if (sourceValue.IsInteger())
        targetEnumerator = enumeration.FindEnumerator(sourceValue.GetInteger());

    if (nullptr == targetEnumerator)
        return ECObjectsStatus::EnumeratorNotFound;

    BentleyStatus beStatus = BentleyStatus::ERROR;
    if (targetValue.IsInteger() && targetEnumerator->IsInteger())
        beStatus = targetValue.SetInteger(targetEnumerator->GetInteger());
    else if (targetValue.IsString() && targetEnumerator->IsString())
        beStatus = targetValue.SetUtf8CP(targetEnumerator->GetString().c_str());

    if (BentleyStatus::SUCCESS != beStatus)
        {
        LOG.errorv("Couldn't set value of %s to %s", propertyName.c_str(), targetEnumerator->GetDisplayLabel().c_str());
        return ECObjectsStatus::Error;
        }

    ECObjectsStatus status = targetCustomAttribute.SetValue(propertyName.c_str(), targetValue);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Couldn't set value of %s to %s", propertyName.c_str(), targetEnumerator->GetDisplayLabel().c_str());
        return ECObjectsStatus::Error;
        }

    return status;
    }

//=======================================================================================
//! PropertyPriorityConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus PropertyPriorityConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        LOG.warningv("Found PropertyPriority custom attribute on a container which is not a property, removing.  Container is %s", container.GetContainerName());
        container.RemoveCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);
        container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);
        return ECObjectsStatus::Success;
        }

    ECValue previousPriority;
    ECObjectsStatus status = instance.GetValue(previousPriority, "Priority");
    if (ECObjectsStatus::Success != status || previousPriority.IsNull())
        {
        LOG.warningv("Found a PropertyPriority custom attribute on an the ECProperty, '%s.%s', but it did not contain a Priority value. Dropping custom attribute....",
            prop->GetClass().GetFullName(), prop->GetName().c_str());

        container.RemoveCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);
        container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);
        return ECObjectsStatus::Success;
        }

    status = prop->SetPriority(previousPriority.GetInteger());
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Failed to set the priority on ECProperty, %s.%s, from the PropertyPriority custom attribute", 
            prop->GetClass().GetFullName(), prop->GetName().c_str());
        }

    container.RemoveCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);
    container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY);

    return status;
    }

//=======================================================================================
//! PropertyPriorityConverter
//=======================================================================================

PropertyCategoryP getExistingCategory(ECSchemaR schema, ECPropertyP prop, IECInstanceR categoryCA, Utf8CP existingName)
    {
    PropertyCategoryP existingCategory = schema.GetPropertyCategoryP(existingName);
    if (nullptr != existingCategory)
        {
        ECValue value;
        if (ECObjectsStatus::Success == categoryCA.GetValue(value, "DisplayLabel") && !value.IsNull() && value.IsString() &&
            !existingCategory->GetDisplayLabel().Equals(value.GetUtf8CP()))
            LOG.warningv("Found a Category custom attribute on '%s.%s' with a name that matches an existing category '%s' but the attributes of the existing category and the custom attribute don't match, using the existing category anyway.",
                         prop->GetClass().GetFullName(), prop->GetName().c_str(), existingName);
        // TODO: Do we need to handle this better?
        }
    return existingCategory;
    }

ECObjectsStatus createPropertyCategory(ECSchemaR schema, PropertyCategoryP& newCategory, Utf8CP newName, IECInstanceR categoryCA)
    {
    ECObjectsStatus status = schema.CreatePropertyCategory(newCategory, newName, false);
    if (ECObjectsStatus::Success == status)
        {
        ECValue value;
        if (ECObjectsStatus::Success == categoryCA.GetValue(value, "DisplayLabel") && !value.IsNull() && value.IsString())
            status = newCategory->SetDisplayLabel(value.GetUtf8CP());
        if (ECObjectsStatus::Success == categoryCA.GetValue(value, "Description") && !value.IsNull() && value.IsString())
            status = newCategory->SetDescription(value.GetUtf8CP());
        if (ECObjectsStatus::Success == categoryCA.GetValue(value, "Priority") && !value.IsNull() && value.IsInteger())
            status = newCategory->SetPriority((uint32_t)value.GetInteger());
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CategoryConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        LOG.warningv("Found Category custom attribute on a container which is not a property, removing.  Container is %s", container.GetContainerName());
        container.RemoveCustomAttribute(BECA_SCHEMANAME, CATEGORY);
        container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, CATEGORY);
        return ECObjectsStatus::Success;
        }

    ECValue existingName;
    ECObjectsStatus status = instance.GetValue(existingName, "Name");
    if (ECObjectsStatus::Success != status || existingName.IsNull())
        {
        LOG.warningv("Found a Category custom attribute on an the ECProperty, '%s.%s', but it did not contain a Name value. Dropping custom attribute....",
            prop->GetClass().GetFullName(), prop->GetName().c_str());
        
        // TODO: Support standard categories
        return ECObjectsStatus::Success;
        }

    Utf8String newName = existingName.GetUtf8CP();
    if (!ECNameValidation::IsValidName(newName.c_str()))
        newName = ECNameValidation::EncodeToValidName(newName);

    PropertyCategoryP newPropCategory = getExistingCategory(schema, prop, instance, newName.c_str());
    if (nullptr == newPropCategory)
        {
        if (ECObjectsStatus::Success != (status = createPropertyCategory(schema, newPropCategory, newName.c_str(), instance)))
            {
            if (ECObjectsStatus::NamedItemAlreadyExists == status)
                {
                LOG.warningv("Renaming PropertyCategory '%s' to '%s_Category' due to naming conflict", newName.c_str(), newName.c_str());
                newName += "_Category";
                newPropCategory = getExistingCategory(schema, prop, instance, newName.c_str());
                status = nullptr == newPropCategory ? createPropertyCategory(schema, newPropCategory, newName.c_str(), instance) : ECObjectsStatus::Success;
                }

            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed to create PropertyCategory, %s, from the category defined on property %s.%s because it conflicts with an existing type name within the schema '%s'.",
                           newName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetClass().GetSchema().GetFullSchemaName().c_str());
                return status;
                }
            }
        }

    status = prop->SetCategory(newPropCategory);
    if (ECObjectsStatus::Success != status)
        return status;

    container.RemoveCustomAttribute(BECA_SCHEMANAME, CATEGORY);
    container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, CATEGORY);

    return status;
    }

//=======================================================================================
//! HidePropertyConverter
//=======================================================================================

bool getBoolValue(IECInstanceR instance, Utf8CP accessString, bool defaultValue)
    {
    ECValue ecValue;
    if (ECObjectsStatus::Success != instance.GetValue(ecValue, accessString))
        return defaultValue;

    return ecValue.IsNull() ? defaultValue : ecValue.GetBoolean();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus HidePropertyConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        LOG.warningv("Found HideProperty custom attribute on a container which is not a property, removing.  Container is %s", container.GetContainerName());
        container.RemoveCustomAttribute(BECA_SCHEMANAME, HIDE_PROPERTY);
        container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, HIDE_PROPERTY);
        return ECObjectsStatus::Success;
        }

    // Property should be shown if either of the legacy properties is set to false
    bool if2d = getBoolValue(instance, IF2D, true);
    bool if3d = getBoolValue(instance, IF3D, true);
    bool showProp = !if2d && !if3d;

    auto customAttributeSchema = CoreCustomAttributeHelper::GetSchema();
    IECInstancePtr hiddenProperty = customAttributeSchema->GetClassCP(HIDDEN_PROPERTY)->GetDefaultStandaloneEnabler()->CreateInstance();
    
    ECValue value(showProp);
    hiddenProperty->SetValue(SHOW, value);

    container.RemoveCustomAttribute(instance.GetClass());
    container.RemoveSupplementedCustomAttribute(instance.GetClass());

    schema.AddReferencedSchema(*customAttributeSchema);
    container.SetCustomAttribute(*hiddenProperty);

    return ECObjectsStatus::Success;
    }

//=======================================================================================
//! DisplayOptionsConverter
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus DisplayOptionsConverter::ConvertSchemaDisplayOptions(ECSchemaR schema, IECInstanceR instance)
    {
    bool hideSchema = getBoolValue(instance, HIDDEN, false) || getBoolValue(instance, HIDE_INSTANCES, false);
    if (hideSchema)
        {
        auto customAttributeSchema = CoreCustomAttributeHelper::GetSchema();
        IECInstancePtr hiddenSchema = customAttributeSchema->GetClassCP(HIDDEN_SCHEMA)->GetDefaultStandaloneEnabler()->CreateInstance();
        schema.AddReferencedSchema(*customAttributeSchema);
        schema.SetCustomAttribute(*hiddenSchema);
        }
    schema.RemoveCustomAttribute(instance.GetClass());
    schema.RemoveSupplementedCustomAttribute(instance.GetClass());
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus DisplayOptionsConverter::ConvertClassDisplayOptions(ECSchemaR schema, ECClassR ecClass, IECInstanceR instance)
    {
    bool hideClass = getBoolValue(instance, HIDDEN, false) || getBoolValue(instance, HIDE_INSTANCES, false);

    auto customAttributeSchema = CoreCustomAttributeHelper::GetSchema();
    IECInstancePtr hiddenClass = customAttributeSchema->GetClassCP(HIDDEN_CLASS)->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue show(!hideClass);
    hiddenClass->SetValue(SHOW, show);

    schema.AddReferencedSchema(*customAttributeSchema);
    ecClass.SetCustomAttribute(*hiddenClass);


    ecClass.RemoveCustomAttribute(instance.GetClass());
    ecClass.RemoveSupplementedCustomAttribute(instance.GetClass());
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus DisplayOptionsConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context)
    {
    ECClassP ecClass = dynamic_cast<ECClassP> (&container);
    if (nullptr != ecClass)
        return ConvertClassDisplayOptions(schema, *ecClass, instance);

    ECSchemaP ecSchema = dynamic_cast<ECSchemaP> (&container);
    if (nullptr != ecSchema)
        return ConvertSchemaDisplayOptions(schema, instance);

    LOG.warningv("Found DisplayOptions custom attribute on a container which is not a schema or class, removing.  Container is %s", container.GetContainerName());
    container.RemoveCustomAttribute(BSCA_SCHEMANAME, DISPLAY_OPTIONS);
    container.RemoveSupplementedCustomAttribute(BSCA_SCHEMANAME, DISPLAY_OPTIONS);
    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
