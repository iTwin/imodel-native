//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTagFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
inline bool HTagFile::IsTiff64() const
    {
    return m_pFile->m_IsTiff64;
    }

inline bool HTagFile::IsValid(HTIFFError**  po_ppError) const
    {
    HFCMonitor Monitor(m_Key);

    if (po_ppError != 0)
        *po_ppError = m_pError;

    return (bool)(m_pError == 0);
    }

inline HTagFile::DirectoryID HTagFile::CurrentDirectory() const
    {
    HFCMonitor Monitor(m_Key);

    return m_CurDir;
    }

inline uint32_t HTagFile::GetCurrentDirIndex () const
    {
    return GetDirectoryNum(m_CurDir);
    }

inline HTagFile::DirectoryType HTagFile::GetCurrentDirType () const
    {
    return GetDirectoryType(m_CurDir);
    }

inline HTagFile::DirectoryID HTagFile::GetCurrentDirID () const
    {
    return m_CurDir;
    }

inline const char* HTagFile::GetTagNameString(HTagID pi_Tag) const
    {
    HFCMonitor Monitor(m_Key);

    if (m_pCurDir->TagIsPresent(pi_Tag))
        return m_pCurDir->GetTagNameString(pi_Tag);
    else
        return 0;
    }

inline HTagInfo::DataType HTagFile::GetTagDataType(HTagID pi_Tag) const
    {
    HFCMonitor Monitor(m_Key);

    if (m_pCurDir->TagIsPresent(pi_Tag))
        return m_pCurDir->GetTagDataType(pi_Tag);
    else
        return HTagInfo::_NOTYPE;
    }

inline uint64_t HTagFile::DirectoryOffset(DirectoryID pi_DirID) const
    {
    HFCMonitor Monitor(m_Key);

    HTagFile::DirectoryType DirType = HTagFile::GetDirectoryType(pi_DirID);
    uint32_t DirNum = HTagFile::GetDirectoryNum(pi_DirID);

    if (DirType == STANDARD && DirNum < m_ListDirCount)
        return m_pListDirOffset64[DirNum];
    else if (DirType == HMR && DirNum < m_ListHMRDirCount)
        return m_ppListHMRDir64[DirNum]->m_DirOffset64;
    else
        return 0;
    }

//----------------------------------------------------------------------------------
// Offset Count tags stuff
//----------------------------------------------------------------------------------
inline uint64_t HTagFile::GetOffset (size_t pi_Index) const
    {
    return (IsTiff64() ? m_pOffset64[pi_Index] : m_pOffset32[pi_Index]);
    }
inline uint32_t HTagFile::GetCount(size_t pi_Index) const
    {
    if (IsTiff64())
        {
        HASSERT(m_pCount64[pi_Index] < ULONG_MAX);
        return (uint32_t)m_pCount64[pi_Index];
        }
    else
        return m_pCount32[pi_Index];
    }

inline bool HTagFile::IsStoredAsBigEndian() const
    {
    return (m_ByteOrder.IsStoredAsBigEndian());
    }

/** -----------------------------------------------------------------------------
    Sets the file as big endian byte ordering. After this call, the tiff file will
    store all endian dependant values as BIG or LITTLE endian.

    Changing this value upon an existing file will not result imediately in the
    rewriting of the file, ????
    This value should only be modified on a new file.


    @param pi_AsBigEndian IN Boolean value indicating if the TIFF file should
                             store values as big or little endian.

    @see IsStoredAsBigEndian()
    -----------------------------------------------------------------------------
*/

inline void HTagFile::SetAsBigEndian(bool pi_AsBigEndian)
    {
    // Force rewriting the Tiff header if the Endian has been changed.
    if (IsStoredAsBigEndian() != pi_AsBigEndian)
        m_EndianAsChanged = true;

    if (pi_AsBigEndian)
        m_Header.Magic = GetBigEndianMagicNumber();
    else
        m_Header.Magic = GetLittleEndianMagicNumber();

    m_ByteOrder.SetStoredAsBigEndian(pi_AsBigEndian);
    }

inline HFCBinStream* HTagFile::GetFilePtr() const
    {
    HFCBinStream* pStream = 0;

    if (m_pFile != 0)
        {
        pStream = m_pFile->GetFilePtr();
        }

    return pStream;
    }

inline HFCBinStreamLockManager* HTagFile::GetLockManager() const
    {
// DMx
    HASSERT(false);
    return m_pLockManager.get();
    }

/*
inline HFCAccessMode HTagFile::GetAccessMode () const
{
    HPRECONDITION(0 != m_pFile);
    return GetFilePtr()->GetAccessMode();
}
*/


/** -----------------------------------------------------------------------------
static method to manipulate DirectoryID
-----------------------------------------------------------------------------
*/
inline HTagFile::DirectoryID HTagFile::MakeDirectoryID(HTagFile::DirectoryType  pi_DirType,
                                                       uint32_t                  pi_DirNum)
    {
    HPRECONDITION(pi_DirNum <  0x0FFFFFFF);
    HPRECONDITION(pi_DirType < 0x0000000F);

    return (pi_DirType << 28 | pi_DirNum & 0x0FFFFFFF);
    }
inline uint32_t HTagFile::GetDirectoryNum(HTagFile::DirectoryID pi_DirectoryID)
    {
    return pi_DirectoryID & 0x0FFFFFFF;
    }
inline HTagFile::DirectoryType HTagFile::GetDirectoryType(HTagFile::DirectoryID pi_DirectoryID)
    {
    return (HTagFile::DirectoryType)(((uint32_t)pi_DirectoryID >> 28) & 0x0000000F);
    }


inline bool HTagFile::TagIsPresent (HTagID pi_Tag) const
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->TagIsPresent(pi_Tag);
    }

inline bool HTagFile::IsVariableSizeTag (HTagID pi_Tag) const
    {
    return m_rTagInfo.IsVariableSizeTag(pi_Tag);
    }


inline bool HTagFile::RemoveTag(HTagID pi_Tag)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->Remove(pi_Tag);
    }

// Get/Set Values Methods
//
inline bool HTagFile::GetField (HTagID pi_Tag, unsigned short* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pVal);
    }


inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pVal);
    }


inline bool HTagFile::GetField (HTagID pi_Tag, double* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint64_t* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, char** po_ppVal) const
    {
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, WChar** po_ppVal) const
    {
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, unsigned short* po_pVal1, unsigned short* po_pVal2) const
    {
    HPRECONDITION(po_pVal1 != 0);
    HPRECONDITION(po_pVal2 != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pVal1, po_pVal2);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, Byte** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, uint32_t** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, double** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
    }

inline bool HTagFile::GetField (HTagID pi_Tag, uint32_t* po_pCount, uint64_t** po_ppVal) const
    {
    HPRECONDITION(po_pCount != 0);
    HPRECONDITION(po_ppVal != 0);
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->GetValues(pi_Tag, po_pCount, po_ppVal);
    }


inline bool HTagFile::SetField (HTagID pi_Tag, unsigned short pi_Val)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Val);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Val)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Val);
    }


inline bool HTagFile::SetField (HTagID pi_Tag, double pi_Val)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Val);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint64_t pi_Val)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Val);
    }

inline bool HTagFile::SetFieldA (HTagID pi_Tag, const char* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValuesA(pi_Tag, pi_pVal);
    }

inline bool HTagFile::SetFieldW (HTagID pi_Tag, const WChar* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValuesW(pi_Tag, pi_pVal);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, unsigned short pi_Val1, unsigned short pi_Val2)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Val1, pi_Val2);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const Byte* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const uint32_t* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const double* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal);
    }

inline bool HTagFile::SetField (HTagID pi_Tag, uint32_t pi_Count, const uint64_t* pi_pVal)
    {
    HFCMonitor Monitor(m_Key);
    return m_pCurDir->SetValues(pi_Tag, pi_Count, pi_pVal);
    }
END_IMAGEPP_NAMESPACE
