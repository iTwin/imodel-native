/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnabler.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    
    ECSchemaCP schema = m_ecClass.GetSchema().FindSchema(schemaKey, SchemaMatchType::Exact);//TODO: Test change of behavior we need to match schemas exactly:Abeesh
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
    return ECObjectsStatus::Success == GetAccessString (accessString, propertyIndex) ? _LookupECProperty (accessString) : NULL;
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
    return ECObjectsStatus::Success == enabler.GetPropertyIndices (m_propertyIndices, parentPropertyIndex) && !m_propertyIndices.empty();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
    
