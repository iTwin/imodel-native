/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECType.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

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
     
END_BENTLEY_EC_NAMESPACE