//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTGFFTagFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTGFFFile
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HCDCodec.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDPacket.h>

#include "HTagFile.h"
#include "HTGFFDataDescriptor.h"
#include "HTGFFCompression.h"


namespace HTGFF {

class PacketIdIter;
class SubDirIdIter;

//-----------------------------------------------------------------------------
// Pure virtual class specifying what is required by the generic file to work.
// Users of the HTGFFFile will have to define these parameters.
//-----------------------------------------------------------------------------
class FileDefinition
    {
public:
    class TagInfo : public HTagInfo
        {
    public:
        virtual HTagID      GetPacketOffsetsTagID          () const = 0;
        virtual HTagID      GetPacketByteCountsTagID       () const = 0;

        virtual HTagID      GetVersionTagID                () const = 0;

        virtual HTagID      GetDataDimensionsTypeTagID     () const = 0;
        virtual HTagID      GetDataDimensionsRoleTagID     () const = 0;
        virtual HTagID      GetDataCompressTypeTagID       () const = 0;


        virtual HTagID      GetPacketCountTagID            () const = 0;
        virtual HTagID      GetDataSizeTagID               () const = 0;
        virtual HTagID      GetUncompressedDataSizeTagID   () const = 0;
        virtual HTagID      GetMaxPacketSizeTagID          () const = 0;
        virtual HTagID      GetMaxUncompressedPacketSizeTagID
        () const = 0;

        virtual HTagID      GetSubDirectoryListingTagID    () const = 0;
        virtual HTagID      GetRemovedDirectoryListingTagID
        () const = 0;
        };

    virtual unsigned short  GetLittleEndianMagicNumber     () const = 0;
    virtual unsigned short  GetBigEndianMagicNumber        () const = 0;



    virtual const TagInfo&  GetTagInfo                     () const = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Tagged generic file format. File format using tags to store attributes
*               and that has a directory organization. This format is meant to be
*               generic and not specialized for a specific domain. Can also
*               read/write/append compressed/uncompressed data in the form of packets.
*
* @see          HTagFile
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class TagFile : public HTagFile
    {
public:
    typedef uint32_t           PacketID;
    typedef uint32_t            DirectoryTagID;
    typedef HTGFF::FileDefinition
    Definition;

    explicit                    TagFile                        (const WString&          pi_rInputFilePath,
                                                                const HFCAccessMode&    pi_rAccessMode,
                                                                const Definition&       pi_rFileDefinition);

    virtual                     ~TagFile                       ();

    bool                        IsReadOnly                     () const;

    const HFCAccessMode&        GetAccessMode                  () const;

    const DataType&             GetDataType                    () const;

    const Compression&          GetDataCompressType            () const;
    bool                        IsDataCompressed               () const;
    double                      GetCompressionRatio            () const;

    uint32_t                    GetVersion                     () const;
    bool                        SetVersion                     (uint32_t                 pi_version);

    bool                        SetDataType                    (const DataType&         pi_DataType);
    bool                        SetDataCompressType            (const Compression&      pi_DataCompress);

    size_t                      GetPacketCount                 () const;

    uint64_t                    GetTotalCompressedPacketSize   () const;
    uint64_t                    GetTotalUncompresedPacketSize  () const;

    size_t                      GetMaxCompressedPacketSize     () const;
    size_t                      GetMaxUncompressedPacketSize   () const;

    size_t                      GetCompressedPacketSize        (PacketID                pi_PacketID) const;
    size_t                      GetUncompressedPacketSize      (PacketID                pi_PacketID) const;


    DirectoryID                 GetSubDir                      (DirectoryTagID          pi_SubDirTagID) const;
    DirectoryID                 GetSubDir                      (DirectoryTagID          pi_SubDirsTagID,
                                                                size_t                  pi_SubDirIndex) const;

    DirectoryID                 GetSubDirEx                    (DirectoryTagID          pi_SubDirTagID,
                                                                uint32_t&                 po_rVersion) const;
    DirectoryID                 GetSubDirEx                    (DirectoryTagID          pi_SubDirsTagID,
                                                                size_t                  pi_SubDirIndex,
                                                                uint32_t&                 po_rVersion) const;

    DirectoryID                 CreateSubDir                   (DirectoryTagID          pi_SubDirTagID);

    DirectoryID                 AddSubDir                      (DirectoryTagID          pi_SubDirsTagID,
                                                                size_t&                 po_rSubDirIndex);

    DirectoryID                 CreateSubDir                   (DirectoryTagID          pi_SubDirsTagID,
                                                                size_t                  pi_SubDirIndex);

    bool                        RemoveSubDirs                  (DirectoryTagID          pi_SubDirsTagID);
    bool                        RemoveSubDirs                  ();

    size_t                      GetSubDirCount                 (DirectoryTagID          pi_SubDirsTagID) const;

    SubDirIdIter                SubDirIDsBegin                 (DirectoryTagID          pi_SubDirsTagID) const;
    SubDirIdIter                SubDirIDsEnd                   (DirectoryTagID          pi_SubDirsTagID) const;

    
    PacketIdIter                PacketIDsBegin                 () const;
    PacketIdIter                PacketIDsEnd                   () const;

    PacketIdIter                FindPacketIterFor              (PacketID                pi_PacketID) const;

    bool                        DoesPacketExist                (PacketID                pi_PacketID) const;

    HSTATUS                     RemovePacket                   (PacketID                pi_PacketID);

    HSTATUS                     AppendPacket                   (const HCDPacket&        pi_rPacket,
                                                                PacketID&               po_rPacketID);

    HSTATUS                     WritePacket                    (const HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    HSTATUS                     ReadPacket                     (HCDPacket&              po_rPacket,
                                                                PacketID                pi_PacketID) const;

private:


    struct                      DirInfo;
    struct                      DirInfoList
        {
    private:
        // NTERAY: Use C++0x unique_ptr when on VC2010???. If it is the case, check destructor first.
        typedef std::vector<DirInfo*>
                                List;
        List                    m_array;
        // Disable copy
                                DirInfoList                    (const DirInfoList&);
        DirInfoList&            operator=                      (const DirInfoList&);

        static bool             UpdateDataType                 (const HTIFFDirectory&   dir,
                                                                const TagFile&          file,
                                                                DirInfo&                dirInfo);
        static bool             UpdateDataCompressType         (const HTIFFDirectory&   dir,
                                                                const TagFile&          file,
                                                                DirInfo&                dirInfo);

    public:
        explicit                DirInfoList                    (uint32_t                directoryCount);
                                ~DirInfoList                   ();

        DirInfo&                Edit                           (uint32_t                idx);
        const DirInfo&          Get                            (uint32_t                idx) const;

        void                    Remove                         (uint32_t                idx);
        void                    Resize                         (uint32_t                newSize);


        bool                    Save                           (TagFile&                file);

        bool                    Load                           (const TagFile&          file);

        bool                    UpdateDataType                 (const TagFile&          file);
        bool                    UpdateDataCompressType         (const TagFile&          file);
        };


    struct                      DirRemover;
    struct                      PacketRemover;

    HSTATUS                     WriteRawPacket                 (const HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    HSTATUS                     ReadRawPacket                  (HCDPacket&              po_rPacket,
                                                                PacketID                pi_PacketID) const;

    HSTATUS                     WriteCompressedPacket          (const HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    HSTATUS                     ReadCompressedPacket           (HCDPacket&              po_rPacket,
                                                                PacketID                pi_PacketID) const;

    PacketID                    GetNextAvailablePacketID       ();

    bool                        ReleasePacketID                (PacketID                pi_PacketID);

    size_t                      GetPacketCountCapacity         () const;
    void                        SetPacketCountCapacity         (size_t                  pi_Capacity);

    bool                        NeedReallocDataPacketIndex     (size_t                  pi_PacketQty) const;
    void                        ReallocDataPacketIndex         (size_t                  pi_PacketQty);

    size_t                      GetUncompressedPacketSizeImpl  (PacketID                pi_PacketID) const;


    void                        ResetPacketIndexes             ();

    virtual MagicNumber         GetLittleEndianMagicNumber     () const override;
    virtual MagicNumber         GetBigEndianMagicNumber        () const override;

    virtual HTagID              GetPacketOffsetsTagID          () const override;
    virtual HTagID              GetPacketByteCountsTagID       () const override;

    HTagID                      GetVersionTagID                () const;

    HTagID                      GetDataDimensionsTypeTagID     () const;
    HTagID                      GetDataDimensionsRoleTagID     () const;
    HTagID                      GetDataCompressTypeTagID       () const;

    HTagID                      GetPacketCountTagID            () const;
    HTagID                      GetDataSizeTagID               () const;
    HTagID                      GetUncompressedDataSizeTagID   () const;
    HTagID                      GetMaxPacketSizeTagID          () const;
    HTagID                      GetMaxUncompressedPacketSizeTagID
    () const;

    HTagID                      GetSubDirectoryListingTagID    () const;
    HTagID                      GetRemovedSubDirectoryListingTagID () const;

    bool                        GetRemovedSubDirectoryListingField
                                                               (uint32_t*                po_pCount,
                                                                DirectoryID**           po_ppVal);

    bool                        SetRemovedSubDirectoryListingField
                                                               (uint32_t               pi_Count, 
                                                                const uint32_t*          pi_pVal);

    bool                        RemoveRemovedSubDirectoryListingTag
                                                               ();

    bool                        AddSubDirToListing             (DirectoryID             pi_DirectoryID);
    bool                        RemoveSubDirTagFromListing     (HTagID                  pi_SubDirsTagID);

    bool                        RemoveSubDirTag                (HTagID                  pi_SubDirsTagID);

    DirectoryID                 MakeSubDirectoryID             ();
    void                        CreateSubDirectory             (DirectoryID             pi_DirectoryID);


    bool                        GetDirVersion                  (DirectoryID             pi_DirectoryID,
                                                                uint32_t&                 po_rVersion) const;

    bool                        RemoveAllTags                  ();

    bool                        RemoveDir                      (DirectoryID             pi_DirectoryID);

    DirInfo&                    EditCurrentDirInfo             ();
    const DirInfo&              GetCurrentDirInfo              () const;

    virtual uint32_t           _NumberOfDirectory             (HTagFile::DirectoryType pi_DirType) const override;


    virtual bool                IsValidTopDirectory            (HTIFFError**            po_ppError) override;

    virtual void                OnTopDirectoryFirstInitialized () override;

    virtual bool                DirectoryIsValid               (HTIFFDirectory*         pi_Dir,
                                                                HTIFFDirectory*         pi_pCurPageDir) override;

    // TDORAY: Find a way to remove it from here. Not generic.
    virtual bool                IsValidReducedImage            (HTIFFDirectory*         pi_ReducedImageDir,
                                                                HTIFFDirectory*         pi_pCurPageDir) override;

    virtual void                _PrintCurrentDirectory         (FILE* po_pOutput, uint32_t pi_Flag) override;


    virtual void                _PostReallocDirectories        (uint32_t                pi_NewDirCountCapacity) override;

    virtual void                _PreWriteCurrentDir            (HTagFile::DirectoryID pi_DirID) override;

    virtual bool                PreCurrentDirectoryChanged     (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;

    // Invoked only when current directory is changed successfully
    virtual bool                OnCurrentDirectoryChanged      (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;

    // Always invoked after current directory is set
    virtual bool                PostCurrentDirectorySet        (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;


    bool                        RefreshCurrentDirState         (HTagFile::DirectoryID   pi_DirID);
    bool                        SaveCurrentDirState            (HTagFile::DirectoryID   pi_DirID);

    bool                        LoadCurrentDirInfo             ();
    bool                        SaveCurrentDirInfo             ();

    const Definition&           m_rFileDefinition;

    //-----------------------------------------------------------------------------
    // File state
    //-----------------------------------------------------------------------------
    HFCAccessMode               m_AccessMode;

    //-----------------------------------------------------------------------------
    // Current directory cached states
    //-----------------------------------------------------------------------------
    PacketID                    m_NextPacketID;
    DirInfoList                 m_DirectoriesInfo;
    HFCPtr<HCDPacket>           m_pCompressedPacket;
    };

} //End namespace HTGFF


BEGIN_IMAGEPP_NAMESPACE

template <typename T>
void                            expandCopyBuffer               (const T*            pi_pOldBuffer,
                                                                size_t              pi_OldSize,
                                                                HArrayAutoPtr<T>&   pi_rpNewBuffer,
                                                                size_t              pi_NewSize,
                                                                const T&            pi_rInitValue = T());

END_IMAGEPP_NAMESPACE

#include "HTGFFTagFile.hpp"