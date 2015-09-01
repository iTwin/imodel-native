//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFWcsFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"
#include "HRFOGCService.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include <BeXml/BeXml.h>

BEGIN_IMAGEPP_NAMESPACE
class HFCURL;


class HRFWCSCapabilities : public HRFOGCServiceCapabilities
    {
public:
    HRFWCSCapabilities();
    };

class HRFWCSFile : public HRFOGCService
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_WCS, HRFOGCService)

    HRFWCSFile  (const HFCPtr<HFCURL>&      pi_rpURL,
                 HFCAccessMode              pi_AccessMode = HFC_READ_ONLY,
                 uint64_t                  pi_Offset = 0);
    virtual ~HRFWCSFile ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

protected:

// Methods

    virtual void                CreateDescriptors       (uint64_t                  pi_Width,
                                                         uint64_t                  pi_Height);

private:

    bool                       m_NeedAuthentification;
    string                      m_CSR;

    void                        ReadWCS_1_0(BentleyApi::BeXmlNodeP pi_pNode);



    // Methods Disabled
    HRFWCSFile(const HRFWCSFile& pi_rObj);
    HRFWCSFile&                 operator= (const HRFWCSFile& pi_rObj);
    };


// WCS Creator.
struct HRFWCSCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFWCSCreator)



    // Disabled methodes
    HRFWCSCreator();
    };
END_IMAGEPP_NAMESPACE
