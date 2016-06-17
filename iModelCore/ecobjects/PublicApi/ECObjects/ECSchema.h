/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/CalculatedProperty.h>
/*__PUBLISH_SECTION_END__*/
#include <ECObjects/SchemaLocalizedStrings.h>
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECEnabler.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeId.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/bset.h>
#include <Bentley/BeFileName.h>

#define DEFAULT_VERSION_MAJOR       1
#define DEFAULT_VERSION_WRITE      0
#define DEFAULT_VERSION_MINOR       0

EC_TYPEDEFS(QualifiedECAccessor);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
* Comparison function that is used within various schema related data structures
* for string comparison in STL collections.
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct less_str
{
bool operator()(Utf8CP s1, Utf8CP s2) const
    {
    if (BeStringUtilities::StricmpAscii(s1, s2) < 0)
        return true;

    return false;
    }
};

typedef bvector<ECPropertyP>                    PropertyList;
typedef bmap<Utf8CP, ECPropertyP,    less_str>  PropertyMap;
typedef bmap<Utf8CP, ECClassP,       less_str>  ClassMap;
typedef bmap<Utf8CP, ECEnumerationP, less_str>  EnumerationMap;
typedef bvector<ECEnumeratorP>                  EnumeratorList;
typedef bmap<Utf8CP, KindOfQuantityP, less_str> KindOfQuantityMap;

/*---------------------------------------------------------------------------------**//**
* Used to hold property name and display label forECProperty, ECClass, and ECSchema.
* Property name supports only a limited set of characters; unsupported characters must
* be escaped as "__x####__" where "####" is a UTF-16 character code.
* If no explicit display label is provided, the property name is used as the display
* label, with encoded special characters decoded.
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECValidatedName
    {
private:
    Utf8String          m_name;
    Utf8String          m_displayLabel;
    bool                m_hasExplicitDisplayLabel;
public:
    ECValidatedName() : m_hasExplicitDisplayLabel (false) { }

    Utf8StringCR        GetName() const                 { return m_name; }
    bool                IsDisplayLabelDefined() const   { return m_hasExplicitDisplayLabel; }
    Utf8StringCR        GetDisplayLabel() const;
    void                SetName (Utf8CP name);
    void                SetDisplayLabel (Utf8CP label);
    };

//=======================================================================================
//! Handles validation, encoding, and decoding of names for ECSchemas, ECClasses, and
//! ECProperties.
//! The names of ECSchemas, ECClasses, and ECProperties must conform to the following
//! rules:
//!     -Contains only alphanumeric characters in the ranges ['A'..'Z'], ['a'..'z'], ['0'..'9'], and ['_']
//!     -Contains at least one character
//!     -Does not begin with a digit
//! @addtogroup ECObjectsGroup
//! @beginGroup
//! @bsiclass
//=======================================================================================
struct ECNameValidation
    {
private:
/*__PUBLISH_SECTION_END__*/
    static void AppendEncodedCharacter (WStringR encoded, WChar c);
public:
    ECOBJECTS_EXPORT static Utf8String             EncodeToValidName(Utf8StringCR name);
/*__PUBLISH_SECTION_START__*/
public:

    //! Enumeration defining the result of a validation check
    enum ValidationResult
        {
        RESULT_Valid = 0,                   //!< The name is valid
        RESULT_NullOrEmpty,                 //!< The string to check was NULL or empty
        RESULT_BeginsWithDigit,             //!< The string begins with a digit
        RESULT_IncludesInvalidCharacters    //!< The string contains invalid characters
        };

    //! Encodes special characters in a possibly invalid name to produce a valid name
    //! @param[out] encoded     Will hold the valid name
    //! @param[in]  name        The name to encode
    //! @returns true if any special characters were encoded, false if the name was already valid. In either case encoded will contain a valid name.
    ECOBJECTS_EXPORT static bool                EncodeToValidName (Utf8StringR encoded, Utf8StringCR name);

    //! Decodes special characters in a name encoded by EncodeToValidName() to produce a name suitable for display
    //! @param[out] decoded     Will hold the decoded name
    //! @param[in]  name        The name to decode
    //! @returns true if any special characters were decoded. Regardless of return value, decoded will contain a decoded name.
    ECOBJECTS_EXPORT static bool                DecodeFromValidName (Utf8StringR decoded, Utf8StringCR name);

    //! Checks a name against the rules for valid names
    //! @param[in] name     The name to validate
    //! @returns RESULT_Valid if the name is valid, or a ValidationResult indicating why the name is invalid.
    ECOBJECTS_EXPORT static ValidationResult    Validate (Utf8CP name);

    //! Returns true if the specified name is a valid EC name
    ECOBJECTS_EXPORT static bool                IsValidName (Utf8CP name);

    //! Checks whether a character is valid for use in an ECName, e.g. alphanumeric, plus '_'
    ECOBJECTS_EXPORT static bool IsValidAlphaNumericCharacter (WChar c);
    ECOBJECTS_EXPORT static bool IsValidAlphaNumericCharacter (Utf8Char c);
    };

//=======================================================================================
//! Used to represent the type of an ECProperty
//! @bsiclass
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
    //! Creates a TypeDescriptor for the given primitiveType
    //! @param[in] primitiveType    The primitive type to describe
    //! @returns an ECTypeDescriptor describing this primitive type
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveTypeDescriptor (PrimitiveType primitiveType);

    //! Creates a TypeDescriptor of an array of the given primitiveType
    //! @param[in] primitiveType    The primitiveType to create an array descriptor for
    //! @returns An ECTypeDescriptor describing an array of the given primitiveType
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType);

    //! Creates a TypeDescriptor for a struct array type
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructArrayTypeDescriptor ();
    //! Creates a TypeDescriptor for a struct
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateStructTypeDescriptor ();

    //! Creates a TypeDescriptor for a navigation property.  The type descriptor will be of the PrimitiveType 'type'.
    //! @param[in]  type         The type of the navigation property.  Use NavigationECProperty::GetType to get the type of a property
    //! @param[in]  isMultiple   True if the navigation property points to a relationship endpoint which allows more than one related instance.  Can use the result of the NavigationECProperty::IsMultiple method as input.
    //! @returns an ECTypeDescriptor describing a navigation property.
    ECOBJECTS_EXPORT static ECTypeDescriptor   CreateNavigationTypeDescriptor (PrimitiveType type, bool isMultiple);

    //! Constructor that takes a PrimitiveType
    ECTypeDescriptor (PrimitiveType primitiveType) : m_typeKind (VALUEKIND_Primitive), m_primitiveType (primitiveType) { };

    ECTypeDescriptor () : m_typeKind ((ValueKind) 0), m_primitiveType ((PrimitiveType) 0) { };
    ECTypeDescriptor (ValueKind valueKind, short valueKindQualifier) : m_typeKind (valueKind), m_primitiveType ((PrimitiveType)valueKindQualifier) { };

    bool operator==(ECTypeDescriptor const& rhs) const { return m_typeKind == rhs.m_typeKind && m_primitiveType == rhs.m_primitiveType; }
    bool operator!=(ECTypeDescriptor const& rhs) const { return !(*this == rhs); }

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
    inline short                GetTypeKindQualifier() const   { return m_primitiveType; }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECSchemaId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(ECSchemaId, BeInt64Id)
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECClassId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(ECClassId, BeInt64Id)
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECPropertyId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(ECPropertyId, BeInt64Id)
    };

typedef bvector<IECInstancePtr> ECCustomAttributeCollection;
struct ECCustomAttributeInstanceIterable;
struct SupplementedSchemaBuilder;

//=======================================================================================
//! Interface adopted by a container object which can hold custom attributes, such as those
//! associated with an ECProperty, ECSchema, or ECClass.
//! @see ECSchema::GetCustomAttributeContainer()
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECCustomAttributeContainer
{
private:
    friend struct ECCustomAttributeInstanceIterable;
    friend struct SupplementedSchemaBuilder;

    ECCustomAttributeCollection         m_primaryCustomAttributes;
    ECCustomAttributeCollection         m_supplementedCustomAttributes;
    bmap<IECInstanceCP, bvector<Utf8String>>  m_customAttributeXmlComments;
    SchemaWriteStatus                   AddCustomAttributeProperties (BeXmlNodeR oldNode, BeXmlNodeR newNode) const;

    IECInstancePtr                      GetCustomAttributeInternal(Utf8StringCR schemaName, Utf8StringCR className, bool includeBaseClasses, bool includeSupplementalAttributes) const;
    IECInstancePtr                      GetCustomAttributeInternal(ECClassCR ecClass, bool includeBaseClasses, bool includeSupplementalAttributes) const;

    ECObjectsStatus                     SetCustomAttributeInternal(ECCustomAttributeCollection& customAttributeCollection, IECInstanceR customAttributeInstance, bool requireSchemaReference = false);
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetPrimaryCustomAttribute(IECInstanceR customAttributeInstance);

    //! LEGECY METHOD
    IECInstancePtr                      GetCustomAttributeInternal(Utf8StringCR className, bool includeBaseClasses, bool includeSupplementalAttributes) const;

protected:
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetSupplementedCustomAttribute(IECInstanceR customAttributeInstance);

    CustomAttributeReadStatus           ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR context, ECSchemaCR fallBackSchema, int ecXmlVersionMajor);
    SchemaWriteStatus                   WriteCustomAttributes(BeXmlWriterR xmlWriter) const;
    //! Only copies primary ones, not consolidated ones. Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     CopyCustomAttributesTo(IECCustomAttributeContainerR destContainer) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
    void                                AddUniquePrimaryCustomAttributesToList(ECCustomAttributeCollection& returnList);
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP                  _GetContainerSchema() const = 0;// {return NULL;};
    virtual CustomAttributeContainerType _GetContainerType() const = 0;

    ECOBJECTS_EXPORT virtual ~IECCustomAttributeContainer();

/*__PUBLISH_SECTION_END__*/
public:
    ECSchemaP                           GetContainerSchema();
    //! Retrieves the local custom attribute matching the class name.  If the attribute is not 
    //! a supplemented attribute it will be copied and added to the supplemented list before it is returned.
    IECInstancePtr                      GetLocalAttributeAsSupplemented(Utf8StringCR schemaName, Utf8StringCR className);

    //! LEGECY METHOD
    IECInstancePtr                      GetLocalAttributeAsSupplemented(Utf8StringCR className);

//__PUBLISH_SECTION_START__
public:
    //! Returns true if the container has a custom attribute of a class of the specified name
    ECOBJECTS_EXPORT bool               IsDefined (Utf8StringCR schemaName, Utf8StringCR className) const;
    //! Returns true if the container has a custom attribute of a class of the specified class definition
    ECOBJECTS_EXPORT bool               IsDefined (ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes supplemental custom attributes
    //! and custom attributes from the base containers
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(Utf8StringCR schemaName, Utf8StringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes supplemental custom attributes
    //! and custom attributes from the base containers
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes supplemental custom attributes
    //! but not base containers
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(Utf8StringCR schemaName, Utf8StringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes supplemental custom attributes
    //! but not base containers
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes custom attributes from base containers
    //! but not supplemental custom attributes
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttribute(Utf8StringCR schemaName, Utf8StringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes custom attributes from base containers
    //! but not supplemental custom attributes
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttribute(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  DoesNot include custom attributes from either base
    //! containers or supplemental custom attributes
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttributeLocal(Utf8StringCR schemaName, Utf8StringCR className) const;

    //! Retrieves the custom attribute matching the class name.  DoesNot include custom attributes from either base
    //! containers or supplemental custom attributes
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttributeLocal(ECClassCR classDefinition) const;

    //! Retrieves all custom attributes from the container including supplemental custom attributes
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetCustomAttributes(bool includeBase) const;

    //! Retrieves all custom attributes from the container NOT including supplemental custom attributes
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetPrimaryCustomAttributes(bool includeBase) const;

    //! Adds a custom attribute to the container
    ECOBJECTS_EXPORT ECObjectsStatus    SetCustomAttribute(IECInstanceR customAttributeInstance);

    //! Removes a custom attribute from the container
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   Name of the class of the custom attribute to remove
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(Utf8StringCR schemaName, Utf8StringCR className);

    //! Removes a custom attribute from the container
    //! @param[in]  classDefinition ECClass of the custom attribute to remove
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(ECClassCR classDefinition);

    //! Removes a custom attribute from the container
    //! @param[in]  schemaName  The name of the schema the CustomAttribute is defined in
    //! @param[in]  className   Name of the class of the custom attribute to remove
    ECOBJECTS_EXPORT bool               RemoveSupplementedCustomAttribute(Utf8StringCR schemaName, Utf8StringCR className);

    //! Removes a supplemented custom attribute from the container
    //! @param[in]  classDefinition ECClass of the custom attribute to remove
    ECOBJECTS_EXPORT bool               RemoveSupplementedCustomAttribute(ECClassCR classDefinition);

    //! LEGECY METHODS
    ECOBJECTS_EXPORT bool               IsDefined(Utf8StringCR className) const;
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(Utf8StringCR className) const;
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(Utf8StringCR className) const;
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttribute(Utf8StringCR className) const;
    ECOBJECTS_EXPORT bool               RemoveCustomAttribute(Utf8StringCR className);
};

//=======================================================================================
//! Iterates over the custom attribute instances in a container
//! @bsiclass
//=======================================================================================
struct ECCustomAttributeInstanceIterable
{
private:
    friend struct IECCustomAttributeContainer;

    IECCustomAttributeContainerCR m_container;
    bool                        m_includeBaseContainers;
    bool                        m_includeSupplementalAttributes;
    ECCustomAttributeInstanceIterable( IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes) : m_container(container), m_includeBaseContainers(includeBase),
        m_includeSupplementalAttributes(includeSupplementalAttributes) {};
public:
    struct IteratorState : RefCountedBase
        {
        friend struct const_iterator;
        public:
/*__PUBLISH_SECTION_END__*/
        ECCustomAttributeCollection* m_customAttributes;
        ECCustomAttributeCollection::const_iterator m_customAttributesIterator;

        IteratorState(IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes);
        ~IteratorState();

        static RefCountedPtr<IteratorState> Create (IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes)
            { return new IteratorState(container, includeBase, includeSupplementalAttributes); } ;
/*__PUBLISH_SECTION_START__*/
        };

    //! Iterator for the custom attribute instances
    struct const_iterator : std::iterator<std::forward_iterator_tag, IECInstancePtr const>
        {
        private:
            friend struct ECCustomAttributeInstanceIterable;
            RefCountedPtr<IteratorState> m_state;
            bool m_isEnd;
/*__PUBLISH_SECTION_END__*/
            const_iterator (IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes);
            const_iterator () : m_isEnd(true) {;}
/*__PUBLISH_SECTION_START__*/
            const_iterator (char* ) {;} // must publish at least one private constructor to prevent instantiation

        public:
            ECOBJECTS_EXPORT const_iterator&     operator++(); //!< Increment the iterator
            ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const; //!< Compare iterator to value
            ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const; //!< Compare iterator to value
            ECOBJECTS_EXPORT IECInstancePtr const& operator* () const; //!< Return the IECInstance at the iterator location
        };

public:
    ECOBJECTS_EXPORT const_iterator begin () const; //!< Returns the beginning of the collection
    ECOBJECTS_EXPORT const_iterator end ()   const; //!< Returns the end of the collection
};

struct PrimitiveECProperty;

/*=================================================================================**//**
Base class for an object which provides the context for an IECTypeAdapter
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IECTypeAdapterContext : RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
private:
    EvaluationOptions m_evalOptions;

protected:
    virtual ECPropertyCP                        _GetProperty() const = 0;
    virtual uint32_t                            _GetComponentIndex() const = 0;
    virtual bool                                _Is3d() const = 0;
    virtual IECInstanceCP                       _GetECInstance() const = 0;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GetInstanceValue (ECValueR v, Utf8CP accessString, uint32_t arrayIndex) const;
    virtual IECClassLocaterR   _GetUnitsECClassLocater() const = 0;
    ECOBJECTS_EXPORT virtual EvaluationOptions  _GetEvaluationOptions () const;
    ECOBJECTS_EXPORT virtual void               _SetEvaluationOptions (EvaluationOptions evalOptions);

public:

    ECOBJECTS_EXPORT  IECInstanceCP     GetECInstance() const;
    ECOBJECTS_EXPORT  ECPropertyCP      GetProperty() const;
    ECOBJECTS_EXPORT  ECObjectsStatus   GetInstanceValue (ECValueR v, Utf8CP accessString, uint32_t arrayIndex = -1) const;

    ECOBJECTS_EXPORT EvaluationOptions          GetEvaluationOptions () const;
    ECOBJECTS_EXPORT void                       SetEvaluationOptions (EvaluationOptions evalOptions);

    //! The following are relevant to adapters for point types.
    ECOBJECTS_EXPORT  uint32_t                  GetComponentIndex() const;
    ECOBJECTS_EXPORT  bool                      Is3d() const;

    IECClassLocaterR                    GetUnitsECClassLocater() const {return _GetUnitsECClassLocater();}

    //! internal use only, primarily for ECExpressions
    typedef RefCountedPtr<IECTypeAdapterContext> (* FactoryFn)(ECPropertyCR, IECInstanceCR instance, uint32_t componentIndex);
    ECOBJECTS_EXPORT static void                RegisterFactory (FactoryFn fn);
    static RefCountedPtr<IECTypeAdapterContext> Create (ECPropertyCR ecproperty, IECInstanceCR instance, uint32_t componentIndex = COMPONENT_INDEX_None);
/*__PUBLISH_SECTION_START__*/
    static const uint32_t COMPONENT_INDEX_None = -1;
    };

typedef RefCountedPtr<IECTypeAdapterContext> IECTypeAdapterContextPtr;
    
/*=================================================================================**//**
Base class for an object which adapts the internal value of an ECProperty to a user-friendly string representation.
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECTypeAdapter : RefCountedBase
    {
    typedef bvector<Utf8String> StandardValuesCollection;
/*__PUBLISH_SECTION_END__*/
    // Note that the implementation of the extended type system is implemented in DgnPlatform in IDgnECTypeAdapter, which subclasses IECTypeAdapter and makes
    // use of IDgnECTypeAdapterContext which provides dgn-specific context such as file, model, and element.
protected:
    virtual bool                _HasStandardValues() const = 0;
    virtual bool                _IsStruct() const = 0;
    virtual bool                _IsTreatedAsString() const = 0;

    virtual IECInstancePtr      _CondenseFormatterForSerialization (ECN::IECInstanceCR formatter) const = 0;
    virtual IECInstancePtr      _PopulateDefaultFormatterProperties (ECN::IECInstanceCR formatter) const = 0;
    virtual IECInstancePtr      _CreateDefaultFormatter (bool includeAllValues, bool forDwg) const = 0;
    virtual bool                _GetPlaceholderValue (ECValueR v, IECTypeAdapterContextCR context) const = 0;

    virtual bool                _CanConvertToString (IECTypeAdapterContextCR context) const = 0;
    virtual bool                _CanConvertFromString (IECTypeAdapterContextCR context) const = 0;
    virtual bool                _ConvertToString (Utf8StringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP formatter) const = 0;
    virtual bool                _ConvertFromString (ECValueR v, Utf8CP str, IECTypeAdapterContextCR context) const = 0;

    virtual bool                _RequiresExpressionTypeConversion (EvaluationOptions evalOptions) const = 0;
    virtual bool                _ConvertToExpressionType (ECValueR v, IECTypeAdapterContextCR context) const = 0;
    virtual bool                _ConvertFromExpressionType (ECValueR v, IECTypeAdapterContextCR context) const = 0;

    virtual bool                _GetDisplayType (PrimitiveType& type) const = 0;
    virtual bool                _ConvertToDisplayType (ECValueR v, IECTypeAdapterContextCR context, IECInstanceCP formatter) const = 0;
    virtual bool                _AllowExpandMembers() const = 0;

    virtual bool                _SupportsUnits() const = 0;
    virtual bool                _GetUnits (UnitSpecR unit, IECTypeAdapterContextCR context) const = 0;

    virtual bool                _GetPropertyNotSetValue (ECValueR v) const { return false; }

    virtual bool                _IsOrdinalType () const = 0;
public:
    // For DgnPlatform interop
    struct Factory
        {
        virtual IECTypeAdapter& GetForProperty (ECPropertyCR ecproperty) const = 0;
        virtual IECTypeAdapter& GetForArrayMember (ArrayECPropertyCR ecproperty) const = 0;
        };
    ECOBJECTS_EXPORT static void          SetFactory (Factory const& factory);

    //! "Display Type" refers to the type that the user would associate with the displayed value of the property.
    //! For example, a value of type "Area" may be stored as a double in UORs, with a string representation in working units with unit label.
    //! In some contexts the user wants to operate on the displayed value as a double, but in working units. In this case the "display type"
    //! matches the stored type but not the stored value.
    //! Similarly a property with StandardValues custom attribute is stored as an integer but presented as a string. In this case the "display type"
    //! is also a string as the integer values have no meaning to the user.
    //! If the type adapter returns true from CanConvertToString(), it should also return true from GetDisplayType().
    //! @param[out] type      Will be initialized to the display type if one exists.
    //! @return true if the type adapter has a display type.
    ECOBJECTS_EXPORT bool       GetDisplayType (PrimitiveType& type) const;

    //! Converts a stored value to a value of the display type.
    //! For example, if the same arguments passed to ConvertToString() would return "44.5 Square Meters", this method should return 44.5
    //! @param[out] v           The stored value, to be converted in-place to display type.
    //! @param[in] context      Context in which to perform conversion
    //! @param[in] formatter    Formatting options, e.g. floating point precision, unit labels, etc.
    //! @return true if successfully converted to display type.
    ECOBJECTS_EXPORT bool       ConvertToDisplayType (ECValueR v, IECTypeAdapterContextCR context, IECInstanceCP formatter) const;

    //! Some extended types have a sigil value like -1 which is treated as if the property is not set from the user's perspective - often displayed to
    //! the user as "(None)" in UI. If this type adapter has such a sigil value, set it in the ECValue and return true; else return false;
    ECOBJECTS_EXPORT bool       GetPropertyNotSetValue (ECValueR v) const;

    //! Should return true if adapter type is ordinal. This gives hints for the UI in order to decide which operators to show.
    //! For example, for some numeric properties it doesn't make sense to show ordinal operators, because adapter gives a list
    //! of string rather than numbers.
    ECOBJECTS_EXPORT bool       IsOrdinalType () const;

    //! Returns a default value which can be used as a placeholder for e.g. testing formatting options
    ECOBJECTS_EXPORT bool       GetPlaceholderValue (ECValueR v, IECTypeAdapterContextCR context) const;
//__PUBLISH_SECTION_START__
public:
    //! @return true if it is possible to convert the underlying type to a string
    ECOBJECTS_EXPORT bool                 CanConvertToString (IECTypeAdapterContextCR context) const;

    //! @return true if it is possible to extract the underlying type from a string
    ECOBJECTS_EXPORT bool                 CanConvertFromString (IECTypeAdapterContextCR context) const;

    //! Converts the ECValue to a display string
    //! @param[out] str     The string representation of the value
    //! @param[in] v        Value to convert
    //! @param[in] context  Context under which conversion is performed
    //! @param[in] formatter Optional formatting specification, for ECFields
    //! @return true if successfully converted to string
    ECOBJECTS_EXPORT        bool ConvertToString (Utf8StringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP formatter = NULL) const;

    //! Converts from a string to the underlying ECValue type. Input string typically comes from user input
    //! @param[out] v       The converted value
    //! @param[in] str      The string to convert
    //! @param[in] context  Context under which conversion is performed
    //! @return true if conversion is successful
    ECOBJECTS_EXPORT        bool ConvertFromString (ECValueR v, Utf8CP str, IECTypeAdapterContextCR context) const;

    //! @return true if the value must be converted for use in ECExpressions.
    ECOBJECTS_EXPORT        bool RequiresExpressionTypeConversion (EvaluationOptions evalOptions = EVALOPT_Legacy) const;

    //! Converts the value to the value which should be used when evaluating ECExpressions.
    //! Typically no conversion is required. If the value has units, it should be converted to master units
    //! @param[out] v     The value to convert in-place
    //! @param[in] context  The context under which conversion is performed
    //! @returns true if successfully converted
    ECOBJECTS_EXPORT        bool ConvertToExpressionType (ECValueR v, IECTypeAdapterContextCR context) const;

    //! Converts the value from the value which should be used when evaluating ECExpressions.
    //! Typically no conversion is required. If the value has units, it should be converted from master units
    //! @param[out] v     The value to convert in-place
    //! @param[in] context  The context under which conversion is performed
    //! @returns true if successfully converted
    ECOBJECTS_EXPORT        bool ConvertFromExpressionType (ECValueR v, IECTypeAdapterContextCR context) const;

    //! Create an IECInstance representing default formatting options for converting to string.
    //! @param[in] includeAllValues If false, property values will be left NULL to save space; otherwise they will be initialized with default values
    //! @param[in] forDwg           If true, creates a formatting instance compatible with DWG
    //! @return An IECInstance which can be passed to ConvertToString(), or NULL if no special formatting options are supported.
    ECOBJECTS_EXPORT ECN::IECInstancePtr   CreateDefaultFormatter (bool includeAllValues, bool forDwg = false) const;

    //! @return true if this type adapter provides a finite set of permissible values for the property it represents
    ECOBJECTS_EXPORT bool                 HasStandardValues() const;

/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT bool                 AllowExpandMembers() const;
/*__PUBLISH_SECTION_START__*/

    //! @return true if the underlying type is a struct.
    ECOBJECTS_EXPORT bool                 IsStruct() const;

    //! Indicates if the value is intended to be interpreted as a string by the user, regardless of the underlying property type.
    //! For example: a TextStyle property may be stored internally as an integer ID, but presented to the user as a string.
    //! This method is checked when executing queries that perform string comparisons on property values - if it returns true, the property's string representation will be compared against the query; otherwise the property will be ignored.
    ECOBJECTS_EXPORT bool                 IsTreatedAsString() const;

    //! Given an instance representing formatting options, returns a copy of the instance optimized for serialization
    //! @param[in] formatter    The formatter instance to condense
    //! @return an IECInstance suitable for serialization, or NULL if the input formatter contained only default values in which case serialization is not necessary
    ECOBJECTS_EXPORT ECN::IECInstancePtr    CondenseFormatterForSerialization (ECN::IECInstanceCR formatter) const;

    //! Given an instance containing custom formatting options, replace any null properties with their default values
    //! @param[in] formatter    The formatter instance to populate
    //! @return an IECInstance in which all null properties have been replaced by their default values
    ECOBJECTS_EXPORT ECN::IECInstancePtr    PopulateDefaultFormatterProperties (ECN::IECInstanceCR formatter) const;

    //! Returns true if ECProperties of the extended type handled by this type adapter may have units.
    ECOBJECTS_EXPORT bool                   SupportsUnits() const;

    //! If SupportsUnits() returns true, this method is called to obtain the units for a specific ECProperty.
    //! @param[in]      unit       Holds the unit specification. If the ECProperty has no units, leave this default-initialized and return true.
    //! @param[in]      context    Context in which to evaluate the units.
    //! @return true if the unit information could be obtained, even if the unit is not specified. Return false if unit information could not be obtained.
    ECOBJECTS_EXPORT bool                   GetUnits (UnitSpecR unit, IECTypeAdapterContextCR context) const;
    };

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECProperty /*abstract*/ : public IECCustomAttributeContainer
{
friend struct ECClass;

private:
    Utf8String              m_description;
    Utf8String              m_minimumValue;
    Utf8String              m_maximumValue;
    ECValidatedName         m_validatedName;
    mutable ECPropertyId    m_ecPropertyId;
    bool                    m_readOnly;
    ECClassCR               m_class;
    ECPropertyCP            m_baseProperty;
    mutable IECTypeAdapter* m_cachedTypeAdapter;

    static void     SetErrorHandling (bool doAssert);
protected:
    Utf8String              m_originalTypeName; //Will be empty unless the typeName was unrecognized. Keep this so that we can re-write the ECSchema without changing the type to string
    bool                    m_forSupplementation;   // If when supplementing the schema, a local property had to be created, then don't serialize this property
    ECProperty (ECClassCR ecClass);
    virtual ~ECProperty();

    ECObjectsStatus                     SetName (Utf8StringCR name);

    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor);
    virtual SchemaWriteStatus           _WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor);
    SchemaWriteStatus                   _WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, int ecXmlVersionMajor, bvector<bpair<Utf8CP, Utf8CP>>* attributes=nullptr, bool writeType=true);
    virtual Utf8String                  _GetTypeNameForXml(int ecXmlVersionMajor) const { return GetTypeName(); }

    virtual bool                        _IsPrimitive () const { return false; }
    virtual PrimitiveECPropertyCP       _GetAsPrimitivePropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual PrimitiveECPropertyP        _GetAsPrimitivePropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsStruct () const { return false; }
    virtual StructECPropertyCP          _GetAsStructPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual StructECPropertyP           _GetAsStructPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsArray () const { return false; }
    virtual ArrayECPropertyCP           _GetAsArrayPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual ArrayECPropertyP            _GetAsArrayPropertyP()        { return nullptr; } // used to avoid dynamic_cast
    virtual bool                        _IsPrimitiveArray() const { return false;  }

    virtual bool                        _IsStructArray() const { return false; }
    virtual StructArrayECPropertyCP     _GetAsStructArrayPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual StructArrayECPropertyP      _GetAsStructArrayPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsNavigation() const { return false; }
    virtual NavigationECPropertyCP      _GetAsNavigationPropertyCP() const  { return nullptr; } // used to avoid dynamic_cast
    virtual NavigationECPropertyP       _GetAsNavigationPropertyP()         { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _HasExtendedType() const { return false; }
    virtual ExtendedTypeECPropertyCP    _GetAsExtendedTypePropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual ExtendedTypeECPropertyP     _GetAsExtendedTypePropertyP() { return nullptr; } // used to avoid dynamic_cast

    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    // prefix relative to the containing schema.
    virtual Utf8String                  _GetTypeName () const = 0;
    virtual ECObjectsStatus             _SetTypeName (Utf8StringCR typeName) = 0;
    

    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const = 0;

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP                  _GetContainerSchema() const override;
    
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const { return NULL; }
    virtual bool                                _IsCalculated() const { return false; }
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) { return false; }

    void                                InvalidateClassLayout();
/*__PUBLISH_SECTION_END__*/
public:
    // The following are used by the 'extended type' system which is currently implemented in DgnPlatform
    IECTypeAdapter*                     GetCachedTypeAdapter() const { return m_cachedTypeAdapter; }
    void                                SetCachedTypeAdapter (IECTypeAdapter* adapter) const { m_cachedTypeAdapter = adapter; }
    IECTypeAdapter*                     GetTypeAdapter() const;
    bool                                IsReadOnlyFlagSet() const { return m_readOnly; }
    bool                                IsForSupplementation() const { return m_forSupplementation; }

    //! Intended to be called by ECDb or a similar system
    void SetId(ECPropertyId id) { BeAssert(!m_ecPropertyId.IsValid()); m_ecPropertyId = id; };
    bool HasId() const { return m_ecPropertyId.IsValid(); };

/*__PUBLISH_SECTION_START__*/
public:
    //! Returns the CalculatedPropertySpecification associated with this ECProperty, if any
     CalculatedPropertySpecificationCP   GetCalculatedPropertySpecification() const { return _GetCalculatedPropertySpecification(); }
    //! Returns true if this ECProperty has a CalculatedECPropertySpecification custom attribute applied to it.
     bool               IsCalculated() const { return _IsCalculated(); }

    //! Sets or removes the CalculatedECPropertySpecification custom attribute associated with this ECProperty.
    //! @param[in] expressionAttribute  An IECInstance of the ECClass CalculatedECPropertySpecification, or NULL to remove the specification.
    //! @return true if the specification was successfully updated.
    //! @remarks Call this method rather than setting the custom attribute directly, to ensure internal state is updated.
    ECOBJECTS_EXPORT bool                                SetCalculatedPropertySpecification(IECInstanceP expressionAttribute);

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    ECOBJECTS_EXPORT ECPropertyId       GetId() const;
    //! Returns the name of the ECClass that this property is contained within
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;
    // ECClass implementation will index property by name so publicly name can not be reset
    //! Gets the name of the ECProperty
    ECOBJECTS_EXPORT Utf8StringCR       GetName() const;
    //! Returns whether the DisplayLabel is explicitly set
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;
    //! Returns whether this property is a Struct property
    bool                                GetIsStruct() const { return _IsStruct(); }
    //! Returns whether this property is an Array property (either a Primitive array or a StructArray)
    //! Use either GetIsPrimitiveArray or GetIsStructArray to differentiate.
    bool                                GetIsArray() const { return _IsArray(); }
    //! Returns whether this property is a Primitive property
    bool                                GetIsPrimitive() const { return _IsPrimitive(); }
    //! Returns whether this property is a StructArray property
    bool                                GetIsStructArray() const { return _IsStructArray(); }
    //! Returns whether this property is a Primitive array
    bool                                GetIsPrimitiveArray() const { return _IsPrimitiveArray(); }
    //! Returns whether this property is a NavigationECProperty
    bool                                GetIsNavigation() const { return _IsNavigation(); }
    
    //! Sets the ECXML typename for the property.  @see GetTypeName()
    ECOBJECTS_EXPORT ECObjectsStatus    SetTypeName(Utf8String value);
    //! The ECXML typename for the property.
    //! The TypeName for struct properties will be the ECClass name of the struct.  It may be qualified with a namespacePrefix if
    //! the struct belongs to a schema that is referenced by the schema actually containing this property.
    //! The TypeName for array properties will be the type of the elements the array contains.
    //! This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with a namespace
    //! prefix relative to the containing schema.
    ECOBJECTS_EXPORT Utf8String         GetTypeName() const;
    //! Sets the description for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(Utf8StringCR value);
    //! Sets the minimum value for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetMinimumValue(Utf8StringCR min);
    //! Gets whether the minimum value has been defined explicitly
    ECOBJECTS_EXPORT bool               IsMinimumValueDefined() const;
    //! Gets the minimum value for this ECProperty
    ECOBJECTS_EXPORT Utf8StringCR       GetMinimumValue() const;
    //! Sets the maximum value for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetMaximumValue(Utf8StringCR max);
    //! Gets whether the maximum value has been defined explicitly
    ECOBJECTS_EXPORT bool               IsMaximumValueDefined() const;
    //! Gets the maximum value for this ECProperty
    ECOBJECTS_EXPORT Utf8StringCR       GetMaximumValue() const;
    //! The Description of this ECProperty.  Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR       GetDescription() const;
    //! Gets the invariant description for this ECProperty.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDescription() const;
    //! Sets the Display Label for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(Utf8StringCR value);
    //! Gets the Display Label for this ECProperty.  If no label has been set explicitly, it will return the Name of the property
    ECOBJECTS_EXPORT Utf8StringCR       GetDisplayLabel() const;
    //! Gets the invariant display label for this ECSchema.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDisplayLabel() const;
    //! Sets whether this ECProperty's value is read only
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsReadOnly(bool value);
    //! Gets whether this ECProperty's value is read only
    ECOBJECTS_EXPORT bool               GetIsReadOnly() const;
    //! Sets the base property that this ECProperty inherits from
    //! @remarks In the case of multiple inheritance where more than one base class has a
    //! property with the same name the first base class found will provide the base property.
    //! It is considered bad design to have the same property in multiple base classes.  In
    //! future this may become an error, but due to the current implementation details
    //! it is not an error and the base property is deterministic because the order of the
    //! base classes is preserved
    ECOBJECTS_EXPORT ECObjectsStatus    SetBaseProperty(ECPropertyCP value);
    //! Gets the base property, if any, that this ECProperty inherits from
    //! @remarks In the case of multiple inheritance where more than one base class has a
    //! property with the same name the first base class found will provide the base property.
    //! It is considered bad design to have the same property in multiple base classes.  In
    //! future this may become an error, but due to the current implementation details
    //! it is not an error and the base property is deterministic because the order of the
    //! base classes is preserved
    ECOBJECTS_EXPORT ECPropertyCP       GetBaseProperty() const;

    //! Sets whether this ECProperty's value is read only
    //@param[in]    isReadOnly  Valid values are 'True' and 'False' (case insensitive)
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsReadOnly (Utf8CP isReadOnly);

    //! Returns whether this property has an extended type specified
    ECOBJECTS_EXPORT bool               HasExtendedType() const;

    PrimitiveECPropertyCP  GetAsPrimitiveProperty() const       { return _GetAsPrimitivePropertyCP(); } //!< Returns the property as a const PrimitiveECProperty*
    PrimitiveECPropertyP   GetAsPrimitivePropertyP()            { return _GetAsPrimitivePropertyP(); } //!< Returns the property as a PrimitiveECProperty*
    ArrayECPropertyCP      GetAsArrayProperty() const           { return _GetAsArrayPropertyCP(); } //!< Returns the property as a const ArrayECProperty*
    ArrayECPropertyP       GetAsArrayPropertyP()                { return _GetAsArrayPropertyP(); } //!< Returns the property as an ArrayECProperty*
    StructECPropertyCP     GetAsStructProperty() const          { return _GetAsStructPropertyCP(); } //!< Returns the property as a const StructECProperty*
    StructECPropertyP      GetAsStructPropertyP()               { return _GetAsStructPropertyP(); } //!< Returns the property as a StructECProperty*
    StructArrayECPropertyCP GetAsStructArrayProperty() const    { return _GetAsStructArrayPropertyCP(); } //! <Returns the property as a const StructArrayECProperty*
    StructArrayECPropertyP GetAsStructArrayPropertyP()          { return _GetAsStructArrayPropertyP(); } //! <Returns the property as a StructArrayECProperty*
    NavigationECPropertyCP GetAsNavigationProperty() const      { return _GetAsNavigationPropertyCP(); } //! <Returns the property as a const NavigationECProperty*
    NavigationECPropertyP  GetAsNavigationPropertyP()           { return _GetAsNavigationPropertyP(); } //! <Returns the property as a NavigationECProperty*
    ExtendedTypeECPropertyCP GetAsExtendedTypeProperty() const  { return _GetAsExtendedTypePropertyCP(); } //! <Returns the property as a const ExtendedTypeECProperty*
    ExtendedTypeECPropertyP GetAsExtendedTypePropertyP()        { return _GetAsExtendedTypePropertyP(); } //! <Returns the property as a ExtendedTypeECProperty*
};

//=======================================================================================
//! The in-memory representation of an ExtendedTypeECProperty which enhances the ECProperty
//! baseclass by the methods to support ExtendedType handling.
//! @bsiclass
//=======================================================================================
struct ExtendedTypeECProperty : public ECProperty
 {
private:
    Utf8String    m_extendedTypeName;
    KindOfQuantityCP m_kindOfQuantity;
        
protected:
    ExtendedTypeECProperty(ECClassCR ecClass) : ECProperty(ecClass), m_kindOfQuantity(nullptr) {};
    virtual bool  _HasExtendedType() const override { return GetExtendedTypeName().size() > 0; }
    virtual ExtendedTypeECPropertyCP _GetAsExtendedTypePropertyCP () const override { return this; }
    virtual ExtendedTypeECPropertyP  _GetAsExtendedTypePropertyP() override { return this; }
    bool ExtendedTypeLocallyDefined() const { return m_extendedTypeName.size() > 0; }

    SchemaReadStatus ReadExtendedTypeAndKindOfQuantityXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext);
    SchemaWriteStatus WriteExtendedTypeAndKindOfQuantityXml(bvector<bpair<Utf8CP, Utf8CP>>& attributes, int ecXmlVersionMajor) const;

public:
    //! Gets the extended type name of this ECProperty
    ECOBJECTS_EXPORT Utf8StringCR GetExtendedTypeName() const;
    //! Sets the Name of the Extended Type of this property.
    ECOBJECTS_EXPORT ECObjectsStatus SetExtendedTypeName(Utf8CP extendedTypeName);
    //! Resets the extended type on this property.
    ECOBJECTS_EXPORT bool RemoveExtendedTypeName();
    //! Sets the KindOfQuantity of this property, provide nullptr to unset.
    void SetKindOfQuantity(KindOfQuantityCP value) { m_kindOfQuantity = value; }
    //! Gets the KindOfQuantity of this property or nullptr, if none has been set
    KindOfQuantityCP GetKindOfQuantity() const { return m_kindOfQuantity; }
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct PrimitiveECProperty : public ExtendedTypeECProperty
{
    DEFINE_T_SUPER(ECProperty)
friend struct ECClass;
private:
    PrimitiveType                               m_primitiveType;
    ECEnumerationCP                             m_enumeration;
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;   // lazily-initialized

    PrimitiveECProperty(ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), ExtendedTypeECProperty(ecClass), m_enumeration(nullptr) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor) override;
    virtual SchemaWriteStatus           _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual PrimitiveECPropertyCP       _GetAsPrimitivePropertyCP() const override { return this; }
    virtual PrimitiveECPropertyP        _GetAsPrimitivePropertyP() override { return this; }
    virtual Utf8String                  _GetTypeName () const override;
    virtual Utf8String                  _GetTypeNameForXml(int ecXmlVersionMajor) const override;
    virtual ECObjectsStatus             _SetTypeName (Utf8StringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const override;
    virtual bool                                _IsCalculated() const override;
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) override;
    virtual CustomAttributeContainerType        _GetContainerType() const override { return CustomAttributeContainerType::PrimitiveProperty; }

public:
    //! Sets the PrimitiveType of this ECProperty.  The default type is ::PRIMITIVETYPE_String
    ECOBJECTS_EXPORT ECObjectsStatus SetType(PrimitiveType value);
    //! Gets the PrimitiveType of this ECProperty
    ECOBJECTS_EXPORT PrimitiveType GetType() const;
    //! Sets an ECEnumeration as type of this ECProperty.
    ECOBJECTS_EXPORT ECObjectsStatus SetType(ECEnumerationCR value);
    //! Gets the Enumeration of this ECProperty or nullptr if none used.
    ECOBJECTS_EXPORT ECEnumerationCP GetEnumeration() const;
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct StructECProperty : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
friend struct ECStructClass;
private:
    ECStructClassCP   m_structType;

    StructECProperty (ECClassCR ecClass) : m_structType(NULL), ECProperty(ecClass) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor) override;
    virtual SchemaWriteStatus           _WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual StructECPropertyCP          _GetAsStructPropertyCP() const override { return this; }
    virtual StructECPropertyP           _GetAsStructPropertyP()        override { return this; }
    virtual Utf8String                  _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (Utf8StringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::StructProperty; }

//__PUBLISH_SECTION_START__
public:
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    ECOBJECTS_EXPORT ECObjectsStatus    SetType(ECStructClassCR value);
    //! Gets the ECClass that defines the type for this property
    ECOBJECTS_EXPORT ECStructClassCR    GetType() const; 
};

//=======================================================================================
//! The in-memory representation of an ECArrayProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ArrayECProperty : public ExtendedTypeECProperty
{
    DEFINE_T_SUPER(ECProperty)
friend struct ECClass;

private:
    uint32_t                                    m_minOccurs;
    uint32_t                                    m_maxOccurs;    // D-106653 we store this as read from the schema, but all arrays are considered to be of unbounded size
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;
    PrimitiveType                               m_primitiveType;
    mutable IECTypeAdapter*                     m_cachedMemberTypeAdapter;

protected:
    ArrayKind               m_arrayKind;

    ArrayECProperty(ECClassCR ecClass)
        : ExtendedTypeECProperty(ecClass), m_primitiveType(PRIMITIVETYPE_String), m_arrayKind(ARRAYKIND_Primitive),
        m_minOccurs(0), m_maxOccurs(UINT_MAX), m_cachedMemberTypeAdapter(NULL)
        {};
    ECObjectsStatus                     SetMinOccurs(Utf8StringCR minOccurs);
    ECObjectsStatus                     SetMaxOccurs(Utf8StringCR maxOccurs);

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor) override;
    virtual SchemaWriteStatus           _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual bool                        _IsPrimitiveArray() const override { return ARRAYKIND_Primitive == m_arrayKind; }
    virtual ArrayECPropertyCP           _GetAsArrayPropertyCP() const override { return this; }
    virtual ArrayECPropertyP            _GetAsArrayPropertyP()        override { return this; }
    virtual Utf8String                  _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (Utf8StringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const override;
    virtual bool                                _IsCalculated() const override;
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) override;
    virtual CustomAttributeContainerType        _GetContainerType() const override { return CustomAttributeContainerType::ArrayProperty; }

/*__PUBLISH_SECTION_END__*/
public:
    // The following are used by the 'extended type' system which is currently implemented in DgnPlatform
    IECTypeAdapter*                     GetCachedMemberTypeAdapter() const  { return m_cachedMemberTypeAdapter; }
    void                                SetCachedMemberTypeAdapter (IECTypeAdapter* adapter) const { m_cachedMemberTypeAdapter = adapter; }
    IECTypeAdapter*                     GetMemberTypeAdapter() const;
/*__PUBLISH_SECTION_START__*/
public:
    //! The ArrayKind of this ECProperty
    ECOBJECTS_EXPORT ArrayKind GetKind() const;

    //! Sets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    ECOBJECTS_EXPORT ECObjectsStatus    SetPrimitiveElementType(PrimitiveType value);
    //! Gets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    ECOBJECTS_EXPORT PrimitiveType      GetPrimitiveElementType() const;
    //! Sets the Minimum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus    SetMinOccurs(uint32_t value);
    //! Gets the Minimum number of array members.
    ECOBJECTS_EXPORT uint32_t           GetMinOccurs() const;
    //! Sets the Maximum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus    SetMaxOccurs(uint32_t value);
    //! Gets the Maximum number of array members.
    ECOBJECTS_EXPORT uint32_t           GetMaxOccurs() const;

//__PUBLISH_SECTION_END__
    //! Because of a legacy bug GetMaxOccurs always returns "unbounded". For components that need to persist
    //! the ECSchema as is, GetStoredMaxOccurs can be called as a workaround until the max occurs issue has been resolved.
    uint32_t                            GetStoredMaxOccurs () const { return m_maxOccurs; }
//__PUBLISH_SECTION_START__
    };

//=======================================================================================
//! The in-memory representation of an StructArrayECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct StructArrayECProperty : ArrayECProperty
    {
DEFINE_T_SUPER(ArrayECProperty)
private:
    friend struct ECClass;
    ECStructClassCP m_structType;
    StructArrayECProperty(ECClassCR ecClass)
        : ArrayECProperty(ecClass)
        {
        m_arrayKind = ARRAYKIND_Struct;
        };
protected:
    virtual Utf8String                  _GetTypeName() const override;
    virtual ECObjectsStatus             _SetTypeName(Utf8StringCR typeName) override;
    virtual bool                        _IsStructArray() const override { return true; }
    virtual StructArrayECPropertyCP     _GetAsStructArrayPropertyCP() const override { return this; }
    virtual StructArrayECPropertyP      _GetAsStructArrayPropertyP()        override { return this; }
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::StructArrayProperty; }

public:
    //! Sets the ECClass to be used for the array's struct elements
    ECOBJECTS_EXPORT ECObjectsStatus    SetStructElementType(ECStructClassCP value);
    //! Gets the ECClass of the array's struct elements
    ECOBJECTS_EXPORT ECStructClassCP    GetStructElementType() const;

    };

//=======================================================================================
//! The in-memory representation of an ECNavigationProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavigationECProperty : public ECProperty
    {
DEFINE_T_SUPER(ECProperty)

friend struct ECEntityClass;
friend struct ECClass;
private:
    ECRelationshipClassCP       m_relationshipClass;
    ECRelatedInstanceDirection  m_direction;
    PrimitiveType               m_type;
    ValueKind                   m_valueKind;

protected:
    explicit NavigationECProperty(ECClassCR ecClass)
        : ECProperty(ecClass), m_relationshipClass(nullptr), m_direction(ECRelatedInstanceDirection::Forward), m_valueKind(ValueKind::VALUEKIND_Uninitialized), m_type(PrimitiveType::PRIMITIVETYPE_Long) {};

    ECObjectsStatus                 SetRelationshipClassName(Utf8CP relationshipName);
    ECObjectsStatus                 SetDirection(Utf8CP directionString);
    Utf8String                      GetRelationshipClassName() const;

protected:
    virtual SchemaReadStatus        _ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor) override;
    virtual SchemaWriteStatus       _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) override;

    virtual bool                    _IsNavigation() const override { return true; }
    virtual NavigationECPropertyCP  _GetAsNavigationPropertyCP() const override { return this; }
    virtual NavigationECPropertyP   _GetAsNavigationPropertyP() override { return this; }

    virtual Utf8String              _GetTypeName() const override;
    // Not valid because type cannot be set from xml, it must be set at runtime
    virtual ECObjectsStatus         _SetTypeName(Utf8StringCR typeName) override { return ECObjectsStatus::OperationNotSupported; }

    virtual bool                    _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::NavigationProperty; }

public:
    // !Gets the relationship class used to determine what related instance this navigation property points to
    ECRelationshipClassCP       GetRelationshipClass() const { return m_relationshipClass; }
    // !Gets the direction used to determine what related instance this navigation property points to
    ECRelatedInstanceDirection  GetDirection() const { return m_direction; }
    // !Sets the relationship and direction used by the navigation property
    // @param[in]   relClass    The relationship this navigation property will represent
    // @param[in]   direction   The direction the relationship will be traversed.  Forward if the class containing this property is a source constraint, Backward if the class is a target constraint
    // @param[in]   verify      If true the relationshipClass an direction will be verified to ensure the navigation property fits within the relationship constraints.  Default is true.  If not verified at creation the Verify method must be called before the navigation property is used or it's type descriptor will not be valid.
    // @returns     Returns Success if validation successful or not performed, SchemaNotFound if the schema containing the relationship class is not referenced and RelationshipConstraintsNotCompatible if validation
    //              is performed but not successfull
    ECOBJECTS_EXPORT ECObjectsStatus            SetRelationshipClass(ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, bool verify = true);
    
    // !Verifies that the relationship class and direction is valid.  
    ECOBJECTS_EXPORT bool       Verify();
    // !Returns true if the Verify method has been called on this Navigation Property, false if it has not.
    bool                        IsVerified() const { return ValueKind::VALUEKIND_Uninitialized != m_valueKind; }
    // !Returns true if the navigation property points to an endpoint which can have more than one related instance
    bool                        IsMultiple() const { return ValueKind::VALUEKIND_Array == m_valueKind; }
    
    //! Sets the PrimitiveType of this ECProperty.  The default type is ::PRIMITIVETYPE_String
    ECOBJECTS_EXPORT ECObjectsStatus SetType(PrimitiveType type);
    //! Gets the PrimitiveType of this ECProperty
    PrimitiveType GetType() const { return m_type; }
    };

//=======================================================================================
//! Container holding ECProperties that supports STL like iteration
//! @bsiclass
//=======================================================================================
struct ECPropertyIterable
{
private:
    friend struct ECClass;

    ECClassCR       m_ecClass;
    bool            m_includeBaseProperties;

    ECPropertyIterable(ECClassCR ecClass, bool includeBaseProperties);

public:
    struct IteratorState : RefCountedBase
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

    //! Iterator over the properties
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
            const_iterator (char* ) {;} // must publish at least one private constructor to prevent instantiation

        public:
            ECOBJECTS_EXPORT const_iterator&     operator++(); //!< Increments the iterator
            ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const; //!< Checks for inequality
            ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const; //!< Checks for equality
            ECOBJECTS_EXPORT ECPropertyP const&  operator* () const; //!< Returns the value at the current location
        };

public:
    ECOBJECTS_EXPORT const_iterator begin () const; //!< Returns the beginning of the collection
    ECOBJECTS_EXPORT const_iterator end ()   const; //!< Returns the end of the collection

    //! Attempts to look up an ECProperty by its display label
    //! @param[in]      label The label of the ECProperty to find
    //! @return     The ECProperty with the specified label, or nullptr if no such ECProperty exists
    ECOBJECTS_EXPORT ECPropertyCP   FindByDisplayLabel (Utf8CP label) const;
    };

typedef bvector<ECClassP> ECBaseClassesList;
typedef bvector<ECClassP> ECDerivedClassesList;
typedef bvector<ECEntityClassP> ECConstraintClassesList;
typedef bool (*TraversalDelegate) (ECClassCP, const void *);
struct SchemaXmlReader;
struct SchemaXmlWriter;

struct StandaloneECEnabler;
struct SearchPathSchemaFileLocater;
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
typedef StandaloneECEnabler*                StandaloneECEnablerP;
typedef RefCountedPtr<ECSchema>             ECSchemaPtr;
typedef RefCountedPtr<SearchPathSchemaFileLocater> SearchPathSchemaFileLocaterPtr;

enum class ECClassType
    {
    Entity,
    Relationship,
    Struct,
    CustomAttribute
    };
//=======================================================================================
//! The in-memory representation of an ECClass as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECClass /*abstract*/ : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/

friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlReader2;
friend struct SchemaXmlReader3;
friend struct SchemaXmlWriter;
friend struct ECPropertyIterable::IteratorState;
friend struct SupplementedSchemaBuilder;
friend struct ECProperty; // for access to InvalidateDefaultStandaloneEnabler() when property is modified

//__PUBLISH_SECTION_START__

private:
    mutable Utf8String              m_fullName;
    mutable Utf8String              m_ecsqlName;
    Utf8String                      m_description;
    ECValidatedName                 m_validatedName;
    mutable ECClassId               m_ecClassId;
    ECClassModifier                 m_modifier;
    ECSchemaCR                      m_schema;
    ECBaseClassesList               m_baseClasses;
    mutable ECDerivedClassesList    m_derivedClasses;

    PropertyMap                     m_propertyMap;
    PropertyList                    m_propertyList;
    mutable StandaloneECEnablerPtr  m_defaultStandaloneEnabler;
    bvector<Utf8String> m_xmlComments;
    bmap<Utf8String, bvector<Utf8String>> m_contentXmlComments;
    bmap<int, bvector<Utf8String>> m_customAttributeXmlComments;

    ECObjectsStatus AddProperty (ECPropertyP& pProperty, bool resolveConflicts = false);
    ECObjectsStatus RemoveProperty (ECPropertyR pProperty);
    void FindUniquePropertyName(Utf8StringR newName, Utf8CP prefix, Utf8CP originalName);
    ECObjectsStatus RenameConflictProperty(ECPropertyP thisProperty, bool renameDerivedProperties);

    static bool     SchemaAllowsOverridingArrays(ECSchemaCP schema);

    static bool     CheckBaseClassCycles(ECClassCP currentBaseClass, const void * arg);
    static bool     AddUniquePropertiesToList(ECClassCP crrentBaseClass, const void * arg);
    bool            TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, const void * arg) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetProperties(bool includeBaseProperties, PropertyList* propertyList) const;

    ECObjectsStatus CanPropertyBeOverridden(ECPropertyCR baseProperty, ECPropertyCR newProperty) const;
    void            AddDerivedClass(ECClassCR baseClass) const;
    void            RemoveDerivedClass(ECClassCR baseClass) const;
    void            RemoveDerivedClasses ();
    void            RemoveBaseClasses ();
    static void     SetErrorHandling (bool doAssert);
    ECObjectsStatus CopyPropertyForSupplementation(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes);
    ECObjectsStatus CopyProperty(ECPropertyP& destProperty, ECPropertyP sourceProperty, bool copyCustomAttributes);
    ECObjectsStatus CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, Utf8CP destPropertyName, bool copyCustomAttributes);

    void            OnBaseClassPropertyRemoved (ECPropertyCR baseProperty);
    ECObjectsStatus OnBaseClassPropertyAdded (ECPropertyCR baseProperty, bool resolveConflicts);
protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECSchemaCR schema);
    virtual ~ECClass();

    ECObjectsStatus                     AddProperty(ECPropertyP pProperty, Utf8StringCR name);
    virtual ECObjectsStatus             _AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false);

    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    virtual ECSchemaCP                  _GetContainerSchema() const override;

    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus                     SetName (Utf8StringCR name);

    virtual SchemaReadStatus            _ReadXmlAttributes (BeXmlNodeR classNode);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @param[in]  context         The read context that contains information about schemas used for deserialization
    //! @param[in]  conversionSchema  If there was a supplied schema to assist in converting from V2 to V3
    //! @param[in]  ecXmlVersionMajor The major version of the ECXml spec used for serializing this ECClass
    //! @param[out] navigationProperties A running list of all navigation properties in the schema.  This list is used for validation, which may only happen after all classes are loaded
    //! @return   Status code
    virtual SchemaReadStatus            _ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, int ecXmlVersionMajor, bvector<NavigationECPropertyP>& navigationProperties);

    void _ReadCommentsInSameLine(BeXmlNodeR childNode, bvector<Utf8String>& comments);

    SchemaReadStatus                    _ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema);
    SchemaReadStatus                    _ReadPropertyFromXmlAndAddToClass( ECPropertyP ecProperty, BeXmlNodeP& childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, Utf8CP childNodeName, int ecXmlVersionMajor);

    virtual SchemaWriteStatus           _WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const;
    SchemaWriteStatus                   _WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor, Utf8CP elementName, bmap<Utf8CP, Utf8CP>* additionalAttributes, bool doElementEnd) const;

    virtual ECClassType                  _GetClassType() const { return ECClassType::Entity;} // default type

    virtual ECRelationshipClassCP       _GetRelationshipClassCP () const { return NULL; }  // used to avoid dynamic_cast
    virtual ECRelationshipClassP        _GetRelationshipClassP ()        { return NULL; }  // used to avoid dynamic_cast

    virtual ECEntityClassCP             _GetEntityClassCP () const { return nullptr; }  // used to avoid dynamic_cast
    virtual ECEntityClassP              _GetEntityClassP ()        { return nullptr; }  // used to avoid dynamic_cast

    virtual ECStructClassCP             _GetStructClassCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual ECStructClassP              _GetStructClassP()        { return nullptr; } // used to avoid dynamic_cast

    virtual ECCustomAttributeClassCP    _GetCustomAttributeClassCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual ECCustomAttributeClassP     _GetCustomAttributeClassP()        { return nullptr; } // used to avoid dynamic_cast

    void                                InvalidateDefaultStandaloneEnabler() const;
public:
    ECOBJECTS_EXPORT ECPropertyP            GetPropertyByIndex (uint32_t index) const;
    ECOBJECTS_EXPORT ECPropertyP            GetBaseClassPropertyP (Utf8CP name) const;
    ECOBJECTS_EXPORT ECObjectsStatus        RenameProperty (ECPropertyR ecProperty, Utf8CP newName);
    ECOBJECTS_EXPORT ECObjectsStatus        ReplaceProperty (ECPropertyP& newProperty, ValueKind valueKind, ECPropertyR propertyToRemove);
    ECOBJECTS_EXPORT ECObjectsStatus        DeleteProperty (ECPropertyR ecProperty);
    ECSchemaR                               GetSchemaR() { return const_cast<ECSchemaR>(m_schema); }

    //! Intended to be called by ECDb or a similar system
    void SetId(ECClassId id) { BeAssert(!m_ecClassId.IsValid()); m_ecClassId = id; };
    bool HasId() const { return m_ecClassId.IsValid(); };

public:
    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    ECOBJECTS_EXPORT ECClassId             GetId() const;
    //! Returns the StandaloneECEnabler for this class
    ECOBJECTS_EXPORT StandaloneECEnablerP  GetDefaultStandaloneEnabler() const;

    //! The type of derived ECClass this is
    ECOBJECTS_EXPORT ECClassType           GetClassType() const;

    //! Is the class an entity class
    bool                                   IsEntityClass() const { return ECClassType::Entity == GetClassType(); }

    //! Is the class a struct class
    bool                                   IsStructClass() const { return ECClassType::Struct == GetClassType();}

    //! Is the class a custom attribute class
    bool                                   IsCustomAttributeClass() const { return ECClassType::CustomAttribute == GetClassType(); }

    //! Is the class a relationship class
    bool                                   IsRelationshipClass() const { return ECClassType::Relationship == GetClassType(); }

    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECRelationshipClassCP GetRelationshipClassCP() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECRelationshipClassP GetRelationshipClassP();

    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECEntityClassCP GetEntityClassCP() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECEntityClassP GetEntityClassP();

    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECCustomAttributeClassCP GetCustomAttributeClassCP() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECCustomAttributeClassP GetCustomAttributeClassP();

    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECStructClassCP GetStructClassCP() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECStructClassP GetStructClassP();

    //! Returns the class modifier
    ECOBJECTS_EXPORT ECClassModifier GetClassModifier() const;
    //! Sets the class modifier
    ECOBJECTS_EXPORT void SetClassModifier(ECClassModifier modifier);

    //! The ECSchema that this class is defined in
    ECOBJECTS_EXPORT ECSchemaCR         GetSchema() const;
    // schemas index class by name so publicly name can not be reset
    //! The name of this ECClass
    ECOBJECTS_EXPORT Utf8StringCR       GetName() const;
    //! {SchemaName}:{ClassName} The pointer will remain valid as long as the ECClass exists.
    ECOBJECTS_EXPORT Utf8CP             GetFullName() const;
    //! Formats the class name for use in an ECSQL statement. ([{SchemaName}].[{ClassName}])
    //! @remarks The pointer will remain valid as long as the ECClass exists.
    ECOBJECTS_EXPORT Utf8StringCR       GetECSqlName() const;
    //! Whether the display label is explicitly defined or not
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;
    //! Returns an iterable of all the ECProperties defined on this class
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties() const;
    //! Returns the number of ECProperties in this class
    ECOBJECTS_EXPORT size_t GetPropertyCount (bool includeBaseProperties = true) const;
    //! Returns a list of the classes this ECClass is derived from
    ECOBJECTS_EXPORT const ECBaseClassesList& GetBaseClasses() const;
    //! Returns a list of the classes that derive from this class.
    ECOBJECTS_EXPORT const ECDerivedClassesList& GetDerivedClasses() const;

    //! Sets the description of this ECClass
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(Utf8StringCR value);
    //! Gets the description of this ECClass.  Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR       GetDescription() const;
    //! Gets the invariant description for this ECClass.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDescription() const;
    //! Sets the display label of this ECClass
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(Utf8StringCR value);
    //! Gets the display label of this ECClass.  If no display label has been set explicitly, it will return the name of the ECClass
    ECOBJECTS_EXPORT Utf8StringCR       GetDisplayLabel() const;
    //! Gets the invariant display label for this ECClass.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDisplayLabel() const;

    //! Returns a list of properties for this class.
    //! @param[in]  includeBaseProperties If true, then will return properties that are contained in this class's base class(es)
    //! @return     An iterable container of ECProperties
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties(bool includeBaseProperties) const;

    //! Adds a base class
    //! You cannot add a base class if it creates a cycle. For example, if A is a base class
    //! of B, and B is a base class of C, you cannot make C a base class of A. Attempting to do
    //! so will return an error. You also can't add a base class to final classes
    //! Note: baseClass must be of same derived class type
    //! @param[in] baseClass The class to derive from
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ECClassCR baseClass);

    //! Adds a base class at either the beginning or end of the base class list
    //! @remarks This method is intended for the rare case where you need to control at which position
    //! the base class is inserted in the list of base classes. This is relevant when you care about
    //! the traversal order of a base class graph. By specification, base classes are traversed in a depth-first
    //! fashion.
    //! You cannot insert a base class if it creates a cycle. For example, if A is a base class
    //! of B, and B is a base class of C, you cannot make C a base class of A. Attempting to do
    //! so will return an error.
    //! Note: baseClass must be of same derived class type
    //! @param[in] baseClass The class to derive from
    //! @param[in] insertAtBeginning true, if @p baseClass is inserted at the beginning of the list. 
    //! @param[in] resolveConflicts if true, will automatically resolve conflicts with property names by renaming the property in the current (and derived) class
    //! false if @p baseClass is added to the end of the list
    ECOBJECTS_EXPORT ECObjectsStatus AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false);
    
    //! Returns whether there are any base classes for this class
    ECOBJECTS_EXPORT bool            HasBaseClasses() const;

    //! Removes a base class.
    ECOBJECTS_EXPORT ECObjectsStatus RemoveBaseClass(ECClassCR baseClass);

    //! Returns true if the class is the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(ECClassCP targetClass) const;

    //! Returns true if the class name  is of the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(Utf8CP name) const;
    //! Returns true if this class matches the specified schema and class name, or is derived from a matching class
    ECOBJECTS_EXPORT bool            Is (Utf8CP schemaName, Utf8CP className) const;

    //! If the given name is valid, creates a primitive property object with the default type of STRING
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name);

    //! If the given name is valid, creates a primitive property object with the given primitive type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates a struct property object using the current class (if a StructClass) as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, Utf8StringCR name);

    //! If the given name is valid, creates a struct property object using the specified class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, Utf8StringCR name, ECStructClassCR structType);

    //! If the given name is valid, creates an array property object using the current class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, Utf8StringCR name);

    //! If the given name is valid, creates an array property object using the specified primitive type as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateArrayProperty(ArrayECPropertyP& ecProperty, Utf8StringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates an array property object using the specified class as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructArrayProperty(StructArrayECPropertyP& ecProperty, Utf8StringCR name, ECStructClassCP structType);

    //! If the given name is valid, creates a primitive property object with the given enumeration type
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerationProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name, ECEnumerationCR enumerationType);

    //! Remove the named property
    //! @param[in] name The name of the property to be removed
    ECOBJECTS_EXPORT ECObjectsStatus RemoveProperty(Utf8StringCR name);

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WCharCP name, bool includeBaseClasses=true) const;

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (Utf8StringCR name, bool includeBaseClasses=true) const;

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (Utf8CP name, bool includeBaseClasses=true) const;
    
    //! Get the property that stores the instance label for the class.
    //! @return A pointer to ECN::ECProperty if the instance label has been specified; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP GetInstanceLabelProperty() const;

    //__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT ECObjectsStatus CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes);
    ECOBJECTS_EXPORT bool IsSingularlyDerivedFrom(ECClassCR baseClass) const;
    //__PUBLISH_SECTION_START__

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! Given a qualified class name, will parse out the schema's namespace prefix and the class name.
    //! @param[out] prefix  The namespace prefix of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedClassName  The qualified name of the class, in the format of ns:className
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName (Utf8StringR prefix, Utf8StringR className, Utf8StringCR qualifiedClassName);

    //! Given a schema and a class, will return the fully qualified class name.  If the class is part of the passed in schema, there
    //! is no namespace prefix.  Otherwise, the class's schema must be a referenced schema in the passed in schema
    //! @param[in]  primarySchema   The schema used to lookup the namespace prefix of the class's schema
    //! @param[in]  ecClass         The class whose schema should be searched for
    //! @return WString    The namespace prefix if the class's schema is not the primarySchema
    ECOBJECTS_EXPORT static Utf8String GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass);

    //! Given two ECClass's, checks to see if they are equal by name
    //! @param[in]  currentBaseClass    The source class to check against
    //! @param[in]  arg                 The target to compare to (this parameter must be an ECClassP)
    ECOBJECTS_EXPORT static bool    ClassesAreEqualByName(ECClassCP currentBaseClass, const void * arg);
}; // ECClass

//=======================================================================================
//! The in-memory representation of an ECEnumeration as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EnumeratorIterable : NonCopyableClass
    {
    friend struct ECEnumeration;
    
    private:
        EnumeratorList const& m_list;
        explicit EnumeratorIterable(EnumeratorList const& list) : m_list(list) {}

    public:
        typedef EnumeratorList::const_iterator const_iterator;
        const_iterator begin() const { return m_list.begin(); }
        const_iterator end() const { return m_list.end(); }
    };

//=======================================================================================
//! The in-memory representation of an ECEnumeration as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECEnumeration : NonCopyableClass
    {
friend struct ECSchema;
friend struct SchemaXmlWriter;
friend struct SchemaXmlReaderImpl;

    private:
        ECSchemaCR m_schema;
        mutable Utf8String m_fullName;
        ECValidatedName m_validatedName;
        PrimitiveType m_primitiveType;
        Utf8String m_description;
        EnumeratorList m_enumeratorList;
        EnumeratorIterable m_enumeratorIterable;
        bool m_isStrict;

        //  Lifecycle management:  The schema implementation will
        //  serve as a factory for enumerations and will manage their lifecycle.
        explicit ECEnumeration(ECSchemaCR schema);
        ~ECEnumeration();

        // schemas index enumeration by name so publicly name can not be reset
        void SetName(Utf8CP name);

        //! Sets the PrimitiveType of this Enumeration.  The default type is ::PRIMITIVETYPE_Integer
        ECObjectsStatus SetType(PrimitiveType value);
        //! Sets the backing primitive type by its name.
        ECObjectsStatus SetTypeName(Utf8CP typeName);

        SchemaReadStatus ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context);
        SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const;

    public:
        //! The ECSchema that this enumeration is defined in
        ECOBJECTS_EXPORT ECSchemaCR GetSchema() const;
        //! The name of this Enumeration
        ECOBJECTS_EXPORT Utf8StringCR GetName() const;
        //! {SchemaName}:{EnumerationName} The pointer will remain valid as long as the ECEnumeration exists.
        ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
        //! Given a schema and an enumeration, will return the fully qualified name.  If the enumeration is part of the passed in schema, there
        //! is no namespace prefix.  Otherwise, the enumeration's schema must be a referenced schema in the passed in schema
        //! @param[in]  primarySchema   The schema used to lookup the namespace prefix of the class's schema
        //! @param[in]  ecEnumeration         The enumeration whose schema should be searched for
        //! @return WString    The namespace prefix if the enumeration's schema is not the primarySchema
        ECOBJECTS_EXPORT static Utf8String GetQualifiedEnumerationName(ECSchemaCR primarySchema, ECEnumerationCR ecEnumeration);
        //! Given a qualified enum name, will parse out the schema's namespace prefix and the enum name.
        //! @param[out] prefix  The namespace prefix of the schema
        //! @param[out] enumName   The name of the enum
        //! @param[in]  qualifiedEnumName  The qualified name of the enum, in the format of ns:enumName
        //! @return A status code indicating whether the qualified name was successfully parsed or not
        ECOBJECTS_EXPORT static ECObjectsStatus ParseEnumerationName(Utf8StringR prefix, Utf8StringR enumName, Utf8StringCR qualifiedEnumName);

        //! Gets the PrimitiveType of this ECEnumeration
        PrimitiveType GetType() const { return m_primitiveType; }
        //! Gets the name of the backing primitive type.
        ECOBJECTS_EXPORT Utf8String GetTypeName() const;

        //! Whether the display label is explicitly defined or not
        ECOBJECTS_EXPORT bool GetIsDisplayLabelDefined() const;
        //! Sets the display label of this ECEnumeration
        ECOBJECTS_EXPORT void SetDisplayLabel(Utf8CP value);
        //! Gets the display label of this ECEnumeration.  If no display label has been set explicitly, it will return the name of the ECEnumeration
        ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
        //! Gets the invariant display label for this ECEnumeration.
        ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const;

        //! Sets the description of this ECEnumeration
        ECOBJECTS_EXPORT void SetDescription(Utf8CP value) { m_description = value; }
        //! Gets the description of this ECEnumeration.  Returns the localized description if one exists.
        ECOBJECTS_EXPORT Utf8StringCR GetDescription() const;
        //! Gets the invariant description for this ECEnumeration.
        ECOBJECTS_EXPORT Utf8StringCR GetInvariantDescription() const;

        //! Gets the IsStrict flag of this enum. True means that values on properties will be enforced.
        bool GetIsStrict() const { return m_isStrict; }
        //! Sets the IsStrict flag to a given value. NonStrict enums will be treated as suggestions and not enforce values.
        void SetIsStrict(bool value) { m_isStrict = value; }

        //!Creates a new enumerator at the end of this enumeration.
        ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, Utf8CP value);
        //!Creates a new enumerator at the end of this enumeration.
        ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, int32_t value);
        //! Finds the enumerator with the provided integer value, returns nullptr if none found.
        ECOBJECTS_EXPORT ECEnumeratorP FindEnumerator(int32_t value) const;
        //! Finds the enumerator with the provided string value, returns nullptr if none found.
        ECOBJECTS_EXPORT ECEnumeratorP FindEnumerator(Utf8CP value) const;
        //! Removes the provided enumerator from this enumeration
        ECOBJECTS_EXPORT ECObjectsStatus DeleteEnumerator(ECEnumeratorCR enumerator);
        //! Removes all enumerators in this enumeration
        ECOBJECTS_EXPORT void Clear();
        //! Get the enumerator list held by this object
        EnumeratorIterable const& GetEnumerators() const { return m_enumeratorIterable; }
        //! Get the amount of enumerators in this enumeration
        size_t GetEnumeratorCount() const { return m_enumeratorList.size(); }
    };

//=======================================================================================
//! The in-memory representation of an ECEnumerator which is a single element in an ECEnumeration
//! @bsiclass
//=======================================================================================
struct ECEnumerator : NonCopyableClass
    {
    friend struct ECEnumeration;

    private:
        ECEnumerationCR m_enum;
        int32_t m_intValue;
        Utf8String m_stringValue;

        mutable Utf8String m_displayLabel;
        bool m_hasExplicitDisplayLabel;

        //  Lifecycle management:  The enumeration implementation will
        //  serve as a factory for enumerators and will manage their lifecycle.
        explicit ECEnumerator(ECEnumerationCR parent, int32_t value) : m_enum(parent), m_intValue(value), m_hasExplicitDisplayLabel(false) {}
        explicit ECEnumerator(ECEnumerationCR parent, Utf8CP value) : m_enum(parent), m_stringValue(value), m_hasExplicitDisplayLabel(false) {}
        ~ECEnumerator() {}

    public:
        //! The ECEnumeration that this enumerator is defined in
        ECEnumerationCR GetEnumeration() const { return m_enum; }
        //! Whether the display label is explicitly defined or not
        bool GetIsDisplayLabelDefined() const { return m_hasExplicitDisplayLabel; }
        //! Sets the display label of this enumerator
        ECOBJECTS_EXPORT void SetDisplayLabel(Utf8CP value);
        //! Gets the display label of this enumerator.  If no display label has been set explicitly, it will return the name of the enumerator
        ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
        //! Gets the invariant display label for this enumerator. This will return nullptr if no label is available, and the value is not a string.
        ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const;

        //!Returns true if this enumerator holds an integer value
        bool IsInteger() const { return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_Integer; }
        //! Returns the integer value, if this ECEnumerator holds an Integer 
        int32_t GetInteger() const { return m_intValue; }
        //! Sets the value of this ECEnumerator to the given integer
        //! @param[in] integer  The value to set
        ECOBJECTS_EXPORT ECObjectsStatus SetInteger(int32_t integer);
        //!Returns true if this enumerator holds an integer value
        bool IsString() const { return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_String; }
        //! Gets the string content of this ECEnumerator in UTF-8 encoding.
        //! @return string content in UTF-8 encoding
        Utf8StringCR GetString() const { return m_stringValue; }
        //! Sets the value of this ECEnumerator to the given string
        //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
        //! @param[in] value The value to set
        ECOBJECTS_EXPORT ECObjectsStatus SetString(Utf8CP value);
    };

//=======================================================================================
//! The in-memory representation of a KindOfQuantity as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct KindOfQuantity : NonCopyableClass
    {
    friend struct ECSchema;
    friend struct SchemaXmlWriter;
    friend struct SchemaXmlReaderImpl;

    private:
        ECSchemaCR m_schema;
        mutable Utf8String m_fullName; //cached nsprefix:name representation
        ECValidatedName m_validatedName; //wraps name and displaylabel
        Utf8String m_description;
        //! Unit used for persisting the information
        Utf8String m_persistenceUnit;
        //! Precision
        uint32_t m_persistencePrecision;
        //! Unit used for presenting the information
        Utf8String m_defaultPresentationUnit;
        //! list of alternative presentation Units
        bvector<Utf8String> m_alternativePresentationUnitList;

        //  Lifecycle management:  The schema implementation will
        //  serve as a factory for kind of quantities and will manage their lifecycle.
        explicit KindOfQuantity(ECSchemaCR schema) : m_schema(schema), m_persistencePrecision(0) {};

        ~KindOfQuantity() {};

        // schemas index KindOfQuantity by name so publicly name can not be reset
        void SetName(Utf8CP name);

        SchemaReadStatus ReadXml(BeXmlNodeR kindOfQuantityNode, ECSchemaReadContextR context);
        SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const;

    public:
        //! The ECSchema that this kind of quantity is defined in
        ECSchemaCR GetSchema() const { return m_schema; }

        //! The name of this Instance
        Utf8StringCR GetName() const { return m_validatedName.GetName(); }

        //! {SchemaName}:{KindOfQuantityName} The pointer will remain valid as long as the KindOfQuantity exists.
        ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
        //! Given a qualified enum name, will parse out the schema's namespace prefix and the kind of quantity name.
        //! @param[out] prefix  The namespace prefix of the schema
        //! @param[out] kindOfQuantityName   The name of the KindOfQuantity
        //! @param[in]  stringToParse  The qualified name, in the format of {SchemaName}:{KindOfQuantityName}
        //! @return A status code indicating whether the qualified name was successfully parsed or not
        ECOBJECTS_EXPORT static ECObjectsStatus ParseName(Utf8StringR prefix, Utf8StringR kindOfQuantityName, Utf8StringCR stringToParse);

        //! Gets a qualified name of the enumeration, prefixed by the schema prefix if it does not match the primary schema.
        ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

        //! Sets the display label of this KindOfQuantity
        //! @param[in]  value  The new value to apply
        ECOBJECTS_EXPORT void SetDisplayLabel(Utf8CP value);
        //! Gets the display label of this KindOfQuantity.  If no display label has been set explicitly, it will return the name of the KindOfQuantity
        ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;

        //! Sets the description of this KindOfQuantity
        //! @param[in]  value  The new value to apply
        void SetDescription(Utf8CP value) { m_description = value; }
        //! Gets the description of this KindOfQuantity.
        Utf8StringCR GetDescription() const { return m_description; };

        //! Sets the Unit of measuerement used for persisting the information
        //! @param[in]  value  The new value to apply
        void SetPersistenceUnit(Utf8CP value) { m_persistenceUnit = value; }
        //! Gets the Unit of measuerement used for persisting the information
        Utf8StringCR GetPersistenceUnit() const { return m_persistenceUnit; };

        //! Sets the Precision used for persisting the information. A precision of zero indicates that a default will be used.
        //! @param[in]  value  The new value to apply
        void SetPrecision(uint32_t value) { m_persistencePrecision = value; }
        //! Gets the precision used for persisting the information. A precision of zero indicates that a default will be used.
        uint32_t GetPrecision() const { return m_persistencePrecision; };
        
        //! Sets the default presentation Unit of this KindOfQuantity
        //! @param[in]  value  The new value to apply
        void SetDefaultPresentationUnit(Utf8CP value) { m_defaultPresentationUnit = value; }
        //! Gets the default presentation Unit of this KindOfQuantity.
        Utf8StringCR GetDefaultPresentationUnit() const { return m_defaultPresentationUnit; };

        //! Gets a list of alternative Unit�s appropriate for presenting quantities on the UI and available for the user selection.
        bvector<Utf8String> const& GetAlternativePresentationUnitList() const { return m_alternativePresentationUnitList; };
        //! Gets an editable list of alternative Unit�s appropriate for presenting quantities on the UI and available for the user selection.
        bvector<Utf8String>& GetAlternativePresentationUnitListR() { return m_alternativePresentationUnitList; };
    };

//---------------------------------------------------------------------------------------
// The in-memory representation of an EntityClass as defined by ECSchemaXML
// @bsiclass                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct EXPORT_VTABLE_ATTRIBUTE ECEntityClass : ECClass
{
DEFINE_T_SUPER(ECClass)

friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECEntityClass (ECSchemaCR schema);
    virtual ~ECEntityClass() {}

    virtual SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const override;
    virtual ECEntityClassCP _GetEntityClassCP() const override {return this;}
    virtual ECEntityClassP _GetEntityClassP() override { return this; }
    virtual ECClassType _GetClassType() const override { return ECClassType::Entity;}
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::EntityClass; }

public:
    //! Creates a navigation property object using the relationship class and direction.  To succeed the relationship class, direction and name must all be valid.
    // @param[out]  ecProperty          Outputs the property if successfully created
    // @param[in]   name                The name for the navigation property.  Must be a valid ECName
    // @param[in]   relationshipClass   The relationship class this navigation property will traverse.  Must list this class as an endpoint constraint.  The cardinality of the other constraint determiness if the nav prop is a primitive or an array.
    // @param[in]   direction           The direction the relationship will be traversed.  Forward indicates that this class is a source constraint, Backward indicates that this class is a target constraint.
    // @param[in]   type                The type of the navigation property.  Should match type used for InstanceIds in the current session.  Default is string.
    // @param[in]   verify              If true the relationshipClass an direction will be verified to ensure the navigation property fits within the relationship constraints.  Default is true.  If not verified at creation the Verify method must be called before the navigation property is used or it's type descriptor will not be valid.
    ECOBJECTS_EXPORT ECObjectsStatus CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, PrimitiveType type = PRIMITIVETYPE_String, bool verify = true);
};

//---------------------------------------------------------------------------------------
// The in-memory representation of a CustomAttributeClass as defined by ECSchemaXML
// @bsiclass                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct EXPORT_VTABLE_ATTRIBUTE ECCustomAttributeClass : public ECClass
{
DEFINE_T_SUPER(ECClass)

friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

private:
    CustomAttributeContainerType m_containerType;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECCustomAttributeClass (ECSchemaCR schema);
    virtual ~ECCustomAttributeClass () {}

protected:
    SchemaReadStatus _ReadXmlAttributes(BeXmlNodeR classNode) override;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const override;
    ECClassType _GetClassType() const override { return ECClassType::CustomAttribute;}
    ECCustomAttributeClassCP _GetCustomAttributeClassCP() const override { return this;}
    ECCustomAttributeClassP _GetCustomAttributeClassP() override { return this; }
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::CustomAttributeClass; }

public:
    CustomAttributeContainerType GetContainerType() const { return m_containerType; }

    //! Sets the container type which this custom attribute can be applied to. Use this carefully as it might render existing instances invalid!
    // @param[in]   containerType   The new container type to apply
    void SetContainerType(CustomAttributeContainerType containerType) { m_containerType = containerType; }

    //! Returns true if the containerType is compatible with the CustomAttributeContainerType of this ECCustomAttributeClass
    //@param[in]    containerType   The type of the container you wish to apply an instance of this class to
    bool CanBeAppliedTo(CustomAttributeContainerType containerType) const { return 0 != static_cast<int>(m_containerType & containerType); } // Compare to 0 instead of containerType so comparisons like Class & Any return true
};

//---------------------------------------------------------------------------------------
// The in-memory representation of a StructClass as defined by ECSchemaXML
// @bsiclass                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct EXPORT_VTABLE_ATTRIBUTE ECStructClass : public ECClass
{
DEFINE_T_SUPER(ECClass)

friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

private:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECStructClass (ECSchemaCR schema);
    virtual ~ECStructClass () {}


protected:
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const override;
    virtual ECClassType _GetClassType() const override { return ECClassType::Struct;}
    virtual ECStructClassCP _GetStructClassCP() const override { return this;}
    virtual ECStructClassP _GetStructClassP() override { return this; }
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::StructClass; }

};


//=======================================================================================
//! This class describes the cardinality of a relationship. It is based on the
//!     Martin notation. Valid cardinalities are (x,y) where x is smaller or equal to y,
//!     x >= 0 and y >= 1 or y = n (where n represents infinity).
//!     For example, (0,1), (1,1), (1,n), (0,n), (1,10), (2,5), ...
//! @bsiclass
//=======================================================================================
struct RelationshipCardinality
{
private:
    uint32_t   m_lowerLimit;
    uint32_t   m_upperLimit;

public:
    //! Default constructor.  Creates a cardinality of (0, 1)
    ECOBJECTS_EXPORT RelationshipCardinality();

    //! Constructor with lower and upper limit parameters.
    //! @param[in]  lowerLimit  must be less than or equal to upperLimit and greater than or equal to 0
    //! @param[in]  upperLimit  must be greater than or equal to lowerLimit and greater than 0
    ECOBJECTS_EXPORT RelationshipCardinality(uint32_t lowerLimit, uint32_t upperLimit);

    //! Returns the lower limit of the cardinality
    ECOBJECTS_EXPORT uint32_t GetLowerLimit() const;
    //! Returns the upper limit of the cardinality
    ECOBJECTS_EXPORT uint32_t GetUpperLimit() const;

    //! Indicates if the cardinality is unbound (ie, upper limit is equal to "n")
    ECOBJECTS_EXPORT bool     IsUpperLimitUnbounded() const;

    //! Converts the cardinality to a string, for example "(0,n)", "(1,1)"
    ECOBJECTS_EXPORT Utf8String ToString() const;

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

    //! Compares the two Cardinalities and returns whether they are equal (0). Otherwise the
    //! larger scope will be returned either rhs (1) or lhs (-1)
    ECOBJECTS_EXPORT static int Compare(RelationshipCardinality const& lhs, RelationshipCardinality const& rhs);
};
//=======================================================================================
//! This class holds a class in an ECRelationship constraint plus its key properties
//! if the constraint class has any.
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECRelationshipConstraintClass : NonCopyableClass
    {
private:
    bvector<Utf8String> m_keys;
    ECEntityClassCP m_ecClass;

public:
#ifndef DOCUMENTATION_GENERATOR 
    explicit ECRelationshipConstraintClass(ECEntityClassCR ecClass);
#endif

    ECRelationshipConstraintClass(ECRelationshipConstraintClass&& rhs);
    ECRelationshipConstraintClass& operator= (ECRelationshipConstraintClass&& rhs);
    
    //! Gets the constraint's ECClass
    ECEntityClassCR GetClass() const { return *m_ecClass; }
    //! Gets the constraint's key property names
    bvector<Utf8String> const& GetKeys() const { return m_keys; }
    //! Adds name of key property.
    //! @param[in] keyPropertyName Name of key property to add
    ECOBJECTS_EXPORT void AddKey(Utf8CP keyPropertyName);

    //! Removes all Keys from the constraint
    ECOBJECTS_EXPORT void DropAllKeys() { m_keys.clear(); }
    };

//=======================================================================================
//! This class holds the list of source or target Constraint on ECRelationships
//! @bsiclass
//=======================================================================================
struct ECRelationshipConstraintClassList : NonCopyableClass
    {
    //__PUBLISH_SECTION_END__

    private:
    std::vector<std::unique_ptr<ECRelationshipConstraintClass>> m_constraintClasses;
    //__PUBLISH_SECTION_START__
    
    public:
    struct iterator
        {
        friend struct ECRelationshipConstraintClassList;// TODO: specify begin and end functions;

        public:
            struct Impl;

        private:
            Impl *m_pimpl;
        //__PUBLISH_SECTION_END__

        private:
            iterator(std::vector<std::unique_ptr<ECRelationshipConstraintClass>>::const_iterator x);
        //__PUBLISH_SECTION_START__

        public:
            iterator(const iterator &);
            iterator& operator=(const iterator & rhs);
            iterator();

        public:
            ECOBJECTS_EXPORT ECRelationshipConstraintClassCP operator->()const; //!< Returns the value at the current location
            ECOBJECTS_EXPORT iterator&                           operator++(); //!< Increments the iterator
            ECOBJECTS_EXPORT bool                                operator!=(iterator const& rhs) const; //!< Checks for inequality
            ECOBJECTS_EXPORT bool                                operator==(iterator const& rhs) const; //!< Checks for equality
            ECOBJECTS_EXPORT ECRelationshipConstraintClassCP     operator* () const; //!< Returns the value at the current location
            ECOBJECTS_EXPORT ~iterator();
        };
    //__PUBLISH_SECTION_END__
    private:

        ECRelationshipClassP m_relClass;
   //__PUBLISH_SECTION_START__
    public:
        ECRelationshipConstraintClassList(ECRelationshipClassP relClass);
        ECOBJECTS_EXPORT iterator begin() const;    //!< Returns the beginning of the iterator
        ECOBJECTS_EXPORT iterator end() const;      //!< Returns the end of the iterator
        ECOBJECTS_EXPORT ECRelationshipConstraintClassCP operator[](size_t x)const; //!< Array operator overloaded
        //! Adds the specified class to the constraint.
        //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
        //! will replace the current class applied to the constraint with the new class.
        //! @param[out] classConstraint ECRelationshipConstraintClass for current ECClass
        //! @param[in] ecClass  The class to add
        ECOBJECTS_EXPORT ECObjectsStatus            Add(ECRelationshipConstraintClass*& classConstraint, ECEntityClassCR ecClass);
        //! Clears the vector Constraint classes
        ECOBJECTS_EXPORT ECObjectsStatus            clear();
        //! Clears the vector Constraint classes
        ECOBJECTS_EXPORT uint32_t            size()const;
        //! Removes specified ECClass from Constraint class vector
        ECOBJECTS_EXPORT ECObjectsStatus            Remove(ECEntityClassCR);
        ~ECRelationshipConstraintClassList();
           
    };


//=======================================================================================
//! The in-memory representation of the source and target constraints for an ECRelationshipClass as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECRelationshipConstraint : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
friend struct ECRelationshipClass;

private:
    // NEEDSWORK: To be completely compatible, we need to store an ECRelationshipConstraintClass with properties in order
    // to support implicit relationships.  For now, just support explicit relationships
//    stdext::hash_map<ECClassCP, ECRelationshipConstrainClassCP> m_constraintClasses;

    ECRelationshipConstraintClassList    m_constraintClasses;

    Utf8String                  m_roleLabel;
    bool                        m_isPolymorphic;
    RelationshipCardinality*    m_cardinality;
    ECRelationshipClassP        m_relClass;
    bool                        m_isSource;

    ECObjectsStatus             SetCardinality(uint32_t& lowerLimit, uint32_t& upperLimit);

    SchemaWriteStatus           WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName) const;
    SchemaReadStatus            ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor);

    ECObjectsStatus             ValidateClassConstraint(ECEntityClassCR constraintClass) const;
    ECObjectsStatus             ValidateCardinalityConstraint(uint32_t& lowerLimit, uint32_t& upperLimit) const;

protected:
    virtual ECSchemaCP          _GetContainerSchema() const override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return m_isSource ? CustomAttributeContainerType::SourceRelationshipConstraint : CustomAttributeContainerType::TargetRelationshipConstraint; }

    //! Initializes a new instance of the ECRelationshipConstraint class
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isSource); // WIP_CEM... should not be public... create a factory method

/*__PUBLISH_SECTION_START__*/
public:
 
    ECOBJECTS_EXPORT virtual ~ECRelationshipConstraint(); //!< Destructor

    //! Sets the label of the constraint role in the relationship.
    ECOBJECTS_EXPORT ECObjectsStatus            SetRoleLabel (Utf8StringCR value);
    //! Gets the label of the constraint role in the relationship.
    //! If the role label is not defined, the display label of the relationship class is returned
    ECOBJECTS_EXPORT Utf8String const           GetRoleLabel() const;
    //! Gets the invariant label of the constraint role in the relationship.
    //! If the role label is not defined, the invariant display label of the relationship class is returned
    ECOBJECTS_EXPORT Utf8String const           GetInvariantRoleLabel() const;

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
    ECOBJECTS_EXPORT ECObjectsStatus            SetIsPolymorphic(Utf8CP isPolymorphic);

    //! Sets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus            SetCardinality(RelationshipCardinalityCR value);
    //! Sets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus            SetCardinality(Utf8CP cardinality);

    //! Gets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT RelationshipCardinalityCR  GetCardinality() const;

    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] classConstraint  The class to add
    ECOBJECTS_EXPORT ECObjectsStatus            AddClass(ECEntityClassCR classConstraint);
    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @note Only Entity classes are allowed as constraints.
    //! @param[in] ecClass  The class to add
    //! @param[out] classConstraint  list of constraint classes
    ECOBJECTS_EXPORT ECObjectsStatus            AddConstraintClass(ECRelationshipConstraintClass*& classConstraint, ECEntityClassCR ecClass);

    //! Removes the specified class from the constraint.
    //! @param[in] classConstraint  The class to remove
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveClass(ECEntityClassCR classConstraint);

    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT const ECConstraintClassesList GetClasses() const;

    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT ECRelationshipConstraintClassList const & GetConstraintClasses() const;

    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT ECRelationshipConstraintClassList& GetConstraintClassesR();

    ECOBJECTS_EXPORT bool SupportsClass(ECClassCR ecClass) const;
    
    //! Copies this constraint to the destination
    ECOBJECTS_EXPORT ECObjectsStatus            CopyTo(ECRelationshipConstraintR toRelationshipConstraint);

    //! Returns whether the relationship is ordered on this constraint.
    ECOBJECTS_EXPORT bool                       GetIsOrdered () const;

    //! Returns the storage mode of the OrderId for this constraint.
    ECOBJECTS_EXPORT OrderIdStorageMode         GetOrderIdStorageMode () const;

    //! Gets the name of the OrderId property for this constraint.
    ECOBJECTS_EXPORT ECObjectsStatus            GetOrderedRelationshipPropertyName (Utf8String& propertyName)  const;
};

//=======================================================================================
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECRelationshipClass : public ECEntityClass
{
DEFINE_T_SUPER(ECEntityClass)
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

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

    ECObjectsStatus                     SetStrength (Utf8CP strength);
    ECObjectsStatus                     SetStrengthDirection (Utf8CP direction);

    bool                                ValidateStrengthConstraint(StrengthType value, bool compareValue=true) const;
    bool                                ValidateStrengthDirectionConstraint(ECRelatedInstanceDirection value, bool compareValue = true) const;

protected:
    virtual SchemaWriteStatus           _WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const override;

    virtual SchemaReadStatus            _ReadXmlAttributes (BeXmlNodeR classNode) override;
    virtual SchemaReadStatus            _ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, int ecXmlVersionMajor, bvector<NavigationECPropertyP>& navigationProperties) override;
    virtual ECRelationshipClassCP       _GetRelationshipClassCP () const override {return this;};
    virtual ECRelationshipClassP        _GetRelationshipClassP ()  override {return this;};
    virtual ECClassType                 _GetClassType() const override { return ECClassType::Relationship; }
    virtual ECObjectsStatus             _AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false) override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::RelationshipClass; }

//__PUBLISH_SECTION_START__
public:
    //! Returns pointer to ECRelationshipClassP,  used to avoid dynamic_cast.
    //! @return     Returns NULL if not an ECRelationshipClass
    ECOBJECTS_EXPORT ECObjectsStatus            GetOrderedRelationshipPropertyName (Utf8String& propertyName, ECRelationshipEnd end)  const;
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

//! Defines what sort of match should be used when locating a schema
enum class SchemaMatchType
    {
    //! Find exact VersionMajor, VersionWrite, VersionMinor match as well as Data
    Identical,
    //! Find exact VersionMajor, VersionWrite, VersionMinor match.
    Exact,
    //! Find latest version with matching VersionMajor and VersionWrite
    LatestCompatible,
    //! Find latest version.
    Latest,
    //! Find latest version with matching VersionMajor
    LatestMajorCompatible,
    };

//=======================================================================================
//! Fully defines a schema with its name, major and minor versions, and a checksum
//! 
//! The following table shows how schema version changes over time
//! 
//! |                             | Changes Write Compatible | Changes Read Compatible | Version Number |
//! |-----------------------------|--------------------------|-------------------------|----------------|
//! | Initial schema release      | N/A                      | N/A                     | 1.0.0          |
//! | Additions to schema         | Yes                      | Yes                     | 1.0.1          |
//! | Additions to schema         | Yes                      | Yes                     | 1.0.2          |
//! | Additions to schema         | No                       | Yes                     | 1.1.0          |
//! | Additions to schema         | Yes                      | Yes                     | 1.1.1          |
//! | Additions to schema         | Yes                      | Yes                     | 1.1.2          |
//! | Additions to schema         | Yes                      | Yes                     | 1.1.3          |
//! | Additions to schema         | Yes                      | Yes                     | 1.2.0          |
//! | Significant schema revision | No                       | No                      | 2.0.0          |
//! 
//! The general logic for an application written for a particular version of a schema working with a repository that potentially
//! has a different version of the schema would be:
//!   -	If schema in the repository is newer (or same):
//!       -	If first digit matches, app can safely read
//!       -	If first two digits match, app can safely write (and read)
//!   -	If schema in the repository is older:
//!       -	If first two digits match, app can upgrade repository schema without breaking read or write for other apps
//!       -	If only first digit matches, app can upgrade repository, but upgrade will prevent some older apps from writing
//! 
//! For traditional EC developers it may be difficult to envision when a schema change would require a change
//! to the middle version number. Consider in schema 1 that we have a Student class that stores grades and has (double) properties:
//!   -	Language
//!   -	Math
//!   -	Science
//!   -	Music
//!   -	Overall GPA (an average of the previous 4 properties)
//! 
//! If schema 2 adds to Student a double property Psychology, the meaning of Overall GPA changes slightly and hence, applications written for Schema 1:
//!   -	Can still safely read all the values that were in schema 1
//!   -	Cannot modify any values that were in schema 1 because they will likely set Overall GPA incorrectly.
//! 
// @bsiclass                                                
//=======================================================================================
struct SchemaKey
    {
    Utf8String    m_schemaName;
    uint32_t      m_versionMajor;
    uint32_t      m_versionWrite;
    uint32_t      m_versionMinor;
    uint32_t      m_checkSum;

    //! Creates a new SchemaKey with the given name and version information
    //! @param[in]  name    The name of the ECSchema
    //! @param[in]  major   The major portion of the version
    //! @param[in]  minor   The minor portion of the version
    SchemaKey (Utf8CP name, uint32_t major, uint32_t minor) : m_schemaName(name), m_versionMajor(major), m_versionWrite(DEFAULT_VERSION_WRITE), m_versionMinor(minor), m_checkSum(0){}

    //! Creates a new SchemaKey with the given name and version information
    //! @param[in]  name    The name of the ECSchema
    //! @param[in]  major   The major portion of the version
    //! @param[in]  write   The  write compatibility version
    //! @param[in]  minor   The minor portion of the version
    SchemaKey(Utf8CP name, uint32_t major, uint32_t write, uint32_t minor) : m_schemaName(name), m_versionMajor(major), m_versionWrite(write), m_versionMinor(minor), m_checkSum(0) {}

    //! Default constructor
    SchemaKey () : m_versionMajor(DEFAULT_VERSION_MAJOR), m_versionWrite(DEFAULT_VERSION_WRITE), m_versionMinor(DEFAULT_VERSION_MINOR), m_checkSum(0) {}

    Utf8StringCR GetName() const { return m_schemaName; }

    //! Gets the major schema version. Identifies the generation of the schema that guarantees that newer schemas can be
    //! read by older software.
    //! Example: 
    uint32_t GetVersionMajor() const { return m_versionMajor; };

    //! Gets the major for write version. This is less significant than the major version. It identifies the generation of the schema
    //! that guarantees that newer schemas can be written by older software.
    uint32_t GetVersionWrite() const { return m_versionWrite; };

    //! Least significant version number that increments with read/write compatible additions.
    uint32_t GetVersionMinor() const { return m_versionMinor; };

    //! Given a full schema name (which includes the version information), will return a SchemaKey with the schema name and version information set
    //! @param[out] key             A SchemaKey with the schema's name and version set
    //! @param[in]  schemaFullName  The full name of the schema.
    static ECObjectsStatus ParseSchemaFullName (SchemaKey& key, Utf8CP schemaFullName) { return ParseSchemaFullName(key.m_schemaName, key.m_versionMajor, key.m_versionWrite, key.m_versionMinor, schemaFullName); }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionWrite   Write compatibility version. Will default to zero if the string only contains two numbers.
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString(uint32_t& versionMajor, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionWrite The write compatibility version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName(Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP fullName);

    //! Return full schema name in format GetName().MM.ww.mm where Name is the schema name MM is major version,ww is the  write compatibility version and mm is minor version.
    Utf8String GetFullSchemaName() const { return FormatFullSchemaName(m_schemaName.c_str(), m_versionMajor, m_versionWrite, m_versionMinor); }

    //! Generate a schema full name string given the major,  write compatibility and minor version values.
    //! @param[in] schemaName      Name of the schema
    //! @param[in] versionMajor    The major version number
    //! @param[out] versionWrite  The  write compatibility version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatFullSchemaName(Utf8CP schemaName, uint32_t versionMajor, uint32_t versionWrite, uint32_t versionMinor);

    //! Generate a legacy schema full name, which does not contain the  write compatibility version.
    //! @param[in] schemaName      Name of the schema
    //! @param[in] versionMajor    The major version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatLegacyFullSchemaName(Utf8CP schemaName, uint32_t versionMajor, uint32_t versionMinor);

    //! Generate a schema version string given the major write and minor version values.
    //! @param[in] versionMajor    The major version number
    //! @param[out] versionWrite The  write compatibility version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatSchemaVersion(uint32_t versionMajor, uint32_t versionWrite, uint32_t versionMinor);

    //! Generate a legacy schema version string given the major and minor version values.
    //! @param[in] versionMajor    The major version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatLegacySchemaVersion(uint32_t versionMajor, uint32_t versionMinor);

    //! Generate a legacy schema version string given the major write and minor version values.
    Utf8String GetVersionString() const { return FormatSchemaVersion(m_versionMajor, m_versionWrite, m_versionMinor); }

    //! Generate a legacy schema version string given the major and minor version values.
    Utf8String GetLegacyVersionString() const { return FormatLegacySchemaVersion(m_versionMajor, m_versionMinor); }

    //! Compares two SchemaKeys and returns whether the target schema is less than this SchemaKey, where LessThan is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SchemaMatchType::Identical - Returns whether the current schema's checksum is less than the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SchemaMatchType::Exact - This will first test the names, then the major version, and lastly the minor version
    //! @li SchemaMatchType::LatestCompatible - This will first test the names and then the major versions.
    //! @li SchemaMatchType::Latest - Returns whether the current schema's name is less than the target's.
    ECOBJECTS_EXPORT bool LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const;
    
    //! Compares two SchemaKeys and returns whether the target schema matches this SchemaKey, where "matches" is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SchemaMatchType::Identical - Returns whether the current schema's checksum is equal to the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SchemaMatchType::Exact - Returns whether this schema's name, major version, and minor version are all equal to the target's.
    //! @li SchemaMatchType::LatestCompatible - Returns whether this schema's name and major version are equal, and this schema's minor version is greater than or equal to the target's.
    //! @li SchemaMatchType::Latest - Returns whether the current schema's name is equal to the target's.
    ECOBJECTS_EXPORT bool Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const;

    //! Compares two schema names and returns whether the target schema matches this m_schemaName. Comparison is case-sensitive
    //! @param[in]  schemaName  The schema name to compare to
    ECOBJECTS_EXPORT int CompareByName(Utf8StringCR schemaName) const;

    //! Compares the schema version of this key to the schema version of @p rhs.
    //! @param[in] rhs Right-hand side schema key
    //! @return a negative number if the LHS version is less than the RHS version, a positive number if LHS version is
    //! greater than the RHS version, 0 if the versions are the same.
    ECOBJECTS_EXPORT int CompareByVersion(SchemaKeyCR rhs) const;

    //! Returns whether this SchemaKey is Identical to the target SchemaKey
    bool operator == (SchemaKeyCR rhs) const { return Matches(rhs, SchemaMatchType::Identical); }

    //! Returns true if the target SchemaKey is not Identical to this SchemaKey, false otherwise
    bool operator != (SchemaKeyCR rhs) const { return !(*this == rhs); }

    //! Returns whether this SchemaKey's checksum is less than the target SchemaKey's.
    bool operator < (SchemaKeyCR rhs) const { return LessThan (rhs, SchemaMatchType::Identical); }
    };

//---------------------------------------------------------------------------------------
//! Determines whether two SchemaKeys match
//! @ingroup ECObjectsGroup
//+---------------+---------------+---------------+---------------+---------------+------
template <SchemaMatchType MatchType>
struct SchemaKeyMatch : std::binary_function<SchemaKey, SchemaKey, bool>
    {
    //! Determines whether two SchemaKeys match
    bool operator () (SchemaKeyCR lhs, SchemaKeyCR rhs) const
        {
        return lhs.Matches (rhs, MatchType);
        }
    };

//---------------------------------------------------------------------------------------
//! Determines whether one SchemaKey is less than the other
//! @ingroup ECObjectsGroup
//+---------------+---------------+---------------+---------------+---------------+------
template <SchemaMatchType MatchType>
struct SchemaKeyLessThan : std::binary_function<SchemaKey, SchemaKey, bool>
    {
    //! Determines whether one SchemaKey is less than the other
    bool operator () (SchemaKeyCR lhs, SchemaKeyCR rhs) const
        {
        return lhs.LessThan (rhs, MatchType);
        }
    };


typedef bmap<SchemaKey , ECSchemaPtr> SchemaMap;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaKeyMatchPredicate
    {
    SchemaKeyCR     m_key;
    SchemaMatchType m_matchType;

    //! Constructs a SchemaKeyMatchPredicate
    //! @param[in]      key         The key to compare against
    //! @param[in]      matchType   The type of matching to perform for comparisons
    SchemaKeyMatchPredicate(SchemaKeyCR key, SchemaMatchType matchType) :m_key(key), m_matchType(matchType) {}

    typedef bpair<SchemaKey, ECSchemaPtr> MapVal;

    //! Performs comparison against a MapVal
    //! @return true if this SchemaKeyMatchPredicate is equivalent to the specified MapVal
    bool operator () (MapVal const& rhs)
        {
        return rhs.first.Matches (m_key, m_matchType);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaNameClassNamePair
    {
public:
    Utf8String m_schemaName;
    Utf8String m_className;

    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair (Utf8StringCR schemaName, Utf8StringCR className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair (Utf8CP schemaName, Utf8CP className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs an empty SchemaNameClassNamePair
    SchemaNameClassNamePair() { }
    //! Constructs a SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    SchemaNameClassNamePair (Utf8StringCR schemaAndClassNameSeparatedByColon)
        {
        BeAssert(Utf8String::npos != schemaAndClassNameSeparatedByColon.find (':'));
        Parse (schemaAndClassNameSeparatedByColon);
        }

    //! Attempts to populate this SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    //! @param[in]      schemaAndClassNameSeparatedByColon a string of the format "SCHEMANAME:CLASSNAME"
    //! @return true if the string was successfully parsed, false otherwise. If it returns false, this SchemaNameClassNamePair will not be modified.
    bool Parse (Utf8StringCR schemaAndClassNameSeparatedByColon)
        {
        size_t pos = schemaAndClassNameSeparatedByColon.find (':');
        if (Utf8String::npos != pos)
            {
            m_schemaName = schemaAndClassNameSeparatedByColon.substr (0, pos);
            m_className = schemaAndClassNameSeparatedByColon.substr (pos+1);
            return true;
            }
        else
            return false;
        }

    //! Performs a less-than comparison against another SchemaNameClassNamePair
    //! @param[in]      other The SchemaNameClassNamePair against which to compare
    //! @return true if this SchemaNameClassNamePair is considered "less than" the specified SchemaNameClassNamePair
    bool operator<(SchemaNameClassNamePair other) const
        {
        if (m_schemaName < other.m_schemaName)
            return true;

        if (m_schemaName > other.m_schemaName)
            return false;

        return m_className < other.m_className;
        };

    //! Performs equality comparison against another SchemaNameClassNamePair
    //! @param[in]      rhs The SchemaNameClassNamePair against which to compare
    //! @return true if the SchemaNameClassNamePairs are equivalent
    bool operator==(SchemaNameClassNamePair const& rhs) const
        {
        return 0 == m_schemaName.CompareTo(rhs.m_schemaName) && 0 == m_className.CompareTo(rhs.m_className);
        }

    //! Concatenates the schema and class names into a single colon-separated string of the format "SCHEMANAME:CLASSNAME"
    //! @return a string of the format "SCHEMANAME:CLASSNAME"
    Utf8String     ToColonSeparatedString() const
        {
        Utf8String str;
        str.Sprintf ("%s:%s", m_schemaName.c_str(), m_className.c_str());
        return str;
        }

//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT bool   Remap (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper);
//__PUBLISH_SECTION_START__
    };

/*---------------------------------------------------------------------------------**//**
* Identifies an ECProperty by schema name, class name, and access string. The class name
* may refer to the ECClass containing the ECProperty, or a subclass thereof.
* @bsistruct                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct QualifiedECAccessor
    {
protected:
    Utf8String         m_schemaName;
    Utf8String         m_className;
    Utf8String         m_accessString;
public:
    //! Constructs an empty QualifiedECAccessor
    QualifiedECAccessor() { }
    //! Constructs a QualifiedECAccessor referring to a property of an ECClass specified by access string
    QualifiedECAccessor (Utf8CP schemaName, Utf8CP className, Utf8CP accessString)
        : m_schemaName(schemaName), m_className(className), m_accessString(accessString) { }

    //! Returns the name of the schema containing the ECClass
    Utf8CP GetSchemaName() const           { return m_schemaName.c_str(); }
    //! Returns the name of the ECClass (or subclass thereof) containing the ECProperty
    Utf8CP     GetClassName() const            { return m_className.c_str(); }
    //! Returns the access string identifying the ECProperty within the ECClass
    Utf8CP     GetAccessString() const         { return m_accessString.c_str(); }

    //! Sets the name of the schema containing the ECClass
    void        SetSchemaName (Utf8CP name)    { m_schemaName = name; }
    //! Sets the name of the ECClass (or subclass thereof) containing the ECProperty
    void        SetClassName (Utf8CP name)     { m_className = name; }
    //! Sets the access string identifying the ECProperty within the ECClass
    void        SetAccessString (Utf8CP acStr) { m_accessString = acStr; }

    //! Returns a colon-separated string of the format "Schema:Class:AccessString"
    ECOBJECTS_EXPORT Utf8String    ToString() const;
    //! Attempts to initialize this QualifiedECAccessor from a colon-separated string. If the string cannot be parsed, this QualifiedECAccessor will not be modified
    //! @param[in]      str A string of the format "Schema:Class:AccessString"
    //! @return true if the string was successfully parsed
    ECOBJECTS_EXPORT bool       FromString (Utf8CP str);

    //! Attempts to initialize this QualifiedECAccessor from an access string identifying a property of the specified ECEnabler.
    //! If the ECProperty cannot be found within the ECEnabler, this QualifiedECAccessor will not be modified
    //! @param[in]      rootEnabler  The ECEnabler containing the desired ECProperty
    //! @param[in]      accessString The access string identifying the ECProperty within the ECEnabler
    //! @return true if the access string identifies a valid ECProperty within the ECEnabler
    ECOBJECTS_EXPORT bool       FromAccessString (ECN::ECEnablerCR rootEnabler, Utf8CP accessString);
//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT bool       Remap (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper);

    Utf8StringR                    GetSchemaNameR()    { return m_schemaName; }
    Utf8StringR                    GetClassNameR()     { return m_className; }
    Utf8StringR                    GetAccessStringR()  { return m_accessString; }
//__PUBLISH_SECTION_START__
    };

typedef bvector<QualifiedECAccessor> QualifiedECAccessorList;
typedef QualifiedECAccessorList const& QualifiedECAccessorListCR;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaMapExact:bmap<SchemaKey, ECSchemaPtr, SchemaKeyLessThan <SchemaMatchType::Exact> >
    {
    //! Returns an iterator to the entry in this map matching the specified key using the specified match type, or an iterator to the end of this map if no such entry exists
    SchemaMapExact::const_iterator Find (SchemaKeyCR key, SchemaMatchType matchType) const
        {
        switch (matchType)
            {
            case SchemaMatchType::Exact:
                return find(key);
            default:
                return std::find_if(begin(), end(), SchemaKeyMatchPredicate(key, matchType));
            }
        }

    //! Get a class by name within the context of this list.
    //! @param[in]  classNamePair     The name of the class and schema to lookup.  This must be an unqualified (short) class name.
    //! @return   A pointer to an ECN::ECClass if the named class exists in within the current list; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP  FindClassP (ECN::SchemaNameClassNamePair const& classNamePair) const;
    };

typedef SchemaMapExact                  ECSchemaReferenceList;
typedef const ECSchemaReferenceList&    ECSchemaReferenceListCR;

//=======================================================================================
//! Supports STL like iterator of classes in a schema
//! @bsiclass
//=======================================================================================
struct ECClassContainer
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECSchema;
    friend struct ECClass;
    friend struct ECRelationshipConstraint;

    ClassMap const&     m_classMap;

public:
    ECOBJECTS_EXPORT ECClassContainer (ClassMap const& classMap) : m_classMap (classMap) {}; //public for test purposes only

//__PUBLISH_SECTION_START__
public:
    //=======================================================================================
    // @bsistruct
    //=======================================================================================
    struct IteratorState : RefCountedBase
        {
        friend struct const_iterator;
/*__PUBLISH_SECTION_END__*/
        public:
            ClassMap::const_iterator     m_mapIterator;

            IteratorState (ClassMap::const_iterator mapIterator) { m_mapIterator = mapIterator; };
            static RefCountedPtr<IteratorState> Create (ClassMap::const_iterator mapIterator) { return new IteratorState(mapIterator); };
//__PUBLISH_SECTION_START__
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
        const_iterator (char* ) {;} // must publish at least one private constructor to prevent instantiation

    public:
        ECOBJECTS_EXPORT const_iterator&     operator++(); //!< Increments the iterator
        ECOBJECTS_EXPORT bool                operator!=(const_iterator const& rhs) const; //!< Checks for inequality
        ECOBJECTS_EXPORT bool                operator==(const_iterator const& rhs) const; //!< Checks for equality
        ECOBJECTS_EXPORT ECClassP const&     operator* () const; //!< Returns the value at the current location
    };

public:
    ECOBJECTS_EXPORT const_iterator begin () const; //!< Returns the beginning of the iterator
    ECOBJECTS_EXPORT const_iterator end ()   const; //!< Returns the end of the iterator

};

//=======================================================================================
//! Supports STL like iterator of enumerations in a schema
//! @bsiclass
//=======================================================================================
struct ECEnumerationContainer
    {
    private:
        friend struct ECSchema;
        friend struct ECEnumeration;

        EnumerationMap const&     m_enumerationMap;
        ECEnumerationContainer(EnumerationMap const& enumerationMap) : m_enumerationMap(enumerationMap) {}; //public for test purposes only

    public:
        //=======================================================================================
        // @bsistruct
        //=======================================================================================
        struct IteratorState : RefCountedBase
            {
            friend struct const_iterator;
            public:
                EnumerationMap::const_iterator     m_mapIterator;

                IteratorState(EnumerationMap::const_iterator mapIterator) { m_mapIterator = mapIterator; };
                static RefCountedPtr<IteratorState> Create(EnumerationMap::const_iterator mapIterator) { return new IteratorState(mapIterator); };
            };

        //=======================================================================================
        // @bsistruct
        //=======================================================================================
        struct const_iterator : std::iterator<std::forward_iterator_tag, ECEnumerationP const>
            {
            private:
                friend struct ECEnumerationContainer;
                RefCountedPtr<IteratorState>   m_state;

                const_iterator(EnumerationMap::const_iterator mapIterator) { m_state = IteratorState::Create(mapIterator); };
                const_iterator(char*) { ; } // must publish at least one private constructor to prevent instantiation

            public:
                ECOBJECTS_EXPORT const_iterator&       operator++(); //!< Increments the iterator
                ECOBJECTS_EXPORT bool                  operator!=(const_iterator const& rhs) const; //!< Checks for inequality
                ECOBJECTS_EXPORT bool                  operator==(const_iterator const& rhs) const; //!< Checks for equality
                ECOBJECTS_EXPORT ECEnumerationP const& operator* () const; //!< Returns the value at the current location
            };

    public:
        const_iterator begin() const { return ECEnumerationContainer::const_iterator(m_enumerationMap.begin()); } //!< Returns the beginning of the iterator
        const_iterator end()   const { return ECEnumerationContainer::const_iterator(m_enumerationMap.end()); } //!< Returns the end of the iterator
    };

//=======================================================================================
//! Supports STL like iterator of kind of quantities in a schema
//! @bsiclass
//=======================================================================================
struct KindOfQuantityContainer
    {
    private:
        friend struct ECSchema;
        friend struct KindOfQuantity;

        KindOfQuantityMap const&     m_koqMap;
        KindOfQuantityContainer(KindOfQuantityMap const& koqMap) : m_koqMap(koqMap) {}; //public for test purposes only

    public:
        //=======================================================================================
        // @bsistruct
        //=======================================================================================
        struct IteratorState : RefCountedBase
            {
            friend struct const_iterator;
            public:
                KindOfQuantityMap::const_iterator     m_mapIterator;

                IteratorState(KindOfQuantityMap::const_iterator mapIterator) { m_mapIterator = mapIterator; };
                static RefCountedPtr<IteratorState> Create(KindOfQuantityMap::const_iterator mapIterator) { return new IteratorState(mapIterator); };
            };

        //=======================================================================================
        // @bsistruct
        //=======================================================================================
        struct const_iterator : std::iterator<std::forward_iterator_tag, KindOfQuantityP const>
            {
            private:
                friend struct KindOfQuantityContainer;
                RefCountedPtr<IteratorState>   m_state;

                const_iterator(KindOfQuantityMap::const_iterator mapIterator) { m_state = IteratorState::Create(mapIterator); };
                const_iterator(char*) { ; } // must publish at least one private constructor to prevent instantiation

            public:
                ECOBJECTS_EXPORT const_iterator&       operator++(); //!< Increments the iterator
                ECOBJECTS_EXPORT bool                  operator!=(const_iterator const& rhs) const; //!< Checks for inequality
                ECOBJECTS_EXPORT bool                  operator==(const_iterator const& rhs) const; //!< Checks for equality
                ECOBJECTS_EXPORT KindOfQuantityP const& operator* () const; //!< Returns the value at the current location
            };

    public:
        const_iterator begin() const { return KindOfQuantityContainer::const_iterator(m_koqMap.begin()); } //!< Returns the beginning of the iterator
        const_iterator end()   const { return KindOfQuantityContainer::const_iterator(m_koqMap.end()); } //!< Returns the end of the iterator
    };

//=======================================================================================
//! Interface to find a standalone enabler, typically for an embedded ECStruct in an ECInstance.
//! @bsiclass
//=======================================================================================
struct IStandaloneEnablerLocater
{
private:
    DECLARE_KEY_METHOD

/*__PUBLISH_SECTION_END__*/
protected:
    virtual    StandaloneECEnablerPtr  _LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className) = 0;

/*__PUBLISH_SECTION_START__*/

public:
    //! Given a SchemaKey and a className, tries to locate the StandaloneEnabler for the ECClass
    //! @param[in] schemaKey    SchemaKey fully describing the schema that the class belongs to
    //! @param[in] className    The name of the class to find the enabler for
    //! @returns A valid StandaloneECEnabler, if one was located
    ECOBJECTS_EXPORT StandaloneECEnablerPtr  LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className);
};


//=======================================================================================
//! Interface implemented by class that provides schema location services.
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECSchemaLocater
{
protected:
    //! Tries to locate the requested schema.
    virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) = 0;

public:
    virtual ~IECSchemaLocater() {}

    //! Tries to locate the requested schema.
    //! @param[in] key  The SchemaKey fully describing the schema to locate
    //! @param[in] matchType    The SchemaMatchType defining how exact of a match for the located schema is tolerated
    //! @param[in] schemaContext    Contains the information of where to look for referenced schemas
    //! @returns A valid ECSchemaPtr if the schema was located
    ECOBJECTS_EXPORT ECSchemaPtr LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext);
};

typedef RefCountedPtr<ECSchemaCache>        ECSchemaCachePtr;
//=======================================================================================
//! An object that controls the lifetime of a set of ECSchemas.  When the schema
//! owner is destroyed, so are the schemas that it owns.
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECSchemaCache : public RefCountedBase
//__PUBLISH_SECTION_END__
    ,public IECSchemaLocater
//__PUBLISH_SECTION_START__
{
/*__PUBLISH_SECTION_END__*/
protected:
    SchemaMap   m_schemas;

    // TODO: Uncomment this and remove the public desctructor once ECDb stops declaring this on the stack.
    // ECSchemaCache() {}
    // ECOBJECTS_EXPORT virtual ~ECSchemaCache ();

    ECOBJECTS_EXPORT virtual ECSchemaPtr     _LocateSchema (SchemaKeyR schema, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;
public:
    ECObjectsStatus DropAllReferencesOfSchema(ECSchemaR schema);
//__PUBLISH_SECTION_START__
public:
    //! Adds a schema to the cache
    //! @param[in] schema   The ECSchema to add to the cache
    //! @returns ECObjectsStatus::DuplicateSchema is the schema is already in the cache, otherwise ECObjectsStatus::Success
    ECOBJECTS_EXPORT ECObjectsStatus AddSchema   (ECSchemaR schema);

    //! Removes the specified schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema that should be removed from the cache
    //! @returns ECObjectsStatus::SchemaNotFound is the schema was not found in the cache, otherwise ECObjectsStatus::Success
    ECOBJECTS_EXPORT ECObjectsStatus DropSchema  (SchemaKeyCR key );

    //! Get the requested schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @returns The ECSchema if it is contained in the cache, NULL otherwise
    //! @remarks This will do an Identical match type for the requested schema
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key) const;

    //! Get the requested schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @param[in] matchType    The SchemaMatchType defining how exact of a match for the located schema is tolerated
    //! @returns The ECSchema if it is contained in the cache, NULL otherwise
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key, SchemaMatchType matchType) const;

    ECOBJECTS_EXPORT virtual ~ECSchemaCache (); //!< Destructor
    ECOBJECTS_EXPORT static  ECSchemaCachePtr Create (); //!< Creates an ECSchemaCachePtr
    ECOBJECTS_EXPORT int     GetCount() const; //!< Returns the number of schemas currently in the cache
    ECOBJECTS_EXPORT void    Clear(); //!< Removes all schemas from the cache
    ECOBJECTS_EXPORT IECSchemaLocater& GetSchemaLocater(); //!< Returns the SchemaCache as an IECSchemaLocater
    ECOBJECTS_EXPORT size_t GetSchemas (bvector<ECSchemaP>& schemas) const;
    ECOBJECTS_EXPORT void   GetSupplementalSchemasFor(Utf8CP schemaName, bvector<ECSchemaP>& supplementalSchemas) const;
};


/*__PUBLISH_SECTION_END__*/


//=======================================================================================
//! Locates schemas by looking in a given set of file system folder for ECSchemaXml files
//=======================================================================================
struct SearchPathSchemaFileLocater : IECSchemaLocater, RefCountedBase, NonCopyableClass
{
private:
    struct CandidateSchema
        {
        BeFileName FileName;
        WString SearchPath;
        SchemaKey Key;
        };

    bmap<bpair<SchemaKey, SchemaMatchType>, ECSchemaPtr> m_knownSchemas;
    bvector<WString> m_searchPaths;
    SearchPathSchemaFileLocater (bvector<WString> const& searchPaths);
    virtual ~SearchPathSchemaFileLocater();
    static bool TryLoadingSupplementalSchemas(Utf8StringCR schemaName, WStringCR schemaFilePath, ECSchemaReadContextR schemaContext, bvector<ECSchemaP>& supplementalSchemas);

    void FindEligibleSchemaFiles(bvector<CandidateSchema>& foundFiles, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext);
    void AddCandidateSchemas(bvector<CandidateSchema>& foundFiles, WStringCR schemaPath, WStringCR fileFilter, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext);

    static bool SchemyKeyIsLessByVersion(CandidateSchema const& lhs, CandidateSchema const& rhs);

protected:
    virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;

public:
    bvector<WString>const& GetSearchPath () const {return m_searchPaths;}
public:
    ECOBJECTS_EXPORT static SearchPathSchemaFileLocaterPtr CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths);
};

/*__PUBLISH_SECTION_START__*/
struct SupplementalSchemaInfo;
typedef RefCountedPtr<SupplementalSchemaInfo> SupplementalSchemaInfoPtr;

enum class ECSchemaElementType
    {
    ECClass,
    ECEnumeration,
    KindOfQuantity
    };

//=======================================================================================
//! 
//! @bsiclass
//=======================================================================================
struct ECSchemaElementsOrder : NonCopyableClass
{
private:
    bvector<bpair<Utf8String, ECSchemaElementType>> m_elementVector;
    bool m_preserveElementOrder;
public:
    ECSchemaElementsOrder() : m_preserveElementOrder(false) {}

    void AddElement(Utf8CP name, ECSchemaElementType type);
    void RemoveElement(Utf8CP name);
    void CreateAlphabeticalOrder(ECSchemaCR ecSchema);
    bool GetPreserveElementOrder() const { return m_preserveElementOrder; }
    void SetPreserveElementOrder(bool value) { m_preserveElementOrder = value; }
    bvector<bpair<Utf8String, ECSchemaElementType>>::const_iterator begin() const { return m_elementVector.begin(); } //!< Returns the beginning of the iterator
    bvector<bpair<Utf8String, ECSchemaElementType>>::const_iterator end() const { return m_elementVector.end(); } //!< Returns the end of the iterator
};

//=======================================================================================
//! The in-memory representation of a schema as defined by ECSchemaXML
//! For information about schema versioning please check SchemaKey class documentation.
//! @bsiclass
//=======================================================================================
struct ECSchema : RefCountedBase, IECCustomAttributeContainer
{
private:
    ECSchema (ECSchema const&);
    ECSchema& operator= (ECSchema const&);

friend struct SearchPathSchemaFileLocater;
friend struct SupplementedSchemaBuilder;
friend struct SchemaXmlReader;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    SchemaKey               m_key;
    Utf8String              m_namespacePrefix;
    Utf8String              m_displayLabel;
    mutable ECSchemaId      m_ecSchemaId;
    Utf8String              m_description;
    ECClassContainer        m_classContainer;
    ECEnumerationContainer  m_enumerationContainer;
    KindOfQuantityContainer m_kindOfQuantityContainer;

    // maps class name -> class pointer
    ClassMap                    m_classMap;
    EnumerationMap              m_enumerationMap;
    KindOfQuantityMap           m_kindOfQuantityMap;
    ECSchemaReferenceList       m_refSchemaList;
    bool                        m_isSupplemented;
    bool                        m_hasExplicitDisplayLabel;
    SupplementalSchemaInfoPtr   m_supplementalSchemaInfo;
    bool                        m_immutable;
    mutable ECSchemaElementsOrder m_serializationOrder; //mutable because we might modify this during serialization

    bmap<ECSchemaP, Utf8String> m_referencedSchemaNamespaceMap;
    /*__PUBLISH_SECTION_END__*/
    SchemaLocalizedStrings      m_localizedStrings;
    /*__PUBLISH_SECTION_START__*/

    ECSchema ();
    virtual ~ECSchema();

    bool                                AddingSchemaCausedCycles () const;
    void                                SetIsSupplemented(bool isSupplemented);

    void                                FindUniqueClassName(Utf8StringR newName, Utf8CP originalName);
    bool                                NamedElementExists(Utf8CP name);
    ECObjectsStatus                     AddClass (ECClassP pClass, bool resolveConflicts = false);
    ECObjectsStatus                     AddEnumeration(ECEnumerationP pEnumeration);
    ECObjectsStatus                     AddKindOfQuantity(KindOfQuantityP valueToAdd);
    ECObjectsStatus                     SetVersionFromString (Utf8CP versionString);
    ECObjectsStatus                     CopyConstraints(ECRelationshipConstraintR toRelationshipConstraint, ECRelationshipConstraintR fromRelationshipConstraint);

    void SetSupplementalSchemaInfo(SupplementalSchemaInfo* info);

    ECObjectsStatus                     AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR prefix, ECSchemaReadContextR readContext);
    void                                CollectAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas,  bool includeRootSchema) const;

protected:
    virtual ECSchemaCP                  _GetContainerSchema() const override;
    virtual CustomAttributeContainerType _GetContainerType() const override { return CustomAttributeContainerType::Schema; }

/*__PUBLISH_SECTION_END__*/
public:
    ECOBJECTS_EXPORT void               ReComputeCheckSum ();
    //! Intended to be called by ECDb or a similar system
    void SetId(ECSchemaId id) { BeAssert(!m_ecSchemaId.IsValid()); m_ecSchemaId = id; };
    bool HasId() const { return m_ecSchemaId.IsValid(); };

    ECOBJECTS_EXPORT ECObjectsStatus    DeleteClass (ECClassR ecClass);
    ECOBJECTS_EXPORT ECObjectsStatus    RenameClass (ECClassR ecClass, Utf8CP newName);
    SchemaLocalizedStringsCR            GetLocalizedStrings() const { return m_localizedStrings; }

//__PUBLISH_SECTION_START__
public:
    ECOBJECTS_EXPORT SchemaKeyCR        GetSchemaKey() const; //!< Returns a SchemaKey fully describing this schema
    ECOBJECTS_EXPORT void               DebugDump() const; //!< Prints out detailed information about this ECSchema, and then calls Dump() on each ECClass.

    //! Used for debugging purposes.
    //! @param[in] showMessages Controls whether messages are displayed during BeXml operations. Defaults to true.
    //! @param[in] doAssert Controls whether asserts should be tested or not.  Defaults to true.
    ECOBJECTS_EXPORT static void        SetErrorHandling (bool showMessages, bool doAssert);

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    ECOBJECTS_EXPORT ECSchemaId         GetId() const;
    //! Sets the name of this schema
    //! @param[in]  value   The name of the ECSchema
    //! @returns Success if the name passes validation and is set, ECObjectsStatus::InvalidName otherwise
    ECOBJECTS_EXPORT ECObjectsStatus    SetName(Utf8StringCR value);
    //! Returns the name of this ECSchema
    ECOBJECTS_EXPORT Utf8StringCR       GetName() const;
    //! Sets the namespace prefix for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetNamespacePrefix(Utf8StringCR value);
    //! Gets the namespace prefix for this ECSchema
    ECOBJECTS_EXPORT Utf8StringCR       GetNamespacePrefix() const;
    //! Sets the description for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(Utf8StringCR value);
    //! Gets the description for this ECSchema.  Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR       GetDescription() const;
    //! Gets the invariant description for this ECSchema.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDescription() const;
    //! Sets the display label for this ECSchema
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(Utf8StringCR value);
    //! Gets the DisplayLabel for this ECSchema.  If no DisplayLabel has been set explicitly, returns the name of the schema
    //! Gets the DisplayLabel for this ECSchema.  If no DisplayLabel has been set explicitly, returns the name of the schema.
    ECOBJECTS_EXPORT Utf8StringCR       GetDisplayLabel() const;
    //! Gets the invariant display label for this ECSchema.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDisplayLabel() const;
    //! Sets the major version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMajor(uint32_t value);
    //! Gets the major version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT uint32_t           GetVersionMajor() const;
    //! Sets the write compatibility version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionWrite(uint32_t value);
    //! Gets the write compatibility version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT uint32_t           GetVersionWrite() const;
    //! Sets the minor version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMinor(uint32_t value);
    //! Gets the minor version of this schema, check SchemaKey doc for detailed description.
    ECOBJECTS_EXPORT uint32_t           GetVersionMinor() const;
    //! Returns an iterable container of ECClasses sorted by name. For unsorted called overload.
    ECOBJECTS_EXPORT ECClassContainerCR GetClasses() const;
    //! Returns an iterable container of ECClasses sorted by name. For unsorted called overload.
    ECEnumerationContainerCR GetEnumerations() const { return m_enumerationContainer; }
    //! Removes an enumeration from this schema.
    ECOBJECTS_EXPORT ECObjectsStatus    DeleteEnumeration(ECEnumerationR ecEnumeration);
    //! Returns an iterable container of ECClasses sorted by name. For unsorted called overload.
    KindOfQuantityContainerCR GetKindOfQuantities() const { return m_kindOfQuantityContainer; }
    //! Removes a kind of quantity from this schema.
    ECOBJECTS_EXPORT ECObjectsStatus    DeleteKindOfQuantity(KindOfQuantityR kindOfQuantity);
    
    
    //! Indicates whether this schema is a so-called @b dynamic schema by
    //! checking whether the @b DynamicSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @return true, if this schema is a dynamic schema. false, otherwise
    ECOBJECTS_EXPORT bool IsDynamicSchema () const;

    //! Marks a schema as @b dynamic schema by adding the custom attribute @b DynamicSchema from the standard schema @b 
    //! Bentley_Standard_CustomAttributes to it.
    //! If the standard schema is not yet referenced, an error will be returned.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @param[in]  isDynamic true, if this schema should be marked as dynamic schema. false, otherwise.
    //! @return A status code indicating success or error
    ECOBJECTS_EXPORT ECObjectsStatus SetIsDynamicSchema (bool isDynamic);

    //! Indicates whether this schema is a system schema (in contrast to a user-supplied schema) by
    //! checking whether the @b %SystemSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A system schema is a schema used and managed internally by the software.
    //! @return true, if this schema is a system schema. false, otherwise
    ECOBJECTS_EXPORT bool IsSystemSchema () const;

    //! Gets the number of classes in the schema
    uint32_t           GetClassCount() const { return (uint32_t) m_classMap.size(); }

    //! Gets the number of enumerations in the schema
    uint32_t           GetEnumerationCount() const { return (uint32_t) m_enumerationMap.size(); }

    //! Gets the number of kind of quantity in the schema
    uint32_t           GetKindOfQuantityCount() const { return (uint32_t) m_kindOfQuantityMap.size(); }

    //! Returns true if the display label has been set explicitly for this schema or not
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    ECOBJECTS_EXPORT bool               IsStandardSchema() const;

    //! Returns true if the passed in schema is the same base schema as the current schema
    //! @remarks FullName, NamespacePrefix, and ClassCount are checked
    //! @return True    if the schemas are the same
    ECOBJECTS_EXPORT bool               IsSamePrimarySchema(ECSchemaR primarySchema) const;

    //! Indicates whether this schema is a supplemental schema or not.
    //! @return True if this schema is a supplemental schema
    ECOBJECTS_EXPORT bool               IsSupplementalSchema() const;

    //! Returns true if the schema is a supplemented schema
    //! @return True if the schema is a supplemented schema
    ECOBJECTS_EXPORT bool               IsSupplemented() const;

    //! Gets the SupplementalSchemaInfo for this ECSchema
    ECOBJECTS_EXPORT SupplementalSchemaInfoPtr const GetSupplementalInfo() const;

    //! Returns true if and only if the full schema name (including version) represents a standard schema that should never
    //! be stored persistently in a repository (we expect it to be found elsewhere)
    //! @return True if this version of the schema is one that should never be imported into a repository
    ECOBJECTS_EXPORT bool               ShouldNotBeStored() const;

    //! Returns true if and only if the full schema name (including version) represents a standard schema that should never
    //! be stored persistently in a repository (we expect it to be found elsewhere)
    //! @param[in]  key SchemaKey to test
    //! @return True if this version of the schema is one that should never be imported into a repository
    ECOBJECTS_EXPORT static bool        ShouldNotBeStored (SchemaKeyCR key);

    //! If the class name is valid, will create an ECEntityClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECEntityClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateEntityClass (ECEntityClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create an ECStructClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECStructClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateStructClass (ECStructClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create an ECCustomAttributeClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECCustomAttributeClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateCustomAttributeClass (ECCustomAttributeClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create an ECRelationshipClass object and add the new class to the schema
    //! @param[out] relationshipClass If successful, will contain a new ECRelationshipClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateRelationshipClass (ECRelationshipClassP& relationshipClass, Utf8StringCR name);

    //! Creates a new KindOfQuantity and adds it to the schema.
    //! @param[out] kindOfQuantity If successful, will contain a new KindOfQuantity object
    //! @param[in] name    Name of the object to create
    //! @return A status code indicating whether or not the object was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateKindOfQuantity(KindOfQuantityP& kindOfQuantity, Utf8CP name);

    //! Creates a new ECEnumeration and adds it to the schema.
    //! @param[out] ecEnumeration If successful, will contain a new ECEnumeration object
    //! @param[in] name    Name of the enumeration to create
    //! @param[in] type    Type for the enumeration to create. Must be integer or string.
    //! @return A status code indicating whether or not the enumeration was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus    CreateEnumeration(ECEnumerationP& ecEnumeration, Utf8CP name, PrimitiveType type);

    //! Get a schema by namespace prefix within the context of this schema and its referenced schemas.
    //! @param[in]  namespacePrefix     The prefix of the schema to lookup in the context of this schema and it's references.
    //!                                 Passing an empty namespacePrefix will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an ECN::ECSchema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaCP         GetSchemaByNamespacePrefixP(Utf8StringCR namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup a namespace prefix in the context of this schema and its references.
    //! @param[out] namespacePrefix The namespace prefix if schema is a referenced schema; empty string if the sechema is the current schema;
    //! @return   Success if the schema is either the current schema or a referenced schema;  ECObjectsStatus::SchemaNotFound if the schema is not found in the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus    ResolveNamespacePrefix (ECSchemaCR schema, Utf8StringR namespacePrefix) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A const pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassCP          GetClassCP (Utf8CP name) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (Utf8CP name);

    //! Get an enumeration by name within the context of this schema.
    //! @param[in]  name     The name of the enumeration to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECEnumeration if the named enumeration exists in within the current schema; otherwise, nullptr
    ECEnumerationCP          GetEnumerationCP(Utf8CP name) const { return const_cast<ECSchemaP> (this)->GetEnumerationP(name); }

    //! Get an enumeration by name within the context of this schema.
    //! @param[in]  name     The name of the enumeration to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECEnumeration if the named enumeration exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECEnumerationP           GetEnumerationP(Utf8CP name);

    //! Get a kind of quantity by name within the context of this schema.
    //! @param[in]  name     The name of the kind of quantity to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::KindOfQuantity if the named kind of quantity exists in within the current schema; otherwise, nullptr
    KindOfQuantityCP          GetKindOfQuantityCP(Utf8CP name) const { return const_cast<ECSchemaP> (this)->GetKindOfQuantityP(name); }

    //! Get an kind of quantity by name within the context of this schema.
    //! @param[in]  name     The name of the kind of quantity to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::KindOfQuantity if the named enumeration exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT KindOfQuantityP           GetKindOfQuantityP(Utf8CP name);

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
    ECOBJECTS_EXPORT ECObjectsStatus            AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR prefix);

    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveReferencedSchema(ECSchemaR refSchema);

    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  schemaKey   The key for the schema that should be removed from the list of referenced schemas.  Must be an exact match
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveReferencedSchema(SchemaKeyCR schemaKey);

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @param[in]  ecXmlVersionMajor   The major version of the ECXml spec to be used for serializing this schema
    //! @param[in]  ecXmlVersionMinor   The minor version of the ECXml spec to be used for serializing this schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlString (WStringR ecSchemaXml, int ecXmlVersionMajor = 2, int ecXmlVersionMinor = 0) const;

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @param[in]  ecXmlVersionMajor   The major version of the ECXml spec to be used for serializing this schema
    //! @param[in]  ecXmlVersionMinor   The minor version of the ECXml spec to be used for serializing this schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlString (Utf8StringR ecSchemaXml, int ecXmlVersionMajor = 2, int ecXmlVersionMinor = 0) const;

    //! Serializes an ECXML schema to a file
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @param[in]  ecXmlVersionMajor   The major version of the ECXml spec to be used for serializing this schema
    //! @param[in]  ecXmlVersionMinor   The minor version of the ECXml spec to be used for serializing this schema
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlFile (WCharCP ecSchemaXmlFile, int ecXmlVersionMajor = 2, int ecXmlVersionMinor = 0, bool utf16 = false) const;

    //! Writes an ECXML schema to an IStream
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @param[in]  ecXmlVersionMajor   The major version of the ECXml spec to be used for serializing this schema
    //! @param[in]  ecXmlVersionMinor   The minor version of the ECXml spec to be used for serializing this schema
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlStream (IStreamP ecSchemaXmlStream, int ecXmlVersionMajor = 2, int ecXmlVersionMinor = 0, bool utf16 = false);
    
    //! Return full schema name in format GetName().MM.ww.mm where Name is the schema name MM is major version,ww is the  write compatibility version and mm is minor version.
    Utf8String             GetFullSchemaName() const { return m_key.GetFullSchemaName(); }

    //! Return a legacy full schema name in format GetName().MM.mm where Name is the schema name MM is major version and mm is minor version.
    //! This overload is missing the  write compatibility version
    Utf8String             GetLegacyFullSchemaName() const { return SchemaKey::FormatLegacyFullSchemaName(
        m_key.GetName().c_str(), m_key.GetVersionMajor(), m_key.GetVersionMinor()); }

    //! Given a source class, will copy that class into this schema if it does not already exist
    //! @param[out] targetClass If successful, will contain a new ECClass object that is a copy of the sourceClass
    //! @param[in]  sourceClass The class to copy
    ECOBJECTS_EXPORT ECObjectsStatus        CopyClass(ECClassP& targetClass, ECClassCR sourceClass);

    //! Given a source enumeration, will copy that enumeration into this schema if it does not already exist
    //! @param[out] targetEnumeration If successful, will contain a new ECEnumeration object that is a copy of the sourceEnumeration
    //! @param[in]  sourceEnumeration The enumeration to copy
    ECOBJECTS_EXPORT ECObjectsStatus        CopyEnumeration(ECEnumerationP& targetEnumeration, ECEnumerationCR sourceEnumeration);

    //! Copies this schema
    //! @param[out] schemaOut   If successful, will contain a copy of this schema
    ECOBJECTS_EXPORT ECObjectsStatus        CopySchema(ECSchemaPtr& schemaOut) const;

    //! Get the IECCustomAttributeContainer holding this schema's custom attributes
    ECOBJECTS_EXPORT IECCustomAttributeContainer&   GetCustomAttributeContainer();
    //! Get the const IECCustomAttributeContainer holding this schema's custom attributes
    ECOBJECTS_EXPORT IECCustomAttributeContainer const& GetCustomAttributeContainer() const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    //! Given a str containing SchemaXml, will compute the CheckSum
    ECOBJECTS_EXPORT static uint32_t        ComputeSchemaXmlStringCheckSum(Utf8CP str, size_t len);

    //! Try using the other overload, this one is just included for legacy reasons.
    //! If the given schemaName is valid, this will create a new schema object
    //! @param[out] schemaOut   if successful, will contain a new schema object
    //! @param[in]  schemaName  Name of the schema to be created.
    //! @param[in]  versionMajor The major version number.
    //! @param[in]  versionMinor The minor version number.
    //! @return A status code indicating whether the call was succesfull or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema(ECSchemaPtr& schemaOut, Utf8StringCR schemaName,
                                                         uint32_t versionMajor, uint32_t versionMinor);

    //! Generate a schema version string given the major and minor version values.
    //! @param[in] versionMajor    The major version number
    //! @param[out] versionWrite The  write compatibility version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    static Utf8String FormatSchemaVersion(uint32_t versionMajor, uint32_t versionWrite, uint32_t versionMinor) { return SchemaKey::FormatSchemaVersion(versionMajor, versionWrite, versionMinor); }

    //! If the given schemaName and namespacePrefix is valid, this will create a new schema object
    //! @param[out] schemaOut       if successful, will contain a new schema object
    //! @param[in]  schemaName      Name of the schema to be created.
    //! @param[in]  namespacePrefix Namespace prefix of the schema to be created
    //! @param[in]  versionMajor    The major version number.
    //! @param[out] versionWrite The  write compatibility version number
    //! @param[in]  versionMinor    The minor version number.
    //! @return A status code indicating whether the call was succesfull or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema(ECSchemaPtr& schemaOut, Utf8StringCR schemaName, 
                                                         Utf8StringCR namespacePrefix, uint32_t versionMajor, uint32_t versionWrite, uint32_t versionMinor);

    //! Generate a schema version string given the major and minor version values.
    //! @param[in] versionMajor    The major version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    static Utf8String      FormatSchemaVersion(uint32_t versionMajor, uint32_t versionMinor)
        {
        return FormatSchemaVersion(versionMajor, DEFAULT_VERSION_WRITE, versionMinor);
        }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionWrite The  write compatibility version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionMajor, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP fullName) { return SchemaKey::ParseSchemaFullName(schemaName, versionMajor, versionWrite, versionMinor, fullName); }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8CP fullName)
        {
        uint32_t unwanted = DEFAULT_VERSION_WRITE;
        return ParseSchemaFullName(schemaName, versionMajor, unwanted, versionMinor, fullName);
        }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8StringCR fullName) { return ParseSchemaFullName(schemaName, versionMajor, versionMinor, fullName.c_str()); }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseVersionString(uint32_t& versionMajor, uint32_t& versionMinor, Utf8CP versionString)
        {
        uint32_t unwanted = DEFAULT_VERSION_WRITE;
        return ParseVersionString(versionMajor, unwanted, versionMinor, versionString);
        }

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionWrite    write compatibility version number, will default to zero if versionString only has two numbers.
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseVersionString(uint32_t& versionMajor, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString) { return SchemaKey::ParseVersionString(versionMajor, versionWrite, versionMinor, versionString); }

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
    //! ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    //!
    //! ECSchemaPtr schema;
    //! WCharCP ecSchemaFilename = L"ECSchema file path";
    //! SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ecSchemaFilename, *schemaContext);
    //! if (SchemaReadStatus::Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXmlFile     The absolute path of the file to write.
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlFile (ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema paths
    //! relative to the executing dll will be searched.
    //! @param[in]    schema              Key describing the schema to be located
    //! @param[in]    schemaContext       Required to create schemas
    ECOBJECTS_EXPORT static ECSchemaPtr  LocateSchema (SchemaKeyR schema, ECSchemaReadContextR schemaContext);

    //!
    //! Reads an ECSchema from a UTF-8 encoded ECSchemaXML-formatted string.
    //! @code
    //! // The IECSchemaOwner determines the lifespan of any ECSchema objects that are created using it.
    //! ECSchemaCachePtr                  schemaOwner = ECSchemaCache::Create();
    //!
    //! // The schemaContext supplies an IECSchemaOwner to control the lifetime of read ECSchemas and a
    //!
    //! ECSchemaP schema;
    //! SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, ecSchemaAsString, *schemaContext);
    //! if (SchemaReadStatus::Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXml         The UTF-8 encoded string containing ECSchemaXML to write
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString (ECSchemaPtr& schemaOut, Utf8CP ecSchemaXml, ECSchemaReadContextR schemaContext);

    //!
    //! Reads an ECSchema from an ECSchemaXML-formatted string.
    //! @code
    //! // The IECSchemaOwner determines the lifespan of any ECSchema objects that are created using it.
    //! ECSchemaCachePtr                  schemaOwner = ECSchemaCache::Create();
    //!
    //! // The schemaContext supplies an IECSchemaOwner to control the lifetime of read ECSchemas and a
    //!
    //! ECSchemaP schema;
    //! SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, ecSchemaAsString, *schemaContext);
    //! if (SchemaReadStatus::Success != status)
    //!     return ERROR;
    //! @endcode
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXml         The string containing ECSchemaXML to write
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString (ECSchemaPtr& schemaOut, WCharCP ecSchemaXml, ECSchemaReadContextR schemaContext);

    //! Writes an ECSchema from an ECSchemaXML-formatted string in an IStream.
    //! @param[out]   schemaOut           The read schema
    //! @param[in]    ecSchemaXmlStream   The IStream containing ECSchemaXML to write
    //! @param[in]    schemaContext       Required to create schemas
    //! @return   A status code indicating whether the schema was successfully read.  If SUCCESS is returned then schemaOut will
    //!           contain the read schema.  Otherwise schemaOut will be unmodified.
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlStream (ECSchemaPtr& schemaOut, IStreamP ecSchemaXmlStream, ECSchemaReadContextR schemaContext);

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    //! @param[in]  schemaName  Name of the schema to test.  This is just the schema name, no version info
    ECOBJECTS_EXPORT static bool IsStandardSchema(Utf8StringCR schemaName);

    //! Find all ECSchemas in the schema graph, avoiding duplicates and any cycles.
    //! @param[out]   allSchemas            Vector of schemas including rootSchema.
    //! @param[in]    includeRootSchema     If true then root schema is added to the vector of allSchemas. Defaults to true.
    ECOBJECTS_EXPORT void FindAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema=true) const;
    //! Find all ECSchemas in the schema graph, avoiding duplicates and any cycles.
    //! @param[out]   allSchemas            Vector of schemas including rootSchema.
    //! @param[in]    includeRootSchema     If true then root schema is added to the vector of allSchemas. Defaults to true.
    ECOBJECTS_EXPORT void FindAllSchemasInGraph (bvector<ECN::ECSchemaP>& allSchemas, bool includeRootSchema=true);

    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaCP FindSchema (SchemaKeyCR schema, SchemaMatchType matchType) const;

    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaP FindSchemaP (SchemaKeyCR schema, SchemaMatchType matchType);

    //!Set the schema to be immutable. Immutable schema cannot be modified.
    ECOBJECTS_EXPORT void   SetImmutable();
}; // ECSchema

//typedef RefCountedPtr<IECClassLocater> IECClassLocaterPtr;

//*=================================================================================**//**
//* @bsistruct                                                  Ramanujam.Raman   12/12
//+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IECClassLocater
    {
    protected:
        virtual ECClassCP _LocateClass (Utf8CP schemaName, Utf8CP className) = 0;
    public:
        virtual ~IECClassLocater() {}

        ECClassCP LocateClass (Utf8CP schemaName, Utf8CP className) { return _LocateClass (schemaName, className); }
    };

typedef IECClassLocater& IECClassLocaterR;

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

//#pragma make_public (ECN::ECClass)
//#pragma make_public (ECN::ECSchema)
