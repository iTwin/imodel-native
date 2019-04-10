//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFUtils.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTIFFUtils
//-----------------------------------------------------------------------------

#pragma once

#include "HFCAccessMode.h"
#include "HFCException.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCBinStream;
class HFCURL;


// Macros
//
// Compute how many times is Y in X
#define    HowMany(x, y) ((((uint32_t)(x))+(((uint32_t)(y))-1))/((uint32_t)(y)))

#define SWAPBYTE_SHORT(aShortValue) ( (((aShortValue) << 8) & 0xFF00) | (((aShortValue) >> 8) & 0x00FF) )


typedef    struct {
    uint64_t Offset;
    uint64_t Size;
    uint32_t Dir;
    uint32_t Position;
    bool    isATag;
    bool    isHMRDir;
    } OffsetAndSize;

typedef    struct {
    uint64_t Offset;
    uint64_t Size;
    uint32_t Dir;
    } FreeblockData;


typedef map<uint64_t, FreeblockData> FreeBlockDataMap;
typedef map<uint64_t, OffsetAndSize> MergedDirMap;
class HTIFFByteOrdering
    {
public:
    HTIFFByteOrdering();
    virtual ~HTIFFByteOrdering();
    HTIFFByteOrdering (HTIFFByteOrdering& pi_rObject);


    void    SetStoredAsBigEndian (bool pi_BigEndian);
    bool   IsStoredAsBigEndian () const
        {
        return m_FileBigEndian;
        }

    bool   IsBigEndianSystem () const
        {
        return m_SystemBigEndian;
        }

    bool   NeedSwapByte        () const
        {
        return m_NeedSwapByte;
        }

    bool   NeedBitRev          () const
        {
        return m_NeedBitRev;
        }
    void    SetBitRev           (bool pi_Rev)
        {
        m_NeedBitRev = pi_Rev;
        }
    void    ReverseBits(Byte* pio_pBuffer, size_t pi_Len);


private:
    static const Byte BitRevTable[256];

    bool       m_NeedBitRev;         // Need BitRev ?
    bool       m_NeedSwapByte;       // Swap byte ?
    bool       m_FileBigEndian;      // Stored as big endian
    bool       m_SystemBigEndian;    // Compute at execution time
    };



// Class HTIFFError
//
class HTIFFError
    {
public:

#define IMPLEMENT_CLONE_FNC(pi_StructName) \
    virtual ErInfo* Clone() const \
    { \
        ErInfo* pClone = new pi_StructName; \
        *pClone = *this; \
        return pClone; \
    }

    struct ErInfo
        {
        virtual ~ErInfo() {}

        virtual ErInfo& operator=(const ErInfo&) = 0;
        virtual ErInfo* Clone() const = 0;
        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const = 0;
        };

    //Detailed error information
    struct NegativeValueForRationalErInfo : ErInfo
        {
        double m_RationalValue;
        Utf8String m_TagName;

        virtual ~NegativeValueForRationalErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_RationalValue = ((NegativeValueForRationalErInfo const*)&pi_rObj)->m_RationalValue;
            m_TagName      = ((NegativeValueForRationalErInfo const*)&pi_rObj)->m_TagName;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_RationalValue, m_TagName.c_str());
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(NegativeValueForRationalErInfo)
        };

    struct CannotWritePWblobErInfo : ErInfo
        {
        int64_t m_Offset;
        int64_t m_Length;

        virtual ~CannotWritePWblobErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_Offset = ((CannotWritePWblobErInfo const*)&pi_rObj)->m_Offset;
            m_Length = ((CannotWritePWblobErInfo const*)&pi_rObj)->m_Length;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_Offset, m_Length);
            pio_rUnformattedErrMsg = message; 
            }

        IMPLEMENT_CLONE_FNC(CannotWritePWblobErInfo)
        };

    struct BadMagicNbErInfo : ErInfo
        {
        int32_t m_MagicNb;

        virtual ~BadMagicNbErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_MagicNb = ((BadMagicNbErInfo const*)&pi_rObj)->m_MagicNb;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_MagicNb);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(BadMagicNbErInfo)
        };

    struct BadVersionNbErInfo : ErInfo
        {
        int32_t m_VersionNb;

        virtual ~BadVersionNbErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_VersionNb = ((BadVersionNbErInfo const*)&pi_rObj)->m_VersionNb;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_VersionNb);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(BadVersionNbErInfo)
        };

    struct BadBlockNbErInfo : ErInfo
        {
        int32_t m_BlockNb;

        virtual ~BadBlockNbErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_BlockNb = ((BadBlockNbErInfo const*)&pi_rObj)->m_BlockNb;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_BlockNb);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(BadBlockNbErInfo)
        };

    struct BlockIOErInfo : ErInfo
        {
        int32_t m_BlockNb;
        int64_t m_Offset;
        int32_t m_Length;

        virtual ~BlockIOErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_BlockNb = ((BlockIOErInfo const*)&pi_rObj)->m_BlockNb;
            m_Offset  = ((BlockIOErInfo const*)&pi_rObj)->m_Offset;
            m_Length  = ((BlockIOErInfo const*)&pi_rObj)->m_Length;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_BlockNb, m_Offset, m_Length);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(BadBlockNbErInfo)
        };

    struct UnknownCompressionErInfo : ErInfo
        {
        uint16_t m_CompressionType;

        virtual ~UnknownCompressionErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_CompressionType = ((UnknownCompressionErInfo const*)&pi_rObj)->m_CompressionType;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_CompressionType);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(UnknownCompressionErInfo)
        };

    struct InvalidGeotiffCountOrIndexErInfo : ErInfo
        {
        int32_t m_GeoKey;

        virtual ~InvalidGeotiffCountOrIndexErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_GeoKey = ((InvalidGeotiffCountOrIndexErInfo const*)&pi_rObj)->m_GeoKey;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_GeoKey);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(InvalidGeotiffCountOrIndexErInfo)
        };

    struct InvalidGeotiffTagErInfo : ErInfo
        {
        int32_t m_GeoKey;
        int32_t m_Tag;

        virtual ~InvalidGeotiffTagErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_GeoKey = ((InvalidGeotiffTagErInfo const*)&pi_rObj)->m_GeoKey;
            m_Tag    = ((InvalidGeotiffTagErInfo const*)&pi_rObj)->m_Tag;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_GeoKey, m_Tag);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(InvalidGeotiffTagErInfo)
        };

    struct UnknownTagEnumErInfo : ErInfo
        {
        int32_t m_Tag;

        virtual ~UnknownTagEnumErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_Tag = ((UnknownTagEnumErInfo const*)&pi_rObj)->m_Tag;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_Tag);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(UnknownTagEnumErInfo)
        };

    struct WrongTagDataTypeErInfo : ErInfo
        {
        int16_t m_Type;
        Utf8String m_TagName;
        int32_t m_TagFileNb;

        virtual ~WrongTagDataTypeErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_Type      = ((WrongTagDataTypeErInfo const*)&pi_rObj)->m_Type;
            m_TagName   = ((WrongTagDataTypeErInfo const*)&pi_rObj)->m_TagName;
            m_TagFileNb = ((WrongTagDataTypeErInfo const*)&pi_rObj)->m_TagFileNb;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_TagName.c_str(), m_Type);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(WrongTagDataTypeErInfo)
        };

    struct UnknownTagErInfo : ErInfo
        {
        int32_t m_TagFileNb;
        Utf8String m_Type;
        uint64_t m_Length;
        uint64_t m_Offset;

        virtual ~UnknownTagErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_TagFileNb = ((UnknownTagErInfo const*)&pi_rObj)->m_TagFileNb;
            m_Type      = ((UnknownTagErInfo const*)&pi_rObj)->m_Type;
            m_Length    = ((UnknownTagErInfo const*)&pi_rObj)->m_Length;
            m_Offset    = ((UnknownTagErInfo const*)&pi_rObj)->m_Offset;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_TagFileNb, m_Type.c_str(), m_Length, m_Offset);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(UnknownTagErInfo)
        };

    struct UnkwnownFirstTagErInfo : ErInfo
        {
        uint32_t m_TagFile;

        virtual ~UnkwnownFirstTagErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_TagFile = ((UnkwnownFirstTagErInfo const*)&pi_rObj)->m_TagFile;

            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_TagFile);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(UnkwnownFirstTagErInfo)
        };

    struct TagIOErInfo : ErInfo
        {
        Utf8String m_TagName;
        int32_t m_TagFileNb;
        int32_t m_DataLength;

        virtual ~TagIOErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_DataLength = ((TagIOErInfo const*)&pi_rObj)->m_DataLength;
            m_TagName    = ((TagIOErInfo const*)&pi_rObj)->m_TagName;
            m_TagFileNb  = ((TagIOErInfo const*)&pi_rObj)->m_TagFileNb;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_TagName.c_str(), m_TagFileNb, m_DataLength);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(TagIOErInfo)
        };

    struct BadTagCountIOErInfo : ErInfo
        {
        Utf8String m_TagName;
        int64_t m_Count;
        int16_t m_ExpectedCount;

        virtual ~BadTagCountIOErInfo() {}

        virtual ErInfo& operator=(const ErInfo& pi_rObj)
            {
            m_TagName       = ((BadTagCountIOErInfo const*)&pi_rObj)->m_TagName;
            m_Count         = ((BadTagCountIOErInfo const*)&pi_rObj)->m_Count;
            m_ExpectedCount = ((BadTagCountIOErInfo const*)&pi_rObj)->m_ExpectedCount;
            return *((ErInfo*)this);
            }

        virtual void FormatErrorMessage(Utf8String& pio_rUnformattedErrMsg) const
            {
            Utf8PrintfString message(pio_rUnformattedErrMsg.c_str(), m_TagName.c_str(), m_Count, m_ExpectedCount);
            pio_rUnformattedErrMsg = message;
            }

        IMPLEMENT_CLONE_FNC(BadTagCountIOErInfo)
        };

    enum ErrorCode
        {
        UNKNOWN_ERROR = 0,
        CANNOT_READ_DIR_COUNT,
        CANNOT_WRITE_DIR_BUFFER,
        CANNOT_WRITE_DIR_COUNT,
        CANNOT_WRITE_DIR_NEXT_DIR_OFFSET,
        CANNOT_WRITE_DIR_LINK,
        NEGATIVE_VALUE_FOR_RATIONAL,
        HMR_DIR_ALREADY_EXIST_IN_PAGE,
        CANNOT_WRITE_PW_BLOB,
        CANNOT_OPEN_FILE,
        CANNOT_READ_HEADER,
        BAD_MAGIC_NUMBER,
        BAD_FILE_VERSION,
        CANNOT_CREATE_FILE,
        CANNOT_WRITE_HEADER,
        ERROR_FOUND_IN_DIRECTORY,
        IMAGE_DEPTH_DIFFERENT_THAN_1_NOT_SUPPORTED,
        IMAGE_WIDTH_AND_LENGTH_ARE_REQUIRED,
        MISSING_STRIP_OR_TILE_OFFSET,
        MISSING_IMAGE_PLANAR_CONFIGURATION,
        MISSING_COLOR_MAP,
        BAD_BLOCK_NUMBER,
        CANNOT_WRITE_SEPARATE_PLANE_ON_SEPARATE_CFG,
        CANNOT_WRITE_BLOCK,
        CANNOT_READ_SEPARATE_PLANE_ON_SEPARATE_CFG,
        CANNOT_READ_BLOCK,
        UNKNOWN_COMPRESSION_TYPE,
        SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED,
        UNSUPPORTED_GEOTIFF_VERSION,
        INVALID_GEOTIFF_COUNT,
        INVALID_GEOTIFF_INDEX_OR_COUNT,
        INVALID_GEOTIFF_TAG,
        UNKNOWN_TAG_ENUM,
        WRONG_DATA_TYPE_FOR_TAG,
        UNKNOWN_TAG,
        UNKNOWN_FIRST_TAG,
        DIRECTORY_ENTRY_READ_ERROR,
        TAG_READ_ERROR,
        DIRECTORY_ENTRY_WRITE_ERROR,
        TAG_WRITE_ERROR,
        INCORRECT_COUNT_FOR_TAG_READ,
        INCORRECT_COUNT_FOR_TAG_WRITTEN,
        SIZE_OUT_OF_RANGE,
        //Should be the last tag
        ERROR_COUNT
        };

    IMAGEPP_EXPORT HTIFFError(bool pi_Fatal=false);

    HTIFFError(HTIFFError::ErrorCode pi_ErrorCode,
               const ErInfo*         pi_pErrorInfo,
               bool                 pi_Fatal=false);
    IMAGEPP_EXPORT virtual     ~HTIFFError();

    HTIFFError(const HTIFFError& pi_rObj);
    IMAGEPP_EXPORT HTIFFError& operator=(const HTIFFError& pi_rObj);

    void            AddError (ErrorCode pi_ErrorCode, const HTIFFError::ErInfo* pi_pErrorInfo, bool pi_Fatal=false);
    //void            AddMsg (const char* pi_pMsg, bool pi_Fatal=false);
    // const char*    GetMsg ()   { return m_pMsg;}

    const ErrorCode GetErrorCode ()   {
        return m_ErrorCode;
        }
    const ErInfo*   GetErrorInfo ()   {
        return m_pErrorInfo;
        }
    IMAGEPP_EXPORT void     GetErrorMsg(Utf8String& po_rErrorMsg) const;
    bool           IsFatal()   {
        return m_Fatal;
        }

private:

    void CopyData(const HTIFFError& pi_rSrcObj);

    bool            m_Fatal;
    ErInfo*          m_pErrorInfo;
    ErrorCode        m_ErrorCode;


    };


IMAGEPP_EXPORT bool       ErrorMsg (HTIFFError** pio_pError, HTIFFError::ErrorCode pi_ErrorCode, const HTIFFError::ErInfo* pi_pErrorInfo, bool pi_Fatal);
//bool       ErrorMsg (HTIFFError** pio_pError, const char* pi_pMsg, bool pi_Fatal);
bool       ErrorMsg (HTIFFError** pio_pError, HTIFFError& pi_rNewError);



// Class HTIFFStream
//

struct HTIFFStreamFreeBlock
    {
    uint64_t Offset;
    uint64_t Size;

    uint64_t GetStartOffset () const
        {
        return Offset;
        }

    uint64_t GetEndOffset () const
        {
        return Offset + Size;
        }

    bool FollowsImmediatly (const HTIFFStreamFreeBlock& pi_rRight) const
        {
        return pi_rRight.GetEndOffset() == GetStartOffset();
        }

    bool PrecedesImmediatly (const HTIFFStreamFreeBlock& pi_rRight) const
        {
        return GetEndOffset() == pi_rRight.GetStartOffset();
        }

    bool operator==(const HTIFFStreamFreeBlock& pi_rObj) const
        {
        return (Offset == pi_rObj.Offset);
        }

    bool operator!=(const HTIFFStreamFreeBlock& pi_rObj) const
        {
        return (Offset != pi_rObj.Offset);
        }

    bool operator<(const HTIFFStreamFreeBlock& pi_rObj) const
        {
        return (Offset < pi_rObj.Offset);
        }

    bool operator>(const HTIFFStreamFreeBlock& pi_rObj) const
        {
        return (Offset > pi_rObj.Offset);
        }
    };

class HTIFFStream
    {
public:
    HTIFFStream(const Utf8String& pi_rFilename, HFCAccessMode pi_Mode, uint64_t pi_OriginOffset=0);
    HTIFFStream(const HFCPtr<HFCURL>& pi_rpURL, HFCAccessMode pi_AccessMode = HFC_READ_ONLY, uint64_t pi_OriginOffset=0);
    virtual ~HTIFFStream();


    HFCPtr<HFCURL>      GetURL() const;

    IMAGEPP_EXPORT size_t              Read        (void* po_pBuffer, size_t pi_Size, size_t pi_Count);
    IMAGEPP_EXPORT size_t              Write       (const void* pi_pBuffer, size_t pi_Size, size_t pi_Count);
    IMAGEPP_EXPORT bool               Seek        (uint64_t pi_Offset, int32_t pi_Origin);
    uint64_t           Tell        ();
    bool               SetEOF      ();

    uint64_t           GetSize     ();
    HFCAccessMode       GetMode     () const;

    void                AddInitialFreeBlock (const uint64_t* pi_pOffset, const uint64_t* pi_pSize, uint32_t pi_Count);
    void                AddInitialFreeBlock (const uint32_t* pi_pOffset, const uint32_t* pi_pSize, uint32_t pi_Count);

    bool               GetListFreeBlock    (uint64_t** pi_ppOffset, uint64_t** pi_ppSize, uint32_t* pi_pCount) const;
    IMAGEPP_EXPORT void                CheckAlloc          (uint64_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize);
    IMAGEPP_EXPORT void                ReleaseBlock        (uint64_t pi_Offset, uint32_t pi_Size);
    void                GetBlock            (uint64_t* po_pOffset, uint32_t pi_Size);

    bool               OverlapsFreeBlocks  (uint64_t pi_Offset, uint64_t pi_Size) const;

    IMAGEPP_EXPORT HFCBinStream* GetFilePtr          () const;

    // Use only to allocate memory in file without memory management
    // Presently internally used to write TAGS FREEOFFSETS and FREEBYTECOUNTS
    void    CheckAllocWithoutUpdate (uint64_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize);


    bool               m_IsTiff64;     // BifTiff == Tiff > 4Gig support

private:

    typedef list<HTIFFStreamFreeBlock, allocator<HTIFFStreamFreeBlock> > ListOfFreeBlock;

    // Members
    HAutoPtr<HFCBinStream>  m_pStream;

    // The List is sorted by Offset;
    ListOfFreeBlock         m_ListOfFreeBlock;
    bool                   m_ListDirty;

    // Methods
    // Not implemented
    HTIFFStream (HTIFFError& pi_rObject);
    HTIFFStream&      operator=(const HTIFFStream& pi_rObj);
    };

// Swap Functions
//
void    SwabArrayOfShort    (uint16_t* pio_pData, size_t pi_Count);
void    SwabArrayOfLong     (uint32_t* pio_pData, size_t pi_Count);
void    SwabArrayOfDouble   (double* pio_pData, size_t pi_Count);
void    SwabArrayOfUInt64   (uint64_t* pio_pData, size_t pi_Count);

bool   SystemIsBigEndian   ();
bool   IsValidDoubleArray  (const double* pi_pValues, int32_t pi_ArraySize);
END_IMAGEPP_NAMESPACE

