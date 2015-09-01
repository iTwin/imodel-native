//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTWFPageFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFTWFPageFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFPageFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HGF2DWorld.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCBinStream;
class HFCURL;

class HRFTWFCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFTWFCapabilities();
    };


class HRFTWFPageFile : public HRFPageFile
    {
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_TWFPage, HRFPageFile)

public:

    enum WFParameter
        {
        PIXEL_SIZE_X = 1,
        ROT_ABOUT_Y,
        ROT_ABOUT_X,
        PIXEL_SIZE_Y,
        TRANSLATION_X,
        TRANSLATION_Y
        };

    // Constructor and destructor.
    IMAGEPP_EXPORT                                            HRFTWFPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                                                                     HFCAccessMode           pi_AccessMode = HFC_READ_ONLY);
    IMAGEPP_EXPORT virtual                                    ~HRFTWFPageFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const;

    virtual const HGF2DWorldIdentificator   GetWorldIdentificator () const;

    IMAGEPP_EXPORT virtual void                     WriteToDisk();

private:
    // Members.
    HAutoPtr<HFCBinStream>  m_pFile;
    double                 m_A00;
    double                 m_A01;
    double                 m_A10;
    double                 m_A11;
    double                 m_Tx;
    double                 m_Ty;

    // Private methods.
    void                        CreateDescriptor();
    HFCPtr<HGF2DTransfoModel>   BuildTransfoModel() const;
    bool                       IsValidTWFFile() const;
    void                        ReadFile();
    void                        ReadLine(string*   po_pString);

    void                        WriteFile();

    void                        CleanUpString(string* pio_pString) const;
    bool                       IsValidChar(const char pi_Char) const;
    bool                       ConvertStringToDouble(string&   pio_rString,
                                                      double*  po_pDouble) const;

    // Static members


    // Disabled methods.
    HRFTWFPageFile(const HRFTWFPageFile& pi_rObj);
    HRFTWFPageFile&             operator=(const HRFTWFPageFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------
class HRFTWFPageFileCreator : public HRFPageFileCreator
    {
public:

    // Constructor
    HRFTWFPageFileCreator();

    // Creation of this specific instance
    virtual bool               HasFor       (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const;
    virtual HFCPtr<HRFPageFile> CreateFor    (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const;
    virtual HFCPtr<HFCURL>      ComposeURLFor(const HFCPtr<HFCURL>&   pi_rpURLFileName) const;
    virtual HFCPtr<HFCURL>      FoundFileFor (const HFCPtr<HFCURL>& pi_rpURL) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTWFPageFileCreator)
    };
END_IMAGEPP_NAMESPACE
