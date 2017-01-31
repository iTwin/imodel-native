/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/memory/embeddedarraycachemanager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef USE_CACHE
static VPList *pCacheList = NULL;
#endif
/**
* Invoke all registered cache flush functions
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedArrayManager_flushRegisteredCaches
(
)
    {
#ifdef USE_CACHE
    int i;
    PPtrCacheHeader pCache;
    for (i = 0;
           NULL != pCacheList
        && NULL != (pCache = (PPtrCacheHeader)vpList_get (pCacheList, i));
        i++
        )
        {
        omdlPtrCache_flushCache (pCache);
        }
#endif
    }

/**
* Register a cache for synchronized management.
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedArrayManager_registerCache
(
PPtrCacheHeader pCache
)
    {
#ifdef USE_CACHE
    if (!pCacheList)
        pCacheList = vpList_new ();
    vpList_add (pCacheList, pCache);
#endif
    }
END_BENTLEY_GEOMETRY_NAMESPACE
