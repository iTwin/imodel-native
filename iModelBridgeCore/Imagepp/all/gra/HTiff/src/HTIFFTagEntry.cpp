//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFTagEntry.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFTagEntry
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HTIFFTagEntry.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HFCMemcpy.h>
#include <Imagepp/all/h/HTIFFDirectory.h>

#include <Imagepp/all/h/HTIFFRational.h>

#define MAX_SPP     48      // Maximun number of sample by pixels


//-----------------------------------------------------------------------------
// public
// Constructor, Create an empty entry.
//-----------------------------------------------------------------------------
HTIFFTagEntry:: HTIFFTagEntry(const HTagInfo&           pi_rTagInfo,
                              const HTIFFByteOrdering*  pi_pByteOrder)


    {
    m_pTagDef   = 0;
    m_pError    = 0;
    m_pEntry    = 0;
    m_pByteOrder= (HTIFFByteOrdering*)pi_pByteOrder;
    }


//-----------------------------------------------------------------------------
// public
// Constructor, Create an empty entry to a specify Tag..
//-----------------------------------------------------------------------------
HTIFFTagEntry::HTIFFTagEntry(const HTagInfo&            pi_rTagInfo,
                             HTagID                     pi_Tag,
                             const HTIFFByteOrdering*   pi_pByteOrder,
                             bool                      pi_IsTiff64)

    {
    m_pTagDef   = new HTagDefinition(pi_rTagInfo, pi_Tag, pi_IsTiff64);
    m_pError    = 0;
    m_pEntry    = 0;

    m_pByteOrder= (HTIFFByteOrdering*)pi_pByteOrder;

    HTIFFError* pMsg;
    if (m_pTagDef->IsValid(&pMsg))
        {
        m_pEntry = new TagEntry64;
        HASSERT(m_pEntry != 0);
        memset(m_pEntry, 0, sizeof(TagEntry64));

        // Allocate Space for Data Tag
        if (m_pTagDef->GetWriteCount() == HTagInfo::TAG_IO_ANY)
            {
            m_pEntry->pData = 0;
            }
        else if (m_pTagDef->GetWriteCount() == HTagInfo::TAG_IO_VARIABLE)
            {
            m_pEntry->pData = 0;
            }
        else if (m_pTagDef->GetWriteCount() == HTagInfo::TAG_IO_USE_SPP)
            {
            m_pEntry->pData = new Byte[m_pTagDef->GetDataLen()*MAX_SPP];
            m_pEntry->Status.DataInOffset = false;
            }
        else
            {
            m_pEntry->pData = new Byte[m_pTagDef->GetDataLen()*m_pTagDef->GetWriteCount()];
            m_pEntry->Status.DataInOffset   = false;
            m_pEntry->DirCount            = m_pTagDef->GetWriteCount();
            }
        }
    else
        ErrorMsg(&m_pError, *pMsg);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HTIFFTagEntry::~HTIFFTagEntry()
    {
    delete m_pTagDef;
    delete m_pError;

    // Delete pData if Allocated
    if ((m_pEntry != 0) && (m_pEntry->pData != 0) && !m_pEntry->Status.DataInOffset)
        delete[] m_pEntry->pData;

    delete m_pEntry;
    }



//-----------------------------------------------------------------------------
// public
// ReadTagEntry - Read the Tag Entry
//
//  The pio_pFile must be at the beginning of the entry.
//  The pio_pFile pointed on the Next Tag Entry.
//-----------------------------------------------------------------------------
bool HTIFFTagEntry::ReadTagEntry (const HTagInfo&      pi_rTagInfo,
                                   HTIFFStream*         pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);

    HTagDefinition::FileDirEntry64 TagDescriptor;

    bool ReadFail = false;
    ReadFail |= (pio_pFile->Read(&TagDescriptor.FileTag, sizeof(unsigned short), 1) != 1);
    ReadFail |= (pio_pFile->Read(&TagDescriptor.DataType, sizeof(unsigned short), 1) != 1);

    if (pio_pFile->m_IsTiff64)
        {
        ReadFail |= (pio_pFile->Read(&TagDescriptor.DirCount64, sizeof(uint64_t), 1) != 1);
        ReadFail |= (pio_pFile->Read(&TagDescriptor.Offset64, sizeof(uint64_t), 1) != 1);
        }
    else
        {
        ReadFail |= (pio_pFile->Read(&TagDescriptor.DirCount32, sizeof(uint32_t), 1) != 1);
        ReadFail |= (pio_pFile->Read(&TagDescriptor.Offset32, sizeof(uint32_t), 1) != 1);
        }
    if (ReadFail)
        {
        ErrorMsg (&m_pError, HTIFFError::DIRECTORY_ENTRY_READ_ERROR, 0, true);
        goto WRAPUP;
        }

    if (m_pByteOrder->NeedSwapByte())
        {
        SwabArrayOfShort(&TagDescriptor.FileTag, 1);
        SwabArrayOfShort(&TagDescriptor.DataType, 1);
        if (pio_pFile->m_IsTiff64)
            {
            SwabArrayOfUInt64(&TagDescriptor.DirCount64, 1);
            SwabArrayOfUInt64(&TagDescriptor.Offset64, 1);
            }
        else
            {
            SwabArrayOfLong(&TagDescriptor.DirCount32, 1);
            SwabArrayOfLong(&TagDescriptor.Offset32, 1);
            }
        }

    // If Entry not allocated
    if (m_pTagDef == 0)
        {
        m_pTagDef   = new HTagDefinition(pi_rTagInfo, TagDescriptor, pio_pFile->m_IsTiff64);

        HTIFFError* pMsg;
        if (!m_pTagDef->IsValid(&pMsg))
            {
            ErrorMsg(&m_pError, *pMsg);
            goto WRAPUP;
            }
        }

    // Check the count field of a directory
    //
    if (!ValidateDataLen(pio_pFile->m_IsTiff64 ? TagDescriptor.DirCount64 : TagDescriptor.DirCount32))
        goto WRAPUP;


    // Alloc an entry, if not already yet
    if (m_pEntry == 0)
        {
        m_pEntry = new TagEntry64;
        HASSERT(m_pEntry != 0);
        memset(m_pEntry, 0, sizeof(TagEntry64));
        }

    if (pio_pFile->m_IsTiff64)
        {
        // The standard support Count64, not me

        HASSERT(TagDescriptor.DirCount64 < ULONG_MAX);
        m_pEntry->DirCount = (uint32_t)TagDescriptor.DirCount64;
        m_pEntry->Offset64 = TagDescriptor.Offset64;
        }
    else
        {
        m_pEntry->DirCount = TagDescriptor.DirCount32;
        m_pEntry->Offset64 = TagDescriptor.Offset32;
        }

    return true;
WRAPUP:
    delete m_pEntry;
    m_pEntry = 0;

    return (false);
    }


//-----------------------------------------------------------------------------
// public
// ReadData - Read the Data for the Specify Tag if necessary.
//
//  The pio_pFile pointed on the Tag Data end offset or unchanged..
//-----------------------------------------------------------------------------
bool HTIFFTagEntry::ReadData (const HTagInfo&      pi_rTagInfo,
                               HTIFFStream*         pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(m_pEntry != 0);

    // Check if Data is in the Offset field
    uint32_t NbByte = m_pEntry->DirCount * m_pTagDef->GetDataLen();
    if (NbByte > (pio_pFile->m_IsTiff64 ? (uint32_t)8 : (uint32_t)4))
        {
        m_pEntry->pData      = new Byte[NbByte];
        m_pEntry->SizeInFile = NbByte;

        if (!pio_pFile->Seek(m_pEntry->Offset64, SEEK_SET)  ||
            pio_pFile->Read(m_pEntry->pData, 1, NbByte) != NbByte)
            {
            HTIFFError::TagIOErInfo ErInfo;
            ErInfo.m_DataLength = NbByte;
            string TagName(m_pTagDef->GetTagName());
            ErInfo.m_TagName   = WString(TagName.c_str(),false);
            ErInfo.m_TagFileNb = m_pTagDef->GetFileTag();

            ErrorMsg (&m_pError, HTIFFError::TAG_READ_ERROR, &ErInfo, true);
            goto WRAPUP;
            }

        //TR #163545 : to be sure that the string ends with a NULL character
        if(m_pTagDef->GetDataType() == HTagInfo::ASCII && *((char*)m_pEntry->pData + NbByte - 1) != 0)
            {
            Byte* pTempToBeDeleted = m_pEntry->pData;
            m_pEntry->pData = new Byte[NbByte + 1];
            HFCMemcpy(m_pEntry->pData, pTempToBeDeleted, NbByte);
            *((char*)m_pEntry->pData + NbByte) = 0;

            delete pTempToBeDeleted;
            }


        // Swap, platform independent
        if (m_pByteOrder->NeedSwapByte())
            m_pEntry->Status.NeedSwap = true;

        m_pEntry->Status.DataInOffset = false;
        }
    else
        {
        switch (m_pTagDef->GetDataType())
            {
            case HTagInfo::BYTE:
            case HTagInfo::ASCII:
            case HTagInfo::SBYTE:
            case HTagInfo::UNDEFINED:
                {
                m_pEntry->pData = new Byte[NbByte];

                if (pio_pFile->m_IsTiff64)
                    {
                    uint64_t LVal = m_pEntry->Offset64;
                    if (m_pByteOrder->NeedSwapByte())
                        SwabArrayOfUInt64(&LVal, 1);
                    HFCMemcpy(m_pEntry->pData, &LVal, NbByte);
                    }
                else
                    {
                    uint32_t LVal = (uint32_t)m_pEntry->Offset64;
                    if (m_pByteOrder->NeedSwapByte())
                        SwabArrayOfLong(&LVal, 1);
                    HFCMemcpy(m_pEntry->pData, &LVal, NbByte);
                    }

                m_pEntry->Status.DataInOffset = false;
                }
            break;


            case HTagInfo::SHORT:
            case HTagInfo::SSHORT:
                if (pio_pFile->m_IsTiff64)
                    {
                    switch (m_pEntry->DirCount)
                        {
                        case 1:
                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                m_pEntry->Offset64 = m_pEntry->Offset64 >> 48;

                            m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                            m_pEntry->Status.DataInOffset = true;
                            break;

                        case 2:
                            HASSERT(false);
                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                m_pEntry->Offset64 = m_pEntry->Offset64 >> 32;

                            m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                            m_pEntry->Status.DataInOffset = true;
                            break;

                        case 3:
                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                m_pEntry->Offset64 = m_pEntry->Offset64 >> 16;

                            m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                            m_pEntry->Status.DataInOffset = true;
                            break;

                        case 4:
                            m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                            m_pEntry->Status.DataInOffset = true;
                            break;
                        }
                    break;
                    }
                else
                    {
                    if (m_pEntry->DirCount == 1)
                        {
                        if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                            m_pEntry->Offset64 = m_pEntry->Offset64 >> 16;

                        m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                        m_pEntry->Status.DataInOffset = true;
                        break;
                        }
                    }

            case HTagInfo::LONG:
            case HTagInfo::SLONG:
            case HTagInfo::FLOAT:
                if (pio_pFile->m_IsTiff64)
                    {
                    if (m_pEntry->DirCount == 1)
                        {
                        if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                            m_pEntry->Offset64 = m_pEntry->Offset64 >> 32;

                        m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                        m_pEntry->Status.DataInOffset = true;
                        break;
                        }
                    }
                else
                    {
                    m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                    m_pEntry->Status.DataInOffset = true;
                    break;
                    }

            case HTagInfo::RATIONAL:
            case HTagInfo::SRATIONAL:
            case HTagInfo::DOUBLE:
            case HTagInfo::LONG64:
            case HTagInfo::SLONG64:
                HASSERT(pio_pFile->m_IsTiff64);

                m_pEntry->pData = (Byte*)&(m_pEntry->Offset64);
                m_pEntry->Status.DataInOffset = true;
                break;

            default:
                HASSERT(false);
                break;
            }
        }

    //TR #163545 : to be sure that the string ends with a NULL character
    if(m_pTagDef->GetDataType() == HTagInfo::ASCII && *((char*)m_pEntry->pData + NbByte - 1) != 0)
        {
        // Cause Chuck problem HASSERT(*((char*)m_pEntry->pData + NbByte -1) == 0 ); //this ASCII tag doesn't end with a NULL character

        // If the dataset is stored in the offset field and the space is enough for the null, add it.
        if (m_pEntry->Status.DataInOffset && NbByte < (pio_pFile->m_IsTiff64 ? (uint32_t)8 : (uint32_t)4))
            {
            *((char*)m_pEntry->pData + NbByte) = 0;
            }
        else
            {
            Byte* pTempToBeDeleted = m_pEntry->pData;
            m_pEntry->pData = new Byte[NbByte + 1];
            HFCMemcpy(m_pEntry->pData, pTempToBeDeleted, NbByte);
            *((char*)m_pEntry->pData + NbByte) = 0;

            if (m_pEntry->Status.DataInOffset)
                m_pEntry->Status.DataInOffset = false;
            else
                delete pTempToBeDeleted;        // do not delete it, if in the offset field previously
            }
        }

    return true;
WRAPUP:
    delete m_pEntry;
    m_pEntry = 0;

    return (false);
    }


// The FilePtr must be at the beginning of the entry
//
bool HTIFFTagEntry::WriteTagEntry (const HTagInfo&     pi_rTagInfo,
                                    HTIFFStream*        pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);

    HTagDefinition::FileDirEntry64 TagDescriptor;

    TagDescriptor.FileTag   = (unsigned short)m_pTagDef->GetFileTag();
    TagDescriptor.DataType  = (unsigned short)m_pTagDef->GetDataType();
    if (pio_pFile->m_IsTiff64)
        {
        TagDescriptor.DirCount64  = m_pEntry->DirCount;
        TagDescriptor.Offset64    = m_pEntry->Offset64;
        }
    else
        {
        TagDescriptor.DirCount32  = m_pEntry->DirCount;
        TagDescriptor.Offset32    = (uint32_t)m_pEntry->Offset64;
        }

    // Check if Data is in the Offset field
    // Update Offset Field

    uint32_t NbByte = m_pEntry->DirCount * m_pTagDef->GetDataLen();
    if (NbByte <= (pio_pFile->m_IsTiff64 ? (uint32_t)8 : (uint32_t)4))
        {
        switch (m_pTagDef->GetDataType())
            {
            case HTagInfo::BYTE:
            case HTagInfo::ASCII:
            case HTagInfo::SBYTE:
            case HTagInfo::UNDEFINED:
                if (pio_pFile->m_IsTiff64)
                    {
                    HFCMemcpy(&TagDescriptor.Offset64, m_pEntry->pData, NbByte);

                    if (m_pByteOrder->NeedSwapByte())
                        SwabArrayOfUInt64(&TagDescriptor.Offset64, 1);
                    }
                else
                    {
                    HFCMemcpy(&TagDescriptor.Offset32, m_pEntry->pData, NbByte);

                    if (m_pByteOrder->NeedSwapByte())
                        SwabArrayOfLong(&TagDescriptor.Offset32, 1);
                    }
                break;

            case HTagInfo::SHORT:
            case HTagInfo::SSHORT:
                if (pio_pFile->m_IsTiff64)
                    {
                    switch (m_pEntry->DirCount)
                        {
                        case 1:
                            TagDescriptor.Offset64 = 0;
                            memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 2);

                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                TagDescriptor.Offset64 = TagDescriptor.Offset64 << 48;
                            break;

                        case 2:
                            HASSERT(false);
                            TagDescriptor.Offset64 = 0;
                            memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 4);

                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                TagDescriptor.Offset64 = TagDescriptor.Offset64 << 32;
                            break;

                        case 3:
                            TagDescriptor.Offset64 = 0;
                            memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 6);

                            if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                                TagDescriptor.Offset64 = TagDescriptor.Offset64 << 16;
                            break;

                        case 4:
                            memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 8);
                            break;
                        }
                    break;
                    }
                else
                    {
                    if (m_pEntry->DirCount == 1)
                        {
                        TagDescriptor.Offset64 = 0;
                        memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 2);

                        if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                            TagDescriptor.Offset64 = TagDescriptor.Offset64 << 16;

                        TagDescriptor.Offset32 = (uint32_t)TagDescriptor.Offset64;
                        break;
                        }
                    }

            case HTagInfo::LONG:
            case HTagInfo::SLONG:
            case HTagInfo::FLOAT:
                if (pio_pFile->m_IsTiff64)
                    {
                    if (m_pEntry->DirCount == 1)
                        {
                        memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 4);
                        if (m_pByteOrder->IsStoredAsBigEndian()) // Motorola
                            TagDescriptor.Offset64 = TagDescriptor.Offset64 << 32;
                        break;
                        }
                    }
                else
                    {
                    memcpy (&TagDescriptor.Offset32, m_pEntry->pData, 4);
//                    TagDescriptor.Offset32 = *((UInt32*)m_pEntry->pData);
                    break;
                    }

            case HTagInfo::RATIONAL:
            case HTagInfo::SRATIONAL:
            case HTagInfo::DOUBLE:
            case HTagInfo::LONG64:
            case HTagInfo::SLONG64:
                HASSERT(pio_pFile->m_IsTiff64);

                memcpy (&TagDescriptor.Offset64, m_pEntry->pData, 8);
//                TagDescriptor.Offset64 = m_pEntry->Offset64;
                break;

            default:
                break;
            }
        }

    if (m_pByteOrder->NeedSwapByte())
        {
        SwabArrayOfShort(&TagDescriptor.FileTag, 1);
        SwabArrayOfShort(&TagDescriptor.DataType, 1);
        if (pio_pFile->m_IsTiff64)
            {
            SwabArrayOfUInt64(&TagDescriptor.DirCount64, 1);
            SwabArrayOfUInt64(&TagDescriptor.Offset64, 1);
            }
        else
            {
            SwabArrayOfLong(&TagDescriptor.DirCount32, 1);
            SwabArrayOfLong(&TagDescriptor.Offset32, 1);
            }
        }

    bool WriteFail = false;
    WriteFail |= (pio_pFile->Write(&TagDescriptor.FileTag, sizeof(unsigned short), 1) != 1);
    WriteFail |= (pio_pFile->Write(&TagDescriptor.DataType, sizeof(unsigned short), 1) != 1);
    if (pio_pFile->m_IsTiff64)
        {
        WriteFail |= (pio_pFile->Write(&TagDescriptor.DirCount64, sizeof(uint64_t), 1) != 1);
        WriteFail |= (pio_pFile->Write(&TagDescriptor.Offset64, sizeof(uint64_t), 1) != 1);
        }
    else
        {
        WriteFail |= (pio_pFile->Write(&TagDescriptor.DirCount32, sizeof(uint32_t), 1) != 1);
        WriteFail |= (pio_pFile->Write(&TagDescriptor.Offset32, sizeof(uint32_t), 1) != 1);
        }
    if (WriteFail)
        {
        ErrorMsg (&m_pError, HTIFFError::DIRECTORY_ENTRY_WRITE_ERROR, 0, true);
        goto WRAPUP;
        }

    return true;
WRAPUP:
    return false;
    }


bool HTIFFTagEntry::WriteData (const HTagInfo&     pi_rTagInfo,
                                HTIFFStream*        pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);
    HPRECONDITION(m_pEntry != 0);

    // Check if Data is in the Offset field then do nothing
    uint32_t NbByte = m_pEntry->DirCount * m_pTagDef->GetDataLen();
    if (NbByte > (pio_pFile->m_IsTiff64 ? (uint32_t)8 : (uint32_t)4))
        {
        // Alloc, if new tag
        // If Data size changed, then realloc
        if ((m_pEntry->Offset64 == 0) || m_pEntry->Status.Resize)
            {
            if (GetTagID() == pi_rTagInfo.GetFreeOffsetsTagID() ||
                GetTagID() == pi_rTagInfo.GetFreeByteCountsTagID())
                pio_pFile->CheckAllocWithoutUpdate (&(m_pEntry->Offset64), m_pEntry->SizeInFile, NbByte);
            else
                pio_pFile->CheckAlloc (&(m_pEntry->Offset64), m_pEntry->SizeInFile, NbByte);

            m_pEntry->SizeInFile = NbByte;
            }

        bool StatusSwapChanged(false);
        // Swap, platform independent, possibly already swapped, if never used.
        if (m_pByteOrder->NeedSwapByte() && !m_pEntry->Status.NeedSwap)
            {
            SwapData (m_pEntry->pData);
            StatusSwapChanged = true;
            }

        if (!pio_pFile->Seek(m_pEntry->Offset64, SEEK_SET) ||
            pio_pFile->Write(m_pEntry->pData, 1, NbByte) != NbByte)
            {
            HTIFFError::TagIOErInfo ErInfo;
            ErInfo.m_DataLength = NbByte;
            string TagName(m_pTagDef->GetTagName());
            ErInfo.m_TagName   = WString(TagName.c_str(),false);
            ErInfo.m_TagFileNb = m_pTagDef->GetFileTag();
            ErrorMsg (&m_pError, HTIFFError::TAG_READ_ERROR, &ErInfo, true);

            goto WRAPUP;
            }

        // Keep the previous status
        if (StatusSwapChanged)
            SwapData (m_pEntry->pData);

        m_pEntry->Status.Dirty = false;
        m_pEntry->Status.Resize= false;
        }

    return true;
WRAPUP:
    return false;
    }


// GetConvertedValue Methods
void HTIFFTagEntry::GetConvertedValues(vector<double>& po_rValues)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(m_pEntry->DirCount > 0);


    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    const uint32_t Count = m_pEntry->DirCount;

    if (po_rValues.capacity() < Count)
        po_rValues.reserve(Count);

    switch (m_pTagDef->GetDataType())
        {
        case HTagInfo::BYTE:
            po_rValues.assign(((Byte*)m_pEntry->pData), ((Byte*)m_pEntry->pData) + Count);
            break;
        case HTagInfo::SHORT:
            po_rValues.assign(((unsigned short*)m_pEntry->pData), ((unsigned short*)m_pEntry->pData) + Count);
            break;
        case HTagInfo::LONG:
            po_rValues.assign(((int32_t*)m_pEntry->pData), ((int32_t*)m_pEntry->pData) + Count);
            break;
        case HTagInfo::FLOAT:
            po_rValues.assign(((float*)m_pEntry->pData), ((float*)m_pEntry->pData) + Count);
            break;
        case HTagInfo::DOUBLE:
            po_rValues.assign(((double*)m_pEntry->pData), ((double*)m_pEntry->pData) + Count);
            break;
        case HTagInfo::RATIONAL:
            po_rValues.clear();
            po_rValues.reserve(Count);

            // Convert each rational numbers to double stocking to result in our output vector
            transform(  ((HTIFFURational32*)m_pEntry->pData),
                        ((HTIFFURational32*)m_pEntry->pData) + Count,
                        back_inserter(po_rValues),
                        mem_fun_ref(&HTIFFURational32::ConvertToDouble));
            break;
        default:
            HASSERT(!"Conversion is not yet implemented or there is no possible conversion");
            break;

        }
    }


// GetValues Methods
//
void HTIFFTagEntry::GetValues (Byte* po_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(m_pEntry->Status.NeedSwap == false);
    HPRECONDITION((HTagInfo::UNDEFINED == m_pTagDef->GetDataType()) ||
                  (HTagInfo::BYTE == m_pTagDef->GetDataType()));

    *po_pVal = *((Byte*)m_pEntry->pData);
    }


void HTIFFTagEntry::GetValues (unsigned short* po_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pVal = *((unsigned short*)m_pEntry->pData);
    }


void HTIFFTagEntry::GetValues (uint32_t* po_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION((HTagInfo::SHORT == m_pTagDef->GetDataType()) ||
                  (HTagInfo::LONG == m_pTagDef->GetDataType()) );

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }


    if (HTagInfo::SHORT == m_pTagDef->GetDataType())
        *po_pVal = (uint32_t)(*((unsigned short*)m_pEntry->pData));
    else
        *po_pVal = *((uint32_t*)m_pEntry->pData);
    }


void HTIFFTagEntry::GetValues (double* po_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION((HTagInfo::DOUBLE == m_pTagDef->GetDataType()) ||
                  (HTagInfo::RATIONAL == m_pTagDef->GetDataType()));

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pVal = *((double*)m_pEntry->pData);
    }

void HTIFFTagEntry::GetValues (uint64_t* po_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::LONG64 == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pVal = *((uint64_t*)m_pEntry->pData);
    }

void HTIFFTagEntry::GetValues (char** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::ASCII == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_ppVal = (char*)m_pEntry->pData;


    }

void HTIFFTagEntry::GetValues (WChar** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::ASCIIW == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_ppVal = (WChar*)m_pEntry->pData;
    }

void HTIFFTagEntry::GetValues (unsigned short* po_pVal1, unsigned short* po_pVal2)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pVal1 = ((unsigned short*)m_pEntry->pData)[0];
    *po_pVal2 = ((unsigned short*)m_pEntry->pData)[1];
    }

void HTIFFTagEntry::GetValues (uint32_t* po_pVal1, uint32_t* po_pVal2)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION((HTagInfo::RATIONAL == m_pTagDef->GetDataType()) ||
                  (HTagInfo::SRATIONAL == m_pTagDef->GetDataType()));

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pVal1 = ((uint32_t*)m_pEntry->pData)[0];
    *po_pVal2 = ((uint32_t*)m_pEntry->pData)[1];
    }


void HTIFFTagEntry::GetValues (uint32_t* po_pCount, unsigned short** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pCount = m_pEntry->DirCount;
    *po_ppVal  = (unsigned short*)m_pEntry->pData;
    }


void HTIFFTagEntry::GetValues (uint32_t* po_pCount, uint32_t** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION((HTagInfo::LONG == m_pTagDef->GetDataType()) ||
                  (HTagInfo::RATIONAL == m_pTagDef->GetDataType()));

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pCount = m_pEntry->DirCount;
    *po_ppVal  = (uint32_t*)m_pEntry->pData;
    }

void HTIFFTagEntry::GetValues (uint32_t* po_pCount, uint64_t** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::LONG64 == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pCount = m_pEntry->DirCount;
    *po_ppVal  = (uint64_t*)m_pEntry->pData;
    }

void HTIFFTagEntry::GetValues (uint32_t* po_pCount, double** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::DOUBLE == m_pTagDef->GetDataType() ||
                  HTagInfo::RATIONAL == m_pTagDef->GetDataType() ||
                  HTagInfo::SRATIONAL == m_pTagDef->GetDataType());

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pCount = m_pEntry->DirCount;
    *po_ppVal  = (double*)m_pEntry->pData;
    }

void HTIFFTagEntry::GetValues (const HTagInfo& pi_rTagInfo, uint32_t* po_pCount, Byte** po_ppVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);

    // HMR_DECIMATION_METHOD is define with 2 types (BYTE and LONG) but it is
    // read always as a byte.
    if(m_pTagDef->GetID() != pi_rTagInfo.GetHMRDecimationMethodTagID())
        {
        HPRECONDITION((HTagInfo::BYTE == m_pTagDef->GetDataType()) ||
                      (HTagInfo::UNDEFINED == m_pTagDef->GetDataType()) );
        }

    if (m_pEntry->Status.NeedSwap)
        {
        m_pEntry->Status.NeedSwap = false;
        SwapData (m_pEntry->pData);
        }

    *po_pCount = m_pEntry->DirCount;
    *po_ppVal  = (Byte*)m_pEntry->pData;
    }



bool HTIFFTagEntry::SetValues (unsigned short pi_Val)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    // Check the count field of a directory
    //
    if (!ValidateDataLen(1, false))
        goto WRAPUP;

    *((unsigned short*)m_pEntry->pData) = pi_Val;

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
WRAPUP:
    return false;
    }

bool HTIFFTagEntry::SetValues (uint32_t pi_Val)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::LONG == m_pTagDef->GetDataType());

    // Check the count field of a directory
    //
    if (!ValidateDataLen(1, false))
        goto WRAPUP;

    *((uint32_t*)m_pEntry->pData) = pi_Val;

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
WRAPUP:
    return false;
    }

bool HTIFFTagEntry::SetValues (double pi_Val)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::DOUBLE == m_pTagDef->GetDataType());

    // Check the count field of a directory
    //
    if (!ValidateDataLen(1, false))
        goto WRAPUP;

    *((double*)m_pEntry->pData) = pi_Val;

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
WRAPUP:
    return false;
    }

bool HTIFFTagEntry::SetValues (uint64_t pi_Val)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::LONG64 == m_pTagDef->GetDataType());

    // Check the count field of a directory
    //
    if (ValidateDataLen(1, false))
        {
        *((uint64_t*)m_pEntry->pData) = pi_Val;

        // Status, Dirty and Data is always in the current platform.
        m_pEntry->Status.Dirty          = true;
        m_pEntry->Status.NeedSwap       = false;

        return true;
        }
    else
        return false;
    }
bool HTIFFTagEntry::SetValuesA (const char*  pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::ASCII == m_pTagDef->GetDataType());

    SetArray ((uint32_t)strlen(pi_pVal)+1, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }

bool HTIFFTagEntry::SetValuesW (const WChar* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::ASCIIW == m_pTagDef->GetDataType());

    SetArray ((uint32_t)wcslen(pi_pVal)+1, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }

bool HTIFFTagEntry::SetValues (unsigned short pi_Val1,  unsigned short pi_Val2)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    // Check the count field of a directory
    //
    if (!ValidateDataLen(2, false))
        goto WRAPUP;

    ((unsigned short*)m_pEntry->pData)[0] = pi_Val1;
    ((unsigned short*)m_pEntry->pData)[1] = pi_Val2;

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
WRAPUP:
    return false;
    }

bool HTIFFTagEntry::SetValues (uint32_t pi_Val1,  uint32_t pi_Val2)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION((HTagInfo::RATIONAL == m_pTagDef->GetDataType()) ||
                  (HTagInfo::SRATIONAL == m_pTagDef->GetDataType()));

    // Check the count field of a directory
    //
    if (!ValidateDataLen(1, false))
        goto WRAPUP;

    ((uint32_t*)m_pEntry->pData)[0] = pi_Val1;
    ((uint32_t*)m_pEntry->pData)[1] = pi_Val2;

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
WRAPUP:
    return false;
    }

bool HTIFFTagEntry::SetValues (uint32_t pi_Count, const unsigned short* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::SHORT == m_pTagDef->GetDataType());

    SetArray (pi_Count, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }

bool HTIFFTagEntry::SetValues (uint32_t pi_Count, const uint32_t* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::LONG == m_pTagDef->GetDataType());

    SetArray (pi_Count, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }

bool HTIFFTagEntry::SetValues (uint32_t pi_Count, const uint64_t* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::LONG64 == m_pTagDef->GetDataType());

    SetArray (pi_Count, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }


bool HTIFFTagEntry::SetValues (uint32_t pi_Count, const double* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(HTagInfo::DOUBLE == m_pTagDef->GetDataType());

    SetArray (pi_Count, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }

bool HTIFFTagEntry::SetValues (const HTagInfo& pi_rTagInfo, uint32_t pi_Count, const Byte* pi_pVal)
    {
    HPRECONDITION(m_pEntry != 0);
    HPRECONDITION(m_pTagDef != 0);
    HPRECONDITION(pi_pVal != 0);

    // HMR_DECIMATION_METHOD is define with 2 types (BYTE and LONG) but it is
    // writed always as a byte.
    if(m_pTagDef->GetID() != pi_rTagInfo.GetHMRDecimationMethodTagID())
        {
        HPRECONDITION((HTagInfo::BYTE == m_pTagDef->GetDataType()) ||
                      (HTagInfo::UNDEFINED == m_pTagDef->GetDataType()) );
        }

    SetArray (pi_Count, (Byte*)pi_pVal);

    // Status, Dirty and Data is always in the current platform.
    m_pEntry->Status.Dirty          = true;
    m_pEntry->Status.NeedSwap       = false;

    return true;
    }



// ------------------------------------------------ Privates

bool HTIFFTagEntry::ValidateDataLen (uint64_t pi_Count, bool pi_Read)
    {
    // Check the count field of a directory
    //
    if (pi_Read)
        {
        if ((m_pTagDef->GetReadCount() != HTagInfo::TAG_IO_VARIABLE) &&
            (m_pTagDef->GetReadCount() != HTagInfo::TAG_IO_USE_SPP))
            {
            if ((uint32_t)m_pTagDef->GetReadCount() != pi_Count)
                {
                HTIFFError::BadTagCountIOErInfo ErInfo;
                string TagName(m_pTagDef->GetTagName());
                ErInfo.m_TagName       = WString(TagName.c_str(),false);
                ErInfo.m_Count         = pi_Count;
                ErInfo.m_ExpectedCount = m_pTagDef->GetReadCount();
                ErrorMsg(&m_pError, HTIFFError::INCORRECT_COUNT_FOR_TAG_READ, &ErInfo, false);

                goto WRAPUP;
                }
            }
        }
    else
        {
        if ((m_pTagDef->GetWriteCount() != HTagInfo::TAG_IO_VARIABLE) &&
            (m_pTagDef->GetWriteCount() != HTagInfo::TAG_IO_USE_SPP))
            {
            if ((uint32_t)m_pTagDef->GetWriteCount() != pi_Count)
                {
                HTIFFError::BadTagCountIOErInfo ErInfo;

                string TagName(m_pTagDef->GetTagName());
                ErInfo.m_TagName       = WString(TagName.c_str(),false);
                ErInfo.m_Count         = pi_Count;
                ErInfo.m_ExpectedCount = m_pTagDef->GetWriteCount();
                ErrorMsg(&m_pError, HTIFFError::INCORRECT_COUNT_FOR_TAG_WRITTEN, &ErInfo, false);
                goto WRAPUP;
                }
            }
        else
            // The standard support Count64, not me
            HASSERT(pi_Count < ULONG_MAX);
        m_pEntry->DirCount = (uint32_t)pi_Count;
        }

    return true;
WRAPUP:
    return false;
    }


void HTIFFTagEntry::SwapData (void* pio_pData)
    {
    switch (m_pTagDef->GetDataType())
        {
        case HTagInfo::SHORT:
        case HTagInfo::SSHORT:
        case HTagInfo::ASCIIW:
            SwabArrayOfShort((unsigned short*)pio_pData, m_pEntry->DirCount);
            break;
        case HTagInfo::LONG:
        case HTagInfo::SLONG:
        case HTagInfo::FLOAT:
            SwabArrayOfLong((uint32_t*)pio_pData, m_pEntry->DirCount);
            break;
        case HTagInfo::RATIONAL:
        case HTagInfo::SRATIONAL:
            SwabArrayOfLong((uint32_t*)pio_pData, 2*m_pEntry->DirCount);
            break;
        case HTagInfo::DOUBLE:
            SwabArrayOfDouble((double*)pio_pData, m_pEntry->DirCount);
            break;
        case HTagInfo::LONG64:
        case HTagInfo::SLONG64:
        case HTagInfo::IFD64:
            SwabArrayOfUInt64((uint64_t*)pio_pData, m_pEntry->DirCount);
            break;
        }
    }


void HTIFFTagEntry::SetArray (uint32_t pi_DirCount, const Byte* pi_pData)
    {
    uint32_t NbByte = pi_DirCount * m_pTagDef->GetDataLen();

    // If Size is differrent, Realloc the Data field
    //
    if (pi_DirCount != m_pEntry->DirCount)
        {
        // Delete pData if Allocated
        if ((m_pEntry->pData != 0) && !m_pEntry->Status.DataInOffset)
            delete[] m_pEntry->pData;

        m_pEntry->DirCount = pi_DirCount;

        m_pEntry->pData = new Byte[NbByte];

        // Specify, possibly resize
        m_pEntry->Status.Resize       = true;
        m_pEntry->Status.DataInOffset = false;
        }

    // If same pointer, don't copy, because the User can't change this
    // pointer.
    if ((Byte*)m_pEntry->pData != pi_pData)
        HFCMemcpy(m_pEntry->pData, pi_pData, NbByte);
    }


/**----------------------------------------------------------------------------
 Make sure that the tag data isn't using space that is marked as free
 (to support files created by buggy code :)
-----------------------------------------------------------------------------*/
void HTIFFTagEntry::ValidateTagDataAddress(const HTagInfo& pi_rTagInfo, HTIFFStream* pi_pFile)
    {
    if ((GetTagID() == pi_rTagInfo.GetFreeOffsetsTagID() || GetTagID() == pi_rTagInfo.GetFreeByteCountsTagID()) &&
        pi_pFile->OverlapsFreeBlocks(m_pEntry->Offset64, m_pEntry->SizeInFile))
        {
        // Force resizing, which will reallocate the space.
        m_pEntry->Status.Resize = true;
        m_pEntry->Offset64 = 0;
        m_pEntry->SizeInFile = 0;
        }
    }


uint64_t HTIFFTagEntry::GetTagOffSet()
    {
    return m_pEntry->Offset64;
    }

uint64_t HTIFFTagEntry::GetTagSize()
    {
    return m_pEntry->SizeInFile;
    }

void HTIFFTagEntry::SetTagOffSet(uint64_t p_NewOffSet)
    {
    m_pEntry->Offset64 = p_NewOffSet;
    }