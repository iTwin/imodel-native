/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>
#include <ECObjects\ECEnabler.h>
#include <Bentley\RefCounted.h>
#include <Bentley\bvector.h>
#include <Bentley\bmap.h>
#include <Bentley\bset.h>

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
bool operator()(WCharCP s1, WCharCP s2) const
    {
    if (wcscmp(s1, s2) < 0)
        return true;

    return false;
    }
};

struct NameValidator abstract
{
public:
    static bool Validate(const WString& name);
};
    
typedef bvector<ECPropertyP> PropertyList;
typedef bmap<WCharCP , ECPropertyP, less_str> PropertyMap;
typedef bmap<WCharCP , ECClassP,    less_str> ClassMap;
typedef bmap<WCharCP , ECSchemaP,   less_str> SchemaMap;

/*__PUBLISH_SECTION_START__*/

//=======================================================================================    
//! Used to represent the type of an ECProperty
//=======================================================================================
struct ECTypeDescriptor
{
private:
    ValueKind       m_typeKind;

    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_primitiveType;
        };      

public:
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveTypeDescriptor (PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructArrayTypeDescriptor ();
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructTypeDescriptor ();

    ECTypeDescriptor (PrimitiveType primitiveType) : m_typeKind (VALUEKIND_Primitive), m_primitiveType (primitiveType) { };

/*__PUBLISH_SECTION_END__*/
    ECTypeDescriptor () : m_typeKind ((ValueKind) 0), m_primitiveType ((PrimitiveType) 0) { };
    ECTypeDescriptor (ValueKind valueKind, short valueKindQualifier) : m_typeKind (valueKind), m_primitiveType ((PrimitiveType)valueKindQualifier) { };
/*__PUBLISH_SECTION_START__*/    

    inline ValueKind            GetTypeKind() const         { return m_typeKind; }
    inline ArrayKind            GetArrayKind() const        { return (ArrayKind)(m_arrayKind & 0xFF); }    
    inline bool                 IsPrimitive() const         { return (GetTypeKind() == VALUEKIND_Primitive ); }
    inline bool                 IsStruct() const            { return (GetTypeKind() == VALUEKIND_Struct ); }
    inline bool                 IsArray() const             { return (GetTypeKind() == VALUEKIND_Array ); }
    inline bool                 IsPrimitiveArray() const    { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Primitive); }
    inline bool                 IsStructArray() const       { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Struct); }
    inline PrimitiveType        GetPrimitiveType() const    { return m_primitiveType; }
/*__PUBLISH_SECTION_END__*/
    inline short                GetTypeKindQualifier() const   { return m_primitiveType; }
/*__PUBLISH_SECTION_START__*/        
};    

// NEEDSWORK - unsure what the best way is to model ECProperty.  Managed implementation has a single ECProperty and introduces an ECType concept.  My gut is that
// this is overkill for the native implementation.  Alternatively we could have a single ECProperty class that could act as primitive/struct/array or we can take the
// appoach I've implemented below.

/*__PUBLISH_SECTION_END__*/
// These BENTLEY_EXCLUDE_WINDOWS_HEADERS shenanigans are necessary to allow ECObjects headers to be included without sucking in conflicting windows headers
#ifdef BENTLEY_EXCLUDE_WINDOWS_HEADERS
/*__PUBLISH_SECTION_START__*/        
    #define MSXML2_IXMLDOMNode      void *
    #define MSXML2_IXMLDOMNodePtr   void *
    #define MSXML2_IXMLDOMDocument2 void *
    #define MSXML2_IXMLDOMElement   void *
/*__PUBLISH_SECTION_END__*/
#else
    #define MSXML2_IXMLDOMNode      MSXML2::IXMLDOMNode
    #define MSXML2_IXMLDOMNodePtr   MSXML2::IXMLDOMNodePtr
    #define MSXML2_IXMLDOMDocument2 MSXML2::IXMLDOMDocument2
    #define MSXML2_IXMLDOMElement   MSXML2::IXMLDOMElement
#endif
/*__PUBLISH_SECTION_START__*/        

typedef bvector<IECInstancePtr> ECCustomAttributeCollection;
struct ECCustomAttributeInstanceIterable;

//=======================================================================================
//
//=======================================================================================
struct IECCustomAttributeContainer /*__PUBLISH_ABSTRACT__*/  
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECCustomAttributeInstanceIterable;
    ECCustomAttributeCollection         m_customAttributes;
    SchemaSerializationStatus           AddCustomAttributeProperties(MSXML2_IXMLDOMNode& oldNode, MSXML2_IXMLDOMNode& newNode) const;

protected:
    InstanceDeserializationStatus       ReadCustomAttributes(MSXML2_IXMLDOMNode& containerNode, ECSchemaCR schema, IStandaloneEnablerLocatorR standaloneEnablerLocator);
    SchemaSerializationStatus           WriteCustomAttributes(MSXML2_IXMLDOMNode& parentNode) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
/*__PUBLISH_SECTION_START__*/
protected:
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP                  _GetContainerSchema() const {return NULL;};

public:
    ECOBJECTS_EXPORT virtual ~IECCustomAttributeContainer();

    //! Returns true if the conainer has a custom attribute of a class of the specified name
    ECOBJECTS_EXPORT bool               IsDefined(WStringCR className) ;
    //! Returns true if the conainer has a custom attribute of a class of the specified class definition
    ECOBJECTS_EXPORT bool               IsDefined(ECClassCR classDefinition) ;

    //! Retrieves the custom attribute matching the class name.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(WStringCR className) const;
    //! Retrieves the custom attribute matching the class definition.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(ECClassCR classDefinition) const;
    //! Retrieves all custom attributes from the container
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers 
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetCustomAttributes(bool includeBase) const; 

    ECOBJECTS_EXPORT ECObjectsStatus    SetCustomAttribute(IECInstanceR customAttributeInstance);
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(WStringCR className);
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(ECClassCR classDefinition);
};

//=======================================================================================
//
//=======================================================================================
struct ECCustomAttributeInstanceIterable
{
private:
    friend struct IECCustomAttributeContainer;

    IECCustomAttributeContainerCR m_container;
    bool                        m_includeBaseContainers;
 /*__PUBLISH_SECTION_END__*/
   ECCustomAttributeInstanceIterable( IECCustomAttributeContainerCR container, bool includeBase) : m_container(container), m_includeBaseContainers(includeBase) {};
/*__PUBLISH_SECTION_START__*/
public:
    struct IteratorState /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
        {
        friend struct const_iterator;
/*__PUBLISH_SECTION_END__*/

        ECCustomAttributeCollection* m_customAttributes;
        ECCustomAttributeCollection::const_iterator m_customAttributesIterator;

        IteratorState(IECCustomAttributeContainerCR container, bool includeBase);
        ~IteratorState();

        static RefCountedPtr<IteratorState> Create (IECCustomAttributeContainerCR container, bool includeBase)
            { return new IteratorState(container, includeBase); } ;
/*__PUBLISH_SECTION_START__*/                        
        };

    struct const_iterator
        {
        private:
            friend ECCustomAttributeInstanceIterable;
            RefCountedPtr<IteratorState> m_state;
            bool m_isEnd;
/*__PUBLISH_SECTION_END__*/
            const_iterator (IECCustomAttributeContainerCR container, bool includeBase);
            const_iterator () : m_isEnd(true) {};
/*__PUBLISH_SECTION_START__*/                        
           
        public:
            ECOBJECTS_EXPORT const_iterator&     operator++();
            ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
            ECOBJECTS_EXPORT IECInstancePtr      operator* () const;
        };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;    
};

struct PrimitiveECProperty;

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct ECProperty abstract : public IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;

private:
    WString        m_name;        
    WString        m_displayLabel;
    WString        m_description;
    bool            m_readOnly;
    ECClassCR       m_class;
    ECPropertyCP    m_baseProperty;    
    bool            m_hideFromLeakDetection;

protected:
    ECProperty (ECClassCR ecClass, bool hideFromLeakDetection);
    virtual ~ECProperty();

    ECObjectsStatus                     SetName (WStringCR name);

    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator);
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode);
    SchemaSerializationStatus           _WriteXml(MSXML2_IXMLDOMElement& parentNode, WCharCP elementName);

    virtual bool                        _IsPrimitive () const { return false; }
    virtual bool                        _IsStruct () const { return false; }
    virtual bool                        _IsArray () const { return false; }
    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual WString                    _GetTypeName () const abstract;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) abstract;

    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const abstract;

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP                  _GetContainerSchema() const override;

    virtual PrimitiveECProperty*        _GetAsPrimitiveECProperty() {return NULL;}
    
public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_READONLY_PROPERTY (ECClassCR,              Class);   
    // ECClass implementation will index property by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (WStringCR,        Name);        
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
    EXPORTED_PROPERTY  (WString,               TypeName);        
    EXPORTED_PROPERTY  (WStringCR,        Description);
    EXPORTED_PROPERTY  (WStringCR,        DisplayLabel);    
    EXPORTED_PROPERTY  (bool,                   IsReadOnly);
    EXPORTED_PROPERTY  (ECPropertyCP,           BaseProperty);    

    ECOBJECTS_EXPORT ECObjectsStatus            SetIsReadOnly (WCharCP isReadOnly);

    // NEEDSWORK, don't necessarily like this pattern but it will suffice for now.  Necessary since you can't dynamic_cast when using the published headers.  How
    // do other similiar classes deal with this.
    ECOBJECTS_EXPORT PrimitiveECPropertyP       GetAsPrimitiveProperty () const;    // FUSION_WIP: this removes const!
    ECOBJECTS_EXPORT ArrayECPropertyP           GetAsArrayProperty () const;        //  "
    ECOBJECTS_EXPORT StructECPropertyP          GetAsStructProperty () const;       //  "
};


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct PrimitiveECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    PrimitiveType   m_primitiveType;   

    PrimitiveECProperty (ECClassCR ecClass, bool hideFromLeakDetection) : m_primitiveType(PRIMITIVETYPE_String), ECProperty(ecClass, hideFromLeakDetection) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual WString                    _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual PrimitiveECProperty*        _GetAsPrimitiveECProperty() {return this;}

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY  (PrimitiveType, Type);    
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct StructECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    ECClassCP   m_structType;   

    StructECProperty (ECClassCR ecClass, bool hideFromLeakDetection) : m_structType(NULL), ECProperty(ecClass, hideFromLeakDetection) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual WString                    _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;

/*__PUBLISH_SECTION_START__*/
public:    
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    EXPORTED_PROPERTY  (ECClassCR, Type);    
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
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
      
    ArrayECProperty (ECClassCR ecClass, bool hideFromLeakDetection)
        : m_primitiveType(PRIMITIVETYPE_String), m_arrayKind (ARRAYKIND_Primitive),
          m_minOccurs (0), m_maxOccurs (UINT_MAX), ECProperty(ecClass, hideFromLeakDetection) {};
    ECObjectsStatus                     SetMinOccurs (WStringCR minOccurs);          
    ECObjectsStatus                     SetMaxOccurs (WStringCR maxOccurs);          

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual WString                    _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;

/*__PUBLISH_SECTION_START__*/
public:      
    EXPORTED_READONLY_PROPERTY  (ArrayKind, Kind);        

    EXPORTED_PROPERTY  (PrimitiveType,      PrimitiveElementType);        
    EXPORTED_PROPERTY  (ECClassCP,          StructElementType);    
    EXPORTED_PROPERTY  (UInt32,             MinOccurs);  
    EXPORTED_PROPERTY  (UInt32,             MaxOccurs);     
};

//=======================================================================================
//! Container holding ECProperties that supports STL like iteration
//=======================================================================================
struct ECPropertyIterable
{
private:
    friend struct ECClass;
    
    ECClassCR       m_ecClass;
    bool            m_includeBaseProperties;
    
/*__PUBLISH_SECTION_END__*/
    ECPropertyIterable(ECClassCR ecClass, bool includeBaseProperties) : m_ecClass(ecClass), m_includeBaseProperties(includeBaseProperties) {};
/*__PUBLISH_SECTION_START__*/
public:

    struct IteratorState /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
        {        
        friend struct const_iterator;
/*__PUBLISH_SECTION_END__*/
        public:            
            PropertyList::const_iterator     m_listIterator;
            PropertyList*                    m_properties;

            IteratorState (ECClassCR ecClass, bool includeBaseProperties);
            ~IteratorState();
            static RefCountedPtr<IteratorState> Create (ECClassCR ecClass, bool includeBaseProperties) 
                { return new IteratorState(ecClass, includeBaseProperties); };
/*__PUBLISH_SECTION_START__*/                        
        };
        
    struct const_iterator
        {
        private:
            friend ECPropertyIterable;
            RefCountedPtr<IteratorState>   m_state;
            bool m_isEnd;
 
/*__PUBLISH_SECTION_END__*/
            const_iterator (ECClassCR ecClass, bool includeBaseProperties);
            const_iterator () : m_isEnd(true) {};
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

typedef bvector<ECClassP> ECBaseClassesList;
typedef bvector<ECClassP> ECDerivedClassesList;
typedef bvector<ECClassP> ECConstraintClassesList;

/*__PUBLISH_SECTION_END__*/
typedef bool (*TraversalDelegate) (ECClassCP, const void *);
/*__PUBLISH_SECTION_START__*/

//=======================================================================================
//! The in-memory representation of an ECClass as defined by ECSchemaXML
//=======================================================================================
struct ECClass /*__PUBLISH_ABSTRACT__*/ : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/

friend struct ECSchema;
friend struct ECPropertyIterable::IteratorState;

private:
    WString                        m_name;
    WString                        m_displayLabel;
    WString                        m_description;
    bool                            m_isStruct;
    bool                            m_isCustomAttributeClass;
    bool                            m_isDomainClass;
    ECSchemaCR                      m_schema;
    ECBaseClassesList               m_baseClasses;
    mutable ECDerivedClassesList    m_derivedClasses;
    bool                            m_hideFromLeakDetection;

    PropertyMap                     m_propertyMap;
    PropertyList                    m_propertyList;    
    
    ECObjectsStatus AddProperty (ECPropertyP& pProperty);
    ECObjectsStatus AddProperty (ECPropertyP pProperty, WStringCR name);
    
    static bool     CheckBaseClassCycles(ECClassCP currentBaseClass, const void * arg);
    static bool     AddUniquePropertiesToList(ECClassCP crrentBaseClass, const void * arg);
    bool            TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, const void * arg) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetProperties(bool includeBaseProperties, PropertyList* propertyList) const;

    ECObjectsStatus CanPropertyBeOverridden(ECPropertyCR baseProperty, ECPropertyCR newProperty) const;
    void            AddDerivedClass(ECClassCR baseClass) const;
    void            RemoveDerivedClass(ECClassCR baseClass) const;

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECSchemaCR schema, bool hideFromLeakDetection);
    virtual ~ECClass();    

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP                  _GetContainerSchema() const override;

    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus                     SetName (WStringCR name);    

    virtual SchemaDeserializationStatus ReadXmlAttributes (MSXML2_IXMLDOMNode& classNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @return   Status code
    virtual SchemaDeserializationStatus ReadXmlContents (MSXML2_IXMLDOMNode& classNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator);    
    
    virtual SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode) const;
    SchemaSerializationStatus           WriteXml(MSXML2_IXMLDOMElement& parentNode, WCharCP elementName) const;

public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_READONLY_PROPERTY (ECSchemaCR,             Schema);                
    // schemas index class by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (WStringCR,        Name);        
    EXPORTED_READONLY_PROPERTY (bool,                   IsDisplayLabelDefined);
    EXPORTED_READONLY_PROPERTY (ECPropertyIterable,     Properties); 
    EXPORTED_READONLY_PROPERTY (const ECBaseClassesList&,     BaseClasses);   
    EXPORTED_READONLY_PROPERTY (const ECDerivedClassesList&,  DerivedClasses);   

    EXPORTED_PROPERTY  (WStringCR,                Description);
    EXPORTED_PROPERTY  (WStringCR,                DisplayLabel);
    EXPORTED_PROPERTY  (bool,                           IsStruct);    
    EXPORTED_PROPERTY  (bool,                           IsCustomAttributeClass);    
    EXPORTED_PROPERTY  (bool,                           IsDomainClass);    
    
    //! Returns pointer to ECRelationshipClassP,  used to avoid dynamic_cast.
    //! @return     Returns NULL if not an ECRelationshipClass
    ECOBJECTS_EXPORT virtual ECRelationshipClassCP GetRelationshipClassCP () const {return NULL;}  // used to avoid dynamic_cast

    //! Returns a list of properties for this class.
    //! @param[in]  includeBaseProperties If true, then will return properties that are contained in this class's base class(es)
    //! @return     An iterable container of ECProperties
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties(bool includeBaseProperties) const;

    //! Sets the bool value of whether this class can be used as a struct
    //! @param[in] isStruct String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsStruct (WCharCP isStruct);
    
    //! Sets the bool value of whether this class can be used as a custom attribte
    //! @param[in] isCustomAttribute String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsCustomAttributeClass (WCharCP isCustomAttribute);
    
    //! Sets the bool value of whether this class can be used as a domain object
    //! @param[in] isDomainClass String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsDomainClass (WCharCP isDomainClass);
    
    //! Adds a base class
    //! You cannot add a base class if it creates a cycle. For example, if A is a base class
    //! of B, and B is a base class of C, you cannot make C a base class of A. Attempting to do
    //! so will return an error.
    //! @param[in] baseClass The class to derive from
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ECClassCR baseClass);
    
    //! Returns whether there are any base classes for this class
    ECOBJECTS_EXPORT bool            HasBaseClasses() const;
    
    //! Removes a base class.
    ECOBJECTS_EXPORT ECObjectsStatus RemoveBaseClass(ECClassCR baseClass);
    
    //! Returns true if the class is the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(ECClassCP targetClass) const;

    //! If the given name is valid, creates a primitive property object with the default type of STRING
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, WStringCR name);

    //! If the given name is valid, creates a primitive property object with the given primitive type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, WStringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates a struct property object using the current class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, WStringCR name);

    //! If the given name is valid, creates a struct property object using the specified class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, WStringCR name, ECClassCR structType);

    //! If the given name is valid, creates an array property object using the current class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, WStringCR name);

    //! If the given name is valid, creates an array property object using the specified primitive type as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, WStringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates an array property object using the specified class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, WStringCR name, ECClassCP structType);
    
    //! Remove the named property
    //! @param[in] name The name of the property to be removed
    ECOBJECTS_EXPORT ECObjectsStatus RemoveProperty(WStringCR name);
     
    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @return   A pointer to an EC::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WCharCP name) const;

    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WStringCR name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! Given a qualified class name, will parse out the schema's namespace prefix and the class name.
    //! @param[out] prefix  The namespace prefix of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedClassName  The qualified name of the class, in the format of ns:className
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (WString & prefix, WString & className, WStringCR qualifiedClassName);

    //! Given a schema and a class, will return the fully qualified class name.  If the class is part of the passed in schema, there
    //! is no namespace prefix.  Otherwise, the class's schema must be a referenced schema in the passed in schema
    //! @param[in]  primarySchema   The schema used to lookup the namespace prefix of the class's schema
    //! @param[in]  ecClass         The class whose schema should be searched for
    //! @return WString    The namespace prefix if the class's schema is not the primarySchema
    ECOBJECTS_EXPORT static WString GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass);

    //! Given two ECClass's, checks to see if they are equal by name
    //! @param[in]  currentBaseClass    The source class to check against
    //! @param[in]  arg                 The target to compare to (this parameter must be an ECClassP)
    ECOBJECTS_EXPORT static bool    ClassesAreEqualByName(ECClassCP currentBaseClass, const void * arg);

   
}; // ECClass

enum ECRelationshipEnd { ECRelationshipEnd_Source = 0, ECRelationshipEnd_Target };
//! Used to describe the direction of a related instance within the context
//! of an IECRelationshipInstance
enum ECRelatedInstanceDirection
    {
    //! Related instance is the target in the relationship instance
    STRENGTHDIRECTION_Forward = 1,
    //! Related instance is the source in the relationship instance
    STRENGTHDIRECTION_Backward = 2
    };
 
//! The various strengths supported on a relationship class.
enum StrengthType
    {
    //!  'Referencing' relationships imply no ownership and no cascading deletes when the
    //! object on either end of the relationship is deleted.  For example, a document
    //! object may have a reference to the User that last modified it.
    //! This is like "Association" in UML.
    STRENGTHTYPE_Referencing,
    //! 'Holding' relationships imply shared ownership.  A given object can be "held" by
    //! many different objects, and the object will not get deleted unless all of the
    //! objects holding it are first deleted (or the relationships severed.)
    //! This is like "Aggregation" in UML.
    STRENGTHTYPE_Holding,
    //! 'Embedding' relationships imply exclusive ownership and cascading deletes.  An
    //! object that is the target of an 'embedding' relationship may also be the target
    //! of other 'referencing' relationships, but cannot be the target of any 'holding'
    //! relationships.  For examples, a Folder 'embeds' the Documents that it contains.
    //! This is like "Composition" in UML.
    STRENGTHTYPE_Embedding
    } ;
    
//=======================================================================================
//! This class describes the cardinality of a relationship. It is based on the
//!     Martin notation. Valid cardinalities are (x,y) where x is smaller or equal to y,
//!     x >= 0 and y >= 1 or y = n (where n represents infinity).
//!     For example, (0,1), (1,1), (1,n), (0,n), (1,10), (2,5), ...
//=======================================================================================
struct RelationshipCardinality 
{
private:
    UInt32     m_lowerLimit;
    UInt32     m_upperLimit;

public:
    //! Default constructor.  Creates a cardinality of (0, 1)
    ECOBJECTS_EXPORT RelationshipCardinality();
    
    //! Constructor with lower and upper limit parameters.
    //! @param[in]  lowerLimit  must be less than or equal to upperLimit and greater than or equal to 0
    //! @param[in]  upperLimit  must be greater than or equal to lowerLimit and greater than 0
    ECOBJECTS_EXPORT RelationshipCardinality(UInt32 lowerLimit, UInt32 upperLimit);
    
    //! Returns the lower limit of the cardinality
    EXPORTED_READONLY_PROPERTY  (UInt32, LowerLimit);
    //! Returns the upper limit of the cardinality
    EXPORTED_READONLY_PROPERTY  (UInt32, UpperLimit);
    
    //! Indicates if the cardinality is unbound (ie, upper limit is equal to "n")
    ECOBJECTS_EXPORT bool     IsUpperLimitUnbounded() const;
    
    //! Converts the cardinality to a string, for example "(0,n)", "(1,1)"
    ECOBJECTS_EXPORT WString ToString() const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    
    ECOBJECTS_EXPORT static RelationshipCardinalityCR ZeroOne();
    ECOBJECTS_EXPORT static RelationshipCardinalityCR ZeroMany();
    ECOBJECTS_EXPORT static RelationshipCardinalityCR OneOne();
    ECOBJECTS_EXPORT static RelationshipCardinalityCR OneMany();
};
   

//=======================================================================================
//! The in-memory representation of the source and target constraints for an ECRelationshipClass as defined by ECSchemaXML
//=======================================================================================
struct ECRelationshipConstraint : IECCustomAttributeContainer
{
friend struct ECRelationshipClass;

private:
    // NEEDSWORK: To be completely compatible, we need to store an ECRelationshipConstraintClass with properties in order
    // to support implicit relationships.  For now, just support explicit relationships
//    stdext::hash_map<ECClassCP, ECRelationshipConstrainClassCP> m_constraintClasses;

    ECConstraintClassesList     m_constraintClasses;
    
    WString                     m_roleLabel;
    bool                        m_isPolymorphic;
    bool                        m_isMultiple;
    RelationshipCardinality*    m_cardinality;
    ECRelationshipClassP        m_relClass;
    
    ECObjectsStatus             SetCardinality(WCharCP cardinality);
    ECObjectsStatus             SetCardinality(UInt32& lowerLimit, UInt32& upperLimit);
   
    SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode, WStringCR elementName) const;
    SchemaDeserializationStatus ReadXml(MSXML2_IXMLDOMNode& constraintNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator);
    
    virtual ~ECRelationshipConstraint();
    
protected:
    virtual ECSchemaCP          _GetContainerSchema() const override;
  
public:
    //! Initializes a new instance of the ECRelationshipConstraint class.
    //! IsPolymorphic defaults to true and IsMultiple defaults to false 
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass);
    
    //! Initializes a new instance of the ECRelationshipConstraint class
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isMultiple);
    
    //! Returns true if the constraint allows for a variable number of classes
    EXPORTED_READONLY_PROPERTY  (bool, IsMultiple);
    
    //! Gets or sets the label of the constraint role in the relationship.
    //! If the role label is not defined, the display label of the relationship class is returned
    EXPORTED_PROPERTY (WString const, RoleLabel);
    
    ECOBJECTS_EXPORT bool IsRoleLabelDefined() const;
    
    //! Returns true if this constraint can also relate to instances of subclasses of classes
    //! applied to the constraint.
    EXPORTED_PROPERTY   (bool, IsPolymorphic) ;
    
    //! Sets the bool value of whether this constraint can also relate to instances of subclasses of classes applied to the constraint.
    //! @param[in] isPolymorphic String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsPolymorphic(WCharCP isPolymorphic);
    
    //! Gets the cardinality of the constraint in the relationship
    EXPORTED_PROPERTY (RelationshipCardinalityCR, Cardinality) ;
    
    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] classConstraint  The class to add
    ECOBJECTS_EXPORT ECObjectsStatus AddClass(ECClassCR classConstraint);
    
    //! Removes the specified class from the constraint.
    //! @param[in] classConstraint  The class to remove
    ECOBJECTS_EXPORT ECObjectsStatus RemoveClass(ECClassCR classConstraint);
    
    //! Returns the classes applied to the constraint.
    EXPORTED_READONLY_PROPERTY (const ECConstraintClassesList&, Classes);
    
};

//=======================================================================================
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//=======================================================================================
struct ECRelationshipClass /*__PUBLISH_ABSTRACT__*/ : public ECClass
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;

private:
    StrengthType     m_strength;
    ECRelatedInstanceDirection     m_strengthDirection;
    ECRelationshipConstraintP      m_target;
    ECRelationshipConstraintP      m_source;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECRelationshipClass (ECSchemaCR schema);
    virtual ~ECRelationshipClass ();
    
    ECObjectsStatus SetStrength(WCharCP strength);
    ECObjectsStatus SetStrengthDirection(WCharCP direction);
    
protected:
    virtual SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode) const override;

    virtual SchemaDeserializationStatus ReadXmlAttributes (MSXML2_IXMLDOMNode& classNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator) override;
    virtual SchemaDeserializationStatus ReadXmlContents (MSXML2_IXMLDOMNode& classNode, IStandaloneEnablerLocatorR  standaloneEnablerLocator) override;

/*__PUBLISH_SECTION_START__*/
public:
    //! Returns pointer to ECRelationshipClassP,  used to avoid dynamic_cast.
    //! @return     Returns NULL if not an ECRelationshipClass
    ECOBJECTS_EXPORT virtual ECRelationshipClassCP        GetRelationshipClassCP () const override {return this;};
    ECOBJECTS_EXPORT ECObjectsStatus                      GetOrderedRelationshipPropertyName (WString& propertyName, ECRelationshipEnd end)  const;
                                                           
    EXPORTED_PROPERTY (StrengthType, Strength);                
    EXPORTED_PROPERTY (ECRelatedInstanceDirection, StrengthDirection);
    //! Gets the constraint at the target end of the relationship
    EXPORTED_READONLY_PROPERTY (ECRelationshipConstraintR, Target);
    //! Gets the constraint at the source end of the relationship
    EXPORTED_READONLY_PROPERTY (ECRelationshipConstraintR, Source);
    EXPORTED_READONLY_PROPERTY (bool, IsExplicit);
    EXPORTED_READONLY_PROPERTY (bool, IsOrdered );

}; // ECRelationshipClass

typedef RefCountedPtr<ECRelationshipClass>      ECRelationshipClassPtr;

typedef bvector<ECSchemaP> ECSchemaReferenceList;
//=======================================================================================
//! Supports STL like iterator of classes in a schema
//=======================================================================================
struct ECClassContainer /*__PUBLISH_ABSTRACT__*/
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECSchema;
    friend struct ECClass;
    friend struct ECRelationshipConstraint;
        
    ClassMap const&     m_classMap;
    
    ECClassContainer (ClassMap const& classMap) : m_classMap (classMap) {};

/*__PUBLISH_SECTION_START__*/

public:    
    //=======================================================================================
    // @bsistruct
    //=======================================================================================
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

    //=======================================================================================
    // @bsistruct
    //=======================================================================================
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

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
enum SchemaMatchType
    {
    //! Find exact VersionMajor, VersionMinor match.
    SCHEMAMATCHTYPE_Exact               =   0,
    //! Find latest version with matching VersionMajor and VersionMinor that is equal or greater.
    SCHEMAMATCHTYPE_LatestCompatible    =   1,
    //! Find latest version.
    SCHEMAMATCHTYPE_Latest              =   2,
    };
   
//=======================================================================================
//! Interface implemented by class that provides schema ownership services.</summary>
//=======================================================================================
struct IECSchemaOwner
{
/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_END__*/
protected:
    virtual ECObjectsStatus _AddSchema   (ECSchemaR) = 0;
    virtual ECObjectsStatus _DropSchema  (ECSchemaR) = 0;
    virtual ECSchemaP       _GetSchema   (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor) = 0;
    virtual ECSchemaP       _LocateSchema (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType) = 0;

public:
    ECObjectsStatus         AddSchema   (ECSchemaR);
    ECObjectsStatus         DropSchema  (ECSchemaR);
    ECSchemaP               GetSchema   (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor);
    ECSchemaP               LocateSchema (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType);

/*__PUBLISH_SECTION_START__*/
};

struct StandaloneECEnabler;
typedef RefCountedPtr<StandaloneECEnabler>    StandaloneECEnablerPtr;
typedef RefCountedPtr<ECSchemaCache>        ECSchemaCachePtr;

//=======================================================================================
//! Interface to find a standalone enabler for a child class of an ECInstance.</summary>
//=======================================================================================
struct IStandaloneEnablerLocator
{
/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_END__*/
protected:
    virtual    StandaloneECEnablerPtr  _ObtainStandaloneInstanceEnabler (WCharCP schemaName, WCharCP className) = 0;

/*__PUBLISH_SECTION_START__*/

public:
    ECOBJECTS_EXPORT StandaloneECEnablerPtr  ObtainStandaloneInstanceEnabler (WCharCP schemaName, WCharCP className);
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaNameClassNamePair
{
public:
    WString m_schemaName;
    WString m_className;

    SchemaNameClassNamePair (WStringCR schemaName, WStringCR className) : m_schemaName (schemaName), m_className  (className) {}
    SchemaNameClassNamePair (WCharCP schemaName, WCharCP className) : m_schemaName (schemaName), m_className  (className) {}
    SchemaNameClassNamePair () {};
    
    bool operator<(SchemaNameClassNamePair other) const
        {
        if (m_schemaName < other.m_schemaName)
            return true;

        if (m_schemaName > other.m_schemaName)
            return false;

        return m_className < other.m_className;
        };
};


//=======================================================================================
//! An object that controls the lifetime of a set of ECSchemas.  When the schema
//! owner is destroyed, so are the schemas that it owns.</summary>
//=======================================================================================
struct ECSchemaCache /*__PUBLISH_ABSTRACT__*/ :  RefCountedBase, IECSchemaOwner, IStandaloneEnablerLocator
{
/*__PUBLISH_SECTION_END__*/
protected:
    bvector<ECSchemaP>                                     m_schemas;
    bmap<SchemaNameClassNamePair, StandaloneECEnablerPtr>  m_ecEnablerMap;
    
    ECOBJECTS_EXPORT virtual ~ECSchemaCache ();

    // IECSchemaOwner
    ECOBJECTS_EXPORT virtual ECObjectsStatus _AddSchema   (ECSchemaR) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _DropSchema  (ECSchemaR) override;
    ECOBJECTS_EXPORT virtual ECSchemaP       _GetSchema   (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor);
    ECOBJECTS_EXPORT virtual ECSchemaP       _LocateSchema (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType);
    
    // IStandaloneEnablerLocator
    ECOBJECTS_EXPORT virtual StandaloneECEnablerPtr _ObtainStandaloneInstanceEnabler (WCharCP schemaName, WCharCP className);

/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT static  ECSchemaCachePtr Create ();
};

//=======================================================================================
//! Interface implemented by class that provides schema location services.</summary>
//=======================================================================================
struct IECSchemaLocator
{
protected:
    virtual ECSchemaP _LocateSchema(WCharCP name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaDeserializationContextR schemaContext) const = 0;

public:
    ECOBJECTS_EXPORT ECSchemaP LocateSchema(WCharCP name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaDeserializationContextR schemaContext);
};

//=======================================================================================
//! The in-memory representation of a schema as defined by ECSchemaXML
//=======================================================================================
struct ECSchema /*__PUBLISH_ABSTRACT__*/ : public IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/

// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    WString            m_name;
    WString            m_namespacePrefix;
    WString            m_displayLabel;
    WString            m_description;
    UInt32              m_versionMajor;
    UInt32              m_versionMinor;    
    ECClassContainer    m_classContainer;
    bool                m_hideFromLeakDetection;

    // maps class name -> class pointer    
    ClassMap m_classMap;
    
    ECSchemaReferenceList m_refSchemaList;
    
    bmap<ECSchemaP, const WString> m_referencedSchemaNamespaceMap;

    ECSchema (bool hideFromLeakDetection);
    virtual ~ECSchema();

    static SchemaDeserializationStatus  ReadXml (ECSchemaP& schemaOut, MSXML2_IXMLDOMDocument2& pXmlDoc, ECSchemaDeserializationContextR context);
    SchemaSerializationStatus           WriteXml (MSXML2_IXMLDOMDocument2* pXmlDoc) const;

    ECObjectsStatus                     AddClass (ECClassP& pClass);
    ECObjectsStatus                     SetVersionFromString (WCharCP versionString);

    typedef bvector<bpair<ECClassP, MSXML2_IXMLDOMNodePtr>>  ClassDeserializationVector;
    SchemaDeserializationStatus         ReadClassStubsFromXml(MSXML2_IXMLDOMNode& schemaNodePtr,ClassDeserializationVector& classes, ECSchemaDeserializationContextR context);
    SchemaDeserializationStatus         ReadClassContentsFromXml(ClassDeserializationVector&  classes, ECSchemaDeserializationContextR context);
    SchemaDeserializationStatus         ReadSchemaReferencesFromXml(MSXML2_IXMLDOMNode& schemaNodePtr, ECSchemaDeserializationContextR context);
    static ECSchemaP                    LocateSchemaByPath(const WString & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR context, bool useLatestCompatibleMatch);
    static ECSchemaP                    LocateSchemaByPath(const WString & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR context);
    static ECSchemaP                    LocateSchemaByStandardPaths(const WString & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR context);
    
    struct  ECSchemaSerializationContext
        {
        bset<WCharCP> m_alreadySerializedClasses;
        };

    SchemaSerializationStatus           WriteSchemaReferences(MSXML2_IXMLDOMElement& parentNode) const;
    SchemaSerializationStatus           WriteClass(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass, ECSchemaSerializationContext&) const;
    SchemaSerializationStatus           WriteCustomAttributeDependencies(MSXML2_IXMLDOMElement& parentNode, IECCustomAttributeContainerCR container, ECSchemaSerializationContext&) const;
    SchemaSerializationStatus           WritePropertyDependencies(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass, ECSchemaSerializationContext&) const;

protected:
    virtual ECSchemaCP                  _GetContainerSchema() const override;

public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY (WStringCR, Name);    
    EXPORTED_PROPERTY (WStringCR, NamespacePrefix);
    EXPORTED_PROPERTY (WStringCR, Description);
    EXPORTED_PROPERTY (WStringCR, DisplayLabel);
    EXPORTED_PROPERTY (UInt32,          VersionMajor);
    EXPORTED_PROPERTY (UInt32,          VersionMinor);

    EXPORTED_READONLY_PROPERTY (ECClassContainerCR, Classes);
    EXPORTED_READONLY_PROPERTY (bool,               IsDisplayLabelDefined);

    //! If the class name is valid, will create an ECClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateClass (ECClassP& ecClass, WStringCR name);

    //! If the class name is valid, will create an ECRelationshipClass object and add the new class to the schema
    //! @param[out] relationshipClass If successful, will contain a new ECRelationshipClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateRelationshipClass (ECRelationshipClassP& relationshipClass, WStringCR name);

    //! Get a schema by namespace prefix within the context of this schema and its referenced schemas.
    //! @param[in]  namespacePrefix     The prefix of the schema to lookup in the context of this schema and it's references.
    //!                                 Passing an empty namespacePrefix will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an EC::ECSchema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaP          GetSchemaByNamespacePrefixP(WStringCR namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup a namespace prefix in the context of this schema and its references.
    //! @param[out] namespacePrefix The namespace prefix if schema is a referenced schema; empty string if the sechema is the current schema;    
    //! @return   Success if the schema is either the current schema or a referenced schema;  ECOBJECTS_STATUS_SchemaNotFound if the schema is not found in the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus    ResolveNamespacePrefix(ECSchemaCR schema, WString & namespacePrefix) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (WCharCP name) const;

    //! Gets the other schemas that are used by classes within this schema.
    //! Referenced schemas are the schemas that contain definitions of base classes,
    //! embedded structures, and custom attributes of classes within this schema.
    ECOBJECTS_EXPORT const ECSchemaReferenceList& GetReferencedSchemas() const;
    
    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus            AddReferencedSchema(ECSchemaR refSchema);
    
    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveReferencedSchema(ECSchemaR refSchema);

    //! Serializes an ECXML schema to a string
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus  WriteXmlToString (WString & ecSchemaXml) const;
    
    //! Serializes an ECXML schema to a file
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus  WriteXmlToFile (WCharCP ecSchemaXmlFile);
    
    
    //! Serializes an ECXML schema to an IStream
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    ECOBJECTS_EXPORT SchemaSerializationStatus  WriteXmlToStream (IStreamP ecSchemaXmlStream);
    
    
    //! Return full schema name in format Name.MM.mm where Name is the schema name, MM is major version and mm is minor version.
    ECOBJECTS_EXPORT WString               GetFullSchemaName () const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! If the given schemaName is valid, this will create a new schema object
    //! @param[out] schemaOut   if successful, will contain a new schema object
    //! @param[in]  schemaName  Name of the schema to be created.
    //! @param[in]  versionMajor The major version number.
    //! @param[in]  versionMinor The minor version number.
    //! @param[in]  owner        An object that will control the lifecycle of the newly created schema object.
    //! @return A status code indicating whether the call was succesfull or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaP& schemaOut, WStringCR schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR owner);

/*__PUBLISH_SECTION_END__*/
    // Should only be called by SchemaOwners.  Since IECSchemaOwner is not published, neither should this method be published
    ECOBJECTS_EXPORT static void            DestroySchema (ECSchemaP& schema);

    // This method is intended for use by internal code that needs to manage the schema's lifespan internally.
    // For example one ECXLayout schema is created for each session and never freed.  Obviously care should be
    // taken that Schema's allocated this way are not actually leaked.       
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaP& schemaOut, WStringCR schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR owner,
                                                          bool hideFromLeakDetection);
/*__PUBLISH_SECTION_START__*/

    //! Generate a schema version string given the major and minor version values.
    //! @param[in]  versionMajor    The major version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static WString        FormatSchemaVersion (UInt32& versionMajor, UInt32& versionMinor);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (Name.MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, UInt32& versionMajor, UInt32& versionMinor, WCharCP fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (Name.MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, UInt32& versionMajor, UInt32& versionMinor, WStringCR fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, WCharCP versionString);
    
    //! Given a match type, will determine whether the two schemas match based on name, major version and minor version.  This does not compare actual schemas
    //! @param[in]  matchType   An enum indicating what type of match should be done (exact, latest, latestCompatible)
    //! @param[in]  soughtName  The name of one of the schemas to test
    //! @param[in]  soughtMajor The major version number of one of the schemas to test
    //! @param[in]  soughtMinor The minor version nubmer of one of the schemas to test
    //! @param[in]  candidateName   The name of the schema to compare against
    //! @param[in]  candidateMajor  The major version of the schema to compare against
    //! @param[in]  candidateMinor  The minor version of the schema to compare against
    //! @return True if the schemas do match, false otherwise
    ECOBJECTS_EXPORT static bool SchemasMatch (SchemaMatchType matchType,
                          WCharCP soughtName,    UInt32 soughtMajor,    UInt32 soughtMinor,
                          WCharCP candidateName, UInt32 candidateMajor, UInt32 candidateMinor);

    //! Given two schemas, will check to see if the second schema is referenced by the first schema
    //! @param[in]    thisSchema            The base schema to check the references of
    //! @param[in]    thatSchema            The schema to search for
    //! @return True if thatSchema is referenced by thisSchema, false otherwise
    ECOBJECTS_EXPORT static bool                        IsSchemaReferenced (ECSchemaCR thisSchema, ECSchemaCR thatSchema);

    //! Compare two schemas and returns true if the schema pointers are equal or the names and version of the schemas are the same 
    //! @param[out]   thisSchema           Pointer to schema
    //! @param[out]   thatSchema           Pointer to schema
    ECOBJECTS_EXPORT static bool                        SchemasAreEqualByName (ECSchemaCP thisSchema, ECSchemaCP thatSchema);

    //! Deserializes an ECXML schema from a file.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to deserialize.
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromFile (ECSchemaP& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaDeserializationContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema pathes 
    //! relative to the executing dll will be searched.
    //! @param[in]    name                The schema name to locate.
    //! @param[in]    versionMajor        The major version number of the schema to locate.
    //! @param[in]    versionMinor        The minor version number of the schema to locate.
    //! @param[in]    schemaContext       Required to create schemas
    ECOBJECTS_EXPORT static ECSchemaP                   LocateSchema(const WString & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR schemaContext);

    //! Deserializes an ECXML schema from a string.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to deserialize
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromString (ECSchemaP& schemaOut, WCharCP ecSchemaXml, ECSchemaDeserializationContextR schemaContext);

    //! Deserializes an ECXML schema from an IStream.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to deserialize
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromStream (ECSchemaP& schemaOut, IStreamP ecSchemaXmlStream, ECSchemaDeserializationContextR schemaContext);
    
}; // ECSchema

END_BENTLEY_EC_NAMESPACE

#undef MSXML2_IXMLDOMNode
#undef MSXML2_IXMLDOMNodePtr
#undef MSXML2_IXMLDOMDocument2
#undef MSXML2_IXMLDOMElementPtr
