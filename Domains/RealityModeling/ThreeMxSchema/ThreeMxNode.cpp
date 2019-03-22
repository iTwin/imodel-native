/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"

USING_NAMESPACE_DGN_CESIUM

/*---------------------------------------------------------------------------------**//**
* Must be called from client thread because it references "m_parent" which can become invalid
* if that node gets unloaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetChildFile() const
    {
    Utf8String parentPath("/");
    if (m_parent)
        parentPath = m_parent->_GetName();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
LoaderPtr Node::_CreateLoader(LoadStateR state, OutputR output)
    {
    return new Loader(GetRoot()._ConstructTileResource(*this), *this, state, output);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::Scene(ThreeMxModelR model, TransformCR location, Utf8CP sceneFile)
    : Scene(model.GetDgnDb(), location, sceneFile)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* Used by DgnV8Converter simply to extract the scene name...
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::Scene(DgnDbR db, TransformCR location, Utf8CP sceneFile)
    : T_Super(db, location, sceneFile), m_sceneFile(sceneFile)
    {
    //
    }

