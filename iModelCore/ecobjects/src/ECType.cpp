/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECType.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
     
END_BENTLEY_ECOBJECT_NAMESPACE