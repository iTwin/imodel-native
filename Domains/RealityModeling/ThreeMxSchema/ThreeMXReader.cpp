/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMXReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include "OpenCTM/openctm.h"

#define LOG_ERROR(...) {BeAssert(false); (*NativeLogging::LoggingManager::GetLogger(L"3MX")).errorv(__VA_ARGS__);}

BEGIN_UNNAMED_NAMESPACE
USING_NAMESPACE_TILETREE

static Utf8String GetMagicString() {return "3MXBO";}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/16
//=======================================================================================
struct CtmContext
{
    void* m_openCTM;

    static bool ReadBytes(StreamBuffer& in, void* buf, uint32_t size)
        {
        ByteCP start = in.GetCurrent();
        if (nullptr == in.Advance(size)) {BeAssert(false); return false;}
        memcpy(buf, start, size);
        return true;
        }

    static CTMuint CTMCALL ReadFunc(void* buf, CTMuint count, void* userData) {return ReadBytes(*(StreamBuffer*)userData, buf, count) ? count : 0;}

    CtmContext(StreamBuffer& in, uint32_t offset) {m_openCTM=::ctmNewContext(CTM_IMPORT); in.SetPos(offset); ::ctmLoadCustom(m_openCTM, ReadFunc, &in);}
    ~CtmContext() {::ctmFreeContext(m_openCTM);}

    CTMenum GetError() {return ::ctmGetError(m_openCTM);}
    uint32_t GetInteger(CTMenum val) {return ::ctmGetInteger(m_openCTM, val);}
    FPoint3d const* GetFloatArray(CTMenum val) {return (FPoint3d const*) ::ctmGetFloatArray(m_openCTM, val);}
    int32_t const* GetIntegerArray(CTMenum val){return (int32_t const*) ::ctmGetIntegerArray(m_openCTM, val);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool readVectorEntry(JsonValueCR pt, Utf8CP tag, bvector<Utf8String>& v)
    {
    JsonValueCR entry = pt[tag];
    if (!entry.isArray())
        return false;

    for (Json::ArrayIndex i = 0; i < entry.size(); ++i)
        v.push_back(entry[i].asCString());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void dPoint3dFromJson(DPoint3dR point, JsonValueCR inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::ReadHeader(JsonValueCR pt, Utf8String& name, bvector<Utf8String>& nodeResources)
    {
    if (name.empty())
        {
        LOG_ERROR("No node id");
        return false;
        }

    JsonValueCR bbMin=pt["bbMin"];
    if (bbMin.size() != 3)
        {
        LOG_ERROR("Malformed bbMin");
        return false;
        }

    JsonValueCR bbMax=pt["bbMax"];
    if (bbMax.size() != 3)
        {
        LOG_ERROR("Malformed bbMax");
        return false;
        }

    dPoint3dFromJson(m_range.low, bbMin);
    dPoint3dFromJson(m_range.high, bbMax);

    JsonValueCR val = pt["maxScreenDiameter"];
    if (val.empty())
        {
        LOG_ERROR("Cannot find \"maxScreenDiameter\" entry");
        return false;
        }

    m_maxDiameter = val.asDouble();
    if (!readVectorEntry(pt, "resources", nodeResources))
        {
        LOG_ERROR("Cannot find \"resources\" entry");
        return false;
        }

    bvector<Utf8String> children;
    if (!readVectorEntry(pt, "children", children))
        return false;

    BeAssert(children.size() <= 1);

    if (1 == children.size())
        m_childPath = children[0];

    if (m_parent)
        m_parent->ExtendRange(m_range);

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Node::DoRead(StreamBuffer& in, SceneR scene, Dgn::Render::SystemP renderSys)
    {
    BeAssert(IsQueued());
    m_loadStatus.store(LoadStatus::Loading);

    BeAssert(m_children.empty());

    bmap<Utf8String, int> textureIds, nodeIds;
    bmap<Utf8String, Utf8String> geometryNodeCorrespondence;
    int nodeCount = 0;

    uint32_t magicSize = (uint32_t) GetMagicString().size();
    ByteCP currPos = in.GetCurrent();
    if (!in.Advance(magicSize))
        {
        LOG_ERROR("Can't read magic number");
        return ERROR;
        }

    Utf8String magicNumber((Utf8CP) currPos, (Utf8CP) in.GetCurrent());
    if (magicNumber != GetMagicString())
        {
        LOG_ERROR("wrong magic number");
        return ERROR;
        }

    uint32_t infoSize;
    if (!CtmContext::ReadBytes(in, &infoSize, 4))
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

            NodePtr nodeptr  = new Node(GetRootR(), this);

            if (!nodeptr->ReadHeader(node, nodeName, nodeResources))
                return ERROR;

            nodeIds[nodeName] = nodeCount++;
            for (size_t i = 0; i < nodeResources.size(); ++i)
                geometryNodeCorrespondence[nodeResources[i]] = nodeName;

            m_children.push_back(nodeptr);
            }
        }

    Utf8String resourceType, resourceFormat, resourceName;
    uint32_t resourceSize;
    uint32_t offset = (uint32_t) GetMagicString().size() + 4 + infoSize;

    JsonValueCR resources = pt["resources"];
    if (resources.empty())
        return SUCCESS;

    bmap<Utf8String, Dgn::Render::TexturePtr> renderTextures;
    for (JsonValueCR resource : resources)
        {
        resourceType   = resource.get("type", "").asCString();
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
                LOG_ERROR("Cannot read texture data");
                return ERROR;
                }

            ImageSource jpeg(ImageSource::Format::Jpeg, ByteStream(buffer, resourceSize));
            renderTextures[resourceName] = scene._CreateTexture(jpeg, Image::BottomUp::Yes);
            }
        }

    offset = (uint32_t) GetMagicString().size() + 4 + infoSize;
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
                LOG_ERROR("Node name is unknown");
                return ERROR;
                }

            CtmContext ctm(in, thisOffset);
            if (CTM_NONE != ctm.GetError())
                {
                LOG_ERROR("CTM read error: %s", ctmErrorString(ctm.GetError()));
                return ERROR;
                }

            uint32_t textureCoordsArrays = ctm.GetInteger(CTM_UV_MAP_COUNT);
            if (textureCoordsArrays != 1)
                continue;

            Utf8String texName = resource.get("texture", Json::Value("")).asCString();
            if (texName.empty())
                continue;

            Geometry::CreateParams geomParams;
            geomParams.m_numPoints  = ctm.GetInteger(CTM_VERTEX_COUNT);
            geomParams.m_points     = ctm.GetFloatArray(CTM_VERTICES);
            geomParams.m_normals    = (ctm.GetInteger(CTM_HAS_NORMALS) == CTM_TRUE) ? ctm.GetFloatArray(CTM_NORMALS) : nullptr;
            geomParams.m_numIndices = 3 * ctm.GetInteger(CTM_TRIANGLE_COUNT);
            geomParams.m_vertIndex  = ctm.GetIntegerArray(CTM_INDICES);
            geomParams.m_textureUV  = (FPoint2d const*) ctm.GetFloatArray(CTM_UV_MAP_1);

            auto texture = renderTextures.find(texName);
            geomParams.m_texture = (texture == renderTextures.end()) ? nullptr : texture->second;

            ((Node*)m_children[nodeId->second].get())->m_geometry.push_front(scene._CreateGeometry(geomParams, m_children[nodeId->second]->GetRange(), renderSys));
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::Read3MXB(StreamBuffer& in, SceneR scene, Dgn::Render::SystemP renderSys)
    {
    BeAssert(!IsReady());

    if (SUCCESS != DoRead(in, scene, renderSys))
        {
        SetNotFound();
        BeAssert(false);
        return ERROR;
        }

    // only after we've successfully read the entire node, mark it as ready so other threads can look at its child nodes.
    SetIsReady();
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus SceneInfo::Read(StreamBuffer& buffer) 
    {
    // If we weren't able to read the file...
    if (0 == buffer.GetSize())
        return ERROR;

    Json::Value val;
    Json::Reader reader;
    Utf8CP buffStr = (Utf8CP) buffer.GetData();
    if (!reader.parse(buffStr, buffStr+buffer.GetSize(), val))
        {
        LOG_ERROR("Cannot parse info");
        return ERROR;
        } 

    int version = val.get("3mxVersion", 0).asInt();
    if (version != 1)
        {
        LOG_ERROR("Unsupported version of 3MX");
        return ERROR;
        }

    m_sceneName = val.get("name", "").asCString();
    m_logo = val.get("logo", "").asCString();

    JsonValueCR layers = val["layers"];
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
            JsonValueCR orgJson = layer["SRSOrigin"];
            if (!orgJson.empty())
                dPoint3dFromJson(m_origin, orgJson);
            }
        return SUCCESS;
        }

    // we didn't find a mesh pyramid
    return ERROR;
    }
