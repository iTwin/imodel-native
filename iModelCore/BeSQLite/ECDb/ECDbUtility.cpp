/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbUtility.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbUtility::DuplicateProperties (IECInstanceR target, ECValuesCollectionCR source)
    {
    for (ECPropertyValueCR prop : source)
        {
        if (prop.HasChildValues())
            {
            DuplicateProperties (target, *prop.GetChildValues());
            continue;
            }
        target.SetValueUsingAccessor (prop.GetValueAccessor(), prop.GetValue());
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECDbUtility::GetPropertyIndex (ECClassCR ecClass, ECPropertyCR ecProperty)
    {
    WCharCP propertyName = ecProperty.GetName().c_str();
        
    //needswork: we want to be able to get the propertyIndex... but it can vary per-project
    uint32_t propertyIndex = 0;
    ecClass.GetDefaultStandaloneEnabler()->GetPropertyIndex (propertyIndex, propertyName);

    //GetClassMap().GetECDbMap().GetDgnProject();GetPropertyIndex()  // Project.GetEnabler(ecClass).GetPropertyIndex.
    return propertyIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECDbUtility::GetPropertyFromIndex (ECClassCR ecClass, uint32_t propertyIndex)
    {
    // TODO: Check if this convoluted call is indeed the best way to get the property!! Can the instance not have the default standalone enabler??
    WCharCP propertyName;
    ECObjectsStatus status = 
        ecClass.GetDefaultStandaloneEnabler()->GetClassLayout().GetAccessStringByIndex (propertyName, propertyIndex);
    if (status != ECOBJECTS_STATUS_Success)
        return nullptr;

    ECPropertyP ecProperty = ecClass.GetPropertyP (propertyName);

    return ecProperty;
    }

//--------------------------------------------------------------------------------------
//! ECValue::IsNull() does not work for arrays/structs. This encapsulates the check. 
//! @bsimethod                                   Krischan.Eberle                  10/12
//--------------------------------------------------------------------------------------
bool ECDbUtility::IsECValueEmpty (ECValueCR value)
    {
    if (value.IsPrimitive())
        return value.IsNull();
    
    if (value.IsArray())
        return (value.GetArrayInfo ().GetCount () == 0);
    
    if (value.IsStruct())
        return false; // There isn't a way to determine if a struct is NULL with just the value. 
    
    BeAssert(false);
    return true;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE

