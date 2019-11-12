/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/Tile.h>

#define BEGIN_TILE_IO_NAMESPACE BEGIN_TILE_NAMESPACE namespace IO {
#define END_TILE_IO_NAMESPACE } END_TILE_NAMESPACE
#define USING_NAMESPACE_TILE_IO using namespace BentleyApi::Dgn::Tile::IO;

//=======================================================================================
//! Classes and constants for reading and writing 3D tiles in various formats.
//! The binary stream for a 3D tile is always prefaced with a 4-character 'magic' value
//! which identifies the tile format and a 32-bit tile format version.
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
BEGIN_TILE_IO_NAMESPACE

//=======================================================================================
//! Return codes when deserializing tiles.
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
enum class ReadStatus
    {
    Success = 0,
    InvalidHeader,    
    ReadError,
    BatchTableParseError,
    SceneParseError,
    SceneDataError,
    FeatureTableError,
    };

//=======================================================================================
//! Return codes when serializing tiles.
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
enum class WriteStatus
    {
    Success = 0,
    NoGeometry,
    UnableToLoadTile,
    UnableToOpenFile,
    UnableToWriteFile,
    Aborted,
    };

//=======================================================================================
//! Enumerates the various tile formats. These are the 'magic' 4-character values which
//! identify the format within the binary GLTF stream.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
enum class Format : uint32_t
{
    Unknown = 0,
    Gltf = 'FTlg',
    B3dm = 'md3b',
    I3dm = 'md3i',
    Vector = 'rtcv',
    PointCloud = 'stnp',
    Composite = 'tpmc',
    Dgn = 'Tngd',
    IModel = 'ldMi',
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct TileHeader
{
    Format      format = Format::Unknown;
    uint32_t    version;

    bool Read(StreamBufferR buffer)
        {
        bool valid = buffer.Read(format) && buffer.Read(version) && IsValidFormat(format);
        if (!valid)
            Invalidate();

        return valid;
        }

    bool IsValid() const { return Format::Unknown != format; }
    void Invalidate() { format = Format::Unknown; }

    static bool IsValidFormat(Format f)
        {
        switch (f)
            {
            case Format::Unknown:
            case Format::Gltf:
            case Format::B3dm:
            case Format::I3dm:
            case Format::Vector:
            case Format::PointCloud:
            case Format::Composite:
            case Format::Dgn:
            case Format::IModel:
                return true;
            default:
                return false;
            }
        }

    DGNPLATFORM_EXPORT static Utf8CP FileExtensionFromFormat(Format fmt);
    DGNPLATFORM_EXPORT static Format FormatFromFileExtension(Utf8CP ext);
    static Format FormatFromFileName(BeFileNameCR filename) { return FormatFromFileExtension(filename.GetExtension().c_str()); }
    static Format FormatFromFileExtension(WCharCP ext) { return FormatFromFileExtension(Utf8String(ext).c_str()); }
};

//=======================================================================================
//! A web standard for defining 3D content.
//! 3D tile formats contain gltf data along with additional data often expressed in JSON.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct Gltf
{
    enum Versions : uint32_t
    {
        Version1 = 1,
        Version2 = 2,
        Version = Version1,
        SceneFormat = 0,
    };

    struct Header : TileHeader
    {
        uint32_t    gltfLength;
        uint32_t    sceneStrLength;
        uint32_t    gltfSceneFormat;

        bool Read(StreamBufferR buffer)
            {
            if (TileHeader::Read(buffer) && Format::Gltf == format && (Version1 == version || Version2 == version)
                && buffer.Read(gltfLength) && buffer.Read(sceneStrLength) && buffer.Read(gltfSceneFormat) && SceneFormat == gltfSceneFormat)
                return true;

            Invalidate();
            return false;
            }
    };

    //! GLTF constants for primitive types
    enum class PrimitiveType : int32_t
    {
        Lines = 1,
        LineStrip = 3,
        Triangles = 4,
    };

    //! GLTF constants for data types
    enum class DataType : int32_t
    {
        SignedByte = 0x1400,
        UnsignedByte = 0x1401,
        SignedShort = 5122,
        UnsignedShort = 5123,
        UInt32 = 5125,
        Float = 5126,
        Rgb = 6407,
        Rgba = 6408,
        IntVec2 = 0x8b53,
        IntVec3 = 0x8b54,
        FloatVec2 = 35664,
        FloatVec3 = 35665,
        FloatVec4 = 35666,
        FloatMat3 = 35675,
        FloatMat4 = 35676,
        Sampler2d = 35678,
    };

    //! Miscellaneous GLTF constants
    enum Constants : int32_t
    {
        CullFace = 2884,
        DepthTest = 2929,
        Nearest = 0x2600,
        Linear = 9729,
        LinearMipmapLinear = 9987,
        ClampToEdge = 33071,
        ArrayBuffer = 34962,
        ElementArrayBuffer = 34963,
        FragmentShader = 35632,
        VertexShader = 35633,
    };

    static DataType ToDataType(int32_t val) { BeAssert(IsValidDataType(val)); return static_cast<DataType>(val); }

    static bool IsValidDataType(int32_t val)
        {
        switch (static_cast<DataType>(val))
            {
            case DataType::SignedByte: return true;
            case DataType::UnsignedByte: return true;
            case DataType::SignedShort: return true;
            case DataType::UnsignedShort: return true;
            case DataType::UInt32: return true;
            case DataType::Float: return true;
            case DataType::Rgb: return true;
            case DataType::Rgba: return true;
            case DataType::IntVec2: return true;
            case DataType::IntVec3: return true;
            case DataType::FloatVec2: return true;
            case DataType::FloatVec3: return true;
            case DataType::FloatVec4: return true;
            case DataType::FloatMat3: return true;
            case DataType::FloatMat4: return true;
            case DataType::Sampler2d: return true;
            default: return false;
            }
        }
};

//=======================================================================================
//! A web standard for defining 'batched 3D models'
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct B3dm
{
    enum Versions : uint32_t { Version = 1 };

    struct Header : TileHeader
    {
        uint32_t    b3dmLength;
        uint32_t    featureTableStrLen;
        uint32_t    featureTableBinaryLen;
        uint32_t    batchTableStrLen;
        uint32_t    batchTableBinaryLen;

        bool Read(StreamBufferR buffer)
            {
            if (TileHeader::Read(buffer) && Format::B3dm == format && Version == version
                && buffer.Read(b3dmLength)
                && buffer.Read(featureTableStrLen) && buffer.Read(featureTableBinaryLen)
                && buffer.Read(batchTableStrLen) && buffer.Read(batchTableBinaryLen))
                return true;

            Invalidate();
            return false;
            }
    };
};

//=======================================================================================
//! A web standard for defining 'instanced 3D models'
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct I3dm
{
    enum Versions : uint32_t { Version = 1 };
};

//=======================================================================================
//! A web standard for defining point clouds.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct PointCloud
{
    enum Versions : uint32_t { Version = 1 };
};

//=======================================================================================
//! A web standard for defining vector tiles.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct Vector
{
    enum Versions : uint32_t { Version = 1 };
};

//=======================================================================================
//! A web standard for combining multiple tiles of other formats (b3dm, i3dm, etc) into a single file.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct Composite
{
    enum Versions : uint32_t { Version = 1 };
};

//=======================================================================================
// DgnTiles are tiles with GLTF-like structure (JSon with binary data) - but optimized
// for caching tiles (storing feature table directly etc.)
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct DgnTile
{
    enum Versions : uint32_t { Version = 0 };

    enum class Flags : uint32_t
    {
        None            = 0,
        ContainsCurves  = 0x0001,
        Incomplete      = 0x0001 << 1,
        IsLeaf          = 0x0001 << 2,
        HasZoomFactor   = 0x0001 << 3,
    };

    struct Header : TileHeader
    {
        Flags               flags;
        ElementAlignedBox3d contentRange;
        double              zoomFactor;
        uint32_t            length;

        bool Read(StreamBufferR buffer)
            {
            if (TileHeader::Read(buffer) && IsValid() && buffer.Read(flags) && buffer.Read(contentRange) && buffer.Read(zoomFactor) && buffer.Read(length))
                return true;

            Invalidate();
            return false;
            }
    };

    struct FeatureTableHeader
    {
        uint32_t    length;
        uint32_t    maxFeatures;
        uint32_t    count;

        bool Read(StreamBufferR buffer)
            {
            return buffer.Read(length) && buffer.Read(maxFeatures) && buffer.Read(count);
            }
    };
};

ENUM_IS_FLAGS(DgnTile::Flags);

//=======================================================================================
// IModel tiles are tiles with glTF-like structure (JSON with binary data) - but optimized
// for deserialization and display in imodeljs front-end.
// Specifically, geometry is stored in a format more or less ready for submission to WebGL.
//
// Version history:
//  Major version 1:
//      1.1: Introduction of major-minor versioning.
//      1.2: Added animation group IDs.
//      1.3: Moved aux channels from vertex table to separate table.
//      1.4: Fixed duplication of polyline and simple edges.
//  Major version 2:
//      2.0: Introduction of major version 2
//      2.1: Fixed SurfaceMaterial::isLessThan() incorrectly returning true resulting in failure to batch meshes having materials.
//  Major version 3:
//      3.0: Introduction of major version 3:
//          Improved instancing heuristic.
//          Decimation of polylines.
//      4.0: Introduction of major version 4:
//          Versioning of TileTree JSON. Changes format of Tree::Id to "_vMaj_flags-..." where "..." is unchanged from Id format used by previous versions.
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct IModelTile
{
    // Describes the version of the tile format. The front-end which receives the tile data uses this information to interpret the contents.
    // The front-end code that receives the tile data may not be up to date with the current format version produced by the back-end. In this case:
    //  - If the front-end recognizes the major version, it is able to read the tile data regardless of the minor version. It may skip any data introduced
    //  by a newer minor version than the version it understands.
    //  - If the major version in the tile data is greater than any major version recognized by the front-end, it cannot read the tile data.
    // Changes to major version should therefore be rare.
    // Any changes to the tile format must be documented and accompanied by a corresponding bump in either the current major or current minor version number.
    struct Version
    {
        uint16_t    m_major;
        uint16_t    m_minor;

        explicit constexpr Version(uint16_t v_maj, uint16_t v_min) : m_major(v_maj), m_minor(v_min) { }
        uint32_t ToUint32() const { return (m_major << 0x10) | m_minor; }
        Utf8String ToString() const { return Utf8PrintfString("%08x", ToUint32()); }

        static constexpr Version Invalid() { return Version(0, 0); }
        static constexpr Version V1() { return Version(1, 4); }
        static constexpr Version V2() { return Version(2, 1); }
        static constexpr Version V3() { return Version(3, 0); }
        static constexpr Version V4() { return Version(4, 0); }
        static constexpr Version V5() { return Version(5, 0); }

        // !!! IMPORTANT !!! If you change the major version you must update IModelTile::Version::FromMajorVersion !!!
        static constexpr Version Current() { return V5(); }

        DGNPLATFORM_EXPORT static Version FromMajorVersion(uint16_t major);

        constexpr bool IsValid() const { return 0 != m_major; }
        constexpr bool IsKnownMajorVersion() const { return IsKnownMajorVersion(m_major); }
        constexpr bool IsKnown() const { return IsKnownMajorVersion() && m_minor <= Current().m_minor; }
        static constexpr bool IsKnownMajorVersion(uint16_t major) { return 0 < major && major <= Current().m_major; }
    };

    enum class WriteStatus { Success, Error, Aborted };

    struct Header : TileHeader
    {
        uint32_t                headerLength;   // size of this header in bytes.
        Tile::Content::Metadata metadata;
        uint32_t                tileLength;     // size of entire tile data in bytes, including this header.

        bool Read(StreamBufferR buffer)
            {
            if (TileHeader::Read(buffer) && IsValid() && buffer.Read(headerLength) && buffer.Read(metadata.m_flags) && buffer.Read(metadata.m_contentRange) && buffer.Read(metadata.m_tolerance)
                && buffer.Read(metadata.m_numElementsIncluded) && buffer.Read(metadata.m_numElementsExcluded) && buffer.Read(tileLength))
                return true;

            Invalidate();
            return false;
            }
    };

    struct FeatureTableHeader
    {
        uint32_t    length;
        uint32_t    maxFeatures;
        uint32_t    count;

        bool Read(StreamBufferR buffer)
            {
            return buffer.Read(length) && buffer.Read(maxFeatures) && buffer.Read(count);
            }
    };
};

DGNPLATFORM_EXPORT BentleyStatus WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, bool isLeaf, double const* zoomFactor);
DGNPLATFORM_EXPORT ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf, DRange3dCR tileRange);

// Read geometry from one of the web-standard tile formats (i3dm, b3dm, cmpt, pnts, vctr)
DGNPLATFORM_EXPORT ReadStatus ReadWebTile(Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem);

// Return false if feature table contains data not valid for DgnDb.
bool VerifyFeatureTable(StreamBufferR, DgnDbR);

IModelTile::WriteStatus WriteIModelTile(StreamBufferR streamBuffer, Tile::Content::MetadataCR metadata, Render::Primitives::GeometryCollectionCR geometry, Tile::LoaderCR loader);

END_TILE_IO_NAMESPACE
