//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTImportFromFileExportToFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTImportFromFileExportToFile
//-----------------------------------------------------------------------------
// This class describes the basic interface of a raster file format
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileFactory.h"
#include "HRFImportExport.h"
#include "HGF2DWorldCluster.h"
#include "HGSTypes.h"


BEGIN_IMAGEPP_NAMESPACE
class HUTImportFromFileExportToFile : public HRFImportExport
    {
public:
    // Creation and destruction interface
    IMAGEPP_EXPORT HUTImportFromFileExportToFile (const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster,
                                          bool                            pi_BestMatchTag = true);
    IMAGEPP_EXPORT virtual ~HUTImportFromFileExportToFile();

    // Import File interface
    virtual void                            SelectImportFilename(const HFCPtr<HFCURL>& pi_rpURLPath);
    virtual const HFCPtr<HFCURL>&           GetSelectedImportFilename() const;
    virtual void                            SetImportRasterFile(const HFCPtr<HRFRasterFile>& pi_rpRasterFile);
    virtual const HFCPtr<HRFRasterFile>&    GetImportRasterFile() const;


    // Export interface
    IMAGEPP_EXPORT virtual HFCPtr<HRFRasterFile>    StartExport();


    // BestMatch interface
    virtual void                            BestMatchSelectedValues();


    // Set number of color, if dest to palette
    //  0 --> MaxEntries
    IMAGEPP_EXPORT void                             SetNumberOfColorDestination(uint32_t pi_NbColors);
    IMAGEPP_EXPORT uint32_t                         GetNumberOfColorDestination() const;

    IMAGEPP_EXPORT const HGSResampling&             GetResamplingMode() const;
    IMAGEPP_EXPORT void                             SetResamplingMode(const HGSResampling& pi_rResampling);


protected:
    void                    InitImportRasterFile();

    HFCPtr<HFCURL>          m_pSelectedImportFilename;
    HFCPtr<HRFRasterFile>   m_pSelectedImportFile;


private:

    // members
    uint32_t                m_NbColorsIfIndexed;
    HGSResampling           m_Resampling;
    bool                   m_BestMatchTag;

    // Disabled methods
    HUTImportFromFileExportToFile(const HUTImportFromFileExportToFile&);
    HUTImportFromFileExportToFile& operator=(const HUTImportFromFileExportToFile&);
    };
END_IMAGEPP_NAMESPACE
