//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTagDefinition.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFTagDefinition
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HTIFFError;
class HTagFileDef;

class HTagID
    {
public:
    HTagID                     () : m_ID(0) {}
    HTagID                     (uint32_t    pi_ID) : m_ID(pi_ID) {}
    HTagID&                         operator=                  (uint32_t    pi_ID) {
        m_ID = pi_ID;
        return *this;
        }

    operator uint32_t          () const {
        return m_ID;
        }

private:
    uint32_t                        m_ID;
    };


class HTagInfo
    {
public:
    // Tag data type information.
    //
    typedef    enum {
        _NOTYPE    = 0,        // ???
        BYTE    = 1,        // 8-bit unsigned integer
        ASCII    = 2,        // 8-bit bytes w/ last byte null
        SHORT    = 3,        // 16-bit unsigned integer
        LONG    = 4,        // 32-bit unsigned integer
        RATIONAL    = 5,    // 64-bit unsigned fraction
        SBYTE    = 6,        // 8-bit signed integer
        UNDEFINED    = 7,    // 8-bit untyped data
        SSHORT    = 8,        // 16-bit signed integer
        SLONG    = 9,        // 32-bit signed integer
        SRATIONAL    = 10,    // 64-bit signed fraction
        FLOAT    = 11,        // 32-bit IEEE floating point
        DOUBLE    = 12,        // 64-bit IEEE floating point
        ASCIIW  = 13,       // Wide char (16-bits short)
        Datatype_unknow1 = 14,
        Datatype_unknow2 = 15,
        LONG64  = 16,       // 64-bit unsigned integer
        SLONG64 = 17,       // 64-bit signed integer
        IFD64   = 18        // 64 bit unsigned byte IFD offset
        } DataType;

    typedef    struct {
        uint32_t        FileTag;        // TAG File number
        HTagID          TagID;          // Sequential Number associate to FileTag
        // Use internally only.
        short ReadCount;      // read count see TAG_IO_...
        short WriteCount;     // write count see TAG_IO... unknown)
        DataType        Type;           // Data type
        unsigned char          ReadWriteTag;   // false, ReadOnly Tag
        unsigned char          PassDirCount;   // if true, pass dir count on set
        char const*          pTagName;       // ASCII name
        } Info;


    // Read/Write field definition
    static const short TAG_IO_ANY;         // field descriptor
    IMAGEPP_EXPORT static const short TAG_IO_VARIABLE;    // Variable, use lenght
    static const short TAG_IO_USE_SPP;     // use SamplePerPixel

    static size_t                   sGetDataLen                        (DataType                pi_Type);
    static const char*             sConvertDataTypetoText             (DataType                pi_DataType);

    bool                           IsVariableSizeTag                  (HTagID                  pi_TagID) const;

    virtual bool                   IsBigTiffTag                       (uint32_t                pi_FileTagNumber) const
        {
        return false;
        }

    virtual HTagID                  GetFreeOffsetsTagID                () const = 0;
    virtual HTagID                  GetFreeByteCountsTagID             () const = 0;

    virtual HTagID                  GetSubFileTypeTagID                () const = 0;
    virtual HTagID                  GetHMRSyncronizationTagID          () const = 0;
    virtual HTagID                  GetHMRDirectoryV1TagID             () const = 0;
    virtual HTagID                  GetHMRDirectoryV2TagID             () const = 0;
    virtual HTagID                  GetHMRDecimationMethodTagID        () const = 0;

    virtual HTagID                  GetGeoKeyDirectoryTagID            () const = 0;
    virtual HTagID                  GetGeoDoubleParamsTagID            () const = 0;
    virtual HTagID                  GetGeoAsciiParamsTagID             () const = 0;

    virtual HTagID                  GetNotSavedTagIDBegin              () const = 0;
    virtual uint32_t                GetTagQty                          () const = 0;

    virtual size_t                  GetTagDefinitionQty                () const = 0;
    virtual const Info*             GetTagDefinitionArray              () const = 0;

private:
    static const int                sDataLen[];        // Len in bytes of each DataType.

    };


class HTagDefinition
    {
public:

#define FileDirEntry32_size 12
#define FileDirEntry64_size 20
    typedef    struct {
        unsigned short FileTag;        // TAG File number
        unsigned short DataType;        // see HTIFFTagDefinition
        uint64_t DirCount64;        // Number of items
        uint32_t DirCount32;
        uint64_t Offset64;        // Byte offset to field data
        uint32_t Offset32;
        } FileDirEntry64;

    // Primary methods

    HTagDefinition  (const HTagInfo& pi_rTagInfo, FileDirEntry64& pi_TagDescriptor, bool pi_IsTiff64);
    HTagDefinition  (const HTagInfo& pi_rTagInfo, HTagID pi_Tag, bool pi_IsTiff64=false);
    HTagDefinition  (const HTagInfo& pi_rTagInfo, uint32_t pi_TagFile, bool pi_IsTiff64=false);


    virtual             ~HTagDefinition ();


    bool               IsValid(HTIFFError**  po_ppError) const;


    // Others Methods

    uint32_t            GetFileTag() const;
    HTagID              GetID() const;
    short GetReadCount() const;
    short GetWriteCount() const;
    HTagInfo::DataType  GetDataType() const;
    uint32_t            GetDataLen() const;
    bool               IsReadOnly() const;
    bool               IsPassDirCount() const;
    const char*        GetTagName() const;

protected:
private:
    // Members
    const HTagInfo::Info*
    m_pTagInfo;     // Pointer on the Entry
    HTIFFError*         m_pError;       // Error message

    // Methods

    // Not implemented
    HTagDefinition      (const HTagDefinition& pi_rObj);
    HTagDefinition& operator=(const HTagDefinition& pi_rObj);


    bool               FindFirstTag            (size_t                 pi_TagDefinitionQty,
                                                 const HTagInfo::Info*  pi_pTagDefinitionArray,
                                                 uint32_t               pi_TagFile,
                                                 size_t*                po_pIndex);
    bool               FindNextTag             (size_t                 pi_TagDefinitionQty,
                                                 const HTagInfo::Info*  pi_pTagDefinitionArray,
                                                 uint32_t               pi_TagFile,
                                                 size_t*                pio_pIndex);
    };
END_IMAGEPP_NAMESPACE

#include "HTagDefinition.hpp"

