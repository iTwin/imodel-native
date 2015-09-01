//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFJpegFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HFCBinStream.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

struct jpeg_compress_struct;
struct jpeg_decompress_struct;
struct jpeg_common_struct;
struct jpeg_error_mgr;
typedef struct jpeg_common_struct* j_common_ptr;
struct HRFJpegFileErrorManager;

BEGIN_IMAGEPP_NAMESPACE
class HRFJpegCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFJpegCapabilities();

    };

static void HRFJpegErrorExit(j_common_ptr cinfo);

class HRFJpegFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Jpeg, HRFRasterFile)

    friend class HRFJpegLineEditor;

#if defined (ANDROID) || defined (__APPLE__)
    friend void HRFJpegErrorExit(j_common_ptr cinfo); 
#elif defined (_WIN32)
    friend static void HRFJpegErrorExit(j_common_ptr cinfo); 
#endif



    // allow to Open an image file
    HRFJpegFile           (const HFCPtr<HFCURL>&          pi_rpURL,
                           HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                           uint64_t                      pi_Offset = 0);

    virtual                               ~HRFJpegFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    // Exceptions...
    bool                                 IsDestroying() const {
        return m_Destructor;
        };

    virtual uint64_t                        GetFileCurrentSize() const;

    static void                           ThrowExBasedOnJPGErrCode(uint32_t pi_GetLastErrorCode,
                                                                   const WString& pi_rUrl);

    void                                  GetExifTags(bool            pi_ExifTags,
                                                      bool            pi_ExifRelatedGPSTags,
                                                      HPMAttributeSet& po_rTags);

protected:
    // JPEG struct to join the compress and decompress
    // structures into a single one.  This is done to
    // be able to return one structure in the GetFilePtr()
    // inline method.
    typedef struct
        {
        // See the IJG JPEG library for a decription
        // of both these structures.
        jpeg_decompress_struct* m_pDecompress;
        jpeg_compress_struct*   m_pCompress;
        } JPEG;

    // same comment as the CountPage method
    JPEG*           GetFilePtr();

    // Methods
    // Constructor use only to create a child
    //
    HRFJpegFile         (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                       CreateDescriptors   ();
    virtual bool                       AssignPageToStruct  ();
    // Duplicate this method for binary compatibility.
    bool                               AssignPageToStruct2 (jpeg_compress_struct* pi_pTable);        

private:
    // member that indicates if we're in the destructor
    bool m_Destructor;

    // Members
    HAutoPtr<HRFJpegFileErrorManager>
    m_pErrorManager;
    JPEG                        m_Jpeg;
    HAutoPtr<HFCBinStream>      m_pJpegFile;
    bool                       m_IfCompressionNotTerminated;

    // Create the file
    void                    SaveJpegFile(bool pi_CloseFile);
    bool                   Create();

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile() const;

    // Methods Disabled
    HRFJpegFile(const HRFJpegFile& pi_rObj);
    HRFJpegFile& operator=(const HRFJpegFile& pi_rObj);
    };

// Jpeg Creator.
struct HRFJpegCreator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFJpegCreator)

    // Disabled methodes
    HRFJpegCreator();
    };
END_IMAGEPP_NAMESPACE

