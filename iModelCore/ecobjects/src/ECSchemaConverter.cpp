/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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

        if (ECObjectsStatus::Success != (status = structInstance->GetValue(value, "Value")))
            return status;
        int index = value.GetInteger();

        if (ECObjectsStatus::Success != (status = structInstance->GetValue(value, "DisplayString")))
            return status;
        sdInfo.m_valuesMap[index] = value.ToString();
        }
    return ECObjectsStatus::Success;
    }

static Utf8CP const  STANDARDVALUES_CUSTOMATTRIBUTE = "StandardValues";
static Utf8CP const  BECA_SCHEMANAME = "EditorCustomAttributes";

static Utf8CP const  UNIT_ATTRIBUTES                = "Unit_Attributes";
static Utf8CP const  KOQ_NAME                       = "KindOfQuantityName";
static Utf8CP const  DIMENSION_NAME                 = "DimensionName";
static Utf8CP const  UNIT_SPECIFICATION             = "UnitSpecificationAttr";
static Utf8CP const  UNIT_SPECIFICATIONS            = "UnitSpecifications";
//static Utf8CP const  UNIT_SPECIFICATION_LIST        = "UnitSpecificationList"; UNUSED
static Utf8CP const  DISPLAY_UNIT_SPECIFICATION     = "DisplayUnitSpecificationAttr";
static Utf8CP const IS_UNIT_SYSTEM                  = "IsUnitSystemSchema";
static Utf8CP const MIXED_UNIT_SYSTEM               = "Mixed_UnitSystem";
static Utf8CP const SI_UNIT_SYSTEM                  = "SI_UnitSystem";
static Utf8CP const US_UNIT_SYSTEM                  = "US_UnitSystem";
static Utf8CP const PROPERTY_PRIORITY               = "PropertyPriority";
static Utf8CP const CATEGORY                        = "Category";


struct UnitSpecification
    {
    UnitSpecification() = delete;
    static bool TryGetNewKOQName(IECInstanceCR instance, Utf8StringR newKindOfQuantityName)
        {
        return TryGetStringValue(instance, KOQ_NAME, newKindOfQuantityName) || TryGetStringValue(instance, DIMENSION_NAME, newKindOfQuantityName);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchemaConverter::Convert(ECSchemaR schema)
    {
    ECSchemaConverterP ecSchemaConverter = GetSingleton();
    ecSchemaConverter->m_convertedOK = true;

    auto classes = GetHierarchicallySortedClasses(schema);
    ecSchemaConverter->ConvertClassLevel(classes);

    ecSchemaConverter->ConvertPropertyLevel(classes);

    ecSchemaConverter->ConvertSchemaLevel(schema);

    ecSchemaConverter->RemoveSchemaReferences(schema);

    schema.RemoveUnusedSchemaReferences();

    return ecSchemaConverter->m_convertedOK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaConverterP ECSchemaConverter::GetSingleton()
    {
    static ECSchemaConverterP ECSchemaConverterSingleton = nullptr;

    if (nullptr == ECSchemaConverterSingleton)
        {
        ECSchemaConverterSingleton = new ECSchemaConverter();
        IECCustomAttributeConverterPtr scConv = new StandardValuesConverter();
        ECSchemaConverterSingleton->AddConverter("EditorCustomAttributes", "StandardValues", scConv);

        IECCustomAttributeConverterPtr priorityConv = new PropertyPriorityConverter();
        ECSchemaConverterSingleton->AddConverter(BECA_SCHEMANAME, PROPERTY_PRIORITY, priorityConv);

        IECCustomAttributeConverterPtr categoryConv = new CategoryConverter();
        ECSchemaConverterSingleton->AddConverter(BECA_SCHEMANAME, CATEGORY, categoryConv);

        IECCustomAttributeConverterPtr unitSchemaConv = new UnitSpecificationsConverter();
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecifications", unitSchemaConv);

        IECCustomAttributeConverterPtr unitSystemConv = new UnitSystemConverter();
        ECSchemaConverterSingleton->AddConverter(UNIT_ATTRIBUTES, IS_UNIT_SYSTEM, unitSystemConv);
        ECSchemaConverterSingleton->AddConverter(UNIT_ATTRIBUTES, MIXED_UNIT_SYSTEM, unitSystemConv);
        ECSchemaConverterSingleton->AddConverter(UNIT_ATTRIBUTES, SI_UNIT_SYSTEM, unitSystemConv);
        ECSchemaConverterSingleton->AddConverter(UNIT_ATTRIBUTES, US_UNIT_SYSTEM, unitSystemConv);

        IECCustomAttributeConverterPtr unitPropConv = new UnitSpecificationConverter();
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecification", unitPropConv);
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecificationAttr", unitPropConv);

        ECSchemaConverterSingleton->AddSchemaReferenceToRemove("Unit_Attributes");

        // Iterates over the Custom Attributes classes that will be converted. This converter basically
        // handles Custom Attributes that moved into a new schema but with no content change.
        auto const& mappingDictionary = StandardCustomAttributeReferencesConverter::GetCustomAttributesMapping();
        IECCustomAttributeConverterPtr standardClassConverter = new StandardCustomAttributeReferencesConverter();
        for (auto classMapping : mappingDictionary)
            {
            ECSchemaConverterSingleton->AddConverter(classMapping.first, standardClassConverter);
            }
        }

    return ECSchemaConverterSingleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchemaConverter::AddConverter(Utf8StringCR schemaName, Utf8StringCR customAttributeName, IECCustomAttributeConverterPtr& converter)
    {
    Utf8String converterName = GetQualifiedClassName(schemaName, customAttributeName);
    return AddConverter(converterName, converter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Stefan.Apfel                  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchemaConverter::AddConverter(Utf8StringCR customAttributeKey, IECCustomAttributeConverterPtr& converter)
    {
    ECSchemaConverterP ECSchemaConverter = GetSingleton();
    ECSchemaConverter->m_converterMap[customAttributeKey] = converter;

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
void ECSchemaConverter::FindRootBaseClasses(ECPropertyP ecProperty, bvector<ECClassP>& rootClasses)
    {
    GatherRootBaseClasses(&ecProperty->GetClass(), ecProperty->GetName().c_str(), rootClasses);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
bvector<ECClassP> ECSchemaConverter::GetDerivedAndBaseClasses(ECClassCR ecClass)
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
IECCustomAttributeConverterP ECSchemaConverter::GetConverter(Utf8StringCR converterName)
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
    bvector<Utf8CP> oldCAs = { "HideProperty", "DisplayOptions" };
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
void ECSchemaConverter::ProcessCustomAttributeInstance(ECCustomAttributeInstanceIterable iterable, IECCustomAttributeContainerR container, Utf8String containerName)
    {
    ECSchemaP schema = container.GetContainerSchema();
    for (auto const& attr : iterable)
        {
        Utf8CP fullName = attr->GetClass().GetFullName();
        IECCustomAttributeConverterP converter = GetConverter(fullName);
        if (nullptr != converter)
            {
            LOG.debugv("Started [%s Converter][Container %s]. ", fullName, containerName.c_str());
            auto status = converter->Convert(*schema, container, *attr);
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed [%s Converter][Container %s]. ", fullName, containerName.c_str());
                m_convertedOK = false;
                }
            else    
                LOG.debugv("Succeded [%s Converter][Container %s]. ", fullName, containerName.c_str());
            }
        else if (IsCustomAttributeFromOldStandardSchemas(*attr))
            {
            m_convertedOK = container.RemoveCustomAttribute(attr->GetClass());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                 5/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::ProcessRelationshipConstraint(ECRelationshipConstraintR constraint, bool isSource)
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
void ECSchemaConverter::ConvertClassLevel(bvector<ECClassP>& classes)
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
void ECSchemaConverter::ConvertPropertyLevel(bvector<ECClassP>& classes)
    {
    bvector<Utf8CP> reservedNames {"ECInstanceId", "Id", "ECClassId", "SourceECInstanceId", "SourceId", "SourceECClassId", "SourceId", "SourceECClassId", "TargetECInstanceId", "TargetId", "TargetECClassId"};

    for (auto const& ecClass : classes)
        {
        ECN::ECClassP nonConstClass = const_cast<ECClassP>(ecClass);
        for (auto const& ecProp : ecClass->GetProperties(false))
            {
            Utf8String debugName = Utf8String("ECProperty:") + ecClass->GetFullName() + Utf8String(".") + ecProp->GetName();
            ProcessCustomAttributeInstance(ecProp->GetCustomAttributes(false), *ecProp, debugName);
            // Need to make sure that property name does not conflict with one of the reserved system properties or aliases.
            Utf8CP thisName = ecProp->GetName().c_str();
            auto found = std::find_if(reservedNames.begin(), reservedNames.end(), [thisName] (Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(thisName, reserved) == 0; });
            if (found != reservedNames.end())
                {
                nonConstClass->RenameConflictProperty(ecProp, true);
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                         4/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::RemoveSchemaReferences(ECSchemaR schema)
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
void ECSchemaConverter::SortClassesByNameAndHierarchy(bvector<ECClassP>& ecClasses, bool reverse)
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
void ECSchemaConverter::SortClassesByHierarchy(bvector<ECClassP>& ecClasses)
    {
    bvector<ECClassP> classes;
    bmap<Utf8CP, ECClassP> visited;
    AddClassesRootsFirst(classes, ecClasses, visited);
    
    ecClasses.assign(classes.begin(), classes.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::SortClassesByName(bvector<ECClassP>& ecClasses, bool ascending)
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
bvector<ECClassP> ECSchemaConverter::GetHierarchicallySortedClasses(ECSchemaR schema)
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
        enumeration->CreateEnumerator(enumerator, pair.first);
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
            return status;
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
ECObjectsStatus StandardValuesConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    //are standard values even added to ecclass or ecschema?
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (nullptr == prop)
        {
        LOG.warning("StandardValues custom attribute applied to a container which is not a property.  Removing Custom Attribute and skipping.");
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
        StandardValueInfo existingEnumSdInfo(*existingEnum);
        if ((sdInfo.m_mustBeFromList && sdInfo.Equals(existingEnumSdInfo))
            || (existingEnumSdInfo.Contains(sdInfo)))
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
            enumeration->CreateEnumerator(enumerator, pair.first);
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

        StandardValueInfo enumSdInfo(ecEnum);
        if ((sdInfo.m_mustBeFromList && sdInfo.Equals(enumSdInfo))
            || (enumSdInfo.Contains(sdInfo)))
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
        status = enumeration->CreateEnumerator(enumerator, pair.first);
        if (ECObjectsStatus::Success != status)
            return status;

        enumerator->SetDisplayLabel(pair.second.c_str());
        }
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus UnitSpecificationConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    PrimitiveECPropertyP prop = dynamic_cast<PrimitiveECPropertyP> (&container);
    if (prop == nullptr)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Found UnitSpecification custom attribute on a container which is not a property, removing.  Container is in schema %s", fullName.c_str());
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
        container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
        container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
        return ECObjectsStatus::Success;
        }

    Unit oldUnit;
    if(!Unit::GetUnitForECProperty(oldUnit, *prop))
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.errorv("The property %s:%s.%s has a UnitSpecification but it does not resolve to a unit.", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str());
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
        container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
        container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
        return ECObjectsStatus::Success;
        }

    Units::UnitCP newUnit = Units::UnitRegistry::Instance().LookupUnitUsingOldName(oldUnit.GetName());
    if (nullptr == newUnit)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("The property %s:%s.%s has an old unit '%s' that does not resolve to a new unit.  Creating a dummy unit to continue", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldUnit.GetName());
        newUnit = Units::UnitRegistry::Instance().AddDummyUnit(oldUnit.GetName());
        }

    KindOfQuantityP newKOQ;
    Utf8String newKOQName;
    if (!UnitSpecification::TryGetNewKOQName(instance, newKOQName))
        newKOQName = newUnit->GetPhenomenon()->GetName();

    newKOQ = schema.GetKindOfQuantityP(newKOQName.c_str());
    ECObjectsStatus status;
    if (nullptr != newKOQ)
        {
        if (!Units::Unit::AreEqual(newKOQ->GetPersistenceUnit().GetUnit(), newUnit))
            {
            Utf8String fullName = schema.GetFullSchemaName();
            LOG.infov("Found property %s:%s.%s with KindOfQuantity '%s' and unit '%s' but the KindOfQuantity defines the unit '%s'.  Looking for alternate KindOFQuantity",
                        fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), newKOQName.c_str(), newUnit->GetName(), newKOQ->GetPersistenceUnit().GetUnit()->GetName());

            Utf8PrintfString newKoqString("%s_%s", newKOQName.c_str(), newUnit->GetName());
            ECValidatedName validatedKoqName;
            validatedKoqName.SetName(newKoqString.c_str());
            newKOQ = schema.GetKindOfQuantityP(validatedKoqName.GetName().c_str());
            if (nullptr == newKOQ)
                {
                if (ECObjectsStatus::Success != (status = schema.CreateKindOfQuantity(newKOQ, validatedKoqName.GetName().c_str())))
                    {
                    LOG.errorv("Failed to create KindOfQuantity, %s, from the unit '%s' defined on property %s.%s because it conflicts with an existing type name within the schema '%s'.",
                               validatedKoqName.GetName().c_str(), oldUnit.GetName(), prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetClass().GetSchema().GetFullSchemaName().c_str());
                    return status;
                    }
                newKOQ->SetPersistenceUnit(Formatting::FormatUnitSet("DefaultReal", newUnit->GetName()));
                newKOQ->SetRelativeError(1e-4);
                }
            }
        }
    else
        {
        if (ECObjectsStatus::Success != (status = schema.CreateKindOfQuantity(newKOQ, newKOQName.c_str())))
            {
            LOG.errorv("Failed to create KindOfQuantity, %s, from the unit '%s' defined on property %s.%s because it conflicts with an existing type name within the schema '%s'.",
                       newKOQName.c_str(), oldUnit.GetName(), prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetClass().GetSchema().GetFullSchemaName().c_str());
            return status;
            }
        newKOQ->SetPersistenceUnit(Formatting::FormatUnitSet("DefaultReal", newUnit->GetName()));
        newKOQ->SetRelativeError(1e-4);
        }


    Unit oldDisplayUnit;
    Utf8String oldFormatString;
    if (Unit::GetDisplayUnitAndFormatForECProperty(oldDisplayUnit, oldFormatString, oldUnit, *prop) && (0 != strcmp(oldDisplayUnit.GetName(), oldUnit.GetName())))
        {
        if (!newKOQ->HasPresentationUnits())
            {
            Units::UnitCP newDisplayUnit = Units::UnitRegistry::Instance().LookupUnitUsingOldName(oldDisplayUnit.GetName());
            if (nullptr == newUnit)
                {
                Utf8String fullName = schema.GetFullSchemaName();
                LOG.warningv("The property %s:%s.%s has an old display unit '%s' that does not resolve to a new unit.", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldUnit.GetName());
                }
            else
                newKOQ->SetDefaultPresentationUnit(Formatting::FormatUnitSet("DefaultReal", newDisplayUnit->GetName()));
            }
        }
    
    if (ECObjectsStatus::KindOfQuantityNotCompatible == prop->SetKindOfQuantity(newKOQ))
        {
        LOG.warningv("Unable to convert KindOfQuantity '%s' on property %s.%s because it conflicts with another KindOfQuantity in the base class hierarchy.", newKOQName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str());
        }
    prop->RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
    prop->RemoveCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);
    prop->RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
    prop->RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);

    return ECObjectsStatus::Success;
    }

ECObjectsStatus UnitSpecificationsConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    ECSchemaP containerAsSchema = dynamic_cast<ECSchemaP>(&container);
    if (containerAsSchema == nullptr)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Found a 'UnitSpecifications' custom attribute on a container which is not an ECSchema, removing.  Container is in schema %s", fullName.c_str());
        container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
        return ECObjectsStatus::Success;
        }

    container.RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    container.RemoveSupplementedCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATIONS);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus UnitSystemConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
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

Utf8String CustomAttributeReplacement::GetPropertyMapping(Utf8CP oldPropertyName)
    {
    auto const it = m_propertyMapping.find(oldPropertyName);
    if (m_propertyMapping.end() == it)
        return Utf8String();

    return it->second;
    }

bmap<Utf8String, CustomAttributeReplacement> StandardCustomAttributeReferencesConverter::s_entries = bmap<Utf8String, CustomAttributeReplacement>();
bool StandardCustomAttributeReferencesConverter::s_isInitialized = false;
ECSchemaReadContextPtr schemaContext = NULL;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardCustomAttributeReferencesConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR sourceCustomAttribute)
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

    if (schemaContext == NULL)
        schemaContext = ECSchemaReadContext::CreateContext();

    SchemaKey key(mapping.GetNewSchemaName().c_str(), 1, 0);
    auto customAttributeSchema = ECSchema::LocateSchema(key, *schemaContext);

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

    // Attempt to remove the old referenced schema. If it fails that means it is still in use, so don't fail conversion.
    schema.RemoveUnusedSchemaReferences();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bmap<Utf8String, CustomAttributeReplacement> const& StandardCustomAttributeReferencesConverter::GetCustomAttributesMapping()
    {
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus PropertyPriorityConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Found PropertyPriority custom attribute on a container which is not a property, removing.  Container is in schema %s", fullName.c_str());
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
        return ECObjectsStatus::Success;
        }

    status = prop->SetPriority(previousPriority.GetInteger());
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Failed to set the priority on ECProperty, %s.%s, from the PropertyPriority custom attribute", 
            prop->GetClass().GetFullName(), prop->GetName().c_str());
        return status;
        }

    if (!prop->RemoveCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY) &&
        !prop->RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, PROPERTY_PRIORITY))
        {
        LOG.errorv("Couldn't remove the CustomAttribute %s:%s from the property %s.%s", BECA_SCHEMANAME, PROPERTY_PRIORITY, 
            prop->GetClass().GetFullName(), prop->GetName().c_str());
        return ECObjectsStatus::Error;
        }

    // Attempt to remove the old referenced schema. If it fails that means it is still in use, so don't fail conversion.
    schema.RemoveUnusedSchemaReferences();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus CategoryConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (prop == nullptr)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("Found Category custom attribute on a container which is not a property, removing.  Container is in schema %s", fullName.c_str());
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
    PropertyCategoryP newPropCategory = schema.GetPropertyCategoryP(newName.c_str());
    if (nullptr != newPropCategory)
        {
        ECValue value;
        if (ECObjectsStatus::Success == instance.GetValue(value, "DisplayLabel") && !value.IsNull() && value.IsString() &&
            !newPropCategory->GetDisplayLabel().Equals(value.GetUtf8CP()))
            LOG.warningv("Found a Category custom attribute on '%s.%s' with a name that matches an existing category '%s' but the attributes of the existing category and the custom attribute done match, using the existing category anyway.",
                         prop->GetClass().GetFullName(), prop->GetName().c_str(), newName.c_str());
        // TODO: Do we need to handle this better?
        }
    else
        {
        if (ECObjectsStatus::Success != (status = schema.CreatePropertyCategory(newPropCategory, newName.c_str())))
            {
            LOG.errorv("Failed to create PropertyCategory, %s, from the category defined on property %s.%s because it conflicts with an existing type name within the schema '%s'.",
                       newName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetClass().GetSchema().GetFullSchemaName().c_str());
            return status;
            }

        ECValue value;
        if (ECObjectsStatus::Success == instance.GetValue(value, "DisplayLabel") && !value.IsNull() && value.IsString())
            status = newPropCategory->SetDisplayLabel(value.GetUtf8CP());
        if (ECObjectsStatus::Success == instance.GetValue(value, "Description") && !value.IsNull() && value.IsString())
            status = newPropCategory->SetDescription(value.GetUtf8CP());
        if (ECObjectsStatus::Success == instance.GetValue(value, "Priority") && !value.IsNull() && value.IsInteger())
            status = newPropCategory->SetPriority((uint32_t)value.GetInteger());
        }

    status = prop->SetCategory(newPropCategory);
    if (ECObjectsStatus::Success != status)
        return status;

    container.RemoveCustomAttribute(BECA_SCHEMANAME, CATEGORY);
    container.RemoveSupplementedCustomAttribute(BECA_SCHEMANAME, CATEGORY);

    return status;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
