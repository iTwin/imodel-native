/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileWriter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/TileIO.h>

BEGIN_TILETREE_IO_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(ICesiumPublisher)
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublishedTile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshMaterial)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileTexture)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileTextureImage)

DEFINE_REF_COUNTED_PTR(TileTexture)
DEFINE_REF_COUNTED_PTR(TileTextureImage)
DEFINE_REF_COUNTED_PTR(PublishedTile)

//=======================================================================================
// ! Holds a texture image.
// @bsistruct                                                   Paul.Connelly   07/16
//=======================================================================================
struct TileTextureImage : RefCountedBase
    {
    private:
        Render::ImageSource     m_imageSource;
        bool                    m_repeat;

        TileTextureImage(Render::ImageSource&& imageSource, bool repeat) : m_imageSource(std::move(imageSource)), m_repeat(repeat) { BeAssert(m_imageSource.IsValid()); }
        TileTextureImage(Render::ImageSource& imageSource, bool repeat) : m_imageSource (imageSource), m_repeat(repeat) { BeAssert(m_imageSource.IsValid()); }
    public:
        static TileTextureImagePtr Create(Render::ImageSource&& imageSource, bool repeat=true) { return new TileTextureImage(std::move(imageSource), repeat); }
        static TileTextureImagePtr Create(Render::ImageSource& imageSource, bool repeat=true) { return new TileTextureImage(imageSource, repeat); }
        static TileTextureImagePtr Create(GradientSymbCR gradient);
        //static Render::ImageSource Load(TileDisplayParamsCR params, DgnDbR db);

        Render::ImageSourceCR GetImageSource() const { return m_imageSource; }
        bool GetRepeat() const { return m_repeat; }
    };

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct TileTexture : Render::Texture
 {
    CreateParams                m_createParams;
    Render::ImageSource         m_imageSource;
    Render::Image::BottomUp     m_bottomUp;
    bool                        m_repeat = false;
    Dimensions                  m_dimensions;
    
    TileTexture(Render::ImageCR image, Texture::CreateParams const& createParams) : Render::Texture(createParams), m_createParams(createParams), 
            m_imageSource(image, Render::Image::Format::Rgba == image.GetFormat() ? Render::ImageSource::Format::Png : Render::ImageSource::Format::Jpeg), m_bottomUp(Render::Image::BottomUp::No), m_dimensions(image.GetWidth(), image.GetHeight())  { }
    TileTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp, Texture::CreateParams const& createParams) : Render::Texture(createParams), m_createParams(createParams), m_imageSource(source), m_bottomUp(bottomUp) { }

    TileTextureImagePtr CreateTexture() const { return TileTextureImage::Create(Render::ImageSource(m_imageSource), !m_createParams.m_isTileSection); }
    virtual Dimensions GetDimensions() const override { return m_dimensions; }

 }; // Texture


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016                                                     
//=======================================================================================
struct Writer
{
public:
    Writer(StreamBufferR buffer, GeometricModelR model) : m_buffer(buffer), m_model(model) { }
    BentleyStatus WriteGltf();
    void                AddPrimitivesJson(Json::Value const& primitives);


protected:
    Json::Value         m_json;
    ByteStream          m_binaryData;
    StreamBufferR       m_buffer;
    GeometricModelCR    m_model;

    size_t              BinaryDataSize() const { return m_binaryData.size(); }
    void const*         BinaryData() const;
    void                WriteLength(uint32_t startPosition, uint32_t lengthDataPosition);
    void                AddBinaryData (void const* data, size_t size); 
    void                PadBinaryDataToBoundary(size_t boundarySize = 4);
    void                PadToBoundary(size_t boundarySize = 4);
    void                AddExtensions();
    void                AddDefaultScene ();
    void                AddMeshUInt16Attributes(Json::Value& primitive, uint16_t const* attributes16, size_t nAttributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void                AddBatchIds(Json::Value& primitive, Render::FeatureIndex const& featureIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddColors(Json::Value& primitive, Render::ColorIndex const& colorIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddAccessor(Gltf::DataType componentType, Utf8StringCR accessorId, Utf8StringCR bufferViewId, size_t count, Utf8CP type);
    Utf8String          AddQuantizedPointsAttribute(Render::QPoint3dCP qPoints, size_t nPoints, Render::QPoint3d::Params params, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddQuantizedParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddPointAttribute(FPoint3d const* points, size_t nPoints, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddMeshIndices(Utf8StringCR name, uint32_t const* indices, size_t numIndices, Utf8StringCR idStr, size_t maxIndex);
    Utf8String          AddMeshTriangleIndices(Utf8StringCR name, Render::Primitives::TriangleList const& triangles, Utf8StringCR idStr, size_t maxIndex);
    void                AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange);
    BentleyStatus       CreateMeshMaterialJson(Json::Value& matJson, Render::Primitives::ColorTableCR colorTable, MeshMaterialCR meshMaterial, Utf8StringCR suffix); 
    Json::Value         AddNormals (Render::OctEncodedNormalCP normals, size_t numNormals, Utf8String name, Utf8CP id);
    Json::Value         AddNormalPairs(Render::OctEncodedNormalPairCP pairs, size_t numPairs, Utf8String name, Utf8CP id);
    void                AppendPolylineToBufferView(Render::MeshPolylineCR polyline, bool useShortIndices);
    Json::Value         AddPolylines(Render::Primitives::PolylineList const& polylines, size_t maxIndex, Utf8StringCR name, Utf8StringCR idStr); 

    static Json::Value  CreateColorJson(RgbFactorCR color);
    static Json::Value  CreateDecodeQuantizeValues(double const* min, double const* max, size_t nComponents);


    template<typename T> void AddBufferView(Utf8CP name, T const* bufferData, size_t count)
        {
        Json::Value&    views = m_json["bufferViews"];

        auto bufferDataSize = count * sizeof(*bufferData);
        auto& view = (views[name] = Json::objectValue);
        view["buffer"] = "binary_glTF";
        view["byteOffset"] = static_cast<uint32_t>(m_binaryData.size());
        view["byteLength"] = static_cast<uint32_t>(bufferDataSize);

        size_t binaryDataSize = m_binaryData.size();
        m_binaryData.resize(binaryDataSize + bufferDataSize);
        memcpy(m_binaryData.data() + binaryDataSize, bufferData, bufferDataSize);
        PadBinaryDataToBoundary();
        }

    template<typename T> void AddBufferView(Utf8CP name, T const& bufferData)
        {
        AddBufferView (name, bufferData.data(), bufferData.size());
        }
};  // Writer


//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! A node in the published tileset tree.  
//! This tree is used to construct the tileset metadata.
//=======================================================================================
struct PublishedTile : RefCountedBase
    {
private:
    BeFileName                  m_outputDirectory;
    Utf8String                  m_name;
    Utf8String                  m_extension;
    bvector<PublishedTilePtr>   m_children;
    DRange3d                    m_publishedRange;
    DRange3d                    m_tileRange;
    double                      m_tolerance;

public:
    PublishedTile(TileTree::TileCR inputTile, BeFileNameCR outputDirectory);
    bvector<PublishedTilePtr>& GetChildren() { return m_children; }
    bvector<PublishedTilePtr>const& GetChildren() const { return m_children; }
    void SetPublishedRange(DRange3dCR range) { m_publishedRange = range; }
    bool GetIsEmpty() const { return m_publishedRange.IsNull(); }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    double GetTolerance() const { return m_tolerance; }
    void SetExtension(CharCP extension) { m_extension = extension; }
    Utf8String GetURL() const;
    BeFileName GetFileName() const;
    };

//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! Interface adopted by an object that publishes a cesium tileset to represent the
//! a GeometricModel.
//=======================================================================================
struct ICesiumPublisher
{
    //! Returns directory for tileset.
    virtual BeFileName  _GetOutputDirectory(GeometricModelCR model) const = 0;
    //! Invoked before a model is processed.
    virtual TileTree::IO::WriteStatus _BeginProcessModel(GeometricModelCR model) { return TileTree::IO::WriteStatus::Success; }
    //! Invoked after a model is processed, with the result of processing.
    virtual TileTree::IO::WriteStatus _EndProcessModel(GeometricModelCR model, TransformCR location, PublishedTileCR rootTile, TileTree::IO::WriteStatus status) = 0;
    //! Write cesium tileset for a GeometricModel
    DGNPLATFORM_EXPORT static TileTree::IO::WriteStatus PublishCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);
    DGNPLATFORM_EXPORT static TileTree::IO::WriteStatus WriteCesiumTileset(BeFileName outputFileName, BeFileNameCR tileOutputDirectory, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);


};  // ICesiumPublisher




END_TILETREE_IO_NAMESPACE







                                                                                                           

























