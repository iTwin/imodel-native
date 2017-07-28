/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"

USING_NAMESPACE_TILETREE

/*---------------------------------------------------------------------------------**//**
* Must be called from client thread because it references "m_parent" which can become invalid
* if that node gets unloaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetChildFile() const
    {
    Utf8String parentPath("/");
    if (m_parent)
        parentPath = m_parent->_GetTileCacheKey();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
TileLoaderPtr Node::_CreateTileLoader(TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    {
    return new Loader(GetRoot()._ConstructTileResource(*this), *this, loads, renderSys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::_WantDebugRangeGraphics() const
    {
    static bool s_debugRange = false;
    return s_debugRange;
    }

