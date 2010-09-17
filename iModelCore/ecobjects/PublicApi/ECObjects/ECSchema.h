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
#include <Bentley\bvector.h>
#include <Bentley\bmap.h>

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

struct NameValidator abstract
{
public:
    static bool Validate(const bwstring& name);
};
    
typedef std::list<ECPropertyP> PropertyList;
typedef bmap<const wchar_t * , ECPropertyP, stdext::hash_compare<const wchar_t *, less_str>> PropertyMap;
typedef stdext::hash_map<const wchar_t * , ECClassP, stdext::hash_compare<const wchar_t *, less_str>>    ClassMap;
typedef stdext::hash_map<const wchar_t * , ECSchemaP, stdext::hash_compare<const wchar_t *, less_str>>   SchemaMap;

//=======================================================================================    
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ValueKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
//=======================================================================================    
/*__PUBLISH_SECTION_START__*/
//=======================================================================================    
//! Represents the classification of the data type of an EC ECValue.  The classification is not the data type itself, but a category of type
//! such as struct, array or primitive.
//=======================================================================================    
enum ValueKind : unsigned short
    {
    VALUEKIND_Uninitialized                  = 0x00,
    VALUEKIND_Primitive                      = 0x01,
    VALUEKIND_Struct                         = 0x02,
    VALUEKIND_Array                          = 0x04
    };

/*__PUBLISH_SECTION_END__*/
//=======================================================================================    
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ArrayKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
//=======================================================================================    
/*__PUBLISH_SECTION_START__*/
//=======================================================================================    
//! Represents the classification of the data type of an EC array element.  The classification is not the data type itself, but a category of type.
//! Currently an ECArray can only contain primitive or struct data types.
//=======================================================================================    
enum ArrayKind : unsigned short
    {
    ARRAYKIND_Primitive       = 0x01,
    ARRAYKIND_Struct          = 0x02
    };

/*__PUBLISH_SECTION_END__*/
//=======================================================================================    
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the PrimitiveType enum please be sure to note that the lower order byte must stay fixed as '1' and the upper order byte can be incremented.
// If you add any additional types you must update 
//    - ECXML_TYPENAME_X constants
//    - PrimitiveECProperty::_GetTypeName
// NEEDSWORK types: common geometry, installed primitives
//=======================================================================================    
/*__PUBLISH_SECTION_START__*/

typedef bmap<WString, IECInstalledTypeValueP> NameECInstalledTypeHandlerMap;

struct ECInstalledTypeHandlerMgr
{
NameECInstalledTypeHandlerMap   m_installTypeHandlers;

ECInstalledTypeHandlerMgr();

static ECInstalledTypeHandlerMgrR GetManager();
IECInstalledTypeValueP          GetIECInstalledTypeValue (wchar_t const* typeName) const;
ECObjectsStatus                   AddIECInstalledTypeValue (wchar_t const* typeName, IECInstalledTypeValueP typeHandler);
};

struct IECInstalledTypeValue
{
virtual byte const *               GetBytePointer(UInt32& numBytes)=0;
virtual ECObjectsStatus            LoadFromByteData (byte const * byteBuffer)=0;
virtual WString                    GetStringValue () const=0;
virtual ECObjectsStatus            LoadFromStringValue (wchar_t const* stringValue)=0;
virtual IECInstalledTypeValueP     Clone ()=0;
virtual wchar_t const*             GetInstalledTypeName () const=0;
};

struct DgnColorValue : IECInstalledTypeValue
{
UInt8*      m_data;

UInt8       m_colorSource;  // color source. If m_colorSource is ColorByLevel, color is the level color.
UInt8       m_colorType;    // color type. Valid only if ColorSource is Element.
UInt8       m_colorIndex;   // 0-255, only valid if m_colorType == Indexed
Int8        m_red;
Int8        m_green;
Int8        m_blue;
WString     m_colorBook;    // valid only if m_colorType == ColorBook.
WString     m_colorName;    // valid only if m_colorType == ColorBook.
WString     m_typeName;

ECOBJECTS_EXPORT DgnColorValue ();
ECOBJECTS_EXPORT ~DgnColorValue ();
virtual byte const*                GetBytePointer(UInt32& numBytes);
virtual ECObjectsStatus            LoadFromByteData (byte const * byteBuffer);
virtual WString                    GetStringValue () const;
virtual ECObjectsStatus            LoadFromStringValue (wchar_t const* stringValue);
virtual IECInstalledTypeValueP     Clone ();
virtual wchar_t const*             GetInstalledTypeName ()const;
ECOBJECTS_EXPORT static  wchar_t const*             GetTypeName() {return L"ColorType";}
void                               ClearData ();
};

struct GradientKeyColor
{
double location; 
Int8   red;
Int8   green;
Int8   blue;
};

struct DgnFillColorValue : public DgnColorValue
{
UInt8*              m_data;

double              m_gradientAngle;    // Set by angle control.
double              m_tint;             // used only when there is just one color.
double              m_shift;            // I think this is adjusted using the "Center" toggle.
UInt32              m_gradientFlags;    // see flags above.
UInt32              m_gradientMode;     // one of the GradientMode enumeration (but should never be GradientModeNone).
int                 m_activeColorKeys;  // the number of color keys in the gradient.
GradientKeyColor*   m_keys;

ECOBJECTS_EXPORT DgnFillColorValue ();
ECOBJECTS_EXPORT ~DgnFillColorValue ();

virtual byte const*                GetBytePointer(UInt32& numBytes);
virtual ECObjectsStatus            LoadFromByteData (byte const * byteBuffer);
virtual WString                    GetStringValue () const;
virtual ECObjectsStatus            LoadFromStringValue (wchar_t const* stringValue);
virtual IECInstalledTypeValueP     Clone ();
virtual wchar_t const*             GetInstalledTypeName ()const;
ECOBJECTS_EXPORT static  wchar_t const*             GetTypeName () {return L"FillColorType";}
void                               ClearData ();
};


//=======================================================================================    
//! Enumeration of primitive datatypes supported by native "ECObjects" implementation.
//! These should correspond to all of the datatypes supported in .NET ECObjects
//=======================================================================================    
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
    PRIMITIVETYPE_String                    = 0x901,
    PRIMITIVETYPE_Installed                 = 0xA01
    };

//=======================================================================================    
//! Used to represent the type of an ECProperty
//=======================================================================================
struct ECTypeDescriptor
{
private:
    ValueKind       m_typeKind;
    WString         m_installedTypeName;

    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_primitiveType;
        };      

public:
    ECOBJECTS_EXPORT ECTypeDescriptor& operator= (ECTypeDescriptor const & rhs);

    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveTypeDescriptor (PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType);
/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateInstalledPrimitiveTypeDescriptor (wchar_t const* installedTypeName);
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateInstalledPrimitiveArrayTypeDescriptor (wchar_t const* installedTypeName); 
/*__PUBLISH_SECTION_START__*/
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructArrayTypeDescriptor ();
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructTypeDescriptor ();

    ECTypeDescriptor (PrimitiveType primitiveType) : m_typeKind (VALUEKIND_Primitive), m_primitiveType (primitiveType), m_installedTypeName (WString()) { };

/*__PUBLISH_SECTION_END__*/
    ECTypeDescriptor () : m_typeKind ((ValueKind) 0), m_primitiveType ((PrimitiveType) 0), m_installedTypeName (WString()) { };
    ECTypeDescriptor (ValueKind valueKind, short valueKindQualifier) : m_typeKind (valueKind), m_primitiveType ((PrimitiveType)valueKindQualifier), m_installedTypeName (WString()) { };
/*__PUBLISH_SECTION_START__*/    

    inline ValueKind            GetTypeKind() const         { return m_typeKind; }
    inline ArrayKind            GetArrayKind() const        { return (ArrayKind)(m_arrayKind & 0xFF); }    
    inline bool                 IsPrimitive() const         { return (GetTypeKind() == VALUEKIND_Primitive ); }
    inline bool                 IsStruct() const            { return (GetTypeKind() == VALUEKIND_Struct ); }
    inline bool                 IsArray() const             { return (GetTypeKind() == VALUEKIND_Array ); }
    inline bool                 IsInstalledPrimitive() const { return (GetTypeKind() == VALUEKIND_Primitive && GetPrimitiveType() == PRIMITIVETYPE_Installed ); }
    inline bool                 IsPrimitiveArray() const    { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Primitive); }
    inline bool                 IsStructArray() const       { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Struct); }
    inline PrimitiveType        GetPrimitiveType() const    { return m_primitiveType; }
/*__PUBLISH_SECTION_END__*/
    wchar_t const  *            GetInstalledPrimitiveTypeName () {return m_installedTypeName.c_str();}
    IECInstalledTypeValueP      GetInstalledPrimitiveTypeValue () const;
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

typedef std::list<IECInstancePtr> ECCustomAttributeCollection;
struct ECCustomAttributeInstanceIterable;

//=======================================================================================
//
//=======================================================================================
struct IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECCustomAttributeInstanceIterable;
    ECCustomAttributeCollection         m_customAttributes;
    SchemaSerializationStatus           AddCustomAttributeProperties(MSXML2_IXMLDOMNode& oldNode, MSXML2_IXMLDOMNode& newNode) const;

protected:
    InstanceDeserializationStatus       ReadCustomAttributes(MSXML2_IXMLDOMNode& containerNode, ECSchemaCR schema);
    SchemaSerializationStatus           WriteCustomAttributes(MSXML2_IXMLDOMNode& parentNode) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
/*__PUBLISH_SECTION_START__*/
protected:
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP			_GetContainerSchema() const {return NULL;};

public:
    ECOBJECTS_EXPORT ~IECCustomAttributeContainer();

    //! Returns true if the conainer has a custom attribute of a class of the specified name
    ECOBJECTS_EXPORT bool               IsDefined(bwstring const& className) ;
    //! Returns true if the conainer has a custom attribute of a class of the specified class definition
    ECOBJECTS_EXPORT bool               IsDefined(ECClassCR classDefinition) ;

    //! Retrieves the custom attribute matching the class name.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(bwstring const& className) const;
    //! Retrieves the custom attribute matching the class definition.  Includes looking on base containers
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(ECClassCR classDefinition) const;
    //! Retrieves all custom attributes from the container
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers 
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetCustomAttributes(bool includeBase) const; 

    ECOBJECTS_EXPORT ECObjectsStatus    SetCustomAttribute(IECInstanceR customAttributeInstance);
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(bwstring const& className);
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
    bwstring    m_name;        
    bwstring    m_displayLabel;
    bwstring    m_description;
    bool            m_readOnly;
    ECClassCR       m_class;
    ECPropertyCP     m_baseProperty;    

protected:
    ECProperty (ECClassCR ecClass) : m_class(ecClass), m_readOnly(false), m_baseProperty(NULL) {};        
    ECObjectsStatus                     SetName (bwstring const& name);    

    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode);
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode);
    SchemaSerializationStatus           _WriteXml(MSXML2_IXMLDOMElement& parentNode, const wchar_t *elementName);

    virtual bool                        _IsPrimitive () const { return false; }
    virtual bool                        _IsStruct () const { return false; }
    virtual bool                        _IsArray () const { return false; }
    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual bwstring                    _GetTypeName () const abstract;
    virtual ECObjectsStatus             _SetTypeName (bwstring const& typeName) abstract;

    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const abstract;

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP			_GetContainerSchema() const override;

    virtual PrimitiveECProperty*        _GetAsPrimitiveECProperty() {return NULL;}
    
/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_READONLY_PROPERTY (ECClassCR,              Class);   
    // ECClass implementation will index property by name so publicly name can not be reset
    EXPORTED_READONLY_PROPERTY (bwstring const&,    Name);        
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
    EXPORTED_PROPERTY  (bwstring,           TypeName);        
    EXPORTED_PROPERTY  (bwstring const&,    Description);
    EXPORTED_PROPERTY  (bwstring const&,    DisplayLabel);    
    EXPORTED_PROPERTY  (bool,                   IsReadOnly);
    EXPORTED_PROPERTY  (ECPropertyCP,           BaseProperty);    

    ECOBJECTS_EXPORT ECObjectsStatus            SetIsReadOnly (const wchar_t * isReadOnly);

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
    WString         m_installedTypeName;      // convert to pointer, allocate, and add destructor to free ?

    PrimitiveECProperty (ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), ECProperty(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual bwstring                    _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (bwstring const& typeName) override;
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

    StructECProperty (ECClassCR ecClass) : m_structType(NULL), ECProperty(ecClass) {};

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual bwstring                _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (bwstring const& typeName) override;
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
    WString             m_installedTypeName;      // only used if m_primitiveType = PRIMITIVETYPE_Installed

    union
        {
        PrimitiveType   m_primitiveType;
        ECClassCP       m_structType;
        };

    ArrayKind           m_arrayKind;
      
    ArrayECProperty (ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), m_arrayKind (ARRAYKIND_Primitive),
        m_minOccurs (0), m_maxOccurs (UINT_MAX), ECProperty(ecClass) {};
    ECObjectsStatus SetMinOccurs (bwstring const& minOccurs);          
    ECObjectsStatus SetMaxOccurs (bwstring const& maxOccurs);          

protected:
    virtual SchemaDeserializationStatus _ReadXml (MSXML2_IXMLDOMNode& propertyNode) override;
    virtual SchemaSerializationStatus   _WriteXml(MSXML2_IXMLDOMElement& parentNode) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual bwstring                    _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (bwstring const& typeName) override;
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

typedef std::list<ECClassP> ECBaseClassesList;
typedef std::list<ECClassP> ECDerivedClassesList;
typedef std::list<ECClassP> ECConstraintClassesList;

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
    bwstring                        m_name;
    bwstring                        m_displayLabel;
    bwstring                        m_description;
    bool                            m_isStruct;
    bool                            m_isCustomAttributeClass;
    bool                            m_isDomainClass;
    ECSchemaCR                      m_schema;
    ECBaseClassesList               m_baseClasses;
    mutable ECDerivedClassesList    m_derivedClasses;

    PropertyMap                     m_propertyMap;
    PropertyList                    m_propertyList;    
    
    ECObjectsStatus                 AddProperty (ECPropertyP& pProperty);
    ECObjectsStatus                 AddProperty (ECPropertyP pProperty, bwstring const& name);
    
    static bool CheckBaseClassCycles(ECClassCP currentBaseClass, const void * arg);
    static bool AddUniquePropertiesToList(ECClassCP crrentBaseClass, const void * arg);
    bool TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, const void * arg) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetProperties(bool includeBaseProperties, PropertyList* propertyList) const;

    ECObjectsStatus CanPropertyBeOverridden(ECPropertyCR baseProperty, ECPropertyCR newProperty) const;
    void            AddDerivedClass(ECClassCR baseClass) const;
    void            RemoveDerivedClass(ECClassCR baseClass) const;

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECSchemaCR schema) : m_schema(schema), m_isStruct(false), m_isCustomAttributeClass(false), m_isDomainClass(true){ };
    ~ECClass();    

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
	virtual ECSchemaCP					_GetContainerSchema() const override;

    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus                     SetName (bwstring const& name);    

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
    EXPORTED_READONLY_PROPERTY (bwstring const&,    Name);        
    EXPORTED_READONLY_PROPERTY (bool,                   IsDisplayLabelDefined);
    EXPORTED_READONLY_PROPERTY (ECPropertyIterable,  Properties); 
    EXPORTED_READONLY_PROPERTY (const ECBaseClassesList&,     BaseClasses);   
    EXPORTED_READONLY_PROPERTY (const ECDerivedClassesList&,  DerivedClasses);   

    EXPORTED_PROPERTY  (bwstring const&,            Description);
    EXPORTED_PROPERTY  (bwstring const&,            DisplayLabel);
    EXPORTED_PROPERTY  (bool,                           IsStruct);    
    EXPORTED_PROPERTY  (bool,                           IsCustomAttributeClass);    
    EXPORTED_PROPERTY  (bool,                           IsDomainClass);    
    
    //! Returns a list of properties for this class.
    //! @param[in]  includeBaseProperties If true, then will return properties that are contained in this class's base class(es)
    //! @return     An iterable container of ECProperties
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties(bool includeBaseProperties) const;

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
    
    //! Removes a base class.
    ECOBJECTS_EXPORT ECObjectsStatus RemoveBaseClass(ECClassCR baseClass);
    
    //! Returns true if the class is the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(ECClassCP targetClass) const;

    //! If the given name is valid, creates a primitive property object with the default type of STRING
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, bwstring const& name);

    //! If the given name is valid, creates a primitive property object with the given primitive type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, bwstring const& name, PrimitiveType primitiveType);

    //! If the given name is valid, creates a struct property object using the current class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, bwstring const& name);

    //! If the given name is valid, creates a struct property object using the specified class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, bwstring const& name, ECClassCR structType);

    //! If the given name is valid, creates an array property object using the current class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, bwstring const& name);

    //! If the given name is valid, creates an array property object using the specified primitive type as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, bwstring const& name, PrimitiveType primitiveType);

    //! If the given name is valid, creates an array property object using the specified class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, bwstring const& name, ECClassCP structType);
    
    //! Remove the named property
    //! @param[in] name The name of the property to be removed
    ECOBJECTS_EXPORT ECObjectsStatus RemoveProperty(bwstring const& name);
     
    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @return   A pointer to an EC::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (wchar_t const* name) const;

    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (bwstring const& name) const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! Given a qualified class name, will parse out the schema's namespace prefix and the class name.
    //! @param[out] prefix  The namespace prefix of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedClassName  The qualified name of the class, in the format of ns:className
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (bwstring & prefix, bwstring & className, bwstring const& qualifiedClassName);

    //! Given a schema and a class, will return the fully qualified class name.  If the class is part of the passed in schema, there
    //! is no namespace prefix.  Otherwise, the class's schema must be a referenced schema in the passed in schema
    //! @param[in]  primarySchema   The schema used to lookup the namespace prefix of the class's schema
    //! @param[in]  ecClass         The class whose schema should be searched for
    //! @return bwstring    The namespace prefix if the class's schema is not the primarySchema
    ECOBJECTS_EXPORT static bwstring GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass);

    //! Given two ECClass's, checks to see if they are equal by name
    //! @param[in]  currentBaseClass    The source class to check against
    //! @param[in]  arg                 The target to compare to (this parameter must be an ECClassP)
    ECOBJECTS_EXPORT static bool ClassesAreEqualByName(ECClassCP currentBaseClass, const void * arg);

   
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
    ECOBJECTS_EXPORT bool IsUpperLimitUnbounded() const;
    
    //! Converts the cardinality to a string, for example "(0,n)", "(1,1)"
    ECOBJECTS_EXPORT bwstring ToString() const;

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

    ECConstraintClassesList        m_constraintClasses;
    
    bwstring                    m_roleLabel;
    bool                        m_isPolymorphic;
    bool                        m_isMultiple;
    RelationshipCardinality*    m_cardinality;
    ECRelationshipClassP        m_relClass;
    
    ECObjectsStatus SetCardinality(const wchar_t *cardinality);
    ECObjectsStatus SetCardinality(UInt32& lowerLimit, UInt32& upperLimit);
   
    SchemaSerializationStatus   WriteXml(MSXML2_IXMLDOMElement& parentNode, bwstring const& elementName) const;
    SchemaDeserializationStatus ReadXml(MSXML2_IXMLDOMNode& constraintNode);
    
    ~ECRelationshipConstraint();
    
protected:
    virtual ECSchemaCP					_GetContainerSchema() const override;
  
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
    EXPORTED_PROPERTY (bwstring const, RoleLabel);
    
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

typedef RefCountedPtr<ECRelationshipClass>      ECRelationshipClassPtr;

typedef std::list<ECSchemaP> ECSchemaReferenceList;
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
    virtual ECSchemaP       _GetSchema   (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor) = 0;
    virtual ECSchemaP       _LocateSchema (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType) = 0;

public:
    ECObjectsStatus         AddSchema   (ECSchemaR);
    ECObjectsStatus         DropSchema  (ECSchemaR);
    ECSchemaP               GetSchema   (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor);
    ECSchemaP               LocateSchema (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType);

/*__PUBLISH_SECTION_START__*/
};

typedef RefCountedPtr<ECSchemaOwner>        ECSchemaOwnerPtr;
//=======================================================================================
//! An object that controls the lifetime of a set of ECSchemas.  When the schema
//! owner is destroyed, so are the schemas that it owns.</summary>
//=======================================================================================
struct ECSchemaOwner /*__PUBLISH_ABSTRACT__*/ : RefCountedBase, IECSchemaOwner
{
/*__PUBLISH_SECTION_END__*/
private:
    bvector<ECSchemaP> m_schemas;

    ~ECSchemaOwner();

protected:
    virtual ECObjectsStatus _AddSchema   (ECSchemaR) override;
    virtual ECObjectsStatus _DropSchema  (ECSchemaR) override;
    virtual ECSchemaP       _GetSchema   (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor);
    virtual ECSchemaP       _LocateSchema (const wchar_t* schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType);

/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT static  ECSchemaOwnerPtr    CreateOwner();
};

typedef RefCountedPtr<ECSchemaDeserializationContext>      ECSchemaDeserializationContextPtr;
//=======================================================================================
//! Context object used for schema creation and deserialization.</summary>
//=======================================================================================
struct ECSchemaDeserializationContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
friend  ECSchema;

private:
    IECSchemaOwnerR                 m_schemaOwner;

    bvector<IECSchemaLocatorP>      m_locators;
    bvector<const wchar_t *>        m_searchPaths;
    bool                            m_hideSchemasFromLeakDetection;

    ECSchemaDeserializationContext(IECSchemaOwnerR);

    bvector<IECSchemaLocatorP>& GetSchemaLocators ();
    bvector<const wchar_t *>&   GetSchemaPaths ();
    IECSchemaOwnerR             GetSchemaOwner();
    bool                        GetHideSchemasFromLeakDetection();

    void                        ClearSchemaPaths();

public:
    ECOBJECTS_EXPORT void HideSchemasFromLeakDetection();

/*__PUBLISH_SECTION_START__*/
    ECOBJECTS_EXPORT static ECSchemaDeserializationContextPtr CreateContext (IECSchemaOwnerR);

    ECOBJECTS_EXPORT void AddSchemaLocators (bvector<EC::IECSchemaLocatorP>&);

    ECOBJECTS_EXPORT void AddSchemaLocator (IECSchemaLocatorR);
    ECOBJECTS_EXPORT void AddSchemaPath (const wchar_t *);
};

//=======================================================================================
//! Interface implemented by class that provides schema location services.</summary>
//=======================================================================================
struct IECSchemaLocator
{
public:
    virtual ECOBJECTS_EXPORT ECSchemaP LocateSchema(const wchar_t *name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaDeserializationContextR schemaContext) const = 0;
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
    bwstring            m_name;
    bwstring            m_namespacePrefix;
    bwstring            m_displayLabel;
    bwstring            m_description;
    UInt32              m_versionMajor;
    UInt32              m_versionMinor;    
    ECClassContainer    m_classContainer;
    bool                m_hideFromLeakDetection;

    // maps class name -> class pointer    
    ClassMap m_classMap;
    
    ECSchemaReferenceList m_refSchemaList;
    
    std::set<const wchar_t *> m_alreadySerializedClasses;
    stdext::hash_map<ECSchemaP, const bwstring> m_referencedSchemaNamespaceMap;

    ECSchema (bool hideFromLeakDetection);
    ~ECSchema();    

    static SchemaDeserializationStatus  ReadXml (ECSchemaP& schemaOut, MSXML2_IXMLDOMDocument2& pXmlDoc, ECSchemaDeserializationContextR context);
    SchemaSerializationStatus           WriteXml (MSXML2_IXMLDOMDocument2* pXmlDoc);

    ECObjectsStatus                     AddClass (ECClassP& pClass);
    ECObjectsStatus                     SetVersionFromString (bwstring const& versionString);

    typedef bvector<std::pair<ECClassP, MSXML2_IXMLDOMNodePtr>>  ClassDeserializationVector;
    SchemaDeserializationStatus         ReadClassStubsFromXml(MSXML2_IXMLDOMNode& schemaNodePtr,ClassDeserializationVector& classes);
    SchemaDeserializationStatus         ReadClassContentsFromXml(ClassDeserializationVector&  classes);
    SchemaDeserializationStatus         ReadSchemaReferencesFromXml(MSXML2_IXMLDOMNode& schemaNodePtr, ECSchemaDeserializationContextR context);
    static ECSchemaP                    LocateSchemaByPath(const bwstring & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR context);
    static ECSchemaP                    LocateSchemaByStandardPaths(const bwstring & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR context);
    
    SchemaSerializationStatus           WriteSchemaReferences(MSXML2_IXMLDOMElement& parentNode);
    SchemaSerializationStatus           WriteClass(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass);
    SchemaSerializationStatus           WriteCustomAttributeDependencies(MSXML2_IXMLDOMElement& parentNode, IECCustomAttributeContainerCR container);
    SchemaSerializationStatus           WritePropertyDependencies(MSXML2_IXMLDOMElement& parentNode, ECClassCR ecClass);

protected:
	virtual ECSchemaCP					_GetContainerSchema() const override;

public:    
/*__PUBLISH_SECTION_START__*/
// WIP_FUSION: temporarily published this.  We need an ecObjectsNativeTest version of the BackDoor.
    ECOBJECTS_EXPORT static void        Debug_ResetAllocationStats ();
    ECOBJECTS_EXPORT static void        Debug_DumpAllocationStats (const wchar_t* prefix);
    ECOBJECTS_EXPORT static void        Debug_GetAllocationStats (int* currentLive, int* totalAllocs, int* totalFrees);
    ECOBJECTS_EXPORT static void        Debug_ReportLeaks ();
/*__PUBLISH_SECTION_END__*/

/*__PUBLISH_SECTION_START__*/
public:    
    EXPORTED_PROPERTY (bwstring const&, Name);    
    EXPORTED_PROPERTY (bwstring const&, NamespacePrefix);
    EXPORTED_PROPERTY (bwstring const&, Description);
    EXPORTED_PROPERTY (bwstring const&, DisplayLabel);
    EXPORTED_PROPERTY (UInt32,              VersionMajor);
    EXPORTED_PROPERTY (UInt32,              VersionMinor);

    EXPORTED_READONLY_PROPERTY (ECClassContainerCR, Classes);
    EXPORTED_READONLY_PROPERTY (bool,               IsDisplayLabelDefined);

    //! If the class name is valid, will create an ECClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateClass (ECClassP& ecClass, bwstring const& name);

    //! If the class name is valid, will create an ECRelationshipClass object and add the new class to the schema
    //! @param[out] relationshipClass If successful, will contain a new ECRelationshipClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateRelationshipClass (ECRelationshipClassP& relationshipClass, bwstring const& name);

    //! Get a schema by namespace prefix within the context of this schema and its referenced schemas.
    //! @param[in]  namespacePrefix     The prefix of the schema to lookup in the context of this schema and it's references.
    //!                                 Passing an empty namespacePrefix will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an EC::ECSchema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaP          GetSchemaByNamespacePrefixP(bwstring const& namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup a namespace prefix in the context of this schema and its references.
    //! @param[out] namespacePrefix The namespace prefix if schema is a referenced schema; empty string if the sechema is the current schema;    
    //! @return   Success if the schema is either the current schema or a referenced schema;  ECOBJECTS_STATUS_SchemaNotFound if the schema is not found in the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus ResolveNamespacePrefix(ECSchemaCR schema, bwstring & namespacePrefix) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.    
    //! @return   A pointer to an EC::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (const wchar_t * name) const;

    //! Gets the other schemas that are used by classes within this schema.
    //! Referenced schemas are the schemas that contain definitions of base classes,
    //! embedded structures, and custom attributes of classes within this schema.
    ECOBJECTS_EXPORT const ECSchemaReferenceList& GetReferencedSchemas() const;
    
    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus AddReferencedSchema(ECSchemaR refSchema);
    
    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus RemoveReferencedSchema(ECSchemaR refSchema);

    //! Serializes an ECXML schema to a string
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus          WriteXmlToString (bwstring & ecSchemaXml);
    
    //! Serializes an ECXML schema to a file
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaSerializationStatus          WriteXmlToFile (const wchar_t * ecSchemaXmlFile);
    
    
    //! Serializes an ECXML schema to an IStream
    //! Xml Serialization utilizes MSXML through COM. <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    ECOBJECTS_EXPORT SchemaSerializationStatus WriteXmlToStream (IStreamP ecSchemaXmlStream);
    
    
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
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaP& schemaOut, bwstring const& schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR owner);

/*__PUBLISH_SECTION_END__*/
    // Should only be called by SchemaOwners.  Since IECSchemaOwner is not published, neither should this method be published
    ECOBJECTS_EXPORT static void            DestroySchema (ECSchemaP& schema);

    // This method is intended for use by internal code that needs to manage the schema's lifespan internally.
    // For example one ECXLayout schema is created for each session and never freed.  Obviously care should be
    // taken that Schema's allocated this way are not actually leaked.       
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaP& schemaOut, bwstring const& schemaName, 
                                                          UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR owner,
                                                          bool hideFromLeakDetection);
/*__PUBLISH_SECTION_START__*/

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out]  versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed

    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, bwstring const& versionString);
    
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
                          const wchar_t * soughtName,    UInt32 soughtMajor,    UInt32 soughtMinor,
                          const wchar_t * candidateName, UInt32 candidateMajor, UInt32 candidateMinor);

    //! Given two schemas, will check to see if the second schema is referenced by the first schema
    //! @param[in]    thisSchema            The base schema to check the references of
    //! @param[in]    thatSchema            The schema to search for
    //! @return True if thatSchema is referenced by thisSchema, false otherwise
    ECOBJECTS_EXPORT static bool IsSchemaReferenced (ECSchemaCR thisSchema, ECSchemaCR thatSchema);

    //! Compare two schemas and returns true if the schema pointers are equal or the names and version of the schemas are the same 
    //! @param[out]   thisSchema           Pointer to schema
    //! @param[out]   thatSchema           Pointer to schema
    ECOBJECTS_EXPORT static bool SchemasAreEqualByName (ECSchemaCP thisSchema, ECSchemaCP thatSchema);

    //! Deserializes an ECXML schema from a file.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to deserialize.
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromFile (ECSchemaP& schemaOut, const wchar_t * ecSchemaXmlFile, ECSchemaDeserializationContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema pathes 
    //! relative to the executing dll will be searched.
    //! @param[in]    name                The schema name to locate.
    //! @param[in]    versionMajor        The major version number of the schema to locate.
    //! @param[in]    versionMinor        The minor version number of the schema to locate.
    //! @param[in]    schemaContext       Required to create schemas
    ECOBJECTS_EXPORT static ECSchemaP                   LocateSchema(const bwstring & name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaDeserializationContextR schemaContext);

    //! Deserializes an ECXML schema from a string.
    //! XML Deserialization utilizes MSXML through COM.  <b>Any thread calling this method must therefore be certain to initialize and
    //! uninitialize COM using CoInitialize/CoUninitialize</b>
    //! @param[out]   schemaOut           The deserialized schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to deserialize
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully deserialized.  If SUCCESS is returned then schemaOut will
    //!           contain the deserialized schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaDeserializationStatus ReadXmlFromString (ECSchemaP& schemaOut, const wchar_t * ecSchemaXml, ECSchemaDeserializationContextR schemaContext);

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
