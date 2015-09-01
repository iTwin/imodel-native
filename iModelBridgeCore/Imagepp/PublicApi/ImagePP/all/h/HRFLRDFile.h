//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFLRDFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

// Want to hang on other plateform because pragma instruction is normally compiler dependent
#ifdef _WIN32
// Disable the automatic word alingnement for intergraph structure.
#pragma pack( push, IntergraphIdent,  1)
#else
#pragma pack( push, 1)
#endif

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
                                                                 unsigned short           pi_Resolution,
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
        long VecLevel;
        long Recordcount;
        char Date[10];
        char Time[10];
        char rFile[40];
        long bStart;
        long bStop;
        long BitsCount;
        long rStart;
        long rStop;
        long maxmis;
        long minwid;
        long eps;
        long mintr0;
        long maxtr0;
        long fuzz;
        long widfly;
        long widhol;
        long talhol;
        long newtop;
        long botmin;
        long hmod;
        long corsys;
        long xpdens;
        long ypdens;
        long vsmin;
        long vsmax;
        long vxmin;
        long vxmax;                     // 168 bytes from the begining.
        Byte TheExpense[208];  // Unknow space... 512 - (168 + 136)
        long valid;
        long slo;
        double Matrix[16];              // 128 bytes used.
        };

    bool                   m_HasHeaderFilled;
    unsigned short         m_BitPerPixel;
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

    int             swaplong   ( char buf[] );
    int             swapdouble ( char buf[] );

    HFCPtr<HRPPixelType>
    GetPixelType() const;

    // Methods Disabled
    HRFLRDFile (const HRFLRDFile& pi_rObj);
    HRFLRDFile& operator=(const HRFLRDFile& pi_rObj);
    };

// Re-enable the automatic word alingnement by removing the previous pragma instruction
// at the begining of this class
#ifdef _WIN32
#pragma pack( pop, IntergraphIdent)
#else
#pragma pack( pop)
#endif


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

