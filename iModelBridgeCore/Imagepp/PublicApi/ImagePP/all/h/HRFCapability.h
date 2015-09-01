//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCapability.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HRFTypes.h"
#include "HTIFFTag.h"
#include "HFCAccessMode.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFileCapabilities;
class HPMGenericAttribute;

/** -----------------------------------------------------------------------------
    @version 1.0
    @end

    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @end

    This class is used to hold description of any raster file format capabilities. These
    capabilities describe the functionalities supported by a file format. Capabilities
    include such thing as multi-page support, different pixel types, compressions,
    still image/animation and data storage organization possibilities. All these can be
    descendants from the present abstract class.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    Just to show that we can put image in doc:
    <img src="../../Images/HMR_StratUp.jpg" alt="HMR logo">

    @see HRFRasteFileCapabilities
    @see <a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFCapability.doc"> HRFCapability.doc </a></LI>
    -----------------------------------------------------------------------------
 */
class HRFCapability : public HFCShareableObject<HRFCapability>
    {
    HDECLARE_BASECLASS_ID(HRFCapabilityId_Base)

public:
    HRFCapability(HFCAccessMode pi_AccessMode);
    virtual ~HRFCapability();

    virtual bool           SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual bool           IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual HFCAccessMode   GetAccessMode() const;

protected:
    HFCAccessMode m_AccessMode;
private:
    HRFCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in specific parametrized blocked organization. Althoug the class
    is not abstract, it is meant to be inherited from by other more precise blocking
    architectures such as line, tiled and so on. Refer to descendants for details.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFLineCapability
    @see HRFStripCapability
    @see HRFTileCapability
    @see HRFImageCapability
    @see HRFCodecCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFBlockCapability.doc"> HRFBlockCapability.doc </a></LI>
    -----------------------------------------------------------------------------
 */
class HRFBlockCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Block, HRFCapability)


public:
    IMAGEPP_EXPORT HRFBlockCapability(HFCAccessMode    pi_AccessMode,
                              HRFBlockType     pi_BlockType,
                              uint32_t         pi_MaxSizeInBytes,
                              HRFBlockAccess   pi_BlockAccess = HRFBlockAccess::RANDOM);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

    virtual HRFBlockType     GetBlockType() const;
    virtual uint32_t         GetMaxSizeInBytes() const;
    virtual HRFBlockAccess   GetBlockAccess() const;

protected:
    HRFBlockType     m_BlockType;
    uint32_t         m_MaxSizeInBytes;
    HRFBlockAccess   m_BlockAccess;

private:
    HRFBlockCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in specific compression-decompression algorithm.

    The relation between block organization and raster data compression is very intimate.
    Some compression algorithm will work better if the block organization is in a specific
    way. For example jpeg compression and BMP compress file need to have a  specific
    block organization.

    Note that codec capability is also related to pixel type capability. Since,
    some compress are available only for specific pixel type.

    Went no compression is available on a specific pixel type the codec Identity
    should always be used.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFBlockCapability
    @see HRFPixelTypeCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFCodecCapability.doc"> HRFCodecCapability.doc </a></LI>
    -----------------------------------------------------------------------------
 */
class HRFCodecCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Codec, HRFCapability)

public:
    IMAGEPP_EXPORT                 HRFCodecCapability(HFCAccessMode                            pi_AccessMode,
                                              HCLASS_ID                              pi_Codec,
                                              const HFCPtr<HRFRasterFileCapabilities>& pi_rBlockTypeList);

    // Utilities
    virtual bool       SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual bool       IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual HCLASS_ID   GetCodecClassID() const;


    // BlockType operations
    virtual const HFCPtr<HRFBlockCapability>         GetBlockTypeCapability(uint32_t pi_Index) const;
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetBlockTypeCapabilities() const;
    virtual uint32_t                                 CountBlockType() const;
    virtual bool                                    SupportsBlockType(HRFBlockType pi_BlockType) const;

protected:
    HCLASS_ID m_Codec;

    // List of block type
    HFCPtr<HRFRasterFileCapabilities> m_pBlockTypeList;
private:
    HRFCodecCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to hold description of pixel type support by raster file format
    capabilities. These capabilities describe the functionalities supported by a file
    format directly related to pixel type. Pixel type capabilities include such thing
    as raster data compression support, down sampling methods and so on.

    The relation between raster data compression and pixel type is very intimate.
    Some compression algorithms can compress some pixel types and no other, while
    other compression algorithms are completely color blind and non-raster aware.
    The capabilities related to the pixel types are used to describe what kind of pixel
    types is supported by specific raster file format as well as data compression.

    Note that a given pixel type must have at least a associate codec, if no
    compression is permitted the codec Identity must be used.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFPixelTypeCapability.doc"> HRFPixelTypeCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFPixelTypeCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_PixelType, HRFCapability)

public:
    IMAGEPP_EXPORT                 HRFPixelTypeCapability(HFCAccessMode                            pi_AccessMode,
                                                  HCLASS_ID                              pi_PixelType,
                                                  const HFCPtr<HRFRasterFileCapabilities>& pi_rpListOfCodec);

    // Utilities
    virtual bool        SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual bool        IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual HCLASS_ID    GetPixelTypeClassID() const;


    // Codec operation.
    virtual const HFCPtr<HRFCodecCapability>         GetCodecCapabilityByIndex(uint32_t pi_Index) const;
    virtual const HFCPtr<HRFCodecCapability>         GetCodecCapability(HCLASS_ID pi_Codec) const;
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCodecCapabilities() const;
    virtual uint32_t                                 CountCodecs() const;
    virtual bool                                    SupportsCodec(HCLASS_ID pi_Codec) const;


    // DownSamplingMethod operation.
    virtual void                   AddDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    virtual HRFDownSamplingMethod  GetDownSamplingMethod(uint32_t pi_Index) const;
    virtual uint32_t               CountDownSamplingMethod() const;
    virtual bool                  SupportsDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod) const;

protected:
    HCLASS_ID m_PixelType;

    // List of codec
    HFCPtr<HRFRasterFileCapabilities>  m_pCodecList;

    // list of DownSamplingMethod
    typedef vector<HRFDownSamplingMethod>
    ListOfDownSamplingMethod;

    ListOfDownSamplingMethod  m_ListOfDownSamplingMethod;
private:
    HRFPixelTypeCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    interlaced raster data in raster data blocks. This interlace must be applicable to all
    block types supported by raster.

    Interlace is the property of rasters to be stored using some scanlines while jumping over
    some others then after the bottom of the block has been reached, start again at the top
    and then scan again to the bottom with some other scan lines. A typical use of interlacing
    requires that in the raw block of data, all odd numbers scanlines are stored and after
    this very last odd rank scanline, the even numbered scanlines are stored. The Interlacing
    is used for example in US style television signals where TV frames are sent and displayed
    in 2 consecutive interlaced frames.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    <h3>see </h3>
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFInterlaceCapability.doc"> HRFInterlaceCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFInterlaceCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Interlace, HRFCapability)

public:
    IMAGEPP_EXPORT HRFInterlaceCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFInterlaceCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in specific scanline orientation. <font color=red>This orientation must
    be applicable to all block types supported by raster.</font> Many orientation can
    be supported by a same file format.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFScanlineOrientation
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFScanlineOrientationCapability.doc"> HRFScanlineOrientationCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFScanlineOrientationCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_ScanlineOrientation, HRFCapability)

public:
    IMAGEPP_EXPORT HRFScanlineOrientationCapability(HFCAccessMode           pi_AccessMode,
                                            HRFScanlineOrientation  pi_ScanlineOrientation);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual HRFScanlineOrientation
    GetScanlineOrientation() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    HRFScanlineOrientation  m_ScanlineOrientation;

private:
    HRFScanlineOrientationCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in different interleave of raster data organization. This interleave
    must be applicable to all block types supported by raster.

    Interleave is the property of storing in the raw data blocks pixels channels in
    separate scanlines, or planes. For a true color RGB pixel type, the first scanline
    might contain the values of the Red channel, the next scanline the values of the
    Green then another scanline for the Blue. All three color separated scanlines form
    a single pixel scanline. Interleave can be applied on each individual scanlines,
    or on the whole block of data.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFInterleaveType
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFInterleaveCapability.doc"> HRFInterleaveCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFInterleaveCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Interleave, HRFCapability)

public:
    IMAGEPP_EXPORT HRFInterleaveCapability(HFCAccessMode       pi_AccessMode,
                                   HRFInterleaveType   pi_InterleaveType);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;
    virtual HRFInterleaveType
    GetInterleaveType() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    HRFInterleaveType m_InterleaveType;

private:
    HRFInterleaveCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in multi-resolution.

    Went this capability is set on a file format it mean that we <font color=red>must</font>
    have sub-resolution data in the file.

    This capability is closely connected to the HRFSingleResolutionCapability. At least one
    of those two must be set on a file format. If the HRFMultiResolutionCapability is set on
    a specific file format, we absolutly need to write the sub-resolution. While if
    HRFSingleResolutionCapability is set, we only write the first resolution (1:1)

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFSingleResolutionCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFMultiResolutionCapability.doc"> HRFMultiResolutionCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFMultiResolutionCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_MultiResolution, HRFCapability)

public:
    IMAGEPP_EXPORT HRFMultiResolutionCapability(
        HFCAccessMode       pi_AccessMode,
        bool               pi_SinglePixelType      = true,
        bool               pi_SingleBlockType      = true,
        bool               pi_ArbitaryXRatio       = false,
        bool               pi_ArbitaryYRatio       = false,
        bool               pi_XYRatioLocked        = true,
        uint32_t            pi_SmallestResWidth     = 256,
        uint32_t            pi_SmallestResHeight    = 256,
        uint64_t           pi_BiggestResWidth      = ULONG_MAX,
        uint64_t           pi_BiggestResHeight     = ULONG_MAX,
        bool               pi_UnlimitedResolution  = false);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual bool   IsUnlimitedResolution() const;
    virtual uint32_t GetSmallestResWidth() const;
    virtual uint32_t GetSmallestResHeight() const;
    virtual uint64_t GetBiggestResWidth() const;
    virtual uint64_t GetBiggestResHeight() const;
    virtual bool   IsSinglePixelType() const;
    virtual bool   IsSingleBlockType() const;
    virtual bool   IsArbitaryXRatio() const;
    virtual bool   IsArbitaryYRatio() const;
    virtual bool   IsXYRatioLocked() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    bool   m_UnlimitedResolution;
    bool   m_SinglePixelType;
    bool   m_SingleBlockType;
    bool   m_ArbitaryXRatio;
    bool   m_ArbitaryYRatio;
    bool   m_XYRatioLocked;
    uint32_t m_SmallestResWidth;
    uint32_t m_SmallestResHeight;
    uint64_t m_BiggestResWidth;
    uint64_t m_BiggestResHeight;

private:
    HRFMultiResolutionCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    a single resolution per page only.

    This capability is closely connected to the HRFMultiResolutionCapability. At least one
    of those two must be set on a file format. If the HRFMultiResolutionCapability is set on
    a specific file format, we absolutly need to write the sub-resolution. While if
    HRFSingleResolutionCapability is set, we only write the first resolution (1:1)

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFMultiResolutionCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFSingleResolutionCapability.doc"> HRFSingleResolutionCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFSingleResolutionCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_SingleResolution, HRFCapability)

public:
    IMAGEPP_EXPORT HRFSingleResolutionCapability(HFCAccessMode  pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

private:
    HRFSingleResolutionCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in line organization. The class indicates if lines can be access randomly
    or if they must be read sequentially. A parameter also indicates the maximum number
    of bytes that can be stored in a raster compressed line.

    This capability is one of many possible block capabilities indicating a raster format
    supports this type of architecture. Different block architecture can be specified for
    a same format and are thus not mutually exclusive.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFStripCapability
    @see HRFTileCapability
    @see HRFImageCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFLineCapability.doc"> HRFLineCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFLineCapability : public HRFBlockCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Line, HRFBlockCapability)

public:
    IMAGEPP_EXPORT HRFLineCapability (HFCAccessMode    pi_AccessMode,
                              uint32_t         pi_MaxSizeInBytes,
                              HRFBlockAccess   pi_BlockAccess = HRFBlockAccess::RANDOM);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

private:
    HRFLineCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in strip organization. The class indicates if strips can be accessed
    randomly or if they must be read sequentially. A parameter also indicates the maximum
    number of bytes that can be stored in a raster strip compressed. Additionaly, strip
    geometric limitations can be imposed such as minimum and maximum strip sizes as well
    a strip size increments.

    The increment implies that the strip height can be any size between minimum height
    plus N times the increment as long as the result is smaller or equal to maximum height,
    where N must be an integer. Minimum and maximum strip height are valid values for
    strip height.

    This capability is one of many possible block capabilities indicating a raster format
    supports this type of architecture. Different block architecture can be specified for
    a same format and are thus not mutually exclusive.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFLineCapability
    @see HRFTileCapability
    @see HRFImageCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFStripCapability.doc"> HRFStripCapability.doc </a></LI>
    -----------------------------------------------------------------------------
 */
class HRFStripCapability : public HRFBlockCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Strip, HRFBlockCapability)

public:
    IMAGEPP_EXPORT HRFStripCapability( HFCAccessMode    pi_AccessMode,
                               uint32_t         pi_MaxSizeInBytes,

                               uint32_t         pi_MinHeight,
                               uint32_t         pi_MaxHeight,
                               uint32_t         pi_HeightIncrement,
                               HRFBlockAccess   pi_BlockAccess = HRFBlockAccess::RANDOM);

    virtual uint32_t GetMinHeight() const;
    virtual uint32_t GetMaxHeight() const;
    virtual uint32_t GetHeightIncrement() const;

    // Validate the specified value
    // return the same value if valid
    // otherwise return the near value
    virtual uint32_t ValidateHeight(uint32_t pi_Height);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    uint32_t m_MinHeight;
    uint32_t m_MaxHeight;
    uint32_t m_HeightIncrement;

private:
    HRFStripCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in tiled organization. Tiles can always be accessed. A parameter indicates
    the maximum number of bytes that can be stored in a raster tile compressed.  Additionaly,
    tile geometric limitations can be imposed such as minimum and maximum width or
    height sizes as well a width and height size increments. Tile can also always be square
    or permitted to become rectaangular.

    The increment implies that the tile height or width can be any size between minimum
    height(or width) plus N times the increment as long as the result is smaller or equal to
    maximum height(or width), where N must be an integer. Minimum and maximum tile height or
    width are valid values for tiles.

    This capability is one of many possible block capabilities indicating a raster format
    supports this type of architecture. Different block architecture can be specified for a
    same format and are thus not mutually exclusive.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFLineCapability
    @see HRFStripCapability
    @see HRFImageCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFTileCapability.doc"> HRFTileCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFTileCapability : public HRFBlockCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Tile, HRFBlockCapability)

public:
    IMAGEPP_EXPORT HRFTileCapability ( HFCAccessMode   pi_AccessMode,
                               uint32_t        pi_MaxSizeInBytes,

                               uint32_t        pi_MinWidth,
                               uint32_t        pi_MaxWidth,
                               uint32_t        pi_WidthIncrement,

                               uint32_t        pi_MinHeight,
                               uint32_t        pi_MaxHeight,
                               uint32_t        pi_HeightIncrement,
                               bool           pi_IsSquare = true);

    virtual uint32_t GetMinWidth() const;
    virtual uint32_t GetMaxWidth() const;
    virtual uint32_t GetWidthIncrement() const;

    virtual uint32_t GetMinHeight() const;
    virtual uint32_t GetMaxHeight() const;
    virtual uint32_t GetHeightIncrement() const;

    virtual bool   IsSquare() const;

    // Validate the specified value
    // return the same value if valid
    // otherwise return the near value
    virtual uint32_t ValidateWidth(uint32_t pi_Width);
    virtual uint32_t ValidateHeight(uint32_t pi_Height);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    uint32_t m_MinWidth;
    uint32_t m_MaxWidth;
    uint32_t m_WidthIncrement;

    uint32_t m_MinHeight;
    uint32_t m_MaxHeight;
    uint32_t m_HeightIncrement;
    bool  m_IsSquare;

private:
    HRFTileCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    raster data in image monoblock organization. A parameter indicates the maximum number
    of bytes that can be stored in a raster image compressed. Additionaly, image geometric
    limitations can be imposed such as minimum and maximum width or height sizes in
    pixels. If additional conditions apply concerning image geometry limitations,
    the class can be overloaded and the ValidateWidth() and ValidateHeight() methods
    overridden to take these into acount. These cannot however include limitations on width
    dependent on height and vice versa.

    Whenever selecting image sizes these MUST be validated using the ValidateWidth() and
    ValidateHeight() methods.

    Image architecture is quite different from tile architecture even though they appear
    to have the same parameters. The image architecture implies that the whole content
    of the resolution is stored in a single chunk of data.

    This capability is one of many possible block capabilities indicating a raster format
    supports this type of architecture. Different block architecture can be specified for a
    same format and are thus not mutually exclusive.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFLineCapability
    @see HRFTileCapability
    @see HRFStripCapability
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFImageCapability.doc"> HRFImageCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFImageCapability : public HRFBlockCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Image, HRFBlockCapability)

public:
    IMAGEPP_EXPORT HRFImageCapability( HFCAccessMode    pi_AccessMode,
                               uint32_t         pi_MaxSizeInBytes,
                               uint32_t         pi_MinWidth,
                               uint32_t         pi_MaxWidth,

                               uint32_t         pi_MinHeight,
                               uint32_t         pi_MaxHeight,
                               HRFBlockAccess   pi_BlockAccess = HRFBlockAccess::RANDOM);

    virtual uint32_t GetMinWidth() const;
    virtual uint32_t GetMaxWidth() const;

    virtual uint32_t GetMinHeight() const;
    virtual uint32_t GetMaxHeight() const;

    // Validate the specified value
    // return the same value if valid
    // otherwise return the near value
    virtual uint32_t ValidateWidth(uint32_t pi_Width);
    virtual uint32_t ValidateHeight(uint32_t pi_Height);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    uint32_t m_MinWidth;
    uint32_t m_MaxWidth;

    uint32_t m_MinHeight;
    uint32_t m_MaxHeight;
private:
    HRFImageCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    and using specific filters.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    <h3>see </h3>
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFCodecCapability.doc"> HRFCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFFilterCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Filter, HRFCapability)

public:
    IMAGEPP_EXPORT HRFFilterCapability(HFCAccessMode   pi_AccessMode,
                               HCLASS_ID     pi_Filter);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual HCLASS_ID GetFilter() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    HCLASS_ID m_Filter;

private:
    HRFFilterCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing a
    clip shape.

    This clipping shape is used to clip the raster rectangular area to another shape not
    rectangular. Data located outside the clip shape can or not be meaningful and it is
    up to the application to decide whether to display it or not.

    <font style="bold" color="red">
    The clip shape is usually represented as a polygon closed on itself made of many
    corners of segments. Some file formats limit the number of points that can be stored
    while other may store any number of points. It is also possible for a file format to
    store shapes in any native shape. Besides polygons, circles, rectangles, and ellipses
    can all represent a clip shape, and some file format support direct storage of such
    shape definition. In any case, the capacity to store natively shapes is not part of the
    capability but instead is part of the hidden implementation. The limit to the number
    of points apply only to file format that can only store shapes in the form of polygons
    and for which the amount of corner is limited.
    </font>

    Shapes can be stored using coordinate values expressed in either physical coordinate
    system or logical coordinate system. Most file format support storage of clip shapes
    as physical coordinate system coordinates, since modification to the geo-reference
    information will then not affect the visible part of the raster. It is however possible
    to indicate shapes should be stored using logical coordinate system coordinates.
    In such case, the shape has an exact location within the logical space.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCoordinateType
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFClipShapeCapability.doc"> HRFClipShapeCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFClipShapeCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_ClipShape, HRFCapability)

public:
    IMAGEPP_EXPORT HRFClipShapeCapability(HFCAccessMode     pi_AccessMode,
                                  HRFCoordinateType pi_CoordinateType);

    virtual bool             SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual HRFCoordinateType GetCoordinateType() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    HRFCoordinateType m_CoordinateType;

private:
    HRFClipShapeCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    transformation models of some given types.

    The capacity to store transformation models is a way to
    indicate for a file format the extent of geometric transformation movements
    that can be stored within the file to express the relation between physical
    coordinate system and logical coordinate system. The movements can only be
    described by adding to the file format capabilities all existing transformation
    models classes that can be represented in stored format. The presence of a
    model that can represent complex geometric movements should imply that all less
    powerful transformation model should also be added as capability. For example
    if the transformation model HGF2DAffine is indicated supported by a HRFTransfoModelCapability,
    then the file format should also indicate support for HGF2DSimilitude,
    HGF2DStretch, HGF2DTranslation and HGF2DIdentity since the affine
    transformation includes all movements that can be expressed by these classes.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HGF2DTransfoModel
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFTransfoModelCapability.doc"> HRFTransfoModelCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFTransfoModelCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_TransfoModel, HRFCapability)

public:
    IMAGEPP_EXPORT HRFTransfoModelCapability(HFCAccessMode    pi_AccessMode,
                                     HCLASS_ID      pi_TransfoModel);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual HCLASS_ID GetTransfoModelClassKey () const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

protected:
    HCLASS_ID m_TransfoModel;

private:
    HRFTransfoModelCapability();
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format
    is capable of storing the histogram of a raster page. Indication of the
    capacity to store histogram <font color="red">will only apply to palette based pixel
    types with 256 entries or less.</font> Indication of such support implies that all palette
    based pixel types with 256 entries (<font color="red"> a bit limitative ??? maybe add a
    paramter ?)</font> or less can and will  store the histogram on demand. If this is not
    the case, then the histogram capability should not be indicated

    The indication of histogram support indicates that the file
    format can store up to 2<sup>N</sup> entries where N is the number of bits used
    to represent the palette index. A 1 bit palette based pixel type implies the
    file format can store at least 2 histogram entries. It is not a requirement for
    the file format to be able to always store 256 histogram entries.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    <h3>see HRFDownSamplingMethod</h3>
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFHistogramCapability.doc"> HRFHistogramCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFHistogramCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Histogram, HRFCapability)

public:
    IMAGEPP_EXPORT HRFHistogramCapability(HFCAccessMode    pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

private:
    HRFHistogramCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing
    a thumbnail for each of the raster page of the format. A thumbnail is a small visual
    or logical representation of the image represented by the page. A thumbnail must be
    stored in IMAGE block architecture and be considered small (A thumbnail are usually
    less than 256 by 256 pixels).

    A parameter indicates the maximum number of bytes that can be stored in a raster
    thumbnail. Additionaly, thumbnail geometric limitations can be imposed such as
    minimum and maximum width or height sizes as well a width and height size increments.

    The increment implies that the thumbnail height or width can be any size between
    minimum height (or width) plus N times the increment as long as the result is smaller
    or equal to maximum height(or width), where N must be an integer. Minimum and maximum
    thumbnail height or width are valid values for thumbnails.

    @null{AR: What is the COMPOSED setting!}

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFThumbnail
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFThumbnailCapability.doc"> HRFThumbnailCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFThumbnailCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Thumbnail, HRFCapability)

public:
    IMAGEPP_EXPORT HRFThumbnailCapability(HFCAccessMode    pi_AccessMode,

                                  uint32_t         pi_MinWidth        = 64,
                                  uint32_t         pi_MaxWidth        = 64,
                                  uint32_t         pi_WidthIncrement  = 1,

                                  uint32_t         pi_MinHeight       = 64,
                                  uint32_t         pi_MaxHeight       = 64,
                                  uint32_t         pi_HeightIncrement = 1,

                                  uint32_t         pi_MaxSizeInBytes  = 65535,
                                  bool            pi_IsComposed      = true);

    // Color space interface
    void        AddPixelType(HCLASS_ID pi_PixelType);

    uint32_t    CountPixelType() const;
    HCLASS_ID   GetPixelTypeClassID(uint32_t pi_Index) const;

    // dimension interface
    uint32_t    GetMinWidth() const;
    uint32_t    GetMaxWidth() const;
    uint32_t    GetWidthIncrement() const;

    uint32_t    GetMinHeight() const;
    uint32_t    GetMaxHeight() const;
    uint32_t    GetHeightIncrement() const;

    uint32_t    GetMaxSizeInBytes() const;

    bool       IsComposed() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }


private:
    HRFThumbnailCapability();

    // Define the list of pixel type.
    typedef vector<HCLASS_ID> ListOfPixelType;

    // Members.
    ListOfPixelType m_ListOfPixelType;

    uint32_t         m_MinWidth;
    uint32_t         m_MaxWidth;
    uint32_t         m_WidthIncrement;

    uint32_t         m_MinHeight;
    uint32_t         m_MaxHeight;
    uint32_t         m_HeightIncrement;

    uint32_t         m_MaxSizeInBytes;
    bool            m_IsComposed;
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    This class is used to indicate that the raster file format is capable of storing the
    representative palette of a raster page. Indication of the capacity to store
    representative palette will apply to all pixel types but will be exact for palette
    based pixel types with 256 entries or less.

    The indication of representative palette support indicates that the file format can
    store up to 2<sup>N</sup> entries where N is the number of bits used to represent
    the palette index. A 1 bit palette based pixel type implies the file format can
    store at least 2 representative palette entries. It is not a requirement for the file
    format to be able to always store 256 representative palette entries.

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFRepresentativePaletteCapability.doc"> HRFRepresentativePaletteCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFRepresentativePaletteCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_RepresentativePalette, HRFCapability)

public:
    IMAGEPP_EXPORT HRFRepresentativePaletteCapability(HFCAccessMode    pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFRepresentativePaletteCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFMultiPageCapability.doc"> HRFMultiPageCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFMultiPageCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_MultiPage, HRFCapability)

public:
    IMAGEPP_EXPORT HRFMultiPageCapability(HFCAccessMode    pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFMultiPageCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFTagCapability.doc"> HRFTagCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFTagCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Tag, HRFCapability)

public:
    IMAGEPP_EXPORT HRFTagCapability(   HFCAccessMode                       pi_AccessMode,
                               const HFCPtr<HPMGenericAttribute>&  pi_rpTag);

    virtual bool   SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const;

    virtual const HFCPtr<HPMGenericAttribute>& GetTag() const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }


protected:
    HRFTagCapability();

    HFCPtr<HPMGenericAttribute> m_Tag;
    };


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sébastien Tardif (${mailto:Sebastien.Tardif@hmrinc.com})
    @end

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFTagCapability.doc"> HRFTagCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFUniversalTagCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_UniversalTag, HRFCapability)

public:
    IMAGEPP_EXPORT HRFUniversalTagCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFUniversalTagCapability();
    };



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @end

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFBlocksDataFlagCapability.doc"> HRFBlocksDataFlagCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFBlocksDataFlagCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_BlocksDataFlag, HRFCapability)

public:
    IMAGEPP_EXPORT HRFBlocksDataFlagCapability(HFCAccessMode    pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFBlocksDataFlagCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})
    @end

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFSubSamplingCapability.doc"> HRFSubSamplingCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFSubSamplingCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_SubSampling, HRFCapability)

public:
    IMAGEPP_EXPORT HRFSubSamplingCapability(HFCAccessMode    pi_AccessMode);
    
    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFSubSamplingCapability();
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})
    @author Yvan Noury    (${mailto:Yvan.Noury@hmrinc.com})

    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The <b>write</b> access mode should be seen as update access, which means that re-write this
    capability to the file. The <b>create</b> access mode should be seen as export access.

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFEmbedingCapability.doc"> HRFEmbedingCapability.doc </a></LI>
   -----------------------------------------------------------------------------
 */
class HRFEmbedingCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Embeding, HRFCapability)

public:
    IMAGEPP_EXPORT HRFEmbedingCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFEmbedingCapability();
    };

/** -----------------------------------------------------------------------------
    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The @b{write} access mode should be seen as update access, which means that re-write this
    capability to the file. The @b{create} access mode should be seen as export access.

    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFStillImageCapability.doc"> HRF Still Image Capability documentation </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc"> HRF user guide documentation </a>}
   -----------------------------------------------------------------------------
 */
class HRFStillImageCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_StillImage, HRFCapability)

public:
    IMAGEPP_EXPORT HRFStillImageCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
    HRFStillImageCapability();
    };

/** -----------------------------------------------------------------------------
    To each capability is assigned an access mode. This access mode is simply an indication
    published to holders of the capability that the capability is available for reading,
    writing, creating or a combinaision of those accesses to a raster file.

    The @b{write} access mode should be seen as update access, which means that re-write this
    capability to the file. The @b{create} access mode should be seen as export access.

    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    @see HRFCodecCapability
    @see HRFDownSamplingMethod
    @end

    @h3{Word documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFAnimationCapability.doc"> HRF Animation Capability Documantation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc"> HRF user guide documentation </a>}
    -----------------------------------------------------------------------------
 */
class HRFAnimationCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Animation, HRFCapability)

public:
    IMAGEPP_EXPORT HRFAnimationCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }
private:
        HRFAnimationCapability();
    };

/** -----------------------------------------------------------------------------
To each capability is assigned an access mode. This access mode is simply an indication
published to holders of the capability that the capability is available for reading,
writing, creating or a combinision of those accesses to a raster file.

The @b{write} access mode should be seen as update access, which means that re-write this
capability to the file. The @b{create} access mode should be seen as export access.

@version 1.0
@author Mathieu St-Pierre
-----------------------------------------------------------------------------
*/
class HRFMaxFileSizeCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_MaxFileSize, HRFCapability)

public:
    IMAGEPP_EXPORT HRFMaxFileSizeCapability(HFCAccessMode pi_AccessMode,
                                    uint64_t        pi_MaxFileSize);

    uint64_t GetMaxFileSize() const;

    void    SetMaxFileSize(uint64_t pi_MaxFileSize);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }


private:
    HRFMaxFileSizeCapability();
    uint64_t m_MaxFileSize; //In bytes
    };

/** -----------------------------------------------------------------------------
To each capability is assigned an access mode. This access mode is simply an indication
published to holders of the capability that the capability is available for reading,
writing, creating or a combinision of those accesses to a raster file.

The @b{write} access mode should be seen as update access, which means that re-write this
capability to the file. The @b{create} access mode should be seen as export access.

@version 1.0
@author Mathieu St-Pierre
-----------------------------------------------------------------------------
*/
class HRFGeocodingCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Geocoding, HRFCapability)

public:
    IMAGEPP_EXPORT HRFGeocodingCapability(HFCAccessMode pi_AccessMode);

    IMAGEPP_EXPORT void AddSupportedKey 	(BentleyApi::ImagePP::TIFFGeoKey pi_GeoKey);
    IMAGEPP_EXPORT bool 		IsKeySupported  	(BentleyApi::ImagePP::TIFFGeoKey pi_GeoKey) const;
    IMAGEPP_EXPORT unsigned short GetNbGeotiffKeys	() const;
    IMAGEPP_EXPORT BentleyApi::ImagePP::TIFFGeoKey   GetGeotiffKey		(unsigned short pi_KeyIndex) const;

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }


private:
    HRFGeocodingCapability();

    typedef vector<BentleyApi::ImagePP::TIFFGeoKey> TIFFGeoKeyVector;

    TIFFGeoKeyVector m_SupportedGeoKeys;
    };



/** -----------------------------------------------------------------------------
This capability is used to define if the format can is ability to change size

@version 1.0
@author Ghislain Tardif
-----------------------------------------------------------------------------
*/
class HRFResizableCapability : public HRFCapability
    {
    HDECLARE_CLASS_ID(HRFCapabilityId_Resizable, HRFCapability)

public:
    IMAGEPP_EXPORT HRFResizableCapability(HFCAccessMode pi_AccessMode);

    virtual bool   IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const {
        return T_Super::IsCompatibleWith(pi_rpCapability);
        }

private:
    HRFResizableCapability();
    };
END_IMAGEPP_NAMESPACE
