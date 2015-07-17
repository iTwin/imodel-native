/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/StdAfx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
    
#include <GeoCoord\BaseGeoCoord.h>
#include <DgnPlatform\DgnPlatformAPI.h>
#include <DgnView\IViewManager.h>
#include <DgnView\IRedraw.h>
#include <DgnView\AccuSnap.h>
#include <DgnPlatform\DgnECProviderBase.h>
#include <DgnPlatform\ElementProperties.h>

#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>
#include <DgnPlatform\TerrainModel\TMSymbologyOverrideManager.h>
#include <TerrainModel\ElementHandler\TMHandlersResources.h>
#include <TerrainModel\ElementHandler\TMHandlersResources.r.h>
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <TerrainModel\Core\bcDTMElement.h>


//Number of power platform views
#define NUMBER_OF_PP_VIEWS 8

#if defined (NDEBUG)
#   define TRACE __noop
#elif defined (DEBUG)
#   pragma warning ( push )
#   pragma warning ( disable : 4793 )
    inline void TRACE ( char const formatBuf[], ... )
        {
        va_list     va;
        int         size;
        char        *buf;

        va_start ( va, formatBuf );
        size = _vscprintf ( formatBuf, va );
        va_end ( va );
        BeAssert ( size > 0 );
        size += 10;
        buf = reinterpret_cast<char*>( _malloca ( size * sizeof ( char ) ) );
        BeAssert ( buf );
        va_start ( va, formatBuf );
        vsprintf_s ( buf, size, formatBuf, va );
        va_end ( va );
        strcat ( buf, "\n" );
        OutputDebugStringA ( buf );
        _freea (buf);
        }

    inline void TRACE ( wchar_t const formatBuf[], ... )
        {
        va_list     va;
        int         size;
        wchar_t     *buf;

        va_start ( va, formatBuf );
        size = _vscwprintf ( formatBuf, va );
        va_end ( va );
        BeAssert ( size > 0 );
        size += 10;
        buf = reinterpret_cast<wchar_t*>( _malloca ( size * sizeof ( wchar_t ) ) );
        BeAssert ( buf );
        va_start ( va, formatBuf );
        vswprintf_s ( buf, size, formatBuf, va );
        va_end ( va );
        wcscat ( buf, L"\n" );
        OutputDebugStringW ( buf );
        _freea (buf);
        }
#   pragma warning ( pop )
#else
#   define TRACE __noop
#endif

#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>   
#include <DgnPlatform\TerrainModel\TMReferenceXAttributeHandler.h>

#include "DTMDrawingInfo.h"
#include "DTMDataRefCachingManager.h"
#include "DTMXAttributeHandler.h"
#include "DTMDataRefXAttribute.h"
#include "DTMDisplayUtils.h"
#include "DTMPointDrawer.h"
#include "DTMDisplayHandlers.h"
#include "DTMBinaryData.h"
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_MSTNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL
using namespace std;
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT