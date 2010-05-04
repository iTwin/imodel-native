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

typedef struct IStream* IStreamP;

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<IECInstance> IECInstancePtr;

//! EC::IECInstance is the native equivalent of a .NET IECInstance.
//! Unlike IECInstance, it is not a pure interface, but is a concrete struct.
//! Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
//! in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a class that implements
//! the DgnECInstanceEnabler interface and one or more related interfaces to supply functionality 
//! to the EC::IECInstance.
//! We could call these "enabled" instances as opposed to "lightweight".
//! @see ECEnabler
struct IECInstance : RefCountedBase
    {
private:
protected:    
    ECOBJECTS_EXPORT IECInstance(); 
    ECOBJECTS_EXPORT ~IECInstance();
    ECOBJECTS_EXPORT virtual std::wstring _GetInstanceId() const = 0; // Virtual and returning std::wstring because a subclass may want to calculate it on demand
    ECOBJECTS_EXPORT virtual StatusInt    _GetValue (ECValueR v, const wchar_t * propertyAccessString) const = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _SetValue (const wchar_t * propertyAccessString, ECValueCR v) = 0;    
    ECOBJECTS_EXPORT virtual StatusInt    _SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index) = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size) = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _AddArrayElements (const wchar_t * propertyAccessString, UInt32 size) = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index) = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _ClearArray (const wchar_t * propertyAccessString) = 0;    
    ECOBJECTS_EXPORT virtual ECEnablerCR  _GetEnabler() const = 0;
    ECOBJECTS_EXPORT virtual bool         _IsReadOnly() const = 0;
    //! This should dump the instance's property values using the logger
    ECOBJECTS_EXPORT virtual void         _Dump () const = 0;
    
public:
    ECOBJECTS_EXPORT ECEnablerCR          GetEnabler() const;
    ECOBJECTS_EXPORT std::wstring         GetInstanceId() const;
    ECOBJECTS_EXPORT bool                 IsReadOnly() const;
    
    ECOBJECTS_EXPORT ECClassCR            GetClass() const;
    ECOBJECTS_EXPORT StatusInt            GetValue (ECValueR v, const wchar_t * propertyAccessString) const;    
    ECOBJECTS_EXPORT StatusInt            GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const;
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ECValueCR v);    
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index);
        
    //! Contract:
    //! - For all of the methods, the propertyAccessString should be in the "array element" form, 
    //!   e.g. "Aliases[]" instead of "Aliases"         
    ECOBJECTS_EXPORT StatusInt          InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size); //WIP_FUSION Return the new count?   
    ECOBJECTS_EXPORT StatusInt          AddArrayElements (const wchar_t * propertyAccessString, UInt32 size); //WIP_FUSION Return the new count?
    ECOBJECTS_EXPORT StatusInt          RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index); //WIP_FUSION return the removed one? YAGNI? Return the new count?
    ECOBJECTS_EXPORT StatusInt          ClearArray (const wchar_t * propertyAccessString);    
    
    //WIP_FUSION ParseExpectedNIndices should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    ECOBJECTS_EXPORT static int         ParseExpectedNIndices (const wchar_t * propertyAccessString);
    
    // These are more than just convenience methods... they enable an access pattern 
    // from managed code that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT StatusInt GetInteger       (int & value,             const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetLong          (Int64 & value,           const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetDouble        (double & value,          const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetString        (const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetBoolean       (bool & value,            const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetPoint2D       (DPoint2dR value,         const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetPoint3D       (DPoint3dR value,         const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetDateTime      (SystemTimeR value,       const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetDateTimeTicks (Int64 & value,           const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;

    ECOBJECTS_EXPORT StatusInt SetLongValue     (const wchar_t * propertyAccessString, Int64 value,           UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetIntegerValue  (const wchar_t * propertyAccessString, int value,             UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetStringValue   (const wchar_t * propertyAccessString, const wchar_t * value, UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetDoubleValue   (const wchar_t * propertyAccessString, double value,          UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetBooleanValue  (const wchar_t * propertyAccessString, bool value,            UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetPoint2DValue  (const wchar_t * propertyAccessString, DPoint2dR value,       UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetPoint3DValue  (const wchar_t * propertyAccessString, DPoint3dR value,       UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetDateTimeValue (const wchar_t * propertyAccessString, SystemTimeR value,     UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetDateTimeTicks (const wchar_t * propertyAccessString, Int64 value,           UInt32 nIndices = 0, UInt32 const * indices = NULL);

    ECOBJECTS_EXPORT void      Dump () const;

    ECOBJECTS_EXPORT static void      Debug_ResetAllocationStats ();
    ECOBJECTS_EXPORT static void      Debug_DumpAllocationStats (const wchar_t* prefix);
    ECOBJECTS_EXPORT static void      Debug_GetAllocationStats (int* currentLive, int* totalAllocs, int* totalFrees);
    ECOBJECTS_EXPORT static void      Debug_ReportLeaks (std::vector<std::wstring> classNamesToExclude);

    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromFile   (IECInstancePtr& ecInstance, const wchar_t* fileName, ECSchemaP schema);
    ECOBJECTS_EXPORT static InstanceDeserializationStatus   ReadXmlFromStream (IECInstancePtr& ecInstance, IStreamP stream, ECSchemaP schema);

    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToFile   (const wchar_t* fileName);
    ECOBJECTS_EXPORT InstanceSerializationStatus            WriteXmlToStream (IStreamP stream);
    };
    
//! EC::IECRelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! @see IECInstance, ECRelationshipClass
struct IECRelationshipInstance : public IECInstance
    {
private:
    //needswork: needs EC::IECInstance Source/Target
    };
        
END_BENTLEY_EC_NAMESPACE
