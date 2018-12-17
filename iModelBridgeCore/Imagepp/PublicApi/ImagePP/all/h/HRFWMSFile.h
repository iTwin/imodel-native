//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFWMSFile.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFOGCService.h"
#include <BeXml/BeXml.h>

BEGIN_IMAGEPP_NAMESPACE
class HFCURL;
class HMDLayersWMS;
class HMDVolatileLayers;

class HRFWMSCapabilities : public HRFOGCServiceCapabilities
    {
public:
    HRFWMSCapabilities();
    };

class HRFWMSFile : public HRFOGCService
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_WMS, HRFOGCService)

    HRFWMSFile              (const HFCPtr<HFCURL>&      pi_rpURL,
                             HFCAccessMode              pi_AccessMode = HFC_READ_ONLY,
                             uint64_t                  pi_Offset = 0);
    virtual     ~HRFWMSFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const override;


protected:

    void    CreateDescriptors   (uint64_t                  pi_Width,
                                         uint64_t                  pi_Height) override;

    void    SetContext          (uint32_t                 pi_Page,
                                         const HFCPtr<HMDContext>& pi_rContext) override;

    // This overload is not usually required but we have found servers (see TTR#314855) that actually complain about
    // the authentication required probe dummy request that the request is invalid before complaining about
    // the required authentication. We overload the present protected method to compose
    // a fully valid request based on the WMS configuration so that we can make sure the authentication
    // is really noty required.
    virtual Utf8String _GetValidateConnectionRequest() const override;

private:

    HFCPtr<HMDLayersWMS>        m_pLayers;

    bool                       m_NeedAuthentification;
    Utf8String                 m_Layers;
    Utf8String                 m_Styles;
    Utf8String                 m_CRS;

    void ReadWMS_1_0(BeXmlNodeP pi_pNode);
    void ReadWMS_1_1(BeXmlNodeP pi_pNode);
    void ReadWMS_1_2(BeXmlNodeP pi_pNode, Utf8String const& version);

    // Methods Disabled
    HRFWMSFile(const HRFWMSFile& pi_rObj);
    HRFWMSFile&             operator= (const HRFWMSFile& pi_rObj);
    };


// WMS Creator.
struct HRFWMSCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "WMS"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFWMSCreator)

    // Disabled methodes
    HRFWMSCreator();
    };
END_IMAGEPP_NAMESPACE
