/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
