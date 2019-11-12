//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFcTiffFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

#include "HRFiTiffFile.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFcTiffCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFcTiffCapabilities();
    };



class HRFcTiffFile : public HRFiTiffFile
    {
public:
    friend class HRFiTiffTileEditor;
    friend class HRFiTiffLuraWavePaddedTileEditor;
    friend class HRFiTiffLuraWaveNonPaddedTileEditor;
    friend class HRFiTiffStripEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_cTiff, HRFiTiffFile)

    // allow to Open an image file
    HRFcTiffFile          (const HFCPtr<HFCURL>&          pi_rpURL,
						   HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
						   uint64_t                       pi_Offset = 0);

    virtual                                 ~HRFcTiffFile         ();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&      GetCapabilities       () const override;

    bool                                    AddPage(HFCPtr<HRFPageDescriptor> pi_pPage) override;

    void                                    SetOriginalFileAccessMode (HFCAccessMode pi_AccessMode);

    // The format of the string must be idem at ctime() function
    void                                    SetSourceFile_CreationDateTime(const char* pi_DateTime);

    void                            Save() override;

    uint64_t                        GetFileCurrentSize() const override;

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFcTiffFile(const HFCPtr<HFCURL>&  pi_rpURL,
                 HFCAccessMode          pi_AccessMode,
                 uint64_t              pi_Offset,
                 bool                  pi_DontOpenFile);

    void            InitPrivateTagDefault(HRFiTiffFile::HMRHeader* po_pHMRHeader) override;
    void            WritePrivateDirectory(uint32_t pi_Page) override;
    void            CreateDescriptors () override;
//        virtual void          ReloadDescriptors();
//        virtual void          SaveDescriptors();

    // Filters method.
    HRPFilter*              GetFilter();
    string                  ReadString(istringstream& pi_rStream) const;
    void                    SetFilter(const HRPFilter& pi_rFilter);
    void                    StreamFilter(ostringstream& po_rStream, const HRPFilter& pi_rFilter) const;
    HRPFilter*              UnstreamFilter(istringstream& pi_rStream) const;

private:
    friend struct HRFcTiffCreator;

    // Attributes
    HFCAccessMode           m_OriginalFileAccessMode;

    string                  m_SourceFileCreationTime;
    bool                   m_SourceFileCreationTimeChanged;
    // Methods

    void                    SavecTiffFile();
    // Methods Disabled
    HRFcTiffFile(const HRFcTiffFile& pi_rObj);
    HRFcTiffFile& operator=(const HRFcTiffFile& pi_rObj);
    };

// Cache Creator.
struct HRFcTiffCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "cTIFF"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;

protected:
    HRFcTiffCreator();

    virtual bool ValidateHMRDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page,
                                       uint32_t pi_HMRVersion, Byte** pTransPalette,
                                       uint16_t*    po_pHMRPixelTypeSpec) const;
    virtual bool ValidatePageDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page, uint32_t pi_HMRVersion) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFcTiffCreator)


    };
END_IMAGEPP_NAMESPACE


