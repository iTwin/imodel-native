//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTagFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HTagFile
//-----------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/HTagFile.h>
#include <ImagePP/all/h/HFCLocalBinStream.h>

// Internal used
//
// Number maximun the directory in a file.
// This can be increase by the program if necessary
#define DEF_MAX_DIR_COUNT_INCR  16
static uint32_t DEF_MAX_DIR_COUNT  = DEF_MAX_DIR_COUNT_INCR;

// Number maximun of HMR directory in a file.
// This can be increase by the program if necessary
#define DEF_MAX_HMR_DIR_COUNT_INCR  DEF_MAX_DIR_COUNT_INCR
static uint32_t DEF_MAX_HMR_DIR_COUNT  = DEF_MAX_HMR_DIR_COUNT_INCR;


// TDORAY: Duplicated values from HTIFFTag.h. Find another way?
#define     TAGFILE_VERSION         42
#define     TAGFILE_VERSION_BIG     43      // Tagfile larger than 4Gig

#define     FILETYPE_FULLIMAGE      0x0     // full image
#define     FILETYPE_REDUCEDIMAGE   0x1     // reduced resolution version
#define     FILETYPE_PAGE           0x2     // one page of many
#define     FILETYPE_MASK           0x4     // transparency mask
#define     FILETYPE_EMPTYPAGE      0x8     // for cTiff


uint32_t HTagFile::GetDirCountCapacity ()
    {
    return DEF_MAX_DIR_COUNT;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HTagFile::~HTagFile ()
    {
    // File is saved by derived class. See SaveTagFile method.

    // Free the list allocated internally

    FreeOffsetCount();

    delete m_pError;
    }


HTagFile::HTagFile (const WString&          pi_rFilename,
                    const HTagInfo&         pi_rTagInfo,
                    HFCAccessMode           pi_Mode,
                    uint64_t               pi_OriginOffset,
                    bool                   pi_CreateBigTifFormat,
                    bool                   pi_ValidateDir,
                    bool*                  po_pNewFile)
    :   m_rTagInfo(pi_rTagInfo),
        m_ByteOrder(),
        m_pFile(0)
    {
    Initialize ();
    }

HTagFile::HTagFile (const HFCPtr<HFCURL>&   pi_rpURL,
                    const HTagInfo&         pi_rTagInfo,
                    HFCAccessMode           pi_AccessMode,
                    bool                   pi_CreateBigTifFormat,
                    bool*                  po_pNewFile)
    :   m_rTagInfo(pi_rTagInfo),
        m_ByteOrder(),
        m_pFile(0)
    {
    Initialize ();
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
void HTagFile::Initialize ()

    {
    // Init Members
    m_pError                = 0;
    m_CurDir                = 0;

    m_DataWasModified       = false;


    m_NbData32 = 0;
    m_AllocatedOffset = false;
    m_pOffset32 = 0;
    m_pOffset64 = 0;
    m_AllocatedCount = false;
    m_pCount32 = 0;
    m_pCount64 = 0;

    m_EndianAsChanged   = false;

    m_ListDirCount     = 0;
    m_ppListDir        = new HAutoPtr<HTIFFDirectory>[DEF_MAX_DIR_COUNT];
    m_pListDirOffset64 = new uint64_t[DEF_MAX_DIR_COUNT];
    memset(m_pListDirOffset64, 0, sizeof(uint64_t)*DEF_MAX_DIR_COUNT);

    m_ListHMRDirCount   = 0;
    m_ppListHMRDir64    = new HAutoPtr<HMRDirectory64>[DEF_MAX_HMR_DIR_COUNT];
    }


//TDORAY: Ideally, this method should be generic and called from an initialization method of the base class.
//        This must never be called by the base class's constructor because it invokes virtual methods.
//-----------------------------------------------------------------------------
// protected
// Construct
// Enable file opening/creation on initialization. Must be called from derived
// class's constructor. This permits us to have polymorphic behavior on
// initialization.
//-----------------------------------------------------------------------------
void HTagFile::Construct (const HFCPtr<HFCURL>& pi_rpURL,
                          const WString*        pi_pFilename,
                          HFCAccessMode         pi_Mode,
                          uint64_t             pi_OriginOffset,
                          bool                 pi_CreateBigTifFormat,
                          bool                 pi_ValidateDir,
                          bool*                po_pNewFile)
    {
    bool CreateFileW = false;

    if (!pi_Mode.m_HasCreateAccess)
        {
        bool OpenSuccess;
        if (pi_pFilename == 0)
            OpenSuccess = OpenTiffFile (&pi_rpURL, 0/*string*/, pi_Mode, pi_OriginOffset);
        else
            OpenSuccess = OpenTiffFile (0/*URL*/, pi_pFilename, pi_Mode, pi_OriginOffset);

        if (OpenSuccess)
            {
            ReadDirectories(pi_ValidateDir);
            if((m_pError == 0) || !m_pError->IsFatal())
                {
                SetDirectory (HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0));


                // Validate our top directory if validation is required
                if (pi_ValidateDir)
                    IsValidTopDirectory(&m_pError);

                // Check for the possibility of free block list
                // Only use, if the file can be updated
                if ((m_pFile != 0) &&
                    (pi_Mode.m_HasWriteAccess || pi_Mode.m_HasCreateAccess) &&
                    m_pCurDir->TagIsPresent(m_rTagInfo.GetFreeOffsetsTagID()))
                    {
                    if (IsTiff64())
                        {
                        uint64_t* pOffset;
                        uint64_t* pCount;
                        uint32_t NbEntry;

                        m_pCurDir->GetValues (m_rTagInfo.GetFreeOffsetsTagID(),    &NbEntry, &pOffset);
                        m_pCurDir->GetValues (m_rTagInfo.GetFreeByteCountsTagID(), &NbEntry, &pCount);

                        m_pFile->AddInitialFreeBlock (pOffset, pCount, NbEntry);
                        }
                    else
                        {
                        uint32_t* pOffset;
                        uint32_t* pCount;
                        uint32_t NbEntry;

                        m_pCurDir->GetValues (m_rTagInfo.GetFreeOffsetsTagID(),    &NbEntry, &pOffset);
                        m_pCurDir->GetValues (m_rTagInfo.GetFreeByteCountsTagID(), &NbEntry, &pCount);

                        m_pFile->AddInitialFreeBlock (pOffset, pCount, NbEntry);
                        }

                    m_pCurDir->ValidateFreeBlockTags(m_pFile);
                    }

                // Run whatever has to run when top directory is first initialized
                OnTopDirectoryFirstInitialized();

                // Disable                SetSynchroField();
                }
            }
        else if (pi_Mode.m_OpenAlways && m_pFile != 0)
            {
            CreateFileW = true;
            }
        }
    else
        CreateFileW = true;


    if (CreateFileW)
        {
        bool _dummyStatus;     // Use dummy in case po_pNewFile == NULL.
        if(po_pNewFile == 0)
            po_pNewFile = &_dummyStatus;

        if (m_pFile != 0)
            *po_pNewFile = CreateTiffFile(0/*URL*/, 0/*string*/, pi_Mode, pi_CreateBigTifFormat);
        else if (pi_pFilename == 0)
            *po_pNewFile = CreateTiffFile (&pi_rpURL, 0/*string*/, pi_Mode, pi_CreateBigTifFormat);
        else
            *po_pNewFile = CreateTiffFile (0/*URL*/, pi_pFilename, pi_Mode, pi_CreateBigTifFormat);
        }
    else if (po_pNewFile != 0)
        *po_pNewFile = false;

    }

//-----------------------------------------------------------------------------
// protected
// SaveTagFile
// Enable file saving on destruction. Must be called from derived class's
// destructor. This permits us to have polymorphic behavior on destruction.
//-----------------------------------------------------------------------------
void HTagFile::SaveTagFile ()
    {
    if (m_pFile != 0 && m_pFile->GetFilePtr() != 0 && m_pFile->GetFilePtr()->IsOpened())
        {
        try
            {
            Save();

            }
        catch(...)
            {
            // Stop possible write exceptions here, so we can
            // finish deleting our objects.
            }
        }
    }



bool HTagFile::OpenTiffFile (const HFCPtr<HFCURL>* pi_pURL,       // by URL
                              const WString* pi_pFilename,         // by string
                              HFCAccessMode  pi_AccessMode,
                              uint64_t      pi_OriginOffset)
    {
    HFCMonitor Monitor(m_Key);

    // The Lock monitor is embeded in an HAutoPtr object because we can not
    // instantiate it directly when we need it. The initialization of the
    // object may be skiped by the precedent 'goto WRAPUP'
    HAutoPtr<HFCLockMonitor> pCacheFileLock;

    if (pi_pURL != 0)
        m_pFile = new HTIFFStream(*pi_pURL, pi_AccessMode, pi_OriginOffset);
    else
        m_pFile = new HTIFFStream(*pi_pFilename, pi_AccessMode, pi_OriginOffset);

    bool ReadFail=true;
    if (m_pFile->GetFilePtr() == 0 || m_pFile->GetFilePtr()->GetLastException() != 0)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_OPEN_FILE, 0, true);
        goto WRAPUP;
        }

    // Creation of the lock manager object;
//DMx    m_pLockManager = new HFCBinStreamLockManager(m_pFile->GetFilePtr(), 0, 8, false);
    m_pLockManager = new HFCBinStreamLockManager(0, 0, 8, false);
    pCacheFileLock = new HFCLockMonitor(m_pLockManager.get());

    // Read TIFF Header
    ReadFail = (m_pFile->Read(&m_Header.Magic, sizeof(unsigned short), 1) != 1);
    ReadFail |= (m_pFile->Read(&m_Header.Version, sizeof(unsigned short), 1) != 1);
    if (ReadFail)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_READ_HEADER, 0, true);
        goto WRAPUP;
        }

    pCacheFileLock->ReleaseKey();

    // Validate TIFF HEader
    if (m_Header.Magic != GetBigEndianMagicNumber() &&
        m_Header.Magic != GetLittleEndianMagicNumber())
        {
        HTIFFError::BadMagicNbErInfo ErInfo;
        ErInfo.m_MagicNb = m_Header.Magic;

        ErrorMsg(&m_pError, HTIFFError::BAD_MAGIC_NUMBER, &ErInfo, true);
        goto WRAPUP;
        }
    m_ByteOrder.SetStoredAsBigEndian(GetBigEndianMagicNumber() == m_Header.Magic);

    if (m_ByteOrder.NeedSwapByte())
        SwabArrayOfShort(&m_Header.Version, 1);

    // Validate TIFF Version.
    if (m_Header.Version != TAGFILE_VERSION && m_Header.Version != TAGFILE_VERSION_BIG)
        {
        HTIFFError::BadVersionNbErInfo ErInfo;
        ErInfo.m_VersionNb = m_Header.Version;
        ErrorMsg(&m_pError, HTIFFError::BAD_FILE_VERSION, &ErInfo, true);
        goto WRAPUP;
        }

    if (m_Header.Version == TAGFILE_VERSION_BIG)
        {
        // Is a Big Tiff
        m_pFile->m_IsTiff64 = true;
        if (m_pFile->GetFilePtr()->IsCompatibleWith(HFCLocalBinStream::CLASS_ID))
            static_cast<HFCLocalBinStream*>(m_pFile->GetFilePtr())->SetMaxFileSizeSupported(HFCLocalBinStream::OffsetIs64Bits);

        // We can ignore the next 2 UShort
        //                                  Should be set to 8
        ReadFail |= (m_pFile->Read(&m_Header.BytesizeOfOffset, sizeof(unsigned short), 1) != 1);
        //                                  Should be set to 0
        ReadFail |= (m_pFile->Read(&m_Header.Reserved, sizeof(unsigned short), 1) != 1);
        ReadFail |= (m_pFile->Read(&m_Header.DirOffset64, sizeof(uint64_t), 1) != 1);

        if (m_ByteOrder.NeedSwapByte())
            {
            SwabArrayOfShort(&m_Header.BytesizeOfOffset, 1);
            SwabArrayOfUInt64(&m_Header.DirOffset64, 1);
            }
        }
    else
        {
        m_pFile->m_IsTiff64 = false;

        uint32_t Offset;
        ReadFail |= (m_pFile->Read(&Offset, sizeof(uint32_t), 1) != 1);

        if (m_ByteOrder.NeedSwapByte())
            SwabArrayOfLong(&Offset, 1);

        m_Header.DirOffset64 = Offset;
        }

    return !ReadFail;
WRAPUP:
    return false;
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HTagFile::Save()
    {
    HFCMonitor Monitor(m_Key);

    if (m_pFile != 0)
        {
        // If no ReadOnly, Update the file directories.
        if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
            {
            // Write the directories before, because the list of free block
            // is update-to-date only after.
            WriteDirectories();

            uint64_t* pOffset = 0;
            uint64_t* pSize = 0;
            uint32_t Count;

            // Set Block free if present
            if (m_pFile->GetListFreeBlock (&pOffset, &pSize, &Count) &&
                (Count > 0))
                {
                // Set the Tags in the first Directory
                SetDirectoryImpl (HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0), false);

                if (IsTiff64())
                    {
                    m_pCurDir->SetValues(m_rTagInfo.GetFreeOffsetsTagID(), Count, pOffset);
                    m_pCurDir->SetValues(m_rTagInfo.GetFreeByteCountsTagID(), Count, pSize);
                    }
                else
                    {
                    HAutoPtr<uint32_t> pVal(new uint32_t[Count]);

                    for (uint32_t i=0; i<Count; ++i)
                        pVal[i] = (uint32_t)pOffset[i];
                    m_pCurDir->SetValues(m_rTagInfo.GetFreeOffsetsTagID(), Count, pVal);

                    for (uint32_t i=0; i<Count; ++i)
                        pVal[i] = (uint32_t)pSize[i];
                    m_pCurDir->SetValues(m_rTagInfo.GetFreeByteCountsTagID(), Count, pVal);
                    }
                // Rewrite again, but only these Tags are Changes.
                WriteDirectories();
                }

            delete[] pOffset;
            delete[] pSize;

            //Ensure that the nothing is kept in memory.
            //m_pFile->GetFilePtr()->Flush();
            }
        }
    }

bool HTagFile::CreateTiffFile (const HFCPtr<HFCURL>*  pi_pURL,
                                const WString*         pi_pFilename,
                                HFCAccessMode          pi_AccessMode,          // default RWC only used by URL
                                bool                  pi_CreateBigTifFormat)  // false
    {
    HFCMonitor Monitor(m_Key);

    // The Lock monitor is embeded in an HAutoPtr object because we can not
    // instantiate it directly when we need it. The initialization of the
    // object may be skiped by the precedent 'goto WRAPUP'
    HAutoPtr<HFCLockMonitor> pCacheFileLock;

    if (pi_pURL != 0)
        m_pFile = new HTIFFStream(*pi_pURL, pi_AccessMode);
    else if (pi_pFilename != 0)
        m_pFile = new HTIFFStream(*pi_pFilename, HFC_READ_WRITE_CREATE);
    else
        {
        HASSERT(m_pFile != 0);
        }

    // Open Error.
    if (m_pFile->GetFilePtr() == 0 || m_pFile->GetFilePtr()->GetLastException() != 0)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_CREATE_FILE, 0, true);
        goto WRAPUP;
        }

    // Creation of the lock manager object;
//DMx    m_pLockManager = new HFCBinStreamLockManager(m_pFile->GetFilePtr(), 0, 8, false);
    m_pLockManager = new HFCBinStreamLockManager(0, 0, 8, false);

    // Set TIFF Header
    if (m_ByteOrder.IsBigEndianSystem())
        m_Header.Magic = GetBigEndianMagicNumber();
    else
        m_Header.Magic = GetLittleEndianMagicNumber();
    m_ByteOrder.SetStoredAsBigEndian(m_ByteOrder.IsBigEndianSystem());

    pCacheFileLock  = new HFCLockMonitor(m_pLockManager.get());

    bool WriteFail;
    if (pi_CreateBigTifFormat)
        {
        // Is a Big Tiff
        m_pFile->m_IsTiff64         = true;
        if (m_pFile->GetFilePtr()->IsCompatibleWith(HFCLocalBinStream::CLASS_ID))
            static_cast<HFCLocalBinStream*>(m_pFile->GetFilePtr())->SetMaxFileSizeSupported(HFCLocalBinStream::OffsetIs64Bits);

        m_Header.Version            = TAGFILE_VERSION_BIG;
        m_Header.BytesizeOfOffset   = 8;
        m_Header.Reserved           = 0;
        m_Header.DirOffset64        = 0;

        WriteFail = (m_pFile->Write(&m_Header.Magic, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&m_Header.Version, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&m_Header.BytesizeOfOffset, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&m_Header.Reserved, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&m_Header.DirOffset64, sizeof(uint64_t), 1) != 1);
        }
    else
        {
        m_pFile->m_IsTiff64         = false;
        m_Header.Version      = TAGFILE_VERSION;
        m_Header.DirOffset64  = 0;                // Create File

        uint32_t Offset=0;
        WriteFail = (m_pFile->Write(&m_Header.Magic, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&m_Header.Version, sizeof(unsigned short), 1) != 1);
        WriteFail |= (m_pFile->Write(&Offset, sizeof(uint32_t), 1) != 1);
        }
    if (WriteFail)
        {
        ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_HEADER, 0, true);
        goto WRAPUP;
        }

#if 0 // Disable
    // Synchro field(the data value is not important)
    m_SynchroOffset = m_pFile->Tell();
    if (m_pFile->Write(&m_SynchroOffset, sizeof(uint32_t), 1) != 1)
        {
        char Msg[312];

        sprintf (Msg, "Cannot Write Synchro field. File:%s.", pi_pFilename);
        ErrorMsg(&m_pError, Msg, true);
        goto WRAPUP;
        }
#endif

    pCacheFileLock->ReleaseKey();

    return true;
WRAPUP:
    return false;
    }



HFCPtr<HFCURL> HTagFile::GetURL() const
    {
    return m_pFile->GetURL();
    }

uint32_t HTagFile::NumberOfDirectory (HTagFile::DirectoryType pi_DirType) const
    {
    return _NumberOfDirectory(pi_DirType);
    }

// This method set the current Directory +
// In creation mode or when we add a new directory
// this method set the field Strip\Tile Offset - Count if necessary
//

bool HTagFile::SetDirectory (HTagFile::DirectoryID pi_DirID)
    {
    return SetDirectoryImpl(pi_DirID, true);
    }

//
// SetDirectoryImpl
//
bool HTagFile::SetDirectoryImpl (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    HFCMonitor Monitor(m_Key);
    bool Ret = true;

    if ((pi_DirID != m_CurDir) || (m_NbData32 == 0))
        {
        uint32_t DirNum   = GetDirectoryNum(pi_DirID);
        HTagFile::DirectoryType DirType = GetDirectoryType(pi_DirID);

        // Standard Directories
        if (DirType == HTagFile::STANDARD && DirNum < m_ListDirCount)
            {
            m_CurDir  = pi_DirID;
            m_pCurDir = m_ppListDir[DirNum];
            }

        // HMR DIrectory
        else if (DirType == HTagFile::HMR && DirNum < m_ListHMRDirCount)
            {
            m_CurDir  = pi_DirID;
            m_pCurDir = m_ppListHMRDir64[DirNum]->m_pDirectory;
            m_NbData32  = 0;
            }
        else
            Ret = false;

        // Invoked only when current directory is changed successfully
        if (Ret && !OnCurrentDirectoryChanged(pi_DirID, pi_ReloadCodec))
            Ret = false;
        }

    // Always invoked after current directory is set
    if (!PostCurrentDirectorySet(pi_DirID, pi_ReloadCodec))
        Ret = false;


    return Ret;
    }


HTIFFDirectory& HTagFile::GetCurrentDir ()
    {
    HPRECONDITION(0 != m_pCurDir);
    return *m_pCurDir;
    }

const HTIFFDirectory& HTagFile::GetCurrentDir () const
    {
    HPRECONDITION(0 != m_pCurDir);
    return *m_pCurDir;
    }


//----------------------------------------------------------------------------------
// Offset Count tags stuff
//----------------------------------------------------------------------------------
void HTagFile::FreeOffsetCount()
    {
    if (m_AllocatedOffset)
        {
        if (m_pOffset32)
            delete[] m_pOffset32;

        if (m_pOffset64)
            delete[] m_pOffset64;

        m_pOffset32 = 0;
        m_pOffset64 = 0;
        }
    m_AllocatedOffset = false;

    if (m_AllocatedCount)
        {
        if (m_pCount32)
            delete[] m_pCount32;

        if (m_pCount64)
            delete[] m_pCount64;

        m_pCount32 = 0;
        m_pCount64 = 0;
        }
    m_AllocatedCount = false;
    }



// This method will delete the pointer if not used internally
//
void HTagFile::SetOffsetCountData(uint32_t pi_NbData, uint64_t* pi_pOffset, uint32_t* pi_pCount)
    {
    FreeOffsetCount();

    m_NbData32 = pi_NbData;
    if (pi_pOffset)
        {
        m_AllocatedOffset = true;
        if (IsTiff64())
            m_pOffset64 = pi_pOffset;
        else
            {
            m_pOffset32 = new uint32_t[pi_NbData];
            for(uint32_t i=0; i<pi_NbData; ++i)
                {
                HASSERT(pi_pOffset[i] < ULONG_MAX);
                m_pOffset32[i] = (uint32_t)pi_pOffset[i];
                }

            delete[] pi_pOffset;
            }
        }

    if (pi_pCount)
        {
        m_AllocatedCount = true;
        if (IsTiff64())
            {
            m_pCount64 = new uint64_t[pi_NbData];
            for(uint32_t i=0; i<pi_NbData; ++i)
                m_pCount64[i] = pi_pCount[i];

            delete[] pi_pCount;
            }
        else
            m_pCount32 = pi_pCount;
        }
    }

bool HTagFile::SetOffset(size_t pi_Index, uint64_t pi_Offset)
    {
    HPRECONDITION(pi_Index < m_NbData32);

    if (IsTiff64())
        m_pOffset64[pi_Index] = pi_Offset;
    else
        {
        HASSERT(pi_Offset < ULONG_MAX);
        m_pOffset32[pi_Index] = (uint32_t)pi_Offset;
        }

    return true;
    }

bool HTagFile::SetCount(size_t pi_Index, uint64_t pi_Count)
    {
    HPRECONDITION(pi_Index < m_NbData32);

    if (IsTiff64())
        m_pCount64[pi_Index] = pi_Count;
    else
        {
        HASSERT(pi_Count < ULONG_MAX);
        m_pCount32[pi_Index] = (uint32_t)pi_Count;
        }

    return true;
    }


void HTagFile::ReadOffsetCountTags()
    {
    FreeOffsetCount();

    // Must call Count before Offset, because Count is not always
    // present.
    if (IsTiff64())
        {
        GetCurrentDir().GetValues (GetPacketByteCountsTagID(),   &m_NbData32, &m_pCount64);
        GetCurrentDir().GetValues (GetPacketOffsetsTagID(),   &m_NbData32, &m_pOffset64);
        }
    else
        {
        GetCurrentDir().GetValues (GetPacketByteCountsTagID(),   &m_NbData32, &m_pCount32);
        GetCurrentDir().GetValues (GetPacketOffsetsTagID(),   &m_NbData32, &m_pOffset32);
        }
    }

//-----------------------------------------------------------------------------
// private
// InsertSortMergedDirectories()
//-----------------------------------------------------------------------------
void HTagFile::InsertSortMergedDirectories()
    {
    size_t i, j;
    uint64_t indexOffset;
    uint64_t indexSize;
    uint32_t indexDir;
    uint32_t indexPos;
    bool   isHMRDir;
    bool   isTag;
    size_t max = m_MergedDirectories.size();

    for(i=1; i < max; i++)
        {
        indexOffset  = (uint64_t) m_MergedDirectories[i].Offset;
        indexSize    = (uint64_t) m_MergedDirectories[i].Size;
        indexDir     =           m_MergedDirectories[i].Dir;
        indexPos     =           m_MergedDirectories[i].Position;
        isHMRDir     =           m_MergedDirectories[i].isHMRDir;
        isTag        =           m_MergedDirectories[i].isATag;
        j            =           i;

        while((j > 0) && (m_MergedDirectories[j-1].Offset > indexOffset))
            {
            m_MergedDirectories[j].Offset   = m_MergedDirectories[j-1].Offset;
            m_MergedDirectories[j].Size     = m_MergedDirectories[j-1].Size;
            m_MergedDirectories[j].Dir      = m_MergedDirectories[j-1].Dir;
            m_MergedDirectories[j].Position = m_MergedDirectories[j-1].Position;
            m_MergedDirectories[j].isHMRDir = m_MergedDirectories[j-1].isHMRDir;
            m_MergedDirectories[j].isATag   = m_MergedDirectories[j-1].isATag;
            j                               = j-1;
            }

        m_MergedDirectories[j].Offset    = indexOffset;
        m_MergedDirectories[j].Size      = indexSize;
        m_MergedDirectories[j].Dir       = indexDir;
        m_MergedDirectories[j].Position  = indexPos;
        m_MergedDirectories[j].isHMRDir  = isHMRDir;
        m_MergedDirectories[j].isATag    = isTag;
        }
    }

//-----------------------------------------------------------------------------
// private
// InsertSortFreeBlock()
//-----------------------------------------------------------------------------
void HTagFile::InsertSortFreeBlock()
    {
    size_t i, j;
    uint64_t indexOffset;
    uint64_t indexSize;
    uint32_t indexDir;

    size_t max = m_FreeBlockDirectory.size();

    for(i=1; i < max; i++)
        {
        indexOffset  = (uint64_t) m_FreeBlockDirectory[i].Offset;
        indexSize    = (uint64_t) m_FreeBlockDirectory[i].Size;
        indexDir     =           m_FreeBlockDirectory[i].Dir;
        j            =           i;

        while((j > 0) && (m_FreeBlockDirectory[j-1].Offset > indexOffset))
            {
            m_FreeBlockDirectory[j].Offset   = m_FreeBlockDirectory[j-1].Offset;
            m_FreeBlockDirectory[j].Size     = m_FreeBlockDirectory[j-1].Size;
            m_FreeBlockDirectory[j].Dir      = m_FreeBlockDirectory[j-1].Dir;
            j                                = j-1;
            }

        m_FreeBlockDirectory[j].Offset    = indexOffset;
        m_FreeBlockDirectory[j].Size      = indexSize;
        m_FreeBlockDirectory[j].Dir       = indexDir;
        }
    }



//-----------------------------------------------------------------------------
// private
// SetFreeBlockList()
//-----------------------------------------------------------------------------
void HTagFile::SetFreeBlockList()
    {
    FreeblockData temp;
    FreeblockData temp1;
    uint32_t NbEntryFreeBlock;
    uint64_t FreeBlockSize;

    SetDirectory(0);

    if (m_pCurDir->TagIsPresent(m_rTagInfo.GetFreeOffsetsTagID()) && m_pCurDir->TagIsPresent(m_rTagInfo.GetFreeByteCountsTagID()))
        {
        if (IsTiff64())
            {
            uint64_t* pOffsetFreeBlock64;
            uint64_t* pCountFreeBlock64;

            //Get the list of the freeblocks in the file
            m_pCurDir->GetValues        ( m_rTagInfo.GetFreeOffsetsTagID(),
                                          &NbEntryFreeBlock,
                                          &pOffsetFreeBlock64);
            m_pCurDir->GetValues        ( m_rTagInfo.GetFreeByteCountsTagID(),
                                          &NbEntryFreeBlock,
                                          &pCountFreeBlock64);
            //Put the real freeblock in the vector
            for (uint32_t i=0; i < NbEntryFreeBlock; i++)
                {
                m_NbByteBlockFree += pOffsetFreeBlock64[i];
                temp.Offset        = pOffsetFreeBlock64[i];
                temp.Size          = pCountFreeBlock64[i];

                m_FreeBlockDirectory.push_back(temp);
                }
            }
        else
            {
            uint32_t* pOffsetFreeBlock;
            uint32_t* pCountFreeBlock;

            //Get the list of the freeblocks in the file
            m_pCurDir->GetValues ( m_rTagInfo.GetFreeOffsetsTagID(),
                                   &NbEntryFreeBlock,
                                   &pOffsetFreeBlock);

            m_pCurDir->GetValues ( m_rTagInfo.GetFreeByteCountsTagID(),
                                   &NbEntryFreeBlock,
                                   &pCountFreeBlock);
            //Put the real freeblock in the vector
            for (uint32_t i=0; i < NbEntryFreeBlock; i++)
                {
                m_NbByteBlockFree += pOffsetFreeBlock[i];
                temp.Offset        = pOffsetFreeBlock[i];
                temp.Size          = pCountFreeBlock[i];

                m_FreeBlockDirectory.push_back(temp);
                }
            }

        //For each Directory we add it as a new freeblock
        //they will be rewritten at the end
        //Add the HMR Directory
        FreeBlockSize = m_ppListDir[0]->GetDirectorySize(m_pFile,
                                                         m_ppListHMRDir64[0]->m_DirOffset64);

        temp.Offset = m_ppListHMRDir64[0]->m_DirOffset64;
        temp.Size   = FreeBlockSize;
        temp.Dir    = 0;
        m_FreeBlockDirectory.push_back(temp);

        //Add the others directories
        for (uint32_t Index=0; Index < m_ListDirCount; Index++)
            {
            FreeBlockSize = m_ppListDir[Index]->GetDirectorySize(m_pFile,
                                                                 m_pListDirOffset64[Index]);

            temp.Offset = m_pListDirOffset64[Index];
            temp.Size   = FreeBlockSize;
            temp.Dir    = Index;
            m_FreeBlockDirectory.push_back(temp);
            }

        //Add the freeblock data as a freeblock
        m_ppListDir[0]->GetFreeBlockInfo(temp.Size,temp.Offset,temp1.Size,temp1.Offset);

        m_FreeBlockDirectory.push_back(temp);
        m_FreeBlockDirectory.push_back(temp1);

        //sort the vector
        InsertSortFreeBlock();
        }
    }

//-----------------------------------------------------------------------------
// private
// DataFreeBlockCanBeMerged()
//-----------------------------------------------------------------------------
bool HTagFile::DataFreeBlockCanBeMerged(uint32_t& p_Position)
    {

    if ((m_FreeBlockDirectory.size() - 1) > p_Position)
        {
        if ((m_FreeBlockDirectory[p_Position].Offset + m_FreeBlockDirectory[p_Position].Size) != m_FreeBlockDirectory[p_Position+1].Offset)
            return true;
        else
            {
            m_FreeBlockDirectory[p_Position].Size = m_FreeBlockDirectory[p_Position].Size + m_FreeBlockDirectory[p_Position+1].Size;
            m_FreeBlockDirectory.erase(m_FreeBlockDirectory.begin()+p_Position+1);
            p_Position--;
            return false;
            }
        }
    else
        return true;
    }

//-----------------------------------------------------------------------------
// private
// CompareDataWithBlockFree()
//-----------------------------------------------------------------------------
void HTagFile::CompareDataWithBlockFree(uint64_t p_PositionBlockFree,
                                        uint32_t&  p_PositionSortedData,
                                        uint32_t&  p_StartPositionSortedData)
    {
    //Set the position where the current iteration starts
    p_StartPositionSortedData = p_PositionSortedData;

    //Set where the iteration will stop for the current blockfree
    //Reach the next block free
    while (m_MergedDirectories[p_PositionSortedData].Offset < p_PositionBlockFree &&
           (p_PositionSortedData+1) < m_MergedDirectories.size())
        {
        p_PositionSortedData++;
        }
    }

//-----------------------------------------------------------------------------
// private
// GetDataForEachTag(UInt32 p_Index)
//-----------------------------------------------------------------------------
void HTagFile::GetDataForEachTag(uint32_t p_Index, bool p_isHMRDir)
    {
    vector<OffsetAndSize> Data;

    if (!p_isHMRDir)
        m_ppListDir[p_Index]->GetListTagPerDirectory(m_pFile,
                                                     Data,
                                                     p_isHMRDir,
                                                     p_Index);
    else
        m_ppListHMRDir64[0]->m_pDirectory->GetListTagPerDirectory(m_pFile,
                                                                  Data,
                                                                  p_isHMRDir,
                                                                  p_Index);

    //Insert the data returned in the directories vector.
    for (uint32_t Index = 0; Index < Data.size(); Index++)
        {
        m_MergedDirectories.push_back(Data[Index]);
        }
    }

//-----------------------------------------------------------------------------
// public
// AddHMRDirectory
//
// Add private HMR directory into the current directory
//-----------------------------------------------------------------------------
bool HTagFile::AddHMRDirectory(HTagID pi_HMRTag)
    {
    HPRECONDITION(m_pCurDir != 0);
    HPRECONDITION(GetDirectoryType(m_CurDir) == STANDARD);

    bool Ret = true;
    HFCMonitor Monitor(m_Key);

    // find the current directory offset
    uint32_t ParentDirIndex = 0;
    HTIFFDirectory* pParentDir = m_ppListDir[0];
    while (pParentDir != m_pCurDir && ParentDirIndex < m_ListDirCount)
        {
        ParentDirIndex++;
        pParentDir = m_ppListDir[ParentDirIndex];
        }

    HASSERT(ParentDirIndex < m_ListDirCount);

    // TDORAY: HMR_IMAGEINFORMATION is not generic, remove from here
    bool HMRDirExist;
    if (IsTiff64())
        {
        uint64_t HMRDirOffset;
        HMRDirExist = m_ppListDir[ParentDirIndex]->GetValues(m_rTagInfo.GetHMRDirectoryV1TagID(), &HMRDirOffset);
        }
    else
        {
        uint32_t HMRDirOffset;
        HMRDirExist = m_ppListDir[ParentDirIndex]->GetValues(m_rTagInfo.GetHMRDirectoryV1TagID(), &HMRDirOffset);
        }
    if (HMRDirExist)
        {
        ErrorMsg(&m_pError,
                 HTIFFError::HMR_DIR_ALREADY_EXIST_IN_PAGE,
                 0,
                 false);
        Ret = false;
        }
    else
        {
        Realloc_MAX_HMR_DIR_COUNT();
        m_ListHMRDirCount++;
        HMRDirectory64* pNewHMRDir = new HMRDirectory64;
        m_ppListHMRDir64[m_ListHMRDirCount - 1] = new HMRDirectory64;

        pNewHMRDir->m_pDirectory     = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
        pNewHMRDir->m_DirOffset64    = 0;
        pNewHMRDir->m_HMRTagUsed     = pi_HMRTag;
        pNewHMRDir->m_ParentDirIndex = ParentDirIndex;

        // Add Entry in the parent directory
        if (IsTiff64())
            m_ppListDir[ParentDirIndex]->SetValues(pi_HMRTag, pNewHMRDir->m_DirOffset64);
        else
            m_ppListDir[ParentDirIndex]->SetValues(pi_HMRTag, (uint32_t)pNewHMRDir->m_DirOffset64);


        // add directory into the HMRDirectories
        m_ppListHMRDir64[m_ListHMRDirCount - 1] = pNewHMRDir;

        // Add TAG use for the concurrence support
        // Disable        HASSERT(m_SynchroOffset != 0);
        pNewHMRDir->m_pDirectory->SetValues (m_rTagInfo.GetHMRSyncronizationTagID(), (uint32_t)0 /*m_SynchroOffset*/);

        SetDirectoryImpl(HTagFile::MakeDirectoryID(HTagFile::HMR, m_ListHMRDirCount - 1), false);
        }

    return Ret;
    }


// The new Directory is now the current.
//
bool HTagFile::AppendDirectory()
    {
    HFCMonitor Monitor(m_Key);

    // Check if structure is too small
    Realloc_MAX_DIR_COUNT();

    m_ppListDir[m_ListDirCount] = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
    m_ListDirCount++;

    SetDirectory(HTagFile::MakeDirectoryID(HTagFile::STANDARD, m_ListDirCount-1));

    return true;
    }

bool HTagFile::ReadDirectories (bool pi_ValidateDir)
    {
    HPRECONDITION(m_pFile != 0);
    HFCMonitor Monitor(m_Key);

    HFCLockMonitor CacheFileLock(m_pLockManager.get());

    // Force reload in SetDirectory
    m_CurDir = ULONG_MAX;

    uint64_t DirOffset = m_Header.DirOffset64; // 0 if create a new file

    // Reset the list
    m_ListDirCount  = 0;
    m_ListHMRDirCount = 0;
    HTIFFDirectory* pCurPageDir = 0;
    uint64_t HMRDirOffset;
    HTagID HMRTag;
    while (DirOffset)
        {
        // Check if structure is too small
        Realloc_MAX_DIR_COUNT();

        // Keep Offset
        m_pListDirOffset64[m_ListDirCount] = DirOffset;

        m_ppListDir[m_ListDirCount] = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());

        // If Offset 0, 2 possibility, end of directory or Error
        DirOffset = m_ppListDir[m_ListDirCount]->ReadDirectory(m_pFile, DirOffset);
            {
            HTIFFError* pErrInfo;
            if (!m_ppListDir[m_ListDirCount]->IsValid(&pErrInfo))
                {
                if (ErrorMsg(&m_pError, *pErrInfo))
                    goto WRAPUP;
                }
            }

        uint32_t DirType = 0;
        if (pCurPageDir == 0 ||
            (m_ppListDir[m_ListDirCount]->GetValues (m_rTagInfo.GetSubFileTypeTagID(), &DirType) && (DirType == FILETYPE_FULLIMAGE ||
                    DirType == FILETYPE_PAGE ||
                    DirType == FILETYPE_EMPTYPAGE)))
            {
            pCurPageDir = m_ppListDir[m_ListDirCount];
            }

        // Check if the necesssary Tags are present.
        if ((pi_ValidateDir == true) &&
            !DirectoryIsValid(m_ppListDir[m_ListDirCount], pCurPageDir))
            goto WRAPUP;

        // TDORAY: This part is not generic and should not be here.
        // TR 269105: Make sure that sub-res is compatible with the main res.
        if(pi_ValidateDir && pCurPageDir && FILETYPE_REDUCEDIMAGE == DirType && !IsValidReducedImage(m_ppListDir[m_ListDirCount], pCurPageDir))
            {
            HASSERT(m_ListDirCount > 0);

            // Invalid sub-res! Schedule deletion.
            m_ppListDir[m_ListDirCount] = 0;
            m_pListDirOffset64[m_ListDirCount] = 0;
            m_ppListDir[m_ListDirCount-1]->SetNextDirectoryOffsetIsInvalid(true);
            }
        else
            {

            // TDORAY: HMR_IMAGEINFORMATION is not generic, remove from here
            // Check HMR private Dir is present, and which version (HMR1 or HMR2)
            HMRDirOffset = 0;
            if (IsTiff64())
                {
                if (m_ppListDir[m_ListDirCount]->GetValues(m_rTagInfo.GetHMRDirectoryV1TagID(), &HMRDirOffset))
                    HMRTag = m_rTagInfo.GetHMRDirectoryV1TagID();
                else if (m_ppListDir[m_ListDirCount]->GetValues(m_rTagInfo.GetHMRDirectoryV2TagID(), &HMRDirOffset))
                    HMRTag = m_rTagInfo.GetHMRDirectoryV2TagID();
                }
            else
                {
                // Get Offset in 64bit, but we have a tiff32
                //
                uint32_t Val;
                if (m_ppListDir[m_ListDirCount]->GetValues(m_rTagInfo.GetHMRDirectoryV1TagID(), &Val))
                    {
                    HMRTag = m_rTagInfo.GetHMRDirectoryV1TagID();
                    HMRDirOffset = Val;
                    }
                else if (m_ppListDir[m_ListDirCount]->GetValues(m_rTagInfo.GetHMRDirectoryV2TagID(), &Val))
                    {
                    HMRTag = m_rTagInfo.GetHMRDirectoryV2TagID();
                    HMRDirOffset = Val;
                    }
                }

            if (HMRDirOffset != 0)
                {
                // Check if structure is too small
                Realloc_MAX_HMR_DIR_COUNT();
                m_ListHMRDirCount++;
                HMRDirectory64* pNewHMRDir = new HMRDirectory64;
                pNewHMRDir->m_pDirectory = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
                pNewHMRDir->m_ParentDirIndex = m_ListDirCount;
                pNewHMRDir->m_HMRTagUsed = HMRTag;
                pNewHMRDir->m_DirOffset64 = HMRDirOffset;
                m_ppListHMRDir64[m_ListHMRDirCount - 1] = pNewHMRDir;
                }
            else if (DirType == FILETYPE_EMPTYPAGE)
                {
                // add an empty entry into the HMR directory list
                Realloc_MAX_HMR_DIR_COUNT();
                m_ListHMRDirCount++;
                m_ppListHMRDir64[m_ListHMRDirCount - 1] = 0;
                }

            m_ListDirCount++;
            }
        }

    // Create a new File
    //
    if (m_ListDirCount == 0)
        {
        m_pListDirOffset64[m_ListDirCount] = 0;
        m_ppListDir[m_ListDirCount] = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
        m_ListDirCount = 1;
        }

    if (m_ListHMRDirCount > 0)
        {
        for (uint32_t i = 0; i < m_ListHMRDirCount; i++)
            {
            // read HMR Directories
            // If Offset 0, 2 possibility, end of directory or Error
            if (m_ppListHMRDir64[i] != 0)
                {
                m_ppListHMRDir64[i]->m_pDirectory->ReadDirectory(m_pFile, m_ppListHMRDir64[i]->m_DirOffset64);

                HTIFFError* pErrInfo;
                if (!m_ppListHMRDir64[i]->m_pDirectory->IsValid(&pErrInfo))
                    {
                    if (ErrorMsg(&m_pError, *pErrInfo))
                        goto WRAPUP;
                    }
                }
            }
        }

    CacheFileLock.ReleaseKey();
    return true;
WRAPUP:
    CacheFileLock.ReleaseKey();
    return false;
    }

void HTagFile::WriteDirectories()
    {
    HFCMonitor Monitor(m_Key);

    HFCLockMonitor CacheFileLock (m_pLockManager.get());


    // Do nothing if file open in ReadOnly mode
    //

    if (m_pFile->GetMode().m_HasWriteAccess || m_pFile->GetMode().m_HasCreateAccess)
        {
        uint64_t PrevOffset = 0;

        // Write HMR Directories
        if (m_ListHMRDirCount > 0)
            {
            uint64_t PrevHMRDirOffset;
            for (uint32_t i = 0; i < m_ListHMRDirCount; i++)
                {
                PrevHMRDirOffset = m_ppListHMRDir64[i]->m_DirOffset64;
                m_ppListHMRDir64[i]->m_pDirectory->WriteDirectory(m_pFile,
                                                                  &m_ppListHMRDir64[i]->m_DirOffset64,
                                                                  0);

                // Directory changed offset
                if (m_ppListHMRDir64[i]->m_DirOffset64 != PrevHMRDirOffset)
                    {
                    if (IsTiff64())
                        m_ppListDir[m_ppListHMRDir64[i]->m_ParentDirIndex]->SetValues(m_ppListHMRDir64[i]->m_HMRTagUsed,
                                                                                      m_ppListHMRDir64[i]->m_DirOffset64);
                    else
                        m_ppListDir[m_ppListHMRDir64[i]->m_ParentDirIndex]->SetValues(m_ppListHMRDir64[i]->m_HMRTagUsed,
                                                                                      (uint32_t)m_ppListHMRDir64[i]->m_DirOffset64);
                    }
                }
            }

        HTagFile::DirectoryID CurDir = CurrentDirectory();   // Save current Dir
        PrevOffset      = 0;
        for(uint32_t i=0; i<m_ListDirCount; i++)
            {
            // I change the directory, because the method alloc the list
            // of strip\tile if not already yet.
            SetDirectoryImpl(HTagFile::MakeDirectoryID(HTagFile::STANDARD, i), false);

            // Give the opportunity to implementations to save their cached state to tags
            _PreWriteCurrentDir(GetCurrentDirID());

            if(m_DataWasModified && m_pCurDir->NextDirectoryOffsetIsInvalid())
                m_pCurDir->SetDirty();       // Force current dir to rewrites so it updates the nextDirOffset.


            // Rewrite dir if needed.  It will clear the NextDirectoryOffsetIsInvalid if the directory is rewritten.
            PrevOffset = m_pCurDir->WriteDirectory(m_pFile,
                                                   &(m_pListDirOffset64[i]),
                                                   PrevOffset,
                                                   (i+1 < m_ListDirCount) ? m_pListDirOffset64[i+1] : 0);
            }
        SetDirectoryImpl(CurDir, false);        // Restore previous Dir

        // If the first directory is changed position
        // rewrite the Header
        if ( ((m_ListDirCount >= 1) && (m_Header.DirOffset64 != m_pListDirOffset64[0])) || m_EndianAsChanged)
            {
            m_Header.DirOffset64 = m_pListDirOffset64[0];

            bool WriteFail = !m_pFile->Seek(0, SEEK_SET);
            if (IsTiff64())
                {
                unsigned short Version = m_Header.Version;
                unsigned short BytesizeOfOffset = m_Header.BytesizeOfOffset;
                uint64_t Offset = m_Header.DirOffset64;
                // Swap if needed before Write
                if (m_ByteOrder.NeedSwapByte())
                    {
                    SwabArrayOfShort(&Version, 1);
                    SwabArrayOfShort(&BytesizeOfOffset, 1);
                    SwabArrayOfUInt64(&Offset, 1);
                    }

                WriteFail |= (m_pFile->Write(&m_Header.Magic, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&Version, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&BytesizeOfOffset, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&m_Header.Reserved, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&Offset, sizeof(uint64_t), 1) != 1);
                }
            else
                {
                unsigned short Version = m_Header.Version;
                uint32_t Offset = (uint32_t)m_Header.DirOffset64;
                // Swap if needed before Write
                if (m_ByteOrder.NeedSwapByte())
                    {
                    SwabArrayOfShort(&Version, 1);
                    SwabArrayOfLong(&Offset, 1);
                    }

                WriteFail |= (m_pFile->Write(&m_Header.Magic, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&Version, sizeof(unsigned short), 1) != 1);
                WriteFail |= (m_pFile->Write(&Offset, sizeof(uint32_t), 1) != 1);
                }
            if (WriteFail)
                {
                ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_HEADER, 0, true);
                }
            }
        }
    CacheFileLock.ReleaseKey();
    }


void HTagFile::PrintCurrentDirectory (FILE* po_pOutput, uint32_t pi_Flag)
    {
    _PrintCurrentDirectory(po_pOutput, pi_Flag);
    }

void HTagFile::PrintDirectory (FILE* po_pOutput, DirectoryID pi_Dir, uint32_t pi_Flag)
    {

    HTagFile::DirectoryID CurDir = CurrentDirectory();
    if (!SetDirectory (pi_Dir))
        {
        fprintf (po_pOutput, "ERROR: Directory contains error or not found: %ld\n", pi_Dir);
        // Want to print tags even if we do not support the file.
        //return;
        }
    PrintCurrentDirectory(po_pOutput, pi_Flag);

    SetDirectory(CurDir);
    }

//
// Realloc_MAX_DIR_COUNT
//
// This method increase the memry for  m_ppListDir and m_pListDirOffset
// if necessary.
//
void HTagFile::Realloc_MAX_DIR_COUNT()
    {
    if (m_ListDirCount >= DEF_MAX_DIR_COUNT)
        {
        uint32_t i;
        int32_t NewCount = DEF_MAX_DIR_COUNT + DEF_MAX_DIR_COUNT_INCR;

        HTIFFDirectory** ppListDir      = new HTIFFDirectory*[NewCount];
        uint64_t*          pListDirOffset= new uint64_t[NewCount];
        memset(pListDirOffset, 0, sizeof(uint64_t)*NewCount);

        memset(ppListDir, 0, NewCount*sizeof(HTIFFDirectory*));
        // Copy the previous value
        for (i=0; i<m_ListDirCount; i++)
            {
            ppListDir[i] = m_ppListDir[i].get();
            m_ppListDir[i].release();
            }

        memcpy(pListDirOffset, m_pListDirOffset64, m_ListDirCount*sizeof(uint64_t));
        m_ppListDir = new HAutoPtr<HTIFFDirectory>[NewCount];

        for (i=0; i<m_ListDirCount; i++)
            m_ppListDir[i] = ppListDir[i];

        delete[] ppListDir;

        // Assign new pointer
        m_pListDirOffset64 = pListDirOffset;
        DEF_MAX_DIR_COUNT= NewCount;

        _PostReallocDirectories(NewCount);
        }
    }

//
// Realloc_MAX_HMR_DIR_COUNT
//
// This method increase the memory for m_HMRDirectories
// if necessary.
//
void HTagFile::Realloc_MAX_HMR_DIR_COUNT()
    {
    if (m_ListHMRDirCount >= DEF_MAX_HMR_DIR_COUNT)
        {
        HArrayAutoPtr<HAutoPtr<HMRDirectory64> > ppListHMRDir(m_ppListHMRDir64.release());
        uint32_t i;
        uint32_t NewCount = DEF_MAX_HMR_DIR_COUNT + DEF_MAX_HMR_DIR_COUNT_INCR;

        m_ppListHMRDir64 = new HAutoPtr<HMRDirectory64>[NewCount];

        // Copy the previous value
        for (i = 0; i < m_ListHMRDirCount; i++)
            m_ppListHMRDir64[i]                   = ppListHMRDir[i].release();

        DEF_MAX_HMR_DIR_COUNT = NewCount;
        }
    }


