/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDataRefCachingManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

#include "DTMDataRefXAttribute.h"
#include "MrDTMDataRef.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void DTMDisplayCache::AddAppData (ElementHandleCR element, DTMDisplayCache* cache)
    {
    BeAssert (element.IsPersistent() && element.GetElementRef());

    ElementRefP elementRef = element.GetElementRef();

    if (elementRef)
        elementRef->AddAppData (s_key, cache, elementRef->GetHeapZone());
    }

void DTMDisplayCache::DropAppData (ElementHandleCR element)
    {
    BeAssert (element.IsPersistent() && element.GetElementRef());

    ElementRefP elementRef = element.GetElementRef();

    if (elementRef)
        elementRef->DropAppData (s_key);
    }

DTMDisplayCache* DTMDisplayCache::GetAppData (ElementHandleCR element)
    {
    BeAssert (element.IsPersistent() && element.GetElementRef());

    ElementRefP elementRef = element.GetElementRef();

    if (elementRef)
        {
        DTMDisplayCache* appData = static_cast<DTMDisplayCache*>(elementRef->FindAppData (s_key));
        return appData;
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
bool CheckStop(ViewContextR context)
    {
    return context.CheckStop();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
static void WaitForQvThread()
    {
    // DeleteQvCache is called as the function I want to call isn't accessable, this does the call and then exits.
    //ToDo QvOutput::DeleteQvCache (nullptr)
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
static void DeleteQvElem(QvElem* qvElem)
    {
    T_HOST.GetGraphicsAdmin()._DeleteQvElem (qvElem);
    }

ElementRefAppData::Key DTMDisplayCache::s_key;

list<DSHandlerKeyStorage*> dsKeyStorage;

inline bool FilterMatches (DisplayStyleHandlerKeyPtr a, DisplayStyleHandlerKeyPtr b)
    {
    DisplayStyleHandlerKey* aP = a.get();
    DisplayStyleHandlerKey* bP = b.get();
    if (aP == bP)
        return true;

    if (!aP || !bP)
        return false;

    return aP->Matches (*b.get());
    }

DSHandlerKeyStoragePtr GetCommonHandlerKey (DisplayStyleHandlerKeyPtr key)
    {
    if (key.IsNull())
        return nullptr;

    for (DSHandlerKeyStorage* s : dsKeyStorage)
        {
        if (FilterMatches (key, s->m_key))
           return s;
        }    
    DSHandlerKeyStorage* s = DSHandlerKeyStorage::Create (key);
    dsKeyStorage.push_back (s);
    return s;
    }

void RemoveCommonHandlerKey (DSHandlerKeyStorage* key)
    {
    dsKeyStorage.remove (key);
    }

DSHandlerKeyStorage* DSHandlerKeyStorage::Create (Bentley::DgnPlatform::DisplayStyleHandlerKeyPtr key)
    {
    return new DSHandlerKeyStorage (key);
    }

DSHandlerKeyStorage::~DSHandlerKeyStorage()
    {
    RemoveCommonHandlerKey (this);
    }

QvCacheP DTMDisplayCacheManager::m_qvCache = nullptr;
bmap <DTMDisplayCache*, DTMDisplayCache*> DTMDisplayCacheManager::s_displayCaches;

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
ModelElementViewDescription GetModelElementViewDescription (UInt32 id, int view, int tileIndex)
    {
    return ModelElementViewDescription (id, view, tileIndex);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
TiledModelElementFilterDescription GetTiledModelElementFilterDescription (UInt32 id, DSHandlerKeyStoragePtr filter, int tileIndex)
    {
    return TiledModelElementFilterDescription (id, filter, tileIndex);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMQvCacheDetails::Release() const
    {
    if (m_count == 1)
        delete this;
    else
        m_count--;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMQvCacheDetails::AddRef() const
    {
    m_count++;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache::~DTMDataRefQvCache()
    {
    if (m_details != nullptr)
        {
        m_details = nullptr;
        }
    if (m_qvElem)
        DeleteQvElem (m_qvElem);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDisplayCache::DTMDisplayCache()
    {
    DTMDisplayCacheManager::s_displayCaches[this] = this;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDisplayCache::~DTMDisplayCache()
    {
    RemoveAll();
    DTMDisplayCacheManager::s_displayCaches.erase (this);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/13
//=======================================================================================
void DTMDisplayCache::RemoveAll ()
    {
    for (DTMDataRefQvCache* qvCache : m_qvElemView)
        {
        DeleteDTMDataRefQvCache (qvCache);
        }    
    m_qvElemView.clear();
    m_elementToQvCache.clear();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCache::GetCacheElem (UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details)
    {
    if (!context.GetViewport())
        return nullptr;

    ModelElementViewDescription desc = GetModelElementViewDescription (id, context.GetViewport()->GetViewNumber(), -1);
    bmap<ModelElementViewDescription, DTMDataRefQvCache*>::const_iterator it = m_elementToQvCache.find (desc);

    // Get the current Cache for this display. (model id /element id/sub id/view num.
    if (it != m_elementToQvCache.end())
        {
        // DH - Matched a cached Element is the details the same.
        DTMDataRefQvCache* qvCache = it->second;
        // Need to check if qvCache still is valid for this el.
        // If not remove reference and if 0 then remove entry.
        // and Set map entry = nullptr
        if (filter == qvCache->m_filter && type == qvCache->m_type)
            {
            if (details == nullptr && qvCache->m_details == nullptr)
                return qvCache;

            if (details != nullptr && qvCache->m_details != nullptr && qvCache->m_details->UseCache (details))
                {
                if (!qvCache->m_details->m_checkStop)
                    return qvCache;
                }
            }

        // DH - Element is no longer valid so delete.
        qvCache->m_refCount--;
        if (qvCache->m_refCount == 0)
            {
            DeleteDTMDataRefQvCache (qvCache, true);
            m_qvElemView.remove(qvCache);
            }

        m_elementToQvCache.erase(desc);
        }

    // DH - See if there is another match
    if (details)
        {
        for (DTMDataRefQvCache* cache : m_qvElemView)
            {
            // See if this element matches
            if (cache->m_type == type && cache->m_filter == filter)
                {
                if ((details == nullptr && cache->m_details == nullptr) || (details != nullptr && cache->m_details != nullptr && cache->m_details->UseCache (details)))
                    {
                    m_elementToQvCache[desc] = cache;
                    cache->m_refCount++;
                    return cache;
                    }
                }
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCache::CreateCacheElemAndDraw (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroker)
    {
    if (!context.GetViewport())
        return nullptr;

    ModelElementViewDescription desc = GetModelElementViewDescription (id, context.GetViewport()->GetViewNumber(), -1);
    bmap<ModelElementViewDescription, DTMDataRefQvCache*>::const_iterator it = m_elementToQvCache.find (desc);
    DTMDataRefQvCache* qvCache = nullptr;

    if (it != m_elementToQvCache.end())
        {
        BeAssert (false);
        qvCache = it->second;
        if (qvCache->m_qvElem)
            DeleteQvElem(qvCache->m_qvElem);
        qvCache->m_qvElem = nullptr;
        qvCache->m_type = type;
        qvCache->m_details = details;
        qvCache->m_filter = filter;
        qvCache->m_refCount++;
        }
    else
        {
        // Find to see if any existing caches matches this one.
        qvCache = new DTMDataRefQvCache();
        qvCache->m_refCount = 1;
        qvCache->m_type = type;
        qvCache->m_details = details;
        qvCache->m_filter = filter;
        qvCache->m_qvElem = nullptr;
        m_qvElemView.push_back(qvCache);
        m_elementToQvCache[desc] = qvCache;
        }

    if (!details)
        return nullptr;

    DTMQvCacheTilingDetails* tilingDetails;
    
    if (stroker.SupportTiling() && (tilingDetails = details->GetTilingDetails()) != nullptr)
        {
#ifdef SHOWTIMING
        DWORD  took = GetTickCount();

        tilingDetails->GetNumberOfTiles();
        took = GetTickCount() - took;
        char buffer[10000];
        sprintf(buffer, "GetNumberOfTiles %f\n", 0.001 * (double)took);
        OutputDebugString(buffer);

        DWORD tookGetTileDetail= 0;
        DWORD tookGetTiledCacheElem = 0;
        DWORD tookGetTileFence = 0;
        DWORD tookCreateTiledCacheElem = 0;
        DWORD tookDraw = 0;
#endif

        qvCache->m_qvElem = nullptr;
        int syncTimeout = 0;
        for (unsigned int index = 0; index < tilingDetails->GetNumberOfTiles(); index++)
            {
#ifdef SHOWTIMING
            tookGetTileDetail -= GetTickCount();
#endif

            RefCountedPtr<DTMQvCacheDetails> tileDetails = tilingDetails->GetTileDetail(index);

#ifdef SHOWTIMING
            tookGetTileDetail += GetTickCount();
            tookGetTiledCacheElem -= GetTickCount();
#endif
            DTMDataRefQvCache* qvElem = GetTiledCacheElem (id, context, type, filter, tileDetails.get());
#ifdef SHOWTIMING
            tookGetTiledCacheElem += GetTickCount();
#endif
            if (!qvElem || (!qvElem->m_qvElem && !qvElem->m_isEmpty))
                {
                if (!details->m_checkStop)
                    {
                    DPoint3d lowPt; DPoint3d highPt;
#ifdef SHOWTIMING
                    tookGetTileFence -= GetTickCount();
#endif
                    tilingDetails->GetTileFence (index, lowPt, highPt);
#ifdef SHOWTIMING
                    tookGetTileFence += GetTickCount();

                    tookCreateTiledCacheElem -= GetTickCount();
#endif
                    stroker.SetTileFence (lowPt, highPt);
                    qvElem = CreateTiledCacheElem (element, id, context, type, filter, tileDetails.get(), stroker);
#ifdef SHOWTIMING
                    tookCreateTiledCacheElem += GetTickCount();
#endif
                    if (CheckStop(context))
                        {
                        if (qvElem)
                            {
                            if (qvElem->m_qvElem)
                                {
                                DeleteQvElem(qvElem->m_qvElem);
                                qvElem->m_qvElem = nullptr;
                                }
                            }
                        details->m_checkStop = true;
                        }

                    if (qvElem)
                        {
                        qvCache->AddTile(qvElem);
#ifdef SHOWTIMING
                        tookDraw -= GetTickCount();
#endif
                        qvElem->DrawQVElem(context);

#ifdef SHOWTIMING
                        tookDraw += GetTickCount();
#endif
                        }
                    }
                }
            else
                {
                qvCache->AddTile(qvElem);
#ifdef SHOWTIMING
                tookDraw -= GetTickCount();
#endif
                if (!details->m_checkStop)
                    {
                    qvElem->DrawQVElem(context);
                    syncTimeout++;
                    if (syncTimeout % 10 == 0)
                        WaitForQvThread();

                    if (CheckStop(context))
                        details->m_checkStop = true;
                    }


#ifdef SHOWTIMING
                tookDraw += GetTickCount();
#endif
                }
            }
#ifdef SHOWTIMING
        sprintf(buffer, "Various %f %f %f %f %f\n", 0.001 * (double)tookGetTileDetail, 0.001 * (double) tookGetTiledCacheElem, 0.001 * (double)tookGetTileFence, 0.001 * (double)tookCreateTiledCacheElem, 0.001 * (double)tookDraw);
        OutputDebugString(buffer);
#endif
        }
    else
        {
        qvCache->m_qvElem = context.CreateCacheElem (element, DTMDisplayCacheManager::GetQvCache(), stroker, context.GetViewFlags(), 0);
#ifdef SHOWTIMING
        DWORD  took = GetTickCount();
#endif
        qvCache->DrawQVElem(context);
#ifdef SHOWTIMING
        took = GetTickCount() - took;
        char buffer[10000];
        sprintf(buffer, "Draw %f\n", 0.001 * (double)took);
        OutputDebugString(buffer);
#endif
        }


#ifdef SHOWTIMING
    DWORD  took = GetTickCount();
#endif
    DeleteDelayTiles();
#ifdef SHOWTIMING
    took = GetTickCount() - took;
    char buffer[10000];
    sprintf(buffer, "DeleteDelayTiles %f\n", 0.001 * (double)took);
    OutputDebugString(buffer);
#endif
    return qvCache;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/11
//=======================================================================================
void DTMDisplayCache::DeleteCacheElem()
    {
    bmap<ModelElementViewDescription, DTMDataRefQvCache*>::const_iterator it = m_elementToQvCache.begin();

    while (it != m_elementToQvCache.end())
        {
        DTMDataRefQvCache* qvCache = it->second;
        qvCache->m_refCount--;
        if (qvCache->m_refCount == 0)
            {
            DeleteDTMDataRefQvCache (qvCache);
            m_qvElemView.remove (qvCache);
            }
        }
        m_elementToQvCache.clear();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDisplayCache::DeleteCacheElem (UInt32 id)
    {
    ModelElementViewDescription desc = GetModelElementViewDescription(id, -1, -1);
    bmap<ModelElementViewDescription, DTMDataRefQvCache*>::iterator it = m_elementToQvCache.begin();

    while (it != m_elementToQvCache.end())
        {
        bool isSame = (id == 0xffffffff);

        if (!isSame)
            isSame = it->first.IsSameSubElement(desc);

        if (isSame)
            {
            DTMDataRefQvCache* qvCache = it->second;
            qvCache->m_refCount--;
            if (qvCache->m_refCount == 0)
                {
                DeleteDTMDataRefQvCache (qvCache);
                m_qvElemView.remove (qvCache);
                }
            it = m_elementToQvCache.erase(it);
            continue;
            }
        it++;
        }
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
void DTMDisplayCache::DeleteCacheForView (int view)
    {
    bmap<ModelElementViewDescription, DTMDataRefQvCache*>::iterator it = m_elementToQvCache.begin();
    while (it != m_elementToQvCache.end())
        {
        if (it->first.m_view == view)
            {
            DTMDataRefQvCache* qvCache = it->second;
            qvCache->m_refCount--;
            if (qvCache->m_refCount == 0)
                {
                DeleteDTMDataRefQvCache(qvCache);
                m_qvElemView.remove(qvCache);
                }
            it = m_elementToQvCache.erase (it);
            }
        else
            it++;
        }    
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCache::GetTiledCacheElem (UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details)
    {
    if (!context.GetViewport())
        return nullptr;

    TiledModelElementFilterDescription desc = GetTiledModelElementFilterDescription (id, filter, details->GetTileIndex());
    bmap<TiledModelElementFilterDescription, DTMDataRefQvCache*>::const_iterator it = m_elementTileToQvCache.find(desc);

    if (it != m_elementTileToQvCache.end() && it->second)
        {
        DTMDataRefQvCache* qvCache = it->second;
        // Need to check if qvCache still is valid for this el.
        // If not remove reference and if 0 then remove entry.
        // and Set map entry = nullptr
        if (filter == qvCache->m_filter && type == qvCache->m_type)
            {
            if (details == nullptr && qvCache->m_details == nullptr)
                return qvCache;

            if (details != nullptr && qvCache->m_details != nullptr)
                return qvCache;
            }
        }

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCache::CreateTiledCacheElem (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroker)
    {
    if (!context.GetViewport())
        return nullptr;
    TiledModelElementFilterDescription desc = GetTiledModelElementFilterDescription(id, filter, details->GetTileIndex());
    bmap<TiledModelElementFilterDescription, DTMDataRefQvCache*>::const_iterator it = m_elementTileToQvCache.find(desc);
    DTMDataRefQvCache* qvCache = nullptr;

    if (it != m_elementTileToQvCache.end() && it->second)
        {
        qvCache = it->second;
        if (qvCache->m_qvElem)
            DeleteQvElem(qvCache->m_qvElem);
        qvCache->m_qvElem = nullptr;
        qvCache->m_type = type;
        qvCache->m_details = details;
        qvCache->m_filter = filter;
//        qvCache->m_refCount++;
        }
    else
        {
        // Find to see if any existing caches matches this one.
        qvCache = new DTMDataRefQvCache();
        qvCache->m_refCount = 0;
        qvCache->m_type = type;
        qvCache->m_details = details;
        qvCache->m_filter = filter;
        qvCache->m_qvElem = nullptr;
        m_elementTileToQvCache[desc] = qvCache;
        }

    qvCache->m_qvElem = context.CreateCacheElem (element, DTMDisplayCacheManager::GetQvCache(), stroker, context.GetViewFlags(), 0); 
    if (!qvCache->m_qvElem && !CheckStop (context))
        qvCache->m_isEmpty = true;

    return qvCache;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDisplayCache::DeleteDelayTiles ()
    {
    bmap<TiledModelElementFilterDescription, DTMDataRefQvCache*>::iterator it = m_elementTileToQvCache.begin ();

    while (it != m_elementTileToQvCache.end())  
        {
        if (it->second && it->second->m_refCount == 0)
            {
            DeleteDTMDataRefQvCache (it->second, true);
            it->second = nullptr;
            }
        it++;
        }
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDisplayCache::DeleteDTMDataRefQvCache (DTMDataRefQvCache* qvCache, bool delayTileDelete)
    {
    bool hasTiles = qvCache->m_tiles.size() != 0;
    for (unsigned int i = 0; i < qvCache->m_tiles.size(); i++)
        qvCache->m_tiles[i]->m_refCount--;

    delete qvCache;

    if (!delayTileDelete && hasTiles)
        DeleteDelayTiles ();
    }

//=======================================================================================
// @bsiclass                                                    Daryl.Holmwood  04/10
//=======================================================================================
class DTMViewMonitor : public Bentley::DgnPlatform::IViewMonitor
    {
    public: DgnModelRefP m_modelRefs[8];
    //=======================================================================================
    // @bsimethod                                                    Daryl.Holmwood  04/10
    //=======================================================================================
    public: void Initalize()
        {
        for (int view = 0; view < 8; view++)
            {
            m_modelRefs[view] = nullptr;
            ViewportP viewport = Bentley::DgnPlatform::IViewManager::GetActiveViewSet ().GetViewport (view);
            if (viewport)
                _OnViewOpen(viewport);
            }
        }

    //=======================================================================================
    // @bsimethod                                                    Daryl.Holmwood  04/10
    //=======================================================================================
    public: virtual void _OnViewChanged (ViewportP viewport) override
        {
        int view = viewport->GetViewNumber();
        if (view == -1)
            return;
        if (m_modelRefs[view] != viewport->GetTargetModel())
            {
            if (m_modelRefs[view])
                DTMDisplayCacheManager::DeleteCacheForView (view);
            m_modelRefs[view] = viewport->GetTargetModel();
            }
        }

    //=======================================================================================
    // @bsimethod                                                    Daryl.Holmwood  04/10
    //=======================================================================================
    public: virtual void _OnViewOpen (ViewportP viewport) override
        {
        int view = viewport->GetViewNumber();
        if (view == -1)
            return;
        m_modelRefs[view] = viewport->GetTargetModel();
        }

    //=======================================================================================
    // @bsimethod                                                    Daryl.Holmwood  04/10
    //=======================================================================================
    public: virtual void _OnViewClose (ViewportP viewport) override
        {
        int view = viewport->GetViewNumber();
        if (view == -1)
            return;
        if (m_modelRefs[view])
            DTMDisplayCacheManager::DeleteCacheForView (view);
        m_modelRefs[view] = nullptr;
        }
    };

static DTMViewMonitor viewMonitor;

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/10
//=======================================================================================
void DTMDisplayCacheManager::Initialize()
    {
    viewMonitor.Initalize();
    Bentley::DgnPlatform::IViewManager::GetManager().AddViewMonitor (&viewMonitor);
    m_qvCache = nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  02/10
//=======================================================================================
void DTMDisplayCacheManager::Uninitialize()
    {
    Bentley::DgnPlatform::IViewManager::GetManager().DropViewMonitor (&viewMonitor);

    if (m_qvCache)
        {
        T_HOST.GetGraphicsAdmin()._DeleteQvCache (m_qvCache);
        m_qvCache = nullptr;
        }
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
QvCacheP DTMDisplayCacheManager::GetQvCache()
    {
    if (m_qvCache == nullptr)
        m_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache();

    return m_qvCache;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCacheManager::GetCacheElem (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details)
    {
    if (!context.GetIViewDraw().IsOutputQuickVision() || !element.IsPersistent ())
        return nullptr;

    DTMDisplayCache* cache = DTMDisplayCache::GetAppData (element);

    if (cache)
        return cache->GetCacheElem (id, context, type, filter, details);
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMDataRefQvCache* DTMDisplayCacheManager::CreateCacheElemAndDraw (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroker)
    {
    if (!context.GetIViewDraw().IsOutputQuickVision() || !element.IsPersistent ())
        return nullptr;

    DTMDisplayCache* cache = DTMDisplayCache::GetAppData (element);

    if (!cache)
        {
        cache = new DTMDisplayCache ();
        DTMDisplayCache::AddAppData (element, cache);
        }

    if (cache)
        return cache->CreateCacheElemAndDraw (element, id, context, type, filter, details, stroker);
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDisplayCacheManager::DeleteCacheElem (ElementHandleCR element)
    {
    DTMDisplayCache* cache = DTMDisplayCache::GetAppData (element);
    
    if (cache != 0)
        cache->DeleteCacheElem (0xffffffff);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDisplayCacheManager::DeleteCacheElem (ElementHandleCR element, UInt32 id)
    {
    DTMDisplayCache* cache = DTMDisplayCache::GetAppData (element);
    if (cache)
        cache->DeleteCacheElem (id);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  06/10
//=======================================================================================
void DTMDisplayCacheManager::DeleteCacheForView (int view)
    {
    bmap<DTMDisplayCache*, DTMDisplayCache*>::const_iterator it = s_displayCaches.begin();

    while (it != s_displayCaches.end())
        {
        it->second->DeleteCacheForView (view);
        it++;
        }    
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMDataRefQvCache::DrawQVElem (ViewContextR context)
    {
    for (unsigned int i = 0; i < m_tiles.size(); i++)
        {
        if (CheckStop(context))
            return;
        m_tiles[i]->DrawQVElem(context);
        if (i % 10 == 9) WaitForQvThread();
        }
    if (m_qvElem)
        context.GetIViewDraw().DrawQvElem3d (m_qvElem, 0);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
