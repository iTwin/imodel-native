/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>
#include <Bentley\RefCounted.h>

#define DEFAULT_VERSION_MAJOR   1
#define DEFAULT_VERSION_MINOR   0

BEGIN_BENTLEY_EC_NAMESPACE

/*__PUBLISH_SECTION_END__*/

/*=================================================================================**//**
* Comparison function that is used within various schema related data structures
* for string comparison in STL collections.
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct less_str
{
bool operator()(const wchar_t * s1, const wchar_t * s2) const
    {
    if (wcscmp(s1, s2) < 0)
        return true;

    return false;
    }
};

typedef std::list<PropertyP> PropertyList;
typedef stdext::hash_map<const wchar_t * , PropertyP, stdext::hash_compare<const wchar_t *, less_str>>   PropertyMap;
typedef stdext::hash_map<const wchar_t * , ClassP, stdext::hash_compare<const wchar_t *, less_str>> ClassMap;


// ValueClassification, ArrayElementClassification & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueClassification or ArrayElementClassification is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ValueClassification enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
/*__PUBLISH_SECTION_START__*/
//! Represents the classification of the data type of an EC Value.  The classification is not the data type itself, but a category of type
//! such as struct, array or primitive.
enum ValueClassification : unsigned short
    {
    VALUECLASSIFICATION_Uninitialized                  = 0x00,
    VALUECLASSIFICATION_Primitive                      = 0x01,
    VALUECLASSIFICATION_Struct                         = 0x02,    
    VALUECLASSIFICATION_Array                          = 0x04,    
    };

/*__PUBLISH_SECTION_END__*/
// ValueClassification, ArrayElementClassification & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueClassification or ArrayElementClassification is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ArrayElementClassification enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
/*__PUBLISH_SECTION_START__*/
//! Represents the classification of the data type of an EC array element.  The classification is not the data type itself, but a category of type.
//! Currently an ECArray can only contain primitive or struct data types.
enum ArrayElementClassification : unsigned short
    {
    ELEMENTCLASSIFICATION_Primitive       = 0x01,
    ELEMENTCLASSIFICATION_Struct          = 0x02
    };

/*__PUBLISH_SECTION_END__*/
// ValueClassification, ArrayElementClassification & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueClassification or ArrayElementClassification is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the PrimitiveType enum please be sure to note that the lower order byte must stay fixed as '1' and the upper order byte can be incremented.
// If you add any additional types you must update 
//    - ECXML_TYPENAME_X constants
//    - PrimitiveProperty::_GetTypeName
// NEEDSWORK types: common geometry, installed primitives
/*__PUBLISH_SECTION_START__*/
//! Enumeration of primitive datatypes supported by native "ECObjects" implementation.
//! These should correspond to all of the datatypes supported in .NET ECObjects
enum PrimitiveType : unsigned short
    {
    PRIMITIVETYPE_Binary                    = 0x101,
    PRIMITIVETYPE_Boolean                   = 0x201,
    PRIMITIVETYPE_DateTime                  = 0x301,
    PRIMITIVETYPE_Double                    = 0x401,
    PRIMITIVETYPE_Integer                   = 0x501,
    PRIMITIVETYPE_Long                      = 0x601,
    PRIMITIVETYPE_Point2D                   = 0x701,
    PRIMITIVETYPE_Point3D                   = 0x801,
    PRIMITIVETYPE_String                    = 0x901
    };

// NEEDSWORK - unsure what the best way is to model ECProperty.  Managed implementation has a single ECProperty and introduces an ECType concept.  My gut is that
// this is overkill for the native implementation.  Alternatively we could have a single Property class that could act as primitive/struct/array or we can take the
// appoach I've implemented below.

// These BENTLEY_EXCLUDE_WINDOWS_HEADERS shenanigans are necessary to allow ECObjects headers to be included without sucking in conflicting windows headers
#ifdef BENTLEY_EXCLUDE_WINDOWS_HEADERS
    #define MSXML2_IXMLDOMNode      void *
    #define MSXML2_IXMLDOMNodePtr   void *
    #define MSXML2_IXMLDOMDocument2 void *
    #define MSXML2_IXMLDOMElementPtr void *
#else
    #define MSXML2_IXMLDOMNode      MSXML2::IXMLDOMNode
    #define MSXML2_IXMLDOMNodePtr   MSXML2::IXMLDOMNodePtr
    #define MSXML2_IXMLDOMDocument2 MSXML2::IXMLDOMDocument2
    #define MSXML2_IXMLDOMElementPtr MSXML2::IXMLDOMElementPtr
#endif

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct Property abstract
{
/*__PUBLISH_SECTION_END__*/
friend struct Class;

private:
    std::wstring    m_name;        
    std::wstring    m_displayLabel;
    std::wstring    m_description;
    bool            m_readOnly;
    ClassCR         m_class;
    PropertyP       m_baseProperty;    

protected:
    Property (ClassCR ecClass) : m_class(ecClass), m_readOnly(false), m_baseProperty(NULL) {};        
    ECObjectsStatus SetName (std::wstring const& name);    

    virtual SchemaDeserializationStatus _ReadXML (MSXML2_IXMLDOMNode& propertyNode);
    virtual SchemaSerializationStatus _Serialize(MSXML2_IXMLDOMElementPtr parentNode);

    virtual bool _IsPrimitive () const { return false; }
    virtual bool _IsStruct () const { return false; }
    virtual bool _IsArray () const { return false; }
    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual std::wstring _GetTypeName () const abstract;
    virtual ECObjectsStatus _SetTypeName (std::wstring const& typeName) abstract;

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_READONLY_PROPERTY (ClassCR, Class);   
    // Class implementation will index property by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (std::wstring const&, Name);        
    EXPORTED_READONLY_PROPERTY (bool, IsDisplayLabelDefined);    
    EXPORTED_READONLY_PROPERTY (bool, IsStruct);    
    EXPORTED_READONLY_PROPERTY (bool, IsArray);    
    EXPORTED_READONLY_PROPERTY (bool, IsPrimitive);    

    //! The ECXML typename for the property.  
    //! The TypeName for struct properties will be the ECClass name of the struct.  It may be qualified with a namespacePrefix if 
    //! the struct belongs to a schema that is referenced by the schema actually containing this property.
    //! The TypeName for array properties will be the type of the elements the array contains.
    //! This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    //! prefix relative to the containing schema.
    EXPORTED_PROPERTY  (std::wstring, TypeName);        
    EXPORTED_PROPERTY  (std::wstring const&, Description);
    EXPORTED_PROPERTY  (std::wstring const&, DisplayLabel);    
    EXPORTED_PROPERTY  (bool, IsReadOnly);    

    ECOBJECTS_EXPORT ECObjectsStatus SetIsReadOnly (const wchar_t * isReadOnly);

    // NEEDSWORK, don't necessarily like this pattern but it will suffice for now.  Necessary since you can't dynamic_cast when using the published headers.  How
    // do other similiar classes deal with this.
    ECOBJECTS_EXPORT PrimitivePropertyP GetAsPrimitiveProperty () const;
    ECOBJECTS_EXPORT ArrayPropertyP GetAsArrayProperty () const;
    ECOBJECTS_EXPORT StructPropertyP GetAsStructProperty () const;
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct PrimitiveProperty /*__PUBLISH_ABSTRACT__*/ : public Property
{
/*__PUBLISH_SECTION_END__*/
friend struct Class;
private:
    PrimitiveType   m_primitiveType;   

    PrimitiveProperty (ClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), Property(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXML (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus _Serialize(MSXML2_IXMLDOMElementPtr parentNode) override;
    virtual bool _IsPrimitive () const override { return true;}
    virtual std::wstring _GetTypeName () const override;
    virtual ECObjectsStatus _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY  (PrimitiveType, Type);    
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct StructProperty /*__PUBLISH_ABSTRACT__*/ : public Property
{
/*__PUBLISH_SECTION_END__*/
friend struct Class;
private:
    ClassCP   m_structType;   

    StructProperty (ClassCR ecClass) : m_structType(NULL), Property(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXML (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus _Serialize(MSXML2_IXMLDOMElementPtr parentNode) override;
    virtual bool _IsStruct () const override { return true;}
    virtual std::wstring _GetTypeName () const override;
    virtual ECObjectsStatus _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:    
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    EXPORTED_PROPERTY  (ClassCR, Type);    
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ArrayProperty /*__PUBLISH_ABSTRACT__*/ : public Property
{
/*__PUBLISH_SECTION_END__*/
friend struct Class;

private:
    UInt32  m_minOccurs;
    UInt32  m_maxOccurs;

    union
        {
        PrimitiveType   m_primitiveType;
        ClassCP         m_structType;
        };

    ArrayElementClassification m_elementClassification;
      
    ArrayProperty (ClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), m_elementClassification (ELEMENTCLASSIFICATION_Primitive),
        m_minOccurs (0), m_maxOccurs (UINT_MAX), Property(ecClass) {};
    ECObjectsStatus SetMinOccurs (std::wstring const& minOccurs);          
    ECObjectsStatus SetMaxOccurs (std::wstring const& maxOccurs);          

protected:
    virtual SchemaDeserializationStatus _ReadXML (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus _Serialize(MSXML2_IXMLDOMElementPtr parentNode) override;
    virtual bool _IsArray () const override { return true;}
    virtual std::wstring _GetTypeName () const override;
    virtual ECObjectsStatus _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:      
    EXPORTED_READONLY_PROPERTY  (ArrayElementClassification, ElementClassification);        

    EXPORTED_PROPERTY  (PrimitiveType, PrimitiveElementType);        
    EXPORTED_PROPERTY  (ClassCP, StructElementType);    
    EXPORTED_PROPERTY  (UInt32, MinOccurs);  
    EXPORTED_PROPERTY  (UInt32, MaxOccurs);     
};

/*=================================================================================**//**
//
//! Container holding ECProperties that supports STL like iteration
//
+===============+===============+===============+===============+===============+======*/
struct      PropertyContainer /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct Class;
        
    PropertyMap const&     m_propertyMap;
    PropertyList const&     m_propertyList;
        
    PropertyContainer (PropertyMap const& propertyMap, PropertyList const& propertyList) 
        : m_propertyMap (propertyMap), m_propertyList(propertyList) {};


/*__PUBLISH_SECTION_START__*/

public:    
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorState /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
        {        
        friend struct const_iterator;
/*__PUBLISH_SECTION_END__*/
        public:            
            PropertyList::const_iterator     m_listIterator;                   

            IteratorState (PropertyList::const_iterator listIterator) { m_listIterator = listIterator; };
            static RefCountedPtr<IteratorState> Create (PropertyList::const_iterator listIterator) { return new IteratorState(listIterator); };
/*__PUBLISH_SECTION_START__*/                        
        };

    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct const_iterator
    {    
    private:                
        friend          PropertyContainer;                   
        RefCountedPtr<IteratorState>   m_state;

/*__PUBLISH_SECTION_END__*/
        const_iterator (PropertyList::const_iterator listIterator) { m_state = IteratorState::Create (listIterator); };
/*__PUBLISH_SECTION_START__*/                        

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++();
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT PropertyP           operator* () const;
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

}; 

/*=================================================================================**//**
//
//! The in-memory representation of an ECClass as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct Class /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/

friend struct Schema;
friend struct PropertyContainer;

private:
    std::wstring    m_name;
    std::wstring    m_displayLabel;
    std::wstring    m_description;
    bool            m_isStruct;
    bool            m_isCustomAttributeClass;
    bool            m_isDomainClass;
    SchemaCR        m_schema;
    std::vector<ClassP>    m_baseClasses;
    PropertyContainer   m_propertyContainer;

    // Needswork:  Does STL provide any type of hypbrid list/dictionary collection?  We need fast lookup by name as well as retained order.  For now we will
    // just use a hash_map but we need to start retaining order once we implement serialization.
    PropertyMap m_propertyMap;
    PropertyList m_propertyList;    
    
    ECObjectsStatus AddProperty (PropertyP& pProperty);    

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    Class (SchemaCR schema) : m_schema(schema), m_isStruct(false), m_isCustomAttributeClass(false), m_isDomainClass(true), m_propertyContainer(PropertyContainer(m_propertyMap, m_propertyList)){ };
    ~Class();    

    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus SetName (std::wstring const& name);    

    virtual SchemaDeserializationStatus ReadXMLAttributes (MSXML2_IXMLDOMNode& classNode);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @return   Status code
    virtual SchemaDeserializationStatus ReadXMLContents (MSXML2_IXMLDOMNode& classNode);    
    
    virtual SchemaSerializationStatus Serialize(MSXML2_IXMLDOMElementPtr parentNode);

/*__PUBLISH_SECTION_START__*/

public:    
    EXPORTED_READONLY_PROPERTY (SchemaCR, Schema);                
    // schemas index class by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (std::wstring const&, Name);        
    EXPORTED_READONLY_PROPERTY (bool, IsDisplayLabelDefined);    
    EXPORTED_READONLY_PROPERTY (PropertyContainerCR, Properties);    

    EXPORTED_PROPERTY  (std::wstring const&, Description);
    EXPORTED_PROPERTY  (std::wstring const&, DisplayLabel);
    EXPORTED_PROPERTY  (bool, IsStruct);    
    EXPORTED_PROPERTY  (bool, IsCustomAttributeClass);    
    EXPORTED_PROPERTY  (bool, IsDomainClass);    
    
    ECOBJECTS_EXPORT ECObjectsStatus SetIsStruct (const wchar_t * isStruct);          
    ECOBJECTS_EXPORT ECObjectsStatus SetIsCustomAttributeClass (const wchar_t * isCustomAttribute);
    ECOBJECTS_EXPORT ECObjectsStatus SetIsDomainClass (const wchar_t * isDomainClass);
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ClassCR baseClass);
    ECOBJECTS_EXPORT bool HasBaseClasses();
    //NEEDSWORK: Method to iterate/get base classes
    //NEEDSWORK: Is method (test if is or derived from class X)

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the Class containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @return   A pointer to an EC::Property if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT PropertyP GetPropertyP (std::wstring const& name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (std::wstring & prefix, std::wstring & className, std::wstring const& qualifiedClassName);

}; // Class

/*=================================================================================**//**
//
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct RelationshipClass /*__PUBLISH_ABSTRACT__*/ : public Class
{
/*__PUBLISH_SECTION_END__*/
friend struct Schema;

// NEEDSWORK  missing full implementation
private:
    //std::wstring     m_strength;
    //std::wstring     m_strengthDirection;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    RelationshipClass (SchemaCR schema) : Class (schema) {};

/*__PUBLISH_SECTION_START__*/
public:
    //EXPORTED_PROPERTY (std::wstring const&, Strength);                
    //EXPORTED_PROPERTY (std::wstring const&, StrengthDirection);                

}; // RelationshipClass

/*=================================================================================**//**
//
//! Supports STL like iterator of classes in a schema
//
+===============+===============+===============+===============+===============+======*/
struct      ClassContainer /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct Schema;
        
    ClassMap const&     m_classMap;
    
    ClassContainer (ClassMap const& classMap) : m_classMap (classMap) {};

/*__PUBLISH_SECTION_START__*/

public:    
    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct IteratorState /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
        {        
        friend struct const_iterator;
/*__PUBLISH_SECTION_END__*/
        public:            
            ClassMap::const_iterator     m_mapIterator;                   

            IteratorState (ClassMap::const_iterator mapIterator) { m_mapIterator = mapIterator; };
            static RefCountedPtr<IteratorState> Create (ClassMap::const_iterator mapIterator) { return new IteratorState(mapIterator); };
/*__PUBLISH_SECTION_START__*/                        
        };

    /*=================================================================================**//**
    * @bsistruct
    +===============+===============+===============+===============+===============+======*/
    struct const_iterator
    {    
    private:                
        friend          ClassContainer;                   
        RefCountedPtr<IteratorState>   m_state;

/*__PUBLISH_SECTION_END__*/
        const_iterator (ClassMap::const_iterator mapIterator) { m_state = IteratorState::Create (mapIterator); };
/*__PUBLISH_SECTION_START__*/                        

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++();
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT ClassP             operator* () const;
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

}; 

typedef RefCountedPtr<Schema>                  SchemaPtr;

/*=================================================================================**//**
//
//! The in-memory representation of a schema as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct Schema /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/

// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    std::wstring        m_name;
    std::wstring        m_namespacePrefix;
    std::wstring        m_displayLabel;
    std::wstring        m_description;
    UInt32              m_versionMajor;
    UInt32              m_versionMinor;    
    ClassContainer      m_classContainer;

    // maps class name -> class pointer    
    ClassMap m_classMap;

    // Hide these as part of the RefCounted pattern    
    Schema () : m_versionMajor (DEFAULT_VERSION_MAJOR), m_versionMinor (DEFAULT_VERSION_MINOR), m_classContainer(ClassContainer(m_classMap)) {};
    ~Schema();    

    static SchemaDeserializationStatus ReadXML (SchemaPtr& schemaOut, MSXML2_IXMLDOMDocument2& pXmlDoc);
    SchemaSerializationStatus WriteXml (MSXML2_IXMLDOMDocument2* pXmlDoc);

    ECObjectsStatus AddClass (ClassP& pClass);
    ECObjectsStatus SetVersionFromString (std::wstring const& versionString);

    typedef std::vector<std::pair<ClassP, MSXML2_IXMLDOMNodePtr>>  ClassDeserializationVector;
    SchemaDeserializationStatus ReadClassStubsFromXML(MSXML2_IXMLDOMNode& schemaNodePtr,ClassDeserializationVector& classes);
    SchemaDeserializationStatus ReadClassContentsFromXML(ClassDeserializationVector&  classes);

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY (std::wstring const&, Name);    
    EXPORTED_PROPERTY (std::wstring const&, NamespacePrefix);
    EXPORTED_PROPERTY (std::wstring const&, Description);
    EXPORTED_PROPERTY (std::wstring const&, DisplayLabel);
    EXPORTED_PROPERTY (UInt32, VersionMajor);
    EXPORTED_PROPERTY (UInt32, VersionMinor);

    EXPORTED_READONLY_PROPERTY (ClassContainerCR, Classes);
    EXPORTED_READONLY_PROPERTY (bool, IsDisplayLabelDefined);

    ECOBJECTS_EXPORT ECObjectsStatus CreateClass (ClassP& ecClass, std::wstring const& name);
    ECOBJECTS_EXPORT ECObjectsStatus CreateRelationshipClass (RelationshipClassP& relationshipClass, std::wstring const& name);

    //! Get a schema by namespace prefix within the context of this schema and it's referenced schemas.
    //! It is important to note that this method does not return a RefCountedPtr.  If you want to hold a pointer to the returned schema that will exceed the
    //! lifetime of the RefCountedPtr on which you invoked this method then it is critical you assign the return value to a SchemaPtr.   
    //! @param[in]  namespacePrefix     The prefix of the schema to lookup in the context of this schema and it's references.
    //!                                 Passing an empty namespacePrefix will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an EC::Schema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT SchemaP GetSchemaByNamespacePrefixP(std::wstring const& namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and it's references.
    //! @param[in]  schema     The schemato lookup a namespace prefix in the context of this schema and it's references.    
    //! @return   The namespace prefix if schema is a referenced schema; empty string if the schema is the current schema; otherwise, NULL
    ECOBJECTS_EXPORT std::wstring const* ResolveNamespacePrefix(SchemaCR schema) const;

    //! Get a class by name within the context of this schema.
    //! It is important to note that this method does not return a RefCountedPtr.  You must hold onto to the reference counted SchemaPtr on which you invoke
    //! this method for the lifetime that you which to keep the returned class alive.  If you do not, there is a chance that the returned class pointer will go
    //! stale and result in a memory access violation when used.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::Class if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ClassP GetClassP (std::wstring const& name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (SchemaPtr& schemaOut, std::wstring const& schemaName);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, std::wstring const& versionString);
    

    //! Deserializes an ECXML schema from a file.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to deserialize.
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXMLFromFile (SchemaPtr& schemaOut, const wchar_t * ecSchemaXmlFile);

    //! Deserializes an ECXML schema from a string.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to deserialize
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXMLFromString (SchemaPtr& schemaOut, const wchar_t * ecSchemaXml);

    //! Deserializes an ECXML schema from an IStream.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to deserialize
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    //ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXMLFromStream (SchemaPtr& schemaOut, IStream * ecSchemaXmlStream);


    //! Serializes an ECXML schema to a string
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus WriteXmlToString (const wchar_t * & ecSchemaXml);
    
    //! Serializes an ECXML schema to a file
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus WriteXmlToFile (const wchar_t * ecSchemaXmlFile);
    
    //! Serializes an ECXML schema to an IStream
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    //ECOBJECTS_EXPORT SchemaSerializationStatus WriteXmlToStream (IStream * ecSchemaXmlStream);
    
}; // Schema


END_BENTLEY_EC_NAMESPACE

#undef MSXML2_IXMLDOMNode
#undef MSXML2_IXMLDOMNodePtr
#undef MSXML2_IXMLDOMDocument2
#undef MSXML2_IXMLDOMElementPtr
