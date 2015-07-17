/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDataRefCachingManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel\ElementHandler\DTMDataRef.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNPLATFORM

enum DTMQvCacheType
    {
    Triangles,
    Region,
    Banding,
    Ponds
    };

struct DTMQvCacheDetails;

struct ModelElementViewDescription
    {
    int m_view;
    UInt32 m_xattrId;
    int m_tileIndex;

    ModelElementViewDescription ()  // Needed for bmap
        {
        }

    ModelElementViewDescription (UInt32 xAttrId, int view, int tileIndex) 
        {
        m_xattrId = xAttrId;
        m_view = view;
        m_tileIndex = tileIndex;
        }
    inline bool IsSameSubElement (const ModelElementViewDescription& s2) const
        {
        return (m_xattrId == s2.m_xattrId);
        }
    inline bool operator== (const ModelElementViewDescription& s2) const
        {
        return m_xattrId == s2.m_xattrId && m_view == s2.m_view && m_tileIndex == s2.m_tileIndex;
        }
    inline bool operator< (const ModelElementViewDescription& s2) const
        {
        if (m_xattrId != s2.m_xattrId)
            return m_xattrId < s2.m_xattrId;
        if (m_view != s2.m_view)
            return m_view < s2.m_view;
        return m_tileIndex < s2.m_tileIndex;
        }
    };

struct DSHandlerKeyStorage : public Bentley::RefCountedBase
    {
    private: DTMELEMENT_EXPORT DSHandlerKeyStorage (Bentley::DgnPlatform::DisplayStyleHandlerKeyPtr key)
        {
        m_key = key;
        }
    public: virtual ~DSHandlerKeyStorage ();

    static DTMELEMENT_EXPORT DSHandlerKeyStorage* Create (Bentley::DgnPlatform::DisplayStyleHandlerKeyPtr key);
    Bentley::DgnPlatform::DisplayStyleHandlerKeyPtr m_key;
    };

typedef Bentley::RefCountedPtr <DSHandlerKeyStorage> DSHandlerKeyStoragePtr;

DTMELEMENT_EXPORT DSHandlerKeyStoragePtr GetCommonHandlerKey (Bentley::DgnPlatform::DisplayStyleHandlerKeyPtr key);

struct TiledModelElementFilterDescription
    {
    DSHandlerKeyStoragePtr m_filter;
    UInt32 m_xattrId;
    int m_tileIndex;

    TiledModelElementFilterDescription()    // Needed for bmap
        {
        }
    TiledModelElementFilterDescription (UInt32 xAttrId, DSHandlerKeyStoragePtr filter, int tileIndex)
        {
        m_xattrId = xAttrId;
        m_filter = filter;
        m_tileIndex = tileIndex;
        }
    inline bool IsSameSubElement (const TiledModelElementFilterDescription& s2) const
        {
        return (m_xattrId == s2.m_xattrId);
        }
    inline bool operator== (const TiledModelElementFilterDescription& s2) const
        {
        return m_xattrId == s2.m_xattrId && m_tileIndex == s2.m_tileIndex && m_filter != s2.m_filter;
        }
    inline bool operator< (const TiledModelElementFilterDescription& s2) const
        {
        if (m_xattrId != s2.m_xattrId)
            return m_xattrId < s2.m_xattrId;
        if (m_filter != s2.m_filter)
            return m_filter.get() < s2.m_filter.get();
        return m_tileIndex < s2.m_tileIndex;
        }

    };

struct DTMQvCacheTilingDetails
    {
    virtual size_t GetNumberOfTiles () = 0;
    virtual DTMQvCacheDetails* GetTileDetail (unsigned int index) = 0;
    virtual void GetTileFence (unsigned index, DPoint3d& lowPt, DPoint3d& highPt) = 0;
    };

struct DTMQvCacheDetails
    {
    protected:
    mutable int m_count;
    int m_tileIndex;
    public: bool m_checkStop;
    public: DTMELEMENT_EXPORT void AddRef() const;
    public: DTMELEMENT_EXPORT void Release() const;
    public: DTMQvCacheDetails()
                {
                m_count = 0;
                m_tileIndex = -1;
                m_checkStop = false;
                }
    public: virtual ~DTMQvCacheDetails() {}
    public: virtual bool UseCache(DTMQvCacheDetails* details) = 0;
    public: virtual DTMQvCacheTilingDetails* GetTilingDetails()
                {
                return nullptr;
                }
    public:
        int GetTileIndex()
            {
            return m_tileIndex;
            }
    };

struct DTMQvCacheDetailsRange : public DTMQvCacheDetails
    {
    bool m_useContainedRange;
    DRange3d range;

    public: DTMQvCacheDetailsRange(DRange3dCR range)
                {
                m_useContainedRange = true;
                this->range = range;
                }
    public: virtual bool UseCache(DTMQvCacheDetails* details) override
                {
                DTMQvCacheDetailsRange* details2 = dynamic_cast<DTMQvCacheDetailsRange*>(details);
                if (m_useContainedRange)
                    return isContainedXY(&details2->range, &range, 0.00001) != 0;
                return isSame(&details2->range, &range, 0.00001) != 0;
                }
    private: bool isContainedXY(DRange3dCP inner, DRange3dCP outer, const double& tol)
                 {
                 if ((inner->low.x - outer->low.x) < -tol) return false;
                 if ((inner->low.y - outer->low.y) < -tol) return false;
                 if ((outer->high.x - inner->high.x) < -tol) return false;
                 if ((outer->high.y - inner->high.y) < -tol) return false;
                 return true;
                 }
    private: bool isSame(DRange3dCP inner, DRange3dCP outer, const double& tol)
                 {
                 if (fabs(inner->low.x - outer->low.x) > tol) return false;
                 if (fabs(inner->low.y - outer->low.y) > tol) return false;
                 if (fabs(outer->high.x - inner->high.x) > tol) return false;
                 if (fabs(outer->high.y - inner->high.y) > tol) return false;
                 return true;
                 }
    };

struct MrDTMQvCacheDetails : public DTMQvCacheDetails
    {        
    ViewGeomInfoP m_geomInfoP;        
    double        m_pointDensityForShadedView;    
    double        m_pointDensityForWireframeView;
    DrawPurpose   m_drawPurpose;    
    DMatrix4d     m_localToViewTransformation;
    
    //ElementHiliteState  m_elementHiliteState;
    bool                m_needToDrapeRaster;

    
    public: MrDTMQvCacheDetails(ViewGeomInfoCP   geomInfoP,   
                                const DMatrix4d& localToViewTransformation,
                                double           pointDensityForShadedView,    
                                double           pointDensityForWireframeView,
                                bool             needToDrapeRaster, 
                                DrawPurpose      drawPurpose)
                {                                    
                if (geomInfoP != 0)
                    {
                    m_geomInfoP = new ViewGeomInfo;
                    memcpy(m_geomInfoP, geomInfoP, sizeof(ViewGeomInfo));
                    }
                else
                    {
                    m_geomInfoP = 0;
                    }
                
                m_drawPurpose = drawPurpose;
                memcpy(&m_localToViewTransformation, &localToViewTransformation, sizeof(DMatrix4d));

                m_needToDrapeRaster = needToDrapeRaster; 
                m_pointDensityForShadedView = pointDensityForShadedView;
                m_pointDensityForWireframeView = pointDensityForWireframeView;                                

                }

    public: virtual ~MrDTMQvCacheDetails()
                {
                if (m_geomInfoP != 0)
                    {
                    delete m_geomInfoP;
                    }
                }

    public: virtual bool UseCache(DTMQvCacheDetails* details) override
                {
                MrDTMQvCacheDetails* mrDetails = (MrDTMQvCacheDetails*)details;
                                
                return ((m_needToDrapeRaster == false) && 
                        (m_geomInfoP != 0) &&
                        (mrDetails->m_geomInfoP != 0) &&
                        (memcmp(m_geomInfoP, mrDetails->m_geomInfoP, sizeof(ViewGeomInfo)) == 0) &&                                                
                        (memcmp(&m_localToViewTransformation, &mrDetails->m_localToViewTransformation, sizeof(DMatrix4d)) == 0) &&                                                
                        (m_pointDensityForShadedView == mrDetails->m_pointDensityForShadedView) &&
                        (m_pointDensityForWireframeView == mrDetails->m_pointDensityForWireframeView) && 
                        (m_drawPurpose != DrawPurpose::UpdateDynamic));
                }
    };

struct DTMDataRefQvCache
    {
    bvector<DTMDataRefQvCache*> m_tiles;
public:
    int m_refCount;
    bool m_isEmpty;
    DTMQvCacheType m_type;
    DSHandlerKeyStoragePtr m_filter;
    // Details.
    RefCountedPtr<DTMQvCacheDetails> m_details;
    QvElemP m_qvElem;

    DTMDataRefQvCache()
        {
        m_isEmpty = false;
        }
    virtual ~DTMDataRefQvCache();
    void DrawQVElem(ViewContextR context);
public:
    void AddTile(DTMDataRefQvCache* tile)
        {
        tile->m_refCount++;
        m_tiles.push_back(tile);
        }
    };

struct IDTMStrokeForCache : public Bentley::DgnPlatform::IStrokeForCache
    {
    public:
        virtual bool SupportTiling() { return false; }
        virtual void SetTileFence(DPoint3d& lowPt, DPoint3d& highPt) {};
    };

struct DTMDisplayCache : ElementRefAppData
    {
private:
    static Key s_key;

    std::list<DTMDataRefQvCache*> m_qvElemView;
    bmap<ModelElementViewDescription, DTMDataRefQvCache*> m_elementToQvCache;
    bmap<TiledModelElementFilterDescription, DTMDataRefQvCache*> m_elementTileToQvCache;
public:
    static void AddAppData(ElementHandleCR element, DTMDisplayCache* cache);
    static void DropAppData(ElementHandleCR element);
    static DTMDisplayCache* GetAppData(ElementHandleCR element);

protected:
    void RemoveAll();
public:
    DTMDisplayCache();
    virtual ~DTMDisplayCache();

    virtual void _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
        {
        delete this;
        }

    virtual bool _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason) override
        {
        return false;
        }

    DTMDataRefQvCache* GetCacheElem (UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details);
    DTMDataRefQvCache* CreateCacheElemAndDraw (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroker);

    void DeleteCacheElem (UInt32 id = 0xffffffff);
    void DeleteCacheElem ();
    void DeleteCacheForView (int view);

    private: DTMDataRefQvCache* GetTiledCacheElem (UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details);
    private: DTMDataRefQvCache* CreateTiledCacheElem (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroker);
    private: void DeleteDTMDataRefQvCache (DTMDataRefQvCache* qvCache, bool delayTileDelete = false);
    private: void DeleteDelayTiles();    
    };

struct DTMDisplayCacheManager
    {
    friend struct DTMDisplayCache;
private:
    static QvCacheP m_qvCache;
    static bmap <DTMDisplayCache*, DTMDisplayCache*> s_displayCaches;
public:
    static QvCacheP GetQvCache();

    static void Initialize();
    static void Uninitialize();

    DTMELEMENT_EXPORT static DTMDataRefQvCache* GetCacheElem (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details);
    DTMELEMENT_EXPORT static DTMDataRefQvCache* CreateCacheElemAndDraw (ElementHandleCR element, UInt32 id, ViewContextR context, DTMQvCacheType type, DSHandlerKeyStoragePtr filter, DTMQvCacheDetails* details, IDTMStrokeForCache& stroke);
    static void DeleteCacheElem (ElementHandleCR element);
    static void DeleteCacheElem (ElementHandleCR element, UInt32 id);
    static void DeleteCacheForView (int view);
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
