/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECInstance.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Instance::Instance(EnablerCR enabler) : m_enabler (&enabler)
    {
    size_t sizeofInstance = sizeof(Instance);
    size_t sizeofVoid = sizeof (void*);
    
    ECAssert (sizeof(Instance) == 2 * sizeof (void*) && L"Increasing the size or memory layout of the base EC::Instance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool        Instance::IsReadOnly() const
    {        
    return (NULL != dynamic_cast<ISetValueCP>(GetEnabler()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassCP     Instance::GetClass() const 
    {
    ECAssert(m_enabler);
    if (NULL == m_enabler)
        return NULL;
        
    return m_enabler->GetClass();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
bool Instance::AccessStringAndNIndicesAgree (const wchar_t * propertyAccessString, UInt32 nIndices, bool errorIfFalse)
    {
    const wchar_t * pointerToBrackets = pointerToBrackets = wcsstr (propertyAccessString, L"[]"); ;
    int nBrackets = 0;
    while (NULL != pointerToBrackets)
        {
        nBrackets++;
        pointerToBrackets += 2; // skip past the brackets
        pointerToBrackets = wcsstr (pointerToBrackets, L"[]"); ;
        }
    
    ECAssert (!errorIfFalse || (nIndices == nBrackets && "nIndices must match the number of brackets '[]' found in the propertyAccessString"));
    //WIP_FUSION log this as an error if errorIfFalse!
    
    return (nIndices == nBrackets);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Instance::GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    //WIP_FUSION add a CheckForNull (propertyAccessString) macro here that logs error and triggers debugger in a debug build
    //WIP_FUSION in debug mode could find and validate ECProperty here
    
    if (!AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true)) // .04
        return ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices;
            
    EnablerCP e = GetEnabler();
    ECAssert (NULL != e);
    
    IGetValueCP enabler = e->GetIGetValue(); // replaces a dynamic_cast that was costing .38. Now about .01
    ECAssert (NULL != enabler);
    
    return enabler->GetValue (v, *this, propertyAccessString, nIndices, indices); // .36  (now less expensive since one dynamic_cast is avoided internally
    // Now about .21
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Instance::SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    //WIP_FUSION in debug mode we could find and validate ECProperty here
    
    if (!AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true))
        return ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices;
    
    ISetValueCP enabler = dynamic_cast<ISetValueCP>(GetEnabler());
    return enabler->SetValue (*this, propertyAccessString, v, nIndices, indices);
    }        

#ifdef WIP_MANAGEDACCESS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 Instance::JustIntArgs (UInt32 nIndices, UInt32 const * indices) const
    {
    return 24;
    }
    
UInt32 Instance::NoArgs () const
    {
    return 24;
    }
    
UInt32 Instance::GetInteger (const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    Bentley::EC::Value v;                                  // .07
    
    GetValue (v, propertyAccessString, nIndices, indices); // .80
    
    return v.GetInteger();                                 // .01
    }    
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Instance::InsertArrayElement (const wchar_t * propertyAccessString, ValueCR v, UInt32 index)
    {
    IArrayManipulatorCP manipulator = dynamic_cast<IArrayManipulatorCP>(GetEnabler());
    if (NULL == manipulator)
        return ECOBJECTS_STATUS_OperationNotSupportedByEnabler;
        
    PropertyP property;
    StatusInt status = GetClass()->GetProperty (property, propertyAccessString);
    if (status != SUCCESS)
        return status;
        
    return manipulator->InsertArrayElement(*this, propertyAccessString, v, index);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Instance::RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    IArrayManipulatorCP manipulator = dynamic_cast<IArrayManipulatorCP>(GetEnabler());
    if (NULL == manipulator)
        return ECOBJECTS_STATUS_OperationNotSupportedByEnabler;
        
    PropertyP property;
    StatusInt status = GetClass()->GetProperty (property, propertyAccessString);
    if (status != SUCCESS)
        return status;
        
    return manipulator->RemoveArrayElement(*this, propertyAccessString, index);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Instance::ClearArray (const wchar_t * propertyAccessString)
    {
    IArrayManipulatorCP manipulator = dynamic_cast<IArrayManipulatorCP>(GetEnabler());
    if (NULL == manipulator)
        return ECOBJECTS_STATUS_OperationNotSupportedByEnabler;
        
    PropertyP property = NULL;
    StatusInt status = GetClass()->GetProperty (property, propertyAccessString);
    if (status != SUCCESS)
        return status;
        
    return manipulator->ClearArray(*this, propertyAccessString);
    }     
 
END_BENTLEY_EC_NAMESPACE
