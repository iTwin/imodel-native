//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFLRDFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HCDCodecLRDRLE.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRPPixelType;
class  HFCBinStream;
class HGF2DTransfoModel;

#define HRF_LRD_BLOCK_SIZE   512

class HRFLRDCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFLRDCapabilities();

    };

// Disable the automatic word alingnement for intergraph structure.
#pragma pack( push, IntergraphIdent,  1)

class HRFLRDFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_LRD, HRFRasterFile)

    friend class HRFLRDLineEditor;

    // allow to Open an image file
    HRFLRDFile (const HFCPtr<HFCURL>& pi_rpURL,
                HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                uint64_t             pi_Offset = 0);

    virtual        ~HRFLRDFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;
    const   HRFScanlineOrientation
    GetScanlineOrientation () const;

    const HFCPtr<HCDCodecLRDRLE>&        GetLRDCodecPtr() const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

protected:
    // Constructor use only to create a child
    HRFLRDFile (const HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode          pi_AccessMode,
                uint64_t              pi_Offset,
                bool                  pi_DontOpenFile);
    virtual bool                         Open ();
    virtual void                          CreateDescriptors ();
    const   HFCBinStream*                 GetLRDFilePtr () const;

private:

    struct LRDHeaderBlock
        {
        char MagicWord[4];
        int32_t VecLevel;
        int32_t Recordcount;
        char Date[10];
        char Time[10];
        char rFile[40];
        int32_t bStart;
        int32_t bStop;
        int32_t BitsCount;
        int32_t rStart;
        int32_t rStop;
        int32_t maxmis;
        int32_t minwid;
        int32_t eps;
        int32_t mintr0;
        int32_t maxtr0;
        int32_t fuzz;
        int32_t widfly;
        int32_t widhol;
        int32_t talhol;
        int32_t newtop;
        int32_t botmin;
        int32_t hmod;
        int32_t corsys;
        int32_t xpdens;
        int32_t ypdens;
        int32_t vsmin;
        int32_t vsmax;
        int32_t vxmin;
        int32_t vxmax;                     // 168 bytes from the begining.
        Byte TheExpense[208];  // Unknow space... 512 - (168 + 136)
        int32_t valid;
        int32_t slo;
        double Matrix[16];              // 128 bytes used.
        };

    bool                   m_HasHeaderFilled;
    uint16_t         m_BitPerPixel;
    uint32_t                m_Width;
    uint32_t                m_Height;

    bool                   m_IsBigEndian;

    HRFScanlineOrientation  m_ScanlineOrientation;
    HAutoPtr<HFCBinStream>  m_pLRDFile;
    HFCPtr<HRPPixelType>    m_pPixelType;

    HFCPtr<HCDCodecLRDRLE>  m_pCodec;

    LRDHeaderBlock*         m_pLRDHeader;

    // Method
    void            SaveLRDFile(bool pi_CloseFile);
    void            InitOpenedFile();
    void            InitScanlineOrientation();

    void            AsciiDate (char adate[]);
    void            AsciiTime (char atime[]);


    HFCPtr<HGF2DTransfoModel>
    GetTransfoModel() const;

    bool           ComputeRasterDimension();
    bool           Create();
    bool           CreateFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage);
    bool           CreateHeaderBlock(HRFResolutionDescriptor* pi_pResolutionDescriptor);

    void            WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_pModel);

    int32_t             swaplong   ( char buf[] );
    int32_t             swapdouble ( char buf[] );

    HFCPtr<HRPPixelType>
    GetPixelType() const;

    // Methods Disabled
    HRFLRDFile (const HRFLRDFile& pi_rObj);
    HRFLRDFile& operator=(const HRFLRDFile& pi_rObj);
    };

// Re-enable the automatic word alingnement by removing the previous pragma instruction
// at the begining of this class
#pragma pack( pop, IntergraphIdent)



// LRD Creator.
struct HRFLRDCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;
    // Identification information
    virtual WString                   GetLabel()      const;
    virtual WString                   GetSchemes()    const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFLRDCreator)

    // Disabled methodes
    HRFLRDCreator();
    };
END_IMAGEPP_NAMESPACE

