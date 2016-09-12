/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    DgnDb::VerifyClientThread();
    Utf8String parentPath("/");
    if (m_parent)
        parentPath = m_parent->_GetTileName();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. 
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::_DrawGraphics(DrawArgsR args, int depth) const
    {
    static bool s_debugRange = false;
    if (s_debugRange)
        {
        GraphicParams params;
        params.SetLineColor(ColorDef::Red());

        Render::GraphicBuilderPtr graphic = args.m_context.CreateGraphic();
        graphic->ActivateGraphicParams(params);
        graphic->AddRangeBox(m_range);
        args.m_graphics.Add(*graphic);
        }

    if (!m_geometry.empty()) // if we have geometry, draw it now
        {
        for (auto geom : m_geometry)
            geom->Draw(args);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Create a PolyfaceHeader from a Geometry
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr Geometry::GetPolyface() const
    {
    IGraphicBuilder::TriMeshArgs trimesh;
    trimesh.m_numIndices = (int32_t) m_indices.size();
    trimesh.m_vertIndex = m_indices.empty() ? nullptr : &m_indices.front();
    trimesh.m_numPoints = (int32_t) m_points.size();
    trimesh.m_points  = m_points.empty() ? nullptr : &m_points.front();
    trimesh.m_normals = m_normals.empty() ? nullptr : &m_normals.front();
    trimesh.m_textureUV = m_textureUV.empty() ? nullptr : &m_textureUV.front();;

    return trimesh.ToPolyface();
    }

/*-----------------------------------------------------------------------------------**//**
* Construct a Geometry from a TriMeshArgs and a Scene. The scene is necessary to get the Render::System, and this
* Geometry is only valid for that Render::System
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(IGraphicBuilder::TriMeshArgs const& args, SceneR scene)
    {
    // After we create a Render::Graphic, we only need the points/indices/normals for picking.
    // To save memory, only store them if the model is locatable.
    if (scene.IsLocatable())
        {
        m_indices.resize(args.m_numIndices);
        memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));

        m_points.resize(args.m_numPoints);
        memcpy(&m_points.front(), args.m_points, args.m_numPoints * sizeof(FPoint3d));

        if (nullptr != args.m_normals)
            {
            m_normals.resize(args.m_numPoints);
            memcpy(&m_normals.front(), args.m_normals, args.m_numPoints * sizeof(FPoint3d));
            }
        }

    if (nullptr == scene.GetRenderSystem() || !args.m_texture.IsValid())
        return;

    auto graphic = scene.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams());
    graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
    graphic->AddTriMesh(args);
    graphic->Close();

    m_graphic = graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Geometry::Draw(DrawArgsR args)
    {
    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }
