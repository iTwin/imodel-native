//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFDirectory.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFDirectory
//-----------------------------------------------------------------------------

#pragma once

#include "HTIFFUtils.h"
#include "HTIFFTagEntry.h"

BEGIN_IMAGEPP_NAMESPACE
class HTIFFGeoKey;
class HTagIdIter;

class HTIFFDirectory
    {
public:

    // Primary methods

    HTIFFDirectory(const HTagInfo& pi_rTagInfo, const HTIFFByteOrdering* pi_pByteOrder, bool pi_IsTiff64);
    virtual             ~HTIFFDirectory();


    bool               IsValid         (HTIFFError**  po_ppError) const;

    uint64_t           ReadDirectory   (HTIFFStream* pi_pFile, uint64_t pi_Offset);
    uint64_t           WriteDirectory  (HTIFFStream* pi_pFile, uint64_t* pio_pOffset, uint64_t pi_OffsetPreviousDirLink,
                                         uint64_t    pi_NextDirIfKnown=0);

    uint64_t           GetDirectorySize(HTIFFStream* pi_pFile, uint64_t pi_Offset);

    bool               TagIsPresent    (HTagID pi_Tag) const;
    const char*        GetTagNameString(HTagID pi_Tag) const;
    HTagInfo::DataType  GetTagDataType  (HTagID pi_Tag) const;

    bool               NextDirectoryOffsetIsInvalid();
    void                SetNextDirectoryOffsetIsInvalid(bool pi_InvalidDir);
    void                SetDirty();

    bool               Touched   (HTagID pi_Tag);
    IMAGEPP_EXPORT bool               Remove    (HTagID pi_Tag);

    IMAGEPP_EXPORT HTagIdIter          TagIDBegin () const;
    IMAGEPP_EXPORT HTagIdIter          TagIDEnd () const;

    void                ValidateFreeBlockTags(HTIFFStream* pi_pFile);

    HTIFFGeoKey&        GetGeoKeyInterpretation();

    void                ConvertRationalToDblValues (uint32_t    pi_Count,
                                                    double*    pi_RationalVals,
                                                    double*    po_pDblVal,
                                                    bool       pi_IsSigned = false);

    bool               GetConvertedValues (HTagID pi_Tag, vector<double>& po_rValues);

    bool               GetValues (HTagID pi_Tag, Byte* po_pVal);
    IMAGEPP_EXPORT bool        GetValues (HTagID pi_Tag, unsigned short* po_pVal);
    IMAGEPP_EXPORT bool        GetValues (HTagID pi_Tag, uint32_t* po_pVal);
    bool               GetValues (HTagID pi_Tag, double* po_pVal);
    IMAGEPP_EXPORT bool               GetValues (HTagID pi_Tag, uint64_t* po_pVal);
    bool               GetValues (HTagID pi_Tag, char** po_ppVal);
    IMAGEPP_EXPORT bool               GetValues (HTagID pi_Tag, WChar** po_ppVal);
    bool               GetValues (HTagID pi_Tag, unsigned short* po_pVal1, unsigned short* po_pVal2);
    bool               GetValues (HTagID pi_Tag, uint32_t* po_pVal1, uint32_t* po_pVal2);     // RATIONAL
    bool               GetValues (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal);
    IMAGEPP_EXPORT bool               GetValues (HTagID pi_Tag, uint32_t* po_pCount, uint32_t** po_ppVal);
    bool               GetValues (HTagID pi_Tag, uint32_t* po_pCount, double** po_ppVal);
    IMAGEPP_EXPORT bool        GetValues (HTagID pi_Tag, uint32_t* po_pCount, Byte** po_ppVal);
    IMAGEPP_EXPORT bool               GetValues (HTagID pi_Tag, uint32_t* po_pCount, uint64_t** po_ppVal);


    bool               SetValues (HTagID pi_Tag, unsigned short pi_Val);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint32_t pi_Val);
    bool               SetValues (HTagID pi_Tag, double pi_Val);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint64_t pi_Val);
    IMAGEPP_EXPORT bool      SetValuesA (HTagID pi_Tag, const char*  pi_pVal);
    IMAGEPP_EXPORT bool      SetValuesW (HTagID pi_Tag, const WChar*  pi_pVal);
    bool               SetValues (HTagID pi_Tag, unsigned short pi_Val1,  unsigned short pi_Val2);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint32_t pi_pVal1, uint32_t pi_pVal2);     // RATIONAL
    bool               SetValues (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint32_t pi_Count, const uint32_t* pi_pVal);
    bool               SetValues (HTagID pi_Tag, uint32_t pi_Count, const double* pi_pVal);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint32_t pi_Count, const Byte* pi_pVal);
    IMAGEPP_EXPORT bool               SetValues (HTagID pi_Tag, uint32_t pi_Count, const uint64_t* pi_pVal);


    //Added for compact iTIFF function
    void                GetListTagPerDirectory(HTIFFStream*                pi_pFile,
                                               vector<OffsetAndSize>&      p_TagList,
                                               bool                       isHMRDir,
                                               uint32_t                    p_NoDir);

    void                SetEntryOffset        (uint32_t p_PositionTag,
                                               uint64_t p_NewOffset);
    void                GetFreeBlockInfo      (uint64_t& p_SizeFreeOffset,
                                               uint64_t& p_OffsetFreeOffset,
                                               uint64_t& p_SizeFreebytecounts,
                                               uint64_t& p_OffsetFreebytecounts);
    //------
protected:

    // Members

    HTIFFByteOrdering*  m_pByteOrder;   // Share with HTIFFDirectory
    bool               m_IsTiff64;     // Is it a bigtiff ?

private:

    struct DirStatus
        {
        unsigned Dirty  : 1;        // At least one Tag is modified
        unsigned Resize : 1;        // The size of the Directory has been changed
        };

    struct TagInDir
        {
        uint64_t OffSet;        // At least one Tag is modified
        uint64_t Size;        // The size of the Directory has been changed
        };
    // Members

    DirStatus           m_Status;

    const HTagInfo&     m_rTagInfo;
    HTIFFTagEntry**     m_ppDirEntry;
    uint32_t            m_DirCount;

    // GeoTiff
    HTIFFGeoKey*        m_pGeoKeys;

    HTIFFError*         m_pError;           // Error message

    bool               m_NextDirectoryOffsetIsInvalid;


    // Methods

    // Not implemented
    HTIFFDirectory   (const HTIFFDirectory& pi_rObj);
    HTIFFDirectory&          operator=(const HTIFFDirectory& pi_rObj);

    void                    Reset               ();
    bool                   AddEntry            (HTagID pi_Tag);
    void                    ConvertSHORTtoULONG (HTagID pi_Tag);
    void                    ConvertToLONG64 (HTagID pi_Tag);


    };
END_IMAGEPP_NAMESPACE

#include "HTIFFDirectory.hpp"

