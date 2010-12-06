/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <Geom\GeomApi.h>

BEGIN_BENTLEY_EC_NAMESPACE

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

    virtual bwstring            _GetInstanceId() const = 0; // Virtual and returning bwstring because a subclass may want to calculate it on demand
    virtual ECObjectsStatus     _GetValue (ECValueR v, const wchar_t * managedPropertyAccessor, bool useArrayIndex, UInt32 arrayIndex) const = 0;
    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const = 0;
public:
    virtual ECObjectsStatus     _SetValue (const wchar_t * managedPropertyAccessor, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) = 0;
protected:
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) = 0;
    virtual ECObjectsStatus     _InsertArrayElements (const wchar_t * managedPropertyAccessor, UInt32 index, UInt32 size) = 0;
    virtual ECObjectsStatus     _AddArrayElements (const wchar_t * managedPropertyAccessor, UInt32 size) = 0;
    virtual ECObjectsStatus     _RemoveArrayElement (const wchar_t * managedPropertyAccessor, UInt32 index) = 0;
    virtual ECObjectsStatus     _ClearArray (const wchar_t * managedPropertyAccessor) = 0;    
    virtual ECEnablerCR         _GetEnabler() const = 0;
    virtual bool                _IsReadOnly() const = 0;
    virtual bwstring            _ToString (const wchar_t* indent) const = 0;
    virtual size_t              _GetOffsetToIECInstance () const = 0;

    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const;

public:
    ECOBJECTS_EXPORT void const*        GetBaseAddress () {return this;}
    ECOBJECTS_EXPORT ECEnablerCR        GetEnabler() const;
    ECOBJECTS_EXPORT bwstring           GetInstanceId() const;
    ECOBJECTS_EXPORT bool               IsReadOnly() const;
    
    ECOBJECTS_EXPORT ECClassCR          GetClass() const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, const wchar_t * managedPropertyAccessor) const;    
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, const wchar_t * managedPropertyAccessor, UInt32 index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (const wchar_t * managedPropertyAccessor, ECValueCR v);    
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (const wchar_t * managedPropertyAccessor, ECValueCR v, UInt32 index);
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v);
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex);
    ECOBJECTS_EXPORT ECObjectsStatus    GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const;
    ECOBJECTS_EXPORT ECObjectsStatus    SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR v);
    
    ECOBJECTS_EXPORT static bool        IsFixedArrayProperty (EC::IECInstanceR instance, const wchar_t* accessString);

    //! Contract:
    //! - For all of the methods, the managedPropertyAccessor should be in the "array element" form, 
    //!   e.g. "Aliases[]" instead of "Aliases"         
    ECOBJECTS_EXPORT ECObjectsStatus      InsertArrayElements (const wchar_t * managedPropertyAccessor, UInt32 index, UInt32 size); //WIP_FUSION Return the new count?   
    ECOBJECTS_EXPORT ECObjectsStatus      AddArrayElements (const wchar_t * managedPropertyAccessor, UInt32 size); //WIP_FUSION Return the new count?
    ECOBJECTS_EXPORT ECObjectsStatus      RemoveArrayElement (const wchar_t * managedPropertyAccessor, UInt32 index); //WIP_FUSION return the removed one? YAGNI? Return the new count?
    ECOBJECTS_EXPORT ECObjectsStatus      ClearArray (const wchar_t * managedPropertyAccessor);    
    
    ECOBJECTS_EXPORT MemoryECInstanceBase* GetAsMemoryECInstance () const;
    ECOBJECTS_EXPORT size_t                GetOffsetToIECInstance () const;

    //WIP_FUSION ParseExpectedNIndices should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    ECOBJECTS_EXPORT static int         ParseExpectedNIndices (const wchar_t * managedPropertyAccessor);
    
    ECOBJECTS_EXPORT bwstring           ToString (const wchar_t* indent) const;

    ECOBJECTS_EXPORT static void        Debug_ResetAllocationStats ();
    ECOBJECTS_EXPORT static void        Debug_DumpAllocationStats (const wchar_t* prefix);
    ECOBJECTS_EXPORT static void        Debug_GetAllocationStats (int* currentLive, int* totalAllocs, int* totalFrees);
    ECOBJECTS_EXPORT static void        Debug_ReportLeaks (std::vector<bwstring>& classNamesToExclude);

    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromFile   (IECInstancePtr& ecInstance, const wchar_t* fileName, ECSchemaCR schema);
    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromStream (IECInstancePtr& ecInstance, IStreamP stream, ECSchemaCR schema);
    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromString (IECInstancePtr& ecInstance, const wchar_t* xmlString, ECSchemaCR schema);

    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToFile   (const wchar_t* fileName, bool isStandAlone);
    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToStream (IStreamP stream, bool isStandAlone);
    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToString (bwstring & ecInstanceXml, bool isStandAlone);
    };
    
//=======================================================================================    
//! EC::IECRelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! @see IECInstance, ECRelationshipClass
//=======================================================================================    
struct IECRelationshipInstance
    {
    private:
        virtual void          _SetSource (IECInstanceP instance) = 0;
        virtual IECInstanceP  _GetSource () const = 0;
        virtual void          _SetTarget (IECInstanceP instance)= 0;
        virtual IECInstanceP  _GetTarget () const = 0;

    public:
        ECOBJECTS_EXPORT void          SetSource (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstanceP  GetSource () const;
        ECOBJECTS_EXPORT void          SetTarget (IECInstanceP instance);
        ECOBJECTS_EXPORT IECInstanceP  GetTarget () const;
    };
        
/*__PUBLISH_SECTION_END__*/

struct ECInstanceInteropHelper
    {
    // These are not convenience methods.  They are intended for managed callers.  They enable
    // an access pattern that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT static ECObjectsStatus GetValue         (IECInstanceCR, ECValueR value,          const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetInteger       (IECInstanceCR, int & value,             const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetLong          (IECInstanceCR, Int64 & value,           const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDouble        (IECInstanceCR, double & value,          const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetString        (IECInstanceCR, const wchar_t * & value, const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetBoolean       (IECInstanceCR, bool & value,            const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint2D       (IECInstanceCR, DPoint2dR value,         const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetPoint3D       (IECInstanceCR, DPoint3dR value,         const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTime      (IECInstanceCR, SystemTimeR value,       const wchar_t * managedPropertyAccessor);
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeTicks (IECInstanceCR, Int64 & value,           const wchar_t * managedPropertyAccessor);

    ECOBJECTS_EXPORT static ECObjectsStatus SetValue         (IECInstanceR, const wchar_t * managedPropertyAccessor, ECValueCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetLongValue     (IECInstanceR, const wchar_t * managedPropertyAccessor, Int64 value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetIntegerValue  (IECInstanceR, const wchar_t * managedPropertyAccessor, int value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetStringValue   (IECInstanceR, const wchar_t * managedPropertyAccessor, const wchar_t * value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDoubleValue   (IECInstanceR, const wchar_t * managedPropertyAccessor, double value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetBooleanValue  (IECInstanceR, const wchar_t * managedPropertyAccessor, bool value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint2DValue  (IECInstanceR, const wchar_t * managedPropertyAccessor, DPoint2dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetPoint3DValue  (IECInstanceR, const wchar_t * managedPropertyAccessor, DPoint3dCR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeValue (IECInstanceR, const wchar_t * managedPropertyAccessor, SystemTimeR value);
    ECOBJECTS_EXPORT static ECObjectsStatus SetDateTimeTicks (IECInstanceR, const wchar_t * managedPropertyAccessor, Int64 value);
    };

/*__PUBLISH_SECTION_START__*/


END_BENTLEY_EC_NAMESPACE
