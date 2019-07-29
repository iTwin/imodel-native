/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>

/* ###TODO Currently we do not use the "distance" attribute associated with polylines.
 * Ray suggests in the future, we may introduce non-cosmetic line styles, which would make use of this attribute.
 * Until then, we avoid including them in the tile content by omitting this #define
 * #define INCLUDE_POLYLINE_DISTANCE
 */

USING_NAMESPACE_TILE_IO
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

BEGIN_UNNAMED_NAMESPACE

struct PolylineEdgeParams;
struct PolylineParams;

using PolylineEdgeParamsUPtr = std::unique_ptr<PolylineEdgeParams>;
using PolylineParamsUPtr = std::unique_ptr<PolylineParams>;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/17
//=======================================================================================
enum class SurfaceType : uint8_t
{
    Unlit,
    Lit,
    Textured,
    TexturedLit,
    Classifier, // treated as Unlit; handled specially on front-end
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wantJointTriangles(uint32_t lineWeight, bool is2d)
    {
    return Render::Primitives::Geometry::WantPolylineEdges(lineWeight, !is2d);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct LUTDimensions
{
private:
    uint32_t    m_width;
    uint32_t    m_height;

    LUTDimensions(uint32_t w, uint32_t h) : m_width(w), m_height(h) { }
public:
    LUTDimensions() { }
    void Init(uint32_t nEntries, uint32_t nRgbaPerEntry, uint32_t nExtraRgba=0, uint32_t nTables = 1);
    static LUTDimensions Compute(uint32_t nEntries, uint32_t nRgbaPerEntry, uint32_t nExtraRgba=0, uint32_t nTables = 1)
        {
        LUTDimensions dims;
        dims.Init(nEntries, nRgbaPerEntry, nExtraRgba, nTables);
        return dims;
        }

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct LUTVertex
{
    QPoint3d    m_position;             // 0x00
    uint16_t    m_colorIndexOrNormal;   // 0x06
    uint32_t    m_featureIndex;         // 0x08
protected:
    template<typename T> static QPoint3dCR GetPosition(T const& args, uint32_t idx) { return args.m_points[idx]; }
    template<typename T> static uint16_t GetColorIndex(T const& args, uint32_t idx)
        {
        return args.m_colors.IsUniform() ? 0 : args.m_colors.m_nonUniform.m_indices[idx];
        }
    template<typename T> static uint32_t GetFeatureIndex(T const& args, uint32_t idx)
        {
        return args.m_features.IsNonUniform() ? args.m_features.m_featureIDs[idx] : args.m_features.m_featureID;
        }

    LUTVertex(QPoint3dCR pos, uint16_t colorIndexOrNormal, uint32_t feature) : m_position(pos), m_colorIndexOrNormal(colorIndexOrNormal), m_featureIndex(feature) { }
    template<typename Args> LUTVertex(Args const& args, uint32_t idx)
        : LUTVertex(GetPosition(args, idx), GetColorIndex(args, idx), GetFeatureIndex(args, idx)) { }
};
static_assert(0x0C == sizeof(LUTVertex), "unexpected size");
static_assert(0 == (sizeof(LUTVertex) % 4), "unexpected size");

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct LinearVertex : LUTVertex
{
    template<typename Args> LinearVertex(Args const& args, uint32_t idx, int ignored) : LUTVertex(args, idx) { }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct MeshVertex : LUTVertex
{
    MeshVertex(TriMeshArgsCR args, uint32_t idx, QPoint2d::ParamsCR uvParams) : LUTVertex(args, idx)
        {
        if (nullptr != args.m_material.m_indices)
            {
            uint32_t materialIndex = args.m_material.m_indices[idx] << 24;
            m_featureIndex |= materialIndex;
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct TexturedMeshVertex : MeshVertex
{
    QPoint2d    m_textureUV;    // 0x0C

    TexturedMeshVertex(TriMeshArgsCR args, uint32_t idx, QPoint2d::ParamsCR uvParams)
        : MeshVertex(args, idx, uvParams)
        {
        m_textureUV = QPoint2d(args.m_textureUV[idx], uvParams);
        }
};
static_assert(0x10 == sizeof(TexturedMeshVertex), "unexpected size");
static_assert(0 == (sizeof(TexturedMeshVertex) % 4), "unexpected size");

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct TexturedLitMeshVertex : TexturedMeshVertex
{
    TexturedLitMeshVertex(TriMeshArgsCR args, uint32_t idx, QPoint2d::ParamsCR uvParams)
        : TexturedMeshVertex(args, idx, uvParams)
        {
        m_colorIndexOrNormal = args.m_normals[idx].Value();
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct LitMeshVertex : MeshVertex
{
    uint16_t    m_normal;   // 0x0C
    uint16_t    m_unused;   // 0x0E

    LitMeshVertex(TriMeshArgsCR args, uint32_t idx, QPoint2d::ParamsCR uvParams)
        : MeshVertex(args, idx, uvParams), m_normal(args.m_normals[idx].Value()), m_unused(0) { }
};
static_assert(0x10 == sizeof(LitMeshVertex), "unexpected size");
static_assert(0 == (sizeof(LitMeshVertex) % 4), "unexpected size");

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/17
//=======================================================================================
struct FeaturesInfo
{
    uint32_t            m_uniform;
    FeatureIndex::Type  m_type;

    FeaturesInfo() : m_type(FeatureIndex::Type::Empty) { }
    FeaturesInfo(FeatureIndex::Type type, uint32_t uniform) : m_uniform(uniform), m_type(type) { }
    explicit FeaturesInfo(FeatureIndex const& src) : FeaturesInfo(src.m_type, src.m_featureID) { }

    bool IsEmpty() const { return FeatureIndex::Type::Empty == m_type; }
    bool IsUniform() const { return FeatureIndex::Type::Uniform == m_type; }
    bool IsNonUniform() const { return FeatureIndex::Type::NonUniform == m_type; }

    void SetUniform(uint32_t uniform) { m_type=FeatureIndex::Type::Uniform; m_uniform=uniform; }
    void Clear() { m_type = FeatureIndex::Type::Empty; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct ColorInfo
{
    ColorDef    m_uniform;
    uint16_t    m_numColors;
    bool        m_isUniform;
    bool        m_hasAlpha;

    ColorInfo() : m_isUniform(false), m_hasAlpha(false), m_numColors(0) { }

    explicit ColorInfo(ColorIndex const& colorIndex) : m_isUniform(colorIndex.IsUniform())
        {
        if (m_isUniform)
            {
            m_numColors = 0;
            m_uniform = ColorDef(colorIndex.m_uniform);
            m_hasAlpha = 0 != m_uniform.GetAlpha();
            }
        else
            {
            m_hasAlpha = colorIndex.m_nonUniform.m_hasAlpha;
            m_numColors = colorIndex.m_numColors;
            }
        }
};

//=======================================================================================
//! Holds vertex data (position, color index, normal, UV params, etc) in a texture.
//! Color table is appended to the end of this data.
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct VertexTable
{
private:
     void AppendColor(uint32_t colorValue)
        {
        ColorDef color(colorValue);
        uint8_t a = 255 - color.GetAlpha();
        color.SetAlpha(a); // for case transparency=0, alpha=255...
        switch (a)
            {
            case 0:
                *reinterpret_cast<uint32_t*>(m_pDataEnd) = 0;
                break;
            case 255:
                *reinterpret_cast<uint32_t*>(m_pDataEnd) = color.GetValue();
                break;
            default:
                {
                double alpha = a / 255.0;
                m_pDataEnd[0] = static_cast<uint8_t>(color.GetRed() * alpha + 0.5);
                m_pDataEnd[1] = static_cast<uint8_t>(color.GetGreen() * alpha + 0.5);
                m_pDataEnd[2] = static_cast<uint8_t>(color.GetBlue() * alpha + 0.5);
                m_pDataEnd[3] = a;
                break;
                }
            }

        m_pDataEnd += 4;
        }

    uint8_t*        m_pDataEnd = nullptr;

public:
    ByteStream              m_data;
    LUTDimensions           m_dimensions;
    uint32_t                m_numVertices = 0;
    uint32_t                m_numRgbaPerVertex = 0;
    FeaturesInfo            m_features;
    ColorInfo               m_colors;
    MaterialAtlasPtr        m_materialAtlas;

    uint32_t CurrSize() const { return static_cast<uint32_t> (m_pDataEnd - m_data.GetDataP()); }

    void AppendData(void const* pData, size_t dataSize)
        {
        if (0 != dataSize)
            {
            memcpy(m_pDataEnd, pData, dataSize);
            m_pDataEnd += dataSize;
            }
        }

    void AppendUInt32(uint32_t value)
        {
        AppendData(&value, sizeof(value));
        }

    template<typename T_Vertex, typename T_Args, typename T_ExtraData> void Init(T_Args const& args, T_ExtraData const& extraData, uint32_t nExtraRgba = 0)
        {
        m_features = FeaturesInfo(args.m_features);
        m_colors = ColorInfo(args.m_colors);

        uint32_t nVerts = args.m_numPoints;
        m_numVertices = nVerts;
        uint32_t nBytesPerVert = sizeof(T_Vertex);
        uint32_t nRgbaPerVert = nBytesPerVert / 4;
        m_numRgbaPerVertex = nRgbaPerVert;

        ColorIndex const& colorIndex = args.m_colors;
        uint32_t nColors = colorIndex.IsUniform() ? 0 : colorIndex.m_numColors;

        m_dimensions.Init(nVerts, nRgbaPerVert, nColors + nExtraRgba);
        BeAssert(0 == m_dimensions.GetWidth() % nRgbaPerVert || (0 < nColors && 1 == m_dimensions.GetHeight()));

        m_data = ByteStream(m_dimensions.GetWidth() * m_dimensions.GetHeight() * 4);
        m_pDataEnd = m_data.GetDataP();
        uint32_t vertIndex = 0;
        while (vertIndex < nVerts)
            {
            T_Vertex vertex(args, vertIndex++, extraData);
            AppendData(&vertex, sizeof(vertex));
            }

        BeAssert(m_data.size() >= sizeof(T_Vertex) * nVerts + 4 * nColors);
        if (!colorIndex.IsUniform())
            {
            for (uint32_t i = 0; i < nColors; i++)
                AppendColor(colorIndex.m_nonUniform.m_colors[i]);
            }
        }

    template<typename T_Vertex> void InitMesh(TriMeshArgsCR args, QPoint2d::ParamsCR uvParams)
        {
        m_materialAtlas = args.m_material.m_atlas;
        Init<T_Vertex>(args, uvParams, m_materialAtlas.IsValid() ? m_materialAtlas->NumMaterials() * 4 : 0);

        if (m_materialAtlas.IsValid())
            {
            auto materials = m_materialAtlas->ToList();
            for (auto const& material : materials)
                AppendMaterial(material);
            }
        }

    explicit VertexTable() { }

    void AppendMaterial(Render::Material::CreateParams const& mat);
};

//=======================================================================================
//! A 32-bit floating point value encoded into 4 bytes.
// @bsistruct                                                   Paul.Connelly   06/18
//=======================================================================================
struct PackedFloat
{
    union Repr
    {
        uint32_t    m_value;
        uint8_t     m_bytes[4];
    };

    Repr    m_repr;

    static constexpr float Bias() { return 38.0; }

    explicit PackedFloat(float value) { m_repr = Pack(value); }
    explicit PackedFloat (double value) : PackedFloat(static_cast<float>(value)) { }

    static Repr Pack(float value);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
PackedFloat::Repr PackedFloat::Pack(float value)
    {
    Repr repr;

    if (0.0 == value)
        {
        repr.m_value = 0;
        return repr;
        }

    float sign = value < 0.0f ? 1.0f : 0.0f;
    value = std::fabs(value);
    float exponent = floor(log10(value)) + 1.0f;
    value = value / pow(10.0f, exponent);

    float temp = value * 256.0f;
    repr.m_bytes[0] = static_cast<uint8_t>(floor(temp));
    temp = (temp - repr.m_bytes[0]) * 256.0f;
    repr.m_bytes[1] = static_cast<uint8_t>(floor(temp));
    temp = (temp - repr.m_bytes[1]) * 256.0f;
    repr.m_bytes[2] = static_cast<uint8_t>(floor(temp));
    repr.m_bytes[3] = static_cast<uint8_t>((exponent + Bias()) * 2.0f + sign);

    return repr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
static uint8_t scaleWeight(double f)
    {
    if (f < 0.0)
        f = 0.0;
    else if (f > 1.0)
        f = 1.0;

    return static_cast<uint8_t>(f * 255 + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void VertexTable::AppendMaterial(Render::Material::CreateParams const& params)
    {
    // diffuse rgb; alpha
    auto rgbOverridden = params.m_diffuseColor.IsValid();
    auto diffuseColor = rgbOverridden ? params.m_diffuseColor.m_value : ColorDef::White();

    auto alphaOverridden = params.m_transparency.IsOverridden();
    uint8_t alpha = alphaOverridden ? 255 - scaleWeight(params.m_transparency.GetValue()) : 255;
    diffuseColor.SetAlpha(alpha);

    AppendUInt32(diffuseColor.GetValue());

    // unused, diffuse weight, specular weight, flags (1 = rgb overridden, 2 = alpha overridden)
    // NB: Texture weight is not represented in atlas because textured materials never end up in an atlas.
    uint8_t flags = (rgbOverridden ? 1 : 0) | (alphaOverridden ? 2 : 0);

    ColorDef weightsAndFlags(0, scaleWeight(params.m_diffuse), scaleWeight(params.m_specular), flags);
    AppendUInt32(weightsAndFlags.GetValue());

    // specular rgb; unused
    auto specularColor = params.m_specularColor.IsValid() ? params.m_specularColor.m_value : ColorDef::White();
    specularColor.SetAlpha(0);
    AppendUInt32(specularColor.GetValue());

    // specular exponent (packed float)
    PackedFloat specularExponent(params.m_specularExponent);
    AppendUInt32(specularExponent.m_repr.m_value);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
struct AuxChannelTable
{
private:
    void AddNormals(PolyfaceAuxChannelCR channel, uint32_t byteOffset);
    void AddParams(PolyfaceAuxChannelCR channel, uint32_t byteOffset)
        {
        m_params.append(AddChannel<DRange1d, double, QPoint1d>(channel, byteOffset));
        }
    void AddDisplacements(PolyfaceAuxChannelCR channel, uint32_t byteOffset)
        {
        m_displacements.append(AddChannel<DRange3d, DPoint3d, QPoint3d>(channel, byteOffset));
        }

    template<typename Range, typename DPoint, typename QPoint> Json::Value AddChannel(PolyfaceAuxChannelCR, uint32_t byteOffset);
    template<typename T> void SetData(uint32_t byteIndex, T const& data)
        {
        BeAssert(byteIndex + sizeof(data) <= m_data.size());
        memcpy(m_data.GetDataP() + byteIndex, &data, sizeof(data));
        }
public:
    ByteStream      m_data;
    LUTDimensions   m_dimensions;
    Json::Value     m_displacements;
    Json::Value     m_normals;
    Json::Value     m_params;
    uint32_t        m_numVertices = 0;
    uint32_t        m_numBytesPerVertex = 0;

    void Init(TriMeshArgsCR args);

    bool IsValid() const { return !m_data.empty(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t computeNumBytesPerVertex(PolyfaceAuxChannelCR channel)
    {
    auto nEntries = static_cast<uint32_t>(channel.GetData().size());
    switch (channel.GetDataType())
        {
        case PolyfaceAuxChannel::DataType::Vector:
        case PolyfaceAuxChannel::DataType::Point:
            return 6 * nEntries;
        case PolyfaceAuxChannel::DataType::Scalar:
        case PolyfaceAuxChannel::DataType::Distance:
        case PolyfaceAuxChannel::DataType::Covector:
            return 2 * nEntries;
        default:
            BeAssert(false);
            return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxChannelTable::Init(TriMeshArgsCR args)
    {
    // Scalars and vectors get quantized to 16-bits per component.
    // Normals get oct-encoded to 16-bits.
    // We put each vertex's channel in-line. So for example, if there are 2 channels - a displacement and a normal - we produce a table like:
    // [x0 y0] [z0 n0] [x1 y1] [z1 n1] ...
    // where each [xx xx] represents a 4-byte RGBA entry in the table.
    // We look up a channel's data using 2-byte-aligned indices - so index 0 refers to the RG component of the first RGBA entry, index 1 to the BA component of same, etc.
    // We ensure that each vertex's data fits in the same row of the table so that we can simply add to the x component of the texture coord in order to find the next RGBA value.

    for (auto const& channel : args.m_auxChannels)
        m_numBytesPerVertex += computeNumBytesPerVertex(*channel);

    if (0 == m_numBytesPerVertex)
        return;

    uint32_t nRgbaPerVertex = (m_numBytesPerVertex + 3) / 4;
    uint32_t nUnusedBytesPerVertex = nRgbaPerVertex * 4 - m_numBytesPerVertex;
    BeAssert(0 == nUnusedBytesPerVertex || 2 == nUnusedBytesPerVertex);

    // We don't want any unused bytes - if we've got 2 extra, make every other vertex's channel start in the middle of the first texel.
    // (Lie to LUTDimensions::Init() to make it compute this for us).
    m_numVertices = args.m_numPoints;
    if (0 != nUnusedBytesPerVertex)
        m_dimensions.Init((m_numVertices + 1) / 2, m_numBytesPerVertex / 2); // twice as many RGBA for half as many vertices...
    else
        m_dimensions.Init(m_numVertices, nRgbaPerVertex);

    m_data = ByteStream(m_dimensions.GetWidth() * m_dimensions.GetHeight() * 4);

    uint32_t byteOffset = 0;
    for (auto const& channel : args.m_auxChannels)
        {
        auto channelType = channel->GetDataType();
        switch (channelType)
            {
            case PolyfaceAuxChannel::DataType::Covector:
                AddNormals(*channel, byteOffset);
                break;
            case PolyfaceAuxChannel::DataType::Vector:
            case PolyfaceAuxChannel::DataType::Point:
                AddDisplacements(*channel, byteOffset);
                break;
            case PolyfaceAuxChannel::DataType::Scalar:
            case PolyfaceAuxChannel::DataType::Distance:
                AddParams(*channel, byteOffset);
                break;
            default:
                BeAssert(false);
                break;
            }

        byteOffset += computeNumBytesPerVertex(*channel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxChannelTable::AddNormals(PolyfaceAuxChannelCR channel, uint32_t byteOffset)
    {
    Json::Value inputs(Json::arrayValue);
    Json::Value indices(Json::arrayValue);

    for (uint32_t i = 0; i < channel.GetData().size(); i++)
        {
        uint32_t byteIndex = byteOffset + i * 2; // 2 bytes per normal
        indices.append(byteIndex / 2); // indices aligned to 2-byte intervals

        auto data = channel.GetData()[i];
        inputs.append(data->GetInput());

        double const* values = data->GetValues().data();
        for (size_t j = 0; j < data->GetValues().size(); j += 3)
            {
            DVec3d normal = *reinterpret_cast<DVec3d const*>(values + j);
            normal.Normalize();
            uint16_t encodedNormal = OctEncodedNormal::From(normal).Value();

            SetData(byteIndex, encodedNormal);
            byteIndex += m_numBytesPerVertex;
            }
        }

    Json::Value json(Json::objectValue);
    json["name"] = channel.GetName();
    json["type"] = static_cast<int>(channel.GetDataType());
    json["inputs"] = std::move(inputs);
    json["indices"] = std::move(indices);

    // NB: Json::Value::append() will turn itself into an arrayValue if it's currently nullValue...
    m_normals.append(std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Range, typename DPoint, typename QPoint> Json::Value AuxChannelTable::AddChannel(PolyfaceAuxChannelCR channel, uint32_t byteOffset)
    {
    Json::Value json(Json::objectValue);
    json["name"] = channel.GetName();
    json["type"] = static_cast<int>(channel.GetDataType());

    Json::Value inputs(Json::arrayValue);

    uint32_t blockSize = static_cast<uint32_t>(channel.GetBlockSize());
    Range range = Range::NullRange();
    for (auto const& data : channel.GetData())
        {
        inputs.append(data->GetInput());
        double const* value = data->GetValues().data();
        for (size_t i = 0; i < data->GetValues().size(); i += blockSize)
            {
            auto point = reinterpret_cast<DPoint const*>(value + i);
            range.Extend(*point);
            }
        }

    json["inputs"] = std::move(inputs);

    typename QPoint::Params qparams(range);

    Json::Value originJson(Json::arrayValue);
    Json::Value scaleJson(Json::arrayValue);
    auto pOrigin = reinterpret_cast<double const*>(&qparams.origin);
    auto pScale = reinterpret_cast<double const*>(&qparams.scale);
    for (size_t i = 0; i < blockSize; i++)
        {
        originJson.append(pOrigin[i]);
        double scale = pScale[i];
        scaleJson.append(0.0 == scale ? 0.0 : (1.0 / scale));
        }

    json["qOrigin"] = std::move(originJson);
    json["qScale"] = std::move(scaleJson);

    Json::Value indices(Json::arrayValue);
    for (uint32_t i = 0; i < channel.GetData().size(); i++)
        {
        uint32_t byteIndex = byteOffset + i * (2 * blockSize); // 2 bytes per double
        indices.append(byteIndex / 2); // indices aligned to 2-byte intervals

        auto data = channel.GetData()[i];
        double const* value = data->GetValues().data();
        for (size_t j = 0; j < data->GetValues().size(); j += blockSize)
            {
            auto dpoint = reinterpret_cast<DPoint const*>(value + j);
            QPoint qpoint(*dpoint, qparams);

            SetData(byteIndex, qpoint);
            byteIndex += m_numBytesPerVertex;
            }
        }

    json["indices"] = std::move(indices);
    return json;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/17
//=======================================================================================
struct MeshParams
{
private:
    QPoint2d::Params InitUVParams(TriMeshArgsCR args);

    template<typename VertexType> void Init(TriMeshArgsCR args)
        {
        m_uvParams = InitUVParams(args);
        m_lutParams.InitMesh<VertexType>(args, m_uvParams);
        m_auxChannels.Init(args);
        }
public:
    VertexTable                     m_lutParams;
    QPoint3d::Params                m_vertexParams;
    QPoint2d::Params                m_uvParams;
    AuxChannelTable                 m_auxChannels;
    SurfaceType                     m_type;

    MeshParams(TriMeshArgsCR, bool isVolumeClassifier);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/17
//=======================================================================================
struct SurfaceParams
{
    Render::TexturePtr  m_texture;
    ByteStream          m_vertexIndices;

    explicit SurfaceParams(TriMeshArgsCR args) : m_texture(args.m_texture), m_vertexIndices(args.m_numIndices * 3)
        {
        // In shader we have less than 32 bits precision...
        BeAssert((uint32_t)((float)args.m_numPoints) == args.m_numPoints && "Max index range exceeded");
        uint8_t* pData = m_vertexIndices.GetDataP();
        for (size_t i = 0; i < args.m_numIndices; i++)
            {
            memcpy(pData, args.m_vertIndex+i, 3);
            pData += 3;
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct EdgeParams
{
    ByteStream  m_vertexIndices;            // vec3 (unsigned byte) holding index of this vertex in LUT
    ByteStream  m_endPointAndQuadIndices;   // vec4 (unsigned byte): xyz=index of other vertex in LUT; w=index of this vertex within quad in [0,3]
    uint32_t    m_vertexIndicesCount;

    explicit EdgeParams(EdgeArgsCR args);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct SilhouetteParams : EdgeParams
{
    ByteStream  m_normalPairs;  // vec4 oct-encoded normal pairs

    explicit SilhouetteParams(SilhouetteEdgeArgsCR args) : EdgeParams(args)
        {
        m_normalPairs.resize(m_endPointAndQuadIndices.size());
        uint8_t* pNormals = m_normalPairs.GetDataP();
        for (uint32_t i = 0; i < args.m_numEdges; i++)
            {
            OctEncodedNormalPair pair = args.m_normals[i];
            OctEncodedNormalPair pairs[6] = { pair, pair, pair, pair, pair, pair };
            memcpy(pNormals, pairs, sizeof(pairs));
            pNormals += sizeof(pairs);
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct TesselatedPolyline
{
    friend struct PolylineTesselator;
private:
    explicit TesselatedPolyline() { }
public:
    enum Param : uint8_t
    {
        kNone = 0,
        kSquare = 1*3,
        kMiter = 2*3,
        kMiterInsideOnly = 3*3,
        kJointBase = 4*3,
        kNegatePerp = 8*3,
        kNegateAlong = 16*3,
        kNoneAdjWt = 32*3,
    };

    struct PosIndex
    {
        uint8_t m_bytes[3];

        PosIndex() { }
        PosIndex(uint32_t index)
            {
            BeAssert(0 == (index & 0xff000000));
            memcpy(m_bytes, &index, 3);
            }

        uint32_t ToIndex() const { return m_bytes[0] | (m_bytes[1] << 8) || (m_bytes[2] << 16); }
    };

    struct PosIndexAndParam : PosIndex
    {
        Param   m_param;

        PosIndexAndParam() { }
        PosIndexAndParam(uint32_t index, Param param) : PosIndex(index), m_param(param) { }
    };

    bvector<PosIndex>           m_vertIndex;        // index into LUT for this vertex's data
    bvector<PosIndex>           m_prevIndex;        // index into LUT for prev vertex's position
    bvector<PosIndexAndParam>   m_nextIndexAndParam;// index into LUT for next vertex, interleaved with this vertex's param
    bvector<float>              m_distance;         // distance of this vertex along polyline

    bool IsValid() const { return !m_vertIndex.empty(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct PolylineTesselator
{
    using Param = TesselatedPolyline::Param;
    using Polyline = IndexedPolylineArgs::Polyline;

    struct Vertex
    {
        uint32_t            m_vertIndex;
        uint32_t            m_prevIndex;
        uint32_t            m_nextIndex;
        double              m_length;
        bool                m_isSegmentStart;
        bool                m_isPolylineStartOrEnd;

        Vertex(bool isSegmentStart, bool isPolylineStartOrEnd, uint32_t vertIndex, uint32_t prevIndex, uint32_t nextIndex, double length)
            : m_vertIndex(vertIndex), m_prevIndex(prevIndex), m_nextIndex(nextIndex), m_length(length),
              m_isSegmentStart(isSegmentStart), m_isPolylineStartOrEnd(isPolylineStartOrEnd) { }

        Param ComputeParam(bool negatePerp, bool adjacentToJoint=false, bool joint=false, bool noDisplacement=false) const;
    };
private:
    TesselatedPolyline  m_polyline;
    Polyline const*     m_lines;
    QPoint3dCP          m_points;
    QPoint3d::Params    m_pointParams;
    uint32_t            m_numLines;
    bool                m_doJointTriangles;

    explicit PolylineTesselator(IndexedPolylineArgsCR args);
    explicit PolylineTesselator(TriMeshArgsCR args);
    PolylineTesselator(Polyline const* lines, uint32_t numLines, QPoint3dCP points, uint32_t numPoints, bool doJointTriangles, QPoint3d::ParamsCR pointParams)
        : m_lines(lines), m_points(points), m_numLines(numLines), m_doJointTriangles(doJointTriangles), m_pointParams(pointParams) { }

    void Tesselate();

    DPoint3d GetPosition(uint32_t index) const { return m_points[index].Unquantize(m_pointParams); }
    double DotProduct(Vertex const& vertex) const;
    void AddSimpleSegment(Vertex const& v0, Vertex const& v1);
    void AddJoints(Vertex const& v0, Vertex const& v1, bool jointAt0, bool jointAt1);
    void AddJointTriangles(Vertex const& v0, Param p0, Vertex const& v1);
    void AddVertex(Vertex const& vertex, Param param);
public:
    template<typename T> static TesselatedPolyline Tesselate(T const& args)
        {
        PolylineTesselator tesselator(args);
        tesselator.Tesselate();
        return tesselator.m_polyline;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct PolylineEdgeParams
{
    TesselatedPolyline  m_polyline;

private:
    explicit PolylineEdgeParams(TesselatedPolyline&& polyline) : m_polyline(std::move(polyline)) { }
public:
    static PolylineEdgeParamsUPtr Create(TriMeshArgsCR args);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct PolylineParams
{
    VertexTable         m_lutParams;
    TesselatedPolyline  m_polyline;
    QPoint3d::Params    m_vertexParams;
private:
    PolylineParams(IndexedPolylineArgsCR args, TesselatedPolyline&& polyline);
public:
    static PolylineParamsUPtr Create(IndexedPolylineArgsCR args);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct PointStringParams
{
    VertexTable         m_lutParams;
    ByteStream          m_vertexIndices;
    QPoint3d::Params    m_vertexParams;

    explicit PointStringParams(IndexedPolylineArgsCR args) : m_vertexParams(args.m_pointParams)
        {
        m_lutParams.Init<LinearVertex>(args, 0);
        uint32_t nVertices = 0;
        for (uint32_t lineIdx = 0; lineIdx < args.m_numLines; lineIdx++)
            nVertices += args.m_lines[lineIdx].m_numIndices;

        m_vertexIndices.resize(nVertices*3);
        uint8_t* pData = m_vertexIndices.GetDataP();
        for (uint32_t lineIdx = 0; lineIdx < args.m_numLines; lineIdx++)
            {
            auto const& line = args.m_lines[lineIdx];
            for (uint32_t i = 0; i < line.m_numIndices; i++)
                {
                memcpy(pData, line.m_vertIndex+i, 3);
                pData += 3;
                }
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct IModelTileWriter : Tile::IO::Writer
{
    DEFINE_T_SUPER(Tile::IO::Writer);
private:
    Tile::LoaderCR  m_loader;

    bool IsCanceled() const { return m_loader.IsCanceled(); }

    void AddVertexTable(Json::Value& json, VertexTable const& table, QPoint3d::Params const& qparams, Utf8StringCR idStr);
    void AddAuxChannelTable(Json::Value& json, AuxChannelTable const& table, Utf8StringCR idStr);
    void AddVertexIndices(Json::Value& json, ByteStreamCR indices, Utf8StringCR idStr, Utf8CP name="indices") { AddVertexIndices(json, indices.data(), indices.size(), idStr, name); }
    void AddVertexIndices(Json::Value& json, uint8_t const* data, size_t nBytes, Utf8StringCR idStr, Utf8CP name);
    template<typename T> void AddVertexIndices(Json::Value& json, bvector<T> const& values, Utf8StringCR idStr, Utf8CP name)
        {
        uint8_t const* data = reinterpret_cast<uint8_t const*>(values.data());
        size_t nBytesPerEntry = sizeof(*values.data());
        size_t nBytes = nBytesPerEntry * values.size();
        AddVertexIndices(json, data, nBytes, idStr, name);
        }

    void AddPolyline(Json::Value& json, TesselatedPolyline const& polyline, Utf8StringCR idStr);
    BentleyStatus AddMaterialJson(Render::MaterialCR material);
    BentleyStatus AddTextureJson(TextureMappingCR mapping, Json::Value& matJson);
    BentleyStatus CreateDisplayParamJson(Json::Value& matJson, MeshCR mesh, Utf8StringCR suffix);
    void WriteFeatureTable(FeatureTableCR featureTable);
    void AddAnimationNodeMap(FeatureTableCR featureTable);
    void AddMeshes(Render::Primitives::GeometryCollectionCR geometry);
    void AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index, FeatureTableCR featureTable);
    BentleyStatus CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr);
    Json::Value CreateMeshEdges(TriMeshArgs::Edges const&, TriMeshArgs const&, Utf8StringCR idStr);
    BentleyStatus CreatePolylines(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr);
    BentleyStatus CreatePolyline(Json::Value& primitiveJson, IndexedPolylineArgsCR args, Utf8StringCR idStr);
    BentleyStatus CreatePointString(Json::Value& primitiveJson, IndexedPolylineArgsCR args, Utf8StringCR idStr);
public:
    IModelTileWriter(StreamBufferR streamBuffer, Tile::LoaderCR loader) : T_Super(streamBuffer, *loader.GetTree().FetchModel()), m_loader(loader) { }

    IModelTile::WriteStatus WriteTile(Tile::Content::MetadataCR metadata, Render::Primitives::GeometryCollectionCR geometry);
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Constraints:
*   - Dimensions < max texture size
*       - only 64x64 guaranteed;
*       - in practice expect at least 2048 and most tablets/phones at least 4096 (96.3% of all browsers according to webglstats.com)
*   - Roughly square to reduce unused bytes at end of last row
*   - No extra unused bytes at end of each row
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void LUTDimensions::Init(uint32_t nEntries, uint32_t nRgbaPerEntry, uint32_t nExtraRgba, uint32_t nTables)
    {
    // This makes assumptions, backed up by statistics available online, about the minimum texture size supported by any hardware on which imodeljs frontend will run.
    static constexpr uint32_t maxWidth = 4096;
    uint32_t nRgba = nEntries * nRgbaPerEntry*nTables + nExtraRgba;

    if (nRgba < maxWidth)
        {
        m_width = nRgba;
        m_height = 1;
        return;
        }

    // Make roughly square to reduce unused space in last row
    uint32_t width = static_cast<uint32_t>(ceil(sqrt(nRgba)));

    // Ensure a given entry's RGBA values all fit on the same row.
    uint32_t remainder = width % nRgbaPerEntry;
    if(0 != remainder)
        width += nRgbaPerEntry - remainder;

    // Compute height
    uint32_t height = nRgba / width;
    if (width*height < nRgba)
        ++height;

    BeAssert(height <= maxWidth);
    BeAssert(width <= maxWidth);
    BeAssert(width * height >= nRgba);

    // Row padding should never be necessary...
    BeAssert(0 == width % nRgbaPerEntry);

    m_width = width;
    m_height = height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshParams::MeshParams(TriMeshArgsCR args, bool isVolumeClassifier) : m_vertexParams(args.m_pointParams)
    {
    if (isVolumeClassifier)
        {
        m_type = SurfaceType::Classifier;
        }
    else
        {
        bool textured = args.m_texture.IsValid();
        bool normals = nullptr != args.m_normals;

        if (textured)
            m_type = normals ? SurfaceType::TexturedLit : SurfaceType::Textured;
        else
            m_type = normals ? SurfaceType::Lit : SurfaceType::Unlit;
        }

    switch (m_type)
        {
        case SurfaceType::Lit:          Init<LitMeshVertex>(args); break;
        case SurfaceType::Textured:     Init<TexturedMeshVertex>(args); break;
        case SurfaceType::TexturedLit:  Init<TexturedLitMeshVertex>(args); break;
        default:                        Init<MeshVertex>(args); break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
QPoint2d::Params MeshParams::InitUVParams(TriMeshArgsCR args)
    {
    DRange2d range = DRange2d::NullRange();
    auto fpts = args.m_textureUV;
    if (nullptr != fpts)
        for (uint32_t i = 0; i < args.m_numPoints; i++)
            range.Extend(DPoint2d::From(fpts[i].x, fpts[i].y));

    return QPoint2d::Params(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeParams::EdgeParams(EdgeArgsCR args)
    {
    uint32_t numEdges = args.m_numEdges;
    BeAssert(0 < numEdges);

    // Each MeshEdge becomes a quad
    uint32_t nVerts = numEdges * 6;
    m_vertexIndicesCount = nVerts;

    // Each primary vertex identified by vec3-encoded index into LUT
    m_vertexIndices.resize(nVerts * 3);

    // Each 'other endpoint' vertex identified by vec3-encoded index into LUT plus a quad index in [0,3]
    m_endPointAndQuadIndices.resize(nVerts * 4);

    uint8_t* pVertexIndices = m_vertexIndices.GetDataP();
    uint8_t* pEndPointAndQuadIndices = m_endPointAndQuadIndices.GetDataP();
    auto addPoint = [&](uint32_t p0, uint32_t p1, uint8_t quadIndex)
        {
        memcpy(pVertexIndices, &p0, 3);
        memcpy(pEndPointAndQuadIndices, &p1, 3);
        pEndPointAndQuadIndices[3] = quadIndex;
        pVertexIndices += 3;
        pEndPointAndQuadIndices += 4;
        };

    for (uint32_t i = 0; i < numEdges; i++)
        {
        uint32_t p0 = args.m_edges[i].m_indices[0];
        uint32_t p1 = args.m_edges[i].m_indices[1];

        addPoint(p0, p1, 0);
        addPoint(p1, p0, 2);
        addPoint(p0, p1, 1);

        addPoint(p0, p1, 1);
        addPoint(p1, p0, 2);
        addPoint(p1, p0, 3);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineTesselator::PolylineTesselator(IndexedPolylineArgsCR args) : PolylineTesselator(args.m_lines, args.m_numLines,
    args.m_points, args.m_numPoints, wantJointTriangles(args.m_width, args.m_flags.Is2d()), args.m_pointParams)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineTesselator::PolylineTesselator(TriMeshArgsCR args) : PolylineTesselator(args.m_edges.m_polylines.m_lines, args.m_edges.m_polylines.m_numLines,
    args.m_points, args.m_numPoints, wantJointTriangles(args.m_edges.m_width, args.m_is2d), args.m_pointParams)
    {
    BeAssert(args.m_edges.m_polylines.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
double PolylineTesselator::DotProduct(Vertex const& v) const
    {
    DPoint3d pos = GetPosition(v.m_vertIndex);
    DVec3d prevDir = DVec3d::FromStartEndNormalize(GetPosition(v.m_prevIndex), pos);
    DVec3d nextDir = DVec3d::FromStartEndNormalize(GetPosition(v.m_nextIndex), pos);

    return prevDir.DotProduct(nextDir);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/18
//=======================================================================================
struct PolylineVert
{
    PolylineTesselator::Vertex const& v;
    PolylineTesselator::Param p;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineTesselator::AddSimpleSegment(Vertex const& v0, Vertex const& v1)
    {
    PolylineVert verts[4] =
        {
            { v0, v0.ComputeParam(true) },
            { v0, v0.ComputeParam(false) },
            { v1, v1.ComputeParam(false) },
            { v1, v1.ComputeParam(true) }
        };

    constexpr uint32_t indices[6] = { 0, 2, 1, 1, 2, 3 };
    for (uint32_t i : indices)
        {
        auto const& vert = verts[i];
        AddVertex(vert.v, vert.p);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineTesselator::AddJoints(Vertex const& v0, Vertex const& v1, bool jointAt0, bool jointAt1)
    {
    bool isV0Joint = v0.m_isSegmentStart ? jointAt0 : jointAt1;
    bool isV1Joint = v1.m_isSegmentStart ? jointAt0 : jointAt1;

    PolylineVert verts[6] =
        {
            { v0, v0.ComputeParam(true, isV0Joint, false, false) }, // 0
            { v0, v0.ComputeParam(false, isV0Joint, false, true) }, // 1
            { v0, v0.ComputeParam(false, isV0Joint, false, false) },// 2
            { v1, v1.ComputeParam(false, isV1Joint, false, false) },// 3
            { v1, v1.ComputeParam(false, isV1Joint, false, true) }, // 4
            { v1, v1.ComputeParam(true, isV1Joint, false, false) }, // 5
        };

    constexpr uint32_t indices[12] =
        {
            0, 3, 1,
            1, 3, 4,
            1, 4, 2,
            2, 4, 5,
        };

    for (uint32_t i : indices)
        {
        auto const& vert = verts[i];
        AddVertex(vert.v, vert.p);
        }

    if (jointAt0)
        AddJointTriangles(verts[1].v, verts[1].p, v0);

    if (jointAt1)
        AddJointTriangles(verts[4].v, verts[4].p, v1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineTesselator::AddJointTriangles(Vertex const& v0, Param p0, Vertex const& v1)
    {
    auto param = v1.ComputeParam(false, false, true);

    PolylineVert verts[5] =
        {
            { v0, p0 },
            { v1, static_cast<Param>(param + 0) },
            { v1, static_cast<Param>(param + 1) },
            { v1, static_cast<Param>(param + 2) },
            { v1, static_cast<Param>(param + 3) },
        };

    constexpr uint32_t indices[9] =
        {
            0, 2, 1,
            0, 3, 2,
            0, 4, 3,
        };

    for (uint32_t i : indices)
        {
        auto const& vert = verts[i];
        AddVertex(vert.v, vert.p);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineTesselator::AddVertex(Vertex const& vertex, Param param)
    {
    m_polyline.m_vertIndex.push_back(vertex.m_vertIndex);
    m_polyline.m_prevIndex.push_back(vertex.m_prevIndex);
    m_polyline.m_nextIndexAndParam.push_back(TesselatedPolyline::PosIndexAndParam(vertex.m_nextIndex, param));
    m_polyline.m_distance.push_back(static_cast<float>(vertex.m_length));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineTesselator::Tesselate()
    {
    bool doJoints = m_doJointTriangles;

    for (uint32_t lineIndex = 0; lineIndex < m_numLines; lineIndex++)
        {
        auto const& line = m_lines[lineIndex];
        if (line.m_numIndices < 2)
            continue;

        uint32_t const* lineIndices = line.m_vertIndex;
        double cumulativeLength = line.m_startDistance;
        bool isClosed = lineIndices[0] == lineIndices[line.m_numIndices-1];

        for (uint32_t i = 0, last = line.m_numIndices - 1; i < last; i++)
            {
            uint32_t idx0 = lineIndices[i];
            uint32_t idx1 = lineIndices[i+1];
            DPoint3d pos0 = GetPosition(idx0);
            DPoint3d pos1 = GetPosition(idx1);

            double thisLength = pos0.Distance(pos1);
            bool isStart = (0 == i);
            bool isEnd = (last-1 == i);

            uint32_t prevIdx0;
            if (isStart)
                prevIdx0 = isClosed ? lineIndices[last-1] : idx0;
            else
                prevIdx0 = lineIndices[i-1];

            uint32_t nextIdx1;
            if (isEnd)
                nextIdx1 = isClosed ? lineIndices[1] : idx1;
            else
                nextIdx1 = lineIndices[i+2];

            Vertex v0(true, isStart && !isClosed, idx0, prevIdx0, idx1, cumulativeLength);
            Vertex v1(false, isEnd && !isClosed, idx1, nextIdx1, idx0, cumulativeLength += thisLength);

            constexpr float s_maxJointDot = -0.7;
            bool jointAt0 = doJoints && (isClosed || !isStart) && DotProduct(v0) > s_maxJointDot;
            bool jointAt1 = doJoints && (isClosed || !isEnd) && DotProduct(v1) > s_maxJointDot;

            if (jointAt0 || jointAt1)
                AddJoints(v0, v1, jointAt0, jointAt1);
            else
                AddSimpleSegment(v0, v1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TesselatedPolyline::Param PolylineTesselator::Vertex::ComputeParam(bool negatePerp, bool adjacentToJoint, bool joint, bool noDisplacement) const
    {
    if (joint)
        return TesselatedPolyline::kJointBase;

    TesselatedPolyline::Param param;
    if (noDisplacement)
        param = TesselatedPolyline::kNoneAdjWt; // prevent getting tossed before width adjustment
    else if (adjacentToJoint)
        param = TesselatedPolyline::kMiterInsideOnly;
    else
        param = m_isPolylineStartOrEnd ? TesselatedPolyline::kSquare : TesselatedPolyline::kMiter;

    uint8_t adjust = 0;
    if (negatePerp)
        adjust = TesselatedPolyline::kNegatePerp;

    if (!m_isSegmentStart)
        adjust += TesselatedPolyline::kNegateAlong;

    param = static_cast<TesselatedPolyline::Param>(adjust + param);
    return param;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineEdgeParamsUPtr PolylineEdgeParams::Create(TriMeshArgsCR args)
    {
    TesselatedPolyline polyline = PolylineTesselator::Tesselate(args);
    return PolylineEdgeParamsUPtr(polyline.IsValid() ? new PolylineEdgeParams(std::move(polyline)) : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineParams::PolylineParams(IndexedPolylineArgsCR args, TesselatedPolyline&& polyline) : m_polyline(std::move(polyline)),
    m_vertexParams(args.m_pointParams)
    {
    m_lutParams.Init<LinearVertex>(args, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineParamsUPtr PolylineParams::Create(IndexedPolylineArgsCR args)
    {
    TesselatedPolyline polyline = PolylineTesselator::Tesselate(args);
    if (!polyline.IsValid())
        return nullptr;

    return PolylineParamsUPtr(new PolylineParams(args, std::move(polyline)));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::CreatePointString(Json::Value& primitiveJson, IndexedPolylineArgsCR args, Utf8StringCR idStr)
    {
    PointStringParams params(args);
    AddVertexTable(primitiveJson, params.m_lutParams, params.m_vertexParams, idStr);
    AddVertexIndices(primitiveJson, params.m_vertexIndices, idStr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::CreatePolyline(Json::Value& primitiveJson, IndexedPolylineArgsCR args, Utf8StringCR idStr)
    {
    auto params = PolylineParams::Create(args);
    if (nullptr == params || IsCanceled())
        return ERROR;

    AddVertexTable(primitiveJson, params->m_lutParams, params->m_vertexParams, idStr);
    AddPolyline(primitiveJson, params->m_polyline, idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::CreatePolylines(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    PolylineArgs args;
    if (!args.Init(mesh))
        return ERROR;
    else if (args.m_flags.IsDisjoint())
        return CreatePointString(primitiveJson, args, idStr);
    else
        return CreatePolyline(primitiveJson, args, idStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    MeshArgs args;
    if (!args.Init(mesh))
        return ERROR;

    MeshParams meshParams(args, m_loader.GetTree().IsVolumeClassifier());
    AddVertexTable(primitiveJson, meshParams.m_lutParams, meshParams.m_vertexParams, idStr);
    AddAuxChannelTable(primitiveJson, meshParams.m_auxChannels, idStr);

    SurfaceParams surface(args);
    AddVertexIndices(primitiveJson["surface"], surface.m_vertexIndices, idStr + "Surface");
    primitiveJson["surface"]["type"] = static_cast<uint32_t>(meshParams.m_type);

    if (args.m_texture.IsValid())
        {
        DRange2d uvRange = meshParams.m_uvParams.GetRange();
        primitiveJson["surface"]["uvParams"] = CreateDecodeQuantizeValues(&uvRange.low.x, &uvRange.high.x, 2);
        }

    if (args.m_edges.IsValid())
        primitiveJson["edges"] = CreateMeshEdges(args.m_edges, args, idStr);



    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value IModelTileWriter::CreateMeshEdges(TriMeshArgs::Edges const& edges, TriMeshArgs const& meshArgs, Utf8StringCR idStr)
    {
    Json::Value json(Json::objectValue);

    if (edges.m_edges.IsValid())
        {
        EdgeParams edgeParams(edges.m_edges);
        auto segIdStr = idStr + "Segments";
        AddVertexIndices(json["segments"], edgeParams.m_vertexIndices, segIdStr);
        AddVertexIndices(json["segments"], edgeParams.m_endPointAndQuadIndices, segIdStr, "endPointAndQuadIndices");
        }

    if (edges.m_silhouettes.IsValid())
        {
        SilhouetteParams silhouetteParams(edges.m_silhouettes);
        auto silIdStr = idStr + "Silhouettes";
        AddVertexIndices(json["silhouettes"], silhouetteParams.m_vertexIndices, silIdStr);
        AddVertexIndices(json["silhouettes"], silhouetteParams.m_endPointAndQuadIndices, silIdStr, "endPointAndQuadIndices");
        AddVertexIndices(json["silhouettes"], silhouetteParams.m_normalPairs, silIdStr, "normalPairs");
        }

    if (edges.m_polylines.IsValid())
        {
        auto polylineParams = PolylineEdgeParams::Create(meshArgs);
        if (nullptr != polylineParams)
            AddPolyline(json["polylines"], polylineParams->m_polyline, idStr + "PolylineEdges");
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddPolyline(Json::Value& json, TesselatedPolyline const& polyline, Utf8StringCR idStr)
    {
    AddVertexIndices(json, polyline.m_vertIndex, idStr, "indices");
    AddVertexIndices(json, polyline.m_prevIndex, idStr, "prevIndices");
    AddVertexIndices(json, polyline.m_nextIndexAndParam, idStr, "nextIndicesAndParams");

#if defined(INCLUDE_POLYLINE_DISTANCE)
    Utf8String bvId("bvDistance");
    bvId.append(idStr);
    AddBufferView(bvId.c_str(), polyline.m_distance);
    json["distances"] = bvId;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddVertexIndices(Json::Value& primitiveJson, uint8_t const* data, size_t nBytes, Utf8StringCR idStr, Utf8CP name)
    {
    Utf8String bufferViewId("bv");
    bufferViewId.append(name);
    bufferViewId.append(idStr);

    AddBufferView(bufferViewId.c_str(), data, nBytes);
    primitiveJson[name] = bufferViewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddVertexTable(Json::Value& primitiveJson, VertexTable const& table, QPoint3d::Params const& qparams, Utf8StringCR idStr)
    {
    Utf8String bufferViewId("bv");
    bufferViewId.append("Vertex");
    bufferViewId.append(idStr);

    AddBufferView(bufferViewId.c_str(), table.m_data);
    primitiveJson["vertices"]["bufferView"] = bufferViewId;

    DRange3d range = qparams.GetRange();
    primitiveJson["vertices"]["params"] = CreateDecodeQuantizeValues(&range.low.x, &range.high.x, 3);

    primitiveJson["vertices"]["featureIndexType"] = static_cast<uint32_t>(table.m_features.m_type);
    if (FeatureIndex::Type::Uniform == table.m_features.m_type)
        primitiveJson["vertices"]["featureID"] = table.m_features.m_uniform;

    primitiveJson["vertices"]["hasTranslucency"] = table.m_colors.m_hasAlpha;
    primitiveJson["vertices"]["numColors"] = table.m_colors.m_numColors;
    if (table.m_colors.m_isUniform)
        primitiveJson["vertices"]["uniformColor"] = table.m_colors.m_uniform.GetValue();

    auto atlas = table.m_materialAtlas;
    if (atlas.IsValid())
        {
        primitiveJson["vertices"]["materialAtlas"]["numMaterials"] = atlas->NumMaterials();
        primitiveJson["vertices"]["materialAtlas"]["hasTranslucency"] = atlas->IsTranslucent();
        primitiveJson["vertices"]["materialAtlas"]["overridesAlpha"] = atlas->OverridesAlpha();
        }

    primitiveJson["vertices"]["count"] = table.m_numVertices;
    primitiveJson["vertices"]["width"] = table.m_dimensions.GetWidth();
    primitiveJson["vertices"]["height"] = table.m_dimensions.GetHeight();
    primitiveJson["vertices"]["numRgbaPerVertex"] = table.m_numRgbaPerVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddAuxChannelTable(Json::Value& primitiveJson, AuxChannelTable const& table, Utf8StringCR idStr)
    {
    if (!table.IsValid())
        return;

    Utf8String bufferViewId("bv");
    bufferViewId.append("AuxChannels").append(idStr);

    AddBufferView(bufferViewId.c_str(), table.m_data);
    auto& json = primitiveJson["auxChannels"];
    json["bufferView"] = bufferViewId;
    json["count"] = table.m_numVertices;
    json["width"] = table.m_dimensions.GetWidth();
    json["height"] = table.m_dimensions.GetHeight();
    json["numBytesPerVertex"] = table.m_numBytesPerVertex;

    if (!table.m_displacements.isNull())
        json["displacements"] = table.m_displacements;

    if (!table.m_normals.isNull())
        json["normals"] = table.m_normals;

    if (!table.m_params.isNull())
        json["params"] = table.m_params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddMeshes(Render::Primitives::GeometryCollectionCR geometry)
    {
    size_t          primitiveIndex = 0;

    Json::Value     meshes     = Json::objectValue;
    Json::Value     nodes      = Json::objectValue;

    for (auto& geomMesh : geometry.Meshes())
        {
        Utf8String  id = (0 == geomMesh->GetNodeIndex()) ? Utf8String("Root") : Utf8PrintfString("%d", geomMesh->GetNodeIndex());
        Utf8String  meshId = "Mesh_" + id;

        if (!meshes.isMember(meshId))
            {
            Utf8String     nodeId = "Node_" + id;
            nodes[nodeId] = meshId;
            meshes[meshId] = Json::objectValue;
            meshes[meshId]["primitives"] = Json::arrayValue;
            }

        AddMesh(meshes[meshId]["primitives"], *geomMesh, primitiveIndex, geometry.Meshes().FeatureTable());
        if (IsCanceled())
            return;
        }
    m_json["meshes"] = meshes;
    m_json["nodes"]  = nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setTransform(float* dst, TransformCR transform)
    {
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 4; j++)
            *dst++ = static_cast<float>(transform.form3d[i][j]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setOverrides(uint8_t* dst, SharedGeom::SymbologyOverrides ovrFlags, DisplayParamsCR params)
    {
    // The first byte holds what's actually overridden.
    // Easier to just write all the symbology than to branch based on each flag.
    ColorDef colorDef = params.GetFillColorDef();
    colorDef.SetAlpha(0xff - colorDef.GetAlpha());

    auto weight = static_cast<uint8_t>(params.GetLineWidth());
    if (weight < 1)
        weight = 1;
    else if (weight > 31)
        weight = 31;

    // Line code IDs used by webgl renderer
    uint8_t lineCode = 0;
    switch (params.GetLinePixels())
        {
        case LinePixels::Code0: lineCode = 0; break;
        case LinePixels::Code1: lineCode = 1; break;
        case LinePixels::Code2: lineCode = 2; break;
        case LinePixels::Code3: lineCode = 3; break;
        case LinePixels::Code4: lineCode = 4; break;
        case LinePixels::Code5: lineCode = 5; break;
        case LinePixels::Code6: lineCode = 6; break;
        case LinePixels::Code7: lineCode = 7; break;
        case LinePixels::HiddenLine: lineCode = 8; break;
        case LinePixels::Invisible: lineCode = 9; break;
        }

    dst[0] = static_cast<uint8_t>(ovrFlags);
    dst[1] = weight;
    dst[2] = lineCode;
    dst[3] = 0;

    uint32_t color = colorDef.GetValue();
    memcpy(dst + 4, &color, sizeof(color));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18                       p
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index, FeatureTableCR featureTable)
    {
    Utf8String idStr(std::to_string(index++).c_str());
    Json::Value materialJson = Json::objectValue;
    Json::Value primitiveJson = Json::objectValue;

    if (SUCCESS != CreateDisplayParamJson(materialJson, mesh, idStr))
        return;

    primitiveJson["type"] = static_cast<uint32_t>(mesh.GetType());
    primitiveJson["isPlanar"] = mesh.IsPlanar();

    if ((!mesh.Triangles().Empty() && SUCCESS == CreateTriMesh(primitiveJson, mesh, idStr)) ||
        (!mesh.Polylines().empty() && SUCCESS == CreatePolylines(primitiveJson, mesh, idStr)))
        {
        Utf8String  materialName = "Material" + idStr;
        m_json["materials"][materialName] = materialJson;
        primitiveJson["material"] = materialName;

        auto part = mesh.GetSharedGeom();
        if (nullptr != part)
            {
            auto const& instances = part->GetInstances();
            uint32_t nInstances = static_cast<uint32_t>(instances.size());
            BeAssert(nInstances > 0);

            static constexpr uint32_t sizeOfTransform = sizeof(float) * 12;
            ByteStream transforms(sizeOfTransform * nInstances);
            ByteStream featureIds(3 * nInstances);

            ByteStream ovrs;
            auto ovrFlags = part->GetSymbologyOverrides();
            static constexpr uint32_t sizeOfOvrs = sizeof(uint32_t) * 2;
            if (SharedGeom::SymbologyOverrides::None != ovrFlags)
                ovrs = ByteStream(sizeOfOvrs * nInstances);

            DRange3d transformRange = DRange3d::NullRange();
            for (uint32_t i = 0; i < nInstances; i++)
                {
                auto const& instance = instances[i];
                transformRange.Extend(instance.GetTransform().Origin());
                }

            DPoint3d transformCenter = DPoint3d::FromInterpolate(transformRange.low, 0.5, transformRange.high);

            for (uint32_t i = 0; i < nInstances; i++)
                {
                auto const& instance = instances[i];
                uint32_t featureIndex = 0;
                if (!featureTable.FindIndex(featureIndex, instance.GetFeature()))
                    {
                    BeAssert(false && "Instance not present in feature table");
                    }

                // Feature IDs are limited to 24 bits.
                size_t featureByteIndex = 3 * i;
                memcpy(featureIds.data() + featureByteIndex, &featureIndex, 3);

                // Make transform relative to center to eliminate precision issues in shader...
                Transform tf = instance.GetTransform();
                DPoint3d origin;
                origin.DifferenceOf(tf.Origin(), transformCenter);
                tf.SetTranslation(origin);

                size_t transformIndex = sizeOfTransform * i;
                setTransform(reinterpret_cast<float*>(transforms.data() + transformIndex), tf);

                // The first byte holds what's actually overridden.
                // Easier to just write all the symbology than to branch based on each flag.
                if (SharedGeom::SymbologyOverrides::None != ovrFlags)
                    {
                    size_t ovrsIndex = sizeOfOvrs * i;
                    setOverrides(ovrs.data() + ovrsIndex, part->GetSymbologyOverrides(), instance.GetDisplayParams());
                    }
                }

            Utf8String featureBufferId("bv"), transformBufferId("bv");
            featureBufferId.append("InstanceFeatures").append(idStr);
            transformBufferId.append("InstanceTransforms").append(idStr);

            AddBufferView(featureBufferId.c_str(), featureIds);
            AddBufferView(transformBufferId.c_str(), transforms);

            primitiveJson["instances"]["count"] = nInstances;
            primitiveJson["instances"]["featureIds"] = featureBufferId;
            primitiveJson["instances"]["transforms"] = transformBufferId;

            Json::Value centerJson(Json::arrayValue);
            centerJson.append(transformCenter.x);
            centerJson.append(transformCenter.y);
            centerJson.append(transformCenter.z);
            primitiveJson["instances"]["transformCenter"] = std::move(centerJson);

            if (SharedGeom::SymbologyOverrides::None != ovrFlags)
                {
                Utf8String ovrBufferId("bv");
                ovrBufferId.append("InstanceOverrides").append(idStr);
                AddBufferView(ovrBufferId.c_str(), ovrs);
                primitiveJson["instances"]["symbologyOverrides"] = ovrBufferId;
                }
            }

        primitivesNode.append(primitiveJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::CreateDisplayParamJson(Json::Value& matJson, MeshCR mesh, Utf8StringCR suffix)
    {
    auto const& displayParams = mesh.GetDisplayParams();

    matJson["type"] = (uint8_t) displayParams.GetType();

    // GeomParams...
    if (displayParams.GetCategoryId().IsValid())
        matJson["categoryId"] = displayParams.GetCategoryId().ToHexStr();

    if (displayParams.GetSubCategoryId().IsValid())
        matJson["subCategoryId"] = displayParams.GetSubCategoryId().ToHexStr();

    // ###TODO: Support non-persistent materials if/when necessary...
    auto material = (nullptr == mesh.GetMaterialAtlas() || mesh.GetMaterialAtlas()->NumMaterials() < 2) ? displayParams.GetSurfaceMaterial().GetMaterial() : nullptr;
    if (nullptr != material && material->GetKey().IsPersistent())
        {
        matJson["materialId"] = material->GetKey().GetId().ToHexStr();
        AddMaterialJson(*material);
        }

    matJson["ignoreLighting"] = displayParams.IgnoresLighting();

    matJson["fillColor"] = displayParams.GetFillColor();
    matJson["fillFlags"] = static_cast<uint32_t> (displayParams.GetFillFlags());

    matJson["lineColor"]  = displayParams.GetLineColor();     // Edges?
    matJson["lineWidth"]  = displayParams.GetLineWidth();
    matJson["linePixels"] = (uint32_t) displayParams.GetLinePixels();     // Edges?

    if (nullptr != displayParams.GetSurfaceMaterial().GetGradient())
        matJson["gradient"] = displayParams.GetSurfaceMaterial().GetGradient()->ToJson();

    TextureCP texture = displayParams.GetSurfaceMaterial().GetTextureMapping().GetTexture();
    if (nullptr != texture)
        {
        if (texture->GetKey().IsValid() && SUCCESS != AddTextureJson(displayParams.GetSurfaceMaterial().GetTextureMapping(), matJson))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nate.Rex        06/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::AddMaterialJson(Render::MaterialCR material)
    {
    BeAssert(material.GetKey().IsValid());	// Assume that each persistent material will contain an id
    Utf8String id = material.GetKey().GetId().ToHexStr();

    if (!m_json.isMember("renderMaterials") || !m_json["renderMaterials"].isMember(id))
        {
        Json::Value& materialJson = m_json["renderMaterials"][id];

        // Add texture contained by material.
        if (material.HasTextureMapping())
            {
            Json::Value materialTextureJson;
            AddTextureJson(material.GetTextureMapping(), materialTextureJson);	// This will cause it to get added to "namedTextures" as well
            materialJson["textureMapping"] = materialTextureJson;
            }

        RenderMaterialCPtr matElem = RenderMaterial::Get(m_model.GetDgnDb(), material.GetKey().GetId());
        RenderingAssetCP asset = matElem.IsValid() ? &matElem->GetRenderingAsset() : nullptr;
        BeAssert(asset != nullptr);
        if (asset != nullptr)
            {
            if (asset->GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_Color);
                materialJson["diffuseColor"][0] = rgb.red;
                materialJson["diffuseColor"][1] = rgb.green;
                materialJson["diffuseColor"][2] = rgb.blue;
                }
            if (asset->GetBool(RENDER_MATERIAL_FlagHasSpecularColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_SpecularColor);
                materialJson["specularColor"][0] = rgb.red;
                materialJson["specularColor"][1] = rgb.green;
                materialJson["specularColor"][2] = rgb.blue;
                }
            if (asset->GetBool(RENDER_MATERIAL_FlagHasFinish, false))
                {
                materialJson["specularExponent"] = asset->GetDouble(RENDER_MATERIAL_Finish, Material::Defaults::SpecularExponent());
                }

            auto transparency = asset->GetBool(RENDER_MATERIAL_FlagHasTransmit, false) ? asset->GetDouble(RENDER_MATERIAL_Transmit, 0.0) : 0.0;
            materialJson["transparency"] = transparency;

            if (asset->GetBool(RENDER_MATERIAL_FlagHasDiffuse, false))
                materialJson["diffuse"] = asset->GetDouble(RENDER_MATERIAL_Diffuse, Material::Defaults::Diffuse());

            double specular;
            if (asset->GetBool(RENDER_MATERIAL_FlagHasSpecular, false))
                specular = asset->GetDouble(RENDER_MATERIAL_Specular, Material::Defaults::Specular());
            else
                specular = 0.0;     // Lack of specular means 0.0 -- not default (painting overspecular in PhotoRealistic Rendering
            materialJson["specular"] = specular;

            if (asset->GetBool(RENDER_MATERIAL_FlagHasReflect, false))
                {
                // Reflectance stored as fraction of specular in V8 material settings.
                materialJson["reflect"] = specular * asset->GetDouble(RENDER_MATERIAL_Reflect, Material::Defaults::Reflect());
                }

            if (asset->GetBool(RENDER_MATERIAL_FlagHasReflectColor, false))
                {
                RgbFactor rgb = asset->GetColor(RENDER_MATERIAL_ReflectColor);
                materialJson["reflectColor"][0] = rgb.red;
                materialJson["reflectColor"][1] = rgb.green;
                materialJson["reflectColor"][2] = rgb.blue;
                }


            if (asset->GetBool(RENDER_MATERIAL_FlagHasRefract, false))
                materialJson["refract"] = asset->GetDouble(RENDER_MATERIAL_Refract, RenderingAsset::Default::Refract());
            materialJson["shadows"] = !(asset->GetBool(RENDER_MATERIAL_FlagNoShadows, false));
            materialJson["ambient"] = asset->GetDouble(RENDER_MATERIAL_Ambient, RenderingAsset::Default::Ambient());
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Paul.Connelly   01/18
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelTileWriter::AddTextureJson(TextureMappingCR mapping, Json::Value& matJson)
    {
    // NB: I am specifically not using the same representation we use for textures in Cesium tiles because
    // it includes much extra data and indirection which we don't need; and doesn't include some of
    // the mapping params we need.
    BeAssert(mapping.IsValid());
    TextureCR texture = *mapping.GetTexture();

    // Identifier will be name for named textures, and hex id string for persistent textures (if we are supposed to embed persistents)
    Utf8String name = "";
    if (texture.GetKey().IsNamed())
        {
        name = texture.GetKey().GetName();
        }
    else
        {
        name = texture.GetKey().GetId().ToHexStr();
        }

    if (!m_json.isMember("namedTextures") || !m_json["namedTextures"].isMember(name))
        {
        ImageSourceCP img = texture.GetImageSource();
        if (nullptr == img || !img->IsValid())
            {
            BeAssert(false);
            return ERROR;
            }

        AddBufferView(name.c_str(), img->GetByteStream());

        Json::Value& json = m_json["namedTextures"][name];
        json["format"] = static_cast<uint32_t>(img->GetFormat());
        json["bufferView"] = name;
        json["isGlyph"] = texture.IsGlyph();

        auto dimensions = texture.GetDimensions();
        json["width"] = dimensions.width;
        json["height"] = dimensions.height;
        }

    auto const& params = mapping.GetParams();
    Json::Value& paramsJson = matJson["texture"]["params"];
    paramsJson["mode"] = static_cast<uint32_t>(params.m_mapMode);
    paramsJson["weight"] = params.m_textureWeight;
    paramsJson["worldMapping"] = params.m_worldMapping;
    for (uint32_t i = 0; i < 2; i++)
        for (uint32_t j = 0; j < 3; j++)
            paramsJson["transform"][i][j] = params.m_textureMat2x3.m_val[i][j];

    matJson["texture"]["name"] = name;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::WriteFeatureTable(FeatureTableCR featureTable)
    {
    uint32_t startPosition = m_buffer.GetSize();

    auto packed = featureTable.Pack();
    auto const& bytes = packed.GetBytes();

    m_buffer.Append(startPosition); // Overwritten below.
    m_buffer.Append(packed.GetMaxFeatures());
    m_buffer.Append(packed.GetNumFeatures());
    m_buffer.Append(bytes.data(), bytes.GetSize());

    WriteLength(startPosition, startPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelTileWriter::AddAnimationNodeMap(FeatureTableCR featureTable)
    {
    auto const& nodeMap = m_loader.GetTree().GetAnimationNodeMap();
    if (nodeMap.IsEmpty() || featureTable.empty())
        return;

    uint8_t nBytesPerId = 1;
    uint32_t maxNodeId = nodeMap.GetMaxNodeIndex();
    if (maxNodeId > 0xff)
        nBytesPerId = (maxNodeId > 0xffff) ? 4 : 2;

    bool haveNodeId = false;
    ByteStream bytes(featureTable.GetNumIndices() * nBytesPerId);
    for (auto const& feature : featureTable)
        {
        uint32_t featureIndex = feature.second;
        uint32_t nodeId = nodeMap.GetNodeIndex(feature.first.GetElementId());
        haveNodeId = haveNodeId || (0 != nodeId);
        switch (nBytesPerId)
            {
            case 1:
                BeAssert(nodeId <= 0xff);
                bytes.GetDataP()[featureIndex] = static_cast<uint8_t>(nodeId);
                break;
            case 2:
                BeAssert(nodeId <= 0xffff);
                reinterpret_cast<uint16_t*>(bytes.GetDataP())[featureIndex] = static_cast<uint16_t>(nodeId);
                break;
            case 4:
                reinterpret_cast<uint32_t*>(bytes.GetDataP())[featureIndex] = nodeId;
                break;
            }
        }

    if (!haveNodeId)
        return; // none of the elements in the feature table belong to any animation node - don't bother writing out the zero node IDs.

    Utf8CP bufferViewId = "bv_animationIds";
    AddBufferView(bufferViewId, bytes);

    Json::Value animationNodes(Json::objectValue);
    animationNodes["bytesPerId"] = nBytesPerId;
    animationNodes["bufferView"] = bufferViewId;

    m_json["animationNodes"]["bytesPerId"] = nBytesPerId;
    m_json["animationNodes"]["bufferView"] = bufferViewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
IModelTile::WriteStatus IModelTileWriter::WriteTile(Tile::Content::MetadataCR metadata, Render::Primitives::GeometryCollectionCR geometry)
    {
    uint32_t startPosition = m_buffer.GetSize();
    m_buffer.Append(Format::IModel);

    auto version = IModelTile::Version::FromMajorVersion(m_loader.GetContentId().GetMajorVersion());
    BeAssert(version.IsValid() && version.IsKnown());
    m_buffer.Append(version.ToUint32());

    // NB: headerLength NOT equal to sizeof(Header) due to padding...
    uint32_t headerLengthDataPosition = m_buffer.GetSize();
    m_buffer.Append(static_cast<uint32_t>(0));

    m_buffer.Append(static_cast<uint32_t>(metadata.m_flags));
    m_buffer.Append(metadata.m_contentRange);
    m_buffer.Append(metadata.m_tolerance);
    m_buffer.Append(metadata.m_numElementsIncluded);
    m_buffer.Append(metadata.m_numElementsExcluded);

    // Length will be filled in afterward
    uint32_t lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);

    // NB: New header fields follow tileLength field.
    m_buffer.Append(static_cast<uint32_t>(metadata.m_emptySubRanges));

    WriteLength(startPosition, headerLengthDataPosition);

    if (geometry.Meshes().FeatureTable().size() == 0 && geometry.Meshes().size() != 0)
        BeAssert(false && "WriteTile: empty feature table");

    WriteFeatureTable(geometry.Meshes().FeatureTable());
    PadToBoundary();

    if (IsCanceled())
        return IModelTile::WriteStatus::Aborted;

    AddAnimationNodeMap(geometry.Meshes().FeatureTable());
    AddMeshes(geometry);

    if (IsCanceled())
        return IModelTile::WriteStatus::Aborted;

    if (SUCCESS != WriteGltf())
        return IModelTile::WriteStatus::Error;

    PadToBoundary();
    WriteLength(startPosition, lengthDataPosition);

    return IModelTile::WriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
IModelTile::WriteStatus Tile::IO::WriteIModelTile(StreamBufferR streamBuffer, Tile::Content::MetadataCR metadata, Render::Primitives::GeometryCollectionCR geometry, Tile::LoaderCR loader)
    {
    IModelTileWriter writer(streamBuffer, loader);
    return writer.WriteTile(metadata, geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
IModelTile::Version IModelTile::Version::FromMajorVersion(uint16_t major)
    {
    switch (major)
        {
        case 1: return V1();
        case 2: return V2();
        case 3: return V3();
        case 4: return V4();
        case 5: return V5();
        default: return Invalid();
        }
    }

