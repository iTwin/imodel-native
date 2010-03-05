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
typedef stdext::hash_map<const wchar_t * , ECPropertyP, stdext::hash_compare<const wchar_t *, less_str>> PropertyMap;
typedef stdext::hash_map<const wchar_t * , ECClassP, stdext::hash_compare<const wchar_t *, less_str>>    ClassMap;
typedef stdext::hash_map<const wchar_t * , ECSchemaP, stdext::hash_compare<const wchar_t *, less_str>>   SchemaMap;


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

//=======================================================================================    
//! Used to represent the type of an ECProperty
struct ECTypeDescriptor
{
private:
    ValueKind       m_typeKind;

    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_primitiveType;
        };  
    ECTypeDescriptor () : m_typeKind ((ValueKind) 0), m_primitiveType ((PrimitiveType) 0) { };

public:
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveTypeDescriptor (PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructArrayTypeDescriptor ();
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructTypeDescriptor ();

    ECTypeDescriptor (PrimitiveType primitiveType) : m_typeKind (VALUEKIND_Primitive), m_primitiveType (primitiveType) { };

    inline ValueKind        GetTypeKind() const         { return m_typeKind; }
    inline ArrayKind        GetArrayKind() const        { return (ArrayKind)(m_arrayKind & 0xFF); }    
    inline bool             IsPrimitive() const         { return (GetTypeKind() == VALUEKIND_Primitive ); }
    inline bool             IsStruct() const            { return (GetTypeKind() == VALUEKIND_Struct ); }
    inline bool             IsArray() const             { return (GetTypeKind() == VALUEKIND_Array ); }
    inline bool             IsPrimitiveArray() const    { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Primitive); }
    inline bool             IsStructArray() const       { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Struct); }
    inline PrimitiveType    GetPrimitiveType() const    { return m_primitiveType; }
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


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
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


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
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


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
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


//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
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

//=======================================================================================
//! Container holding ECProperties that supports STL like iteration
struct ECPropertyContainer /*__PUBLISH_ABSTRACT__*/
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
    //=======================================================================================
    // @bsistruct
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

    //=======================================================================================
    // @bsistruct
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
typedef std::vector<ECClassP> ECConstraintClassesVector;

/*__PUBLISH_SECTION_END__*/
typedef bool (*TraversalDelegate) (ECClassCP, ECClassCP);
/*__PUBLISH_SECTION_START__*/

//=======================================================================================
//! The in-memory representation of an ECClass as defined by ECSchemaXML
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
    ECObjectsStatus                     AddProperty (ECPropertyP pProperty, std::wstring const& name);
    
    static bool ClassesAreEqualByName(ECClassCP thisClass, ECClassCP thatClass);
    static bool CheckBaseClassCycles(ECClassCP thisClass, ECClassCP proposedParentClass);
    bool TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, ECClassCP arg) const;

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
    
    //! Sets the bool value of whether this class can be used as a struct
    //! @param[in] isStruct String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsStruct (const wchar_t * isStruct);
    
    //! Sets the bool value of whether this class can be used as a custom attribte
    //! @param[in] isCustomAttribute String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsCustomAttributeClass (const wchar_t * isCustomAttribute);
    
    //! Sets the bool value of whether this class can be used as a domain object
    //! @param[in] isDomainClass String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsDomainClass (const wchar_t * isDomainClass);
    
    //! Adds a base class
    //! You cannot add a base class if it creates a cycle. For example, if A is a base class
    //! of B, and B is a base class of C, you cannot make C a base class of A. Attempting to do
    //! so will return an error.
    //! @param[in] baseClass The class to derive from
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ECClassCR baseClass);
    
    //! Returns whether there are any base classes for this class
    ECOBJECTS_EXPORT bool            HasBaseClasses() const;
    
    //! Returns true if the class is the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(ECClassCP targetClass) const;

    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, std::wstring const& name);
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, std::wstring const& name, PrimitiveType primitiveType);
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, std::wstring const& name);
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, std::wstring const& name, ECClassCR structType);
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, std::wstring const& name);
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, std::wstring const& name, PrimitiveType primitiveType);
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, std::wstring const& name, ECClassCP structType);
     
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
    
//! This class describes the cardinality of a relationship. It is based on the
//!     Martin notation. Valid cardinalities are (x,y) where x is smaller or equal to y,
//!     x >= 0 and y >= 1 or y = n (where n represents infinity).
//!     For example, (0,1), (1,1), (1,n), (0,n), (1,10), (2,5), ...
struct RelationshipCardinality 
{
/*__PUBLISH_SECTION_END__*/
private:
    UInt32     m_lowerLimit;
    UInt32     m_upperLimit;

/*__PUBLISH_SECTION_START__*/    
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
    ECOBJECTS_EXPORT bool IsUpperLimitUnbounded() const;
    
    //! Converts the cardinality to a string, for example "(0,n)", "(1,1)"
    ECOBJECTS_EXPORT std::wstring ToString() const;

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
struct ECRelationshipConstraint 
{
friend struct ECRelationshipClass;

/*__PUBLISH_SECTION_END__*/
private:
    // NEEDSWORK: To be completely compatible, we need to store an ECRelationshipConstraintClass with properties in order
    // to support implicit relationships.  For now, just support explicit relationships
//    stdext::hash_map<ECClassCP, ECRelationshipConstrainClassCP> m_constraintClasses;

    ECConstraintClassesVector        m_constraintClasses;
    
    std::wstring    m_roleLabel;
    bool            m_isPolymorphic;
    bool            m_isMultiple;
    RelationshipCardinality*   m_cardinality;
    ECRelationshipClassP        m_relClass;
    
    ECObjectsStatus SetCardinality(const wchar_t *cardinality);
    ECObjectsStatus SetCardinality(UInt32& lowerLimit, UInt32& upperLimit);
    
    SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode, std::wstring const& elementName) const;
    SchemaDeserializationStatus ReadXml(MSXML2_IXMLDOMNode& constraintNode);
    
    ~ECRelationshipConstraint();
    
/*__PUBLISH_SECTION_START__*/    
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
    EXPORTED_PROPERTY (std::wstring const, RoleLabel);
    
    ECOBJECTS_EXPORT bool IsRoleLabelDefined() const;
    
    //! Returns true if this constraint can also relate to instances of subclasses of classes
    //! applied to the constraint.
    EXPORTED_PROPERTY   (bool, IsPolymorphic) ;
    
    //! Sets the bool value of whether this constraint can also relate to instances of subclasses of classes applied to the constraint.
    //! @param[in] isPolymorphic String representation of true/false
    //! @return    Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsPolymorphic(const wchar_t* isPolymorphic);
    
    //! Gets the cardinality of the constraint in the relationship
    EXPORTED_PROPERTY (RelationshipCardinalityCR, Cardinality) ;
    
    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] classConstraint  The class to add
    ECOBJECTS_EXPORT ECObjectsStatus AddClass(ECClassCR classConstraint);
    
    //! Returns the classes applied to the constraint.
    EXPORTED_READONLY_PROPERTY (const ECConstraintClassesVector&, Classes);
    
};

//=======================================================================================
//! The in-memory representation of a relationship class as defined by ECSchemaXML
struct ECRelationshipClass /*__PUBLISH_ABSTRACT__*/ : public ECClass
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;

// NEEDSWORK  missing full implementation
private:
    StrengthType     m_strength;
    ECRelatedInstanceDirection     m_strengthDirection;
    ECRelationshipConstraintP      m_target;
    ECRelationshipConstraintP      m_source;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECRelationshipClass (ECSchemaCR schema);
    ~ECRelationshipClass ();
    
    ECObjectsStatus SetStrength(const wchar_t * strength);
    ECObjectsStatus SetStrengthDirection(const wchar_t *direction);
    
protected:
    virtual SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode) const override;

    virtual SchemaDeserializationStatus ReadXmlAttributes (MSXML2_IXMLDOMNode& classNode) override;
    virtual SchemaDeserializationStatus ReadXmlContents (MSXML2_IXMLDOMNode& classNode) override;

/*__PUBLISH_SECTION_START__*/
public:
    EXPORTED_PROPERTY (StrengthType, Strength);                
    EXPORTED_PROPERTY (ECRelatedInstanceDirection, StrengthDirection);
    //! Gets the constraint at the target end of the relationship
    EXPORTED_READONLY_PROPERTY (ECRelationshipConstraintR, Target);
    //! Gets the constraint at the source end of the relationship
    EXPORTED_READONLY_PROPERTY (ECRelationshipConstraintR, Source);
    EXPORTED_READONLY_PROPERTY (bool, IsExplicit);

}; // ECRelationshipClass

typedef std::list<ECSchemaP> ECSchemaReferenceList;
typedef RefCountedPtr<ECSchema>                  ECSchemaPtr;
//=======================================================================================
//! Supports STL like iterator of classes in a schema
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
//! Interface implemented by class that provides schema location services.</summary>
struct IECSchemaLocator
{
public:
    virtual ECOBJECTS_EXPORT ECSchemaPtr LocateSchema(const wchar_t *name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, void * schemaContext) const = 0;
};

//! The in-memory representation of a schema as defined by ECSchemaXML
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
    
    ECSchemaReferenceList m_refSchemaList;
    
    std::set<const wchar_t *> m_alreadySerializedClasses;
    stdext::hash_map<ECSchemaP, const std::wstring *> m_referencedSchemaNamespaceMap;

    // Hide these as part of the RefCounted pattern    
    ECSchema () : m_versionMajor (DEFAULT_VERSION_MAJOR), m_versionMinor (DEFAULT_VERSION_MINOR), m_classContainer(ECClassContainer(m_classMap)) {};
    ~ECSchema();    

    static SchemaDeserializationStatus  ReadXml (ECSchemaPtr& schemaOut, MSXML2_IXMLDOMDocument2& pXmlDoc, const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, void * schemaContext);
    SchemaSerializationStatus           WriteXml (MSXML2_IXMLDOMDocument2* pXmlDoc);

    ECObjectsStatus                     AddClass (ECClassP& pClass);
    ECObjectsStatus                     SetVersionFromString (std::wstring const& versionString);

    typedef std::vector<std::pair<ECClassP, MSXML2_IXMLDOMNodePtr>>  ClassDeserializationVector;
    SchemaDeserializationStatus         ReadClassStubsFromXml(MSXML2_IXMLDOMNode& schemaNodePtr,ClassDeserializationVector& classes);
    SchemaDeserializationStatus         ReadClassContentsFromXml(ClassDeserializationVector&  classes);
    SchemaDeserializationStatus         ReadSchemaReferencesFromXml(MSXML2_IXMLDOMNode& schemaNodePtr, const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, void * schemaContext);
    ECSchemaPtr                         LocateSchema(const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, const std::wstring & name, UInt32& versionMajor, UInt32& versionMinor, SchemaMap * schemasUnderConstruction);
    ECSchemaPtr                         LocateSchemaByPath(const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, const std::wstring & name, UInt32& versionMajor, UInt32& versionMinor, SchemaMap * schemasUnderConstruction);
    
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
    ECOBJECTS_EXPORT const ECSchemaReferenceList& GetReferencedSchemas() const;
    
    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus AddReferencedSchema(ECSchemaCR refSchema);
    
    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus RemoveReferencedSchema(ECSchemaCR refSchema);

    //! Serializes an ECXML schema to a string
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus          WriteXmlToString (std::wstring & ecSchemaXml);
    
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
    
    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaPtr& schemaOut, std::wstring const& schemaName);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, std::wstring const& versionString);
    ECOBJECTS_EXPORT static bool SchemasMatch (SchemaMatchType matchType,
                          const wchar_t * soughtName,    UInt32 soughtMajor,    UInt32 soughtMinor,
                          const wchar_t * candidateName, UInt32 candidateMajor, UInt32 candidateMinor);


    //! Deserializes an ECXML schema from a file.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to deserialize.
    //! @param[in]    schemaLocators      A list of IECSchemaLocatorP that will be used to locate referenced schemas
    //! @param[in]    schemaPaths         A list of paths that should be searched to locate referenced schemas
    //! @param[in]    schemaContext       Usually NULL, but when used it is usually a pointer to a SchemaMap used to locate referenced schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromFile (ECSchemaPtr& schemaOut, const wchar_t * ecSchemaXmlFile, const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, void * schemaContext = NULL);

    //! Deserializes an ECXML schema from a string.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to deserialize
    //! @param[in]    schemaLocators      A list of IECSchemaLocatorP that will be used to locate referenced schemas
    //! @param[in]    schemaPaths         A list of paths that should be searched to locate referenced schemas
    //! @param[in]    schemaContext       Usually NULL, but when used it is usually a pointer to a SchemaMap used to locate referenced schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromString (ECSchemaPtr& schemaOut, const wchar_t * ecSchemaXml, const std::vector<IECSchemaLocatorP> * schemaLocators, const std::vector<const wchar_t *> * schemaPaths, void * schemaContext = NULL);

/**************** commented out because there are problems with the include for IStream

    //! Deserializes an ECXML schema from an IStream.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to deserialize
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    //ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromStream (ECSchemaPtr& schemaOut, IStream * ecSchemaXmlStream);
*/
    
}; // ECSchema

END_BENTLEY_EC_NAMESPACE

#undef MSXML2_IXMLDOMNode
#undef MSXML2_IXMLDOMNodePtr
#undef MSXML2_IXMLDOMDocument2
#undef MSXML2_IXMLDOMElementPtr
