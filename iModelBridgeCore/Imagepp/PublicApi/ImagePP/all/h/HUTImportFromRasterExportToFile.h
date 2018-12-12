//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTImportFromRasterExportToFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTImportFromRasterExportToFile
//-----------------------------------------------------------------------------
// This class describes the basic interface of a raster file format
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileFactory.h"
#include "HRFImportExport.h"
#include "HRASamplingOptions.h"

#include "HGF2DWorldCluster.h"
#include "HRFRasterFile.h"
#include "HPMPool.h"
#include "HRARaster.h"
#include "HVEShape.h"
#include "HGSTypes.h"

BEGIN_IMAGEPP_NAMESPACE
class HUTImportFromRasterExportToFile : public HRFImportExport
    {
public:
    // Creation and destruction interface
    IMAGEPP_EXPORT HUTImportFromRasterExportToFile (HFCPtr<HRARaster>                  pi_pRaster,
                                            const HVEShape&                    pi_rClipShape,
                                            const HFCPtr<HGF2DWorldCluster>&   pi_pWorldCluster);

    IMAGEPP_EXPORT virtual ~HUTImportFromRasterExportToFile();

    // Export interface
    IMAGEPP_EXPORT virtual HFCPtr<HRFRasterFile>    StartExport();

    // BestMatch interface
    virtual void                            BestMatchSelectedValues();

    IMAGEPP_EXPORT void                             SelectBestPixelType(const HFCPtr<HRPPixelType>& pPixelType);

    // Set number of color, if dest to palette
    //  0 --> MaxEntries
    IMAGEPP_EXPORT void                             SetNumberOfColorDestination(uint32_t pi_NbColors);
    uint32_t                                GetNumberOfColorDestination() const;

    IMAGEPP_EXPORT void                                    SetBlendAlpha(bool pi_BlendAlpha);
    bool                                   GetBlendAlpha() const;

    const HGSResampling&                    GetResamplingMode() const;
    IMAGEPP_EXPORT void                                    SetResamplingMode(const HGSResampling& pi_rResampling);

    IMAGEPP_EXPORT HRASamplingOptions                      GetRepresentativePaletteSamplingOptions() const;
    IMAGEPP_EXPORT void                                    SetRepresentativePaletteSamplingOptions(const HRASamplingOptions& pi_rRepPalSamplingOptions);

protected:
    HFCPtr<HRARaster>   m_pRaster;
    uint32_t            m_NbColorsIfIndexed;
    bool               m_BlendAlpha;
    HGSResampling       m_Resampling;
    HRASamplingOptions  m_RepPalSamplingOptions;
    bool               m_HasRepPalSamplingOptions;

private:

    // Disabled methods
    HUTImportFromRasterExportToFile(const HUTImportFromRasterExportToFile&);
    HUTImportFromRasterExportToFile& operator=(const HUTImportFromRasterExportToFile&);
    };
END_IMAGEPP_NAMESPACE
