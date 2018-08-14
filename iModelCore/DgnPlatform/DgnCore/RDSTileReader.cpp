/*-------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/RDSTileReader.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>
#include <folly/BeFolly.h>
#include <RealityPlatformTools/SimpleRDSApi.h>


USING_NAMESPACE_TILETREE
USING_NAMESPACE_TILETREE_IO
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
USING_NAMESPACE_BENTLEY_REALITYPLATFORM



BEGIN_TILETREE_IO_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr RDSTileReader::ReadTileTree(Utf8CP url)
    {
    static bool s_initialized = false;

    if (!s_initialized)
        {
        s_initialized = true;
        RDSRequestManager::Setup();
        }

    return nullptr;
    }


END_TILETREE_IO_NAMESPACE
   



