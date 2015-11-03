//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPRJPageFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFHGRPageFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFPageFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HGF2DWorld.h"


BEGIN_IMAGEPP_NAMESPACE
class HFCRUL;
class HFCBinStream;

//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------
//Blocks

class HRFPRJCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFPRJCapabilities();
    };

class HRFPRJPageFile : public HRFPageFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_PRJPage, HRFPageFile)

    // allow to Open an image file
    IMAGEPP_EXPORT                     HRFPRJPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                                              HFCAccessMode           pi_AccessMode = HFC_READ_ONLY);

    IMAGEPP_EXPORT virtual             ~HRFPRJPageFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&    GetCapabilities       () const;

    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    virtual void                          WriteToDisk();
protected:
    // capabilities
    HFCPtr<HRFRasterFileCapabilities>     m_pPageCapabilities;

private:



    // Members.
    HAutoPtr<HFCBinStream>  m_pFile;

    WString    m_WKT;

    void                    ReadFile();

    void                    CreateDescriptor();

    // Methods Disabled
    HRFPRJPageFile(const HRFPRJPageFile& pi_rObj);
    HRFPRJPageFile&  operator=(const HRFPRJPageFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------
class HRFPRJPageFileCreator : public HRFPageFileCreator
    {
public:

    // Creation of this specific instance
    virtual bool               HasFor          (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const;
    virtual HFCPtr<HRFPageFile> CreateFor       (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const;
    virtual HFCPtr<HFCURL>      ComposeURLFor   (const HFCPtr<HFCURL>&   pi_rpURLFileName) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&    GetCapabilities();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPRJPageFileCreator)
    };
END_IMAGEPP_NAMESPACE

