/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDrawingInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>
#include <TerrainModel\ElementHandler\DTMDataRef.h>
#include "DTMUnitsConverter.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct DTMDrawingInfo : public DTMUnitsConverter
    {
private:
    RefCountedPtr<DTMDataRef> m_DTMDataRef;
    Bentley::DgnPlatform::ElementHandle m_originalEl;
    Bentley::DgnPlatform::ElementHandle m_symbologyEl;
    DMatrix4d m_rootToCurrLocalTrans;
    DMatrix4d m_currLocalToRootTrans;
    DTMFenceParams m_fence;
    bool m_isVisibile;
    bvector<DPoint3d> m_fencePts;

    bvector<bvector<DPoint3d>> m_majorContourCache;
    bvector<bvector<DPoint3d>> m_majorContourDepressionCache;
    bool m_hasMajorContourCache;
public:
    DTMDrawingInfo ()
        {
        m_hasMajorContourCache = false;
        m_rootToCurrLocalTrans.initIdentity();
        m_currLocalToRootTrans.initIdentity();
        m_isVisibile = true;
        }

    DTMDrawingInfo (RefCountedPtr<DTMDataRef> DTMDataRef, Transform trsf, DMatrix4dCR currLocalToRootTrans, ElementHandleCR originalEl, ElementHandleCR symbologyEl) : DTMUnitsConverter(trsf)
        {
        m_DTMDataRef = DTMDataRef;
        m_hasMajorContourCache = false;
        m_isVisibile = true;
        m_originalEl = originalEl;
        m_symbologyEl = symbologyEl;
        SetCurrLocalToRootTrans (currLocalToRootTrans);
        }

    void SetCurrLocalToRootTrans (DMatrix4dCR currLocalToRootTrans)
        {
        m_currLocalToRootTrans = currLocalToRootTrans;
        m_rootToCurrLocalTrans.qrInverseOf (&currLocalToRootTrans);
        }

    ElementHandleCR GetSymbologyElement () const
        {
        return m_symbologyEl;
        }

    ElementHandleCR GetOriginalElement () const
        {
        return m_originalEl;
        }

    void SetIsVisible (bool value)
        {
        m_isVisibile = value;
        }

    bool IsVisible() const
        {
        return m_isVisibile;
        }
    bvector<bvector<DPoint3d>>& GetMajorContourCache()
        {
        return m_majorContourCache;
        }

    bvector<bvector<DPoint3d>>& GetMajorContourDepressionCache()
        {
        return m_majorContourDepressionCache;
        }

    bool GetHasMajorContourCache()
        {
        return m_hasMajorContourCache;
        }
    void SetHasMajorContourCache()
        {
        m_hasMajorContourCache = true;
        }

    void SetFence (const DTMFenceParams& fence)
        {
        m_fence.numPoints = fence.numPoints;
        m_fence.fenceType = fence.fenceType;
        m_fence.fenceOption = fence.fenceOption;

        m_fencePts.resize (fence.numPoints);
        m_fence.points = m_fencePts.data ();
        if (fence.points)
            memcpy (m_fencePts.data(), fence.points, sizeof (DPoint3d) * fence.numPoints);
        }

    const DTMFenceParams& GetFence () const
        {
        return m_fence;
        }

    DTMDataRef* GetDTMDataRef()
        {
        return m_DTMDataRef.get();
        }
    DMatrix4dCR GetCurrLocalToRootTrans() const
        {
        return m_currLocalToRootTrans;
        }
    DMatrix4dCR GetRootToCurrLocalTrans() const
        {
        return m_rootToCurrLocalTrans;
        }

    void RootToStorage (DPoint3dR pt) const
        {
        GetRootToCurrLocalTrans().multiplyAndRenormalize (&pt, &pt, 1);
        GetUORToStorageTransformation()->multiply (&pt);
        }

    void StorageToRoot (DPoint3dR pt) const
        {
        GetStorageToUORTransformation()->multiply (&pt);
        GetCurrLocalToRootTrans().multiplyAndRenormalize (&pt, &pt, 1);
        }

    void RootToStorage (DPoint3dP p, size_t len) const
        {
        for (DPoint3dCP const end = p + len; p < end; ++p)
            { RootToStorage (*p); }
        }

    void StorageToRoot (DPoint3dP p, size_t len) const
        {
        for (DPoint3dCP const end = p + len; p < end; ++p)
            { StorageToRoot (*p); }
        }

    template <size_t _ARRAY_LENGTH_> void RootToStorage (DPoint3d (&arr)[_ARRAY_LENGTH_]) const
        {
        RootToStorage (arr, _countof(arr));
        }

    template <size_t _ARRAY_LENGTH_> void StorageToRoot (DPoint3d (&arr)[_ARRAY_LENGTH_]) const
        {
        StorageToRoot (arr, _countof(arr));
        }

    }; // End DTMDrawingInfo struct

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
