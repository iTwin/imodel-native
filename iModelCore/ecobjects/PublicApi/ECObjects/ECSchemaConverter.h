/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchemaConverter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECSchema.h>

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
    virtual ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance) = 0;

    //! destructor
    virtual ~IECCustomAttributeConverter() {};
    };

typedef std::function<ECObjectsStatus(ECClassP)> ECClassProcessor;
typedef std::function<ECObjectsStatus(ECPropertyP)> ECPropertyProcessor;
typedef RefCountedPtr<IECCustomAttributeConverter> IECCustomAttributeConverterPtr;

struct ECSchemaConverter
    {
private:
    ECSchemaConverter() {}
    ECSchemaConverter(const ECSchemaConverter & rhs) = delete;
    ECSchemaConverter & operator= (const ECSchemaConverter & rhs) = delete;

    static ECSchemaConverterP GetSingleton();
    bool m_convertedOK = true;
    bmap<Utf8String, IECCustomAttributeConverterPtr> m_converterMap;
    bvector<Utf8String> m_schemaReferencesToRemove;

    void ProcessCustomAttributeInstance(ECCustomAttributeInstanceIterable iterable, IECCustomAttributeContainerR container, Utf8String containerName);
    void ProcessRelationshipConstraint(ECRelationshipConstraintR constraint, bool isSource);
    void ConvertSchemaLevel(ECSchemaR schema) {ProcessCustomAttributeInstance(schema.GetCustomAttributes(false), schema.GetCustomAttributeContainer(), "ECSchema:" + schema.GetName());}
    void ConvertClassLevel(bvector<ECClassP>& classes);
    void ConvertPropertyLevel(bvector<ECClassP>& classes);
    void RemoveSchemaReferences(ECSchemaR schema);
    IECCustomAttributeConverterP GetConverter(Utf8StringCR converterName);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts classes first by name(ascending) then by hierarchy (descending) in order
    //               if reverse is set it will reverse after sorting name and hierarchy in order
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //---------------+---------------+---------------+---------------+---------------+------
    static void SortClassesByNameAndHierarchy(bvector<ECClassP>& ecClasses, bool reverse = false);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts the classes by className (ignoring case), default order is ascending
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------/
    static void SortClassesByName(bvector<ECClassP>& ecClasses, bool ascending = true);

    //---------------------------------------------------------------------------------
    // @remarks      sorts classes based on inheritance where base class comes before child class
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //---------------+---------------+---------------+---------------+---------------+------/
    static void SortClassesByHierarchy(bvector<ECClassP>& ecClasses);

    static bvector<ECClassP> GetDerivedAndBaseClasses(ECClassCR ecClass);

public:
    //! Traverses the schema supplied and calls converters based on schemaName:customAttributeName
    //! @param[in] schema   The schema to traverse
    ECOBJECTS_EXPORT static bool Convert(ECSchemaR schema);

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] schemaName   The schemaName that the customattribute belongs to
    //! @param[in] customAttributeName The name of customAttribute
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if schemaName+customAttribute name already exists. 
    ECOBJECTS_EXPORT static ECObjectsStatus AddConverter(Utf8StringCR schemaName, Utf8StringCR customAttributeName, IECCustomAttributeConverterPtr& converter);

    //! Adds the supplied IECCustomAttributeConverterP which will be later called when ECSchemaConverter::Convert is run
    //! @param[in] customAttributeQualifiedName Key used to retrieve converter
    //! @param[in] converter The converter that is to be called when schemaName:customAtrributeName is found
    //! @remarks   Overwrites converter if key already exists. 
    ECOBJECTS_EXPORT static ECObjectsStatus AddConverter(Utf8StringCR customAttributeQualifiedName, IECCustomAttributeConverterPtr& converter);

    //! Adds the name of a schema to remove at the end of schema conversion.
    //! @param[in] schemaName   The name of the schema to remove.  Name only, do not include version.
    ECOBJECTS_EXPORT static void AddSchemaReferenceToRemove(Utf8CP schemaName) { ECSchemaConverter::GetSingleton()->m_schemaReferencesToRemove.push_back(schemaName); };

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

    static Utf8String GetQualifiedClassName(Utf8StringCR schemaName, Utf8StringCR className) { return schemaName + ":" + className; }
    };

//+===============+===============+===============+===============+===============+======
// Data of standard values and the property that it is attached to
// @bsiclass                                                    Basanta.Kharel   12/2015
//+===============+===============+===============+===============+===============+======
struct StandardValueInfo
    {
    friend struct StandardValuesConverter;
private: 
    bmap<int, Utf8String> m_valuesMap;
    bool m_mustBeFromList;

public:
    bool Equals(const StandardValueInfo& sd) const;
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
// @bsiclass                                                    Basanta.Kharel   12/2015
//+===============+===============+===============+===============+===============+======
struct StandardValuesConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    
private:
    //! Gives name to enumeration, based on className and propertyName 
    //! Usually is (baseClassName+propertyName)
    //! returns rootClass i.e. baseclass
    static Utf8String CreateEnumerationName(bvector<ECClassP>& rootClasses, ECPropertyP& ecProperty);

    //! Append number to string : returns name_number
    static Utf8String AppendNumberToString(Utf8CP name, int number);

    //! Attempts to merge the StandardValueInfo into the ECEnumeration
    static ECObjectsStatus MergeEnumeration(ECEnumerationP& enumeration, StandardValueInfo& sdInfo);

    //! Finds enumeration in schema that matches sdInfo
    static ECObjectsStatus FindEnumeration(ECSchemaR schema, ECEnumerationP& enumeration, StandardValueInfo& sdInfo);

    //! Creates enumeration in schema : given enumName, sdInfo
    static ECObjectsStatus CreateEnumeration(ECEnumerationP& enumeration, ECSchemaR schema, Utf8CP enumName, StandardValueInfo& sdInfo);

    //! Converts all derived class's properties to the given enum
    //! @param[in] rootClass                The class started with for conversion
    //! @param[in] currentClass             The class currently being converted
    //! @param[in] propName                 The name of the property to be converted
    //! @param[in] enumeration              The enumeration to be converted to.
    //! @param[in] sdInfo                   The standard value info of the enumeration
    static ECObjectsStatus ConvertToEnum (ECClassP rootClass, ECClassP currentClass, Utf8CP propName, ECEnumerationP enumeration, StandardValueInfo sdInfo);

    //! Tries to merge the current property's StandardValue info with the existing enumeration/sdInfo
    //! @param[in] ecProperty               The property with the standard value to merge
    //! @param[in] sdInfo                   The standard value info to of the enumeration to merge with
    //! @param[in] enumeration              The enumeration to merge with
    static ECObjectsStatus Merge(ECPropertyP ecProperty, StandardValueInfo* sdInfo, ECEnumerationP enumeration);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert UnitSpecification Custom Attribute to KindOfQuantity
// @bsiclass                                                    Robert.Schili   03/2016
//+===============+===============+===============+===============+===============+======
struct UnitSpecificationConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    };

//+===============+===============+===============+===============+===============+======
// Implements IECCustomAttributeConverter to convert UnitSpecifications Custom Attribute to KindOfQuantity
// @bsiclass                                                    Robert.Schili   03/2016
//+===============+===============+===============+===============+===============+======
struct UnitSpecificationsConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    };

//---------------------------------------------------------------------------------------
// Removes UnitSystem custom attributes from the schema
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct UnitSystemConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
    };

//---------------------------------------------------------------------------------------
// Removes ECA:PropertyPriority custom attributes from the schema
// @bsiclass                                   Caleb.Shafer                 06/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct PropertyPriorityConverter : IECCustomAttributeConverter
    {
    ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
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
// Implements IECCustomAttributeConverter to convert the schema references of certain Custom Attributes.
// Which attributes will be handled depends which CustomATtributeEntries will returned from the
// StandardCustomAttributeReferencesConverter::GetCustomAttributesToConvert method.
// @bsiclass                                                     Stefan.Apfel   04/2016
//+===============+===============+===============+===============+===============+======
struct StandardCustomAttributeReferencesConverter : IECCustomAttributeConverter
    {
    private:
        static bool s_isInitialized;
        static bmap<Utf8String, CustomAttributeReplacement> s_entries;

        static ECObjectsStatus AddMapping(Utf8CP oSchema, Utf8CP oName, Utf8CP nSchema, Utf8CP nName);
        static ECObjectsStatus AddPropertyMapping(Utf8CP oldName, Utf8CP propertyName, Utf8CP newPropertyName);

    public:
        ECObjectsStatus Convert(ECSchemaR schema, IECCustomAttributeContainerR container, IECInstanceR instance);
        static bmap<Utf8String, CustomAttributeReplacement> const& GetCustomAttributesMapping();
        ECObjectsStatus ConvertPropertyValue(Utf8CP sourcePropAccessor, CustomAttributeReplacement mapping, IECInstanceR sourceCustomAttribute, IECInstanceR targetCustomAttribute);
        ECObjectsStatus ConvertPropertyToEnum(Utf8StringCR propertyName, ECEnumerationCR enumeration, IECInstanceR targetCustomAttribute, ECValueR targetValue, ECValueR sourceValue);
        Utf8String GetContainerName(IECCustomAttributeContainerR container) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE
