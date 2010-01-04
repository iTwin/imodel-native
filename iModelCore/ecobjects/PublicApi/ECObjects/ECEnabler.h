/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECEnabler.h $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>
#include <Bentley\RefCounted.h>

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<Enabler>                  EnablerPtr;

//! Primary interface for a struct that enables EC functionality. 
//! This interface only supports identification and registration of enablers.
//! The implementing struct will likely implement other EC "enabler" interfaces like
//! IArrayManipulator, etc.
//!
//! The "contract" for being a well-behaved EC::Enabler
//! - If passed an EC::Instance with an implementation that you don't handle 
//!   (e.g. you need a certain subclass), you must return ERROR_ECInstanceImplementationNotSupported
//! - If passed an EC::Instance of an EC::Class that you don't handle, return ERROR_ECClassNotSupported
//! - If passed an EC::Instance of an EC::Schema that you don't handle, return ERROR_ECSchemaNotSupported
//! - If asked about a property that you don't handle or is not in the EC::Class, return ERROR_PropertyNotFound
//! - The caller is responsible for pass EC::Values of the correct DataType. 
//!   If you detect a mismatch, return ERROR_DataTypeMismatch
//!   correct DataType. If you return an error status in such a case
//!
//! @see ICreateInstance, IDeleteInstance, IArrayManipulator, IArrayAccessor, etc.
struct Enabler : RefCountedBase
    {
private:
    UInt32                  m_id;    
    std::wstring            m_name;
    ClassCP                 m_ecClass;

    // Hide these as part of the RefCounted pattern
    Enabler(){};
    
protected:
    //! Protected as part of the RefCounted pattern
    ~Enabler(){};

    //! Subclasses of Enabler should implement a FactoryMethod to construct the enabler, as
    //! part of the RefCounted pattern.
    //! It should be of the form:
    //! /code
    //!   static ____EnablerPtr Create(ClassCR ecClass)
    //!       {
    //!       return new ____Enabler (ecClass);    
    //!       };
    //! /endcode
    //! where the ____ is a name specific to your subclass.
    ECOBJECTS_EXPORT Enabler(ClassCR ecClass, UInt32 enablerId, std::wstring name) : m_ecClass (&ecClass), m_name(name), 
        m_id(enablerId) {};

public:
    
    //! Should be obtained from the Linkage/Handler ID Pool
    ECOBJECTS_EXPORT inline UInt32          GetId()   const { return m_id; }
    
    //! Primarily for debugging/logging purposes. Should match your fully-qualified class name
    ECOBJECTS_EXPORT inline wchar_t const * GetName() const { return m_name.c_str(); }
    
    ECOBJECTS_EXPORT inline ClassCP         GetClass() const { return m_ecClass; }
    };

//! Implemented by enablers that support manipulation of array properties, i.e. operations
//! that change the count of elements.
//!
//! Contract:
//! - For all of the methods, the propertyAccessString should be in the "array element" form, 
//!   e.g. "Aliases[]" instead of "Aliases" 
//! @see IArrayAccessor, Enabler
struct IArrayManipulator  // WIP_FUSION: these responsibilities should move to the Instance, just like GetValue and SetValue moved
    {
    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt InsertArrayElement (InstanceR instance, const wchar_t * propertyAccessString, ValueCR value, UInt32 index) const = 0;
    
    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt RemoveArrayElement (InstanceR instance, const wchar_t * propertyAccessString, UInt32 index) const = 0;
    
    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt ClearArray (InstanceR instance, const wchar_t * propertyAccessString) const = 0;    
    };    


//! Implemented by enablers that support creation of new standalone (non-persisted) EC::Instances
//! @see Enabler
struct ICreateInstance // WIP_FUSION: needs to change
    {
    virtual StatusInt CreateInstance (InstanceP& instance, ClassCR ecClass, const wchar_t * instanceId) const = 0;
    };    

//WIP_FUSION ICacheValues : ReloadCachedValues, SaveCachedValues, FreeCachedValues
END_BENTLEY_EC_NAMESPACE
