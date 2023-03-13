/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECContext.h>
#include <ECObjects/IssueReporter.h>
#include <unordered_set>
#include <string>

namespace std
    {
    // Implementation of hash function for user-defined class Utf8String
    template<> 
    struct hash<Utf8String>
        {
        typedef Utf8String argument_type;
        typedef size_t result_type;
        result_type operator()(argument_type const& string) const { return hash<std::string>{}(string.c_str()); }
        };
    }

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=====================================================================================
//Interface for Custom Attribute converter. When implemented and add to ECSchemaConverter,
//it will be called to modify the related custom attribute
//@bsiclass
//+===============+===============+===============+===============+===============+======
struct IECCustomAttributeConverter : RefCountedBase, NonCopyableClass
    {
    //! Converts the custom attribute
    //! @param[in] schema   The schema that holds the container
    //! @param[in] container   The custom attribute container that holds the instance
    //! @param[in] instance    The custom attribute instance
    //! @param[in] context     A context to deserialize and additional ECSchemas that are not already referenced by the schema.
    virtual ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context) = 0;

    virtual ~IECCustomAttributeConverter() {};
    };

typedef RefCountedPtr<IECCustomAttributeConverter> IECCustomAttributeConverterPtr;
typedef RefCountedPtr<CustomECSchemaConverter> CustomECSchemaConverterPtr;

//+===============+===============+===============+===============+===============+======
// Converts schemas using registered converters.  Use the ECSchemaConverter to do standard conversion.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct CustomECSchemaConverter : RefCountedBase, NonCopyableClass
    {
private:
    bool m_convertedOK = true;
    bool m_removeLegacyStandardCustomAttributes = false;
    bmap<Utf8String, IECCustomAttributeConverterPtr> m_converterMap;
    bvector<Utf8String> m_schemaReferencesToRemove;
    std::unordered_set<Utf8String> m_customAttributesToRemove;

    void ProcessCustomAttributeInstances(IECCustomAttributeContainerR container, Utf8String containerName, ECSchemaReadContextR context);
    static void ProcessRelationshipConstraint(ECRelationshipConstraintR constraint, bool isSource);
    void ConvertSchemaLevel(ECSchemaR schema, ECSchemaReadContextR context) { ProcessCustomAttributeInstances(schema.GetCustomAttributeContainer(), "ECSchema:" + schema.GetName(), context); }
    void ConvertClassLevel(bvector<ECClassP>& classes, ECSchemaReadContextR context);
    void ConvertPropertyLevel(bvector<ECClassP>& classes, ECSchemaReadContextR context);
    void RemoveSchemaReferences(ECSchemaR schema);
    IECCustomAttributeConverterP GetConverter(Utf8StringCR converterName);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts classes first by name(ascending) then by hierarchy (descending) in order
    //               if reverse is set it will reverse after sorting name and hierarchy in order
    // @bsimethod
    //---------------+---------------+---------------+---------------+---------------+------
    static void SortClassesByNameAndHierarchy(bvector<ECClassP>& ecClasses, bool reverse = false);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts the classes by className (ignoring case), default order is ascending
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+------/
    static void SortClassesByName(bvector<ECClassP>& ecClasses, bool ascending = true);

    //---------------------------------------------------------------------------------
    // @remarks      sorts classes based on inheritance where base class comes before child class
    // @bsimethod
    //---------------+---------------+---------------+---------------+---------------+------/
    static void SortClassesByHierarchy(bvector<ECClassP>& ecClasses);

    static bvector<ECClassP> GetDerivedAndBaseClasses(ECClassCR ecClass);

    //---------------------------------------------------------------------------------------
    //! @param[in] customAttributeFullName Custom Attribute Full name in the form <SchemaName>:<CustomAttributeClassName>
    //! @return Returns true if a match is found. Else, returns false
    // @remarks      Searches the list for a matching custom attribute
    //+---------------+---------------+---------------+---------------+---------------+------/
    bool ShouldRemoveCustomAttribute(const Utf8String& customAttributeFullName) const { return (m_customAttributesToRemove.find(customAttributeFullName) != m_customAttributesToRemove.end()); }

public:
    CustomECSchemaConverterPtr static Create() { return new CustomECSchemaConverter(); }

    //! Converts EC2 schema metadata to EC3 concepts by traversing custom attributes of the supplied schema and calling converters based on schemaName:customAttributeName
    //! @param[in] schema   The schema to traverse
    //! @param[in] context  A context to use if a particular converter needs to locate a schema which is not currently a schema reference.
    //! @param[in] doValidate Flag saying whether to validate the schema or not.  This is used by the DgnV8Converter to disable validation until it has had a chance to fix the schemas
    ECOBJECTS_EXPORT bool Convert(ECSchemaR schema, ECSchemaReadContextR context, bool doValidate = true);

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] schemaName   The schemaName that the customattribute belongs to
    //! @param[in] customAttributeName The name of customAttribute
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if schemaName+customAttribute name already exists. 
    ECOBJECTS_EXPORT ECObjectsStatus AddConverter(Utf8StringCR schemaName, Utf8StringCR customAttributeName, IECCustomAttributeConverterPtr& converter);

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] customAttributeQualifiedName Key used to retrieve converter
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if key already exists. 
    ECOBJECTS_EXPORT ECObjectsStatus AddConverter(Utf8StringCR customAttributeQualifiedName, IECCustomAttributeConverterPtr& converter);

    //! Removes the IECCustomAttributeConverterP associated with the given customAttributeQualifiedName, if it exists
    //! @param[in] customAttributeQualifiedName Key used to retrieve converter
    ECOBJECTS_EXPORT ECObjectsStatus RemoveConverter(Utf8StringCR customAttributeQualifiedName);

    //! Adds the name of a schema to remove at the end of schema conversion.
    //! @param[in] schemaName   The name of the schema to remove.  Name only, do not include version.
    void AddSchemaReferenceToRemove(Utf8CP schemaName) { m_schemaReferencesToRemove.push_back(schemaName); };

    //! Gets a vector of EClasses sorted by hierarchy in descending order (parent comes first)
    //! @param[in] schema           The schema whose classes are to be sorted
    //! @returns bvector<ECClassP>  the hierarchically sorted list in descending order
    //! @remarks   First sorts the classes by className in ascending order(ignoring case)
    //!            EC 2.0 does this by default but doesnot ignore cases. 3.0 should fix that
    ECOBJECTS_EXPORT static bvector<ECClassP> GetHierarchicallySortedClasses(ECSchemaR schema);

    //! Finds the root base classes for the property
    //! @param[in] ecProperty The ecProperty 
    //! @param[in] rootClasses The rootClasses of the ecclass found for the ecProperty 
    //! @remarks If the property has multiple inheritance, the first class in the rootClasses vector will be the root class
    //! that would be found if you traversed the base property of the input ecProperty.
    ECOBJECTS_EXPORT static void FindRootBaseClasses(ECPropertyP ecProperty, bvector<ECClassP>& rootClasses);

    //! Turns removal of standard custom attributes from the EC2 standard schemas on and off.  Defaults to false.
    void SetRemoveLegacyStandardCustomAttributes(bool removeLegacy) { m_removeLegacyStandardCustomAttributes = removeLegacy; }
    //! Gets flag which determines if standard custom attributes from the EC2 standard standard schemas are removed.  Defaults to false.
    bool GetRemoveLegacyStandardCustomAttributes() { return m_removeLegacyStandardCustomAttributes; }

    static Utf8String GetQualifiedClassName(Utf8StringCR schemaName, Utf8StringCR className) { return schemaName + ":" + className; }

    //! @param[in] customAttributeFullName Custom Attribute Full name in the form <SchemaName>:<CustomAttributeClassName>
    // @remarks      Adds a new Custom attribute class to the list, if a matching entry does not already exist.
    ECOBJECTS_EXPORT void AddCustomAttributeNameToRemove(const Utf8String& customAttributeFullName) { m_customAttributesToRemove.emplace(customAttributeFullName); }

    //! @param[in] customAttributeFullName Custom Attribute Full name in the form <SchemaName>:<CustomAttributeClassName>
    // @remarks      Removes the Custom attribute class supplied in the arg from the list if it exists.
    ECOBJECTS_EXPORT void RemoveCustomAttributeNameToRemove(const Utf8String& customAttributeFullName) { m_customAttributesToRemove.erase(customAttributeFullName); }
    };

//+===============+===============+===============+===============+===============+======
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSchemaConverter
    {
private:
    ECSchemaConverter() {}
    ECSchemaConverter(const ECSchemaConverter & rhs) = delete;
    ECSchemaConverter & operator= (const ECSchemaConverter & rhs) = delete;

    ECOBJECTS_EXPORT static CustomECSchemaConverterP GetSingleton();

public:
    //! Traverses the schema supplied and calls converters based on schemaName:customAttributeName
    //! @param[in] schema   The schema to traverse
    //! @param[in] context  A context to use if a particular converter needs to locate a schema which is not currently a schema reference.
    //! @param[in] doValidate Flag saying whether to validate the schema or not.  This is used by the DgnV8Converter to disable validation until it has had a chance to fix the schemas
    static bool Convert(ECSchemaR schema, ECSchemaReadContextR context, bool doValidate = true)
        {
        bool returnVal = GetSingleton()->Convert(schema, context, doValidate);
        return returnVal;
        }

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] schemaName   The schemaName that the customattribute belongs to
    //! @param[in] customAttributeName The name of customAttribute
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if schemaName+customAttribute name already exists. 
    static ECObjectsStatus AddConverter(Utf8StringCR schemaName, Utf8StringCR customAttributeName, IECCustomAttributeConverterPtr& converter) 
        { return GetSingleton()->AddConverter(schemaName, customAttributeName, converter); }

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] customAttributeQualifiedName Key used to retrieve converter
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if key already exists. 
    static ECObjectsStatus AddConverter(Utf8StringCR customAttributeQualifiedName, IECCustomAttributeConverterPtr& converter)
        { return GetSingleton()->AddConverter(customAttributeQualifiedName, converter); }

    static ECObjectsStatus RemoveConverter(Utf8StringCR customAttributeQualifiedName) { return GetSingleton()->RemoveConverter(customAttributeQualifiedName); }

    //! Adds the name of a schema to remove at the end of schema conversion.
    //! @param[in] schemaName   The name of the schema to remove.  Name only, do not include version.
    static void AddSchemaReferenceToRemove(Utf8CP schemaName) { GetSingleton()->AddSchemaReferenceToRemove(schemaName); };

    //! @param[in] customAttributeFullName Custom Attribute Full name in the form <SchemaName>:<CustomAttributeClassName>
    // @remarks      Adds a new Custom attribute class to the list, if a matching entry does not already exist.
    static void AddCustomAttributeNameToRemove(const Utf8String& customAttributeFullName) { GetSingleton()->AddCustomAttributeNameToRemove(customAttributeFullName); }

    //! @param[in] customAttributeFullName Custom Attribute Full name in the form <SchemaName>:<CustomAttributeClassName>
    // @remarks      Removes the Custom attribute class supplied in the arg from the list if it exists.
    static void RemoveCustomAttributeNameToRemove(const Utf8String& customAttributeFullName) { GetSingleton()->RemoveCustomAttributeNameToRemove(customAttributeFullName); }

    //! Gets a vector of EClasses sorted by hierarchy in descending order (parent comes first)
    //! @param[in] schema           The schema whose classes are to be sorted
    //! @returns bvector<ECClassP>  the hierarchically sorted list in descending order
    //! @remarks   First sorts the classes by className in ascending order(ignoring case)
    //!            EC 2.0 does this by default but doesnot ignore cases. 3.0 should fix that
    static bvector<ECClassP> GetHierarchicallySortedClasses(ECSchemaR schema) { return CustomECSchemaConverter::GetHierarchicallySortedClasses(schema); }

    //! Finds the root base classes for the property
    //! @param[in] ecProperty The ecProperty 
    //! @param[in] rootClasses The rootClasses of the ecclass found for the ecProperty 
    //! @remarks If the property has multiple inheritance, the first class in the rootClasses vector will be the root class
    //! that would be found if you traversed the base property of the input ecProperty.
    static void FindRootBaseClasses(ECPropertyP ecProperty, bvector<ECClassP>& rootClasses) { return CustomECSchemaConverter::FindRootBaseClasses(ecProperty, rootClasses); }

    static Utf8String GetQualifiedClassName(Utf8StringCR schemaName, Utf8StringCR className) { return schemaName + ":" + className; }
    };

//+===============+===============+===============+===============+===============+======
// Converts the ECDbMap.01.00:ClassMap custom attribute to the ECDbMap 2.0 version.  
// Only supports 'SharedTable' mapping with 'AppliesToSubclasses' flag set to true and 
// 'NotMapped' when it applies to an entire class hierarchy.
// Use with the CustomECSchemaConverter like:
// <code>
// CustomECSchemaConverterPtr converter = CustomECSchemaConverter::Create();
// IECCustomAttributeConverterPtr classMapConv = new ECDbClassMapConverter(ECSchemaConverter::Issues());
// converter->AddConverter(ECDbClassMapConverter::GetSchemaName(), ECDbClassMapConverter::GetClassName(), classMapConv);
// converter->Convert(schema, true);
// </code>
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbClassMapConverter : IECCustomAttributeConverter
    {
    public:
        ECOBJECTS_EXPORT static Utf8CP GetSchemaName();
        ECOBJECTS_EXPORT static Utf8CP GetClassName();

        //! Fulfills the IECCustomAttributeConverter interface
        //! @param[in] schema                   The schema being converted.
        //! @param[in] container                The schema element which holds the custom attribute instance being converted
        //! @param[in] instance                 The custom attribute being converted
        //! @param[in] context                  The context to locate the ECDbMap schema from.
        ECOBJECTS_EXPORT ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//+===============+===============+===============+===============+===============+======
// Data of standard values and the property that it is attached to
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct StandardValueInfo
    {
    friend struct StandardValuesConverter;
private: 
    bmap<int, Utf8String> m_valuesMap;
    bool m_mustBeFromList;

public:
    bool Equals(const StandardValueInfo& sd) const;
    bool Equals(ECEnumerationCP ecEnum) const;
    bool ContainedBy(ECEnumerationCP ecEnum) const;
    bool Contains(const StandardValueInfo& sd) const;

    StandardValueInfo(ECEnumerationP& ecEnum);
    StandardValueInfo(ECEnumerationCR ecEnum);
    StandardValueInfo() {};

    //--------------------------------------------------------------------------------------
    // Extracts Standard Values Instance data from the instance
    //! @param[in]  instance    The instance of StandValues customattribute
    //! @param[out] sdInfo      Extracted data 
    //---------------+---------------+---------------+---------------+---------------+------
    static ECObjectsStatus ExtractInstanceData(IECInstanceR instance, StandardValueInfo& sdInfo);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert Standard Values Custom Attribute to ECEnumeration
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct StandardValuesConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context = nullptr);

private:
    //! Gives name to enumeration, based on className and propertyName 
    //! Usually is (baseClassName+propertyName)
    //! returns rootClass i.e. baseclass
    static Utf8String CreateEnumerationName(bvector<ECClassP>& rootClasses, ECPropertyP& ecProperty);

    //! Append number to string : returns name_number
    Utf8String AppendNumberToString(Utf8CP name, int number);

    //! Attempts to merge the StandardValueInfo into the ECEnumeration
    static ECObjectsStatus MergeEnumeration(ECEnumerationP& enumeration, StandardValueInfo& sdInfo);

    //! Finds enumeration in schema that matches sdInfo
    ECObjectsStatus FindEnumeration(ECSchemaR schema, ECEnumerationP& enumeration, StandardValueInfo& sdInfo);

    //! Creates enumeration in schema : given enumName, sdInfo
    static ECObjectsStatus CreateEnumeration(ECEnumerationP& enumeration, ECSchemaR schema, Utf8CP enumName, StandardValueInfo& sdInfo);

    //! Converts all derived class's properties to the given enum
    //! @param[in] rootClass                The class started with for conversion
    //! @param[in] currentClass             The class currently being converted
    //! @param[in] propName                 The name of the property to be converted
    //! @param[in] enumeration              The enumeration to be converted to.
    //! @param[in] sdInfo                   The standard value info of the enumeration
    ECObjectsStatus ConvertToEnum (ECClassP rootClass, ECClassP currentClass, Utf8CP propName, ECEnumerationP enumeration, StandardValueInfo sdInfo);

    //! Tries to merge the current property's StandardValue info with the existing enumeration/sdInfo
    //! @param[in] ecProperty               The property with the standard value to merge
    //! @param[in] sdInfo                   The standard value info to of the enumeration to merge with
    //! @param[in] enumeration              The enumeration to merge with
    static ECObjectsStatus Merge(ECPropertyP ecProperty, StandardValueInfo* sdInfo, ECEnumerationP enumeration);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert UnitSpecification Custom Attribute to KindOfQuantity
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnitSpecificationConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert UnitSpecifications Custom Attribute to KindOfQuantity
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnitSpecificationsConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//---------------------------------------------------------------------------------------
// Removes UnitSystem custom attributes from the schema
// @bsiclass
//---------------+---------------+---------------+---------------+---------------+-------
struct UnitSystemConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//---------------------------------------------------------------------------------------
// Removes ECA:PropertyPriority custom attributes from the schema
// @bsiclass
//---------------+---------------+---------------+---------------+---------------+-------
struct PropertyPriorityConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//---------------------------------------------------------------------------------------
// Removes ECA:Category custom attributes from the schema
// @bsiclass
//---------------+---------------+---------------+---------------+---------------+-------
struct CategoryConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//+===============+===============+===============+===============+===============+======
//@bsiclass
//+===============+===============+===============+===============+===============+======
struct CustomAttributeReplacement final
    {
private:
    Utf8String m_oldSchemaName;
    Utf8String m_oldCustomAttributeName;

    Utf8String m_newSchemaName;
    Utf8String m_newCustomAttributeName;

    bmap<Utf8String, Utf8String> m_propertyMapping;

public:
    CustomAttributeReplacement(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName)
        : m_oldSchemaName(oSchema), m_oldCustomAttributeName(oName), m_newSchemaName(nSchema), m_newCustomAttributeName(nName)
        {
        m_propertyMapping = bmap<Utf8String, Utf8String>();
        }
    CustomAttributeReplacement() {CustomAttributeReplacement("", "", "", "");}

    Utf8String GetOldSchemaName() {return m_oldSchemaName;}
    Utf8String GetOldCustomAttributeName() {return m_oldCustomAttributeName;}
    Utf8String GetNewSchemaName() {return m_newSchemaName;}
    Utf8String GetNewCustomAttributeName() {return m_newCustomAttributeName;}

    ECObjectsStatus AddPropertyMapping(Utf8CP oldPropertyName, Utf8CP newPropertyName);
    Utf8String GetPropertyMapping(Utf8CP oldPropertyName);
    };

//+===============+===============+===============+===============+===============+======
//@bsistruct
//+===============+===============+===============+===============+===============+======
struct HidePropertyConverter : IECCustomAttributeConverter
    {
    public:
        ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//+===============+===============+===============+===============+===============+======
//@bsistruct
//+===============+===============+===============+===============+===============+======
struct DisplayOptionsConverter : IECCustomAttributeConverter
    {
    private:
        static ECObjectsStatus ConvertSchemaDisplayOptions(ECSchemaR schema, IECInstanceR instance, ECSchemaReadContextR context);
        static ECObjectsStatus ConvertClassDisplayOptions(ECSchemaR schema, ECClassR ecClass, IECInstanceR instance, ECSchemaReadContextR context);

    public:
        ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert the schema references of certain Custom Attributes.
// Which attributes will be handled depends which CustomATtributeEntries will returned from the
// StandardCustomAttributeReferencesConverter::GetCustomAttributesToConvert method.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct StandardCustomAttributeReferencesConverter : IECCustomAttributeConverter
    {
    private:
        static bool s_isInitialized;
        static bmap<Utf8String, CustomAttributeReplacement> s_entries;

        static ECObjectsStatus AddMapping(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName);
        static ECObjectsStatus AddPropertyMapping(Utf8CP oldName, Utf8CP propertyName, Utf8CP newPropertyName);

    public:
        ECOBJECTS_EXPORT ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance, ECSchemaReadContextP context);
        static bmap<Utf8String, CustomAttributeReplacement> const& GetCustomAttributesMapping();
        ECObjectsStatus ConvertPropertyValue(Utf8CP sourcePropAccessor, CustomAttributeReplacement mapping, IECInstanceR sourceCustomAttribute, IECInstanceR targetCustomAttribute);
        ECObjectsStatus ConvertPropertyToEnum(Utf8StringCR propertyName, ECEnumerationCR enumeration, IECInstanceR targetCustomAttribute, ECValueR targetValue, ECValueR sourceValue);
        ECOBJECTS_EXPORT Utf8String GetContainerName(IECCustomAttributeContainerR container) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE
