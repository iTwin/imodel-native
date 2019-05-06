//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGdalSupportedRasterFileUtilities.h
//-----------------------------------------------------------------------------
// This class provides an interface to some GDAL hidden C method having not been
// declared in an header file.
#pragma once

#ifdef IPP_HAVE_GDAL_SUPPORT
BEGIN_IMAGEPP_NAMESPACE
class HPMAttributeSet;
class HCPGeoTiffKeys;

class HRFGdalUtilities
    {
public :

    static HCPGeoTiffKeys*      ConvertOGCWKTtoGeotiffKeys(CharCP pOGCWKT);

    static bool                 ConvertGeotiffKeysToOGCWKT(AStringR OGCWKT, HCPGeoTiffKeys const& geoTiffKeys);

    static bool                 ConvertERMToOGCWKT(AStringR OGCWKT, CharCP pErmProjection, CharCP pErmDatum, CharCP pErmUnits);

private :
    HRFGdalUtilities();
    ~HRFGdalUtilities();

    // Methods Disabled
    HRFGdalUtilities(const HRFGdalUtilities& pi_rObj);
    HRFGdalUtilities& operator=(const HRFGdalUtilities& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
#endif
