/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTypeDescriptor                ECTypeDescriptor::CreatePrimitiveTypeDescriptor 
(
PrimitiveType primitiveType
) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Primitive; 
    type.m_primitiveType = primitiveType; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_primitiveType = primitiveType; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructArrayTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_arrayKind = ARRAYKIND_Struct; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Struct; 
    type.m_arrayKind = (ArrayKind)0; 
    return type; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  01/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECTypeDescriptor                ECTypeDescriptor::CreateNavigationTypeDescriptor(PrimitiveType type, bool isMultiple)
    {
    ECTypeDescriptor descriptor;
    /*if (isMultiple)
        {
        descriptor.m_typeKind = ValueKind::VALUEKIND_Array;
        descriptor.m_arrayKind = ArrayKind::ARRAYKIND_Navigation;
        }
    else
        {*/
        
    //    }
    descriptor.m_typeKind = ValueKind::VALUEKIND_Navigation;
    descriptor.m_primitiveType = type;

    return descriptor;
    }
END_BENTLEY_ECOBJECT_NAMESPACE