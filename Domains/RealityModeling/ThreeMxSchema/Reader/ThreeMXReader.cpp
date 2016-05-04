/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/Reader/ThreeMXReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"
#include <DgnPlatform/JsonUtils.h>

#include "openctm/openctm.h"

#define LOG_ERROR (*NativeLogging::LoggingManager::GetLogger(L"3MX")).errorv

BEGIN_UNNAMED_NAMESPACE
struct ThreeMX
    {
    static Utf8String GetMagicNumber() { return "3MXBO"; }
    static Utf8String GetSceneExtension() { return "3mx"; }
    static Utf8String GetNodeExtension() { return "3mxb"; }
    static Utf8String GetSceneVersionTag() { return "3mxVersion"; }
    static Utf8String GetLongName(){ return "Smart3DCapture 3MX"; }
    static Utf8String GetDescription(){ return "Smart3DCapture 3D Multiresolution Mesh Exchange Format"; }
    static Utf8String GetShortName(){ return "3MX"; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool readBytes(MxStreamBuffer& in, void* buf, uint32_t size)
    {
    ByteCP start = in.GetCurrent();
    ByteCP end   = in.Advance(size);
    if (nullptr == end)
        {
        BeAssert(false);
        return false;
        }

    memcpy(buf, start, size);
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CTMuint ctmReadFunc(void* buf, CTMuint count, void* userData)
    {
    return readBytes(*(MxStreamBuffer*)userData, buf, count) ? count : 0;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/16
//=======================================================================================
struct CtmContext
{
    void* m_openCTM;

    CtmContext(MxStreamBuffer& in, uint32_t offset) {m_openCTM=ctmNewContext(CTM_IMPORT); in.SetPos(offset); ctmLoadCustom(m_openCTM, (CTMreadfn)ctmReadFunc, &in);}
    ~CtmContext() {ctmFreeContext(m_openCTM);}

    CTMenum GetError() {return ctmGetError(m_openCTM);}
    uint32_t GetInteger(CTMenum val) {return ctmGetInteger(m_openCTM, val);}
    FPoint3d const* GetFloatArray(CTMenum val) {return (FPoint3d const*)ctmGetFloatArray(m_openCTM, val);}
    int32_t const* GetIntegerArray(CTMenum val){return (int32_t const*) ctmGetIntegerArray(m_openCTM, val);}
};

template <typename T> T getJsonValue(JsonValueCR pt);
template<> double getJsonValue(JsonValueCR pt) {return pt.asDouble();}
template<> size_t getJsonValue(JsonValueCR pt) {return pt.asUInt();}
template<> Utf8String getJsonValue(JsonValueCR pt) {return pt.asCString();}

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool readVectorEntry(JsonValueCR pt, Utf8StringCR tag, bvector<T>& v)
    {
    Json::Value entry = pt[tag.c_str()];
    if (!entry.isArray())
        return false;

    for (Json::ArrayIndex i = 0; i < entry.size(); ++i)
        v.push_back(getJsonValue<T>(entry[i]));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void dPoint3dFromJson(DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }
END_UNNAMED_NAMESPACE

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::ReadHeader(JsonValueCR pt, Utf8String& name, bvector<Utf8String>& nodeResources)
    {
    if (name.empty())
        {
        LOG_ERROR("No node id");
        return false;
        }

    Json::Value bbMin=pt["bbMin"];
    if (bbMin.size() != 3)
        {
        LOG_ERROR("Malformed bbMin");
        return false;
        }

    Json::Value bbMax=pt["bbMax"];
    if (bbMax.size() != 3)
        {
        LOG_ERROR("Malformed bbMax");
        return false;
        }

    dPoint3dFromJson(m_range.low, bbMin);
    dPoint3dFromJson(m_range.high, bbMax);

    m_center.Interpolate(m_range.low, .5, m_range.high);
    m_radius = 0.5 * m_range.low.Distance(m_range.high);

    Json::Value val = pt["maxScreenDiameter"];
    if (val.empty())
        {
        LOG_ERROR("Cannot find \"maxScreenDiameter\" entry");
        return false;
        }

    m_maxScreenDiameter = val.asDouble();
    if (!readVectorEntry(pt, "resources", nodeResources))
        {
        LOG_ERROR("Cannot find \"resources\" entry");
        return false;
        }

    bvector<Utf8String> children;
    if (!readVectorEntry(pt, "children", children))
        return false;

    if (1 == children.size())
        m_childPath = children[0];

    return true;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::Read3MXB(MxStreamBuffer& in, SceneR scene)
    {
    bmap<Utf8String, int> textureIds, nodeIds;
    bmap<Utf8String, Utf8String> geometryNodeCorrespondence;
    int nodeCount = 0;

    uint32_t magicSize = (uint32_t) ThreeMX::GetMagicNumber().size();
    ByteCP currPos = in.GetCurrent();
    if (!in.Advance(magicSize))
        {
        LOG_ERROR("Can't read magic number");
        return ERROR;
        }

    Utf8String magicNumber((Utf8CP) currPos, (Utf8CP) in.GetCurrent());
    if (magicNumber != ThreeMX::GetMagicNumber())
        {
        LOG_ERROR("wrong magic number");
        return ERROR;
        }

    uint32_t infoSize;
    if (!readBytes(in, &infoSize, 4))
        {
        LOG_ERROR("Can't read size");
        return ERROR;
        }

    Utf8P infoStr = (Utf8P) in.GetCurrent();
    Json::Value pt;
    Json::Reader reader;
    if (!reader.parse(infoStr, infoStr+infoSize, pt))
        {
        LOG_ERROR("Cannot parse info: ");
        return ERROR;
        }

    int version = pt.get("version", 0).asInt();
    if (version != 1)
        {
        LOG_ERROR("Unsupported version");
        return ERROR;
        }

    JsonValueCR nodes = pt["nodes"];
    if (!nodes.empty())
        {
        for (JsonValueCR node : nodes)
            {
            Utf8String nodeName;
            bvector<Utf8String> nodeResources;
            nodeName = node.get("id", "").asCString();

            NodePtr nodeptr  = new Node(this);

            if (!nodeptr->ReadHeader(node, nodeName, nodeResources))
                return ERROR;

            nodeIds[nodeName] = nodeCount++;
            for (size_t i = 0; i < nodeResources.size(); ++i)
                geometryNodeCorrespondence[nodeResources[i]] = nodeName;

            m_childNodes.push_back(nodeptr);
            }
        }

    if (nullptr == scene.m_renderSystem)
        return SUCCESS; // if we dont have a render system, we don't get geometry

    Utf8String resourceType, resourceFormat, resourceName;
    uint32_t resourceSize;
    uint32_t offset = (uint32_t) ThreeMX::GetMagicNumber().size() + 4 + infoSize;

    JsonValueCR resources = pt["resources"];
    if (resources.empty())
        return SUCCESS;

    bmap<Utf8String, Dgn::Render::TexturePtr> renderTextures;
    for (JsonValueCR resource : resources)
        {
        resourceType = resource.get("type", "").asCString();
        resourceFormat = resource.get("format", "").asCString();
        resourceName   = resource.get("id", "").asCString();
        resourceSize   = resource.get("size", 0).asUInt();

        uint32_t thisOffset = offset;
        offset += resourceSize;

        if (resourceType == "textureBuffer" && resourceFormat == "jpg" && !resourceName.empty() && resourceSize > 0)
            {
            in.SetPos(thisOffset);
            ByteCP buffer=in.GetCurrent();
            if (!in.Advance(resourceSize))
                {
                BeAssert(false);
                LOG_ERROR("Cannot read texture data");
                return ERROR;
                }

            ByteStream jpeg(buffer, resourceSize);
            RgbImageInfo imageInfo;
            Render::Image rgba;
            if (SUCCESS != imageInfo.ReadImageFromJpgBuffer(rgba, jpeg.GetData(), jpeg.GetSize()))
                {
                BeAssert(false);
                LOG_ERROR("bad texture data");
                return ERROR;
                }

            renderTextures[resourceName] = scene.m_renderSystem->_CreateTexture(rgba, imageInfo.m_hasAlpha);
            }
        }

    if (renderTextures.empty())
        return SUCCESS;

    offset = (uint32_t) ThreeMX::GetMagicNumber().size() + 4 + infoSize;
    for (JsonValueCR resource : resources)
        {
        resourceType   = resource.get("type", "").asCString();
        resourceFormat = resource.get("format", "").asCString();
        resourceName   = resource.get("id", "").asCString();
        resourceSize   = resource.get("size", 0).asUInt();

        uint32_t thisOffset = offset;
        offset += resourceSize;

        if (resourceType == "geometryBuffer" && resourceFormat == "ctm" && !resourceName.empty() && resourceSize > 0)
            {
            if (geometryNodeCorrespondence.find(resourceName) == geometryNodeCorrespondence.end())
                {
                LOG_ERROR("Geometry is not referenced by any node");
                return ERROR;
                }

            Utf8String nodeName = geometryNodeCorrespondence[resourceName];
            auto nodeId = nodeIds.find(nodeName);
            if (nodeId == nodeIds.end())
                {
                BeAssert(false);
                LOG_ERROR("Node name is unknown");
                return ERROR;
                }

            CtmContext ctm(in, thisOffset);
            if (CTM_NONE != ctm.GetError())
                {
                BeAssert(false);
                LOG_ERROR("CTM read error: %s", ctmErrorString(ctm.GetError()));
                return ERROR;
                }

            uint32_t textureCoordsArrays = ctm.GetInteger(CTM_UV_MAP_COUNT);
            if (textureCoordsArrays != 1)
                continue;

            Utf8String texName = resource.get("texture", Json::Value("")).asCString();
            if (texName.empty())
                continue;

            if (renderTextures.find(texName) == renderTextures.end())
                {
                BeAssert(false);
                LOG_ERROR("Bad texture name %s", texName.c_str());
                return ERROR;
                }

            Render::Graphic::TriMeshArgs trimesh;
            trimesh.m_numPoints  = ctm.GetInteger(CTM_VERTEX_COUNT);
            trimesh.m_points     = ctm.GetFloatArray(CTM_VERTICES);
            trimesh.m_normals    = (ctm.GetInteger(CTM_HAS_NORMALS) == CTM_TRUE) ? ctm.GetFloatArray(CTM_NORMALS) : nullptr;
            trimesh.m_numIndices = 3 * ctm.GetInteger(CTM_TRIANGLE_COUNT);
            trimesh.m_vertIndex  = ctm.GetIntegerArray(CTM_INDICES);
            trimesh.m_textureUV  = (FPoint2d const*) ctm.GetFloatArray(CTM_UV_MAP_1);
            trimesh.m_texture    = renderTextures[texName];

            BeAssert(!m_childNodes[nodeId->second]->m_geometry.IsValid());
            m_childNodes[nodeId->second]->m_geometry = new Geometry(trimesh, scene);
            }
        }

    for (auto& child : m_childNodes)
        {
        if (child->IsDisplayable() && !child->m_geometry.IsValid())
            {
            BeAssert(false);
            }
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SceneInfo::Read(MxStreamBuffer& buffer) 
    {
    Json::Value pt;
    Json::Reader reader;
    Utf8CP buffStr = (Utf8CP) buffer.GetData();
    if (!reader.parse(buffStr, buffStr+buffer.GetSize(), pt))
        {
        LOG_ERROR("Cannot parse info");
        return ERROR;
        } int version = pt.get(ThreeMX::GetSceneVersionTag().c_str(), 0).asInt();
    if (version != 1)
        {
        LOG_ERROR("Unsupported version of %s", ThreeMX::GetShortName().c_str());
        return ERROR;
        }

    m_sceneName = pt.get("name", "").asCString();

    JsonValueCR layers = pt["layers"];
    if (layers.empty())
        {
        LOG_ERROR("Cannot find \"layers\" tag");
        return ERROR;
        }

    for (auto& layer : layers)
        {
        Utf8String meshType = layer.get("type", "").asCString();
        if (meshType != "meshPyramid")
            continue;

        m_rootNodePath = layer.get("root", "").asCString();
        if (m_rootNodePath.empty())
            {
            LOG_ERROR("Could not find the root node path");
            return ERROR;
            }

        m_reprojectionSystem = layer.get("SRS", "").asCString();
        if (!m_reprojectionSystem.empty())
            {
            if (!readVectorEntry(layer, "SRSOrigin", m_origin))
                {
                LOG_ERROR("Could not find the SRS origin");
                return ERROR;
                }
            if (m_origin.size() != 3)
                {
                LOG_ERROR("Malformed SRS origin");
                return ERROR;
                }
            }
        return SUCCESS;
        }

    return ERROR;
    }
