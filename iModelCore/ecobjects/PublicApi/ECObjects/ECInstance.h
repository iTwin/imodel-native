/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstance.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE

//! EC::IECInstance is the native equivalent of a .NET IECInstance.
//! Unlike IECInstance, it is not a pure interface, but is a concrete struct.
//! Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
//! in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a class that implements
//! the EC::ECEnabler interface and one or more related interfaces to supply functionality 
//! to the EC::IECInstance.
//! We could call these "enabled" instances as opposed to "lightweight".
//! @see ECEnabler
struct IECInstance
    {
private:
protected:    
    ECOBJECTS_EXPORT IECInstance(); 
    ECOBJECTS_EXPORT ~IECInstance(){};
    ECOBJECTS_EXPORT virtual std::wstring _GetInstanceId() const = 0; // Virtual and returning std::wstring because a subclass may want to calculate it on demand
    ECOBJECTS_EXPORT virtual StatusInt    _GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const = 0;
    ECOBJECTS_EXPORT virtual StatusInt    _SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices = 0, UInt32 const * indices = NULL) = 0;
    ECOBJECTS_EXPORT virtual ECEnablerCR  _GetEnabler() const = 0;
    ECOBJECTS_EXPORT virtual bool         _IsReadOnly() const = 0;
    //! This should dump the instance's property values using the logger
    ECOBJECTS_EXPORT virtual void         _Dump () const = 0;
    ECOBJECTS_EXPORT virtual void         _Free () = 0;
    
public:
    ECOBJECTS_EXPORT ECEnablerCR          GetEnabler() const;
    ECOBJECTS_EXPORT std::wstring         GetInstanceId() const;
    ECOBJECTS_EXPORT bool                 IsReadOnly() const;
    
    ECOBJECTS_EXPORT ECClassCR            GetClass() const;
    ECOBJECTS_EXPORT StatusInt            GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt            GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const;
    ECOBJECTS_EXPORT StatusInt            SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index);
        
    // AZK Are these not exported just because we have not yet worked out the details?
    //! Contract:
    //! - For all of the methods, the propertyAccessString should be in the "array element" form, 
    //!   e.g. "Aliases[]" instead of "Aliases"         
    StatusInt   InsertArrayElement (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index); //WIP_FUSION Return the new count?
    StatusInt   RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index); //WIP_FUSION return the removed one? YAGNI? Return the new count?
    StatusInt   ClearArray (const wchar_t * propertyAccessString);    
    
    //WIP_FUSION ParseExpectedNIndices should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    ECOBJECTS_EXPORT static int          ParseExpectedNIndices (const wchar_t * propertyAccessString);
    
    // These are more than just convenience methods... they enable an access pattern 
    // from managed code that can get a value with only one managed to native transition    
    ECOBJECTS_EXPORT StatusInt GetInteger (int & value,             const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetDouble  (double & value,          const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt GetString  (const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    ECOBJECTS_EXPORT StatusInt SetIntegerValue (const wchar_t * propertyAccessString, int value,             UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT StatusInt SetStringValue  (const wchar_t * propertyAccessString, const wchar_t * value, UInt32 nIndices = 0, UInt32 const * indices = NULL);
    ECOBJECTS_EXPORT void      Dump () const;

    //AZK not clear to me why we need this?  Is it fragile?  Can we just override delete?
    //! Call this instead of "delete" to ensure that the EC::IECInstance is freed from the same heap that it was allocated.
    //! This method does not affect the persisted EC::IECInstance (if it has been persisted)
    ECOBJECTS_EXPORT void      Free ();
    
    };
    
//! EC::IECRelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
//! @see IECInstance, ECRelationshipClass
struct IECRelationshipInstance : public IECInstance
    {
private:
    //needswork: needs EC::IECInstance Source/Target
    };
        
END_BENTLEY_EC_NAMESPACE
