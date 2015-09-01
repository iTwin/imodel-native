//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCapability.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFCapability
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HPMAttribute.h>
#include <Imagepp/all/h/HCDCodecFactory.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRFCapability::~HRFCapability()
    {
    };

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFCapability::HRFCapability(HFCAccessMode pi_AccessMode)
    {
    m_AccessMode = pi_AccessMode;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.
    @end

    @h3{Inheritance notes:}
        The base SameAs() method checks the class identifier which
        must be identical and makes sure that the self access mode
        is included in given. If additional conditions are required
        such as comparing additional members in descendants then
        this method must be overridden.
    @end


    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.
    @end

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.
    @end

    @see HFCAccessMode
    @see HRFRasterFileCapabilities#GetCapabilityOfType()
    @end

    @h3{Word documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFCapability.doc">Capability documentation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc">HRF Tutorial </a>}
    @end
    -----------------------------------------------------------------------------
 */
bool HRFCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    bool Result = false;

    if (pi_rpCapability->GetClassID() == GetClassID() &&
        m_AccessMode.IsIncluded(pi_rpCapability->m_AccessMode))
        {
        Result = true;
        }

    return Result;
    }

/** -----------------------------------------------------------------------------
    Compatibility compare operator. This method indicates if self is alike given
    capability. This method will return true if seft is supported or include in the
    given capability. For example, in the case of a block type capability we need
    to look up the minHeigth and maxHeigth and so on ... to be sure the that is
    include in the given capability.

    This method will be chage by a method call Supports().(and the role of self
    and the given capability will be reverse)

    <font style="bold" size=2>
    Inheritance notes:</font> The base IsCompatibleWith() method checks the class
                              identifier which must be identical and makes sure that
                              the self access mode is included in given. If additional
                              conditions are required such as comparing additional members
                              in descendants then this method must be overridden.

    @param pi_rpCapability Constant reference to a smart pointer making reference
                           to the capability to compare with.

    @return This method will return true if seft is supported or include in the
            given capability.

    @see HRFCapability::SameAs(const HFCPtr<HRFCapability>&)
    @see HRFRasterFileCapabilities::Supports(const HFCPtr<HRFCapability>&)
    -----------------------------------------------------------------------------
 */
bool HRFCapability::IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return SameAs(pi_rpCapability);
    }

/** -----------------------------------------------------------------------------
    This method return the capability access mode.

    @return The access mode returned indicates if the capability applies to
            reading, writing, creating raster file access etc.

    -----------------------------------------------------------------------------
 */
HFCAccessMode HRFCapability::GetAccessMode() const
    {
    return m_AccessMode;
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFPixelTypeCapability::HRFPixelTypeCapability(HFCAccessMode                            pi_AccessMode,
                                               HCLASS_ID                              pi_PixelType,
                                               const HFCPtr<HRFRasterFileCapabilities>& pi_rpCodecList)
    : HRFCapability(pi_AccessMode)
    {
    m_PixelType  = pi_PixelType;
    m_pCodecList = pi_rpCodecList;

    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFPixelTypeCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_PixelType == ((HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->GetPixelTypeClassID()));
    }

/** -----------------------------------------------------------------------------
    Compatibility compare operator. This method indicates if self is alike given
    capability. This method will return true if seft is supported or include in the
    given capability. For example, in the case of a block type capability we need
    to look up the minHeigth and maxHeigth and so on ... to be sure the that is
    include in the given capability.

    This method will be chage by a method call Supports().(and the role of self
    and the given capability will be reverse)

    @param pi_rpCapability Constant reference to a smart pointer making reference
                           to the capability to compare with.

    @return This method will return true if seft is supported or include in the
            given capability.

    @see HRFCapability::SameAs(const HFCPtr<HRFCapability>&)
    @see HRFRasterFileCapabilities::Supports(const HFCPtr<HRFCapability>&)
    -----------------------------------------------------------------------------
 */
bool HRFPixelTypeCapability::IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    bool Result = SameAs(pi_rpCapability);

    if(Result)
        {
        int32_t CodecCount = ((const HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->GetCodecCapabilities()->
                           CountCapabilities();

        if ((Result) && (CodecCount > 0))
            {
            bool CodecFound = false;

            for (int32_t index = 0; (index < CodecCount) && (!CodecFound); index++)
                {
                CodecFound = SupportsCodec(((const HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->GetCodecCapabilityByIndex(index)->GetCodecClassID());
                }

            Result = CodecFound;
            }

        if ((Result) && (((const HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->CountDownSamplingMethod() > 0))
            {
            bool DownSamplingMethodFound = false;
            for (uint32_t index = 0; (index < ((const HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->CountDownSamplingMethod()) && (!DownSamplingMethodFound); index++)
                {
                DownSamplingMethodFound = SupportsDownSamplingMethod(((const HFCPtr<HRFPixelTypeCapability>&)pi_rpCapability)->GetDownSamplingMethod(index));
                }

            Result = DownSamplingMethodFound;
            }
        }
    return Result;
    }

/** -----------------------------------------------------------------------------
    @return This method return the pixel type Class ID represante this
            pixel type capability.
    -----------------------------------------------------------------------------
 */
HCLASS_ID HRFPixelTypeCapability::GetPixelTypeClassID() const
    {
    return m_PixelType;
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::GetCodec
//-----------------------------------------------------------------------------
const HFCPtr<HRFCodecCapability> HRFPixelTypeCapability::GetCodecCapabilityByIndex(uint32_t pi_Index) const
    {
    return (const HFCPtr<HRFCodecCapability>&) m_pCodecList->GetCapability(pi_Index);
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::GetCodec
//-----------------------------------------------------------------------------
const HFCPtr<HRFCodecCapability> HRFPixelTypeCapability::GetCodecCapability(HCLASS_ID pi_Codec) const
    {
    HPRECONDITION(SupportsCodec(pi_Codec));

    bool Found = false;
    HFCPtr<HRFCodecCapability> pCodecCapability;

    for (uint32_t index = 0; (index < CountCodecs()) && (!Found); index++)
        {
        if (pi_Codec == GetCodecCapabilityByIndex(index)->GetCodecClassID())
            {
            pCodecCapability = GetCodecCapabilityByIndex(index);
            Found = true;
            }
        }

    return pCodecCapability;
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::GetCodecCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPixelTypeCapability::GetCodecCapabilities() const
    {
    return m_pCodecList;
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::CountCodecs
//-----------------------------------------------------------------------------
uint32_t HRFPixelTypeCapability::CountCodecs() const
    {
    return m_pCodecList->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::SupportsCodec
//-----------------------------------------------------------------------------
bool HRFPixelTypeCapability::SupportsCodec(HCLASS_ID pi_Codec) const
    {
    bool Result = false;

    for (uint32_t index = 0; (index < CountCodecs()) && (!Result); index++)
        if (pi_Codec == GetCodecCapabilityByIndex(index)->GetCodecClassID())
            Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::AddDownSamplingMethod
//-----------------------------------------------------------------------------
void HRFPixelTypeCapability::AddDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_ListOfDownSamplingMethod.push_back(pi_DownSamplingMethod);
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::GetDownSamplingMethod
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFPixelTypeCapability::GetDownSamplingMethod(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_ListOfDownSamplingMethod.size());
    return m_ListOfDownSamplingMethod[pi_Index];
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::CountDownSamplingMethod
//-----------------------------------------------------------------------------
uint32_t HRFPixelTypeCapability::CountDownSamplingMethod() const
    {
    return (uint32_t)m_ListOfDownSamplingMethod.size();
    }

//-----------------------------------------------------------------------------
// HRFPixelTypeCapability::SupportsDownSamplingMethod
//-----------------------------------------------------------------------------
bool HRFPixelTypeCapability::SupportsDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod) const
    {
    bool Result = false;

    for (uint32_t index = 0; (index < CountDownSamplingMethod()) && (!Result); index++)
        if (pi_DownSamplingMethod == GetDownSamplingMethod(index))
            Result = true;

    return Result;
    }


//-----------------------------------------------------------------------------
// HRFCodecCapability
//-----------------------------------------------------------------------------


/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFCodecCapability::HRFCodecCapability(HFCAccessMode                            pi_AccessMode,
                                       HCLASS_ID                              pi_Codec,
                                       const HFCPtr<HRFRasterFileCapabilities>& pi_rpBlockTypeList)

    : HRFCapability(pi_AccessMode)
    {
    HPRECONDITION(HCDCodecFactory::GetInstance().Create(pi_Codec) != NULL);
    m_Codec          = pi_Codec;
    m_pBlockTypeList = pi_rpBlockTypeList;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFCodecCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_Codec == ((HFCPtr<HRFCodecCapability>&)pi_rpCapability)->GetCodecClassID()));
    }

/** -----------------------------------------------------------------------------
    Compatibility compare operator. This method indicates if self is alike given
    capability. This method will return true if seft is supported or include in the
    given capability. For example, in the case of a block type capability we need
    to look up the minHeigth and maxHeigth and so on ... to be sure the that is
    include in the given capability.

    This method will be chage by a method call Supports().(and the role of self
    and the given capability will be reverse)

    @param pi_rpCapability Constant reference to a smart pointer making reference
                           to the capability to compare with.

    @return This method will return true if seft is supported or include in the
            given capability.

    @see HRFCapability::SameAs(const HFCPtr<HRFCapability>&)
    @see HRFRasterFileCapabilities::Supports(const HFCPtr<HRFCapability>&)
    -----------------------------------------------------------------------------
 */
bool HRFCodecCapability::IsCompatibleWith(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    bool Result         = SameAs(pi_rpCapability);
    int32_t BlockTypeCount = ((const HFCPtr<HRFCodecCapability>&)pi_rpCapability)->CountBlockType();

    if ((Result) && (BlockTypeCount > 0))
        {
        bool BlockTypeFound = false;

        for (int32_t index = 0; (index < BlockTypeCount) && (!BlockTypeFound); index++)
            BlockTypeFound = m_pBlockTypeList->Supports((const HFCPtr<HRFCapability>)((const HFCPtr<HRFCodecCapability>&)pi_rpCapability)->GetBlockTypeCapability(index));
        Result = BlockTypeFound;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// HRFCodecCapability::GetCodec
//-----------------------------------------------------------------------------
HCLASS_ID HRFCodecCapability::GetCodecClassID() const
    {
    return m_Codec;
    }

//-----------------------------------------------------------------------------
// HRFCodecCapability::GetBlockType
//-----------------------------------------------------------------------------
const HFCPtr<HRFBlockCapability> HRFCodecCapability::GetBlockTypeCapability(uint32_t pi_Index) const
    {
    return (const HFCPtr<HRFBlockCapability>&) m_pBlockTypeList->GetCapability(pi_Index);
    }

//-----------------------------------------------------------------------------
// HRFCodecCapability::GetBlockTypeCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFCodecCapability::GetBlockTypeCapabilities() const
    {
    return m_pBlockTypeList;
    }

//-----------------------------------------------------------------------------
// HRFCodecCapability::CountBlockType
//-----------------------------------------------------------------------------
uint32_t HRFCodecCapability::CountBlockType() const
    {
    return m_pBlockTypeList->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// HRFCodecCapability::SupportsBlockType
//-----------------------------------------------------------------------------
bool HRFCodecCapability::SupportsBlockType(HRFBlockType pi_BlockType) const
    {
    bool Result = false;

    for (uint32_t index = 0; (index < CountBlockType()) && (!Result); index++)
        if (pi_BlockType == GetBlockTypeCapability(index)->GetBlockType())
            Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// HRFInterlaceCapability
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFInterlaceCapability::HRFInterlaceCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFRepresentativePaletteCapability
//-----------------------------------------------------------------------------


/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFRepresentativePaletteCapability::HRFRepresentativePaletteCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFScanlineOrientationCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFScanlineOrientationCapability::HRFScanlineOrientationCapability(
    HFCAccessMode           pi_AccessMode,
    HRFScanlineOrientation  pi_ScanlineOrientation)

    : HRFCapability(pi_AccessMode)
    {
    m_ScanlineOrientation = pi_ScanlineOrientation;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFScanlineOrientationCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability)  const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_ScanlineOrientation == ((HFCPtr<HRFScanlineOrientationCapability>&)pi_rpCapability)->GetScanlineOrientation()));
    }

//-----------------------------------------------------------------------------
// HRFScanlineOrientationCapability::GetScanlineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFScanlineOrientationCapability::GetScanlineOrientation()  const
    {
    return m_ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// HRFInterleaveCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFInterleaveCapability::HRFInterleaveCapability(HFCAccessMode       pi_AccessMode,
                                                 HRFInterleaveType   pi_InterleaveType)
    : HRFCapability(pi_AccessMode)
    {
    m_InterleaveType = pi_InterleaveType;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFInterleaveCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_InterleaveType == ((HFCPtr<HRFInterleaveCapability>&)pi_rpCapability)->GetInterleaveType()));
    }

//-----------------------------------------------------------------------------
// HRFInterleaveCapability::GetInterleaveType
//-----------------------------------------------------------------------------
HRFInterleaveType HRFInterleaveCapability::GetInterleaveType() const
    {
    return m_InterleaveType;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFMultiResolutionCapability::HRFMultiResolutionCapability(
    HFCAccessMode       pi_AccessMode,
    bool               pi_SinglePixelType,
    bool               pi_SingleBlockType,
    bool               pi_ArbitaryXRatio,
    bool               pi_ArbitaryYRatio,
    bool               pi_XYRatioLocked,
    uint32_t            pi_SmallestResWidth,
    uint32_t            pi_SmallestResHeight,
    uint64_t           pi_BiggestResWidth,
    uint64_t           pi_BiggestResHeight,
    bool               pi_UnlimitedResolution)
    : HRFCapability(pi_AccessMode)
    {
    m_SinglePixelType      = pi_SinglePixelType;
    m_SingleBlockType      = pi_SingleBlockType;
    m_ArbitaryXRatio       = pi_ArbitaryXRatio;
    m_ArbitaryYRatio       = pi_ArbitaryYRatio;
    m_XYRatioLocked        = pi_XYRatioLocked;
    m_SmallestResWidth     = pi_SmallestResWidth;
    m_SmallestResHeight    = pi_SmallestResHeight;
    m_BiggestResWidth      = pi_BiggestResWidth;
    m_BiggestResHeight     = pi_BiggestResHeight;
    m_UnlimitedResolution  = pi_UnlimitedResolution;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFMultiResolutionCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return (HRFCapability::SameAs(pi_rpCapability));
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::GetSmallestResWidth
//-----------------------------------------------------------------------------
uint32_t HRFMultiResolutionCapability::GetSmallestResWidth() const
    {
    return m_SmallestResWidth;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::GetSmallestResHeight
//-----------------------------------------------------------------------------
uint32_t HRFMultiResolutionCapability::GetSmallestResHeight() const
    {
    return m_SmallestResHeight;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::GetBiggestResWidth
//-----------------------------------------------------------------------------
uint64_t HRFMultiResolutionCapability::GetBiggestResWidth() const
    {
    HPRECONDITION(m_UnlimitedResolution);
    return m_BiggestResWidth;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::GetBiggestResHeight
//-----------------------------------------------------------------------------
uint64_t HRFMultiResolutionCapability::GetBiggestResHeight() const
    {
    HPRECONDITION(m_UnlimitedResolution);
    return m_BiggestResHeight;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsSinglePixelType
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsSinglePixelType() const
    {
    return m_SinglePixelType;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsSingleBlockType
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsSingleBlockType() const
    {
    return m_SingleBlockType;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsArbitaryXRatio
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsArbitaryXRatio() const
    {
    return m_ArbitaryXRatio;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsArbitaryYRatio
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsArbitaryYRatio() const
    {
    return m_ArbitaryYRatio;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsXYRatioLocked
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsXYRatioLocked() const
    {
    return m_XYRatioLocked;
    }

//-----------------------------------------------------------------------------
// HRFMultiResolutionCapability::IsUnlimitedResolution
//-----------------------------------------------------------------------------
bool HRFMultiResolutionCapability::IsUnlimitedResolution() const
    {
    return m_UnlimitedResolution;
    }

//-----------------------------------------------------------------------------
// HRFSingleResolutionCapability
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFSingleResolutionCapability::HRFSingleResolutionCapability(HFCAccessMode  pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFBlockCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFBlockCapability::HRFBlockCapability(HFCAccessMode    pi_AccessMode,
                                       HRFBlockType     pi_BlockType,
                                       uint32_t         pi_MaxSizeInBytes,
                                       HRFBlockAccess   pi_BlockAccess)
    : HRFCapability(pi_AccessMode)
    {
    m_BlockType      = pi_BlockType;
    m_MaxSizeInBytes = pi_MaxSizeInBytes;
    m_BlockAccess    = pi_BlockAccess;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFBlockCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_BlockType == ((HFCPtr<HRFBlockCapability>&)pi_rpCapability)->GetBlockType()));
    }

//-----------------------------------------------------------------------------
// HRFBlockCapability::GetBlockType
//-----------------------------------------------------------------------------
HRFBlockType HRFBlockCapability::GetBlockType() const
    {
    return m_BlockType;
    }

//-----------------------------------------------------------------------------
// HRFBlockCapability::GetMaxSizeInBytes
//-----------------------------------------------------------------------------
uint32_t HRFBlockCapability::GetMaxSizeInBytes() const
    {
    return m_MaxSizeInBytes;
    }

//-----------------------------------------------------------------------------
// HRFBlockCapability::GetBlockAccess
//-----------------------------------------------------------------------------
HRFBlockAccess HRFBlockCapability::GetBlockAccess() const
    {
    return m_BlockAccess;
    }

//-----------------------------------------------------------------------------
// HRFLineCapability
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HRFLineCapability
//-----------------------------------------------------------------------------
HRFLineCapability::HRFLineCapability (HFCAccessMode    pi_AccessMode,
                                      uint32_t         pi_MaxSizeInBytes,
                                      HRFBlockAccess    pi_BlockAccess)
    : HRFBlockCapability(pi_AccessMode, HRFBlockType::LINE, pi_MaxSizeInBytes, pi_BlockAccess)
    {
    }


//-----------------------------------------------------------------------------
// HRFStripCapability
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFStripCapability
//-----------------------------------------------------------------------------
HRFStripCapability::HRFStripCapability( HFCAccessMode    pi_AccessMode,
                                        uint32_t         pi_MaxSizeInBytes,

                                        uint32_t         pi_MinHeight,
                                        uint32_t         pi_MaxHeight,
                                        uint32_t         pi_HeightIncrement,
                                        HRFBlockAccess   pi_BlockAccess)
    : HRFBlockCapability(pi_AccessMode, HRFBlockType::STRIP, pi_MaxSizeInBytes, pi_BlockAccess)
    {
    m_MinHeight         = pi_MinHeight;
    m_MaxHeight         = pi_MaxHeight;
    m_HeightIncrement   = pi_HeightIncrement;
    }

//-----------------------------------------------------------------------------
// HRFStripCapability::GetMinHeight
//-----------------------------------------------------------------------------
uint32_t HRFStripCapability::GetMinHeight() const
    {
    return m_MinHeight;
    }

//-----------------------------------------------------------------------------
// HRFStripCapability::GetMaxHeight
//-----------------------------------------------------------------------------
uint32_t HRFStripCapability::GetMaxHeight() const
    {
    return m_MaxHeight;
    }

//-----------------------------------------------------------------------------
// HRFStripCapability::GetHeightIncrement
//-----------------------------------------------------------------------------
uint32_t HRFStripCapability::GetHeightIncrement() const
    {
    return m_HeightIncrement;
    }

//-----------------------------------------------------------------------------
// HRFStripCapability::ValidateHeight
// Validate the specified value
// return the same value if valid
// otherwise return the near value
//-----------------------------------------------------------------------------
uint32_t HRFStripCapability::ValidateHeight(uint32_t pi_Height)
    {
    // Calc the block size near of pi_Width
    uint32_t BlockHeight = GetMinHeight();
    uint32_t HeightIncrement = GetHeightIncrement();

    if (HeightIncrement > 0)
        {
        BlockHeight = pi_Height / HeightIncrement * HeightIncrement;
        // Test if the block is include inside the range
        if (BlockHeight > GetMaxHeight())
            BlockHeight = GetMaxHeight();
        else if (BlockHeight < GetMinHeight())
            BlockHeight = GetMinHeight();
        }
    else
        // Default Block Height
        BlockHeight = GetMinHeight();

    return BlockHeight;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HRFTileCapability
//-----------------------------------------------------------------------------
HRFTileCapability::HRFTileCapability(   HFCAccessMode   pi_AccessMode,
                                        uint32_t        pi_MaxSizeInBytes,

                                        uint32_t        pi_MinWidth,
                                        uint32_t        pi_MaxWidth,
                                        uint32_t        pi_WidthIncrement,

                                        uint32_t        pi_MinHeight,
                                        uint32_t        pi_MaxHeight,
                                        uint32_t        pi_HeightIncrement,
                                        bool           pi_IsSquare)
    : HRFBlockCapability(pi_AccessMode, HRFBlockType::TILE, pi_MaxSizeInBytes)
    {
    m_MinWidth          = pi_MinWidth;
    m_MaxWidth          = pi_MaxWidth;
    m_WidthIncrement    = pi_WidthIncrement;

    m_MinHeight         = pi_MinHeight;
    m_MaxHeight         = pi_MaxHeight;
    m_HeightIncrement   = pi_HeightIncrement;
    m_IsSquare          = pi_IsSquare;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetMinWidth
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetMinWidth() const
    {
    return m_MinWidth;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetMaxWidth
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetMaxWidth() const
    {
    return m_MaxWidth;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetWidthIncrement
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetWidthIncrement() const
    {
    return m_WidthIncrement;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetMinHeight
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetMinHeight() const
    {
    return m_MinHeight;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetMaxHeight
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetMaxHeight() const
    {
    return m_MaxHeight;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::GetHeightIncrement
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::GetHeightIncrement() const
    {
    return m_HeightIncrement;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::IsQuare
//-----------------------------------------------------------------------------
bool HRFTileCapability::IsSquare() const
    {
    return m_IsSquare;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::ValidateWidth
// Validate the specified value
// return the same value if valid
// otherwise return the near value
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::ValidateWidth(uint32_t pi_Width)
    {
    // Calc the block size near of pi_Width
    uint32_t BlockWidth = GetMinWidth();
    uint32_t WidthIncrement = GetWidthIncrement();

    if (WidthIncrement > 0)
        {
        BlockWidth = pi_Width / WidthIncrement * WidthIncrement;
        // Test if the block is include inside the range
        if (BlockWidth > GetMaxWidth())
            BlockWidth = GetMaxWidth();
        else if (BlockWidth < GetMinWidth())
            BlockWidth = GetMinWidth();
        }
    else
        // Default Block Width
        BlockWidth = GetMinWidth();

    return BlockWidth;
    }

//-----------------------------------------------------------------------------
// HRFTileCapability::ValidateHeight
// Validate the specified value
// return the same value if valid
// otherwise return the near value
//-----------------------------------------------------------------------------
uint32_t HRFTileCapability::ValidateHeight(uint32_t pi_Height)
    {
    // Calc the block size near of pi_Width
    uint32_t BlockHeight = GetMinHeight();
    uint32_t HeightIncrement = GetHeightIncrement();

    if (HeightIncrement > 0)
        {
        BlockHeight = pi_Height / HeightIncrement * HeightIncrement;
        // Test if the block is include inside the range
        if (BlockHeight > GetMaxHeight())
            BlockHeight = GetMaxHeight();
        else if (BlockHeight < GetMinHeight())
            BlockHeight = GetMinHeight();
        }
    else
        // Default Block Height
        BlockHeight = GetMinHeight();

    return BlockHeight;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HRFImageCapability
//-----------------------------------------------------------------------------
HRFImageCapability::HRFImageCapability( HFCAccessMode    pi_AccessMode,
                                        uint32_t         pi_MaxSizeInBytes,
                                        uint32_t         pi_MinWidth,
                                        uint32_t         pi_MaxWidth,

                                        uint32_t         pi_MinHeight,
                                        uint32_t         pi_MaxHeight,
                                        HRFBlockAccess   pi_BlockAccess)
    : HRFBlockCapability(pi_AccessMode, HRFBlockType::IMAGE, pi_MaxSizeInBytes, pi_BlockAccess)
    {
    m_MinWidth          = pi_MinWidth;
    m_MaxWidth          = pi_MaxWidth;

    m_MinHeight         = pi_MinHeight;
    m_MaxHeight         = pi_MaxHeight;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::GetMinWidth
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::GetMinWidth() const
    {
    return m_MinWidth;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::GetMaxWidth
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::GetMaxWidth() const
    {
    return m_MaxWidth;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::GetMinHeight
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::GetMinHeight() const
    {
    return m_MinHeight;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::GetMaxHeight
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::GetMaxHeight() const
    {
    return m_MaxHeight;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::ValidateWidth
// Validate the specified value
// return the same value if valid
// otherwise return the near value
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::ValidateWidth(uint32_t pi_Width)
    {
    // Calc the block size near of pi_Width
    uint32_t BlockWidth = pi_Width;

    // Test if the block is include inside the range
    if (pi_Width > GetMaxWidth())
        BlockWidth = GetMaxWidth();
    else if (pi_Width < GetMinWidth())
        BlockWidth = GetMinWidth();

    return BlockWidth;
    }

//-----------------------------------------------------------------------------
// HRFImageCapability::ValidateHeight
// Validate the specified value
// return the same value if valid
// otherwise return the near value
//-----------------------------------------------------------------------------
uint32_t HRFImageCapability::ValidateHeight(uint32_t pi_Height)
    {
    // Calc the block size near of pi_Width
    uint32_t BlockHeight = pi_Height;

    // Test if the block is include inside the range
    if (pi_Height > GetMaxHeight())
        BlockHeight = GetMaxHeight();
    else if (pi_Height < GetMinHeight())
        BlockHeight = GetMinHeight();

    return BlockHeight;
    }


//-----------------------------------------------------------------------------
// HRFFilterCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFFilterCapability::HRFFilterCapability(HFCAccessMode   pi_AccessMode,
                                         HCLASS_ID     pi_Filter)
    : HRFCapability(pi_AccessMode)
    {
    m_Filter = pi_Filter;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFFilterCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_Filter == ((HFCPtr<HRFFilterCapability>&)pi_rpCapability)->GetFilter()));
    }

//-----------------------------------------------------------------------------
// HRFFilterCapability::GetFilter()
//-----------------------------------------------------------------------------
HCLASS_ID HRFFilterCapability::GetFilter() const
    {
    return m_Filter;
    }

//-----------------------------------------------------------------------------
// HRFClipShapeCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFClipShapeCapability::HRFClipShapeCapability(HFCAccessMode     pi_AccessMode,
                                               HRFCoordinateType pi_CoordinateType)
    : HRFCapability(pi_AccessMode)
    {
    m_CoordinateType = pi_CoordinateType;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFClipShapeCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_CoordinateType == ((HFCPtr<HRFClipShapeCapability>&)pi_rpCapability)->GetCoordinateType()));
    }

//-----------------------------------------------------------------------------
// HRFClipShapeCapability:GetCoordinateType
//-----------------------------------------------------------------------------
HRFCoordinateType HRFClipShapeCapability::GetCoordinateType() const
    {
    return m_CoordinateType;
    }

//-----------------------------------------------------------------------------
// HRFTransfoModelCapability
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFTransfoModelCapability::HRFTransfoModelCapability(HFCAccessMode  pi_AccessMode,
                                                     HCLASS_ID    pi_TransfoModel)
    : HRFCapability(pi_AccessMode)
    {
    m_TransfoModel = pi_TransfoModel;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFTransfoModelCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_TransfoModel == ((HFCPtr<HRFTransfoModelCapability>&)pi_rpCapability)->GetTransfoModelClassKey()));
    }

//-----------------------------------------------------------------------------
// HRFTransfoModelCapability::GetTransfoModel()
//-----------------------------------------------------------------------------
HCLASS_ID HRFTransfoModelCapability::GetTransfoModelClassKey() const
    {
    return m_TransfoModel;
    }

//-----------------------------------------------------------------------------
// HRFHistogramCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFHistogramCapability::HRFHistogramCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
//-----------------------------------------------------------------------------
HRFThumbnailCapability::HRFThumbnailCapability( HFCAccessMode    pi_AccessMode,
                                                uint32_t         pi_MinWidth,
                                                uint32_t         pi_MaxWidth,
                                                uint32_t         pi_WidthIncrement,

                                                uint32_t         pi_MinHeight,
                                                uint32_t         pi_MaxHeight,
                                                uint32_t         pi_HeightIncrement,

                                                uint32_t         pi_MaxSizeInBytes,
                                                bool            pi_IsComposed)

    : HRFCapability(pi_AccessMode),

      m_MinWidth(pi_MinWidth),
      m_MaxWidth(pi_MaxWidth),
      m_WidthIncrement(pi_WidthIncrement),

      m_MinHeight(pi_MinHeight),
      m_MaxHeight(pi_MaxHeight),
      m_HeightIncrement(pi_HeightIncrement),

      m_MaxSizeInBytes(pi_MaxSizeInBytes),
      m_IsComposed(pi_IsComposed)
    {
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Add the a pixel type to the supported pixel type list.
//-----------------------------------------------------------------------------
void HRFThumbnailCapability::AddPixelType(HCLASS_ID pi_PixelType)
    {
    m_ListOfPixelType.push_back(pi_PixelType);
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the number of pixel type supported by the thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::CountPixelType() const
    {
    return (uint32_t)m_ListOfPixelType.size();
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the class key of the specified pixel type.
//-----------------------------------------------------------------------------
HCLASS_ID HRFThumbnailCapability::GetPixelTypeClassID(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_ListOfPixelType.size());

    return m_ListOfPixelType[pi_Index];
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the minimum width of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetMinWidth() const
    {
    return m_MinWidth;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the maximum width of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetMaxWidth() const
    {
    return m_MaxWidth;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the width increment of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetWidthIncrement() const
    {
    return m_WidthIncrement;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the minimum Height of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetMinHeight() const
    {
    return m_MinHeight;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the maximum Height of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetMaxHeight() const
    {
    return m_MaxHeight;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the Height increment of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetHeightIncrement() const
    {
    return m_HeightIncrement;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the maximum size in byte of the Thumbnail.
//-----------------------------------------------------------------------------
uint32_t HRFThumbnailCapability::GetMaxSizeInBytes() const
    {
    return m_MaxSizeInBytes;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailCapability
// Return the Thumbnail is composed.
//-----------------------------------------------------------------------------
bool HRFThumbnailCapability::IsComposed() const
    {
    return m_IsComposed;
    }

//-----------------------------------------------------------------------------
// HRFMultiPageCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFMultiPageCapability::HRFMultiPageCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFTagCapability
//-----------------------------------------------------------------------------


/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFTagCapability::HRFTagCapability(
    HFCAccessMode                       pi_AccessMode,
    const HFCPtr<HPMGenericAttribute>&  pi_rpTag)
    : HRFCapability(pi_AccessMode)
    {
    m_Tag = pi_rpTag;
    }

/** -----------------------------------------------------------------------------
    Equality compare operator. This method indicates if self is similar to the given
    capability. This is if the data containt by the two capabilities are the same at
    the first level. As example, this method will return true if we compare two pixel type
    capabilities of type I8R8G8B8 without checking if the codec and block type are the
    same. In addition, the access mode of self must be included in the access mode of the
    given capability.

    @param pi_rpCapability   Constant reference to a smart pointer making reference to the
                             capability to compare with.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    @see HFCAccessMode
    @see HRFRasterFileCapabilities::GetCapabilityOfType(const HFCPtr<HRFCapability>&) const
    -----------------------------------------------------------------------------
 */
bool HRFTagCapability::SameAs(const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    return ((HRFCapability::SameAs(pi_rpCapability)) &&
            (m_Tag->GetID() == ((HFCPtr<HRFTagCapability>&)pi_rpCapability)->GetTag()->GetID()));
    }

//-----------------------------------------------------------------------------
// HRFTagCapability::GetTag()
//-----------------------------------------------------------------------------
const HFCPtr<HPMGenericAttribute>& HRFTagCapability::GetTag() const
    {
    return m_Tag;
    }

//-----------------------------------------------------------------------------
// HRFUniversalTagCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFUniversalTagCapability::HRFUniversalTagCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFBlocksDataFlagCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFBlocksDataFlagCapability::HRFBlocksDataFlagCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFSubSamplingCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFSubSamplingCapability::HRFSubSamplingCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFEmbedingCapability
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFEmbedingCapability::HRFEmbedingCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFStillImageCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFStillImageCapability::HRFStillImageCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

//-----------------------------------------------------------------------------
// HRFAnimationCapability
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This is the constructors for the abstract class.

    @param pi_AccessMode  The access mode of the capability indicating if this
                          capability is applicable to reading a raster file,
                          writing or creating a file.
                          Valide value are : A combinasion of READ/WRITE/CREATE
    -----------------------------------------------------------------------------
 */
HRFAnimationCapability::HRFAnimationCapability(HFCAccessMode   pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }
//-----------------------------------------------------------------------------
// HRFMaximumFileSizeCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
This is the constructors for the class.

@param pi_AccessMode  The access mode of the capability indicating if this
                      capability is applicable to reading a raster file,
                      writing or creating a file.

@param pi_MaxFileSize The file's maximum size in bytes.

Valid values are : A combinaison of READ/WRITE/CREATE
-----------------------------------------------------------------------------
*/

HRFMaxFileSizeCapability::HRFMaxFileSizeCapability(HFCAccessMode pi_AccessMode,
                                                   uint64_t       pi_MaxFileSize)
    : HRFCapability(pi_AccessMode)
    {
    HPRECONDITION(pi_MaxFileSize > 0);
    m_MaxFileSize = pi_MaxFileSize;
    }

/** -----------------------------------------------------------------------------
This method returns the maximum file size supported.

@return The file's maximum size.
-----------------------------------------------------------------------------
*/
uint64_t HRFMaxFileSizeCapability::GetMaxFileSize() const
    {
    return m_MaxFileSize;
    }

/** -----------------------------------------------------------------------------
This method sets the maximum file size supported.

@param pi_MaxFileSize The file's maximum size in bytes.
-----------------------------------------------------------------------------
*/
void HRFMaxFileSizeCapability::SetMaxFileSize(uint64_t pi_MaxFileSize)
    {
    HPRECONDITION(pi_MaxFileSize > 0);
    m_MaxFileSize = pi_MaxFileSize;
    }

//-----------------------------------------------------------------------------
// HRFGeocodingCapability
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
This is the constructors for the class.

@param pi_AccessMode  The access mode of the capability indicating if this
capability is applicable to reading a raster file,
writing or creating a file.

Valid values are : A combinison of READ/WRITE/CREATE
-----------------------------------------------------------------------------
*/
HRFGeocodingCapability::HRFGeocodingCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }

/** -----------------------------------------------------------------------------
This method adds a supported geocoding key to the list of geocoding keys
supported by the file format supporting this capability.

@param pi_GeoKey The geocoding key to add
-----------------------------------------------------------------------------
*/
void HRFGeocodingCapability::AddSupportedKey(TIFFGeoKey pi_GeoKey)
    {
    HPRECONDITION(IsKeySupported(pi_GeoKey) == false);

    m_SupportedGeoKeys.push_back(pi_GeoKey);
    }

/** -----------------------------------------------------------------------------
This method returns true if the given geocoding key is supported, false
otherwise.

@param pi_GeoKey The geocoding key to test
-----------------------------------------------------------------------------
*/
bool HRFGeocodingCapability::IsKeySupported(TIFFGeoKey pi_GeoKey) const
    {
    TIFFGeoKeyVector::const_iterator KeyIter = m_SupportedGeoKeys.begin();
    TIFFGeoKeyVector::const_iterator KeyIterEnd = m_SupportedGeoKeys.end();

    while (KeyIter != KeyIterEnd)
        {
        if (*KeyIter == pi_GeoKey)
            {
            break;
            }
        KeyIter++;
        }

    return KeyIter != KeyIterEnd;
    }

/** -----------------------------------------------------------------------------
This method returns true if the given geocoding key is supported, false
otherwise.

@param pi_GeoKey The geocoding key to test
-----------------------------------------------------------------------------
*/
unsigned short HRFGeocodingCapability::GetNbGeotiffKeys() const
    {
    HPRECONDITION(m_SupportedGeoKeys.size() <= USHRT_MAX);

    return (unsigned short)m_SupportedGeoKeys.size();
    }

/** -----------------------------------------------------------------------------
This method return the geotiff key at the given index


@param pi_KeyIndex The index of the requested geocoding key
-----------------------------------------------------------------------------
*/
TIFFGeoKey HRFGeocodingCapability::GetGeotiffKey(unsigned short pi_KeyIndex) const
    {
    HPRECONDITION(pi_KeyIndex < m_SupportedGeoKeys.size());

    return m_SupportedGeoKeys[pi_KeyIndex];
    }



//-----------------------------------------------------------------------------
// HRFResizableCapability
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRFResizableCapability::HRFResizableCapability(HFCAccessMode pi_AccessMode)
    : HRFCapability(pi_AccessMode)
    {
    }
