//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileBlockAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileBlockAdapter
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFRasterFileExtender.h"
#include "HRFCombinedRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFCacheFileCreator;

class HRFRasterFileBlockAdapterBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRasterFileBlockAdapterBlockCapabilities();
    };

class HRFRasterFileBlockAdapterCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRasterFileBlockAdapterCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilities);

    };

class HRFRasterFileBlockAdapter : public HRFRasterFileExtender
    {
public:
    friend class HRFResolutionEditorDecorator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFRasterFileId_BlockAdapter, HRFRasterFileExtender)

    // This methods allow to Create the best adapter for the specified raster file.
    // If it is not possible to adapt the raster file we return the original raster file.
    IMAGEPP_EXPORT static HFCPtr<HRFRasterFile> CreateBestAdapterFor(HFCPtr<HRFRasterFile>  pi_rpForRasterFile);

    IMAGEPP_EXPORT static HFCPtr<HRFRasterFile> CreateBestAdapterBasedOnCacheFor(HFCPtr<HRFRasterFile> pi_rpForRasterFile,
                                                                         const HRFCacheFileCreator* pi_pCreator);

    struct BlockDescriptor
        {
        HRFBlockType    m_BlockType;
        uint32_t        m_BlockWidth;
        uint32_t        m_BlockHeight;
        };

    typedef map<uint32_t, BlockDescriptor> BlockDescriptorMap;

    // This methods allow to find the best adapter type for the specified raster file.
    static bool FindBestAdapterTypeFor (const HFCPtr<HRFRasterFile>&   pi_rpForRasterFile,
                                         BlockDescriptorMap*            po_pBlockDescriptorMap);

    static bool FindBestAdapterTypeFor(HFCPtr<HRFRasterFile>  pi_rpForRasterFile,
                                        HRFBlockType*          po_ToBlockType,
                                        uint32_t*                po_ToBlockWidth,
                                        uint32_t*                po_ToBlockHeight);

    // This methods allow to if it is possible to adapt this raster file.
    IMAGEPP_EXPORT static bool             CanAdapt(const HFCPtr<HRFRasterFile>&   pi_rpFromRasterFile,
                                             const BlockDescriptorMap&      pi_rBlockDescMap);

    IMAGEPP_EXPORT static bool             CanAdapt(HFCPtr<HRFRasterFile>  pi_rpFromRasterFile,
                                             HRFBlockType           pi_ToBlockType,
                                             uint32_t               pi_ToWidth,
                                             uint32_t               pi_ToHeight);


    // allow to Open an image file
    IMAGEPP_EXPORT                  HRFRasterFileBlockAdapter(HFCPtr<HRFRasterFile>&    pi_rpAdaptedFile,
                                                      const BlockDescriptorMap& pi_rBlockDescMap);

    IMAGEPP_EXPORT                  HRFRasterFileBlockAdapter(HFCPtr<HRFRasterFile>&  pi_rpAdaptedFile,
                                                      HRFBlockType            pi_AdaptToBlockType = HRFBlockType::STRIP,
                                                      uint32_t                pi_AdaptToBlockWidth  = 0,
                                                      uint32_t                pi_AdaptToBlockHeight = 1024);

    virtual                 ~HRFRasterFileBlockAdapter();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities         () const;

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator   () const;

    // File manipulation
    virtual bool                           AddPage                 (HFCPtr<HRFPageDescriptor>  pi_pPage);

    virtual HRFResolutionEditor*            CreateResolutionEditor  (uint32_t                   pi_Page,
                                                                     unsigned short            pi_Resolution,
                                                                     HFCAccessMode              pi_AccessMode);

    virtual bool                           ResizePage              (uint32_t                   pi_Page,
                                                                     uint64_t                  pi_NewWidth,
                                                                     uint64_t                  pi_NewHeight);

    virtual void                            Save();


    virtual bool                           IsOriginalRasterDataStorage() const;

protected:

    // Adapted
    HFCPtr<HRFRasterFileCapabilities>     m_pRasterFileCapabilities;

    // Methods
    virtual bool                       Open                (const BlockDescriptorMap&  pi_rBlockDescMap);
    virtual void                        CreateDescriptors   (const BlockDescriptorMap&  pi_rBlockDescMap);


private:

    void                                Close               ();
    void                                SynchronizeFiles    ();

    // Create the file
    bool                               Create              (const BlockDescriptorMap&  pi_rBlockDescMap);

    // Methods Disabled
    HRFRasterFileBlockAdapter(const HRFRasterFileBlockAdapter& pi_rObj);
    HRFRasterFileBlockAdapter&  operator=(const HRFRasterFileBlockAdapter& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

