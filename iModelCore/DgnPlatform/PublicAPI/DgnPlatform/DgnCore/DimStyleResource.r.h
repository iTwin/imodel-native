/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DimStyleResource.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

// ------------------------------------------------------------
//  This file is included by both .h/cpp and .r files
// ------------------------------------------------------------

#include <RmgrTools/Tools/rtypes.r.h>
#include <DgnPlatform/DgnHandlers/DimensionStyleProps.r.h>

#define RTYPE_DimStylePropToOverrideMap     RTYPE ('d', 'S', 'P', 'O')
#define RSCID_DIMSTYLE_PROPTOOVERRIDES      0

BEGIN_BENTLEY_DGN_NAMESPACE

typedef struct PropToOverrideMap
    {
#ifdef resource
    int32_t         override;
    int32_t         property;
#else
    DimStyleProp    m_override;
    DimStyleProp    m_property;
#endif
    int32_t         inverted;
    int32_t         notInDimElm;
    } PropToOverrideMap;

typedef struct DimStylePropToOverrideMapRsc
    {
#ifdef resource
    PropToOverrideMap   map[];
#else
    unsigned long       nMaps;
    PropToOverrideMap   map[1];
#endif
    } DimStylePropToOverrideMapRsc;

#ifdef resource
resourceclass DimStylePropToOverrideMapRsc      RTYPE_DimStylePropToOverrideMap;
#endif

END_BENTLEY_DGN_NAMESPACE
