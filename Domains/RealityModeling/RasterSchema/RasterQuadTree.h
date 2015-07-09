/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterQuadTree.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

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

    //&&MM TODO status. The goal is to avoid requesting over and over when errors occurs.
    // Also when a request timeout we should somehow retry a number of time.
    // Maybe we need flags? or 2 status. 
    enum class TileStatus
        {
        Unloaded,
        Requested,
        Loaded,
        ConnectionError,
        //Error,
        //Expired,
        OutOfDomain,    // reprojection error  
        };

    //! Create a root node(without a parent)
    static RasterTilePtr CreateRoot(RasterQuadTreeR tree);

    //! return NULL if not available.
    DisplayTileP GetDisplayTileP(bool request);

    // Raster 4 corners(UOR) in this order:
    //  [0]  [1]
    //  [2]  [3]
    DPoint3dCR GetCorners() const; 

    //! Draw this tile in the view. Tile might not be loaded. Return true if successful.
    bool Draw(Dgn::ViewContextR context);    

    TileId const& GetId() const {return m_tileId;}

    //! Direct access to coarser resolution. Might be NULL it its the root.
    RasterTileP GetParentP() {return m_pParent;}

    //! Return allocated child only. May be NULL.
    RasterTileP GetChildP(size_t index) {return m_pChilds[index].get();}

    //! Build the list of visibles tiles in this context. Nodes will be created as we walk the tree but pixels won't.
    void QueryVisible(bvector<RasterTilePtr>& visibles, Dgn::ViewContextR context);

    //! Return true if the tiles pixels are loaded.
    bool IsLoaded() const {return m_pDisplayTile.IsValid();}

private:
    RasterTile(TileId const& id, RasterTileP parent, RasterQuadTreeR tree);

    bool IsLeaf() const {return 0 == m_tileId.resolution;}

    bool IsVisible (Dgn::ViewContextR viewContext, double& factor) const;

    bool IsBorderTile() const{return GetResolution().GetTileCountX() == m_tileId.x +1 || GetResolution().GetTileCountY() == m_tileId.y + 1;}

    RasterSource::Resolution const& GetResolution() const;

    //! Allocate children according to the resolution description.
    void AllocateChildren();

    static ReprojectStatus ReprojectCorners(DPoint3dP outUors, DPoint3dCP srcCartesian, RasterQuadTreeR tree);

    DisplayTilePtr m_pDisplayTile;      // Can be NULL, will be loaded when needed. 

    TileId m_tileId; 
    RasterQuadTreeR m_tree;          // Hold a ref only.

    DPoint3d m_corners[4];      // Corners in uor.

    TileStatus m_status;

    //&&MM add the concept of lastDrawTime and use that to cleanup old tiles. Make sure time query does not impact performance.
    //it might be better to use the same time for the all tiles of a single draw operation. Set during QueryVisible?
    // use time and proximity to the current visibles tile to select what needs to be clean.

    RasterTileP m_pParent;      // NULL for root node.
    RasterTilePtr m_pChilds[4];    
};


//----------------------------------------------------------------------------------------
// Quad-tree that hold raster tiles.
// The tree structure use the rasterSource resolutions and tile to define the nodes.
// Pixels are not resampled by this class it uses the 'source' object resolution and tiles size
// to define the tree nodes.
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterQuadTree : public RefCountedBase
{
public:
    static RasterQuadTreePtr Create(RasterSourceR source, Dgn::DgnDbR dgnDb);

    RasterTileR GetRoot() {return *m_pRoot;}

    RasterSourceR GetSource() {return *m_pSource;}

    //! Build the list of visibles tiles in this context. Nodes will be created as we walk the tree but pixels won't.
    void QueryVisible(bvector<RasterTilePtr>& visibles, Dgn::ViewContextR context);

    Dgn::DgnDbR GetDgnDb() {return m_dgnDb;}

    void Draw (Dgn::ViewContextR context);

    //! Some format look best when increased quality is used. ex. WMS. Or we just want to display faster. Full quality is 1.0.
    void SetVisibleQualityFactor(double factor) {m_visibleQualityFactor=factor;}
    double GetVisibleQualityFactor() const {return m_visibleQualityFactor;}

private:
    
    RasterQuadTree(RasterSourceR source, Dgn::DgnDbR dgnDb);

    Dgn::DgnDbR m_dgnDb;                 
    RasterSourcePtr m_pSource; 
    RasterTilePtr m_pRoot;          // The lowest/coarser resolution. 
    double m_visibleQualityFactor;
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE