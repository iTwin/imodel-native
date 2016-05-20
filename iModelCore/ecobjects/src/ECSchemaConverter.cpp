/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaConverter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//-------------------------------------------------------------------------------------
// Data of standard values and the property that it is attached to
// @bsistruct                                                    Basanta.Kharel   12/2015
//---------------+---------------+---------------+---------------+---------------+------
struct StandardValueInfo
    {
    friend struct StandardValuesConverter;
private: 
    bmap<int, Utf8String> m_valuesMap;
    bool m_mustBeFromList;

public:
    bool Equals(const StandardValueInfo& sd) const;

    StandardValueInfo(ECEnumerationP& ecEnum);
    StandardValueInfo() {};

    //--------------------------------------------------------------------------------------
    // Extracts Standard Values Instance data from the instance
    //! @param[in]  instance    The instance of StandValues customattribute
    //! @param[out] sdInfo      Extracted data 
    //---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus ExtractInstanceData(IECInstanceR instance, StandardValueInfo& sdInfo);

    };

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
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
StandardValueInfo::StandardValueInfo(ECEnumerationP& ecEnum)
    {
    m_mustBeFromList = ecEnum->GetIsStrict();
    for (auto const& enumerator : ecEnum->GetEnumerators())
        m_valuesMap[enumerator->GetInteger()] = enumerator->GetDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// Extracts Standard Values Instance data from the instance
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

//---------------------------------------------------------------------------------------
// Implements IECCustomAttributeConverter to convert Standard Values Custom Attribute to ECEnumeration
// @bsistruct                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct StandardValuesConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    
private:

    //---------------------------------------------------------------------------------------
    // And checks if the standard values conflict inbetween inherited properties
    // Also returns list of classNames that have this property
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus CheckForConflict(ECPropertyP standardValueProperty, ECSchemaR schema,StandardValueInfo& sdInfo, Utf8String& classes);

    //---------------------------------------------------------------------------------------
    // Sets strictness of the enum. 
    // If the rootClass that has this property has no CA, it will be false
    // Otherwise it will be the same as what the mustbefromlist value it has. default is true
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    static void SetStrictness(ECPropertyP standardValueProperty, ECSchemaR schema,StandardValueInfo& sdInfo);

    //---------------------------------------------------------------------------------------
    // Adds enumeration to ecProperty by finding enumeration that matches sdInfo or by creating one
    // @bsimethod                                    Basanta.Kharel                  12/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus AddEnumeration(ECSchemaR schema, StandardValueInfo& sdInfo, ECPropertyP& ecProperty);

    //---------------------------------------------------------------------------------------
    // Gives name to enumeration, based on className and propertyName 
    // Usually is (baseClassName+propertyName)
    // returns rootClass i.e. baseclass
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    static Utf8String CreateEnumerationName(ECClassP& rootClass, ECSchemaR schema, ECPropertyP& ecProperty);

    //---------------------------------------------------------------------------------------
    // Append number to string : returns name_number
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    static Utf8String AppendNumberToString(Utf8CP name, int number);

    //---------------------------------------------------------------------------------------
    // Finds enumeration in schema that matches sdInfo
    // @bsimethod                                    Basanta.Kharel                  12/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus FindEnumeration(ECSchemaR schema, ECEnumerationP& enumeration, StandardValueInfo& sdInfo);

    //---------------------------------------------------------------------------------------
    // Creates enumeration in schema : given enumName, sdInfo
    // @bsimethod                                    Basanta.Kharel                  12/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus CreateEnumeration(ECEnumerationP& enumeration, ECSchemaR schema, Utf8CP enumName, StandardValueInfo& sdInfo);

    //! Converts primitive property type to enum type
    //! @param[in] primitiveEcProperty      The primitive property whose type is being changed
    //! @param[in] ecSchema                 The schema the ecProperty belongs to
    //! @param[in] enumeration              The enumeration to be set to the primitiveEcProperty
    static ECObjectsStatus ConvertToEnumType(ECPropertyP& primitiveEcProperty, ECSchemaR ecSchema, ECEnumerationP& enumeration);
        
    };


//---------------------------------------------------------------------------------------
// Implements IECCustomAttributeConverter to convert UnitSpecification Custom Attribute to KindOfQuantity
// @bsistruct                                                    Robert.Schili   03/2016
//+---------------+---------------+---------------+---------------+---------------+------
struct UnitSpecificationConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    };

//---------------------------------------------------------------------------------------
// Implements IECCustomAttributeConverter to convert UnitSpecifications Custom Attribute to KindOfQuantity
// @bsistruct                                                    Robert.Schili   03/2016
//+---------------+---------------+---------------+---------------+---------------+------
struct UnitSpecificationsConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    };

static Utf8CP const  UNIT_ATTRIBUTES                = "Unit_Attributes";
static Utf8CP const  KOQ_NAME                       = "KindOfQuantityName";
static Utf8CP const  DIMENSION_NAME                 = "DimensionName";
static Utf8CP const  UNIT_SPECIFICATION             = "UnitSpecificationAttr";
static Utf8CP const  UNIT_SPECIFICATIONS            = "UnitSpecifications";
static Utf8CP const  UNIT_SPECIFICATION_LIST        = "UnitSpecificationList";
static Utf8CP const  DISPLAY_UNIT_SPECIFICATION     = "DisplayUnitSpecificationAttr";

struct UnitSpecification
    {
    UnitSpecification() = delete;
    static Utf8CP GetNewKOQName(IECInstanceCR instance)
        {
        Utf8CP newKOQName = GetStringValue(instance, KOQ_NAME);
        if (Utf8String::IsNullOrEmpty(newKOQName))
            newKOQName = GetStringValue(instance, DIMENSION_NAME);

        return newKOQName;
        }

    static Utf8CP GetStringValue(IECInstanceCR instance, Utf8CP propName)
        {
        ECValue v;
        if (ECObjectsStatus::Success == instance.GetValue(v, propName) && !v.IsNull())
            return v.GetUtf8CP();
        
        return nullptr;
        }
    };

struct CustomAttributeReplacement
    {
    Utf8String oldSchemaName;
    Utf8String oldCustomAttributeName;

    Utf8String newSchemaName;
    Utf8String newCustomAttributeName;

    CustomAttributeReplacement(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName)
        : oldSchemaName(oSchema), oldCustomAttributeName(oName), newSchemaName(nSchema), newCustomAttributeName(nName)
        { }
    CustomAttributeReplacement()
        {
        }
    };

//---------------------------------------------------------------------------------------
// Implements IECCustomAttributeConverter to convert the schema references of certain Custom Attributes.
// Which attributes will be handled depends which CustomATtributeEntries will returned from the
// StandardCustomAttributeReferencesConverter::GetCustomAttributesToConvert method.
// @bsistruct                                                     Stefan.Apfel   04/2016
//+---------------+---------------+---------------+---------------+---------------+------
struct StandardCustomAttributeReferencesConverter : IECCustomAttributeConverter
    {
    private:
        static bool s_isInitialized;
        static bmap<Utf8String, CustomAttributeReplacement> s_entries;

        static ECObjectsStatus AddMapping(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName);

    public:
        ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
        static bmap<Utf8String, CustomAttributeReplacement> const& GetCustomAttributesMapping();
        ECObjectsStatus ConvertPropertyValue(Utf8StringCR propertyName, IECInstanceR oldCustomAttribute, IECInstanceR newCustomAttribute);
        ECObjectsStatus ConvertPropertyToEnum(Utf8StringCR propertyName, ECEnumerationCR enumeration, IECInstanceR targetCustomAttribute, ECValueR targetValue, ECValueR sourceValue);
        int FindEnumerationValue(ECEnumerationCR enumeration, Utf8CP displayName);
        Utf8String GetContainerName(IECCustomAttributeContainerR container) const;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchemaConverter::Convert(ECSchemaR schema)
    {
    ECSchemaConverterP eCSchemaConverter = GetSingleton();
    eCSchemaConverter->m_convertedOK = true;

    eCSchemaConverter->ConvertClassLevel(schema);

    eCSchemaConverter->ConvertPropertyLevel(schema);

    eCSchemaConverter->ConvertSchemaLevel(schema);

    return eCSchemaConverter->m_convertedOK;
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

        IECCustomAttributeConverterPtr unitSchemaConv = new UnitSpecificationsConverter();
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecifications", unitSchemaConv);

        IECCustomAttributeConverterPtr unitPropConv = new UnitSpecificationConverter();
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecification", unitPropConv);
        ECSchemaConverterSingleton->AddConverter("Unit_Attributes", "UnitSpecificationAttr", unitPropConv);

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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchemaConverter::RemoveCustomAttribute(ECPropertyP& ecProperty, ECSchemaR ecSchema, Utf8StringCR customAttributeName)
    {
    auto propertyProcessor = [&customAttributeName](ECPropertyP localProp)
        {
        IECInstancePtr currentInstance = localProp->GetPrimaryCustomAttributeLocal(customAttributeName);
        if (currentInstance.IsValid())
            {
            Utf8String propertyName = localProp->GetClass().GetFullName() + Utf8String(".") + localProp->GetName();
            
            if (!localProp->RemoveCustomAttribute(customAttributeName))
                {
                LOG.errorv("Error removing %s CustomAttribute for %s", customAttributeName.c_str(), propertyName.c_str());
                return ECObjectsStatus::Error;
                }
            LOG.debugv("Removed %s CustomAttribute for %s", customAttributeName.c_str(), propertyName.c_str());
            }
        return ECObjectsStatus::Success;
        };
    
    return TraverseProperty(ecProperty, ecSchema, propertyProcessor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchemaConverter::IsBaseClass(ECClassCP ecClass, ECClassCP baseClass)
    {
    //if ecClass is same as baseClass, baseClass in not really a base class
    if (ECClass::ClassesAreEqualByName(ecClass, baseClass))
        return false; 
    return ecClass->Is(baseClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchemaConverter::TraverseProperty(ECPropertyP ecProperty, ECSchemaR ecSchema,ECPropertyProcessor propertyProcessor)
    {
    Utf8String propertyName = ecProperty->GetName();
    auto classProcessor = [&propertyProcessor, &propertyName](ECClassP ecClass)
        {
        ECPropertyP prop = ecClass->GetPropertyP(propertyName, false);
        if (nullptr != prop)
            return propertyProcessor(prop);

        return ECObjectsStatus::Success;
        };
    ECClassP classP = ecSchema.GetClassP(ecProperty->GetClass().GetName().c_str());
    return TraverseClass(classP, classProcessor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECClassP ECSchemaConverter::FindRootBaseClass(ECPropertyP ecProperty, ECSchemaR schema)
    {
    Utf8String propertyName = ecProperty->GetName();
    bvector<ECClassP> classes;
    auto classProcessor = [&propertyName, &classes](ECClassP ecClassLocal)
        {
        ECPropertyP prop = ecClassLocal->GetPropertyP(propertyName, false);
        if (nullptr != prop)
            classes.push_back(ecClassLocal);

        return ECObjectsStatus::Success;
        };
    ECClassP ecClass = schema.GetClassP(ecProperty->GetClass().GetName().c_str());
    TraverseClass(ecClass, classProcessor);
    SortClassesByNameAndHierarchy(classes);
    return classes.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchemaConverter::TraverseClass(ECClassP ecClass, ECClassProcessor classProcessor)
    {
    bmap<Utf8String, ECClassCP> visited;
    bvector<ECClassP> classes;
    classes.push_back(ecClass);
    ECObjectsStatus status = ECObjectsStatus::Success;

    while (0 != classes.size())
        {
        ECClassP nodeClass = classes.back();
        classes.pop_back();
        visited.Insert(nodeClass->GetFullName(), nodeClass);
        status = classProcessor(nodeClass);
        if (ECObjectsStatus::Success != status)
            return status;
        for (auto const& i : GetDerivedAndBaseClasses(*nodeClass))
            {
            auto iter = visited.Insert(i->GetFullName(), i);
            bool notVisited = iter.second;
            if (notVisited)
                classes.push_back(i);
            }
        }
    return status;
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
        
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::ConvertSchemaLevel(ECSchemaR schema)
    {
    ProcessCustomAttributeInstance(schema.GetPrimaryCustomAttributes(false), schema.GetCustomAttributeContainer(), "ECSchema:" + schema.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::ConvertClassLevel(ECSchemaR schema)
    {
    for (auto const& ecClass : GetHierarchicallySortedClasses(schema))
        {
        ProcessCustomAttributeInstance(ecClass->GetPrimaryCustomAttributes(false), *ecClass, "ECClass:" + ecClass->GetName());
        if (ecClass->IsRelationshipClass())
            {
            ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();

            ECRelationshipConstraintR source = relClass->GetSource();
            ProcessCustomAttributeInstance(source.GetPrimaryCustomAttributes(false), source, "ECRelationshipConstraint:" + source.GetRoleLabel());

            ECRelationshipConstraintR target = relClass->GetTarget();
            ProcessCustomAttributeInstance(target.GetPrimaryCustomAttributes(false), target, "ECRelationshipConstraint:" + target.GetRoleLabel());
            }
        
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaConverter::ConvertPropertyLevel(ECSchemaR schema)
    {
    for (auto const& ecClass : GetHierarchicallySortedClasses(schema))
        {
        for (auto const& ecProp : ecClass->GetProperties(false))
            {
            Utf8String debugName = Utf8String("ECProperty:") + ecClass->GetFullName() + Utf8String(".") + ecProp->GetName();
            ProcessCustomAttributeInstance(ecProp->GetPrimaryCustomAttributes(false), *ecProp, debugName);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @remarks      sorts classes first by name(ascending) then by hierarchy (descending) in order
//               if reverse is set it will reverse after sorting name and hierarchy in order
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
template <typename T>
void ECSchemaConverter::SortClassesByNameAndHierarchy(bvector<T>& ecClasses, bool reverse)
    {
    SortClassesByName(ecClasses, true);
    SortClassesByHierarchy(ecClasses);

    if (reverse)
        std::reverse(ecClasses.begin(), ecClasses.end());
    }

//---------------------------------------------------------------------------------------
// @remarks      sorts classes based on inheritance where base class comes before child class
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
template <typename T>
void ECSchemaConverter::SortClassesByHierarchy(bvector<T>& ecClasses)
    {
    if (ecClasses.size() < 2)
        return;

    bvector<T> classes;
    
    for (auto const& ecClass : ecClasses)
        {
        bool inserted = false;
        for (auto const& localClass : classes)
            {
            if (IsBaseClass(localClass, ecClass))
                {
                auto it = std::find(classes.begin(), classes.end(), localClass);
                classes.insert(it, ecClass);
                inserted = true;
                break;
                }
            }
        if (!inserted)
            classes.push_back(ecClass);
        }
    ecClasses.assign(classes.begin(), classes.end());
    }

//---------------------------------------------------------------------------------------
// @remarks      sorts the classes by className (ignoring case), default order is ascending
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
template <typename T>
void ECSchemaConverter::SortClassesByName(bvector<T>& ecClasses, bool ascending)
    {
    auto classComparer = [](T ecClass1, T ecClass2)
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
        defaultOrder += ecClass->GetName() + ",";
        classes.push_back(ecClass);
        }

    if (classes.size() < 2)
        return classes;

    SortClassesByNameAndHierarchy(classes);
    Utf8String sortedOrder = "";
    std::for_each(classes.begin(), classes.end(), 
        [&sortedOrder](ECClassP& ecClass) 
        { 
        sortedOrder += ecClass->GetName() + ","; 
        });

    LOG.tracev("Default Order For classes in schema: %s", defaultOrder.c_str());
    LOG.tracev("Name and Hierarchical Sorted Order For classes in schema: %s", sortedOrder.c_str());
    return classes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance)
    {
    //are standard values even added to ecclass or ecschema?
    ECPropertyP prop = dynamic_cast<ECPropertyP> (&container);
    if (nullptr == prop)
        return ECObjectsStatus::PropertyNotFound;

    ECObjectsStatus status;
    StandardValueInfo sdInfo;
    if (ECObjectsStatus::Success != (status = StandardValueInfo::ExtractInstanceData(instance, sdInfo)))
        {
        LOG.errorv("Unable to extract '%s' Standard Value. Status %d", prop->GetName().c_str(), status);
        return status;
        }
    Utf8String classes;
    if (ECObjectsStatus::DataTypeMismatch == CheckForConflict(prop, schema, sdInfo, classes))
        {
        LOG.warningv("Conflict between Standard Values for the Property %s. Used by following classes: %s. NO ENUM will be added !!!", prop->GetName().c_str(), classes.c_str());
        status = ECObjectsStatus::Success;
        }
    else
        {
        SetStrictness(prop, schema, sdInfo);
        status = AddEnumeration(schema, sdInfo, prop);
        }
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Something went wrong while adding Enumeration for '%s' Standard Value", prop->GetName().c_str());
        return status;
        }

    return ECSchemaConverter::RemoveCustomAttribute(prop, schema, "StandardValues");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::CheckForConflict(ECPropertyP standardValueProperty, ECSchemaR schema,StandardValueInfo& sdInfo, Utf8String& classes)
    {
    bool conflict = false;
    classes = standardValueProperty->GetClass().GetFullName();
    auto propertyProcessor = [&conflict, &sdInfo, &classes, &standardValueProperty](ECPropertyP localProperty)
        {
        if (standardValueProperty == localProperty)
            return ECObjectsStatus::Success;

        IECInstancePtr currentInstance = localProperty->GetPrimaryCustomAttributeLocal("StandardValues");
        if (!currentInstance.IsValid())
            return ECObjectsStatus::Success;

        classes.append(",");
        classes.append(localProperty->GetClass().GetFullName());
        StandardValueInfo localInfo;
        StandardValueInfo::ExtractInstanceData(*currentInstance, localInfo);
        if (!localInfo.Equals(sdInfo))
            conflict = true;
        return ECObjectsStatus::Success;
        };

    ECSchemaConverter::TraverseProperty(standardValueProperty, schema, propertyProcessor);
    if (conflict)
        return ECObjectsStatus::DataTypeMismatch;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
void StandardValuesConverter::SetStrictness(ECPropertyP standardValueProperty, ECSchemaR schema, StandardValueInfo& sdInfo)
    {
    ECClassP rootClass = ECSchemaConverter::FindRootBaseClass(standardValueProperty, schema);
    ECPropertyP rootProperty = rootClass->GetPropertyP(standardValueProperty->GetName(), false);

    IECInstancePtr rootInstance = rootProperty->GetPrimaryCustomAttributeLocal("StandardValues");
    if (!rootInstance.IsValid())
        sdInfo.m_mustBeFromList = false;
    else
        {
        StandardValueInfo rootInfo;
        StandardValueInfo::ExtractInstanceData(*rootInstance, rootInfo);
        sdInfo.m_mustBeFromList = rootInfo.m_mustBeFromList;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::AddEnumeration(ECSchemaR schema, StandardValueInfo& sdInfo, ECPropertyP& ecProperty)
    {
    ECEnumerationP enumeration = nullptr;

    ECObjectsStatus status = FindEnumeration(schema, enumeration, sdInfo);

    if (ECObjectsStatus::EnumerationNotFound == status)
        {
        ECClassP rootClass = nullptr;
        Utf8String enumName = CreateEnumerationName(rootClass, schema, ecProperty);
        status = CreateEnumeration(enumeration, rootClass->GetSchemaR(), enumName.c_str(), sdInfo);

        int number = 1;
        while (ECObjectsStatus::NamedItemAlreadyExists == status)
            {
            Utf8String newEnumName = AppendNumberToString(enumName.c_str(), number);
            status = CreateEnumeration(enumeration, rootClass->GetSchemaR(), newEnumName.c_str(), sdInfo);
            number++;
            }
        }

    if (ECObjectsStatus::Success != status)
        return status;

    return ConvertToEnumType(ecProperty, schema, enumeration);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StandardValuesConverter::CreateEnumerationName(ECClassP& rootClass, ECSchemaR schema, ECPropertyP& ecProperty)
    {
    rootClass = ECSchemaConverter::FindRootBaseClass(ecProperty, schema);
    Utf8String baseClassName = rootClass->GetName();
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
// @bsimethod                                    Basanta.Kharel                  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::FindEnumeration(ECSchemaR schema, ECEnumerationP& enumeration, StandardValueInfo& sdInfo)
    {
    auto enumSize = sdInfo.m_valuesMap.size();
    for (auto ecEnum : schema.GetEnumerations())
        {
        if (PrimitiveType::PRIMITIVETYPE_Integer != ecEnum->GetType()
            || enumSize != ecEnum->GetEnumeratorCount())
            continue;

        StandardValueInfo enumSdInfo(ecEnum);
        if (sdInfo.Equals(enumSdInfo) 
            && sdInfo.m_mustBeFromList == enumSdInfo.m_mustBeFromList)
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
// @bsimethod                                    Basanta.Kharel                  01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus StandardValuesConverter::ConvertToEnumType(ECPropertyP& primitiveEcProperty, ECSchemaR ecSchema, ECEnumerationP& enumeration)
    {
    auto propertyProcessor = [&enumeration](ECPropertyP localProp)
        {
        if (!localProp->GetIsPrimitive())
            return ECObjectsStatus::DataTypeMismatch;

        ECObjectsStatus status = ECObjectsStatus::Success;
        PrimitiveECPropertyP primitive = localProp->GetAsPrimitivePropertyP();

        if (nullptr == primitive->GetEnumeration())
            status = primitive->SetType(*enumeration);
        else if (primitive->GetEnumeration() != enumeration)
            return ECObjectsStatus::DataTypeMismatch;

        return status;
        };

    return ECSchemaConverter::TraverseProperty(primitiveEcProperty, ecSchema, propertyProcessor);
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
        container.RemoveCustomAttribute("UnitSpecification");
        return ECObjectsStatus::Success;
        }

    Unit oldUnit;
    if(!Unit::GetUnitForECProperty(oldUnit, *prop))
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.errorv("The property %s:%s.%s has a UnitSpecification but it does not resolve to a unit.", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str());
        return ECObjectsStatus::Error;
        }

    Units::UnitCP newUnit = Units::UnitRegistry::Instance().LookupUnitUsingOldName(oldUnit.GetName());
    if (nullptr == newUnit)
        {
        Utf8String fullName = schema.GetFullSchemaName();
        LOG.warningv("The property %s:%s.%s has an old unit '%s' that does not resolve to a new unit.  Creating a dummy unit to continue", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldUnit.GetName());
        newUnit = Units::UnitRegistry::Instance().AddDummyUnit(oldUnit.GetName());
        //return ECObjectsStatus::Error;
        }

    KindOfQuantityP newKOQ;
    Utf8CP newKOQName = UnitSpecification::GetNewKOQName(instance);
    if (Utf8String::IsNullOrEmpty(newKOQName))
        newKOQName = newUnit->GetPhenomenon()->GetName();

    newKOQ = schema.GetKindOfQuantityP(newKOQName);
    if (nullptr != newKOQ)
        {
        if (!newKOQ->GetPersistenceUnit().Equals(newUnit->GetName()))
            {
            Utf8String fullName = schema.GetFullSchemaName();
            LOG.infov("Found property %s:%s.%s with KindOfQuantity '%s' and unit '%s' but the KindOfQuantity defines the unit '%s'.  Looking for alternate KindOFQuantity",
                        fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), newKOQName, newUnit->GetName(), newKOQ->GetPersistenceUnit().c_str());

            Utf8PrintfString newKoqName("%s_%s", newKOQName, newUnit->GetName());
            newKOQ = schema.GetKindOfQuantityP(newKoqName.c_str());
            if (nullptr == newKOQ)
                {
                schema.CreateKindOfQuantity(newKOQ, newKoqName.c_str());
                newKOQ->SetPersistenceUnit(newUnit->GetName());
                }
            }
        }
    else
        {
        schema.CreateKindOfQuantity(newKOQ, newKOQName);
        newKOQ->SetPersistenceUnit(newUnit->GetName());
        }


    Unit oldDisplayUnit;
    Utf8String oldFormatString;
    if (Unit::GetDisplayUnitAndFormatForECProperty(oldDisplayUnit, oldFormatString, oldUnit, *prop) && (0 != strcmp(oldDisplayUnit.GetName(), oldUnit.GetName())))
        {
        if (Utf8String::IsNullOrEmpty(newKOQ->GetDefaultPresentationUnit().c_str()))
            {
            Units::UnitCP newDisplayUnit = Units::UnitRegistry::Instance().LookupUnitUsingOldName(oldDisplayUnit.GetName());
            if (nullptr == newUnit)
                {
                Utf8String fullName = schema.GetFullSchemaName();
                LOG.warningv("The property %s:%s.%s has an old display unit '%s' that does not resolve to a new unit.", fullName.c_str(), prop->GetClass().GetName().c_str(), prop->GetName().c_str(), oldUnit.GetName());
                }
            else
                newKOQ->SetDefaultPresentationUnit(newDisplayUnit->GetName());
            }
        }

    prop->SetKindOfQuantity(newKOQ);
    prop->RemoveCustomAttribute(UNIT_ATTRIBUTES, UNIT_SPECIFICATION);
    prop->RemoveCustomAttribute(UNIT_ATTRIBUTES, DISPLAY_UNIT_SPECIFICATION);

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
    schema.RemoveReferencedSchema(instance.GetClass().GetSchema().GetSchemaKey());
    return ECObjectsStatus::Success;
    }

bmap<Utf8String, CustomAttributeReplacement> StandardCustomAttributeReferencesConverter::s_entries = bmap<Utf8String, CustomAttributeReplacement>();
bool StandardCustomAttributeReferencesConverter::s_isInitialized = false;
ECSchemaReadContextPtr schemaContext = NULL;

//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
ECObjectsStatus StandardCustomAttributeReferencesConverter::Convert
(
ECSchemaR schema,
IECCustomAttributeContainerR container,
IECInstanceR sourceCustomAttribute
)
    {
    //
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

    SchemaKey key(mapping.newSchemaName.c_str(), 1, 0);
    auto customAttributeSchema = ECSchema::LocateSchema(key, *schemaContext);

    ECClassP customAttributeClass = customAttributeSchema->GetClassP(mapping.newCustomAttributeName.c_str());
    IECInstancePtr targetAttributeInstance = customAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    schema.AddReferencedSchema(*customAttributeSchema);

    ECObjectsStatus propertyConversionStatus;
    for (uint32_t i = 0; i < sourceCustomAttributeClass->GetPropertyCount(); i++)
        {
        ECPropertyP propP = sourceCustomAttributeClass->GetPropertyByIndex((uint32_t)i);

        propertyConversionStatus = ConvertPropertyValue(propP->GetName(), sourceCustomAttribute, *targetAttributeInstance);
        if (ECObjectsStatus::Success != propertyConversionStatus)
            return propertyConversionStatus;
        }

    // Remove the old Custom Attribute and add the new one to the container
    if (!container.RemoveCustomAttribute(mapping.oldSchemaName, mapping.oldCustomAttributeName))
        {
        LOG.errorv("Couldn't remove the CustomAttribute %s from %s", sourceCustomAttributeClass->GetName().c_str(), GetContainerName(container).c_str());
        return ECObjectsStatus::Error;
        }

    ECObjectsStatus status;
    if ((status = container.SetCustomAttribute(*targetAttributeInstance)) != ECObjectsStatus::Success)
        return status;

    return ECObjectsStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
bmap<Utf8String, CustomAttributeReplacement> const& StandardCustomAttributeReferencesConverter::GetCustomAttributesMapping()
    {
    if (!s_isInitialized)
        {
        // Converts reference of DateTimeInfo CA to the new class
        AddMapping("Bentley_Standard_CustomAttributes", "DateTimeInfo", "CoreCustomAttributes", "DateTimeInfo");
        AddMapping("Bentley_Standard_CustomAttributes", "ClassHasCurrentTimeStampProperty", "CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
        
        // Add More...
            
        s_isInitialized = true;
        }
    
        return s_entries;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
ECObjectsStatus StandardCustomAttributeReferencesConverter::AddMapping
(
Utf8CP oSchema, 
Utf8CP oName, 
Utf8CP nSchema, 
Utf8CP nName
)
    {
    Utf8String qualifiedName = ECSchemaConverter::GetQualifiedClassName(oSchema, oName);
    s_entries[qualifiedName] = CustomAttributeReplacement(oSchema, oName, nSchema, nName);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
Utf8String StandardCustomAttributeReferencesConverter::GetContainerName
(
IECCustomAttributeContainerR container
) const
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
//---------------------------------------------------------------------------------
ECObjectsStatus StandardCustomAttributeReferencesConverter::ConvertPropertyValue
(
Utf8StringCR propertyName,
IECInstanceR sourceCustomAttribute,
IECInstanceR targetCustomAttribute
)
    {
    ECObjectsStatus status;

    ECValue sourceValue;
    if ((status = sourceCustomAttribute.GetValue(sourceValue, propertyName.c_str())) != ECObjectsStatus::Success)
        return status;

    // Checks if property Value is actually set. In case it isn't the conversion will be stopped
    // as only properties that are defined on the source Custom Attribute will be set on the target
    // Custom Attribute.
    if (sourceValue.IsUninitialized() || sourceValue.IsNull())
        return ECObjectsStatus::Success;

    ECValue targetValue;
    if ((status = targetCustomAttribute.GetValue(targetValue, propertyName.c_str())) != ECObjectsStatus::Success)
        return status;

    auto targetProperty = targetCustomAttribute.GetClass().GetPropertyP(propertyName);
    if (!targetProperty->GetIsPrimitive())
        {
        LOG.errorv("Complex Properties are not supported in the current version of StandardCustomAttributeReferencesConverter");
        return ECObjectsStatus::Error;
        }

    auto targetPrimitiveProperty = targetProperty->GetAsPrimitiveProperty();
    auto enumeration = targetPrimitiveProperty->GetEnumeration();
    if (enumeration != nullptr)
        {
        return ConvertPropertyToEnum(propertyName, *enumeration, targetCustomAttribute, targetValue, sourceValue);
        }

    targetValue.From(sourceValue);
    targetCustomAttribute.SetValue(propertyName.c_str(), targetValue);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
ECObjectsStatus StandardCustomAttributeReferencesConverter::ConvertPropertyToEnum
(
Utf8StringCR propertyName,
ECEnumerationCR enumeration,
IECInstanceR targetCustomAttribute,
ECValueR targetValue,
ECValueR sourceValue
)
    {
    auto enumerationValue = FindEnumerationValue(enumeration, sourceValue.GetUtf8CP());
    if (enumerationValue == -1)
        {
        LOG.errorv("Couldn't find value '%s' in ECEnumeration %s", sourceValue.GetUtf8CP(), enumeration.GetName().c_str());
        return ECObjectsStatus::ParseError;
        }

    if (!targetValue.IsInteger() || targetValue.SetInteger(enumerationValue) != BentleyStatus::SUCCESS)
        {
        LOG.errorv("Couldn't set value of %s to %d", propertyName.c_str(), enumerationValue);
        return ECObjectsStatus::Error;
        }

    if (targetCustomAttribute.SetValue(propertyName.c_str(), targetValue) != ECObjectsStatus::Success)
        {
        LOG.errorv("Couldn't set value of %s to %d", propertyName.c_str(), enumerationValue);
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     
//---------------------------------------------------------------------------------
int StandardCustomAttributeReferencesConverter::FindEnumerationValue(ECEnumerationCR enumeration, Utf8CP displayName)
    {
    for (auto enumerator : enumeration.GetEnumerators())
        {
        if (enumerator->GetDisplayLabel() == displayName)
            {
            return enumerator->GetInteger();
            }
        }
    return -1;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
