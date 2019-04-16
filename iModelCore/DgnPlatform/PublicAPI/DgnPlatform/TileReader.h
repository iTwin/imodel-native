/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>

BEGIN_TILETREE_IO_NAMESPACE

//=======================================================================================
//! A read-only view of a GLTF buffer, pointing directly into the buffer data.
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct BufferView
{
    void const*     pData = nullptr;
    size_t          count;
    size_t          byteLength;
    Gltf::DataType  type;
    Json::Value     accessor;

    bool IsValid() const { return nullptr != pData; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
template<Gltf::DataType T, typename UnderlyingType> struct DataTypeConverter
{
    static bool IsConvertible(Gltf::DataType type) { return T == type; }
    static UnderlyingType ConvertFrom(void const* data, size_t index, Gltf::DataType type) { return reinterpret_cast<UnderlyingType const*>(data)[index]; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct DataTypeConverter16
{
    using UnderlyingType = uint16_t;
    static bool IsConvertible(Gltf::DataType type) { return Gltf::DataType::UnsignedShort == type || Gltf::DataType::UnsignedByte == type; }
    static uint16_t ConvertFrom(void const* data, size_t index, Gltf::DataType type)
        {
        return Gltf::DataType::UnsignedShort == type ? reinterpret_cast<uint16_t const*>(data)[index] : reinterpret_cast<uint8_t const*>(data)[index];
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct DataTypeConverter32
{
    using UnderlyingType = uint32_t;
    static bool IsConvertible(Gltf::DataType type) { return Gltf::DataType::UInt32 == type || DataTypeConverter16::IsConvertible(type); }
    static uint32_t ConvertFrom(void const* data, size_t index, Gltf::DataType type)
        {
        return Gltf::DataType::UInt32 == type ? reinterpret_cast<uint32_t const*>(data)[index] : DataTypeConverter16::ConvertFrom(data, index, type);
        }
};

//=======================================================================================
//! A read-only pointer to GLTF data pointing directly into the binary stream.
//! The template parameter specifies the type of the data; the data may be stored
//! in a smaller type, in which case it will be converted to the declared type
//! on retrieval.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
template<typename Converter> struct BufferData
{
    void const*     m_data;
    Gltf::DataType  m_storageType;

    BufferData() : m_data(nullptr) { }
    BufferData(void const* data, Gltf::DataType storageType) : m_storageType(storageType)
        {
        BeAssert(Converter::IsConvertible(storageType));
        m_data = Converter::IsConvertible(storageType) ? data : nullptr;
        }

    bool IsValid() const { return nullptr != m_data; }

    auto operator[](size_t index) const -> typename Converter::UnderlyingType
        {
        BeAssert(IsValid());
        return Converter::ConvertFrom(m_data, index, m_storageType);
        }
};

//! Exposes a GLTF buffer as an array of uint32_t
using BufferData32 = BufferData<DataTypeConverter32>;

//! Exposes a GLTF buffer as an array of uint16_t
using BufferData16 = BufferData<DataTypeConverter16>;

//! Exposes a GLTF buffer as an array of float
using BufferDataFloat = BufferData<DataTypeConverter<Gltf::DataType::Float, float>>;

/*=================================================================================**//**
* Reads GLTF data.
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct GltfReader
{
    StreamBufferR       m_buffer;
    DgnModelR           m_model;
    Json::Value         m_batchData;
    Json::Value         m_materialValues;
    Json::Value         m_accessors;     
    Json::Value         m_bufferViews;
    Json::Value         m_textures;
    Json::Value         m_images;
    Json::Value         m_namedTextures;
    uint8_t const*      m_binaryData;
    Render::System&     m_renderSystem;

    GltfReader(StreamBufferR buffer, DgnModelR model, Render::System& renderSystem) : m_buffer(buffer), m_model(model), m_renderSystem(renderSystem) { }

    void Increment(void const*& in, size_t size) { in = (uint8_t const*) in + size; }
    void CopyAndIncrement(void* out, void const*& in, size_t size)
        {
        memcpy(out, in, size);
        Increment(in, size);
        }

    virtual Render::Primitives::DisplayParamsCPtr _CreateDisplayParams(Json::Value const& materialValue)
        {
        // ###TODO: Create DisplayParams from GLTF Material...
        return Render::Primitives::DisplayParams::Create(Render::Primitives::DisplayParams::Type::Mesh, DgnCategoryId(), DgnSubCategoryId(),
                nullptr, RenderMaterialId(), ColorDef::White(), ColorDef::White(), 0, Render::LinePixels::Solid, Render::FillFlags::ByView,
                Render::DgnGeometryClass::Primary, false, m_model.GetDgnDb(), m_renderSystem, TextureMapping());
        }

    virtual BentleyStatus _ReadFeatures(bvector<uint32_t>& featureIndices, Json::Value const& primitiveValue);
    virtual BentleyStatus _ReadColorTable(Render::Primitives::ColorTableR colorTable, Json::Value const& primitiveValue);

    BentleyStatus GetAccessorAndBufferView(Json::Value& accessor, Json::Value& bufferView, Json::Value const& rootValue, const char* accessorName);
    BentleyStatus GetBufferView (void const*& pData, size_t& count, size_t& byteLength, Gltf::DataType& type, Json::Value& accessor, Json::Value const& primitiveValue, Utf8CP accessorName);
    BufferView GetBufferView(Json::Value const& json, Utf8CP accessorName);

    BufferData32 ReadBufferData32(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount=nullptr);
    BufferData16 ReadBufferData16(Json::Value const& json, Utf8CP accessorName, uint32_t* pCount=nullptr);

    BentleyStatus ReadIndices(bvector<uint32_t>& indices, Json::Value const& primitiveValue, Utf8CP accessorName);
    BentleyStatus ReadMeshIndices(Render::Primitives::MeshR mesh, Json::Value const& primitiveValue);

    BentleyStatus ReadVertexAttributes(bvector<double>& values, Json::Value const& primitiveValue, size_t nComponents, char const* accessorName);
    BentleyStatus ReadVertexBatchIds (bvector<uint16_t>& batchIds, Json::Value const& primitiveValue);
    BentleyStatus ReadVertices(Render::QPoint3dListR vertexList, Json::Value const& primitiveValue);

    BentleyStatus ReadNormalPairs(Render::OctEncodedNormalPairListR pairs, Json::Value const& value, Utf8CP accessorName);
    BentleyStatus ReadNormals(Render::OctEncodedNormalListR normals, Json::Value const& value, Utf8CP accessorName);

    template<typename T_Point> BentleyStatus ReadPoints(bvector<T_Point>& params, Json::Value const& value, Utf8CP accessorName);

    Render::ImageSource GetImageSource(Utf8CP imageName);
    Render::ImageSource GetTextureImageSource(Utf8CP textureName);
    Render::Image GetImage(Utf8CP imageName);
    Render::Image GetTextureImage(Utf8CP textureName);
    Render::TextureMapping GetTextureMapping(Utf8CP textureName, Json::Value const& paramsJson);

    void ReadColors(bvector<uint16_t>& colors, Json::Value const& primitiveValue);
    BentleyStatus ReadColorTable(Render::Primitives::ColorTableR colorTable, Json::Value const& primitiveValue);

    BentleyStatus ReadPolylines(bvector<Render::MeshPolyline>& polylines, Json::Value value, Utf8CP name, bool disjoint);
    Render::MeshEdgesPtr ReadMeshEdges(Json::Value const& primitiveValue);

    BentleyStatus ReadFeatures(Render::Primitives::MeshR mesh, Json::Value const& primitiveValue);

    Render::Primitives::MeshPtr ReadMeshPrimitive(Json::Value const& primitiveValue, Render::FeatureTableP featureTable);

    // Initializes the members m_binaryData, m_materialValues, m_accessors, and m_bufferViews; and returns the JSON representation of the meshes.
    ReadStatus InitGltf(Json::Value& meshValues);

    // Reads gltf data into a GeometryCollection
    ReadStatus ReadGltf(Render::Primitives::GeometryCollectionR geometryCollection);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename T_Channel, typename T_ChannelData, typename T_Point> T_Channel* ReadAuxChannel(JsonValueCR channelValue)
        {
        bvector<RefCountedPtr<T_ChannelData>>     dataVector;

        if (channelValue.isArray())
            {
            for (uint32_t i=0; i<channelValue.size(); i++)
                {
                bvector<T_Point>   values;

                if (SUCCESS == ReadPoints (values, channelValue[i], "values"))
                    dataVector.push_back(new T_ChannelData(channelValue[i]["input"].asFloat(), std::move(values)));
                }
            }
        return dataVector.empty() ? nullptr : new T_Channel(std::move(dataVector));
        }

};

/*=================================================================================**//**
* Reads a b3dm tile.
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct B3dmReader : GltfReader
{
private:
    static ReadStatus ReadFeatureTable(Render::FeatureTableR, Json::Value const& batchTableJson, uint32_t batchTableLength);
public:
    B3dmReader(StreamBufferR buffer, DgnModelR model, Render::System& renderSystem) : GltfReader(buffer, model, renderSystem) { }

    ReadStatus ReadTile(Render::Primitives::GeometryCollectionR);
};

/*=================================================================================**//**
* Reads a Dgn tile.
* @bsiclass                                                     Ray.Bentley     06/2017
+===============+===============+===============+===============+===============+======*/
struct DgnTileReader : GltfReader
{
protected:
    Render::Primitives::DisplayParamsCPtr _CreateDisplayParams(Json::Value const&) override;
    BentleyStatus _ReadFeatures(bvector<uint32_t>& featureIndices, Json::Value const& primitiveValue) override;
    BentleyStatus _ReadColorTable(Render::Primitives::ColorTableR, Json::Value const&) override;

    ReadStatus ReadFeatureTable(Render::FeatureTableR);

public:
    DgnTileReader(StreamBufferR buffer, DgnModelR model, Render::System& system) : GltfReader(buffer, model, system) { }

    ReadStatus ReadTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR, bool& isLeaf);

    bool VerifyFeatureTable();
    bool FindDeletedElements(DgnElementIdSet& elemIds);
    bool GetElements(DgnElementIdSet& deletedOrModified, DgnElementIdSet& unmodified, uint64_t lastModTime);
};

END_TILETREE_IO_NAMESPACE

