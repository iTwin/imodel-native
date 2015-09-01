//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPWRasterFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#if defined(_WIN32)
#define IPP_HAVE_PROJECTWISE_SUPPORT
#endif

#if defined(IPP_HAVE_PROJECTWISE_SUPPORT) 

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFcTiffFile.h"

BEGIN_IMAGEPP_NAMESPACE
class IHRFPWFileHandler;

//-----------------------------------------------------------------------------
// class HRFPWCapabilities
//-----------------------------------------------------------------------------
class HRFPWCapabilities : public HRFcTiffCapabilities
    {
public:
    HRFPWCapabilities();
    };

//-----------------------------------------------------------------------------
// class HRFPWRasterFile
//-----------------------------------------------------------------------------
class HRFPWHandler;
struct HRFPWCreator;
class HRFPWRasterFile : public HRFcTiffFile
    {
public:
    friend class HRFPWEditor;
    friend struct HRFPWCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_PWRaster, HRFcTiffFile)

    IMAGEPP_EXPORT    HRFPWRasterFile    (const HFCPtr<HFCURL>&          pi_rpURL,
                                  HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                                  uint64_t                      pi_Offset = 0);

    virtual     ~HRFPWRasterFile();



    IMAGEPP_EXPORT static HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>&   pi_rpURL,
                                                                GUID                    pi_DocumentID,
                                                                const HFCPtr<HFCURL>&   pi_rpPWUrl);

    IMAGEPP_EXPORT static HFCPtr<HRFRasterFile> Create(const HFCPtr<HRFRasterFile>&    pi_rpRasterFile,
                                                                GUID                            pi_DocumentID,
                                                                const HFCPtr<HFCURL>&           pi_rpPWUrl);

    const HGF2DWorldIdentificator         GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    IMAGEPP_EXPORT HCLASS_ID           GetOriginalClassID() const;

    bool               ReadProjectWiseBlob(uint32_t pi_Page, Byte* po_pData, uint32_t* po_pSize) const;
    bool               WriteProjectWiseBlob(uint32_t pi_Page, const Byte* pi_pData, uint32_t pi_Size);



protected:

    // Methods
    // Constructor use only to create a child
    //

    virtual void            InitPrivateTagDefault(HRFiTiffFile::HMRHeader* po_pHMRHeader);

private:

    typedef struct
        {
        char       FileIdentification[31];
        __time32_t  DocumentTimestamps;
        } Header;

    typedef struct
        {
        HGF2DWorldIdentificator     WorldID;
        HCLASS_ID                   ClassID;
        GUID                        DocumentID;
        __time32_t                  Timestamp;
        } OriginalFileInfo;

    // Members
    OriginalFileInfo                    m_OriginalFileInfo;
    HFCPtr<HRFRasterFileCapabilities>   m_pCapabilities;

    IHRFPWFileHandler*                  m_pPWHandler;


    // Methods Disabled
    HRFPWRasterFile(const HRFPWRasterFile& pi_rObj);
    HRFPWRasterFile&             operator= (const HRFPWRasterFile& pi_rObj);
    };



struct HRFPWCreator : public HRFcTiffCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;


    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;


    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPWCreator)


    // Disabled methodes
    HRFPWCreator();
    };
END_IMAGEPP_NAMESPACE

#endif  // IPP_HAVE_PROJECTWISE_SUPPORT