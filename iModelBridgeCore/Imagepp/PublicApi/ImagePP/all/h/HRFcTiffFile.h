//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFcTiffFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual const HFCPtr<HRFRasterFileCapabilities>&      GetCapabilities       () const;

    bool                                    AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    void                                    SetOriginalFileAccessMode (HFCAccessMode pi_AccessMode);

    // The format of the string must be idem at ctime() function
    void                                    SetSourceFile_CreationDateTime(const char* pi_DateTime);

    virtual void                            Save();

    virtual uint64_t                        GetFileCurrentSize() const;

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFcTiffFile(const HFCPtr<HFCURL>&  pi_rpURL,
                 HFCAccessMode          pi_AccessMode,
                 uint64_t              pi_Offset,
                 bool                  pi_DontOpenFile);

    virtual void            InitPrivateTagDefault(HRFiTiffFile::HMRHeader* po_pHMRHeader);
    virtual void            WritePrivateDirectory(uint32_t pi_Page);
    virtual void            CreateDescriptors ();
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

protected:
    HRFcTiffCreator();

    virtual bool ValidateHMRDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page,
                                       uint32_t pi_HMRVersion, Byte** pTransPalette,
                                       unsigned short*    po_pHMRPixelTypeSpec) const;
    virtual bool ValidatePageDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page, uint32_t pi_HMRVersion) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFcTiffCreator)


    };
END_IMAGEPP_NAMESPACE


