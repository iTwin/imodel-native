/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/MarshalHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "MarshalHelper.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyG06::BeFileName MarshalHelper::MarshalBimBeFileNameTo06BeFileName(BentleyApi::BeFileNameCR fileName)
    {
    return BentleyG06::BeFileName(fileName.c_str());
    }

END_CS06BRIDGE_NAMESPACE
