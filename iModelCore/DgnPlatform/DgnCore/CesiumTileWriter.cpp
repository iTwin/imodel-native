/*-------------------------------------------------------------------------------------+                                                                                           
|

|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>
#include <folly/BeFolly.h>

USING_NAMESPACE_TILETREE_IO
USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       


BEGIN_TILETREE_IO_NAMESPACE

#define JSON_Root "root"
#define JSON_GeometricError "geometricError"
#define JSON_BoundingVolume "boundingVolume"
#define JSON_Box "box"
#define JSON_Children "children"
#define JSON_Content "content"
#define JSON_Transform "transform"


// Used for reality meshes.
static Utf8String s_unlitTextureVertexShader = R"RAW_STRING(
    attribute vec3 a_pos;
    attribute vec2 a_texc;
    varying vec2 v_texc;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
    void main(void)
        {
        v_texc = a_texc;
        gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);
        }
)RAW_STRING";

static Utf8String s_unlitTextureFragmentShader = R"RAW_STRING(
    varying vec2 v_texc;  
    uniform sampler2D u_tex; 
    void main(void)
        {
        gl_FragColor = texture2D(u_tex, v_texc);
        }
)RAW_STRING";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String      getJsonString(Json::Value const& value)
    {
    Utf8String      string =  Json::FastWriter().write(value);

    // Pad to 4 byte boundary...
    while (0 != string.size() % 4)
        string = string + " ";

    return string;
    }


//=======================================================================================
// Flat (non-hierarchical) batch table builder.
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct BatchTableBuilder
{
private:
    Json::Value                             m_json;
    DgnDbR                                  m_db;
    bool                                    m_is3d;
    DgnCategoryId                           m_uncategorized;
#if defined(ERROR_UNUSED_FIELD)
    FeatureTableCR                          m_attrs;
#endif
    bmap<DgnElementId, DgnElementId>        m_assemblyIds;
    bmap<DgnSubCategoryId, DgnCategoryId>   m_categoryIds;
                            
    bool IsUncategorized(DgnCategoryId id) const { return id.IsValid() && id == m_uncategorized; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId QueryCategoryId(DgnSubCategoryId subCategoryId)
    {
    auto    found = m_categoryIds.find(subCategoryId);

    if (found != m_categoryIds.end())
        return found->second;

    DgnCategoryId       categoryId;

    DgnSubCategoryCPtr subCategory = m_db.Elements().Get<DgnSubCategory> (subCategoryId);

    if (subCategory.IsValid())
        categoryId = subCategory->GetCategoryId();

    m_categoryIds.Insert(subCategoryId, categoryId);

    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId QueryAssemblyId(DgnElementId childId) 
    {
    auto    found = m_assemblyIds.find(childId);

    if (found != m_assemblyIds.end())
        return found->second;

    DgnElementId        assemblyId;
    DgnCategoryId       assemblyCategoryId;

    assemblyId = childId;
    if (!childId.IsValid())
        return assemblyId;

    // Get this element's category and parent element
    // Recurse until no more parents (or we find a non-geometric parent)
    static constexpr Utf8CP s_3dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
    static constexpr Utf8CP s_2dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE ECInstanceId=?";

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db.GetPreparedECSqlStatement(m_is3d ? s_3dsql : s_2dsql);
    stmt->BindId(1, childId);

    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto thisCatId = stmt->GetValueId<DgnCategoryId>(1);
        if (assemblyCategoryId.IsValid() && IsUncategorized(thisCatId) && !IsUncategorized(assemblyCategoryId))
            break; // yuck. if have children with valid categories, stop before first uncategorized parent (V8 complex header).

        assemblyCategoryId = thisCatId;
        assemblyId = childId;

        childId = stmt->GetValueId<DgnElementId>(0);
        if (!childId.IsValid())
            break;

        // Try to get the parent's category. If parent is not geometric, this will fail and we will treat current child as the assembly root.
        stmt->Reset();
        stmt->BindId(1, childId);
        }
    m_assemblyIds.Insert(childId, assemblyId);

    return assemblyId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void InitUncategorizedCategory()
    {
    // This is dumb. See OfficeBuilding.dgn - cells have no level in V8, which translates to 'Uncategorized' (2d and 3d variants) in DgnDb
    // We don't want to create an 'Uncategorized' assembly if its children belong to a real category.
    // We only can detect this because for whatever reason, "Uncategorized" is not a localized string.
    DefinitionModelR dictionary = m_db.GetDictionaryModel();
    DgnCode code = m_is3d ? SpatialCategory::CreateCode(dictionary, "Uncategorized") : DrawingCategory::CreateCode(dictionary, "Uncategorized");

    m_uncategorized = DgnCategory::QueryCategoryId(m_db, code);
    }

public:
    Json::Value& GetJson()          { return m_json; }
    Utf8String ToString() const     { return getJsonString(m_json); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BatchTableBuilder(FeatureTableCR attrs, DgnDbR db, bool is3d) : m_json(Json::objectValue), m_db(db), m_is3d(is3d)
    {
    InitUncategorizedCategory();

    Json::Value             geomClasses    = Json::arrayValue, 
                            elementIds     = Json::arrayValue, 
                            assemblyIds    = Json::arrayValue, 
                            categoryIds    = Json::arrayValue,
                            subCategoryIds = Json::arrayValue;
                        
    for (auto const& kvp : attrs)
        {
        FeatureCR           attr = kvp.first;                                                                                                                                    
        uint32_t            index = kvp.second;
        DgnElementId        elementId = attr.GetElementId();

        geomClasses[index]    = (int) attr.GetClass();
        elementIds[index]     = elementId.ToString();
        assemblyIds[index]    = QueryAssemblyId(elementId).ToString();
        subCategoryIds[index] = attr.GetSubCategoryId().ToString();
        categoryIds[index]    = QueryCategoryId(attr.GetSubCategoryId()).ToString();

        }
    
    m_json["geomClass"]   = std::move(geomClasses);
    m_json["element"]     = std::move(elementIds);
    m_json["assembly"]    = std::move(assemblyIds);
    m_json["subCategory"] = std::move(subCategoryIds);
    m_json["category"]    = std::move(categoryIds);
    }


};  // BatchTableBuilder


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct PointCloudData
{
    bvector<QPoint3d>   m_points;
    bvector<uint8_t>    m_colors;
    QPoint3d::Params    m_qParams;

    PointCloudData(PointCloudArgsCR args) : m_qParams(args.m_qParams)
        {
        m_points.resize(args.m_numPoints);
        memcpy (m_points.data(), args.m_points, args.m_numPoints * sizeof(QPoint3d));
        if (nullptr != args.m_colors)
            {
            m_colors.resize(args.m_numPoints * 3);
            memcpy (m_colors.data(), args.m_colors, args.m_numPoints*3);
            }
        }

};  // PointCloudData

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct MeshMaterial 
{ 
    Utf8String              m_name;
    Render::MaterialCPtr    m_material;
    TileTextureCPtr         m_texture;

    MeshMaterial (Utf8StringCR suffix, MaterialCP material, TextureCP texture) : m_name("Material_" + suffix), m_material(material), m_texture(dynamic_cast<TileTextureCP>(texture)) { }

    bool IsTextured() const { return m_texture.IsValid(); }
    bool IgnoresLighting() const { return false; }
    Utf8StringCR GetName() const { return m_name; }
    TileTextureCPtr     GetTexture() const { return m_texture; }


};  // MeshMaterial


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct CesiumTileWriter : TileTree::IO::Writer
{
    CesiumTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : TileTree::IO::Writer(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddShader(Json::Value& shaders, Utf8CP name, int32_t type, Utf8CP buffer)
    {
    auto& shader = (shaders[name] = Json::objectValue);
    shader["type"] = type;
    shader["extensions"]["KHR_binary_glTF"]["bufferView"] = buffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTechniqueParameter(Json::Value& technique, Utf8CP name, Gltf::DataType type, Utf8CP semantic)
    {
    auto& param = technique["parameters"][name];
    param["type"] = static_cast<int32_t>(type);
    if (nullptr != semantic)
        param["semantic"] = semantic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendProgramAttribute(Json::Value& program, Utf8CP attrName)
    {
    program["attributes"].append(attrName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddMeshShaderTechnique(Utf8StringCR idStr, Utf8StringCR vertexShaderString, Utf8StringCR fragmentShaderString, bool textured = true)
    {
    Utf8String techniqueName = "Technique_" + idStr;

    if (m_json.isMember("techniques") && m_json["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value technique(Json::objectValue);

    AddTechniqueParameter(technique, "mv", Gltf::DataType::FloatMat4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", Gltf::DataType::FloatMat4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", Gltf::DataType::FloatVec3, "POSITION");

    Utf8String         programName               = "Program_" + idStr;
    Utf8String         vertexShader              = "VertexShader_" + idStr;
    Utf8String         fragmentShader            = "FragmentShader" + idStr;;
    Utf8String         vertexShaderBufferView    = vertexShader + "_BufferView";
    Utf8String         fragmentShaderBufferView  = fragmentShader + "_BufferView";

    technique["program"] = programName.c_str();

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(Gltf::DepthTest);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";

    auto& rootProgramNode = (m_json["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");

    rootProgramNode["vertexShader"]   = vertexShader.c_str();
    rootProgramNode["fragmentShader"] = fragmentShader.c_str();

    auto& shaders = m_json["shaders"];
    AddShader(shaders, vertexShader.c_str(), Gltf::VertexShader, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), Gltf::FragmentShader, fragmentShaderBufferView.c_str());

    AddBufferView(vertexShaderBufferView.c_str(),  vertexShaderString.c_str(), vertexShaderString.size());
    AddBufferView(fragmentShaderBufferView.c_str(), fragmentShaderString.c_str(), fragmentShaderString.size());
    
    if (textured)
        {
        AddTechniqueParameter(technique, "tex", Gltf::DataType::Sampler2d, "_3DTILESDIFFUSE");
        AddTechniqueParameter(technique, "texc", Gltf::DataType::FloatVec2, "TEXCOORD_0");
        technique["uniforms"]["u_tex"] = "tex";
        technique["attributes"]["a_texc"] = "texc";
        AppendProgramAttribute(rootProgramNode, "a_texc");
        }

    m_json["techniques"][techniqueName.c_str()] = technique;

    return techniqueName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTextureSampler(Utf8StringCR sampler, TileTextureCR texture)
    {
    if (!m_json.isMember("samplers"))
        m_json["samplers"] = Json::objectValue;

    if (!m_json["samplers"].isMember(sampler))
        {
        m_json["samplers"][sampler] = Json::objectValue;
        m_json["samplers"][sampler]["minFilter"] = Gltf::Linear;
        m_json["samplers"][sampler]["magFilter"] = Gltf::Linear;
        if (!texture.m_repeat)
            {
            m_json["samplers"][sampler]["wrapS"] = Gltf::ClampToEdge;
            m_json["samplers"][sampler]["wrapT"] = Gltf::ClampToEdge;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddTextureImage (TileTextureCR texture, Utf8StringCR idStr)
    {
    Render::ImageSourceCR   imageSource = texture.m_imageSource;
    bool                    hasAlpha = imageSource.GetFormat() == ImageSource::Format::Png;

    Utf8String  textureId = Utf8String ("texture_") + idStr;
    Utf8String  imageId   = Utf8String ("image_")   + idStr;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + idStr;
    Utf8String  samplerName = texture.m_repeat ? "sampler_0" : "sampler_2";

    m_json["textures"][textureId] = Json::objectValue;
    m_json["textures"][textureId]["format"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    m_json["textures"][textureId]["internalFormat"] = hasAlpha ? static_cast<int32_t>(Gltf::DataType::Rgba) : static_cast<int32_t>(Gltf::DataType::Rgb);
    m_json["textures"][textureId]["sampler"] = samplerName;
    m_json["textures"][textureId]["source"] = imageId;

    AddTextureSampler(samplerName, texture);

    m_json["images"][imageId] = Json::objectValue;


    m_json["bufferViews"][bvImageId] = Json::objectValue;
    m_json["bufferViews"][bvImageId]["buffer"] = "binary_glTF";


    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = imageSource.GetFormat() == ImageSource::Format::Png ? "image/png" : "image/jpeg";

    Render::Texture::Dimensions     dimensions = texture.GetDimensions();
    if (0 == dimensions.width || 0 == dimensions.height)
        {
        Image       image (imageSource, hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);
        
        dimensions.width = image.GetWidth();
        dimensions.height = image.GetHeight();
        }

    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"]  = dimensions.width;
    m_json["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = dimensions.height;

    ByteStream const& imageData = imageSource.GetByteStream();
    m_json["bufferViews"][bvImageId]["byteOffset"] = static_cast<uint32_t>(BinaryDataSize());
    m_json["bufferViews"][bvImageId]["byteLength"] = static_cast<uint32_t>(imageData.size());
    AddBinaryData (imageData.data(), imageData.size());

    return textureId;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     CreateMaterialJson(MeshMaterialCR material, Utf8StringCR idStr)
    { 
    Json::Value         json = Json::objectValue, values = Json::objectValue;
    
    if (material.GetTexture().IsValid())
        values["tex"] = AddTextureImage(*material.GetTexture(), idStr);

    json["technique"] = AddMeshShaderTechnique(idStr, s_unlitTextureVertexShader.c_str(), s_unlitTextureFragmentShader.c_str(), material.GetTexture().IsValid());
    json["values"] = values;
    return json; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, TriMeshArgs const& meshArgs, MeshMaterial const& meshMaterial, Utf8StringCR idStr)
    {
    primitiveJson["mode"] = static_cast<int32_t>(Gltf::PrimitiveType::Triangles);

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(meshArgs.m_points, meshArgs.m_numPoints, meshArgs.m_pointParams, "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    bool isTextured = meshMaterial.IsTextured();
    if (nullptr != meshArgs.m_textureUV && isTextured)
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (meshArgs.m_textureUV, meshArgs.m_numPoints, "Param", idStr.c_str());
    if (meshArgs.m_colors.m_numColors > 1)
        AddColors(primitiveJson, meshArgs.m_colors, meshArgs.m_numPoints, idStr);
    if (nullptr != meshArgs.m_normals && !meshMaterial.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"] = AddNormals(meshArgs.m_normals, meshArgs.m_numPoints, "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshIndices ("Indices", (uint32_t const*) meshArgs.m_vertIndex, meshArgs.m_numIndices, idStr, meshArgs.m_numPoints);
    AddMeshPointRange(m_json["accessors"][accPositionId], meshArgs.m_pointParams.GetRange());

    return SUCCESS;
    }
                                                                                     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTriMesh(Json::Value& primitivesNode, TriMeshArgsCR meshArgs, ColorTableCR colorTable, MeshMaterial const& meshMaterial, size_t& index)
    {
    if (0 == meshArgs.m_numIndices)
        return;

    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         primitiveJson;

    if (SUCCESS == CreateTriMesh(primitiveJson, meshArgs, meshMaterial, idStr))
        {
        m_json["materials"][meshMaterial.GetName()] = CreateMaterialJson(meshMaterial, idStr);
        primitiveJson["material"] = meshMaterial.GetName();
        primitivesNode.append(primitiveJson);
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginBatchedModel(uint32_t& startPosition, uint32_t& lengthDataPosition, Render::FeatureTableCR featureTable)
    {
    Utf8String          batchTableStr = BatchTableBuilder (featureTable, m_model.GetDgnDb(), m_model.Is3d()).ToString();
    Json::Value         featureTableJson;

    featureTableJson["BATCH_LENGTH"] = static_cast<uint32_t>(featureTable.size());
    Utf8String      featureTableStr = getJsonString(featureTableJson);

    startPosition = m_buffer.GetSize();
    m_buffer.Append(Format::B3dm);
    m_buffer.Append(B3dm::Version);                                                          
    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);                                                      // total length - filled in later.
    m_buffer.Append(static_cast<uint32_t>(featureTableStr.size()));                     // feature table JSon length.
    m_buffer.Append((uint32_t) 0);                                                      // length of binary portion of feature table (zero).
    m_buffer.Append(static_cast<uint32_t>(batchTableStr.size()));                       // batch table JSon length.
    m_buffer.Append((uint32_t) 0);                                                      // length of binary portion of batch table (zero)
    m_buffer.Append((const uint8_t *) featureTableStr.data(), featureTableStr.size());  // Feature table Json.
    m_buffer.Append((const uint8_t *) batchTableStr.data(), batchTableStr.size());      // Batch table Json.

    PadToBoundary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void EndBatchedModel(uint32_t startPosition, uint32_t lengthDataPosition)
    {
    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  WriteBatchedModel(Render::FeatureTableCR featureTable)
    {
    uint32_t       startPosition = 0, lengthDataPosition = 0;

    BeginBatchedModel(startPosition, lengthDataPosition, featureTable);
    WriteGltf();
    EndBatchedModel(startPosition, lengthDataPosition);

    return SUCCESS;
    }


};  // CesiumTileWriter


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct RenderSystem : Render::System
{
    static  constexpr uint32_t          s_maxFeatures = 0xffff;

    mutable StreamBuffer                    m_streamBuffer;
    mutable CesiumTileWriter                m_writer;
    mutable Json::Value                     m_primitives;
    mutable DRange3d                        m_range;
    mutable FeatureTable                    m_featureTable;
    mutable bvector<PointCloudData>         m_pointClouds;

    RenderSystem(GeometricModelR model) : m_writer(m_streamBuffer, model), m_range(DRange3d::NullRange()), m_featureTable(model.GetModelId(), s_maxFeatures)  { }

    MaterialPtr _FindMaterial(MaterialKeyCR, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _GetMaterial(RenderMaterialId, DgnDbR) const override { return nullptr; }
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    TexturePtr _FindTexture(TextureKeyCR, DgnDbR) const override { return nullptr; }
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { return nullptr; }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual Render::TargetPtr _CreateOffscreenTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return new Graphic(dgndb); }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return s_maxFeatures; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, DgnDbR, Texture::CreateParams const& params)  const override {return new TileTexture(image, params); }
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, DgnDbR, Texture::CreateParams const& params)  const override {return new TileTexture(source, bottomUp, params); }
    virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override  { return nullptr; }
    virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const override { return nullptr; }
    virtual bool _DoCacheTiles() const override { return false; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& featureTable, DRange3dCR range) const override 
    {
//  m_featureTable = std::move(featureTable);
    m_range = range;
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual MaterialPtr _CreateMaterial(Material::CreateParams const&, DgnDbR) const override
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR triMesh, DgnDbR db) const override 
    {
    ColorTable      colorTable;
    size_t          index = (size_t) m_primitives.size();
    Utf8String      idStr(std::to_string(index).c_str());
    MeshMaterial    meshMaterial(idStr, triMesh.m_material.get(), triMesh.m_texture.get());

    m_range.Extend(triMesh.m_pointParams.GetRange());
    m_writer.AddTriMesh(m_primitives, triMesh, colorTable, meshMaterial, index); 

    return new Graphic(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR pointCloud, DgnDbR db)  const override 
    {
    m_range.Extend(pointCloud.m_qParams.GetRange());
    m_pointClouds.push_back(pointCloud);
    return new Graphic(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WriteTile(PublishedTileR outputTile)
    {
    if (m_range.IsNull())
        return WriteStatus::NoGeometry;

    outputTile.SetPublishedRange(m_range);
    outputTile.SetExtension(m_pointClouds.empty() ? "b3dm" : "pnts");

    // Either pointClouds or (mesh) primitives - not both.
    return m_pointClouds.empty() ? WriteMeshTile(outputTile) : WritePointCloudTile(outputTile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WriteMeshTile(PublishedTileR outputTile)
    {
    BeAssert (m_pointClouds.empty());

    m_writer.AddPrimitivesJson(m_primitives);
    m_writer.WriteBatchedModel(m_featureTable);

    BeFile  outputFile;

   if (BeFileStatus::Success != outputFile.Create (outputTile.GetFileName()) ||
       BeFileStatus::Success != outputFile.Write (nullptr, m_streamBuffer.data(), m_streamBuffer.size()))
       return WriteStatus::UnableToOpenFile;
   
   outputFile.Close();
   return WriteStatus::Success;
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WritePointCloudTile(PublishedTileR outputTile)
    {
    BeAssert (m_primitives.empty());
    BeAssert (m_pointClouds.size() == 1);       // Needs work to handle multiple clouds in a single tile.
    BeFile  outputFile;

   if (BeFileStatus::Success != outputFile.Create (outputTile.GetFileName()))
       return WriteStatus::UnableToOpenFile;
   
    Json::Value     featureTable;
    auto            pointCloud = m_pointClouds.front();
    auto            paramRange = pointCloud.m_qParams.GetRange();
    auto            paramRangeDiagonal = paramRange.DiagonalVector();
    size_t          nPoints = pointCloud.m_points.size();

    featureTable["POINTS_LENGTH"] = static_cast<uint32_t>(nPoints);
    featureTable["POSITION_QUANTIZED"]["byteOffset"] = 0;

    featureTable["QUANTIZED_VOLUME_OFFSET"].append(paramRange.low.x);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(paramRange.low.y);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(paramRange.low.z);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(paramRangeDiagonal.x);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(paramRangeDiagonal.y);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(paramRangeDiagonal.z);

    size_t pointBytes =  nPoints * sizeof(QPoint3d);
    if (!pointCloud.m_colors.empty())
        featureTable["RGB"]["byteOffset"] = static_cast<uint32_t>(pointBytes);

    Utf8String      featureTableStr =  getJsonString(featureTable);
    uint32_t        featureTableStrLen = featureTableStr.size(); 
    uint32_t        binaryLength = pointBytes + (pointCloud.m_colors.empty() ? 0 : (3 * nPoints));
    uint32_t        magic = (uint32_t) Format::PointCloud, version = (uint32_t) PointCloud::Versions::Version;
    uint32_t        totalSize = binaryLength + featureTableStrLen + 28, zero = 0;

    outputFile.Write(nullptr, &magic, sizeof(magic));
    outputFile.Write(nullptr, &version, sizeof(version));
    outputFile.Write(nullptr, &totalSize, sizeof(totalSize));
    outputFile.Write(nullptr, &featureTableStrLen, sizeof(featureTableStrLen));
    outputFile.Write(nullptr, &binaryLength, sizeof(binaryLength));
    outputFile.Write(nullptr, &zero, sizeof(zero));     // Batch JSON.
    outputFile.Write(nullptr, &zero, sizeof(zero));     // Batch Binary.
    outputFile.Write(nullptr, featureTableStr.c_str(), featureTableStrLen);
    outputFile.Write(nullptr, pointCloud.m_points.data(), pointBytes);
    if (!pointCloud.m_colors.empty())
        outputFile.Write(nullptr, pointCloud.m_colors.data(), 3 * nPoints);

   outputFile.Close();
   return WriteStatus::Success;
   }

};  // RenderSystem

   
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct Context
{
    TileTree::TilePtr   m_inputTile;
    PublishedTilePtr    m_outputTile;
    ICesiumPublisherP   m_publisher;
    double              m_leafTolerance;
    GeometricModelP     m_model;
    ClipVectorPtr       m_clip;
    std::shared_ptr<BeFolly::LimitingTaskQueue<BentleyStatus>> m_requestTileQueue;


    Context(TileTree::TileP inputTile, PublishedTileP outputTile, ClipVectorCP clip, ICesiumPublisher* publisher, double leafTolerance, GeometricModelP model, std::shared_ptr<BeFolly::LimitingTaskQueue<BentleyStatus>> requestTileQueue) : 
            m_inputTile(inputTile), m_outputTile(outputTile), m_publisher(publisher), m_leafTolerance(leafTolerance), m_model(model), m_requestTileQueue(requestTileQueue) { if (nullptr != clip) m_clip = clip->Clone(nullptr); }

    Context(TileTree::TileP inputTile, PublishedTileP outputTile, Context const& inContext) :
            m_inputTile(inputTile), m_outputTile(outputTile), m_clip(inContext.m_clip), m_publisher(inContext.m_publisher), m_leafTolerance(inContext.m_leafTolerance), m_model(inContext.m_model), m_requestTileQueue(inContext.m_requestTileQueue) { }


};

typedef folly::Future<WriteStatus> FutureWriteStatus;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTile::PublishedTile(TileTree::TileCR inputTile, BeFileNameCR outputDirectory) : m_publishedRange(DRange3d::NullRange()), m_outputDirectory(outputDirectory)
    {
    m_name = inputTile._GetTileCacheKey().c_str();

    m_name.ReplaceAll(":", "_");
    m_name.ReplaceAll(".", "_");
    m_name.ReplaceAll("/", "_");

    m_tileRange = inputTile.GetRange();
    m_tolerance = 0.0 == inputTile._GetMaximumSize() ? 1.0E8 : m_tileRange.DiagonalDistance() / inputTile._GetMaximumSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PublishedTile::GetURL() const
    {
    BeAssert (!m_extension.empty());
    return m_name + Utf8String(".") + m_extension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName PublishedTile::GetFileName() const
    {
    BeAssert (!m_extension.empty());
    return BeFileName(nullptr, m_outputDirectory.c_str(), WString(m_name.c_str(), true).c_str(), WString(m_extension.c_str(), true).c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<BentleyStatus> requestTile(Context context, std::shared_ptr<TileTree::IO::RenderSystem> renderSystem)
{
    if (context.m_inputTile->IsNotLoaded())
        {
        TileTree::TileLoadStatePtr loadState;
        return context.m_requestTileQueue->Push([=]()
            {
            return context.m_inputTile->GetRootR()._RequestTile(*context.m_inputTile, loadState, renderSystem.get(), BeDuration());
            });
        }
        
    return SUCCESS;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static FutureWriteStatus generateParentTile (Context context)
    {
    std::shared_ptr<TileTree::IO::RenderSystem> renderSystem = std::make_shared<RenderSystem>(*context.m_model);

    return requestTile(context, renderSystem).then([=](BentleyStatus status)
        {
        if (SUCCESS != status)
            return folly::makeFuture(WriteStatus::UnableToLoadTile);
        
        WriteStatus writeStatus = renderSystem->WriteTile(*context.m_outputTile);

        return folly::makeFuture(writeStatus);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017 
+---------------+---------------+---------------+---------------+---------------+------*/
static FutureWriteStatus generateChildTiles (WriteStatus parentStatus, Context context)
    {
    double      parentTolerance = context.m_outputTile->GetTolerance();

    if (parentTolerance < context.m_leafTolerance)
        return folly::makeFuture(WriteStatus::Success);

    auto const& children = context.m_inputTile->_GetChildren(true);

    if (nullptr == children)
        return folly::makeFuture(parentStatus);

    std::vector<FutureWriteStatus> childFutures;

    for (auto& child : *children)
        {
        PublishedTilePtr    publishedTile = new PublishedTile(*child, context.m_publisher->_GetOutputDirectory(*context.m_model));
        Context             childContext(child.get(), publishedTile.get(), context);

        context.m_outputTile->GetChildren().push_back(publishedTile);
        auto childFuture = generateParentTile(childContext).then([=](FutureWriteStatus result) { return generateChildTiles(result.value(), childContext); });
        childFutures.push_back(std::move(childFuture));
        }

    return folly::unorderedReduce(childFutures, WriteStatus::Success, [=](WriteStatus reduced, WriteStatus next)
        {
        return WriteStatus::Aborted == reduced || WriteStatus::Aborted == next ? WriteStatus::Aborted : WriteStatus::Success;
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
FutureWriteStatus writeCesiumTileset(ICesiumPublisher* publisher, GeometricModelP model, double leafTolerance)
    {
    auto                            renderSystem = std::make_shared<RenderSystem>(*model);
    TileTree::RootCPtr              tileRoot = model->GetTileTree(renderSystem.get());
    ClipVectorPtr                   clip;
    PublishedTilePtr                publishedRoot = new PublishedTile(*tileRoot->GetRootTile(), publisher->_GetOutputDirectory(*model));

    if (!tileRoot.IsValid())
        return folly::makeFuture(WriteStatus::NoGeometry);

    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [=]()
        {
        return publisher->_BeginProcessModel(*model);
        })
    .then([=](WriteStatus status)
        {
        if (WriteStatus::Success != status)
            return folly::makeFuture(status);

        TileTree::TilePtr   inputTile = tileRoot->GetRootTile();
        auto                requestTileQueue = std::make_shared<BeFolly::LimitingTaskQueue<BentleyStatus>>(BeFolly::ThreadPool::GetIoPool(), 20);

        Context             context(inputTile.get(), publishedRoot.get(), clip.get(), publisher, leafTolerance, model, requestTileQueue);
                                                                                                                
        return generateParentTile(context).then([=](FutureWriteStatus status) { return generateChildTiles(status.value(), context); });
        })
    .then([=](FutureWriteStatus status)
        {
        auto pRenderSys = renderSystem.get();
        UNUSED_VARIABLE(pRenderSys);

        return folly::makeFuture(publisher->_EndProcessModel(*model, tileRoot->GetLocation(), *publishedRoot, status.value()));   
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bewntley    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus ICesiumPublisher::PublishCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance)
    {
    return TileTree::IO::writeCesiumTileset(&publisher, const_cast<GeometricModelP> (&model), leafTolerance).get();
    }


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/2017
//=======================================================================================
struct CesiumTilesetPublisher: ICesiumPublisher
{
public:
    CesiumTilesetPublisher(BeFileName outputFileName, BeFileNameCR outputDirectory, TransformCR dbToEcef) : m_outputFileName(outputFileName), m_outputDirectory(outputDirectory), m_dbToEcef(dbToEcef) { }

protected:
    BeFileName      m_outputDirectory;
    BeFileName      m_outputFileName;
    Transform       m_dbToEcef;


    BeFileName  _GetOutputDirectory(GeometricModelCR model) const override { return m_outputDirectory; }

    static void AppendPoint(Json::Value& val, DPoint3dCR pt) { val.append(pt.x); val.append(pt.y); val.append(pt.z); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBoundingVolume(Json::Value& val, DRange3dCR range)
    {
    BeAssert (!range.IsNull());
    DPoint3d    center = DPoint3d::FromInterpolate (range.low, .5, range.high);
    DVec3d      diagonal = range.DiagonalVector();

    // Range identified by center point and axes
    // Axes are relative to origin of box, not center
    static double      s_minSize = .001;   // Meters...  Don't allow degenerate box.

    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    AppendPoint(box, center);
    AppendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, diagonal.x)/2.0, 0.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, diagonal.y)/2.0, 0.0));
    AppendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, diagonal.z/2.0)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteModelMetadataTree (DRange3dR range, Json::Value& root, PublishedTileCR tile, GeometricModelCR model)
    {
    if (tile.GetIsEmpty() && tile.GetChildren().empty())
        {
        range = DRange3d::NullRange();
        return;
        }

    WString         rootName = TileUtil::GetRootNameForModel(model.GetModelId(), false);
    DRange3d        contentRange, publishedRange = tile.GetPublishedRange();

    
    // the published range represents the actual range of the published meshes. - This may be smaller than the 
    // range estimated when we built the tile tree. -- However we do not clip the meshes to the tile range.
    // so start the range out as the intersection of the tile range and the published range.
    range = contentRange = DRange3d::FromIntersection (tile.GetTileRange(), publishedRange, true);

    if (!tile.GetChildren().empty())
        {
        root[JSON_Children] = Json::arrayValue;

        // Append children to this tileset.
        for (auto& childTile : tile.GetChildren())
            {
            Json::Value         child;
            DRange3d            childRange;

            WriteModelMetadataTree (childRange, child, *childTile, model);
            if (!childRange.IsNull())
                {
                root[JSON_Children].append(child);
                range.Extend (childRange);
                }
            }
        }
    if (range.IsNull())
        return;

    root["refine"] = "REPLACE";
    root[JSON_GeometricError] = tile.GetTolerance();

    WriteBoundingVolume (root, tile.GetTileRange());
    
    if (!tile.GetIsEmpty())
        {
        // TBD.  ContentRange.
        root[JSON_Content]["url"] = tile.GetURL();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value TransformToJson(TransformCR tf)
    {
    auto matrix = DMatrix4d::From(tf);
    Json::Value json(Json::arrayValue);
    for (size_t i=0;i<4; i++)
        for (size_t j=0; j<4; j++)
            json.append (matrix.coff[j][i]);


    return json;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus WriteTileset (GeometricModelCR model, TransformCR tileToDb, PublishedTileCR rootTile)
    {
    Json::Value val, modelRoot;

    val["asset"]["version"] = "0.0";
    val["asset"]["gltfUpAxis"] = "Z";
 
    DRange3d    rootRange;
    WriteModelMetadataTree (rootRange, modelRoot, rootTile, model);

    val[JSON_Root] = std::move(modelRoot);
    val[JSON_Root][JSON_Transform] = TransformToJson(Transform::FromProduct(m_dbToEcef, tileToDb));

    return SUCCESS == TileUtil::WriteJsonToFile (m_outputFileName.c_str(), val) ? WriteStatus::Success : WriteStatus::UnableToWriteFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus InitializeDirectories()
    {
    return BeFileName::DoesPathExist(m_outputDirectory) ? BeFileName::EmptyDirectory (m_outputDirectory) :  BeFileName::CreateNewDirectory(m_outputDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus _BeginProcessModel(GeometricModelCR model) override
    {
    return (BeFileNameStatus::Success == InitializeDirectories()) ? WriteStatus::Success : WriteStatus::UnableToOpenFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WriteStatus _EndProcessModel(GeometricModelCR model, TransformCR tileToDb, PublishedTileCR rootTile, WriteStatus status) override
    {
    if (WriteStatus::Success != status)
        {
        BeFileName::EmptyAndRemoveDirectory (m_outputDirectory);
        return status;
        }
    return WriteTileset(model, tileToDb, rootTile);
    }
};  // CesiumTilesetPublisher.


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::IO::WriteStatus ICesiumPublisher::WriteCesiumTileset(BeFileName outputFileName, BeFileNameCR tileOutputDirectory, GeometricModelCR model, TransformCR dbToEcef, double leafTolerance) 
    {   
    CesiumTilesetPublisher publisher(outputFileName, tileOutputDirectory, dbToEcef);

    return ICesiumPublisher::PublishCesiumTileset(publisher, model, dbToEcef, leafTolerance);
    }    
END_TILETREE_IO_NAMESPACE

