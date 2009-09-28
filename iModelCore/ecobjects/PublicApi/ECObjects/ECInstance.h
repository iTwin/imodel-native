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
#include <string>

BEGIN_BENTLEY_EC_NAMESPACE
    
/*=================================================================================**//**
* EC::Instance is the native equivalent of a .NET IECInstance.
* Unlike IECInstance, it is not a pure interface, but is a concrete struct.
* Whereas in .NET, one might implement IECInstance, or use the "Lightweight" system
* in Bentley.ECObjects.Lightweight, in native "ECObjects" you write a struct that implements
* the EC::Enabler interface and one or more related interfaces to supply functionality 
* to the EC::Instance.
* We could call these "enabled" instances as opposed to "lightweight".
* @see Enabler
* @bsistruct                                                     
+===============+===============+===============+===============+===============+======*/    
struct Instance
    {
private:
    ECEnablerCP         m_enabler;
    std::wstring        m_instanceId;
    ECClassCP           m_class; //needs a refcounted ptr

    //wip: AccessStringAndNIndicesAgree should move to AccessStringHelper struct... along with method to convert to/from .NET ECObjects style accessString
    static bool AccessStringAndNIndicesAgree (const wchar_t * propertyAccessString, UInt32 nIndices, bool assertIfFalse);
    
protected:    
    Instance() {}; 

    ECEnablerCP   GetEnabler() const;    
    
public:
    Instance(ECEnablerCR enabler, ECClassCR ecClass, const wchar_t * instanceId);
    Instance(ECEnablerCR enabler, ECClassCR ecClass);
    
    virtual const wchar_t * GetInstanceId() const; // A subclass may want to calculate it on demand
    bool        IsReadOnly() const;
    
    ECClassCP   GetClass() const;
    
    StatusInt   GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    StatusInt   SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices = 0, UInt32 const * indices = NULL);
    
    StatusInt   InsertArrayElement (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index); //wip: Return the new count?
    StatusInt   RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index); //wip: return the removed one? YAGNI? Return the new count?
    StatusInt   ClearArray (const wchar_t * propertyAccessString);    
    
#ifdef WIP_MANAGEDACCESS
    // These are more than just convenience methods... they enable an access pattern 
    // from managed code that can get a value with only one managed to native transition
    StatusInt GetDouble (double value, const wchar_t * propertyAccessString); // marshal as few args as possible 
    StatusInt GetDouble (double value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices); // marshal as few args as possible 
    
    UInt32 GetInteger (const wchar_t * propertyAccessString, UInt32 nIndices = 0, UInt32 const * indices = NULL) const; // marshal as few args as possible     
    UInt32 JustIntArgs (UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    UInt32 NoArgs () const;
#endif

    // needswork: experiment with the best way to return strings in one shot
    };
    
/*=================================================================================**//**
* EC::RelationshipInstance is the native equivalent of a .NET IECRelationshipInstance.
* @see Instance, RelationshipClass
* @bsistruct                                                     
+===============+===============+===============+===============+===============+======*/    
struct RelationshipInstance : public Instance
    {
private:
    //needswork: needs EC::Instance Source/Target
    };
        
END_BENTLEY_EC_NAMESPACE