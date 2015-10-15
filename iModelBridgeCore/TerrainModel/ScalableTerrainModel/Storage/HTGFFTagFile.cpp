//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFTagFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFTagFile.h>


#include <ImagePP/all/h/HTiffUtils.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecZlib.h>

#include <STMInternal/Storage/HTGFFPacketIdIter.h>
#include <STMInternal/Storage/HTGFFSubDirIdIter.h>
#include <ImagePP/all/h/HTagIdIterator.h>

;

namespace HTGFF {

static const uint64_t EMPTY_PACKET_OFFSET = (numeric_limits<uint64_t>::max) ();


/*---------------------------------------------------------------------------------**//**
* @description  Factory that creates customized codecs for a specified compression.
*
* @see          HTagFile
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class CodecFactory
    {
public:
    HFCPtr<HCDCodec>                    CreateFor                      (const Compression&                 pi_rCompression);

    };

HFCPtr<HCDCodec> CodecFactory::CreateFor (const Compression&   pi_rCompression)
    {
    HFCPtr<HCDCodec> pCodec;

    struct CompressVisitor : Compression::Visitor
        {

        explicit CompressVisitor (HFCPtr<HCDCodec>& pi_rpCodec) : m_rpCodec(pi_rpCodec) {}

        virtual void    _Visit       (const Compression::None&      pi_rCompression)
            {
            m_rpCodec = new HCDCodecIdentity();
            }

        virtual void    _Visit       (const Compression::Deflate&   pi_rCompression)
            {
            // TDORAY: Get specific deflate codecs
            m_rpCodec = new HCDCodecZlib();
            }

    private:
        HFCPtr<HCDCodec>& m_rpCodec;
        };

    CompressVisitor visitor(pCodec);
    pi_rCompression.Accept(visitor);

    return pCodec;
    }


//-----------------------------------------------------------------------------
// Per directory cached states. One instance of this class is spawned when
// accessing a directory for the first time (lazy initialization). Information
// cached here is only stored when saving the file.
//-----------------------------------------------------------------------------
struct TagFile::DirInfo
    {
    private:
        // Disable copy
                                DirInfo                        (const DirInfo&);
        DirInfo&                operator=                      (const DirInfo&);

    public:
        explicit                DirInfo                        ();

        bool                    m_Dirty;
        uint64_t               m_PacketCount;
        uint64_t               m_DataSize;
        uint64_t               m_UncompressedDataSize;
        uint64_t               m_MaxPacketSize;
        uint64_t               m_MaxUncompressedPacketSize;
        DataType                m_DataType;
        Compression             m_DataCompressType;
        HFCPtr<HCDCodec>        m_pCodec;
    };




TagFile::DirInfo::DirInfo ()
    :   m_Dirty(false),
        m_PacketCount(0),
        m_DataSize(0),
        m_UncompressedDataSize(0),
        m_MaxPacketSize(0),
        m_MaxUncompressedPacketSize(0),
        m_DataType(),
        m_DataCompressType(Compression::None::Create())
    {
    }



TagFile::DirInfoList::DirInfoList (uint32_t directoryCount)
    :   m_array(directoryCount)
    {
    }

TagFile::DirInfoList::~DirInfoList ()
    {
    for_each(m_array.begin(), m_array.end(), DestroyReinitPtr());
    }


inline TagFile::DirInfo& TagFile::DirInfoList::Edit (uint32_t idx)
    {
    HPRECONDITION(0 != m_array[idx]);
    DirInfo& info = *m_array[idx];
    info.m_Dirty = true;
    return info;
    }

inline const TagFile::DirInfo& TagFile::DirInfoList::Get (uint32_t idx) const
    {
    HPRECONDITION(0 != m_array[idx]);
    return *m_array[idx];
    }


inline void TagFile::DirInfoList::Remove (uint32_t idx)
    {
    delete m_array[idx];
    m_array[idx] = 0;
    }

inline void TagFile::DirInfoList::Resize (uint32_t newSize)
    {
    m_array.resize(newSize, 0);
    }

namespace {

template <typename T>
bool LoadFromTag      (const HTIFFDirectory&    pi_rDir,
                        HTagID                   pi_TagID,
                        T&                       po_rData)
    {
    if (pi_rDir.TagIsPresent(pi_TagID))
        return const_cast<HTIFFDirectory&>(pi_rDir).GetValues(pi_TagID, &po_rData);
    return true;
    }
}

bool TagFile::DirInfoList::Load   (const TagFile&  file)
    {
    const uint32_t CurrentDirIndex = file.GetCurrentDirIndex();
    if (0 != m_array[CurrentDirIndex])
        return true;

    m_array[CurrentDirIndex] = new DirInfo;
    DirInfo& rCurrentDirInfo = *m_array[CurrentDirIndex];


    const HTIFFDirectory& dir = file.GetCurrentDir();

    bool Success = true;
    Success &= LoadFromTag(dir, file.GetPacketCountTagID(), rCurrentDirInfo.m_PacketCount);
    Success &= LoadFromTag(dir, file.GetDataSizeTagID(), rCurrentDirInfo.m_DataSize);
    Success &= LoadFromTag(dir, file.GetUncompressedDataSizeTagID(), rCurrentDirInfo.m_UncompressedDataSize);
    Success &= LoadFromTag(dir, file.GetMaxPacketSizeTagID(), rCurrentDirInfo.m_MaxPacketSize);
    Success &= LoadFromTag(dir, file.GetMaxUncompressedPacketSizeTagID(), rCurrentDirInfo.m_MaxUncompressedPacketSize);

    Success &= UpdateDataType(dir, file, rCurrentDirInfo);
    Success &= UpdateDataCompressType(dir, file, rCurrentDirInfo);

    return Success;
    }


bool TagFile::DirInfoList::UpdateDataType (const HTIFFDirectory&   dir,
                                            const TagFile&          file,
                                            DirInfo&                dirInfo)
{
    bool Success = true;

    // Get current directory's point type
    uint32_t* pDimension   = 0;
    uint32_t DimensionQty = 0;
    if (dir.TagIsPresent(file.GetDataDimensionsTypeTagID()))
    {
        Success &= const_cast<HTIFFDirectory&>(dir).GetValues(file.GetDataDimensionsTypeTagID(), &DimensionQty, &pDimension);
        dirInfo.m_DataType = DataType(reinterpret_cast<Dimension::Type*>(pDimension),
                                                      reinterpret_cast<Dimension::Type*>(pDimension + DimensionQty));
    }
    else
    {
        dirInfo.m_DataType = DataType::CreateVoid();
    }

    uint32_t* pRole   = 0;
    uint32_t RoleQty = 0;
    if (dir.TagIsPresent(file.GetDataDimensionsRoleTagID()))
    {
        Success &= const_cast<HTIFFDirectory&>(dir).GetValues(file.GetDataDimensionsRoleTagID(), &RoleQty, &pRole);
        HASSERT(RoleQty == DimensionQty);
        copy(pRole, pRole + MIN(RoleQty, DimensionQty), SetterIter(dirInfo.m_DataType.Begin(), &Dimension::SetRole));
    }

    return Success;
}

bool TagFile::DirInfoList::UpdateDataCompressType (const HTIFFDirectory&   dir,
                                                    const TagFile&          file,
                                                    DirInfo&                dirInfo)
{
    bool Success = true;

    // Get current directory's compress type
    uint32_t CompressType = Compression::TYPE_NONE;
    if (dir.TagIsPresent(file.GetDataCompressTypeTagID()))
        Success &= const_cast<HTIFFDirectory&>(dir).GetValues(file.GetDataCompressTypeTagID(), &CompressType);


    switch (CompressType)
    {
    case Compression::TYPE_NONE:
        dirInfo.m_DataCompressType = Compression::None::Create();
        break;
    case Compression::TYPE_DEFLATE:
        dirInfo.m_DataCompressType = Compression::Deflate::Create();
        break;
    default:
        HASSERT(!"Unsupported compress type");
        Success = false;
        CompressType = Compression::TYPE_NONE;
        break;
    }

    dirInfo.m_pCodec = CodecFactory().CreateFor(dirInfo.m_DataCompressType);

    return Success;
}


bool TagFile::DirInfoList::UpdateDataType (const TagFile&  file)
{
    HPRECONDITION(0 != m_array[file.GetCurrentDirID()]);
    return UpdateDataType(file.GetCurrentDir(), file, *m_array[file.GetCurrentDirID()]);
}

bool TagFile::DirInfoList::UpdateDataCompressType (const TagFile&  file)
{
    HPRECONDITION(0 != m_array[file.GetCurrentDirID()]);
    return UpdateDataCompressType(file.GetCurrentDir(), file, *m_array[file.GetCurrentDirID()]);
}


namespace {

template <typename T>
bool SaveToTagIfNotEqualTo            (HTIFFDirectory&         pio_rDir,
                                        HTagID                  pi_TagID,
                                        const T&                pi_rData,
                                        const T&                pi_rComparisonValue = T())
    {
    if (pi_rComparisonValue != pi_rData)
        return pio_rDir.SetValues(pi_TagID, pi_rData);

    if (pio_rDir.TagIsPresent(pi_TagID))
        return pio_rDir.Remove(pi_TagID);

    return true;
    }
}

bool TagFile::DirInfoList::Save   (TagFile&    file)
    {
    const uint32_t CurrentDirIndex = file.GetCurrentDirIndex();
    HPRECONDITION(0 != m_array[CurrentDirIndex]);

    const DirInfo& rCurrentDirInfo = *m_array[CurrentDirIndex];
    if (!rCurrentDirInfo.m_Dirty)
        return true;

    HTIFFDirectory& dir = file.GetCurrentDir();

    bool Success = true;

    Success &= SaveToTagIfNotEqualTo(dir, file.GetPacketCountTagID(), rCurrentDirInfo.m_PacketCount);
    Success &= SaveToTagIfNotEqualTo(dir, file.GetDataSizeTagID(), rCurrentDirInfo.m_DataSize);
    Success &= SaveToTagIfNotEqualTo(dir, file.GetUncompressedDataSizeTagID(), rCurrentDirInfo.m_UncompressedDataSize);
    Success &= SaveToTagIfNotEqualTo(dir, file.GetMaxPacketSizeTagID(), rCurrentDirInfo.m_MaxPacketSize);
    Success &= SaveToTagIfNotEqualTo(dir, file.GetMaxUncompressedPacketSizeTagID(), rCurrentDirInfo.m_MaxUncompressedPacketSize);

    return Success;
    }



TagFile::TagFile   (const WString&          pi_rInputFilePath,
                    const HFCAccessMode&    pi_rAccessMode,
                    const Definition&       pi_rFileDefinition)
    :   HTagFile(pi_rInputFilePath, pi_rFileDefinition.GetTagInfo(), pi_rAccessMode, 0, true),
        m_AccessMode(pi_rAccessMode),
        m_rFileDefinition(pi_rFileDefinition),
        m_NextPacketID(0),
        m_DirectoriesInfo(HTagFile::GetDirCountCapacity()),
        m_pCompressedPacket(new HCDPacket())
    {
    m_pCompressedPacket->SetBufferOwnership(true);

    // See HTagFile class documentation
    Construct(0, &pi_rInputFilePath, pi_rAccessMode, 0, true);
    }

TagFile::~TagFile ()
    {
    // See HTagFile class documentation
    SaveTagFile();
    }


/*
 * HTagFile required file information
 */
TagFile::MagicNumber TagFile::GetLittleEndianMagicNumber    () const {
    return m_rFileDefinition.GetLittleEndianMagicNumber();
    }
TagFile::MagicNumber TagFile::GetBigEndianMagicNumber       () const {
    return m_rFileDefinition.GetBigEndianMagicNumber();
    }

/*
 * HTagFile required tags
 */
HTagID TagFile::GetPacketOffsetsTagID                       () const {
    return m_rFileDefinition.GetTagInfo().GetPacketOffsetsTagID();
    }
HTagID TagFile::GetPacketByteCountsTagID                    () const {
    return m_rFileDefinition.GetTagInfo().GetPacketByteCountsTagID();
    }


/*
 * Internal tags aliases
 */

inline HTagID TagFile::GetVersionTagID                      () const {
    return m_rFileDefinition.GetTagInfo().GetVersionTagID();
    }
inline HTagID TagFile::GetDataDimensionsTypeTagID           () const {
    return m_rFileDefinition.GetTagInfo().GetDataDimensionsTypeTagID();
    }
inline HTagID TagFile::GetDataDimensionsRoleTagID           () const {
    return m_rFileDefinition.GetTagInfo().GetDataDimensionsRoleTagID();
    }
inline HTagID TagFile::GetDataCompressTypeTagID             () const {
    return m_rFileDefinition.GetTagInfo().GetDataCompressTypeTagID();
    }

inline HTagID TagFile::GetPacketCountTagID                  () const {
    return m_rFileDefinition.GetTagInfo().GetPacketCountTagID();
    }
inline HTagID TagFile::GetDataSizeTagID                     () const {
    return m_rFileDefinition.GetTagInfo().GetDataSizeTagID();
    }
inline HTagID TagFile::GetUncompressedDataSizeTagID         () const {
    return m_rFileDefinition.GetTagInfo().GetUncompressedDataSizeTagID();
    }
inline HTagID TagFile::GetMaxPacketSizeTagID                () const {
    return m_rFileDefinition.GetTagInfo().GetMaxPacketSizeTagID();
    }
inline HTagID TagFile::GetMaxUncompressedPacketSizeTagID    () const {
    return m_rFileDefinition.GetTagInfo().GetMaxUncompressedPacketSizeTagID();
    }

inline HTagID TagFile::GetSubDirectoryListingTagID          () const {
    return m_rFileDefinition.GetTagInfo().GetSubDirectoryListingTagID();
    }
inline HTagID TagFile::GetRemovedSubDirectoryListingTagID   () const {
    return m_rFileDefinition.GetTagInfo().GetRemovedDirectoryListingTagID();
    }





inline TagFile::DirInfo& TagFile::EditCurrentDirInfo ()
    {
    return m_DirectoriesInfo.Edit(GetCurrentDirIndex());
    }

inline const TagFile::DirInfo& TagFile::GetCurrentDirInfo () const
    {
    return m_DirectoriesInfo.Get(GetCurrentDirIndex());
    }


const DataType& TagFile::GetDataType () const
    {
    return GetCurrentDirInfo().m_DataType;
    }

const Compression& TagFile::GetDataCompressType () const
    {
    return GetCurrentDirInfo().m_DataCompressType;
    }

bool TagFile::IsDataCompressed () const
    {
    return Compression::TYPE_NONE != GetCurrentDirInfo().m_DataCompressType.GetType();
    }

double TagFile::GetCompressionRatio () const
    {
    HFCMonitor Monitor(m_Key);
    return (IsDataCompressed() && 0 != GetCurrentDirInfo().m_UncompressedDataSize) ?
           ((double)GetCurrentDirInfo().m_DataSize / GetCurrentDirInfo().m_UncompressedDataSize)
           : 1.0;
    }

bool TagFile::SetVersion (uint32_t pi_version)
    {
    // Version is not saved to file when 0
    if (0 == pi_version)
        return true;

    // Do not set version if already present
    if (GetCurrentDir().TagIsPresent(GetVersionTagID()))
        return true;

    return GetCurrentDir().SetValues(GetVersionTagID(), static_cast<uint32_t>(pi_version));
    }


uint32_t TagFile::GetVersion () const
    {
    uint32_t version= 0;
    const_cast<TagFile&>(*this).GetCurrentDir().GetValues(GetVersionTagID(), &version);
    return static_cast<uint32_t>(version);
    }

bool TagFile::SetDataType (const DataType& pi_DataType)
    {
    HPRECONDITION(0 < pi_DataType.GetDimensionQty());
    HPRECONDITION(!GetCurrentDir().TagIsPresent(GetDataDimensionsTypeTagID()));

    HArrayAutoPtr<uint32_t> pDimensionsType(new uint32_t[pi_DataType.GetDimensionQty()]);
    transform(pi_DataType.Begin(), pi_DataType.End(), pDimensionsType.get(), mem_fun_ref(&Dimension::GetType));

    if (!GetCurrentDir().SetValues(GetDataDimensionsTypeTagID(), static_cast<uint32_t>(pi_DataType.GetDimensionQty()), pDimensionsType))
        return false;

    HArrayAutoPtr<uint32_t> pDimensionsRole(new uint32_t[pi_DataType.GetDimensionQty()]);
    transform(pi_DataType.Begin(), pi_DataType.End(), pDimensionsRole.get(), mem_fun_ref(&Dimension::GetRole));

    if (!GetCurrentDir().SetValues(GetDataDimensionsRoleTagID(), static_cast<uint32_t>(pi_DataType.GetDimensionQty()), pDimensionsRole))
        return false;

    return m_DirectoriesInfo.UpdateDataType(*this);
    }

bool TagFile::SetDataCompressType (const Compression& pi_DataCompress)
    {
    HPRECONDITION(!GetCurrentDir().TagIsPresent(GetDataCompressTypeTagID()));

    if (!GetCurrentDir().SetValues(GetDataCompressTypeTagID(), uint32_t(pi_DataCompress.GetType())))
        return false;


    return m_DirectoriesInfo.UpdateDataCompressType(*this);
    }



const HFCAccessMode& TagFile::GetAccessMode () const
    {
    return m_AccessMode;
    }

TagFile::PacketID TagFile::GetNextAvailablePacketID ()
    {
    // Increment next packet id until we find an available slot
    while (DoesPacketExist(m_NextPacketID)) {
        ++m_NextPacketID;
        }
    return m_NextPacketID;
    }


bool TagFile::ReleasePacketID (PacketID pi_PacketID)
    {
    if (!DoesPacketExist(pi_PacketID))
        return false;

    if (0 != GetCount(pi_PacketID))
        m_pFile->ReleaseBlock(GetOffset(pi_PacketID), GetCount(pi_PacketID));

    SetOffset(pi_PacketID, 0);
    SetCount(pi_PacketID, 0);

    // Notify that offset and byte count changed and that corresponding tags need to be updated
    GetCurrentDir().Touched(GetPacketOffsetsTagID());
    GetCurrentDir().Touched(GetPacketByteCountsTagID());

    if (pi_PacketID < m_NextPacketID)
        m_NextPacketID = pi_PacketID;

    return true;
    }

size_t TagFile::GetPacketCount () const
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(GetCurrentDirInfo().m_PacketCount <= (numeric_limits<size_t>::max)());
    return static_cast<size_t>(GetCurrentDirInfo().m_PacketCount);
    }

uint64_t TagFile::GetTotalCompressedPacketSize () const
    {
    HFCMonitor Monitor(m_Key);
    return GetCurrentDirInfo().m_DataSize;
    }

uint64_t TagFile::GetTotalUncompresedPacketSize  () const
    {
    HFCMonitor Monitor(m_Key);
    return IsDataCompressed() ? GetCurrentDirInfo().m_UncompressedDataSize : GetCurrentDirInfo().m_DataSize;
    }


size_t TagFile::GetMaxCompressedPacketSize () const
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(GetCurrentDirInfo().m_MaxPacketSize <= (numeric_limits<size_t>::max)());
    return static_cast<size_t>(GetCurrentDirInfo().m_MaxPacketSize);
    }

size_t TagFile::GetMaxUncompressedPacketSize () const
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(GetCurrentDirInfo().m_MaxUncompressedPacketSize <= (numeric_limits<size_t>::max)());
    HPRECONDITION(GetCurrentDirInfo().m_MaxPacketSize <= (numeric_limits<size_t>::max)());
    return static_cast<size_t>(IsDataCompressed() ? GetCurrentDirInfo().m_MaxUncompressedPacketSize
                               : GetCurrentDirInfo().m_MaxPacketSize);
    }


size_t TagFile::GetCompressedPacketSize (PacketID pi_PacketID) const
    {
    if (pi_PacketID >= GetPacketCountCapacity())
        return 0;

    return GetCount(pi_PacketID);
    }

size_t TagFile::GetUncompressedPacketSize (PacketID pi_PacketID) const
    {
    if (pi_PacketID >= GetPacketCountCapacity())
        return 0;

    if (!IsDataCompressed())
        return GetCount(pi_PacketID);

    return GetUncompressedPacketSizeImpl(pi_PacketID);
    }


size_t TagFile::GetUncompressedPacketSizeImpl  (PacketID pi_PacketID) const
    {
    if (pi_PacketID >= GetPacketCountCapacity() || (0 == GetCount(pi_PacketID)))
        return 0;

    uint32_t UncompressedDataSize;
    if ((!m_pFile->Seek (GetOffset(pi_PacketID), SEEK_SET)) ||
        (m_pFile->Read (&UncompressedDataSize, 1, sizeof(UncompressedDataSize)) != sizeof(UncompressedDataSize)))
        return 0;

    // TODO: Consider endianess
    return UncompressedDataSize;
}

namespace {

const HTagFile::DirectoryID ROOT_DIR_ID = HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0);

}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::GetRemovedSubDirectoryListingField  (uint32_t*         po_pCount,
                                                    DirectoryID**   po_ppVal)
    {
    const HTagFile::DirectoryID CurDir = CurrentDirectory();
    if (ROOT_DIR_ID == CurDir)
        return GetField(GetRemovedSubDirectoryListingTagID(), po_pCount, po_ppVal);

    const bool Success = SetDirectory (ROOT_DIR_ID) &&
                          GetField(GetRemovedSubDirectoryListingTagID(), po_pCount, po_ppVal);
    const bool RestoreDirSuccess = SetDirectory(CurDir);
    HASSERT(RestoreDirSuccess);

    return Success && RestoreDirSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::SetRemovedSubDirectoryListingField  (uint32_t        pi_Count, 
                                                    const uint32_t*   pi_pVal)
    {
    HTagFile::DirectoryID CurDir = CurrentDirectory();
    if (ROOT_DIR_ID == CurDir)
        return SetField(GetRemovedSubDirectoryListingTagID(), pi_Count, pi_pVal);

    const bool Success = SetDirectory (ROOT_DIR_ID) &&
                          SetField(GetRemovedSubDirectoryListingTagID(), pi_Count, pi_pVal);
    const bool RestoreDirSuccess = SetDirectory(CurDir);
    HASSERT(RestoreDirSuccess);

    return Success && RestoreDirSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveRemovedSubDirectoryListingTag ()
    {
    HTagFile::DirectoryID CurDir = CurrentDirectory();
    if (ROOT_DIR_ID == CurDir)
        return RemoveTag(GetRemovedSubDirectoryListingTagID());

    const bool Success = SetDirectory (ROOT_DIR_ID) && 
                          RemoveTag(GetRemovedSubDirectoryListingTagID());
    const bool RestoreDirSuccess = SetDirectory(CurDir);
    HASSERT(RestoreDirSuccess);

    return Success && RestoreDirSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static TagFile::DirectoryID* GetRealSubDirListingEnd (TagFile::DirectoryID* pSubDirListingBegin, TagFile::DirectoryID* pSubDirListingEnd)
    {
    return std::find(pSubDirListingBegin, pSubDirListingEnd, TagFile::DirectoryID(0));
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::AddSubDirToListing (DirectoryID pi_DirectoryID)
    {
    uint32_t     SubDirListingCapacity = 0;
    DirectoryID* pSubDirListingBegin = 0;
    GetField(GetSubDirectoryListingTagID(), &SubDirListingCapacity, &pSubDirListingBegin);

    DirectoryID* pSubDirListingCapacityEnd = pSubDirListingBegin + SubDirListingCapacity;

    // First zeroed directory id is our end
    DirectoryID* pSubDirListingEnd = GetRealSubDirListingEnd(pSubDirListingBegin, pSubDirListingCapacityEnd);

    if (pSubDirListingEnd != pSubDirListingCapacityEnd)
        {
        *pSubDirListingEnd = pi_DirectoryID;
        return true;
        }

    HArrayAutoPtr<DirectoryID>  pNewDirectoryIds;
    const uint32_t              NewDirectoryIdQty = SubDirListingCapacity + 1;

    expandCopyBuffer(pSubDirListingBegin, SubDirListingCapacity, pNewDirectoryIds, NewDirectoryIdQty, DirectoryID(0));

    *(pNewDirectoryIds.get() + SubDirListingCapacity) = pi_DirectoryID;

    if (!SetField(GetSubDirectoryListingTagID(), NewDirectoryIdQty, pNewDirectoryIds.get()))
        {
        HASSERT(!"Unable to initialize sub directory ids");
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*bool TagFile::RemoveSubDirFromListing (DirectoryID pi_DirectoryID)
{
    UInt32       DirectoryQty = 0;
    DirectoryID* pDirectoryIds = 0;
    const bool Success = GetField(GetDirSubDirectoryListingTagID(), &DirectoryQty, &pDirectoryIds);

    // First zeroed directory id is our end
    DirectoryID* pDirectoryIdsEnd = std::find(pDirectoryIds, pDirectoryIds + DirectoryQty, DirectoryID(0));
    DirectoryID* pNewDirectoryIdsEnd = std::remove(pDirectoryIds, pDirectoryIdsEnd, pi_DirectoryID);

    // Tag is not required anymore
    if (pDirectoryIds == pNewDirectoryIdsEnd)
        return RemoveTag(GetDirSubDirectoryListingTagID());

    // Edit tag directly in memory. Fill remaining space with zero directory ids
    std::fill(pNewDirectoryIdsEnd,
              pDirectoryIdsEnd,
              DirectoryID(0));

    return Success;
}*/

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveSubDirTagFromListing (HTagID pi_SubDirsTagID)
    {
    bool Success = true;

    if (pi_SubDirsTagID != GetSubDirectoryListingTagID())
        {
        uint32_t     SubDirListingCapacity = 0;
        DirectoryID* pSubDirListingBegin = 0;
        GetField(GetSubDirectoryListingTagID(), &SubDirListingCapacity, &pSubDirListingBegin);

        // First zeroed directory id is our end
        DirectoryID* pSubDirListingEnd = GetRealSubDirListingEnd(pSubDirListingBegin, pSubDirListingBegin + SubDirListingCapacity);


        // Remove directory described in the tag from our sub directories listing.
        DirectoryID* pNewSubDirListingEnd = pSubDirListingEnd;

        SubDirIdIter subDirId = SubDirIDsBegin(pi_SubDirsTagID);
        SubDirIdIter subDirIdEnd = SubDirIDsEnd(pi_SubDirsTagID);

        for (; subDirId != subDirIdEnd; ++subDirId)
            {
            pNewSubDirListingEnd = std::remove(pSubDirListingBegin, pNewSubDirListingEnd, *subDirId);
            }

        // Edit tag directly in memory. Fill remaining space with zero directory ids
        std::fill(pNewSubDirListingEnd,
                  pSubDirListingEnd,
                  DirectoryID(0));

        // Listing tag is not required anymore. Remove it.
        if (pSubDirListingBegin == pNewSubDirListingEnd)
            Success &= (TagIsPresent(GetSubDirectoryListingTagID())) ? RemoveTag(GetSubDirectoryListingTagID()) : true;

        }

    // Remove the tag
    Success &= (TagIsPresent(pi_SubDirsTagID)) ? RemoveTag(pi_SubDirsTagID) : true;
    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*bool TagFile::RemoveAllSubDirFromListing ()
{
    return RemoveTag(GetDirSubDirectoryListingTagID());
}*/


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveSubDirTag  (HTagID pi_SubDirsTagID)
    {
    uint32_t     SubDirListingCapacity = 0;
    DirectoryID* pSubDirListingBegin = 0;
    GetRemovedSubDirectoryListingField(&SubDirListingCapacity, &pSubDirListingBegin);

    DirectoryID* pSubDirListingEnd = GetRealSubDirListingEnd(pSubDirListingBegin, pSubDirListingBegin + SubDirListingCapacity);

    const size_t subDirListingCount = distance(pSubDirListingBegin, pSubDirListingEnd);
    const size_t subDirCount = GetSubDirCount(pi_SubDirsTagID);
    const size_t newSubDirListingCount = subDirListingCount + subDirCount;

    // Special case for empty tag
    if (0 == subDirCount)
        {
        return (TagIsPresent(pi_SubDirsTagID)) ? RemoveTag(pi_SubDirsTagID) : true;
        }

    if (SubDirListingCapacity < newSubDirListingCount)
        {
        HArrayAutoPtr<DirectoryID> pNewDirectoryIds(new DirectoryID[newSubDirListingCount]);
        copy(SubDirIDsBegin(pi_SubDirsTagID),
             SubDirIDsEnd(pi_SubDirsTagID),
             copy(pSubDirListingBegin,
                  pSubDirListingEnd,
                  pNewDirectoryIds.get()));


        if (!SetRemovedSubDirectoryListingField(uint32_t(newSubDirListingCount), pNewDirectoryIds.get()))
            {
            HASSERT(!"Unable to initialize sub directory ids");
            return false;
            }
        }
    else
        {
        copy(SubDirIDsBegin(pi_SubDirsTagID), SubDirIDsEnd(pi_SubDirsTagID), pSubDirListingBegin);
        }

    return RemoveSubDirTagFromListing(pi_SubDirsTagID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::MakeSubDirectoryID ()
    {
    uint32_t     DirectoryQty = 0;
    DirectoryID* pDirectoryIds = 0;
    if (!GetRemovedSubDirectoryListingField(&DirectoryQty, &pDirectoryIds))
        return HTagFile::MakeDirectoryID(HTagFile::STANDARD, NumberOfDirectory());

    typedef std::reverse_iterator<DirectoryID*> RevIt;

    RevIt beginRemovedSubDirs = RevIt(pDirectoryIds + DirectoryQty);
    RevIt endRemovedSubDirs = RevIt(pDirectoryIds);

    RevIt foundSubDir = find_if (beginRemovedSubDirs, endRemovedSubDirs, bind2nd(not_equal_to<DirectoryID>(), DirectoryID(0)));

    if (endRemovedSubDirs == foundSubDir)
        {
        const bool TagRemovalSuccess = RemoveRemovedSubDirectoryListingTag();
        HASSERT(TagRemovalSuccess);
        return HTagFile::MakeDirectoryID(HTagFile::STANDARD, NumberOfDirectory());
        }

    const DirectoryID NewDirID = *foundSubDir;
    *foundSubDir = DirectoryID(0);

    return NewDirID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void TagFile::CreateSubDirectory (DirectoryID pi_DirectoryID)
    {
    // Append directory only if directoryID does not exist yet
    if (pi_DirectoryID >= NumberOfDirectory())
        {
        HASSERT(pi_DirectoryID == NumberOfDirectory());
        const bool AppendSuccess = AppendDirectory();
        HASSERT(AppendSuccess); // Append is assumed to be an always successful operation
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::GetSubDir    (DirectoryTagID pi_SubDirTagID) const
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(!IsVariableSizeTag(pi_SubDirTagID));

    DirectoryID DirectoryId = 0;
    if (!GetField(pi_SubDirTagID, &DirectoryId))
        return 0;

    return DirectoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::GetSubDir    (DirectoryTagID  pi_SubDirsTagID,
                                            size_t          pi_SubDirIndex) const
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(IsVariableSizeTag(pi_SubDirsTagID));

    uint32_t                   DirectoryQty = 0;
    DirectoryID*                pDirectoryIds = 0;
    GetField(pi_SubDirsTagID, &DirectoryQty, &pDirectoryIds);

    if (pi_SubDirIndex >= DirectoryQty)
        return 0;

    return pDirectoryIds[pi_SubDirIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::GetDirVersion(DirectoryID     pi_DirectoryID,
                            uint32_t&         po_rVersion) const
    {
    const HTagFile::DirectoryID curDir = CurrentDirectory();
    if (curDir == pi_DirectoryID)
        {
        po_rVersion = GetVersion();
        return true;
        }


    const bool success = const_cast<TagFile&>(*this).SetDirectory (pi_DirectoryID);
    if (success)
        po_rVersion = GetVersion();


    const bool restoreDirSuccess = const_cast<TagFile&>(*this).SetDirectory(curDir);
    HASSERT(restoreDirSuccess);

    return success && restoreDirSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::GetSubDirEx  (DirectoryTagID          pi_SubDirTagID,
                                            uint32_t&                  po_rVersion) const
    {
    const DirectoryID dirID = GetSubDir(pi_SubDirTagID);
    if (0 == dirID || !GetDirVersion(dirID, po_rVersion))
        return 0;

    return dirID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::GetSubDirEx  (DirectoryTagID          pi_SubDirsTagID,
                                            size_t                  pi_SubDirIndex,
                                            uint32_t&                  po_rVersion) const
    {
    const DirectoryID dirID = GetSubDir(pi_SubDirsTagID, pi_SubDirIndex);
    if (0 == dirID || !GetDirVersion(dirID, po_rVersion))
        return 0;

    return dirID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::CreateSubDir     (DirectoryTagID pi_SubDirTagID)
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(!IsReadOnly());
    HPRECONDITION(!IsVariableSizeTag(pi_SubDirTagID));

    DirectoryID DirectoryId = 0;
    if (!GetField(pi_SubDirTagID, &DirectoryId))
        {
        DirectoryId = MakeSubDirectoryID();

        if (!SetField(pi_SubDirTagID, DirectoryId) || !AddSubDirToListing(DirectoryId))
            {
            HASSERT(!"Unable to set sub directory id");
            return 0;
            }

        CreateSubDirectory(DirectoryId);
        }
    else
        {
        // Trying to recreate an existing directory!
        return 0;
        }

    return DirectoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::AddSubDir   (DirectoryTagID  pi_SubDirsTagID,
                                           size_t&         po_rSubDirIndex)
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(!IsReadOnly());
    HPRECONDITION(IsVariableSizeTag(pi_SubDirsTagID));

    uint32_t        DirectoryQty = 0;
    DirectoryID*    pDirectoryIds = 0;
    GetField(pi_SubDirsTagID, &DirectoryQty, &pDirectoryIds);

    //const bool EmptyTag = (0 == DirectoryQty);

    HArrayAutoPtr<DirectoryID>  pNewDirectoryIds;
    const uint32_t              NewDirectoryIdQty = DirectoryQty + 1;

    expandCopyBuffer(pDirectoryIds, DirectoryQty, pNewDirectoryIds, NewDirectoryIdQty, DirectoryID(0));

    DirectoryID& rDirectoryId = pNewDirectoryIds[DirectoryQty];

    rDirectoryId = MakeSubDirectoryID();

    if (!SetField(pi_SubDirsTagID, NewDirectoryIdQty, pNewDirectoryIds.get()))
        {
        HASSERT(!"Unable to initialize sub directory ids");
        return 0;
        }
    if (!AddSubDirToListing(rDirectoryId))
        {
        HASSERT(!"Unable to add sub dir tag");
        return 0;
        }

    CreateSubDirectory(rDirectoryId);

    po_rSubDirIndex = DirectoryQty;
    return rDirectoryId;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile::DirectoryID TagFile::CreateSubDir    (DirectoryTagID  pi_SubDirsTagID,
                                               size_t          pi_SubDirIndex)
    {
    HFCMonitor Monitor(m_Key);

    HPRECONDITION(!IsReadOnly());
    HPRECONDITION(IsVariableSizeTag(pi_SubDirsTagID));

    uint32_t                    DirectoryQty = 0;
    DirectoryID*                pDirectoryIds = 0;
    GetField(pi_SubDirsTagID, &DirectoryQty, &pDirectoryIds);

    //const bool EmptyTag = (0 == DirectoryQty);

    HArrayAutoPtr<DirectoryID>  pNewDirectoryIds;
    if (pi_SubDirIndex >= DirectoryQty)
        {
        const uint32_t NewDirectoryIdQty = static_cast<uint32_t>(pi_SubDirIndex + 1);
        expandCopyBuffer(pDirectoryIds, DirectoryQty, pNewDirectoryIds, NewDirectoryIdQty, DirectoryID(0));

        pDirectoryIds = pNewDirectoryIds.get();
        DirectoryQty = NewDirectoryIdQty;
        }
    else if (0 != pDirectoryIds[pi_SubDirIndex])
        {
        // Trying to recreate an existing directory!
        return 0;
        }

    DirectoryID& rDirectoryId = pDirectoryIds[pi_SubDirIndex];

    rDirectoryId = MakeSubDirectoryID();

    // Set attribute only if directory buffer expended (was set directly to memory if not)
    if (0 != pNewDirectoryIds && !SetField(pi_SubDirsTagID, DirectoryQty, pDirectoryIds))
        {
        HASSERT(!"Unable to initialize sub directory ids");
        return 0;
        }
    if (!AddSubDirToListing(rDirectoryId))
        {
        HASSERT(!"Unable to add sub dir tag");
        return 0;
        }

    CreateSubDirectory(rDirectoryId);

    return rDirectoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TagFile::DirRemover : public unary_function<DirectoryID, bool>
    {
    TagFile& m_rFile;
    explicit DirRemover (TagFile& pio_rFile) : m_rFile(pio_rFile) {}

    bool operator () (DirectoryID pi_directoryID) const
        {
        return m_rFile.RemoveDir(pi_directoryID);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TagFile::PacketRemover : public unary_function<PacketID, bool>
    {
    TagFile& m_rDir;
    explicit PacketRemover (TagFile& pio_rDir) : m_rDir(pio_rDir) {}

    bool operator () (PacketID pi_packetID) const
        {
        return H_SUCCESS == m_rDir.RemovePacket(pi_packetID);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveAllTags ()
    {
    typedef vector<HTagID> KeptTagsList;

    struct TagRemover
        {
        HTIFFDirectory& m_rDir;
        bool& m_rSuccess;

        explicit TagRemover (HTIFFDirectory& pi_rDir, bool& po_rSuccess) 
            : m_rDir(pi_rDir), m_rSuccess(po_rSuccess) {}

        void operator () (HTagID pi_id) const
            {
            m_rSuccess &= m_rDir.Remove(pi_id);
            }
    };

    // Ensure that this operation is never performed on root directory.
    HPRECONDITION(HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0) != CurrentDirectory());

    bool Success = true;

    // Remove all tags but the removed sub directory listing tag
    for_each(GetCurrentDir().TagIDBegin(), GetCurrentDir().TagIDEnd(), 
             TagRemover(GetCurrentDir(), Success));

    return Success;
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveDir (DirectoryID pi_DirectoryID)
    {
    // Ensure we points on the right directory
    const DirectoryID previousDirID = CurrentDirectory();
    if (pi_DirectoryID != previousDirID)
        SetDirectory(pi_DirectoryID);

    bool Success = true;

    // Remove all sub directories
    const HTagID subDirsTagID = GetSubDirectoryListingTagID ();
    const size_t subDirCount = GetSubDirCount(subDirsTagID);
    Success &= (count_if(SubDirIDsBegin(subDirsTagID), SubDirIDsEnd(subDirsTagID), DirRemover(*this)) == subDirCount);
    Success &= RemoveSubDirTag(subDirsTagID);

    // Remove/release all packets
    const size_t packetCount = distance(PacketIDsBegin(), PacketIDsEnd());
    Success &= (count_if(PacketIDsBegin(), PacketIDsEnd(), PacketRemover(*this)) == packetCount);

    // Remove cache
    m_DirectoriesInfo.Remove(GetCurrentDirIndex());


    // Remove actual directory
    Success &= RemoveAllTags();

    SetDirectory(previousDirID);
    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveSubDirs (DirectoryTagID pi_SubDirsTagID)
    {
    HFCMonitor Monitor(m_Key);

    // Remove all specified sub directories
    const size_t subDirCount = GetSubDirCount(pi_SubDirsTagID);
    bool Success = (count_if(SubDirIDsBegin(pi_SubDirsTagID), SubDirIDsEnd(pi_SubDirsTagID), DirRemover(*this)) == subDirCount);
    Success &= RemoveSubDirTag(pi_SubDirsTagID);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TagFile::RemoveSubDirs ()
    {
    HFCMonitor Monitor(m_Key);

    // Remove all sub directories
    const HTagID subDirsTagID = GetSubDirectoryListingTagID ();
    const size_t subDirCount = GetSubDirCount(subDirsTagID);
    bool Success = (count_if(SubDirIDsBegin(subDirsTagID), SubDirIDsEnd(subDirsTagID), DirRemover(*this)) == subDirCount);
    Success &= RemoveSubDirTag(subDirsTagID);

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TagFile::GetSubDirCount (DirectoryTagID pi_SubDirsTagID) const
    {
    return count_if(SubDirIDsBegin(pi_SubDirsTagID), SubDirIDsEnd(pi_SubDirsTagID), bind1st(not_equal_to<DirectoryID>(), 0));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter TagFile::SubDirIDsBegin (DirectoryTagID pi_SubDirsTagID) const
    {
    HPRECONDITION(IsVariableSizeTag(pi_SubDirsTagID));

    uint32_t        DirectoryQty = 0;
    DirectoryID*    pDirectoryIds = 0;
    GetField(pi_SubDirsTagID, &DirectoryQty, &pDirectoryIds);

    return SubDirIdIter(pDirectoryIds,
                        pDirectoryIds,
                        pDirectoryIds + DirectoryQty);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter TagFile::SubDirIDsEnd (DirectoryTagID pi_SubDirsTagID) const
    {
    HPRECONDITION(IsVariableSizeTag(pi_SubDirsTagID));

    uint32_t        DirectoryQty = 0;
    DirectoryID*    pDirectoryIds = 0;
    GetField(pi_SubDirsTagID, &DirectoryQty, &pDirectoryIds);

    return SubDirIdIter(pDirectoryIds + DirectoryQty,
                        pDirectoryIds,
                        pDirectoryIds + DirectoryQty);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter TagFile::PacketIDsBegin () const
    {
    // NTERAY: There is room to optimize here by working from the real begin

    return PacketIdIter(m_pOffset64, m_pOffset64, m_pOffset64 + GetPacketCountCapacity(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter TagFile::PacketIDsEnd () const
    {
    // NTERAY: There is room to optimize here by working from the real end
    const uint64_t* pEndOffset = m_pOffset64 + GetPacketCountCapacity();
    return PacketIdIter(pEndOffset, m_pOffset64, pEndOffset, true);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter TagFile::FindPacketIterFor (PacketID pi_PacketID) const
    {
    return (DoesPacketExist(pi_PacketID)) ? PacketIdIter(m_pOffset64 + pi_PacketID, m_pOffset64, m_pOffset64 + GetPacketCountCapacity(), true) :
           PacketIDsEnd();
    }


size_t TagFile::GetPacketCountCapacity () const
    {
    return m_NbData32;
    }


void TagFile::SetPacketCountCapacity (size_t pi_Capacity)
    {
    ReallocDataPacketIndex(pi_Capacity);
    }


bool TagFile::DoesPacketExist (PacketID pi_PacketID) const
    {
    return (pi_PacketID < GetPacketCountCapacity()) && (0 != GetOffset(pi_PacketID));
    }


//-----------------------------------------------------------------------------
// Remove an existing packet releasing the id / storage so that it could be
// used again.
//-----------------------------------------------------------------------------
HSTATUS TagFile::RemovePacket (PacketID pi_PacketID)
    {
    HFCMonitor Monitor(m_Key);

    const uint32_t OldByteCount = GetCount(pi_PacketID);

    if (!ReleasePacketID(pi_PacketID))
        {
        // TDORAY: Change for: "could not remove block"
        HTIFFError::BadBlockNbErInfo ErInfo;
        ErInfo.m_BlockNb = (int32_t)pi_PacketID;

        ErrorMsg(&m_pError, HTIFFError::BAD_BLOCK_NUMBER, &ErInfo, true);
        return H_NOT_FOUND;
        }

    DirInfo& dirInfo = EditCurrentDirInfo();

    --dirInfo.m_PacketCount;
    dirInfo.m_DataSize -= OldByteCount;

    if (IsDataCompressed())
        dirInfo.m_UncompressedDataSize -= GetUncompressedPacketSizeImpl(pi_PacketID);

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// Append a new packet (compression wise) returning an id. Will reuse
// released storage and ids when possible.
//-----------------------------------------------------------------------------
HSTATUS TagFile::AppendPacket  (const HCDPacket&    pi_rPacket,
                                PacketID&           po_rPacketID)
    {
    HFCMonitor Monitor(m_Key);
    PacketID ID = GetNextAvailablePacketID();

    const HSTATUS Ret = WritePacket(pi_rPacket, ID);

    if (H_SUCCESS == Ret)
        po_rPacketID = ID;

    return Ret;
    }


//-----------------------------------------------------------------------------
// Write a packet, compressing it
//-----------------------------------------------------------------------------
HSTATUS TagFile::WriteCompressedPacket (const HCDPacket&        pi_rPacket,
                                        PacketID                pi_PacketID)
    {
    DirInfo& rDirInfo = EditCurrentDirInfo();

    HPRECONDITION(0 != m_pCompressedPacket && 0 != rDirInfo.m_pCodec);
    HPRECONDITION(pi_rPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

    const size_t PreviousUncompressedPacketSize = GetUncompressedPacketSizeImpl(pi_PacketID);
    const uint32_t UncompressedPacketSize = static_cast<uint32_t>(pi_rPacket.GetDataSize());

    rDirInfo.m_pCodec->SetSubsetSize(UncompressedPacketSize);
    if ((rDirInfo.m_pCodec->GetSubsetMaxCompressedSize() + sizeof(UncompressedPacketSize)) > m_pCompressedPacket->GetBufferSize())
        {
        if (!m_pCompressedPacket->HasBufferOwnership())
            {
            HASSERT(!"Working buffer should have buffer ownership");
            return H_ERROR;
            }

        // Reallocate more than required to avoid further reallocations
        const size_t NewCompressedBufferSize
        = rDirInfo.m_pCodec->GetSubsetMaxCompressedSize() + rDirInfo.m_pCodec->GetSubsetMaxCompressedSize() / 3;
        m_pCompressedPacket->SetBuffer(new Byte[NewCompressedBufferSize], NewCompressedBufferSize);
        }

    const size_t CompressedDataSize
    = rDirInfo.m_pCodec->CompressSubset(pi_rPacket.GetBufferAddress(),
                                        pi_rPacket.GetDataSize(),
                                        m_pCompressedPacket->GetBufferAddress() + sizeof(UncompressedPacketSize),
                                        m_pCompressedPacket->GetBufferSize() - sizeof(UncompressedPacketSize));

    m_pCompressedPacket->SetDataSize(CompressedDataSize + sizeof(UncompressedPacketSize));

    // TODO: Consider endianess
    // Write the size of the uncompressed packet as a 32 bit field just before compressed data
    reinterpret_cast<uint32_t&>(*m_pCompressedPacket->GetBufferAddress()) = UncompressedPacketSize;

    HASSERT(reinterpret_cast<uint32_t&>(*m_pCompressedPacket->GetBufferAddress()) == UncompressedPacketSize);

    HASSERT(m_pCompressedPacket->GetDataSize() <= m_pCompressedPacket->GetBufferSize());
    HASSERT(0 == CompressedDataSize ? 0 == pi_rPacket.GetDataSize() : true);

    const HSTATUS Ret = WriteRawPacket(*m_pCompressedPacket, pi_PacketID);

    if (H_SUCCESS == Ret)
        {
        (rDirInfo.m_UncompressedDataSize += UncompressedPacketSize) -= PreviousUncompressedPacketSize;
        rDirInfo.m_MaxUncompressedPacketSize = std::max<uint64_t>(rDirInfo.m_MaxUncompressedPacketSize, UncompressedPacketSize);
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Read a packet of compressed data, uncompressing it
//-----------------------------------------------------------------------------
HSTATUS TagFile::ReadCompressedPacket  (HCDPacket&  po_rPacket,
                                        PacketID    pi_PacketID) const
    {
    const DirInfo& rDirInfo = GetCurrentDirInfo();

    HPRECONDITION(0 != m_pCompressedPacket && 0 != rDirInfo.m_pCodec);
    const HSTATUS Ret = ReadRawPacket(*m_pCompressedPacket, pi_PacketID);

    if (H_SUCCESS != Ret)
        return Ret;

    // TODO: Consider endianess
    // Read our uncompressed data size 32 bit field
    const uint32_t UncompressedPacketSize = reinterpret_cast<uint32_t&>(*m_pCompressedPacket->GetBufferAddress());

    if (UncompressedPacketSize > po_rPacket.GetBufferSize())
        {
        if (!po_rPacket.HasBufferOwnership())
            {
            HASSERT(!"Working buffer should have buffer ownership or have enough space");
            return H_ERROR;
            }
        po_rPacket.SetBuffer(new Byte[UncompressedPacketSize], UncompressedPacketSize);
        }


    po_rPacket.SetDataSize(rDirInfo.m_pCodec->DecompressSubset(m_pCompressedPacket->GetBufferAddress() + sizeof(UncompressedPacketSize),
                                                               m_pCompressedPacket->GetDataSize() - sizeof(UncompressedPacketSize),
                                                               po_rPacket.GetBufferAddress(),
                                                               po_rPacket.GetBufferSize()));



    //if (0 == po_rPacket.GetDataSize() && 0 != m_pCompressedPacket->GetDataSize())
    //return H_ERROR;

    HPOSTCONDITION(UncompressedPacketSize == po_rPacket.GetDataSize());
    HPOSTCONDITION(po_rPacket.GetDataSize() <= po_rPacket.GetBufferSize());

    return Ret;
    }

//-----------------------------------------------------------------------------
// Write a packet of raw data
//-----------------------------------------------------------------------------
HSTATUS TagFile::WriteRawPacket(const HCDPacket&    pi_rPacket,
                                PacketID            pi_PacketID)
    {
    HSTATUS Ret = H_SUCCESS;

    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    //TDORAY: Check if has write/create access


    ReallocDataPacketIndex(pi_PacketID + 1);


    const bool IsNewPacket = GetOffset(pi_PacketID) == 0;
    const uint32_t OldByteCount = GetCount(pi_PacketID);
    const size_t NewByteCount = pi_rPacket.GetDataSize();
    DirInfo* pDirInfo(0);
    
    if (0 != NewByteCount)
        {
        // New Packet ? or Packet data size changed.
        if (IsNewPacket || (OldByteCount != NewByteCount))
            {
            uint64_t Offset = GetOffset(pi_PacketID);
            m_pFile->CheckAlloc (&Offset, OldByteCount, static_cast<uint32_t>(NewByteCount));
            SetOffset(pi_PacketID, Offset);

            // Notify that offset and byte count changed and that corresponding tags need to be updated
            GetCurrentDir().Touched(GetPacketOffsetsTagID());
            GetCurrentDir().Touched(GetPacketByteCountsTagID());
            }

        // Set Tile Size and Write it
        SetCount(pi_PacketID, NewByteCount);
        if (!m_pFile->Seek (GetOffset(pi_PacketID), SEEK_SET) ||
            (m_pFile->Write (pi_rPacket.GetBufferAddress(), 1, NewByteCount) != NewByteCount))
            {
            ReleasePacketID(pi_PacketID);

            HTIFFError::BlockIOErInfo ErInfo;
            ErInfo.m_BlockNb = (int32_t)pi_PacketID;
            ErInfo.m_Offset = GetOffset(pi_PacketID);
            ErInfo.m_Length = GetCount(pi_PacketID);

            if (dynamic_cast<HFCFileOutOfRangeException const*>(m_pFile->GetFilePtr()->GetLastException()) != 0)
                {
                ErrorMsg(&m_pError, HTIFFError::SIZE_OUT_OF_RANGE, 0, true);
                Ret = H_OUT_OF_RANGE;
                }
            else
                {
                ErrorMsg(&m_pError, HTIFFError::CANNOT_WRITE_BLOCK, &ErInfo, true);
                Ret = H_WRITE_ERROR;
                }

            goto WRAPUP;
            }
        }
    else // Enable handling of empty packets
        {
        if (0 != OldByteCount)
            m_pFile->ReleaseBlock(GetOffset(pi_PacketID), OldByteCount);

        SetOffset(pi_PacketID, EMPTY_PACKET_OFFSET);
        SetCount(pi_PacketID, 0);

        // Notify that offset and byte count changed and that corresponding tags need to be updated
        GetCurrentDir().Touched(GetPacketOffsetsTagID());
        GetCurrentDir().Touched(GetPacketByteCountsTagID());
        }

    // TDORAY: See whether this is necessary... It sets all directories dirty so that they are all rewritten??
    //         do not uses for the moment.
    //m_DataWasModified = true;

    pDirInfo = &EditCurrentDirInfo();

    if (IsNewPacket)
        ++pDirInfo->m_PacketCount;

    (pDirInfo->m_DataSize += NewByteCount) -= OldByteCount;
    pDirInfo->m_MaxPacketSize = std::max<uint64_t>(pDirInfo->m_MaxPacketSize, NewByteCount);

WRAPUP:

    // TDORAY: Check if possible to release file block on error

    CacheFileLock.ReleaseKey();
    return Ret;
    }

//-----------------------------------------------------------------------------
// Read a packet of raw data
//-----------------------------------------------------------------------------
HSTATUS TagFile::ReadRawPacket     (HCDPacket&  po_rPacket,
                                    PacketID    pi_PacketID) const
    {
    HPRECONDITION(pi_PacketID <= m_NbData32);

    HSTATUS Ret = H_SUCCESS;

    HFCLockMonitor CacheFileLock (m_pLockManager.get());

    // TDORAY: Check if has read access

    uint32_t DataSize(0);
    if (pi_PacketID >= m_NbData32)
        {
        HTIFFError::BadBlockNbErInfo ErInfo;
        ErInfo.m_BlockNb = (int32_t)pi_PacketID;
        ErrorMsg(&const_cast<TagFile&>(*this).m_pError, HTIFFError::BAD_BLOCK_NUMBER, &ErInfo, true);
        Ret = H_ERROR;
        goto WRAPUP;
        }

    if (0 == GetOffset(pi_PacketID))
        {
        // Data not allocated, return an error
        Ret = H_DATA_NOT_AVAILABLE;
        goto WRAPUP;
        }

    DataSize = GetCount(pi_PacketID);

    // Source data is uncompressed

    // Verify if the packet is large enough to hold the uncompressed data
    if (po_rPacket.GetBufferSize() < DataSize)
        {
        if (po_rPacket.HasBufferOwnership())
            po_rPacket.SetBuffer(new Byte[DataSize], DataSize);
        else
            {
            Ret = H_READ_ERROR;
            goto WRAPUP;
            }
        }

    // Set an Identity codec
    // TDORAY: This probably hampers our performances. Will we really use packet's codec?
    //po_rPacket.SetCodec(new HCDCodecIdentity());


    // Enable reading empty packets
    if (0 != DataSize)
        {
        if ((!m_pFile->Seek (GetOffset(pi_PacketID), SEEK_SET) ||
             (m_pFile->Read (po_rPacket.GetBufferAddress(), 1, DataSize) != DataSize)))
            {
            HTIFFError::BlockIOErInfo ErInfo;
            ErInfo.m_BlockNb = (int32_t)pi_PacketID;
            ErInfo.m_Offset = GetOffset(pi_PacketID);
            ErInfo.m_Length = DataSize;

            ErrorMsg(&const_cast<TagFile&>(*this).m_pError, HTIFFError::CANNOT_READ_BLOCK, &ErInfo, true);
            Ret = H_READ_ERROR;
            goto WRAPUP;
            }
        }

    // Set packet data size
    po_rPacket.SetDataSize(DataSize);

WRAPUP:
    CacheFileLock.ReleaseKey();
    return Ret;
    }

bool TagFile::NeedReallocDataPacketIndex (size_t pi_PacketQty) const
    {
    if (m_NbData32 < pi_PacketQty)
        return true;

    return false;
    }

void TagFile::ReallocDataPacketIndex (size_t pi_PacketQty)
    {
    HPRECONDITION(IsTiff64());

    if (!NeedReallocDataPacketIndex(pi_PacketQty))
        return;

    static const size_t MIN_CAPACITY = 1024;

    // TDORAY: when NbData32 is size_t work with size_t instead
    size_t NewBufferSize = (std::max)(MIN_CAPACITY, pi_PacketQty);
    NewBufferSize += (NewBufferSize >> 1);

    // Realloc offsets buffer
    uint32_t OldOffsetsBufferSize = m_NbData32;
    uint64_t* pOldOffsetsData = 0;
    GetCurrentDir().GetValues (GetPacketOffsetsTagID(), &OldOffsetsBufferSize, &pOldOffsetsData);
    HASSERT (OldOffsetsBufferSize == m_NbData32);

    HArrayAutoPtr<uint64_t> pNewOffsetsData;
    expandCopyBuffer(pOldOffsetsData, OldOffsetsBufferSize, pNewOffsetsData, NewBufferSize);


    // Realloc byte counts buffer
    uint32_t OldByteCountsBufferSize = m_NbData32;
    uint64_t* pOldByteCountsData = 0;
    GetCurrentDir().GetValues (GetPacketByteCountsTagID(), &OldByteCountsBufferSize, &pOldByteCountsData);
    HASSERT (OldByteCountsBufferSize == m_NbData32);

    HArrayAutoPtr<uint64_t> pNewByteCountsData;
    expandCopyBuffer(pOldByteCountsData, OldByteCountsBufferSize, pNewByteCountsData, NewBufferSize);

    // Save as a tag
    bool SuccessWritingOffsetTag = GetCurrentDir().SetValues (GetPacketOffsetsTagID(), uint32_t(NewBufferSize), pNewOffsetsData);
    bool SuccessWritingByteCountTag = GetCurrentDir().SetValues (GetPacketByteCountsTagID(), uint32_t(NewBufferSize), pNewByteCountsData);
    HASSERT (SuccessWritingOffsetTag && SuccessWritingByteCountTag);

    m_NbData32 = uint32_t(NewBufferSize);
    ReadOffsetCountTags();
    }


void TagFile::ResetPacketIndexes ()
    {
    m_NbData32 = 0;
    m_pOffset32 = 0;
    m_pOffset64 = 0;
    m_pCount32 = 0;
    m_pCount64 = 0;
    }

uint32_t TagFile::_NumberOfDirectory (HTagFile::DirectoryType pi_DirType) const
    {
    HFCMonitor Monitor(m_Key);

    if (pi_DirType == STANDARD)
        return m_ListDirCount;
    else if (pi_DirType == HMR)
        return m_ListHMRDirCount;
    else
        {
        HASSERT(0);
        return 0;
        }
    }

bool TagFile::IsValidTopDirectory (HTIFFError** po_ppError)
    {
    return true;
    }

void TagFile::OnTopDirectoryFirstInitialized ()
    {
    // Nothing
    }

bool TagFile::DirectoryIsValid(HTIFFDirectory*         pi_Dir,
                                HTIFFDirectory*         pi_pCurPageDir)
    {
    return true;
    }

// TDORAY: Find a way to remove it from here. Not generic.
bool TagFile::IsValidReducedImage (HTIFFDirectory*         pi_ReducedImageDir,
                                    HTIFFDirectory*         pi_pCurPageDir)
    {
    return true;
    }

void TagFile::_PrintCurrentDirectory (FILE* po_pOutput, uint32_t pi_Flag)
    {
    // Nothing
    }


void TagFile::_PostReallocDirectories (uint32_t pi_NewDirCountCapacity)
    {
    m_DirectoriesInfo.Resize(pi_NewDirCountCapacity);
    }

void TagFile::_PreWriteCurrentDir (HTagFile::DirectoryID pi_DirID)
    {
    const bool Success = SaveCurrentDirState(pi_DirID);
    HPOSTCONDITION(Success);
    }

bool TagFile::PreCurrentDirectoryChanged (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    bool Success = true;
    return Success;
    }

// Invoked only when current directory is changed successfully
bool TagFile::OnCurrentDirectoryChanged (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    if (HTagFile::STANDARD != GetDirectoryType(pi_DirID))
        return false; // HMR directories not supported

    return RefreshCurrentDirState(pi_DirID);
    }

// Always invoked after current directory is set
bool TagFile::PostCurrentDirectorySet (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec)
    {
    return true;
    }


bool TagFile::RefreshCurrentDirState (HTagFile::DirectoryID pi_DirID)
    {
    bool Success = true;

    // Free the list allocated internally, else do nothing.
    FreeOffsetCount();

    // Reinitialize offsets and byte counts buffers.
    ResetPacketIndexes();
    m_NextPacketID = 0;

    // Load or Create Offset\Size for Data
    if (GetCurrentDir().TagIsPresent(GetPacketOffsetsTagID()))
        ReadOffsetCountTags();

    Success &= LoadCurrentDirInfo();

    return Success;
    }


bool TagFile::SaveCurrentDirState (HTagFile::DirectoryID pi_DirID)
    {
    const bool Success = SaveCurrentDirInfo();
    return Success;
    }


bool TagFile::LoadCurrentDirInfo ()
    {
    return m_DirectoriesInfo.Load(*this);
    }

bool TagFile::SaveCurrentDirInfo ()
    {
    return m_DirectoriesInfo.Save(*this);
    }


} //End namespace HTGFF