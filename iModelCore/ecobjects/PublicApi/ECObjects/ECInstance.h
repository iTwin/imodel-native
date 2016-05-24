/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/DateTime.h>
#include "ECObjects.h"
#include <Geom/GeomApi.h>

BENTLEY_NAMESPACE_TYPEDEFS (BeXmlDom)
BENTLEY_NAMESPACE_TYPEDEFS (BeXmlNode)
BENTLEY_NAMESPACE_TYPEDEFS (BeXmlWriter)

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @addtogroup ECObjectsGroup
//! ECObjects is a set of abstractions for working with engineering/business data and metadata. 
//! "EC" stands for "Engineering Content".
//! There are several implementations of the ECObjects abstractions:
//! @li In XML (two formats ECSchemaXML and ECInstanceXML)
//! @li This native C++ implementation
//! @li In .NET (multiple implementations of IECInstance, IECClass, and related interfaces.
//!
//! You can think of an ECClass as being like a C++ or .NET class that only defines properties (ECClasses define no methods or behaviors.) In some ways, they are closer to .NET interfaces that hold only properties... or C++ pure virtual abstract base classes that only contain property getters and setters. They are also very analogous to a database table definition.
//!
//! ECClasses contain ECProperties. These are property *definitions*, not values.
//!
//! ECInstances represent instances of objects. Each "belongs" to an ECClass and holds ECPropertyValues. They are somewhat analogous to the rows of a database table.
//!
//! An ECSchema is just a collection of ECClasses.
//!
//! There are also ECRelationshipClasses that are ECClasses that also define "RelationshipConstraints" indicating what ECClasses they relate. ECRelationshipInstances represent the relationships between the ECinstances (defined/constrainted by their ECRelationshipClass) ECRelationships work more like database foreign key constraint that C++ pointers or .NET object references.
//! @beginGroup
//! @see Bentley::EC

//////////////////////////////////////////////////////////////////////////////////
//  The following definitions are used to allow a struct property to generate a
//  custom XML representation of itself. This was required to support 8.11
//  installedTypes as Vancouver ECStructs
//////////////////////////////////////////////////////////////////////////////////
typedef bmap<Utf8String, ICustomECStructSerializerP> NameSerializerMap;

//////////////////////////////////////////////////////////////////////////////////
//  The following definitions are used to allow a attribute property to generate a
//  instance from  XML This was required to support Units in EC3.0 
//////////////////////////////////////////////////////////////////////////////////
typedef bmap<Utf8String, ICustomAttributeDeserializerP> AttributeDeserializerMap;

//////////////////////////////////////////////////////////////////////////////////
//  Typedef for a list of values
//////////////////////////////////////////////////////////////////////////////////
typedef bvector<ECValue> ValueList;

//! Interface for a custom ECStruct Serializer.  Implement this class if you need to allow a struct property
//! to generate a custom XML representation of itself.
struct ICustomECStructSerializer
    {
    //! Returns whether the given property uses a custom xml string
    //! @param[in] structProperty   The property to check against
    //! @param[in] ecInstance   The instance the property comes from
    //! @returns True if this property uses a custom xml string
    virtual bool            UsesCustomStructXmlString  (StructECPropertyP structProperty, IECInstanceCR ecInstance) const = 0; 

    //! Generates a custom XML representation of the property
    //! @param[out] xmlString   A string with the Xml representing the serialized struct
    //! @param[in] structProperty   The property to serialize
    //! @param[in] ecInstance   The instance the property comes from
    //! @param[in] baseAccessString The access string used to get the struct property value
    //! @returns SUCCESS if the serialization was successful, otherwise an error code indicating the type of failure
    virtual ECObjectsStatus GenerateXmlString  (Utf8String& xmlString, StructECPropertyP structProperty, IECInstanceCR ecInstance, Utf8CP baseAccessString) const = 0;

    //! Given an Xml string, deserializes the StructProperty and sets the value on the instance
    //! @param[in] structProperty   The StructECProperty that the Xml string describes
    //! @param[in] ecInstance   The instance upon which to set the de-serialized value
    //! @param[in] baseAccessString The access string used to get the struct property value
    //! @param[in] valueString  The Xml contained the serialized struct information
    virtual void            LoadStructureFromString (StructECPropertyP structProperty, IECInstanceR ecInstance, Utf8CP baseAccessString, Utf8CP valueString) = 0;
    };

//! Used to manage multiple custom struct serializers
struct CustomStructSerializerManager
{
private:
    NameSerializerMap   m_serializers;

    CustomStructSerializerManager();
    ~CustomStructSerializerManager();

    ICustomECStructSerializerP GetCustomSerializer (Utf8CP serializerName) const;
public:

    //! Given a struct property and an instance, returns the ICustomECStructSerializer
    //! @param[in] structProperty   The StructECProperty that we want the serializer for
    //! @param[in] ecInstance   The instance that the struct property comes from
    //! @returns An ICustomECStructSerializer pointer if one has been added for this particular struct property, NULL otherwise
    ECOBJECTS_EXPORT  ICustomECStructSerializerP            GetCustomSerializer (StructECPropertyP structProperty, IECInstanceCR ecInstance) const;

    //! Returns the static CustomStructSerializerManager
    ECOBJECTS_EXPORT  static CustomStructSerializerManagerR GetManager(); 

    //! Adds a CustomStructSerializer to the manager with the given serializer name
    //! @param[in] serializerName   Name of the custom serializer.  
    //! @param[in] serializer   The ICustomECStructSerializer to add
    ECOBJECTS_EXPORT  BentleyStatus                         AddCustomSerializer (Utf8CP serializerName, ICustomECStructSerializerP serializer);
};

typedef RefCountedPtr<IECInstance> IECInstancePtr;

//! Interface for a custom ECSCustomAttribute Deserializer.
struct ICustomAttributeDeserializer
    {
    //! Given an Xml string, deserializes the CustomAttribute and sets the value on the instance
    //@remarks set the instance to null if it should not be instantiated as customattribute
    virtual InstanceReadStatus            LoadCustomAttributeFromString (IECInstancePtr& ecInstance, BeXmlNodeR xmlNode, ECInstanceReadContextR context, ECSchemaReadContextR schemaContext, IECCustomAttributeContainerR customAttributeContainer) = 0;
    };

//! Used to manage multiple custom attribute deserializers
struct CustomAttributeDeserializerManager
    {
    private:
        AttributeDeserializerMap   m_deserializers;

        CustomAttributeDeserializerManager ();
        ~CustomAttributeDeserializerManager ();

    public:

        //! Given an deserializerName and an instance, returns the ICustomAttributeDeserializer
        //! @param[in] deserializerName   name of the deserializer
        //! @returns An ICustomAttributeDeserializer pointer if one has been added for this particular deserializerName, NULL otherwise
        ECOBJECTS_EXPORT  ICustomAttributeDeserializerP GetCustomDeserializer (Utf8CP deserializerName) const;

        //! Returns the static CustomStructDeserializerManager
        ECOBJECTS_EXPORT  static CustomAttributeDeserializerManagerR GetManager ();

        //! Adds a CustomAttributeDeserializer to the manager with the given serializer name
        //! @param[in] deserializerName   Name of the custom deserializer.  
        //! @param[in] deserializer   The ICustomAttributeDeserializer to add
        ECOBJECTS_EXPORT  BentleyStatus                         AddCustomDeserializer (Utf8CP deserializerName, ICustomAttributeDeserializerP deserializer);
    };

//=======================================================================================
//! An IECInstance represents an instance of an ECClass.
//!
//! IECInstance is not a pure interface. Functionality is supplied to an IECInstance through
//! the implementation of an ECEnabler for the IECInstance.
//! @see ECEnabler
//!
//! @if BENTLEY_SDK_Internal
//! ### Comparison to .NET ECObjects
//! ECN::IECInstance is the native equivalent of a .NET IECInstance.
//! In .NET %IECInstance is a pure interface. One might implement %IECInstance, 
//! or use the "Lightweight" system in Bentley.ECObjects.Lightweight.
//! Native IECInstances could be called "enabled" as opposed to "lightweight".
//! @endif
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECInstance : RefCountedBase
    {
private:
    ECObjectsStatus ChangeValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex);
    ECObjectsStatus GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const;

    bool                        GetInstanceLabelPropertyName (Utf8String& propertyName) const;

    //! If the property is a DateTime property looks up the DateTimeInfo custom attribute and, if present,
    //! validates whether the DateTime metadata of the input ECValue matches the DateTimeInfo custom attribute
    //! information.
    //! @param[in] propertyIndex Index of property to validate against (index is 1-based)
    //! @param[in] v ECValue to validate
    //! @return ECOBJECT_STATUS_Success if the validation was successful. ECObjectsStatus::DataTypeMismatch otherwise
    ECObjectsStatus ValidateDateTimeMetadata (uint32_t propertyIndex, ECValueCR v) const;
    //! If the property is a DateTime property looks up the DateTimeInfo custom attribute and, if present,
    //! applies the DateTime metadata to the given ECValue.
    //! @remarks The metadata is used to build the DateTime object when the client calls ECValue::GetDateTime.
    //! @param[in] v ECValue to apply metadata to
    //! @param[in] propertyIndex Index of property to retrieve metadata from (index is 1-based)
    //! @return ECOBJECT_STATUS_Success if successful. ECObjectsStatus::DataTypeMismatch if the 
    ECObjectsStatus SetDateTimeMetadataInECValue (ECValueR v, uint32_t propertyIndex) const;
    ECObjectsStatus GetDateTimeInfo (DateTimeInfoR dateTimeInfo, uint32_t propertyIndex) const;

protected:
    ECOBJECTS_EXPORT IECInstance();
    ECOBJECTS_EXPORT virtual ~IECInstance();

    //! Gets the unique ID for this instance
    virtual Utf8String             _GetInstanceId() const = 0; // Virtual and returning WString because a subclass may want to calculate it on demand
    //! Gets the ID which should be written to ECInstance XML for this instance
    virtual Utf8String             _GetInstanceIdForSerialization() const { return GetInstanceId(); }

    //! Gets the value stored in the ECProperty referred to by the specified property index
    //! @param[out] v               If successful, will contain the value of the property
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (property index is 1-based)
    //! @param[in]  useArrayIndex   true, if the respective property is an array property. false otherwise. If true, \p arrayIndex
    //!                             indicates the index in the array
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (array index is 0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const = 0;

    //! Sets the value for the property referred to by the specified property index
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (property index is 1-based)
    //! @param[in]  v               The value to set onto the property
    //! @param[in]  useArrayIndex   true, if the respective property is an array property. false otherwise. If true, \p arrayIndex
    //!                             indicates the index in the array
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (array index is 0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) = 0;

    //! Given a propertyIndex, will insert size number of empty array elements at the given index
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve
    //! @param[in]  index           The starting index of the array at which to insert the new elements
    //! @param[in]  size            The number of empty array elements to insert
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size) = 0;

    //! Given a property index, will add size number of empty array elements to the end of the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] size The number of empty array elements to add
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _AddArrayElements (uint32_t propertyIndex, uint32_t size) = 0;

    //! Given a property index and an array index, will remove a single array element
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] index    The index of the element to remove
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _RemoveArrayElement (uint32_t propertyIndex, uint32_t index) = 0;

    //! Given a property index, removes all array elements from the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    virtual ECObjectsStatus     _ClearArray (uint32_t propertyIndex) = 0;    
    //! Returns a const reference to the ECEnabler that supports this instance
    virtual ECEnablerCR         _GetEnabler() const = 0;
    //! Returns whether the ECInstance as a whole is ReadOnly
    virtual bool                _IsReadOnly() const = 0;
    //! Returns a dump of the instance
    //! @param[in] indent   String to prepend to each line of the dump
    virtual Utf8String          _ToString (Utf8CP indent) const = 0;
    //! Returns the offset to the IECInstance
    virtual size_t              _GetOffsetToIECInstance () const = 0;

    //! Check for Null property value
    //! @param[out] isNull          If successful, will contain true if property value is NULL.
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve
    //! @param[in]  useArrayIndex   Flag indicating whether an array index should be used
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const;

    //! Sets the unique id for this instance
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _SetInstanceId(Utf8CP);
    //! Returns the display label for the instance
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _GetDisplayLabel (Utf8String& displayLabel) const;    
    //! Sets the display label for this instance
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _SetDisplayLabel (Utf8CP displayLabel);    
    //! Returns the instance as a MemoryECInstance
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const;
    //! Returns the underlying ECDBuffer for this instance
    ECOBJECTS_EXPORT virtual ECDBuffer*            _GetECDBuffer() const;

    //! Given an access string, returns whether that property is readonly
    //! @param[in] accessString The access string to the property to check the read-only state for
    //! @returns true if the property is read-only
    //! @remarks If you override one of these IsPropertyReadOnly methods, you should override the other.
    ECOBJECTS_EXPORT virtual bool                   _IsPropertyReadOnly (Utf8CP accessString) const;

    //! Given the propertyIndex (into the ClassLayout) of a property, returns whether that property is readonly
    //! @param[in] propertyIndex    Index into the ClassLayout indicating which property to check
    //! @returns true if the property is read-only
    //! @remarks If you override one of these IsPropertyReadOnly methods, you should override the other.
    ECOBJECTS_EXPORT virtual bool                   _IsPropertyReadOnly (uint32_t propertyIndex) const;

    //! Sets the value for the specified property
    //! @param[in]  propertyIndex   Index into the PropertyLayout indicating which property to retrieve
    //! @param[in]  v               The value to set onto the property
    //! @param[in]  useArrayIndex   Flag indicating whether an array index should be used
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    //! @remarks For when the caller wants to directly set the InternalValue, and not go through any processing, for example with calculated properties
    ECOBJECTS_EXPORT virtual ECObjectsStatus        _SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex);

/*__PUBLISH_SECTION_END__*/
#ifdef DGN_IMPORTER_REORG_WIP
    virtual DgnPlatform::DgnECInstance const*              _GetAsDgnECInstance() const   { return NULL; }
#endif
/*__PUBLISH_SECTION_START__*/
    //! Allow each instance type to determine if it want to only serialize "loaded" properties to XML
    virtual bool              _SaveOnlyLoadedPropertiesToXml() const   { return false; }

    //! Returns true if callers are permitted to modify values of this IECInstance in memory. This may differ from the return value of _IsReadOnly(), which
    //! returns true if the IECInstance is permitted to be modified persistently.
    virtual bool            _ChangeValuesAllowed() const { return ! IsReadOnly(); }
public:
    //! Returns true if callers are permitted to modify values of this IECInstance in memory. This may differ from the return value of IsReadOnly(), which
    //! returns true if the IECInstance is permitted to be modified persistently.
    bool    ChangeValuesAllowed()  { return _ChangeValuesAllowed(); }

    //! Returns the base address for this instance
    ECOBJECTS_EXPORT void const*        GetBaseAddress () {return this;}

    //! Returns a const reference to the ECEnabler that supports this instance
    ECOBJECTS_EXPORT ECEnablerCR        GetEnabler() const;

    //! Returns a reference to the ECEnabler that supports this instance
    ECOBJECTS_EXPORT ECEnablerR         GetEnablerR() const;      // use when enabler.ObtainStandaloneEnabler is called since a new enabler may be created.

    //! Gets the unique ID for this instance
    ECOBJECTS_EXPORT Utf8String         GetInstanceId() const;
    //! Gets the ID which should be serialized to ECInstance XML for this instance
    ECOBJECTS_EXPORT Utf8String         GetInstanceIdForSerialization() const;
    //! Sets the unique id for this instance
    ECOBJECTS_EXPORT ECObjectsStatus    SetInstanceId(Utf8CP instanceId);
    //! Returns whether the ECInstance as a whole is ReadOnly
    ECOBJECTS_EXPORT bool               IsReadOnly() const;
    //! Returns the ECClass that defines this ECInstance
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, Utf8CP propertyAccessString) const;
//__PUBLISH_SECTION_END__
    //! Gets the value stored in the specified ECProperty, optionally including ad-hoc property values. If propertyAccessString does not identify an ECProperty of the ECClass, attempts to locate an ad-hoc property with the specified access string
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValueOrAdhoc (ECValueR v, Utf8CP propertyAccessString) const;    
//__PUBLISH_SECTION_START__
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v                       If successful, will contain the value of the property
    //! @param[in]  propertyAccessString    Name of the property to retrieve
    //! @param[in]  arrayIndex              The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, Utf8CP propertyAccessString, uint32_t arrayIndex) const;
    //! Gets the value stored in the ECProperty referred to by the specified property index
    //! @note The property index is 1-based!
    //! @param[out] v               If successful, will contain the value of the property
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, uint32_t propertyIndex) const;
    //! Gets the value stored in the specified ECProperty
    //! @note The property index is 1-based!
    //! @param[out] v               If successful, will contain the value of the property
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, uint32_t propertyIndex, uint32_t arrayIndex) const;
    //! Sets the value for the specified property
    //! @param[in]  propertyAccessString The name of the property to set the value of
    //! @param[in]  v                    The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (Utf8CP propertyAccessString, ECValueCR v);
//__PUBLISH_SECTION_END__
    //! Sets the value for the specified property. If propertyAccessString does not identify an ECProperty of the ECClass, attempts to locate an ad-hoc property with the specified access string
    //! @param[in]  propertyAccessString The name of the property to set the value of
    //! @param[in]  v                    The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValueOrAdhoc (Utf8CP propertyAccessString, ECValueCR v);    
//__PUBLISH_SECTION_START__
    //! Sets the value for the specified property
    //! @param[in]  propertyAccessString The name of the property to set the value of
    //! @param[in]  v                    The value to set onto the property
    //! @param[in]  arrayIndex           The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex);
    //! Sets the value for the specified property
    //! @note The property index is 1-based!
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  v               The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (uint32_t propertyIndex, ECValueCR v);
    //! Sets the value for the specified property
    //! @note The property index is 1-based!
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  v               The value to set onto the property
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex);

    //! Change the value for the specified property
    //! @param[in]  propertyAccessString    The name of the property to set the value of
    //! @param[in]  v                       The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ChangeValue (Utf8CP propertyAccessString, ECValueCR v);
//__PUBLISH_SECTION_END__
    //! Change the value for the specified property. If propertyAccessString does not identify an ECProperty of the ECClass, attempts to locate an ad-hoc property with the specified access string
    //! @param[in]  propertyAccessString    The name of the property to set the value of
    //! @param[in]  v                       The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ChangeValueOrAdhoc (Utf8CP propertyAccessString, ECValueCR v);    
//__PUBLISH_SECTION_START__
    //! Change the value for the specified property
    //! @param[in]  propertyAccessString    The name of the property to set the value of
    //! @param[in]  v                       The value to set onto the property
    //! @param[in]  arrayIndex              The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, ECObjectsStatus::PropertyValueMatchesNoChange if no change, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ChangeValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex);
    //! Change the value for the specified property
    //! @note The property index is 1-based!
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  v               The value to set onto the property
    //! @returns ECObjectsStatus::Success if successful, ECObjectsStatus::PropertyValueMatchesNoChange if no change, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ChangeValue (uint32_t propertyIndex, ECValueCR v);
    //! Change the value for the specified property
    //! @note The property index is 1-based!
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  v               The value to set onto the property
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, ECObjectsStatus::PropertyValueMatchesNoChange if no change, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ChangeValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex);

    //! Gets the value of the ECProperty specified by the ECValueAccessor
    ECOBJECTS_EXPORT ECObjectsStatus    GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const;
    //! Sets the value of the ECProperty specified by the ECValueAccessor
    ECOBJECTS_EXPORT ECObjectsStatus    SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR v);

    //! Check is the NullFlags for the Null setting of a specific value without read the actual value
    //! Check for Null property value
    //! @param[out] isNull                  If successful, will contain true if property value is NULL.
    //! @param[in]  propertyAccessString    Name of the property to retrieve
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, Utf8CP propertyAccessString) const;

    //! Check for Null property value
    //! @param[out] isNull                  If successful, will contain true if property value is NULL.
    //! @param[in]  propertyAccessString    Name of the property to retrieve
    //! @param[in]  arrayIndex              The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, Utf8CP propertyAccessString, uint32_t arrayIndex) const;

    //! Check for Null property value
    //! @note The property index is 1-based!
    //! @param[out] isNull          If successful, will contain true if property value is NULL.
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, uint32_t propertyIndex) const;

    //! Check for Null property value
    //! @note The property index is 1-based!
    //! @param[out] isNull   If successful, will contain true if property value is NULL.
    //! @param[in]  propertyIndex   Index into the ClassLayout indicating which property to retrieve (1-based)
    //! @param[in]  arrayIndex      The array index, if this is an ArrayProperty (0-based)
    //! @returns ECObjectsStatus::Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, uint32_t propertyIndex, uint32_t arrayIndex) const;

    ECOBJECTS_EXPORT ECObjectsStatus    GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex);
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (uint32_t propertyIndex, ECValueCR v);
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR valueToSet);
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (Utf8CP propertyAccessString, ECValueCR v);
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex);

    // These are provided to avoid the cost of dynamic cast.
#ifdef DGN_IMPORTER_REORG_WIP
    ECOBJECTS_EXPORT DgnPlatform::DgnECInstance const* AsDgnECInstanceCP() const;
    ECOBJECTS_EXPORT DgnPlatform::DgnECInstance*       AsDgnECInstanceP();
#endif
    ECOBJECTS_EXPORT InstanceWriteStatus    WriteToBeXmlDom (BeXmlWriterR xmlWriter, bool writeInstanceId);

    // Copy any properties which are common to both IECInstances, skip the rest.
    ECOBJECTS_EXPORT ECObjectsStatus    CopyCommonValues (ECN::IECInstanceCR source);
    //! Attempts to copy all property values from one instance to this instance.
    //! It is expected that the source instance is of the same class as this instance.
    //! @param[in] source   Instance to copy values from.
    //! @returns ECObjectsStatus::Success if values were successfully copied.
    ECOBJECTS_EXPORT ECObjectsStatus    CopyValues(ECN::IECInstanceCR source);

    //! Check property to see it is a fixed size array and optionally return the fixed size.
    //! @param[in]  instance Instance to process
    //! @param[in]  accessString   The access string for the array
    //! @param[out] numFixedEntries   Optional pointer to size of fixed size array.
    //! @returns true if the property is a fixed size array.
    ECOBJECTS_EXPORT static bool        IsFixedArrayProperty (ECN::IECInstanceR instance, Utf8CP accessString, uint32_t* numFixedEntries=NULL);

    //! Given an access string, returns whether that property is readonly
    //! @param[in] accessString The access string to the property to check the read-only state for
    //! @returns true if the property is read-only
    ECOBJECTS_EXPORT bool               IsPropertyReadOnly (Utf8CP accessString) const;
//__PUBLISH_SECTION_END__
    //! Given an access string, returns whether that property is readonly. If propertyAccessString does not identify an ECProperty of the ECClass, attempts to locate an ad-hoc property with the specified access string
    //! @param[in] accessString The access string to the property to check the read-only state for
    //! @returns true if the property is read-only
    ECOBJECTS_EXPORT bool               IsPropertyOrAdhocReadOnly (Utf8CP accessString) const;
//__PUBLISH_SECTION_START__

    //! Given the propertyIndex (into the ClassLayout) of a property, returns whether that property is readonly
    //! @param[in] propertyIndex    Index into the ClassLayout indicating which property to check
    ECOBJECTS_EXPORT bool               IsPropertyReadOnly (uint32_t propertyIndex) const;

    //! Contract:
    //! - For all of the methods, the propertyAccessString shall not contain brackets.
    //!   e.g. "Aliases", not "Aliases[]" or "Aliases[0]"
    //! Given an access string, will inserting size number of empty array elements at the given index
    //! @param[in] propertyAccessString     The access string to the array property (shall not contain brackets)
    //! @param[in] index                    The starting index of the array at which to insert the new elements
    //! @param[in] size                     The number of empty array elements to insert
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    InsertArrayElements (Utf8CP propertyAccessString, uint32_t index, uint32_t size);

    //! Given an access string and an array index, will remove a single array element
    //! @param[in] propertyAccessString     The access string to the array property (shall not contain brackets)
    //! @param[in] index                    The index of the element to remove
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    RemoveArrayElement (Utf8CP propertyAccessString, uint32_t index);

    //! Given an access string, will add size number of empty array elements to the end of the array
    //! @param[in] propertyAccessString The access string to the array property (shall not contain brackets)
    //! @param[in] size The number of empty array elements to add
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    AddArrayElements (Utf8CP propertyAccessString, uint32_t size);

    //! Given an access string, removes all array elements from the array
    //! @param[in] propertyAccessString The access string to the array property (shall not contain brackets)
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ClearArray (Utf8CP propertyAccessString);    

    //! Given a property index, will inserting size number of empty array elements at the given index
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] index    The starting index of the array at which to insert the new elements
    //! @param[in] size The number of empty array elements to insert
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size);

    //! Given a property index and an array index, will remove a single array element
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] index    The index of the element to remove
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    RemoveArrayElement (uint32_t propertyIndex, uint32_t index);

    //! Given a property index, will add size number of empty array elements to the end of the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] size The number of empty array elements to add
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    AddArrayElements (uint32_t propertyIndex, uint32_t size);

    //! Given a property index, removes all array elements from the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    ClearArray (uint32_t propertyIndex);
    
    //! Returns the display label for the instance.  The following are checked (in order) for the label:
    //! @li InstanceLabelSpecification custom attribute set on the instance itself
    //! @li InstanceLabelSpecification custom attribute set on base classes
    //! @li An ECProperty with one of the following propertyNames: "DisplayLabel", "DISPLAYLABEL", "displaylabel", "Name", "NAME", "name"
    //! @param[out] displayLabel    The display label for this instance
    //! @returns SUCCESS if the display label was found, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetDisplayLabel (Utf8String& displayLabel) const;    

    //! Sets the display label for this instance
    //! @param[in]  displayLabel    The display label
    //! @returns SUCCESS if the display label was set, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel (Utf8CP displayLabel);    

    ECOBJECTS_EXPORT ECDBuffer const*               GetECDBuffer() const; //!< Returns the underlying ECDBuffer for this instance.
    ECOBJECTS_EXPORT ECDBuffer*                     GetECDBufferP(); //!< Returns the underlying ECDBuffer for this instance.
    ECOBJECTS_EXPORT MemoryECInstanceBase const*    GetAsMemoryECInstance () const; //!< Returns the instance as a MemoryECInstance.
    ECOBJECTS_EXPORT MemoryECInstanceBase*          GetAsMemoryECInstanceP(); //!< Returns the instance as a MemoryECInstance.
    ECOBJECTS_EXPORT size_t                GetOffsetToIECInstance () const; //!< Returns the offset to the IECInstance

    //! Returns a dump of the instance
    //! @param[in] indent   String to prepend to each line of the dump
    ECOBJECTS_EXPORT Utf8String            ToString (Utf8CP indent) const;

    //! Creates a copy of this instance by serializing and then deserializing
    ECOBJECTS_EXPORT IECInstancePtr     CreateCopyThroughSerialization();

    //! Given an xml file and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] fileName The name of the file contained the serialized Xml
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlFile   (IECInstancePtr& ecInstance, WCharCP fileName,   ECInstanceReadContextR context);

    //! Given a stream containing Xml and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] stream The stream containing the xml
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlStream (IECInstancePtr& ecInstance, IStreamP stream,    ECInstanceReadContextR context);

    //! Given a string containing Xml and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] xmlString The string containing the xml
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlString (IECInstancePtr& ecInstance, WCharCP xmlString,  ECInstanceReadContextR context);

    //! Given a string containing Xml and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] xmlString The string containing the xml
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlString (IECInstancePtr& ecInstance, Utf8CP xmlString,   ECInstanceReadContextR context);

    //! Given a BeXmlDom and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] xmlNode The BeXmlDom that contains a single serialized instance
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromBeXmlDom  (IECInstancePtr& ecInstance, BeXmlDomR xmlNode,  ECInstanceReadContextR context);

    //! Given a BeXmlNode and an instance read context, deserializes and constructs an IECInstance
    //! @param[out] ecInstance  The instance constructed from deserializing the xml file
    //! @param[in] xmlNode The BeXmlNode from an xml document that contains a single serialized instance
    //! @param[in] context  The ECInstanceReadContext which is used to deserialize the xml (for locating schemas and resolving references)
    //! @returns SUCCESS if the instance is successfully deserialized, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromBeXmlNode (IECInstancePtr& ecInstance, BeXmlNodeR xmlNode, ECInstanceReadContextR context);

    //! Serializes the instance to a file
    //! @param[in] fileName Full path to the file that will be written to.
    //! @param[in] writeInstanceId  If true, the instanceId will be written as an attribute on the node
    //! @param[in] utf16    If true, the Xml will be written as utf16
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToXmlFile   (WCharCP fileName, bool writeInstanceId, bool utf16);

    //! Serializes the instance to a stream
    //! @param[in] stream   The stream to write to
    //! @param[in] isStandAlone If true, the Xml will start with the Xml declaration.  Otherwise, if this is part of a larger Xml stream, no declaration will be written
    //! @param[in] writeInstanceId  If true, the instanceId will be written as an attribute on the node
    //! @param[in] utf16    If true, the Xml will be written as utf16
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToXmlStream (IStreamP stream, bool isStandAlone, bool writeInstanceId, bool utf16);

    //! Serializes the instance to a Utf8 string
    //! @param[out] ecInstanceXml   The string to write to
    //! @param[in] isStandAlone If true, the Xml will start with the Xml declaration.  Otherwise, if this is part of a larger Xml stream, no declaration will be written
    //! @param[in] writeInstanceId  If true, the instanceId will be written as an attribute on the node
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToXmlString (Utf8String & ecInstanceXml, bool isStandAlone, bool writeInstanceId);

    //! Serializes the instance to a WString
    //! @param[out] ecInstanceXml   The string to write to
    //! @param[in] isStandAlone If true, the Xml will start with the Xml declaration.  Otherwise, if this is part of a larger Xml stream, no declaration will be written
    //! @param[in] writeInstanceId  If true, the instanceId will be written as an attribute on the node
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToXmlString (WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId);

    //! Serializes the instance to an existing BeXmlWriter
    //! @param[in] xmlWriter The writer to write to.  It should be at the current point where to insert the instance
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToBeXmlNode (BeXmlWriterR xmlWriter);

    //! Serializes the instance to an existing BeXmlWriter
    //! @param[in] xmlWriter The writer to write to.  It should be at the current point where to insert the instance
    //! @param[in] className The overriding class name while serializing a schema
    //! @returns SUCCESS if the instance was successfully written, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToBeXmlNode(BeXmlWriterR xmlWriter, Utf8CP className);

    //! Allow each instance type to determine if it want to only serialize "loaded" properties to XML.  If the instance
    //! returns true then the instance insures the ECValue returned for a property will properly set the "IsLoaded" flag in the ECValue.
    ECOBJECTS_EXPORT bool                       SaveOnlyLoadedPropertiesToXml() const;
    };

//=======================================================================================
//! ECN::IECRelationshipInstance is an instance of an ECRelationshipClass and represents
//! the relationship between two \ref ECN::IECInstance "IECInstances"
//! @see IECInstance, ECRelationshipClass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECRelationshipInstance : virtual IECInstance
    {
    private:
        virtual void            _SetSource (IECInstanceP instance) = 0;
        virtual IECInstancePtr  _GetSource () const = 0;
        virtual ECObjectsStatus _GetSourceOrderId (int64_t& sourceOrderId) const = 0;
        virtual void            _SetTarget (IECInstanceP instance)= 0;
        virtual IECInstancePtr  _GetTarget () const = 0;
        virtual ECObjectsStatus _GetTargetOrderId (int64_t& targetOrderId) const = 0;
        DECLARE_KEY_METHOD

    public:

        //! Sets the Source instance of the relationship
        //! @param[in] instance The source instance
        ECOBJECTS_EXPORT void            SetSource (IECInstanceP instance);

        //! Gets the source instance of the relationship
        ECOBJECTS_EXPORT IECInstancePtr  GetSource () const; 

        //! Gets the source order id
        //! @param[out] sourceOrderId Contains the orderId of the source instance
        ECOBJECTS_EXPORT ECObjectsStatus GetSourceOrderId (int64_t& sourceOrderId) const;

        //! Sets the Target instance of the relationship
        //! @param[in] instance The target instance
        ECOBJECTS_EXPORT void            SetTarget (IECInstanceP instance);

        //! Gets the target instance of the relationship
        ECOBJECTS_EXPORT IECInstancePtr  GetTarget () const;

        //! Gets the target order id
        //! @param[out] targetOrderId Contains the orderId of the target instance
        ECOBJECTS_EXPORT ECObjectsStatus GetTargetOrderId (int64_t& targetOrderId) const;
    };

typedef RefCountedPtr<IECRelationshipInstance> IECRelationshipInstancePtr;

struct ECStructArrayMemberAccessor;

struct ECInstanceInteropHelper
    {
    // These are not convenience methods.  They are intended for managed callers.  They enable
    // an access pattern that can get a value with only one managed to native transition
    ECOBJECTS_EXPORT static ECObjectsStatus GetValue         (IECInstanceCR, ECValueR value,    Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,       Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, int64_t & value,   Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,    Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, Utf8StringR value, Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetBoolean       (IECInstanceCR, bool & value,      Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint2D       (IECInstanceCR, DPoint2dR value,   Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint3D       (IECInstanceCR, DPoint3dR value,   Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTime      (IECInstanceCR, DateTimeR value,   Utf8CP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeTicks (IECInstanceCR, int64_t & value,   Utf8CP managedPropertyAccessor);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValue         (IECInstanceR, Utf8CP managedPropertyAccessor, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetLongValue     (IECInstanceR, Utf8CP managedPropertyAccessor, int64_t value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetIntegerValue  (IECInstanceR, Utf8CP managedPropertyAccessor, int value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetStringValue   (IECInstanceR, Utf8CP managedPropertyAccessor, Utf8CP value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDoubleValue   (IECInstanceR, Utf8CP managedPropertyAccessor, double value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetBooleanValue  (IECInstanceR, Utf8CP managedPropertyAccessor, bool value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint2DValue  (IECInstanceR, Utf8CP managedPropertyAccessor, DPoint2dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint3DValue  (IECInstanceR, Utf8CP managedPropertyAccessor, DPoint3dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeValue (IECInstanceR, Utf8CP managedPropertyAccessor, DateTimeCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeTicks (IECInstanceR, Utf8CP managedPropertyAccessor, int64_t value);

    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,       ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, int64_t & value,   ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,    ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, Utf8StringR value, ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetBoolean       (IECInstanceCR, bool & value,      ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint2D       (IECInstanceCR, DPoint2dR value,   ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint3D       (IECInstanceCR, DPoint3dR value,   ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTime      (IECInstanceCR, DateTimeR value,   ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeTicks (IECInstanceCR, int64_t & value,   ECValueAccessorCR accessor);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValue         (IECInstanceR, ECValueAccessorCR accessor, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetLongValue     (IECInstanceR, ECValueAccessorCR accessor, int64_t value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetIntegerValue  (IECInstanceR, ECValueAccessorCR accessor, int value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetStringValue   (IECInstanceR, ECValueAccessorCR accessor, Utf8CP value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDoubleValue   (IECInstanceR, ECValueAccessorCR accessor, double value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetBooleanValue  (IECInstanceR, ECValueAccessorCR accessor, bool value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint2DValue  (IECInstanceR, ECValueAccessorCR accessor, DPoint2dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint3DValue  (IECInstanceR, ECValueAccessorCR accessor, DPoint3dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeValue (IECInstanceR, ECValueAccessorCR accessor, DateTimeCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeTicks (IECInstanceR, ECValueAccessorCR accessor, int64_t value);

    ECOBJECTS_EXPORT static bool            IsNull (IECInstanceR, ECValueAccessorCR);
    ECOBJECTS_EXPORT static void            SetToNull (IECInstanceR, ECValueAccessorCR);

    ECOBJECTS_EXPORT static bool            IsPropertyReadOnly (IECInstanceCR, ECValueAccessorR);
    ECOBJECTS_EXPORT static ECN::ECEnablerP  GetEnablerForStructArrayEntry (IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, Utf8CP className);
    ECOBJECTS_EXPORT static ECObjectsStatus GetStructArrayEntry (ECN::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, uint32_t index, ECN::ECValueAccessorCR structArrayValueAccessor,
                                                                  bool createPropertyIfNotFound, Utf8CP wcharAccessString, SchemaKeyCR schemaKey, Utf8CP className);

    ECOBJECTS_EXPORT static bool            IsCalculatedECProperty (IECInstanceCR instance, int propertyIndex);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValueByIndex         (IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus GetValueByIndex         (ECValueR value, IECInstanceCR instance, int propertyIndex, int arrayIndex);

    ECOBJECTS_EXPORT static ECObjectsStatus ClearArray (IECInstanceR instance, Utf8CP accessString);
    ECOBJECTS_EXPORT static ECObjectsStatus RemoveArrayElement (IECInstanceR instance, Utf8CP accessString, uint32_t index);
    ECOBJECTS_EXPORT static ECObjectsStatus AddArrayElements (IECInstanceR instance, Utf8CP accessString, uint32_t count, uint32_t atIndex = -1);
    };

/*---------------------------------------------------------------------------------**//**
* When updating serialized ECInstances to a new version of a schema, resolves the old data
* to the new schema.
* @bsistruct                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECSchemaRemapper
    {
protected:
    virtual bool                _ResolvePropertyName (Utf8StringR serializedPropertyName, ECClassCR ecClass) const = 0;
    virtual bool                _ResolveClassName (Utf8StringR serializedClassName, ECSchemaCR ecSchema) const = 0;
public:
    //! Given a serialized property name, return the name of the possibly renamed property in the specified ECClass. Returns true if a remapping for the specified property name was found.
    ECOBJECTS_EXPORT bool       ResolvePropertyName (Utf8StringR serializedPropertyName, ECClassCR ecClass) const;
    //! Given a serialized class name, return the name of the possibly renamed class in the specified ECSchema. Returns true if a remapping for the specified class name was found.
    ECOBJECTS_EXPORT bool       ResolveClassName (Utf8StringR serializedClassName, ECSchemaCR ecSchema) const;
    };

/*---------------------------------------------------------------------------------**//**
* Wrapper interface for a context that represents an IECInstance. Used by native
* presentation system and in interop contexts in which the IECInstance may not be
* available, or loaded only on demand.
* @bsistruct                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECInstanceInterface
    {
protected:
    virtual ECObjectsStatus         _GetInstanceValue (ECValueR v, Utf8CP managedAccessor) const = 0;
    virtual ECClassCP               _GetInstanceClass() const = 0;
    virtual IECInstanceCP           _ObtainECInstance() const = 0;
    virtual Utf8String              _GetInstanceId() const = 0;

public:
    virtual ~IECInstanceInterface() { }

    //! Get a property value 
    //! @param[out]     v               Holds the property value after successful return
    //! @param[in]      managedAccessor The managed-style access string (e.g. including array indices: "MyStructs[0].Property"...)
    //! @return ECObjectsStatus::Success if the value was retrieved, or an error code
    ECOBJECTS_EXPORT ECObjectsStatus    GetInstanceValue (ECValueR v, Utf8CP managedAccessor) const;

    //! @return The schema and class name of the IECInstance
    ECOBJECTS_EXPORT ECClassCP          GetInstanceClass() const;

    //! Attempts to obtain the underlying IECInstance. This method may be expensive to call in some contexts.
    //! @return The IECInstance represented by this interface, or nullptr if the IECInstance could not be obtained.
    ECOBJECTS_EXPORT IECInstanceCP      ObtainECInstance() const;

    //! Returns the instance ID of the underlying IECInstance
    ECOBJECTS_EXPORT Utf8String         GetInstanceId() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceInterface : IECInstanceInterface
    {
private:
    IECInstanceCR           m_instance;
protected:
    ECOBJECTS_EXPORT virtual ECObjectsStatus            _GetInstanceValue (ECValueR v, Utf8CP managedAccessor) const override;
    ECOBJECTS_EXPORT virtual ECClassCP                  _GetInstanceClass() const override;
    ECOBJECTS_EXPORT virtual IECInstanceCP              _ObtainECInstance() const override;
    ECOBJECTS_EXPORT virtual Utf8String                 _GetInstanceId() const override;
public:
    ECInstanceInterface (IECInstanceCR instance) : m_instance (instance) { }
    };


typedef bvector<IECInstancePtr>         ECInstanceList;
typedef ECInstanceList                  *ECInstanceListP, &ECInstanceListR;
typedef ECInstanceList const            *ECInstanceListCP;
typedef ECInstanceList const            &ECInstanceListCR;

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/
//#pragma make_public (ECN::IECInstance)
/*__PUBLISH_SECTION_START__*/
