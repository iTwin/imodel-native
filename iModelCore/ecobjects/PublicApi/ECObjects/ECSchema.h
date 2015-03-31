/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
/*__PUBLISH_SECTION_END__*/
#include <ECObjects/CalculatedProperty.h>
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECEnabler.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/bset.h>

#define DEFAULT_VERSION_MAJOR   1
#define DEFAULT_VERSION_MINOR   0

EC_TYPEDEFS(QualifiedECAccessor);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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

typedef bvector<ECPropertyP> PropertyList;
typedef bmap<WCharCP , ECPropertyP, less_str> PropertyMap;
typedef bmap<WCharCP , ECClassP,    less_str> ClassMap;

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
    WString             m_name;
    WString             m_displayLabel;
    bool                m_hasExplicitDisplayLabel;
public:
    ECValidatedName() : m_hasExplicitDisplayLabel (false) { }

    WStringCR           GetName() const                 { return m_name; }
    bool                IsDisplayLabelDefined() const   { return m_hasExplicitDisplayLabel; }
    WStringCR           GetDisplayLabel() const;
    void                SetName (WCharCP name);
    void                SetDisplayLabel (WCharCP label);
    };

/*__PUBLISH_SECTION_START__*/

//=======================================================================================
//! Handles validation, encoding, and decoding of names for ECSchemas, ECClasses, and
//! ECProperties.
//! The names of ECSchemas, ECClasses, and ECProperties must conform to the following
//! rules:
//!     -Contains only alphanumeric characters in the ranges ['A'..'Z'], ['a'..'z'], ['0'..'9'], and ['_']
//!     -Contains at least one character
//!     -Does not begin with a digit
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECNameValidation
    {
private:
/*__PUBLISH_SECTION_END__*/
    static void AppendEncodedCharacter (WStringR encoded, WChar c);
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
    ECOBJECTS_EXPORT static bool                EncodeToValidName (WStringR encoded, WStringCR name);

    //! Decodes special characters in a name encoded by EncodeToValidName() to produce a name suitable for display
    //! @param[out] decoded     Will hold the decoded name
    //! @param[in]  name        The name to decode
    //! @returns true if any special characters were decoded. Regardless of return value, decoded will contain a decoded name.
    ECOBJECTS_EXPORT static bool                DecodeFromValidName (WStringR decoded, WStringCR name);

    //! Checks a name against the rules for valid names
    //! @param[in] name     The name to validate
    //! @returns RESULT_Valid if the name is valid, or a ValidationResult indicating why the name is invalid.
    ECOBJECTS_EXPORT static ValidationResult    Validate (WCharCP name);

    //! Returns true if the specified name is a valid EC name
    ECOBJECTS_EXPORT static bool                IsValidName (WCharCP name);

    //! Checks whether a character is valid for use in an ECName, e.g. alphanumeric, plus '_'
    ECOBJECTS_EXPORT static bool IsValidAlphaNumericCharacter (WChar c);
    ECOBJECTS_EXPORT static bool IsValidAlphaNumericCharacter (Utf8Char c);
    };

//=======================================================================================
//! Used to represent the type of an ECProperty
//! @ingroup ECObjectsGroup
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

    //! Constructor that takes a PrimitiveType
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

typedef int64_t ECClassId;
typedef int64_t ECPropertyId;
typedef int64_t ECSchemaId;

typedef bvector<IECInstancePtr> ECCustomAttributeCollection;
struct ECCustomAttributeInstanceIterable;
struct SupplementedSchemaBuilder;

//=======================================================================================
//! @ingroup ECObjectsGroup
//! Base class for ECSchema, ECClass, ECProperty.  Represents a container object that can hold 
//! Custom Attributes.
//! @bsiclass
//=======================================================================================
struct IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECCustomAttributeInstanceIterable;
    friend struct SupplementedSchemaBuilder;

    ECCustomAttributeCollection         m_primaryCustomAttributes;
    ECCustomAttributeCollection         m_consolidatedCustomAttributes;
    SchemaWriteStatus                   AddCustomAttributeProperties (BeXmlNodeR oldNode, BeXmlNodeR newNode) const;

    IECInstancePtr                      GetCustomAttributeInternal(WStringCR className, bool includeBaseClasses, bool includeSupplementalAttributes) const;
    IECInstancePtr                      GetCustomAttributeInternal(ECClassCR ecClass, bool includeBaseClasses, bool includeSupplementalAttributes) const;

    ECObjectsStatus                     SetCustomAttributeInternal(ECCustomAttributeCollection& customAttributeCollection, IECInstanceR customAttributeInstance, bool requireSchemaReference = false);
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetPrimaryCustomAttribute(IECInstanceR customAttributeInstance);

protected:
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetConsolidatedCustomAttribute(IECInstanceR customAttributeInstance);
    //! Removes a consolidated custom attribute from the container
    //! @param[in]  classDefinition ECClass of the custom attribute to remove
    bool                                RemoveConsolidatedCustomAttribute(ECClassCR classDefinition);

    InstanceReadStatus                  ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR context, ECSchemaCR fallBackSchema);
    SchemaWriteStatus                   WriteCustomAttributes(BeXmlNodeR parentNode) const;
    //! Only copies primary ones, not consolidated ones. Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     CopyCustomAttributesTo(IECCustomAttributeContainerR destContainer) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
    void                                AddUniquePrimaryCustomAttributesToList(ECCustomAttributeCollection& returnList);
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP                  _GetContainerSchema() const = 0;// {return NULL;};

    ECOBJECTS_EXPORT virtual ~IECCustomAttributeContainer();

public:
    ECSchemaP                           GetContainerSchema();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Returns true if the container has a custom attribute of a class of the specified name
    ECOBJECTS_EXPORT bool               IsDefined (WStringCR className) const;
    //! Returns true if the container has a custom attribute of a class of the specified class definition
    ECOBJECTS_EXPORT bool               IsDefined (ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes supplemental custom attributes
    //! and custom attributes from the base containers
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(WStringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes supplemental custom attributes
    //! and custom attributes from the base containers
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttribute(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes supplemental custom attributes
    //! but not base containers
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(WStringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes supplemental custom attributes
    //! but not base containers
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetCustomAttributeLocal(ECClassCR classDefinition) const;

    //! Retrieves the custom attribute matching the class name.  Includes custom attributes from base containers
    //! but not supplemental custom attributes
    //! @param[in]  className   The name of the CustomAttribute Class to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttribute(WStringCR className) const;

    //! Retrieves the custom attribute matching the class definition.  Includes custom attributes from base containers
    //! but not supplemental custom attributes
    //! @param[in]  classDefinition   The ECClass to look for an instance of
    //! @returns An IECInstancePtr.  If IsValid(), will be the matching custom attribute.  Otherwise, no instance of
    //! the custom attribute was found on the container.
    ECOBJECTS_EXPORT IECInstancePtr     GetPrimaryCustomAttribute(ECClassCR classDefinition) const;

    //! Retrieves all custom attributes from the container including supplemental custom attributes
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetCustomAttributes(bool includeBase) const;

    //! Retrieves all custom attributes from the container NOT including supplemental custom attributes
    //! @param[in]  includeBase  Whether to include custom attributes from the base containers
    ECOBJECTS_EXPORT ECCustomAttributeInstanceIterable GetPrimaryCustomAttributes(bool includeBase) const;

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
//! @ingroup ECObjectsGroup
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

//__PUBLISH_CLASS_VIRTUAL__
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
@ingroup ECObjectsGroup
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECTypeAdapterContext : RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
private:
    EvaluationOptions m_evalOptions;

protected:
    virtual ECPropertyCP                        _GetProperty() const = 0;
    virtual uint32_t                            _GetComponentIndex() const = 0;
    virtual bool                                _Is3d() const = 0;
    virtual IECInstanceCP                       _GetECInstance() const = 0;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GetInstanceValue (ECValueR v, WCharCP accessString, uint32_t arrayIndex) const;
    ECOBJECTS_EXPORT virtual IECClassLocaterR   _GetUnitsECClassLocater() const = 0;
    ECOBJECTS_EXPORT virtual EvaluationOptions  _GetEvaluationOptions () const;
    ECOBJECTS_EXPORT virtual void               _SetEvaluationOptions (EvaluationOptions evalOptions);

public:

    ECOBJECTS_EXPORT  IECInstanceCP     GetECInstance() const;
    ECOBJECTS_EXPORT  ECPropertyCP      GetProperty() const;
    ECOBJECTS_EXPORT  ECObjectsStatus   GetInstanceValue (ECValueR v, WCharCP accessString, uint32_t arrayIndex = -1) const;

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
//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/
    static const uint32_t COMPONENT_INDEX_None = -1;
    };

typedef RefCountedPtr<IECTypeAdapterContext> IECTypeAdapterContextPtr;
    
/*=================================================================================**//**
Base class for an object which adapts the internal value of an ECProperty to a user-friendly string representation.
@ingroup ECObjectsGroup
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECTypeAdapter : RefCountedBase
    {
    typedef bvector<WString> StandardValuesCollection;
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

    virtual bool                _CanConvertToString (IECTypeAdapterContextCR context) const = 0;
    virtual bool                _CanConvertFromString (IECTypeAdapterContextCR context) const = 0;
    virtual bool                _ConvertToString (WStringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP formatter) const = 0;
    virtual bool                _ConvertFromString (ECValueR v, WCharCP str, IECTypeAdapterContextCR context) const = 0;

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
    //! For example, for some numeric properties it doesn't make sence to show ordinal operators, because adapter gives a list
    //! of string rather than numbers.
    ECOBJECTS_EXPORT bool       IsOrdinalType () const;

//__PUBLISH_CLASS_VIRTUAL__
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
    ECOBJECTS_EXPORT        bool ConvertToString (WStringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP formatter = NULL) const;

    //! Converts from a string to the underlying ECValue type. Input string typically comes from user input
    //! @param[out] v       The converted value
    //! @param[in] str      The string to convert
    //! @param[in] context  Context under which conversion is performed
    //! @return true if conversion is successful
    ECOBJECTS_EXPORT        bool ConvertFromString (ECValueR v, WCharCP str, IECTypeAdapterContextCR context) const;

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
//! @ingroup ECObjectsGroup
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECProperty /*abstract*/ : public IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;

private:
    WString                 m_description;
    ECValidatedName         m_validatedName;
    mutable ECPropertyId    m_ecPropertyId;
    bool                    m_readOnly;
    ECClassCR               m_class;
    ECPropertyCP            m_baseProperty;
    mutable IECTypeAdapter* m_cachedTypeAdapter;

    static void     SetErrorHandling (bool doAssert);
protected:
    WString         m_originalTypeName; //Will be empty unless the typeName was unrecognized. Keep this so that we can re-write the ECSchema without changing the type to string
    bool                    m_forSupplementation;   // If when supplementing the schema, a local property had to be created, then don't serialize this property
    ECProperty (ECClassCR ecClass);
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
    
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const { return NULL; }
    virtual bool                                _IsCalculated() const { return false; }
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) { return false; }
public:
    // The following are used by the 'extended type' system which is currently implemented in DgnPlatform
    IECTypeAdapter*                     GetCachedTypeAdapter() const { return m_cachedTypeAdapter; }
    void                                SetCachedTypeAdapter (IECTypeAdapter* adapter) const { m_cachedTypeAdapter = adapter; }
    IECTypeAdapter*                     GetTypeAdapter() const;
    bool                                IsReadOnlyFlagSet() const { return m_readOnly; }

    //! Intended to be called by ECDb or a similar system
    ECOBJECTS_EXPORT void SetId(ECPropertyId id) { BeAssert (0 == m_ecPropertyId); m_ecPropertyId = id; };
    ECOBJECTS_EXPORT bool HasId() const { return m_ecPropertyId != 0; };

/*__PUBLISH_SECTION_START__*/
public:
    //! Returns the CalculatedPropertySpecification associated with this ECProperty, if any
    ECOBJECTS_EXPORT CalculatedPropertySpecificationCP   GetCalculatedPropertySpecification() const;
    //! Returns true if this ECProperty has a CalculatedECPropertySpecification custom attribute applied to it.
    ECOBJECTS_EXPORT bool               IsCalculated() const;

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

    ECOBJECTS_EXPORT PrimitiveECPropertyCP  GetAsPrimitiveProperty () const; //!< Returns the property as a const PrimitiveECProperty*
    ECOBJECTS_EXPORT PrimitiveECPropertyP   GetAsPrimitivePropertyP (); //!< Returns the property as a PrimitiveECProperty*
    ECOBJECTS_EXPORT ArrayECPropertyCP      GetAsArrayProperty () const; //!< Returns the property as a const ArrayECProperty*
    ECOBJECTS_EXPORT ArrayECPropertyP       GetAsArrayPropertyP (); //!< Returns the property as an ArrayECProperty*
    ECOBJECTS_EXPORT StructECPropertyCP     GetAsStructProperty () const; //!< Returns the property as a const StructECProperty*
    ECOBJECTS_EXPORT StructECPropertyP      GetAsStructPropertyP (); //!< Returns the property as a StructECProperty*
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct PrimitiveECProperty : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    PrimitiveType                               m_primitiveType;
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;   // lazily-initialized

    PrimitiveECProperty (ECClassCR ecClass) : m_primitiveType(PRIMITIVETYPE_String), ECProperty(ecClass) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsPrimitive () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const override;
    virtual bool                                _IsCalculated() const override;
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) override;
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Sets the PrimitiveType of this ECProperty.  The default type is ::PRIMITIVETYPE_String
    ECOBJECTS_EXPORT ECObjectsStatus SetType(PrimitiveType value);
    //! Gets the PrimitiveType of this ECProperty
    ECOBJECTS_EXPORT PrimitiveType GetType() const;
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct StructECProperty : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;
private:
    ECClassCP   m_structType;

    StructECProperty (ECClassCR ecClass) : m_structType(NULL), ECProperty(ecClass) {};

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsStruct () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! The property type.
    //! This type must be an ECClass where IsStruct is set to true.
    ECOBJECTS_EXPORT ECObjectsStatus    SetType(ECClassCR value);
    //! Gets the ECClass that defines the type for this property
    ECOBJECTS_EXPORT ECClassCR          GetType() const; 
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ArrayECProperty : public ECProperty
{
    DEFINE_T_SUPER(ECProperty)
/*__PUBLISH_SECTION_END__*/
friend struct ECClass;

private:
    uint32_t                                    m_minOccurs;
    uint32_t                                    m_maxOccurs;    // D-106653 we store this as read from the schema, but all arrays are considered to be of unbounded size
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;
    union
        {
        PrimitiveType   m_primitiveType;
        ECClassCP       m_structType;
        };

    ArrayKind               m_arrayKind;
    mutable IECTypeAdapter* m_cachedMemberTypeAdapter;

    ArrayECProperty (ECClassCR ecClass)
        : ECProperty(ecClass), m_primitiveType(PRIMITIVETYPE_String), m_arrayKind (ARRAYKIND_Primitive),
          m_minOccurs (0), m_maxOccurs (UINT_MAX), m_cachedMemberTypeAdapter (NULL) {};
    ECObjectsStatus                     SetMinOccurs (WStringCR minOccurs);
    ECObjectsStatus                     SetMaxOccurs (WStringCR maxOccurs);

protected:
    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    virtual SchemaWriteStatus           _WriteXml(BeXmlNodeP& createdPropertyNode, BeXmlNodeR parentNode) override;
    virtual bool                        _IsArray () const override { return true;}
    virtual WString                     _GetTypeName () const override;
    virtual ECObjectsStatus             _SetTypeName (WStringCR typeName) override;
    virtual bool                        _CanOverride(ECPropertyCR baseProperty) const override;
    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const override;
    virtual bool                                _IsCalculated() const override;
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) override;

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
    //! Sets the ECClass to be used for the array's struct elements
    ECOBJECTS_EXPORT ECObjectsStatus    SetStructElementType(ECClassCP value);
    //! Gets the ECClass of the array's struct elements
    ECOBJECTS_EXPORT ECClassCP          GetStructElementType() const;
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
//! Container holding ECProperties that supports STL like iteration
//! @ingroup ECObjectsGroup
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
//__PUBLISH_CLASS_VIRTUAL__
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
    ECOBJECTS_EXPORT ECPropertyCP   FindByDisplayLabel (WCharCP label) const;
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
//! @bsiclass
//=======================================================================================
struct ECClass : IECCustomAttributeContainer
{
/*__PUBLISH_SECTION_END__*/

friend struct ECSchema;
friend struct ECPropertyIterable::IteratorState;
friend struct SupplementedSchemaBuilder;
friend bool ECProperty::SetCalculatedPropertySpecification (IECInstanceP);

private:
    mutable WString                 m_fullName;
    WString                         m_description;
    ECValidatedName                 m_validatedName;
    mutable ECClassId               m_ecClassId;
    bool                            m_isStruct;
    bool                            m_isCustomAttributeClass;
    bool                            m_isDomainClass;
    ECSchemaCR                      m_schema;
    ECBaseClassesList               m_baseClasses;
    mutable ECDerivedClassesList    m_derivedClasses;

    PropertyMap                     m_propertyMap;
    PropertyList                    m_propertyList;
    mutable StandaloneECEnablerPtr  m_defaultStandaloneEnabler;

    ECObjectsStatus AddProperty (ECPropertyP& pProperty);
    ECObjectsStatus AddProperty (ECPropertyP pProperty, WStringCR name);
    ECObjectsStatus RemoveProperty (ECPropertyR pProperty);

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

    void            OnBaseClassPropertyRemoved (ECPropertyCR baseProperty);
    ECObjectsStatus OnBaseClassPropertyAdded (ECPropertyCR baseProperty);
protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECSchemaCR schema);
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

    SchemaReadStatus                    _ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context);
    SchemaReadStatus                    _ReadPropertyFromXmlAndAddToClass( ECPropertyP ecProperty, BeXmlNodeP& childNode, ECSchemaReadContextR context, Utf8CP childNodeName );

    virtual SchemaWriteStatus           _WriteXml (BeXmlNodeP& createdClassNode, BeXmlNodeR parentNode) const;
    SchemaWriteStatus                   _WriteXml (BeXmlNodeP& createdClassNode, BeXmlNodeR parentNode, Utf8CP elementName) const;

    virtual ECRelationshipClassCP       _GetRelationshipClassCP () const { return NULL; }  // used to avoid dynamic_cast
    virtual ECRelationshipClassP        _GetRelationshipClassP ()        { return NULL; }  // used to avoid dynamic_cast

    void                                InvalidateDefaultStandaloneEnabler() const;
public:
    ECOBJECTS_EXPORT ECPropertyP            GetPropertyByIndex (uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus        RenameProperty (ECPropertyR ecProperty, WCharCP newName);
    ECOBJECTS_EXPORT ECObjectsStatus        ReplaceProperty (ECPropertyP& newProperty, ValueKind valueKind, ECPropertyR propertyToRemove);
    ECOBJECTS_EXPORT ECObjectsStatus        DeleteProperty (ECPropertyR ecProperty);
    ECSchemaR                               GetSchemaR() { return const_cast<ECSchemaR>(m_schema); }

    //! Intended to be called by ECDb or a similar system
    ECOBJECTS_EXPORT void SetId(ECClassId id) { BeAssert (0 == m_ecClassId); m_ecClassId = id; };
    ECOBJECTS_EXPORT bool HasId() const { return m_ecClassId != 0; };

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    ECOBJECTS_EXPORT ECClassId             GetId() const;
    //! Returns the StandaloneECEnabler for this class
    ECOBJECTS_EXPORT StandaloneECEnablerP  GetDefaultStandaloneEnabler() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECRelationshipClassCP GetRelationshipClassCP() const;
    //! Used to avoid dynamic_cast
    ECOBJECTS_EXPORT ECRelationshipClassP GetRelationshipClassP();
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
    //! Returns the number of ECProperties in this class
    ECOBJECTS_EXPORT size_t GetPropertyCount (bool includeBaseProperties = true) const;
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

    //! Returns true if the class name  is of the type specified or derived from it.
    ECOBJECTS_EXPORT bool            Is(WCharCP name) const;
    //! Returns true if this class matches the specified schema and class name, or is derived from a matching class
    ECOBJECTS_EXPORT bool            Is (WCharCP schemaName, WCharCP className) const;

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
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WCharCP name, bool includeBaseClasses=true) const;

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP     GetPropertyP (WStringCR name, bool includeBaseClasses=true) const;

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
    //__PUBLISH_SECTION_START__

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

//! Used to define how the relationship OrderId is handled.
//! @ingroup ECObjectsGroup
enum OrderIdStorageMode: uint8_t
    {
    ORDERIDSTORAGEMODE_None                     = 0,
    ORDERIDSTORAGEMODE_ProvidedByPersistence    = 1,         
    ORDERIDSTORAGEMODE_ProvidedByClient         = 2,
    };

//! Used to define which end of the relationship, source or target
//! @ingroup ECObjectsGroup
enum ECRelationshipEnd 
    { 
    ECRelationshipEnd_Source = 0, //!< End is the source
    ECRelationshipEnd_Target  //!< End is the target
    };

//! Used to describe the direction of a related instance within the context
//! of an IECRelationshipInstance
//! @ingroup ECObjectsGroup
enum class ECRelatedInstanceDirection
    {
    //! Related instance is the target in the relationship instance
    Forward = 1,
    //! Related instance is the source in the relationship instance
    Backward = 2
    };

//! The various strengths supported on a relationship class.
//! @ingroup ECObjectsGroup
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
//! @ingroup ECObjectsGroup
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
//! This class describes the relationship source or target cardinality. It also holds relationships key properties
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECRelationshipConstraintClass : NonCopyableClass
    {
    //__PUBLISH_SECTION_END__
    private:
        std::vector<WString> m_keys;
        ECClassCP m_ecClass;
    //__PUBLISH_SECTION_START__

    public:
        //! Constructor of ECRelationshipConstraintClass require ECClass name.
        //! @param ecClass is name of Constraint class.
        ECRelationshipConstraintClass(ECClassCR ecClass);

        //! Move constructor of ECRelationshipConstraintClass.
        ECRelationshipConstraintClass(ECRelationshipConstraintClass const && rhs);
        //! Move assignment operator of ECRelationshipConstraintClass.
        const ECRelationshipConstraintClass & operator = (ECRelationshipConstraintClass const && rhs);
        //! Returns reference of current Constraint ECClass
        ECOBJECTS_EXPORT ECClassCR GetClass() const;
        //! Returns vector of Contraint ECClass keys
        ECOBJECTS_EXPORT const std::vector<WString>& GetKeys() const;
        //! Adds constraint key if it is already not in list.
        ECOBJECTS_EXPORT void AddKey(WCharCP key);
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
        bool m_isMultiple;
   //__PUBLISH_SECTION_START__
    public:
        ECRelationshipConstraintClassList(ECRelationshipClassP relClass, bool isMultiple = false);
        //! Returns true if the constraint allows for a variable number of classes
        ECOBJECTS_EXPORT bool GetIsMultiple() const;
        ECOBJECTS_EXPORT iterator begin() const;    //!< Returns the beginning of the iterator
        ECOBJECTS_EXPORT iterator end() const;      //!< Returns the end of the iterator
        ECOBJECTS_EXPORT ECRelationshipConstraintClassCP operator[](size_t x)const; //!< Array operator overloaded
        //! Adds the specified class to the constraint.
        //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
        //! will replace the current class applied to the constraint with the new class.
        //! @param[out] classConstraint ECRelationshipConstraintClass for current ECClass
        //! @param[in] ecClass  The class to add
        ECOBJECTS_EXPORT ECObjectsStatus            Add(ECRelationshipConstraintClass*& classConstraint, ECClassCR ecClass);
        //! Clears the vector Constraint classes
        ECOBJECTS_EXPORT ECObjectsStatus            clear();
        //! Clears the vector Constraint classes
        ECOBJECTS_EXPORT uint32_t            size()const;
        //! Removes specified ECClass from Constraint class vector
        ECOBJECTS_EXPORT ECObjectsStatus            Remove(ECClassCR);
        ~ECRelationshipConstraintClassList();
           
    };


//=======================================================================================
//! The in-memory representation of the source and target constraints for an ECRelationshipClass as defined by ECSchemaXML
//! @ingroup ECObjectsGroup
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

    WString                     m_roleLabel;
    bool                        m_isPolymorphic;
    RelationshipCardinality*    m_cardinality;
    ECRelationshipClassP        m_relClass;

    ECObjectsStatus             SetCardinality(uint32_t& lowerLimit, uint32_t& upperLimit);

    SchemaWriteStatus           WriteXml (BeXmlNodeR parentNode, Utf8CP elementName) const;
    SchemaReadStatus            ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext);


protected:
    virtual ECSchemaCP          _GetContainerSchema() const override;

    //! Initializes a new instance of the ECRelationshipConstraint class.
    //! IsPolymorphic defaults to true and IsMultiple defaults to false
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass);  // WIP_CEM... should not be public... create a factory method

    //! Initializes a new instance of the ECRelationshipConstraint class
    ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isMultiple); // WIP_CEM... should not be public... create a factory method

/*__PUBLISH_SECTION_START__*/
public:
 
    ECOBJECTS_EXPORT virtual ~ECRelationshipConstraint(); //!< Destructor

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
    //! Sets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus            SetCardinality(WCharCP cardinality);

    //! Gets the cardinality of the constraint in the relationship
    ECOBJECTS_EXPORT RelationshipCardinalityCR  GetCardinality() const;

    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] classConstraint  The class to add
    ECOBJECTS_EXPORT ECObjectsStatus            AddClass(ECClassCR classConstraint);
    //! Adds the specified class to the constraint.
    //! If the constraint is variable, add will add the class to the list of classes applied to the constraint.  Otherwise, Add
    //! will replace the current class applied to the constraint with the new class.
    //! @param[in] ecClass  The class to add
    //! @param[out] classConstraint  list of contraint classes
    ECOBJECTS_EXPORT ECObjectsStatus            AddConstraintClass(ECRelationshipConstraintClass*& classConstraint, ECClassCR ecClass);

    //! Removes the specified class from the constraint.
    //! @param[in] classConstraint  The class to remove
    ECOBJECTS_EXPORT ECObjectsStatus            RemoveClass(ECClassCR classConstraint);

    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT const bvector<ECClassP> GetClasses() const;

    //! Returns the classes applied to the constraint.
    ECOBJECTS_EXPORT ECRelationshipConstraintClassList const & GetConstraintClasses() const;

    ECOBJECTS_EXPORT ECRelationshipConstraintClassList& GetConstraintClassesR() ;

    
    //! Copies this constraint to the destination
    ECOBJECTS_EXPORT ECObjectsStatus            CopyTo(ECRelationshipConstraintR toRelationshipConstraint);

    //! Returns whether the relationship is ordered on this constraint.
    ECOBJECTS_EXPORT bool                       GetIsOrdered () const;

    //! Returns the storage mode of the OrderId for this contraint.
    ECOBJECTS_EXPORT OrderIdStorageMode         GetOrderIdStorageMode () const;

    //! Gets the name of the OrderId property for this constraint.
    ECOBJECTS_EXPORT ECObjectsStatus            GetOrderedRelationshipPropertyName (WString& propertyName)  const;
};

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of a relationship class as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECRelationshipClass : public ECClass
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
    virtual ECRelationshipClassP        _GetRelationshipClassP ()  override {return this;};

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
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

//! Defines what sort of match should be used when locating a schema
//! @ingroup ECObjectsGroup
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
* @ingroup ECObjectsGroup
* Fully defines a schema with its name, major and minor versions, and a checksum
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct SchemaKey
    {
    WString       m_schemaName;
    uint32_t      m_versionMajor;
    uint32_t      m_versionMinor;
    uint32_t      m_checkSum;

    //! Creates a new SchemaKey with the given name and version information
    //! @param[in]  name    The name of the ECSchema
    //! @param[in]  major   The major portion of the version
    //! @param[in]  minor   The minor portion of the version
    SchemaKey (WCharCP name, uint32_t major, uint32_t minor) : m_schemaName(name), m_versionMajor(major), m_versionMinor(minor), m_checkSum(0){}

    //! Default constructor
    SchemaKey () : m_versionMajor(DEFAULT_VERSION_MAJOR), m_versionMinor(DEFAULT_VERSION_MINOR), m_checkSum(0) {}

    //! Given a full schema name (which includes the version information), will return a SchemaKey with the schema name and version information set
    //! @param[out] key             A SchemaKey with the schema's name and version set
    //! @param[in]  schemaFullName  The full name of the schema.
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (SchemaKey& key, WCharCP schemaFullName);

    //! Compares two SchemaKeys and returns whether the target schema is less than this SchemaKey, where LessThan is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SCHEMAMATCHTYPE_Identical - Returns whether the current schema's checksum is less than the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SCHEMAMATCHTYPE_Exact - This will first test the names, then the major version, and lastly the minor version
    //! @li SCHEMAMATCHTYPE_LatestCompatible - This will first test the names and then the major versions.
    //! @li SCHEMAMATCHTYPE_Latest - Returns whether the current schema's name is less than the target's.
    ECOBJECTS_EXPORT bool LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const;
    
    //! Compares two SchemaKeys and returns whether the target schema matches this SchemaKey, where "matches" is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SCHEMAMATCHTYPE_Identical - Returns whether the current schema's checksum is equal to the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SCHEMAMATCHTYPE_Exact - Returns whether this schema's name, major version, and minor version are all equal to the target's.
    //! @li SCHEMAMATCHTYPE_LatestCompatible - Returns whether this schema's name and major version are equal, and this schema's minor version is greater than or equal to the target's.
    //! @li SCHEMAMATCHTYPE_Latest - Returns whether the current schema's name is equal to the target's.
    ECOBJECTS_EXPORT bool Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const;

    //! Compares two schema names and returns whether the target schema matches this m_schemaName. Comparison is case insensitive
    //! @param[in]  schemaName  The schema name to compare to
    ECOBJECTS_EXPORT int  CompareByName (WString schemaName) const;

    //! Returns whether this SchemaKey is Identical to the target SchemaKey
    bool operator == (SchemaKeyCR rhs) const
        {
        return Matches(rhs, SCHEMAMATCHTYPE_Identical);
        }

    //! Returns true if the target SchemaKey is not Identical to this SchemaKey, false otherwise
    bool operator != (SchemaKeyCR rhs) const
        {
        return !(*this == rhs);
        }

    //! Returns whether this SchemaKey's checksum is less than the target SchemaKey's.
    bool operator < (SchemaKeyCR rhs) const
        {
        return LessThan (rhs, SCHEMAMATCHTYPE_Identical);
        }
/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT WStringCR GetName() const {return m_schemaName;}
    ECOBJECTS_EXPORT WString GetFullSchemaName() const;
    ECOBJECTS_EXPORT uint32_t GetVersionMajor() const { return m_versionMajor; };
    ECOBJECTS_EXPORT uint32_t GetVersionMinor() const { return m_versionMinor; };

/*__PUBLISH_SECTION_START__*/
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
* @ingroup ECObjectsGroup
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaNameClassNamePair
    {
public:
    WString m_schemaName;
    WString m_className;

    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair (WStringCR schemaName, WStringCR className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair (WCharCP schemaName, WCharCP className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs an empty SchemaNameClassNamePair
    SchemaNameClassNamePair() { }
    //! Constructs a SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    SchemaNameClassNamePair (WStringCR schemaAndClassNameSeparatedByColon)
        {
        BeAssert (WString::npos != schemaAndClassNameSeparatedByColon.find (':'));
        Parse (schemaAndClassNameSeparatedByColon);
        }

    //! Attempts to populate this SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    //! @param[in]      schemaAndClassNameSeparatedByColon a string of the format "SCHEMANAME:CLASSNAME"
    //! @return true if the string was successfully parsed, false otherwise. If it returns false, this SchemaNameClassNamePair will not be modified.
    bool Parse (WStringCR schemaAndClassNameSeparatedByColon)
        {
        size_t pos = schemaAndClassNameSeparatedByColon.find (':');
        if (WString::npos != pos)
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
    WString     ToColonSeparatedString() const
        {
        WString str;
        str.Sprintf (L"%ls:%ls", m_schemaName.c_str(), m_className.c_str());
        return str;
        }
    };

/*---------------------------------------------------------------------------------**//**
* Identifies an ECProperty by schema name, class name, and access string. The class name
* may refer to the ECClass containing the ECProperty, or a subclass thereof.
* @bsistruct                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct QualifiedECAccessor
    {
protected:
    WString         m_schemaName;
    WString         m_className;
    WString         m_accessString;
public:
    //! Constructs an empty QualifiedECAccessor
    QualifiedECAccessor() { }
    //! Constructs a QualifiedECAccessor referring to a property of an ECClass specified by access string
    QualifiedECAccessor (WCharCP schemaName, WCharCP className, WCharCP accessString)
        : m_schemaName(schemaName), m_className(className), m_accessString(accessString) { }

    //! Returns the name of the schema containing the ECClass
    WCharCP     GetSchemaName() const           { return m_schemaName.c_str(); }
    //! Returns the name of the ECClass (or subclass thereof) containing the ECProperty
    WCharCP     GetClassName() const            { return m_className.c_str(); }
    //! Returns the access string identifying the ECProperty within the ECClass
    WCharCP     GetAccessString() const         { return m_accessString.c_str(); }

    //! Sets the name of the schema containing the ECClass
    void        SetSchemaName (WCharCP name)    { m_schemaName = name; }
    //! Sets the name of the ECClass (or subclass thereof) containing the ECProperty
    void        SetClassName (WCharCP name)     { m_className = name; }
    //! Sets the access string identifying the ECProperty within the ECClass
    void        SetAccessString (WCharCP acStr) { m_accessString = acStr; }

    //! Returns a colon-separated string of the format "Schema:Class:AccessString"
    ECOBJECTS_EXPORT WString    ToString() const;
    //! Attempts to initialize this QualifiedECAccessor from a colon-separated string. If the string cannot be parsed, this QualifiedECAccessor will not be modified
    //! @param[in]      str A string of the format "Schema:Class:AccessString"
    //! @return true if the string was successfully parsed
    ECOBJECTS_EXPORT bool       FromString (WCharCP str);

    //! Attempts to initialize this QualifiedECAccessor from an access string identifying a property of the specified ECEnabler.
    //! If the ECProperty cannot be found within the ECEnabler, this QualifiedECAccessor will not be modified
    //! @param[in]      rootEnabler  The ECEnabler containing the desired ECProperty
    //! @param[in]      accessString The access string identifying the ECProperty within the ECEnabler
    //! @return true if the access string identifies a valid ECProperty within the ECEnabler
    ECOBJECTS_EXPORT bool       FromAccessString (ECN::ECEnablerCR rootEnabler, WCharCP accessString);
    };

typedef bvector<QualifiedECAccessor> QualifiedECAccessorList;
typedef QualifiedECAccessorList const& QualifiedECAccessorListCR;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaMapExact:bmap<SchemaKey, ECSchemaPtr, SchemaKeyLessThan <SCHEMAMATCHTYPE_Exact> >
    {
    //! Returns an iterator to the entry in this map matching the specified key using the specified match type, or an iterator to the end of this map if no such entry exists
    SchemaMapExact::const_iterator Find (SchemaKeyCR key, SchemaMatchType matchType) const
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
    //! @param[in]  classNamePair     The name of the class and schema to lookup.  This must be an unqualified (short) class name.
    //! @return   A pointer to an ECN::ECClass if the named class exists in within the current list; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP  FindClassP (ECN::SchemaNameClassNamePair const& classNamePair) const;
    };

typedef SchemaMapExact                  ECSchemaReferenceList;
typedef const ECSchemaReferenceList&    ECSchemaReferenceListCR;

//=======================================================================================
//! Supports STL like iterator of classes in a schema
//! @ingroup ECObjectsGroup
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

//__PUBLISH_CLASS_VIRTUAL__
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
//__PUBLISH_CLASS_VIRTUAL__
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
//! Interface to find a standalone enabler, typically for an embedded ECStruct in an ECInstance.
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct IStandaloneEnablerLocater
{
private:
    DECLARE_KEY_METHOD

/*__PUBLISH_SECTION_END__*/
protected:
    virtual    StandaloneECEnablerPtr  _LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className) = 0;

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/

public:
    //! Given a SchemaKey and a className, tries to locate the StandaloneEnabler for the ECClass
    //! @param[in] schemaKey    SchemaKey fully describing the schema that the class belongs to
    //! @param[in] className    The name of the class to find the enabler for
    //! @returns A valid StandaloneECEnabler, if one was located
    ECOBJECTS_EXPORT StandaloneECEnablerPtr  LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className);
};


//=======================================================================================
//! Interface implemented by class that provides schema location services.
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct IECSchemaLocater
{
protected:
    //! Tries to locate the requested schema.
    virtual ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) = 0;

public:
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
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECSchemaCache : public RefCountedBase
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
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Adds a schema to the cache
    //! @param[in] schema   The ECSchema to add to the cache
    //! @returns ECOBJECTS_STATUS_DuplicateSchema is the schema is already in the cache, otherwise ECOBJECTS_STATUS_Success
    ECOBJECTS_EXPORT ECObjectsStatus AddSchema   (ECSchemaR schema);

    //! Removes the specified schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema that should be removed from the cache
    //! @returns ECOBJECTS_STATUS_SchemaNotFound is the schema was not found in the cache, otherwise ECOBJECTS_STATUS_Success
    ECOBJECTS_EXPORT ECObjectsStatus DropSchema  (SchemaKeyCR key );

    //! Get the requested schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @returns The ECSchema if it is contained in the cache, NULL otherwise
    //! @remarks This will do an Identical match type for the requested schema
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key);

    //! Get the requested schema from the cache
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @param[in] matchType    The SchemaMatchType defining how exact of a match for the located schema is tolerated
    //! @returns The ECSchema if it is contained in the cache, NULL otherwise
    ECOBJECTS_EXPORT ECSchemaP       GetSchema   (SchemaKeyCR key, SchemaMatchType matchType);

    ECOBJECTS_EXPORT virtual ~ECSchemaCache (); //!< Destructor
    ECOBJECTS_EXPORT static  ECSchemaCachePtr Create (); //!< Creates an ECSchemaCachePtr
    ECOBJECTS_EXPORT int     GetCount(); //!< Returns the number of schemas currently in the cache
    ECOBJECTS_EXPORT void    Clear(); //!< Removes all schemas from the cache
    ECOBJECTS_EXPORT IECSchemaLocater& GetSchemaLocater(); //!< Returns the SchemaCache as an IECSchemaLocater
    ECOBJECTS_EXPORT size_t GetSchemas (bvector<ECSchemaP>& schemas) const;
};


/*__PUBLISH_SECTION_END__*/

//=======================================================================================
//! Locates schemas by looking in a given set of file system folder for ECSchemaXml files
//=======================================================================================
struct SearchPathSchemaFileLocater : IECSchemaLocater, RefCountedBase, NonCopyableClass
{
private:
    bvector<WString> m_searchPaths;
    SearchPathSchemaFileLocater (bvector<WString> const& searchPaths);
    virtual ~SearchPathSchemaFileLocater();
    static bool TryLoadingSupplementalSchemas(WStringCR schemaName, WStringCR schemaFilePath, ECSchemaReadContextR schemaContext, bvector<ECSchemaP>& supplementalSchemas);

    static ECSchemaPtr                  LocateSchemaByPath (SchemaKeyR key, ECSchemaReadContextR context, SchemaMatchType matchType, bvector<WString>& searchPaths);

    static ECSchemaPtr                  FindMatchingSchema (WStringCR schemaMatchExpression, SchemaKeyR key, ECSchemaReadContextR schemaContext, SchemaMatchType matchType, bvector<WString>& searchPaths);

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

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The in-memory representation of a schema as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECSchema : RefCountedBase
//__PUBLISH_SECTION_END__
                                          , IECCustomAttributeContainer
//__PUBLISH_SECTION_START__
{
private:
    ECSchema (ECSchema const&);
    ECSchema& operator= (ECSchema const&);

/*__PUBLISH_SECTION_END__*/
friend struct SearchPathSchemaFileLocater;
friend struct SupplementedSchemaBuilder;

// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    SchemaKey               m_key;
    WString                 m_namespacePrefix;
    WString                 m_displayLabel;
    mutable ECSchemaId      m_ecSchemaId;
    WString                 m_description;
    ECClassContainer        m_classContainer;

    // maps class name -> class pointer
    ClassMap                    m_classMap;
    ECSchemaReferenceList       m_refSchemaList;
    bool                        m_isSupplemented;
    bool                        m_hasExplicitDisplayLabel;
    SupplementalSchemaInfoPtr   m_supplementalSchemaInfo;
    bool                        m_immutable;

    bmap<ECSchemaP, WString> m_referencedSchemaNamespaceMap;

    ECSchema ();
    virtual ~ECSchema();

    bool                                AddingSchemaCausedCycles () const;
    void                                SetIsSupplemented(bool isSupplemented);
    bool                                IsOpenPlantPidCircularReferenceSpecialCase(WString& referencedECSchemaName);
    static SchemaReadStatus             ReadXml (ECSchemaPtr& schemaOut, BeXmlDomR xmlDom, uint32_t checkSum, ECSchemaReadContextR context);
    SchemaWriteStatus                   WriteXml (BeXmlDomR xmlDoc) const;

    ECObjectsStatus                     AddClass (ECClassP& pClass, bool deleteClassIfDuplicate = true);
    ECObjectsStatus                     SetVersionFromString (WCharCP versionString);
    ECObjectsStatus                     CopyConstraints(ECRelationshipConstraintR toRelationshipConstraint, ECRelationshipConstraintR fromRelationshipConstraint);

    void SetSupplementalSchemaInfo(SupplementalSchemaInfo* info);

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
    void                                CollectAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas,  bool includeRootSchema) const;

    bool                                IsSupplementalSchema();

protected:
    virtual ECSchemaCP                  _GetContainerSchema() const override;

public:
    ECOBJECTS_EXPORT void               ReComputeCheckSum ();
    //! Intended to be called by ECDb or a similar system
    ECOBJECTS_EXPORT void SetId(ECSchemaId id) { BeAssert (0 == m_ecSchemaId); m_ecSchemaId = id; };
    ECOBJECTS_EXPORT bool HasId() const { return m_ecSchemaId != 0; };

    ECOBJECTS_EXPORT ECObjectsStatus    DeleteClass (ECClassR ecClass);
    ECOBJECTS_EXPORT ECObjectsStatus    RenameClass (ECClassR ecClass, WCharCP newName);

//__PUBLISH_CLASS_VIRTUAL__
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
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMajor(uint32_t value);
    //! Gets the major version of this schema
    ECOBJECTS_EXPORT uint32_t           GetVersionMajor() const;
    //! Sets the minor version of this schema
    ECOBJECTS_EXPORT ECObjectsStatus    SetVersionMinor(uint32_t value);
    //! Gets the minor version of this schema
    ECOBJECTS_EXPORT uint32_t           GetVersionMinor() const;
    //! Returns an iterable container of ECClasses sorted by name. For unsorted called overload.
    ECOBJECTS_EXPORT ECClassContainerCR GetClasses() const;
    
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
    ECOBJECTS_EXPORT uint32_t           GetClassCount() const;

    //! Returns true if the display label has been set explicitly for this schema or not
    ECOBJECTS_EXPORT bool               GetIsDisplayLabelDefined() const;

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    ECOBJECTS_EXPORT bool               IsStandardSchema() const;

    //! Returns true if the passed in schema is the same base schema as the current schema
    //! @remarks FullName, NamespacePrefix, and ClassCount are checked
    //! @return True    if the schemas are the same
    ECOBJECTS_EXPORT bool               IsSamePrimarySchema(ECSchemaR primarySchema) const;

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
    //! @return   A non-refcounted pointer to an ECN::ECSchema if it can be successfully resolved from the specified namespacePrefix; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaCP         GetSchemaByNamespacePrefixP(WStringCR namespacePrefix) const;

    //! Resolve a namespace prefix for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup a namespace prefix in the context of this schema and its references.
    //! @param[out] namespacePrefix The namespace prefix if schema is a referenced schema; empty string if the sechema is the current schema;
    //! @return   Success if the schema is either the current schema or a referenced schema;  ECOBJECTS_STATUS_SchemaNotFound if the schema is not found in the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus    ResolveNamespacePrefix (ECSchemaCR schema, WStringR namespacePrefix) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A const pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassCP          GetClassCP (WCharCP name) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, NULL
    ECOBJECTS_EXPORT ECClassP           GetClassP (WCharCP name);

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

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml
    //          will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlString (Utf8StringR ecSchemaXml) const;

    //! Serializes an ECXML schema to a file
    //! @param[in]  ecSchemaXmlFile  The absolute path of the file to serialize the schema to
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed
    //          to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlFile (WCharCP ecSchemaXmlFile, bool utf16 = false) const;


    //! Writes an ECXML schema to an IStream
    //! @param[in]  ecSchemaXmlStream   The IStream to write the serialized XML to
    //! @param[in]  utf16            'false' (the default) to use utf-8 encoding
    //! @return A Status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the IStream
    //! will contain the serialized schema.
    ECOBJECTS_EXPORT SchemaWriteStatus  WriteToXmlStream (IStreamP ecSchemaXmlStream, bool utf16 = false);


    //! Return full schema name in format GetName().MM.mm where Name is the schema name, MM is major version and mm is minor version.
    ECOBJECTS_EXPORT WString               GetFullSchemaName () const;

    //! Given a source class, will copy that class into this schema if it does not already exist
    //! @param[out] targetClass If successful, will contain a new ECClass object that is a copy of the sourceClass
    //! @param[in]  sourceClass The class to copy
    ECOBJECTS_EXPORT ECObjectsStatus        CopyClass(ECClassP& targetClass, ECClassCR sourceClass);

    //! Copies this schema
    //! @param[out] schemaOut   If successful, will contain a copy of this schema
    ECOBJECTS_EXPORT ECObjectsStatus        CopySchema(ECSchemaPtr& schemaOut) const;

    //! Get the IECCustomAttributeContainer holding this schema's custom attributes
    ECOBJECTS_EXPORT IECCustomAttributeContainer&   GetCustomAttributeContainer();

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    //! Given a str containing SchemaXml, will compute the CheckSum
    ECOBJECTS_EXPORT static uint32_t        ComputeSchemaXmlStringCheckSum(WCharCP str, size_t len);

    //! If the given schemaName is valid, this will create a new schema object
    //! @param[out] schemaOut   if successful, will contain a new schema object
    //! @param[in]  schemaName  Name of the schema to be created.
    //! @param[in]  versionMajor The major version number.
    //! @param[in]  versionMinor The minor version number.
    //! @return A status code indicating whether the call was succesfull or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema (ECSchemaPtr& schemaOut, WStringCR schemaName,
                                                          uint32_t versionMajor, uint32_t versionMinor);

    //! Generate a schema version string given the major and minor version values.
    //! @param[in]  versionMajor    The major version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static WString        FormatSchemaVersion (uint32_t& versionMajor, uint32_t& versionMinor);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, uint32_t& versionMajor, uint32_t& versionMinor, WCharCP fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and major and minor versions (GetName().MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName (WString& schemaName, uint32_t& versionMajor, uint32_t& versionMinor, WStringCR fullName);

    //! Given a version string MM.NN, this will parse other major and minor versions
    //! @param[out] versionMajor    The major version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the major and minor versions (MM.NN)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString (uint32_t& versionMajor, uint32_t& versionMinor, WCharCP versionString);

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
    //! if (SCHEMA_READ_STATUS_Success != status)
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
    //! if (SCHEMA_READ_STATUS_Success != status)
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
    ECOBJECTS_EXPORT static bool IsStandardSchema(WStringCR schemaName);

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
struct IECClassLocater /*: RefCountedBase*/
    {
    protected:
        ECOBJECTS_EXPORT virtual ECClassCP _LocateClass (WCharCP schemaName, WCharCP className) = 0;
    public:
        ECClassCP LocateClass (WCharCP schemaName, WCharCP className)
            {
            return _LocateClass (schemaName, className);
            }

        //private:
        //    static IECClassLocaterPtr s_registeredClassLocater;
        //public:
        //    // TODO: This needs to migrate to the ECSchema implementation
        //    static ECOBJECTS_EXPORT void RegisterClassLocater (IECClassLocaterR classLocater);
        //    static ECOBJECTS_EXPORT void UnRegisterClassLocater ();
        //    static IECClassLocaterP GetRegisteredClassLocater();
    };

typedef IECClassLocater& IECClassLocaterR;

END_BENTLEY_ECOBJECT_NAMESPACE

//#pragma make_public (ECN::ECClass)
//#pragma make_public (ECN::ECSchema)
