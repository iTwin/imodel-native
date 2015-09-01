//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGdalUtilities.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGdalSupportedRasterFileUtilities.h
//-----------------------------------------------------------------------------
// This class provides an interface to some GDAL hidden C method having not been
// declared in an header file.
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HPMAttributeSet;
class HCPGeoTiffKeys;

class HRFGdalUtilities
    {
public :

    static HCPGeoTiffKeys*      ConvertOGCWKTtoGeotiffKeys(char const* po_pOGCWKT);

    static bool                 ConvertGeotiffKeysToOGCWKT(
        const HFCPtr<HCPGeoTiffKeys>&        pi_rpGeoTiffKeys,
        WString&                             po_rOGCWKT);

    static bool                 ConvertERMToOGCWKT( WStringR  po_rOGCWKT,
                                                    WStringCR pi_rErmProjection, 
                                                    WStringCR pi_rErmDatum, 
                                                    WStringCR pi_rErmUnits );

private :
    HRFGdalUtilities();
    ~HRFGdalUtilities();

    // Methods Disabled
    HRFGdalUtilities(const HRFGdalUtilities& pi_rObj);
    HRFGdalUtilities& operator=(const HRFGdalUtilities& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
