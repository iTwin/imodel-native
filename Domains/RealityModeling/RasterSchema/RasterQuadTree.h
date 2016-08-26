/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterQuadTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterTile;
struct DrawArgs;

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
template<typename Item_T, uint32_t ItemLimit_T>
struct ItemCache_T
    {
    typedef Item_T& ItemR;
    typedef Item_T* ItemP;
    typedef std::list<ItemP> ItemList;
    typedef typename ItemList::iterator ItemId;

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2015
    //----------------------------------------------------------------------------------------
    ItemId AddItem(ItemR item)
        {
        BeAssert(std::find(m_lru.begin(), m_lru.end(), &item) == m_lru.end());

        Prune();    // if required unload item.

        m_lru.push_back(&item);
        return --m_lru.end();
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2015
    //----------------------------------------------------------------------------------------
    void NotifyAccess(ItemId& id)
        {
        if (!IsValidId(id))
            return;

        BeAssert(id != m_lru.end() && std::find(m_lru.begin(), m_lru.end(), *id) != m_lru.end());

        m_lru.splice(m_lru.end(), m_lru, id);
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2015
    //----------------------------------------------------------------------------------------
    void RemoveItem(ItemId id)
        {
        if (!IsValidId(id))
            return;

        BeAssert(std::find(m_lru.begin(), m_lru.end(), *id) != m_lru.end());

        (*id)->OnItemRemoveFromCache(id);   // Notify owner of the item.
        m_lru.erase(id);
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2015
    //----------------------------------------------------------------------------------------
    void Prune()
        {
        while (m_lru.size() > ItemLimit_T)
            RemoveItem(m_lru.begin());
        }

    bool IsValidId(ItemId const& id) const { return id != m_lru.end(); }
    ItemId GetInvalidId() { return m_lru.end(); }

    ItemList m_lru;
    };

#define TILE_CACHE_LIMIT 350  // The number of texture we keep in memory.
typedef ItemCache_T<RasterTile, TILE_CACHE_LIMIT> RasterTileCache;

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterTile : public RefCountedBase
{
public:

    enum ChildIndex
        {
        UpperLeft   = 0,
        UpperRight  = 1,
        LowerLeft   = 2,
        LowerRight  = 3,
        };

    //! Create a root node(without a parent)
    static RasterTilePtr CreateRoot(RasterQuadTreeR tree);

    // Raster 4 corners(World) in this order:
    //  [0]  [1]
    //  [2]  [3]
    DPoint3dCP GetCorners() const { return m_corners; }

    //! Draw this tile in the view. Tile might not be loaded, it will be loaded only if locally available. Return true if successful.
    bool Draw(DrawArgs& drawArgs);
    
    TileId const& GetId() const {return m_tileId;}

    //! Direct access to coarser resolution. Might be NULL it its the root.
    RasterTileP GetParentP() {return m_pParent;}
    
    //! Return allocated child only. May be NULL.
    RasterTileP GetChildP(size_t index) {return m_pChilds[index].get();}

    //! Build the list of visibles tiles in this context. Nodes will be created as we walk the tree but pixels won't.
    void QueryVisible(std::vector<RasterTilePtr>& visibles, Dgn::ViewContextR context);

    bool HasCachedGraphic(Dgn::DgnViewportCR viewport) const { return nullptr != const_cast<RasterTile*>(this)->GetCachedGraphic(viewport, false); }
    Dgn::Render::GraphicP GetCachedGraphic(Dgn::DgnViewportCR, bool notifyAccess = true);
    void SaveGraphic(Dgn::DgnViewportCR, Dgn::Render::GraphicR);
    void OnItemRemoveFromCache(RasterTileCache::ItemId const& id);
    void DropGraphicsForViewport(Dgn::DgnViewportCR viewport);
    
    RasterQuadTreeR GetTreeR() {return m_tree;}       

    RasterTileP FindCachedCoarserTile(uint32_t resolutionDelta, Dgn::DgnViewportCR viewport);

private:
    RasterTile(TileId const& id, RasterTileP parent, RasterQuadTreeR tree);

    ~RasterTile();

    bool IsLeaf() const {return 0 == m_tileId.resolution;}

    bool IsVisible (Dgn::ViewContextR viewContext, double& factor) const;

    bool IsBorderTile() const{return GetResolution().GetTileCountX() == m_tileId.x +1 || GetResolution().GetTileCountY() == m_tileId.y + 1;}

    RasterSource::Resolution const& GetResolution() const;

    //! Allocate children according to the resolution description.
    void AllocateChildren();


    TileId m_tileId; 
    DPoint3d m_corners[4];      // Corners in World.  [0] [1]
                                //                    [2] [3]

    RasterQuadTreeR m_tree;     // Hold a ref only.


    RasterTileP m_pParent;      // NULL for root node.
    RasterTilePtr m_pChilds[4];    

    struct GraphicCacheEntry
        {
        RasterTileCache::ItemId m_cacheId;
        Dgn::DgnViewportCP      m_pViewport;    // Graphic are created/saved per viewport(~target)
        Dgn::Render::GraphicPtr m_graphic;        
        };

    std::list < GraphicCacheEntry > m_cachedGraphics;
};


//----------------------------------------------------------------------------------------
// Quad-tree that hold raster tiles.
// The tree structure use the rasterSource resolutions and tile to define the nodes.
// Pixels are not resampled by this class it uses the 'source' object resolution and tiles size
// to define the tree nodes.
//
// Raster Coordinate Systems:
//
//      - Physical: A pixel coordinate system that is aligned with the blocks in the raster file. Usually upper-left origin(slo 4)
//      - Source: A pixel coordinate system with a lower-left origin.
//      - World: The BIM file coordinate system that hold the raster model. Origin is Lower-left.

// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterQuadTree : public RefCountedBase
{
public:
    static RasterQuadTreePtr Create(RasterSourceR source, RasterModel& model);

    RasterTileR GetRoot() {return *m_pRoot;}

    RasterSourceR GetSource() {return *m_pSource;}

    RasterModel& GetModel() { return m_model; }

    //! Build the list of visibles tiles in this context. Nodes will be created as we walk the tree but pixels won't.
    void QueryVisible(std::vector<RasterTilePtr>& visibles, Dgn::ViewContextR context);

    Dgn::DgnDbR GetDgnDb() {return m_model.GetDgnDb();}

    void Draw (Dgn::RenderContextR context);

    //! Some format look best when increased quality is used. ex. WMS. Or we just want to display faster. Full quality is 1.0.
    void SetVisibleQualityFactor(double factor) {m_visibleQualityFactor=factor;}
    double GetVisibleQualityFactor() const {return m_visibleQualityFactor;}

    RasterTileCache& GetTileCache() { return m_tileCache;}

    void DropGraphicsForViewport(Dgn::DgnViewportCR viewport);

    DMatrix4dCR GetPhysicalToWorld() const { return m_physicalToWorld.M0; }
    DMatrix4dCR GetWorldToPhysical() const { return m_physicalToWorld.M1; }

private:
    
    RasterQuadTree(RasterSourceR source, RasterModel& model);

    RasterModel& m_model;               // The model to which this instance is associated.
    RasterSourcePtr m_pSource; 
    RasterTilePtr m_pRoot;              // The lowest/coarser resolution. 
    RasterTileCache& m_tileCache;
    double m_visibleQualityFactor;
    DMap4d m_physicalToWorld;           // including units change and reprojection approximation is any
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE