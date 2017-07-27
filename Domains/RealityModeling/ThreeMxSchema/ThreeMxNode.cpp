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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::_HasGraphics() const
    {
    for (auto const& geom : m_geometry)
        if (geom->HasGraphics())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. 
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::_DrawGraphics(DrawArgsR args) const
    {
    static bool s_debugRange = false;
    if (s_debugRange)
        {
        GraphicParams params;
        params.SetLineColor(ColorDef::Red());

        Render::GraphicBuilderPtr graphic = args.m_context.CreateGraphic(GraphicBuilder::CreateParams(GetRoot().GetDgnDb()));
        graphic->ActivateGraphicParams(params);
        graphic->AddRangeBox(m_range);
        args.m_graphics.Add(*graphic->Finish());
        }

    if (!m_geometry.empty()) // if we have geometry, draw it now
        {
        for (auto geom : m_geometry)
            geom->Draw(args);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent Node::_SelectTiles(bvector<TileCPtr>& selectedTiles, DrawArgsR args) const
    {
    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    auto children = _GetChildren(true);
    if (tooCoarse && nullptr != children)
        {
        m_childrenLastUsed = args.m_now;
        for (auto const& child : *children)
            {
            // 3mx requires that we load tiles recursively - we cannot jump directly to the tiles we actually want to draw...
            if (!child->IsReady())
                args.InsertMissing(*child);

            child->_SelectTiles(selectedTiles, args);
            }

        return SelectParent::No;
        }

    // This node is either fine enough for the current view or has some unloaded children. We'll select it.
    selectedTiles.push_back(this);

    if (!tooCoarse)
        {
        // This node was fine enough for the current zoom scale and was successfully drawn. If it has loaded children from a previous pass, they're no longer needed.
        _UnloadChildren(args.m_purgeOlderThan);
        }

    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::_PickGraphics(PickArgsR args, int depth) const
    {
    if (!m_geometry.empty()) // if we have geometry, draw it now
        {
        for (auto geom : m_geometry)
            geom->Pick(args);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
TileLoaderPtr Node::_CreateTileLoader(TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    {
    return new Loader(GetRoot()._ConstructTileResource(*this), *this, loads, renderSys);
    }

/*-----------------------------------------------------------------------------------**//**
* Construct a Geometry from a CreateParams and a Scene. The scene is necessary to get the Render::System, and this
* Geometry is only valid for that Render::System
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(CreateParams const& args, SceneR scene, DRange3dCR tileRange, Dgn::Render::SystemP renderSys) : TriMesh(args, scene, renderSys) { }

