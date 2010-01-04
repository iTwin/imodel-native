/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE

//! EC::Instance is the native equivalent of a .NET IECInstance.
//! Unlike IECInstance, it is not a pure interface, but is a concrete struct.
//! Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
//! in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a class that implements
//! the EC::Enabler interface and one or more related interfaces to supply functionality 
//! to the EC::Instance.
//! We could call these "enabled" instances as opposed to "lightweight".
//! @see Enabler
struct Instance
    {
private:
protected:    
    ECOBJECTS_EXPORT Instance(); 
    ECOBJECTS_EXPORT ~Instance(){};
    ECOBJECTS_EXPORT virtual std::wstring _GetInstanceID() const = 0; // Virtual and returning std::wstring because a subclass may want to calculate it on demand
    ECOBJECTS_EXPORT virtual StatusInt    _GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices = 0, UInt32 const * indices = NULL) = 0;
    ECOBJECTS_EXPORT virtual EnablerCP    _GetEnabler() const = 0; // WIP_FUSION: Should this return an EnablerCR?
    ECOBJECTS_EXPORT virtual bool         _IsReadOnly() const = 0;
    //! This should dump the instance's property values using the logger
    ECOBJECTS_EXPORT virtual void         _Dump () const = 0;
    ECOBJECTS_EXPORT virtual void         _Free () = 0;
    
public:
    ECOBJECTS_EXPORT EnablerCP            GetEnabler() const; // WIP_FUSION: Should this return an EnablerCR?
    ECOBJECTS_EXPORT std::wstring         GetInstanceID() const;
    ECOBJECTS_EXPORT bool                 IsReadOnly() const;
    
    ECOBJECTS_EXPORT ClassCP              GetClass() const;
    ECOBJECTS_EXPORT StatusInt            GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt            GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 index) const;
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 index);
        
    StatusInt   InsertArrayElement (const wchar_t * propertyAccessString, ValueCR v, UInt32 index); //WIP_FUSION Return the new count?
    StatusInt   RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index); //WIP_FUSION return the removed one? YAGNI? Return the new count?
    StatusInt   ClearArray (const wchar_t * propertyAccessString);    
    
    //WIP_FUSION AccessStringAndNIndicesAgree should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    ECOBJECTS_EXPORT static bool          AccessStringAndNIndicesAgree (const wchar_t * propertyAccessString, UInt32 nIndices, bool assertIfFalse);
    
    // These are more than just convenience methods... they enable an access pattern 
    // from managed code that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT StatusInt GetInteger (int & value, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetDouble (double & value, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetString (const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT void      Dump () const;
    //! Call this instead of "delete" to ensure that the EC::Instance is freed from the same heap that it was allocated.
    //! This method does not affect the persisted EC::Instance (if it has been persisted)
    ECOBJECTS_EXPORT void      Free ();
    
    };
    
//! EC::RelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! @see Instance, RelationshipClass
struct RelationshipInstance : public Instance
    {
private:
    //needswork: needs EC::Instance Source/Target
    };
        
END_BENTLEY_EC_NAMESPACE
