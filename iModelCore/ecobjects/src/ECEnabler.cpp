/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnabler.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <typeinfo>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::ECEnabler(ECClassCR ecClass, IStandaloneEnablerLocaterP structStandaloneEnablerLocater) : m_ecClass (ecClass), m_standaloneInstanceEnablerLocater (structStandaloneEnablerLocater)
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnabler::~ECEnabler() 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr          ECEnabler::_LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className)  
    {
    if (NULL != m_standaloneInstanceEnablerLocater)
        return m_standaloneInstanceEnablerLocater->LocateStandaloneEnabler (schemaKey, className);
    
    ECSchemaCP schema = m_ecClass.GetSchema().FindSchema(schemaKey, SCHEMAMATCHTYPE_Exact);//TODO: Test change of behavior we need to match schemas exactly:Abeesh
    if (NULL == schema)
        return NULL;

    ECClassCP ecClass = schema->GetClassCP(className);
    if (NULL == ecClass)
        return NULL;
    
    return ecClass->GetDefaultStandaloneEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr          ECEnabler::GetEnablerForStructArrayMember (SchemaKeyCR schemaName, Utf8CP className)  
    {
    return _LocateStandaloneEnabler (schemaName, className); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnabler::_IsPropertyReadOnly (uint32_t propertyIndex) const
    {
    // Note: this is the default implementation, and it causes significant performance issues because every call to IECInstance::SetValue() checks if the property is read-only
    // Subclasses are capable of implementing this much more efficiently
    ECPropertyCP ecproperty = LookupECProperty (propertyIndex);
    BeAssert (NULL != ecproperty);
    return NULL == ecproperty || ecproperty->GetIsReadOnly();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECEnabler::_LookupECProperty (uint32_t propertyIndex) const
    {
    Utf8CP accessString;
    return ECOBJECTS_STATUS_Success == GetAccessString (accessString, propertyIndex) ? _LookupECProperty (accessString) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPropertyCP propertyFromAccessString (ECClassCR ecClass, Utf8CP accessString)
    {
    Utf8CP dot = strchr (accessString, '.');
    if (NULL == dot)
        {
        return ecClass.GetPropertyP (accessString);
        }
    else
        {
        Utf8String structName (accessString, dot);
        ECPropertyCP prop = ecClass.GetPropertyP (structName.c_str());
        if (NULL == prop)
            { BeAssert (false); return NULL; }
        StructECPropertyCP structProp = prop->GetAsStructProperty();
        if (NULL == structProp)
            { BeAssert (false); return NULL; }
        
        return propertyFromAccessString (structProp->GetType(), dot+1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECEnabler::_LookupECProperty (Utf8CP accessString) const
    {
    return propertyFromAccessString (GetClass(), accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR           ECEnabler::GetClass() const  { return m_ecClass; }
Utf8CP              ECEnabler::GetName() const { return _GetName(); }
ECObjectsStatus     ECEnabler::GetPropertyIndex (uint32_t& propertyIndex, Utf8CP accessString) const { return _GetPropertyIndex (propertyIndex, accessString); }

ECObjectsStatus     ECEnabler::GetAccessString  (Utf8CP& accessString, uint32_t propertyIndex) const { return _GetAccessString  (accessString, propertyIndex); }
uint32_t            ECEnabler::GetFirstPropertyIndex (uint32_t parentIndex) const { return _GetFirstPropertyIndex (parentIndex); }
uint32_t            ECEnabler::GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const { return _GetNextPropertyIndex (parentIndex, inputIndex); }
bool                ECEnabler::HasChildProperties (uint32_t parentIndex) const { return _HasChildProperties (parentIndex); }
uint32_t            ECEnabler::GetParentPropertyIndex (uint32_t childIndex) const { return _GetParentPropertyIndex (childIndex); }
ECObjectsStatus     ECEnabler::GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const{ return _GetPropertyIndices (indices, parentIndex); };
IStandaloneEnablerLocaterR ECEnabler::GetStandaloneEnablerLocater() { return *this; }
ECPropertyCP        ECEnabler::LookupECProperty (uint32_t propertyIndex) const { return _LookupECProperty (propertyIndex); }
ECPropertyCP        ECEnabler::LookupECProperty (Utf8CP accessString) const { return _LookupECProperty (accessString); }
bool                ECEnabler::IsPropertyReadOnly (uint32_t propertyIndex) const { return _IsPropertyReadOnly (propertyIndex); }

#if defined (EXPERIMENTAL_TEXT_FILTER)
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

    for (ECPropertyCP prop: instance.GetClass().GetProperties())
        {
        ECValue v;
        instance.GetValue (v, prop->GetName().c_str());
    
        if (v.IsNull())
            continue;

        if (v.IsPrimitive())
            {
            if (allTypes || v.GetPrimitiveType() == primitiveType)
                {
                anyCandidates = true;
                if (proc._ProcessPrimitiveProperty (instance, prop->GetName().c_str(), v))
/*<==*/             return PROPERTY_PROCESSING_RESULT_Hit;
                }
            }
        else if (v.GetIsStruct() ())
            {
            if (ProcessStructProperty (failedClasses, noCandidateInAnyStruct, v, primitiveType, proc, opts))
/*<==*/         return PROPERTY_PROCESSING_RESULT_Hit;
            }
        else
            {
            BeAssert (v.IsArray());
            ArrayInfo ai = v.GetArrayInfo ();
            bool isStructArray = ai.IsStructArray();

            if (!isStructArray && !allTypes && ai.GetElementPrimitiveType() != primitiveType)
                continue;

            for (uint32_t idx = 0, count = ai.GetCount(); idx < count; ++idx)
                {
                ECValue vitem;
                instance.GetValue (vitem, prop->GetName().c_str(), idx);
                
                bool foundOne;

                if (isStructArray)
                    foundOne = ProcessStructProperty (failedClasses, noCandidateInAnyStruct, vitem, primitiveType, proc, opts);
                else
                    {
                    anyCandidates = true;
                    foundOne = proc._ProcessPrimitiveProperty (instance, prop->GetName().c_str(), vitem);
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
#endif // defined (EXPERIMENTAL_TEXT_FILTER)

 /////////////////////////////////////////////////////////////////////////////////////////
#if defined (_MSC_VER)
    #pragma region IECRelationshipEnabler
#endif // defined (_MSC_VER)
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECWipRelationshipInstancePtr  IECRelationshipEnabler::CreateWipRelationshipInstance() const
     {
     return _CreateWipRelationshipInstance ();
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCR       IECRelationshipEnabler::GetRelationshipClass() const
    {
    return _GetRelationshipClass ();
    }

#if defined (_MSC_VER)
    #pragma endregion //IECRelationshipEnabler
#endif // defined (_MSC_VER)
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//ECObjectsStatus PropertyIndexedEnabler::_GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const
//    {
//    for (UInt32 index = 0; index < m_propertyCount; ++index)
//        {
//        if (0 == BeStringUtilities::Wcsicmp (propertyAccessString, m_propertyNameList[index]))
//            {
//            propertyIndex = index + 1;
//            return ECOBJECTS_STATUS_Success;
//            }
//        }
//    return ECOBJECTS_STATUS_InvalidPropertyAccessString;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
// ECObjectsStatus PropertyIndexedEnabler::_GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const
//    {
//    if (propertyIndex > m_propertyCount || propertyIndex <= 0)
//        return ECOBJECTS_STATUS_IndexOutOfRange;
//
//    propertyAccessString = m_propertyNameList[propertyIndex -1];
//    return ECOBJECTS_STATUS_Success;
//    }
// 
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
// UInt32         PropertyIndexedEnabler::_GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const 
//    {
//    if (inputIndex> 0 && inputIndex <= m_propertyCount)
//        return ++inputIndex;
//            
//    return 0;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//ECObjectsStatus PropertyIndexedEnabler::_GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const
//    {
//    for (UInt32 index = 0; index < m_propertyCount; ++index)
//        indices.push_back(index + 1);
//    return ECOBJECTS_STATUS_Success;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//UInt32          PropertyIndexedEnabler::_GetFirstPropertyIndex (UInt32 parentIndex) const
//    {
//    return 1;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Abeesh.Basheer                  04/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool            PropertyIndexedEnabler::_HasChildProperties (UInt32 parentIndex) const
//    {
//    return false;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyIndexFlatteningIterator::PropertyIndexFlatteningIterator (ECEnablerCR enabler)
    : m_enabler (enabler)
    {
    m_states.push_back (State());
    m_states.back().Init (m_enabler, 0);
    InitForCurrent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyIndexFlatteningIterator::InitForCurrent()
    {
    if (m_states.back().IsEnd())
        {
        m_states.pop_back();
        if (m_states.empty())
            return false;

        ++m_states.back().m_listIndex;
        return InitForCurrent();
        }
    
    uint32_t curPropIdx = m_states.back().GetPropertyIndex();
    if (!m_enabler.HasChildProperties (curPropIdx))
        return true;

    m_states.push_back (State());
    if (!m_states.back().Init (m_enabler, curPropIdx))
        {
        m_states.pop_back();
        if (m_states.empty())
            return false;

        ++m_states.back().m_listIndex;
        return InitForCurrent();
        }
    else
        return InitForCurrent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyIndexFlatteningIterator::GetCurrent (uint32_t& propIdx) const
    {
    if (m_states.empty())
        return false;
    else
        {
        propIdx = m_states.back().GetPropertyIndex();
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyIndexFlatteningIterator::MoveNext()
    {
    if (m_states.empty()) // no more property indices
        return false;

    ++m_states.back().m_listIndex;
    return InitForCurrent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyIndexFlatteningIterator::State::Init (ECEnablerCR enabler, uint32_t parentPropertyIndex)
    {
    m_listIndex = 0;
    m_propertyIndices.clear();
    return ECOBJECTS_STATUS_Success == enabler.GetPropertyIndices (m_propertyIndices, parentPropertyIndex) && !m_propertyIndices.empty();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
    
