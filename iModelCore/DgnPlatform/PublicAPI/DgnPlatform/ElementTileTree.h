/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementTileTree.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/Render.h>

#define BEGIN_ELEMENT_TILETREE_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace ElementTileTree {
#define END_ELEMENT_TILETREE_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_ELEMENT_TILETREE using namespace BentleyApi::Dgn::ElementTileTree;

BEGIN_ELEMENT_TILETREE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);
DEFINE_REF_COUNTED_PTR(Loader);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    uint64_t        m_createTime;       // The time of the most recent change to any element in the associated model when the tile loader was created.
    uint64_t        m_cacheCreateTime;  // The time of the most recent change to any element in the associated model when the tile cache data was created.
    Render::Primitives::GeometryCollection  m_geometry;

    Loader(TileTree::TileR tile, TileTree::TileLoadStateSPtr loads);

    BentleyStatus _GetFromSource() override;
    BentleyStatus _LoadTile() override;
    bool _IsExpired(uint64_t) override;
    bool _IsValidData() override;
    bool _IsCompleteData() override;
    BentleyStatus _ReadFromDb() override;

    BentleyStatus LoadGeometryFromModel();
    BentleyStatus DoGetFromSource();
    bool IsExpired() const { return m_cacheCreateTime < m_createTime; }

    uint64_t _GetCreateTime() const override { return m_createTime; }
public:
    static LoaderPtr Create(TileTree::TileR tile, TileTree::TileLoadStateSPtr loads) { return new Loader(tile, loads); }
};

END_ELEMENT_TILETREE_NAMESPACE

