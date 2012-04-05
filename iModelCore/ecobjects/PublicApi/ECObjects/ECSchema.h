/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECEnabler.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/bset.h>

//__PUBLISH_SECTION_END__
#include <boost/foreach.hpp>
//__PUBLISH_SECTION_START__

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

struct NameValidator /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    static bool Validate(WStringCR name);
};
    
typedef bvector<ECPropertyP> PropertyList;
typedef bmap<WCharCP , ECPropertyP, less_str> PropertyMap;
typedef bmap<WCharCP , ECClassP,    less_str> ClassMap;

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

    //! Returns the ValueKind of the ECProperty
    inline ValueKind            GetTypeKind() const         { return m_typeKind; }
    //! Returns the ArrayKind of the ECProperty, if the ECProperty is an array property
    inline ArrayKind            GetArrayKind() const        { return (ArrayKind)(m_arrayKind & 0xFF); }
    //! Returns true if the ECProperty is a Primitive property
    inline bool                 IsPrimitive() const         { return (GetTypeKind() == VALUEKIND_Primitive ); }
    //! Returns true if the ECProperty is a Struct property
    inline bool                 IsStruct() const            { return (GetTypeKind() == VALUEKIND_Struct ); }
    //! Returns true if the ECProperty is an Array property
    inline bool                 IsArray() const             { return (GetTypeKind() == VALUEKIND_Array ); }
    //! Returns true if the ECProperty is an Array property, and the array elements are Primitives
    inline bool                 IsPrimitiveArray() const    { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Primitive); }
    //! Returns true if the ECProperty is an Array property, and the array elements are Structs
    inline bool                 IsStructArray() const       { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Struct); }
    //! Returns the primitive type of the ECProperty, if the property is a Primitive type
    inline PrimitiveType        GetPrimitiveType() const    { return m_primitiveType; }
/*__PUBLISH_SECTION_END__*/
    inline short                GetTypeKindQualifier() const   { return m_primitiveType; }
/*__PUBLISH_SECTION_START__*/        
};    

// NEEDSWORK - unsure what the best way is to model ECProperty.  Managed implementation has a single ECProperty and introduces an ECType concept.  My gut is that
// this is overkill for the native implementation.  Alternatively we could have a single ECProperty class that could act as primitive/struct/array or we can take the
// appoach I've implemented below.      

typedef bvector<IECInstancePtr> ECCustomAttributeCollection;
struct ECCustomAttributeInstanceIterable;

//=======================================================================================
//
//=======================================================================================
struct IECCustomAttributeContainer /*__PUBLISH_ABSTRACT__*/  
{
/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECCustomAttributeInstanceIterable;
    ECCustomAttributeCollection         m_customAttributes;

    SchemaWriteStatus                   AddCustomAttributeProperties (BeXmlNodeR oldNode, BeXmlNodeR newNode) const;

    IECInstancePtr                      GetCustomAttributeInternal(WStringCR className, bool includeBaseClasses) const;
    IECInstancePtr                      GetCustomAttributeInternal(ECClassCR ecClass, bool includeBaseClasses) const;

protected:
    InstanceReadStatus                  ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR context);
    SchemaWriteStatus                   WriteCustomAttributes(BeXmlNodeR parentNode) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP                  _GetContainerSchema() const = 0;// {return NULL;};

    ECOBJECTS_EXPORT virtual ~IECCustomAttributeContainer();

/*__PUBLISH_SECTION_START__*/
public:
    //! Returns true if the container has a custom attribute of a class of the specified name
    ECOBJECTS_EXPORT bool               IsDefined(WStringCR className) ;
    //! Returns true if the container has a custom attribute of a class of the specified class definition
    ECOBJECTS_EXPORT bool               IsDefined(ECClassCR classDefinition) ;

    //! Retrieves the custom attribute matching the class name.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(WStringCR className) const;

    //! Retrieves the custom attribute matching the class name.  Does not look on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(WStringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class definition.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(ECClassCR classDefinition) const;

    //! Retrieves all custom attributes from the container
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers 
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetCustomAttributes(bool includeBase) const; 

    //! Adds a custom attribute to the container
    ECOBJECTS_EXPORT ECObjectsStatus    SetCustomAttribute(IECInstanceR customAttributeInstance);

    //! Removes a custom attribute from the container
    //! @param[in]  className   Name of the class of the custom attribute to remove
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(WStringCR className);

    //! Removes a custom attribute from the container
    //! @param[in]  classDefinition ECClass of the custom attribute to remove
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

    struct const_iterator : std::iterator<std::forward_iterator_tag, IECInstancePtr const>
        {
        private:
            friend struct ECCustomAttributeInstanceIterable;
            RefCountedPtr<IteratorState> m_state;
            bool m_isEnd;
/*__PUBLISH_SECTION_END__*/
            const_iterator (IECCustomAttributeContainerCR container, bool includeBase);
            const_iterator () : m_isEnd(true) {};
/*__PUBLISH_SECTION_START__*/                        
           
        public:
            ECOBJECTS_EXPORT const_iterator&     operator++();
            ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
            ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const {return !(*this != rhs);}
            ECOBJECTS_EXPORT IECInstancePtr const& operator* () const;
        };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;    
};

struct PrimitiveECProperty;

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct ECProperty /*abstract*/ : public IECCustomAttributeContainer
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
    static void     SetErrorHandling (bool doAssert);
protected:
    WString         m_originalTypeName; //Will be empty unless the typeName was unrecognized. Keep this so that we can re-write the ECSchema without changing the type to string
    ECProperty (ECClassCR ecClass, bool hideFromLeakDetection);
    virtual ~ECProperty();

    ECObjectsStatus                     SetName (WStringCR name);

    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext);
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode);
    SchemaWriteStatus                   _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode, Utf8CP elementName);

    virtual bool                        _IsPrimitive () const { return false; }
    virtual bool                        _IsStruct () const { return false; }
    virtual bool                        _IsArray () const { return false; }
    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual WString                     _GetTypeName () const = 0;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) = 0;

    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const = 0;

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP                  _GetContainerSchema() const override;

    virtual PrimitiveECProperty*        _GetAsPrimitiveECProperty() {return NULL;}
    
public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();

/*__PUBLISH_SECTION_START__*/
public:    
    //! Returns the name of the ECClass that this property is contained within
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;   
    // ECClass implementation will index property by name so publicly name can not be reset
    //! Gets the name of the ECProperty
    ECOBJECTS_EXPORT WStringCR          GetName() const;        
    //! Returns whether the DisplayLabel is explicitly set
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;    
    //! Returns whether this property is a Struct property
    ECOBJECTS_EXPORT bool               GetIsStruct() const;    
    //! Returns whether this property is an Array property
    ECOBJECTS_EXPORT bool               GetIsArray() const;
    //! Returns whether this property is a Primitive property    
    ECOBJECTS_EXPORT bool               GetIsPrimitive() const;    

    //! Sets the ECXML typename for the property.  @see GetTypeName()
    ECOBJECTS_EXPORT ECObjectsStatus    SetTypeName(WString value);
    //! The ECXML typename for the property.  
    //! The TypeName for struct properties will be the ECClass name of the struct.  It may be qualified with a namespacePrefix if 
    //! the struct belongs to a schema that is referenced by the schema actually containing this property.
    //! The TypeName for array properties will be the type of the elements the array contains.
    //! This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    //! prefix relative to the containing schema.
    ECOBJECTS_EXPORT WString            GetTypeName() const;      
    //! Sets the description for this ECProperty  
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(WStringCR value);
    //! The Description of this ECProperty. 
    ECOBJECTS_EXPORT WStringCR          GetDescription() const;
    //! Sets the Display Label for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(WStringCR value);
    //! Gets the Display Label for this ECProperty.  If no label has been set explicitly, it will return the Name of the property
    ECOBJECTS_EXPORT WStringCR          GetDisplayLabel() const;    
    //! Sets whether this ECProperty's value is read only
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsReadOnly(bool value);
    //! Gets whether this ECProperty's value is read only
    ECOBJECTS_EXPORT bool               GetIsReadOnly() const;
    //! Sets the base property that this ECProperty inherits from
    ECOBJECTS_EXPORT ECObjectsStatus    SetBaseProperty(ECPropertyCP value);
    //! Gets the base property, if any, that this ECProperty inherits from
    ECOBJECTS_EXPORT ECPropertyCP       GetBaseProperty() const;    

    //! Sets whether this ECProperty's value is read only
    //@param[in]    isReadOnly  Valid values are 'True' and 'False' (case insensitive)
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsReadOnly (WCharCP isReadOnly);

    // NEEDSWORK, don't necessarily like this pattern but it will suffice for now.  Necessary since you can't dynamic_cast when using the published headers.  How
    // do other similiar classes deal with this.
    ECOBJECTS_EXPORT PrimitiveECPropertyP GetAsPrimitiveProperty () const;    // FUSION_WIP: this removes const!
    ECOBJECTS_EXPORT ArrayECPropertyP     GetAsArrayProperty () const;        //  "
    ECOBJECTS_EXPORT StructECPropertyP    GetAsStructProperty () const;       //  "
};


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct PrimitiveECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    PrimitiveType   m_primitiveType;   

    PrimitiveECProperty (ECClassCR ecClass, bool hideFromLeakDetection) : m_primitiveType(PRIMITIVETYPE_String), ECProperty(ecClass, hideFromLeakDetection) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual PrimitiveECProperty*        _GetAsPrimitiveECProperty() {return this;}

/*__PUBLISH_SECTION_START__*/
public:    
    //! Sets the PrimitiveType of this ECProperty.  The default type is PRIMITIVETYPE_String
    ECOBJECTS_EXPORT ECObjectsStatus SetType(PrimitiveType value);
    //! Gets the PrimitiveType of this ECProperty
    ECOBJECTS_EXPORT PrimitiveType GetType() const;    
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct StructECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    ECClassCP   m_structType;   

    StructECProperty (ECClassCR ecClass, bool hideFromLeakDetection) : m_structType(NULL), ECProperty(ecClass, hideFromLeakDetection) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;

/*__PUBLISH_SECTION_START__*/
public:    
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    ECOBJECTS_EXPORT ECObjectsStatus    SetType(ECClassCR value);
    ECOBJECTS_EXPORT ECClassCR          GetType() const;    
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//=======================================================================================
struct ArrayECProperty /*__PUBLISH_ABSTRACT__*/ : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
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
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml(BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;

/*__PUBLISH_SECTION_START__*/
public:      
    //! The ArrayKind of this ECProperty
    ECOBJECTS_EXPORT ArrayKind GetKind() const;        

    //! Sets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    ECOBJECTS_EXPORT ECObjectsStatus    SetPrimitiveElementType(PrimitiveType value);
    //! Gets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    ECOBJECTS_EXPORT PrimitiveType      GetPrimitiveElementType() const;        
    //! Sets the ECClass to be used for the array's struct elements
    ECOBJECTS_EXPORT ECObjectsStatus    SetStructElementType(ECClassCP value);
    //! Gets the ECClass of the array's struct elements
    ECOBJECTS_EXPORT ECClassCP          GetStructElementType() const;    
    //! Sets the Minimum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus    SetMinOccurs(UInt32 value);
    //! Gets the Minimum number of array members.
    ECOBJECTS_EXPORT UInt32             GetMinOccurs() const;  
    //! Sets the Maximum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus    SetMaxOccurs(UInt32 value);
    //! Gets the Maximum number of array members.
    ECOBJECTS_EXPORT UInt32             GetMaxOccurs() const;     
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
        
    struct const_iterator : std::iterator<std::forward_iterator_tag, const ECPropertyP>
        {
        private:
            friend struct ECPropertyIterable;
            RefCountedPtr<IteratorState>   m_state;
            bool m_isEnd;
 
/*__PUBLISH_SECTION_END__*/
            const_iterator (ECClassCR ecClass, bool includeBaseProperties);
            const_iterator () : m_isEnd(true) {};
/*__PUBLISH_SECTION_START__*/                        
           
        public:
            ECOBJECTS_EXPORT const_iterator&     operator++();
            ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
            ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const {return !(*this != rhs);}
            ECOBJECTS_EXPORT ECPropertyP const&  operator* () const;
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

struct StandaloneECEnabler;
struct SearchPathSchemaFileLocater;
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
typedef StandaloneECEnabler*                StandaloneECEnablerP;
typedef RefCountedPtr<ECSchema>             ECSchemaPtr;
typedef RefCountedPtr<SearchPathSchemaFileLocater> SearchPathSchemaFileLocaterPtr;

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of an ECClass as defined by ECSchemaXML
//=======================================================================================
struct ECClass /*__PUBLISH_ABSTRACT__*/ : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/

friend struct ECSchema;
friend struct ECPropertyIterable::IteratorState;

private:
    WString                         m_name;
    mutable WString                 m_fullName;
    WString                         m_displayLabel;
    WString                         m_description;
    bool                            m_isStruct;
    bool                            m_isCustomAttributeClass;
    bool                            m_isDomainClass;
    ECSchemaCR                      m_schema;
    ECBaseClassesList               m_baseClasses;
    mutable ECDerivedClassesList    m_derivedClasses;
    bool                            m_hideFromLeakDetection;

    PropertyMap                     m_propertyMap;
    PropertyList                    m_propertyList;
    mutable StandaloneECEnablerPtr  m_defaultStandaloneEnabler;
    
    ECObjectsStatus AddProperty (ECPropertyP& pProperty);
    ECObjectsStatus AddProperty (ECPropertyP pProperty, WStringCR name);
    
    static bool     SchemaAllowsOverridingArrays(ECSchemaCP schema);

    static bool     CheckBaseClassCycles(ECClassCP currentBaseClass, const void * arg);
    static bool     AddUniquePropertiesToList(ECClassCP crrentBaseClass, const void * arg);
    bool            TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, const void * arg) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetProperties(bool includeBaseProperties, PropertyList* propertyList) const;

    ECObjectsStatus CanPropertyBeOverridden(ECPropertyCR baseProperty, ECPropertyCR newProperty) const;
    void            AddDerivedClass(ECClassCR baseClass) const;
    void            RemoveDerivedClass(ECClassCR baseClass) const;
    static void     SetErrorHandling (bool doAssert);

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

    virtual SchemaReadStatus            _ReadXmlAttributes (BeXmlNodeR classNode);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @return   Status code
    virtual SchemaReadStatus            _ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context);
    
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdClassNode, BeXmlNodeR parentNode) const;
    SchemaWriteStatus                   _WriteXml (BeXmlNodeP& createdClassNode, BeXmlNodeR parentNode, Utf8CP elementName) const;

    virtual ECRelationshipClassCP       _GetRelationshipClassCP () const { return NULL; }  // used to avoid dynamic_cast
    
public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();

/*__PUBLISH_SECTION_START__*/
public:    
    ECOBJECTS_EXPORT StandaloneECEnablerP  GetDefaultStandaloneEnabler() const;
    ECOBJECTS_EXPORT ECRelationshipClassCP GetRelationshipClassCP() const;
    //! The ECSchema that this class is defined in
    ECOBJECTS_EXPORT ECSchemaCR         GetSchema() const;                
    // schemas index class by name so publicly name can not be reset
    //! The name of this ECClass
    ECOBJECTS_EXPORT WStringCR          GetName() const;
    //! {SchemaName}:{ClassName} The pointer will remain valid as long as the ECClass exists.
    ECOBJECTS_EXPORT WCharCP            GetFullName() const;        
    //! Whether the display label is explicitly defined or not
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;
    //! Returns an iterable of all the ECProperties defined on this class
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties() const; 
    //! Returns a list of the classes this ECClass is derived from
    ECOBJECTS_EXPORT const ECBaseClassesList& GetBaseClasses() const;   
    //! Returns a list of the classes that derive from this class.
    ECOBJECTS_EXPORT const ECDerivedClassesList& GetDerivedClasses() const;   

    //! Sets the description of this ECClass
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(WStringCR value);
    //! Gets the description of this ECClass.
    ECOBJECTS_EXPORT WStringCR          GetDescription() const;
    //! Sets the display label of this ECClass
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(WStringCR value);
    //! Gets the display label of this ECClass.  If no display label has been set explicitly, it will return the name of the ECClass
    ECOBJECTS_EXPORT WStringCR          GetDisplayLabel() const;

    //! Returns a list of properties for this class.
    //! @param[in]  includeBaseProperties If true, then will return properties that are contained in this class's base class(es)
    //! @return     An iterable container of ECProperties
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties(bool includeBaseProperties) const;

    //! Sets the bool value of whether this class can be used as a struct
    //! @param[in] isStruct String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsStruct (WCharCP isStruct);
    //! Sets the bool value of whether this class can be used as a struct
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsStruct(bool value);
    //! Returns whether this class can be used as a struct
    ECOBJECTS_EXPORT bool               GetIsStruct() const; 
        
    //! Sets the bool value of whether this class can be used as a custom attribute
    //! @param[in] isCustomAttribute String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsCustomAttributeClass (WCharCP isCustomAttribute);
    //! Sets the bool value of whether this class can be used as a custom attribute
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsCustomAttributeClass(bool value);
    //! Returns whether this class can be used as a custom attribute
    ECOBJECTS_EXPORT bool               GetIsCustomAttributeClass() const;    
    
    //! Sets the bool value of whether this class can be used as a domain object
    //! @param[in] isDomainClass String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsDomainClass (WCharCP isDomainClass);
    //! Sets the bool value of whether this class can be used as a domain object
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsDomainClass(bool value);
    //! Gets whether this class can be used as a domain object
    ECOBJECTS_EXPORT bool               GetIsDomainClass() const;    

    
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

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @return   A pointer to an EC::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WStringCR name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! Given a qualified class name, will parse out the schema's namespace prefix and the class name.
    //! @param[out] prefix  The namespace prefix of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedClassName  The qualified name of the class, in the format of ns:className
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (WStringR prefix, WStringR className, WStringCR qualifiedClassName);

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
    ECOBJECTS_EXPORT UInt32 GetLowerLimit() const;
    //! Returns the upper limit of the cardinality
    ECOBJECTS_EXPORT UInt32 GetUpperLimit() const;
    
    //! Indicates if the cardinality is unbound (ie, upper limit is equal to "n")
    ECOBJECTS_EXPORT bool     IsUpperLimitUnbounded() const;
    
    //! Converts the cardinality to a string, for example "(0,n)", "(1,1)"
    ECOBJECTS_EXPORT WString ToString() const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    
    //!     Returns the shared static RelationshipCardinality object that represents the
    //!     (0,1) cardinality. This static property can be used instead of a standard
    //!     constructor of RelationshipCardinality to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipCardinalityCR ZeroOne();
    //!     Returns the shared static RelationshipCardinality object that represents the
    //!     (0,n) cardinality. This static property can be used instead of a standard
    //!     constructor of RelationshipCardinality to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipCardinalityCR ZeroMany();
    //!     Returns the shared static RelationshipCardinality object that represents the
    //!     (1,1) cardinality. This static property can be used instead of a standard
    //!     constructor of RelationshipCardinality to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipCardinalityCR OneOne();
    //!     Returns the shared static RelationshipCardinality object that represents the
    //!     (1,n) cardinality. This static property can be used instead of a standard
    //!     constructor of RelationshipCardinality to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipCardinalityCR OneMany();
};
   

//=======================================================================================
//! The in-memory representation of the source and target constraints for an ECRelationshipClass as defined by ECSchemaXML
//=======================================================================================
struct ECRelationshipConstraint : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
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
   
    SchemaWriteStatus           WriteXml (BeXmlNodeR parentNode, Utf8CP elementName) const;
    SchemaReadStatus            ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext);
    
    virtual ~ECRelationshipConstraint();
    
protected:
    virtual ECSchemaCP          _GetContainerSchema() const override;
  
    //! Initializes a new instance of the ECRelationshipConstraint class.
    //! IsPolymorphic defaults to true and IsMultiple defaults to false 
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass);  // WIP_CEM... should not be public... create a factory method
    
    //! Initializes a new instance of the ECRelationshipConstraint class
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isMultiple); // WIP_CEM... should not be public... create a factory method

/*__PUBLISH_SECTION_START__*/
public:
    
    //! Returns true if the constraint allows for a variable number of classes
    ECOBJECTS_EXPORT bool                       GetIsMultiple() const;
    
    //! Sets the label of the constraint role in the relationship.
    ECOBJECTS_EXPORT ECObjectsStatus            SetRoleLabel (WStringCR value);
    //! Gets the label of the constraint role in the relationship.
    //! If the role label is not defined, the display label of the relationship class is returned
    ECOBJECTS_EXPORT WString const              GetRoleLabel() const;
    
    //! Returns whether the RoleLabel has been set explicitly
    ECOBJECTS_EXPORT bool                       IsRoleLabelDefined() const;

    //! Sets whether this constraint can also relate to instances of subclasses of classes applied to the constraint.    
    ECOBJECTS_EXPORT ECObjectsStatus            SetIsPolymorphic(bool value);
    //! Returns true if this constraint can also relate to instances of subclasses of classes
    //! applied to the constraint.
    ECOBJECTS_EXPORT bool                       GetIsPolymorphic() const;
    
    //! Sets the bool value of whether this constraint can also relate to instances of subclasses of classes applied to the constraint.
    //! @param[in] isPolymorphic String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus            SetIsPolymorphic(WCharCP isPolymorphic);
    
    //! Sets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus            SetCardinality(RelationshipCardinalityCR value);
    //! Gets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT RelationshipCardinalityCR  GetCardinality() const;
    
    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] classConstraint  The class to add
    ECOBJECTS_EXPORT ECObjectsStatus            AddClass(ECClassCR classConstraint);
    
    //! Removes the specified class from the constraint.
    //! @param[in] classConstraint  The class to remove
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveClass(ECClassCR classConstraint);
    
    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT const ECConstraintClassesList& GetClasses() const;
    
};

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//=======================================================================================
struct ECRelationshipClass /*__PUBLISH_ABSTRACT__*/ : public ECClass
{
    DEFINE_T_SUPER(ECClass)
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
    
    ECObjectsStatus                     SetStrength (WCharCP strength);
    ECObjectsStatus                     SetStrengthDirection (WCharCP direction);
    
protected:
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdClassNode, BeXmlNodeR parentNode) const override;

    virtual SchemaReadStatus            _ReadXmlAttributes (BeXmlNodeR classNode) override;
    virtual SchemaReadStatus            _ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context) override;
    virtual ECRelationshipClassCP       _GetRelationshipClassCP () const override {return this;};

/*__PUBLISH_SECTION_START__*/
public:
    //! Returns pointer to ECRelationshipClassP,  used to avoid dynamic_cast.
    //! @return     Returns NULL if not an ECRelationshipClass
    ECOBJECTS_EXPORT ECObjectsStatus            GetOrderedRelationshipPropertyName (WString& propertyName, ECRelationshipEnd end)  const;
    //! Sets the StrengthType of this constraint.
    ECOBJECTS_EXPORT ECObjectsStatus            SetStrength(StrengthType value);
    //! Gets the StrengthType of this constraint
    ECOBJECTS_EXPORT StrengthType               GetStrength() const;                
    //! Sets the StrengthDirection (either Forward or Backward) of this constraint
    ECOBJECTS_EXPORT ECObjectsStatus            SetStrengthDirection(ECRelatedInstanceDirection value);
    //! Gets the StrengthDirection (either Forward or Backward) of this constraint
    ECOBJECTS_EXPORT ECRelatedInstanceDirection GetStrengthDirection() const;
    //! Gets the constraint at the target end of the relationship
    ECOBJECTS_EXPORT ECRelationshipConstraintR  GetTarget() const;
    //! Gets the constraint at the source end of the relationship
    ECOBJECTS_EXPORT ECRelationshipConstraintR  GetSource() const;

    //! Returns true if the constraint is explicit
    ECOBJECTS_EXPORT bool                       GetIsExplicit() const;
    //! Returns true if the constraint is ordered.  This is determined by seeing if the custom attribute signifying a Ordered relationship is defined
    ECOBJECTS_EXPORT bool                       GetIsOrdered () const;

}; // ECRelationshipClass

typedef RefCountedPtr<ECRelationshipClass>      ECRelationshipClassPtr;

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
enum SchemaMatchType
    {
    //! Find exact VersionMajor, VersionMinor match as well as Data
    SCHEMAMATCHTYPE_Identical           =   0,
    //! Find exact VersionMajor, VersionMinor match.
    SCHEMAMATCHTYPE_Exact               =   1, //WIP: Rename this to NameAndVersion
    //! Find latest version with matching VersionMajor and VersionMinor that is equal or greater.
    SCHEMAMATCHTYPE_LatestCompatible    =   2,
    //! Find latest version.
    SCHEMAMATCHTYPE_Latest              =   3, //WIP:Rename this to Name
    };

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SchemaKey
    {
    WString       m_schemaName;
    UInt32        m_versionMajor;
    UInt32        m_versionMinor;
    UInt32        m_checkSum;

    SchemaKey (WCharCP name, UInt32 major, UInt32 minor) : m_schemaName(name), m_versionMajor(major), m_versionMinor(minor), m_checkSum(0){}
    SchemaKey () : m_versionMajor(DEFAULT_VERSION_MAJOR), m_versionMinor(DEFAULT_VERSION_MINOR), m_checkSum(0) {}
    
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (SchemaKey& key, WCharCP schemaFullName);
    
    bool LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const
        {
        int nameCompare = 0;
        switch (matchType)
            {
            case SCHEMAMATCHTYPE_Identical:
                {
                if (0 != m_checkSum || 0 != rhs.m_checkSum)
                    return m_checkSum < rhs.m_checkSum;
                //Fall through
                }
            case SCHEMAMATCHTYPE_Exact:
                {
                nameCompare = wcscmp(m_schemaName.c_str(), rhs.m_schemaName.c_str());
                
                if (nameCompare != 0)
                    return nameCompare < 0;

                if (m_versionMajor != rhs.m_versionMajor)
                    return m_versionMajor < rhs.m_versionMajor;

                return m_versionMinor < rhs.m_versionMinor;
                break;
                }
            case SCHEMAMATCHTYPE_Latest: //Only compare by name
                return nameCompare < 0;
            case SCHEMAMATCHTYPE_LatestCompatible:
                {
                if (nameCompare != 0)
                    return nameCompare < 0;

                return m_versionMajor < rhs.m_versionMajor;
                }
            default:
                return false;
            }
        }
    
    bool Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const
        {
        switch (matchType)
            {
            case SCHEMAMATCHTYPE_Identical:
                {
                if (0 != m_checkSum && 0 != rhs.m_checkSum)
                    return m_checkSum == rhs.m_checkSum;
                //fall through
                }
            case SCHEMAMATCHTYPE_Exact:
                return 0 == wcscmp(m_schemaName.c_str(), rhs.m_schemaName.c_str()) && m_versionMajor == rhs.m_versionMajor && m_versionMinor == rhs.m_versionMinor;
            case SCHEMAMATCHTYPE_Latest:
                return 0 == wcscmp(m_schemaName.c_str(), rhs.m_schemaName.c_str());
            case SCHEMAMATCHTYPE_LatestCompatible:
                return 0 == wcscmp(m_schemaName.c_str(), rhs.m_schemaName.c_str()) && m_versionMajor == rhs.m_versionMajor && m_versionMinor >= rhs.m_versionMinor;
            default:
                return false;
            }
        }
    
    bool operator == (SchemaKeyCR rhs) const
        {
        return Matches(rhs, SCHEMAMATCHTYPE_Identical);
        }
    
    bool operator != (SchemaKeyCR rhs) const
        {
        return !(*this == rhs);
        }

    bool operator < (SchemaKeyCR rhs) const
        {
        return LessThan (rhs, SCHEMAMATCHTYPE_Identical);
        }
/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT WString GetNameString() const; // Rename to Full schema name
/*__PUBLISH_SECTION_START__*/
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <SchemaMatchType MatchType>
struct SchemaKeyMatch : std::binary_function<SchemaKey, SchemaKey, bool>
    {
    bool operator () (SchemaKeyCR lhs, SchemaKeyCR rhs) const
        {
        return lhs.Matches (rhs, MatchType);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <SchemaMatchType MatchType>
struct SchemaKeyLessThan : std::binary_function<SchemaKey, SchemaKey, bool>
    {
    bool operator () (SchemaKeyCR lhs, SchemaKeyCR rhs) const
        {
        return lhs.LessThan (rhs, MatchType);
        }
    };


typedef bmap<SchemaKey , ECSchemaPtr> SchemaMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaKeyMatchPredicate
    {
    SchemaKeyCR     m_key;
    SchemaMatchType m_matchType;

    SchemaKeyMatchPredicate(SchemaKeyCR key, SchemaMatchType matchType)
        :m_key(key), m_matchType(matchType)
        {}

    typedef bpair<SchemaKey, ECSchemaPtr> MapVal;
    bool operator () (MapVal const& rhs)
        {
        return rhs.first.Matches (m_key, m_matchType);
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaMapExact:bmap<SchemaKey, ECSchemaPtr, SchemaKeyLessThan <SCHEMAMATCHTYPE_Exact> >
    {
    SchemaMapExact::const_iterator Find (SchemaKeyCR key, SchemaMatchType matchType)
        {
        switch (matchType)
            {
            case SCHEMAMATCHTYPE_Exact:
                return find(key);
            default:
                return std::find_if(begin(), end(), SchemaKeyMatchPredicate(key, matchType));
            }
        }
    
    //! Get a class by name within the context of this list.
    //! @param[in]  name     The name of the class and schema to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::ECClass if the named class exists in within the current list; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP  FindClassP (EC::SchemaNameClassNamePair const& classNamePair) const;
    };

typedef SchemaMapExact              ECSchemaReferenceList;
typedef const ECSchemaReferenceList& ECSchemaReferenceListCR;


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
    
public:
    ECOBJECTS_EXPORT ECClassContainer (ClassMap const& classMap) : m_classMap (classMap) {}; //public for test purposes only

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
    struct const_iterator : std::iterator<std::forward_iterator_tag, ECClassP const>
    {    
    private:                
        friend struct ECClassContainer;                   
        RefCountedPtr<IteratorState>   m_state;

/*__PUBLISH_SECTION_END__*/
        const_iterator (ClassMap::const_iterator mapIterator) { m_state = IteratorState::Create (mapIterator); };
/*__PUBLISH_SECTION_START__*/                        

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++();
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const;
        ECOBJECTS_EXPORT ECClassP const&     operator* () const;
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

}; 


//=======================================================================================
//! Interface to find a standalone enabler, typically for an embedded ECStruct in an ECInstance.</summary>
//=======================================================================================
struct IStandaloneEnablerLocater
{
/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_END__*/
protected:
    virtual    StandaloneECEnablerPtr  _LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className) = 0;

/*__PUBLISH_SECTION_START__*/

public:
    ECOBJECTS_EXPORT StandaloneECEnablerPtr  LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className);
};


//=======================================================================================
//! Interface implemented by class that provides schema location services.</summary>
//=======================================================================================
struct IECSchemaLocater
{
protected:
    virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) = 0;

public:
    ECOBJECTS_EXPORT ECSchemaPtr LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext);
};

typedef RefCountedPtr<ECSchemaCache>        ECSchemaCachePtr;
//=======================================================================================
//! An object that controls the lifetime of a set of ECSchemas.  When the schema
//! owner is destroyed, so are the schemas that it owns.</summary>
//=======================================================================================
struct ECSchemaCache /*__PUBLISH_ABSTRACT__*/ : public IECSchemaLocater,  public RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
protected:
    SchemaMap   m_schemas;
    
    ECOBJECTS_EXPORT virtual ECSchemaPtr     _LocateSchema (SchemaKeyR schema, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;
    
/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT ECObjectsStatus AddSchema   (ECSchemaR);
    ECOBJECTS_EXPORT ECObjectsStatus DropSchema  (ECSchemaR);
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key);
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key, SchemaMatchType matchType);
    ECOBJECTS_EXPORT virtual ~ECSchemaCache ();
    ECOBJECTS_EXPORT static  ECSchemaCachePtr Create ();
    ECOBJECTS_EXPORT int     GetCount();
    ECOBJECTS_EXPORT void    Clear();
};



//=======================================================================================
//! Locates schemas by looking in a given set of file system folder for ECSchemaXml files
//=======================================================================================
struct SearchPathSchemaFileLocater : IECSchemaLocater, RefCountedBase, NonCopyableClass
{
/*__PUBLISH_SECTION_END__*/
private:
    bvector<WString> m_searchPaths;
    SearchPathSchemaFileLocater (bvector<WString> const& searchPaths);
    virtual ~SearchPathSchemaFileLocater();
    static ECSchemaPtr                  LocateSchemaByPath (SchemaKeyR key, ECSchemaReadContextR context, SchemaMatchType matchType, bvector<WString>& searchPaths);
    
    static ECSchemaPtr                  FindMatchingSchema (WStringCR schemaMatchExpression, SchemaKeyR key, ECSchemaReadContextR schemaContext, SchemaMatchType matchType, bvector<WString>& searchPaths);

protected:
    virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;

public:
    bvector<WString>const& GetSearchPath () const {return m_searchPaths;}
/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT static SearchPathSchemaFileLocaterPtr CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths);
};

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of a schema as defined by ECSchemaXML
//=======================================================================================
struct ECSchema /*__PUBLISH_ABSTRACT__*/ :RefCountedBase, IECCustomAttributeContainer, NonCopyableClass
{
/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_END__*/
friend struct SearchPathSchemaFileLocater;
// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    SchemaKey               m_key;
    WString                 m_namespacePrefix;
    WString                 m_displayLabel;
    WString                 m_description;
    ECClassContainer        m_classContainer;
    bool                    m_hideFromLeakDetection;

    // maps class name -> class pointer    
    ClassMap                m_classMap;
    ECSchemaReferenceList   m_refSchemaList;
    
    bmap<ECSchemaP, const WString> m_referencedSchemaNamespaceMap;

    ECSchema (bool hideFromLeakDetection);
    virtual ~ECSchema();

    bool                                AddingSchemaCausedCycles () const;
    bool                                IsOpenPlantPidCircularReferenceSpecialCase(WString& referencedECSchemaName);
    static SchemaReadStatus             ReadXml (ECSchemaPtr& schemaOut, BeXmlDomR xmlDom, UInt32 checkSum, ECSchemaReadContextR context);
    SchemaWriteStatus                   WriteXml (BeXmlDomR xmlDoc) const;

    ECObjectsStatus                     AddClass (ECClassP& pClass);
    ECObjectsStatus                     SetVersionFromString (WCharCP versionString);

    typedef bvector<bpair<ECClassP, BeXmlNodeP> >  ClassDeserializationVector;
    SchemaReadStatus                    ReadClassStubsFromXml (BeXmlNodeR schemaNode, ClassDeserializationVector& classes, ECSchemaReadContextR context);
    SchemaReadStatus                    ReadClassContentsFromXml (ClassDeserializationVector&  classes, ECSchemaReadContextR context);
    SchemaReadStatus                    ReadSchemaReferencesFromXml (BeXmlNodeR schemaNode, ECSchemaReadContextR context);
    ECObjectsStatus                     AddReferencedSchema(ECSchemaR refSchema, WStringCR prefix, ECSchemaReadContextR readContext);

    struct  ECSchemaWriteContext
        {
        bset<WCharCP> m_alreadyWrittenClasses;
        };

    SchemaWriteStatus                   WriteSchemaReferences (BeXmlNodeR parentNode) const;
    SchemaWriteStatus                   WriteClass (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext&) const;
    SchemaWriteStatus                   WriteCustomAttributeDependencies (BeXmlNodeR parentNode, IECCustomAttributeContainerCR container, ECSchemaWriteContext&) const;
    SchemaWriteStatus                   WritePropertyDependencies (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext&) const;
    void                                CollectAllSchemasInGraph (bvector<EC::ECSchemaCP>& allSchemas,  bool includeRootSchema) const;
protected:
    virtual ECSchemaCP                  _GetContainerSchema() const override;

public:    
    ECOBJECTS_EXPORT static ILeakDetector& Debug_GetLeakDetector ();
    

/*__PUBLISH_SECTION_START__*/
public:    
    ECOBJECTS_EXPORT SchemaKeyCR        GetSchemaKey() const;
    ECOBJECTS_EXPORT void               DebugDump() const;
    ECOBJECTS_EXPORT static void        SetErrorHandling (bool showMessages, bool doAssert);

    //! Sets the name of this schema
    //! @param[in]  value   The name of the ECSchema
    //! @returns Success if the name passes validation and is set, ECOBJECTS_STATUS_InvalidName otherwise
    ECOBJECTS_EXPORT ECObjectsStatus    SetName(WStringCR value);
    //! Returns the name of this ECSchema
    ECOBJECTS_EXPORT WStringCR          GetName() const;    
    //! Sets the namespace prefix for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetNamespacePrefix(WStringCR value);
    //! Gets the namespace prefix for this ECSchema
    ECOBJECTS_EXPORT WStringCR          GetNamespacePrefix() const;
    //! Sets the description for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(WStringCR value);
    //! Gets the description for this ECSchema
    ECOBJECTS_EXPORT WStringCR          GetDescription() const;
    //! Sets the display label for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(WStringCR value);
    //! Gets the DisplayLabel for this ECSchema.  If no DisplayLabel has been set explicitly, returns the name of the schema
    ECOBJECTS_EXPORT WStringCR          GetDisplayLabel() const;
    //! Sets the major version of this schema
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMajor(UInt32 value);
    //! Gets the major version of this schema
    ECOBJECTS_EXPORT UInt32             GetVersionMajor() const;
    //! Sets the minor version of this schema
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMinor(UInt32 value);
    //! Gets the minor version of this schema
    ECOBJECTS_EXPORT UInt32             GetVersionMinor() const;
    //! Returns an iterable container of ECClasses sorted by name. For unsorted called overload.
    ECOBJECTS_EXPORT ECClassContainerCR GetClasses() const;
    //! Fills a vector will the ECClasses of the ECSchema in the original order in which they were added.
    ECOBJECTS_EXPORT void               GetClasses(bvector<ECClassP>& classes) const;
    //! Returns true if the display label has been set explicitly for this schema or not
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    ECOBJECTS_EXPORT bool               IsStandardSchema() const;

    //! Returns true if and only if the full schema name (including version) represents a standard schema that should never
    //! be stored persistently in a repository (we expect it to be found elsewhere)
    //@return True if this version of the schema is one that should never be imported into a repository
    ECOBJECTS_EXPORT bool               ShouldNotBeStored() const;

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
    ECOBJECTS_EXPORT ECObjectsStatus    ResolveNamespacePrefix (ECSchemaCR schema, WStringR namespacePrefix) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (WCharCP name) const;

    //! Gets the other schemas that are used by classes within this schema.
    //! Referenced schemas are the schemas that contain definitions of base classes,
    //! embedded structures, and custom attributes of classes within this schema.
    ECOBJECTS_EXPORT ECSchemaReferenceListCR GetReferencedSchemas() const;
    
    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus            AddReferencedSchema(ECSchemaR refSchema);
    
    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    //! @param[in]  prefix      The prefix to use within the context of this schema for referencing the referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus            AddReferencedSchema(ECSchemaR refSchema, WStringCR prefix);

    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveReferencedSchema(ECSchemaR refSchema);

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlString (WStringR ecSchemaXml) const;
    
    //! Serializes an ECXML schema to a file
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlFile (WCharCP ecSchemaXmlFile, bool utf16 = false);
    
    
    //! Writes an ECXML schema to an IStream
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlStream (IStreamP ecSchemaXmlStream, bool utf16 = false);
    
    
    //! Return full schema name in format GetName().MM.mm where Name is the schema name, MM is major version and mm is minor version.
    ECOBJECTS_EXPORT WString               GetFullSchemaName () const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    ECOBJECTS_EXPORT static UInt32          ComputeSchemaXmlStringCheckSum(WCharCP str, size_t len);

    //! If the given schemaName is valid, this will create a new schema object
    //! @param[out] schemaOut   if successful, will contain a new schema object
    //! @param[in]  schemaName  Name of the schema to be created.
    //! @param[in]  versionMajor The major version number.
    //! @param[in]  versionMinor The minor version number.
    //! @return A status code indicating whether the call was succesfull or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaPtr& schemaOut, WStringCR schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor);

/*__PUBLISH_SECTION_END__*/
    // This method is intended for use by internal code that needs to manage the schema's lifespan internally.
    // For example one ECXLayout schema is created for each session and never freed.  Obviously care should be
    // taken that Schema's allocated this way are not actually leaked.       
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaPtr& schemaOut, WStringCR schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor,
                                                          bool hideFromLeakDetection);


    ECOBJECTS_EXPORT void   ReComputeCheckSum ();
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
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, UInt32& versionMajor, UInt32& versionMinor, WCharCP fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, UInt32& versionMajor, UInt32& versionMinor, WStringCR fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, WCharCP versionString);
    
    //! Given two schemas, will check to see if the second schema is referenced by the first schema
    //! @param[in]    thisSchema            The base schema to check the references of
    //! @param[in]    potentiallyReferencedSchema  The schema to search for
    //! @return True if thatSchema is referenced by thisSchema, false otherwise
    ECOBJECTS_EXPORT static bool                        IsSchemaReferenced (ECSchemaCR thisSchema, ECSchemaCR potentiallyReferencedSchema);


    //! Writes an ECSchema from an ECSchemaXML-formatted file
    //! @code
    //! // The IECSchemaOwner determines the lifespan of any ECSchema objects that are created using it.
    //! // ECSchemaCache also caches ECSchemas and implements IStandaloneEnablerLocater for use by ECSchemaReadContext
    //! ECSchemaCachePtr                  schemaOwner = ECSchemaCache::Create();
    //! 
    //! // The schemaContext supplies an IECSchemaOwner to control the lifetime of read ECSchemas and a 
    //! // IStandaloneEnablerLocater to locate enablers for ECCustomAttributes in the ECSchema
    //! ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);
    //! 
    //! ECSchemaP schema;
    //! SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ecSchemaFilename, *schemaContext);
    //! if (SCHEMA_READ_STATUS_Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to write.
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlFile (ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema pathes 
    //! relative to the executing dll will be searched.
    //! @param[in]    name                The schema name to locate.
    //! @param[in]    versionMajor        The major version number of the schema to locate.
    //! @param[in]    versionMinor        The minor version number of the schema to locate.
    //! @param[in]    schemaContext       Required to create schemas
     ECOBJECTS_EXPORT static ECSchemaPtr  LocateSchema (SchemaKeyR schema, ECSchemaReadContextR schemaContext);
    
    //! 
    //! Writes an ECSchema from an ECSchemaXML-formatted string.
    //! @code
    //! // The IECSchemaOwner determines the lifespan of any ECSchema objects that are created using it.
    //! ECSchemaCachePtr                  schemaOwner = ECSchemaCache::Create();
    //! 
    //! // The schemaContext supplies an IECSchemaOwner to control the lifetime of read ECSchemas and a 
    //! 
    //! ECSchemaP schema;
    //! SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, ecSchemaAsString, *schemaContext);
    //! if (SCHEMA_READ_STATUS_Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to write
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString (ECSchemaPtr& schemaOut, WCharCP ecSchemaXml, ECSchemaReadContextR schemaContext);

    //! 
    //! Writes an ECSchema from an ECSchemaXML-formatted string.
    //! @code
    //! ECSchemaPtr schema;
    //! SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, ecSchemaAsString, *schemaOwner);
    //! if (SCHEMA_READ_STATUS_Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to write
    //! @param[in]    schemaCache         Will own the read ECSchema and referenced ECSchemas.
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString (ECSchemaPtr& schemaOut, WCharCP ecSchemaXml, ECSchemaCacheR schemaCache);

    //! Writes an ECSchema from an ECSchemaXML-formatted string in an IStream.
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to write
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlStream (ECSchemaPtr& schemaOut, IStreamP ecSchemaXmlStream, ECSchemaReadContextR schemaContext);

    //! Find all ECSchemas in the schema graph, avoiding duplicates and any cycles.
    //! @param[out]   allSchemas            Vector of schemas including rootSchema.
    //! @param[in]    rootSchema            This schema and it reference schemas will be added to the vector of allSchemas.
    //! @param[in]    includeRootSchema     If true then root schema is added to the vector of allSchemas. Defaults to true.
    ECOBJECTS_EXPORT void FindAllSchemasInGraph (bvector<EC::ECSchemaCP>& allSchemas, bool includeRootSchema=true) const;
    ECOBJECTS_EXPORT void FindAllSchemasInGraph (bvector<EC::ECSchemaP>& allSchemas, bool includeRootSchema=true);
    
    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaCP FindSchema (SchemaKeyCR schema, SchemaMatchType matchType) const;

    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaP FindSchemaP (SchemaKeyCR schema, SchemaMatchType matchType);

}; // ECSchema

END_BENTLEY_EC_NAMESPACE

//__PUBLISH_SECTION_END__
BENTLEY_ENABLE_BOOST_FOREACH_CONST_ITERATOR(Bentley::EC::ECCustomAttributeInstanceIterable)
BENTLEY_ENABLE_BOOST_FOREACH_CONST_ITERATOR(Bentley::EC::ECPropertyIterable)
BENTLEY_ENABLE_BOOST_FOREACH_CONST_ITERATOR(Bentley::EC::ECClassContainer)
//__PUBLISH_SECTION_START__
