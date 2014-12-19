//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTImportFromRasterExportToFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HUTImportFromRasterExportToFile : public HRFImportExport
    {
public:
    // Creation and destruction interface
    _HDLLg HUTImportFromRasterExportToFile (HFCPtr<HRARaster>                  pi_pRaster,
                                            const HVEShape&                    pi_rClipShape,
                                            const HFCPtr<HGF2DWorldCluster>&   pi_pWorldCluster);

    _HDLLg virtual ~HUTImportFromRasterExportToFile();

    // Export interface
    _HDLLg virtual HFCPtr<HRFRasterFile>    StartExport();

    // BestMatch interface
    virtual void                            BestMatchSelectedValues();

    _HDLLg void                             SelectBestPixelType(const HFCPtr<HRPPixelType>& pPixelType);

    // Set number of color, if dest to palette
    //  0 --> MaxEntries
    _HDLLg void                             SetNumberOfColorDestination(uint32_t pi_NbColors);
    uint32_t                                GetNumberOfColorDestination() const;

    _HDLLg void                                    SetBlendAlpha(bool pi_BlendAlpha);
    bool                                   GetBlendAlpha() const;

    const HGSResampling&                    GetResamplingMode() const;
    _HDLLg void                                    SetResamplingMode(const HGSResampling& pi_rResampling);

    _HDLLg HRASamplingOptions                      GetRepresentativePaletteSamplingOptions() const;
    _HDLLg void                                    SetRepresentativePaletteSamplingOptions(const HRASamplingOptions& pi_rRepPalSamplingOptions);

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
