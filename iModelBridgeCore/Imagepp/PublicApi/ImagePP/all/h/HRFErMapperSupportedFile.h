//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFErMapperSupportedFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#if (_MSC_VER < 1600) // 1600 => VC 2010
    #if !defined(_DEBUG) // We do not support these format when using the debug version of the C run-time library
        #define IPP_HAVE_ERMAPPER_SUPPORT
    #endif
#endif


#include "HTIFFTag.h"

#include "HFCAccessMode.h"
#include "HFCBinStream.h"
#include "HFCMacros.h"
#include "HFCURL.h"
#include "HFCURLECWP.h"
#include "HFCURLECWPS.h"
#include "HCPGeoTiffKeys.h"
#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

#define BLOCK_WIDTH_ERMAPPER       256
#define BLOCK_HEIGHT_ERMAPPER      256

class HRFErMapperSupportedFileEditor;

class HRFErMapperSupportedFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(1959, HRFRasterFile);

    friend class HRFErMapperSupportedFileEditor;

    // allow to Open an image file
    HRFErMapperSupportedFile (const  HFCPtr<HFCURL>&                    pi_rpURL,
                              const HFCPtr<HRFRasterFileCapabilities>& pi_prCapabilities,
                              HFCAccessMode                            pi_AccessMode = HFC_READ_ONLY,
                              uint64_t                                pi_Offset = 0,
                              bool                                    pi_IsJpeg2000 = false);

    virtual     ~HRFErMapperSupportedFile();

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    void                                  Close();
    void                                  Open(WCharCP pi_pFileName, unsigned int pi_AsReadOnly);

    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page,
                                                                 bool   pi_CheckSpecificUnitSpec,
                                                                 bool   pi_GeoModelDefaultUnit,
                                                                 bool   pi_InterpretUnitINTGR);

    // LookAhead related methods
    bool                                 HasLookAheadByExtent(uint32_t pi_Page) const;

    virtual bool                         CanPerformLookAhead(uint32_t pi_Page) const;

    // Sets the LookAhead for a list of blocks
    virtual void                        SetLookAhead(uint32_t             pi_Page,
                                                     const HGFTileIDList& pi_rBlocks,
                                                     uint32_t             pi_ConsumerID,
                                                     bool                pi_Async);

    // Sets the LookAhead for a shape
    virtual void                        SetLookAhead(uint32_t        pi_Page,
                                                     unsigned short pi_Resolution,
                                                     const HVEShape& pi_rShape,
                                                     uint32_t        pi_ConsumerID,
                                                     bool           pi_Async);

    // Stops LookAhead for a consumer
    virtual void            StopLookAhead(uint32_t pi_Page,
                                                      uint32_t pi_ConsumerID);

    static void             InitErMapperLibrary();

protected:

    virtual bool            Open                ();
    virtual void            CreateDescriptors   ();

    double                  GetRatio(unsigned short pi_ResolutionNb);
    void*                   GetFileView();
    const void*             GetFileViewFileInfoEx();
    uint32_t*                 GetBandList();

    void                    BuildTransfoModelMatrix(IRasterBaseGcsPtr              pi_rpGeoTiffKeys,
                                                                unsigned short                 pi_ModelType,
                                                                HFCPtr<HGF2DTransfoModel>&     po_prTranfoModel);

//    void                                GetGeoTiffKeys(HFCPtr<HRFGeoTiffKeys>& po_rpGeoTiffKeys);
    IRasterBaseGcsPtr       GetGeocoding(HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys);

    double                  RoundRatio(unsigned long pi_MainImageSize, unsigned long pi_ResImageSize);

    // members
    void*                   m_pNCSFileView;            //NCSFileView
    void*                   m_pNCSFileViewFileInfoEx;  //NCSFileViewFileInfoEx
    uint32_t*                 m_pBandList;
    double*                 m_pRatio;
    bool                    m_IsJpeg2000;
    bool                    m_IsECWP;

private:

    typedef map<uint64_t, Byte*> TilePool;
    HFCExclusiveKey              m_TilePoolKey;
    TilePool                     m_TilePool;

    HFCPtr<HRFRasterFileCapabilities>    m_pCapabilities;
    bool                                Create();

    // Methods Disabled
    HRFErMapperSupportedFile(const HRFErMapperSupportedFile& pi_rObj);
    HRFErMapperSupportedFile&             operator= (const HRFErMapperSupportedFile& pi_rObj);

    typedef vector<unsigned short> BandList;
    static BandList s_SpecifiedBands;
    };

class HRFEcwFile : public HRFErMapperSupportedFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(1429, HRFErMapperSupportedFile);

    // allow to Open an image file
    HRFEcwFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode              pi_AccessMode = HFC_READ_ONLY,
                uint64_t                pi_Offset = 0);

    virtual     ~HRFEcwFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

private:

    // Methods Disabled
    HRFEcwFile(const HRFEcwFile& pi_rObj);
    HRFEcwFile&             operator= (const HRFEcwFile& pi_rObj);
    };


class HRFJpeg2000File : public HRFErMapperSupportedFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(1477, HRFErMapperSupportedFile);

    // allow to Open an image file
    HRFJpeg2000File (const  HFCPtr<HFCURL>&  pi_rpURL,
                     HFCAccessMode              pi_AccessMode = HFC_READ_ONLY,
                     uint64_t                pi_Offset = 0);

    virtual     ~HRFJpeg2000File();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

private:

    // Methods Disabled
    HRFJpeg2000File(const HRFJpeg2000File& pi_rObj);
    HRFJpeg2000File&            operator= (const HRFJpeg2000File& pi_rObj);
    };

class HRFEcwCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFEcwCapabilities();
    };


// Ecw Creator.
struct HRFEcwCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // Capability - The Generic SupportsURL function can be overwrite
    virtual bool                     SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFEcwCreator)

    // members
    WCharP          m_pLabel;
    WCharP          m_pExtensions;
    uint32_t*         m_pSupportedPixelTypeList;

    // Disabled methodes
    HRFEcwCreator();
    };

class HRFJpeg2000Capabilities : public HRFRasterFileCapabilities
    {
public:
    HRFJpeg2000Capabilities();
    };

struct HRFJpeg2000Creator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFJpeg2000Creator)

    // members
    WCharP      m_pLabel;
    WCharP      m_pExtensions;
    uint32_t*     m_pSupportedPixelTypeList;

    // Disabled methodes
    HRFJpeg2000Creator();
    };

