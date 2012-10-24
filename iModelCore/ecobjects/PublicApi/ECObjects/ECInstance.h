/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <Geom/GeomApi.h>

BENTLEY_TYPEDEFS (BeXmlDom)
BENTLEY_TYPEDEFS (BeXmlNode)

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
//! @see Bentley::EC

//////////////////////////////////////////////////////////////////////////////////
//  The following definitions are used to allow a struct property to generate a
//  custom XML representation of itself. This was required to support 8.11 
//  installedTypes as Vancouver ECStructs  
//////////////////////////////////////////////////////////////////////////////////
typedef bmap<WString, ICustomECStructSerializerP> NameSerializerMap;

//! @ingroup ECObjectsGroup
//! @bsiclass
struct ICustomECStructSerializer
    {
    virtual bool            UsesCustomStructXmlString  (StructECPropertyP structProperty, IECInstanceCR ecInstance) const = 0;
    virtual ECObjectsStatus GenerateXmlString  (WString& xmlString, StructECPropertyP structProperty, IECInstanceCR ecInstance, WCharCP baseAccessString) const = 0;
    virtual void            LoadStructureFromString (StructECPropertyP structProperty, IECInstanceR ecInstance, WCharCP baseAccessString, WCharCP valueString) = 0;
    };

//! @ingroup ECObjectsGroup
//! @bsiclass
struct CustomStructSerializerManager
{
//__PUBLISH_SECTION_END__
private:
    NameSerializerMap   m_serializers;

    CustomStructSerializerManager();
    ~CustomStructSerializerManager();

    ICustomECStructSerializerP GetCustomSerializer (WCharCP serializerName) const;
//__PUBLISH_SECTION_START__
public:
    ECOBJECTS_EXPORT  ICustomECStructSerializerP            GetCustomSerializer (StructECPropertyP structProperty, IECInstanceCR ecInstance) const;
    ECOBJECTS_EXPORT  static CustomStructSerializerManagerR GetManager();
    ECOBJECTS_EXPORT  BentleyStatus                         AddCustomSerializer (WCharCP serializerName, ICustomECStructSerializerP serializer);
};

typedef RefCountedPtr<IECInstance> IECInstancePtr;
//=======================================================================================    
//! @ingroup ECObjectsGroup
//! ECN::IECInstance is the native equivalent of a .NET IECInstance.
//! Unlike IECInstance, it is not a pure interface, but is a concrete struct.
//! Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
//! in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a class that implements
//! the DgnElementECInstanceEnabler interface and one or more related interfaces to supply functionality 
//! to the ECN::IECInstance.
//! We could call these "enabled" instances as opposed to "lightweight".
//! @see ECEnabler
//! @bsiclass
//=======================================================================================    
struct EXPORT_VTABLE_ATTRIBUTE IECInstance : RefCountedBase
    {
private:
    WCharCP GetInstanceLabelPropertyName () const;

protected:    
    ECOBJECTS_EXPORT IECInstance(); 
    ECOBJECTS_EXPORT virtual ~IECInstance();

    virtual WString             _GetInstanceId() const = 0; // Virtual and returning WString because a subclass may want to calculate it on demand
    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const = 0;
protected:
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) = 0;
    virtual ECObjectsStatus     _InsertArrayElements (WCharCP managedPropertyAccessor, UInt32 index, UInt32 size) = 0;
    virtual ECObjectsStatus     _AddArrayElements (WCharCP managedPropertyAccessor, UInt32 size) = 0;
    virtual ECObjectsStatus     _RemoveArrayElement (WCharCP managedPropertyAccessor, UInt32 index) = 0;
    virtual ECObjectsStatus     _ClearArray (WCharCP managedPropertyAccessor) = 0;    
    virtual ECEnablerCR         _GetEnabler() const = 0;
    virtual bool                _IsReadOnly() const = 0;
    virtual WString             _ToString (WCharCP indent) const = 0;
    virtual size_t              _GetOffsetToIECInstance () const = 0;
    virtual ECObjectsStatus     _GetIsPropertyNull (bool& v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const = 0;

    ECOBJECTS_EXPORT virtual ECObjectsStatus       _SetInstanceId(WCharCP);
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _GetDisplayLabel (WString& displayLabel) const;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus       _SetDisplayLabel (WCharCP displayLabel);    
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const;
    //! If you override one of these IsPropertyReadOnly methods, you should override the other.
    ECOBJECTS_EXPORT virtual bool                   _IsPropertyReadOnly (WCharCP accessString) const;
    ECOBJECTS_EXPORT virtual bool                   _IsPropertyReadOnly (UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus        _SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex);

public:
    ECOBJECTS_EXPORT void const*        GetBaseAddress () {return this;}
    ECOBJECTS_EXPORT ECEnablerCR        GetEnabler() const;
    ECOBJECTS_EXPORT ECEnablerR         GetEnablerR() const;      // use when enabler.ObtainStandaloneEnabler is called since a new enabler may be created.

    //! Gets the unique ID for this instance
    ECOBJECTS_EXPORT WString            GetInstanceId() const;
    //! Sets the unique id for this instance
    ECOBJECTS_EXPORT ECObjectsStatus    SetInstanceId(WCharCP instanceId);
    //! Returns whether the ECInstance as a whole is ReadOnly
    ECOBJECTS_EXPORT bool               IsReadOnly() const;
    //! Returns the ECClass that defines this ECInstance
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, WCharCP propertyAccessString) const;    
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @param[in]  index   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, WCharCP propertyAccessString, UInt32 index) const;
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex) const;
    //! Gets the value stored in the specified ECProperty
    //! @param[out] v   If successful, will contain the value of the property
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @param[in]  arrayIndex   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const;
    //! Sets the value for the specified property
    //! @param[in]  propertyAccessString The name of the property to set the value of
    //! @param[in]  v   The value to set onto the property
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (WCharCP propertyAccessString, ECValueCR v);    
    //! Sets the value for the specified property
    //! @param[in]  propertyAccessString The name of the property to set the value of
    //! @param[in]  v   The value to set onto the property
    //! @param[in]  index   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (WCharCP propertyAccessString, ECValueCR v, UInt32 index);
    //! Sets the value for the specified property
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @param[in]  v   The value to set onto the property
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v);
    //! Sets the value for the specified property
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @param[in]  v   The value to set onto the property
    //! @param[in]  arrayIndex   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex);
    //! Gets the value of the ECProperty specified by the ECValueAccessor
    ECOBJECTS_EXPORT ECObjectsStatus    GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const;
    //! Sets the value of the ECProperty specified by the ECValueAccessor
    ECOBJECTS_EXPORT ECObjectsStatus    SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR v);

    //! Check is the NullFlags for the Null setting of a specific value without read the actual value
    //! Check for Null property value
    //! @param[out] isNull   If successful, will contain true if property value is NULL.
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, WCharCP propertyAccessString) const; 

    //! Check for Null property value
    //! @param[out] isNull   If successful, will contain true if property value is NULL.
    //! @param[in]  propertyAccessString Name of the property to retrieve
    //! @param[in]  arrayIndex   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, WCharCP propertyAccessString, UInt32 arrayIndex) const;

    //! Check for Null property value
    //! @param[out] isNull   If successful, will contain true if property value is NULL.
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, UInt32 propertyIndex) const; 

    //! Check for Null property value
    //! @param[out] isNull   If successful, will contain true if property value is NULL.
    //! @param[in]  propertyIndex Index into the PropertyLayout indicating which property to retrieve
    //! @param[in]  arrayIndex   The array index, if this is an ArrayProperty
    //! @returns ECOBJECTS_STATUS_Success if successful, otherwise an error code indicating the failure
    ECOBJECTS_EXPORT ECObjectsStatus     IsPropertyNull (bool& isNull, UInt32 propertyIndex, UInt32 arrayIndex) const; 

/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT ECObjectsStatus    GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex); 
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (UInt32 propertyIndex, ECValueCR v); 
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR valueToSet);
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (WCharCP propertyAccessString, ECValueCR v); 
    ECOBJECTS_EXPORT ECObjectsStatus    SetInternalValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex); 
/*__PUBLISH_SECTION_START__*/

    //! Check property to see it is a fixed size array and optionally return the fixed size.
    //! @param[in]  instance Instance to process
    //! @param[in]  accessString   The access string for the array
    //! @param[out] numFixedEntries   Optional pointer to size of fixed size array.
    //! @returns true if the property is a fixed size array.
    ECOBJECTS_EXPORT static bool        IsFixedArrayProperty (ECN::IECInstanceR instance, WCharCP accessString, UInt32* numFixedEntries=NULL);

    ECOBJECTS_EXPORT bool               IsPropertyReadOnly (WCharCP accessString) const;
    ECOBJECTS_EXPORT bool               IsPropertyReadOnly (UInt32 propertyIndex) const;

    //! Contract:
    //! - For all of the methods, the propertyAccessString should be in the "array element" form, 
    //!   e.g. "Aliases[]" instead of "Aliases"         
    ECOBJECTS_EXPORT ECObjectsStatus      InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size);
    ECOBJECTS_EXPORT ECObjectsStatus      AddArrayElements (WCharCP propertyAccessString, UInt32 size);
    ECOBJECTS_EXPORT ECObjectsStatus      RemoveArrayElement (WCharCP propertyAccessString, UInt32 index);
    ECOBJECTS_EXPORT ECObjectsStatus      ClearArray (WCharCP propertyAccessString);    
    ECOBJECTS_EXPORT ECObjectsStatus      GetDisplayLabel (WString& displayLabel) const;    
    ECOBJECTS_EXPORT ECObjectsStatus      SetDisplayLabel (WCharCP displayLabel);    
    
    ECOBJECTS_EXPORT MemoryECInstanceBase* GetAsMemoryECInstance () const;
    ECOBJECTS_EXPORT size_t                GetOffsetToIECInstance () const;

    ECOBJECTS_EXPORT WString            ToString (WCharCP indent) const;

    ECOBJECTS_EXPORT IECInstancePtr     CreateCopyThroughSerialization();

    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlFile   (IECInstancePtr& ecInstance, WCharCP fileName,   ECInstanceReadContextR context);
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlStream (IECInstancePtr& ecInstance, IStreamP stream,    ECInstanceReadContextR context);
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromXmlString (IECInstancePtr& ecInstance, WCharCP xmlString,  ECInstanceReadContextR context);
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromBeXmlDom  (IECInstancePtr& ecInstance, BeXmlDomR xmlNode,  ECInstanceReadContextR context);
    ECOBJECTS_EXPORT static InstanceReadStatus  ReadFromBeXmlNode (IECInstancePtr& ecInstance, BeXmlNodeR xmlNode, ECInstanceReadContextR context);

    ECOBJECTS_EXPORT InstanceWriteStatus            WriteToXmlFile   (WCharCP fileName, bool isStandAlone, bool writeInstanceId, bool utf16);
    ECOBJECTS_EXPORT InstanceWriteStatus            WriteToXmlStream (IStreamP stream, bool isStandAlone, bool writeInstanceId, bool utf16);
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToXmlString (WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId);
    ECOBJECTS_EXPORT InstanceWriteStatus        WriteToBeXmlNode (BeXmlNodeR xmlNode);
    };
    
//=======================================================================================    
//! @ingroup ECObjectsGroup
//! ECN::IECRelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! @see IECInstance, ECRelationshipClass
//! @bsiclass
//=======================================================================================    
struct EXPORT_VTABLE_ATTRIBUTE IECRelationshipInstance : virtual IECInstance
    {
    private:
        virtual void            _SetSource (IECInstanceP instance) = 0;
        virtual IECInstancePtr  _GetSource () const = 0;
        virtual ECObjectsStatus _GetSourceOrderId (Int64& sourceOrderId) const = 0;
        virtual void            _SetTarget (IECInstanceP instance)= 0;
        virtual IECInstancePtr  _GetTarget () const = 0;
        virtual ECObjectsStatus _GetTargetOrderId (Int64& targetOrderId) const = 0;
        DECLARE_DUMMY_VIRTUAL_IN_INTERFACE

    public:
        ECOBJECTS_EXPORT void            SetSource (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstancePtr  GetSource () const;
        ECOBJECTS_EXPORT ECObjectsStatus GetSourceOrderId (Int64& targetOrderId) const;
        ECOBJECTS_EXPORT void            SetTarget (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstancePtr  GetTarget () const;
        ECOBJECTS_EXPORT ECObjectsStatus GetTargetOrderId (Int64& targetOrderId) const;
    };

typedef RefCountedPtr<IECRelationshipInstance> IECRelationshipInstancePtr;

/*__PUBLISH_SECTION_END__*/

struct ECStructArrayMemberAccessor;

struct ECInstanceInteropHelper
    {
    // These are not convenience methods.  They are intended for managed callers.  They enable
    // an access pattern that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT static ECObjectsStatus GetValue         (IECInstanceCR, ECValueR value,          WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,             WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, Int64 & value,           WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,          WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, WStringR value,          WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetBoolean       (IECInstanceCR, bool & value,            WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint2D       (IECInstanceCR, DPoint2dR value,         WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint3D       (IECInstanceCR, DPoint3dR value,         WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTime      (IECInstanceCR, SystemTimeR value,       WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeTicks (IECInstanceCR, Int64 & value,           WCharCP managedPropertyAccessor);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValue         (IECInstanceR, WCharCP managedPropertyAccessor, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetLongValue     (IECInstanceR, WCharCP managedPropertyAccessor, Int64 value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetIntegerValue  (IECInstanceR, WCharCP managedPropertyAccessor, int value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetStringValue   (IECInstanceR, WCharCP managedPropertyAccessor, WCharCP value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDoubleValue   (IECInstanceR, WCharCP managedPropertyAccessor, double value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetBooleanValue  (IECInstanceR, WCharCP managedPropertyAccessor, bool value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint2DValue  (IECInstanceR, WCharCP managedPropertyAccessor, DPoint2dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint3DValue  (IECInstanceR, WCharCP managedPropertyAccessor, DPoint3dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeValue (IECInstanceR, WCharCP managedPropertyAccessor, SystemTimeR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeTicks (IECInstanceR, WCharCP managedPropertyAccessor, Int64 value);

    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,        ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, Int64 & value,      ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,     ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, WStringR value,     ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetBoolean       (IECInstanceCR, bool & value,       ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint2D       (IECInstanceCR, DPoint2dR value,    ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint3D       (IECInstanceCR, DPoint3dR value,    ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTime      (IECInstanceCR, SystemTimeR value,  ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeTicks (IECInstanceCR, Int64 & value,      ECValueAccessorCR accessor);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValue         (IECInstanceR, ECValueAccessorCR accessor, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetLongValue     (IECInstanceR, ECValueAccessorCR accessor, Int64 value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetIntegerValue  (IECInstanceR, ECValueAccessorCR accessor, int value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetStringValue   (IECInstanceR, ECValueAccessorCR accessor, WCharCP value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDoubleValue   (IECInstanceR, ECValueAccessorCR accessor, double value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetBooleanValue  (IECInstanceR, ECValueAccessorCR accessor, bool value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint2DValue  (IECInstanceR, ECValueAccessorCR accessor, DPoint2dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint3DValue  (IECInstanceR, ECValueAccessorCR accessor, DPoint3dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeValue (IECInstanceR, ECValueAccessorCR accessor, SystemTimeR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeTicks (IECInstanceR, ECValueAccessorCR accessor, Int64 value);

    ECOBJECTS_EXPORT static bool            IsNull (IECInstanceR, ECValueAccessorCR);
    ECOBJECTS_EXPORT static void            SetToNull (IECInstanceR, ECValueAccessorCR);

    ECOBJECTS_EXPORT static bool            IsPropertyReadOnly (IECInstanceCR, ECValueAccessorR);
    ECOBJECTS_EXPORT static ECN::ECEnablerP  GetEnablerForStructArrayEntry (IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, WCharCP className);
    ECOBJECTS_EXPORT static ECObjectsStatus GetStructArrayEntry (ECN::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, UInt32 index, ECN::ECValueAccessorCR structArrayValueAccessor, 
                                                                  bool createPropertyIfNotFound, WCharCP wcharAccessString, SchemaKeyCR schemaKey, WCharCP className);

    ECOBJECTS_EXPORT static bool            IsCalculatedECProperty (IECInstanceCR instance, int propertyIndex);

    //! Gets the next property index in the instance or struct that is being enumerated.
    //! @param [I/O] propertyIndex    - the next property index of a property belonging to the struct or instance that is being enumerated.
    //!                                 Should start as -1 for instance enumeration, or the first index of a struct for struct enumeration.
    //! @param [Out] structNameLength - if an embedded struct has been found, the length of the name of that embedded struct.
    //! @param [Out] accessor         - a pointer to the access string of the next property
    //! @param       instance         - the incoming instance
    //! @param       prefix           - the length of the name of the struct or embedded struct that is being enumerated
    //! @param       includeNulls     - whether or not the enumerator should skip null values (per ECObjects, structs are never null)
    //! @param       firstRunInStruct - whether or not this is the first time this method has been called in a struct enumerator.
    //!                                 This parameter will cause GetNextInteropProperty to avoid incrementing propertyIndex, as 
    //!                                 propertyIndex starts is the first property of that struct.
    ECOBJECTS_EXPORT static bool            GetNextInteropProperty (int& propertyIndex, int& structNameLength, WCharCP& accessor, IECInstanceCR instance, int prefix, bool includeNulls, bool firstRunInStruct);
    ECOBJECTS_EXPORT static int             FirstIndexOfStruct     (IECInstanceCR instance, WCharCP structName);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValueByIndex         (IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus GetValueByIndex         (ECValueR value, IECInstanceCR instance, int propertyIndex, int arrayIndex);
    };

/*__PUBLISH_SECTION_START__*/


END_BENTLEY_ECOBJECT_NAMESPACE
