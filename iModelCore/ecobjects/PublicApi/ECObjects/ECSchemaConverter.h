/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchemaConverter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
/*__PUBLISH_SECTION_END__*/
private:
    ECSchemaConverter() { }
    ECSchemaConverter(const ECSchemaConverter & rhs) = delete;
    ECSchemaConverter & operator= (const ECSchemaConverter & rhs) = delete;

    static ECSchemaConverterP GetSingleton();
    bool m_convertedOK = true;
    bmap<Utf8String, IECCustomAttributeConverterPtr> m_converterMap;

    void ProcessCustomAttributeInstance(ECCustomAttributeInstanceIterable iterable, IECCustomAttributeContainerR container, Utf8String containerName);
    void ConvertSchemaLevel(ECSchemaR schema);
    void ConvertClassLevel(ECSchemaR schema);
    void ConvertPropertyLevel(ECSchemaR schema);
    IECCustomAttributeConverterP GetConverter(Utf8StringCR converterName);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts classes first by name(ascending) then by hierarchy (descending) in order
    //               if reverse is set it will reverse after sorting name and hierarchy in order
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //---------------+---------------+---------------+---------------+---------------+------
    template <typename T>
    static void SortClassesByNameAndHierarchy(bvector<T>& ecClasses, bool reverse = false);

    //---------------------------------------------------------------------------------------
    // @remarks      sorts the classes by className (ignoring case), default order is ascending
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //+---------------+---------------+---------------+---------------+---------------+------/
    template <typename T> 
    static void SortClassesByName(bvector<T>& ecClasses, bool ascending = true);

    //---------------------------------------------------------------------------------
    // @remarks      sorts classes based on inheritance where base class comes before child class
    // @bsimethod                                    Basanta.Kharel                  01/2016
    //---------------+---------------+---------------+---------------+---------------+------/
    template <typename T>
    static void SortClassesByHierarchy(bvector<T>& ecClasses);        
    static bvector<ECClassP> GetDerivedAndBaseClasses(ECClassCR ecClass);

/*__PUBLISH_SECTION_START__*/
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

    //! Removes a custom attribute from the ecProperty including its base and child ecProperties
    //! @param[in] ecProperty               The ecProperty whose custom attribute is being removed
    //! @param[in] ecSchema                 The schema the ecProperty belongs to
    //! @param[in] customAttributeName      The name of the cutomAtrribute being removed
    //! @remarks   Traverse the ecProperty hierarchy to remove customattribute from all of them
    ECOBJECTS_EXPORT static ECObjectsStatus RemoveCustomAttribute(ECPropertyP& ecProperty, ECSchemaR ecSchema, Utf8StringCR customAttributeName);

    //! Gets a vector of EClasses sorted by hierarchy in descending order (parent comes first)
    //! @param[in] schema           The schema whose classes are to be sorted
    //! @returns bvector<ECClassP>  the hierarchically sorted list in descending order
    //! @remarks   First sorts the classes by className in ascending order(ignoring case)
    //!            EC 2.0 does this by default but doesnot ignore cases. 3.0 should fix that
    ECOBJECTS_EXPORT static bvector<ECClassP> GetHierarchicallySortedClasses(ECSchemaR schema);

    //! Traveses ecClass up and down the class hierarchy and calls classProcessor 
    //! @param[in] ecClass           The ecClass to traverse
    //! @param[in] classProcessor    The function that is called on each traversed ecClass 
    //! @returns ECObjectsStatus     If any call to classProcessor is not success returns immediately 
    //!                              with that status. Otherwise returns success after traversing all
    //! @remarks It also calls the classProcessor for the ecClass passed in
    ECOBJECTS_EXPORT static ECObjectsStatus TraverseClass(ECClassP ecClass, ECClassProcessor classProcessor);

    //! Traveses property up and down the class hierarchy and calls propertyProcessor 
    //! @param[in] ecProperty           The ecProperty to traverse
    //! @param[in] ecSchema             The schema the ecProperty belongs to
    //! @param[in] propertyProcessor    The function that is called on each traversed ecProperty 
    //! @returns ECObjectsStatus        If any call to propertyProcessor is not success returns immediately 
    //!                                 with that status. Otherwise returns success after traversing all
    //! @remarks It also calls the propertyProcessor for the ecProperty passed in
    ECOBJECTS_EXPORT static ECObjectsStatus TraverseProperty(ECPropertyP ecProperty, ECSchemaR ecSchema, ECPropertyProcessor propertyProcessor);

    //! Finds the root base class for the property
    //! @param[in] ecProperty The ecProperty 
    //! @param[in] ecSchema             The schema the ecProperty belongs to
    //! @returns ECClassCP    the root base class having the property
    //! @remarks If the property has multiple inheritance, it will return the first
    //!          from the list sorted by className(case insensitive) in ascending order
    ECOBJECTS_EXPORT static ECClassP FindRootBaseClass(ECPropertyP ecProperty, ECSchemaR ecSchema);

    //! checks if second parameter is base class of first parameter
    //! @param[in] ecClass   The ecClass that is supposed to be inherited from baseClass
    //! @param[in] baseClass The class that is supposed to be a base class
    //! @returns bool        True if baseClass is infact baseClass of ecClass
    ECOBJECTS_EXPORT static bool IsBaseClass(ECClassCP ecClass, ECClassCP baseClass);

    static Utf8String GetQualifiedClassName(Utf8StringCR schemaName, Utf8StringCR className) { return schemaName + ":" + className; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/