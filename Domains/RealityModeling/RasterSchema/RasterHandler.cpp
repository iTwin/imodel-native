/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "WmsSource.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterModelHandler)
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::RasterModel(CreateParams const& params) : T_Super (params)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
RasterQuadTreeP RasterModel::GetTree() const
    {
    if(!m_rasterTreeP.IsValid())
        _LoadQuadTree();
        
    return m_rasterTreeP.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DropGraphicsForViewport(DgnViewportCR viewport)
    {
    if (m_rasterTreeP.IsValid())
        m_rasterTreeP->DropGraphicsForViewport(viewport);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_AddSceneGraphics(SceneContextR context) const
    {
    //Note that this call occurs on the client thread and that is must be fast.
    RasterQuadTreeP pTree = GetTree();
    if(NULL != pTree)
        pTree->Draw(context);
    }
