/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/MeshTile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

//#include "Render.h"
//#include "DgnTexture.h"
//#include "SolidKernel.h"
#include <map> // NB: Because bmap doesn't support move semantics...
#include <ScalableMesh/GeoCoords/GCS.h>

#if defined(VANCOUVER_API) || defined(DGNDB06_API)

namespace BENTLEY_NAMESPACE_NAME
    {
    //=======================================================================================
    // Base class for 64 bit Ids.
    // @bsiclass                                                    Keith.Bentley   02/11
    //=======================================================================================
    struct BeInt64Id
        {
        public:
            //! @see BeInt64Id::ToString(Utf8Char*)
            static const size_t ID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

        protected:
            uint64_t m_id;

        public:
            //! Construct an invalid BeInt64Id
            BeInt64Id() { Invalidate(); }

            //! Construct a BeInt64Id from a 64 bit value.
            explicit BeInt64Id(uint64_t u) : m_id(u) {}

            //! Move constructor.
            BeInt64Id(BeInt64Id&& rhs) { m_id = rhs.m_id; }

            //! Construct a copy.
            BeInt64Id(BeInt64Id const& rhs) { m_id = rhs.m_id; }

            BeInt64Id& operator=(BeInt64Id const& rhs) { m_id = rhs.m_id; return *this; }

            bool IsValid() const { return Validate(); }

            //! Compare two BeInt64Id for equality
            bool operator==(BeInt64Id const& rhs) const { return rhs.m_id == m_id; }

            //! Compare two BeInt64Id for inequality
            bool operator!=(BeInt64Id const& rhs) const { return !(*this == rhs); }

            //! Compare two BeInt64Id
            bool operator<(BeInt64Id const& rhs) const { return m_id < rhs.m_id; }
            bool operator<=(BeInt64Id const& rhs) const { return m_id <= rhs.m_id; }
            bool operator>(BeInt64Id const& rhs) const { return m_id > rhs.m_id; }
            bool operator>=(BeInt64Id const& rhs) const { return m_id >= rhs.m_id; }

            //! Get the 64 bit value of this BeInt64Id
            uint64_t GetValue() const { BeAssert(IsValid()); return m_id; }

            //! Get the 64 bit value of this BeGuid. Does not check for valid value in debug builds.
            uint64_t GetValueUnchecked() const { return m_id; }

            //! Test to see whether this BeInt64Id is valid. 0 is not a valid id.
            bool Validate() const { return m_id != 0; }

            //! Set this BeInt64Id to an invalid value (0).
            void Invalidate() { m_id = 0; }

            //! Converts this BeInt64Id to its string representation.
            //! 
            //! Typical example:
            //!
            //!     Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
            //!     myId.ToString(idStrBuffer);
            //!
            //! @remarks The method does not have any checks that the buffer is large enough. Callers
            //! must ensure this to avoid unexpected behavior. 
            //!
            //! @param[in,out] stringBuffer The output buffer for the id string. Must be large enough
            //! to hold the maximal number of decimal digits of UInt64 plus the trailing 0 character.
            //! You can use BeInt64Id::ID_STRINGBUFFER_LENGTH to allocate the @p stringBuffer.
            void ToString(Utf8P stringBuffer) const
                {
//#ifndef VANCOUVER_API
//                BeStringUtilities::FormatUInt64(stringBuffer, m_id); //BeStringUtilities::FormatUInt64 is faster than sprintf.
//#else
                sprintf(stringBuffer, "%lld", m_id);
//#endif
                }

                                                                                                             //! Converts this BeInt64Id to its string representation.
                                                                                                             //! @remarks Consider the overload BeInt64Id::ToString(Utf8Char*) if you want
                                                                                                             //! to avoid allocating Utf8Strings.
            Utf8String ToString() const
                {
                Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
                ToString(idStrBuffer);
                return Utf8String(idStrBuffer);
                }
        };
    }

#endif

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
class XYZRangeTreeRoot;
END_BENTLEY_GEOMETRY_NAMESPACE

#ifndef VANCOUVER_API
#define CREATE_TYPEDEFS(_name_) DGNPLATFORM_TYPEDEFS(_name_)
#else
#define CREATE_TYPEDEFS(_name_) \
DGNPLATFORM_TYPEDEFS(_name_); \
typedef _name_##P _name_##Ptr; \
typedef _name_##CP _name_##CPtr;
#endif

CREATE_TYPEDEFS(Triangle);
CREATE_TYPEDEFS(TilePolyline);
CREATE_TYPEDEFS(TileMesh);
CREATE_TYPEDEFS(TileMeshBuilder);
CREATE_TYPEDEFS(TileNode);
CREATE_TYPEDEFS(TileGenerator);
CREATE_TYPEDEFS(TileGeometry);
CREATE_TYPEDEFS(TileDisplayParams);
CREATE_TYPEDEFS(Image);
CREATE_TYPEDEFS(ImageSource);
CREATE_TYPEDEFS(TileTextureImage);

DGNPLATFORM_REF_COUNTED_PTR(TileMesh);
DGNPLATFORM_REF_COUNTED_PTR(TileNode);
//BENTLEY_RENDER_REF_COUNTED_PTR(ElementTileNode);
//BENTLEY_RENDER_REF_COUNTED_PTR(ModelTileNode);
DGNPLATFORM_REF_COUNTED_PTR(TileMeshBuilder);
DGNPLATFORM_REF_COUNTED_PTR(TileGeometry);
DGNPLATFORM_REF_COUNTED_PTR(TileTextureImage);
DGNPLATFORM_REF_COUNTED_PTR(Image);
DGNPLATFORM_REF_COUNTED_PTR(ImageSource);
DGNPLATFORM_REF_COUNTED_PTR(TileDisplayParams);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector<TileMeshPtr> TileMeshList;
typedef bvector<TileNodePtr> TileNodeList;
typedef bvector<TileNodeP>   TileNodePList;
typedef bvector<TileGeometryPtr> TileGeometryList;

//=======================================================================================
//! Describes the type of entity from which a tile node or mesh was produced.
//! The IDs of the source entities are recorded in the TileMesh as BeInt64Ids.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
enum class TileSource
{
    Element,    //!< Geometry in the tile is associated with DgnElementIds
    Model,      //!< Geometry in the tile is associated with DgnModelIds
    None,       //!< No IDs are associated with geometry in the tile
};

#ifndef VANCOUVER_API
//=======================================================================================
//! A stream of bytes in a resizeable buffer. Released on destruction, never gets smaller.
//! This class is more efficient than bvector<byte> since it does not initialize the memory to zeros.
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
#ifdef DGNDB06_API
struct ByteStream
    {
    private:
        uint32_t m_size;
        uint32_t m_allocSize;
        uint8_t* m_data;
        void swap(ByteStream& rhs) { std::swap(m_size, rhs.m_size); std::swap(m_allocSize, rhs.m_allocSize); std::swap(m_data, rhs.m_data); }

    public:
        void Init() { m_size = m_allocSize = 0; m_data = nullptr; }
        ByteStream() { Init(); }
        explicit ByteStream(uint32_t size) { Init(); Resize(size); }
        ByteStream(uint8_t const* data, uint32_t size) { Init(); SaveData(data, size); }
        ByteStream(ByteStream const& other) { Init(); SaveData(other.m_data, other.m_size); }
        ~ByteStream() { Clear(); }
        ByteStream(ByteStream&& rhs) : m_size(rhs.m_size), m_allocSize(rhs.m_allocSize), m_data(rhs.m_data) { rhs.m_size = rhs.m_allocSize = 0; rhs.m_data = nullptr; }
        ByteStream& operator=(ByteStream const& other) { if (this != &other) SaveData(other.m_data, other.m_size); return *this; }
        ByteStream& operator=(ByteStream&& rhs) { ByteStream(std::move(rhs)).swap(*this); return *this; }

        //! Get the size, in bytes, of the memory allocated for this ByteStream.
        //! @note The allocated size may be larger than the currently used size returned by GetSize.
        uint32_t GetAllocSize() const { return m_allocSize; }
        uint32_t GetSize() const { return m_size; }   //!< Get the size in bytes of the current data in this ByteStream.
        uint8_t const* GetData() const { return m_data; } //!< Get a const pointer to the ByteStream.
        uint8_t* GetDataP() const { return m_data; }      //!< Get a writable pointer to the ByteStream.
        bool HasData() const { return 0 != m_size; }  //!< return false if this ByteStream is empty.
        void Clear() { FREE_AND_CLEAR(m_data); m_size = m_allocSize = 0; } //!< Return this object to an empty/uninitialized state.
        uint8_t* ExtractData() { uint8_t* data = m_data; m_data = nullptr; m_size = m_allocSize = 0; return data; }

        //! Reserve memory for this ByteStream. The stream capacity will change but not its size.
        //! @param[in] size the number of bytes to reserve
        void Reserve(uint32_t size) { if (size <= m_allocSize) return; m_data = (uint8_t*)realloc(m_data, size); m_allocSize = size; }

        //! Resize the stream. If more memory is required, the new portion won't be initialized.
        //! @param[in] newSize number of bytes
        void Resize(uint32_t newSize) { Reserve(newSize); m_size = newSize; }

        //! Save a stream of bytes into this ByteStream.
        //! @param[in] data the data to save
        //! @param[in] size number of bytes in data
        void SaveData(uint8_t const* data, uint32_t size) { m_size = 0; Append(data, size); }

        //! Append a stream of byes to the current end of this ByteStream.
        //! @param[in] data the data to save
        //! @param[in] size number of bytes in data
        void Append(uint8_t const* data, uint32_t size)
            {
            if (data)
                {
                Reserve(m_size + size);
                memcpy(m_data + m_size, data, size);
                m_size += size;
                }
            }

        bool empty() const { return !HasData(); }
        size_t size() const { return GetSize(); }
        size_t capacity() const { return GetAllocSize(); }
        void reserve(size_t size) { Reserve(static_cast<uint32_t>(size)); }
        void resize(size_t newSize) { Resize(static_cast<uint32_t>(newSize)); }
        void clear() { Clear(); }
        uint8_t const* data() const { return GetData(); }
        uint8_t* data() { return GetDataP(); }

        typedef uint8_t* iterator;
        typedef uint8_t const* const_iterator;

        iterator begin() { return data(); }
        iterator end() { return data() + size(); }
        const_iterator begin() const { return data(); }
        const_iterator end() const { return data() + size(); }
        uint8_t const& operator[](size_t i) const { return data()[i]; }
        uint8_t& operator[](size_t i) { return data()[i]; }
    };
#endif
#endif

//=======================================================================================
//! An uncompressed image in Rgb (3 bytes per pixel) or Rgba (4 bytes per pixel) format suitable for rendering.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Image
    {
    enum class Format { Rgba = 0, Rgb = 2 }; // must match qvision.h values
    enum class BottomUp : bool { No = 0, Yes = 1 }; //!< whether the rows in the image should be flipped top-to-bottom
    protected:
        uint32_t   m_width = 0;
        uint32_t   m_height = 0;
        Format     m_format = Format::Rgb;
        bool       m_headerOnly = false;
        ByteStream m_image;

        void ClearData() { m_image.Clear(); }
        void Initialize(uint32_t width, uint32_t height, Format format = Format::Rgb) { m_height = height; m_width = width; m_format = format; ClearData(); }

        void ReadJpeg(uint8_t const* srcData, uint32_t srcLen, Format targetFormat, BottomUp bottomUp);
        //void ReadPng(uint8_t const* srcData, uint32_t srcLen, Format targetFormat);

    public:
        //! Construct a blank invalid Image
        Image() {}

        //! Construct an image from a ByteStream containing either Rgb or Rgba data.
        //! @param[in] width the width of the image in pixels
        //! @param[in] height the height of the image in pixels
        //! @param[in] image an rvalue reference to the ByteStream holding the data. The ByteStream is moved into the newly constructed Image.
        //! @param[in] format the format of the data held in image
        //! @note the ByteStream is moved by this constructor, so it will be empty after this call. To use this method, you must
        //! either pass a temporary variable or use std::move on a non-temporary variable.
        Image(uint32_t width, uint32_t height, ByteStream&& image, Format format) : m_width(width), m_height(height), m_format(format), m_image(std::move(image)) {}

        //! Construct an image from an ImageSource.
        //! @param[in] source the ImageSource from which the image is to be created.
        //! @param[in] targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded. If
        //! the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
        //! @param[in] bottomUp If Yes, the source image is flipped vertically (top-to-bottom) to create the image.
        //! @note If the source is invalid, or if the decompression fails, IsValid() will return false on the new Image.
        /*DGNPLATFORM_EXPORT explicit*/ Image(ImageSourceCR source, Format targetFormat = Format::Rgba, BottomUp bottomUp = BottomUp::No, bool headerOnly = false);

        //! Create an image from a Jpeg.
        //! @param[in]      srcData      the Jpeg data
        //! @param[in]      srcLen       the number of bytes of Jpeg data
        //! @param[in]      targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
        //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
        //! @param[in]      bottomUp     If Yes, the source image is flipped vertically (top-to-bottom) to create the image.
        //! @return The decompressed Image, or an invalid Image if decompression failed.
        DGNPLATFORM_EXPORT static Image FromJpeg(uint8_t const* srcData, uint32_t srcLen, Format targetFormat = Format::Rgba, BottomUp bottomUp = BottomUp::No);

        //! Create an image from a Png.
        //! @param[in]      srcData      the Png data
        //! @param[in]      srcLen       the number of bytes of Png data
        //! @param[in]      targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
        //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
        //! @return The decompressed Image, or an invalid Image if decompression failed.
        //DGNPLATFORM_EXPORT static Image FromPng(uint8_t const* srcData, uint32_t srcLen, Format targetFormat = Format::Rgba);

        //! Create an image by resizing a source image.
        //! @param[in] width the width of the image in pixels
        //! @param[in] height the height of the image in pixels
        //! @param[in] sourceImage the source image
        /*DGNPLATFORM_EXPORT*/ static Image FromResizedImage(uint32_t width, uint32_t height, ImageCR sourceImage);

        int GetBytesPerPixel()const { return m_format == Format::Rgba ? 4 : 3; } //!< get the number of bytes per pixel
        void Invalidate() { m_width = m_height = 0; ClearData(); } //!< Clear the contents and invalidate this image.
        uint32_t GetWidth() const { return m_width; } //!< Get the width of this image in pixels
        uint32_t GetHeight() const { return m_height; } //!< Get the height of this image in pixels
        Format GetFormat() const { return m_format; } //!< Get the format (Rgb or Rgba) of this image
        void SetFormat(Format format) { m_format = format; } //!< Change the format of this image in pixels
        bool IsValid() const { return 0 != m_width && 0 != m_height && 0 != m_image.GetSize(); } //!< @return true if this image holds valid data
        ByteStream const& GetByteStream() const { return m_image; } //!< get a readonly reference to the ByteStream of this image
        ByteStream& GetByteStreamR() { return m_image; }//!< Get a writable reference to the ByteStream of this image
        void SetSize(uint32_t width, uint32_t height) { BeAssert(0 == m_width && 0 == m_height); m_width = width; m_height = height; } //!< change the size in pixels of this image
        void SetHeaderOnly(bool headerOnly) { m_headerOnly = headerOnly; }
        void ReadImageData(ImageSourceCR source, Format targetFormat, Image::BottomUp bottomUp = BottomUp::No);
    };

//=======================================================================================
//! A compressed image in either JPEG or PNG format. This is called a "source" because it is usually
//! stored externally and can be used to create an Image.
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct ImageSource
    {
    enum class Format : uint32_t { Jpeg = 0, Png = 2 }; // don't change values, saved in DgnTexture elements

    private:
        Format m_format = Format::Jpeg;
        ByteStream m_stream;

    public:
        Format GetFormat() const { return m_format; } //!< Get the format of this ImageSource
        void SetFormat(Format format) { m_format = format; } //!< Change the format of this ImageSource
        ByteStream const& GetByteStream() const { return m_stream; } //!< Get a readonly reference to the ByteStream of this ImageSource.
        ByteStream& GetByteStreamR() { return m_stream; } //!< Get a writable reference to the ByteStream of this ImageSource.
        bool IsValid() const { return 0 < m_stream.GetSize(); } //!< @return true if this ImageSource holds valid data
        DGNPLATFORM_EXPORT Point2d GetSize() const; //!< Reads and returns the width and height of this ImageSource

                                                    //! Construct a blank invalid ImageSource
        ImageSource() {}

        //! Construct an ImageSource from a ByteStream containing either Jpeg or Png data.
        //! @param[in] format the format of the data held in stream
        //! @param[in] stream an rvalue reference to a ByteStream holding the data. The ByteStream is moved into the newly constructed ImageSource.
        //! @note the ByteStream is moved by this constructor, so it will be empty after this call. To use this method, you must
        //! either pass a temporary variable or use std::move on a non-temporary variable.
        explicit ImageSource(Format format, ByteStream&& stream) : m_format(format), m_stream(stream) {}

        //! Construct an ImageSource by compressing the pixels of an Image. The compression is done using either Jpeg or Png format.
        //! @param[in] image the Rgb or Rgba image data to compress to create the ImageSource
        //! @param[in] format the format (either Jpeg or Png) to compress the image
        //! @param[in] quality a value between 1 and 100 to control the level of compression. Used only if format==Jpeg.
        //! @param[in] bottomUp If Yes, the image is flipped vertically (top-to-bottom) in the new ImageSource.
        //! @note If the image is invalid, or if the compression fails, IsValid() will return false on the new ImageSource.
        /*DGNPLATFORM_EXPORT explicit*/ ImageSource(ImageCR image, Format format, int quality = 100, Image::BottomUp bottomUp = Image::BottomUp::No);
    };

//=======================================================================================
// ! Holds a texture image.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTextureImage : RefCountedBase
    {
    private:
        ImageSource       m_imageSource;

//#ifndef VANCOUVER_API
//        static ImageSource Load(TileDisplayParamsCR params, DgnDbR db);
//#else
        static ImageSource Load(TileDisplayParamsCR params);
//#endif

        TileTextureImage(ImageSource&& imageSource) : m_imageSource(std::move(imageSource)) { BeAssert(m_imageSource.IsValid()); }
        TileTextureImage(ImageSource& imageSource) : m_imageSource (imageSource) { BeAssert(m_imageSource.IsValid()); }
    public:
        static TileTextureImagePtr Create(ImageSource&& imageSource) { return new TileTextureImage(std::move(imageSource)); }
        static TileTextureImagePtr Create(ImageSource& imageSource) { return new TileTextureImage(imageSource); }

        ImageSourceCR GetImageSource() const { return m_imageSource; }
//#ifndef VANCOUVER_API
//        static void ResolveTexture(TileDisplayParamsR params, DgnDbR db);
//#else
        static void ResolveTexture(TileDisplayParamsCR params);
//#endif
    };

//=======================================================================================
//! Display params associated with TileGeometry. Based on GraphicParams and GeometryParams.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TileDisplayParams : RefCountedBase
{
private:
    uint32_t                m_fillColor;
    //DgnMaterialId           m_materialId;
    TileTextureImagePtr     m_textureImage;
    bool                    m_ignoreLighting;

    //TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams);
    TileDisplayParams(uint32_t fillColor, TileTextureImagePtr texture, bool ignoreLighting) : m_fillColor(fillColor), m_textureImage(texture), m_ignoreLighting(ignoreLighting) { }
public:
    static TileDisplayParamsPtr Create() 
		{ 
#ifndef DGNDB06_API
		return Create(0, nullptr,false); 
#endif		
		}
    //static TileDisplayParamsPtr Create(GraphicParamsCR graphicParams, GeometryParamsCR geometryParams) { return Create(&graphicParams, &geometryParams); }
    //static TileDisplayParamsPtr Create(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams) { return new TileDisplayParams(graphicParams, geometryParams); }
    static TileDisplayParamsPtr Create(uint32_t fillColor, TileTextureImagePtr textureImage, bool ignoreLighting) { return new TileDisplayParams(fillColor, textureImage, ignoreLighting); }

    bool operator<(TileDisplayParams const& rhs) const;

    //DgnMaterialId GetMaterialId() const { return m_materialId; }
    uint32_t GetFillColor() const { return m_fillColor; }
    bool GetIgnoreLighting() const { return m_ignoreLighting; }
    //DgnTextureCPtr QueryTexture(DgnDbR db) const;
    TileTextureImagePtr& TextureImage() { return m_textureImage; }
    TileTextureImageCP GetTextureImage() const { return m_textureImage.get(); }
};

//=======================================================================================
//! Represents one triangle of a TileMesh.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct Triangle
{
    uint32_t    m_indices[3];   // indexes into point/normal/uvparams/elementID vectors
    bool        m_singleSided;

    explicit Triangle(bool singleSided=true) : m_singleSided(singleSided) { SetIndices(0, 0, 0); }
    Triangle(uint32_t indices[3], bool singleSided) : m_singleSided(singleSided) { SetIndices(indices); }

    void SetIndices(uint32_t indices[3]) { SetIndices(indices[0], indices[1], indices[2]); }
    void SetIndices(uint32_t a, uint32_t b, uint32_t c) { m_indices[0] = a; m_indices[1] = b; m_indices[2] = c; }

    bool IsDegenerate() const
        {
        return m_indices[0] == m_indices[1] || m_indices[0] == m_indices[2] || m_indices[1] == m_indices[2];
        }
};

//=======================================================================================
//! Represents a single polyline  of a TileMesh
//! 
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TilePolyline
{
     bvector <uint32_t>     m_indices;
};  // TilePolyline


//=======================================================================================
//! Represents a single mesh of uniform symbology within a TileNode, consisting of
//! vertex/normal/uv-param/elementID arrays indexed by an array of triangles.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMesh : RefCountedBase
{
private:
    TileDisplayParamsPtr    m_displayParams;
    bvector<Triangle>       m_triangles;
    bvector<TilePolyline>   m_polylines;
    bvector<DPoint3d>       m_points;
    bvector<DVec3d>         m_normals;
    bvector<DPoint2d>       m_uvParams;
    bvector<BeInt64Id>      m_entityIds;   // invalid IDs for clutter geometry
    bool                    m_validIdsPresent;

    explicit TileMesh(TileDisplayParamsPtr& params) : m_displayParams(params), m_validIdsPresent (false) { }

    template<typename T> T const* GetMember(bvector<T> const& from, uint32_t at) const { return at < from.size() ? &from[at] : nullptr; }
public:
    static TileMeshPtr Create(TileDisplayParamsPtr& params) { return new TileMesh(params); }

    //TileMesh() : TileMesh(nullptr) {}

    DRange3d GetTriangleRange(TriangleCR triangle) const;
    DVec3d GetTriangleNormal(TriangleCR triangle) const;
    bool HasNonPlanarNormals() const;

    TileDisplayParamsCP GetDisplayParams() const { return m_displayParams.get(); } //!< The mesh symbology
    TileDisplayParamsPtr GetDisplayParamsPtr() const { return m_displayParams; } //!< The mesh symbology
    bvector<Triangle> const& Triangles() const { return m_triangles; } //!< Triangles defined as a set of 3 indices into the vertex attribute arrays.
    bvector<TilePolyline> const& Polylines() const { return m_polylines; } //!< Polylines defined as a set of indices into the vertex attribute arrays.
    bvector<DPoint3d> const& Points() const { return m_points; } //!< Position vertex attribute array
    bvector<DVec3d> const& Normals() const { return m_normals; } //!< Normal vertex attribute array
    bvector<DPoint2d> const& Params() const { return m_uvParams; } //!< UV params vertex attribute array
    bvector<BeInt64Id> const& EntityIds() const { return m_entityIds; } //!< Vertex attribute array specifying the ID of the entity (element or model) from which the vertex was produced

    TriangleCP GetTriangle(uint32_t index) const { return GetMember(m_triangles, index); }
    DPoint3dCP GetPoint(uint32_t index) const { return GetMember(m_points, index); }
    DVec3dCP GetNormal(uint32_t index) const { return GetMember(m_normals, index); }
    DPoint2dCP GetParam(uint32_t index) const { return GetMember(m_uvParams, index); }
    BeInt64Id GetEntityId(uint32_t index) const { auto pId = GetMember(m_entityIds, index); return nullptr != pId ? *pId : BeInt64Id(); }
    bool IsEmpty() const { return m_triangles.empty() && m_polylines.empty(); }
    DRange3d GetRange() const { return DRange3d::From (m_points); }

    DRange3d GetUVRange() const { return DRange3d::From (m_uvParams, 0.0); }

    bool ValidIdsPresent() const { return m_validIdsPresent; }

    void AddTriangle(TriangleCR triangle) { m_triangles.push_back(triangle); }
    void AddPolyline (TilePolyline polyline) { m_polylines.push_back(polyline); }
    uint32_t AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param/*, BeInt64Id entityId*/);

    void ReprojectPoints(GeoCoordinates::BaseGCSCPtr sourceGCS, GeoCoordinates::BaseGCSCPtr destinationGCS);
    void ApplyTransform(const Transform& transform);
};

//=======================================================================================
//! Builds a single TileMesh to a specified level of detail, optionally applying vertex
//! clustering.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileMeshBuilder : RefCountedBase
{
    struct VertexKey
        {
        DPoint3d        m_point;
        DVec3d          m_normal;
        DPoint2d        m_param;
        //BeInt64Id       m_entityId;
        bool            m_normalValid = false;
        bool            m_paramValid = false;

        VertexKey() { }
        VertexKey(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param/*, BeInt64Id entityId*/) : m_point(point), m_normalValid(nullptr != normal), m_paramValid(nullptr != param)/*, m_entityId(entityId)*/
            {
            if (m_normalValid) m_normal = *normal;
            if (m_paramValid) m_param = *param;
            }

        DVec3dCP GetNormal() const { return m_normalValid ? &m_normal : nullptr; }
        DPoint2dCP GetParam() const { return m_paramValid ? &m_param : nullptr; }

        struct Comparator
            {
            double  m_tolerance;

            explicit Comparator(double tolerance) : m_tolerance(tolerance) { }
            bool operator()(VertexKey const& lhs, VertexKey const& rhs) const;
            };
        };
private:
    struct TriangleKey
    {
        uint32_t    m_sortedIndices[3];

        TriangleKey() { }
        explicit TriangleKey(TriangleCR triangle);

        bool operator<(TriangleKey const& rhs) const;
    };

    typedef bmap<VertexKey, uint32_t, VertexKey::Comparator> VertexMap;
    typedef bset<TriangleKey> TriangleSet;

    TileMeshPtr             m_mesh;
    VertexMap               m_clusteredVertexMap;
    VertexMap               m_unclusteredVertexMap;
    TriangleSet             m_triangleSet;
    double                  m_tolerance;
    size_t                  m_triangleIndex;

    TileMeshBuilder(TileDisplayParamsPtr& params, double tolerance) : m_mesh(TileMesh::Create(params)), m_unclusteredVertexMap (VertexKey::Comparator(1.0E-4)), m_clusteredVertexMap(VertexKey::Comparator(tolerance)), 
            m_tolerance(tolerance), m_triangleIndex(0) { }
public:
    static TileMeshBuilderPtr Create(TileDisplayParamsPtr& params, double tolerance) { return new TileMeshBuilder(params, tolerance); }

    void AddTriangle(PolyfaceVisitorR visitor, /*DgnMaterialId materialId, DgnDbR dgnDb, BeInt64Id entityId,*/ bool doVertexClustering, bool duplicateTwoSidedTriangles);
    void AddPolyline (bvector<DPoint3d>const& polyline, BeInt64Id entityId, bool doVertexClustering);
    void AddPolyface(PolyfaceQueryCR polyface, bool duplicateTwoSidedTriangles);

    void AddTriangle(TriangleCR triangle, TileMeshCR mesh);
    void AddTriangle(TriangleCR triangle);
    uint32_t AddClusteredVertex(VertexKey const& vertex);
    uint32_t AddVertex(VertexKey const& vertex);

    TileMeshP GetMesh() { return m_mesh.get(); } //!< The mesh under construction
    double GetTolerance() const { return m_tolerance; }
};

//=======================================================================================
//! Representation of geometry processed by a TileGenerator.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileGeometry : RefCountedBase
{
    enum class NormalMode
    {
        Never,              //!< Never generate normals
        Always,             //!< Always generate normals
        CurvedSurfacesOnly, //!< Generate normals only for curved surfaces
    };
private:
    //TileDisplayParamsPtr    m_params;
    Transform               m_transform;
    DRange3d                m_tileRange;
    BeInt64Id               m_entityId;
    size_t                  m_facetCount;
    double                  m_facetCountDensity;
    bool                    m_isCurved;
    bool                    m_hasTexture;

protected:
//#ifndef VANCOUVER_API
//    TileGeometry(TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ bool isCurved, DgnDbR db);
//#else
    TileGeometry(TransformCR tf, DRange3dCR tileRange, bool isCurved);
//#endif

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) = 0;
    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) = 0;
    virtual bool _IsPolyface() const = 0;

    void SetFacetCount(size_t numFacets);
    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode) const;
public:
    //TileDisplayParamsPtr GetDisplayParams() const { return m_params; }
    TransformCR GetTransform() const { return m_transform; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    BeInt64Id GetEntityId() const { return m_entityId; } //!< The ID of the element from which this geometry was produced

    size_t GetFacetCount() const { return m_facetCount; }
    double GetFacetCountDensity() const { return m_facetCountDensity; }

    bool IsCurved() const { return m_isCurved; }
    bool HasTexture() const { return m_hasTexture; }

    PolyfaceHeaderPtr GetPolyface(double chordTolerance, NormalMode normalMode);
    bool IsPolyface() const { return _IsPolyface(); }
    CurveVectorPtr    GetStrokedCurve (double chordTolerance) { return _GetStrokedCurve(chordTolerance); }
    
//#ifndef VANCOUVER_API
//    //! Create a TileGeometry for an IGeometry
//    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ IFacetOptionsR facetOptions, bool isCurved, DgnDbR db);
//    //! Create a TileGeometry for an ISolidKernelEntity
//    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ IFacetOptionsR facetOptions, DgnDbR db);
//#else
    //! Create a TileGeometry for an IGeometry
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ IFacetOptionsR facetOptions, bool isCurved);
    //! Create a TileGeometry for an ISolidKernelEntity
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
	static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ IFacetOptionsR facetOptions);
#else
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR tileRange, BeInt64Id entityId, /*TileDisplayParamsPtr& params,*/ IFacetOptionsR facetOptions);
#endif
//#endif
    };

////=======================================================================================
////! Filters elements during TileNode generation. Elements are selected according to their
////! intersection with a TileNode's range, then tested against the supplied ITileGenerationFilter
////! to apply additional selection criteria.
//// @bsistruct                                                   Paul.Connelly   09/16
////=======================================================================================
//struct ITileGenerationFilter
//{
//protected:
//    virtual bool _AcceptElement(DgnElementId elementId) = 0;
//public:
//    //! Invoked for each element in the tile's range. Returns false to exclude the element from the tile geometry, or true to include it.
//    bool AcceptElement(DgnElementId elementId) { return _AcceptElement(elementId); }
//};

////=======================================================================================
////! Accepts all elements.
//// @bsistruct                                                   Paul.Connelly   09/16
////=======================================================================================
//struct UnconditionalTileGenerationFilter : ITileGenerationFilter
//{
//protected:
//    virtual bool _AcceptElement(DgnElementId) override { return true; }
//};
//
////=======================================================================================
////! Filters elements according to a set of models and categories.
//// @bsistruct                                                   Paul.Connelly   09/16
////=======================================================================================
//struct TileModelCategoryFilter : ITileGenerationFilter
//{
//    struct ModelAndCategorySet : BeSQLite::VirtualSet
//        {
//    private:
//        DgnModelIdSet       m_models;
//        DgnCategoryIdSet    m_categories;
//
//        virtual bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override
//            {
//            return m_models.Contains(DgnModelId(vals[0].GetValueUInt64())) && m_categories.Contains(DgnCategoryId(vals[1].GetValueUInt64()));
//            }
//    public:
//        ModelAndCategorySet(DgnModelIdSet const* models, DgnCategoryIdSet const* categories)
//            {
//            if (nullptr != models)      m_models = *models;
//            if (nullptr != categories)  m_categories = *categories;
//            }
//
//        bool IsEmpty() const { return m_models.empty() && m_categories.empty(); }
//        };
//protected:
//    ModelAndCategorySet                     m_set;
//    BeSQLite::EC::CachedECSqlStatementPtr   m_stmt;
//
//    DGNPLATFORM_EXPORT virtual bool _AcceptElement(DgnElementId elementId) override;
//public:
//#ifndef VANCOUVER_API
//    DGNPLATFORM_EXPORT TileModelCategoryFilter(DgnDbR dgndb, DgnModelIdSet const* modelIds, DgnCategoryIdSet const* categoryIds);
//#else
//    DGNPLATFORM_EXPORT TileModelCategoryFilter(DgnModelIdSet const* modelIds, DgnCategoryIdSet const* categoryIds);
//#endif
//
//    bool IsEmpty() const { return m_set.IsEmpty(); }
//};

////=======================================================================================
////! Filters elements according to the viewed models and categories associated with a
////! ViewController.
//// @bsistruct                                                   Paul.Connelly   09/16
////=======================================================================================
//struct TileViewControllerFilter : TileModelCategoryFilter
//{
//public:
//    TileViewControllerFilter(ViewControllerCR view) : TileModelCategoryFilter(view.GetDgnDb(), &view.GetViewedModels(), &view.GetViewedCategories()) { }
//};

////=======================================================================================
////! Caches information used during tile generation.
//// @bsistruct                                                   Paul.Connelly   09/16
////=======================================================================================
//struct TileGenerationCache
//{
//    // ###TODO: Put upper limit on sizes of geometry source/list caches...
//    // The following options are mutually exclusive
//    enum class Options
//    {
//        None = 0,               // cache nothin
//        CacheGeometrySources,   // cache GeometrySources by element ID
//        CacheGeometryLists,     // cache TileGeometryLists by element ID
//    };
//private:
//    typedef bmap<DgnElementId, TileGeometryList>                    GeometryListMap;
//    typedef std::map<DgnElementId, std::unique_ptr<GeometrySource>> GeometrySourceMap;
//
//    XYZRangeTreeRoot*           m_tree;
//    mutable GeometryListMap     m_geometry;
//    mutable GeometrySourceMap   m_geometrySources;
//    mutable BeMutex             m_mutex;    // for geometry cache
//    mutable BeSQLite::BeDbMutex m_dbMutex;  // for multi-threaded access to database
//    Options                     m_options;
//
//    friend struct TileGenerator; // Invokes Populate() from ctor
//    TileGenerationCache(Options options = Options::CacheGeometrySources);
//    void Populate(DgnDbR db, ITileGenerationFilterR filter);
//public:
//    DGNPLATFORM_EXPORT ~TileGenerationCache();
//
//    XYZRangeTreeRoot& GetTree() const { return *m_tree; }
//    DGNPLATFORM_EXPORT DRange3d GetRange() const;
//
//    bool WantCacheGeometrySources() const { return Options::CacheGeometrySources == m_options; }
//    GeometrySourceCP GetCachedGeometrySource(DgnElementId elementId) const;
//    GeometrySourceCP AddCachedGeometrySource(std::unique_ptr<GeometrySource>& source, DgnElementId elementId) const;
//
//    bool WantCacheGeometry() const { return Options::CacheGeometryLists == m_options; }
//    bool GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const;
//    void AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const;
//
//    BeSQLite::BeDbMutex& GetDbMutex() const { return m_dbMutex; }
//};

//=======================================================================================
//! Represents one tile in a HLOD tree occupying a given range and containing higher-LOD
//! child tiles within the same range.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileNode : RefCountedBase
{
protected:
    DRange3d            m_dgnRange;
    TileNodeList        m_children;
    size_t              m_depth;
    size_t              m_siblingIndex;
    double              m_tolerance;
    TileNodeP           m_parent;
    WString             m_subdirectory;
    Transform           m_transformFromDgn;
    mutable DRange3d    m_publishedRange;

    TileNode(TransformCR transformFromDgn) : TileNode(DRange3d::NullRange(), transformFromDgn, 0, 0, nullptr) { }
    TileNode(DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : m_dgnRange(range), m_depth(depth), m_siblingIndex(siblingIndex), m_tolerance(tolerance), m_parent(parent), m_transformFromDgn(transformFromDgn), m_publishedRange(DRange3d::NullRange()) { }

    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }

    virtual TileSource _GetSource() const = 0;
    virtual TileMeshList _GenerateMeshes(/*TileGenerationCacheCR cache, DgnDbR dgndb,*/ TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false, bool doPolylines=false) const = 0;
public:
    DRange3dCR GetDgnRange() const { return m_dgnRange; }
    DRange3d GetTileRange() const { DRange3d range = m_dgnRange; m_transformFromDgn.Multiply(range, range); return range; }
    DPoint3d GetTileCenter() const 
        { 
        //#define CESIUM_RTC_ZERO
#ifdef CESIUM_RTC_ZERO
        return DPoint3d::FromXYZ(0, 0, 0);
#endif
        DRange3d range = GetTileRange(); 
        return DPoint3d::FromInterpolate (range.low, .5, range.high);
        }
    size_t GetDepth() const { return m_depth; } //!< This node's depth from the root tile node
    size_t GetSiblingIndex() const { return m_siblingIndex; } //!< This node's order within its siblings at the same depth
    double GetTolerance() const { return m_tolerance; }

    TileNodeCP GetParent() const { return m_parent; } //!< The direct parent of this node
    TileNodeP GetParent() { return m_parent; } //!< The direct parent of this node
    TileNodeList const& GetChildren() const { return m_children; } //!< The direct children of this node
    TileNodeList& GetChildren() { return m_children; } //!< The direct children of this node
    WStringCR GetSubdirectory() const { return m_subdirectory; }
    void SetSubdirectory (WStringCR subdirectory) { m_subdirectory = subdirectory; }
    void SetDgnRange (DRange3dCR range) { m_dgnRange = range; }
    void SetTileRange(DRange3dCR range) { Transform tf; DRange3d dgnRange = range; tf.InverseOf(m_transformFromDgn); tf.Multiply(dgnRange, dgnRange); SetDgnRange(dgnRange); }
    void SetPublishedRange (DRange3dCR publishedRange) const { m_publishedRange = publishedRange; }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }

    size_t GetNodeCount() const;
    size_t GetMaxDepth() const;
    void GetTiles(TileNodePList& tiles);
    TileNodePList GetTiles();
    WString GetNameSuffix() const;
    size_t   GetNameSuffixId() const;
    BeFileNameStatus GenerateSubdirectories (size_t maxTilesPerDirectory, BeFileNameCR dataDirectory);
    WString GetRelativePath (WCharCP rootName, WCharCP extension) const;

    TileSource GetSource() const { return _GetSource(); }
    TileMeshList GenerateMeshes(/*TileGenerationCacheCR cache, DgnDbR dgndb,*/ TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false, bool doPolylines=false) const
        { return _GenerateMeshes(/*cache, dgndb,*/ normalMode, twoSidedTriangles, doPolylines); }
};

////=======================================================================================
////! A TileNode generated from a set of elements.
//// @bsistruct                                                   Paul.Connelly   10/16
////=======================================================================================
//struct ElementTileNode : TileNode
//{
//protected:
//    ElementTileNode(TransformCR transformFromDgn) : TileNode(transformFromDgn) { }
//    ElementTileNode(DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
//        : TileNode(range, transformFromDgn, depth, siblingIndex, parent, tolerance) { }
//
//    bool ExceedsFacetCount(size_t maxFacetCount, TileGenerationCacheCR cache) const;
//
//    virtual TileSource _GetSource() const override final { return TileSource::Element; }
//    DGNPLATFORM_EXPORT virtual TileMeshList _GenerateMeshes(TileGenerationCacheCR, DgnDbR, TileGeometry::NormalMode, bool, bool) const override;
//public:
//    static ElementTileNodePtr Create(TransformCR transformFromDgn) { return new ElementTileNode(transformFromDgn); }
//    static ElementTileNodePtr Create(DRange3dCR dgnRange, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent)
//        { return new ElementTileNode(dgnRange, transformFromDgn, depth, siblingIndex, parent); }
//
//    DGNPLATFORM_EXPORT void ComputeTiles(double chordTolerance, size_t maxPointsPerTile, TileGenerationCacheCR cache);
//    DGNPLATFORM_EXPORT static void ComputeChildTileRanges(bvector<DRange3d>& subTileRanges, DRange3dCR range, size_t splitCount);
//};

//=======================================================================================
//! A TileNode generated from a model implementing IGenerateMeshTiles.
//! This does not include geometry of elements within the model.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct ModelTileNode : TileNode
{
protected:
    ModelTileNode(TransformCR transformFromDgn) : TileNode(transformFromDgn) { }
    ModelTileNode(DRange3dCR range, TransformCR transformFromDgn, size_t depth, size_t siblingIndex, TileNodeP parent, double tolerance = 0.0)
        : TileNode(range, transformFromDgn, depth, siblingIndex, parent, tolerance) { }

    virtual TileSource _GetSource() const override final { return TileSource::Model; }
};

//=======================================================================================
//! Interface adopted by an object which tracks progress of the tile generation process
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ITileGenerationProgressMonitor
{
    enum class TaskName
    {
        PopulatingCache,
        GeneratingTileNodes,
        CollectingTileMeshes,
    };

    virtual void _IndicateProgress(uint32_t completed, uint32_t total) { } //!< Invoked to announce the current ratio completed
    virtual bool _WasAborted() { return false; } //!< Return true to abort tile generation
    virtual void _SetTaskName(TaskName taskName) { } //!< Invoked to announce the current task
    virtual void _SetModel (DgnModelCP dgnModel) { }
};

////=======================================================================================
////! Generates a HLOD tree of TileNodes from a set of tiles.
//// @bsistruct                                                   Paul.Connelly   07/16
////=======================================================================================
//struct TileGenerator
//{
//    enum class Status
//    {
//        Success = SUCCESS,
//        NoGeometry,
//        NotImplemented,
//        Aborted,
//    };
//
//    //! Interface adopted by an object which collects generated tiles
//    struct EXPORT_VTABLE_ATTRIBUTE ITileCollector
//    {
//        //! Invoked from one of several worker threads for each generated tile.
//        virtual Status _AcceptTile(TileNodeCR tileNode) = 0;
//    };
//
//    //! Accumulates statistics during tile generation
//    struct Statistics
//    {
//        size_t      m_tileCount = 0;
//        size_t      m_tileDepth = 0;
//        double      m_collectionTime = 0.0;
//        double      m_tileCreationTime = 0.0;
//        double      m_cachePopulationTime = 0.0;
//    };
//private:
//    Statistics                      m_statistics;
//    //ITileGenerationProgressMonitorR m_progressMeter;
//    Transform                       m_transformFromDgn;
//    //DgnDbR                          m_dgndb;
//    //TileGenerationCache             m_cache;
//
//public:
//    DGNPLATFORM_EXPORT explicit TileGenerator(TransformCR transformFromDgn, DgnDbR dgndb, ITileGenerationFilterP filter=nullptr, ITileGenerationProgressMonitorP progress=nullptr);
//
//    DGNPLATFORM_EXPORT Status CollectTiles(TileNodeR rootTile, ITileCollector& collector);
//
//    DgnDbR GetDgnDb() const { return m_dgndb; }
//    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
//    Statistics const& GetStatistics() const { return m_statistics; }
//    TileGenerationCacheCR GetCache() const { return m_cache; }
//
//    DGNPLATFORM_EXPORT Status GenerateTiles(TileNodePtr& root, size_t maxPointsPerTile);
//};

////=======================================================================================
//// Interface for models to generate HLOD tree of TileNodes 
//// @bsistruct                                                   Ray.Bentley     08/2016
////=======================================================================================
//struct IGenerateMeshTiles
//{
//    virtual TileGenerator::Status _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile) = 0;
//
//};  // IPublishModelMeshTiles

END_BENTLEY_DGNPLATFORM_NAMESPACE

