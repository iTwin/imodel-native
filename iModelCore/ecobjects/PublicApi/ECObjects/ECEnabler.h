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

BEGIN_BENTLEY_EC_NAMESPACE

/*=================================================================================**//**
* Interface implemented by EC::Enablers that can GetValues from an EC::Instance
* (which is essentially all of them, but the interface is kept separate for consistency
* with the other "small" interfaces and to allow the possibiliy of implementing this
* without being an EC::Enabler.)
*
* @see Enabler
* @bsistruct                                                     CaseyMullen    09/09
+===============+===============+===============+===============+===============+======*/
struct IGetValue
    {
private:
public:
    virtual StatusInt GetValue (ECValueR v, ECInstanceCR instance, const wchar_t * propertyAccessString, 
                                UInt32 nIndices = 0, UInt32 const * indices = NULL) const = 0;
    };

/*=================================================================================**//**
* Interface implemented by EC::Enablers that can SetValues from an EC::Instance
* (which is essentially all of them, but the interface is kept separate for consistency
* with the other "small" interfaces and to allow the possibiliy of implementing this
* without being an EC::Enabler.)
*
* @see Enabler
* @bsistruct                                                     CaseyMullen    09/09
+===============+===============+===============+===============+===============+======*/
struct ISetValue
    {
private:
public:
    virtual StatusInt SetValue (ECInstanceR instance, const wchar_t * propertyAccessString, ECValueCR v,
                                UInt32 nIndices = 0, UInt32 const * indices = NULL) const = 0;
    };

/*=================================================================================**//**
* Primary interface for a struct that enables EC functionality. 
* This interface only supports identification and registration of enablers.
* The implementing struct will likely implement other EC "enabler" interfaces like
* IGetValue, ISetValue, IArrayManipulator, etc.
*
* The "contract" for being a well-behaved EC::Enabler
* - If passed an EC::Instance with an implementation that you don't handle 
*   (e.g. you need a certain subclass), you must return ERROR_ECInstanceImplementationNotSupported
* - If passed an EC::Instance of an EC::Class that you don't handle, return ERROR_ECClassNotSupported
* - If passed an EC::Instance of an EC::Schema that you don't handle, return ERROR_ECSchemaNotSupported
* - If asked about a property that you don't handle or is not in the EC::Class, return ERROR_PropertyNotFound
* - The caller is responsible for pass EC::Values of the correct DataType. 
*   If you detect a mismatch, return ERROR_DataTypeMismatch
*   correct DataType. If you return an error status in such a case
*
* @see IGetValue, ISetValue, ICreateInstance, IDeleteInstance, IArrayManipulator, IArrayAccessor, etc.
* @bsistruct                                                     CaseyMullen    09/09
+===============+===============+===============+===============+===============+======*/
struct Enabler
    {
private:
    bool                    m_initialized;
    ECIGetValueCP           m_iGetValue;
public:
    ECOBJECTS_EXPORT Enabler(): m_initialized(false), m_iGetValue(NULL) {};
    ECOBJECTS_EXPORT void                    Initialize();
    ECOBJECTS_EXPORT virtual UInt32          GetId()   const = 0;  // From Linkage/Handler ID Pool
    ECOBJECTS_EXPORT virtual const wchar_t * GetName() const = 0;  // Mostly for debugging info 
    
    ECOBJECTS_EXPORT ECIGetValueCP           IGetValue() const;
    };

/*=================================================================================**//**
* Implemented by enablers that support manipulation of array properties, i.e. operations
* that change the count of elements.
*
* Contract:
* - For all of the methods, the propertyAccessString should be in the "array element" form, 
*   e.g. "Aliases[]" instead of "Aliases" 
* @see IArrayAccessor, Enabler
* @bsistruct                                                     CaseyMullen    09/09
+===============+===============+===============+===============+===============+======*/
struct IArrayManipulator
    {
private:
public:

    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt InsertArrayElement (ECInstanceR instance, const wchar_t * propertyAccessString, ECValueCR value, UInt32 index) const = 0;
    
    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt RemoveArrayElement (ECInstanceR instance, const wchar_t * propertyAccessString, UInt32 index) const = 0;
    
    // @param propertyAccessString should be in the "array element" form, e.g. "Aliases[]" instead of "Aliases"
    virtual StatusInt ClearArray (ECInstanceR instance, const wchar_t * propertyAccessString) const = 0;    
    };    


/*=================================================================================**//**
* Implemented by enablers that support creation of new standalone (non-persisted) EC::Instances
* This one should live in Bentley::EC
* @see Enabler
* @bsistruct                                                     CaseyMullen    09/09
+===============+===============+===============+===============+===============+======*/
struct ICreateInstance
    {
private:
public:
    virtual StatusInt CreateInstance (ECInstanceP& instance, ECClassCR ecClass, const wchar_t * instanceId) const = 0;
    };    

//wip: ICacheValues : ReloadCachedValues, SaveCachedValues, FreeCachedValues
END_BENTLEY_EC_NAMESPACE
