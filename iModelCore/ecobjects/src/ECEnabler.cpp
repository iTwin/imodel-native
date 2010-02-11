/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECEnabler.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::ECEnabler(ECClassCR ecClass) : m_privateRefCount(0), m_ecClass (ecClass) 
    {
    ECSchemaR schema = const_cast<ECSchemaR>(ecClass.Schema);
    schema.AddRef();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::~ECEnabler() 
    {
    Logger::GetLogger()->tracev (L"%S(%s) at 0x%x is being destructed.", typeid(*this).name(), m_ecClass.GetName().c_str(), this);

    ECSchemaR schema = const_cast<ECSchemaR>(m_ecClass.Schema);
    schema.Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     02/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32      ECEnabler::AddRef()
    {
    m_privateRefCount++;
    Logger::GetLogger()->tracev (L"++(%d)%S(%s) Refcount increased to %d.", m_privateRefCount, typeid(*this).name(), m_ecClass.GetName().c_str(), m_privateRefCount);
    
    return RefCountedBase::AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     02/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32      ECEnabler::Release()
    { 
    --m_privateRefCount;
    Logger::GetLogger()->tracev (L"--(%d)%S(%s) Refcount decreased to %d.", m_privateRefCount, typeid(*this).name(), m_ecClass.GetName().c_str(), m_privateRefCount);
    return RefCountedBase::Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const * ECEnabler::GetName() const
    {
    return _GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR ECEnabler::GetClass() const 
    {
    return m_ecClass;
    }

END_BENTLEY_EC_NAMESPACE
    