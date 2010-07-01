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
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::~ECEnabler() 
    {
    Logger::GetLogger()->tracev (L"%S(%s) at 0x%x is being destructed.", typeid(*this).name(), m_ecClass.GetName().c_str(), this);
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
ECClassCR           ECEnabler::GetClass() const  { return m_ecClass; }
wchar_t const *     ECEnabler::GetName() const { return _GetName(); }
StatusInt           ECEnabler::GetPropertyIndex (UInt32& propertyIndex, const wchar_t * accessString) const { return _GetPropertyIndex (propertyIndex, accessString); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnabler::ProcessStructProperty (bset<ECClassCP>& failedClasses, bool& noCandidateInAnyStruct, ECValueCR propValue, PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const
    {
    IECInstancePtr svalue = propValue.GetStruct ();

    if (failedClasses.find (&svalue->GetClass()) != failedClasses.end())
        return false;

    PropertyProcessingResult result = svalue->GetEnabler().ProcessPrimitiveProperties (failedClasses, *svalue, primitiveType, proc, opts);

    if (PROPERTY_PROCESSING_RESULT_NoCandidates == result)
        failedClasses.insert (&svalue->GetClass());
    else
        noCandidateInAnyStruct = false;

    return (PROPERTY_PROCESSING_RESULT_Hit == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::PropertyProcessingResult ECEnabler::_ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR instance, PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const
    {
    if (failedClasses.find (&instance.GetClass()) != failedClasses.end())
        return PROPERTY_PROCESSING_RESULT_NoCandidates;

    bool noCandidateInAnyStruct = true;
    bool anyCandidates = false;
    bool allTypes = (opts & PROPERTY_PROCESSING_OPTIONS_AllTypes) != 0;

    for each (ECPropertyCP prop in instance.GetClass().GetProperties())
        {
        ECValue v;
        instance.GetValue (v, prop->Name.c_str());
    
        if (v.IsNull())
            continue;

        if (v.IsPrimitive())
            {
            if (allTypes || v.GetPrimitiveType() == primitiveType)
                {
                anyCandidates = true;
                if (proc._ProcessPrimitiveProperty (instance, prop->Name.c_str(), v))
/*<==*/             return PROPERTY_PROCESSING_RESULT_Hit;
                }
            }
        else if (v.IsStruct ())
            {
            if (ProcessStructProperty (failedClasses, noCandidateInAnyStruct, v, primitiveType, proc, opts))
/*<==*/         return PROPERTY_PROCESSING_RESULT_Hit;
            }
        else
            {
            assert (v.IsArray());
            ArrayInfo ai = v.GetArrayInfo ();
            bool isStructArray = ai.IsStructArray();

            if (!isStructArray && !allTypes && ai.GetElementPrimitiveType() != primitiveType)
                continue;

            for (UInt32 idx = 0, count = ai.GetCount(); idx < count; ++idx)
                {
                ECValue vitem;
                instance.GetValue (vitem, prop->Name.c_str(), idx);
                
                bool foundOne;

                if (isStructArray)
                    foundOne = ProcessStructProperty (failedClasses, noCandidateInAnyStruct, vitem, primitiveType, proc, opts);
                else
                    {
                    anyCandidates = true;
                    foundOne = proc._ProcessPrimitiveProperty (instance, prop->Name.c_str(), vitem);
                    }

                if (foundOne)
/*<==*/             return PROPERTY_PROCESSING_RESULT_Hit;
                }
            }
        }

    if (!anyCandidates && noCandidateInAnyStruct)
        {
        failedClasses.insert (&instance.GetClass());
        return PROPERTY_PROCESSING_RESULT_NoCandidates;
        }

    return PROPERTY_PROCESSING_RESULT_Miss;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::PropertyProcessingResult ECEnabler::ProcessPrimitiveProperties (bset<ECClassCP>& a, IECInstanceCR b, PrimitiveType c, IPropertyProcessor const& d, PropertyProcessingOptions e) const {return _ProcessPrimitiveProperties(a,b,c,d,e);}

END_BENTLEY_EC_NAMESPACE
    