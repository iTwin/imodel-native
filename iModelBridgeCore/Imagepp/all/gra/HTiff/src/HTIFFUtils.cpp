//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFUtils.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HTIFFUtils
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

// Class HTIFFByteOrdering
//
const Byte HTIFFByteOrdering::BitRevTable[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
    };

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HTIFFByteOrdering::HTIFFByteOrdering()
    {
    // Determinate if the system is Big or little Endian
    m_SystemBigEndian = SystemIsBigEndian();

    m_NeedSwapByte = false;
    m_NeedBitRev   = false;

    // By default Set the EndianFile value with the System
    //  --> write the file with the System value
    m_FileBigEndian = m_SystemBigEndian;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HTIFFByteOrdering::HTIFFByteOrdering (HTIFFByteOrdering& pi_rObject)
    {
    m_NeedSwapByte    = pi_rObject.m_NeedSwapByte;
    m_FileBigEndian   = pi_rObject.m_FileBigEndian;
    m_SystemBigEndian = pi_rObject.m_SystemBigEndian;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HTIFFByteOrdering::~HTIFFByteOrdering()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HTIFFByteOrdering::SetStoredAsBigEndian (bool pi_BigEndian)
    {
    m_FileBigEndian = pi_BigEndian;
    // If the System and the File are Different Byte Ordering, need swap
    //
    m_NeedSwapByte = false;
    if (m_FileBigEndian)
        {
        if (!m_SystemBigEndian)
            m_NeedSwapByte = true;
        }
    else
        {
        if (m_SystemBigEndian)
            m_NeedSwapByte = true;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HTIFFByteOrdering::ReverseBits(Byte* pio_pBuffer, size_t pi_Len)
    {
    for (; pi_Len>8; pi_Len-=8)
        {
        pio_pBuffer[0] = BitRevTable[pio_pBuffer[0]];
        pio_pBuffer[1] = BitRevTable[pio_pBuffer[1]];
        pio_pBuffer[2] = BitRevTable[pio_pBuffer[2]];
        pio_pBuffer[3] = BitRevTable[pio_pBuffer[3]];
        pio_pBuffer[4] = BitRevTable[pio_pBuffer[4]];
        pio_pBuffer[5] = BitRevTable[pio_pBuffer[5]];
        pio_pBuffer[6] = BitRevTable[pio_pBuffer[6]];
        pio_pBuffer[7] = BitRevTable[pio_pBuffer[7]];
        pio_pBuffer += 8;
        }
    while (pi_Len-- > 0)
        *pio_pBuffer = BitRevTable[*pio_pBuffer], pio_pBuffer++;
    }

//-----------------------------------------------------------------------------
// Class HTIFFError
//-----------------------------------------------------------------------------
HTIFFError::HTIFFError(bool pi_Fatal)
    {
    m_Fatal = pi_Fatal;
    m_ErrorCode = HTIFFError::UNKNOWN_ERROR;
    m_pErrorInfo = 0;
    }



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HTIFFError::HTIFFError(HTIFFError::ErrorCode      pi_ErrorCode,
                       const HTIFFError::ErInfo*  pi_pErrorInfo,
                       bool                      pi_Fatal)
    {
    m_Fatal     = pi_Fatal;
    m_ErrorCode = pi_ErrorCode;

    if (pi_pErrorInfo != 0)
        {
        m_pErrorInfo = pi_pErrorInfo->Clone();
        }
    else
        {
        m_pErrorInfo = 0;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HTIFFError::~HTIFFError()
    {
    if (m_pErrorInfo != 0)
        {
        delete m_pErrorInfo;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HTIFFError::HTIFFError(const HTIFFError& pi_rObj)
    {
    CopyData(pi_rObj);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HTIFFError& HTIFFError::operator=(const HTIFFError& pi_rObj)
    {
    CopyData(pi_rObj);

    return *this;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HTIFFError::AddError (HTIFFError::ErrorCode     pi_ErrorCode,
                           const HTIFFError::ErInfo* pi_pErrorInfo,
                           bool                     pi_Fatal)
    {
    m_Fatal      = pi_Fatal;
    m_ErrorCode  = pi_ErrorCode;

    if (m_pErrorInfo != 0)
        {
        delete m_pErrorInfo;
        }

    if (pi_pErrorInfo != 0)
        {
        m_pErrorInfo = pi_pErrorInfo->Clone();
        }
    else
        {
        m_pErrorInfo = 0;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HTIFFError::GetErrorMsg(WString& po_rErrorMsg) const
    {
    HSTATICASSERTMSG(HTIFFError::ERROR_COUNT == 43, "Missing HTIFF error message.");

    ImagePPMessages::StringId messageID = ImagePPMessages::HTIFF_UnknownError();

    switch(m_ErrorCode)
        {
        case UNKNOWN_ERROR                              : messageID = ImagePPMessages::HTIFF_UnknownError()                   ; break;
        case CANNOT_READ_DIR_COUNT                      : messageID = ImagePPMessages::HTIFF_CannotReadDirCount()             ; break;
        case CANNOT_WRITE_DIR_BUFFER                    : messageID = ImagePPMessages::HTIFF_CannotWriteDirBuffer()           ; break;
        case CANNOT_WRITE_DIR_COUNT                     : messageID = ImagePPMessages::HTIFF_CannotWriteDirCount()            ; break;
        case CANNOT_WRITE_DIR_NEXT_DIR_OFFSET           : messageID = ImagePPMessages::HTIFF_CannotWriteDirOffset()           ; break;
        case CANNOT_WRITE_DIR_LINK                      : messageID = ImagePPMessages::HTIFF_CannotWriteDirLink()             ; break;
        case NEGATIVE_VALUE_FOR_RATIONAL                : messageID = ImagePPMessages::HTIFF_NegativeValueForRational()       ; break;
        case HMR_DIR_ALREADY_EXIST_IN_PAGE              : messageID = ImagePPMessages::HTIFF_HMRDirAlreadyExist()             ; break;
        case CANNOT_WRITE_PW_BLOB                       : messageID = ImagePPMessages::HTIFF_CannotWritePWBlob()              ; break;
        case CANNOT_OPEN_FILE                           : messageID = ImagePPMessages::HTIFF_CannotOpenFile()                 ; break;
        case CANNOT_READ_HEADER                         : messageID = ImagePPMessages::HTIFF_CannotReadHeader()               ; break;
        case BAD_MAGIC_NUMBER                           : messageID = ImagePPMessages::HTIFF_BadMagicNumber()                 ; break;
        case BAD_FILE_VERSION                           : messageID = ImagePPMessages::HTIFF_BadFileVersion()                 ; break;
        case CANNOT_CREATE_FILE                         : messageID = ImagePPMessages::HTIFF_CannotCreateFile()               ; break;
        case CANNOT_WRITE_HEADER                        : messageID = ImagePPMessages::HTIFF_CannotWriteHeader()              ; break;
        case ERROR_FOUND_IN_DIRECTORY                   : messageID = ImagePPMessages::HTIFF_ErrorFoundInDirectory()          ; break;
        case IMAGE_DEPTH_DIFFERENT_THAN_1_NOT_SUPPORTED : messageID = ImagePPMessages::HTIFF_ImageDepthDifferentThan1()       ; break;
        case IMAGE_WIDTH_AND_LENGTH_ARE_REQUIRED        : messageID = ImagePPMessages::HTIFF_ImageWidthAndLengthRequired()    ; break;
        case MISSING_STRIP_OR_TILE_OFFSET               : messageID = ImagePPMessages::HTIFF_MissingStripOrTileOffset()       ; break;
        case MISSING_IMAGE_PLANAR_CONFIGURATION         : messageID = ImagePPMessages::HTIFF_MissingImagePlanarConfig()       ; break;
        case MISSING_COLOR_MAP                          : messageID = ImagePPMessages::HTIFF_MissingColorMap()                ; break;
        case BAD_BLOCK_NUMBER                           : messageID = ImagePPMessages::HTIFF_BadBlockNumber()                 ; break;
        case CANNOT_WRITE_SEPARATE_PLANE_ON_SEPARATE_CFG: messageID = ImagePPMessages::HTIFF_CannotWriteSeparatePlane()       ; break;
        case CANNOT_WRITE_BLOCK                         : messageID = ImagePPMessages::HTIFF_CannotWriteBlock()               ; break;
        case CANNOT_READ_SEPARATE_PLANE_ON_SEPARATE_CFG : messageID = ImagePPMessages::HTIFF_CannotReadSeparatePlane()        ; break;
        case CANNOT_READ_BLOCK                          : messageID = ImagePPMessages::HTIFF_CannotReadBlock()                ; break;
        case UNKNOWN_COMPRESSION_TYPE                   : messageID = ImagePPMessages::HTIFF_UnknownCompressType()            ; break;
        case SEPARATE_PLANAR_CONFIGURATION_NOT_SUPPORTED: messageID = ImagePPMessages::HTIFF_SeparatePlanarConfigNotSuported(); break;
        case UNSUPPORTED_GEOTIFF_VERSION                : messageID = ImagePPMessages::HTIFF_UnsupportedGeotiffVersion()      ; break;
        case INVALID_GEOTIFF_COUNT                      : messageID = ImagePPMessages::HTIFF_InvalidGeotiffCount()            ; break;
        case INVALID_GEOTIFF_INDEX_OR_COUNT             : messageID = ImagePPMessages::HTIFF_InvalidGeotiffIndex()            ; break;
        case INVALID_GEOTIFF_TAG                        : messageID = ImagePPMessages::HTIFF_InvalidGeotiffTag()              ; break;
        case UNKNOWN_TAG_ENUM                           : messageID = ImagePPMessages::HTIFF_UnknownTagEnumeration()          ; break;
        case WRONG_DATA_TYPE_FOR_TAG                    : messageID = ImagePPMessages::HTIFF_WrongDataTypeForTag()            ; break;
        case UNKNOWN_TAG                                : messageID = ImagePPMessages::HTIFF_UnknownTag()                     ; break;
        case UNKNOWN_FIRST_TAG                          : messageID = ImagePPMessages::HTIFF_UnknownFirstTag()                ; break;
        case DIRECTORY_ENTRY_READ_ERROR                 : messageID = ImagePPMessages::HTIFF_DirectoryEntryReadError()        ; break;
        case TAG_READ_ERROR                             : messageID = ImagePPMessages::HTIFF_TagReadError()                   ; break;
        case DIRECTORY_ENTRY_WRITE_ERROR                : messageID = ImagePPMessages::HTIFF_DirectoryEntryWriteError()       ; break;
        case TAG_WRITE_ERROR                            : messageID = ImagePPMessages::HTIFF_TagWriteError()                  ; break;
        case INCORRECT_COUNT_FOR_TAG_READ               : messageID = ImagePPMessages::HTIFF_IncorrectCountForTagRead()       ; break;
        case INCORRECT_COUNT_FOR_TAG_WRITTEN            : messageID = ImagePPMessages::HTIFF_IncorrectCountForTagWritten()    ; break;
        case SIZE_OUT_OF_RANGE                          : messageID = ImagePPMessages::HTIFF_SizeOutOfRange()                 ; break;
        default:
            HASSERT(!"Missing HTIFF error message");
            messageID = ImagePPMessages::HTIFF_UnknownError();
            break;
        }

    // Do not want to translate HTIFF. That's the name of our tiff lib.
    po_rErrorMsg.Sprintf(L"HTIFF(%.4d) - %ls", m_ErrorCode, ImagePPMessages::GetStringW(messageID).c_str());

    if (m_pErrorInfo != 0)
        m_pErrorInfo->FormatErrorMessage(po_rErrorMsg);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HTIFFError::CopyData(const HTIFFError& pi_rSrcObj)
    {
    m_Fatal = pi_rSrcObj.m_Fatal;

    if (pi_rSrcObj.m_pErrorInfo != 0)
        {
        m_pErrorInfo = pi_rSrcObj.m_pErrorInfo->Clone();
        }
    else
        {
        m_pErrorInfo = 0;
        }

    m_ErrorCode = pi_rSrcObj.m_ErrorCode;
    }

//-----------------------------------------------------------------------------
// Class HTIFFStream
//-----------------------------------------------------------------------------

HTIFFStream::HTIFFStream(const WString& pi_rFilename, HFCAccessMode pi_Mode, uint64_t pi_OriginOffset)
    {
    m_ListDirty = false;
    m_ListOfFreeBlock.clear();
    m_pStream = new HFCLocalBinStream(pi_rFilename, pi_Mode, false, pi_OriginOffset);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HTIFFStream::HTIFFStream(const HFCPtr<HFCURL>& pi_rpURL, HFCAccessMode pi_AccessMode, uint64_t pi_OriginOffset)
    {
    m_ListDirty = false;
    m_ListOfFreeBlock.clear();
    m_pStream = HFCBinStream::Instanciate(pi_rpURL, pi_OriginOffset, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HTIFFStream::~HTIFFStream()
    {
    m_ListOfFreeBlock.clear();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HFCPtr<HFCURL> HTIFFStream::GetURL() const
    {
    return m_pStream->GetURL();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HTIFFStream::Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share)
    {
    HASSERT(m_pStream != 0);
    m_pStream->Lock(pi_Pos, pi_Size, pi_Share);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HTIFFStream::Unlock(uint64_t pi_Pos, uint64_t pi_Size)
    {
    HASSERT(m_pStream != 0);
    m_pStream->Unlock(pi_Pos, pi_Size);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HTIFFStream::AddInitialFreeBlock (const uint64_t* pi_pOffset, const uint64_t* pi_pSize, uint32_t pi_Count)
    {
    HTIFFStreamFreeBlock Block;

    for (uint32_t i=0; i<pi_Count; i++)
        {
        Block.Offset = pi_pOffset[i];
        Block.Size   = pi_pSize[i];

        m_ListOfFreeBlock.push_back(Block);
        }
    }
void HTIFFStream::AddInitialFreeBlock (const uint32_t* pi_pOffset, const uint32_t* pi_pSize, uint32_t pi_Count)
    {
    HTIFFStreamFreeBlock Block;

    m_ListOfFreeBlock.clear();

    for (uint32_t i=0; i<pi_Count; i++)
        {
        Block.Offset = pi_pOffset[i];
        Block.Size   = pi_pSize[i];

        m_ListOfFreeBlock.push_back(Block);
        }
    }


//-----------------------------------------------------------------------------
// The caller must delete the lists.
// Return true if the list has been modifying
//-----------------------------------------------------------------------------

bool HTIFFStream::GetListFreeBlock (uint64_t** pi_ppOffset, uint64_t** pi_ppSize, uint32_t* pi_pCount)
    {
    if ((*pi_pCount = (uint32_t)m_ListOfFreeBlock.size()) > 0)
        {
        *pi_ppOffset = new uint64_t[*pi_pCount];
        *pi_ppSize   = new uint64_t[*pi_pCount];

        ListOfFreeBlock::iterator Itr;
        uint32_t i=0;
        for (Itr=m_ListOfFreeBlock.begin(); Itr != m_ListOfFreeBlock.end(); Itr++,i++)
            {
            (*pi_ppOffset)[i] = (*Itr).Offset;
            (*pi_ppSize)[i]   = (*Itr).Size;
            }
        }
    else
        {
        *pi_ppOffset = 0;
        *pi_ppSize   = 0;
        *pi_pCount   = 0;
        }

    return m_ListDirty;
    }


//-----------------------------------------------------------------------------
// CheckAlloc : Check if you can rewrite at the same place, or try to allocate
//              an other block. (take a free block, or append at the end of file)
//
//  If a new place is found, pio_pOffset receive the new offset.
//  If pi_CurSize == 0 no previous block.
//-----------------------------------------------------------------------------
void HTIFFStream::CheckAlloc (uint64_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize)
    {
    // If new block is ==, rewrite at the same place, do nothing
    // Warning: this return is lost in code......
    if (pi_NewSize == pi_CurSize)
        return;

    // Add new free block, Try to merge it
    if (pi_CurSize != 0)
        ReleaseBlock(*pio_pOffset, pi_CurSize);


    // Get best matching block for new size
    GetBlock(pio_pOffset, pi_NewSize);
    }

//-----------------------------------------------------------------------------
// public
// Release a file block so that it is made available again for other uses.
//-----------------------------------------------------------------------------
void HTIFFStream::ReleaseBlock (uint64_t pi_Offset, uint32_t pi_Size)
    {
    // Specify the list is dirty
    m_ListDirty = true;

    const HTIFFStreamFreeBlock ReleasedBlock = {pi_Offset, pi_Size};

    if (m_ListOfFreeBlock.empty())
        {
        m_ListOfFreeBlock.push_front(ReleasedBlock);
        return;
        }

    // NOTE: If the list of free blocks tends to be big most of the time, it would be profitable to
    //       use a set instead of a list in order to be able to use binary search algorithms
    //       (lower_bound in this case)
    // Find the first block whose offset is greater than the released block
    const ListOfFreeBlock::iterator NextBlockIt = find_if(m_ListOfFreeBlock.begin(), m_ListOfFreeBlock.end(),
                                                          bind1st(less<HTIFFStreamFreeBlock>(), ReleasedBlock));

    bool HasPrevious = m_ListOfFreeBlock.begin() != NextBlockIt;
    bool HasNext = m_ListOfFreeBlock.end() != NextBlockIt;

    // Try to merge with previous block
    ListOfFreeBlock::iterator PreviousBlockIt(NextBlockIt);
    if (HasPrevious && (--PreviousBlockIt)->PrecedesImmediatly(ReleasedBlock))
        {
        // Merge with the previous block
        PreviousBlockIt->Size += ReleasedBlock.Size;


        // If merged with the previous, now check for the next block too
        if (HasNext)
            {
            if (PreviousBlockIt->PrecedesImmediatly(*NextBlockIt))
                {
                PreviousBlockIt->Size += NextBlockIt->Size;
                m_ListOfFreeBlock.erase(NextBlockIt);
                }
            }
        }
    // Try to merge with the next block only
    else if (HasNext)
        {
        if (ReleasedBlock.PrecedesImmediatly(*NextBlockIt))
            {
            // Merge with the Next block
            NextBlockIt->Offset  = ReleasedBlock.Offset;
            NextBlockIt->Size   += ReleasedBlock.Size;
            }
        else
            {
            // Unable to merge neither with previous or next but has next
            // so insert before next block
            if (HasPrevious)
                m_ListOfFreeBlock.insert(NextBlockIt, ReleasedBlock);
            else
                m_ListOfFreeBlock.push_front(ReleasedBlock);
            }
        }
    // Do not have a previous or next block
    else
        {
        // Do not have previous or next free block so append at the end of the list.
        m_ListOfFreeBlock.push_back(ReleasedBlock);
        }
    }

//-----------------------------------------------------------------------------
// public
// Get a file block for the specified size. Try to reuse previously freed block
// first, checking for the shortest block matching specified size. If no freed
// block match the specified size, expend the file.
//-----------------------------------------------------------------------------
void HTIFFStream::GetBlock (uint64_t* po_pOffset, uint32_t pi_Size)
    {
    // Now, try to find best suited free block that fits new size
    ListOfFreeBlock::iterator BestSizeItr;
    uint64_t BestSize        = (uint64_t)-1;

    if (!m_ListOfFreeBlock.empty())
        {
        for (ListOfFreeBlock::iterator FreeBlockIt = m_ListOfFreeBlock.begin();
             FreeBlockIt != m_ListOfFreeBlock.end();
             ++FreeBlockIt)
            {
            // Check for the best Size
            //
            if ((FreeBlockIt->Size >= pi_Size) &&
                (FreeBlockIt->Size < BestSize))
                {
                BestSize    = FreeBlockIt->Size;
                BestSizeItr = FreeBlockIt;
                }
            }
        }


    // Check if block found
    //
    if (BestSize != (uint64_t)-1)
        {
        // Set new offset
        *po_pOffset = (*BestSizeItr).Offset;

        // Block, check if you use the complete block
        if (pi_Size == (*BestSizeItr).Size)
            {
            m_ListOfFreeBlock.erase(BestSizeItr);
            }
        else
            {
            // Ajuste the block
            (*BestSizeItr).Offset += pi_Size;
            (*BestSizeItr).Size   -= pi_Size;
            }
        }
    else
        {
        // No block found, extented the file.
        Seek(0, SEEK_END);
        *po_pOffset = Tell();
        }
    }

/**----------------------------------------------------------------------------
 Check if you can rewrite at the same place, else allocate a new block at the
 end.(in this case, the old block is lost)

 @param pio_pOffset receive the offset to use.
 @param pi_CurSize Size of the previous block.(==0 means no previous block)

-----------------------------------------------------------------------------*/
void HTIFFStream::CheckAllocWithoutUpdate (uint64_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize)
    {
    // If new block is <=, rewrite at the same place, do nothing
    // Attention: this return is lost in code......
    if (pi_NewSize > pi_CurSize)
        {
        // add at the end of the file.
        Seek(0, SEEK_END);
        *pio_pOffset = Tell();
        }
    }


/**----------------------------------------------------------------------------
 Verify that the specified block (position and size) does not overlap one
 of our free blocks. This can happen in files that were edited prior to
 adding the CheckAllocWithoutUpdate method for free blocks saving.

 @param pio_Offset receive the offset of the block to check.
 @param pi_Size Size of the block.
 @return true if there is an overlap, false otherwise.

-----------------------------------------------------------------------------*/
bool HTIFFStream::OverlapsFreeBlocks(uint64_t pi_Offset, uint64_t pi_Size) const
    {
    bool Result = false;

    ListOfFreeBlock::const_iterator Itr(m_ListOfFreeBlock.begin());
    while (!Result && Itr != m_ListOfFreeBlock.end())
        {
        // Test block overlapping
        if (pi_Offset < ((*Itr).Offset+(*Itr).Size) &&
            pi_Offset+pi_Size > (*Itr).Offset)
            {
            Result = true;
            }

        ++Itr;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Return true if Fatal Error.
//-----------------------------------------------------------------------------
bool ImagePP::ErrorMsg(HTIFFError**              pio_pError,
               HTIFFError::ErrorCode     pi_ErrorCode,
               const HTIFFError::ErInfo* pi_pErrorInfo,
               bool                     pi_Fatal)
    {
    // Copy Error message
    if (*pio_pError)
        (*pio_pError)->AddError(pi_ErrorCode, pi_pErrorInfo, pi_Fatal);
    else
        *pio_pError = new HTIFFError(pi_ErrorCode, pi_pErrorInfo, pi_Fatal);

    return (pi_Fatal);
    }

/*
//-----------------------------------------------------------------------------
// Return true if Fatal Error.
//-----------------------------------------------------------------------------
bool ImagePP::ErrorMsg (HTIFFError** pio_pError, const char* pi_pMsg, bool pi_Fatal)
{
    // Copy Error message
    if (*pio_pError)
        (*pio_pError)->AddError(pi_pMsg, pi_Fatal);
    else
        *pio_pError = new HTIFFError(pi_pMsg, pi_Fatal);

    return (pi_Fatal);
}
*/
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


bool ImagePP::ErrorMsg (HTIFFError** pio_pError, HTIFFError& pi_rNewError)
    {
    return ErrorMsg(pio_pError, pi_rNewError.GetErrorCode(), pi_rNewError.GetErrorInfo(), pi_rNewError.IsFatal());
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ImagePP::SwabArrayOfShort (unsigned short* pio_pData, register size_t pi_Count)
    {
    register unsigned char*    pData;
    register int        Tmp;

    /* XXX unroll loop some */
    while (pi_Count-- > 0)
        {
        pData = (unsigned char*)pio_pData;
        Tmp = pData[1];
        pData[1] = pData[0];
        pData[0] = (unsigned char)Tmp;
        pio_pData++;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ImagePP::SwabArrayOfLong(register uint32_t* pio_pData, register size_t pi_Count)
    {
    register unsigned char*    pData;
    register int        Tmp;

    /* XXX unroll loop some */
    while (pi_Count-- > 0)
        {
        pData = (unsigned char*)pio_pData;
        Tmp = pData[3];
        pData[3] = pData[0];
        pData[0] = (unsigned char)Tmp;
        Tmp = pData[2];
        pData[2] = pData[1];
        pData[1] = (unsigned char)Tmp;
        pio_pData++;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ImagePP::SwabArrayOfDouble(double* pio_pData, register size_t pi_Count)
    {
    register uint32_t* pLong = (uint32_t*) pio_pData;
    register uint32_t Tmp;

    SwabArrayOfLong(pLong, pi_Count + pi_Count);
    while (pi_Count-- > 0)
        {
        Tmp = pLong[0];
        pLong[0] = pLong[1];
        pLong[1] = Tmp;
        pLong += 2;
        }
    }

void ImagePP::SwabArrayOfUInt64(uint64_t* pio_pData, register size_t pi_Count)
    {
    register uint32_t* pLong = (uint32_t*) pio_pData;
    register uint32_t Tmp;

    SwabArrayOfLong(pLong, pi_Count + pi_Count);
    while (pi_Count-- > 0)
        {
        Tmp = pLong[0];
        pLong[0] = pLong[1];
        pLong[1] = Tmp;
        pLong += 2;
        }
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool ImagePP::SystemIsBigEndian()
    {
    bool SystemBigEndian;
        {
        union {
            int32_t i;
            char c[4];
            } u;
        u.i = 1;
        SystemBigEndian = (u.c[0] == 0);
        }
    return SystemBigEndian;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool ImagePP::IsValidDoubleArray(const double* pi_pValues, int32_t pi_ArraySize)
    {
    HPRECONDITION (pi_pValues != 0);
    HPRECONDITION (pi_ArraySize > 0);

    bool IsValid = true;

    for (int ArrayIndex = 0; ArrayIndex < pi_ArraySize; ArrayIndex++)
        {
        if (BeNumerical::BeIsnan(pi_pValues[ArrayIndex]) || !BeNumerical::BeFinite(pi_pValues[ArrayIndex]))
            {
            IsValid = false;

            // Break the loop after any fail.
            ArrayIndex =  pi_ArraySize;
            }
        }
    return IsValid;
    }


HFCAccessMode HTIFFStream::GetMode() const
    {
    HPRECONDITION(m_pStream != 0);
    return m_pStream->GetAccessMode();
    }

size_t HTIFFStream::Read(void* po_pBuffer, size_t pi_Size, size_t pi_Count)
    {
    HPRECONDITION(m_pStream != 0);
    HPRECONDITION(pi_Size   != 0);
    HPRECONDITION(pi_Count  != 0);
    return (m_pStream->Read(po_pBuffer, pi_Size * pi_Count) / pi_Size);
    }

size_t HTIFFStream::Write(const void* pi_pBuffer, size_t pi_Size, size_t pi_Count)
    {
    HPRECONDITION(m_pStream != 0);
    HPRECONDITION(pi_Size   != 0);
    HPRECONDITION(pi_Count  != 0);
    size_t result = m_pStream->Write(pi_pBuffer, pi_Size * pi_Count) / pi_Size;

    // Stream write error
    if ((pi_Count != result) || (m_pStream->GetLastException() != 0))
        {
#ifdef _WIN32
        // Some error must have occured
        int32_t lastError = GetLastError();

        HASSERT ((lastError == ERROR_DISK_FULL) ||
            (m_pStream->GetLastException() != 0));
#endif

        // Notes on possible error
        // Since we are using huge files, it may happen that the stream WriteFile call will result in error 665
        // This error "The requested operation could not be completed due to a file system limitation"
        // can be caused by file fragmentation on NTFS file system especially if the file is NTFS compressed.
        // If this occurs, smaller file size should be used. Not using NTFS compression or FAT32 file system may help.

        //Make sure that result is set to 0 to indicate failure
        result = 0;
        }

    return result;
    }

bool HTIFFStream::Seek (uint64_t pi_Offset, int pi_Origin)
    {
    HPRECONDITION(m_pStream != 0);

    if (pi_Origin == SEEK_END)
        {
        m_pStream->SeekToEnd();
        }
    else if ( pi_Origin == SEEK_CUR )
        {
        m_pStream->Seek(pi_Offset);
        }
    else
        m_pStream->SeekToPos(pi_Offset);

    return true;
    }

uint64_t HTIFFStream::Tell()
    {
    HPRECONDITION(m_pStream != 0);
    return m_pStream->GetCurrentPos();
    }

uint64_t HTIFFStream::GetSize()
    {
    HPRECONDITION(m_pStream != 0);
    return m_pStream->GetSize();
    }

HFCBinStream* HTIFFStream::GetFilePtr() const
    {
    return m_pStream.get();
    }
