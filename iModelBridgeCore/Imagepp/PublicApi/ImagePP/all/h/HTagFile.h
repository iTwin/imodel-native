//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTagFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTagFile
//-----------------------------------------------------------------------------

#pragma once

//#include <ImagePP/h/HTypes.h> included by hstdcpp.h
#include "HTiffUtils.h"
#include "HTIFFDirectory.h"
#include "HFCURL.h"
#include "HFCBinStreamLockManager.h"
#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE
class HTIFFDirectory;


class HTIFFDirectoryID
    {
    enum DirectoryType
        {
        STANDARD,
        HMR
        };

    HTIFFDirectoryID(DirectoryType pi_DirectoryType, uint32_t pi_DirNum);
    ~HTIFFDirectoryID();
    };

//-----------------------------------------------------------------------------
// HTagFile
//
//
// Requisites:
//     // TDORAY: Update if init method is provided
//     - "Construct" method must be invoked from derived class (in order to
//       permit polymorphic behavior on initialization)
//     - "SaveTagFile" method must be invoked from derived class (in order to
//       permit polymorphic behavior on destruction)
//-----------------------------------------------------------------------------
class HTagFile
    {
public:
    // Directory interface
    typedef enum
        {
        STANDARD,
        HMR
        } DirectoryType;

    typedef uint32_t DirectoryID;

    IMAGEPP_EXPORT static uint32_t             GetDirCountCapacity            ();

    static DirectoryID          MakeDirectoryID                (DirectoryType pi_DirectoryID, uint32_t pi_DirNum);
    static uint32_t             GetDirectoryNum                (DirectoryID pi_DirectoryID);
    static DirectoryType        GetDirectoryType               (DirectoryID pi_DirectoryID);

    IMAGEPP_EXPORT virtual                     ~HTagFile                      ();

    bool                       IsValid                        (HTIFFError**  po_ppError=0) const;

    IMAGEPP_EXPORT void                Save                           ();


    HFCPtr<HFCURL>              GetURL                         () const;

    // Is it a BigTiff == Tiff > 4Giga, many field are 64bits instead of 32bits.
    bool                       IsTiff64                       () const;


    bool                       IsStoredAsBigEndian            () const;

    void                        SetAsBigEndian                 (bool pi_AsBigEndian = true);

    IMAGEPP_EXPORT HFCBinStream*        GetFilePtr                     () const;
    // Returns a pointer on the HFCBinStreamLockManager object.
    HFCBinStreamLockManager*    GetLockManager                 () const;

    //HFCAccessMode               GetAccessMode                  () const;

    IMAGEPP_EXPORT uint32_t             NumberOfDirectory              (HTagFile::DirectoryType pi_DirType = STANDARD) const;

    IMAGEPP_EXPORT bool        AppendDirectory                ();
    bool                       AddHMRDirectory                (HTagID pi_HMRTag);

    IMAGEPP_EXPORT bool                SetDirectory                   (DirectoryID pi_DirID);
    DirectoryID                 CurrentDirectory               () const;

    uint64_t                   DirectoryOffset                (DirectoryID pi_DirID) const;

    void                        PrintCurrentDirectory          (FILE* po_pOutput, uint32_t pi_Flag);
    IMAGEPP_EXPORT void                 PrintDirectory                 (FILE* po_pOutput, DirectoryID pi_Dir, uint32_t pi_Flag);


    const char*                 GetTagNameString               (HTagID pi_Tag) const;
    HTagInfo::DataType          GetTagDataType                 (HTagID pi_Tag) const;

    bool                       ReadDirectories                (bool pi_ValidateDir = true);
    void                        WriteDirectories               ();

    bool                       TagIsPresent                   (HTagID pi_Tag) const;
    bool                       IsVariableSizeTag              (HTagID pi_Tag) const;
    bool                       RemoveTag                      (HTagID pi_Tag);

    bool                       GetField                       (HTagID pi_Tag, unsigned short* po_pVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pVal) const;
    bool                       GetField                       (HTagID pi_Tag, double* po_pVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint64_t* po_pVal) const;
    bool                       GetField                       (HTagID pi_Tag, char** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, WChar** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, unsigned short* po_pVal1, unsigned short* po_pVal2) const;

    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pCount, uint32_t** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pCount, double** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pCount, Byte** po_ppVal) const;
    bool                       GetField                       (HTagID pi_Tag, uint32_t* po_pCount, uint64_t** po_ppVal) const;

    bool                       SetField                       (HTagID pi_Tag, unsigned short pi_Val);
    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Val);
    bool                       SetField                       (HTagID pi_Tag, double pi_Val);
    bool                       SetField                       (HTagID pi_Tag, uint64_t pi_Val);
    bool                       SetFieldA                      (HTagID pi_Tag, const char* pi_pVal);
    bool                       SetFieldW                      (HTagID pi_Tag, const WChar* pi_pVal);
    bool                       SetField                       (HTagID pi_Tag, unsigned short pi_Val1, unsigned short pi_Val2);

    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Count, const Byte* pi_pVal);
    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal);
    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Count, const uint32_t* pi_pVal);
    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Count, const double* pi_pVal);
    bool                       SetField                       (HTagID pi_Tag, uint32_t pi_Count, const uint64_t* pi_pVal);

protected:

    typedef unsigned short     MagicNumber;

    IMAGEPP_EXPORT explicit    HTagFile                       (const WString&          pi_rFilename,
                                                                const HTagInfo&         pi_rTagInfo,
                                                                HFCAccessMode           pi_Mode,
                                                                uint64_t               pi_OriginOffset = 0,
                                                                bool                   pi_CreateBigTifFormat = false,
                                                                bool                   pi_ValidateDir = true,
                                                                bool*                  po_pNewFile = 0);

    explicit                    HTagFile                       (const HFCPtr<HFCURL>&   pi_rpURL,
                                                                const HTagInfo&         pi_rTagInfo,
                                                                HFCAccessMode           pi_AccessMode = HFC_READ_ONLY,
                                                                bool                   pi_CreateBigTifFormat = false,
                                                                bool*                  po_pNewFile = 0);




    IMAGEPP_EXPORT void         Construct                      (const HFCPtr<HFCURL>&   pi_rpURL,           // by URL
                                                                const WString*          pi_pFilename,              // by string
                                                                HFCAccessMode           pi_Mode,
                                                                uint64_t               pi_OriginOffset,
                                                                bool                   pi_CreateBigTifFormat = false,
                                                                bool                   pi_ValidateDir = true,
                                                                bool*                  po_pNewFile = 0);


    IMAGEPP_EXPORT void         SaveTagFile                    ();

    uint32_t                    GetCurrentDirIndex             () const;
    DirectoryType               GetCurrentDirType              () const;
    DirectoryID                 GetCurrentDirID                () const;

    IMAGEPP_EXPORT HTIFFDirectory&             GetCurrentDir                  ();
    IMAGEPP_EXPORT const HTIFFDirectory&       GetCurrentDir                  () const;

    IMAGEPP_EXPORT bool                SetDirectoryImpl               (HTagFile::DirectoryID   pi_DirID,
                                                                bool                   pi_ReloadCodec);

    void                        SetOffsetCountData             (uint32_t pi_NbData, uint64_t* pi_pOffset, uint32_t* pi_pCount);
    IMAGEPP_EXPORT bool                       SetOffset                      (size_t pi_Index, uint64_t pi_Offset);
    IMAGEPP_EXPORT bool                       SetCount                       (size_t pi_Index, uint64_t pi_Count);
    IMAGEPP_EXPORT void                        FreeOffsetCount                ();
    IMAGEPP_EXPORT void                        ReadOffsetCountTags            ();

    uint64_t                   GetOffset                      (size_t pi_Index) const;
    uint32_t                    GetCount                       (size_t pi_Index) const;


    void                        Realloc_MAX_DIR_COUNT          ();
    void                        Realloc_MAX_HMR_DIR_COUNT      ();

    void                        InsertSortFreeBlock            ();
    void                        InsertSortMergedDirectories    ();


    void                        CompareDataWithBlockFree       (uint64_t p_PositionBlockFree,
                                                                uint32_t&  p_PositionSortedData,
                                                                uint32_t&  p_StartPositionSortedData);

    void                        SetFreeBlockList               ();
    bool                       DataFreeBlockCanBeMerged       (uint32_t& p_Position);
    void                        GetDataForEachTag              (uint32_t p_Index,
                                                                bool p_isHMRDir);


    bool                       m_AllocatedOffset;          // true --> the list is allocated internally.
    uint32_t*                     m_pOffset32;
    uint64_t*                    m_pOffset64;

    bool                       m_AllocatedCount;           // true --> the list is allocated internally.
    uint32_t*                     m_pCount32;
    uint64_t*                    m_pCount64;

    bool                       m_DataWasModified;      // tile or strip were written.

    //-------------------------- Offset Count tags ---------------------------------------------------
    uint32_t                    m_NbData32;         // Number of entry (Tiles/Strip).
    // Contig planar = NumberOfStrips or NumberOfTiles
    // Separate planar = m_SamplesByPixel * (NumberOfStrips or NumberOfTiles)

    // Thread-Safety
    mutable HFCExclusiveKey     m_Key;

    typedef    struct                   // Big Tiff support (> 4Gig)
        {
        MagicNumber Magic;              // Magic number (defines byte order)
        unsigned short Version;            // TIFF version (42 or 43 for BigTiff)
        unsigned short BytesizeOfOffset;   // BigTiff only - Always 8 in BigTiff
        unsigned short Reserved;           // BigTiff only - Always 0
        uint64_t   DirOffset64;        // Offset to first directory
        } HeaderFile64;

    // Members
    HTIFFByteOrdering           m_ByteOrder;
    HTIFFError*                 m_pError;

    HAutoPtr<HTIFFStream>       m_pFile;
    HeaderFile64                m_Header;

    // Current Directory and Information
    DirectoryID                 m_CurDir;
    HTIFFDirectory*             m_pCurDir;

    bool                       m_EndianAsChanged;      // If True, force rewriting the file header.

    uint64_t                   m_PositionInFile;


    vector<OffsetAndSize>       m_MergedDirectories;
    vector<FreeblockData>       m_FreeBlockDirectory;

    uint64_t                   m_NbByteBlockFree;

    // List of directories in this file
    HArrayAutoPtr<HAutoPtr<HTIFFDirectory> >
    m_ppListDir;
    HArrayAutoPtr<uint64_t>      m_pListDirOffset64;
    uint32_t                    m_ListDirCount;

    // HMR Directory
    struct HMRDirectory64
        {
        HAutoPtr<HTIFFDirectory>    m_pDirectory;
        uint64_t                   m_DirOffset64;
        uint32_t                    m_ParentDirIndex;
        HTagID                      m_HMRTagUsed;
        };
    HArrayAutoPtr<HAutoPtr<HMRDirectory64> >
    m_ppListHMRDir64;
    uint32_t                    m_ListHMRDirCount;


    HAutoPtr<HFCBinStreamLockManager>
    m_pLockManager;

    const HTagInfo&             m_rTagInfo;

private:

    void                        Initialize                     ();


    bool                       OpenTiffFile                   (const HFCPtr<HFCURL>*   pi_pURL,        // by URL
                                                                const WString*          pi_pFilename,         // by string
                                                                HFCAccessMode           pi_AccessMode,
                                                                uint64_t               pi_OriginOffset=0);

    bool                       CreateTiffFile                 (const HFCPtr<HFCURL>*   pi_pURL,
                                                                const WString*          pi_pFilename,
                                                                HFCAccessMode           pi_AccessMode,
                                                                bool                   pi_CreateBigTifFormat=false);


    virtual MagicNumber         GetLittleEndianMagicNumber     () const = 0;
    virtual MagicNumber         GetBigEndianMagicNumber        () const = 0;

    // NOTE: Should have been moved to HTagInfo, but the dynamic behavior in HTIFF forces us to keep it here.
    virtual HTagID              GetPacketOffsetsTagID          () const = 0;
    virtual HTagID              GetPacketByteCountsTagID       () const = 0;


    virtual uint32_t            _NumberOfDirectory             (HTagFile::DirectoryType pi_DirType = STANDARD) const = 0;

    virtual bool               DirectoryIsValid               (HTIFFDirectory*         pi_Dir,
                                                                HTIFFDirectory*         pi_pCurPageDir) = 0;

    // TDORAY: Find a way to remove it from here. Not generic.
    virtual bool               IsValidReducedImage            (HTIFFDirectory*         pi_ReducedImageDir,
                                                                HTIFFDirectory*         pi_pCurPageDir) = 0;

    virtual void                _PrintCurrentDirectory         (FILE* po_pOutput, uint32_t pi_Flag) = 0;

    // Invoked when the top directory is first initialized and validation is required
    virtual bool               IsValidTopDirectory            (HTIFFError**            po_ppError = 0) = 0;

    // Invoked when the top directory is first initialized
    virtual void                OnTopDirectoryFirstInitialized () = 0;

    // Invoked after directories are reallocated
    virtual void                _PostReallocDirectories        (uint32_t                pi_NewDirCountCapacity) = 0;

    // Invoked before any tags for the current directory is written to the file
    virtual void                _PreWriteCurrentDir            (HTagFile::DirectoryID pi_DirID) = 0;

    // Invoked before current directory is changed
    virtual bool               PreCurrentDirectoryChanged     (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) = 0;

    // Invoked only when current directory is changed successfully
    virtual bool               OnCurrentDirectoryChanged      (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) = 0;

    // Always invoked after current directory is set
    virtual bool               PostCurrentDirectorySet        (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) = 0;
    };
END_IMAGEPP_NAMESPACE


#include "HTagFile.hpp"
