/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Foundations/PrivateStringTools.cpp $
|    $RCSfile: PrivateStringTools.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/06/01 14:05:09 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <STMInternal/Foundations/PrivateStringTools.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString CreateStrFromWCStr (const WChar* cstr)
    {
    return WString(cstr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString CreateStrFrom (const WChar* cstr)
    {
    return CreateStrFromWCStr(cstr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString CreateWStrFrom (const WChar* cstr)
    {
    return WString(cstr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AString CreateAStrFrom (const WChar* cstr)
    {
    return AString(cstr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString CreateWStrFromACStr (const char* cstr)
    {
    return WString(cstr);
    }
    
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE