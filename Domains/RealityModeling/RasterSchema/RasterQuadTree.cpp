/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterQuadTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include "RasterQuadTree.h"

static const uint32_t DRAW_FINER_DELTA = 2;
static const uint32_t DRAW_COARSER_DELTA = 6;
static const double   DRAW_COARSER_BIAS = -100.0 * DgnUnits::OneMillimeter();
static const double   DRAW_FINER_BIAS = -50.0 * DgnUnits::OneMillimeter();

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
RasterTileCache& s_GetSharedTileCache()
    {
    // Tile graphics cache among all rasters
    static RasterTileCache* s_instance = NULL;
    if (NULL == s_instance)
        s_instance = new RasterTileCache;
    return *s_instance;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
static ReprojectStatus s_FilterGeocoordWarning(ReprojectStatus status)
    {
    if ((REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))   // This a warning
        return REPROJECT_Success;

    return status;   
    }

//=======================================================================================
// @bsiclass                                                    BentleySystems
//=======================================================================================
struct TileDataQuery : public RefCountedBase
    {
    TileDataQuery(RasterTileR tileNode, Render::TargetR target)
        :m_tileNode(tileNode), m_target(target), m_isFinished(false)
        {
        }

    void Run(); // Execute in a thread

    // Thread safe methods.
    bool IsCanceled() const { return GetRefCount() == 1; }    // if the pool is the only owner that means no one is interested on the result.
    bool IsFinished() const { return m_isFinished; }

    RasterTileR GetTileNodeR() {return m_tileNode;}

    Render::TextureP GetTileP() {return m_pTile.get();} // might be null is query failed.

    private:
        std::atomic<bool>  m_isFinished;
        Render::TexturePtr m_pTile;
        Render::TargetR    m_target;
        RasterTileR        m_tileNode;
    };
typedef RefCountedPtr<TileDataQuery> TileDataQueryPtr;

class ThreadPool;
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
class Worker
    {
    public:
        Worker(ThreadPool &s) : m_pool(s) {}
        void operator()();
    private:
        ThreadPool &m_pool;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
class ThreadPool
    {
    public:
        ThreadPool(size_t);
        ~ThreadPool();

        void Enqueue(TileDataQuery& task);

        static ThreadPool& GetInstance()
            {
            // At least 2 make sense since we have a mix of io and cpu/memory operations. 
            // But, we need the cpu for other things, don't start one for every core (I added the "/2") - kab
            uint32_t threadCount = std::max((uint32_t) 2, BeThreadUtilities::GetHardwareConcurrency() / 2);
            static ThreadPool* s_pool = new ThreadPool(threadCount);
            return *s_pool;
            }
    private:
        friend class Worker;

        // need to keep track of threads so we can join them
        std::vector< std::thread > m_workers;

        // the task queue
        std::deque<TileDataQueryPtr> m_tasks;

        // synchronization
        std::mutex m_queue_mutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_stop;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
void Worker::operator()()
    {
    BeThreadUtilities::SetCurrentThreadName("QueryTile worker");

    while (true)
        {
        TileDataQueryPtr taskP;

        {   // acquire lock
        std::unique_lock<std::mutex> lock(m_pool.m_queue_mutex);
        while (taskP.IsNull())
            {
            // look for a work item
            while (!m_pool.m_stop && m_pool.m_tasks.empty())
                {
                // if there are none wait for notification
                m_pool.m_condition.wait(lock);
                }

            if (m_pool.m_stop) // exit if the pool is stopped
                return;

            // get the task from the queue
            taskP = m_pool.m_tasks.front();
            m_pool.m_tasks.pop_front();

            if (taskP->IsCanceled())
                taskP = nullptr;
            }

        }   // release lock

        // execute the task
        taskP->Run();

        // Query executed, release it. We do not want to hold into it because we might end up being the last owner of the material and that could mean 
        // deleting a qv texture in the worker thread. This is not allowed.
        taskP = nullptr;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
ThreadPool::ThreadPool(size_t threads)
    :m_stop(false)
    {
    for (size_t i = 0; i < threads; ++i)
        m_workers.push_back(std::thread(Worker(*this)));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
ThreadPool::~ThreadPool()
    {
    // stop all threads
    m_stop = true;
    m_condition.notify_all();

    // join them
    for (size_t i = 0; i < m_workers.size(); ++i)
        m_workers[i].join();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
void ThreadPool::Enqueue(TileDataQuery& task)
    {
        { // acquire lock
        std::unique_lock<std::mutex> lock(m_queue_mutex);

        // add the task and increment ref count.
        m_tasks.push_back(&task);
        } // release lock

        // wake up one thread
        m_condition.notify_one();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
void TileDataQuery::Run()
    {
    bool enableAlphaBlend = false;
    Render::Image image = m_tileNode.GetTreeR().GetSource().QueryTile(m_tileNode.GetId(), enableAlphaBlend);

#ifndef NDEBUG  // debug build only.
    static bool s_missingTilesInRed = false;
    if (s_missingTilesInRed && !image.IsValid())
        {
        ByteStream data(256 * 256 * 3);
        Byte red[3] = {255,0,0};
        for (uint32_t pixel = 0; pixel < 256 * 256; ++pixel)
            memcpy(data.GetDataP() + pixel * 3, red, 3);

        image = Render::Image(256, 256, std::move(data), Render::Image::Format::Rgb);
        enableAlphaBlend = false;
        }
#endif         
    if (image.IsValid() && !IsCanceled())
        m_pTile = m_target.CreateTexture(image);
        
    m_isFinished = true;
    }


BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Mathieu.Marchand  7/2016
//=======================================================================================
struct DrawArgs
    {
    RenderContextR m_context;
    Render::GraphicBranch m_graphics;

    DrawArgs(RenderContextR context) : m_context(context) {}
    void AddGraphic(Render::GraphicR gra) { m_graphics.Add(gra); }
    void DrawGraphics(double zBias);
    RenderContextR GetContext(){return m_context;}
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterProgressiveDisplay : Dgn::ProgressiveTask
{
protected:
    std::vector<RasterTilePtr>      m_visiblesTiles;    // all the tiles to display.    
    std::list<TileDataQueryPtr>     m_queriedTiles;

    RasterQuadTreeR m_raster;
    DgnViewportR    m_viewport;         // the viewport we are scheduled on.   

    //----------------------------------------------------------------------------------------
    // @bsiclass
    //----------------------------------------------------------------------------------------
    struct SortByResolution
        {
        bool operator ()(const RasterTilePtr& lhs, const RasterTilePtr& rhs) const
            {
            if(lhs->GetId().resolution != rhs->GetId().resolution)
                return lhs->GetId().resolution < rhs->GetId().resolution;

            if(lhs->GetId().x != rhs->GetId().x)
                return lhs->GetId().x < rhs->GetId().x;

            return lhs->GetId().y < rhs->GetId().y;
            }
        };
    typedef bset<RasterTilePtr, SortByResolution> SortedTiles;

    RasterProgressiveDisplay (RasterQuadTreeR raster, RenderContextR context);
    virtual ~RasterProgressiveDisplay();

    //! Displays tiled rasters and schedules downloads. 
    virtual Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;

    void FindCoarserTiles(SortedTiles& coarserTiles, std::vector<RasterTilePtr> const& visibleTiles, uint32_t resolutionDelta, DgnViewportCR viewport);
    
    bool DrawFinerTiles(RasterTileR tile, uint32_t resolutionDelta, DrawArgs& drawArgs);

public:
    static bool ShouldDrawInContext(RenderContextR context);

    void Draw (RenderContextR context);

    static RefCountedPtr<RasterProgressiveDisplay> Create(RasterQuadTreeR raster, RenderContextR context);
};
END_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
//-------------------------------  RasterTile --------------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterTilePtr RasterTile::CreateRoot(RasterQuadTreeR tree)
    {
    if(tree.GetSource().GetResolutionCount() == 0)
        return NULL;

    TileId id(tree.GetSource().GetResolutionCount()-1,0,0);

    //&&MM REPROJECTION clip to gcs domain. For now, make sure we can compute the corners in UOR.
    DPoint3d srcCorners[4];
    DPoint3d uorCorners[4];
    tree.GetSource().ComputeTileCorners(srcCorners, id); 
    if(REPROJECT_Success != ReprojectCorners(uorCorners, srcCorners, tree))
        return NULL;

    return new RasterTile(id, NULL, tree);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterTile::RasterTile(TileId const& id, RasterTileP parent, RasterQuadTreeR tree)
    :m_tileId(id),
    m_tree(tree),
    m_pParent(parent)
    {
    BeAssert(m_tree.GetSource().IsValidTileId(id));

    RasterSourceR source = m_tree.GetSource();

    DPoint3d srcCorners[4];
    source.ComputeTileCorners(srcCorners, id); 

    if(REPROJECT_Success != ReprojectCorners(m_corners, srcCorners, tree))
        {
        memcpy(m_corners, srcCorners, sizeof(m_corners));   // Assume coincident for now. We should an invalid reprojection or something.
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2015
//----------------------------------------------------------------------------------------
RasterTile::~RasterTile()
    {
    for (std::list < GraphicCacheEntry >::iterator itr(m_cachedGraphics.begin()); itr != m_cachedGraphics.end(); )
        {
        RasterTileCache::ItemId itemToRemove = itr->m_cacheId;
        // Remove from m_cachedGraphics prior to remove from cache because OnItemRemoveFromCache() would do it and invalidate our iterator.
        itr = m_cachedGraphics.erase(itr);
        m_tree.GetTileCache().RemoveItem(itemToRemove);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
ReprojectStatus RasterTile::ReprojectCorners(DPoint3dP outUors, DPoint3dCP srcCartesian, RasterQuadTreeR tree)
    {
    //&&MM WIP reproject
    tree.GetSourceToWorld().MultiplyAndRenormalize(outUors, srcCartesian, 4);
    
    ReprojectStatus status = REPROJECT_Success;

    static bool s_doExactTransform = false;  // for debugging

    if (s_doExactTransform)
        {
        DPoint3d outUors2[4];
        GeoCoordinates::BaseGCSP pSourceGcs = tree.GetSource().GetGcsP();

        DgnGCSP pDgnGcs = tree.GetDgnDb().Units().GetDgnGCS();

        if (NULL == pSourceGcs || NULL == pDgnGcs || !pSourceGcs->IsValid() || !pDgnGcs->IsValid())
            {
            // Assume raster to be coincident.
            memcpy(outUors, srcCartesian, sizeof(DPoint3d) * 4);
            return REPROJECT_Success;
            }

        GeoPoint srcGeoCorners[4];
        for (size_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromCartesian(srcGeoCorners[i], srcCartesian[i]))))
                {
                BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
                return status;
                }
            }

        // Source latlong to DgnDb latlong.
        GeoPoint dgnGeoCorners[4];
        for (size_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromLatLong(dgnGeoCorners[i], srcGeoCorners[i], *pDgnGcs))))
                return status;
            }

        //Finally to UOR
        for (uint32_t i = 0; i < 4; ++i)
            {
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pDgnGcs->UorsFromLatLong(outUors2[i], dgnGeoCorners[i]))))
                return status;
            }
        }

    return status;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool RasterTile::Draw(DrawArgs& drawArgs)
    {
#ifndef NDEBUG  // debug build only.
    // Corners are in this order:
    //  [0]  [1]
    //  [2]  [3]
    DPoint3d uvPts[4];
    memcpy(uvPts, m_corners, sizeof(uvPts));
    
    static bool s_DrawTileShape = false;
    if(s_DrawTileShape)
        {
        auto pTileInfoGraphic = drawArgs.GetContext().CreateGraphic(Render::Graphic::CreateParams(drawArgs.GetContext().GetViewport()));
        Render::GraphicParams graphicParams;
        graphicParams.SetLineColor(ColorDef(222, 0, 0, 128));
        graphicParams.SetIsFilled(false);
        graphicParams.SetWidth(2);
        pTileInfoGraphic->ActivateGraphicParams(graphicParams);

        DPoint3d box[5];
        box[0] = uvPts[0];
        box[1] = uvPts[1];
        box[2] = uvPts[3];
        box[3] = uvPts[2];
        box[4] = box[0];
        pTileInfoGraphic->AddLineString(_countof(box), box);

        DPoint3d centerPt = DPoint3d::FromInterpolate(uvPts[0], 0.5, uvPts[3]);
        double pixelSize = drawArgs.GetContext().GetPixelSizeAtPoint(&centerPt);

        Utf8String tileText;
        tileText.Sprintf("%d:%d,%d", m_tileId.resolution, m_tileId.x, m_tileId.y);
         
        TextStringPtr textStr = TextString::Create();
        textStr->SetText(tileText.c_str());
        textStr->GetStyleR().SetFont(DgnFontManager::GetDecoratorFont());
        textStr->GetStyleR().SetSize(pixelSize*10);
        textStr->SetOriginFromJustificationOrigin(centerPt, TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);

        graphicParams.SetLineColor(ColorDef(222, 0, 222, 128));
        graphicParams.SetIsFilled(false);
        graphicParams.SetWidth(2);
        pTileInfoGraphic->ActivateGraphicParams(graphicParams);
        pTileInfoGraphic->AddTextString (*textStr);

        drawArgs.AddGraphic(*pTileInfoGraphic);
        }
#endif

    Dgn::Render::GraphicP pTileGraphic = GetCachedGraphic(drawArgs.GetContext().GetViewportR());
    if(pTileGraphic == nullptr)
        return false;

    drawArgs.AddGraphic(*pTileGraphic);
   
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
RasterSource::Resolution const& RasterTile::GetResolution() const {return m_tree.GetSource().GetResolution(m_tileId.resolution);}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
RasterTileP RasterTile::FindCachedCoarserTile(uint32_t resolutionDelta, Dgn::DgnViewportCR viewport)
    {
    uint32_t maxResolution = GetId().resolution + resolutionDelta;
    for (RasterTileP pParent = GetParentP(); NULL != pParent; pParent = pParent->GetParentP())
        {
        if (pParent->HasCachedGraphic(viewport))
            return pParent; // found!

        if (pParent->GetId().resolution >= maxResolution)
            break;
        }

    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterTile::AllocateChildren()
    {
    if(IsLeaf() || m_pChilds[0].IsValid())  // no res or already allocated.
        return;

    RasterSource::Resolution const& childrenResolution = m_tree.GetSource().GetResolution(m_tileId.resolution-1);

    // Upper-Left child, we always have one 
    TileId childUpperLeft(GetId().resolution-1, GetId().x << 1, GetId().y << 1);
    m_pChilds[0] = new RasterTile(childUpperLeft, this, m_tree);
    
    // Upper-Right
    if(childUpperLeft.x + 1 < childrenResolution.GetTileCountX())
        {
        TileId childUpperRight(childUpperLeft.resolution, childUpperLeft.x+1, childUpperLeft.y);
        m_pChilds[1] = new RasterTile(childUpperRight, this, m_tree);
        }

    // Lower-left
    if(childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
        {
        TileId childLowerLeft(childUpperLeft.resolution, childUpperLeft.x, childUpperLeft.y+1);
        m_pChilds[2] = new RasterTile(childLowerLeft, this, m_tree);
        }

    // Lower-Right
    if(childUpperLeft.x + 1 < childrenResolution.GetTileCountX() && 
       childUpperLeft.y + 1 < childrenResolution.GetTileCountY())
        {
        TileId childLowerRight(childUpperLeft.resolution, childUpperLeft.x+1, childUpperLeft.y+1);
        m_pChilds[3] = new RasterTile(childLowerRight, this, m_tree);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterTile::QueryVisible(std::vector<RasterTilePtr>& visibles, ViewContextR context)
    { 
    double factor;
    if (IsVisible(context, factor))
        {
        if (!IsLeaf() && factor > m_tree.GetVisibleQualityFactor())
            {
            AllocateChildren(); 

            size_t visiblesIn = visibles.size();
            for(size_t i=0; i < 4; ++i)
                {
                RasterTileP pChild = GetChildP(i);
                if(pChild != NULL)
                    pChild->QueryVisible(visibles, context);
                }
                
            //&&MM REPROJETION investigate how to handle reprojection where parent is visible and the 4 children are not.
            // reason: parent and children do not encompass the same region.                
            if(visiblesIn == visibles.size())
                {
                visibles.size();
                }
            }
        else
            {
            visibles.push_back(this);
            }
        }    
    }

/*---------------------------------------------------------------------------------**//**
* Inspired from HIEMriGridDisplayable::IsVisible()
* @bsimethod                                                    Mathieu.Marchand  4/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterTile::IsVisible (ViewContextR viewContext, double& factor) const
    {
    //&&MM I did another kind of visibility test win TexturedMesh.cpp(TextureTile::IsVisible) that
    // uses a radius and centerPt. That technique could help with non linear transformation of the
    // raster. (ex: 4 corners reprojection).  Is it faster?
    DPoint3d npcCorners[4];
    DPoint3d frustCorners[4];
    memcpy(frustCorners, m_corners, sizeof(frustCorners));

    viewContext.GetWorldToNpc().M0.MultiplyAndRenormalize(npcCorners, frustCorners, 4);

    DPoint3d npcOrigin;
    npcOrigin.x = MIN(MIN(npcCorners[0].x, npcCorners[1].x), MIN(npcCorners[2].x, npcCorners[3].x));
    npcOrigin.y = MIN(MIN(npcCorners[0].y, npcCorners[1].y), MIN(npcCorners[2].y, npcCorners[3].y));
    npcOrigin.z = MIN(MIN(npcCorners[0].z, npcCorners[1].z), MIN(npcCorners[2].z, npcCorners[3].z));
    
    if (npcOrigin.x > 1.0 || npcOrigin.y > 1.0 || npcOrigin.z > 1.0)
        {
        return false;
        }

    DPoint3d npcCorner;
    npcCorner.x = MAX(MAX(npcCorners[0].x, npcCorners[1].x), MAX(npcCorners[2].x, npcCorners[3].x));
    npcCorner.y = MAX(MAX(npcCorners[0].y, npcCorners[1].y), MAX(npcCorners[2].y, npcCorners[3].y));
    npcCorner.z = MAX(MAX(npcCorners[0].z, npcCorners[1].z), MAX(npcCorners[2].z, npcCorners[3].z));
    
    if (npcCorner.x < 0.0 || npcCorner.y < 0.0 || npcCorner.z < 0.0)
        {
        return false;
        }

    // compute scale factor of this tile
    DPoint3d viewCorners[4];
    viewContext.WorldToView(viewCorners, frustCorners, 4);

    uint64_t physWidth = m_tree.GetSource().GetTileSizeX(m_tileId);
    uint64_t physHeight = m_tree.GetSource().GetTileSizeY(m_tileId);

    //&&MM need to change the factor interpretation. if we want to use the squared version
    //   -- visibles tiles are not always on the same resolution usually on border tiles.
    //          when reprojecting we need all tiles to have the same connection points to avoid gaps. that means use the highest resolution?
#if 0   
    double physicalDiagSquared = (double)(physWidth*physWidth + physHeight*physHeight);
    double viewDiag1Squared = viewCorners[3].DistanceSquaredXY(&viewCorners[0]);
    double viewDiag2Squared = viewCorners[2].DistanceSquaredXY(&viewCorners[1]);
    double averageViewDiagSquared = (viewDiag1Squared + viewDiag2Squared) / 2.0;
    factorSquared = averageViewDiagSquared / physicalDiagSquared;
#else
    double physicalDiag = sqrt(physWidth*physWidth + physHeight*physHeight);
    double viewDiag1 = viewCorners[3].DistanceXY(viewCorners[0]);
    double viewDiag2 = viewCorners[2].DistanceXY(viewCorners[1]);
    double averageViewDiag = (viewDiag1 + viewDiag2) / 2.0;
    factor = averageViewDiag / physicalDiag;

    DPoint3d centerPt = DPoint3d::FromInterpolate(m_corners[0], 0.5, m_corners[3]);(void)centerPt;
    double pixelSize = viewContext.GetPixelSizeAtPoint(&centerPt);(void)pixelSize;

    double widthView = viewCorners[1].DistanceXY(viewCorners[0]);(void)widthView;
    double heightView = viewCorners[3].DistanceXY(viewCorners[2]);(void)heightView;

    double uorDiag1 = m_corners[3].DistanceXY(m_corners[0]);(void)uorDiag1;
    double uorDiag2 = m_corners[2].DistanceXY(m_corners[1]);(void)uorDiag2;
    
#endif

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterTile::DropGraphicsForViewport(Dgn::DgnViewportCR viewport)
    {
    for (auto itr(m_cachedGraphics.begin()); itr != m_cachedGraphics.end(); ++itr)
        {
        if (itr->m_pViewport == &viewport)
            {
            // Will trigger OnItemRemoveFromCache() and remove it from the cache.
            m_tree.GetTileCache().RemoveItem(itr->m_cacheId);
            break;
            }
        }

    // Recurse in children
    for (size_t i = 0; i < 4; ++i)
        {
        RasterTileP pChild = GetChildP(i);
        if (pChild != NULL)
            pChild->DropGraphicsForViewport(viewport);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
Render::GraphicP RasterTile::GetCachedGraphic(DgnViewportCR viewport, bool notifyAccess)
    {
    for (auto& cacheEntry : m_cachedGraphics)
        {
        if (cacheEntry.m_pViewport == &viewport)
            {
            if(notifyAccess)
                m_tree.GetTileCache().NotifyAccess(cacheEntry.m_cacheId);
            return cacheEntry.m_graphic.get();
            }
        }

    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterTile::SaveGraphic(DgnViewportCR viewport, Render::GraphicR graphic)
    {
    BeAssert(!HasCachedGraphic(viewport));

    GraphicCacheEntry entry;
    entry.m_cacheId = m_tree.GetTileCache().AddItem(*this);
    entry.m_pViewport = &viewport;
    entry.m_graphic = &graphic;
   
    m_cachedGraphics.emplace_back(entry);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterTile::OnItemRemoveFromCache(RasterTileCache::ItemId const& id)
    {
    for (std::list < GraphicCacheEntry >::iterator itr(m_cachedGraphics.begin()); itr != m_cachedGraphics.end(); ++itr)
        {
        if (itr->m_cacheId == id)
            {
            m_cachedGraphics.erase(itr);
            break;
            }
        }
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterQuadTree --------------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterQuadTreePtr RasterQuadTree::Create(RasterSourceR source, DgnDbR dgnDb)
    {
    RasterQuadTreePtr pTree = new RasterQuadTree(source, dgnDb);
    // may occurs if we have reprojection errors
    if (pTree->m_pRoot.IsNull())
        return nullptr;

    return pTree;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterQuadTree::RasterQuadTree(RasterSourceR source, DgnDbR dgnDb)
:m_pSource(&source), // increment ref count.
 m_dgnDb(dgnDb),
 m_tileCache(s_GetSharedTileCache())
    {    
    // Compute reprojection matrix if any.
    ComputeSourceToWorldTransform(m_sourceToWorld);  //&&MM detect error and do something about it.
    
    // Create the lowest LOD but do not load its data yet.
    m_pRoot = RasterTile::CreateRoot(*this);

    m_visibleQualityFactor = 1.25; 
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
struct ReprojectionApproximater
    {

    ReprojectionApproximater(GeoCoordinates::BaseGCSR srcGcs, DgnGCSR dgnGcs) :m_sourceGCS(srcGcs), m_destGCS(dgnGcs)
        {
//         HFCPtr<HGF2DPolygonOfSegments> pSrcDomainLL = GetDomainShape(*pSourceGcs);
//         HFCPtr<HGF2DPolygonOfSegments> pDgnDomainLL = GetDomainShape(*pDgnGcs);
//         m_pDomainInLatLong  = pSrcDomainLL->IntersectShape(*pDgnDomainLL);       
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt AddPointUOR(DPoint3dCR uorPt)
        {
        DPoint3d srcCartesianPt;

        ReprojectStatus status;

        GeoPoint dgnGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.LatLongFromUors(dgnGeoPt, uorPt))))
            return ERROR;

        GeoPoint srcGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.LatLongFromLatLong(srcGeoPt, dgnGeoPt, m_sourceGCS))))
            return ERROR;
        
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.CartesianFromLatLong(srcCartesianPt, srcGeoPt))))
            return ERROR;

       
        AddTiePoints(srcCartesianPt, uorPt);

        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt ReprojectPoint(DPoint3dR uorPt, DPoint3dCR srcCartesianPt)
        {
        ReprojectStatus status;

        // Source cartesian to lat/long.
        GeoPoint srcGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.LatLongFromCartesian(srcGeoPt, srcCartesianPt))))
            return ERROR;

        // Source lat/long to DgnDb lat/long.
        GeoPoint dgnGeoPt;
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_sourceGCS.LatLongFromLatLong(dgnGeoPt, srcGeoPt, m_destGCS))))
            return ERROR;

        // Finally to UOR
        if (REPROJECT_Success != (status = s_FilterGeocoordWarning(m_destGCS.UorsFromLatLong(uorPt, dgnGeoPt))))
            return ERROR;

        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt AddPoint(DPoint3dCR srcCartesianPt)
        {
        DPoint3d uorPt;
        if (SUCCESS != ReprojectPoint(uorPt, srcCartesianPt))
            return ERROR;            

        AddTiePoints(srcCartesianPt, uorPt);
        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    void AddTiePoints(DPoint3dCR badPt, DPoint3dCR goodPt)
        {
        m_tiePoints.push_back(badPt);
        m_tiePoints.push_back(goodPt);
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    StatusInt ComputeProjective(DMatrix4dR out)
        {
        if (0 != HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(out.coff, (uint16_t)m_tiePoints.size() * 3, (double const*) m_tiePoints.data()))
            return ERROR;
        
        return SUCCESS;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  7/2016
    //----------------------------------------------------------------------------------------
    static HFCPtr<HGF2DPolygonOfSegments> GetDomainShape(GeoCoordinates::BaseGCSR geoCs)
        {
        bvector<GeoPoint> geoDomainLL;  // in lat/long
        geoCs.GetMathematicalDomain(geoDomainLL);

        HGF2DCoordCollection<double> geoPoints;
        for (GeoPoint const& geoPt : geoDomainLL)
            geoPoints.push_back(HGF2DCoord<double>(geoPt.longitude, geoPt.latitude));

        HFCPtr<HGF2DPolygonOfSegments> pDomainShape(new HGF2DPolygonOfSegments(geoPoints));

        return pDomainShape;
        }

    void ClearAll() { m_tiePoints.clear(); }
    size_t GetTiePointCount() {return m_tiePoints.size()/2;}

    //HFCPtr<HGF2DShape>    m_pDomainInLatLong;
    std::vector<DPoint3d> m_tiePoints;

    GeoCoordinates::BaseGCSR m_sourceGCS;
    DgnGCSR                  m_destGCS;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
StatusInt RasterQuadTree::ComputeSourceToWorldTransform(DMatrix4dR sourceToWorld)
    {
    sourceToWorld.InitIdentity();

    GeoCoordinates::BaseGCSP pSourceGcs = GetSource().GetGcsP();
    DgnGCSP pDgnGcs = GetDgnDb().Units().GetDgnGCS();

    if (nullptr == pSourceGcs || nullptr == pDgnGcs || !pSourceGcs->IsValid() || !pDgnGcs->IsValid())
        return SUCCESS; // Assume raster to be coincident.
        
    //&&MM test for same GCS but also make sure units or UOR factor are applied
    //!m_pDgnGCS->IsEquivalent(*pRasterGcs)

    // Create a shapeCartesian in with src corners.
    DPoint3dCP sourceCorners = &GetSource().GetCorners();
    
    ReprojectionApproximater approx(*pSourceGcs, *pDgnGcs);

    double seed[] = {0.15, 0.5, 0.85};
    size_t  seedCount = sizeof(seed) / sizeof(seed[0]);

    bool hadFailure = false;
    for (size_t y = 0; y < seedCount; ++y)
        {
        for (size_t x = 0; x < seedCount; ++x)
            {
            if (SUCCESS != approx.AddPoint(DPoint3d::FromInterpolateBilinear(sourceCorners[0], sourceCorners[1], sourceCorners[2], sourceCorners[3], seed[x], seed[y])))
                {
                //&&MM and do what? move toward the center of the raster or domain or project extent.
                hadFailure = true;
                x = y = seedCount;
                }
            }
        }

    //&&MM we should test that we have enough pt if not do something else?
    // even if we have enough the distribution might not be good enough.
#if 0
    if (hadFailure)
        {
        approx.ClearAll();

        // Project contains extent of geometries only. ->> not used right now.
        AxisAlignedBox3d projectExtent = GetDgnDb().Units().GetProjectExtents();

        DPoint3d box[8];
        projectExtent.Get8Corners(box);
        for (size_t y = 0; y < seedCount; ++y)
            {
            for (size_t x = 0; x < seedCount; ++x)
                {
                if (SUCCESS != approx.AddPointUOR(DPoint3d::FromInterpolateBilinear(box[0], box[1], box[2], box[3], seed[x], seed[y])))
                    {
                    //&&MM and do what? move toward the center of the raster or domain or project extent.
//                     hadFailure = true;
//                     x = y = seedCount;
                    }
                }
            }
        }
#endif
    
    return approx.ComputeProjective(sourceToWorld);    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterQuadTree::QueryVisible(std::vector<RasterTilePtr>& visibles, ViewContextR context)
    {
    visibles.clear();

    if(m_pRoot.IsValid()) // Might be null if reprojection error occurs.
        m_pRoot->QueryVisible(visibles, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterQuadTree::Draw(RenderContextR context)
    {
    // First, determine if we should draw tiles at all.
    if (!RasterProgressiveDisplay::ShouldDrawInContext(context) || NULL == context.GetViewport())
        return;

    RefCountedPtr<RasterProgressiveDisplay> display = RasterProgressiveDisplay::Create(*this, context);
    display->Draw(context);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterQuadTree::DropGraphicsForViewport(Dgn::DgnViewportCR viewport)
    {
    if (m_pRoot.IsValid())
        m_pRoot->DropGraphicsForViewport(viewport);
    }

//----------------------------------------------------------------------------------------
//-------------------------------  RasterProgressiveDisplay -----------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterProgressiveDisplay::RasterProgressiveDisplay(RasterQuadTreeR raster, RenderContextR context)
:m_raster(raster),
 m_viewport(context.GetViewportR())
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
RasterProgressiveDisplay::~RasterProgressiveDisplay()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RefCountedPtr<RasterProgressiveDisplay> RasterProgressiveDisplay::Create(RasterQuadTreeR rasterTree, RenderContextR context)
    {
    return new RasterProgressiveDisplay(rasterTree, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool RasterProgressiveDisplay::ShouldDrawInContext(RenderContextR context)
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ClashDetection:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
bool RasterProgressiveDisplay::DrawFinerTiles(RasterTileR tile, uint32_t resolutionDelta, DrawArgs& drawArgs)
    {
    if(0 == resolutionDelta)
        return false;

    bool allPartsCovered = true;

    for(uint32_t i=0; i < 4; ++i)
        {
        RasterTileP pChild = tile.GetChildP(i);
        if (NULL == pChild)
            {
            allPartsCovered = false;
            continue;
            }

        if (!pChild->Draw(drawArgs))
            allPartsCovered = DrawFinerTiles(*pChild, resolutionDelta-1, drawArgs);
        }

    return allPartsCovered;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
void RasterProgressiveDisplay::FindCoarserTiles(SortedTiles& coarserTiles, std::vector<RasterTilePtr> const& visibleTiles, uint32_t resolutionDelta, DgnViewportCR viewport)
    {
    for (auto const& pTile : visibleTiles)
        {
        if (!pTile->HasCachedGraphic(viewport))
            {
            RasterTileP pCoarserTile = pTile->FindCachedCoarserTile(resolutionDelta, viewport);
            if (nullptr != pCoarserTile)
                coarserTiles.insert(pCoarserTile);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterProgressiveDisplay::Draw (RenderContextR context)
    {
    BeAssert(nullptr != context.GetViewport()); // a scene should always have a viewport.

    m_raster.QueryVisible(m_visiblesTiles, context);

    std::set<RasterTileP> coarserTiles; // use a set so we do not draw them more than once.

    DrawArgs drawArgs(context);
    DrawArgs finerDrawArgs(context);

    // Draw what we have in cache, everything else will be generated in the background.
    for (RasterTilePtr& pTile : m_visiblesTiles)
        {
        if (!pTile->Draw(drawArgs))
            {
            // Draw finer resolution as substitute while the needed tile is computed.
            if (!DrawFinerTiles(*pTile, DRAW_FINER_DELTA, finerDrawArgs))
                {
                // If finer tiles did not cover all area, we need to draw a coarser tile to fill the void.
                RasterTileP pCoarserTile = pTile->FindCachedCoarserTile(DRAW_COARSER_DELTA, context.GetViewportR());
                if (nullptr != pCoarserTile)
                    coarserTiles.insert(pCoarserTile);
                }

            BeAssert(!pTile->HasCachedGraphic(context.GetViewportR()));

            TileDataQueryPtr pQuery = new TileDataQuery(*pTile, context.GetTargetR());
            ThreadPool::GetInstance().Enqueue(*pQuery);
            m_queriedTiles.emplace_back(pQuery);
            }
        }

    // Draw coarser tiles that we have.
    DrawArgs coarserDrawArgs(context);
    for (auto& pCoarserTile : coarserTiles)
        {
        BeAssert(pCoarserTile->HasCachedGraphic(context.GetViewportR()));
        pCoarserTile->Draw(coarserDrawArgs);
        }
    coarserDrawArgs.DrawGraphics(DRAW_COARSER_BIAS);

    // Finer...
    finerDrawArgs.DrawGraphics(DRAW_FINER_BIAS);

    drawArgs.DrawGraphics(0.0);

    if(!m_queriedTiles.empty())
        context.GetViewportR().ScheduleTerrainProgressiveTask(*this);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion RasterProgressiveDisplay::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    DrawArgs drawArgs(context);

    for (auto pTileQueryItr = m_queriedTiles.begin(); pTileQueryItr != m_queriedTiles.end();  /* incremented or erased in loop*/ )
        {
        TileDataQueryPtr pTileQuery = (*pTileQueryItr).get();

        if (!pTileQuery->IsFinished())
            {
            ++pTileQueryItr;    // Will check back later
            continue;
            }

        pTileQueryItr = m_queriedTiles.erase(pTileQueryItr);

        if (pTileQuery->GetTileP() != nullptr)
            {
            RasterTileR tileNode = pTileQuery->GetTileNodeR();

            auto pTileGraphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport()));

            Render::GraphicParams graphicParams;
            graphicParams.SetLineColor(ColorDef::White());
            graphicParams.SetFillColor(ColorDef::White());
            graphicParams.SetWidth(1);
            pTileGraphic->ActivateGraphicParams(graphicParams);
            pTileGraphic->AddTile(*pTileQuery->GetTileP(), tileNode.GetCorners());
            
            tileNode.SaveGraphic(context.GetViewportR(), *pTileGraphic);
            tileNode.Draw(drawArgs);

            // CheckStop only if we drawn something.
            if (context.CheckStop())
                break;
            }
        }

    drawArgs.DrawGraphics(0.0);

    if (m_queriedTiles.empty())
        {
        //&&MM TODO Needs heal.
//         if (Raster has transparency/translucency)
//             context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones due to transparency
//         else
               wantShow = WantShow::Yes;

        return Completion::Finished;
        }        

    return Completion::Aborted;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void DrawArgs::DrawGraphics(double zBias)
    {
    if (m_graphics.m_entries.empty())
        return;

    //&&MM if we have rasters in 3d space, other than top view, or we see the raster from underneath that z-bias trick won't work.
    //     That z-bias must be perpendicular to the view plane. Review when we will support raster plane et layerInModel(only for 2d top view?!?).
    Transform biasTrans;
    biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, zBias));
    auto group = m_context.CreateBranch(m_graphics, &biasTrans);
    m_context.OutputGraphic(*group, nullptr);
    }
