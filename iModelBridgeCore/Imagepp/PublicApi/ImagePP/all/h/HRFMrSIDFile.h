//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMrSIDFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#if defined(BENTLEY_WIN32)
    #if !defined(_DEBUG)    // We do not support these format when using the debug version of the C run-time library
        #define IPP_HAVE_MRSID_SUPPORT
    #endif
#endif


#if defined(IPP_HAVE_MRSID_SUPPORT) 

#include "HTIFFTag.h"

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HCPGeoTiffKeys.h"

#define BLOCK_WIDTH  256
#define BLOCK_HEIGHT 256

#define STRIP_HEIGHT 256


namespace LizardTech
{
class LTIPixel;
class LTISceneBuffer;
class LTIImageStage;
class LTFileSpec;
class MrSIDImageReader;
}



BEGIN_IMAGEPP_NAMESPACE
class HRFMrSIDCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFMrSIDCapabilities();
    };


class HRFMrSIDFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_MrSID, HRFRasterFile)

    friend class HRFMrSIDEditor;

    // allow to Open an image file
    HRFMrSIDFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                  HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                  uint64_t        pi_Offset = 0);

    virtual     ~HRFMrSIDFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    // TR 246254 - Patch for MrSID for backward compatibility purpose.
    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

    virtual void                          Save();

    void Close();

    bool                               HasLookAheadByExtent    (uint32_t                   pi_Page) const;

    virtual bool                       CanPerformLookAhead     (uint32_t                   pi_Page) const;

    // Sets the LookAhead for a list of blocks
    virtual void                        SetLookAhead            (uint32_t                   pi_Page,
                                                                 const HGFTileIDList&       pi_rBlocks,
                                                                 uint32_t                   pi_ConsumerID,
                                                                 bool                      pi_Async);

    // Sets the LookAhead for a shape
    virtual void                        SetLookAhead            (uint32_t                   pi_Page,
                                                                 uint16_t            pi_Resolution,
                                                                 const HVEShape&            pi_rShape,
                                                                 uint32_t                   pi_ConsumerID,
                                                                 bool                      pi_Async);

    // Stops LookAhead for a consumer
    virtual void                        StopLookAhead           (uint32_t                   pi_Page,
                                                                 uint32_t                   pi_ConsumerID);


protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFMrSIDFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode,
                           uint64_t              pi_Offset,
                           bool                  pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();

    void                                GetFileInfo(HPMAttributeSet&               po_rTagList,
                                                    GeoCoordinates::BaseGCSPtr&    po_fileGeocoding,
                                                    bool&                          po_UnitsInFile);

    void                                BuildTransfoModelMatrix(bool pi_HasModelType, HFCPtr<HGF2DTransfoModel>& po_prTranfoModel);

    size_t                       m_ResCount;
    uint32_t*                    m_pStdViewWidth;
    uint32_t*                    m_pStdViewHeight;
    double*                      m_pRatio;

    LizardTech::LTISceneBuffer* m_pSceneBuffer;

private:

    bool                            CreateFilter_CreatePixelType(LizardTech::LTIImageStage* pi_pImage, LizardTech::LTIImageStage** po_ImageFilter=0, 
                                                                 HRPPixelType** po_PixelType=0);


    // members
    LizardTech::LTFileSpec*         m_pFileSpec;
    LizardTech::LTIImageStage*      m_pImageReader;
    LizardTech::MrSIDImageReader*   m_pImageRawReader;

    typedef map<uint64_t, Byte*> TilePool;
    HFCExclusiveKey             m_TilePoolKey;
    TilePool                    m_TilePool;

    // Methods Disabled
    HRFMrSIDFile(const HRFMrSIDFile& pi_rObj);
    HRFMrSIDFile&             operator= (const HRFMrSIDFile& pi_rObj);
    };


// MrSID Creator.
struct HRFMrSIDCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual Utf8String                   GetLabel() const;
    virtual Utf8String                   GetSchemes() const;
    virtual Utf8String                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFMrSIDCreator)

    // Disabled methodes
    HRFMrSIDCreator();
    };

END_IMAGEPP_NAMESPACE
#endif      // IPP_HAVE_MRSID_SUPPORT
