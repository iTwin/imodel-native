/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_primitiveType = primitiveType; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructArrayTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_arrayKind = ARRAYKIND_Struct; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Struct; 
    type.m_arrayKind = (ArrayKind)0; 
    return type; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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