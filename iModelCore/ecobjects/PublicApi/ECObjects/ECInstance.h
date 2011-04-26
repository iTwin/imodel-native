/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <Geom\GeomApi.h>

BEGIN_BENTLEY_EC_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////
//  The following definitions are used to allow a struct property to generate a
//  custom XML representation of itself. This was required to support 8.11 
//  installedTypes as Vancouver ECStructs  
//////////////////////////////////////////////////////////////////////////////////
typedef bmap<WString, ICustomECStructSerializerP> NameSerializerMap;

struct ICustomECStructSerializer
    {
    virtual bool            UsesCustomStructXmlString  (StructECPropertyP structProperty, IECInstanceCR ecInstance) const = 0;
    virtual ECObjectsStatus GenerateXmlString  (WString& xmlString, StructECPropertyP structProperty, IECInstanceCR ecInstance, WCharCP baseAccessString) const = 0;
    virtual void            LoadStructureFromString (StructECPropertyP structProperty, IECInstanceR ecInstance, WCharCP baseAccessString, WCharCP valueString) = 0;
    };

struct CustomStructSerializerManager
{
private:
    NameSerializerMap   m_serializers;

    CustomStructSerializerManager();
    ~CustomStructSerializerManager();

    ICustomECStructSerializerP GetCustomSerializer (WCharCP serializerName) const;

public:
    ECOBJECTS_EXPORT  ICustomECStructSerializerP            GetCustomSerializer (StructECPropertyP structProperty, IECInstanceCR ecInstance) const;
    ECOBJECTS_EXPORT  static CustomStructSerializerManagerR GetManager();
    ECOBJECTS_EXPORT  BentleyStatus                         AddCustomSerializer (WCharCP serializerName, ICustomECStructSerializerP serializer);
};

typedef RefCountedPtr<IECInstance> IECInstancePtr;
//=======================================================================================    
//! EC::IECInstance is the native equivalent of a .NET IECInstance.
//! Unlike IECInstance, it is not a pure interface, but is a concrete struct.
//! Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
//! in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a class that implements
//! the DgnECInstanceEnabler interface and one or more related interfaces to supply functionality 
//! to the EC::IECInstance.
//! We could call these "enabled" instances as opposed to "lightweight".
//! @see ECEnabler
//=======================================================================================    
struct IECInstance : RefCountedBase
    {
private:
protected:    
    ECOBJECTS_EXPORT IECInstance(); 
    ECOBJECTS_EXPORT virtual ~IECInstance();

    virtual WString            _GetInstanceId() const = 0; // Virtual and returning WString because a subclass may want to calculate it on demand
    virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP managedPropertyAccessor, bool useArrayIndex, UInt32 arrayIndex) const = 0;
    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const = 0;
public:
    virtual ECObjectsStatus     _SetValue (WCharCP managedPropertyAccessor, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) = 0;
protected:
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) = 0;
    virtual ECObjectsStatus     _InsertArrayElements (WCharCP managedPropertyAccessor, UInt32 index, UInt32 size) = 0;
    virtual ECObjectsStatus     _AddArrayElements (WCharCP managedPropertyAccessor, UInt32 size) = 0;
    virtual ECObjectsStatus     _RemoveArrayElement (WCharCP managedPropertyAccessor, UInt32 index) = 0;
    virtual ECObjectsStatus     _ClearArray (WCharCP managedPropertyAccessor) = 0;    
    virtual ECEnablerCR         _GetEnabler() const = 0;
    virtual bool                _IsReadOnly() const = 0;
    virtual WString            _ToString (WCharCP indent) const = 0;
    virtual size_t              _GetOffsetToIECInstance () const = 0;

    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const;

public:
    ECOBJECTS_EXPORT void const*        GetBaseAddress () {return this;}
    ECOBJECTS_EXPORT ECEnablerCR        GetEnabler() const;
    ECOBJECTS_EXPORT ECEnablerR         GetEnablerR() const;      // use when enabler.ObtainStandaloneEnabler is called since a new enabler may be created.

    ECOBJECTS_EXPORT WString           GetInstanceId() const;
    ECOBJECTS_EXPORT bool               IsReadOnly() const;
    
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, WCharCP managedPropertyAccessor) const;    
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, WCharCP managedPropertyAccessor, UInt32 index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (WCharCP managedPropertyAccessor, ECValueCR v);    
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (WCharCP managedPropertyAccessor, ECValueCR v, UInt32 index);
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v);
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex);
    ECOBJECTS_EXPORT ECObjectsStatus    GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR v);
    
    ECOBJECTS_EXPORT static bool        IsFixedArrayProperty (EC::IECInstanceR instance, WCharCP accessString);

    //! Contract:
    //! - For all of the methods, the managedPropertyAccessor should be in the "array element" form, 
    //!   e.g. "Aliases[]" instead of "Aliases"         
    ECOBJECTS_EXPORT ECObjectsStatus      InsertArrayElements (WCharCP managedPropertyAccessor, UInt32 index, UInt32 size); //WIP_FUSION Return the new count?   
    ECOBJECTS_EXPORT ECObjectsStatus      AddArrayElements (WCharCP managedPropertyAccessor, UInt32 size); //WIP_FUSION Return the new count?
    ECOBJECTS_EXPORT ECObjectsStatus      RemoveArrayElement (WCharCP managedPropertyAccessor, UInt32 index); //WIP_FUSION return the removed one? YAGNI? Return the new count?
    ECOBJECTS_EXPORT ECObjectsStatus      ClearArray (WCharCP managedPropertyAccessor);    
    
    ECOBJECTS_EXPORT MemoryECInstanceBase* GetAsMemoryECInstance () const;
    ECOBJECTS_EXPORT size_t                GetOffsetToIECInstance () const;

    //WIP_FUSION ParseExpectedNIndices should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    ECOBJECTS_EXPORT static int         ParseExpectedNIndices (WCharCP managedPropertyAccessor);
    
    ECOBJECTS_EXPORT WString           ToString (WCharCP indent) const;

    ECOBJECTS_EXPORT static void        Debug_ResetAllocationStats ();
    ECOBJECTS_EXPORT static void        Debug_DumpAllocationStats (WCharCP prefix);
    ECOBJECTS_EXPORT static void        Debug_GetAllocationStats (int* currentLive, int* totalAllocs, int* totalFrees);
    ECOBJECTS_EXPORT static void        Debug_ReportLeaks (bvector<WString>& classNamesToExclude);

    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromFile   (IECInstancePtr& ecInstance, WCharCP fileName, ECInstanceDeserializationContextR context);
    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromStream (IECInstancePtr& ecInstance, IStreamP stream, ECInstanceDeserializationContextR context);
    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromString (IECInstancePtr& ecInstance, WCharCP xmlString, ECInstanceDeserializationContextR context);

    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToFile   (WCharCP fileName, bool isStandAlone);
    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToStream (IStreamP stream, bool isStandAlone);
    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToString (WString & ecInstanceXml, bool isStandAlone);
    };
    
//=======================================================================================    
//! EC::IECRelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! Note: This class is not RefCounted because the Legacy and ECX Relationships that 
//! are derived from this class are also derived from IECInstance which is refcounted.
//! @see IECInstance, ECRelationshipClass
//=======================================================================================    
struct IECRelationshipInstance 
    {
    private:
        virtual void            _SetSource (IECInstanceP instance) = 0;
        virtual IECInstancePtr  _GetSource () const = 0;
        virtual void            _SetTarget (IECInstanceP instance)= 0;
        virtual IECInstancePtr  _GetTarget () const = 0;

    public:
        ECOBJECTS_EXPORT void            SetSource (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstancePtr  GetSource () const;
        ECOBJECTS_EXPORT void            SetTarget (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstancePtr  GetTarget () const;
    };
        
typedef RefCountedPtr<ECRelationshipInstanceHolder> ECRelationshipInstanceHolderPtr;

//=======================================================================================
//! ECRelationshipInstanceHolder is used to hold a IECInstance that is also a 
//! IECRelationshipInstance.
//=======================================================================================
struct ECRelationshipInstanceHolder : RefCountedBase 
{
private:
    IECInstancePtr             m_iecInstance;
    IECRelationshipInstanceP   m_relationshipInstance;

    ECRelationshipInstanceHolder (IECInstanceR iecInstance);

public:
    ECOBJECTS_EXPORT IECInstanceP GetAsIECInstance ();
    ECOBJECTS_EXPORT IECRelationshipInstanceP GetAsIECRelationshipInstance ();
    ECOBJECTS_EXPORT static ECRelationshipInstanceHolderPtr Create (IECInstanceR iecInstance);
};

//=======================================================================================
//! IECWipRelationshipInstance is used to set the name and order properties for an 
//! ECRelationship.
//=======================================================================================
struct IECWipRelationshipInstance
    {
    protected:
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetName (WCharCP name) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetSourceOrderId (Int64 sourceOrderId) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetTargetOrderId (Int64 targetOrderId) = 0;
        ECOBJECTS_EXPORT virtual ECEnablerP     _GetECEnablerP () = 0;

    public:
        ECOBJECTS_EXPORT BentleyStatus  SetName (WCharCP name);
        ECOBJECTS_EXPORT BentleyStatus  SetSourceOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT BentleyStatus  SetTargetOrderId (Int64 targetOrderId);
        ECOBJECTS_EXPORT ECEnablerP     GetECEnablerP ();
    };


/*__PUBLISH_SECTION_END__*/

struct ECInstanceInteropHelper
    {
    // These are not convenience methods.  They are intended for managed callers.  They enable
    // an access pattern that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT static ECObjectsStatus GetValue         (IECInstanceCR, ECValueR value,          WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,             WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, Int64 & value,           WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,          WCharCP managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, WCharCP & value, WCharCP managedPropertyAccessor);
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

    ECOBJECTS_EXPORT static PrimitiveType   GetPrimitiveType       (IECInstanceCR instance, int propertyIndex);
#ifdef NOT_USED
    ECOBJECTS_EXPORT static ValueKind       GetValueKind           (IECInstanceCR instance, int propertyIndex);
    ECOBJECTS_EXPORT static ArrayKind       GetArrayKind           (IECInstanceCR instance, int propertyIndex);
#endif
    ECOBJECTS_EXPORT static bool            IsStructArray          (IECInstanceCR instance, int propertyIndex);
    ECOBJECTS_EXPORT static bool            IsArray                (IECInstanceCR instance, int propertyIndex);
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


END_BENTLEY_EC_NAMESPACE
