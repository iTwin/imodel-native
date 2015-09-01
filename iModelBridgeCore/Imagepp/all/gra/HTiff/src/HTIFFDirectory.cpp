//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFDirectory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFDirectory
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HTIFFDirectory.h>
#include <Imagepp/all/h/HTIFFTagEntry.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HTIFFGeoKey.h>

#include <ImagePP/all/h/HTagIdIterator.h>

//-----------------------------------------------------------------------------
// public
// Constructor, Create an empty entry.
//-----------------------------------------------------------------------------
HTIFFDirectory::HTIFFDirectory(const HTagInfo& pi_rTagInfo, const HTIFFByteOrdering* pi_pByteOrder, bool pi_IsTiff64)
    :   m_rTagInfo(pi_rTagInfo)
    {
    m_pError = 0;
    m_pByteOrder = (HTIFFByteOrdering*)pi_pByteOrder;
    m_IsTiff64   = pi_IsTiff64;
    m_NextDirectoryOffsetIsInvalid = false;

    const uint32_t TagQty = m_rTagInfo.GetTagQty();
    m_ppDirEntry = new HTIFFTagEntry*[TagQty];
    HASSERT(m_ppDirEntry != 0);
    memset(m_ppDirEntry, 0, sizeof(HTIFFTagEntry*) * TagQty);
    m_DirCount          = 0;

    memset(&m_Status, 0, sizeof(DirStatus));

    // GeoTiff information,  Default
    m_pGeoKeys = new HTIFFGeoKey();
    }



//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HTIFFDirectory::~HTIFFDirectory()
    {
    Reset ();

    delete[] m_ppDirEntry;
    }



//-----------------------------------------------------------------------------
// public
// ReadDirectory - Read the directory at the specify file position.
//
//      - Return the offset of the next Directory or 0
//      - 0 if error, check with IsValid() method.
//-----------------------------------------------------------------------------
uint64_t HTIFFDirectory::ReadDirectory (HTIFFStream* pi_pFile, uint64_t pi_Offset)
    {
    HPRECONDITION(pi_pFile != 0);

    uint64_t       Index;
    uint64_t       NextDirOffset = 0;
    HTIFFTagEntry*  pEntry;
    uint64_t       OffsetNextDir;
    bool           NextDirExist = true;
    const uint32_t  TagQty = m_rTagInfo.GetTagQty();

    // Reset Directory
    Reset();

    // Read the DirCount (number of Entry)
    bool ReadFail = !pi_pFile->Seek(pi_Offset, SEEK_SET);
    if (pi_pFile->m_IsTiff64)
        {
        uint64_t Count;
        ReadFail |= (pi_pFile->Read(&Count, sizeof(uint64_t), 1) != 1);
        if (m_pByteOrder->NeedSwapByte())
            SwabArrayOfUInt64(&Count, 1);

        // The standard support Count64, not me
        HASSERT(Count < ULONG_MAX);
        m_DirCount = (uint32_t)Count;

        // Support old tiff file, without marker for next directory
        // Compute the adresse of the pointer on the next directory
        OffsetNextDir = pi_Offset + sizeof(uint64_t) +
                        (m_DirCount*FileDirEntry64_size);

        }
    else
        {
        unsigned short Count;
        ReadFail |= (pi_pFile->Read(&Count, sizeof(unsigned short), 1) != 1);
        if (m_pByteOrder->NeedSwapByte())
            SwabArrayOfShort(&Count, 1);
        m_DirCount = Count;

        // Support old tiff file, without marker for next directory
        // Compute the adresse of the pointer on the next directory
        OffsetNextDir = pi_Offset + sizeof(unsigned short) +
                        (m_DirCount*FileDirEntry32_size);

        }

    uint32_t DirCount = m_DirCount;
    if (ReadFail)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_DIR_COUNT, 0, true);
        goto WRAPUP;
        }

    // Read Tag Information, Read all before read Tag Data(performance)
    for (Index=0; Index<DirCount; Index++)
        {
        pEntry = new HTIFFTagEntry(m_rTagInfo, m_pByteOrder);

        if (!pEntry->ReadTagEntry(m_rTagInfo, pi_pFile))
            {
            HTIFFError* pErrorInfo;
            pEntry->IsValid(&pErrorInfo);       // Keep the "delete pEntry" in place, the method IsValid
            // copy a pointer form pEntry to pErrorInfo.

            if (ErrorMsg(&m_pError, *pErrorInfo))
                {
                delete pEntry;
                goto WRAPUP;
                }
            m_DirCount--;           // Tag ignore.

            delete pEntry;
            }
        else
            {
            m_ppDirEntry[pEntry->GetTagID()] = pEntry;

            // Support old tiff file, without marker for next directory
            // If one entry has the same address of pointer of next dir
            // --> Don't have an other directory.
            //   Possibly we need to check also, all data offset, from strip/tile
            //   offset.
            if (pEntry->m_pEntry->Offset64 == OffsetNextDir)
                NextDirExist = false;
            }
        }

    // Read next Dir offset
    if (NextDirExist)
        {
        bool ReadFail;
        if (pi_pFile->m_IsTiff64)
            {
            ReadFail = (pi_pFile->Read(&NextDirOffset, sizeof(uint64_t), 1) != 1);

            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfUInt64(&NextDirOffset, 1);
            }
        else
            {
            uint32_t Offset;
            ReadFail = (pi_pFile->Read(&Offset, sizeof(uint32_t), 1) != 1);

            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfLong(&Offset, 1);

            NextDirOffset = Offset;
            }
        if (ReadFail)
            // Special case, only one directory, without End marker
            NextDirOffset = 0;
        }

    // Read All Data Tag
    for(Index=0; Index<TagQty; Index++)
        {
        if (m_ppDirEntry[Index] != 0)
            {
            if (!m_ppDirEntry[Index]->ReadData(m_rTagInfo, pi_pFile))
                {
                HTIFFError* pErrorInfo;
                m_ppDirEntry[Index]->IsValid(&pErrorInfo);
                if (ErrorMsg(&m_pError, *pErrorInfo))
                    {
                    NextDirOffset = 0;  // Error
                    goto WRAPUP;
                    }
                }
            }
        }

    // GeoTiff information
    //
    if (TagIsPresent(m_rTagInfo.GetGeoKeyDirectoryTagID()))
        {
        unsigned short* pKeyDirectory  = 0;
        uint32_t KeyCount       = 0;
        double* pDoubleParams  = 0;
        uint32_t DoubleCount    = 0;
        char*   pASCIIParams   = 0;

        GetValues (m_rTagInfo.GetGeoKeyDirectoryTagID(), &KeyCount, &pKeyDirectory);
        GetValues (m_rTagInfo.GetGeoDoubleParamsTagID(), &DoubleCount, &pDoubleParams);
        GetValues (m_rTagInfo.GetGeoAsciiParamsTagID(), &pASCIIParams);
        m_pGeoKeys = new HTIFFGeoKey(pKeyDirectory, KeyCount,
                                     pDoubleParams, DoubleCount,
                                     pASCIIParams);

        // Error Msg
        HTIFFError* pErrorInfo;
        if (!m_pGeoKeys->IsValid(&pErrorInfo))
            ErrorMsg(&m_pError, *pErrorInfo);
        }
    else
        m_pGeoKeys = new HTIFFGeoKey();

WRAPUP:
    return NextDirOffset;
    }




//-----------------------------------------------------------------------------
// public
// WriteDirectory - Write the directory at the specify Offset if the directory
//                  can be write at this offset, otherwise it is write at an
//                  other offset.
//
//      - The parameter pio_pOffset can change if the DIrectory has been changed.
//      - Return the offset in the field to link Directory, this must be pass
//        to the next WriteDirectory, to link All directory together.
//      - pi_NextDirIfKnown : 0 if don't know the next offset directory.
//
//      - 0 --> Error.
//-----------------------------------------------------------------------------
uint64_t HTIFFDirectory::WriteDirectory (HTIFFStream* pio_pFile,
                                        uint64_t*     pio_pOffset,
                                        uint64_t    pi_OffsetPreviousDirLink,
                                        uint64_t    pi_NextDirIfKnown)
    {
    HPRECONDITION(pio_pFile != 0);

    uint32_t        Index;
    uint64_t       LinkDirOffset = 0;
    const uint32_t  TagQty = m_rTagInfo.GetTagQty();
    //
    // GeoTiff information
    if ((m_pGeoKeys != 0) && m_pGeoKeys->IsDirty())
        {
        unsigned short* pKeyDirectory;
        uint32_t KeyCount;
        double* pDoubleParams;
        uint32_t DoubleCount;
        char*   pASCIIParams;

        m_pGeoKeys->GetGeoParams(&pKeyDirectory, &KeyCount,
                                 &pDoubleParams, &DoubleCount,
                                 &pASCIIParams);

        SetValues (m_rTagInfo.GetGeoKeyDirectoryTagID(), KeyCount, pKeyDirectory);
        delete[] pKeyDirectory;

        if (pDoubleParams != 0)
            {
            SetValues (m_rTagInfo.GetGeoDoubleParamsTagID(), DoubleCount, pDoubleParams);
            delete[] pDoubleParams;
            }
        else
            Remove(m_rTagInfo.GetGeoDoubleParamsTagID());

        if (pASCIIParams != 0)
            {
            SetValuesA (m_rTagInfo.GetGeoAsciiParamsTagID(), pASCIIParams);
            delete[] pASCIIParams;
            }
        else
            Remove(m_rTagInfo.GetGeoAsciiParamsTagID());

        }

    // We must write the directory ?
    if (m_Status.Dirty)
        {
        // Obtain the Offset for the Directory.
        // If the Directory is New or his size had changed, realloc a new
        // Offset
        if (m_Status.Resize || (*pio_pOffset == 0))
            {
            // Write the Dir at the end, (?? MemManager if possible)
            pio_pFile->Seek(0L, SEEK_END);
            *pio_pOffset = (pio_pFile->Tell() + 1) &~ 1;    // Word Boundary

            // Initialize the byte skipped to go to a Word boundary, otherwise the ATPs say Different randomly.
            if ((pio_pFile->Tell() & 1) != 0)
                {
                Byte ZeroValue = 0;
                pio_pFile->Write(&ZeroValue, sizeof(Byte), 1);
                }

            m_Status.Resize = false;
            }

        // Skip the space for the Directory, because we must support old HMR file,
        // In Old HMR file, the Data tag must be write directly after the Directory.
        uint32_t TmpBufSize;
        if (pio_pFile->m_IsTiff64)
            TmpBufSize = sizeof(uint64_t) + sizeof(uint64_t) +    // Count and Link fields
                         (m_DirCount*FileDirEntry64_size);
        else
            TmpBufSize = sizeof(unsigned short) + sizeof(uint32_t) +     // Count and Link fields
                         (m_DirCount*FileDirEntry32_size);

        HArrayAutoPtr<Byte> pTmpBuffer(new Byte[TmpBufSize]);
        if (!pio_pFile->Seek(*pio_pOffset, SEEK_SET) ||
            pio_pFile->Write(pTmpBuffer, sizeof(Byte), TmpBufSize) != TmpBufSize)
            {
            ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_DIR_BUFFER, 0, true);
            goto WRAPUP;
            }


        // Write All Data Tag
        // because the offset of Data must be know in the Tag Offset before to
        // write it.
        for(Index=0; Index<TagQty; Index++)
            {
            // Skip the Tag, if not saved in file...
            if ((m_ppDirEntry[Index] != 0) &&
                (m_ppDirEntry[Index]->GetTagID() < m_rTagInfo.GetNotSavedTagIDBegin()) )
                {
                if(!m_ppDirEntry[Index]->WriteData(m_rTagInfo, pio_pFile))
                    {
                    HTIFFError* pErrorInfo;
                    m_ppDirEntry[Index]->IsValid(&pErrorInfo);
                    if(ErrorMsg(&m_pError, *pErrorInfo))
                        goto WRAPUP;
                    }
                }
            }

        // Write the Directory, in the previous position.
        // Write the DirCount (number of Entry)
        bool WriteFail = !pio_pFile->Seek(*pio_pOffset, SEEK_SET);
        if (pio_pFile->m_IsTiff64)
            {
            uint64_t DirCount = m_DirCount;
            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfUInt64(&DirCount, 1);

            WriteFail |= (pio_pFile->Write(&DirCount, sizeof(uint64_t), 1) != 1);
            }
        else
            {
            unsigned short DirCount = (unsigned short)m_DirCount;
            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfShort(&DirCount, 1);

            WriteFail |= (pio_pFile->Write(&DirCount, sizeof(unsigned short), 1) != 1);
            }
        if (WriteFail)
            {
            ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_DIR_COUNT, 0, true);
            goto WRAPUP;
            }

        // Write Tag Information, Write all before Write Tag Data(performance)
        for(Index=0; Index<TagQty; Index++)
            {
            if ((m_ppDirEntry[Index] != 0) &&
                (m_ppDirEntry[Index]->GetTagID() < m_rTagInfo.GetNotSavedTagIDBegin()) )
                {
                if(!m_ppDirEntry[Index]->WriteTagEntry(m_rTagInfo, pio_pFile))
                    {
                    HTIFFError* pErrInfo;
                    m_ppDirEntry[Index]->IsValid(&pErrInfo);
                    if (ErrorMsg(&m_pError, *pErrInfo))
                        goto WRAPUP;
                    }
                }
            }

        // Take Offset field Link Dir
        LinkDirOffset = pio_pFile->Tell();

        // Write next Dir offset, 0 by default
        //
        WriteFail = false;
        if (pio_pFile->m_IsTiff64)
            {
            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfUInt64(&pi_NextDirIfKnown, 1);

            WriteFail |= (pio_pFile->Write(&pi_NextDirIfKnown, sizeof(uint64_t), 1) != 1);
            }
        else
            {
            uint32_t Offset = (uint32_t)pi_NextDirIfKnown;
            if (m_pByteOrder->NeedSwapByte())
                SwabArrayOfLong(&Offset, 1);

            WriteFail |= (pio_pFile->Write(&Offset, sizeof(uint32_t), 1) != 1);
            }
        if (WriteFail)
            {
            ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_DIR_NEXT_DIR_OFFSET, 0, true);
            LinkDirOffset = 0;  // Error
            goto WRAPUP;
            }
        m_NextDirectoryOffsetIsInvalid = false;      // Next Dir Offset was updated.

        // Link the Directory with the Previous
        if (pi_OffsetPreviousDirLink != 0)
            {
            WriteFail = !pio_pFile->Seek(pi_OffsetPreviousDirLink, SEEK_SET);
            if (pio_pFile->m_IsTiff64)
                {
                uint64_t Offset = *pio_pOffset;      // We don't want to swap pio_pOffset paramter.
                // Swap before Write if Needed
                if (m_pByteOrder->NeedSwapByte())
                    SwabArrayOfUInt64(&Offset, 1);
                WriteFail |= (pio_pFile->Write(&Offset, sizeof(uint64_t), 1) != 1);
                }
            else
                {
                uint32_t Offset = (uint32_t)*pio_pOffset;
                // Swap before Write if Needed
                if (m_pByteOrder->NeedSwapByte())
                    SwabArrayOfLong(&Offset, 1);
                WriteFail |= (pio_pFile->Write(&Offset, sizeof(uint32_t), 1) != 1);
                }
            if (WriteFail)
                {
                ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_DIR_LINK, 0, true);
                goto WRAPUP;
                }
            }

        m_Status.Dirty = false;
        }
    else
        {
        // Don't write the Directory, but I need to know the link
        // field Offset.                 DirCount            Tags
        if (pio_pFile->m_IsTiff64)
            LinkDirOffset = *pio_pOffset + sizeof(uint64_t) + (m_DirCount*FileDirEntry64_size);
        else
            LinkDirOffset = *pio_pOffset + sizeof(unsigned short) + (m_DirCount*FileDirEntry32_size);
        }


WRAPUP:
    return LinkDirOffset;
    }

// ConvertRationalToDblValues
// This function convert rational 64 bits tag values to double values.
void HTIFFDirectory::ConvertRationalToDblValues (uint32_t   pi_Count,
                                                 double*   pi_RationalVals,
                                                 double*   po_pVal,
                                                 bool      pi_IsSigned /*false*/)
    {
    for (unsigned short ValInd = 0; ValInd < pi_Count; ValInd++)
        {
        if ((((uint32_t*)pi_RationalVals)[0] == 0) &&
            (((uint32_t*)pi_RationalVals)[1] == 0))
            {
            *po_pVal = 0;
            }
        else if (((uint32_t*)pi_RationalVals)[1] != 0)
            {
            if (pi_IsSigned)
                {
                *po_pVal = ((int32_t*)pi_RationalVals)[0] / (double)((uint32_t*)pi_RationalVals)[1];
                }
            else
                {
                *po_pVal = ((uint32_t*)pi_RationalVals)[0] / (double)((uint32_t*)pi_RationalVals)[1];
                }
            }
        else
            {   //Division by 0
            HASSERT(0);
            *po_pVal = 0;
            }

        po_pVal++;
        pi_RationalVals++;
        }
    }

// GetConvertedValues Methods
//
bool HTIFFDirectory::GetConvertedValues (HTagID pi_Tag, vector<double>& po_rValues)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetConvertedValues(po_rValues);
        return true;
        }
    else
        return false;
    }


// GetValues Methods
//
bool HTIFFDirectory::GetValues (HTagID pi_Tag, Byte* po_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, unsigned short* po_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal);
        return true;
        }
    else
        return false;
    }


bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal);
        return true;
        }
    else
        return false;
    }


bool HTIFFDirectory::GetValues (HTagID pi_Tag, double* po_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint64_t* po_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal != 0);
    HPRECONDITION(m_IsTiff64 != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, char** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_ppVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, WChar** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_ppVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, unsigned short* po_pVal1, unsigned short* po_pVal2)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal1 != 0);
    HPRECONDITION(po_pVal2 != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal1, po_pVal2);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pVal1, uint32_t* po_pVal2)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pVal1 != 0);
    HPRECONDITION(po_pVal2 != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pVal1, po_pVal2);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pCount, po_ppVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pCount, uint32_t** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0 && (HTagInfo::SHORT == m_ppDirEntry[pi_Tag]->GetTagType() ||
                                      HTagInfo::LONG == m_ppDirEntry[pi_Tag]->GetTagType() ||
                                      HTagInfo::RATIONAL == m_ppDirEntry[pi_Tag]->GetTagType()))
        {
        // If Tag is SHORT, convert it in LONG if possible.
        // normally SHORT is the Old type for the TAG.
        if (HTagInfo::SHORT == m_ppDirEntry[pi_Tag]->GetTagType())
            ConvertSHORTtoULONG (pi_Tag);

        m_ppDirEntry[pi_Tag]->GetValues(po_pCount, po_ppVal);
        return true;
        }
    else
        return false;
    }

bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pCount, uint64_t** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HPRECONDITION(m_IsTiff64 != 0);

    if((m_ppDirEntry[pi_Tag] != 0) && (HTagInfo::LONG64 == m_ppDirEntry[pi_Tag]->GetTagType() ||
                                       HTagInfo::SHORT == m_ppDirEntry[pi_Tag]->GetTagType() ||
                                       HTagInfo::LONG == m_ppDirEntry[pi_Tag]->GetTagType()))
        {
        if (HTagInfo::LONG64 != m_ppDirEntry[pi_Tag]->GetTagType())
            ConvertToLONG64 (pi_Tag);

        m_ppDirEntry[pi_Tag]->GetValues(po_pCount, po_ppVal);
        return true;
        }
    else
        return false;
    }


bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pCount, double** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(po_pCount, po_ppVal);
        return true;
        }
    else
        return false;
    }


bool HTIFFDirectory::GetValues (HTagID pi_Tag, uint32_t* po_pCount, Byte** po_ppVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);

    if (m_ppDirEntry[pi_Tag] != 0)
        {
        m_ppDirEntry[pi_Tag]->GetValues(m_rTagInfo, po_pCount, po_ppVal);
        return true;
        }
    else
        return false;
    }


bool HTIFFDirectory::SetValues (HTagID pi_Tag, unsigned short pi_Val)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Val)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, double pi_Val)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint64_t pi_Val)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(m_IsTiff64 != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val);
WRAPUP:
    return false;
    }


bool HTIFFDirectory::SetValuesA (HTagID pi_Tag, const char*  pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValuesA(pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValuesW (HTagID pi_Tag, const WChar*  pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValuesW(pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, unsigned short pi_Val1,  unsigned short pi_Val2)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val1, pi_Val2);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Val1,  uint32_t pi_Val2)
    {
    HPRECONDITION(m_ppDirEntry != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Val1, pi_Val2);
WRAPUP:
    return false;
    }


bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Count, pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Count, const uint32_t* pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Count, pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Count, const double* pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Count, pi_pVal);
WRAPUP:
    return false;
    }


bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Count, const Byte* pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    // Special cases
    // Old files have this tag stored in Long, now, it is always save in Byte.
    if (pi_Tag == m_rTagInfo.GetHMRDecimationMethodTagID() && m_ppDirEntry[pi_Tag]->GetTagType() == HTagInfo::LONG)
        {
        Remove(m_rTagInfo.GetHMRDecimationMethodTagID());
        if (!AddEntry(m_rTagInfo.GetHMRDecimationMethodTagID()))          // Create a new one in Byte
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(m_rTagInfo, pi_Count, pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::SetValues (HTagID pi_Tag, uint32_t pi_Count, const uint64_t* pi_pVal)
    {
    HPRECONDITION(m_ppDirEntry != 0);
    HPRECONDITION(pi_pVal != 0);
    HPRECONDITION(m_IsTiff64 != 0);

    m_Status.Dirty = true;

    // Tag Already in directory
    if (m_ppDirEntry[pi_Tag] == 0)
        {
        if (!AddEntry(pi_Tag))
            goto WRAPUP;
        }

    return m_ppDirEntry[pi_Tag]->SetValues(pi_Count, pi_pVal);
WRAPUP:
    return false;
    }

bool HTIFFDirectory::Remove (HTagID pi_Tag)
    {
    if (TagIsPresent(pi_Tag))
        {
        m_Status.Dirty = true;

        // Don't set resize, because normally enough space at the same place.
        //m_Status.Resize = true;

        // Increment number of entry
        m_DirCount--;

        delete m_ppDirEntry[pi_Tag];
        m_ppDirEntry[pi_Tag] = 0;
        return true;
        }
    else
        return false;
    }


HTagIdIter HTIFFDirectory::TagIDBegin () const
    {
    const uint32_t TagQty = m_rTagInfo.GetTagQty();
    return HTagIdIter(m_ppDirEntry, m_ppDirEntry, m_ppDirEntry + TagQty);
    }

HTagIdIter HTIFFDirectory::TagIDEnd () const
    {
    const uint32_t TagQty = m_rTagInfo.GetTagQty();
    return HTagIdIter(m_ppDirEntry + TagQty, m_ppDirEntry, m_ppDirEntry + TagQty);
    }


// ------------------------------------------------ Privates

void HTIFFDirectory::Reset ()
    {
    delete m_pError;
    m_pError = 0;

    const uint32_t TagQty = m_rTagInfo.GetTagQty();
    for (uint32_t i=0; i<TagQty; i++)
        {
        if (m_ppDirEntry[i] != 0)
            {
            delete m_ppDirEntry[i];
            m_ppDirEntry[i] = 0;
            }
        }

    delete m_pGeoKeys;
    m_pGeoKeys = 0;
    }



bool HTIFFDirectory::AddEntry (HTagID pi_Tag)
    {
    HPRECONDITION(0 == m_ppDirEntry[pi_Tag]);
    HAutoPtr<HTIFFTagEntry>  pEntry(new HTIFFTagEntry(m_rTagInfo, pi_Tag, m_pByteOrder, m_IsTiff64));

    HTIFFError* pErrInfo;
    if (pEntry->IsValid(&pErrInfo))
        {
        // Skip not file Tags, that means Tag > 65535
        if(pEntry->GetTagID() < m_rTagInfo.GetNotSavedTagIDBegin())
            {
            m_Status.Resize = true;

            // Increment number of entry
            HASSERT(m_DirCount < ULONG_MAX);
            m_DirCount++;
            }

        m_ppDirEntry[pi_Tag] = pEntry.release();

        return true;
        }
    else
        {
        ErrorMsg(&m_pError, *pErrInfo);
        return false;
        }
    }



void HTIFFDirectory::ConvertSHORTtoULONG (HTagID pi_Tag)
    {
    HPRECONDITION(0 != m_ppDirEntry[pi_Tag]);

    // Create a new Tag
    HAutoPtr<HTIFFTagEntry> pEntry(new HTIFFTagEntry(m_rTagInfo, pi_Tag, m_pByteOrder, m_IsTiff64));

    HTIFFError* pMsg;
    if (pEntry->IsValid(&pMsg))
        {
        // New TAG valid, convert it
        //
        if (HTagInfo::LONG == pEntry->GetTagType())
            {
            unsigned short* pVal;
            uint32_t Count;
            m_ppDirEntry[pi_Tag]->GetValues(&Count, &pVal);

            uint32_t* pLong = new uint32_t[Count];
            for(uint32_t i=0; i<Count; i++)
                pLong[i] = (uint32_t)pVal[i];

            // Delete Old entry
            delete m_ppDirEntry[pi_Tag];

            // Set the new Entry
            m_ppDirEntry[pi_Tag] = pEntry.release();
            SetValues(pi_Tag, Count, pLong);

            delete[] pLong;
            }
        }
    }

void HTIFFDirectory::ConvertToLONG64 (HTagID pi_Tag)
    {
    HPRECONDITION(0 != m_ppDirEntry[pi_Tag]);

    // Create a new Tag
    HAutoPtr<HTIFFTagEntry> pEntry(new HTIFFTagEntry(m_rTagInfo, pi_Tag, m_pByteOrder, true));

    HTIFFError* pMsg;
    if (pEntry->IsValid(&pMsg))
        {
        // New TAG valid, convert it
        //
        if (HTagInfo::LONG64 == pEntry->GetTagType())
            {
            uint64_t* pLong64 = NULL;
            uint32_t Count=0;

            if (HTagInfo::SHORT == m_ppDirEntry[pi_Tag]->GetTagType())
                {
                unsigned short* pVal;
                m_ppDirEntry[pi_Tag]->GetValues(&Count, &pVal);

                pLong64 = new uint64_t[Count];
                for(uint32_t i=0; i<Count; i++)
                    pLong64[i] = (uint64_t)pVal[i];
                }
            else if (HTagInfo::LONG == m_ppDirEntry[pi_Tag]->GetTagType())
                {
                uint32_t* pVal;
                m_ppDirEntry[pi_Tag]->GetValues(&Count, &pVal);

                pLong64 = new uint64_t[Count];
                for(uint32_t i=0; i<Count; i++)
                    pLong64[i] = (uint64_t)pVal[i];
                }

            // Delete Old entry
            delete m_ppDirEntry[pi_Tag];

            // Set the new Entry
            m_ppDirEntry[pi_Tag] = pEntry.release();
            SetValues(pi_Tag, Count, pLong64);

            delete[] pLong64;
            }
        }
    }

/**----------------------------------------------------------------------------
 Validate the 2 tags used to maintain the list of free blocks. Here because
 some TIFF files were created before a bug was fixed...
-----------------------------------------------------------------------------*/
void HTIFFDirectory::ValidateFreeBlockTags(HTIFFStream* pi_pFile)
    {
    if (m_ppDirEntry[m_rTagInfo.GetFreeOffsetsTagID()] != 0 &&
        m_ppDirEntry[m_rTagInfo.GetFreeOffsetsTagID()]->GetTagID() < m_rTagInfo.GetNotSavedTagIDBegin())
        m_ppDirEntry[m_rTagInfo.GetFreeOffsetsTagID()]->ValidateTagDataAddress(m_rTagInfo, pi_pFile);

    if (m_ppDirEntry[m_rTagInfo.GetFreeByteCountsTagID()] != 0 &&
        m_ppDirEntry[m_rTagInfo.GetFreeByteCountsTagID()]->GetTagID() < m_rTagInfo.GetNotSavedTagIDBegin())
        m_ppDirEntry[m_rTagInfo.GetFreeByteCountsTagID()]->ValidateTagDataAddress(m_rTagInfo, pi_pFile);
    }


uint64_t HTIFFDirectory::GetDirectorySize(HTIFFStream* pi_pFile, uint64_t pi_Offset)
    {
    bool    ReadFail = !pi_pFile->Seek(pi_Offset, SEEK_SET);
    uint64_t DirectorySize;

    if (pi_pFile->m_IsTiff64)
        {
        uint64_t Count;
        ReadFail |= (pi_pFile->Read(&Count, sizeof(uint64_t), 1) != 1);
        if (m_pByteOrder->NeedSwapByte())
            SwabArrayOfUInt64(&Count, 1);

        // The standard support Count64, not me
        HASSERT(Count < ULONG_MAX);
        m_DirCount = (uint32_t)Count;

        // Support old tiff file, without marker for next directory
        // Compute the adresse of the pointer on the next directory
        DirectorySize = sizeof(uint64_t) + sizeof(uint64_t) +
                        (m_DirCount*FileDirEntry64_size);

        }
    else
        {
        unsigned short Count;
        ReadFail |= (pi_pFile->Read(&Count, sizeof(unsigned short), 1) != 1);
        if (m_pByteOrder->NeedSwapByte())
            SwabArrayOfShort(&Count, 1);
        m_DirCount = (uint32_t)Count;

        DirectorySize = sizeof(uint32_t) + sizeof(unsigned short) +
                        (m_DirCount*FileDirEntry32_size);

        }
    if (ReadFail)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_DIR_COUNT, 0, true);
        }

    return DirectorySize;
    }

//-----------------------------------------------------------------------------
// public
// GetListTagPerDirectory()
//-----------------------------------------------------------------------------
void HTIFFDirectory::GetListTagPerDirectory(HTIFFStream*                pi_pFile,
                                            vector<OffsetAndSize>&      p_TagList,
                                            bool                       isHMRDir,
                                            uint32_t                    p_NoDir)
    {
    OffsetAndSize temp;

    const uint32_t TagQty = m_rTagInfo.GetTagQty();
    for(uint32_t Index=0; Index < TagQty; Index++)
        {
        if (m_ppDirEntry[Index] != 0 && (Index != m_rTagInfo.GetFreeOffsetsTagID() && Index != m_rTagInfo.GetFreeByteCountsTagID()))
            {
            if (m_ppDirEntry[Index]->GetTagSize() != 0)
                {
                temp.Offset   = m_ppDirEntry[Index]->GetTagOffSet();
                temp.Dir      = p_NoDir;
                temp.Size     = m_ppDirEntry[Index]->GetTagSize();
                temp.Position = Index;
                temp.isATag   = true;

                if (isHMRDir)
                    temp.isHMRDir = true;
                else
                    temp.isHMRDir = false;

                p_TagList.push_back(temp);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// SetEntryOffset(UInt32 p_PositionTag, UInt64 p_NewOffset)
//-----------------------------------------------------------------------------
void HTIFFDirectory::SetEntryOffset(uint32_t p_PositionTag, uint64_t p_NewOffset)
    {
    m_ppDirEntry[p_PositionTag]->SetTagOffSet(p_NewOffset);
    }

//-----------------------------------------------------------------------------
// public
// GetFreeBlockInfo(UInt64& p_SizeFreeOffset,UInt64& p_OffsetFreeOffset,
//                  UInt64& p_SizeFreebytecounts, UInt64& p_OffsetFreebytecounts)
//-----------------------------------------------------------------------------
void HTIFFDirectory::GetFreeBlockInfo(uint64_t& p_SizeFreeOffset,
                                      uint64_t& p_OffsetFreeOffset,
                                      uint64_t& p_SizeFreebytecounts,
                                      uint64_t& p_OffsetFreebytecounts)
    {
    p_OffsetFreeOffset     = m_ppDirEntry[m_rTagInfo.GetFreeOffsetsTagID()]->GetTagOffSet();
    p_SizeFreeOffset       = m_ppDirEntry[m_rTagInfo.GetFreeOffsetsTagID()]->GetTagSize();
    p_OffsetFreebytecounts = m_ppDirEntry[m_rTagInfo.GetFreeByteCountsTagID()]->GetTagOffSet();
    p_SizeFreebytecounts   = m_ppDirEntry[m_rTagInfo.GetFreeByteCountsTagID()]->GetTagSize();
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HTIFFDirectory::SetDirty()
    {
    m_Status.Dirty = true;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HTIFFDirectory::NextDirectoryOffsetIsInvalid()
    {
    return m_NextDirectoryOffsetIsInvalid;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HTIFFDirectory::SetNextDirectoryOffsetIsInvalid(bool pi_InvalidDir)
    {
    m_NextDirectoryOffsetIsInvalid = pi_InvalidDir;
    }
