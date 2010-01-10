/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECInstance.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::IECInstance()
    {
    size_t sizeofInstance = sizeof(IECInstance);
    size_t sizeofVoid = sizeof (void*);
    
    assert (sizeof(IECInstance) == sizeof (void*) && L"Increasing the size or memory layout of the base EC::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring        IECInstance::GetInstanceId() const
    {
    return _GetInstanceId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR             IECInstance::GetClass() const 
    {
    ECEnablerCR enabler = GetEnabler();
        
    return enabler.GetClass();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
bool                IECInstance::AccessStringAndNIndicesAgree (const wchar_t * propertyAccessString, UInt32 nIndices, bool errorIfFalse)
    {
    const wchar_t * pointerToBrackets = pointerToBrackets = wcsstr (propertyAccessString, L"[]"); ;
    int nBrackets = 0;
    while (NULL != pointerToBrackets)
        {
        nBrackets++;
        pointerToBrackets += 2; // skip past the brackets
        pointerToBrackets = wcsstr (pointerToBrackets, L"[]"); ;
        }
    
    assert (!errorIfFalse || (nIndices == nBrackets && "nIndices must match the number of brackets '[]' found in the propertyAccessString"));
    //WIP_FUSION log this as an error if errorIfFalse!
    
    return (nIndices == nBrackets);
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECEnablerCR           IECInstance::GetEnabler() const
    {
    return _GetEnabler();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
bool                IECInstance::IsReadOnly() const
    {
    return _IsReadOnly();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    return _GetValue (v, propertyAccessString, nIndices, indices);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    return _SetValue (propertyAccessString, v, nIndices, indices);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const
    {
    return _GetValue (v, propertyAccessString, 1, &index);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index)
    {
    return _SetValue (propertyAccessString, v, 1, &index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StatusInt           IECInstance::GetInteger (int & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    ECValue v;
    StatusInt status = _GetValue (v, propertyAccessString, 0, NULL);
    value = v.GetInteger();
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
StatusInt           IECInstance::GetDouble (double& value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    ECValue v;
    StatusInt status = _GetValue (v, propertyAccessString, 0, NULL);
    value = v.GetDouble();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::GetString (const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    ECValue v;
    StatusInt status = _GetValue (v, propertyAccessString, 0, NULL);
    value = v.GetString();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::InsertArrayElement (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::ClearArray (const wchar_t * propertyAccessString)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                IECInstance::Dump () const
    {
    _Dump();
    }
 
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                IECInstance::Free ()
    {
    _Free();
    }
END_BENTLEY_EC_NAMESPACE
