//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFTagFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTGFFFile
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HCDCodec.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDPacket.h>

#include <ImagePP/all/h/HTagFile.h>
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
    class TagInfo : public BentleyApi::ImagePP::HTagInfo
        {
    public:
        virtual BentleyApi::ImagePP::HTagID      GetPacketOffsetsTagID          () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetPacketByteCountsTagID       () const = 0;

        virtual BentleyApi::ImagePP::HTagID      GetVersionTagID                () const = 0;

        virtual BentleyApi::ImagePP::HTagID      GetDataDimensionsTypeTagID     () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetDataDimensionsRoleTagID     () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetDataCompressTypeTagID       () const = 0;


        virtual BentleyApi::ImagePP::HTagID      GetPacketCountTagID            () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetDataSizeTagID               () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetUncompressedDataSizeTagID   () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetMaxPacketSizeTagID          () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetMaxUncompressedPacketSizeTagID () const = 0;

        virtual BentleyApi::ImagePP::HTagID      GetSubDirectoryListingTagID    () const = 0;
        virtual BentleyApi::ImagePP::HTagID      GetRemovedDirectoryListingTagID () const = 0;
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
class TagFile : public BentleyApi::ImagePP::HTagFile
    {
public:
    typedef uint32_t           PacketID;
    typedef uint32_t            DirectoryTagID;
    typedef HTGFF::FileDefinition
    Definition;

    explicit                    TagFile                        (const WString&          pi_rInputFilePath,
                                                                const BentleyApi::ImagePP::HFCAccessMode&    pi_rAccessMode,
                                                                const Definition&       pi_rFileDefinition);

    virtual                     ~TagFile                       ();

    bool                        IsReadOnly                     () const;

    const BentleyApi::ImagePP::HFCAccessMode&        GetAccessMode                  () const;

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

    BentleyApi::ImagePP::HSTATUS                     RemovePacket                   (PacketID                pi_PacketID);

    BentleyApi::ImagePP::HSTATUS                     AppendPacket                   (const BentleyApi::ImagePP::HCDPacket&        pi_rPacket,
                                                                PacketID&               po_rPacketID);

    BentleyApi::ImagePP::HSTATUS                     WritePacket                    (const BentleyApi::ImagePP::HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    BentleyApi::ImagePP::HSTATUS                     ReadPacket                     (BentleyApi::ImagePP::HCDPacket&              po_rPacket,
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

        static bool             UpdateDataType                 (const BentleyApi::ImagePP::HTIFFDirectory&   dir,
                                                                const TagFile&          file,
                                                                DirInfo&                dirInfo);
        static bool             UpdateDataCompressType         (const BentleyApi::ImagePP::HTIFFDirectory&   dir,
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

    BentleyApi::ImagePP::HSTATUS                     WriteRawPacket                 (const BentleyApi::ImagePP::HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    BentleyApi::ImagePP::HSTATUS                     ReadRawPacket                  (BentleyApi::ImagePP::HCDPacket&              po_rPacket,
                                                                PacketID                pi_PacketID) const;

    BentleyApi::ImagePP::HSTATUS                     WriteCompressedPacket          (const BentleyApi::ImagePP::HCDPacket&        pi_rPacket,
                                                                PacketID                pi_PacketID);

    BentleyApi::ImagePP::HSTATUS                     ReadCompressedPacket           (BentleyApi::ImagePP::HCDPacket&              po_rPacket,
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

    virtual BentleyApi::ImagePP::HTagID              GetPacketOffsetsTagID          () const override;
    virtual BentleyApi::ImagePP::HTagID              GetPacketByteCountsTagID       () const override;

    BentleyApi::ImagePP::HTagID                      GetVersionTagID                () const;

    BentleyApi::ImagePP::HTagID                      GetDataDimensionsTypeTagID     () const;
    BentleyApi::ImagePP::HTagID                      GetDataDimensionsRoleTagID     () const;
    BentleyApi::ImagePP::HTagID                      GetDataCompressTypeTagID       () const;

    BentleyApi::ImagePP::HTagID                      GetPacketCountTagID            () const;
    BentleyApi::ImagePP::HTagID                      GetDataSizeTagID               () const;
    BentleyApi::ImagePP::HTagID                      GetUncompressedDataSizeTagID   () const;
    BentleyApi::ImagePP::HTagID                      GetMaxPacketSizeTagID          () const;
    BentleyApi::ImagePP::HTagID                      GetMaxUncompressedPacketSizeTagID
    () const;

    BentleyApi::ImagePP::HTagID                      GetSubDirectoryListingTagID    () const;
    BentleyApi::ImagePP::HTagID                      GetRemovedSubDirectoryListingTagID () const;

    bool                        GetRemovedSubDirectoryListingField
                                                               (uint32_t*                po_pCount,
                                                                DirectoryID**           po_ppVal);

    bool                        SetRemovedSubDirectoryListingField
                                                               (uint32_t               pi_Count, 
                                                                const uint32_t*          pi_pVal);

    bool                        RemoveRemovedSubDirectoryListingTag
                                                               ();

    bool                        AddSubDirToListing             (DirectoryID             pi_DirectoryID);
    bool                        RemoveSubDirTagFromListing     (BentleyApi::ImagePP::HTagID                  pi_SubDirsTagID);

    bool                        RemoveSubDirTag                (BentleyApi::ImagePP::HTagID                  pi_SubDirsTagID);

    DirectoryID                 MakeSubDirectoryID             ();
    void                        CreateSubDirectory             (DirectoryID             pi_DirectoryID);


    bool                        GetDirVersion                  (DirectoryID             pi_DirectoryID,
                                                                uint32_t&                 po_rVersion) const;

    bool                        RemoveAllTags                  ();

    bool                        RemoveDir                      (DirectoryID             pi_DirectoryID);

    DirInfo&                    EditCurrentDirInfo             ();
    const DirInfo&              GetCurrentDirInfo              () const;

    virtual uint32_t           _NumberOfDirectory             (BentleyApi::ImagePP::HTagFile::DirectoryType pi_DirType) const override;


    virtual bool                IsValidTopDirectory            (BentleyApi::ImagePP::HTIFFError**            po_ppError) override;

    virtual void                OnTopDirectoryFirstInitialized () override;

    virtual bool                DirectoryIsValid               (BentleyApi::ImagePP::HTIFFDirectory*         pi_Dir,
                                                                BentleyApi::ImagePP::HTIFFDirectory*         pi_pCurPageDir) override;

    // TDORAY: Find a way to remove it from here. Not generic.
    virtual bool                IsValidReducedImage            (BentleyApi::ImagePP::HTIFFDirectory*         pi_ReducedImageDir,
                                                                BentleyApi::ImagePP::HTIFFDirectory*         pi_pCurPageDir) override;

    virtual void                _PrintCurrentDirectory         (FILE* po_pOutput, uint32_t pi_Flag) override;


    virtual void                _PostReallocDirectories        (uint32_t                pi_NewDirCountCapacity) override;

    virtual void                _PreWriteCurrentDir            (BentleyApi::ImagePP::HTagFile::DirectoryID pi_DirID) override;

    virtual bool                PreCurrentDirectoryChanged     (BentleyApi::ImagePP::HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;

    // Invoked only when current directory is changed successfully
    virtual bool                OnCurrentDirectoryChanged      (BentleyApi::ImagePP::HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;

    // Always invoked after current directory is set
    virtual bool                PostCurrentDirectorySet        (BentleyApi::ImagePP::HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;


    bool                        RefreshCurrentDirState         (BentleyApi::ImagePP::HTagFile::DirectoryID   pi_DirID);
    bool                        SaveCurrentDirState            (BentleyApi::ImagePP::HTagFile::DirectoryID   pi_DirID);

    bool                        LoadCurrentDirInfo             ();
    bool                        SaveCurrentDirInfo             ();

    const Definition&           m_rFileDefinition;

    //-----------------------------------------------------------------------------
    // File state
    //-----------------------------------------------------------------------------
    BentleyApi::ImagePP::HFCAccessMode               m_AccessMode;

    //-----------------------------------------------------------------------------
    // Current directory cached states
    //-----------------------------------------------------------------------------
    PacketID                    m_NextPacketID;
    DirInfoList                 m_DirectoriesInfo;
    BentleyApi::ImagePP::HFCPtr<BentleyApi::ImagePP::HCDPacket>           m_pCompressedPacket;
    };



template <typename T>
void                            expandCopyBuffer               (const T*            pi_pOldBuffer,
                                                                size_t              pi_OldSize,
                                                                BentleyApi::ImagePP::HArrayAutoPtr<T>&   pi_rpNewBuffer,
                                                                size_t              pi_NewSize,
                                                                const T&            pi_rInitValue = T());

} //End namespace HTGFF


#include "HTGFFTagFile.hpp"