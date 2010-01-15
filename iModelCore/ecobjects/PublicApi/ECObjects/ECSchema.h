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

typedef std::list<ECPropertyP> PropertyList;
typedef stdext::hash_map<const wchar_t * , ECPropertyP, stdext::hash_compare<const wchar_t *, less_str>>   PropertyMap;
typedef stdext::hash_map<const wchar_t * , ECClassP, stdext::hash_compare<const wchar_t *, less_str>> ClassMap;


// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ValueKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
/*__PUBLISH_SECTION_START__*/
//! Represents the classification of the data type of an EC ECValue.  The classification is not the data type itself, but a category of type
//! such as struct, array or primitive.
enum ValueKind : unsigned short
    {
    VALUEKIND_Uninitialized                  = 0x00,
    VALUEKIND_Primitive                      = 0x01,
    VALUEKIND_Struct                         = 0x02,    
    VALUEKIND_Array                          = 0x04,    
    };

/*__PUBLISH_SECTION_END__*/
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ArrayKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
/*__PUBLISH_SECTION_START__*/
//! Represents the classification of the data type of an EC array element.  The classification is not the data type itself, but a category of type.
//! Currently an ECArray can only contain primitive or struct data types.
enum ArrayKind : unsigned short
    {
    ARRAYKIND_Primitive       = 0x01,
    ARRAYKIND_Struct          = 0x02
    };

/*__PUBLISH_SECTION_END__*/
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the PrimitiveType enum please be sure to note that the lower order byte must stay fixed as '1' and the upper order byte can be incremented.
// If you add any additional types you must update 
//    - ECXML_TYPENAME_X constants
//    - PrimitiveECProperty::_GetTypeName
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
// this is overkill for the native implementation.  Alternatively we could have a single ECProperty class that could act as primitive/struct/array or we can take the
// appoach I've implemented below.

// These BENTLEY_EXCLUDE_WINDOWS_HEADERS shenanigans are necessary to allow ECObjects headers to be included without sucking in conflicting windows headers
#ifdef BENTLEY_EXCLUDE_WINDOWS_HEADERS
    #define MSXML2_IXMLDOMNode      void *
    #define MSXML2_IXMLDOMNodePtr   void *
    #define MSXML2_IXMLDOMDocument2 void *
    #define MSXML2_IXMLDOMElement void *
#else
    #define MSXML2_IXMLDOMNode      MSXML2::IXMLDOMNode
    #define MSXML2_IXMLDOMNodePtr   MSXML2::IXMLDOMNodePtr
    #define MSXML2_IXMLDOMDocument2 MSXML2::IXMLDOMDocument2
    #define MSXML2_IXMLDOMElement   MSXML2::IXMLDOMElement
#endif

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ECProperty abstract
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;

private:
    std::wstring    m_name;        
    std::wstring    m_displayLabel;
    std::wstring    m_description;
    bool            m_readOnly;
    ECClassCR       m_class;
    ECPropertyP     m_baseProperty;    

protected:
    ECProperty (ECClassCR ecClass) : m_class(ecClass), m_readOnly(false), m_baseProperty(NULL) {};        
    ECObjectsStatus                     SetName (std::wstring const& name);    

    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode);
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode);
    SchemaSerializationStatus           _WriteXml(MSXML2_IXMLDOMElement& parentNode, const wchar_t *elementName);

    virtual bool                        _IsPrimitive () const { return false; }
    virtual bool                        _IsStruct () const { return false; }
    virtual bool                        _IsArray () const { return false; }
    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual std::wstring                _GetTypeName () const abstract;
    virtual ECObjectsStatus             _SetTypeName (std::wstring const& typeName) abstract;

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_READONLY_PROPERTY (ECClassCR,              Class);   
    // ECClass implementation will index property by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (std::wstring const&,    Name);        
    EXPORTED_READONLY_PROPERTY (bool,                   IsDisplayLabelDefined);    
    EXPORTED_READONLY_PROPERTY (bool,                   IsStruct);    
    EXPORTED_READONLY_PROPERTY (bool,                   IsArray);    
    EXPORTED_READONLY_PROPERTY (bool,                   IsPrimitive);    

    //! The ECXML typename for the property.  
    //! The TypeName for struct properties will be the ECClass name of the struct.  It may be qualified with a namespacePrefix if 
    //! the struct belongs to a schema that is referenced by the schema actually containing this property.
    //! The TypeName for array properties will be the type of the elements the array contains.
    //! This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    //! prefix relative to the containing schema.
    EXPORTED_PROPERTY  (std::wstring,           TypeName);        
    EXPORTED_PROPERTY  (std::wstring const&,    Description);
    EXPORTED_PROPERTY  (std::wstring const&,    DisplayLabel);    
    EXPORTED_PROPERTY  (bool,                   IsReadOnly);    

    ECOBJECTS_EXPORT ECObjectsStatus            SetIsReadOnly (const wchar_t * isReadOnly);

    // NEEDSWORK, don't necessarily like this pattern but it will suffice for now.  Necessary since you can't dynamic_cast when using the published headers.  How
    // do other similiar classes deal with this.
    ECOBJECTS_EXPORT PrimitiveECPropertyP       GetAsPrimitiveProperty () const;
    ECOBJECTS_EXPORT ArrayECPropertyP           GetAsArrayProperty () const;
    ECOBJECTS_EXPORT StructECPropertyP          GetAsStructProperty () const;
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct PrimitiveECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    PrimitiveType   m_primitiveType;   

    PrimitiveECProperty (ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), ECProperty(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual std::wstring                _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY  (PrimitiveType, Type);    
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct StructECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    ECClassCP   m_structType;   

    StructECProperty (ECClassCR ecClass) : m_structType(NULL), ECProperty(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual std::wstring                _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:    
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    EXPORTED_PROPERTY  (ECClassCR, Type);    
};

/*=================================================================================**//**
//
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ArrayECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;

private:
    UInt32              m_minOccurs;
    UInt32              m_maxOccurs;

    union
        {
        PrimitiveType   m_primitiveType;
        ECClassCP       m_structType;
        };

    ArrayKind           m_arrayKind;
      
    ArrayECProperty (ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), m_arrayKind (ARRAYKIND_Primitive),
        m_minOccurs (0), m_maxOccurs (UINT_MAX), ECProperty(ecClass) {};
    ECObjectsStatus SetMinOccurs (std::wstring const& minOccurs);          
    ECObjectsStatus SetMaxOccurs (std::wstring const& maxOccurs);          

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual std::wstring                _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (std::wstring const& typeName) override;

/*__PUBLISH_SECTION_START__*/
public:      
    EXPORTED_READONLY_PROPERTY  (ArrayKind, Kind);        

    EXPORTED_PROPERTY  (PrimitiveType,      PrimitiveElementType);        
    EXPORTED_PROPERTY  (ECClassCP,          StructElementType);    
    EXPORTED_PROPERTY  (UInt32,             MinOccurs);  
    EXPORTED_PROPERTY  (UInt32,             MaxOccurs);     
};

/*=================================================================================**//**
//
//! Container holding ECProperties that supports STL like iteration
//
+===============+===============+===============+===============+===============+======*/
struct      ECPropertyContainer /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECClass;
        
    PropertyMap const&     m_propertyMap;
    PropertyList const&    m_propertyList;
        
    ECPropertyContainer (PropertyMap const& propertyMap, PropertyList const& propertyList) 
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
        friend ECPropertyContainer;                   
        RefCountedPtr<IteratorState>   m_state;

/*__PUBLISH_SECTION_END__*/
        const_iterator (PropertyList::const_iterator listIterator) { m_state = IteratorState::Create (listIterator); };
/*__PUBLISH_SECTION_START__*/                        

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++();
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT ECPropertyP         operator* () const;
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

}; 

typedef std::vector<ECClassP> ECBaseClassesVector;
/*=================================================================================**//**
//
//! The in-memory representation of an ECClass as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ECClass /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/

friend struct ECSchema;
friend struct ECPropertyContainer;

private:
    std::wstring            m_name;
    std::wstring            m_displayLabel;
    std::wstring            m_description;
    bool                    m_isStruct;
    bool                    m_isCustomAttributeClass;
    bool                    m_isDomainClass;
    ECSchemaCR              m_schema;
    ECBaseClassesVector     m_baseClasses;
    ECPropertyContainer     m_propertyContainer;

    // Needswork:  Does STL provide any type of hypbrid list/dictionary collection?  We need fast lookup by name as well as retained order.  For now we will
    // just use a hash_map but we need to start retaining order once we implement serialization.
    PropertyMap             m_propertyMap;
    PropertyList            m_propertyList;    
    
    ECObjectsStatus                     AddProperty (ECPropertyP& pProperty);    

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECSchemaCR schema) : m_schema(schema), m_isStruct(false), m_isCustomAttributeClass(false), m_isDomainClass(true),
         m_propertyContainer(ECPropertyContainer(m_propertyMap, m_propertyList)){ };
    ~ECClass();    

    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus                     SetName (std::wstring const& name);    

    virtual SchemaDeserializationStatus ReadXmlAttributes (MSXML2_IXMLDOMNode& classNode);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @return   Status code
    virtual SchemaDeserializationStatus ReadXmlContents (MSXML2_IXMLDOMNode& classNode);    
    
    virtual SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode) const;
    SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode, const wchar_t * elementName) const;

/*__PUBLISH_SECTION_START__*/

public:    
    EXPORTED_READONLY_PROPERTY (ECSchemaCR,             Schema);                
    // schemas index class by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (std::wstring const&,    Name);        
    EXPORTED_READONLY_PROPERTY (bool,                   IsDisplayLabelDefined);    
    EXPORTED_READONLY_PROPERTY (ECPropertyContainerCR,  Properties); 
    EXPORTED_READONLY_PROPERTY (const ECBaseClassesVector&,     BaseClasses);   

    EXPORTED_PROPERTY  (std::wstring const&,            Description);
    EXPORTED_PROPERTY  (std::wstring const&,            DisplayLabel);
    EXPORTED_PROPERTY  (bool,                           IsStruct);    
    EXPORTED_PROPERTY  (bool,                           IsCustomAttributeClass);    
    EXPORTED_PROPERTY  (bool,                           IsDomainClass);    
    
    ECOBJECTS_EXPORT ECObjectsStatus SetIsStruct (const wchar_t * isStruct);          
    ECOBJECTS_EXPORT ECObjectsStatus SetIsCustomAttributeClass (const wchar_t * isCustomAttribute);
    ECOBJECTS_EXPORT ECObjectsStatus SetIsDomainClass (const wchar_t * isDomainClass);
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ECClassCR baseClass);
    ECOBJECTS_EXPORT bool            HasBaseClasses();

    //NEEDSWORK: Is method (test if is or derived from class X)

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @return   A pointer to an EC::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (std::wstring const& name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (std::wstring & prefix, std::wstring & className, std::wstring const& qualifiedClassName);
    ECOBJECTS_EXPORT static std::wstring GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass);
    
   
}; // ECClass

/*=================================================================================**//**
//
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ECRelationshipClass /*__PUBLISH_ABSTRACT__*/ : public ECClass
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;

// NEEDSWORK  missing full implementation
private:
    //std::wstring     m_strength;
    //std::wstring     m_strengthDirection;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECRelationshipClass (ECSchemaCR schema) : ECClass (schema) {};

protected:
    virtual SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode) const override;

/*__PUBLISH_SECTION_START__*/
public:
    //EXPORTED_PROPERTY (std::wstring const&, Strength);                
    //EXPORTED_PROPERTY (std::wstring const&, StrengthDirection);                

}; // ECRelationshipClass

typedef std::vector<ECSchemaP> ECSchemaReferenceVector;
typedef RefCountedPtr<ECSchema>                  ECSchemaPtr;

/*=================================================================================**//**
//
//! Supports STL like iterator of classes in a schema
//
+===============+===============+===============+===============+===============+======*/
struct      ECClassContainer /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECSchema;
    friend struct ECClass;
        
    ClassMap const&     m_classMap;
    
    ECClassContainer (ClassMap const& classMap) : m_classMap (classMap) {};

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
        friend          ECClassContainer;                   
        RefCountedPtr<IteratorState>   m_state;

/*__PUBLISH_SECTION_END__*/
        const_iterator (ClassMap::const_iterator mapIterator) { m_state = IteratorState::Create (mapIterator); };
/*__PUBLISH_SECTION_START__*/                        

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++();
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT ECClassP            operator* () const;
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

}; 

/*=================================================================================**//**
//
//! The in-memory representation of a schema as defined by ECSchemaXML
//
+===============+===============+===============+===============+===============+======*/
struct ECSchema /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
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
    ECClassContainer    m_classContainer;

    // maps class name -> class pointer    
    ClassMap m_classMap;
    
    ECSchemaReferenceVector m_refSchemaList;
    
    std::set<const wchar_t *> m_alreadySerializedClasses;
    stdext::hash_map<ECSchemaP, const std::wstring *> m_referencedSchemaNamespaceMap;

    // Hide these as part of the RefCounted pattern    
    ECSchema () : m_versionMajor (DEFAULT_VERSION_MAJOR), m_versionMinor (DEFAULT_VERSION_MINOR), m_classContainer(ECClassContainer(m_classMap)) {};
    ~ECSchema();    

    static SchemaDeserializationStatus  ReadXml (ECSchemaPtr& schemaOut, MSXML2_IXMLDOMDocument2& pXmlDoc);
    SchemaSerializationStatus           WriteXml (MSXML2_IXMLDOMDocument2* pXmlDoc);

    ECObjectsStatus                     AddClass (ECClassP& pClass);
    ECObjectsStatus                     SetVersionFromString (std::wstring const& versionString);

    typedef std::vector<std::pair<ECClassP, MSXML2_IXMLDOMNodePtr>>  ClassDeserializationVector;
    SchemaDeserializationStatus         ReadClassStubsFromXml(MSXML2_IXMLDOMNode& schemaNodePtr,ClassDeserializationVector& classes);
    SchemaDeserializationStatus         ReadClassContentsFromXml(ClassDeserializationVector&  classes);
    
    SchemaSerializationStatus           WriteSchemaReferences(MSXML2_IXMLDOMElement& parentNode);
    SchemaSerializationStatus           WriteClass(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass);
    SchemaSerializationStatus           WritePropertyDependencies(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass);

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY (std::wstring const&, Name);    
    EXPORTED_PROPERTY (std::wstring const&, NamespacePrefix);
    EXPORTED_PROPERTY (std::wstring const&, Description);
    EXPORTED_PROPERTY (std::wstring const&, DisplayLabel);
    EXPORTED_PROPERTY (UInt32,              VersionMajor);
    EXPORTED_PROPERTY (UInt32,              VersionMinor);

    EXPORTED_READONLY_PROPERTY (ECClassContainerCR, Classes);
    EXPORTED_READONLY_PROPERTY (bool,               IsDisplayLabelDefined);

    ECOBJECTS_EXPORT ECObjectsStatus    CreateClass (ECClassP& ecClass, std::wstring const& name);
    ECOBJECTS_EXPORT ECObjectsStatus    CreateRelationshipClass (ECRelationshipClassP& relationshipClass, std::wstring const& name);

    //! Get a schema by namespace prefix within the context of this schema and its referenced schemas.
    //! It is important to note that this method does not return a RefCountedPtr.  If you want to hold a pointer to the returned schema that will exceed the
    //! lifetime of the RefCountedPtr on which you invoked this method then it is critical you assign the return value to a ECSchemaPtr.   
    //! @param[in]  namespacePrefix     The prefix of the schema to lookup in the context of this schema and it's references.
    //!                                 Passing an empty namespacePrefix will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an EC::ECSchema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaP          GetSchemaByNamespacePrefixP(std::wstring const& namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup a namespace prefix in the context of this schema and its references.    
    //! @return   The namespace prefix if schema is a referenced schema; empty string if the schema is the current schema; otherwise, NULL
    ECOBJECTS_EXPORT std::wstring const* ResolveNamespacePrefix(ECSchemaCR schema) const;

    //! Get a class by name within the context of this schema.
    //! It is important to note that this method does not return a RefCountedPtr.  You must hold onto to the reference counted ECSchemaPtr on which you invoke
    //! this method for the lifetime that you which to keep the returned class alive.  If you do not, there is a chance that the returned class pointer will go
    //! stale and result in a memory access violation when used.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (std::wstring const& name) const;

    //! Gets the other schemas that are used by classes within this schema.
    //! Referenced schemas are the schemas that contain definitions of base classes,
    //! embedded structures, and custom attributes of classes within this schema.
    ECOBJECTS_EXPORT const ECSchemaReferenceVector& GetReferencedSchemas() const;
    
    ECOBJECTS_EXPORT ECObjectsStatus AddReferencedSchema(ECSchemaCR refSchema);
    
    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaPtr& schemaOut, std::wstring const& schemaName);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, std::wstring const& versionString);
    

    //! Deserializes an ECXML schema from a file.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to deserialize.
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromFile (ECSchemaPtr& schemaOut, const wchar_t * ecSchemaXmlFile);

    //! Deserializes an ECXML schema from a string.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to deserialize
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromString (ECSchemaPtr& schemaOut, const wchar_t * ecSchemaXml);

/*
    //! Deserializes an ECXML schema from an IStream.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to deserialize
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    //ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromStream (ECSchemaPtr& schemaOut, IStream * ecSchemaXmlStream);
*/

    //! Serializes an ECXML schema to a string
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus          WriteXmlToString (const wchar_t * & ecSchemaXml);
    
    //! Serializes an ECXML schema to a file
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus          WriteXmlToFile (const wchar_t * ecSchemaXmlFile);
    
    /*
    //! Serializes an ECXML schema to an IStream
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    //ECOBJECTS_EXPORT SchemaSerializationStatus WriteXmlToStream (IStream * ecSchemaXmlStream);
    */
    
}; // ECSchema


END_BENTLEY_EC_NAMESPACE

#undef MSXML2_IXMLDOMNode
#undef MSXML2_IXMLDOMNodePtr
#undef MSXML2_IXMLDOMDocument2
#undef MSXML2_IXMLDOMElementPtr
