/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnabler.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <typeinfo>

BEGIN_BENTLEY_EC_NAMESPACE

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
StandaloneECEnablerPtr          ECEnabler::_LocateStandaloneEnabler (SchemaKeyCR schemaKey, WCharCP className)  
    {
    if (NULL != m_standaloneInstanceEnablerLocater)
        return m_standaloneInstanceEnablerLocater->LocateStandaloneEnabler (schemaKey, className);
    
    ECSchemaCP schema = m_ecClass.GetSchema().FindSchema(schemaKey, SCHEMAMATCHTYPE_Exact);//TODO: Test change of behavior we need to match schemas exactly:Abeesh
    if (NULL == schema)
        return NULL;

    ECClassP ecClass = schema->GetClassP(className);
    if (NULL == ecClass)
        return NULL;
    
    return ecClass->GetDefaultStandaloneEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr          ECEnabler::GetEnablerForStructArrayMember (SchemaKeyCR schemaName, WCharCP className)  
    {
    return _LocateStandaloneEnabler (schemaName, className); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnabler::_IsPropertyReadOnly (UInt32 propertyIndex) const
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
ECPropertyCP ECEnabler::_LookupECProperty (UInt32 propertyIndex) const
    {
    WCharCP accessString;
    return ECOBJECTS_STATUS_Success == GetAccessString (accessString, propertyIndex) ? _LookupECProperty (accessString) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPropertyCP propertyFromAccessString (ECClassCR ecClass, WCharCP accessString)
    {
    WCharCP dot = wcschr (accessString, '.');
    if (NULL == dot)
        {
        // Discrepancy between "managed" access strings, which do not include "[]" for array properties, and "native" access strings, which do...annoyingly requires copying the string...
        WString fixedAccessString (accessString);
        if (fixedAccessString.length() >= 2 && '[' == fixedAccessString[fixedAccessString.length()-2])
            fixedAccessString.erase (fixedAccessString.length()-2);

        return ecClass.GetPropertyP (fixedAccessString.c_str());
        }
    else
        {
        WString structName (accessString, dot);
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
ECPropertyCP ECEnabler::_LookupECProperty (WCharCP accessString) const
    {
    return propertyFromAccessString (GetClass(), accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR           ECEnabler::GetClass() const  { return m_ecClass; }
WCharCP             ECEnabler::GetName() const { return _GetName(); }
ECObjectsStatus     ECEnabler::GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const { return _GetPropertyIndex (propertyIndex, accessString); }

ECObjectsStatus     ECEnabler::GetAccessString  (WCharCP& accessString, UInt32 propertyIndex) const { return _GetAccessString  (accessString, propertyIndex); }
UInt32              ECEnabler::GetPropertyCount () const { return _GetPropertyCount (); }
UInt32              ECEnabler::GetFirstPropertyIndex (UInt32 parentIndex) const { return _GetFirstPropertyIndex (parentIndex); }
UInt32              ECEnabler::GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const { return _GetNextPropertyIndex (parentIndex, inputIndex); }
bool                ECEnabler::HasChildProperties (UInt32 parentIndex) const { return _HasChildProperties (parentIndex); }
ECObjectsStatus     ECEnabler::GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const{ return _GetPropertyIndices (indices, parentIndex); };
IStandaloneEnablerLocaterR ECEnabler::GetStandaloneEnablerLocater() { return *this; }
ECPropertyCP        ECEnabler::LookupECProperty (UInt32 propertyIndex) const { return _LookupECProperty (propertyIndex); }
ECPropertyCP        ECEnabler::LookupECProperty (WCharCP accessString) const { return _LookupECProperty (accessString); }
bool                ECEnabler::IsPropertyReadOnly (UInt32 propertyIndex) const { return _IsPropertyReadOnly (propertyIndex); }

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

    FOR_EACH (ECPropertyCP prop, instance.GetClass().GetProperties())
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

            for (UInt32 idx = 0, count = ai.GetCount(); idx < count; ++idx)
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
#pragma region IECRelationshipEnabler
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
EC::ECRelationshipClassCR       IECRelationshipEnabler::GetRelationshipClass() const
    {
    return _GetRelationshipClass ();
    }

#pragma endregion //IECRelationshipEnabler
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
//UInt32          PropertyIndexedEnabler::_GetPropertyCount () const
//    {
//    return m_propertyCount;
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



END_BENTLEY_EC_NAMESPACE
    
