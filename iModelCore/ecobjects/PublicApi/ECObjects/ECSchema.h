/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECName.h>
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/SchemaLocalizedStrings.h>
#include <ECObjects/ECUnit.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Nullable.h>
#include <Logging/bentleylogging.h>
#include <Formatting/FormattingApi.h>

#define DEFAULT_VERSION_READ       1
#define DEFAULT_VERSION_WRITE      0
#define DEFAULT_VERSION_MINOR      0

EC_TYPEDEFS(QualifiedECAccessor)

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

using EnumeratorList = bvector<ECEnumeratorP>;
using PropertyList = bvector<ECPropertyP>;

using ClassMap = bmap<Utf8CP, ECClassP, less_str>;
using EnumerationMap = bmap<Utf8CP, ECEnumerationP, less_str>;
using KindOfQuantityMap = bmap<Utf8CP, KindOfQuantityP, less_str>;
using PropertyMap = bmap<Utf8CP, ECPropertyP, less_str>;
using PropertyCategoryMap = bmap<Utf8CP, PropertyCategoryP, less_str>;
using UnitSystemMap = bmap<Utf8CP, UnitSystemP, less_str>;
using PhenomenonMap = bmap<Utf8CP, PhenomenonP, less_str>;
using UnitMap = bmap<Utf8CP, ECUnitP, less_str>;
using FormatMap = bmap<Utf8CP, ECFormatP, less_str>;

using ECCustomAttributeCollection = bvector<IECInstancePtr>;
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
    friend struct StandardCustomAttributeReferencesConverter;

    ECCustomAttributeCollection         m_primaryCustomAttributes;
    ECCustomAttributeCollection         m_supplementedCustomAttributes;
    bmap<IECInstanceCP, bvector<Utf8String>>  m_customAttributeXmlComments;
    SchemaWriteStatus                   AddCustomAttributeProperties (BeXmlNodeR oldNode, BeXmlNodeR newNode) const;

    IECInstancePtr                      GetCustomAttributeInternal(Utf8StringCR schemaName, Utf8StringCR className, bool includeBaseClasses, bool includeSupplementalAttributes) const;
    IECInstancePtr                      GetCustomAttributeInternal(ECClassCR ecClass, bool includeBaseClasses, bool includeSupplementalAttributes) const;
    bool                                IsDefinedInternal(Utf8StringCR schemaName, Utf8StringCR className, bool includeBaseClasses) const;
    bool                                IsDefinedInternal(ECClassCR classDefinition, bool includeBaseClasses) const;

    ECObjectsStatus                     SetCustomAttributeInternal(ECCustomAttributeCollection& customAttributeCollection, IECInstanceR customAttributeInstance, bool requireSchemaReference = false);
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetPrimaryCustomAttribute(IECInstanceR customAttributeInstance);

    //! LEGACY METHOD, use GetCustomAttributeInternal(Utf8StringCR schemaName, Utf8StringCR className, bool includeBaseClasses, bool includeSupplementalAttributes)
    IECInstancePtr                      GetCustomAttributeInternal(Utf8StringCR className, bool includeBaseClasses, bool includeSupplementalAttributes) const;

protected:
    //! Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     SetSupplementedCustomAttribute(IECInstanceR customAttributeInstance);

    CustomAttributeReadStatus           ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR context, ECSchemaCR fallBackSchema);
    SchemaWriteStatus                   WriteCustomAttributes(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion = ECVersion::Latest) const;
    bool                                WriteCustomAttributes(Json::Value& outValue) const;
    //! Only copies primary ones, not consolidated ones. Does not check if the container's ECSchema references the requisite ECSchema(s). @see SupplementedSchemaBuilder::SetMergedCustomAttribute
    ECObjectsStatus                     CopyCustomAttributesTo(IECCustomAttributeContainerR destContainer, bool copyReferences) const;

    void                                AddUniqueCustomAttributesToList(ECCustomAttributeCollection& returnList);
    void                                AddUniquePrimaryCustomAttributesToList(ECCustomAttributeCollection& returnList);
    virtual void                        _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const;
    virtual ECSchemaCP                  _GetContainerSchema() const = 0;
    virtual CustomAttributeContainerType _GetContainerType() const = 0;
    virtual Utf8CP                      _GetContainerName() const = 0;

    ECOBJECTS_EXPORT virtual ~IECCustomAttributeContainer();

public:
    ECOBJECTS_EXPORT ECSchemaP                           GetContainerSchema();
    Utf8CP                              GetContainerName() const { return _GetContainerName(); }

    //! Retrieves the local custom attribute matching the class name.  If the attribute is not 
    //! a supplemented attribute it will be copied and added to the supplemented list before it is returned.
    IECInstancePtr                      GetLocalAttributeAsSupplemented(Utf8StringCR schemaName, Utf8StringCR className);

    //! LEGACY METHOD, use GetLocalAttributeAsSupplemented(Utf8StringCR schemaName, Utf8StringCR className)
    IECInstancePtr                      GetLocalAttributeAsSupplemented(Utf8StringCR className);

    //! Returns true if the container has a custom attribute of a class of the specified name in the current container or base containers
    ECOBJECTS_EXPORT bool               IsDefined (Utf8StringCR schemaName, Utf8StringCR className) const;
    //! Returns true if the container has a custom attribute of a class of the specified class definition in the current container or base containers
    ECOBJECTS_EXPORT bool               IsDefined (ECClassCR classDefinition) const;
    //! Returns true if the container has a custom attribute of a class of the specified name in the current container, excluding base containers
    ECOBJECTS_EXPORT bool               IsDefinedLocal(Utf8StringCR schemaName, Utf8StringCR className) const;
    //! Returns true if the container has a custom attribute of a class of the specified class definition in the current container, excluding base containers
    ECOBJECTS_EXPORT bool               IsDefinedLocal(ECClassCR classDefinition) const;

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

    //! LEGACY METHODS
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
        ECCustomAttributeCollection* m_customAttributes;
        ECCustomAttributeCollection::const_iterator m_customAttributesIterator;

        IteratorState(IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes);
        ~IteratorState();

        static RefCountedPtr<IteratorState> Create (IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes)
            { return new IteratorState(container, includeBase, includeSupplementalAttributes); } ;
        };

    //! Iterator for the custom attribute instances
    struct const_iterator : std::iterator<std::forward_iterator_tag, IECInstancePtr const>
        {
        private:
            friend struct ECCustomAttributeInstanceIterable;
            RefCountedPtr<IteratorState> m_state;
            bool m_isEnd;
            const_iterator (IECCustomAttributeContainerCR container, bool includeBase, bool includeSupplementalAttributes);
            const_iterator () : m_isEnd(true) {;}
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

//=======================================================================================
//! Used to represent the type of an ECProperty
//! @bsiclass
//=======================================================================================
struct ECTypeDescriptor
{
private:
    ValueKind m_typeKind;
    
    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_primitiveType;
        };

public:
    //! Creates a TypeDescriptor for the given primitiveType
    //! @param[in] primitiveType    The primitive type to describe
    //! @returns an ECTypeDescriptor describing this primitive type
    ECOBJECTS_EXPORT static ECTypeDescriptor CreatePrimitiveTypeDescriptor(PrimitiveType primitiveType);

    //! Creates a TypeDescriptor of an array of the given primitiveType
    //! @param[in] primitiveType    The primitiveType to create an array descriptor for
    //! @returns An ECTypeDescriptor describing an array of the given primitiveType
    ECOBJECTS_EXPORT static ECTypeDescriptor CreatePrimitiveArrayTypeDescriptor(PrimitiveType primitiveType);

    //! Creates a TypeDescriptor for a struct array type
    ECOBJECTS_EXPORT static ECTypeDescriptor CreateStructArrayTypeDescriptor();
    //! Creates a TypeDescriptor for a struct
    ECOBJECTS_EXPORT static ECTypeDescriptor CreateStructTypeDescriptor();

    //! Creates a TypeDescriptor for a navigation property.  The type descriptor will be of the PrimitiveType 'type'.
    //! @param[in]  type         The type of the navigation property.  Use NavigationECProperty::GetType to get the type of a property
    //! @param[in]  isMultiple   True if the navigation property points to a relationship endpoint which allows more than one related instance.  Can use the result of the NavigationECProperty::IsMultiple method as input.
    //! @returns an ECTypeDescriptor describing a navigation property.
    ECOBJECTS_EXPORT static ECTypeDescriptor CreateNavigationTypeDescriptor(PrimitiveType type, bool isMultiple);

    //! Constructor that takes a PrimitiveType
    ECTypeDescriptor(PrimitiveType primitiveType) : m_typeKind(VALUEKIND_Primitive), m_primitiveType(primitiveType) { };

    ECTypeDescriptor() : m_typeKind((ValueKind) 0), m_primitiveType((PrimitiveType) 0) { };
    ECTypeDescriptor(ValueKind valueKind, short valueKindQualifier) : m_typeKind(valueKind), m_primitiveType((PrimitiveType)valueKindQualifier) { };

    bool operator==(ECTypeDescriptor const& rhs) const {return m_typeKind == rhs.m_typeKind && m_primitiveType == rhs.m_primitiveType;}
    bool operator!=(ECTypeDescriptor const& rhs) const {return !(*this == rhs);}

    ValueKind GetTypeKind() const {return m_typeKind;} //!< Returns the ValueKind of the ECProperty
    ArrayKind GetArrayKind() const {return (ArrayKind)(m_arrayKind & 0xFF);} //!< Returns the ArrayKind of the ECProperty, if the ECProperty is an array property
    bool IsPrimitive() const {return (GetTypeKind() == VALUEKIND_Primitive);} //!< Returns true if the ECProperty is a Primitive property
    bool IsStruct() const {return (GetTypeKind() == VALUEKIND_Struct);} //!< Returns true if the ECProperty is a Struct property
    bool IsArray() const {return (GetTypeKind() == VALUEKIND_Array);} //!< Returns true if the ECProperty is an Array property
    bool IsPrimitiveArray() const {return (GetTypeKind() == VALUEKIND_Array) && (GetArrayKind() == ARRAYKIND_Primitive);} //!< Returns true if the ECProperty is an Array property, and the array elements are Primitives
    bool IsStructArray() const {return (GetTypeKind() == VALUEKIND_Array) && (GetArrayKind() == ARRAYKIND_Struct);} //!< Returns true if the ECProperty is an Array property, and the array elements are Structs
    bool IsNavigation() const {return (GetTypeKind() == VALUEKIND_Navigation);} //!< Returns true if the ECProperty is a Navigation property

    PrimitiveType GetPrimitiveType() const {return m_primitiveType;} //!< Returns the primitive type of the ECProperty, if the property is a Primitive type
    short GetTypeKindQualifier() const {return m_primitiveType;}
};

/*=================================================================================**//**
Base class for an object which provides the context for an IECTypeAdapter
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IECTypeAdapterContext : RefCountedBase
{
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

    IECClassLocaterR GetUnitsECClassLocater() const {return _GetUnitsECClassLocater();}

    //! internal use only, primarily for ECExpressions
    typedef RefCountedPtr<IECTypeAdapterContext> (* FactoryFn)(ECPropertyCR, IECInstanceCR instance, uint32_t componentIndex);
    ECOBJECTS_EXPORT static void                RegisterFactory (FactoryFn fn);
    static RefCountedPtr<IECTypeAdapterContext> Create (ECPropertyCR ecproperty, IECInstanceCR instance, uint32_t componentIndex = COMPONENT_INDEX_None);
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
    //! If true and the value has sub/child values the value can be expanded to show child members
    ECOBJECTS_EXPORT bool                 AllowExpandMembers() const;

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
    bool                    m_priorityExplicitlySet;
    int32_t                 m_priority;
    Utf8String              m_description;
    ECValue                 m_minimumValue;
    ECValue                 m_maximumValue;
    uint32_t                m_minimumLength;
    uint32_t                m_maximumLength;
    ECValidatedName         m_validatedName;
    KindOfQuantityCP        m_kindOfQuantity;
    PropertyCategoryCP      m_propertyCategory;
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
    SchemaReadStatus                    ReadMinMaxXml(BeXmlNodeR propertyNode);

    virtual SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext);
    virtual SchemaWriteStatus           _WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion);
    SchemaWriteStatus                   _WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, ECVersion ecXmlVersion, bvector<bpair<Utf8CP, Utf8CP>>* attributes=nullptr, bool writeType=true);

    virtual bool           _ToJson(Json::Value& outValue, bool isInherited) const;
    bool                   _ToJson(Json::Value& outValue, bool isInherited, bvector<bpair<Utf8String, Json::Value>> attributes) const;

    virtual Utf8String                  _GetTypeNameForXml(ECVersion ecXmlVersion) const { return GetTypeName(); }
    void                                _AdjustMinMaxAfterTypeChange();

    virtual bool                        _IsPrimitive () const { return false; }
    virtual PrimitiveECPropertyCP       _GetAsPrimitivePropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual PrimitiveECPropertyP        _GetAsPrimitivePropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsStruct () const { return false; }
    virtual StructECPropertyCP          _GetAsStructPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual StructECPropertyP           _GetAsStructPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsArray () const { return false; }
    virtual ArrayECPropertyCP           _GetAsArrayPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual ArrayECPropertyP            _GetAsArrayPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsPrimitiveArray() const { return false; }
    virtual PrimitiveArrayECPropertyCP  _GetAsPrimitiveArrayPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual PrimitiveArrayECPropertyP   _GetAsPrimitiveArrayPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsStructArray() const { return false; }
    virtual StructArrayECPropertyCP     _GetAsStructArrayPropertyCP() const { return nullptr; } // used to avoid dynamic_cast
    virtual StructArrayECPropertyP      _GetAsStructArrayPropertyP()        { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _IsNavigation() const { return false; }
    virtual NavigationECPropertyCP      _GetAsNavigationPropertyCP() const  { return nullptr; } // used to avoid dynamic_cast
    virtual NavigationECPropertyP       _GetAsNavigationPropertyP()         { return nullptr; } // used to avoid dynamic_cast

    virtual bool                        _HasExtendedType() const { return false; }

    // This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with an alias
    // relative to the containing schema.
    virtual Utf8String                  _GetTypeName (bool useFullName = false) const = 0;
    virtual ECObjectsStatus             _SetTypeName (Utf8StringCR typeName) = 0;

    virtual bool                        _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const = 0;

    void _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;
    ECSchemaCP _GetContainerSchema() const override;
    Utf8CP _GetContainerName() const override;

    virtual CalculatedPropertySpecificationCP   _GetCalculatedPropertySpecification() const {return NULL;}
    virtual bool                                _IsCalculated() const {return false;}
    virtual bool                                _SetCalculatedPropertySpecification (IECInstanceP expressionAttribute) {return false;}

    virtual bool _IsSame(ECPropertyCR target) const;

    void                                InvalidateClassLayout();

public:
    // The following are used by the 'extended type' system which is currently implemented in DgnPlatform
    //!<@private
    IECTypeAdapter*                     GetCachedTypeAdapter() const { return m_cachedTypeAdapter; }
    //!<@private
    void                                SetCachedTypeAdapter (IECTypeAdapter* adapter) const { m_cachedTypeAdapter = adapter; }
    //!<@private
    IECTypeAdapter*                     GetTypeAdapter() const;

    //! Returns true if the property is explicitly set to read only.  Use GetIsReadOnly to determine if this property is actually readonly.
    bool                                IsReadOnlyFlagSet() const { return m_readOnly; }
    //! Returns true if this property was created during supplementation.  Properties created during supplementation should not be serialized.
    bool                                IsForSupplementation() const { return m_forSupplementation; }

    //! Intended to be called by ECDb or a similar system which uses an id to identify elements of a schema
    void SetId(ECPropertyId id) { BeAssert(!m_ecPropertyId.IsValid()); m_ecPropertyId = id; };
    //! returns true if this property has an id defined
    bool HasId() const { return m_ecPropertyId.IsValid(); };

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
    //! Returns whether this property is a StructECProperty
    bool                                GetIsStruct() const { return _IsStruct(); }
    //! Returns whether this property is an ArrayECProperty (either a PrimitiveArrayECProperty or a StructArrayECProperty)
    //! @remarks Use either GetIsPrimitiveArray() or GetIsStructArray() to differentiate.
    bool                                GetIsArray() const { return _IsArray(); }
    //! Returns whether this property is a PrimitiveECProperty
    bool                                GetIsPrimitive() const { return _IsPrimitive(); }
    //! Returns whether this property is a StructArrayECProperty
    bool                                GetIsStructArray() const { return _IsStructArray(); }
    //! Returns whether this property is a PrimitiveArrayECProperty
    bool                                GetIsPrimitiveArray() const { return _IsPrimitiveArray(); }
    //! Returns whether this property is a NavigationECProperty
    bool                                GetIsNavigation() const { return _IsNavigation(); }
    
    //! Sets the ECXML typename for the property.  @see GetTypeName()
    ECOBJECTS_EXPORT ECObjectsStatus    SetTypeName(Utf8String value);
    //! The ECXML typename for the property.
    //! The TypeName for struct properties will be the ECClass name of the struct.  It may be qualified with an alias if
    //! the struct belongs to a schema that is referenced by the schema actually containing this property.
    //! The TypeName for array properties will be the type of the elements the array contains.
    //! This method returns a wstring by value because it may be a computed string.  For instance struct properties may return a qualified typename with an alias
    //! relative to the containing schema.
    ECOBJECTS_EXPORT Utf8String         GetTypeName() const;
    //! The ECJSON typename for the property. Returns the typename qualified by the full schema name of the property in the form {schemaName}.{typeName}.
    ECOBJECTS_EXPORT Utf8String         GetTypeFullName() const;
    //! Sets the description for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    SetDescription(Utf8StringCR value);
    //! The Description of this ECProperty.  Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR       GetDescription() const;
    //! Gets the invariant description for this ECProperty.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDescription() const;
    //! Sets the Display Label for this ECProperty.
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel(Utf8StringCR value);
    //! Gets the Display Label for this ECProperty.  If no label has been set explicitly, it will return the Name of the property
    ECOBJECTS_EXPORT Utf8StringCR       GetDisplayLabel() const;
    //! Gets the invariant display label for this ECSchema.
    ECOBJECTS_EXPORT Utf8StringCR       GetInvariantDisplayLabel() const;

    //! Sets the minimum value for this ECProperty
    //! @remarks Only supported on PrimitiveECProperty and PrimitiveArrayECProperty for the following primitive types: ::PRIMITIVETYPE_Double, ::PRIMITIVETYPE_Integer, and ::PRIMITIVETYPE_Long
    ECOBJECTS_EXPORT ECObjectsStatus    SetMinimumValue(ECValueCR min);
    //! Gets whether the minimum value has been defined explicitly
    ECOBJECTS_EXPORT bool               IsMinimumValueDefined() const;
    //! Resets the minimum value that can be applied to this property
    ECOBJECTS_EXPORT void               ResetMinimumValue();
    //! Gets the minimum value for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    GetMinimumValue(ECValueR value) const;

    //! Sets the maximum value for this ECProperty
    //! @remarks Only supported on PrimitiveECProperty and PrimitiveArrayECProperty for the following primitive types: ::PRIMITIVETYPE_Double, ::PRIMITIVETYPE_Integer, and ::PRIMITIVETYPE_Long
    ECOBJECTS_EXPORT ECObjectsStatus    SetMaximumValue(ECValueCR max);
    //! Gets whether the maximum value has been defined explicitly
    ECOBJECTS_EXPORT bool               IsMaximumValueDefined() const;
    //! Resets the maximum value that can be applied to this property
    ECOBJECTS_EXPORT void               ResetMaximumValue();
    //! Gets the maximum value for this ECProperty
    ECOBJECTS_EXPORT ECObjectsStatus    GetMaximumValue(ECValueR value) const;

    //! Sets the maximum length for this ECProperty
    //! @remarks Supports only primitive types ::PRIMITIVETYPE_String and ::PRIMITIVETYPE_Binary.
    ECOBJECTS_EXPORT ECObjectsStatus SetMaximumLength(uint32_t max);
    //! Gets whether the maximum length has been defined explicitly
    bool IsMaximumLengthDefined() const {return m_maximumLength > 0;}
    //! Removed any maximum length that might have been applied to this ECProperty
    void ResetMaximumLength() {m_maximumLength = 0;}
    //! Gets the maximum length for this ECProperty
    //! @remarks Supports only primitive types ::PRIMITIVETYPE_String and ::PRIMITIVETYPE_Binary.
    uint32_t GetMaximumLength() const {return m_maximumLength;}

    //! Sets the minimum length for this ECProperty
    //! @remarks Supports only primitive types ::PRIMITIVETYPE_String and ::PRIMITIVETYPE_Binary.
    ECOBJECTS_EXPORT ECObjectsStatus SetMinimumLength(uint32_t min);
    //! Gets whether the minimum length has been defined explicitly
    bool               IsMinimumLengthDefined() const {return m_minimumLength > 0;}
    //! Removed any minimum length that might have been applied to this ECProperty
    void               ResetMinimumLength() {m_minimumLength = 0;}
    //! Gets the minimum length for this ECProperty
    //! @remarks Supports only primitive types ::PRIMITIVETYPE_String and ::PRIMITIVETYPE_Binary.
    uint32_t           GetMinimumLength() const {return m_minimumLength;}

    //! Sets the priority for this ECProperty. The default priority is 0.
    //! @param[in]  priority The value to set as the priority
    ECObjectsStatus SetPriority(int32_t priority) {m_priority = priority; m_priorityExplicitlySet = true; return ECObjectsStatus::Success;}
    //! Gets the priority for this ECProperty if it is set locally, otherwise will return it's base property priority if one exists.
    //! @return 0 if the priority is never set.
    ECOBJECTS_EXPORT int32_t GetPriority() const;
    //! Gets whether the priority is set locally.
    bool IsPriorityLocallyDefined() const {return m_priorityExplicitlySet;}

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

    //! Returns whether the KindOfQuantity has been set explicitly and not inherited from base property
    bool IsKindOfQuantityDefinedLocally() const {return nullptr != m_kindOfQuantity;}

    //! Gets the KindOfQuantity of this property or nullptr, if none has been set and cannot be inherited from base property
    ECOBJECTS_EXPORT KindOfQuantityCP GetKindOfQuantity() const;

    //! Sets the KindOfQuantity of this property, provide nullptr to unset.
    ECOBJECTS_EXPORT ECObjectsStatus SetKindOfQuantity(KindOfQuantityCP kindOfQuantity);

    //! Gets the PropertyCategory of this property or nullptr, if none has been set and cannot be inherited from base property
    ECOBJECTS_EXPORT PropertyCategoryCP GetCategory() const;

    //! Sets the PropertyCategory of this property, provide nullptr to unset.
    ECOBJECTS_EXPORT ECObjectsStatus SetCategory(PropertyCategoryCP propertyCategory);

    //! Returns whether the PropertyCategory has been set explicitly and not inherited from base property
    bool IsCategoryDefinedLocally() const {return nullptr != m_propertyCategory;}

    //! Returns whether this property has an extended type specified
    ECOBJECTS_EXPORT bool HasExtendedType() const;

    ArrayECPropertyCP           GetAsArrayProperty() const          {return _GetAsArrayPropertyCP();} //!< Returns the property as a const ArrayECProperty*
    ArrayECPropertyP            GetAsArrayPropertyP()               {return _GetAsArrayPropertyP();} //!< Returns the property as an ArrayECProperty*
    PrimitiveECPropertyCP       GetAsPrimitiveProperty() const      {return _GetAsPrimitivePropertyCP();} //!< Returns the property as a const PrimitiveECProperty*
    PrimitiveECPropertyP        GetAsPrimitivePropertyP()           {return _GetAsPrimitivePropertyP();} //!< Returns the property as a PrimitiveECProperty*
    PrimitiveArrayECPropertyCP  GetAsPrimitiveArrayProperty() const {return _GetAsPrimitiveArrayPropertyCP();} //!< Returns the property as a const PrimitiveArrayECProperty*
    PrimitiveArrayECPropertyP   GetAsPrimitiveArrayPropertyP()      {return _GetAsPrimitiveArrayPropertyP();} //!< Returns the property as a PrimitiveArrayECProperty*
    StructECPropertyCP          GetAsStructProperty() const         {return _GetAsStructPropertyCP();} //!< Returns the property as a const StructECProperty*
    StructECPropertyP           GetAsStructPropertyP()              {return _GetAsStructPropertyP();} //!< Returns the property as a StructECProperty*
    StructArrayECPropertyCP     GetAsStructArrayProperty() const    {return _GetAsStructArrayPropertyCP();} //!< Returns the property as a const StructArrayECProperty*
    StructArrayECPropertyP      GetAsStructArrayPropertyP()         {return _GetAsStructArrayPropertyP();} //!< Returns the property as a StructArrayECProperty*
    NavigationECPropertyCP      GetAsNavigationProperty() const     {return _GetAsNavigationPropertyCP();} //!< Returns the property as a const NavigationECProperty*
    NavigationECPropertyP       GetAsNavigationPropertyP()          {return _GetAsNavigationPropertyP();} //!< Returns the property as a NavigationECProperty*

    //! Returns whether this property is the same as the target property
    bool IsSame(ECPropertyCR target) const { return _IsSame(target); }
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct PrimitiveECProperty : public ECProperty
{
DEFINE_T_SUPER(ECProperty)
friend struct ECClass;
private:
    Utf8String          m_extendedTypeName;
    PrimitiveType       m_primitiveType;
    ECEnumerationCP     m_enumeration;
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;   // lazily-initialized

    PrimitiveECProperty(ECClassCR ecClass) : ECProperty(ecClass), m_primitiveType(PRIMITIVETYPE_String), m_enumeration(nullptr) {};
protected:
    SchemaReadStatus _ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) override;

    bool _ToJson(Json::Value& outValue, bool isInherited) const override;

    bool _IsPrimitive() const override {return true;}
    PrimitiveECPropertyCP _GetAsPrimitivePropertyCP() const override {return this;}
    PrimitiveECPropertyP _GetAsPrimitivePropertyP() override {return this;}
    Utf8String _GetTypeName(bool useFullName = false) const override;
    Utf8String _GetTypeNameForXml(ECVersion ecXmlVersion) const override;
    ECObjectsStatus _SetTypeName(Utf8StringCR typeName) override;
    bool _HasExtendedType() const override {return GetExtendedTypeName().size() > 0;}
    bool _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const override;
    CalculatedPropertySpecificationCP _GetCalculatedPropertySpecification() const override;
    bool _IsCalculated() const override {return m_calculatedSpec.IsValid() || GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification").IsValid();}
    bool _SetCalculatedPropertySpecification(IECInstanceP expressionAttribute) override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::PrimitiveProperty;}
    bool _IsSame(ECPropertyCR target) const override;

public:
    //! Sets the PrimitiveType of this ECProperty.  The default type is ::PRIMITIVETYPE_String
    ECOBJECTS_EXPORT ECObjectsStatus SetType(PrimitiveType value);
    
    //! Gets the PrimitiveType of this ECProperty
    PrimitiveType GetType() const {return m_primitiveType;}
    
    //! Sets an ECEnumeration as type of this ECProperty.
    ECOBJECTS_EXPORT ECObjectsStatus SetType(ECEnumerationCR value);
    
    //! Gets the Enumeration of this ECProperty or nullptr if none used.
    ECEnumerationCP GetEnumeration() const {return m_enumeration;}

    //! Returns whether the ExtendedTypeName has been set explicitly and not inherited from base property
    bool IsExtendedTypeDefinedLocally() const {return m_extendedTypeName.size() > 0;}

    //! Gets the extended type name of this ECProperty, if it is not defined locally will be inherited from base property if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetExtendedTypeName() const;

    //! Sets the Name of the Extended Type of this property.
    ECOBJECTS_EXPORT ECObjectsStatus SetExtendedTypeName(Utf8CP extendedTypeName);

    //! Resets the extended type on this property.
    bool RemoveExtendedTypeName() {return ECObjectsStatus::Success == this->SetExtendedTypeName(nullptr);}
};

//=======================================================================================
//! The in-memory representation of an ECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct StructECProperty : public ECProperty
{
DEFINE_T_SUPER(ECProperty)
friend struct ECClass;
friend struct ECStructClass;
private:
    ECStructClassCP   m_structType;

    StructECProperty (ECClassCR ecClass) : m_structType(NULL), ECProperty(ecClass) {};

protected:
    SchemaReadStatus _ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) override;

    bool _ToJson(Json::Value& outValue, bool isInherited) const override;

    bool _IsStruct() const override { return true;}
    StructECPropertyCP _GetAsStructPropertyCP() const override {return this;}
    StructECPropertyP _GetAsStructPropertyP() override {return this;}
    Utf8String _GetTypeName(bool useFullName = false) const override;
    ECObjectsStatus _SetTypeName(Utf8StringCR typeName) override;
    bool _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::StructProperty;}
    bool _IsSame(ECPropertyCR target) const override;

public:
    //! The property type.
    //! This type must be an ECStructClass.
    ECOBJECTS_EXPORT ECObjectsStatus SetType(ECStructClassCR value);
    //! Gets the ECStructClass that defines the type for this property
    ECOBJECTS_EXPORT ECStructClassCR GetType() const; 
};

//=======================================================================================
//! The in-memory representation of an ECArrayProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArrayECProperty /*abstract*/ : public ECProperty
{
DEFINE_T_SUPER(ECProperty)
friend struct ECClass;

private:
    uint32_t                                    m_minOccurs;
    uint32_t                                    m_maxOccurs;    // D-106653 we store this as read from the schema, but all arrays are considered to be of unbounded size
    mutable IECTypeAdapter*                     m_cachedMemberTypeAdapter;

protected:
    ArrayKind m_arrayKind;

    ArrayECProperty(ECClassCR ecClass) : ECProperty(ecClass), m_minOccurs(0), m_maxOccurs(INT_MAX), m_cachedMemberTypeAdapter(nullptr) {};

    ECObjectsStatus                     SetMinOccurs(Utf8StringCR minOccurs);
    ECObjectsStatus                     SetMaxOccurs(Utf8StringCR maxOccurs);

    SchemaReadStatus            _ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    SchemaWriteStatus           _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) override;

    bool                        _IsArray () const override {return true;}
    ArrayECPropertyCP           _GetAsArrayPropertyCP() const override {return this;}
    ArrayECPropertyP            _GetAsArrayPropertyP() override {return this;}
    bool _IsSame(ECPropertyCR target) const override;

public:
    // The following are used by the 'extended type' system which is currently implemented in DgnPlatform
    //!<@private
    IECTypeAdapter*                     GetCachedMemberTypeAdapter() const  { return m_cachedMemberTypeAdapter; }
    //!<@private
    void                                SetCachedMemberTypeAdapter (IECTypeAdapter* adapter) const { m_cachedMemberTypeAdapter = adapter; }
    //!<@private
    IECTypeAdapter*                     GetMemberTypeAdapter() const;

    //! The ArrayKind of this ECProperty
    ArrayKind GetKind() const {return m_arrayKind;}

    //! Sets the minimum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus SetMinOccurs(uint32_t value);

    //! Gets the minimum number of array members.
    uint32_t GetMinOccurs() const {return m_minOccurs;}
    
    //! Sets the maximum number of array members.
    ECOBJECTS_EXPORT ECObjectsStatus    SetMaxOccurs(uint32_t value);
    
    //! Gets the maximum number of array members.
    //! defect 106653 https://tfs.bentley.com/tfs/ProductLine/Shared%20Components/_workitems?id=106653&_a=edit
    //! defect 120202 https://bentleycs.visualstudio.com/iModelTechnologies/_workitems/edit/120202
    uint32_t GetMaxOccurs() const {return /* m_maxOccurs; */ INT_MAX;}

    //! Because of a legacy bug GetMaxOccurs always returns "unbounded". For components that need to persist
    //! the ECSchema as is, GetStoredMaxOccurs can be called as a workaround until the max occurs issue has been resolved.
    uint32_t GetStoredMaxOccurs () const {return m_maxOccurs;}

    //! Indicates if the stored max occurs is unbound
    bool IsStoredMaxOccursUnbounded() const { return m_maxOccurs == INT_MAX; }
   };

//=======================================================================================
//! The in-memory representation of an PrimitiveArrayECProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct PrimitiveArrayECProperty : ArrayECProperty
{
DEFINE_T_SUPER(ArrayECProperty)
friend struct ECClass;

private: 
    Utf8String          m_extendedTypeName;
    PrimitiveType       m_primitiveType;
    ECEnumerationCP     m_enumeration;
    mutable CalculatedPropertySpecificationPtr  m_calculatedSpec;

protected:
    PrimitiveArrayECProperty(ECClassCR ecClass) : ArrayECProperty(ecClass), m_primitiveType(PRIMITIVETYPE_String), m_enumeration(nullptr)
        {
        m_arrayKind = ARRAYKIND_Primitive;
        };

    SchemaReadStatus _ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    bool _ToJson(Json::Value& outValue, bool isInherited) const override;
    Utf8String _GetTypeName(bool useFullName = false) const override;
    Utf8String _GetTypeNameForXml(ECVersion ecXmlVersion) const override;
    ECObjectsStatus _SetTypeName(Utf8StringCR typeName) override;
    bool _HasExtendedType() const override {return GetExtendedTypeName().size() > 0;}
    bool _IsPrimitiveArray() const override {return true;}
    PrimitiveArrayECPropertyCP _GetAsPrimitiveArrayPropertyCP() const override {return this;}
    PrimitiveArrayECPropertyP _GetAsPrimitiveArrayPropertyP() override {return this;}

    bool _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::PrimitiveArrayProperty;}

    CalculatedPropertySpecificationCP _GetCalculatedPropertySpecification() const override;
    bool _IsCalculated() const override {return m_calculatedSpec.IsValid() || GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification").IsValid();}
    bool _SetCalculatedPropertySpecification(IECInstanceP expressionAttribute) override;

public:
    //! Sets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    ECOBJECTS_EXPORT ECObjectsStatus SetPrimitiveElementType(PrimitiveType value);

    //! Gets the PrimitiveType if this ArrayProperty contains PrimitiveType elements
    PrimitiveType GetPrimitiveElementType() const {return m_primitiveType;}

    //! Sets an ECEnumeration as type of this ECProperty.
    ECOBJECTS_EXPORT ECObjectsStatus SetType(ECEnumerationCR value);

    //! Gets the Enumeration of this ECProperty or nullptr if none used.
    ECEnumerationCP GetEnumeration() const {return m_enumeration;}

    //! Returns whether the ExtendedTypeName has been set explicitly and not inherited from base property
    bool IsExtendedTypeDefinedLocally() const {return m_extendedTypeName.size() > 0;}

    //! Gets the extended type name of this ECProperty, if it is not defined locally will be inherited from base property if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetExtendedTypeName() const;

    //! Sets the Name of the Extended Type of this property.
    ECOBJECTS_EXPORT ECObjectsStatus SetExtendedTypeName(Utf8CP extendedTypeName);

    //! Resets the extended type on this property.
    bool RemoveExtendedTypeName() {return ECObjectsStatus::Success == this->SetExtendedTypeName(nullptr);}
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
    StructArrayECProperty(ECClassCR ecClass) : ArrayECProperty(ecClass)
        {
        m_arrayKind = ARRAYKIND_Struct;
        };

protected:
    bool _ToJson(Json::Value& outValue, bool isInherited) const override;
    Utf8String _GetTypeName(bool useFullName = false) const override;
    ECObjectsStatus _SetTypeName(Utf8StringCR typeName) override;
    bool _IsStructArray() const override {return true;}
    StructArrayECPropertyCP _GetAsStructArrayPropertyCP() const override {return this;}
    StructArrayECPropertyP _GetAsStructArrayPropertyP() override {return this;}
    bool _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::StructArrayProperty;}

public:
    //! Sets the ECClass to be used for the array's struct elements
    ECOBJECTS_EXPORT ECObjectsStatus SetStructElementType(ECStructClassCR value);
    //! Gets the ECClass of the array's struct elements
    ECOBJECTS_EXPORT ECStructClassCR GetStructElementType() const;
};

//=======================================================================================
//! The in-memory representation of an ECNavigationProperty as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NavigationECProperty : public ECProperty
{
DEFINE_T_SUPER(ECProperty)

friend struct ECEntityClass;
friend struct ECRelationshipClass;
friend struct ECClass;
private:
    ECRelationshipClassCP       m_relationshipClass;
    ECRelatedInstanceDirection  m_direction;
    PrimitiveType               m_type;
    ValueKind                   m_valueKind;

protected:
    explicit NavigationECProperty(ECClassCR ecClass)
        : ECProperty(ecClass), m_relationshipClass(nullptr), m_direction(ECRelatedInstanceDirection::Forward), m_valueKind(ValueKind::VALUEKIND_Uninitialized), m_type(PrimitiveType::PRIMITIVETYPE_Long) {};

    ECObjectsStatus SetRelationshipClassName(Utf8CP relationshipName);
    ECObjectsStatus SetDirection(Utf8CP directionString);
    Utf8String GetRelationshipClassName() const;

protected:
    SchemaReadStatus _ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext) override;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) override;
    bool _ToJson(Json::Value& outValue, bool isInherited) const override;

    bool _IsNavigation() const override {return true;}
    NavigationECPropertyCP _GetAsNavigationPropertyCP() const override {return this;}
    NavigationECPropertyP _GetAsNavigationPropertyP() override {return this;}

    // Not valid because type cannot be set from xml, it must be set at runtime
    ECObjectsStatus _SetTypeName(Utf8StringCR typeName) override {return ECObjectsStatus::OperationNotSupported;}
    Utf8String _GetTypeName(bool useFullName = false) const override;

    bool _CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::NavigationProperty;}

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
        public:
            PropertyList::const_iterator     m_listIterator;
            PropertyList*                    m_properties;

            IteratorState (ECClassCR ecClass, bool includeBaseProperties);
            ~IteratorState();
            static RefCountedPtr<IteratorState> Create (ECClassCR ecClass, bool includeBaseProperties)
                { return new IteratorState(ecClass, includeBaseProperties); };
        };

    //! Iterator over the properties
    struct const_iterator : std::iterator<std::forward_iterator_tag, const ECPropertyP>
        {
        private:
            friend struct ECPropertyIterable;
            RefCountedPtr<IteratorState>   m_state;
            bool m_isEnd;

            const_iterator (ECClassCR ecClass, bool includeBaseProperties);
            const_iterator () : m_isEnd(true) {};
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

//=======================================================================================
//! An iterable container of ECEnumerators
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
    const_iterator begin() const {return m_list.begin();}
    const_iterator end() const {return m_list.end();}
};

struct ECEnumeration;

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
    ECValidatedName m_validatedName;
    Utf8String m_description;

    //  Lifecycle management:  The enumeration implementation will
    //  serve as a factory for enumerators and will manage their lifecycle.
    explicit ECEnumerator(ECEnumerationCR parent, int32_t value) : m_enum(parent), m_intValue(value) {}
    explicit ECEnumerator(ECEnumerationCR parent, Utf8CP value) : m_enum(parent), m_stringValue(value) {}
    ~ECEnumerator() {}

public:
    //! The ECEnumeration that this enumerator is defined in
    ECEnumerationCR GetEnumeration() const {return m_enum;}
    //! Whether the display label is explicitly defined or not
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();}
    //! Sets the display label of this enumerator
    ECOBJECTS_EXPORT void SetDisplayLabel(Utf8StringCR value);
    //! Gets the localized display label of this enumerator. If no display label has been set explicitly, it will return the name of the enumerator.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    //! Gets the invariant display label for this enumerator. If no display label has been set explicitly, it will return the name of the enumerator.
    ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const;

    //! Sets the name of this enumerator.
    ECOBJECTS_EXPORT ECObjectsStatus SetName(Utf8StringCR name);
    //! Gets the name of this enumerator.
    Utf8StringCR GetName() const {return m_validatedName.GetName();}

    //! Sets the description of this enumerator.
    void SetDescription(Utf8StringCR value) {m_description = value;}
    //! Gets the description of this enumerator. Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const;
    //! Gets the invariant description for this enumerator.
    Utf8StringCR GetInvariantDescription() const {return m_description;}

    //!Returns true if this enumerator holds an integer value
    ECOBJECTS_EXPORT bool IsInteger() const;
    //! Returns the integer value, if this ECEnumerator holds an Integer 
    int32_t GetInteger() const {return m_intValue;}
    //! Sets the value of this ECEnumerator to the given integer
    //! @param[in] integer  The value to set
    ECOBJECTS_EXPORT ECObjectsStatus SetInteger(int32_t integer);
    //!Returns true if this enumerator holds an integer value
    ECOBJECTS_EXPORT bool IsString() const;
    //! Gets the string content of this ECEnumerator in UTF-8 encoding.
    //! @return string content in UTF-8 encoding
    Utf8StringCR GetString() const {return m_stringValue;}
    //! Sets the value of this ECEnumerator to the given string
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
    //! @param[in] value The value to set
    ECOBJECTS_EXPORT ECObjectsStatus SetString(Utf8CP value);

    //! Returns the name used to create an ECEnumerator.
    //! @param[in] enumerationName          Name of the enumerator's parent enumeration.
    //! @param[in] enumeratorValueString    Value of the enumerator if the enumeration is of type string, nullptr if the enumeration is of type integer.
    //! @param[in] enumeratorValueInteger   Value of the enumerator if the enumeration is of type integer, nullptr if the enumeration is of type string.
    ECOBJECTS_EXPORT static Utf8String DetermineName(Utf8StringCR enumerationName, Utf8CP enumeratorValueString, int32_t const* enumeratorValueInteger);
};

//=======================================================================================
//! The in-memory representation of an ECEnumeration as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct ECEnumeration : NonCopyableClass
{
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    ECSchemaCR m_schema;
    mutable Utf8String m_fullName;
    ECValidatedName m_validatedName;
    PrimitiveType m_primitiveType;
    Utf8String m_description;
    EnumeratorList m_enumeratorList;
    EnumeratorIterable m_enumeratorIterable;
    bool m_isStrict;
    mutable ECEnumerationId m_ecEnumerationId;

    //  Lifecycle management:  The schema implementation will
    //  serve as a factory for enumerations and will manage their lifecycle.
    explicit ECEnumeration(ECSchemaCR schema) : m_schema(schema), m_primitiveType(PrimitiveType::PRIMITIVETYPE_Integer), 
        m_isStrict(true), m_enumeratorList(), m_enumeratorIterable(m_enumeratorList) { }
    ~ECEnumeration()
        {
        for (ECEnumerator* entry : m_enumeratorList)
            delete entry;
        m_enumeratorList.clear();
        }

    // schemas index enumeration by name so publicly name can not be reset
    ECObjectsStatus SetName(Utf8CP name);

    //! Sets the PrimitiveType of this Enumeration.  The default type is ::PRIMITIVETYPE_Integer
    ECObjectsStatus SetType(PrimitiveType value);
    //! Sets the backing primitive type by its name.
    ECObjectsStatus SetTypeName(Utf8CP typeName);

    SchemaReadStatus ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    //! Returns true if an enumerator with the given name and value would not conflict with existing enumerators within this enumeration.
    bool EnumeratorIsUnique(Utf8CP enumeratorName, int32_t enumeratorValue) const;
    bool EnumeratorIsUnique(Utf8CP enumeratorName, Utf8CP enumeratorValue) const;

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this enumeration is defined in

    void SetId(ECEnumerationId id) {BeAssert(!m_ecEnumerationId.IsValid()); m_ecEnumerationId = id;} //!< Intended to be called by ECDb or a similar system
    ECEnumerationId GetId() const {BeAssert(HasId()); return m_ecEnumerationId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    bool HasId() const {return m_ecEnumerationId.IsValid();}
    
    Utf8StringCR GetName() const {return m_validatedName.GetName();} //!< The name of this Enumeration
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this ECEnumeration in the format {SchemaName}:{EnumerationName}.
    //!< Gets a qualified name of the ECEnumeration, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    void SetDescription(Utf8CP value) {m_description = value;} //!< Sets the description of this ECEnumeration
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this ECEnumeration.  Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECEnumeration.

    //! Sets the display label of this ECEnumeration
    ECOBJECTS_EXPORT void SetDisplayLabel(Utf8CP displayLabel);
    //! Gets the display label of this ECEnumeration.  If no display label has been set explicitly, it will return the name of the ECEnumeration
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    Utf8StringCR GetInvariantDisplayLabel() const {return m_validatedName.GetDisplayLabel();} //!< Gets the invariant display label for this ECEnumeration.
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();} //!< Whether the display label is explicitly defined or not

    PrimitiveType GetType() const { return m_primitiveType; } //!< Gets the PrimitiveType of this ECEnumeration.
    ECOBJECTS_EXPORT Utf8String GetTypeName() const; //!< Gets the name of the backing primitive type.

    //! Sets the IsStrict flag to a given value. NonStrict enums will be treated as suggestions and not enforce values.
    void SetIsStrict(bool value) { m_isStrict = value; }
    //! Gets the IsStrict flag of this enum. True means that values on properties will be enforced.
    bool GetIsStrict() const {return m_isStrict;}

    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR name, Utf8CP value); //!< Creates a new enumerator at the end of this enumeration.
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR name, int32_t value); //!< Creates a new enumerator at the end of this enumeration.
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, Utf8CP value); //!< Creates a new enumerator at the end of this enumeration. The name is set to the string given in the value parameter
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerator(ECEnumeratorP& enumerator, int32_t value); //!< Creates a new enumerator at the end of this enumeration. The name is set as [enumeration name][value]
    EnumeratorIterable const& GetEnumerators() const { return m_enumeratorIterable; } //!< Get the enumerator list held by this object
    size_t GetEnumeratorCount() const { return m_enumeratorList.size(); } //!< Get the amount of enumerators in this enumeration

    ECOBJECTS_EXPORT ECEnumeratorP FindEnumeratorByName(Utf8CP name) const; //!< Finds the enumerator with the provided name, returns nullptr if none found.
    ECOBJECTS_EXPORT ECEnumeratorP FindEnumerator(int32_t value) const; //!< Finds the enumerator with the provided integer value, returns nullptr if none found.
    ECOBJECTS_EXPORT ECEnumeratorP FindEnumerator(Utf8CP value) const; //!< Finds the enumerator with the provided string value, returns nullptr if none found.

    ECOBJECTS_EXPORT ECObjectsStatus DeleteEnumerator(ECEnumeratorCR enumerator); //!< Removes the provided enumerator from this enumeration    
    ECOBJECTS_EXPORT void Clear(); //!< Removes all enumerators in this enumeration

    //! Write the Enumeration as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false) const {return ToJson(outValue, true, includeSchemaVersion);};
};

//=======================================================================================
//! The in-memory representation of a KindOfQuantity as defined by ECSchemaXML. KindOfQuantities
//! reference both units and formats which are gone over here:
//! @see @ref UnitsOverview Units
//! @see @ref FormatsOverview Formats
//! @bsiclass
//=======================================================================================
struct KindOfQuantity : NonCopyableClass
{
friend struct ECSchema; // needed for SetName() method 
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

typedef bvector<bpair<ECUnitCP, Nullable<Utf8String>>> UnitAndLabelPairs;
private:
    ECSchemaCR m_schema;
    mutable Utf8String m_fullName; //cached nsprefix:name representation
    ECValidatedName m_validatedName; //wraps name and displaylabel
    Utf8String m_description;
    mutable bpair<Utf8String, bvector<Utf8String>> m_descriptorCache;
    mutable bool m_descriptorCacheValid = false;

    //! Unit used for persistence. Will be formatted with a default format if no formats are provided.
    ECUnitCP m_persistenceUnit;
    //! Absolute Error divided by the measured value.
    double m_relativeError;
    //! Formats which can be used for presentation. First format is default.
    bvector<NamedFormat> m_presentationFormats;
    //! Stores a format override using the persistence unit as the major unit. Also uses persistence unit display label
    mutable Nullable<NamedFormat> m_persFormatCache;
    mutable KindOfQuantityId m_kindOfQuantityId;

    //  Lifecycle management:  The schema implementation will
    //  serve as a factory for kind of quantities and will manage their lifecycle.
    explicit KindOfQuantity(ECSchemaCR schema) : m_schema(schema), m_persistenceUnit(nullptr), m_persFormatCache(nullptr), m_relativeError(1.0) {};

    ~KindOfQuantity() {};

    // schemas index KindOfQuantity by name so publicly name can not be reset
    ECObjectsStatus SetName(Utf8CP name);
    bool Verify() const;

    SchemaReadStatus ReadXml(BeXmlNodeR kindOfQuantityNode, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECSchemaR GetSchemaR() const {return const_cast<ECSchemaR>(m_schema);}
    //! Private function for adding a copy of an existing named format. Currently used by CopySchema as a convenience method.
    ECOBJECTS_EXPORT ECObjectsStatus AddPresentationFormatInternal(NamedFormat format);
    //! Gets the cached persistence format. Creates one based on Formatting::NumericFormatSpec::DefaultFormat() if it does not exist.
    ECOBJECTS_EXPORT NamedFormatCP GetCachedPersistenceFormat() const;
    ECObjectsStatus CreateOverrideString(Utf8StringR out, ECFormatCR parent, Nullable<int32_t> precisionOverride = nullptr, UnitAndLabelPairs const* unitsAndLabels = nullptr) const;
    ECOBJECTS_EXPORT static bool ValidatePresentationFormat(ECFormatCR parent, ECUnitCP persistenceUnit, Nullable<int32_t> precisionOverride, KindOfQuantity::UnitAndLabelPairs const* unitsAndLabels);
    ECOBJECTS_EXPORT static bool ValidatePresentationFormat(Utf8StringCR formatString, ECUnitCP persistenceUnit, ECSchemaCR formats, ECSchemaCR units);
    ECOBJECTS_EXPORT static ECObjectsStatus TransformFormatString(ECFormatCP& outFormat, Nullable<int32_t>& outPrec, UnitAndLabelPairs& outPairs, Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper, ECSchemaCP koqSchema = nullptr);

    ECOBJECTS_EXPORT void ValidateDescriptorCache() const;
    ECOBJECTS_EXPORT ECObjectsStatus FormatToFUSDescriptor(Utf8StringR outFUSDescriptor, NamedFormatCR format) const;

public:
    const bpair<Utf8String, bvector<Utf8String>>& GetDescriptorCache() const { ValidateDescriptorCache(); return m_descriptorCache;} //!< Get the cache of FUS descriptors
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this kind of quantity is defined in.

    void SetId(KindOfQuantityId id) { BeAssert(!m_kindOfQuantityId.IsValid()); m_kindOfQuantityId = id; } //!< Intended to be called by ECDb or a similar system
    KindOfQuantityId GetId() const {BeAssert(HasId()); return m_kindOfQuantityId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    bool HasId() const {return m_kindOfQuantityId.IsValid();}

    Utf8StringCR GetName() const {return m_validatedName.GetName();} //!< The name of this KindOfQuantity
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this KindOfQuantity in the format {SchemaName}:{KindOfQuantityName}.
    //! Gets a qualified name of the kind of quantity, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    //! Sets the description of this KindOfQuantity
    //! @param[in]  value  The new value to apply
    ECObjectsStatus SetDescription(Utf8CP value) {m_description = value; return ECObjectsStatus::Success;}
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Get the description of this KindOfQuantity
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description of this KindOfQuantity.

    //! Sets the display label of this KindOfQuantity
    //! @param[in]  value  The new value to apply
    ECOBJECTS_EXPORT ECObjectsStatus SetDisplayLabel(Utf8CP value);
    //! Gets the display label of this KindOfQuantity.  If a display label has not been explicitly set, returns the name of the KindOfQuantity.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    //! Gets the invariant display label of this KindOfQuantity.  If a display label has not been explicitly set, returns the name of the KindOfQuantity.
    Utf8StringCR GetInvariantDisplayLabel() const {return m_validatedName.GetDisplayLabel();}
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();} //!< Whether the display label is explicitly defined or not

    //! Sets the Precision used for persisting the information. A precision of zero indicates that a default will be used.
    //! @param[in]  value  The new value to apply
    void SetRelativeError(double value) {m_relativeError = value;}
    double GetRelativeError() const {return m_relativeError;}; //!< Gets the precision used for persisting the information. A precision of zero indicates that a default will be used.

    //! Given a string representating the name of a unit and a mapping function, adds the unit as the persistence unit if it is found
    //! by the mapping function
    //!
    //! @param[in] unitName           The name of the unit to be added as a persistence unit
    //! @param[in] nameToUnitMapper   Function that maps unit names to ECUnitCP
    //! @return ECObjectsStatus::Success if successfully updates the persistence unit. Otherwise error codes
    ECOBJECTS_EXPORT ECObjectsStatus AddPersistenceUnitByName(Utf8StringCR unitName, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper);
    ECOBJECTS_EXPORT ECObjectsStatus SetPersistenceUnit(ECUnitCR unit); //!< Sets the provided ECUnit as the persistence unit.
    ECUnitCP GetPersistenceUnit() const {return m_persistenceUnit;} //!< Gets the Unit of measurement used for persisting the information

    //! Constructs a NamedFormat and sets it as the default (First in the presentation format list).
    ECObjectsStatus SetDefaultPresentationFormat(ECFormatCR parent, Nullable<int32_t> precisionOverride = nullptr, ECUnitCP inputUnitOverride = nullptr, Utf8CP labelOverride = nullptr)
        {return AddPresentationFormatSingleUnitOverride(parent, precisionOverride, inputUnitOverride, labelOverride, true);}
    //! Gets the default presentation format of this KindOfQuantity.
    NamedFormatCP GetDefaultPresentationFormat() const {return HasPresentationFormats() ? &m_presentationFormats[0] : GetCachedPersistenceFormat();}
    bvector<NamedFormat> const& GetPresentationFormats() const {return m_presentationFormats;} //!< Gets a list of all presentation formats available for this KoQ.
    bool HasPresentationFormats() const {return m_presentationFormats.size() > 0;} //!< Returns true if one or more presentation formats exist

    //! Adds a NamedFormat to this KoQ's list of presentation formats. If the format has any units,
    //! they must be compatible with the persistence unit and each other.
    //! @param[in]  parent              The format to base this override off of
    //! @param[in]  precisionOverride   Optionally specify an override for precision
    //! @param[in]  unitsAndLabels      Optionally override units and labels for the composite
    //! @param[in]  isDefault           Optionally set the created override as the default presentation unit
    //! @return ECObjectsStatus::Succcess if format is successfully added as a presentation format. Otherwise, ECObjectsStatus::Error.
    ECOBJECTS_EXPORT ECObjectsStatus AddPresentationFormat(ECFormatCR parent, Nullable<int32_t> precisionOverride = nullptr, UnitAndLabelPairs const* unitsAndLabels = nullptr, bool isDefault = false);

    //! Adds NamedFormat to this KoQ's list of presentation formats. If the format has any units,
    //! they must be compatible with the persistence unit and each other. This is a convenience method to handle
    //! the common case where only the major unit needs to be overridden
    //! @param[in]  parent              The format to base this override off of
    //! @param[in]  precisionOverride   Optionally specify an override for precision
    //! @param[in]  majorUnitOverride   Optionally override major unit of the composite
    //! @param[in]  labelOverride       Optionally override the major unit label of the composite
    //! @param[in]  isDefault           Optionally set the created override as the default presentation unit
    //! @return ECObjectsStatus::Succcess if format is successfully added as a presentation format. Otherwise, ECObjectsStatus::Error.
    ECOBJECTS_EXPORT ECObjectsStatus AddPresentationFormatSingleUnitOverride(ECFormatCR parent, Nullable<int32_t> precisionOverride = nullptr, ECUnitCP majorUnitOverride = nullptr, Utf8CP labelOverride = nullptr, bool isDefault = false);

    //! Given a string representating a format string and a mapping function, adds the unit as the format as a presentation format if it is found
    //! by the mapping function
    //!
    //! @param[in] formatString         The format string to add
    //! @param[in] nameToFormatMapper   Function that maps format names to ECFormatCP
    //! @param[in] nameToUnitMapper     Function that maps unit names to ECUnitCP
    //! @return ECObjectsStatus::Success if successfully updates the presentation format. Otherwise error codes
    ECOBJECTS_EXPORT ECObjectsStatus AddPresentationFormatByString(Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper);

    //!< Removes the specified presentation format.
    void RemovePresentationFormat(NamedFormatCR presentationFormat)
        {
        m_presentationFormats.erase(std::remove_if(m_presentationFormats.begin(), m_presentationFormats.end(), [&](NamedFormatCR format) {return format.IsIdentical(presentationFormat);}));
        m_descriptorCacheValid = false;
        }

    void RemoveAllPresentationFormats() {m_presentationFormats.clear(); m_descriptorCacheValid = false;} //!< Removes all presentation formats.

    //! Returns the Phenomenon supported by this KindOfQuantity.
    //!
    //! @remarks All Units within this KindOfQuantity must have the same Phenomenon.
    PhenomenonCP GetPhenomenon() const {return (nullptr == GetPersistenceUnit()) ? nullptr : static_cast<PhenomenonCP>(GetPersistenceUnit()->GetPhenomenon());}

    //! Write the KindOfQuantity as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = true) const { return ToJson(outValue, true, includeSchemaVersion); };
    ECOBJECTS_EXPORT Json::Value GetPresentationFormatsJson() const; //!< Return Json array of allowable presentation formats.
    //! Given an old EC3.1 persistence FUS descriptor as well as the semi-colon separated string of
    //! presentation FUS descriptors in the format: {unitName}({formatName}), it will extract and convert
    //! the persistence unit to a new unit name. If the persistence FUS has a format, it will be added to the end of the
    //! formatString that is returned. This method also caches the FUS descriptors within the koq
    //! Note: This method completely overwrites the Units and Formats of the KindOfQuantity if successful.
    //!
    //! @param[out] persFUS         The descriptor for the persistence FUS that is of the format for the old FUS descriptor,
    //!                             format: {unitName}({formatName}), where the format part is optional.
    //! @param[out] presFuses       List of presentation FUSes
    //! @param[in] formatSchema     Reference to standard formats schema use to locate formats and verify if they have a composite or not
    //! @param[in] unitsSchema      Reference to standard units schema use to locate units for validation purposes
    //! @return ECObjectsStatus::Success if successfully updates the descriptor
    ECOBJECTS_EXPORT ECObjectsStatus FromFUSDescriptors(Utf8CP persFUS, const bvector<Utf8CP>& presFuses, ECSchemaCR formatSchema, ECSchemaCR unitsSchema);
    //! Given an old EC3.1 persistence FUS descriptor as well as the semi-colon separated string of
    //! presentation FUS descriptors in the format: {unitName}({formatName}), it will extract and convert
    //! the persistence unit to a new unit name. If the persistence FUS has a format, it will be added to the end of the
    //! formatString that is returned
    //!
    //! @param[out] persUnitName    The qualified name of the persistence unit mapped to a unit name in the standard units schema
    //! @param[out] presFormatStrings List of presentation format strings representing format overrides.
    //! @param[in] persFus          The descriptor for the persistence FUS that is of the format for the old FUS descriptor,
    //!                             format: {unitName}({formatName}), where the format part is optional.
    //! @param[in] presFuses        List of presentation FUS descriptors
    //! @param[in] formatSchema     Reference to standard formats schema use to locate formats and verify if they have a composite or not
    //! @param[in] unitsSchema      Reference to standard units schema use to locate units for validation purposes
    //! @return ECObjectsStatus::Success if successfully updates the descriptor; otherwise ECObjectsStatus::InvalidUnitName
    //! if the unit name is not found or ECObjectStatus::NullPointerValue if a nullptr is passed in for the descriptor.
    ECOBJECTS_EXPORT static ECObjectsStatus UpdateFUSDescriptors(Utf8StringR persUnitName, bvector<Utf8String>& presFormatStrings, Utf8CP persFus, bvector<Utf8CP> const& presFuses, ECSchemaCR formatSchema, ECSchemaCR unitsSchema);
};

//=======================================================================================
//! The in-memory representation of a PropertyCategory as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct PropertyCategory : NonCopyableClass
{
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    uint32_t m_priority;
    Utf8String m_description;
    ECValidatedName m_validatedName;
    ECSchemaCR m_schema;

    mutable PropertyCategoryId m_propertyCategoryId;
    mutable Utf8String m_fullName;

    PropertyCategory(ECSchemaCR schema) : m_schema(schema), m_priority(0) {};
    ~PropertyCategory() {};

    SchemaReadStatus ReadXml(BeXmlNodeR propertyCategoryNode, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECObjectsStatus SetName(Utf8CP name);

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this PropertyCategory is defined in

    void SetId(PropertyCategoryId id) { BeAssert(!m_propertyCategoryId.IsValid()); m_propertyCategoryId = id; } //!< Intended to be called by ECDb or a similar system
    PropertyCategoryId GetId() const {BeAssert(HasId()); return m_propertyCategoryId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    bool HasId() const {return m_propertyCategoryId.IsValid();}

    Utf8StringCR GetName() const {return m_validatedName.GetName();} //!< The name of this PropertyCategory
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this PropertyCategory in the format, {SchemaName}:{PropertyCategoryName}.
    //! Gets a qualified name of the PropertyCategory, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    //! Sets the description of this PropertyCategory
    //! @param[in]  description  The new value to set as the description
    ECObjectsStatus SetDescription(Utf8CP description) {m_description = description; return ECObjectsStatus::Success;}
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this PropertyCategory
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description of this PropertyCategory.

    //! Sets the display label for this PropertyCategory
    //! @param[in]  displayLabel  The new value to set as the display label
    ECOBJECTS_EXPORT ECObjectsStatus SetDisplayLabel(Utf8CP displayLabel);
    //! Gets the display label for this PropertyCategory.  If a display label has not been explicitly set, returns the name of the PropertyCategory.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    Utf8StringCR GetInvariantDisplayLabel() const {return m_validatedName.GetDisplayLabel();} //!< Gets the invariant display label for this PropertyCategory.
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();} //!< Whether the display label is explicitly defined or not

    //! Sets the priority for this PropertyCategory
    //! @param[in]  priority  The priority to set
    ECObjectsStatus SetPriority(uint32_t priority) {m_priority = priority; return ECObjectsStatus::Success;}
    uint32_t GetPriority() const {return m_priority;} //!< Gets the priority for this PropertyCategory

    //! Write the PropertyCategory as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false) const {return ToJson(outValue, true, includeSchemaVersion);}
};

typedef bvector<ECClassP> ECBaseClassesList;
typedef bvector<ECClassP> ECDerivedClassesList;
typedef bvector<ECClassCP> ECRelationshipConstraintClassList;
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
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaXmlReader2;
friend struct SchemaXmlReader3;
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method
friend struct ECPropertyIterable::IteratorState;
friend struct SupplementedSchemaBuilder;
friend struct ECProperty; // for access to InvalidateDefaultStandaloneEnabler() when property is modified
friend struct ECSchemaConverter;

private:
    const ECClassType               m_classType;
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
    mutable bool                    m_propertyCountCached;
    mutable uint16_t                m_propertyCount;
    mutable StandaloneECEnablerPtr  m_defaultStandaloneEnabler;
    bvector<Utf8String> m_xmlComments;
    bmap<Utf8String, bvector<Utf8String>> m_contentXmlComments;
    bmap<int, bvector<Utf8String>> m_customAttributeXmlComments;

    ECObjectsStatus AddProperty (ECPropertyP& pProperty, bool resolveConflicts = false);
    ECObjectsStatus RemoveProperty (ECPropertyR pProperty);
    void FindUniquePropertyName(Utf8StringR newName, Utf8CP prefix, Utf8CP originalName);
    ECObjectsStatus RenameConflictProperty(ECPropertyP thisProperty, bool renameDerivedProperties, ECPropertyP& renamedProperty, Utf8String newName);
    void RenameDerivedProperties(Utf8String newName);

    // Adds the ECv3ConversionAttributes:RenamedPropertiesMapping Custom Attribute with the original name provided.
    // Used for instance transformation
    void AddPropertyMapping(Utf8CP originalName, Utf8CP newName);

    static bool     SchemaAllowsOverridingArrays(ECSchemaCP schema);

    static bool     CheckBaseClassCycles(ECClassCP currentBaseClass, const void * arg);
    static bool     AddUniquePropertiesToList(ECClassCP curentBaseClass, const void * arg);
    bool            TraverseBaseClasses(TraversalDelegate traverseMethod, bool recursive, const void * arg) const;

    void            AddDerivedClass(ECClassCR derivedClass) const {m_derivedClasses.push_back(const_cast<ECClassP>(&derivedClass));}
    void            RemoveDerivedClass(ECClassCR derivedClass) const;
    void            RemoveDerivedClasses();
    static void     SetErrorHandling(bool doAssert);

    static bool     ConvertPropertyToPrimitiveArray(ECClassP thisClass, ECClassCP startingClass, Utf8String propName, bool includeDerivedClasses = false);
    ECObjectsStatus FixArrayPropertyOverrides();
    ECObjectsStatus CanPropertyBeOverridden(ECPropertyCR baseProperty, ECPropertyCR newProperty, Utf8StringR errMsg) const;
    ECObjectsStatus CopyPropertyForSupplementation(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes);
    ECObjectsStatus CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, Utf8CP destPropertyName, bool copyCustomAttributes, bool andAddProperty = true, bool copyReferences = false);

    void            OnBaseClassPropertyRemoved(ECPropertyCR baseProperty);
    void            OnBaseClassPropertyChanged(ECPropertyCR baseProperty, ECPropertyCP newBaseProperty);
    ECObjectsStatus OnBaseClassPropertyAdded(ECPropertyCR baseProperty, bool resolveConflicts);

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECClass (ECClassType classType, ECSchemaCR schema);
    virtual ~ECClass();

    ECObjectsStatus AddProperty(ECPropertyP pProperty, Utf8StringCR name);
    virtual ECObjectsStatus _AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false, bool validate = true);
    virtual ECObjectsStatus _RemoveBaseClass(ECClassCR baseClass);

    void _GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const override;

    ECSchemaCP _GetContainerSchema() const override {return &m_schema;}
    Utf8CP _GetContainerName() const override {return GetFullName();}

    virtual ECObjectsStatus GetProperties(bool includeBaseProperties, PropertyList* propertyList) const;
    // schemas index class by name so publicly name can not be reset
    ECObjectsStatus SetName (Utf8StringCR name);

    virtual SchemaReadStatus _ReadXmlAttributes (BeXmlNodeR classNode);

    //! Uses the specified xml node (which must conform to an ECClass as defined in ECSchemaXML) to populate the base classes and properties of this class.
    //! Before this method is invoked the schema containing the class must have loaded all schema references and stubs for all classes within
    //! the schema itself otherwise the method may fail because such dependencies can not be located.
    //! @param[in]  classNode       The XML DOM node to read
    //! @param[in]  context         The read context that contains information about schemas used for deserialization
    //! @param[in]  conversionSchema  If there was a supplied schema to assist in converting from V2 to V3
    //! @param[in]  droppedAliases  A list of aliases from referenced schemas that are no longer being referenced
    //! @param[out] navigationProperties A running list of all navigation properties in the schema.  This list is used for validation, which may only happen after all classes are loaded
    //! @return   Status code
    virtual SchemaReadStatus _ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases, bvector<NavigationECPropertyP>& navigationProperties);

    void _ReadCommentsInSameLine(BeXmlNodeR childNode, bvector<Utf8String>& comments);

    SchemaReadStatus _ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases);
    SchemaReadStatus _ReadPropertyFromXmlAndAddToClass(ECPropertyP ecProperty, BeXmlNodeP& childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, Utf8CP childNodeName);

    virtual SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion, Utf8CP elementName, bmap<Utf8CP, Utf8CP>* additionalAttributes, bool doElementEnd) const;

    virtual bool _ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const;
    bool _ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties, bvector<bpair<Utf8String, Json::Value>> attributes) const;

    virtual bool _Validate() const = 0;

    void InvalidateDefaultStandaloneEnabler() const;
public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this class is defined in
    ECSchemaR GetSchemaR() {return const_cast<ECSchemaR>(m_schema);}

    void SetId(ECClassId id) {BeAssert(!m_ecClassId.IsValid()); m_ecClassId = id;} //!< Intended to be called by ECDb or a similar system.
    ECClassId GetId() const {BeAssert(HasId()); return m_ecClassId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    bool HasId() const {return m_ecClassId.IsValid();}

    Utf8StringCR GetName() const {return m_validatedName.GetName();} //!< The name of this ECClass
    ECOBJECTS_EXPORT Utf8CP GetFullName() const; //!< {SchemaName}:{ClassName} The pointer will remain valid as long as the ECClass exists.
    //! Formats the class name for use in an ECSQL statement: [{SchemaName}].[{ClassName}]
    //! @remarks The pointer will remain valid as long as the ECClass exists.
    ECOBJECTS_EXPORT Utf8StringCR GetECSqlName() const;

    ECOBJECTS_EXPORT ECObjectsStatus SetDescription(Utf8StringCR description) {m_description = description; return ECObjectsStatus::Success;} //!< Sets the description of this ECClass
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECClass.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this ECClass.  Returns the localized description if one exists.

    ECOBJECTS_EXPORT ECObjectsStatus SetDisplayLabel(Utf8StringCR displayLabel); //!< Sets the display label of this ECClass
    //! Gets the display label of this ECClass.  If no display label has been set explicitly, it will return the name of the ECClass
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    Utf8StringCR GetInvariantDisplayLabel() const {return m_validatedName.GetDisplayLabel();} //!< Gets the invariant display label for this ECClass.
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();} //!< Whether the display label is explicitly defined or not.

    //! Returns the StandaloneECEnabler for this class
    ECOBJECTS_EXPORT StandaloneECEnablerP GetDefaultStandaloneEnabler() const;

    ECClassType GetClassType() const {return m_classType;} //!< The type of derived ECClass this is

    //! Is the class a Mixin.
    //! Returns true if this class is an Entity class and has the CoreCustomAttributes:IsMixin custom attribute applied.
    bool IsMixin() const {return IsEntityClass() && IsDefinedLocal("CoreCustomAttributes", "IsMixin");}
    bool IsEntityClass() const {return ECClassType::Entity == m_classType;} //!< Is the class an entity class
    bool IsStructClass() const {return ECClassType::Struct == m_classType;} //!< Is the class a struct class
    bool IsCustomAttributeClass() const {return ECClassType::CustomAttribute == m_classType;} //!< Is the class a custom attribute class
    bool IsRelationshipClass() const {return ECClassType::Relationship == m_classType;} //!< Is the class a relationship class

    //! return ECRelationshipClass const pointer if IsRelationshipClass() return true else nullptr
    ECRelationshipClassCP GetRelationshipClassCP() const {return IsRelationshipClass() ? reinterpret_cast<ECRelationshipClassCP>(this) : nullptr;}
    //! return ECRelationshipClass pointer if IsRelationshipClass() return true else nullptr
    ECRelationshipClassP GetRelationshipClassP() {return IsRelationshipClass() ? reinterpret_cast<ECRelationshipClassP>(this) : nullptr;}

    //! return ECEntityClass const pointer if IsEntityClass() return true else nullptr
    ECEntityClassCP GetEntityClassCP() const {return IsEntityClass() ? reinterpret_cast<ECEntityClassCP>(this) : nullptr;}
    //! return ECEntityClass pointer if IsEntityClass() return true else nullptr
    ECEntityClassP GetEntityClassP() {return IsEntityClass() ? reinterpret_cast<ECEntityClassP>(this) : nullptr;}

    //! return ECCustomAttributeClass const pointer if IsCustomAttributeClass() return true else nullptr
    ECCustomAttributeClassCP GetCustomAttributeClassCP() const {return IsCustomAttributeClass() ? reinterpret_cast<ECCustomAttributeClassCP>(this) : nullptr;}
    //! return ECCustomAttributeClass pointer if IsCustomAttributeClass() return true else nullptr
    ECCustomAttributeClassP GetCustomAttributeClassP() {return IsCustomAttributeClass() ? reinterpret_cast<ECCustomAttributeClassP>(this) : nullptr;}

    //! return ECStructClass const pointer if IsStructClass() return true else nullptr
    ECStructClassCP GetStructClassCP() const {return IsStructClass() ? reinterpret_cast<ECStructClassCP>(this) : nullptr;}
    //! return ECStructClass pointer if IsStructClass() return true else nullptr
    ECStructClassP GetStructClassP() {return IsStructClass() ? reinterpret_cast<ECStructClassP>(this) : nullptr;}

    ECClassModifier GetClassModifier() const {return m_modifier;} //!< Returns the class modifier
    void SetClassModifier(ECClassModifier modifier) {m_modifier = modifier;} //!< Sets the class modifier

    //! Adds a base class
    //! You cannot add a base class if it creates a cycle. For example, if A is a base class
    //! of B, and B is a base class of C, you cannot make C a base class of A. Attempting to do
    //! so will return an error. You also can't add a base class to final classes
    //! Note: baseClass must be of same derived class type
    //! @param[in] baseClass The class to derive from
    ECObjectsStatus AddBaseClass(ECClassCR baseClass) { return AddBaseClass(baseClass, false); }

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
    //!                             false if @p baseClass is added to the end of the list
    //! @param[in] validate if true, will validate the class hierarchy
    ECObjectsStatus AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false, bool validate = true) {return _AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, validate);}

    bool HasBaseClasses() const { return m_baseClasses.size() > 0; } //!< Returns whether there are any base classes for this class
    const ECBaseClassesList& GetBaseClasses() const {return m_baseClasses;} //!< Returns a list of the classes this ECClass is derived from
    ECObjectsStatus RemoveBaseClass(ECClassCR baseClass) {return _RemoveBaseClass(baseClass);} //!< Removes the provided base class.
    ECOBJECTS_EXPORT void RemoveBaseClasses(); //!< Removes all base classes

    bool HasDerivedClasses() const { return m_derivedClasses.size() > 0; } //!< Returns whether there are any derived classes for this class
    const ECDerivedClassesList& GetDerivedClasses() const {return m_derivedClasses;} //!< Returns a list of the classes that derive from this class.

    //! Returns true if the class is the type specified or derived from it.
    ECOBJECTS_EXPORT bool Is(ECClassCP targetClass) const;

    //! Returns true if this class matches the specified schema and class name, or is derived from a matching class
    ECOBJECTS_EXPORT bool Is(Utf8CP schemaName, Utf8CP className) const;

    //! Tries to locate a common base class given the starting class and a list of other classes
    ECOBJECTS_EXPORT static void FindCommonBaseClass(ECEntityClassCP &commonClass, ECEntityClassCP startingClass, bvector<ECClassCP> const& constraintClasses);

    //! If the given name is valid, creates a primitive property object with the default type of STRING
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name);

    //! If the given name is valid, creates a primitive property object with the given primitive type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates a struct property object using the specified class as the struct type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, Utf8StringCR name, ECStructClassCR structType);

    //! If the given name is valid, creates an array property object using the default type of STRING
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveArrayProperty(PrimitiveArrayECPropertyP& ecProperty, Utf8StringCR name);

    //! If the given name is valid, creates an array property object using the specified primitive type as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveArrayProperty(PrimitiveArrayECPropertyP& ecProperty, Utf8StringCR name, PrimitiveType primitiveType);

    //! If the given name is valid, creates an array property object using the specified ECEnumeration as the array type
    ECOBJECTS_EXPORT ECObjectsStatus CreatePrimitiveArrayProperty(PrimitiveArrayECPropertyP& ecProperty, Utf8StringCR name, ECEnumerationCR enumerationType);

    //! If the given name is valid, creates a struct array property object using the specified class as the struct array type
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructArrayProperty(StructArrayECPropertyP& ecProperty, Utf8StringCR name, ECStructClassCR structType);

    //! If the given name is valid, creates a primitive property object with the given enumeration type
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumerationProperty(PrimitiveECPropertyP& ecProperty, Utf8StringCR name, ECEnumerationCR enumerationType);

    ECOBJECTS_EXPORT size_t GetPropertyCount(bool includeBaseProperties = true) const; //!< Returns the number of ECProperties in this class
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties() const; //!< Returns an iterable of all the ECProperties defined on this class, including inherited properties.

    //! Returns a list of properties for this class.
    //! @param[in]  includeBaseProperties If true, then will return properties that are contained in this class's base class(es)
    //! @return     An iterable container of ECProperties
    ECOBJECTS_EXPORT ECPropertyIterable GetProperties(bool includeBaseProperties) const;

    //! Renames a property (and potentially all derived properties) if its name conflicts.  A new name is automatically generated.
    //! Note: This does not do any checks to determine if the give property's name does actually conflict.  It will always rename the property.
    //! @param[in]  conflictProperty    The property whose name conflicts with either a base class property or a reserved system property name
    //! @param[in]  renameDerivedProperties Whether to also rename derived properties
    //! @param[out] renamedProperty         The renamed property
    ECOBJECTS_EXPORT ECObjectsStatus RenameConflictProperty(ECPropertyP conflictProperty, bool renameDerivedProperties, ECPropertyP& renamedProperty);

    //! Remove the specified property property
    //! @param[in] name The name of the property to be removed
    ECOBJECTS_EXPORT ECObjectsStatus RemoveProperty(Utf8StringCR name);

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, NULL
    ECOBJECTS_EXPORT ECPropertyP GetPropertyP(WCharCP name, bool includeBaseClasses=true) const;

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, nullptr.
    ECPropertyP GetPropertyP(Utf8StringCR name, bool includeBaseClasses = true) const {return GetPropertyP(name.c_str(), includeBaseClasses);}

    //! Get a property by name within the context of this class and its base classes.
    //! The pointer returned by this method is valid until the ECClass containing the property is destroyed or the property
    //! is removed from the class.
    //! @param[in]  name     The name of the property to lookup.
    //! @param[in]  includeBaseClasses  Whether to look on base classes of the current class for the named property
    //! @return   A pointer to an ECN::ECProperty if the named property exists within the current class; otherwise, nullptr.
    ECOBJECTS_EXPORT ECPropertyP GetPropertyP(Utf8CP name, bool includeBaseClasses=true) const;

    //! Gets a property by name from the base classes of this class.
    ECOBJECTS_EXPORT ECPropertyP GetBaseClassPropertyP(Utf8CP name) const;

    ECOBJECTS_EXPORT ECPropertyP GetPropertyByIndex(uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus RenameProperty(ECPropertyR ecProperty, Utf8CP newName);
    ECOBJECTS_EXPORT ECObjectsStatus ReplaceProperty(ECPropertyP& newProperty, ValueKind valueKind, ECPropertyR propertyToRemove);
    ECOBJECTS_EXPORT ECObjectsStatus DeleteProperty(ECPropertyR ecProperty);
    
    //! Get the property that stores the instance label for the class.
    //! @return A pointer to ECN::ECProperty if the instance label has been specified; otherwise, nullptr.
    ECOBJECTS_EXPORT ECPropertyP GetInstanceLabelProperty() const;

    //! Copies the sourceProperty, adds it to the current class and outputs the copied property if the copy was successful
    //! @param[out]  destProperty           Outputs the copied property.  Only valid if the method returns ::Success
    //! @param[in]   sourceProperty         The property to copy into this class.  If nullptr, ::NullPointerValue returned.
    //! @param[in]   copyCustomAttributes   If true the primary custom attributes are copied to the destProperty, supplemental custom attributes are not copied.  A schema references will be added as needed.
    ECOBJECTS_EXPORT ECObjectsStatus CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes);

    //! Copies the sourceProperty, adds it to the current class and outputs the copied property if the copy was successful
    //! @param[out]  destProperty           Outputs the copied property.  Only valid if the method returns ::Success
    //! @param[in]   sourceProperty         The property to copy into this class.  If nullptr, ::NullPointerValue returned.
    //! @param[in]   destPropertyName       Rename the copied property to the given name
    //! @param[in]   copyCustomAttributes   If true the primary custom attributes are copied to the destProperty, supplemental custom attributes are not copied.  A schema references will be added as needed.
    ECOBJECTS_EXPORT ECObjectsStatus CopyProperty(ECPropertyP& destProperty, Utf8CP destPropertyName, ECPropertyCP sourceProperty, bool copyCustomAttributes);

    //! Returns true if this class derives from the input baseClass once and only once.  Returns false if this class does not derive from the input base class or if it is found more than once when traversing base classes
    ECOBJECTS_EXPORT bool IsSingularlyDerivedFrom(ECClassCR baseClass) const;

    ECOBJECTS_EXPORT bool Validate() const;

    //! @param[out] outValue                    Json object containing the schema item Json if successfully written.
    //! @param[in]  includeSchemaVersion        If true the schema version will be included in the Json object.
    //! @param[in]  includeInheritedProperties  If true inherited properties will be serialized along with noninherited properties.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false, bool includeInheritedProperties = false) {return _ToJson(outValue, true, includeSchemaVersion, includeInheritedProperties);}

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //! Given a qualified class name, will parse out the schema's alias and the class name. 
    //!
    //! If anything other than a qualified class name is provided it will it will be split on the first `:` found.
    //! @param[out] alias       The alias of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedClassName  The qualified name of the class, in the format of ns:className
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseClassName(Utf8StringR alias, Utf8StringR className, Utf8StringCR qualifiedClassName);

    //! Given a schema and a class, will return the fully qualified class name.  If the class is part of the passed in schema, there
    //! is no alias.  Otherwise, the class's schema must be a referenced schema in the passed in schema
    //! @param[in]  primarySchema   The schema used to lookup the alias of the class's schema
    //! @param[in]  ecClass         The class whose schema should be searched for
    //! @return WString    The alias if the class's schema is not the primarySchema
    ECOBJECTS_EXPORT static Utf8String GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass);

    //! Given two ECClass's, checks to see if they are equal by name
    //! @param[in]  currentBaseClass    The source class to check against
    //! @param[in]  arg                 The target to compare to (this parameter must be an ECClassP)
    ECOBJECTS_EXPORT static bool ClassesAreEqualByName(ECClassCP currentBaseClass, const void * arg);
}; // ECClass

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

private:
    bool _Validate() const override;
    bool VerifyMixinHierarchy(bool thisIsMixin, ECEntityClassCP baseAsEntity) const;

protected:
    //  Lifecycle management:  For now, to keep it simple, the class constructor is protected.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECEntityClass(ECSchemaCR schema) : ECClass(ECClassType::Entity, schema) {}
    virtual ~ECEntityClass() {}

    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const override;
    bool _ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::EntityClass;}
    ECObjectsStatus _AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false, bool validate = true) override;

public:
    //! Creates a navigation property object using the relationship class and direction.  To succeed the relationship class, direction and name must all be valid.
    // @param[out]  ecProperty          Outputs the property if successfully created
    // @param[in]   name                The name for the navigation property.  Must be a valid ECName
    // @param[in]   relationshipClass   The relationship class this navigation property will traverse.  Must list this class as an endpoint constraint.  The multiplicity of the other constraint determiness if the nav prop is a primitive or an array.
    // @param[in]   direction           The direction the relationship will be traversed.  Forward indicates that this class is a source constraint, Backward indicates that this class is a target constraint.
    // @param[in]   verify              If true the relationshipClass an direction will be verified to ensure the navigation property fits within the relationship constraints.  Default is true.  If not verified at creation the Verify method must be called before the navigation property is used or it's type descriptor will not be valid.
    ECOBJECTS_EXPORT ECObjectsStatus CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, bool verify = true);

    //! Returns true if the provided mixin class can be applied to this class. 
    //! @remarks The mixin class can be applied to this class if this class is derived from the AppliesToEntityClass property defined in IsMixin custom attribute.
    //! @param[in] mixinClass The ECEntityClass to check if it can be add as a mixin to this class.
    ECOBJECTS_EXPORT bool CanApply(ECEntityClassCR mixinClass) const;

    //! If the class is a mixin, returns the ECEntityClass which restricts where the mixin can be applied, else nullptr. 
    ECOBJECTS_EXPORT ECEntityClassCP GetAppliesToClass() const;
    
    //! Returns true if the class is the type specified, derived from it or is a mixin which applies to the type specified
    ECOBJECTS_EXPORT bool IsOrAppliesTo(ECEntityClassCP entityClass) const;
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
    ECCustomAttributeClass(ECSchemaCR schema) : ECClass(ECClassType::CustomAttribute, schema), m_containerType(CustomAttributeContainerType::Any) {}
    virtual ~ECCustomAttributeClass () {}

    bool _Validate() const override {return true;}

protected:
    SchemaReadStatus _ReadXmlAttributes(BeXmlNodeR classNode) override;
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const override;
    bool _ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::CustomAttributeClass;}

public:
    CustomAttributeContainerType GetContainerType() const {return m_containerType;}

    //! Sets the container type which this custom attribute can be applied to. Use this carefully as it might render existing instances invalid!
    // @param[in]   containerType   The new container type to apply
    void SetContainerType(CustomAttributeContainerType containerType) {m_containerType = containerType;}

    //! Returns true if the containerType is compatible with the CustomAttributeContainerType of this ECCustomAttributeClass
    //@param[in]    containerType   The type of the container you wish to apply an instance of this class to
    bool CanBeAppliedTo(CustomAttributeContainerType containerType) const {return 0 != static_cast<int>(m_containerType & containerType);} // Compare to 0 instead of containerType so comparisons like Class & Any return true
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
    ECStructClass(ECSchemaCR schema) : ECClass(ECClassType::Struct, schema) {}
    virtual ~ECStructClass () {}

    bool _Validate() const override {return true;}

protected:
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const override;
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::StructClass;}
};

//=======================================================================================
//! This class describes the multiplicity of a relationship. It is based on the
//!     Martin notation. Valid multiplicities are (x..y) where x is smaller or equal to y,
//!     x >= 0 and y >= 1 or y = * (where * represents infinity).
//!     For example, (0..1), (1..1), (1..*), (0..*), (1..10), (2..5), ...
//! @bsiclass
//=======================================================================================
struct RelationshipMultiplicity
{
private:
    uint32_t   m_lowerLimit;
    uint32_t   m_upperLimit;

public:
    //! Default constructor.  Creates a multiplicity of (0..1)
    RelationshipMultiplicity() : m_lowerLimit(0), m_upperLimit(1) {}

    //! Constructor with lower and upper limit parameters.
    //! @param[in]  lowerLimit  must be less than or equal to upperLimit and greater than or equal to 0
    //! @param[in]  upperLimit  must be greater than or equal to lowerLimit and greater than 0
    RelationshipMultiplicity(uint32_t lowerLimit, uint32_t upperLimit) : m_lowerLimit(lowerLimit), m_upperLimit(upperLimit)
        { BeAssert(lowerLimit <= upperLimit); BeAssert(upperLimit > 0); BeAssert(upperLimit <= INT_MAX); }

    //! Constructor where upper limit is set to Unbounded
    //! @param[in]  lowerLimit  must be less than or equal to upperLimit and greater than or equal to 0
    RelationshipMultiplicity(uint32_t lowerLimit) : RelationshipMultiplicity(lowerLimit, INT_MAX) {}

    //! Returns the lower limit of the multiplicity
    uint32_t GetLowerLimit() const {return m_lowerLimit;}
    //! Returns the upper limit of the multiplicity
    uint32_t GetUpperLimit() const {return m_upperLimit;}

    //! Indicates if the multiplicity is unbound (ie, upper limit is equal to "*")
    bool IsUpperLimitUnbounded() const {return m_upperLimit == INT_MAX;}

    //! Converts the multiplicity to a string, for example "(0..*)", "(1..1)"
    ECOBJECTS_EXPORT Utf8String ToString() const;

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************

    //!     Returns the shared static RelationshipMultiplicity object that represents the
    //!     (0..1) multiplicity. This static property can be used instead of a standard
    //!     constructor of RelationshipMultiplicity to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipMultiplicityCR ZeroOne();
    //!     Returns the shared static RelationshipMultiplicity object that represents the
    //!     (0..*) multiplicity. This static property can be used instead of a standard
    //!     constructor of RelationshipMultiplicity to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipMultiplicityCR ZeroMany();
    //!     Returns the shared static RelationshipMultiplicity object that represents the
    //!     (1..1) multiplicity. This static property can be used instead of a standard
    //!     constructor of RelationshipMultiplicity to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipMultiplicityCR OneOne();
    //!     Returns the shared static RelationshipMultiplicity object that represents the
    //!     (1..*) multiplicity. This static property can be used instead of a standard
    //!     constructor of RelationshipMultiplicity to reduce memory usage.
    ECOBJECTS_EXPORT static RelationshipMultiplicityCR OneMany();

    //! Compares the two Multiplicities and returns whether they are equal (0). Otherwise the
    //! larger scope will be returned either rhs (1) or lhs (-1)
    ECOBJECTS_EXPORT static int Compare(RelationshipMultiplicity const& lhs, RelationshipMultiplicity const& rhs);
};

//=======================================================================================
//! The in-memory representation of the source and target constraints for an ECRelationshipClass as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECRelationshipConstraint : IECCustomAttributeContainer
{
friend struct ECRelationshipClass;

private:
    bool m_isSource;
    bool m_isPolymorphic;
    bool m_verify;
    bool m_verified;
    Utf8String                  m_roleLabel;
    ECClassCP                   m_abstractConstraint;
    ECRelationshipClassP        m_relClass;
    RelationshipMultiplicity*   m_multiplicity;
    ECRelationshipConstraintClassList    m_constraintClasses;

    ECObjectsStatus             SetMultiplicity(uint32_t& lowerLimit, uint32_t& upperLimit);
    ECObjectsStatus             SetMultiplicityFromLegacyString(Utf8CP multiplicity, bool validate);
    ECObjectsStatus             SetMultiplicity(Utf8CP multiplicity, bool validate);


    ECObjectsStatus             AddClass(ECClassCR classConstraint);
    ECObjectsStatus             RemoveClass(ECClassCR classConstraint);
    ECObjectsStatus             SetAbstractConstraint(ECClassCR abstractConstraint);
    ECObjectsStatus             SetAbstractConstraint(Utf8CP value, bool validate);

    SchemaWriteStatus           WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, ECVersion ecXmlVersion) const;
    SchemaReadStatus            ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext);

    bool           ToJson(Json::Value& outValue);

    bool                        IsValid(bool resolveIssues);
    ECObjectsStatus             ValidateBaseConstraint(ECRelationshipConstraintCR baseConstraint) const;

    ECObjectsStatus ValidateAbstractConstraint(ECClassCP abstractConstraint, bool resolveIssues = false);
    ECObjectsStatus ValidateAbstractConstraint(bool resolveIssues = false) { return ValidateAbstractConstraint(GetAbstractConstraint(), resolveIssues); }
    ECObjectsStatus ValidateRoleLabel(bool resolveIssues = false);
    ECObjectsStatus ValidateClassConstraint() const;
    ECObjectsStatus ValidateClassConstraint(ECClassCR constraintClass) const;
    ECObjectsStatus ValidateMultiplicityConstraint(bool resolveIssues = false) const;
    ECObjectsStatus ValidateMultiplicityConstraint(uint32_t& lowerLimit, uint32_t& upperLimit, bool resolveIssues = false) const;

    ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isSource, bool verify);

protected:
    ECSchemaCP _GetContainerSchema() const override;
    Utf8CP _GetContainerName() const override;
    CustomAttributeContainerType _GetContainerType() const override {return m_isSource ? CustomAttributeContainerType::SourceRelationshipConstraint : CustomAttributeContainerType::TargetRelationshipConstraint;}

public:
    ECOBJECTS_EXPORT virtual ~ECRelationshipConstraint(); //!< Destructor

    //! Return relationshipClass to which this constraint is associated
    ECRelationshipClassCR GetRelationshipClass() const {BeAssert(m_relClass != nullptr); return *m_relClass;}

    //! Set the label of the constraint role in the relationship.
    //! @param[in] roleLabel The role label to be set on this relationship constraint
    //! @returns An ::Error is returned if the given label is null or empty
    ECOBJECTS_EXPORT ECObjectsStatus SetRoleLabel(Utf8CP roleLabel);
    
    //! Get the label of the constraint role in the relationship.
    //! @remarks If the role label is not defined on this constraint it will be inherited from its base constraint, if one exists.
    ECOBJECTS_EXPORT Utf8String const GetRoleLabel() const;
    
    //! Get the invariant label of the constraint role in the relationship.
    //! @remarks If the role label is not defined on this constraint it will be inherited from its base constraint, if one exists.
    Utf8String const GetInvariantRoleLabel() const {return m_roleLabel;}

    //! Determine whether the label of the constraint role has been set explicitly set on this constraint or inherited from a base constraint.
    bool IsRoleLabelDefined() const {return GetInvariantRoleLabel().length() > 0;}

    //! Sets the bool value of whether this constraint can also relate to instances of subclasses of classes applied to the constraint.
    //! @param[in] isPolymorphic String representation of true/false
    //! @return ::Success if the string is parsed into a bool
    ECOBJECTS_EXPORT ECObjectsStatus SetIsPolymorphic(Utf8CP isPolymorphic);
    
    //! Sets whether this constraint can also relate to instances of subclasses of classes applied to the constraint.
    //! @param[in] isPolymorphic The boolean value to set as the polymorphic flag
    ECObjectsStatus SetIsPolymorphic(bool isPolymorphic) {m_isPolymorphic = isPolymorphic; return ECObjectsStatus::Success;}
    
    //! Determine whether the constraint can also relate to instances of subclasses of classes applied to the constraint.
    bool GetIsPolymorphic() const {return m_isPolymorphic;}

    //! Set the given RelationshipMultiplicity of this constraint.
    //! @param[in] multiplicity The multiplicity to be set on this relationship constraint
    ECOBJECTS_EXPORT ECObjectsStatus SetMultiplicity(RelationshipMultiplicityCR multiplicity);

    //! Set the multiplicity of the constraint using the string format of RelationshipMultiplicity. Multiplicity is not set if input string cannot be parsed.
    //! @param[in] multiplicity The string representation of a multiplicity
    //! @return An error if it fails to parse the multiplicity string into a valid RelationshipMultiplicity.
    ECOBJECTS_EXPORT ECObjectsStatus SetMultiplicity(Utf8CP multiplicity);

    //! Set the multiplicity of the constraint using the string format of Cardinality. Multiplicity is not set if input string cannot be parsed.
    //! Legacy: Only used for version 3.0 and previous
    //! @param[in] cardinality The multiplicity in the form of the legacy cardinality string
    ECOBJECTS_EXPORT ECObjectsStatus SetCardinality(Utf8CP cardinality);

    //! Get the RelationshipMultiplicity of the constraint in the relationship
    RelationshipMultiplicityCR GetMultiplicity() const {return *m_multiplicity;}

    //! Set the abstract constraint class by input string of format {SchemaName}:{ClassName}. All of the constraint classes must be or derive from. 
    //! @param[in] abstractConstraint String representation of the full name of an ECEntityClass
    //! @return ::Success if the abstract constraint can be parsed into a valid ECEntityClass
    ECOBJECTS_EXPORT ECObjectsStatus SetAbstractConstraint(Utf8CP abstractConstraint);
    
    //! Set the abstract constraint class of the constraint in the relationship. 
    //! @remarks The specified class must be a base class class of all constraint classes defined in
    //! @param[in] abstractConstraint The ECEntityClass to be set as the abstract constraint of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus SetAbstractConstraint(ECEntityClassCR abstractConstraint);

    //! Set the abstract constraint class of the constraint in the relationship. 
    //! @remarks The specified class must be a base class class of all constraint classes defined in
    //! @param[in] abstractConstraint The ECRelationshipClass to be set as the abstract constraint of the constraint in the relationship
    ECOBJECTS_EXPORT ECObjectsStatus SetAbstractConstraint(ECRelationshipClassCR abstractConstraint);

    //! Get the abstract constraint class for this ECRelationshipConstraint. 
    //! @remarks If the abstract constraint is not explicitly defined locally, it will be inherited from its base constraint, if one exists.
    //! If one does not exist and there is only one constraint class, that constraint class will be returned. If fail to find a valid class
    //! nullptr will be returned.
    //! @return The abstract constraint, an ECEntityClass or ECRelationshipClass, if one is defined, if one cannot be found nullptr is returned.
    ECOBJECTS_EXPORT ECClassCP const GetAbstractConstraint() const;
    
    //! Determine whether the abstract constraint is set in this constraint.
    bool IsAbstractConstraintDefined() const {return nullptr != m_abstractConstraint;}

    //! Remove the abstract constraint
    void RemoveAbstractConstraint() { m_abstractConstraint = nullptr; }

    //! Add the specified entity class to the constraint. 
    //! @param[in] classConstraint  The ECEntityClass to add as a constraint class
    //! @note If the class does not derive from the abstract constraint it will fail to be added and an error will be returned.
    ECOBJECTS_EXPORT ECObjectsStatus AddClass(ECEntityClassCR classConstraint);

    //! Remove the specified entity class from the constraint.
    //! @param[in] classConstraint  The ECEntityClass to remove from the constraint class list
    ECOBJECTS_EXPORT ECObjectsStatus RemoveClass(ECEntityClassCR classConstraint);

    //! Add the specified relationship class to the constraint. 
    //! @param[in] classConstraint  The ECRelationshipClass to add as a constraint class
    //! @note If the class does not derive from the abstract constraint it will fail to be added and an error will be returned.
    ECOBJECTS_EXPORT ECObjectsStatus AddClass(ECRelationshipClassCR classConstraint);

    //! Remove the specified relationship class from the constraint.
    //! @param[in] classConstraint  The ECRelationshipClass to remove from the constraint class list
    ECOBJECTS_EXPORT ECObjectsStatus RemoveClass(ECRelationshipClassCR classConstraint);

    //! Removes all constraint classes.
    void RemoveConstraintClasses() {m_constraintClasses.clear();}

    //! Returns the classes applied to the constraint.
    ECRelationshipConstraintClassList const & GetConstraintClasses() const {return m_constraintClasses;}

    ECOBJECTS_EXPORT bool SupportsClass(ECClassCR ecClass) const;
    
    //! Copies this constraint to the destination
    //! @param[out] toRelationshipConstraint The relationship constraint to copy to
    //! @param[in] copyReferences If false, a shallow copy of the source relationship constraint will be made meaning it will not copy over any constraint classes or abstract constraint that does not live within the target schema. Instead it will create a schema reference back to the source schema if necessary.
    ECOBJECTS_EXPORT ECObjectsStatus CopyTo(ECRelationshipConstraintR toRelationshipConstraint, bool copyReferences = false);

    //! Returns whether the relationship is ordered on this constraint.
    ECOBJECTS_EXPORT bool GetIsOrdered() const;

    //! Returns the storage mode of the OrderId for this constraint.
    ECOBJECTS_EXPORT OrderIdStorageMode GetOrderIdStorageMode() const;

    //! Gets the name of the OrderId property for this constraint.
    ECOBJECTS_EXPORT ECObjectsStatus GetOrderedRelationshipPropertyName(Utf8String& propertyName) const;
};

//=======================================================================================
//! The in-memory representation of an ECRelationshipClass as defined by ECSchemaXML
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECRelationshipClass : public ECClass
{
DEFINE_T_SUPER(ECClass)
friend struct ECSchema;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;

private:
    bool m_verify;
    mutable bool m_verified;
    StrengthType m_strength;
    ECRelatedInstanceDirection m_strengthDirection;
    ECRelationshipConstraintP m_target;
    ECRelationshipConstraintP m_source;

    //  Lifecycle management:  For now, to keep it simple, the class constructor is private.  The schema implementation will
    //  serve as a factory for classes and will manage their lifecycle.  We'll reconsider if we identify a real-world story for constructing a class outside
    //  of a schema.
    ECRelationshipClass(ECSchemaCR schema, bool verify = true) : ECClass(ECClassType::Relationship, schema), m_strength(StrengthType::Referencing), m_strengthDirection(ECRelatedInstanceDirection::Forward), m_verify(verify), m_verified(false)
        {
        m_source = new ECRelationshipConstraint(this, true, verify);
        m_target = new ECRelationshipConstraint(this, false, verify);
        }
    virtual ~ECRelationshipClass() {delete m_source; delete m_target;}

    ECObjectsStatus SetStrength(Utf8CP strength);
    ECObjectsStatus SetStrengthDirection(Utf8CP direction);

    bool _Validate() const override { return Verify(false); }
    bool Verify(bool resolveIssues) const;
    bool ValidateStrengthConstraint(StrengthType value, bool compareValue=true) const;
    bool ValidateStrengthDirectionConstraint(ECRelatedInstanceDirection value, bool compareValue = true) const;

protected:
    SchemaWriteStatus _WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const override;
    bool _ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const override;
    SchemaReadStatus _ReadXmlAttributes(BeXmlNodeR classNode) override;
    SchemaReadStatus _ReadXmlContents(BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases, bvector<NavigationECPropertyP>& navigationProperties) override;

    ECObjectsStatus _AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts = false, bool validate = true) override;
    ECObjectsStatus _RemoveBaseClass(ECClassCR baseClass) override {m_verified = false; return ECClass::_RemoveBaseClass(baseClass);}
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::RelationshipClass;}

public:
    ECRelationshipConstraintR GetSource() const {return *m_source;} //!< Gets the ECRelationshipConstraint at the source end of the relationship
    ECRelationshipConstraintR GetTarget() const {return *m_target;} //!< Gets the ECRelationshipConstraint at the target end of the relationship

    //! Sets the ::StrengthType of this relationship. 
    //! @remarks The ::StrengthType must be consistent with its base class's strength type. If it is not, Verify() will return false.
    //! @return ::Success if the strength is set, otherwise ::RelationshipConstraintsNotCompatible.
    ECOBJECTS_EXPORT ECObjectsStatus SetStrength(StrengthType value); 
    StrengthType GetStrength() const {return m_strength;} //!< Gets the ::StrengthType of this ECRelationshipClass.

    //! Sets the strength direction for this relationship.
    //! @see ECRelatedInstanceDirection
    //! @remarks The Strength Direction must be consistent with its base class to be valid. If it is not, Verify() will return false.
    //! @return ::Success if the strength direction is set, otherwise ::RelationshipConstraintsNotCompatible.
    ECOBJECTS_EXPORT ECObjectsStatus SetStrengthDirection(ECRelatedInstanceDirection value);
    //! Gets the strength direction
    //! @return ::Forward or ::Backward
    ECRelatedInstanceDirection GetStrengthDirection() const {return m_strengthDirection;}

    //! Returns true if the constraint is ordered.  This is determined by seeing if the custom attribute signifying a Ordered relationship is defined
    ECOBJECTS_EXPORT bool GetIsOrdered() const;
    //! If this relationship class supports ordered relationships, it will get the property name for the provided relationship end.
    //! @param[out] propertyName The property name found for the provided relationship end
    //! @param[in] end The end to get the property name from.
    //! @return ::Success if the relationship class supports ordered relationships has a valid property name, otherwise ::Error.
    ECOBJECTS_EXPORT ECObjectsStatus GetOrderedRelationshipPropertyName(Utf8String& propertyName, ECRelationshipEnd end) const;

    //! Creates a navigation property object using the relationship class and direction.  To succeed the relationship class, direction and name must all be valid.
    // @param[out]  ecProperty          Outputs the property if successfully created
    // @param[in]   name                The name for the navigation property.  Must be a valid ECName
    // @param[in]   relationshipClass   The relationship class this navigation property will traverse.  Must list this class as an endpoint constraint.  The multiplicity of the other constraint determiness if the nav prop is a primitive or an array.
    // @param[in]   direction           The direction the relationship will be traversed.  Forward indicates that this class is a source constraint, Backward indicates that this class is a target constraint.
    // @param[in]   verify              If true the relationshipClass an direction will be verified to ensure the navigation property fits within the relationship constraints.  Default is true.  If not verified at creation the Verify method must be called before the navigation property is used or it's type descriptor will not be valid.
    ECOBJECTS_EXPORT ECObjectsStatus CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, bool verify = true);

    //! Returns true if successfully verifies the relationship, otherwise false.
    ECOBJECTS_EXPORT bool Verify() const;
    //! Returns true if the relationship is verified.
    ECOBJECTS_EXPORT bool GetIsVerified();
}; // ECRelationshipClass

typedef RefCountedPtr<ECRelationshipClass>      ECRelationshipClassPtr;

//! Defines what sort of match should be used when locating a schema
enum class SchemaMatchType
{
    //! Find exact VersionRead, VersionWrite, VersionMinor match as well as Data
    Identical,
    //! Find exact VersionRead, VersionWrite, VersionMinor match.
    Exact,
    //! Find latest version with matching VersionRead and VersionWrite
    LatestWriteCompatible,
    //! Find latest version.
    Latest,
    //! Find latest version with matching VersionRead
    LatestReadCompatible,
};

//=======================================================================================
//! Fully defines a schema with its name, read, write and minor versions.
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
//!   - If schema in the repository is newer (or same):
//!       - If first digit matches, app can safely read
//!       - If first two digits match, app can safely write (and read)
//!   - If schema in the repository is older:
//!       - If first two digits match, app can upgrade repository schema without breaking read or write for other apps
//!       - If only first digit matches, app can upgrade repository, but upgrade will prevent some older apps from writing
//! 
//! For traditional EC developers it may be difficult to envision when a schema change would require a change
//! to the middle version number. Consider in schema 1 that we have a Student class that stores grades and has (double) properties:
//!   - Language
//!   - Math
//!   - Science
//!   - Music
//!   - Overall GPA (an average of the previous 4 properties)
//! 
//! If schema 2 adds to Student a double property Psychology, the meaning of Overall GPA changes slightly and hence, applications written for Schema 1:
//!   - Can still safely read all the values that were in schema 1
//!   - Cannot modify any values that were in schema 1 because they will likely set Overall GPA incorrectly.
//!
// @bsiclass
//=======================================================================================
struct SchemaKey
{
    uint32_t      m_versionRead;
    uint32_t      m_versionWrite;
    uint32_t      m_versionMinor;
    Utf8String    m_schemaName;
    Utf8String    m_checksum;

    //! Creates a new SchemaKey with the given name and version information
    //! @param[in]  name    The name of the ECSchema
    //! @param[in]  read    The read portion of the version
    //! @param[in]  minor   The minor portion of the version
    SchemaKey (Utf8CP name, uint32_t read, uint32_t minor) : m_schemaName(name), m_versionRead(read), m_versionWrite(DEFAULT_VERSION_WRITE), m_versionMinor(minor){}

    //! Creates a new SchemaKey with the given name and version information
    //! @param[in]  name    The name of the ECSchema
    //! @param[in]  read    The read portion of the version
    //! @param[in]  write   The write portion of the version
    //! @param[in]  minor   The minor portion of the version
    SchemaKey(Utf8CP name, uint32_t read, uint32_t write, uint32_t minor) : m_schemaName(name), m_versionRead(read), m_versionWrite(write), m_versionMinor(minor){}

    //! Default constructor
    SchemaKey () : m_versionRead(DEFAULT_VERSION_READ), m_versionWrite(DEFAULT_VERSION_WRITE), m_versionMinor(DEFAULT_VERSION_MINOR){}

    Utf8StringCR GetName() const {return m_schemaName;}

    //! Gets the read schema version. Identifies the generation of the schema that guarantees that newer schemas can be
    //! read by older software.
    uint32_t GetVersionRead() const {return m_versionRead;}

    //! Gets the major for write version. This is less significant than the read version. It identifies the generation of the schema
    //! that guarantees that newer schemas can be written by older software.
    uint32_t GetVersionWrite() const {return m_versionWrite;}

    //! Least significant version number that increments with read/write compatible additions.
    uint32_t GetVersionMinor() const {return m_versionMinor;}

    //! Given a full schema name (which includes the version information), will return a SchemaKey with the schema name and version information set
    //! @param[out] key             A SchemaKey with the schema's name and version set
    //! @param[in]  schemaFullName  The full name of the schema.
    static ECObjectsStatus ParseSchemaFullName(SchemaKey& key, Utf8CP schemaFullName) {return ParseSchemaFullName(key.m_schemaName, key.m_versionRead, key.m_versionWrite, key.m_versionMinor, schemaFullName);}

    //! Given a version string RR.WW.MM, this will parse into read, write and minor versions
    //! @param[out] versionRead    The read version number
    //! @param[out] versionWrite   The write version number. Will default to zero if the string only contains two numbers.
    //! @param[out] versionMinor   The minor version number
    //! @param[in]  versionString   A string containing the read, write and minor versions (RR.WW.MM)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionString(uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString);

    //! Given a version string RR.WW.MM, this will parse into read, write and minor versions
    //! @param[out] versionRead    The read version number
    //! @param[out] versionWrite   The write version number.
    //! @param[out] versionMinor   The minor version number
    //! @param[in]  versionString   A string containing the read, write and minor versions (RR.WW.MM)
    //! @return A status code indicating whether the string was successfully parsed. Will return error if there are fewer
    //! than 3 version numbers
    ECOBJECTS_EXPORT static ECObjectsStatus ParseVersionStringStrict(uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString);

    //! Given a version string RR.WW.MM, this will parse read, write and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionRead     The read version number
    //! @param[out] versionWrite    The write  version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and read, write and minor versions (GetName().RR.WW.MM)
    //! @return A status code indicating whether the string was successfully parsed
    ECOBJECTS_EXPORT static ECObjectsStatus ParseSchemaFullName(Utf8StringR schemaName, uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP fullName);

    //! Return full schema name in format GetName().RR.ww.mm where Name is the schema name RR is read version, ww is the write version and mm is minor version.
    Utf8String GetFullSchemaName() const {return FormatFullSchemaName(m_schemaName.c_str(), m_versionRead, m_versionWrite, m_versionMinor);}

    //! Generate a schema full name string given the read, write and minor version values.
    //! @param[in] schemaName      Name of the schema
    //! @param[in] versionRead     The read version number
    //! @param[in] versionWrite    The write version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatFullSchemaName(Utf8CP schemaName, uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor);

    //! Generate a legacy schema full name, which does not contain the write version.
    //! @param[in] schemaName      Name of the schema
    //! @param[in] versionRead     The read version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatLegacyFullSchemaName(Utf8CP schemaName, uint32_t versionRead, uint32_t versionMinor);

    //! Generate a schema version string given the read, write and minor version values.
    //! @param[in] versionRead     The read version number
    //! @param[in] versionWrite    The write version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatSchemaVersion(uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor);

    //! Generate a legacy schema version string given the read and minor version values.
    //! @param[in] versionRead     The read version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    ECOBJECTS_EXPORT static Utf8String FormatLegacySchemaVersion(uint32_t versionRead, uint32_t versionMinor);

    //! Generate a schema version string given the read, write and minor version values.
    Utf8String GetVersionString() const {return FormatSchemaVersion(m_versionRead, m_versionWrite, m_versionMinor);}

    //! Generate a legacy schema version string given the read and minor version values.
    Utf8String GetLegacyVersionString() const {return FormatLegacySchemaVersion(m_versionRead, m_versionMinor);}

    //! Compares two SchemaKeys and returns whether the target schema is less than this SchemaKey, where LessThan is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SchemaMatchType::Identical - Returns whether the current schema's checksum is less than the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SchemaMatchType::Exact - This will first test the names, then the read version, and lastly the minor version
    //! @li SchemaMatchType::LatestReadCompatible - This will first test the names and then the read versions.
    //! @li SchemaMatchType::LatestWriteCompatible - This will first test the names and then the read and write versions.
    //! @li SchemaMatchType::Latest - Returns whether the current schema's name is less than the target's.
    ECOBJECTS_EXPORT bool LessThan(SchemaKeyCR rhs, SchemaMatchType matchType) const;
    
    //! Compares two SchemaKeys and returns whether the target schema matches this SchemaKey, where "matches" is dependent on the match type
    //! @param[in]  rhs         The SchemaKey to compare to
    //! @param[in]  matchType   The type of match to compare for
    //! @returns The comparison is based on the SchemaMatchType, defined by:
    //! @li SchemaMatchType::Identical - Returns whether the current schema's checksum is less than the target's checksum.  If the checksum is not set, it falls through to the Exact match
    //! @li SchemaMatchType::Exact - Returns whether this schema's name, read version, and minor version are all equal to the target's.
    //! @li SchemaMatchType::LatestWriteCompatible - Returns whether this schema's name and read version are equal, and this schema's write version is greater than or equal to the target's.
    //! @li SchemaMatchType::LatestReadCompatible - Returns whether this schema's name and read version are equal, and this schema's write version is greater than or equal to the target's.
    //! @li SchemaMatchType::Latest - Returns whether the current schema's name is equal to the target's.
    ECOBJECTS_EXPORT bool Matches(SchemaKeyCR rhs, SchemaMatchType matchType) const;

    //! Compares two schema names and returns whether the target schema matches this m_schemaName. Comparison is case-sensitive
    //! @param[in]  schemaName  The schema name to compare to
    ECOBJECTS_EXPORT int CompareByName(Utf8StringCR schemaName) const;

    //! Compares the schema version of this key to the schema version of @p rhs.
    //! @param[in] rhs Right-hand side schema key
    //! @return a negative number if the LHS version is less than the RHS version, a positive number if LHS version is
    //! greater than the RHS version, 0 if the versions are the same.
    ECOBJECTS_EXPORT int CompareByVersion(SchemaKeyCR rhs) const;

    //! Returns whether this SchemaKey is Identical to the target SchemaKey
    bool operator == (SchemaKeyCR rhs) const {return Matches(rhs, SchemaMatchType::Identical);}

    //! Returns true if the target SchemaKey is not Identical to this SchemaKey, false otherwise
    bool operator != (SchemaKeyCR rhs) const {return !(*this == rhs);}

    //! Returns whether this SchemaKey's checksum is less than the target SchemaKey's.
    bool operator < (SchemaKeyCR rhs) const {return LessThan (rhs, SchemaMatchType::Identical);}
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

//=======================================================================================
// @bsistruct
//=======================================================================================
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

//=======================================================================================
// @bsistruct
//=======================================================================================
struct SchemaNameClassNamePair
{
public:
    Utf8String m_schemaName;
    Utf8String m_className;

    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair(Utf8StringCR schemaName, Utf8StringCR className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs a SchemaNameClassNamePair from the specified schema and class names
    SchemaNameClassNamePair(Utf8CP schemaName, Utf8CP className) : m_schemaName (schemaName), m_className  (className) {}
    //! Constructs an empty SchemaNameClassNamePair
    SchemaNameClassNamePair() { }
    //! Constructs a SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    SchemaNameClassNamePair(Utf8StringCR schemaAndClassNameSeparatedByColon)
        {
        BeAssert(Utf8String::npos != schemaAndClassNameSeparatedByColon.find (':'));
        Parse (schemaAndClassNameSeparatedByColon);
        }

    //! Attempts to populate this SchemaNameClassNamePair from a string of the format "SCHEMANAME:CLASSNAME"
    //! @param[in]      schemaAndClassNameSeparatedByColon a string of the format "SCHEMANAME:CLASSNAME"
    //! @return true if the string was successfully parsed, false otherwise. If it returns false, this SchemaNameClassNamePair will not be modified.
    bool Parse(Utf8StringCR schemaAndClassNameSeparatedByColon)
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
    Utf8String ToColonSeparatedString() const
        {
        Utf8String str;
        str.Sprintf("%s:%s", m_schemaName.c_str(), m_className.c_str());
        return str;
        }

    //!<@private
    ECOBJECTS_EXPORT bool Remap(ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper);
};

/*---------------------------------------------------------------------------------**//**
* Identifies an ECProperty by schema name, class name, and access string. The class name
* may refer to the ECClass containing the ECProperty, or a subclass thereof.
* @bsistruct                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct QualifiedECAccessor
{
protected:
    Utf8String m_schemaName;
    Utf8String m_className;
    Utf8String m_accessString;
public:
    //! Constructs an empty QualifiedECAccessor
    QualifiedECAccessor() { }
    //! Constructs a QualifiedECAccessor referring to a property of an ECClass specified by access string
    QualifiedECAccessor (Utf8CP schemaName, Utf8CP className, Utf8CP accessString)
        : m_schemaName(schemaName), m_className(className), m_accessString(accessString) { }

    //! Returns the name of the schema containing the ECClass
    Utf8CP GetSchemaName() const {return m_schemaName.c_str();}
    //! Returns the name of the ECClass (or subclass thereof) containing the ECProperty
    Utf8CP GetClassName() const {return m_className.c_str();}
    //! Returns the access string identifying the ECProperty within the ECClass
    Utf8CP GetAccessString() const {return m_accessString.c_str();}

    //! Sets the name of the schema containing the ECClass
    void SetSchemaName(Utf8CP name) {m_schemaName = name;}
    //! Sets the name of the ECClass (or subclass thereof) containing the ECProperty
    void SetClassName(Utf8CP name) {m_className = name;}
    //! Sets the access string identifying the ECProperty within the ECClass
    void SetAccessString(Utf8CP acStr) {m_accessString = acStr;}

    //! Returns a colon-separated string of the format "Schema:Class:AccessString"
    ECOBJECTS_EXPORT Utf8String ToString() const;
    //! Attempts to initialize this QualifiedECAccessor from a colon-separated string. If the string cannot be parsed, this QualifiedECAccessor will not be modified
    //! @param[in]      str A string of the format "Schema:Class:AccessString"
    //! @return true if the string was successfully parsed
    ECOBJECTS_EXPORT bool FromString (Utf8CP str);

    //! Attempts to initialize this QualifiedECAccessor from an access string identifying a property of the specified ECEnabler.
    //! If the ECProperty cannot be found within the ECEnabler, this QualifiedECAccessor will not be modified
    //! @param[in]      rootEnabler  The ECEnabler containing the desired ECProperty
    //! @param[in]      accessString The access string identifying the ECProperty within the ECEnabler
    //! @return true if the access string identifies a valid ECProperty within the ECEnabler
    ECOBJECTS_EXPORT bool FromAccessString(ECN::ECEnablerCR rootEnabler, Utf8CP accessString);
    //!<@private
    ECOBJECTS_EXPORT bool Remap(ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper);
    //!<@private
    Utf8StringR GetSchemaNameR() {return m_schemaName;}
    //!<@private
    Utf8StringR GetClassNameR() {return m_className;}
    //!<@private
    Utf8StringR GetAccessStringR() {return m_accessString;}
};

typedef bvector<QualifiedECAccessor> QualifiedECAccessorList;
typedef QualifiedECAccessorList const& QualifiedECAccessorListCR;

//=======================================================================================
// @bsistruct
//=======================================================================================
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
    ECOBJECTS_EXPORT ECClassP  FindClassP(ECN::SchemaNameClassNamePair const& classNamePair) const;
};

typedef SchemaMapExact                  ECSchemaReferenceList;
typedef const ECSchemaReferenceList&    ECSchemaReferenceListCR;

//=======================================================================================
// @bsistruct
//=======================================================================================
template <typename CONTAINER_MAP_TYPE, typename MAP_TYPE>
struct SchemaItemContainer
{
private:
    CONTAINER_MAP_TYPE const& m_map;

protected:
    SchemaItemContainer(CONTAINER_MAP_TYPE const& map) : m_map(map) {};

public:
    
    //=======================================================================================
    // @bsistruct
    //=======================================================================================
    struct IteratorState : RefCountedBase
    {
    private:
        friend struct const_iterator;
    public:
        typename CONTAINER_MAP_TYPE::const_iterator     m_mapIterator;

        IteratorState (typename CONTAINER_MAP_TYPE::const_iterator mapIterator) { m_mapIterator = mapIterator; };
        static RefCountedPtr<IteratorState> Create (typename CONTAINER_MAP_TYPE::const_iterator mapIterator) { return new IteratorState(mapIterator); };
    };

    //=======================================================================================
    // @bsistruct
    //=======================================================================================
    struct const_iterator : std::iterator<std::forward_iterator_tag, MAP_TYPE const*>
    {
    private:
        friend struct SchemaItemContainer;
        RefCountedPtr<IteratorState>   m_state;

        const_iterator (typename CONTAINER_MAP_TYPE::const_iterator mapIterator) { m_state = IteratorState::Create (mapIterator); };
        const_iterator (char* ) {;} // must publish at least one private constructor to prevent instantiation

    public:
        const_iterator& operator++()
            {
            m_state->m_mapIterator++;
            return *this;
            }
        
        bool operator!=(const_iterator const& rhs) const
            {
            return (m_state->m_mapIterator != rhs.m_state->m_mapIterator);
            }
        
        bool operator==(const_iterator const& rhs) const
            {
            return (m_state->m_mapIterator == rhs.m_state->m_mapIterator);
            }
        
        MAP_TYPE const& operator* () const
            {
            #ifdef CREATES_A_TEMP
                bpair<WCharCP , MAP_TYPE*> const& mapPair = *(m_state->m_mapIterator);
                return mapPair.second;
            #else
                return m_state->m_mapIterator->second;
            #endif
            }
    };

    const_iterator begin() const { return SchemaItemContainer<CONTAINER_MAP_TYPE, MAP_TYPE>::const_iterator(m_map.begin()); } //!< Returns the beginning of the iterator
    const_iterator end()   const { return SchemaItemContainer<CONTAINER_MAP_TYPE, MAP_TYPE>::const_iterator(m_map.end()); } //!< Returns the end of the iterator
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECClassContainer : SchemaItemContainer<ClassMap, ECClassP>
{
friend struct ECSchema;
using SchemaItemContainer<ClassMap, ECClassP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECEnumerationContainer : SchemaItemContainer<EnumerationMap, ECEnumerationP>
{
friend struct ECSchema;
using SchemaItemContainer<EnumerationMap, ECEnumerationP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct KindOfQuantityContainer : SchemaItemContainer<KindOfQuantityMap, KindOfQuantityP>
{
friend struct ECSchema;
using SchemaItemContainer<KindOfQuantityMap, KindOfQuantityP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatContainer : SchemaItemContainer<FormatMap, ECFormatP>
{
friend struct ECSchema;
using SchemaItemContainer<FormatMap, ECFormatP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct UnitSystemContainer : SchemaItemContainer<UnitSystemMap, UnitSystemP>
{
friend struct SchemaUnitContext;
using SchemaItemContainer<UnitSystemMap, UnitSystemP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct PhenomenonContainer : SchemaItemContainer<PhenomenonMap, PhenomenonP>
{
friend struct SchemaUnitContext;
using SchemaItemContainer<PhenomenonMap, PhenomenonP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct UnitContainer : SchemaItemContainer<UnitMap, ECUnitP>
{
friend struct SchemaUnitContext;
using SchemaItemContainer<UnitMap, ECUnitP>::SchemaItemContainer;
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct PropertyCategoryContainer : SchemaItemContainer<PropertyCategoryMap, PropertyCategoryP>
{
friend struct ECSchema;
using SchemaItemContainer<PropertyCategoryMap, PropertyCategoryP>::SchemaItemContainer;
};

//=======================================================================================
//! Interface to find a standalone enabler, typically for an embedded ECStruct in an ECInstance.
//! @bsiclass
//=======================================================================================
struct IStandaloneEnablerLocater
{
private:
    DECLARE_KEY_METHOD

protected:
    virtual StandaloneECEnablerPtr  _LocateStandaloneEnabler(SchemaKeyCR schemaKey, Utf8CP className) = 0;

public:
    //! Given a SchemaKey and a className, tries to locate the StandaloneEnabler for the ECClass
    //! @param[in] schemaKey    SchemaKey fully describing the schema that the class belongs to
    //! @param[in] className    The name of the class to find the enabler for
    //! @returns A valid StandaloneECEnabler, if one was located
    ECOBJECTS_EXPORT StandaloneECEnablerPtr  LocateStandaloneEnabler(SchemaKeyCR schemaKey, Utf8CP className);
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
    ECSchemaPtr LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) {return _LocateSchema(key, matchType, schemaContext);}
};

typedef RefCountedPtr<ECSchemaCache>        ECSchemaCachePtr;
//=======================================================================================
//! An object that controls the lifetime of a set of ECSchemas.  When the schema
//! owner is destroyed, so are the schemas that it owns.
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECSchemaCache : public RefCountedBase, public IECSchemaLocater
{
protected:
    SchemaMap   m_schemas;

    // TODO: Uncomment this and remove the public desctructor once ECDb stops declaring this on the stack.
    // ECSchemaCache() {}
    // ECOBJECTS_EXPORT virtual ~ECSchemaCache ();

    ECSchemaPtr _LocateSchema(SchemaKeyR schema, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override {return GetSchema(schema, matchType);}
public:
    //! Removes the input schema and all schemas which reference it from the cache
    ECObjectsStatus DropAllReferencesOfSchema(ECSchemaR schema);
    //! Adds a schema to the cache
    //! @param[in] schema   The ECSchema to add to the cache
    //! @returns ECObjectsStatus::DuplicateSchema is the schema is already in the cache, otherwise ECObjectsStatus::Success
    ECOBJECTS_EXPORT ECObjectsStatus AddSchema(ECSchemaR schema);

    //! Removes the specified schema from this cache
    //! @param[in] key  The SchemaKey fully describing the schema that should be removed from the cache
    //! @returns ECObjectsStatus::SchemaNotFound is the schema was not found in the cache, otherwise ECObjectsStatus::Success
    ECOBJECTS_EXPORT ECObjectsStatus DropSchema(SchemaKeyCR key );

    //! Get the requested schema from this cache
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @returns The ECSchema if it is contained in the cache, NULL otherwise
    //! @remarks This will do an Identical match type for the requested schema
    ECSchemaP GetSchema(SchemaKeyCR key) const {return GetSchema(key, SchemaMatchType::Identical);}

    //! Get the requested schema from this cache.
    //! @param[in] key  The SchemaKey fully describing the schema to be retrieved
    //! @param[in] matchType    The SchemaMatchType defining how exact of a match for the located schema is tolerated
    //! @returns The ECSchema if it is contained in the cache; otherise nullptr.
    ECOBJECTS_EXPORT ECSchemaP GetSchema(SchemaKeyCR key, SchemaMatchType matchType) const;

    virtual ~ECSchemaCache() {m_schemas.clear();} //!< Destructor
    static ECSchemaCachePtr Create() {return new ECSchemaCache;}; //!< Creates an ECSchemaCachePtr
    int GetCount() const {return (int)m_schemas.size();} //!< Returns the number of schemas currently in the cache
    void Clear() {m_schemas.clear();}; //!< Removes all schemas from the cache
    IECSchemaLocater& GetSchemaLocater() {return *this;} //!< Returns the SchemaCache as an IECSchemaLocater
    ECOBJECTS_EXPORT bvector<ECSchemaCP> GetSchemas() const;
    ECOBJECTS_EXPORT size_t GetSchemas (bvector<ECSchemaP>& schemas) const;
    ECOBJECTS_EXPORT void GetSupplementalSchemasFor(Utf8CP schemaName, bvector<ECSchemaP>& supplementalSchemas) const;
};

//=======================================================================================
//! Locates schemas by looking in a given set of file system folder for ECSchemaXml files
//! For internal use only, use ECSchemaReadContext to setup search paths for schemas
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
    bool m_includeFilesWithNoVersionExt;

    static bool TryLoadingSupplementalSchemas(Utf8StringCR schemaName, WStringCR schemaFilePath, ECSchemaReadContextR schemaContext, bvector<ECSchemaP>& supplementalSchemas);

    void FindEligibleSchemaFiles(bvector<CandidateSchema>& foundFiles, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext);
    void AddCandidateSchemas(bvector<CandidateSchema>& foundFiles, WStringCR schemaPath, WStringCR fileFilter, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext);
    void AddCandidateNoExtensionSchema(bvector<CandidateSchema>& foundFiles, WStringCR schemaPath, Utf8CP schemaName, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext);

    //! Returns true if the first element goes before the second
    static bool SchemyKeyIsLessByVersion(CandidateSchema const& lhs, CandidateSchema const& rhs) {return lhs.Key.CompareByVersion(rhs.Key) < 0;}

protected:
    ECOBJECTS_EXPORT SearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt);
    ECOBJECTS_EXPORT virtual ~SearchPathSchemaFileLocater();
    ECOBJECTS_EXPORT ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;

public:
    //! Get the search paths registered for this locater
    bvector<WString>const& GetSearchPath() const {return m_searchPaths;}
    //! Create a new SearchPathSchemaFileLocater using the input paths as schema search paths
    ECOBJECTS_EXPORT static SearchPathSchemaFileLocaterPtr CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt=false);
};

struct SupplementalSchemaInfo;
typedef RefCountedPtr<SupplementalSchemaInfo> SupplementalSchemaInfoPtr;

enum class ECSchemaElementType
{
    ECClass,
    ECEnumeration,
    KindOfQuantity,
    PropertyCategory,
    UnitSystem,
    Phenomenon,
    Unit,
    InvertedUnit,
    Constant,
    Format
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECSchemaElementsOrder : NonCopyableClass
{
private:
    bvector<bpair<Utf8String, ECSchemaElementType>> m_elementVector;
    bool m_preserveElementOrder;

    template <typename T, typename T_Container>
    void AddElements(T_Container const& container, ECSchemaElementType elementType)
        {
        for (T* schemaChild : container)
            {
            if (nullptr == schemaChild)
                {
                BeAssert(false);
                continue;
                }
            else
                AddElement(schemaChild->GetName().c_str(), elementType);
            }
        }

public:
    ECSchemaElementsOrder() : m_preserveElementOrder(false) {}

    void AddElement(Utf8CP name, ECSchemaElementType type) {m_elementVector.push_back(make_bpair<Utf8String, ECSchemaElementType>(name, type));}
    void RemoveElement(Utf8CP name)
        {
        for (auto iterator = m_elementVector.begin(); iterator != m_elementVector.end(); ++iterator)
            if (iterator->first == name)
                {
                m_elementVector.erase(iterator);
                return;
                }
        }

    ECOBJECTS_EXPORT void CreateAlphabeticalOrder(ECSchemaCR ecSchema);
    bool GetPreserveElementOrder() const {return m_preserveElementOrder;}
    void SetPreserveElementOrder(bool value) {m_preserveElementOrder = value;}
    bvector<bpair<Utf8String, ECSchemaElementType>>::const_iterator begin() const {return m_elementVector.begin();} //!< Returns the beginning of the iterator
    bvector<bpair<Utf8String, ECSchemaElementType>>::const_iterator end() const {return m_elementVector.end();} //!< Returns the end of the iterator
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct SchemaUnitContext : Units::IUnitsContext
{
friend struct ECSchema; // Needed to not expose the add/remove methods publicly
private:
    ECSchemaCR m_schema;

    UnitSystemContainer m_unitSystemContainer;
    PhenomenonContainer m_phenomenonContainer;
    UnitContainer m_unitContainer;

    // maps name -> pointer
    UnitSystemMap m_unitSystemMap;
    PhenomenonMap m_phenomenonMap;
    UnitMap m_unitMap;

    ~SchemaUnitContext();

    template<typename T, typename T_MAP>
    ECObjectsStatus AddToMap(T* toAdd, T_MAP* map, ECSchemaElementType unitType)
        {
        if (false == map->insert(bpair<Utf8CP, T*>(toAdd->GetName().c_str(), toAdd)).second)
            return ECObjectsStatus::Error;
        return ECObjectsStatus::Success;
        }
    template<typename T> ECObjectsStatus Add(T* toAdd, ECSchemaElementType unitType);

    template<typename T, typename T_MAP>
    ECObjectsStatus DeleteFromMap(T& child, T_MAP* map)
        {
        auto iter = map->find(child.GetName().c_str());
        if (iter == map->end() || iter->second != &child)
            return ECObjectsStatus::NotFound;

        map->erase(iter);
        delete &child;
        return ECObjectsStatus::Success;
        }
    template<typename T> ECObjectsStatus Delete(T& toDelete);

    template<typename T, typename T_MAP>
    T* GetFromMap(Utf8CP name, T_MAP const* map) const
        {
        auto iter = map->find(name);
        if (iter != map->end())
            return iter->second;
        return nullptr;
        }
    template<typename T> T* Get(Utf8CP name) const;

protected:
    SchemaUnitContext(ECSchemaCR schema) : m_schema(schema), m_unitSystemContainer(m_unitSystemMap),
        m_phenomenonContainer(m_phenomenonMap), m_unitContainer(m_unitMap) {}

    // Following methods fulfill IUnitsContext requirements
    ECOBJECTS_EXPORT ECUnitP _LookupUnitP(Utf8CP name, bool useFullName) const override;
    ECOBJECTS_EXPORT PhenomenonP _LookupPhenomenonP(Utf8CP name, bool useFullName) const override;
    ECOBJECTS_EXPORT UnitSystemP _LookupUnitSystemP(Utf8CP name, bool useFullName) const override;
    ECOBJECTS_EXPORT void _AllUnits(bvector<Units::UnitCP>& allUnits) const override;
    ECOBJECTS_EXPORT void _AllPhenomena(bvector<Units::PhenomenonCP>& allPhenomena) const override;
    ECOBJECTS_EXPORT void _AllSystems(bvector<Units::UnitSystemCP>& allUnitSystems) const override;

public:
    UnitSystemContainerCR GetUnitSystems() const {return m_unitSystemContainer;} //!< Returns an iterable container of UnitSystems sorted by name.
    uint32_t GetUnitSystemCount() const {return (uint32_t)m_unitSystemMap.size();} //!< Gets the number of UnitSystems in the schema.

    PhenomenonContainerCR GetPhenomena() const {return m_phenomenonContainer;} //!< Returns an iterable container of Phenomena sorted by name.
    uint32_t GetPhenomenonCount() const {return (uint32_t)m_phenomenonMap.size();} //!< Gets the number of Phenomena in the schema.

    UnitContainerCR GetUnits() const {return m_unitContainer;} //!< Returns an iterable container of ECUnits sorted by name.
    uint32_t GetUnitCount() const {return (uint32_t)m_unitMap.size();} //!< Gets the number of ECUnit in the schema.

    //! Looks up an ECUnit by within this. If the name is fully qualified it will search reference
    //! schemas.
    //! @param[in]  name     The name of the unit to lookup.  Can be either an unqualified (short) name or a qualified name.
    //! @param[in]  useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the an ECUnit if the named unit exists within the current schema or one of its reference schemas; otherwise, nullptr.
    ECUnitCP LookupUnit(Utf8CP name, bool useFullName = false) const override {return _LookupUnitP(name, useFullName);}
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
    uint32_t _GetExcessiveRefCountThreshold() const override { return 10000; }

// This is needed to force removal of the Units schema from downconverted schemas.
friend struct ECSchemaDownConverter;

friend struct SearchPathSchemaFileLocater;
friend struct SupplementedSchemaBuilder;
friend struct SchemaXmlReader;
friend struct SchemaXmlReaderImpl;
friend struct SchemaXmlWriter;
friend struct SchemaJsonWriter;

// Schemas are RefCounted but none of the constructs held by schemas (classes, properties, etc.) are.
// They are freed when the schema is freed.

private:
    SchemaKey               m_key;
    Utf8String              m_alias;
    Utf8String              m_displayLabel;
    mutable ECSchemaId      m_ecSchemaId;
    Utf8String              m_description;
    ECClassContainer        m_classContainer;
    ECEnumerationContainer  m_enumerationContainer;
    KindOfQuantityContainer m_kindOfQuantityContainer;
    PropertyCategoryContainer m_propertyCategoryContainer;
    FormatContainer         m_formatContainer;
    SchemaUnitContext       m_unitsContext;

    ECVersion               m_ecVersion;

    uint32_t                m_originalECXmlVersionMajor;
    uint32_t                m_originalECXmlVersionMinor;

    // maps class name -> class pointer
    ClassMap                    m_classMap;
    EnumerationMap              m_enumerationMap;
    KindOfQuantityMap           m_kindOfQuantityMap;
    PropertyCategoryMap         m_propertyCategoryMap;
    FormatMap                   m_formatMap;
    ECSchemaReferenceList       m_refSchemaList;
    bool                        m_isSupplemented;
    bool                        m_hasExplicitDisplayLabel;
    SupplementalSchemaInfoPtr   m_supplementalSchemaInfo;
    bool                        m_immutable;
    mutable ECSchemaElementsOrder m_serializationOrder; //mutable because we might modify this during serialization

    bmap<ECSchemaP, Utf8String> m_referencedSchemaAliasMap;
    SchemaLocalizedStrings      m_localizedStrings;

    ECSchema() : m_classContainer(m_classMap), m_enumerationContainer(m_enumerationMap), m_isSupplemented(false),
        m_hasExplicitDisplayLabel(false), m_immutable(false), m_kindOfQuantityContainer(m_kindOfQuantityMap),
        m_propertyCategoryContainer(m_propertyCategoryMap), m_formatContainer(m_formatMap), m_unitsContext(*this)
        { }
    virtual ~ECSchema();

    bool AddingSchemaCausedCycles() const;
    void SetIsSupplemented(bool isSupplemented) {m_isSupplemented = isSupplemented;}

    void FindUniqueClassName(Utf8StringR newName, Utf8CP originalName);
    bool NamedElementExists(Utf8CP name);
    ECObjectsStatus AddClass(ECClassP pClass, bool resolveConflicts = false);

    ECObjectsStatus SetVersionFromString(Utf8CP versionString);
    ECObjectsStatus SetECVersion(ECVersion ecVersion);

    void SetSupplementalSchemaInfo(SupplementalSchemaInfo* info);

    ECObjectsStatus AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR alias, ECSchemaReadContextR readContext);
    void CollectAllSchemasInGraph(bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const;

    // This is an awful name for this method. There is however a public method called GetSchemaByAliasP which returns
    // a const-pointer. So for now we are not breaking the API to fix the issue.
    ECSchemaP GetSchemaPByAlias(Utf8StringCR alias);

    template<typename T, typename T_MAP>
    ECObjectsStatus AddSchemaChildToMap(T* child, T_MAP* map, ECSchemaElementType childType, bool logError = true);

    template<typename T>
    ECObjectsStatus AddUnitType(T* child, ECSchemaElementType childType);

    // Using explicit template specialization to wrap the calls to AddSchemaChildToMap. This is only used in the SchemaXmlReaderImpl to avoid using the individual maps directly.
    // For information on the NOT_USED template param look at the comments on AddSchemaChildToMap.
    template<typename T>
    ECObjectsStatus AddSchemaChild(T* child, ECSchemaElementType childType);

    template<typename T, typename T_MAP>
    ECObjectsStatus DeleteSchemaChild(T& child, T_MAP* map)
        {
        auto iter = map->find(child.GetName().c_str());
        if (iter == map->end() || iter->second != &child)
            return ECObjectsStatus::NotFound;

        map->erase(iter);
        m_serializationOrder.RemoveElement(child.GetName().c_str());
        delete &child;
        return ECObjectsStatus::Success;
        }

    template<typename T>
    ECObjectsStatus DeleteUnitType(T& child);

    template<typename T, typename T_MAP>
    T* GetSchemaChild(Utf8CP name, T_MAP* map)
        {
        auto iter = map->find(name);
        if (iter != map->end())
            return iter->second;
        return nullptr;
        }

protected:
    ECSchemaCP _GetContainerSchema() const override {return this;}
    Utf8CP _GetContainerName() const override {return GetName().c_str();}
    CustomAttributeContainerType _GetContainerType() const override {return CustomAttributeContainerType::Schema;}

public:
    //! Computes the SHA1 checksum of this schema and sets the resulting checksum on the SchemaKey.
    //! @returns The SHA1 checksum of this schema.
    ECOBJECTS_EXPORT Utf8String ComputeCheckSum ();
    //! Intended to be called by ECDb or a similar system
    void SetId(ECSchemaId id) {BeAssert(!m_ecSchemaId.IsValid()); m_ecSchemaId = id;}
    bool HasId() const {return m_ecSchemaId.IsValid();}

    //!<@private
    ECObjectsStatus DeleteClass(ECClassR ecClass) {return DeleteSchemaChild<ECClass, ClassMap>(ecClass, &m_classMap);}
    //!<@private
    ECOBJECTS_EXPORT ECObjectsStatus RenameClass (ECClassR ecClass, Utf8CP newName);
    SchemaLocalizedStringsCR GetLocalizedStrings() const {return m_localizedStrings;}

    SchemaKeyCR GetSchemaKey() const {return m_key;} //!< Returns a SchemaKey fully describing this schema
    ECOBJECTS_EXPORT void DebugDump() const; //!< Prints out detailed information about this ECSchema, and then calls Dump() on each ECClass.

    //! Used for debugging purposes.
    //! @param[in] showMessages Controls whether messages are displayed during BeXml operations. Defaults to true.
    //! @param[in] doAssert Controls whether asserts should be tested or not.  Defaults to true.
    ECOBJECTS_EXPORT static void SetErrorHandling (bool showMessages, bool doAssert);

    ECSchemaId GetId() const {BeAssert(m_ecSchemaId.IsValid()); return m_ecSchemaId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)

    //! Sets the name of this schema
    //! @param[in]  value   The name of the ECSchema
    //! @returns Success if the name passes validation and is set, ECObjectsStatus::InvalidName otherwise
    ECOBJECTS_EXPORT ECObjectsStatus SetName(Utf8StringCR value);
    Utf8StringCR GetName() const {return m_key.m_schemaName;} //!< Returns the name of this ECSchema

    ECOBJECTS_EXPORT ECObjectsStatus SetAlias(Utf8StringCR value); //!< Sets the alias for this ECSchema
    Utf8StringCR GetAlias() const {return m_alias;} //!< Gets the alias for this ECSchema

    ECOBJECTS_EXPORT ECObjectsStatus SetDescription(Utf8StringCR value); //!< Sets the description for this ECSchema
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description for this ECSchema. Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECSchema.

    ECOBJECTS_EXPORT ECObjectsStatus SetDisplayLabel(Utf8StringCR value); //!< Sets the display label for this ECSchema
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const; //!< Gets the DisplayLabel for this ECSchema.  If no DisplayLabel has been set explicitly, returns the name of the schema.
    Utf8StringCR GetInvariantDisplayLabel() const {return m_hasExplicitDisplayLabel ? m_displayLabel : m_key.m_schemaName;} //!< Gets the invariant display label for this ECSchema.
    bool GetIsDisplayLabelDefined() const {return m_hasExplicitDisplayLabel;} //!< Returns true if the display label has been set explicitly for this schema or not

    ECOBJECTS_EXPORT ECObjectsStatus SetVersionRead(uint32_t value); //!< Sets the read version of this schema, check SchemaKey for detailed description.
    uint32_t GetVersionRead() const {return m_key.m_versionRead;} //!< Gets the read version of this schema, check SchemaKey for detailed description.
    ECOBJECTS_EXPORT ECObjectsStatus SetVersionWrite(uint32_t value); //!< Sets the write compatibility version of this schema, check SchemaKey for detailed description.
    uint32_t GetVersionWrite() const {return m_key.m_versionWrite;} //!< Gets the write compatibility version of this schema, check SchemaKey for detailed description.
    ECOBJECTS_EXPORT ECObjectsStatus SetVersionMinor(uint32_t value); //!< Sets the minor version of this schema, check SchemaKey for detailed description.
    uint32_t GetVersionMinor() const {return m_key.m_versionMinor;} //!< Gets the minor version of this schema, check SchemaKey for detailed description.

    //! Returns true if the original xml version is greater or equal to the input ECVersion
    bool OriginalECXmlVersionAtLeast(ECVersion version) const {return ((m_originalECXmlVersionMajor << 16) | m_originalECXmlVersionMinor) >= static_cast<uint32_t>(version);}
    //! Returns true if the original xml version is less than the input ECVersion
    bool OriginalECXmlVersionLessThan(ECVersion version) const {return ((m_originalECXmlVersionMajor << 16) | m_originalECXmlVersionMinor) < static_cast<uint32_t>(version);}
    //! Returns true if the original xml version is greater than the input ECVersion
    bool OriginalECXmlVersionGreaterThan(ECVersion version) const {return ((m_originalECXmlVersionMajor << 16) | m_originalECXmlVersionMinor) > static_cast<uint32_t>(version);}

    uint32_t GetOriginalECXmlVersionMajor() const {return m_originalECXmlVersionMajor;} //!< Gets the original major xml version.
    uint32_t GetOriginalECXmlVersionMinor() const {return m_originalECXmlVersionMinor;} //!< Gets the original minor xml version.

    //! Sets the original ECXml version of the schema. 
    //! @remarks This method is intended for internal use only.
    //! @param[in] major The version number to set as the major ECXml version
    //! @param[in] minor The version number to set as the minor ECXml version
    //! @return ECObjectsStatus::Success if the ECXml version is successfully set.
    ECObjectsStatus SetOriginalECXmlVersion(uint32_t major, uint32_t minor) { m_originalECXmlVersionMajor = major; m_originalECXmlVersionMinor = minor; return ECObjectsStatus::Success; }

    ECVersion GetECVersion() const {return m_ecVersion;} //!< Gets the EC Version of the schema.
    bool IsECVersion(ECVersion ecVersion) const {return m_ecVersion == ecVersion;} //!< Returns true if this schema's EC version matches the given ECVersion

    ECClassContainerCR GetClasses() const {return m_classContainer;} //!< Returns an iterable container of ECClasses sorted by name.
    uint32_t GetClassCount() const {return (uint32_t) m_classMap.size();} //!< Gets the number of classes in the schema

    ECEnumerationContainerCR GetEnumerations() const {return m_enumerationContainer;} //!< Returns an iterable container of ECEnumerations sorted by name.
    uint32_t GetEnumerationCount() const {return (uint32_t) m_enumerationMap.size();}//!< Gets the number of enumerations in the schema
    ECObjectsStatus DeleteEnumeration(ECEnumerationR ecEnumeration) {return DeleteSchemaChild<ECEnumeration, EnumerationMap>(ecEnumeration, &m_enumerationMap);} //!< Removes an enumeration from this schema.

    FormatContainerCR GetFormats() const {return m_formatContainer;} //!< Returns an iterable container of Formats sorted by name.
    uint32_t GetFormatCount() const {return (uint32_t)m_formatMap.size(); } //!< Gets the number of formats in the schema.
    ECObjectsStatus DeleteFormat(ECFormatR format) {return DeleteSchemaChild<ECFormat, FormatMap>(format, &m_formatMap);} //!< Removes a Format from this schema.

    KindOfQuantityContainerCR GetKindOfQuantities() const {return m_kindOfQuantityContainer;} //!< Returns an iterable container of ECClasses sorted by name.
    uint32_t GetKindOfQuantityCount() const {return (uint32_t) m_kindOfQuantityMap.size();} //!< Gets the number of kind of quantity in the schema
    ECObjectsStatus DeleteKindOfQuantity(KindOfQuantityR kindOfQuantity) {return DeleteSchemaChild<KindOfQuantity, KindOfQuantityMap>(kindOfQuantity, &m_kindOfQuantityMap);} //!< Removes a kind of quantity from this schema.
    
    PropertyCategoryContainerCR GetPropertyCategories() const {return m_propertyCategoryContainer;} //!< Returns an iterable container of PropertyCategories sorted by name.
    uint32_t GetPropertyCategoryCount() const {return (uint32_t) m_propertyCategoryMap.size();} //!< Gets the number of PropertyCategories in the schema.
    ECObjectsStatus DeletePropertyCategory(PropertyCategoryR propertyCategory) {return DeleteSchemaChild<PropertyCategory, PropertyCategoryMap>(propertyCategory, &m_propertyCategoryMap);} //!< Removes a PropertyCategory from this schema.

    UnitSystemContainerCR GetUnitSystems() const {return m_unitsContext.GetUnitSystems();} //!< Returns an iterable container of UnitSystems sorted by name.
    uint32_t GetUnitSystemCount() const {return m_unitsContext.GetUnitSystemCount();} //!< Gets the number of UnitSystems in the schema.
    ECOBJECTS_EXPORT ECObjectsStatus DeleteUnitSystem(UnitSystemR unitSystem);//!< Removes a UnitSystem from this schema.

    PhenomenonContainerCR GetPhenomena() const {return m_unitsContext.GetPhenomena();} //!< Returns an iterable container of Phenomena sorted by name.
    uint32_t GetPhenomenonCount() const {return m_unitsContext.GetPhenomenonCount();} //!< Gets the number of Phenomena in the schema.
    ECOBJECTS_EXPORT ECObjectsStatus DeletePhenomenon(PhenomenonR phenomenon);//!< Removes a Phenomenon from this schema.

    UnitContainerCR GetUnits() const {return m_unitsContext.GetUnits();} //!< Returns an iterable container of ECUnits sorted by name.
    uint32_t GetUnitCount() const {return m_unitsContext.GetUnitCount();} //!< Gets the number of ECUnit in the schema.
    ECOBJECTS_EXPORT ECObjectsStatus DeleteUnit(ECUnitR unit); //!< Removes a ECUnit from this schema.

    SchemaUnitContextCR GetUnitsContext() const {return m_unitsContext;} //< Returns this ECSchema's UnitsContext.

    //! Indicates whether this schema is a so-called @b dynamic schema by
    //! checking whether the @b DynamicSchema custom attribute from the standard schema @b CoreCustomAttributes
    //! is assigned to the schema.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @return true, if this schema is a dynamic schema. false, otherwise
    bool IsDynamicSchema() const {return IsDefined("CoreCustomAttributes", "DynamicSchema") || IsDefined("Bentley_Standard_Custom_Attributes", "DynamicSchema");}

    //! Indicates whether this schema is a system schema (in contrast to a user-supplied schema) by
    //! checking whether the @b %SystemSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A system schema is a schema used and managed internally by the software.
    //! @return true, if this schema is a system schema. false, otherwise
    bool IsSystemSchema() const {return IsDefined("Bentley_Standard_Custom_Attributes", "SystemSchema");}

    //! Validates the schema against the latest version of EC.  
    //! @remarks This method will not attempt to resolve issues found during validation.  Use the overload ECSchema::Validate(bool) to use automatic issue resolution.
    bool Validate() {return Validate(false);}

    //! Validates the schema against the latest version of the EC specification.
    //! @param[in] resolveIssues If true this method will attempt to resolve any issues found.  If false any issues found will fail validation.
    ECOBJECTS_EXPORT bool Validate(bool resolveIssues);

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    ECOBJECTS_EXPORT bool IsStandardSchema() const;

    //! Returns true if the passed in schema is the same base schema as the current schema
    //! @remarks FullName, Alias, and ClassCount are checked
    //! @return True    if the schemas are the same
    ECOBJECTS_EXPORT bool IsSamePrimarySchema(ECSchemaR primarySchema) const;

    //! Indicates whether this schema is a supplemental schema or not.
    //! @return True if this schema is a supplemental schema
    ECOBJECTS_EXPORT bool IsSupplementalSchema() const;

    //! Returns true if the schema is a supplemented schema
    //! @return True if the schema is a supplemented schema
    bool IsSupplemented() const {return m_isSupplemented;}

    //! Gets the SupplementalSchemaInfo for this ECSchema
    SupplementalSchemaInfoPtr const GetSupplementalInfo() const {return m_supplementalSchemaInfo;}

    //! Returns true if and only if the full schema name (including version) represents a standard schema that should never
    //! be stored persistently in a repository (we expect it to be found elsewhere)
    //! @return True if this version of the schema is one that should never be imported into a repository
    bool ShouldNotBeStored() const {return ShouldNotBeStored(GetSchemaKey());}

    //! Returns true if and only if the full schema name (including version) represents a standard schema that should never
    //! be stored persistently in a repository (we expect it to be found elsewhere)
    //! @param[in]  key SchemaKey to test
    //! @return True if this version of the schema is one that should never be imported into a repository
    ECOBJECTS_EXPORT static bool ShouldNotBeStored(SchemaKeyCR key);

    //! If the class name is valid, will create an ECEntityClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECEntityClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateEntityClass(ECEntityClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create a mixin class with the provided appliesTo ECEntityClass and add the new class to the schema.
    //! @remarks A mixin class is an ECEntityClass with the ::ECClassModifier set to ::Abstract and the IsMixin custom attribute set.
    //! @param[out] ecClass If successful, will contain a new ECEntityClass object
    //! @param[in]  name        Name of the class to create
    //! @param[in]  appliesTo   The class used to set the AppliesToEntityClass property in the IsMixin Custom Attribute
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateMixinClass(ECEntityClassP& ecClass, Utf8StringCR name, ECEntityClassCR appliesTo);

    //! If the class name is valid, will create an ECStructClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECStructClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateStructClass(ECStructClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create an ECCustomAttributeClass object and add the new class to the schema
    //! @param[out] ecClass If successful, will contain a new ECCustomAttributeClass object
    //! @param[in]  name    Name of the class to create
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateCustomAttributeClass(ECCustomAttributeClassP& ecClass, Utf8StringCR name);

    //! If the class name is valid, will create an ECRelationshipClass object and add the new class to the schema
    //! @param[out] relationshipClass If successful, will contain a new ECRelationshipClass object
    //! @param[in]  name    Name of the class to create
    //! @param[in]  verify  If true the relationship class will be verified during all in-memory operations. Default is true. 
    //!                     If not verified either the Validate method on the ECSchema or Verify method on the ECRelationshipClass 
    //!                     must be called in order to insure the schema is valid. It is not recommended to set this to false.
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateRelationshipClass(ECRelationshipClassP& relationshipClass, Utf8StringCR name, bool verify = true);

    //! If the class name is valid, will create an ECRelationshipClass object and add the new class to the schema
    //! @param[out] relationshipClass If successful, will contain a new ECRelationshipClass object
    //! @param[in]  name    Name of the class to create
    //! @param[in]  source  The source constraint class
    //! @param[in]  sourceRoleLabel The source role label.  Must not be null or empty.
    //! @param[in]  target  The target constraint class
    //! @param[in]  targetRoleLabel The target role label.  Must not be null or empty.
    //! @return A status code indicating whether or not the class was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateRelationshipClass(ECRelationshipClassP& relationshipClass, Utf8StringCR name, ECEntityClassCR source, Utf8CP sourceRoleLabel, ECEntityClassCR target, Utf8CP targetRoleLabel);

    //! Creates a new KindOfQuantity and adds it to the schema.
    //! @param[out] kindOfQuantity If successful, will contain a new KindOfQuantity object
    //! @param[in] name    Name of the object to create
    //! @return A status code indicating whether or not the object was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateKindOfQuantity(KindOfQuantityP& kindOfQuantity, Utf8CP name);

    //! Creates a new ECEnumeration and adds it to the schema.
    //! @param[out] ecEnumeration If successful, will contain a new ECEnumeration object
    //! @param[in] name    Name of the enumeration to create
    //! @param[in] type    Type for the enumeration to create. Must be integer or string.
    //! @return A status code indicating whether or not the enumeration was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateEnumeration(ECEnumerationP& ecEnumeration, Utf8CP name, PrimitiveType type);

    //! Creates a new PropertyCategory and adds it to the schema.
    //! @param[out] propertyCategory If successful, will contain a new PropertyCategory object
    //! @param[in] name              Name of the propertyCategory to create
    //! @param [in] logError
    //! @return A status code indicating whether or not the property category was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreatePropertyCategory(PropertyCategoryP& propertyCategory, Utf8CP name, bool logError = true);

    //! Creates a new UnitSystem and adds it to the schema.
    //! @param[out] unitSystem If successful, will contain a new UnitSystem object
    //! @param[in] name        Name of the unit system to create 
    //! @param[in] label       Display label of the unit system
    //! @param[in] description Description of the unit system
    //! @return A status code indicating whether or not the Unit Systems was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreateUnitSystem(UnitSystemP& unitSystem, Utf8CP name, Utf8CP label = nullptr, Utf8CP description = nullptr);

    //! Creates a new Phenomenon and adds it to the schema.
    //! @param[out] phenomenon If successful, will contain a new Phenomenon object
    //! @param[in] name        Name of the phenomenon to create
    //! @param[in] defintion   Definition of the phenomenon
    //! @param[in] label       Display label of the phenomenon
    //! @param[in] description Description of the phenomenon
    //! @return A status code indicating whether or not the Phenomenon was successfully created and added to the schema
    ECOBJECTS_EXPORT ECObjectsStatus CreatePhenomenon(PhenomenonP& phenomenon, Utf8CP name, Utf8CP defintion, Utf8CP label = nullptr, Utf8CP description = nullptr);


    //! Creates a new Format and adds it to the schema.
    //! @param[out] format      If successful, will contain a new Format object
    //! @param[in] name         Name of the format to create
    //! @param[in] label        Display label of the format
    //! @param[in] description  Description of the format
    //! @param[in] nfs          A NumericFormatSpec to use to create.
    //! @param[in] composite    A CompositeValueSpec to create this use to create. 
    ECOBJECTS_EXPORT ECObjectsStatus CreateFormat(ECFormatP& format, Utf8CP name, Utf8CP label = nullptr, Utf8CP description = nullptr, Formatting::NumericFormatSpecCP nfs = nullptr, Formatting::CompositeValueSpecCP composite = nullptr);

    //! Creates a new ECUnit and adds it to the schema.
    //! @param[out] unit        If successful, will contain a new ECUnit object
    //! @param[in] name         Name of the unit to create
    //! @param[in] definition   Definition of the unit
    //! @param[in] phenom       Name of the phenomenon this unit is associated with
    //! @param[in] unitSystem   Name of the unit system this unit is associated with
    //! @param[in] numerator    Numerator for unit factor
    //! @param[in] denominator  Denominator for unit factor
    //! @param[in] offset       Offset of this unit
    //! @param[in] label        Display label of the unit
    //! @param[in] description  Description of the unit
    ECOBJECTS_EXPORT ECObjectsStatus CreateUnit(ECUnitP& unit, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, UnitSystemCR unitSystem, Nullable<double> numerator, Nullable<double> denominator, Nullable<double> offset, Utf8CP label = nullptr, Utf8CP description = nullptr);

    //! Creates a new ECUnit and adds it to the schema.
    //! @param[out] unit        If successful, will contain a new ECUnit object
    //! @param[in] name         Name of the unit to create
    //! @param[in] definition   Definition of the unit
    //! @param[in] phenom       Name of the phenomenon this unit is associated with
    //! @param[in] unitSystem   Name of the unit system this unit is associated with
    //! @param[in] label        Display label of the unit
    //! @param[in] description  Description of the unit
    ECObjectsStatus CreateUnit(ECUnitP& unit, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, UnitSystemCR unitSystem, Utf8CP label = nullptr, Utf8CP description = nullptr) 
        {return CreateUnit(unit, name, definition, phenom, unitSystem, nullptr, nullptr, nullptr, label, description);}

    //! Creates a new ECUnit and adds it to the schema.
    //! @param[out] unit        If successful, will contain a new ECUnit object
    //! @param[in] name         Name of the unit to create
    //! @param[in] definition   Definition of the unit
    //! @param[in] phenom       Name of the phenomenon this unit is associated with
    //! @param[in] unitSystem   Name of the unit system this unit is associated with
    //! @param[in] numerator    Numerator for unit factor
    //! @param[in] label        Display label of the unit
    //! @param[in] description  Description of the unit
    ECObjectsStatus CreateUnit(ECUnitP& unit, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, UnitSystemCR unitSystem, Nullable<double> numerator, Utf8CP label = nullptr, Utf8CP description = nullptr) 
        {return CreateUnit(unit, name, definition, phenom, unitSystem, numerator, nullptr, nullptr, label, description);}
    
    //! Creates a new inverted ECUnit and adds it to the schema.
    //! @param[out] unit        If successful, will contain a new inverted ECUnit object
    //! @param[in] parent       Parent unit of this inverted unit
    //! @param[in] name         Name of the unit to create
    //! @param[in] unitSystem   Name of the unit system this unit is associated with
    //! @param[in] label        Display label of the unit
    //! @param[in] description  Description of the unit
    ECOBJECTS_EXPORT ECObjectsStatus CreateInvertedUnit(ECUnitP& unit, ECUnitCR parent, Utf8CP name, UnitSystemCR unitSystem, Utf8CP label = nullptr, Utf8CP description = nullptr);

    //! Creates a new constant ECUnit and adds it to the schema.
    //! @param[out] constant    If successful, will contain a new constant ECUnit object
    //! @param[in] name         Name of the constant to create
    //! @param[in] definition   Definition of the constant
    //! @param[in] phenom       Name of the phenomenon this constant is associated with
    //! @param[in] numerator    Numerator for unit factor
    //! @param[in] denominator  Denominator for unit factor
    //! @param[in] label        Display label of the constant
    //! @param[in] description  Description of the constant
    ECOBJECTS_EXPORT ECObjectsStatus CreateConstant(ECUnitP& constant, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, double numerator, Nullable<double> denominator, Utf8CP label = nullptr, Utf8CP description = nullptr);

    //! Creates a new constant ECUnit and adds it to the schema.
    //! @param[out] constant    If successful, will contain a new constant ECUnit object
    //! @param[in] name         Name of the constant to create
    //! @param[in] definition   Definition of the constant
    //! @param[in] phenom       Name of the phenomenon this constant is associated with
    //! @param[in] numerator    Numerator for unit factor
    //! @param[in] label        Display label of the constant
    //! @param[in] description  Description of the constant
    ECOBJECTS_EXPORT ECObjectsStatus CreateConstant(ECUnitP& constant, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, double numerator, Utf8CP label = nullptr, Utf8CP description = nullptr) 
        {return CreateConstant(constant, name, definition, phenom, numerator, nullptr, label, description);}

    //! Get a schema by alias within the context of this schema and its referenced schemas.
    //! @param[in]  alias   The alias of the schema to lookup in the context of this schema and it's references.
    //!                     Passing an empty alias will return a pointer to the current schema.
    //! @return   A non-refcounted pointer to an ECN::ECSchema if it can be successfully resolved from the specified alias; otherwise, NULL
    ECOBJECTS_EXPORT ECSchemaCP GetSchemaByAliasP(Utf8StringCR alias) const;

    //! Resolve an alias for the specified schema within the context of this schema and its references.
    //! @param[in]  schema     The schema to lookup an alias in the context of this schema and its references.
    //! @param[out] alias      The alias if schema is a referenced schema; empty string if the sechema is the current schema;
    //! @return   Success if the schema is either the current schema or a referenced schema;  ECObjectsStatus::SchemaNotFound if the schema is not found in the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus ResolveAlias(ECSchemaCR schema, Utf8StringR alias) const;

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A const pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, nullptr
    ECClassCP GetClassCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetClassP(name);}

    //! Get a class by name within the context of this schema.
    //! @param[in]  name     The name of the class to lookup.  This must be an unqualified (short) class name.
    //! @return   A pointer to an ECN::ECClass if the named class exists in within the current schema; otherwise, nullptr
    ECClassP GetClassP(Utf8CP name) {return GetSchemaChild<ECClass, ClassMap>(name, &m_classMap);}

    //! Get an ECClass by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECClassCP LookupClass(Utf8CP name, bool useFullName = false) const;

    //! Get an enumeration by name within the context of this schema.
    //! @param[in]  name     The name of the enumeration to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECEnumeration if the named enumeration exists in within the current schema; otherwise, nullptr
    ECEnumerationCP GetEnumerationCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetEnumerationP(name);}

    //! Get an enumeration by name within the context of this schema.
    //! @param[in]  name     The name of the enumeration to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECEnumeration if the named enumeration exists in within the current schema; otherwise, nullptr
    ECEnumerationP GetEnumerationP(Utf8CP name) {return GetSchemaChild<ECEnumeration, EnumerationMap>(name, &m_enumerationMap);}

    //! Get an ECENumeration by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECEnumerationCP LookupEnumeration(Utf8CP name, bool useFullName = false) const;

    //! Get a kind of quantity by name within the context of this schema.
    //! @param[in]  name     The name of the kind of quantity to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::KindOfQuantity if the named kind of quantity exists in within the current schema; otherwise, nullptr
    KindOfQuantityCP GetKindOfQuantityCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetKindOfQuantityP(name);}

    //! Get an kind of quantity by name within the context of this schema.
    //! @param[in]  name     The name of the kind of quantity to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::KindOfQuantity if the named enumeration exists in within the current schema; otherwise, nullptr
    KindOfQuantityP GetKindOfQuantityP(Utf8CP name) {return GetSchemaChild<KindOfQuantity, KindOfQuantityMap>(name, &m_kindOfQuantityMap);}
    
    //! Get a KindOfQuantity by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT KindOfQuantityCP LookupKindOfQuantity(Utf8CP name, bool useFullName = false) const;

    //! Get a property category by name within the context of this schema.
    //! @param[in]  name     The name of the property category to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::PropertyCategory if the named property category exists in within the current schema; otherwise, nullptr
    PropertyCategoryCP GetPropertyCategoryCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetPropertyCategoryP(name);}

    //! Get a property category by name within the context of this schema.
    //! @param[in]  name     The name of the property category to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::PropertyCategory if the named property category exists in within the current schema; otherwise, nullptr
    PropertyCategoryP GetPropertyCategoryP(Utf8CP name) {return GetSchemaChild<PropertyCategory, PropertyCategoryMap>(name, &m_propertyCategoryMap);}

    //! Get a PropertyCategory by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT PropertyCategoryCP LookupPropertyCategory(Utf8CP name, bool useFullName = false) const;

    //! Get a unit system by name within the context of this schema.
    //! @param[in]  name     The name of the unit system to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::UnitSystem if the named unit system exists in within the current schema; otherwise, nullptr
    UnitSystemCP GetUnitSystemCP(Utf8CP name) const {return GetUnitSystemP(name);}

    //! Get a unit system by name within the context of this schema.
    //! @param[in]  name     The name of the unit system to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to an ECN::UnitSystem if the named unit system exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT UnitSystemP GetUnitSystemP(Utf8CP name) const;

    //! Get a UnitSystem by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    UnitSystemCP LookupUnitSystem(Utf8CP name, bool useFullName = false) const {return (UnitSystemCP)GetUnitsContext().LookupUnitSystem(name, useFullName);}

    //! Get a Phenomenon by name within the context of this schema.
    //! @param[in]  name     The name of the phenomenon to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::Phenomenon if the named phenomenon exists in within the current schema; otherwise, nullptr
    PhenomenonCP GetPhenomenonCP(Utf8CP name) const {return GetPhenomenonP(name);}

    //! Get a Phenomenon by name within the context of this schema.
    //! @param[in]  name     The name of the phenomenon to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to an ECN::Phenomenon if the named phenomenon exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT PhenomenonP GetPhenomenonP(Utf8CP name) const;

    //! Get a Phenomenon by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    PhenomenonCP LookupPhenomenon(Utf8CP name, bool useFullName = false) const {return (PhenomenonCP)GetUnitsContext().LookupPhenomenon(name, useFullName);}

    //! Get an ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the unit to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECUnit if the named unit exists in within the current schema; otherwise, nullptr
    ECUnitCP GetUnitCP(Utf8CP name) const {return GetUnitP(name);}

    //! Get an ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the unit to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to an ECN::ECUnit if the named unit exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECUnitP GetUnitP(Utf8CP name) const;

    //! Get an ECUnit by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to the item if the named item exists in within the current schema; otherwise, nullptr
    ECUnitCP LookupUnit(Utf8CP name, bool useFullName = false) const {return (ECUnitCP)GetUnitsContext().LookupUnit(name, useFullName);}

    //! Get an inverted ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the unit to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECUnit if the named unit exists in within the current schema; otherwise, nullptr
    ECUnitCP GetInvertedUnitCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetInvertedUnitP(name);}

    //! Get an inverted ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the unit to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to an ECN::ECUnit if the named unit exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECUnitP GetInvertedUnitP(Utf8CP name);

    //! Get an constant ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the unit to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::ECUnit if the named unit exists in within the current schema; otherwise, nullptr
    ECUnitCP GetConstantCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetConstantP(name);}

    //! Get an constant ECUnit by name within the context of this schema.
    //! @param[in]  name     The name of the constant to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to a ECN::ECUnit if the named constant exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECUnitP GetConstantP(Utf8CP name);

    //! Get a Format by name within the context of this schema.
    //! @param[in]  name     The name of the format to lookup.  This must be an unqualified (short) name.
    //! @return   A const pointer to an ECN::Format if the named format exists in within the current schema; otherwise, nullptr
    ECFormatCP GetFormatCP(Utf8CP name) const {return const_cast<ECSchemaP> (this)->GetFormatP(name);}

    //! Get a Format by name within the context of this schema.
    //! @param[in]  name     The name of the format to lookup.  This must be an unqualified (short) name.
    //! @return   A pointer to an ECN::Format if the named format exists in within the current schema; otherwise, nullptr
    ECFormatP GetFormatP(Utf8CP name) {return GetSchemaChild<ECFormat, FormatMap>(name, &m_formatMap);}

    //! Get a Format by name within the context of this schema and all schemas referenced by this schema.
    //! @param[in]  name        The name of schema item to lookup.  This can be either an qualified or unqualified (short) name (qualified name: [alias]:[item name])
    //! @param[in]  useFullName useFullName If true, name must be fully qualified ([schema name]:[item name]). The lookup will treat the part to the
    //!                         the left of the separator (:) as a schema name and not an alias
    //! @return   A pointer to an ECN::Format if the named format exists in within the current schema; otherwise, nullptr
    ECOBJECTS_EXPORT ECFormatCP LookupFormat(Utf8CP name, bool useFullName = false) const;

    //! Gets the other schemas that are used by classes within this schema.
    //! Referenced schemas are the schemas that contain definitions of base classes,
    //! embedded structures, and custom attributes of classes within this schema.
    ECSchemaReferenceListCR GetReferencedSchemas() const {return m_refSchemaList;}

    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    ECObjectsStatus AddReferencedSchema(ECSchemaR refSchema) {return AddReferencedSchema(refSchema, refSchema.GetAlias());}

    //! Adds an ECSchema as a referenced schema in this schema.
    //! It is necessary to add any ECSchema as a referenced schema that will be used when adding a base
    //! class from a different schema, or custom attributes from a different schema.
    //! @param[in]  refSchema   The schema to add as a referenced schema
    //! @param[in]  alias       The alias to use within the context of this schema for referencing the referenced schema
    ECOBJECTS_EXPORT ECObjectsStatus AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR alias);

    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  refSchema   The schema that should be removed from the list of referenced schemas
    ECOBJECTS_EXPORT ECObjectsStatus RemoveReferencedSchema(ECSchemaR refSchema);

    //! Removes an ECSchema from the list of referenced schemas
    //! @param[in]  schemaKey   The key for the schema that should be removed from the list of referenced schemas.  Version must match according to 'matchType'
    //! @param[in]  matchType   Defines how the version of the input 'schemaKey' is compared to the actual referenced schema version.  Default is ::Exact.
    ECOBJECTS_EXPORT ECObjectsStatus RemoveReferencedSchema(SchemaKeyCR schemaKey, SchemaMatchType matchType = SchemaMatchType::Exact);

    //! Removes any ECSchema from the list of referenced ECSchemas that is not referenced by elements of this schema
    //! @return The number of ECSchemas that were removed
    ECOBJECTS_EXPORT int RemoveUnusedSchemaReferences();

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @param[in]  ecXmlVersion    The version of the ECXml spec to be used for serializing this schema
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus WriteToXmlString(WStringR ecSchemaXml, ECVersion ecXmlVersion = ECVersion::Latest) const;

    //! Serializes an ECXML schema to a string
    //! @param[out] ecSchemaXml     The string containing the Xml of the serialized schema
    //! @param[in]  ecXmlVersion    The version of the ECXml spec to be used for serializing this schema
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml will contain the serialized schema.  Otherwise, ecSchemaXml will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus WriteToXmlString(Utf8StringR ecSchemaXml, ECVersion ecXmlVersion = ECVersion::Latest) const;

    //! Serializes an ECXML schema to a file
    //! @param[in]  ecSchemaXmlFile The absolute path of the file to serialize the schema to
    //! @param[in]  ecXmlVersion    The version of the ECXml spec to be used for serializing this schema
    //! @param[in]  utf16           'false' (the default) to use utf-8 encoding
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the file pointed to by ecSchemaXmlFile will contain the serialized schema.  Otherwise, the file will be unmodified
    ECOBJECTS_EXPORT SchemaWriteStatus WriteToXmlFile(WCharCP ecSchemaXmlFile, ECVersion ecXmlVersion = ECVersion::Latest, bool utf16 = false) const;

    //! Writes a schema to a Json::Value
    //! @param[out] ecSchemaJsonValue Json::Value the schema is serialized to on success.
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the Json value will contain the serialized schema.
    ECOBJECTS_EXPORT bool WriteToJsonValue(Json::Value& ecSchemaJsonValue) const;

    //! Writes a schema as a Json string.
    //! @param[out] ecSchemaJsonString  String the schema is serialized to on success.
    //! @param[in]  minify              If true the Json string will be minified to take up the least number of characters.
    //! If false the Json string will be written in a more human readable format.
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then the string will contain the serialized schema.
    ECOBJECTS_EXPORT bool WriteToJsonString(Utf8StringR ecSchemaJsonString, bool minify = false) const;

    //! Return full schema name in format GetName().RR.ww.mm where Name is the schema name RR is read version, ww is the write compatibility version and mm is minor version.
    Utf8String GetFullSchemaName() const {return m_key.GetFullSchemaName();}

    //! Return a legacy full schema name in format GetName().RR.mm where Name is the schema name RR is read version and mm is minor version.
    //! This overload is missing the  write compatibility version
    Utf8String GetLegacyFullSchemaName() const {return SchemaKey::FormatLegacyFullSchemaName(m_key.GetName().c_str(), m_key.GetVersionRead(), m_key.GetVersionMinor());}

    //! Given a source class, will copy that class into this schema if it does not already exist
    //! @param[out] targetClass If successful, will contain a new ECClass object that is a copy of the sourceClass
    //! @param[in]  sourceClass The class to copy
    //! @remarks    Will not copy references.  Use other overload if you wish to copy things referenced by the sourceClass.
    ECOBJECTS_EXPORT ECObjectsStatus CopyClass(ECClassP& targetClass, ECClassCR sourceClass);

    //! Given a source class, will copy that class into this schema using the targetClassName, if it does not already exist
    //! @param[out] targetClass If successful, will contain a new ECClass object that is a copy of the sourceClass
    //! @param[in]  sourceClass The class to copy
    //! @param[in]  targetClassName Name to use for the copied class (instead of using the source class's name)
    //! @param[in]  copyReferences If true the method will copy types from the source schema into the target schema, if they do not already exist. If false, there will be a schema reference created to the source schema if necessary.
    ECOBJECTS_EXPORT ECObjectsStatus CopyClass(ECClassP& targetClass, ECClassCR sourceClass, Utf8StringCR targetClassName, bool copyReferences = false);

    //! Given a source enumeration, will copy that enumeration into this schema if it does not already exist
    //! @param[out] targetEnumeration If successful, will contain a new ECEnumeration object that is a copy of the sourceEnumeration
    //! @param[in]  sourceEnumeration The enumeration to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyEnumeration(ECEnumerationP& targetEnumeration, ECEnumerationCR sourceEnumeration);

    //! Given a source kind of quantity, will copy that kind of quantity into this schema if it does not already exist
    //! @param[out] targetKOQ If successful, will contain a new KindOfQuantity object that is a copy of the sourceKOQ
    //! @param[in]  sourceKOQ The kind of quantity to copy
    //! @param[in]  copyReferences If true the method will copy types from the source schema into the target schema, if they do not already exist.
    //!             If false, there will be a schema reference created to the source schema if necessary.
    ECOBJECTS_EXPORT ECObjectsStatus CopyKindOfQuantity(KindOfQuantityP& targetKOQ, KindOfQuantityCR sourceKOQ, bool copyReferences = false);

    //! Given a source PropertyCategory, will copy that PropertyCategory into this schema if it does not already exist
    //! @param[out] targetPropCategory If successful, will contain a new PropertyCategory object that is a copy of the sourcePropCategory
    //! @param[in]  sourcePropCategory The PropertyCategory to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyPropertyCategory(PropertyCategoryP& targetPropCategory, PropertyCategoryCR sourcePropCategory);

    //! Given a source UnitSystem, will copy that UnitSystem into this schema if it does not already exist
    //! @param[out] targetUnitSystem If successful, will contain a new UnitSystem object that is a copy of the sourceUnitSystem
    //! @param[in]  sourceUnitSystem The UnitSystem to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyUnitSystem(UnitSystemP& targetUnitSystem, UnitSystemCR sourceUnitSystem);

    //! Given a source Phenomenon, will copy that Phenomenon into this schema if it does not already exist
    //! @param[out] targetPhenom If successful, will contain a new Phenomenon object that is a copy of the sourcePhenom
    //! @param[in]  sourcePhenom The Phenomenon to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyPhenomenon(PhenomenonP& targetPhenom, PhenomenonCR sourcePhenom);

    //! Given a source ECUnit, will copy that ECUnit into this schema if it does not already exist
    //! @param[out] targetUnit If successful, will contain a new ECUnit object that is a copy of the sourceUnit
    //! @param[in]  sourceUnit The ECUnit to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyUnit(ECUnitP& targetUnit, ECUnitCR sourceUnit);

    //! Given a source ECFormat, will copy that ECFormat into this schema if it does not already exist
    //! @param[out] targetFormat If successful, will contain a new ECFormat object that is a copy of the sourceFormat
    //! @param[in]  sourceFormat The ECFormat to copy
    ECOBJECTS_EXPORT ECObjectsStatus CopyFormat(ECFormatP& targetFormat, ECFormatCR sourceFormat);

    //! Copies this schema
    //! @param[out] schemaOut   If successful, will contain a copy of this schema
    ECOBJECTS_EXPORT ECObjectsStatus CopySchema(ECSchemaPtr& schemaOut) const;

    //! Get the IECCustomAttributeContainer holding this schema's custom attributes
    IECCustomAttributeContainer& GetCustomAttributeContainer() {return *this;}
    //! Get the const IECCustomAttributeContainer holding this schema's custom attributes
    IECCustomAttributeContainer const& GetCustomAttributeContainer() const {return *this;}

    // ************************************************************************************************************************
    // ************************************  STATIC METHODS *******************************************************************
    // ************************************************************************************************************************
    //! Given a str containing SchemaXml, will compute the CheckSum
    ECOBJECTS_EXPORT static Utf8String ComputeSchemaXmlStringCheckSum(Utf8CP str, size_t len);

    //! Generate a schema version string given the read, write and minor version values.
    //! @param[in] versionRead     The read version number
    //! @param[in] versionWrite    The write version number
    //! @param[in] versionMinor    The minor version number
    //! @return The version string
    static Utf8String FormatSchemaVersion(uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor) {return SchemaKey::FormatSchemaVersion(versionRead, versionWrite, versionMinor);}

    //! If the given schemaName and alias is valid, this will create a new schema object
    //! @param[out] schemaOut       if successful, will contain a new schema object
    //! @param[in]  schemaName      Name of the schema to be created.
    //! @param[in]  alias           Alias of the schema to be created
    //! @param[in]  versionRead     The read version number.
    //! @param[in]  versionWrite    The write version number
    //! @param[in]  versionMinor    The minor version number.
    //! @param[in]  ecVersion       The EC version of the schema to be created.
    //! @return A status code indicating whether the call was succesful or not
    ECOBJECTS_EXPORT static ECObjectsStatus CreateSchema(ECSchemaPtr& schemaOut, Utf8StringCR schemaName, 
                                                         Utf8StringCR alias, uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor, 
                                                         ECVersion ecVersion = ECVersion::Latest);

    //! Generate a schema version string given the read and minor version values.
    //! @param[in] versionRead    The read version number
    //! @param[in] versionMinor   The minor version number
    //! @return The version string
    static Utf8String FormatSchemaVersion(uint32_t versionRead, uint32_t versionMinor) {return FormatSchemaVersion(versionRead, DEFAULT_VERSION_WRITE, versionMinor);}

    //! Given a schema full name, SchemaName.RR.WW.MM, this will parse into schema name and read, write and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionRead     The read version number
    //! @param[out] versionWrite    The write version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and read, write and minor versions (GetName().RR.WW.MM)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP fullName) {return SchemaKey::ParseSchemaFullName(schemaName, versionRead, versionWrite, versionMinor, fullName);}

    //! Given a schema full name, SchemaName.RR.MM, this will parse into schema name and read and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionRead     The read version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and read and minor versions (GetName().RR.MM)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionRead, uint32_t& versionMinor, Utf8CP fullName)
        {
        uint32_t unwanted = DEFAULT_VERSION_WRITE;
        return ParseSchemaFullName(schemaName, versionRead, unwanted, versionMinor, fullName);
        }

    //! Given a schema full name, SchemaName.RR.MM, this will parse into schema name and read and minor versions
    //! @param[out] schemaName      The schema name without version number qualifiers
    //! @param[out] versionRead     The read version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  fullName        A string containing the schema name and read and minor versions (GetName().RR.MM)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseSchemaFullName(Utf8String& schemaName, uint32_t& versionRead, uint32_t& versionMinor, Utf8StringCR fullName) {return ParseSchemaFullName(schemaName, versionRead, versionMinor, fullName.c_str());}

    //! Given a version string RR.MM, this will parse into read and minor versions
    //! @param[out] versionRead     The read version number
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the read and minor versions (RR.MM)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseVersionString(uint32_t& versionRead, uint32_t& versionMinor, Utf8CP versionString)
        {
        uint32_t unwanted = DEFAULT_VERSION_WRITE;
        return ParseVersionString(versionRead, unwanted, versionMinor, versionString);
        }

    //! Given a version string RR.WW.MM, this will parse into read, write and minor versions
    //! @param[out] versionRead     The read version number
    //! @param[out] versionWrite    The write version number, will default to zero if versionString only has two numbers.
    //! @param[out] versionMinor    The minor version number
    //! @param[in]  versionString   A string containing the read, write (optional) and minor versions (RR.WW.MM)
    //! @return A status code indicating whether the string was successfully parsed
    static ECObjectsStatus ParseVersionString(uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString) { return SchemaKey::ParseVersionString(versionRead, versionWrite, versionMinor, versionString); }

    //! Given two schemas, will check to see if the second schema is referenced by the first schema
    //! @param[in]    thisSchema            The base schema to check the references of
    //! @param[in]    potentiallyReferencedSchema  The schema to search for
    //! @return True if thatSchema is referenced by thisSchema, false otherwise
    ECOBJECTS_EXPORT static bool IsSchemaReferenced (ECSchemaCR thisSchema, ECSchemaCR potentiallyReferencedSchema);

    //! Given a major and minor version number, this will parse them into an ECVersion
    //! @param[out]  ecVersion       The ECVersion to create
    //! @param[in]  ecVersionMajor  The major version number
    //! @param[in]  ecVersionMinor  The minor version number
    //! @return A status code indicating whether the provided major and minor version were successfully used to create an ECVersion.
    ECOBJECTS_EXPORT static ECObjectsStatus CreateECVersion(ECVersion &ecVersion, uint32_t ecVersionMajor, uint32_t ecVersionMinor);

    //! Given an ecVersion it will parse it into a version string M.N
    //! @param[in] ecVersion       The ECVersion to convert to a string
    //! @return The string created from the ecVersion. If fails to convert the given ECVersion it will return nullptr.
    ECOBJECTS_EXPORT static Utf8CP GetECVersionString(ECVersion ecVersion);

    //! Given an ecVersion this will parse it the specific version major and minor
    //! @param[out] ecVersionMajor  The major version of the ECVersion
    //! @param[out] ecVersionMinor  The minor version of the ECVersion
    //! @param[in] ecVersion        The ECVersion to parse
    //! @return A status code indicating whether the ECversion was successfully parsed into the a major and minor version.
    ECOBJECTS_EXPORT static ECObjectsStatus ParseECVersion(uint32_t &ecVersionMajor, uint32_t &ecVersionMinor, ECVersion ecVersion);

    //! Reads an ECSchema from an ECSchemaXML-formatted file
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
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlFile(ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema paths
    //! relative to the executing dll will be searched.
    //! @param[in]    schemaKey           Key describing the schema to be located
    //! @param[in]    schemaContext       Required to create schemas
    ECOBJECTS_EXPORT static ECSchemaPtr LocateSchema(SchemaKeyR schemaKey, ECSchemaReadContextR schemaContext);

    //! Locate a schema using the provided schema locators and paths. If not found in those by either of those parameters standard schema paths
    //! relative to the executing dll will be searched.
    //! @remarks This will attempt to deserialize the provided schema xml file. 
    //! @param[in]    schemaXmlFile       The absolute path of the schema that is being looked for
    //! @param[in]    schemaContext       Required to create schemas
    //! @return A valid ECSchemaPtr, IsValid() will be true, if the schema can be found or deserialized within the provided ECSchemaReadContext
    ECOBJECTS_EXPORT static ECSchemaPtr LocateSchema(WCharCP schemaXmlFile, ECSchemaReadContextR schemaContext);

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
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString(ECSchemaPtr& schemaOut, Utf8CP ecSchemaXml, ECSchemaReadContextR schemaContext);

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
    ECOBJECTS_EXPORT static SchemaReadStatus ReadFromXmlString(ECSchemaPtr& schemaOut, WCharCP ecSchemaXml, ECSchemaReadContextR schemaContext);

    //! Serializes the schema as EC2 Xml with the standard EC3 attributes converted to EC2 standard custom attributes.  
    //! @param[out] ec2SchemaXml        The string containing the EC2 Xml for the input schema
    //! @param[in]  schemaToSerialize   The schema to serialize as EC2 Xml.  See ECSchemaDownConverter for details of conversion.
    //! @return A status code indicating whether the schema was successfully serialized.  If SUCCESS is returned, then ecSchemaXml will contain the serialized schema.  
    //!             Otherwise, ecSchemaXml will be unmodified.  Schema is always unmodified.
    ECOBJECTS_EXPORT static SchemaWriteStatus WriteToEC2XmlString(Utf8StringR ec2SchemaXml, ECSchemaP schemaToSerialize);

    //! Returns true if the schema is an ECStandard schema
    //! @return True if a standard schema, false otherwise
    //! @param[in]  schemaName  Name of the schema to test.  This is just the schema name, no version info
    ECOBJECTS_EXPORT static bool IsStandardSchema(Utf8StringCR schemaName);

    //! Find all ECSchemas in the schema graph, avoiding duplicates and any cycles.
    //! @param[out]   allSchemas            Vector of schemas including rootSchema.
    //! @param[in]    includeRootSchema     If true then root schema is added to the vector of allSchemas. Defaults to true.
    ECOBJECTS_EXPORT void FindAllSchemasInGraph(bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema=true) const;

    //! Find all ECSchemas in the schema graph, avoiding duplicates and any cycles.
    //! @param[out]   allSchemas            Vector of schemas including rootSchema.
    //! @param[in]    includeRootSchema     If true then root schema is added to the vector of allSchemas. Defaults to true.
    void FindAllSchemasInGraph(bvector<ECN::ECSchemaP>& allSchemas, bool includeRootSchema = true) {FindAllSchemasInGraph((bvector<ECN::ECSchemaCP>&)allSchemas, includeRootSchema);}

    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaCP FindSchema(SchemaKeyCR schemaKey, SchemaMatchType matchType) const;

    //! Returns this if the name matches, otherwise searches referenced ECSchemas for one whose name matches schemaName
    ECOBJECTS_EXPORT ECSchemaP FindSchemaP(SchemaKeyCR schemaKey, SchemaMatchType matchType) {return const_cast<ECSchemaP> (FindSchema(schemaKey, matchType));}

    //!Set the schema to be immutable. Immutable schema cannot be modified.
    ECOBJECTS_EXPORT void SetImmutable();
};

//*=================================================================================**//**
//* @bsistruct                                                  Ramanujam.Raman   12/12
//+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IECClassLocater
    {
protected:
    virtual ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) = 0;
    virtual ECClassId _LocateClassId(Utf8CP schemaName, Utf8CP className) 
        { 
        ECClassCP ecClass = LocateClass(schemaName, className);
        if (ecClass == nullptr || !ecClass->HasId())
            return ECClassId(); 

        return ecClass->GetId();
        }

public:
    virtual ~IECClassLocater() {}

    ECClassCP LocateClass(Utf8CP schemaName, Utf8CP className) { return _LocateClass (schemaName, className); }
    ECClassId LocateClassId(Utf8CP schemaName, Utf8CP className) { return _LocateClassId(schemaName, className); }
    };

typedef IECClassLocater& IECClassLocaterR;

//=======================================================================================
// @bsiclass                                                 Gintaras.Volkvicius 09/18
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECClassLocatorByClassId : NonCopyableClass
    {
protected:
    ECSchemaCP m_schema;
    
    virtual ECClassCP LocateClassHelper(ECClassId const& classId) const
        {
        if (nullptr == m_schema)
            return nullptr;

        auto const& ecClasses = m_schema->GetClasses();

        auto const it = std::find_if(std::begin(ecClasses), std::end(ecClasses),
            [&classId] (auto const& ecClass)
                {
                return nullptr != ecClass && ecClass->HasId() && ecClass->GetId() == classId;
                });

        if (std::end(ecClasses) == it)
            return nullptr;

        return *it;
        }
 
public:
    ECClassLocatorByClassId(ECSchemaCP schema = nullptr) : m_schema(schema) {}
    virtual ~ECClassLocatorByClassId() {}

    ECClassCP LocateClass(ECClassId const& classId) const { return LocateClassHelper(classId); }

    };

typedef ECClassLocatorByClassId const& ECClassLocatorByClassIdCR;
typedef ECClassLocatorByClassId const* ECClassLocatorByClassIdCP;

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
